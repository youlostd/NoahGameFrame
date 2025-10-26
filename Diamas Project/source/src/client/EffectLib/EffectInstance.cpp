#include "StdAfx.h"
#include "EffectInstance.h"

#include "../EterBase/StepTimer.h"
#include "ParticleSystemInstance.h"
#include "SimpleLightInstance.h"
#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"
#include "base/Remotery.h"

CDynamicPool<CEffectInstance> CEffectInstance::ms_kPool;
int CEffectInstance::ms_iRenderingEffectCount = 0;

bool CEffectInstance::LessRenderOrder(CEffectInstance *pkEftInst)
{
    return (m_pkEftData < pkEftInst->m_pkEftData);
}

void CEffectInstance::ResetRenderingEffectCount()
{
    ms_iRenderingEffectCount = 0;
}

int CEffectInstance::GetRenderingEffectCount()
{
    return ms_iRenderingEffectCount;
}

CEffectInstance *CEffectInstance::New()
{
    CEffectInstance *pkEftInst = ms_kPool.Alloc();
    return pkEftInst;
}

void CEffectInstance::Delete(CEffectInstance *pkEftInst)
{
    pkEftInst->Clear();
    ms_kPool.Free(pkEftInst);
}

void CEffectInstance::DestroySystem()
{
    ms_kPool.Destroy();

    CParticleSystemInstance::DestroySystem();
    CEffectMeshInstance::DestroySystem();
    CLightInstance::DestroySystem();
}

struct FEffectUpdator
{
    bool isAlive;
    double fElapsedTime;

    FEffectUpdator(double fElapsedTime)
        : isAlive(FALSE), fElapsedTime(fElapsedTime)
    {
    }

    void operator ()(CEffectElementBaseInstance *pInstance)
    {
        if (pInstance->Update(fElapsedTime))
        {
            isAlive = TRUE;
        }
    }
};

void CEffectInstance::OnUpdate()
{
    Transform();

    UpdateLODLevel();

    FEffectUpdator f(DX::StepTimer::Instance().GetTotalSeconds() - m_fLastTime);
    f = std::for_each(m_ParticleInstanceVector.begin(), m_ParticleInstanceVector.end(), f);
    f = std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(), f);
    f = std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(), f);
    m_isAlive = f.isAlive;


    if (m_pSoundInstanceVector)
    {
        // NOTE : 매트릭스에서 위치를 직접 얻어온다 - [levites]
        UpdateSoundInstance(m_dwFrame,
                            *m_pSoundInstanceVector,
                            m_matGlobal._41,
                            m_matGlobal._42,
                            m_matGlobal._43,
                            false);
    }
    m_fLastTime = DX::StepTimer::Instance().GetTotalSeconds();
    ++m_dwFrame;
}

void CEffectInstance::OnRender()
{
    rmt_ScopedCPUSample(EffectInstanceRender, RMTSF_Aggregate);

    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);

    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    Engine::GetDevice().SetDepthEnable(true, false);

    /////

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

    Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_TEX1);

    for (auto &particle : m_ParticleInstanceVector)
        particle->Render(m_addColor);

    for (auto &mesh : m_MeshInstanceVector)
        mesh->Render(m_addColor);

    /////
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);

    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
    Engine::GetDevice().SetDepthEnable(true, true);

    ++ms_iRenderingEffectCount;
}

void CEffectInstance::UpdateLODLevel()
{
    CCamera *pcurCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pcurCamera)
    {
        SPDLOG_ERROR("CEffectInstance::UpdateLODLevel - GetCurrentCamera() == NULL");
        return;
    }

    const Vector3 &c_rv3CameraPosition = pcurCamera->GetEye();
          const Vector3 &c_v3Position = Vector3(m_matGlobal._41,
                            m_matGlobal._42,
                            m_matGlobal._43);
    // NOTE : 중심으로부터의 거리 계산에 z값 차이는 사용하지 않는다. - [levites]
    float distanceFromCamera = (c_rv3CameraPosition - c_v3Position).Length();


    auto lod = std::abs(( 1.0f * distanceFromCamera * tan ( 0.5f * GetFOV() ) )
        / ( 0.5f * (float)ms_iHeight ));
    if(lod < 20.0f)
    {
        SetLodShow();
    }
    else
    {
        SetHiddenByLod();
    }

}

void CEffectInstance::SetGlobalMatrix(const Matrix &c_rmatGlobal)
{
    m_matGlobal = c_rmatGlobal;
}

bool CEffectInstance::isAlive()
{
    return m_isAlive;
}

void CEffectInstance::SetAlwaysRender(bool val)
{
    CGraphicObjectInstance::SetAlwaysRender(val);

    for (auto &particle : m_ParticleInstanceVector)
        particle->SetAlwaysRender(val);

    for (auto &mesh : m_MeshInstanceVector)
        mesh->SetAlwaysRender(val);

    for (auto &light : m_LightInstanceVector)
        light->SetAlwaysRender(val);
}

void CEffectInstance::SetLodShow()
{
    for (auto &particle : m_ParticleInstanceVector)
        particle->SetShownByLod();

    for (auto &mesh : m_MeshInstanceVector)
        mesh->SetShownByLod();

    for (auto &light : m_LightInstanceVector)
        light->SetShownByLod();
}

bool CEffectInstance::isHiddenByLod()
{
    bool anyActive = false;

    for (auto &particle : m_ParticleInstanceVector)
        anyActive = particle->isHiddenByLod();

    for (auto &mesh : m_MeshInstanceVector)
        anyActive = mesh->isHiddenByLod();

    for (auto &light : m_LightInstanceVector)
        anyActive = light->isHiddenByLod();

    return anyActive;
}

void CEffectInstance::SetHiddenByLod()
{
    for (auto &particle : m_ParticleInstanceVector)
        particle->SetHiddenByLod();

    for (auto &mesh : m_MeshInstanceVector)
        mesh->SetHiddenByLod();

    for (auto &light : m_LightInstanceVector)
        light->SetHiddenByLod();
}




void CEffectInstance::SetActive()
{
    for (auto &particle : m_ParticleInstanceVector)
        particle->SetActive();

    for (auto &mesh : m_MeshInstanceVector)
        mesh->SetActive();

    for (auto &light : m_LightInstanceVector)
        light->SetActive();
}

bool CEffectInstance::isActive()
{
    bool anyActive = false;

    for (auto &particle : m_ParticleInstanceVector)
        anyActive = particle->isActive();

    for (auto &mesh : m_MeshInstanceVector)
        anyActive = mesh->isActive();

    for (auto &light : m_LightInstanceVector)
        anyActive = light->isActive();

    return anyActive;
}

void CEffectInstance::SetDeactive()
{
    for (auto &particle : m_ParticleInstanceVector)
        particle->SetDeactive();

    for (auto &mesh : m_MeshInstanceVector)
        mesh->SetDeactive();

    for (auto &light : m_LightInstanceVector)
        light->SetDeactive();
}

void CEffectInstance::__SetParticleData(CParticleSystemData *pData)
{
    CParticleSystemInstance *pInstance = CParticleSystemInstance::New();
    pInstance->SetDataPointer(pData);
    pInstance->SetLocalMatrixPointer(&m_matGlobal);
    pInstance->SetScale(m_scale);

    m_ParticleInstanceVector.push_back(pInstance);
}

void CEffectInstance::__SetMeshData(CEffectMeshScript *pMesh)
{
    CEffectMeshInstance *pMeshInstance = CEffectMeshInstance::New();
    pMeshInstance->SetDataPointer(pMesh);
    pMeshInstance->SetLocalMatrixPointer(&m_matGlobal);

    m_MeshInstanceVector.push_back(pMeshInstance);
}

void CEffectInstance::__SetLightData(CLightData *pData)
{
    CLightInstance *pInstance = CLightInstance::New();
    pInstance->SetDataPointer(pData);
    pInstance->SetLocalMatrixPointer(&m_matGlobal);

    m_LightInstanceVector.push_back(pInstance);
}

void CEffectInstance::SetEffectDataPointer(CEffectData *pEffectData)
{
    m_isAlive = true;

    m_pkEftData = pEffectData;

    m_fLastTime = DX::StepTimer::Instance().GetTotalSeconds();
    m_fBoundingSphereRadius = pEffectData->GetBoundingSphereRadius();
    m_v3BoundingSpherePosition = pEffectData->GetBoundingSpherePosition();

    if (m_fBoundingSphereRadius > 0.0f)
        CGraphicObjectInstance::RegisterBoundingSphere();

    uint32_t i;

    for (i = 0; i < pEffectData->GetParticleCount(); ++i)
    {
        CParticleSystemData *pParticle = pEffectData->GetParticlePointer(i);

        __SetParticleData(pParticle);
    }

    for (i = 0; i < pEffectData->GetMeshCount(); ++i)
    {
        CEffectMeshScript *pMesh = pEffectData->GetMeshPointer(i);

        __SetMeshData(pMesh);
    }

    for (i = 0; i < pEffectData->GetLightCount(); ++i)
    {
        CLightData *pLight = pEffectData->GetLightPointer(i);

        __SetLightData(pLight);
    }

    m_pSoundInstanceVector = pEffectData->GetSoundInstanceVector();
}

void CEffectInstance::SetScale(float scale)
{
    m_scale = scale;
}

bool CEffectInstance::GetBoundingSphere(Vector3 &v3Center, float &fRadius)
{
    v3Center.x = m_matGlobal._41 + m_v3BoundingSpherePosition.x;
    v3Center.y = m_matGlobal._42 + m_v3BoundingSpherePosition.y;
    v3Center.z = m_matGlobal._43 + m_v3BoundingSpherePosition.z;
    fRadius = m_fBoundingSphereRadius;
    return true;
}

void CEffectInstance::SetBaseEffectId(uint32_t id)
{
    m_baseEffectId = id;
}

void CEffectInstance::SetEffectKind(uint32_t kind)
{
    m_effectKind = kind;
}

uint32_t CEffectInstance::GetEffectKind()
{
    return m_effectKind;
}

void CEffectInstance::Clear()
{
    if (!m_ParticleInstanceVector.empty())
    {
        std::for_each(m_ParticleInstanceVector.begin(), m_ParticleInstanceVector.end(),
                      CParticleSystemInstance::Delete);
        m_ParticleInstanceVector.clear();
    }

    if (!m_MeshInstanceVector.empty())
    {
        std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(), CEffectMeshInstance::Delete);
        m_MeshInstanceVector.clear();
    }

    if (!m_LightInstanceVector.empty())
    {
        std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(), CLightInstance::Delete);
        m_LightInstanceVector.clear();
    }

    __Initialize();
}

void CEffectInstance::__Initialize()
{
    m_isAlive = FALSE;
    m_dwFrame = 0;
    m_pSoundInstanceVector = NULL;
    m_fBoundingSphereRadius = 0.0f;
    m_v3BoundingSpherePosition.x = m_v3BoundingSpherePosition.y = m_v3BoundingSpherePosition.z = 0.0f;
    m_addColor = 0;
    m_pkEftData = NULL;

    m_matGlobal = Matrix::Identity;
}

CEffectInstance::CEffectInstance()
{
    __Initialize();
}

CEffectInstance::~CEffectInstance()
{
    assert(m_ParticleInstanceVector.empty());
    assert(m_MeshInstanceVector.empty());
    assert(m_LightInstanceVector.empty());
}
