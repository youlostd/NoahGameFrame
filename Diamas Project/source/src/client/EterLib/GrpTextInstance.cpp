#include "StdAfx.h"
#include "GrpTextInstance.h"
#include "StateManager.h"
#include "IME.h"
#include "TextTag.h"
#include "../EterBase/Utils.h"
#include "../EterLib/GrpImage.h"
#include "../EterLib/ResourceManager.h"
#include "../GameLib/GameType.h"

#include "../GameLib/ItemManager.h"
#include "../EterBase/UnicodeUtil.hpp"

#define CHARACTER_NAME_MAX_LEN 36

#include "Engine.hpp"
#include "GrpDevice.h"
#include "../GameLib/NpcManager.h"

#include <utf8.h>
#include <boost/locale/utf.hpp>

#include "../EterPythonLib/PythonGraphic.h"
#include "../GameLib/EmojiManager.h"

const float c_fFontFeather = 0.5f;

CGraphicTextInstance::CGraphicTextInstance()
    : m_textHasGradient{false}
      , m_dwTextColor(0xFFFFFFFF)
      , m_textColor2{0xFFFFFFFF}
      , m_dwOutLineColor(0xFF000000)
      , m_textWidth(0)
      , m_textHeight(0)
      , m_maxLineHeight(0.0f)
      , m_fLimitWidth(1600.0f) // NOTE : 해상도의 최대치. 이보다 길게 쓸 일이 있을까? - [levites]
      , m_isSecret(false)
      , m_isMultiLine(false)
      , m_isOutline(false)
      , m_dirty(true)
      , m_font()
      , m_sourceLength(0)
      , m_lineCount(0)
{
    // ctor
}

CGraphicTextInstance::~CGraphicTextInstance()
{
    for (auto &ch : m_chars)
        delete ch.emoji;

    m_chars.clear();
}

void CGraphicTextInstance::Update()
{
    if (!m_dirty) // 문자열이 바뀌었을 때만 업데이트 한다.
        return;

    if (!m_font)
    {
        return;
    }

    CGraphicFontTexture &texture = m_font->GetTexturePointer();

    m_maxLineHeight = m_font->getFontInfo().ascender;
    m_textWidth = 0;
    m_lineCount = 0;

    CGraphicFontTexture::GlyphData *replace = nullptr;
    if (m_isSecret)
    {
        replace = texture.GetCharacterInfomation('*');
        assert(replace && "No * character");
    }

    float curX = 0.0f;
    float curY = 0.0f;

    for (auto &ch : m_chars)
    {
        ch.info = replace ? replace : texture.GetCharacterInfomation(ch.ch);
        if ((ch.ch == '\n') && m_isMultiLine)
        {
            curX = 0.0f;
            curY += m_maxLineHeight;
            ++m_lineCount;

            // Ignore this character
            ch.info = nullptr;
            continue;
        }

        if ((ch.ch == ' '))
        {
            ch.x = curX;
            ch.y = curY;
            ch.line = m_lineCount;

            curX += m_font->GetTexturePointer().GetSpaceWidth();
            m_textWidth = std::max<uint16_t>(m_textWidth, curX);

            // Ignore this character
            ch.info = nullptr;
            continue;
        }

        if (ch.ch == '\t')
        {
            ch.x = curX;
            ch.y = curY;
            ch.line = m_lineCount;

            curX += (m_font->GetTexturePointer().GetSpaceWidth()) * 4;
            m_textWidth = std::max<uint16_t>(m_textWidth, curX);

            // Ignore this character
            ch.info = nullptr;
            continue;
        }

        if (!ch.info)
            continue;

        if (curX + ch.info->_width > m_fLimitWidth)
        {
            if (m_isMultiLine)
            {
                curX = 0.0f;
                curY += m_maxLineHeight;
                ++m_lineCount;
            }
            else
            {
                // Ignore this character
                ch.info = nullptr;
                break;
            }
        }

        m_maxLineHeight = std::max<float>(m_maxLineHeight, ch.info->_height);
        m_textWidth = std::max<uint16_t>(m_textWidth, curX + ch.info->_advance);

        ch.x = curX;
        ch.y = curY;
        ch.line = m_lineCount;

        curX += ch.info->_advance;
    }

    ++m_lineCount;
    m_textHeight = curY + m_maxLineHeight;

    m_dirty = false;
}

bool LiesEntirelyOutsideRect(const RECT &r,
                             const Vector2 &start,
                             const Vector2 &end)
{
    return (start.x >= r.right || end.x < r.left) &&
           (start.y >= r.bottom || end.y < r.top);
}

bool LiesEntirelyInsideRect(const RECT &r,
                            const Vector2 &start,
                            const Vector2 &end)
{
    return start.x >= r.left && end.x < r.right &&
           start.y >= r.top && end.y < r.bottom;
}

struct TextRenderState
{
    TextRenderState()
    {
        Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

        STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        STATEMANAGER.SaveRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

        STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);

        CPythonGraphic::instance().SetBlendOperation();

        STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

        STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
        STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
        STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
    }

    ~TextRenderState()
    {
        STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
        STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_FILLMODE);
        STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

        STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);

        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);

        STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
        STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
        STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);

        //STATEMANAGER.RestoreFVF();
    }
};

void CGraphicTextInstance::Render(int32_t x, int32_t y, int32_t z,
                                  const RECT *clip)
{
    if (m_dirty)
        return;

    if (!m_font)
        return;

    D3DPERF_BeginEvent(
        D3DCOLOR_ARGB(255, 50, 50, 0), L"** CGraphicTextInstance::Render **");

    const Vector2 pos(x, y);

    const auto fontInfo = m_font->getFontInfo();

    std::map<CGraphicImageTexture *, std::vector<TPDTVertex>> verticesMap;

    TPDTVertex vertices[4];
    vertices[0].position.z = z;
    vertices[1].position.z = z;
    vertices[2].position.z = z;
    vertices[3].position.z = z;

    const float fFontHalfWeight = 1.0f;
    constexpr float texelOfset = 0.5f;
    // 테두리
    // 테두리
    if (m_isOutline)
    {
        for (const auto &ch : m_chars)
        {
            const auto pCurCharInfo = ch.info;
            if (!ch.info) // skip undrawable chars
                continue;

            if (ch.emoji)
                continue;

            const Vector2 size(pCurCharInfo->_width, pCurCharInfo->_height);
            const Vector2 start(ch.x + pCurCharInfo->_bearingX,
                                    ch.y + fontInfo.ascender + pCurCharInfo->_bearingY);
            const Vector2 end = start + size;

            // Don't even bother sending those to the driver...
            if (clip && LiesEntirelyOutsideRect(*clip, start, end))
                continue;

            const auto tex = ch.info->_tex;
            auto &batchVertices = verticesMap[tex];

            vertices[0].texCoord.x = pCurCharInfo->_left;
            vertices[0].texCoord.y = pCurCharInfo->_top;

            vertices[1].texCoord.x = pCurCharInfo->_left;
            vertices[1].texCoord.y = pCurCharInfo->_bottom;

            vertices[2].texCoord.x = pCurCharInfo->_right;
            vertices[2].texCoord.y = pCurCharInfo->_top;

            vertices[3].texCoord.x = pCurCharInfo->_right;
            vertices[3].texCoord.y = pCurCharInfo->_bottom;

            vertices[3].diffuse = m_dwOutLineColor;
            vertices[2].diffuse = m_dwOutLineColor;
            vertices[1].diffuse = m_dwOutLineColor;
            vertices[0].diffuse = m_dwOutLineColor;

            vertices[0].position.y = pos.y + start.y - texelOfset;
            vertices[1].position.y = pos.y + end.y - texelOfset;
            vertices[2].position.y = pos.y + start.y - texelOfset;
            vertices[3].position.y = pos.y + end.y - texelOfset;

            // 왼
            vertices[0].position.x = pos.x + start.x - texelOfset;
            vertices[1].position.x = pos.x + start.x - texelOfset;
            vertices[2].position.x = pos.x + end.x - texelOfset;
            vertices[3].position.x = pos.x + end.x - texelOfset;

            batchVertices.push_back(vertices[0]);
            batchVertices.push_back(vertices[1]);
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[1]); //[1]
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[3]);

            vertices[0].position.y = pos.y + start.y - texelOfset;
            vertices[1].position.y = pos.y + end.y - texelOfset;
            vertices[2].position.y = pos.y + start.y - texelOfset;
            vertices[3].position.y = pos.y + end.y - texelOfset;

            // 왼
            vertices[0].position.x = pos.x + start.x - fFontHalfWeight - texelOfset;
            vertices[1].position.x = pos.x + start.x - fFontHalfWeight - texelOfset;
            vertices[2].position.x = pos.x + end.x - fFontHalfWeight - texelOfset;
            vertices[3].position.x = pos.x + end.x - fFontHalfWeight - texelOfset;

            batchVertices.push_back(vertices[0]);
            batchVertices.push_back(vertices[1]);
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[1]); //[1]
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[3]);

            // 오른
            vertices[0].position.x = pos.x + start.x + fFontHalfWeight - texelOfset;
            vertices[1].position.x = pos.x + start.x + fFontHalfWeight - texelOfset;
            vertices[2].position.x = pos.x + end.x + fFontHalfWeight - texelOfset;
            vertices[3].position.x = pos.x + end.x + fFontHalfWeight - texelOfset;

            batchVertices.push_back(vertices[0]);
            batchVertices.push_back(vertices[1]);
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[1]); //[1]
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[3]);

            vertices[0].position.x = pos.x + start.x - texelOfset;
            vertices[1].position.x = pos.x + start.x - texelOfset;
            vertices[2].position.x = pos.x + end.x - texelOfset;
            vertices[3].position.x = pos.x + end.x - texelOfset;

            // 위
            vertices[0].position.y = pos.y + start.y - fFontHalfWeight - texelOfset;
            vertices[1].position.y = pos.y + end.y - fFontHalfWeight - texelOfset;
            vertices[2].position.y = pos.y + start.y - fFontHalfWeight - texelOfset;
            vertices[3].position.y = pos.y + end.y - fFontHalfWeight - texelOfset;

            batchVertices.push_back(vertices[0]);
            batchVertices.push_back(vertices[1]);
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[1]); //[1]
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[3]);

            // 아래
            vertices[0].position.y = pos.y + start.y + fFontHalfWeight - texelOfset;
            vertices[1].position.y = pos.y + end.y + fFontHalfWeight - texelOfset;
            vertices[2].position.y = pos.y + start.y + fFontHalfWeight - texelOfset;
            vertices[3].position.y = pos.y + end.y + fFontHalfWeight - texelOfset;

            batchVertices.push_back(vertices[0]);
            batchVertices.push_back(vertices[1]);
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[1]); //[1]
            batchVertices.push_back(vertices[2]);
            batchVertices.push_back(vertices[3]);
        }
    }

    DWORD clippingWasEnabled = STATEMANAGER.GetRenderState(D3DRS_SCISSORTESTENABLE);;
    RECT oldRect;
    RECT newRect;

    if (clip)
    {
        const RECT r = {
            pos.x + clip->left,
            pos.y + clip->top,
            pos.x + clip->right,
            pos.y + clip->bottom,
        };

        STATEMANAGER.GetDevice()->GetScissorRect(&oldRect);
        STATEMANAGER.GetDevice()->SetScissorRect(&r);
        STATEMANAGER.SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    }

    for (const auto &ch : m_chars)
    {
        auto const pCurCharInfo = ch.info;
        if (!ch.info)
        {
            continue;
        }

        const Vector2 size(pCurCharInfo->_width, pCurCharInfo->_height);
        const Vector2 start(ch.x + pCurCharInfo->_bearingX,
                                ch.y + fontInfo.ascender + pCurCharInfo->
                                _bearingY);
        const Vector2 end = start + size;

        // Don't even bother sending those to the driver...
        if (clip && LiesEntirelyOutsideRect(*clip, start, end))
            continue;

#ifndef WORLD_EDITOR
        if (ch.emoji && Engine::GetSettings().IsEnableEmojiSystem())
        {
            ch.emoji->SetPosition(pos.x + ch.x, pos.y + ch.y);
            ch.emoji->Render();
            continue;
        }
#endif

        auto* tex = ch.info->_tex;
        auto &batchVertices = verticesMap[tex];

        vertices[0].position.x = pos.x + start.x - texelOfset;
        vertices[0].position.y = pos.y + start.y - texelOfset;
        vertices[0].texCoord.x = pCurCharInfo->_left;
        vertices[0].texCoord.y = pCurCharInfo->_top;

        vertices[1].position.x = pos.x + start.x - texelOfset;
        vertices[1].position.y = pos.y + end.y - texelOfset;
        vertices[1].texCoord.x = pCurCharInfo->_left;
        vertices[1].texCoord.y = pCurCharInfo->_bottom;

        vertices[2].position.x = pos.x + end.x - texelOfset;
        vertices[2].position.y = pos.y + start.y - texelOfset;
        vertices[2].texCoord.x = pCurCharInfo->_right;
        vertices[2].texCoord.y = pCurCharInfo->_top;

        vertices[3].position.x = pos.x + end.x - texelOfset;
        vertices[3].position.y = pos.y + end.y - texelOfset;
        vertices[3].texCoord.x = pCurCharInfo->_right;
        vertices[3].texCoord.y = pCurCharInfo->_bottom;

        if (m_textHasGradient)
        {
            vertices[0].diffuse = ch.color;
            vertices[1].diffuse = ch.color2;
            vertices[2].diffuse = ch.color;
            vertices[3].diffuse = ch.color2;
        }
        else
        {
            vertices[0].diffuse = ch.color;
            vertices[1].diffuse = ch.color;
            vertices[2].diffuse = ch.color;
            vertices[3].diffuse = ch.color;
        }

        batchVertices.push_back(vertices[0]);
        batchVertices.push_back(vertices[1]);
        batchVertices.push_back(vertices[2]);
        batchVertices.push_back(vertices[1]); //[1]
        batchVertices.push_back(vertices[2]);
        batchVertices.push_back(vertices[3]);
    }

    {
        auto state = TextRenderState();

        for (auto &p : verticesMap)
        {
            if (!p.first)
                continue;

            STATEMANAGER.SetTexture(0, p.first->GetD3DTexture());

            for (auto f = p.second.begin(), l = p.second.end(); f != l;)
            {
                const auto batchCount = std::min<std::size_t>(
                    kLargePdtVertexBufferSize,
                    l - f);

                if (Engine::GetDevice().SetPDTStream(&*f, batchCount))
                {
                    Engine::GetDevice().DrawPrimitive(D3DPT_TRIANGLELIST, 0,
                                               batchCount / 3);
                }

                f += batchCount;
            }
        }

        if (clip)
        {
            STATEMANAGER.GetDevice()->SetScissorRect(&oldRect);
            STATEMANAGER.GetDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, clippingWasEnabled);
        }
    }

    D3DPERF_EndEvent();
}

std::pair<uint32_t, uint32_t> CGraphicTextInstance::GetColorGradient() const
{
    return std::make_pair(m_dwTextColor, m_textColor2);
}

void CGraphicTextInstance::SetColorGradient(uint32_t color, uint32_t color2)
{
    if (m_dwTextColor == color && m_textColor2 == color2)
        return;

    for (auto &ch : m_chars)
    {
        if (ch.color == m_dwTextColor)
            ch.color = color;
        if (ch.color2 == m_textColor2)
            ch.color2 = color2;
    }

    m_textHasGradient = color != color2;
    m_dwTextColor = color;
    m_textColor2 = color2;
}

void CGraphicTextInstance::SetColor(uint32_t color)
{
    SetColorGradient(color, color);
}

void CGraphicTextInstance::SetColor(float r, float g, float b, float a)
{
    SetColor(DirectX::SimpleMath::Color(r, g, b, a).BGRA().c);
}

void CGraphicTextInstance::SetOutLineColor(uint32_t color)
{
    m_dwOutLineColor = color;
}

void CGraphicTextInstance::SetOutLineColor(float r, float g, float b, float a)
{
    m_dwOutLineColor = DirectX::SimpleMath::Color(r, g, b, a).BGRA().c;
}

void CGraphicTextInstance::SetOutline(bool value)
{
    m_isOutline = value;
    if (value)
    {
    }
}

void CGraphicTextInstance::SetSecret(bool Value)
{
    m_isSecret = Value;
    m_dirty = true;
}

void CGraphicTextInstance::SetMultiLine(bool Value)
{
    m_isMultiLine = Value;
    m_dirty = true;
}

void CGraphicTextInstance::SetLimitWidth(float fWidth)
{
    m_fLimitWidth = fWidth;
    m_dirty = true;
}

void CGraphicTextInstance::HandleHyperLink(uint32_t color, TextTag tag,
                                           const std::string::difference_type
                                           sourcePos)
{
#ifndef WORLD_EDITOR
    std::vector<std::string> strs;
    storm::Tokenize(tag.content, ":", strs);
    if (!strs.empty())
    {
        if (strs[0] == "item")
        {
            int itemVnum = std::stoi(strs[1], 0, 16);
            const auto* pItemData = CItemManager::Instance().GetProto(itemVnum);
            if (pItemData)
            {
                auto id = std::stoi(strs[2], 0, 16);
                const auto& hyperLinkItem = CItemManager::Instance().GetHyperlinkItemData(id);

                std::string itemName = "[";

                if (hyperLinkItem)
                {
                    auto sockets = hyperLinkItem.value().sockets;

                    if (pItemData->GetType() == ITEM_TOGGLE && pItemData->GetSubType() == TOGGLE_PET)
                    {
                        if (sockets[0] != 0)
                        {
                            auto mobName = NpcManager::instance().GetName(sockets[0]);
                            if (mobName)
                            {
                                itemName.append(mobName.value());
                                itemName.append(" ");
                            }
                        }
                    }

                    if (pItemData->GetType() == ITEM_TOGGLE && pItemData->GetSubType() == TOGGLE_LEVEL_PET)
                    {
                        if (hyperLinkItem.value().transVnum != 0)
                        {
                            auto mobName = NpcManager::instance().GetName(hyperLinkItem.value().transVnum);
                            if (mobName)
                            {
                                itemName.append(mobName.value());
                                itemName.append(" ");
                            }
                        }
                    }

                    if (itemVnum >= 70103 && itemVnum <= 70106)
                    {
                        if (sockets[0] != 0)
                        {
                            auto mobName = NpcManager::instance().GetName(sockets[0]);
                            if (mobName)
                            {
                                itemName.append(mobName.value());
                                itemName.append(" ");
                            }
                        }
                    }
                }

                itemName.append(pItemData->GetName());
                itemName.append("]");
                auto end_it = utf8::find_invalid(
                    itemName.begin(), itemName.end());

                std::u32string utf32line;
                utf8::utf8to32(itemName.begin(), end_it,
                               back_inserter(utf32line));

                auto basePos = sourcePos;
                for (const auto &character : utf32line)
                {
                    AppendCharacter(character, color, color, basePos);

                    ++basePos;
                }
            }
        }
        else if (strs[0] == "itemname")
        {
            int itemVnum = std::stoi(strs[1]);
            auto pItemData = CItemManager::Instance().GetProto(itemVnum);
            if (pItemData)
            {
                std::string itemName = pItemData->GetName();
                auto end_it = utf8::find_invalid(
                    itemName.begin(), itemName.end());

                std::u32string utf32line;
                utf8::utf8to32(itemName.begin(), end_it,
                               back_inserter(utf32line));

                auto basePos = sourcePos;
                for (const auto &character : utf32line)
                {
                    AppendCharacter(character, color, color, basePos);

                    ++basePos;
                }
            }
        }
        else if (strs[0] == "mobname")
        {
            int mobVnum = std::stoi(strs[1]);
            auto mobData = NpcManager::Instance().GetName(mobVnum);
            if (mobData)
            {
                std::string mobName = mobData.value();
                auto end_it = utf8::
                    find_invalid(mobName.begin(), mobName.end());

                std::u32string utf32line;
                utf8::utf8to32(mobName.begin(), end_it,
                               back_inserter(utf32line));

                auto basePos = sourcePos;
                for (const auto &character : utf32line)
                {
                    AppendCharacter(character, color, color, basePos);

                    ++basePos;
                }
            }
        }
    }
#endif
}

void CGraphicTextInstance::HandleEmoticon(uint32_t color, TextTag tag,
                                          const std::string::difference_type
                                          sourcePos)
{
    CGraphicImageInstance *inst = nullptr;
    CGraphicImage::Ptr r = nullptr;

    if (tag.content.find(':') == std::string::npos)
    {
        r = CResourceManager::instance().LoadResource<CGraphicImage>(
            EmojiManager::instance().GetFilePath(tag.content)->c_str());
        if (r)
        {
            inst = new CGraphicImageInstance;
        }
    }
    else
    {
        std::vector<std::string> strs;
        boost::split(strs, tag.content, boost::is_any_of(":"));
        auto type = strs[0];
        if (type == "item")
        {
            auto itemVnum = std::stoi(strs[1]);
            auto pItemData = CItemManager::Instance().GetProto(itemVnum);
            if (pItemData)
            {
                inst = new CGraphicImageInstance;
                r = pItemData->GetIconImage();
                inst->SetScale(0.7, 0.7);
            }
        }
    }

    auto spaceWidth = m_font->GetTexturePointer().GetSpaceWidth();
    if (inst)
    {
        inst->SetImagePointer(r);
        AppendCharacter('x', color, color, sourcePos, inst);

        for (int i = 0; i < inst->GetWidth() / (spaceWidth); ++i)
            AppendCharacter(' ', color, color, sourcePos);
        if (inst->GetWidth() % ((uint32_t)spaceWidth) > 1)
            AppendCharacter(' ', color, color, sourcePos);
    }
}

void CGraphicTextInstance::SetValue(std::string value)
{
    for (const auto &ch : m_chars)
        delete ch.emoji;

    if (m_isMultiLine)
    {
        boost::replace_all(value, "\\n", "\n");
    }

    m_chars.clear();
    m_chars.reserve(value.length());

    uint32_t color = m_dwTextColor;
    uint32_t color2 = m_textColor2;
    TextTag tag;

    for (auto first = value.begin(), last = value.end(); first != last;)
    {
        const auto sourcePos = first - value.begin();

        if (GetTextTag(value.substr(sourcePos), tag))
        {
            if (tag.type == TEXT_TAG_COLOR)
            {
                color = strtoul(std::string(tag.content).c_str(),
                                nullptr, 16);
                color2 = strtoul(std::string(tag.content).c_str(),
                                 nullptr, 16);
            }

            else if (tag.type == TEXT_TAG_EMOTICON_START)
            {
                CGraphicImageInstance *inst = nullptr;
                CGraphicImage::Ptr r = nullptr;

                if (tag.content.find(':') == std::string::npos)
                {
                    r = CResourceManager::instance().LoadResource<CGraphicImage>(
                        EmojiManager::instance().GetFilePath(tag.content)->c_str());
                    if (r)
                    {
                        inst = new CGraphicImageInstance;
                    }
                }
                else
                {
                    std::vector<std::string> strs;
                    boost::split(strs, tag.content, boost::is_any_of(":"));
                    auto type = strs[0];
                    if (type == "item")
                    {
                        auto itemVnum = std::stoi(strs[1]);
                        auto pItemData = CItemManager::Instance().GetProto(itemVnum);
                        if (pItemData)
                        {
                            inst = new CGraphicImageInstance;
                            r = pItemData->GetIconImage();
                            inst->SetScale(0.7, 0.7);
                        }
                    }
                }

                CGraphicFontTexture &texture = m_font->GetTexturePointer();

                auto pSpaceInfo = texture.GetCharacterInfomation('x');
                if (inst)
                {
                    inst->SetImagePointer(r);
                    AppendCharacter('x', color, color2, sourcePos, inst);

                    for (int i = 0; i < inst->GetWidth() / (pSpaceInfo->_width - 1); ++i)
                        AppendCharacter(' ', color, color2, sourcePos);
                    if (inst->GetWidth() % (int32_t(pSpaceInfo->_width) - 1) > 1)
                        AppendCharacter(' ', color, color2, sourcePos);
                }
            }
            else if (tag.type == TEXT_TAG_HYPERLINK_START)
            {
                HandleHyperLink(color, tag, sourcePos);
            }

            else if (tag.type == TEXT_TAG_RESTORE_COLOR)
            {
                color = m_dwTextColor;
                color2 = m_textColor2;
            }
            else if (tag.type == TEXT_TAG_TAG)
                AppendCharacter('|', color, color2, sourcePos);

            first += tag.length;
            continue;
        }

        // Note: decode() advances first to one-past-last-read.
        AppendCharacter(boost::locale::utf::utf_traits<char>::decode(first, last),
                        color, color2, sourcePos);
    }

    m_sourceLength = value.length();
    m_dirty = true;
}

void CGraphicTextInstance::SetTextPointer(Font *pText)
{
    m_font = pText;
    m_dirty = true;
}

uint32_t CGraphicTextInstance::GetWidth() const
{
    return m_textWidth;
}

uint32_t CGraphicTextInstance::GetHeight() const
{
    return m_textHeight;
}

uint32_t CGraphicTextInstance::GetLineCount() const
{
    return m_lineCount;
}

uint32_t CGraphicTextInstance::GetGlyphLine(uint32_t index)
{
    if (index == m_chars.size())
        return m_lineCount;

    return m_chars[index].line;
}

void CGraphicTextInstance::GetLineGlyphs(uint32_t line,
                                         uint32_t &first, uint32_t &last)
{
    struct CharacterLineCmp
    {
        bool operator()(const Character &c, uint32_t line)
        {
            return c.line < line;
        }

        bool operator()(uint32_t line, const Character &c)
        {
            return line < c.line;
        }
    };

    const auto r = std::equal_range(m_chars.begin(),
                                    m_chars.end(),
                                    line,
                                    CharacterLineCmp());

    first = r.first - m_chars.begin();
    last = r.second - m_chars.begin();
}

void CGraphicTextInstance::GetGlyphPosition(uint32_t index,
                                            float &sx, float &sy,
                                            float &ex, float &ey) const
{
    assert(index <= m_chars.size() && "out-of-range");

    if (m_chars.empty())
    {
        sx = 0.0f;
        ex = 1.0f;
        sy = 0.0f;
        ey = m_maxLineHeight;
        return;
    }

    if (index != m_chars.size())
    {
        const auto &ch = m_chars[index];

        sx = ch.x;
        sy = ch.y;
        ex = ch.x + (ch.info ? ch.info->_advance : m_font->GetTexturePointer().GetSpaceWidth());
        ey = ch.y + m_maxLineHeight;
    }
    else
    {
        // Use one-before-last as starting point.
        const auto &ch = m_chars[index - 1];

        sx = ch.x + (ch.info ? ch.info->_advance : m_font->GetTexturePointer().GetSpaceWidth());
        sy = ch.y;
        ex = sx + 1.0f;
        ey = sy + m_maxLineHeight;
    }
}

bool CGraphicTextInstance::GetGlyphIndex(uint32_t &index,
                                         float x, float y) const
{
    uint32_t first, last;
    if (!FindLineForY(first, last, y))
        return false;

    const auto spaceAdvance = m_font->GetTexturePointer().GetSpaceWidth();

    for (uint32_t i = first; i != last; ++i)
    {
        const auto &ch = m_chars[i];

        if (ch.ch == 0x20 || ch.ch == 0x09)
        {
            if (x >= ch.x && x < ch.x + (spaceAdvance * (ch.ch == 0x20) ? 1 : 4))
            {
                index = i;
                return true;
            }
        }
        else
        {
            if (!ch.info)
                continue;

            if (x >= ch.x && x < ch.x + ch.info->_advance)
            {
                index = i;
                return true;
            }
        }
    }

    return false;
}

uint32_t CGraphicTextInstance::GetStringIndexFromGlyphIndex(
    uint32_t index) const
{
    // If we have no glyphs or |index| is out-of-range (which is valid!),
    // we simply return the source-string length.
    if (m_chars.empty() || index >= m_chars.size())
        return m_sourceLength;

    return m_chars[index].sourcePos;
}

uint32_t CGraphicTextInstance::GetGlyphIndexFromStringIndex(
    uint32_t index) const
{
    auto cmpSourcePos = [](const Character &a, uint32_t p) -> bool
    {
        return a.sourcePos < p;
    };

    const auto it = std::lower_bound(m_chars.begin(), m_chars.end(),
                                     index, cmpSourcePos);
    return it - m_chars.begin();
}

uint32_t CGraphicTextInstance::GetGlyphCount() const
{
    return m_chars.size();
}

void CGraphicTextInstance::AppendCharacter(uint32_t code, uint32_t color,
                                           uint32_t color2, uint32_t sourcePos,
                                           CGraphicImageInstance *emoji)
{
    if (m_font)
        m_font->GetTexturePointer().GetCharacterInfomation(code);

    Character ch = {};
    ch.sourcePos = sourcePos;
    ch.ch = code;
    ch.info = nullptr;
    ch.color = color;
    ch.color2 = color2;
    ch.emoji = emoji;
    m_chars.push_back(ch);
}

#pragma optimize( "", off )

bool CGraphicTextInstance::FindLineForY(uint32_t &first,
                                        uint32_t &last,
                                        float y) const
{
    first = 0;

    float curY = 0.0f;
    for (uint32_t i = 0, size = m_chars.size(); i != size; ++i)
    {
        const auto &ch = m_chars[i];
        if (!ch.info && ch.ch != ' ')
            continue;

        if (ch.y != curY)
        {
            last = i;
            curY = ch.y;

            // If our new Y is greater than the target line's Y, we just finished
            // the target line.
            // We return true here, so |first| still contains the
            // last line's start-index.
            if (y <= curY)
                return true;

            first = i;
        }
    }

    // Special handling for last incomplete line
    if (y <= curY + m_maxLineHeight)
    {
        last = m_chars.size();
        return true;
    }

    return false;
}
#pragma optimize( "", on )
