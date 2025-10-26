#ifndef METIN2_CLIENT_EFFECTLIB_PARTICLEPROPERTY_H
#define METIN2_CLIENT_EFFECTLIB_PARTICLEPROPERTY_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vector>
#include "../eterlib/GrpImageInstance.h"

#include "Type.h"

struct CParticleProperty
{
    enum
    {
        ROTATION_TYPE_NONE,
        ROTATION_TYPE_TIME_EVENT,
        ROTATION_TYPE_CW,
        ROTATION_TYPE_CCW,
        ROTATION_TYPE_RANDOM_DIRECTION,
    };

    enum
    {
        TEXTURE_ANIMATION_TYPE_NONE,
        TEXTURE_ANIMATION_TYPE_CW,
        TEXTURE_ANIMATION_TYPE_CCW,
        TEXTURE_ANIMATION_TYPE_RANDOM_FRAME,
        TEXTURE_ANIMATION_TYPE_RANDOM_DIRECTION,
    };

    CParticleProperty();

    void Clear();

    bool InsertTexture(const char *c_szFileName);
    bool SetTexture(const char *c_szFileName);

    uint8_t GetTextureAnimationType() const
    {
        return m_byTexAniType;
    }

    size_t GetTextureAnimationFrameCount() const
    {
        return m_ImageVector.size();
    }

    double GetTextureAnimationFrameDelay() const
    {
        return m_fTexAniDelay;
    }

    double m_fTexAniDelay;
    bool m_bTexAniRandomStartFrameFlag;
    bool m_bAttachFlag;
    bool m_bStretchFlag;
    uint8_t m_byTexAniType;
    uint8_t m_bySrcBlendType;
    uint8_t m_byDestBlendType;
    uint8_t m_byColorOperationType;
    uint8_t m_byBillboardType;

    uint8_t m_byRotationType;
    float m_fRotationSpeed;
    uint16_t m_wRotationRandomStartingBegin;
    uint16_t m_wRotationRandomStartingEnd;

    TTimeEventTableDouble m_TimeEventGravity;
    TTimeEventTableDouble m_TimeEventAirResistance;

    TTimeEventTableDouble m_TimeEventScaleX;
    TTimeEventTableDouble m_TimeEventScaleY;

    TTimeEventTableDouble m_TimeEventColorRed;
    TTimeEventTableDouble m_TimeEventColorGreen;
    TTimeEventTableDouble m_TimeEventColorBlue;
    TTimeEventTableDouble m_TimeEventAlpha;

    // Combination of the 4 values above
    TTimeEventTableColor m_TimeEventColor;
    TTimeEventTableDouble m_TimeEventRotation;

    std::vector<std::string> m_TextureNameVector;
    std::vector<CGraphicImage::Ptr> m_ImageVector;

    // pre-transformed variables
    Vector3 m_v3ZAxis;
};
#endif /* METIN2_CLIENT_EFFECTLIB_PARTICLEPROPERTY_H */
