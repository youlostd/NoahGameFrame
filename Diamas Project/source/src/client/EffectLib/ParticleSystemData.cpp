#include "StdAfx.h"
#include "ParticleSystemData.h"
#include "Util.hpp"

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
CDynamicPool<CParticleSystemData> CParticleSystemData::ms_kPool;

CParticleSystemData::CParticleSystemData()
{
}

CParticleSystemData::~CParticleSystemData()
{
}

CEmitterProperty *CParticleSystemData::GetEmitterPropertyPointer()
{
    return &m_EmitterProperty;
}

CParticleProperty *CParticleSystemData::GetParticlePropertyPointer()
{
    return &m_ParticleProperty;
}

void CParticleSystemData::ChangeTexture(const char *c_szFileName)
{
    m_ParticleProperty.SetTexture(c_szFileName);
}

void CParticleSystemData::UpdateCache()
{
    std::set<double> times;

    for (auto &i : m_ParticleProperty.m_TimeEventColorRed)
        times.insert(i.m_fTime);

    for (auto &i : m_ParticleProperty.m_TimeEventColorGreen)
        times.insert(i.m_fTime);

    for (auto &i : m_ParticleProperty.m_TimeEventColorBlue)
        times.insert(i.m_fTime);

    for (auto &i : m_ParticleProperty.m_TimeEventAlpha)
        times.insert(i.m_fTime);

    m_ParticleProperty.m_TimeEventColor.clear();
    for (double time : times)
    {
        float r = GetTimeEventBlendValue(time,
                                         m_ParticleProperty.m_TimeEventColorRed);
        float g = GetTimeEventBlendValue(time,
                                         m_ParticleProperty.m_TimeEventColorGreen);
        float b = GetTimeEventBlendValue(time,
                                         m_ParticleProperty.m_TimeEventColorBlue);
        float a = GetTimeEventBlendValue(time,
                                         m_ParticleProperty.m_TimeEventAlpha);

        DirectX::SimpleMath::Color c;
        c.R(r);
        c.G(g);
        c.B(b);
        c.A(a);

        TTimeEventTypeColor t;
        t.m_fTime = time;
        t.m_Value = c.BGRA().c;
        m_ParticleProperty.m_TimeEventColor.push_back(t);
    }
}

CParticleSystemData *CParticleSystemData::New()
{
    return ms_kPool.Alloc();
}

void CParticleSystemData::Delete(CParticleSystemData *pkData)
{
    pkData->Clear();
    ms_kPool.Free(pkData);
}

void CParticleSystemData::DestroySystem()
{
    ms_kPool.Destroy();
}

bool CParticleSystemData::OnLoadScript(CTextFileLoader &loader)
{
    if (!LoadEmitterProperty(loader))
        return false;

    if (!LoadParticleProperty(loader))
        return false;

    UpdateCache();
    return true;
}

void CParticleSystemData::OnClear()
{
    m_EmitterProperty.Clear();
    m_ParticleProperty.Clear();
}

bool CParticleSystemData::OnIsData()
{
    return true;
}

bool CParticleSystemData::LoadEmitterProperty(CTextFileLoader &loader)
{
    CTextFileLoader::CGotoChild GotoChild(&loader, "emitterproperty");

    if (!loader.GetTokenDoubleWord("maxemissioncount", &m_EmitterProperty.m_dwMaxEmissionCount))
        return false;

    if (!loader.GetTokenFloat("cyclelength", &m_EmitterProperty.m_fCycleLength))
        m_EmitterProperty.m_fCycleLength = 0.05f;

    if (!loader.GetTokenBoolean("cycleloopenable", &m_EmitterProperty.m_bCycleLoopFlag))
        m_EmitterProperty.m_bCycleLoopFlag = false;

    if (!loader.GetTokenInteger("loopcount", &m_EmitterProperty.m_iLoopCount))
        m_EmitterProperty.m_iLoopCount = 0;

    if (!loader.GetTokenByte("emittershape", &m_EmitterProperty.m_byEmitterShape))
        return false;

    if (!loader.GetTokenByte("emitteradvancedtype", &m_EmitterProperty.m_byEmitterAdvancedType))
        m_EmitterProperty.m_byEmitterShape = CEmitterProperty::EMITTER_ADVANCED_TYPE_FREE;

    if (!loader.GetTokenPosition("emittingsize", &m_EmitterProperty.m_v3EmittingSize))
        m_EmitterProperty.m_v3EmittingSize = Vector3(0.0f, 0.0f, 0.0f);

    if (!loader.GetTokenFloat("emittingradius", &m_EmitterProperty.m_fEmittingRadius))
        m_EmitterProperty.m_fEmittingRadius = 0.0f;

    if (!loader.GetTokenBoolean("emitteremitfromedgeflag", &m_EmitterProperty.m_bEmitFromEdgeFlag))
        m_EmitterProperty.m_bEmitFromEdgeFlag = false;

    if (!loader.GetTokenPosition("emittingdirection", &m_EmitterProperty.m_v3EmittingDirection))
        m_EmitterProperty.m_v3EmittingDirection = Vector3(0.0f, 0.0f, 0.0f);

    if (!GetTokenTimeEventFloat(loader, "timeeventemittingsize", &m_EmitterProperty.m_TimeEventEmittingSize))
    {
        m_EmitterProperty.m_TimeEventEmittingSize.clear();
        TTimeEventTypeFloat TimeEventFloat;
        TimeEventFloat.m_fTime = 0.0f;
        TimeEventFloat.m_Value = 0.0f;
        m_EmitterProperty.m_TimeEventEmittingSize.push_back(TimeEventFloat);
    }

    if (!GetTokenTimeEventFloat(loader, "timeeventemittingangularvelocity",
                                &m_EmitterProperty.m_TimeEventEmittingAngularVelocity))
    {
        m_EmitterProperty.m_TimeEventEmittingAngularVelocity.clear();
        TTimeEventTypeFloat TimeEventFloat;
        TimeEventFloat.m_fTime = 0.0f;
        TimeEventFloat.m_Value = 0.0f;
        m_EmitterProperty.m_TimeEventEmittingAngularVelocity.push_back(TimeEventFloat);
    }

    if (!GetTokenTimeEventFloat(loader, "timeeventemittingdirectionx",
                                &m_EmitterProperty.m_TimeEventEmittingDirectionX))
    {
        m_EmitterProperty.m_TimeEventEmittingDirectionX.clear();
        TTimeEventTypeFloat TimeEventFloat;
        TimeEventFloat.m_fTime = 0.0f;
        TimeEventFloat.m_Value = 0.0f;
        m_EmitterProperty.m_TimeEventEmittingDirectionX.push_back(TimeEventFloat);
    }

    if (!GetTokenTimeEventFloat(loader, "timeeventemittingdirectiony",
                                &m_EmitterProperty.m_TimeEventEmittingDirectionY))
    {
        m_EmitterProperty.m_TimeEventEmittingDirectionY.clear();
        TTimeEventTypeFloat TimeEventFloat;
        TimeEventFloat.m_fTime = 0.0f;
        TimeEventFloat.m_Value = 0.0f;
        m_EmitterProperty.m_TimeEventEmittingDirectionY.push_back(TimeEventFloat);
    }

    if (!GetTokenTimeEventFloat(loader, "timeeventemittingdirectionz",
                                &m_EmitterProperty.m_TimeEventEmittingDirectionZ))
    {
        m_EmitterProperty.m_TimeEventEmittingDirectionZ.clear();
        TTimeEventTypeFloat TimeEventFloat;
        TimeEventFloat.m_fTime = 0.0f;
        TimeEventFloat.m_Value = 0.0f;
        m_EmitterProperty.m_TimeEventEmittingDirectionZ.push_back(TimeEventFloat);
    }

    if (!GetTokenTimeEventFloat(loader, "timeeventemittingvelocity", &m_EmitterProperty.m_TimeEventEmittingVelocity))
    {
        m_EmitterProperty.m_TimeEventEmittingVelocity.clear();
        TTimeEventTypeFloat TimeEventFloat;
        TimeEventFloat.m_fTime = 0.0f;
        TimeEventFloat.m_Value = 0.0f;
        m_EmitterProperty.m_TimeEventEmittingVelocity.push_back(TimeEventFloat);
    }

    if (!GetTokenTimeEventFloat(loader, "timeeventemissioncountpersecond",
                                &m_EmitterProperty.m_TimeEventEmissionCountPerSecond))
        return false;

    if (!GetTokenTimeEventFloat(loader, "timeeventlifetime", &m_EmitterProperty.m_TimeEventLifeTime))
        return false;

    if (!GetTokenTimeEventFloat(loader, "timeeventsizex", &m_EmitterProperty.m_TimeEventSizeX))
        return false;

    if (!GetTokenTimeEventFloat(loader, "timeeventsizey", &m_EmitterProperty.m_TimeEventSizeY))
        return false;

    return true;
}

bool CParticleSystemData::LoadParticleProperty(CTextFileLoader &loader)
{
    CTextFileLoader::CGotoChild GotoChild(&loader, "particleproperty");

    if (!loader.GetTokenByte("srcblendtype", &m_ParticleProperty.m_bySrcBlendType))
        m_ParticleProperty.m_bySrcBlendType = D3DBLEND_SRCALPHA;

    if (!loader.GetTokenByte("destblendtype", &m_ParticleProperty.m_byDestBlendType))
        m_ParticleProperty.m_byDestBlendType = D3DBLEND_ONE;

    if (!loader.GetTokenByte("coloroperationtype", &m_ParticleProperty.m_byColorOperationType))
        m_ParticleProperty.m_byColorOperationType = D3DTOP_MODULATE;

    if (!loader.GetTokenByte("billboardtype", &m_ParticleProperty.m_byBillboardType))
        return false;

    if (!loader.GetTokenByte("rotationtype", &m_ParticleProperty.m_byRotationType))
        return false;

    if (!loader.GetTokenFloat("rotationspeed", &m_ParticleProperty.m_fRotationSpeed))
        return false;

    if (!loader.GetTokenWord("rotationrandomstartingbegin", &m_ParticleProperty.m_wRotationRandomStartingBegin))
        return false;

    if (!loader.GetTokenWord("rotationrandomstartingend", &m_ParticleProperty.m_wRotationRandomStartingEnd))
        return false;

    if (!loader.GetTokenBoolean("attachenable", &m_ParticleProperty.m_bAttachFlag))
        m_ParticleProperty.m_bAttachFlag = false;

    if (!loader.GetTokenBoolean("stretchenable", &m_ParticleProperty.m_bStretchFlag))
        return false;

    if (!loader.GetTokenByte("texanitype", &m_ParticleProperty.m_byTexAniType))
        return false;
    if (!loader.GetTokenDouble("texanidelay", &m_ParticleProperty.m_fTexAniDelay))
        return false;
    if (!loader.GetTokenBoolean("texanirandomstartframeenable", &m_ParticleProperty.m_bTexAniRandomStartFrameFlag))
        return false;

    float fGravity;

    if (loader.GetTokenFloat("gravity", &fGravity))
    {
        TTimeEventTypeDouble f;
        f.m_fTime = 0.0f;
        f.m_Value = fGravity;
        m_ParticleProperty.m_TimeEventGravity.push_back(f);
    }
    else if (!GetTokenTimeEventDouble(loader, "timeeventgravity", &m_ParticleProperty.m_TimeEventGravity))
    {
        m_ParticleProperty.m_TimeEventGravity.clear();
    }

    float fAirResistance;
    if (loader.GetTokenFloat("airresistance", &fAirResistance))
    {
        TTimeEventTypeDouble f;
        f.m_fTime = 0.0f;
        f.m_Value = fAirResistance;
        m_ParticleProperty.m_TimeEventAirResistance.push_back(f);
    }
    else if (!GetTokenTimeEventDouble(loader, "timeeventairresistance", &m_ParticleProperty.m_TimeEventAirResistance))
    {
        m_ParticleProperty.m_TimeEventAirResistance.clear();
    }

    if (!GetTokenTimeEventDouble(loader, "timeeventscalex", &m_ParticleProperty.m_TimeEventScaleX))
        return false;

    if (!GetTokenTimeEventDouble(loader, "timeeventscaley", &m_ParticleProperty.m_TimeEventScaleY))
        return false;

    if (!GetTokenTimeEventDouble(loader, "timeeventcolorred", &m_ParticleProperty.m_TimeEventColorRed))
        return false;

    if (!GetTokenTimeEventDouble(loader, "timeeventcolorgreen", &m_ParticleProperty.m_TimeEventColorGreen))
        return false;

    if (!GetTokenTimeEventDouble(loader, "timeeventcolorblue", &m_ParticleProperty.m_TimeEventColorBlue))
        return false;

    if (!GetTokenTimeEventDouble(loader, "timeeventalpha", &m_ParticleProperty.m_TimeEventAlpha))
        return false;

    if (!GetTokenTimeEventDouble(loader, "timeeventrotation", &m_ParticleProperty.m_TimeEventRotation))
        return false;

    CTokenVector *pTextureVector;
    if (!loader.GetTokenVector("texturefiles", &pTextureVector))
        return false;

    for (std::string &filename : *pTextureVector)
    {
        if (!IsGlobalFileName(filename.c_str()))
            filename = GetOnlyPathName(loader.GetFileName()) + filename;

        if (!m_ParticleProperty.InsertTexture(filename.c_str()))
        {
            SPDLOG_ERROR("Failed to load {0} referenced in {1}",
                          filename, loader.GetFileName());
        }
    }

    return true;
}
