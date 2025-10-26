#include "GrpText.h"
#include "../EterBase/MappedFile.h"
#include "../eterBase/Utils.h"
#include "Engine.hpp"
#include "FontManager.hpp"
#include "StdAfx.h"
#include <pak/Vfs.hpp>
#include <storm/StringUtil.hpp>

CGraphicFontTexture& Font::GetTexturePointer()
{
    return m_fontTexture;
}

bool Font::Create(FT_Face face, uint8_t size) {
    m_face = face;
    m_familyName = m_face->family_name;
    m_styleName = m_face->style_name;
    m_fontSize = size;
    CreateFontInfo();
    return m_fontTexture.Create(m_face, size);
}

void Font::CreateFontInfo()
{
    assert(m_face != NULL && "TrueTypeFont not initialized");
    assert(FT_IS_SCALABLE(m_face) && "Font is unscalable");

    FT_Size_Metrics metrics = m_face->size->metrics;

    m_info.scale = 1.0f;
    m_info.ascender = metrics.ascender / 64.0f;
    m_info.descender = metrics.descender / 64.0f;
    m_info.lineGap = (metrics.height - metrics.ascender + metrics.descender
        ) / 64.0f;
    m_info.maxAdvanceWidth = metrics.max_advance / 64.0f;

    m_info.underlinePosition = FT_MulFix(m_face->underline_position,
        metrics.y_scale) / 64.0f;
    m_info.underlineThickness = FT_MulFix(m_face->underline_thickness,
        metrics.y_scale) / 64.0f;
}

FontInfo Font::getFontInfo() const
{
    return m_info;
}
