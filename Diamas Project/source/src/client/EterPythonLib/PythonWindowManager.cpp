#include "PythonWindowManager.h"
#include "PythonWindow.h"
#include "StdAfx.h"
#include "WindowUtil.hpp"
#include "base/Remotery.h"

#include <storm/StringFlags.hpp>

namespace UI
{
namespace
{
class Layer : public CWindow
{
  public:
    Layer()
    {
    }

    virtual ~Layer()
    {
    }

    bool IsWindow()
    {
        return false;
    }
};

storm::StringValueTable<uint32_t> kLayers[] = {
    {"GAME", kWindowLayerGame},        {"UI_BOTTOM", kWindowLayerUiBottom}, {"UI", kWindowLayerUi},
    {"TOP_MOST", kWindowLayerTopMost}, {"CURTAIN", kWindowLayerCurtain},
};
} // namespace

CWindowManager::CWindowManager()
    : m_width(0), m_height(0), m_vres(0), m_hres(0), m_mouseX(0), m_mouseY(0), m_dragX(0), m_dragY(0), m_pickedX(0),
      m_pickedY(0), m_bOnceIgnoreMouseLeftButtonUpEventFlag{false}, m_iIgnoreEndTime{0}, m_poMouseHandler(nullptr),
      m_bAttachingFlag(false), m_dwAttachingType(0), m_dwAttachingIndex(0), m_dwAttachingSlotNumber(0),
      m_byAttachingIconWidth(0), m_byAttachingIconHeight(0), m_focusWindow(nullptr), m_lockWindow(nullptr),
      m_captureWindow(nullptr), m_mouseLeftWindow(nullptr), m_mouseRightWindow(nullptr), m_mouseMiddleWindow(nullptr),
      m_pointWindow(nullptr), m_appWindow{nullptr}, m_rootWindow(new CWindow())
{
    m_rootWindow->SetName("root");
    m_rootWindow->Show();

    for (uint32_t i = 0; i < kWindowLayerMax; ++i)
    {
        std::string_view name;
        storm::FormatValueWithTable(name, i, kLayers);

        auto layer = new Layer();
        layer->SetName(std::string(name));
        layer->Show();

        m_rootWindow->AddChild(layer);
    }

    m_focusWindowList.clear();
    m_lockWindowList.clear();
    m_captureWindowList.clear();
}

CWindowManager::~CWindowManager()
{
    for (auto layer : m_rootWindow->GetChildren())
        delete layer;
}

void CWindowManager::Destroy()
{
    PruneLazyDeletionQueue();
}

void CWindowManager::SetMouseHandler(PyObject *poMouseHandler)
{
    m_poMouseHandler = poMouseHandler;
}

void CWindowManager::DestroyWindow(CWindow *win)
{
    if (m_lazyDeleteQueue.end() != std::find(m_lazyDeleteQueue.begin(), m_lazyDeleteQueue.end(), win))
    {
        SPDLOG_TRACE("Double-destruction of {0}", win->GetName());
        return;
    }
    win->OnDestroy();

    win->Hide();
    NotifyHideWindow(win);

    auto parent = win->GetParent();
    if (parent)
        parent->DeleteChild(win);

    win->Clear();
    m_lazyDeleteQueue.push_back(win);
}

bool CWindowManager::IsDragging()
{
    return abs(m_mouseX - m_pickedX) + abs(m_mouseY - m_pickedY) > 10;
}

bool CWindowManager::IsAttaching()
{
    return m_bAttachingFlag;
}

DWORD CWindowManager::GetAttachingType()
{
    return m_dwAttachingType;
}

DWORD CWindowManager::GetAttachingIndex()
{
    return m_dwAttachingIndex;
}

DWORD CWindowManager::GetAttachingSlotNumber()
{
    return m_dwAttachingSlotNumber;
}

DWORD CWindowManager::GetAttachingRealSlotNumber() const
{
    return m_dwAttachingRealSlotNumber;
}

void CWindowManager::GetAttachingIconSize(BYTE *pbyWidth, BYTE *pbyHeight)
{
    *pbyWidth = m_byAttachingIconWidth;
    *pbyHeight = m_byAttachingIconHeight;
}

void CWindowManager::AttachIcon(DWORD dwType, DWORD dwIndex, DWORD dwSlotNumber, BYTE byWidth, BYTE byHeight)
{
    m_bAttachingFlag = true;
    m_dwAttachingType = dwType;
    m_dwAttachingIndex = dwIndex;
    m_dwAttachingSlotNumber = dwSlotNumber;

    m_byAttachingIconWidth = byWidth;
    m_byAttachingIconHeight = byHeight;
}

void CWindowManager::SetAttachingFlag(bool bFlag)
{
    m_bAttachingFlag = bFlag;
}

void CWindowManager::SetAttachingRealSlotNumber(DWORD dwRealSlotNumber)
{
    m_dwAttachingRealSlotNumber = dwRealSlotNumber;
}

void CWindowManager::DeattachIcon()
{
    SetAttachingFlag(false);
    if (m_poMouseHandler)
        PyCallClassMemberFunc(m_poMouseHandler, "DeattachObject");
}

void CWindowManager::SetParent(CWindow *window, CWindow *pParentWindow)
{
    if (!window)
    {
        assert(!"CWindowManager::SetParent - There is no self window!");
        return;
    }

    if (!pParentWindow)
    {
        assert(!"There is no parent window");
        return;
    }

    if (window->HasParent())
    {
        auto pOldParentWindow = window->GetParent();
        if (pParentWindow == pOldParentWindow)
            return;

        pOldParentWindow->DeleteChild(window);
    }

    pParentWindow->AddChild(window);
    window->UpdateRect();
}

void CWindowManager::LockWindow(CWindow *win)
{
    if (m_lockWindow == win)
        return;

    if (m_lockWindow)
    {
        // Make sure we don't end up with multiple entries
        // (or NULLs) in the locking history.
        m_lockWindowList.remove(m_lockWindow);
        m_lockWindowList.push_back(m_lockWindow);
    }

    // Only child windows of |win| may be active at this time.
    if (win && m_focusWindow && !IsParentOf(m_focusWindow, win))
        SetFocus(nullptr);

    m_lockWindow = win;
    SetPointedWindowByPos(m_mouseX, m_mouseY);
}

void CWindowManager::UnlockWindow()
{
    if (!m_lockWindow)
        return;

    if (m_lockWindowList.empty())
    {
        m_lockWindow = nullptr;
        KillFocus();
    }
    else
    {
        m_lockWindow = m_lockWindowList.back();
        m_lockWindowList.pop_back();
    }

    SetPointedWindowByPos(m_mouseX, m_mouseY);
}

void CWindowManager::SetFocus(CWindow *win)
{
    if (win == m_focusWindow)
        return;

    if (m_focusWindow)
    {
        m_focusWindowList.remove(m_focusWindow);
        m_focusWindowList.push_back(m_focusWindow);
        m_focusWindow->OnKillFocus();
    }

    m_focusWindow = win;

    if (m_focusWindow)
    {
        // Activating a window will automatically move it to the
        // top. This has to happen recursively, because its parent might
        // be partially occluded by another window (and so on...).
        SetTopRecursive(m_focusWindow);
        m_focusWindow->OnSetFocus();
    }
}

void CWindowManager::KillFocus()
{
    if (!m_focusWindow)
        return;

    if (m_focusWindowList.empty())
    {
        m_focusWindow->OnKillFocus();
        m_focusWindow = NULL;
    }
    else
    {
        m_focusWindow->OnKillFocus();

        m_focusWindow = m_focusWindowList.back();
        m_focusWindowList.pop_back();

        m_focusWindow->OnSetFocus();
    }
}

void CWindowManager::CaptureMouse(CWindow *win)
{
    if (win == m_captureWindow)
        return;

    if (m_captureWindow)
    {
        m_captureWindowList.remove(m_captureWindow);

        m_captureWindowList.push_back(m_captureWindow);
    }

    m_captureWindow = win;
}

void CWindowManager::ReleaseMouse(CWindow *win)
{
    if (!win || m_captureWindow == win)
    {
        ReleaseMouse();
        return;
    }

    m_captureWindowList.remove(win);
}

void CWindowManager::ReleaseMouse()
{
    if (!m_captureWindow)
        return;

    if (m_captureWindowList.empty())
    {
        m_captureWindow = nullptr;
    }
    else
    {
        m_captureWindow = m_captureWindowList.back();
        m_captureWindowList.pop_back();
    }
}

void CWindowManager::SetTop(CWindow *win)
{
    auto parent = win->GetParent();
    if (!parent)
        return;

    parent->SetTop(win);
}

void CWindowManager::SetResolution(int hres, int vres)
{
    if (hres <= 0 || vres <= 0)
        return;

    m_hres = hres;
    m_vres = vres;
}

float CWindowManager::GetAspect()
{
    return m_hres / float(m_vres);
}

void CWindowManager::SetScreenSize(int32_t width, int32_t height)
{
    m_width = width;
    m_height = height;

    m_rootWindow->SetSize(width, height);

    for (auto layer : m_rootWindow->GetChildren())
        layer->SetSize(width, height);
}

void CWindowManager::PruneLazyDeletionQueue()
{
    for (const auto &win : m_lazyDeleteQueue)
        delete win;

    m_lazyDeleteQueue.clear();
}

void CWindowManager::Update()
{
    PruneLazyDeletionQueue();

    m_rootWindow->Update();
}

void CWindowManager::Render()
{
    rmt_ScopedCPUSample(WindowManagerRender, 0);

    m_rootWindow->Render();
}

CWindow *CWindowManager::PickWindow(int32_t x, int32_t y)
{
    if (m_lockWindow)
        return m_lockWindow->PickWindow(x, y);

    CWindow *win = nullptr;

    if (m_captureWindow && m_captureWindow->IsIn(x, y))
    {
        win = m_captureWindow->PickWindow(x, y);
        if (win)
            return win;
    }

    for (auto* layer : boost::adaptors::reverse(m_rootWindow->GetChildren()))
    {
        win = layer->PickWindow(x, y);
        if (win != layer)
            return win;
    }

    return NULL;
}

void CWindowManager::SetPointedWindowByPos(int32_t x, int32_t y)
{
    auto fOut = [](CWindow *window) -> bool {
        window->OnMouseOverOut();
        return false;
    };

    auto fIn = [](CWindow *window) -> bool {
        window->OnMouseOverIn();
        return false;
    };

    auto* wnd = PickWindow(x, y);
    if (m_pointWindow != wnd)
    {
        if (m_pointWindow)
            TraverseWindowChainUp(m_pointWindow, fOut);

        m_pointWindow = wnd;

        if (m_pointWindow)
            TraverseWindowChainUp(m_pointWindow, fIn);
    }
}

std::tuple<int32_t, int32_t> CWindowManager::GetMousePosition()
{
    return std::make_tuple(m_mouseX, m_mouseY);
}

void CWindowManager::RunMouseMove(int32_t x, int32_t y)
{
    if (m_hres == 0 || m_vres == 0)
        return;

    if (IsAttaching())
    {
        if (x > m_width)
            x = m_width;
        if (y > m_height)
            y = m_height;
    }

    x = m_width * x / m_hres;
    y = m_height * y / m_vres;

    SetPointedWindowByPos(x, y);

    if (m_mouseX == x && m_mouseY == y)
        return;

    m_mouseX = x;
    m_mouseY = y;

    // Callbacks from e.g. SetPosition() could change the capture-window
    // We need to grab a reference here to avoid
    auto captureWnd = m_captureWindow;
    if (captureWnd)
    {
        if (captureWnd->IsFlag(CWindow::FLAG_MOVABLE))
        {
            auto x = m_mouseX - m_dragX;
            auto y = m_mouseY - m_dragY;

            if (captureWnd->HasParent())
            {
                const auto &r = captureWnd->GetParent()->GetRect();
                x -= r.left;
                y -= r.top;
            }

            auto lx = captureWnd->GetPositionX();
            auto ly = captureWnd->GetPositionY();

            if (captureWnd->IsFlag(CWindow::FLAG_RESTRICT_X))
                x = lx;

            if (captureWnd->IsFlag(CWindow::FLAG_RESTRICT_Y))
                y = ly;

            captureWnd->SetPosition(x, y);
            captureWnd->OnMoveWindow(x, y);
        }
        else if (captureWnd->IsFlag(CWindow::FLAG_DRAGABLE))
        {
            auto x = m_mouseX - m_dragX;
            auto y = m_mouseY - m_dragY;

            if (captureWnd->HasParent())
            {
                const auto &r = captureWnd->GetParent()->GetRect();
                x -= r.left;
                y -= r.top;
            }

            auto lx = captureWnd->GetPositionX();
            auto ly = captureWnd->GetPositionY();

            captureWnd->OnMouseDrag(x, y);
        }

        return;
    }

    if (m_pointWindow)
        m_pointWindow->OnMouseOver();
}

void CWindowManager::RunMouseLeftButtonDown()
{
    if (m_captureWindow && m_captureWindow->OnMouseLeftButtonDown())
        return;

    auto win = GetPointWindow();
    if (!win)
        return;

    // Attach
    if (win->IsFlag(CWindow::FLAG_ATTACH))
        win = win->GetRoot();

    m_dragX = m_mouseX - win->GetRect().left;
    m_dragY = m_mouseY - win->GetRect().top;

    m_pickedX = m_mouseX;
    m_pickedY = m_mouseY;

    if (m_lazyDeleteQueue.end() == std::find(m_lazyDeleteQueue.begin(), m_lazyDeleteQueue.end(), win))
        m_mouseLeftWindow = win;

    if (win->OnMouseLeftButtonDown())
        return;

    auto focus = FindWindowUpwardsByFlag(win, CWindow::FLAG_FOCUSABLE);
    if (focus)
        SetFocus(focus);

    // We currently do all the work for windows
    // having the movable/draggable flag set.
    // Is this intented?
    if (nullptr == m_captureWindow && win->IsFlag(CWindow::FLAG_MOVABLE | CWindow::FLAG_DRAGABLE))
        CaptureMouse(win);
}

void CWindowManager::RunMouseLeftButtonUp()
{
    if (m_captureWindow)
    {
        // We currently do all the work for windows
        // having the movable/draggable flag set.
        // Is this intented?
        auto win = m_captureWindow;
        if (win->IsFlag(CWindow::FLAG_MOVABLE | CWindow::FLAG_DRAGABLE))
            ReleaseMouse();

        if (win->OnMouseLeftButtonUp())
            return;
    }

    bool processed = false;
    if (m_mouseLeftWindow)
    {
        processed = m_mouseLeftWindow->OnMouseLeftButtonUp();
        m_mouseLeftWindow = nullptr;
    }

    if (!processed)
    {
        auto win = GetPointWindow();
        if (win)
            win->OnMouseLeftButtonUp();
    }
}

void CWindowManager::RunMouseLeftButtonDoubleClick()
{
    if (m_captureWindow && m_captureWindow->OnMouseLeftButtonDoubleClick())
        return;

    auto win = GetPointWindow();
    if (!win)
        return;

    win->OnMouseLeftButtonDoubleClick();
}

void CWindowManager::RunMouseRightButtonDown()
{
    if (m_captureWindow && m_captureWindow->OnMouseRightButtonDown())
        return;

    auto win = GetPointWindow();
    if (!win)
        return;

    if (win->IsFlag(CWindow::FLAG_ATTACH))
        win = win->GetRoot();

    win->OnMouseRightButtonDown();

    if (m_lazyDeleteQueue.end() == std::find(m_lazyDeleteQueue.begin(), m_lazyDeleteQueue.end(), win))
        m_mouseRightWindow = win;

    auto focus = FindWindowUpwardsByFlag(win, CWindow::FLAG_FOCUSABLE);
    if (focus)
        SetFocus(focus);
}

void CWindowManager::RunMouseRightButtonUp()
{
    if (m_captureWindow && m_captureWindow->OnMouseRightButtonUp())
        return;

    bool processed = false;

    if (m_mouseRightWindow)
    {
        processed = m_mouseRightWindow->OnMouseRightButtonUp();
        m_mouseRightWindow = nullptr;
        return;
    }

    if (!processed)
    {
        auto win = GetPointWindow();
        if (win)
            win->OnMouseRightButtonUp();
    }

    DeattachIcon();
}

void CWindowManager::RunMouseRightButtonDoubleClick()
{
    if (m_captureWindow && m_captureWindow->OnMouseRightButtonDoubleClick())
        return;

    auto win = GetPointWindow();
    if (win)
        win->OnMouseRightButtonDoubleClick();
}

void CWindowManager::RunMouseMiddleButtonDown()
{
    if (m_captureWindow && m_captureWindow->OnMouseMiddleButtonDown())
        return;

    auto win = GetPointWindow();
    if (!win)
        return;

    win->OnMouseMiddleButtonDown();

    if (m_lazyDeleteQueue.end() == std::find(m_lazyDeleteQueue.begin(), m_lazyDeleteQueue.end(), win))
        m_mouseMiddleWindow = win;

    auto focus = FindWindowUpwardsByFlag(win, CWindow::FLAG_FOCUSABLE);
    if (focus)
        SetFocus(focus);
}

void CWindowManager::RunMouseMiddleButtonUp()
{
    if (m_captureWindow && m_captureWindow->OnMouseMiddleButtonUp())
        return;

    if (m_mouseMiddleWindow)
    {
        m_mouseMiddleWindow->OnMouseMiddleButtonUp();
        m_mouseMiddleWindow = nullptr;
        return;
    }

    auto win = GetPointWindow();
    if (!win)
        return;

    win->OnMouseMiddleButtonUp();
}

bool CWindowManager::RunMouseWheelEvent(int nLen)
{
    if (m_captureWindow)
        return m_captureWindow->OnMouseWheelEvent(nLen);

    if (m_lockWindow)
        return m_lockWindow->OnMouseWheelEvent(nLen);

    auto f = [nLen](CWindow *window) -> bool { return window->OnMouseWheelEvent(nLen); };

    if (m_pointWindow && m_pointWindow->IsRendering())
        if (TraverseWindowChainDown(m_pointWindow, f))
            return true;

    if (m_focusWindow && m_focusWindow->IsRendering())
        if (TraverseWindowChainDown(m_focusWindow, f))
            return true;

    if (TraverseWindowChainDown(m_rootWindow.get(), f))
        return true;

    return false;
}

void CWindowManager::RunTab()
{
    // TODO(tim): Rewrite!
    if (m_lockWindow)
    {
        m_lockWindow->OnTab();
        return;
    }

    if (m_focusWindow && m_focusWindow->IsRendering())
        if (m_focusWindow->OnTab())
            return;

    auto f = [](CWindow *window) -> bool { return window->OnTab(); };

    TraverseWindowChainDown(m_rootWindow.get(), f);
}

void CWindowManager::RunChar(uint32_t ch)
{
    auto f = [ch](CWindow *window) -> bool { return window->OnChar(ch); };

    if (m_focusWindow && TraverseWindowChainUp(m_focusWindow, f))
        return;

    if (TraverseWindowChainDown(m_rootWindow.get(), f))
        return;
}

void CWindowManager::RunKeyDown(KeyCode code)
{
    auto f = [code](CWindow *window) -> bool { return window->OnKeyDown(code); };

    CWindow *win = nullptr;

    if (m_focusWindow)
        win = TraverseWindowChainUp(m_focusWindow, f);

    if (!win)
        win = TraverseWindowChainDown(m_rootWindow.get(), f);

    if (win && m_lazyDeleteQueue.end() == std::find(m_lazyDeleteQueue.begin(), m_lazyDeleteQueue.end(), win))
        m_keyHandlers[code] = win;
}

void CWindowManager::RunKeyUp(KeyCode code)
{
    auto f = [code](CWindow *window) -> bool { return window->OnKeyUp(code); };

    auto it = m_keyHandlers.find(code);
    if (it != m_keyHandlers.end())
    {
        TraverseWindowChainUp(it->second, f);
        TraverseWindowChainDown(m_rootWindow.get(), f);
        m_keyHandlers.erase(it);
        return;
    }

    if (m_focusWindow && TraverseWindowChainUp(m_focusWindow, f))
        return;

    if (TraverseWindowChainDown(m_rootWindow.get(), f))
        return;
}

void CWindowManager::RunPressExitKey()
{
    auto f = [](CWindow *window) -> bool { return window->OnPressExitKey(); };

    if (m_focusWindow && TraverseWindowChainUp(m_focusWindow, f))
        return;

    if (TraverseWindowChainDown(m_rootWindow.get(), f))
        return;
}

void CWindowManager::NotifyHideWindow(CWindow *win)
{
    while (IsParentOf(m_focusWindow, win))
        KillFocus();

    while (IsParentOf(m_lockWindow, win))
        UnlockWindow();

    while (IsParentOf(m_captureWindow, win))
        ReleaseMouse();

    if (IsParentOf(m_mouseLeftWindow, win))
        m_mouseLeftWindow = nullptr;

    if (IsParentOf(m_mouseRightWindow, win))
        m_mouseRightWindow = nullptr;

    if (IsParentOf(m_mouseMiddleWindow, win))
        m_mouseMiddleWindow = nullptr;

    if (IsParentOf(m_pointWindow, win))
        SetPointedWindowByPos(m_mouseX, m_mouseY);

    auto f = [win](CWindow *w) -> bool { return IsParentOf(w, win); };

    m_lockWindowList.remove_if(f);
    m_focusWindowList.remove_if(f);
    m_captureWindowList.remove_if(f);

    for (auto first = m_keyHandlers.begin(), last = m_keyHandlers.end(); first != last;)
    {
        if (IsParentOf(first->second, win))
            first = m_keyHandlers.erase(first);
        else
            ++first;
    }
}

CWindow *CWindowManager::GetLayerByName(const std::string &name)
{
    uint32_t index;
    if (!storm::ParseStringWithTable(name, index, kLayers))
        return nullptr;

    const auto &layers = m_rootWindow->GetChildren();
    if (index >= layers.size())
        return nullptr;

    return layers[index];
}

void CWindowManager::SetTopRecursive(CWindow *win)
{
    auto f = [this](CWindow *w) -> bool {
        SetTop(w);
        return false; // Continue traversal
    };

    TraverseWindowChainUp(win, f);
}

void CWindowManager::PopClipRegion()
{
    if (!clipStack_.empty())
        clipStack_.pop();

    CommitClipRegion();
}

bool CWindowManager::CommitClipRegion()
{
    if (clipStack_.empty())
    {
        STATEMANAGER.GetDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        return true;
    }

    auto top = clipStack_.top();
    D3DRECT region;
    region.x1 = (LONG)(top.x);
    region.y1 = (LONG)(top.y);
    region.x2 = (LONG)(top.z);
    region.y2 = (LONG)(top.w);

    if ((region.x2 - region.x1 > 0.f) && (region.y2 - region.y1 > 0.f))
    {
        RECT rect;
        rect.left = region.x1;
        rect.right = region.x2;
        rect.top = region.y1;
        rect.bottom = region.y2;

        STATEMANAGER.GetDevice()->SetScissorRect(&rect);
        return (STATEMANAGER.GetDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE) == S_OK);
    }
    else
        return false;
}

bool CWindowManager::PushClipRegion(const Vector4 &cr)
{
    Vector4 a(cr);

    if (!clipStack_.empty())
    {
        Vector4 b = clipStack_.top();
        a.x = std::max(a.x, b.x);
        a.y = std::min(a.y, b.y);
        a.z = std::min(a.z, b.z);
        a.w = std::max(a.w, b.w);
    }

    clipStack_.push(a);

    bool success = CommitClipRegion();

    if (!success)
        clipStack_.pop();

    return success;
}

bool CWindowManager::PushClipRegion(CWindow *win)
{
    Vector2 pixelToClip(this->PixelToClip());

    Vector4 v(win->GetRect().left + win->GetPositionX(), win->GetRect().top + win->GetPositionY(),
                  win->GetRect().left + win->GetWidth(), win->GetRect().top + win->GetHeight());
    return PushClipRegion(v);
}
} // namespace UI

bool PyTuple_GetWindow(PyObject *poArgs, int pos, UI::CWindow **ppRetWindow)
{
    PyObject *iHandle;
    if (!PyTuple_GetObject(poArgs, pos, &iHandle))
        return false;

    if (!iHandle)
        return false;

    if (!PyCapsule_CheckExact(iHandle))
        return false;

    if (auto *ptr = PyCapsule_GetPointer(iHandle, nullptr); ptr)
    {
        *ppRetWindow = static_cast<UI::CWindow *>(ptr);

        return true;
    }

    return false;
}
