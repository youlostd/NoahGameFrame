#include "Stdafx.h"
#include "SimpleLightInstance.h"
#include "../eterLib/GrpLightManager.h"

CDynamicPool<CLightInstance> CLightInstance::ms_kPool;

void CLightInstance::DestroySystem()
{
    ms_kPool.Destroy();
}

CLightInstance *CLightInstance::New()
{
    return ms_kPool.Alloc();
}

void CLightInstance::Delete(CLightInstance *pkData)
{
    pkData->Destroy();
    ms_kPool.Free(pkData);
}

void CLightInstance::OnSetDataPointer(CEffectElementBase *pElement)
{
    Destroy();

    m_pData = ((CLightData *)pElement);

    m_iLoopCount = m_pData->GetLoopCount();

    D3DLIGHT9 Light;
    m_pData->InitializeLight(Light);
    CLightManager::Instance().RegisterLight(LIGHT_TYPE_DYNAMIC, &m_LightID, Light);
}

bool CLightInstance::OnUpdate(double fElapsedTime)
{
    if (m_localTime >= m_pData->GetDuration())
    {
        if (m_pData->isLoop() && --m_iLoopCount != 0)
        {
            if (m_iLoopCount < 0)
                m_iLoopCount = 0;
            m_localTime -= m_pData->GetDuration();
        }
        else
        {
            Destroy();
            m_iLoopCount = 1;
            return false;
        }
    }

    CLight *pLight = CLightManager::Instance().GetLight(m_LightID);

    if (pLight)
    {
        pLight->SetAmbientColor(m_pData->m_cAmbient.R(), m_pData->m_cAmbient.G(), m_pData->m_cAmbient.B(),
                                m_pData->m_cAmbient.A());
        pLight->SetDiffuseColor(m_pData->m_cDiffuse.R(), m_pData->m_cDiffuse.G(), m_pData->m_cDiffuse.B(),
                                m_pData->m_cDiffuse.A());

        float fRange;
        m_pData->GetRange(m_localTime, fRange);
        pLight->SetRange(fRange);

        Vector3 pos;
        m_pData->GetPosition(m_localTime, pos);
        pos = Vector3::Transform(pos, *m_localMat);
        pLight->SetPosition(pos.x, pos.y, pos.z);
    }

    return true;
}

void CLightInstance::OnRender(uint32_t addColor)
{
    //OnUpdate(0);
}

void CLightInstance::OnInitialize()
{
    m_LightID = 0;
    m_dwRangeIndex = 0;
}

void CLightInstance::OnDestroy()
{
    if (m_LightID)
    {
        CLightManager::Instance().DeleteLight(m_LightID);
    }
}

CLightInstance::CLightInstance()
{
    Initialize();
}

CLightInstance::~CLightInstance()
{
    Destroy();
}
