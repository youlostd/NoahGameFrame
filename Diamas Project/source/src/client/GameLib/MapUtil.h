#ifndef METIN2_CLIENT_GAMELIB_MAPUTIL_HPP
#define METIN2_CLIENT_GAMELIB_MAPUTIL_HPP
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MapType.h"

void PixelPositionToAttributeCellPosition(TPixelPosition PixelPosition, TCellPosition *pAttrCellPosition);
void AttributeCellPositionToPixelPosition(TCellPosition AttrCellPosition, TPixelPosition *pPixelPosition);

float GetPixelPositionDistance(const TPixelPosition &c_rsrcPosition, const TPixelPosition &c_rdstPosition);

class CEaseOutInterpolation
{
  public:
    CEaseOutInterpolation();
    virtual ~CEaseOutInterpolation();

    void Initialize();

    bool Setup(float fStart, float fEnd, float fTime);
    void Interpolate(float fElapsedTime);
    bool isPlaying();

    float GetValue();
    float GetChangingValue();

  protected:
    float m_fRemainingTime;
    float m_fValue;
    float m_fSpeed;
    float m_fAcceleration;

    float m_fStartValue;
    float m_fLastValue;
};

#endif
