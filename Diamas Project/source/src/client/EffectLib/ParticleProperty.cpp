#include "StdAfx.h"
#include "ParticleProperty.h"

CParticleProperty::CParticleProperty()
    : m_fTexAniDelay(0.0f)
      , m_bTexAniRandomStartFrameFlag(false)
      , m_bAttachFlag(false)
      , m_bStretchFlag(false)
      , m_byTexAniType(TEXTURE_ANIMATION_TYPE_NONE)
      , m_bySrcBlendType(D3DBLEND_SRCALPHA)
      , m_byDestBlendType(D3DBLEND_ONE)
      , m_byColorOperationType(D3DTOP_MODULATE)
      , m_byBillboardType(BILLBOARD_TYPE_NONE)
      , m_byRotationType(0)
      , m_fRotationSpeed(0.0f)
      , m_wRotationRandomStartingBegin(0)
      , m_wRotationRandomStartingEnd(0)
      , m_v3ZAxis(0.0f, 0.0f, 0.0f)
{
    // ctor
}

bool CParticleProperty::InsertTexture(const char *c_szFileName)
{
    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    if (!r)
        return false;

    m_ImageVector.push_back(r);
    m_TextureNameVector.emplace_back(c_szFileName);
    return true;
}

bool CParticleProperty::SetTexture(const char *c_szFileName)
{
    if (m_ImageVector.size() > 1)
    {
        assert(false);
        return false;
    }

    m_TextureNameVector.clear();
    m_ImageVector.clear();

    InsertTexture(c_szFileName);
    return true;
}

void CParticleProperty::Clear()
{
    m_byRotationType = 0;
    m_fRotationSpeed = 0.0f;
    m_wRotationRandomStartingBegin = 0;
    m_wRotationRandomStartingEnd = 0;

    m_bAttachFlag = false;
    m_bStretchFlag = false;

    m_bySrcBlendType = D3DBLEND_SRCALPHA;
    m_byDestBlendType = D3DBLEND_ONE;
    m_byColorOperationType = D3DTOP_MODULATE;

    m_byBillboardType = BILLBOARD_TYPE_NONE;

    m_byTexAniType = TEXTURE_ANIMATION_TYPE_NONE;
    m_fTexAniDelay = 0.05f;
    m_bTexAniRandomStartFrameFlag = false;

    m_TimeEventGravity.clear();
    m_TimeEventAirResistance.clear();

    m_TimeEventScaleX.clear();
    m_TimeEventScaleY.clear();

    m_TimeEventColorRed.clear();
    m_TimeEventColorGreen.clear();
    m_TimeEventColorBlue.clear();
    m_TimeEventAlpha.clear();
    m_TimeEventColor.clear();
    m_TimeEventRotation.clear();

    m_TextureNameVector.clear();
    m_ImageVector.clear();
}
