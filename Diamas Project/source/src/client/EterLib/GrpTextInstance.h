#ifndef __INC_ETERLIB_GRPTEXTINSTANCE_H__
#define __INC_ETERLIB_GRPTEXTINSTANCE_H__

#include "Pool.h"
#include "GrpText.h"
#include "GrpImageInstance.h"
#include "GrpScreen.h"
#include "TextTag.h"

class CGraphicTextInstance : CScreen
{
public:
    CGraphicTextInstance();
    ~CGraphicTextInstance();

    void Update();
    void Render(int32_t x, int32_t y, int32_t z = 0,
                const RECT *clip = NULL);
    std::pair<uint32_t, uint32_t> GetColorGradient() const;

    void SetColorGradient(uint32_t color, uint32_t color2);

    void SetColor(uint32_t color);
    void SetColor(float r, float g, float b, float a = 1.0f);

    void SetOutLineColor(uint32_t color);
    void SetOutLineColor(float r, float g, float b, float a = 1.0f);
    void SetOutline(bool value);

    void SetTextPointer(Font *pText);
    void SetValue(std::string value);
    void SetSecret(bool Value);

    bool IsSecret() const
    {
        return m_isSecret;
    }

    void SetMultiLine(bool Value);
    void SetLimitWidth(float fWidth);
    void HandleHyperLink(uint32_t color, TextTag tag, std::string::difference_type sourcePos);
    void HandleEmoticon(uint32_t color, TextTag tag, std::string::difference_type sourcePos);

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetLineCount() const;

    uint32_t GetGlyphLine(uint32_t index);

    // Line range: [first, last)
    void GetLineGlyphs(uint32_t line,
                       uint32_t &first, uint32_t &last);

    // index: [0, count]
    void GetGlyphPosition(uint32_t index,
                          float &sx, float &sy,
                          float &ex, float &ey) const;

    bool GetGlyphIndex(uint32_t &index, float x, float y) const;

    // Get the source string pos. corresponding to |index|
    // Returns the source-string length if offset is out-of-range.
    uint32_t GetStringIndexFromGlyphIndex(uint32_t index) const;

    // Get the position of the first character whose source-position
    // is equal or greater than |index|.
    //
    // This behaviour is necessary, because not all source-string
    // characters are rendered.
    uint32_t GetGlyphIndexFromStringIndex(uint32_t index) const;

    uint32_t GetGlyphCount() const;

protected:
    struct Character
    {
        // precalculated position
        float x;
        float y;

        // UTF32 code-point
        uint32_t ch;

        // Glyph info
        const CGraphicFontTexture::GlyphData *info;

        // Single-glyph color
        uint32_t color;
        uint32_t color2;
        bool isGradient;

        // Position in source string
        uint32_t sourcePos;

        // Index of the enclosing line, starting at 0.
        uint32_t line;

        // Emoji Image
        CGraphicImageInstance *emoji;
    };

    void AppendCharacter(uint32_t code, uint32_t color, uint32_t color2,
                         uint32_t sourcePos, CGraphicImageInstance *emoji = nullptr);

    // Line range: [first, last)
    bool FindLineForY(uint32_t &first,
                      uint32_t &last,
                      float y) const;

    bool m_textHasGradient;
    uint32_t m_dwTextColor;
    uint32_t m_textColor2;
    uint32_t m_dwOutLineColor;

    uint16_t m_textWidth;
    uint16_t m_textHeight;

    float m_maxLineHeight;
    float m_fLimitWidth;

    bool m_isSecret;
    bool m_isMultiLine;
    bool m_isOutline;

    bool m_dirty;

    Font *m_font;

    std::vector<Character> m_chars;
    uint32_t m_sourceLength;
    uint32_t m_lineCount;
};

#endif
