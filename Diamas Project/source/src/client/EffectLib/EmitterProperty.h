#ifndef METIN2_CLIENT_EFFECTLIB_EMITTERPROPERTY_H
#define METIN2_CLIENT_EFFECTLIB_EMITTERPROPERTY_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Type.h"

class CEmitterProperty
{
    friend class CParticleSystemData;
    friend class CParticleSystemInstance;

public:
    enum
    {
        EMITTER_SHAPE_POINT,
        EMITTER_SHAPE_ELLIPSE,
        EMITTER_SHAPE_SQUARE,
        EMITTER_SHAPE_SPHERE,
    };

    enum
    {
        EMITTER_ADVANCED_TYPE_FREE,
        EMITTER_ADVANCED_TYPE_OUTER,
        EMITTER_ADVANCED_TYPE_INNER,
    };

    CEmitterProperty();

    void Clear();

    uint32_t GetMaxEmissionCount()
    {
        return m_dwMaxEmissionCount;
    }

    float GetCycleLength()
    {
        return m_fCycleLength;
    }

    bool isCycleLoop()
    {
        return m_bCycleLoopFlag;
    }

    int GetLoopCount()
    {
        return m_iLoopCount;
    }

    uint8_t GetEmitterShape();
    uint8_t GetEmitterAdvancedType();
    bool isEmitFromEdge();

    void GetEmittingSize(double fTime, float *pfValue);
    void GetEmittingAngularVelocity(double fTime, float *pfValue);

    void GetEmittingDirectionX(double fTime, float *pfValue);
    void GetEmittingDirectionY(double fTime, float *pfValue);
    void GetEmittingDirectionZ(double fTime, float *pfValue);
    void GetEmittingVelocity(double fTime, float *pfValue);
    void GetEmissionCountPerSecond(double fTime, float *pfValue);
    void GetParticleLifeTime(double fTime, float *pfValue);
    void GetParticleSizeX(double fTime, float *pfValue);
    void GetParticleSizeY(double fTime, float *pfValue);

    uint32_t m_dwMaxEmissionCount;

    float m_fCycleLength;
    bool m_bCycleLoopFlag;
    int m_iLoopCount;

    uint8_t m_byEmitterShape;
    uint8_t m_byEmitterAdvancedType;
    bool m_bEmitFromEdgeFlag;
    Vector3 m_v3EmittingSize;
    float m_fEmittingRadius;

    Vector3 m_v3EmittingDirection;

    TTimeEventTableFloat m_TimeEventEmittingSize;
    TTimeEventTableFloat m_TimeEventEmittingAngularVelocity;
    TTimeEventTableFloat m_TimeEventEmittingDirectionX;
    TTimeEventTableFloat m_TimeEventEmittingDirectionY;
    TTimeEventTableFloat m_TimeEventEmittingDirectionZ;
    TTimeEventTableFloat m_TimeEventEmittingVelocity;
    TTimeEventTableFloat m_TimeEventEmissionCountPerSecond;
    TTimeEventTableFloat m_TimeEventLifeTime;
    TTimeEventTableFloat m_TimeEventSizeX;
    TTimeEventTableFloat m_TimeEventSizeY;
};
#endif /* METIN2_CLIENT_EFFECTLIB_EMITTERPROPERTY_H */
