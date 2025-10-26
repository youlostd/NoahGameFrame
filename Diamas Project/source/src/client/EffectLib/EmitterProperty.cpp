#include "StdAfx.h"
#include "EmitterProperty.h"
#include "Util.hpp"
// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
uint8_t CEmitterProperty::GetEmitterShape()
{
    return m_byEmitterShape;
}

uint8_t CEmitterProperty::GetEmitterAdvancedType()
{
    return m_byEmitterAdvancedType;
}

bool CEmitterProperty::isEmitFromEdge()
{
    return m_bEmitFromEdgeFlag;
}

void CEmitterProperty::GetEmittingSize(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingSize);
}

void CEmitterProperty::GetEmittingAngularVelocity(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingAngularVelocity);
}

void CEmitterProperty::GetEmittingDirectionX(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingDirectionX);
}

void CEmitterProperty::GetEmittingDirectionY(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingDirectionY);
}

void CEmitterProperty::GetEmittingDirectionZ(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingDirectionZ);
}

void CEmitterProperty::GetEmittingVelocity(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmittingVelocity);
}

void CEmitterProperty::GetEmissionCountPerSecond(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventEmissionCountPerSecond);
}

void CEmitterProperty::GetParticleLifeTime(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventLifeTime);
}

void CEmitterProperty::GetParticleSizeX(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventSizeX);
}

void CEmitterProperty::GetParticleSizeY(double fTime, float *pfValue)
{
    *pfValue = GetTimeEventBlendValue(fTime, m_TimeEventSizeY);
}

void CEmitterProperty::Clear()
{
    m_dwMaxEmissionCount = 0;

    m_fCycleLength = 0.0f;
    m_bCycleLoopFlag = false;
    m_iLoopCount = 0;

    m_byEmitterShape = EMITTER_SHAPE_POINT;
    m_byEmitterAdvancedType = EMITTER_ADVANCED_TYPE_FREE;
    m_bEmitFromEdgeFlag = false;
    m_v3EmittingSize = Vector3(0.0f, 0.0f, 0.0f);
    m_fEmittingRadius = 0.0f;

    m_v3EmittingDirection = Vector3(0.0f, 0.0f, 0.0f);

    m_TimeEventEmittingSize.clear();
    m_TimeEventEmittingDirectionX.clear();
    m_TimeEventEmittingDirectionY.clear();
    m_TimeEventEmittingDirectionZ.clear();
    m_TimeEventEmittingVelocity.clear();
    m_TimeEventEmissionCountPerSecond.clear();
    m_TimeEventLifeTime.clear();
    m_TimeEventSizeX.clear();
    m_TimeEventSizeY.clear();
    m_TimeEventEmittingAngularVelocity.clear();
}

CEmitterProperty::CEmitterProperty()
{
    Clear();
}
