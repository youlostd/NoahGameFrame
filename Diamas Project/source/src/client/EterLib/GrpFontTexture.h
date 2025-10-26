#ifndef METIN2_CLIENT_ETERLIB_GRPFONTTEXTURE_HPP
#define METIN2_CLIENT_ETERLIB_GRPFONTTEXTURE_HPP

#pragma once

#include "GrpImageTexture.h"
#include "GrpBase.h"

#include <vector>
#include <unordered_map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

constexpr float FONT_TEXTURE_SIZE = 1024.0f;

enum FontDimention
{
    //
    FontDimention_2D,

    //
    FontDimention_3D
};

//
enum FontProperty
{
    //
    FONT_PROP_NONE,

    //
    FONT_PROP_PROJECT_1,

    //
    FONT_PROP_PROJECT_2,

    //
    FONT_PROP_OUTLINE,

    //
    FONT_PROP_BG_1,

    //
    FONT_PROP_OFFSET_1,
};

class CGraphicFontTexture : public CGraphicBase
{
public:
    struct GlyphData
    {
        CGraphicImageTexture* _tex;
        float _left;
        float _top;
        float _right;
        float _bottom;
        float _advance;
        float _bearingX;
        float _bearingY;
        float _width;
        float _height;
    };

    CGraphicFontTexture();
    virtual ~CGraphicFontTexture();

    bool Create(FT_Face face, int fontSize);

    // Texture Atlas handling
    CGraphicImageTexture* AppendTexture();
    bool UpdateTexture();

    GlyphData* GetCharacterInfomation(uint32_t code);
    void AddCodePoint(uint32_t code);

    float GetSpaceWidth()
    {
        return GetCharacterInfomation(L' ')->_advance;
    }

private:
    typedef std::vector<CGraphicImageTexture*> TGraphicImageTexturePointerVector;


private:
    static const unsigned int _TEXTURE_SIZE = 512;
    unsigned int m_verticalOffset;
    static const float _INVERSE_TEXTURE_SIZE;
private:
    FT_Library m_lib = nullptr;
    FT_Face m_face = nullptr;
    uint32_t m_fontSize = 0;

    //
    typedef std::map<unsigned long, GlyphData*> CodeTexMap;
    CodeTexMap m_codePoints;

    //
    CGraphicImageTexture* m_activeTex = nullptr;
    Vector2 m_penPos = {};

    TGraphicImageTexturePointerVector m_textures;
};

#endif
