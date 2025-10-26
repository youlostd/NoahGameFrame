#include "StdAfx.h"
#include "EditControl.hpp"
#include "PythonWindowManager.h"
#include "../EterLib/GrpDevice.h"

namespace UI
{
namespace
{
struct ApplyRenderState
{
    ApplyRenderState()
    {
        STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
        STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);

        Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

        STATEMANAGER.SetTexture(0, NULL);
    }

    ~ApplyRenderState()
    {
        STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

        STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
		STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
    }
};

void RenderSelectionRect(float sx, float sy, float ex, float ey)
{
    const TDiffuse diffuse = DirectX::SimpleMath::Color(0.00f, 0.00f, 1.00f, 0.35f).BGRA().c;

    TPDTVertex vertices[4];
    vertices[0].diffuse = diffuse;
    vertices[1].diffuse = diffuse;
    vertices[2].diffuse = diffuse;
    vertices[3].diffuse = diffuse;
    vertices[0].position = TPosition(sx, sy, 0.0f);
    vertices[1].position = TPosition(ex, sy, 0.0f);
    vertices[2].position = TPosition(sx, ey, 0.0f);
    vertices[3].position = TPosition(ex, ey, 0.0f);

    Engine::GetDevice().SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    if (Engine::GetDevice().SetPDTStream(vertices, 4))
        Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
}
}

EditControl::EditControl()
    : CTextLine()
      , m_focused(false)
      , m_renderCursor(false)
      , m_cursorVisibilityTime(0)
      , m_cursorPosition(0)
      , m_selectionStart(0)
      , m_selectionEnd(0)
      , m_maxLength(0)
      , m_maxVisibleLength(0)
{
    AddFlag(FLAG_DRAGABLE | FLAG_FOCUSABLE);
}

void EditControl::Enable()
{
    m_enabled = true;
}

void EditControl::Disable()
{
    m_enabled = false;
}

void EditControl::SetMax(uint32_t count)
{
    m_maxLength = count;
}

void EditControl::SetMaxVisible(uint32_t count)
{
    m_maxVisibleLength = count;
}

uint32_t EditControl::GetCursorPosition() const
{
    return m_cursorPosition;
}

void EditControl::SetCursorPosition(uint32_t position)
{
    m_cursorPosition = std::max<uint32_t>(position, m_text.length());
}

void EditControl::SetPlaceholderText(std::string text)
{
    m_placeholderText = text;
    m_placeholderInstance.SetTextPointer(Engine::GetFontManager().LoadFont(m_fontName));
    m_placeholderInstance.SetSecret(m_instance.IsSecret());
    m_placeholderInstance.SetValue(text);
    m_placeholderInstance.Update();
}

void EditControl::SetPlaceholderColor(DirectX::SimpleMath::Color color)
{
    m_placeHolderColor = color;
    m_placeholderInstance.SetColor(color.BGRA().c);
    m_placeholderInstance.Update();
}

void EditControl::MoveCursor(int32_t offset, bool updateSelection)
{
    const auto glyphCount = m_instance.GetGlyphCount();
    const auto oldP = m_instance.GetGlyphIndexFromStringIndex(m_cursorPosition);
    const auto newP = std::clamp<int32_t>(oldP + offset, 0, glyphCount);

    m_cursorPosition = m_instance.GetStringIndexFromGlyphIndex(newP);

    if (updateSelection)
        m_selectionEnd = m_cursorPosition;
    else
        m_selectionStart = m_selectionEnd = m_cursorPosition;
}

void EditControl::MoveToBeginning()
{
    m_cursorPosition = m_selectionStart = m_selectionEnd = 0;
}

void EditControl::MoveToEnd()
{
    m_cursorPosition = m_selectionStart = m_selectionEnd = m_text.length();
}

bool EditControl::Insert(uint32_t offset, const std::string &text)
{
    if (0 != m_maxLength && m_text.length() + text.length() > m_maxLength)
        return false;

    if (0 != m_maxVisibleLength)
    {
        uint32_t count = m_instance.GetGlyphCount();
        if (count >= m_maxVisibleLength)
            return false;

        TextTag tag;
        for (auto first = text.begin(), last = text.end(); first != last;)
        {
            if (GetTextTag(std::string(first, last), tag))
            {
                first += tag.length;
                continue;
            }

            boost::locale::utf::utf_traits<char>::decode(first, last);
            if (++count > m_maxVisibleLength)
                return false;
        }
    }

    // We don't want people editing hyperlinks
    offset = GetTextTagBoundary(m_text, offset).first;

    m_text.insert(offset, text.data(), text.length());

    m_instance.SetValue(m_text);
    m_instance.Update();

    if (m_cursorPosition >= offset)
        m_cursorPosition += text.length();

    if (m_selectionEnd >= offset)
        m_selectionEnd += text.length();

    if (m_selectionStart >= offset)
        m_selectionStart += text.length();

    RunCallback("OnChange");
    return true;
}

void EditControl::Erase(uint32_t offset, uint32_t count)
{
    // We don't want people editing hyperlinks
    const auto begin = GetTextTagBoundary(m_text, offset).first;
    const auto end = GetTextTagBoundary(m_text, offset + count).second;

    m_text.erase(m_text.begin() + begin,
                 m_text.begin() + end);

    m_instance.SetValue(m_text);
    m_instance.Update();

    if (m_cursorPosition > begin)
        m_cursorPosition -= std::min(end - begin, m_cursorPosition - begin);

    if (m_selectionStart > begin)
        m_selectionStart -= std::min(end - begin, m_selectionStart - begin);

    if (m_selectionEnd > begin)
        m_selectionEnd -= std::min(end - begin, m_selectionEnd - begin);

    RunCallback("OnChange");
}

void EditControl::EraseGlyphs(uint32_t offset, uint32_t count)
{
    // Since we want to remove |count| shown characters, we translate
    // |offset| to display coordinates, add the amount of characters to remove
    // and then translate everything back.
    const auto glyphCount = m_instance.GetGlyphCount();

    const auto glyphStart = m_instance.GetGlyphIndexFromStringIndex(offset);
    const auto glyphEnd = std::min(glyphCount, glyphStart + count);

    const auto strStart = m_instance.GetStringIndexFromGlyphIndex(glyphStart);
    const auto strEnd = m_instance.GetStringIndexFromGlyphIndex(glyphEnd);

    Erase(strStart, strEnd - strStart);
}

/*virtual*/
void EditControl::OnUpdate()
{
    if (!m_text.empty() || m_renderCursor || m_focused)
    {
        CTextLine::OnUpdate();
    }
    else
    {
        m_placeholderInstance.Update();
    }

    if (m_focused && m_selectionStart == m_selectionEnd)
    {
        const auto now = ELTimer_GetMSec();
        if (now > m_cursorVisibilityTime)
        {
            m_renderCursor = !m_renderCursor;
            m_cursorVisibilityTime = now + 500;
        }
    }
}

/*virtual*/
void EditControl::OnRender()
{
    if (!m_text.empty() || m_renderCursor || m_focused)
    {
        CTextLine::OnRender();
    }
    else
    {
        m_placeholderInstance.Render(m_rect.left, m_rect.top, 0);
    }

    if (m_selectionStart != m_selectionEnd)
        RenderSelection();

    if (m_focused && m_renderCursor)
        RenderCursor();
}

/*virtual*/
void EditControl::OnSetFocus()
{
    if (!m_enabled)
        return;

    assert(!m_focused && "Sanity check");

    m_focused = true;
    ForceShowCursor();

    RunCallback("OnSetFocus");
}

/*virtual*/
void EditControl::OnKillFocus()
{
    if (!m_enabled)
        return;

    assert(m_focused && "Sanity check");

    m_focused = false;
    m_renderCursor = false;

    RunCallback("OnKillFocus");
}

/*virtual*/
bool EditControl::OnMouseLeftButtonDown()
{
    if (!m_enabled)
        return false;

    auto [x, y] = CWindowManager::Instance().GetMousePosition();
    MakeLocalPosition(x, y);

    const uint32_t pos = GetIndexFromPosition(x, y);

    m_cursorPosition = pos;
    m_selectionStart = pos;
    m_selectionEnd = pos;

    ForceShowCursor();
    return CWindow::OnMouseLeftButtonDown();
}

/*virtual*/
void EditControl::OnMouseDrag(int32_t x, int32_t y)
{
    if (!m_enabled)
        return;

    auto [lx, ly] = CWindowManager::Instance().GetMousePosition();
    MakeLocalPosition(lx, ly);

    m_selectionEnd = m_cursorPosition = GetIndexFromPosition(lx, ly);
    ForceShowCursor();
}

/*virtual*/
bool EditControl::OnChar(uint32_t ch)
{
    if (!m_enabled)
        return false;

    // We don't handle character messages if we're not focused.
    if (!m_focused)
        return false;

    // see: http://www.asciitable.com/
    if (ch < 32)
        return true;

    // Allow the python handler to process this event and block chars if needed
    // Note: It should usually return true, except for number mode
    if (!CWindow::OnChar(ch))
        return true;

    char buffer[512];
    char *end = boost::locale::utf::utf_traits<char>::encode(ch, buffer);

    // Escape | as ||
    if (ch == '|')
        end = boost::locale::utf::utf_traits<char>::encode(ch, end);

    if (m_selectionStart != m_selectionEnd)
        EraseSelection();

    if (!Insert(m_cursorPosition, std::string(buffer, end - buffer)))
    {
        return false;
    }

    // Hiding the cursor while the user is typing causes confusion.
    ForceShowCursor();

    return true;
}

/*virtual*/
bool EditControl::OnKeyDown(KeyCode code)
{
    if (!m_enabled)
        return false;

    if (!m_focused)
        return false;

    // This might only be necessary in a few cases
    ForceShowCursor();

    switch (code)
    {
    case kVirtualKeyLeft:
        MoveCursor(-1, GetKeyboardInput().IsKeyPressed(kVirtualKeyShift));
        return true;

    case kVirtualKeyRight:
        MoveCursor(1, GetKeyboardInput().IsKeyPressed(kVirtualKeyShift));
        return true;

    case kVirtualKeyHome:
        MoveToBeginning();
        return true;

    case kVirtualKeyEnd:
        MoveToEnd();
        return true;

    case kVirtualKeyDelete: {
        if (m_selectionStart != m_selectionEnd)
        {
            EraseSelection();
            return true;
        }

        EraseGlyphs(m_cursorPosition, 1);
        return true;
    }

    case kVirtualKeyBack: {
        if (m_selectionStart != m_selectionEnd)
        {
            EraseSelection();
            return true;
        }

        if (m_cursorPosition == 0)
            return true;

        // We want to remove the glyph preceding the current one.
        // This is a bit tricky...
        auto index = m_instance.GetGlyphIndexFromStringIndex(m_cursorPosition);
        --index;
        m_cursorPosition = m_instance.GetStringIndexFromGlyphIndex(index);

        EraseGlyphs(m_cursorPosition, 1);
        return true;
    }

    case kVirtualKeyV:
    case kVirtualKeyInsert: {
        // STRG+V only
        if (code == kVirtualKeyV &&
            !GetKeyboardInput().IsKeyPressed(kVirtualKeyControl))
            break;

        std::string text;
        auto cc = GetClipboardContent(CF_METIN2);
        if (cc.first)
        {
            text.assign(reinterpret_cast<char *>(cc.first.get()));
        }
        else
        {
            text = GetClipboardText();
            if (text.empty())
                return true;

            boost::replace_all(text, "|", "||");
        }

        if (!RunCallback("OnPaste", Py_BuildValue("(s)", text.c_str())))
            return true;

        if (m_selectionStart != m_selectionEnd)
            EraseSelection();

        if (!Insert(m_cursorPosition, text))
            return true;

        return true;
    }

    case kVirtualKeyX:
    case kVirtualKeyC: {
        if (!GetKeyboardInput().IsKeyPressed(kVirtualKeyControl))
            break;

        if (m_selectionStart == m_selectionEnd)
            return true;

        std::pair<uint32_t, uint32_t> pos = std::minmax(m_selectionStart, m_selectionEnd);

        // Don't corrupt text-tags by cutting them
        m_selectionStart = GetTextTagBoundary(m_text, pos.first).first;
        m_selectionEnd = GetTextTagBoundary(m_text, pos.second).second;

        if (!ClearClipboard())
            return true;

        std::string text(m_text.begin() + m_selectionStart,
                         m_text.begin() + m_selectionEnd);

        if (!SetClipboardContent(CF_METIN2, text.c_str(), text.length() + 1))
            return true;

        std::string stripped = StripTextTags(text);

        if (!SetClipboardText(stripped))
        {
            // Try clearing the CF_METIN2 stuff
            ClearClipboard();
            return true;
        }

        if (code == kVirtualKeyX)
            EraseSelection();

        return true;
    }

    case kVirtualKeyA: {
        if (!GetKeyboardInput().IsKeyPressed(kVirtualKeyControl))
            break;

        m_selectionStart = 0;
        m_selectionEnd = m_cursorPosition = m_text.length();
        return true;
    }

    default:
        break;
    }

    // Pass it up the chain - This will invoke OnKeyDown() on
    // the bound python object, allowing client code to handle
    // special keys (e.g. ENTER)
    return CWindow::OnKeyDown(code);
}

/*virtual*/
bool EditControl::OnKeyUp(KeyCode code)
{
    // We leave handling key up events to the phase window
    // returning "handled" here leads to undesired behavior
    // like key ups getting stuck
    return false;
}

/*virtual*/
void EditControl::OnChangeText()
{
    m_cursorPosition = std::min<uint32_t>(m_cursorPosition, m_text.length());
    m_selectionStart = std::min<uint32_t>(m_selectionStart, m_text.length());
    m_selectionEnd = std::min<uint32_t>(m_selectionEnd, m_text.length());

    m_instance.Update();
}

void EditControl::EraseSelection()
{
    if (m_selectionStart > m_selectionEnd)
        std::swap(m_selectionStart, m_selectionEnd);

    // We don't want people editing hyperlinks
    m_selectionStart = GetTextTagBoundary(m_text, m_selectionStart).first;
    m_selectionEnd = GetTextTagBoundary(m_text, m_selectionEnd).second;

    Erase(m_selectionStart, m_selectionEnd - m_selectionStart);
    m_cursorPosition = m_selectionEnd = m_selectionStart;
}

void EditControl::RenderSelection()
{
    auto first = m_instance.GetGlyphIndexFromStringIndex(m_selectionStart);
    auto last = m_instance.GetGlyphIndexFromStringIndex(m_selectionEnd);

    // Ensure proper ordering
    // (m_selectionStart isn't guranteed to be < m_selectionEnd)
    if (first > last)
        std::swap(first, last);

    ApplyRenderState state;

    auto firstLine = m_instance.GetGlyphLine(first);
    const auto lastLine = m_instance.GetGlyphLine(last);

    do
    {
        uint32_t lineFirst, lineLast;
        m_instance.GetLineGlyphs(firstLine, lineFirst, lineLast);

        if (first > lineFirst && first < lineLast)
            lineFirst = first;
        if (last > lineFirst && last < lineLast)
            lineLast = last;

        if (lineFirst == lineLast)
            continue;

        // Dummy coordinates we don't need
        float _1, _2;

        float sx, sy, ex, ey;
        m_instance.GetGlyphPosition(lineFirst, sx, sy, _1, _2);
        m_instance.GetGlyphPosition(lineLast - 1, _1, _2, ex, ey);

        sx += m_rect.left;
        sy += m_rect.top;
        ex += m_rect.left;
        ey += m_rect.top;

        RenderSelectionRect(sx, sy, ex, ey);
    }
    while (firstLine++ != lastLine);
}

void EditControl::RenderCursor()
{
    const auto glyph = m_instance.GetGlyphIndexFromStringIndex(m_cursorPosition);

    float sx, sy, ex, ey;
    m_instance.GetGlyphPosition(glyph, sx, sy, ex, ey);
    ex = sx + 1.0f; // Cursor width is hardcoded

    sx += m_rect.left;
    sy += m_rect.top;
    ex += m_rect.left;
    ey += m_rect.top;

    TDiffuse diffuse = 0xffffffff;

    TPDTVertex vertices[4];
    vertices[0].diffuse = diffuse;
    vertices[1].diffuse = diffuse;
    vertices[2].diffuse = diffuse;
    vertices[3].diffuse = diffuse;
    vertices[0].position = TPosition(sx, sy, 0.0f);
    vertices[1].position = TPosition(ex, sy, 0.0f);
    vertices[2].position = TPosition(sx, ey, 0.0f);
    vertices[3].position = TPosition(ex, ey, 0.0f);

    ApplyRenderState state;

    Engine::GetDevice().SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    if (Engine::GetDevice().SetPDTStream(vertices, 4))
        Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
}

uint32_t EditControl::GetIndexFromPosition(int32_t x, int32_t y) const
{
    x = std::clamp<int32_t>(x, 0, m_instance.GetWidth());
    y = std::clamp<int32_t>(y, 0, m_instance.GetHeight());

    uint32_t glyphIndex;
    if (m_instance.GetGlyphIndex(glyphIndex, x, y))
        return m_instance.GetStringIndexFromGlyphIndex(glyphIndex);

    return m_text.length();
}

void EditControl::ForceShowCursor()
{
    m_cursorVisibilityTime = ELTimer_GetMSec() + 500;
    m_renderCursor = true;
}
}
