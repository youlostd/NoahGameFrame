#include "StdAfx.h"
#include "EffectElementBaseInstance.h"

#include "../EterBase/StepTimer.h"
#include "../EterBase/Timer.h"

bool CEffectElementBaseInstance::Update(double fElapsedTime)
{
    if (m_isStart)
    {
        m_elapsedTime = fElapsedTime;
        m_localTime += fElapsedTime;

        return OnUpdate(fElapsedTime);
    }
    else
    {
        m_remainingTime -= fElapsedTime;
        if (m_remainingTime <= 0.0f)
            m_isStart = true;
        return true;
    }
}

void CEffectElementBaseInstance::Render(uint32_t addColor)
{
    if (!m_isStart)
        return;

    if (!isActive())
        return;

    if(isHiddenByLod() && !m_isAlwaysShow)
        return;

    assert(m_localMat);

    OnRender(addColor);
}

void CEffectElementBaseInstance::SetLocalMatrixPointer(const Matrix *c_pMatrix)
{
    m_localMat = c_pMatrix;
}

void CEffectElementBaseInstance::SetDataPointer(CEffectElementBase *pElement)
{
    m_pBase = pElement;

    m_startTime = DX::StepTimer::instance().GetTotalMillieSeconds();

    //////////////////////////////////////////////////////////////////////////
    //add by ipkn, start time management

    m_remainingTime = pElement->GetStartTime();
    if (m_remainingTime <= 0.0f)
        m_isStart = true;
    else
        m_isStart = false;

    //////////////////////////////////////////////////////////////////////////

    OnSetDataPointer(pElement);
}

bool CEffectElementBaseInstance::isActive()
{
    return m_isActive;
}

void CEffectElementBaseInstance::SetActive()
{
    m_isActive = true;
}

void CEffectElementBaseInstance::SetDeactive()
{
    m_isActive = false;
}

bool CEffectElementBaseInstance::isHiddenByLod()
{
    return m_isHiddenByLod;
}

void CEffectElementBaseInstance::SetHiddenByLod()
{
    m_isHiddenByLod = true;
}

void CEffectElementBaseInstance::SetShownByLod()
{
    m_isHiddenByLod = false;
}


void CEffectElementBaseInstance::Initialize()
{
    m_localMat = NULL;

    m_isActive = true;

    m_localTime = 0.0f;
    m_startTime = 0;
    m_elapsedTime = 0.0f;

    m_isStart = false;
    m_remainingTime = 0.0f;

    OnInitialize();
}

void CEffectElementBaseInstance::Destroy()
{
    OnDestroy();
    Initialize();
}

CEffectElementBaseInstance::CEffectElementBaseInstance()
{
}

CEffectElementBaseInstance::~CEffectElementBaseInstance()
{
}
