#ifndef METIN2_CLIENT_ETERLIB_SCREENFILTER_HPP
#define METIN2_CLIENT_ETERLIB_SCREENFILTER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpScreen.h"

class CScreenFilter : public CScreen
{
public:
    CScreenFilter();

    void SetEnable(bool value);
    void SetBlendType(uint8_t srcType, uint8_t dstType);
    void SetColor(const DirectX::SimpleMath::Color &color);

    void Render();

protected:
    bool m_enabled;
    uint8_t m_srcType;
    uint8_t m_dstType;
    DirectX::SimpleMath::Color m_color;
};

#endif
