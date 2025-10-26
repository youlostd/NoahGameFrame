#include "PythonWindow.h"
#include "StdAfx.h"

#include <optional>

#include "CColorPicker.h"
#include "PythonWindowManager.h"
#include "WindowType.hpp"

#include "../EterLib/RenderTargetManager.h"

#include "../CWebBrowser/WebManager.h"
#include "../EterBase/StepTimer.h"
#include "../EterLib/TextTag.h"

#include "../EterLib/GrpDevice.h"
#include "WindowUtil.hpp"
#include "base/Remotery.h"

constexpr bool g_bOutlineBoxEnable = false;

namespace UI
{
/**
 * \brief Base class for all game UI windows
 */
CWindow::CWindow()
    : m_HorizontalAlign(HORIZONTAL_ALIGN_LEFT), m_VerticalAlign(VERTICAL_ALIGN_TOP), m_x(0), m_y(0), m_width(0),
      m_height(0), m_bSingleAlpha(false), m_fSingleAlpha(1.0f), m_fWindowAlpha(1.0f), m_bAllAlpha(false),
      m_fAllAlpha(1.0f), m_clipRect({0, 0, 0, 0}), m_visible(false), m_flags(0), m_handler(nullptr), m_parent(nullptr)
{
    m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
    m_matScaling = Matrix::Identity;
}

CWindow::~CWindow() {}

DWORD CWindow::Type() { return kWindowNone; }

bool CWindow::IsType(DWORD dwType) { return OnIsType(dwType); }

bool CWindow::OnIsType(DWORD dwType)
{
    if (Type() == dwType)
        return true;

    return false;
}

struct FClear
{
    void operator()(CWindow *win) { win->Clear(); }
};

void CWindow::Clear()
{
    // FIXME: Children are not deleted immediately.
    // Destroy will be called one by one on the Python side anyway ...
    // But the link is broken just in case.
    // Is there a better form? -[levites]
    std::for_each(m_children.begin(), m_children.end(), FClear());
    m_children.clear();

    m_parent = nullptr;
    DestroyHandle();
    Hide();
}

void CWindow::DestroyHandle() { m_handler = nullptr; }

void CWindow::Show()
{
    if (IsFlag(FLAG_ANIMATED_BOARD))
    {
        this->m_visible = true;

        m_sizeAnimation = tweeny::from(0.0f)
                              .to(1.0f)
                              .during(280)
                              .via(tweeny::easing::backOut)
                              .onStep([this](tweeny::tween<float> &t, float scale) {
                                  if (t.progress() == 0.0f)
                                  {
                                      SetScale(0.0f, 0.0f);
                                      SetAllAlpha(0.0f);
                                  }

                                  SetScale(scale, scale);
                                  SetAllAlpha(scale);

                                  if (t.progress() == 1.0f)
                                  {
                                      SetScale(1.0f, 1.0f);
                                      SetAllAlpha(1.0f);

                                      return true;
                                  }
                                  else
                                  {
                                      return false;
                                  }
                              });
        m_sizeAnimation.value().step(0.0f);
    }
    else
    {
        m_visible = true;
    }
}

void CWindow::Hide()
{
    if (IsFlag(FLAG_ANIMATED_BOARD))
    {
        m_sizeAnimation = tweeny::from(1.0f)
                              .to(0.0f)
                              .during(200)
                              .via(tweeny::easing::backIn)
                              .onStep([this](tweeny::tween<float> &t, float scale) {
                                  if (t.progress() == 0.0f)
                                  {
                                      SetScale(1.0f, 1.0f);
                                      SetAllAlpha(1.0f);
                                  }

                                  SetScale(scale, scale);
                                  SetAllAlpha(scale);

                                  if (t.progress() == 1.0f)
                                  {
                                      SetScale(0.0f, 0.0f);
                                      SetAllAlpha(0.0f);

                                      m_visible = false;
                                      return true;
                                  }
                                  else
                                  {
                                      return false;
                                  }
                              });
        m_sizeAnimation.value().step(0.0f);
    }
    else
    {
        m_visible = false;
    }
}

bool CWindow::IsShow() const
{
    if (m_parent && !m_parent->IsFlag(FLAG_COMPONENT) && !m_enableClipping)
    {
        const auto *parentClipper = FindClippingWindowUpwards(m_parent);
        if (parentClipper)
        {
            if (LiesEntirelyOutsideRect(parentClipper->m_rect, m_rect))
                return false;
        }
    }

    return m_visible;
}

// NOTE: IsShow asks, "Do you see yourself?" , But __IsShowing says, "Are you drawing yourself?" Check
// You are Show, but one of the parents at the top of the Tree can be Hide ..-[levites]
bool CWindow::IsRendering()
{
    if (!IsShow())
        return false;

    if (m_parent)
        return m_parent->IsRendering();

    return true;
}

void CWindow::Update()
{
    if (!IsShow())
        return;

    rmt_ScopedCPUSample(CWindowUpdate, RMTSF_Recursive);

    OnUpdate();

    if (m_sizeAnimation)
        m_sizeAnimation.value().step(static_cast<uint32_t>(DX::StepTimer::instance().GetElapsedMillieSeconds()));

    m_childrenCopy = m_children;
    for (CWindow *child : m_childrenCopy)
        child->Update();
}

void CWindow::SetSingleAlpha(float fAlpha)
{
    m_bSingleAlpha = true;
    m_fSingleAlpha = fAlpha;

    if (m_bAllAlpha)
        SetAlpha(fAlpha * m_fAllAlpha);
    else
        SetAlpha(fAlpha);
}

void CWindow::SetAllAlpha(float fAlpha)
{
    if (m_bSingleAlpha)
        return;

    if (m_bAllAlpha && m_fAllAlpha == fAlpha)
        return;

    m_bAllAlpha = true;
    m_fAllAlpha = fAlpha;

    if (m_bSingleAlpha)
        SetAlpha(fAlpha * m_fSingleAlpha);
    else
        SetAlpha(fAlpha);

    for (auto *child : m_children)
    {
        child->SetAllAlpha(fAlpha);
    }
}

void CWindow::Render()
{
    if (!IsRendering())
        return;

    rmt_ScopedCPUSample(RenderWindow, RMTSF_Recursive);

    Matrix currentProj;
    STATEMANAGER.GetTransform(D3DTS_WORLD, &currentProj);

    Matrix newProj;
    newProj = currentProj * m_matScaling;

    STATEMANAGER.SetTransform(D3DTS_WORLD, &newProj);

    if (m_enableClipping)
    {
        Rectangle rc;
        rc.X = std::max<int32_t>(0, m_rect.left);
        rc.Y = std::max<int32_t>(0, m_rect.top);
        rc.Width = m_width * m_v2Scale.x;
        rc.Height = m_height * m_v2Scale.y;

        auto *const parentClipper = FindClippingWindowUpwards(GetParent());

        if (parentClipper)
        {
            Rectangle rcParent;
            rcParent.X = parentClipper->m_rect.left;
            rcParent.Y = parentClipper->m_rect.top;
            rcParent.Width = parentClipper->m_width;
            rcParent.Height = parentClipper->m_height;
            rc = clamp(rc, rcParent);
        }

        if (rc.Height < 0)
            rc.Height = 0;

        if (rc.Width < 0)
            rc.Width = 0;

        ScissorsSetter setter(rc.X, rc.Y, rc.Width, rc.Height);

        OnRender();

        if constexpr (g_bOutlineBoxEnable)
        {
            if (CWindowManager::instance().GetPointWindow() == this)
            {
                CPythonGraphic::Instance().SetDiffuseColor(0.0f, 1.0f, 0.0f);
                CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
            }
        }

        m_childrenCopy = m_children;
        for (CWindow *child : m_childrenCopy)
        {
            child->Render();
        }
    }
    else
    {
        OnRender();

        if constexpr (g_bOutlineBoxEnable)
        {
            if (CWindowManager::instance().GetPointWindow() == this)
            {
                CPythonGraphic::Instance().SetDiffuseColor(0.0f, 1.0f, 0.0f);
                CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
            }
        }

        m_childrenCopy = m_children;
        for (CWindow *child : m_childrenCopy)
            child->Render();
    }

    STATEMANAGER.SetTransform(D3DTS_WORLD, &currentProj);
} // namespace UI

void CWindow::OnUpdate() { RunCallback("OnUpdate"); }

void CWindow::OnRender() { RunCallback("OnRender"); }

void CWindow::OnAfterRender() {}

void CWindow::SetSize(int32_t width, int32_t height)
{
    m_width = width;
    m_height = height;

    UpdateRect();
}

void CWindow::SetScale(float fx, float fy)
{
    m_v2Scale.x = fx;
    m_v2Scale.y = fy;
    UpdateRect();

    Vector2 vCenter(m_x + (m_width / 2), m_y + (m_height / 2));
    m_matScaling = XMMatrixTransformation2D(vCenter, NULL, m_v2Scale, vCenter, NULL, Vector2::Zero);

    // DirectX::SimpleMath::MatrixScaling(&m_matScaling, m_v2Scale.x, m_v2Scale.y, m_v2Scale.x);
}

void CWindow::SetHorizontalAlign(DWORD dwAlign)
{
    m_HorizontalAlign = static_cast<EHorizontalAlign>(dwAlign);
    UpdateRect();
}

void CWindow::SetVerticalAlign(DWORD dwAlign)
{
    m_VerticalAlign = static_cast<EVerticalAlign>(dwAlign);
    UpdateRect();
}

void CWindow::SetPosition(int32_t x, int32_t y)
{
    m_x = x;
    m_y = y;

    UpdateRect();
}

void CWindow::UpdateRect()
{
    if (m_parent)
    {
        switch (m_HorizontalAlign)
        {
        case HORIZONTAL_ALIGN_LEFT:
            m_rect.left = m_x;
            break;

        case HORIZONTAL_ALIGN_RIGHT:
            m_rect.left = ((m_parent->m_width - m_width) * m_v2Scale.x) - m_x;
            break;

        case HORIZONTAL_ALIGN_CENTER:
            m_rect.left = (m_parent->m_width - m_x - m_width * m_v2Scale.x) / 2 + m_x;
            break;
        }

        switch (m_VerticalAlign)
        {
        case VERTICAL_ALIGN_TOP:
            m_rect.top = m_y;
            break;

        case VERTICAL_ALIGN_BOTTOM:
            m_rect.top = m_parent->m_height - m_height * m_v2Scale.y - m_y;
            break;

        case VERTICAL_ALIGN_CENTER:
            m_rect.top = (m_parent->m_height - m_y - m_height * m_v2Scale.y) / 2 + m_y;
            break;
        }

        m_rect.left += m_parent->m_rect.left;
        m_rect.top += m_parent->m_rect.top;
    }
    else
    {
        m_rect.left = m_x;
        m_rect.top = m_y;
    }

    m_rect.bottom = m_rect.top + m_height * m_v2Scale.y;
    m_rect.right = m_rect.left + m_width * m_v2Scale.x;

    for (CWindow *child : m_children)
        child->UpdateRect();

    OnChangePosition();
}

void CWindow::MakeLocalPosition(int32_t &x, int32_t &y)
{
    x = x - m_rect.left;
    y = y - m_rect.top;
}

void CWindow::AddChild(CWindow *win)
{
    assert(!IsChild(win) && "Duplicate");
    assert(win && "No nullptrs");

    win->m_parent = this;
    m_children.push_back(win);
}

CWindow *CWindow::GetRoot()
{
    if (m_parent && m_parent->IsWindow())
        return m_parent->GetRoot();

    return this;
}

CWindow *CWindow::GetParent() { return m_parent; }

bool CWindow::IsChild(CWindow *win) { return m_children.end() != std::find(m_children.begin(), m_children.end(), win); }

void CWindow::DeleteChild(CWindow *win, bool clearParent)
{
    if (clearParent)
        win->m_parent = nullptr;

    const auto it = std::find(m_children.begin(), m_children.end(), win);
    if (it != m_children.end())
    {
        m_children.erase(it);
    }
    else
    {
        SPDLOG_ERROR("Failed to find child window '{0}'' in '{1}'", win->m_name, m_name);
    }
}

void CWindow::SetTop(CWindow *win)
{
    assert(win && "No nullptrs");

    const auto it = std::find(m_children.begin(), m_children.end(), win);
    if (m_children.end() != it)
    {
        m_children.erase(it);
        m_children.push_back(win);
        win->OnTop();
    }
    else
    {
        SPDLOG_ERROR("Failed to find child window '{0}'' in '{1}'", win->m_name, m_name);
    }
}

void CWindow::OnMouseDrag(int32_t x, int32_t y) { RunCallback("OnMouseDrag", Py_BuildValue("(ii)", x, y)); }

void CWindow::OnMoveWindow(int32_t x, int32_t y) { RunCallback("OnMoveWindow", Py_BuildValue("(ii)", x, y)); }

void CWindow::OnSetFocus() { RunCallback("OnSetFocus"); }

void CWindow::OnKillFocus() { RunCallback("OnKillFocus"); }

bool CWindow::OnMouseOverIn() { return RunCallback("OnMouseOverIn"); }

bool CWindow::OnMouseOverOut() { return RunCallback("OnMouseOverOut"); }

void CWindow::OnMouseOver() { RunCallback("OnMouseOver"); }

void CWindow::OnDrop() { RunCallback("OnDrop"); }

void CWindow::OnTop() { RunCallback("OnTop"); }

bool CWindow::OnMouseLeftButtonDown() { return RunCallback("OnMouseLeftButtonDown"); }

bool CWindow::OnMouseLeftButtonUp() { return RunCallback("OnMouseLeftButtonUp"); }

bool CWindow::OnMouseLeftButtonDoubleClick() { return RunCallback("OnMouseLeftButtonDoubleClick"); }

bool CWindow::OnMouseRightButtonDown() { return RunCallback("OnMouseRightButtonDown"); }

bool CWindow::OnMouseRightButtonUp() { return RunCallback("OnMouseRightButtonUp"); }

bool CWindow::OnMouseRightButtonDoubleClick() { return RunCallback("OnMouseRightButtonDoubleClick"); }

bool CWindow::OnMouseMiddleButtonDown() { return RunCallback("OnMouseMiddleButtonDown"); }

bool CWindow::OnMouseMiddleButtonUp() { return RunCallback("OnMouseMiddleButtonUp"); }

bool CWindow::OnMouseWheelEvent(int nLen) { return RunCallback("OnRunMouseWheel", Py_BuildValue("(i)", nLen)); }

bool CWindow::OnChar(uint32_t ch) { return RunCallback("OnChar", Py_BuildValue("(k)", ch)); }

bool CWindow::OnKeyDown(KeyCode code) { return RunCallback("OnKeyDown", Py_BuildValue("(i)", code)); }

bool CWindow::OnKeyUp(KeyCode code) { return RunCallback("OnKeyUp", Py_BuildValue("(i)", code)); }

bool CWindow::OnTab() { return RunCallback("OnTab"); }

bool CWindow::OnPressExitKey() { return RunCallback("OnPressExitKey"); }

void CWindow::OnDestroy() { RunCallback("OnDestroy"); }

bool CWindow::IsIn(int32_t x, int32_t y)
{
    return x >= m_rect.left && x <= m_rect.right && y >= m_rect.top && y <= m_rect.bottom;
}

bool CWindow::IsIn()
{
    auto [x, y] = CWindowManager::Instance().GetMousePosition();
    return IsIn(x, y);
}

CWindow *CWindow::PickWindow(int32_t x, int32_t y)
{
    for (auto* win : boost::adaptors::reverse(m_children))
    {
        if (!win->IsShow())
            continue;

        if (!win->IsFlag(FLAG_IGNORE_SIZE) && !win->IsIn(x, y))
            continue;

        auto* res = win->PickWindow(x, y);
        if (res)
            return res;
    }

    if (IsFlag(FLAG_NOT_PICK) || RunCallback("OnCanIgnorePick"))
        return nullptr;

    return this;
}

// Note: *Only* call this with static strings - name.data() is held in |strings|
bool CWindow::RunCallback(const std::string &name, PyObject *args)
{
    static std::unordered_map<std::string, PyStr> strings;

    auto &str = strings[name];
    if (!str.Get())
        str = name;

    bool res = false;
    return PyCallClassMemberFunc(m_handler, str, args, &res) && res;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CBox::CBox() : CWindow(), m_dwColor(0xff000000) {}

CBox::~CBox() {}

void CBox::SetColor(DWORD dwColor) { m_dwColor = dwColor; }

void CBox::OnRender()
{
    CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
    CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CBar::CBar() : CWindow(), m_dwColor(0xff000000) {}

CBar::~CBar() {}

void CBar::SetColor(DWORD dwColor) { m_dwColor = dwColor; }

void CBar::OnRender()
{
    CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
    CPythonGraphic::Instance().RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CLine::CLine() : CWindow(), m_dwColor(0xff000000) {}

CLine::~CLine() {}

void CLine::SetColor(DWORD dwColor) { m_dwColor = dwColor; }

void CLine::OnRender()
{
    CPythonGraphic &rkpyGraphic = CPythonGraphic::Instance();
    rkpyGraphic.SetDiffuseColor(m_dwColor);
    rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

DWORD CBar3D::Type() { return kWindowBar3D; }

CBar3D::CBar3D() : CWindow()
{
    m_dwLeftColor = DirectX::SimpleMath::Color(0.2f, 0.2f, 0.2f, 1.0f).BGRA().c;
    m_dwRightColor = DirectX::SimpleMath::Color(0.7f, 0.7f, 0.7f, 1.0f).BGRA().c;
    m_dwCenterColor = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 1.0f).BGRA().c;
}

CBar3D::~CBar3D() {}

void CBar3D::SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter)
{
    m_dwLeftColor = dwLeft;
    m_dwRightColor = dwRight;
    m_dwCenterColor = dwCenter;
}

void CBar3D::OnRender()
{
    CPythonGraphic& rkpyGraphic = CPythonGraphic::Instance();

    rkpyGraphic.SetDiffuseColor(m_dwCenterColor);
    rkpyGraphic.RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);

    rkpyGraphic.SetDiffuseColor(m_dwLeftColor);
    rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.top);
    rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom);

    rkpyGraphic.SetDiffuseColor(m_dwRightColor);
    rkpyGraphic.RenderLine2d(m_rect.left, m_rect.bottom, m_rect.right, m_rect.bottom);
    rkpyGraphic.RenderLine2d(m_rect.right, m_rect.top, m_rect.right, m_rect.bottom);
}

/*void CBar3D::OnRender()
{
    CPythonGraphic &rkpyGraphic = CPythonGraphic::Instance();

    rkpyGraphic.SetDiffuseColor(m_dwCenterColor);
    rkpyGraphic.RenderGradationBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom, 0xff0a0303, 0xff250c0c);

    rkpyGraphic.SetDiffuseColor(m_dwLeftColor);
    rkpyGraphic.RenderGradationLine3d(m_rect.left, m_rect.top, m_rect.right, m_rect.top, 0xff140606, 0xff371111);
    rkpyGraphic.RenderGradationLine3d(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom, 0xff140606, 0xff371111);

    rkpyGraphic.SetDiffuseColor(m_dwRightColor);
    rkpyGraphic.RenderGradationLine3d(m_rect.left, m_rect.bottom, m_rect.right, m_rect.bottom, 0xff501717, 0xff501717);
    rkpyGraphic.RenderGradationLine3d(m_rect.right, m_rect.top, m_rect.right, m_rect.bottom, 0xff501717, 0xff501717);
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CTextLine::CTextLine() : CWindow(), m_placeHolderColor{0} { m_instance.SetColor(0.78f, 0.78f, 0.78f); }

CTextLine::~CTextLine()
{
    // dtor
}

DWORD CTextLine::Type() { return kWindowTextLine; }

bool CTextLine::OnIsType(DWORD dwType)
{
    if (CTextLine::Type() == dwType)
    {
        return true;
    }

    return CWindow::OnIsType(dwType);
}

void CTextLine::SetSecret(bool bFlag)
{
    m_instance.SetSecret(bFlag);
    OnChangeText();
}

void CTextLine::SetOutline(bool bFlag) { m_instance.SetOutline(bFlag); }

void CTextLine::SetMultiLine(bool bFlag)
{
    m_instance.SetMultiLine(bFlag);
    OnChangeText();
}

void CTextLine::SetFontName(const std::string &font)
{
    m_fontName = font;
    m_instance.SetTextPointer(Engine::GetFontManager().LoadFont(font));
    OnChangeText();
}

void CTextLine::SetFontGradient(DWORD dwColor, DWORD dwColor2) { m_instance.SetColorGradient(dwColor, dwColor2); }

void CTextLine::SetFontColor(DWORD dwColor) { m_instance.SetColor(dwColor); }

void CTextLine::SetLimitWidth(float fWidth)
{
    m_instance.SetLimitWidth(fWidth);
    OnChangeText();
}

void CTextLine::SetText(std::string text)
{
    m_text = std::move(text);
    m_instance.SetValue(m_text);
    OnChangeText();
}

const std::string &CTextLine::GetText() { return m_text; }

std::string CTextLine::GetHyperlinkAtPos(int x, int y)
{
    uint32_t glyphIndex;
    if (!m_instance.GetGlyphIndex(glyphIndex, x, y))
        return std::string();

    const int pos = m_instance.GetStringIndexFromGlyphIndex(glyphIndex);

    TextTag tag;
    bool inHyperlink = false;
    std::string hyperlinkText;

    for (auto first = m_text.begin(), last = m_text.end(); first != last;)
    {
        const auto sourcePos = first - m_text.begin();

        if (GetTextTag(std::string(m_text).substr(sourcePos), tag))
        {
            if (tag.type == TEXT_TAG_HYPERLINK_START)
            {
                inHyperlink = true;
                hyperlinkText = tag.content;
            }
            else if (tag.type == TEXT_TAG_HYPERLINK_END)
            {
                inHyperlink = false;
            }

            first += tag.length;
            continue;
        }

        if (inHyperlink && pos <= sourcePos)
            return hyperlinkText;

        ++first;
    }

    return std::string();
}

uint32_t CTextLine::GetLineCount() const { return m_instance.GetLineCount(); }

void CTextLine::OnUpdate() { m_instance.Update(); }

void CTextLine::OnRender()
{
    const RECT *clipRect = nullptr;
    if (m_clipRect.left != m_clipRect.right || m_clipRect.top != m_clipRect.bottom)
        clipRect = &m_clipRect;

    m_instance.Render(m_rect.left, m_rect.top, 0, clipRect);
}

/*virtual*/
void CTextLine::OnChangeText()
{
    m_instance.Update();
    SetSize(m_instance.GetWidth(), m_instance.GetHeight());
    UpdateRect();
}

void CTextLine::SetAlpha(float fAlpha)
{
    auto gradient = m_instance.GetColorGradient();

    Color first(gradient.first);
    first.A(fAlpha);

    Color second(gradient.second);
    second.A(fAlpha);

    m_instance.SetColorGradient(first.BGRA().c, second.BGRA().c);
}

float CTextLine::GetAlpha() const
{
    const DirectX::SimpleMath::Color kColor(m_instance.GetColorGradient().first);
    return kColor.A();
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CNumberLine::CNumberLine() : CWindow()
{
    m_strPath = "d:/ymir work/ui/game/taskbar/";
    m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
    m_dwWidthSummary = 0;
}

CNumberLine::CNumberLine(CWindow *parent) : CWindow()
{
    m_strPath = "d:/ymir work/ui/game/taskbar/";
    m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
    m_dwWidthSummary = 0;

    m_parent = parent;
}

CNumberLine::~CNumberLine() { ClearNumber(); }

void CNumberLine::SetPath(const char *c_szPath) { m_strPath = c_szPath; }

void CNumberLine::SetHorizontalAlign(int iType) { m_iHorizontalAlign = iType; }

void CNumberLine::SetNumber(const char *c_szNumber)
{
    if (0 == m_strNumber.compare(c_szNumber))
        return;

    ClearNumber();

    m_strNumber = c_szNumber;

    for (char cChar : m_strNumber)
    {
        std::string strImageFileName(m_strPath.data(), m_strPath.length());

        if (':' == cChar)
        {
            strImageFileName += "colon.sub";
        }
        else if ('?' == cChar)
        {
            strImageFileName += "questionmark.sub";
        }
        else if ('/' == cChar)
        {
            strImageFileName += "slash.sub";
        }
        else if ('%' == cChar)
        {
            strImageFileName += "percent.sub";
        }
        else if ('+' == cChar)
        {
            strImageFileName += "plus.sub";
        }
        else if ('m' == cChar)
        {
            strImageFileName += "m.sub";
        }
        else if ('g' == cChar)
        {
            strImageFileName += "g.sub";
        }
        else if ('p' == cChar)
        {
            strImageFileName += "p.sub";
        }
        else if ('l' == cChar)
        {
             strImageFileName += "p.sub";
        }
        /* else if ('l' == cChar)
        {
            strImageFileName += "l.sub";
        }
        */
        else if (cChar >= '0' && cChar <= '9')
        {
            strImageFileName += cChar;
            strImageFileName += ".sub";
        }
        else
            continue;

        auto pImage = CResourceManager::Instance().LoadResource<CGraphicImage>(strImageFileName);
        if (!pImage)
            continue;

        auto pInstance = std::make_unique<CGraphicImageInstance>();
        pInstance->SetImagePointer(pImage);
        auto rawPtr = pInstance.get();
        m_numberImages.emplace_back(std::move(pInstance));

        m_dwWidthSummary += rawPtr->GetWidth();
    }
}

void CNumberLine::ClearNumber()
{
    m_numberImages.clear();
    m_dwWidthSummary = 0;
    m_strNumber = "";
}

void CNumberLine::OnRender()
{
    for (DWORD i = 0; i < m_numberImages.size(); ++i)
        m_numberImages[i]->Render();
}

void CNumberLine::OnChangePosition()
{
    int ix = m_x;
    int iy = m_y;

    if (m_parent)
    {
        ix = m_rect.left;
        iy = m_rect.top;
    }

    switch (m_iHorizontalAlign)
    {
    case HORIZONTAL_ALIGN_LEFT:
        break;
    case HORIZONTAL_ALIGN_CENTER:
        ix -= int(m_dwWidthSummary) / 2;
        break;
    case HORIZONTAL_ALIGN_RIGHT:
        ix -= int(m_dwWidthSummary);
        break;
    default:
        break;
    }

    for (auto &m_numberImage : m_numberImages)
    {
        m_numberImage->SetPosition(ix, iy);
        ix += m_numberImage->GetWidth();
    }
}

// Web View

CWebView::CWebView() : CWindow() { m_pWebView = nullptr; }

CWebView::~CWebView()
{
    // Ensure that CefRefPtr::~CefRefPtr doesn't try to release it twice (it has already been released in
    // CWebView::OnBeforeClose)
    m_pWebView = nullptr;
}

void CWebView::LoadUrl(const char *url)
{
    if (!m_pWebView)
        return;

    m_pWebView->LoadUrl(url);
}

void CWebView::OnCreateInstance(HWND parent)
{
    m_pWebView = WebManager::instance().CreateWebView(m_width, m_height);
    m_pWebView->Initialise(parent);
}

void CWebView::OnDestroyInstance() {}

void CWebView::OnUpdate()
{
    if (!m_pWebView)
        return;

    if (IsIn())
    {
        auto [lx, ly] = CWindowManager::Instance().GetMousePosition();
        MakeLocalPosition(lx, ly);

        m_pWebView->InjectMouseMove(lx, ly);
    }

    m_pWebView->UpdateTexture();
}

void CWebView::OnRender()
{
    if (!m_pWebView)
        return;

    const auto pTexture = m_pWebView->GetTexture();
    if (!pTexture)
        return;

    const auto fimgWidth = pTexture->GetWidth();
    const auto fimgHeight = pTexture->GetHeight();

    const RECT &c_rRect = {0, 0, fimgWidth, fimgHeight};

    const auto texReverseWidth = 1.0f / float(pTexture->GetWidth());
    const auto texReverseHeight = 1.0f / float(pTexture->GetHeight());
    const auto su = c_rRect.left * texReverseWidth;
    const auto sv = c_rRect.top * texReverseHeight;
    const auto eu = (c_rRect.left + (c_rRect.right - c_rRect.left)) * texReverseWidth;
    const auto ev = (c_rRect.top + (c_rRect.bottom - c_rRect.top)) * texReverseHeight;

    TPDTVertex vertices[4];
    vertices[0].position.x = m_v2Position.x - 0.5f;
    vertices[0].position.y = m_v2Position.y - 0.5f;
    vertices[0].position.z = 0.0f;
    vertices[0].texCoord = TTextureCoordinate(su, sv);
    vertices[0].diffuse = 0xffffffff;

    vertices[1].position.x = m_v2Position.x + fimgWidth - 0.5f;
    vertices[1].position.y = m_v2Position.y - 0.5f;
    vertices[1].position.z = 0.0f;
    vertices[1].texCoord = TTextureCoordinate(eu, sv);
    vertices[1].diffuse = 0xffffffff;

    vertices[2].position.x = m_v2Position.x - 0.5f;
    vertices[2].position.y = m_v2Position.y + fimgHeight - 0.5f;
    vertices[2].position.z = 0.0f;
    vertices[2].texCoord = TTextureCoordinate(su, ev);
    vertices[2].diffuse = 0xffffffff;

    vertices[3].position.x = m_v2Position.x + fimgWidth - 0.5f;
    vertices[3].position.y = m_v2Position.y + fimgHeight - 0.5f;
    vertices[3].position.z = 0.0f;
    vertices[3].texCoord = TTextureCoordinate(eu, ev);
    vertices[3].diffuse = 0xffffffff;

    // 2004.11.18.myevan.ctrl+alt+del ¹Ýº¹ »ç¿ë½Ã Æ¨±â´Â ¹®Á¦
    if (Engine::GetDevice().SetPDTStream(vertices, 4))
    {
        Engine::GetDevice().SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

        STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
        STATEMANAGER.SetTexture(1, nullptr);
        Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
    }
}

void CWebView::OnChangePosition()
{
    m_v2Position.x = m_rect.left;
    m_v2Position.y = m_rect.top;
}

bool CWebView::OnMouseLeftButtonDown()
{
    if (!m_pWebView)
        return false;

    m_pWebView->InjectMouseDown(WebView::BROWSER_MOUSEBUTTON_LEFT);

    return CWindow::OnMouseLeftButtonDown();
}

bool CWebView::OnMouseLeftButtonUp()
{
    if (!m_pWebView)
        return false;

    m_pWebView->InjectMouseUp(WebView::BROWSER_MOUSEBUTTON_LEFT);

    return CWindow::OnMouseLeftButtonUp();
}

bool CWebView::OnMouseRightButtonDown()
{
    if (!m_pWebView)
        return false;

    m_pWebView->InjectMouseDown(WebView::BROWSER_MOUSEBUTTON_RIGHT);

    return true;
}

bool CWebView::OnMouseRightButtonUp()
{
    if (!m_pWebView)
        return false;

    m_pWebView->InjectMouseUp(WebView::BROWSER_MOUSEBUTTON_RIGHT);

    return true;
}

bool CWebView::OnMouseMiddleButtonDown()
{
    if (!m_pWebView)
        return false;

    // if (m_pWebView)
    //	m_pWebView->InjectMouseDown(Awesomium::MouseButton::kMouseButton_Middle);

    m_pWebView->InjectMouseDown(WebView::BROWSER_MOUSEBUTTON_MIDDLE);

    return true;
}

bool CWebView::OnMouseWheelEvent(int nLen)
{
    if (!m_pWebView)
        return false;

    m_pWebView->InjectMouseWheel(0, nLen);

    return true;
}

bool CWebView::OnMouseMiddleButtonUp()
{
    if (!m_pWebView)
        return false;

    m_pWebView->InjectMouseUp(WebView::BROWSER_MOUSEBUTTON_MIDDLE);

    return true;
}

bool CWebView::OnMouseOverIn()
{
    if (m_pWebView)
        m_pWebView->Focus(true);

    return true;
}

bool CWebView::OnMouseOverOut()
{
    if (m_pWebView)
        m_pWebView->Focus(false);

    return true;
}

void CWebView::OnSetFocus()
{
    if (m_pWebView)
        m_pWebView->Focus(true);
}

void CWebView::OnKillFocus()
{
    if (m_pWebView)
        m_pWebView->Focus(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CImageBox::CImageBox() : CWindow() { m_pImageInstance = nullptr; }

CImageBox::~CImageBox() { CImageBox::OnDestroyInstance(); }

void CImageBox::OnCreateInstance()
{
    OnDestroyInstance();

    m_pImageInstance = std::make_unique<CGraphicExpandedImageInstance>();
}

void CImageBox::OnDestroyInstance() { m_pImageInstance.reset(); }

bool CImageBox::LoadImage(const char *c_szFileName)
{
    if (!c_szFileName[0])
        return FALSE;

    OnCreateInstance();

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    if(!r) {
        SPDLOG_ERROR("Failed to Load image {}", c_szFileName);
        return false;
    }
    m_pImageInstance->SetImagePointer(std::move(r));
    m_pImageInstance->SetFilename(c_szFileName);
    if (m_pImageInstance->IsEmpty())
        return FALSE;

    SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
    UpdateRect();
    return TRUE;
}

void CImageBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
    if (!m_pImageInstance)
    {
        return;
    }

    m_diffuse = {fr, fg, fb, fa};
    m_pImageInstance->SetDiffuseColor(fr, fg, fb, fa);
}

void CImageBox::SetAlpha(float fAlpha) { SetDiffuseColor(m_diffuse.r, m_diffuse.g, m_diffuse.b, fAlpha); }

float CImageBox::GetAlpha() const { return m_diffuse.a; }

int CImageBox::GetWidth()
{
    if (!m_pImageInstance)
    {
        return 0;
    }

    return m_pImageInstance->GetWidth();
}

int CImageBox::GetHeight()
{
    if (!m_pImageInstance)
    {
        return 0;
    }

    return m_pImageInstance->GetHeight();
}

void CImageBox::OnUpdate() {}

void CImageBox::OnRender()
{
    if (!m_pImageInstance)
        return;

    if (IsShow())
    {
        if (fabs(m_coolTime) == 0.0f)
        {
            m_pImageInstance->Render();
        }
        else
        {
            float fcurTime = DX::StepTimer::Instance().GetTotalSeconds();
            float fPercentage = (fcurTime - m_startCoolTime) / m_coolTime;

            CPythonGraphic::Instance().RenderCoolTimeImageBox(m_pImageInstance.get(), fPercentage);
        }
    }
}

void CImageBox::OnChangePosition()
{
    if (!m_pImageInstance)
    {
        return;
    }

    m_pImageInstance->SetPosition(m_rect.left, m_rect.top);
}

/// CMoveTextLine
CMoveTextLine::CMoveTextLine()
    : m_v2SrcPos(0.0f, 0.0f), m_v2DstPos(0.0f, 0.0f), m_v2NextPos(0.0f, 0.0f), m_v2Direction(0.0f, 0.0f),
      m_v2NextDistance(0.0f, 0.0f), m_fDistance(0.0f), m_fMoveSpeed(10.0f), m_bIsMove(false)
{
}

CMoveTextLine::~CMoveTextLine() {  }

DWORD CMoveTextLine::Type()
{
    static DWORD s_dwType = GetCRC32("CMoveTextLine", strlen("CMoveTextLine"));
    return (s_dwType);
}

bool CMoveTextLine::OnIsType(DWORD dwType)
{
    if (CMoveTextLine::Type() == dwType)
        return TRUE;

    return FALSE;
}

void CMoveTextLine::SetMoveSpeed(float fSpeed) { m_fMoveSpeed = fSpeed; }

bool CMoveTextLine::GetMove() { return m_bIsMove; }

void CMoveTextLine::MoveStart()
{
    m_bIsMove = true;
    m_v2NextPos = m_v2SrcPos;
}

void CMoveTextLine::MoveStop() { m_bIsMove = false; }

void CMoveTextLine::OnEndMove() { RunCallback("OnEndMove"); }

void CMoveTextLine::OnChangePosition()
{
}

void CMoveTextLine::SetMovePosition(float fDstX, float fDstY)
{
    if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
    {
        m_v2SrcPos.x = m_rect.left;
        m_v2SrcPos.y = m_rect.top;

        m_v2DstPos.x = fDstX;
        m_v2DstPos.y = fDstY;

        D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);

        m_fDistance = (m_v2Direction.y * m_v2Direction.y + m_v2Direction.x * m_v2Direction.x);
        D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

        if (m_v2SrcPos != m_v2NextPos)
        {
            float fDist = sqrtf(m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);
            m_v2NextPos = m_v2Direction * fDist;
        }
    }
}

void CMoveTextLine::OnUpdate()
{
    if (IsShow() && GetMove())
    {
        D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &(m_v2Direction * m_fMoveSpeed));
        D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

        float fNextDistance = m_v2NextDistance.y * m_v2NextDistance.y + m_v2NextDistance.x * m_v2NextDistance.x;
        if (fNextDistance >= m_fDistance)
        {
            m_v2NextPos = m_v2DstPos;
            MoveStop();
            OnEndMove();
        }
    }
}

void CMoveTextLine::OnRender()
{
    if (IsShow())
        m_instance.Render(m_v2NextPos.x, m_v2NextPos.y);
}

/// CMoveImageBox
CMoveImageBox::CMoveImageBox()
    : m_v2SrcPos(0.0f, 0.0f), m_v2DstPos(0.0f, 0.0f), m_v2NextPos(0.0f, 0.0f),
      m_v2Direction(0.0f, 0.0f), m_v2NextDistance(0.0f, 0.0f), m_fDistance(0.0f), m_fMoveSpeed(10.0f), m_bIsMove(false)
{
}

CMoveImageBox::~CMoveImageBox() { OnDestroyInstance(); }

void CMoveImageBox::OnCreateInstance()
{
    OnDestroyInstance();

    m_pImageInstance = std::make_unique<CGraphicExpandedImageInstance>();
}

void CMoveImageBox::OnDestroyInstance()
{
    if (m_pImageInstance)
    {
        m_pImageInstance.reset();
    }
}

DWORD CMoveImageBox::Type()
{
    static DWORD s_dwType = GetCRC32("CMoveImageBox", strlen("CMoveImageBox"));
    return (s_dwType);
}

bool CMoveImageBox::OnIsType(DWORD dwType)
{
    if (CMoveImageBox::Type() == dwType)
        return TRUE;

    return FALSE;
}

void CMoveImageBox::OnEndMove() { RunCallback("OnEndMove"); }

void CMoveImageBox::SetMovePosition(float fDstX, float fDstY)
{
    if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
    {
        m_v2SrcPos.x = m_rect.left;
        m_v2SrcPos.y = m_rect.top;

        m_v2DstPos.x = fDstX;
        m_v2DstPos.y = fDstY;

        m_v2Direction = m_v2DstPos - m_v2SrcPos;

        m_fDistance = (m_v2Direction.x * m_v2Direction.x + m_v2Direction.y * m_v2Direction.y);

        m_v2Direction.Normalize();

        if (m_pImageInstance && m_v2SrcPos != m_v2NextPos)
        {
            float fDist = sqrtf(m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);

            m_v2NextPos = m_v2Direction * fDist;
            m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
        }
    }
}

void CMoveImageBox::SetMoveSpeed(float fSpeed) { m_fMoveSpeed = fSpeed; }

void CMoveImageBox::MoveStart()
{
    m_bIsMove = true;
    m_v2NextPos = m_v2SrcPos;
}

void CMoveImageBox::MoveStop() { m_bIsMove = false; }

bool CMoveImageBox::GetMove() { return m_bIsMove; }

void CMoveImageBox::OnUpdate()
{
    if (!m_pImageInstance)
        return;

    if (IsShow() && GetMove())
    {
        m_v2NextPos = m_v2NextPos + m_v2Direction * m_fMoveSpeed;
        m_v2NextDistance =  m_v2NextPos - m_v2SrcPos;

        float fNextDistance = (m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);
        if (fNextDistance >= m_fDistance)
        {
            m_v2NextPos = m_v2DstPos;
            MoveStop();
            OnEndMove();
        }

        m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
    }
}

void CMoveImageBox::OnRender()
{
    if (!m_pImageInstance)
        return;

    if (IsShow())
        m_pImageInstance->Render();
}

/// CMoveScaleImageBox
CMoveScaleImageBox::CMoveScaleImageBox()
    : CMoveImageBox(), m_fMaxScale(1.0f), m_fMaxScaleRate(1.0f), m_fScaleDistance(0.0f), m_fAdditionalScale(0.0f),
      m_v2CurScale(1.0f, 1.0f)
{
}

CMoveScaleImageBox::~CMoveScaleImageBox() { OnDestroyInstance(); }

void CMoveScaleImageBox::OnCreateInstance()
{
    OnDestroyInstance();

    m_pImageInstance = std::make_unique<CGraphicExpandedImageInstance>();
}

void CMoveScaleImageBox::OnDestroyInstance()
{
    if (m_pImageInstance)
    {
        m_pImageInstance.reset();
    }
}

DWORD CMoveScaleImageBox::Type()
{
    static DWORD s_dwType = GetCRC32("CMoveScaleImageBox", strlen("CMoveScaleImageBox"));
    return (s_dwType);
}

bool CMoveScaleImageBox::OnIsType(DWORD dwType)
{
    if (CMoveScaleImageBox::Type() == dwType)
        return TRUE;

    return FALSE;
}

void CMoveScaleImageBox::SetMaxScale(float fMaxScale) { m_fMaxScale = fMaxScale; }

void CMoveScaleImageBox::SetMaxScaleRate(float fMaxScaleRate)
{
    m_fMaxScaleRate = fMaxScaleRate;
    float fDistanceRate = m_fDistance * fMaxScaleRate;
    m_fScaleDistance = fDistanceRate;
    m_v2CurScale = m_pImageInstance->GetScale();

    float fDiffScale = m_fMaxScale - m_v2CurScale.x;
    m_fAdditionalScale = fDiffScale / (sqrtf(fDistanceRate) / m_fMoveSpeed);
}

void CMoveScaleImageBox::SetScalePivotCenter(bool bScalePivotCenter)
{
    if (m_pImageInstance)
        m_pImageInstance->SetScalePivotCenter(bScalePivotCenter);
}

void CMoveScaleImageBox::OnUpdate()
{
    if (!m_pImageInstance)
        return;

    if (IsShow() && GetMove())
    {
        m_v2NextPos = m_v2NextPos + (m_v2Direction * m_fMoveSpeed);
        m_v2NextDistance = m_v2NextPos - m_v2SrcPos;

        float fNextDistance = (m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);
        if (m_fScaleDistance < fNextDistance)
            m_fAdditionalScale *= -1.0f;

        Vector2 v2NewScale = m_pImageInstance->GetScale() + Vector2(m_fAdditionalScale, m_fAdditionalScale);
        if (m_fMaxScale < v2NewScale.x)
            v2NewScale = Vector2(m_fMaxScale, m_fMaxScale);

        if (m_v2CurScale.x > v2NewScale.x)
            v2NewScale = m_v2CurScale;

        m_pImageInstance->SetScale(v2NewScale);

        if (fNextDistance >= m_fDistance)
        {
            m_pImageInstance->SetScale(m_v2CurScale);
            m_v2NextPos = m_v2DstPos;
            MoveStop();
            OnEndMove();
        }

        m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
// MarkBox - ¸¶Å© Ãâ·Â¿ë UI À©µµ¿ì
///////////////////////////////////////////////////////////////////////////////////////////////
CMarkBox::CMarkBox() : CWindow() { m_pMarkInstance = nullptr; }

CMarkBox::~CMarkBox() { OnDestroyInstance(); }

void CMarkBox::OnCreateInstance()
{
    OnDestroyInstance();
    m_pMarkInstance = CGraphicMarkInstance::New();
}

void CMarkBox::OnDestroyInstance()
{
    if (m_pMarkInstance)
    {
        CGraphicMarkInstance::Delete(m_pMarkInstance);
        m_pMarkInstance = nullptr;
    }
}

void CMarkBox::LoadImage(const char *c_szFilename)
{
    OnCreateInstance();

    m_pMarkInstance->SetImageFileName(c_szFilename);
    m_pMarkInstance->Load();
    SetSize(m_pMarkInstance->GetWidth(), m_pMarkInstance->GetHeight());

    UpdateRect();
}

void CMarkBox::SetScale(FLOAT fScale)
{
    if (!m_pMarkInstance)
    {
        return;
    }

    m_pMarkInstance->SetScale(fScale);
}

void CMarkBox::SetIndex(UINT uIndex)
{
    if (!m_pMarkInstance)
    {
        return;
    }

    m_pMarkInstance->SetIndex(uIndex);
}

void CMarkBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
    if (!m_pMarkInstance)
    {
        return;
    }

    m_pMarkInstance->SetDiffuseColor(fr, fg, fb, fa);
}

void CMarkBox::OnUpdate() {}

void CMarkBox::OnRender()
{
    if (!m_pMarkInstance)
    {
        return;
    }

    if (IsShow())
    {
        m_pMarkInstance->Render();
    }
}

void CMarkBox::OnChangePosition()
{
    if (!m_pMarkInstance)
    {
        return;
    }

    m_pMarkInstance->SetPosition(m_rect.left, m_rect.top);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

DWORD CExpandedImageBox::Type()
{
    static DWORD s_dwType = GetCRC32("CExpandedImageBox", strlen("CExpandedImageBox"));
    return (s_dwType);
}

bool CExpandedImageBox::OnIsType(DWORD dwType)
{
    if (CExpandedImageBox::Type() == dwType)
    {
        return TRUE;
    }

    return FALSE;
}

CExpandedImageBox::CExpandedImageBox() : CImageBox() {}

CExpandedImageBox::~CExpandedImageBox() { OnDestroyInstance(); }

void CExpandedImageBox::OnCreateInstance()
{
    OnDestroyInstance();

    m_pImageInstance = std::make_unique<CGraphicExpandedImageInstance>();
}

void CExpandedImageBox::OnDestroyInstance() { m_pImageInstance.reset(); }

void CExpandedImageBox::SetScale(float fx, float fy)
{
    if (!m_pImageInstance)
    {
        return;
    }

    static_cast<CGraphicExpandedImageInstance *>(m_pImageInstance.get())->SetScale(fx, fy);
    CWindow::SetSize(long(float(GetWidth()) * fx), long(float(GetHeight()) * fy));
}

void CExpandedImageBox::SetOrigin(float fx, float fy)
{
    if (!m_pImageInstance)
    {
        return;
    }

    static_cast<CGraphicExpandedImageInstance *>(m_pImageInstance.get())->SetOrigin(fx, fy);
}

void CExpandedImageBox::SetRotation(float fRotation)
{
    if (!m_pImageInstance)
    {
        return;
    }

    static_cast<CGraphicExpandedImageInstance *>(m_pImageInstance.get())->SetRotation(fRotation);
}

void CExpandedImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
    if (!m_pImageInstance)
    {
        return;
    }

    static_cast<CGraphicExpandedImageInstance *>(m_pImageInstance.get())
        ->SetRenderingRect(fLeft, fTop, fRight, fBottom);
}

void CExpandedImageBox::SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom)
{
    if (!m_pImageInstance)
    {
        return;
    }

    static_cast<CGraphicExpandedImageInstance *>(m_pImageInstance.get())
        ->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);
}

void CExpandedImageBox::SetRenderingMode(int iMode)
{
    static_cast<CGraphicExpandedImageInstance *>(m_pImageInstance.get())->SetRenderingMode(iMode);
}

void CExpandedImageBox::LeftRightReverse()
{
    static_cast<CGraphicExpandedImageInstance *>(m_pImageInstance.get())->SetInverse();
}

void CExpandedImageBox::OnUpdate() {}

void CExpandedImageBox::OnRender()
{
    if (!m_pImageInstance)
        return;

    if (IsShow())
    {
        if (m_coolTime == 0.0f)
        {
            m_pImageInstance->Render();
        }
        else
        {
            float fcurTime = DX::StepTimer::Instance().GetTotalSeconds();
            float fPercentage = (fcurTime - m_startCoolTime) / m_coolTime;
            CPythonGraphic::Instance().RenderCoolTimeImageBox(m_pImageInstance.get(), fPercentage);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

DWORD CAniImageBox::Type()
{
    static DWORD s_dwType = GetCRC32("CAniImageBox", strlen("CAniImageBox"));
    return (s_dwType);
}

bool CAniImageBox::OnIsType(DWORD dwType)
{
    if (CAniImageBox::Type() == dwType)
    {
        return TRUE;
    }

    return FALSE;
}

CAniImageBox::CAniImageBox()
    : CWindow(), m_bycurDelay(0), m_byDelay(4), m_bycurIndex(0), m_isRunning(true), m_diffuse{1.0f, 1.0f, 1.0f, 1.0f}
{
    m_ImageVector.clear();
}

CAniImageBox::~CAniImageBox() { m_ImageVector.clear(); }

void CAniImageBox::SetDelay(int iDelay) { m_byDelay = iDelay; }

void CAniImageBox::AppendImage(const char *c_szFileName)
{
    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);

    auto pImageInstance = std::make_unique<CGraphicExpandedImageInstance>(c_szFileName);

    pImageInstance->SetImagePointer(r);
    if (pImageInstance->IsEmpty())
        return;

    m_ImageVector.push_back(std::move(pImageInstance));
    m_bycurIndex = rand() % m_ImageVector.size();
    //		SetSize(pImageInstance->GetWidth(), pImageInstance->GetHeight());
    //		UpdateRect();
}

void CAniImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
    std::for_each(m_ImageVector.begin(), m_ImageVector.end(),
                  [&](std::unique_ptr<CGraphicExpandedImageInstance> &inst) {
                      inst->SetRenderingRect(fLeft, fTop, fRight, fBottom);
                  });
}

void CAniImageBox::SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom)
{
    std::for_each(m_ImageVector.begin(), m_ImageVector.end(),
                  [&](std::unique_ptr<CGraphicExpandedImageInstance> &inst) {
                      inst->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);
                  });
}

struct FSetRenderingMode
{
    int iMode;

    void operator()(std::unique_ptr<CGraphicExpandedImageInstance> &pInstance) { pInstance->SetRenderingMode(iMode); }
};

void CAniImageBox::SetRenderingMode(int iMode)
{
    FSetRenderingMode setRenderingMode;
    setRenderingMode.iMode = iMode;
    for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingMode);
}

struct FSetDiffuseColor
{
    float r;
    float g;
    float b;
    float a;

    void operator()(std::unique_ptr<CGraphicExpandedImageInstance> &pInstance)
    {
        pInstance->SetDiffuseColor(r, g, b, a);
    }
};

void CAniImageBox::SetDiffuseColor(float r, float g, float b, float a)
{
    m_diffuse = {r, g, b, a};

    FSetDiffuseColor setDiffuseColor;
    setDiffuseColor.r = r;
    setDiffuseColor.g = g;
    setDiffuseColor.b = b;
    setDiffuseColor.a = a;

    for_each(m_ImageVector.begin(), m_ImageVector.end(), setDiffuseColor);
}

void CAniImageBox::SetOrigin(float x, float y)
{
    for_each(m_ImageVector.begin(), m_ImageVector.end(),
             [x, y](std::unique_ptr<CGraphicExpandedImageInstance> &pInstance) { pInstance->SetOrigin(x, y); });
}

void CAniImageBox::SetRotation(float rot)
{
    for_each(m_ImageVector.begin(), m_ImageVector.end(),
             [rot](std::unique_ptr<CGraphicExpandedImageInstance> &pInstance) { pInstance->SetRotation(rot); });
}

void CAniImageBox::SetScale(float scale)
{
    for_each(m_ImageVector.begin(), m_ImageVector.end(),
             [scale](std::unique_ptr<CGraphicExpandedImageInstance> &pInstance) { pInstance->SetScale(scale, scale); });
}

void CAniImageBox::SetAlpha(float fAlpha)
{
    FSetDiffuseColor setDiffuseColor;
    setDiffuseColor.r = m_diffuse.r;
    setDiffuseColor.g = m_diffuse.g;
    setDiffuseColor.b = m_diffuse.b;
    setDiffuseColor.a = fAlpha;

    for_each(m_ImageVector.begin(), m_ImageVector.end(), setDiffuseColor);
}

float CAniImageBox::GetAlpha() const { return m_diffuse.a; }

void CAniImageBox::Stop() { m_isRunning = false; }

void CAniImageBox::Start() { m_isRunning = true; }

struct FSetAniImgScale
{
    float x;
    float y;

    void operator()(std::unique_ptr<CGraphicExpandedImageInstance> &pInstance) { pInstance->SetScale(x, y); }
};

void CAniImageBox::SetAniImgScale(float x, float y)
{
    FSetAniImgScale setAniImgScale;
    setAniImgScale.x = x;
    setAniImgScale.y = y;

    for_each(m_ImageVector.begin(), m_ImageVector.end(), setAniImgScale);
}

void CAniImageBox::ResetFrame() { m_bycurIndex = 0; }

void CAniImageBox::OnUpdate()
{
    if (!m_isRunning)
        return;

    ++m_bycurDelay;
    if (m_bycurDelay < m_byDelay)
    {
        return;
    }

    m_bycurDelay = 0;

    ++m_bycurIndex;
    if (m_bycurIndex >= m_ImageVector.size())
    {
        m_bycurIndex = 0;

        OnEndFrame();
    }
}

void CAniImageBox::OnRender()
{
    if (m_bycurIndex < m_ImageVector.size())
    {
        m_ImageVector[m_bycurIndex]->Render();
    }
}

struct FChangePosition
{
    float fx, fy;

    void operator()(std::unique_ptr<CGraphicExpandedImageInstance> &pInstance) { pInstance->SetPosition(fx, fy); }
};

void CAniImageBox::OnChangePosition()
{
    FChangePosition changePosition;
    changePosition.fx = m_rect.left;
    changePosition.fy = m_rect.top;
    for_each(m_ImageVector.begin(), m_ImageVector.end(), changePosition);
}

void CAniImageBox::OnEndFrame() { RunCallback("OnEndFrame"); }

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

DWORD CRenderTarget::Type()
{
    static DWORD s_dwType = GetCRC32("CRenderTarget", strlen("CRenderTarget"));
    return (s_dwType);
}

bool CRenderTarget::OnIsType(DWORD dwType)
{
    if (CRenderTarget::Type() == dwType)
    {
        return TRUE;
    }

    return FALSE;
}

CRenderTarget::CRenderTarget() : CWindow(), m_renderTarget(nullptr), m_iRenderTargetIndex(-1) {}

CRenderTarget::~CRenderTarget()
{
    m_renderTarget->OnLostDevice();
    m_renderTarget = nullptr;
    m_iRenderTargetIndex = -1;
}

void CRenderTarget::SetRenderTarget(DWORD index)
{
    m_iRenderTargetIndex = index;
    auto &rtm = CRenderTargetManager::instance();

    auto *renderTarget = rtm.GetRenderTarget(m_iRenderTargetIndex);
    if (renderTarget)
        m_renderTarget = renderTarget;
}

bool CRenderTarget::SetWikiRenderTarget(int iRenderTargetInex)
{
    m_iRenderTargetIndex = iRenderTargetInex;

    CGraphicRenderTargetTexture *pTex =
        CRenderTargetManager::Instance().CreateWikiRenderTarget(iRenderTargetInex, m_width, m_height);
    if (!pTex)
    {
        return false;
    }

    m_renderTarget = pTex;
    return true;
}

void CRenderTarget::OnUpdate()
{
    if (!m_renderTarget)
        return;

    if (!IsShow())
        return;

    m_renderTarget->SetRenderingRect(&m_rect);
}

void CRenderTarget::OnRender()
{
    if (!m_renderTarget)
        return;

    if (!IsShow())
        return;

    m_renderTarget->Render();
}

DWORD ColorPicker::Type()
{
    static DWORD s_dwType = GetCRC32("ColorPicker", strlen("ColorPicker"));
    return (s_dwType);
}

bool ColorPicker::OnIsType(DWORD dwType)
{
    if (ColorPicker::Type() == dwType)
    {
        return TRUE;
    }

    return FALSE;
}

void ColorPicker::Create() { CColorPicker::instance().Init(GetWidth(), GetHeight()); }

void ColorPicker::OnChangePosition()
{
    m_v2Position.x = GetPositionX();
    m_v2Position.y = GetPositionY();
}

ColorPicker::ColorPicker() {}

ColorPicker::~ColorPicker() {}

void ColorPicker::OnRender() { CColorPicker::instance().Draw(m_rect.left, m_rect.top); }

void ColorPicker::OnMouseDrag(int32_t x, int32_t y)
{
    auto [lx, ly] = CWindowManager::Instance().GetMousePosition();

    MakeLocalPosition(lx, ly);

    RunCallback("OnPickColor", Py_BuildValue("(I)", CColorPicker::instance().GetPickedColor(lx, ly)));
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CButton::CButton()
    : CWindow(), m_bEnable(TRUE), m_isPressed(FALSE), m_isFlash(FALSE), m_pcurVisual(nullptr), m_pFlashVisual(nullptr)
{
}

void CButton::SetButtonScale(float xScale, float yScale)
{
    if (m_pcurVisual)
        m_pcurVisual->SetScale(xScale, yScale);

    if (!m_upVisual.IsEmpty())
        m_upVisual.SetScale(xScale, yScale);

    if (!m_overVisual.IsEmpty())
        m_overVisual.SetScale(xScale, yScale);

    if (!m_downVisual.IsEmpty())
        m_downVisual.SetScale(xScale, yScale);

    if (!m_disableVisual.IsEmpty())
        m_disableVisual.SetScale(xScale, yScale);

    SetSize(m_upVisual.GetWidth() * xScale, m_upVisual.GetHeight() * yScale);
}

int CButton::GetButtonImageWidth() const
{
    if (m_pcurVisual)
        return m_pcurVisual->GetWidth();

    return 0;
}

int CButton::GetButtonImageHeight() const
{
    if (m_pcurVisual)
        return m_pcurVisual->GetHeight();

    return 0;
}

bool CButton::SetUpVisual(const char *c_szFileName)
{
    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    m_upVisual.SetImagePointer(r);

    if (m_upVisual.IsEmpty())
        return false;

    SetSize(m_upVisual.GetWidth(), m_upVisual.GetHeight());
    SetCurrentVisual(&m_upVisual);
    return true;
}

bool CButton::SetOverVisual(const char *c_szFileName)
{
    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    m_overVisual.SetImagePointer(r);

    if (m_overVisual.IsEmpty())
        return false;

    SetSize(m_overVisual.GetWidth(), m_overVisual.GetHeight());
    return true;
}

bool CButton::SetDownVisual(const char *c_szFileName)
{
    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    m_downVisual.SetImagePointer(r);

    if (m_downVisual.IsEmpty())
        return false;

    SetSize(m_downVisual.GetWidth(), m_downVisual.GetHeight());
    return true;
}

bool CButton::SetDisableVisual(const char *c_szFileName)
{
    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    m_disableVisual.SetImagePointer(r);

    if (m_downVisual.IsEmpty())
        return false;

    SetSize(m_disableVisual.GetWidth(), m_disableVisual.GetHeight());
    return true;
}

const char *CButton::GetUpVisualFileName() { return m_upVisual.GetGraphicImagePointer()->GetFileName(); }

const char *CButton::GetOverVisualFileName() { return m_overVisual.GetGraphicImagePointer()->GetFileName(); }

const char *CButton::GetDownVisualFileName() { return m_downVisual.GetGraphicImagePointer()->GetFileName(); }

void CButton::SetFlashVisual(CGraphicImageInstance *visual)
{
    m_pFlashVisual = visual;

    if (visual)
    {
        visual->SetPosition(m_x, m_y);
    }
}

void CButton::Flash() { m_isFlash = TRUE; }

void CButton::EnableFlash()
{
    m_isFlash = TRUE;
    SetFlashVisual(m_pFlashVisual);
}

void CButton::DisableFlash()
{
    m_isFlash = false;
    SetFlashVisual(nullptr);
}

void CButton::Enable()
{
    SetUp();
    m_bEnable = true;
}

void CButton::Disable()
{
    m_bEnable = false;
    if (!m_disableVisual.IsEmpty())
        SetCurrentVisual(&m_disableVisual);
}

bool CButton::IsDisable() const { return !m_bEnable; }

void CButton::SetUp()
{
    SetCurrentVisual(&m_upVisual);
    m_isPressed = false;
}

void CButton::Up()
{
    if (IsIn())
        SetCurrentVisual(&m_overVisual);
    else
        SetCurrentVisual(&m_upVisual);

    RunCallback("CallEvent");
}

void CButton::Over() { SetCurrentVisual(&m_overVisual); }

void CButton::Down()
{
    m_isPressed = true;
    SetCurrentVisual(&m_downVisual);
    RunCallback("DownEvent");
}

void CButton::LeftRightReverse()
{
    m_upVisual.SetInverse();
    m_overVisual.SetInverse();
    m_downVisual.SetInverse();
    m_disableVisual.SetInverse();
}

void CButton::OnUpdate() {}

void CButton::OnRender()
{
    if (!IsShow())
        return;

    if (m_pcurVisual)
    {
        if (m_isFlash && !IsIn())
        {
            if (uint64_t(GetTickCount64() / 500) % 2)
            {
                if (m_pFlashVisual)
                {
                    m_pFlashVisual->Render();
                }
                return;
            }
        }

        m_pcurVisual->Render();
    }

    RunCallback("OnRender");
}

void CButton::OnChangePosition()
{
    if (m_pcurVisual)
        m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

    if (m_pFlashVisual)
        m_pFlashVisual->SetPosition(m_rect.left, m_rect.top);
}

bool CButton::OnMouseLeftButtonDown()
{
    if (!IsEnable())
        return false;

    m_isPressed = true;
    Down();
    return false;
}

bool CButton::OnMouseLeftButtonDoubleClick()
{
    if (!IsEnable())
        return false;

    OnMouseLeftButtonDown();
    return true;
}

bool CButton::OnMouseLeftButtonUp()
{
    if (!IsEnable() || !IsPressed())
        return true;

    m_isPressed = false;
    Up();

    return true;
}

bool CButton::OnMouseOverIn()
{
    bool enable = IsEnable();

    if (enable || m_bIsAlwaysShowTooltip)
        RunCallback("ShowToolTip");

    if (!enable)
        return false;

    Over();

    return RunCallback("OnMouseOverIn");
}

bool CButton::OnMouseOverOut()
{
    const bool enable = IsEnable();

    if (enable || m_bIsAlwaysShowTooltip)
        RunCallback("HideToolTip");

    if (!enable)
        return false;

    SetUp();
    return RunCallback("OnMouseOverOut");
}

void CButton::SetCurrentVisual(CGraphicImageInstance *pVisual)
{
    m_pcurVisual = pVisual;
    m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
}

bool CButton::IsEnable() const { return m_bEnable; }

bool CButton::IsPressed() const { return m_isPressed; }

void CButton::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
    m_diffuse = Color{fr, fg, fb, fa};
    m_upVisual.SetDiffuseColor(fr, fg, fb, fa);
    m_overVisual.SetDiffuseColor(fr, fg, fb, fa);
    m_downVisual.SetDiffuseColor(fr, fg, fb, fa);
    m_disableVisual.SetDiffuseColor(fr, fg, fb, fa);
}

void CButton::SetAlpha(float fAlpha) { SetDiffuseColor(m_diffuse.R(), m_diffuse.G(), m_diffuse.B(), fAlpha); }

float CButton::GetAlpha() const { return m_diffuse.A(); }

void CButton::SetAlwaysTooltip(bool val) { m_bIsAlwaysShowTooltip = val; }

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CRadioButton::CRadioButton() : CButton() {}

CRadioButton::~CRadioButton() {}

bool CRadioButton::OnMouseLeftButtonDown()
{
    if (!IsEnable())
        return false;

    if (!m_isPressed)
    {
        Down();
        RunCallback("CallEvent");
    }

    return false;
}

bool CRadioButton::OnMouseLeftButtonUp() { return true; }

bool CRadioButton::OnMouseOverIn()
{
    if (!IsEnable())
        return false;

    if (!m_isPressed)
        SetCurrentVisual(&m_overVisual);

    RunCallback("ShowToolTip");
    return true;
}

bool CRadioButton::OnMouseOverOut()
{
    if (!IsEnable())
        return false;

    if (!m_isPressed)
        SetCurrentVisual(&m_upVisual);

    RunCallback("HideToolTip");
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CToggleButton::CToggleButton() : CButton() {}

CToggleButton::~CToggleButton() {}

bool CToggleButton::OnMouseLeftButtonDown()
{
    if (!IsEnable())
        return false;

    if (m_isPressed)
    {
        SetUp();

        if (IsIn())
            SetCurrentVisual(&m_overVisual);
        else
            SetCurrentVisual(&m_upVisual);

        RunCallback("OnToggleUp");
    }
    else
    {
        Down();
        RunCallback("OnToggleDown");
    }

    return false;
}

bool CToggleButton::OnMouseLeftButtonUp() { return true; }

bool CToggleButton::OnMouseOverIn()
{
    if (!IsEnable())
        return false;

    if (!m_isPressed)
        SetCurrentVisual(&m_overVisual);

    RunCallback("ShowToolTip");
    return true;
}

bool CToggleButton::OnMouseOverOut()
{
    if (!IsEnable())
        return false;

    if (!m_isPressed)
        SetCurrentVisual(&m_upVisual);

    RunCallback("HideToolTip");
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CDragButton::CDragButton() : CButton()
{
    m_restrictArea.left = 0;
    m_restrictArea.top = 0;
    m_restrictArea.right = CWindowManager::Instance().GetScreenWidth();
    m_restrictArea.bottom = CWindowManager::Instance().GetScreenHeight();
}

DWORD CDragButton::Type() { return kWindowDragButton; }

bool CDragButton::OnIsType(DWORD dwType)
{
    if (CDragButton::Type() == dwType)
    {
        return true;
    }

    return CWindow::OnIsType(dwType);
}

CDragButton::~CDragButton() {}

void CDragButton::SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight)
{
    m_restrictArea.left = ix;
    m_restrictArea.top = iy;
    m_restrictArea.right = ix + iwidth;
    m_restrictArea.bottom = iy + iheight;
}

void CDragButton::OnChangePosition()
{
    m_x = std::max<int32_t>(m_x, m_restrictArea.left);
    m_y = std::max<int32_t>(m_y, m_restrictArea.top);
    m_x = std::min<int32_t>(m_x, std::max<int32_t>(0, m_restrictArea.right - m_width));
    m_y = std::min<int32_t>(m_y, std::max<int32_t>(0, m_restrictArea.bottom - m_height));

    m_rect.left = m_x;
    m_rect.top = m_y;

    if (m_parent)
    {
        const auto &r = m_parent->GetRect();
        m_rect.left += r.left;
        m_rect.top += r.top;
    }

    m_rect.right = m_rect.left + m_width;
    m_rect.bottom = m_rect.top + m_height;

    for (auto &child : m_children)
        child->UpdateRect();

    if (m_pcurVisual)
        m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

    if (IsPressed())
        RunCallback("OnMove");
}

bool CDragButton::OnMouseOverIn()
{
    if (!IsEnable())
        return false;

    CButton::OnMouseOverIn();
    RunCallback("OnMouseOverIn");
    return true;
}

bool CDragButton::OnMouseOverOut()
{
    if (!IsEnable())
        return false;

    // TODO(tim): Why in?
    CButton::OnMouseOverIn();
    RunCallback("OnMouseOverOut");
    return true;
}

DWORD CDragBar::Type() { return kWindowDragBar; }

bool CDragBar::OnIsType(DWORD dwType)
{
    if (CDragBar::Type() == dwType)
    {
        return true;
    }

    return CWindow::OnIsType(dwType);
}

CDragBar::CDragBar() : CBar()
{
    m_restrictArea.left = 0;
    m_restrictArea.top = 0;
    m_restrictArea.right = CWindowManager::Instance().GetScreenWidth();
    m_restrictArea.bottom = CWindowManager::Instance().GetScreenHeight();
}

CDragBar::~CDragBar() {}

void CDragBar::SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight)
{
    m_restrictArea.left = ix;
    m_restrictArea.top = iy;
    m_restrictArea.right = ix + iwidth;
    m_restrictArea.bottom = iy + iheight;
}

void CDragBar::OnChangePosition()
{
    m_x = std::max<int32_t>(m_x, m_restrictArea.left);
    m_y = std::max<int32_t>(m_y, m_restrictArea.top);
    m_x = std::min<int32_t>(m_x, std::max<int32_t>(0, m_restrictArea.right - m_width));
    m_y = std::min<int32_t>(m_y, std::max<int32_t>(0, m_restrictArea.bottom - m_height));

    m_rect.left = m_x;
    m_rect.top = m_y;

    if (m_parent)
    {
        const auto &r = m_parent->GetRect();
        m_rect.left += r.left;
        m_rect.top += r.top;
    }

    m_rect.right = m_rect.left + m_width;
    m_rect.bottom = m_rect.top + m_height;

    for (auto &child : m_children)
        child->UpdateRect();

    if (IsPressed())
        RunCallback("OnMove");
}

bool CDragBar::OnMouseOverIn()
{
    RunCallback("OnMouseOverIn");
    return true;
}

bool CDragBar::OnMouseOverOut()
{
    RunCallback("OnMouseOverOut");
    return true;
}

bool CDragBar::OnMouseLeftButtonDown()
{
    m_isPressed = true;
    return RunCallback("OnMouseLeftButtonDown");
}

bool CDragBar::OnMouseLeftButtonDoubleClick()
{
    OnMouseLeftButtonDown();
    return RunCallback("OnMouseLeftButtonDown");
}

bool CDragBar::OnMouseLeftButtonUp()
{
    if (!IsPressed())
        return true;

    m_isPressed = false;
    return RunCallback("OnMouseLeftButtonUp");
}

// Video Frame

CVideoFrame::CVideoFrame()
{
#define BANDILIB_ID "YMIRGAMES_METIN2_20130903"
#define BANDILIB_KEY "8ea04df0"

    if (FAILED(m_bvl.Create(BANDIVIDEO_DLL_FILE_NAME, NULL, NULL)))
    {
        MessageBox(nullptr, "Error createing BandiVideoLibrary.", "Error", MB_OK | MB_ICONSTOP);
        return;
    }

    // certification
    if (FAILED(m_bvl.Verify(BANDILIB_ID, BANDILIB_KEY)))
    {
        SPDLOG_ERROR(" bandilib not certification !! ");
        return;
    }

    m_texture = nullptr;
    m_current_time = 0;
    m_current_frame = 0;
}

CVideoFrame::~CVideoFrame()
{
    if (m_texture)
        delete m_texture;
}

void CVideoFrame::OnChangePosition()
{
    m_v2Position.x = m_rect.left;
    m_v2Position.y = m_rect.top;
}

void CVideoFrame::Close() { m_bvl.Close(); }

void CVideoFrame::Open(const char *filename)
{
    Close();

    if (FAILED(m_bvl.Open(filename, FALSE)))
    {
        ASSERT(0);
        return;
    }

    if (FAILED(m_bvl.GetVideoInfo(m_video_info)))
    {
        ASSERT(0);
        return;
    }

    m_texture = new CGraphicImageTexture;
    if (!m_texture->Create(m_video_info.width, m_video_info.height, D3DFMT_X8R8G8B8))
    {
        delete m_texture;
        m_texture = nullptr;
        return;
    }

    BYTE *dest;
    INT pitch;
    // Clear the texture to black.
    if (m_texture->Lock(&pitch, (void **)&dest))
    {
        // Clear the pixels.
        int bytes = GetWidth() * 4;

        for (int y = 0; y < GetHeight(); y++)
        {
            memset(dest, 0, bytes);
            dest += pitch;
        }

        // Unlock the DirectX texture.
        m_texture->Unlock();
    }

    m_bvl.Seek(0, BVL_SEEK_TIME);
    m_bvl.Play();
}

void CVideoFrame::OnRender()
{
    if (!m_bvl.IsCreated())
        return;

    const auto pTexture = m_texture;
    if (!pTexture)
        return;

    const auto fimgWidth = m_video_info.width;
    const auto fimgHeight = m_video_info.height;

    const RECT &c_rRect = {0, 0, fimgWidth, fimgHeight};

    const auto texReverseWidth = 1.0f / float(pTexture->GetWidth());
    const auto texReverseHeight = 1.0f / float(pTexture->GetHeight());
    const auto su = c_rRect.left * texReverseWidth;
    const auto sv = c_rRect.top * texReverseHeight;
    const auto eu = (c_rRect.left + (c_rRect.right - c_rRect.left)) * texReverseWidth;
    const auto ev = (c_rRect.top + (c_rRect.bottom - c_rRect.top)) * texReverseHeight;

    TPDTVertex vertices[4];
    vertices[0].position.x = m_v2Position.x - 0.5f;
    vertices[0].position.y = m_v2Position.y - 0.5f;
    vertices[0].position.z = 0.0f;
    vertices[0].texCoord = TTextureCoordinate(su, sv);
    vertices[0].diffuse = 0xffffffff;

    vertices[1].position.x = m_v2Position.x + fimgWidth - 0.5f;
    vertices[1].position.y = m_v2Position.y - 0.5f;
    vertices[1].position.z = 0.0f;
    vertices[1].texCoord = TTextureCoordinate(eu, sv);
    vertices[1].diffuse = 0xffffffff;

    vertices[2].position.x = m_v2Position.x - 0.5f;
    vertices[2].position.y = m_v2Position.y + fimgHeight - 0.5f;
    vertices[2].position.z = 0.0f;
    vertices[2].texCoord = TTextureCoordinate(su, ev);
    vertices[2].diffuse = 0xffffffff;

    vertices[3].position.x = m_v2Position.x + fimgWidth - 0.5f;
    vertices[3].position.y = m_v2Position.y + fimgHeight - 0.5f;
    vertices[3].position.z = 0.0f;
    vertices[3].texCoord = TTextureCoordinate(eu, ev);
    vertices[3].diffuse = 0xffffffff;

    // 2004.11.18.myevan.ctrl+alt+del ¹Ýº¹ »ç¿ë½Ã Æ¨±â´Â ¹®Á¦
    if (Engine::GetDevice().SetPDTStream(vertices, 4))
    {
        Engine::GetDevice().SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

        STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
        STATEMANAGER.SetTexture(1, nullptr);
        Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
    }
}

void CVideoFrame::OnUpdate()
{
    if (!m_bvl.IsCreated())
        return;

    BVL_STATUS status;
    m_bvl.GetStatus(status);
    if (status == BVL_STATUS_PLAYEND)
    {
        m_bvl.Seek(0, BVL_SEEK_TIME);
        m_bvl.Play();
    }

    if (m_bvl.IsNextFrame())
    {
        if (m_texture == nullptr)
        {
            return;
        }

        void *buf;
        INT pitch;
        // Clear the texture to black.
        if (m_texture->GetD3DTexture() && m_texture->Lock(&pitch, (void **)&buf))
        {
            // Get frame
            BVL_FRAME frame;
            frame.frame_buf = buf;
            frame.frame_buf_size = m_video_info.height * pitch;
            frame.pitch = pitch;
            frame.width = m_video_info.width;
            frame.height = m_video_info.height;
            frame.pixel_format = BVLPF_X8R8G8B8;

            m_bvl.GetFrame(frame, TRUE);

            m_texture->Unlock();

            m_current_time = frame.frame_time;
            m_current_frame = frame.frame_number;
        }
    }
}
}; // namespace UI
