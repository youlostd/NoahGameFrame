#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Resource.h"
#include "GrpFontTexture.h"

#include <pak/VfsFile.hpp>
#include <storm/io/View.hpp>

struct FontInfo
{
    /// The font height in pixel.
    uint16_t pixelSize;
    /// Rendering type used for the font.
    int16_t fontType;

    /// The pixel extents above the baseline in pixels (typically positive).
    float ascender;
    /// The extents below the baseline in pixels (typically negative).
    float descender;
    /// The spacing in pixels between one row's descent and the next row's ascent.
    float lineGap;
    /// This field gives the maximum horizontal cursor advance for all glyphs in the font.
    float maxAdvanceWidth;
    /// The thickness of the under/hover/strike-trough line in pixels.
    float underlineThickness;
    /// The position of the underline relatively to the baseline.
    float underlinePosition;

    /// Scale to apply to glyph data.
    float scale;
};

class Font
{
  public:
    const std::string &GetName() const { return m_fileName; };
    const std::string &GetFamilyName() const { return m_familyName; };

    bool     Create(FT_Face face, uint8_t size);
    void     CreateFontInfo();
    FontInfo getFontInfo() const;

    CGraphicFontTexture &GetTexturePointer();

  private:
    std::string m_fileName;
    std::string m_familyName;
    std::string m_styleName;
    FontInfo m_info = {};
    mutable std::string m_formattedName;
    int32_t m_fontSize = 0;

    CGraphicFontTexture m_fontTexture;
    FT_Face m_face = nullptr;
};
