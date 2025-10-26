#include "../eterLib/StateManager.h"
#include "StdAfx.h"

#include "../EterGrnLib/GrannyState.hpp"
#include "../EterLib/Engine.hpp"
#include "ActorInstance.h"
#include "../EterGrnLib/ModelShader.h"
#include "base/Remotery.h"
#include <DirectXPackedVector.h>

bool CActorInstance::ms_isDirLine = false;

bool CActorInstance::IsDirLine()
{
    return ms_isDirLine;
}

void CActorInstance::ShowDirectionLine(bool isVisible)
{
    ms_isDirLine = isVisible;
}

void CActorInstance::SetMaterialColor(uint32_t dwColor)
{
    if (m_pkHorse)
        m_pkHorse->SetMaterialColor(dwColor);

    m_dwMtrlColor &= 0xff000000;
    m_dwMtrlColor |= (dwColor & 0x00ffffff);
}

void CActorInstance::SetMaterialAlpha(uint32_t dwAlpha)
{
    m_dwMtrlAlpha = dwAlpha;
}

void CActorInstance::OnRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CActorInstance::OnRender **");
    rmt_ScopedCPUSample(ActorInstanceRender, RMTSF_Aggregate);

    D3DMATERIAL9 kMtrl;
    STATEMANAGER.GetMaterial(&kMtrl);

    // todo: better solution ..
    kMtrl.Diffuse = D3DXCOLOR(m_dwMtrlColor);
    STATEMANAGER.SetMaterial(&kMtrl);
    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetMaterial(kMtrl);
    }

    // 현재는 이렇게.. 최종적인 형태는 Diffuse와 Blend의 분리로..
    // 아니면 이런 형태로 가되 Texture & State Sorting 지원으로.. - [levites]
    // STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    switch (m_iRenderMode)
    {
    case RENDER_MODE_NORMAL:
        BeginDiffuseRender();
            RenderWithOneTexture(!__IsEmotionMotion());
        EndDiffuseRender();
        BeginOpacityRender();
            BlendRenderWithOneTexture();
        EndOpacityRender();
        break;
    case RENDER_MODE_DIFFUSE_ONLY:
        BeginDiffuseRender();
            RenderWithOneTexture(!__IsEmotionMotion());
        EndDiffuseRender();
        break;

    case RENDER_MODE_BLEND:
        if (m_fAlphaValue == 1.0f)
        {
            BeginDiffuseRender();
                RenderWithOneTexture(!__IsEmotionMotion());
            EndDiffuseRender();
            BeginOpacityRender();
                BlendRenderWithOneTexture();
            EndOpacityRender();
        }
        else if (m_fAlphaValue > 0.0f)
        {
            BeginBlendRender();
                RenderWithOneTexture(!__IsEmotionMotion());
                BlendRenderWithOneTexture();
            EndBlendRender(); // BlendRenderWithOneTexture();
        }
        break;
    case RENDER_MODE_ADD:
        // BeginAddRender();
        RenderWithOneTexture(!__IsEmotionMotion());
        BlendRenderWithOneTexture();
        // EndAddRender();
        break;
    case RENDER_MODE_MODULATE:
        // BeginModulateRender();
        RenderWithOneTexture(!__IsEmotionMotion());
        BlendRenderWithOneTexture();
        // EndModulateRender();
        break;
    }

    // STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

    kMtrl.Diffuse = D3DXCOLOR(0xffffffff);
    STATEMANAGER.SetMaterial(&kMtrl);

    if (ms)
    {
        ms->SetMaterial(kMtrl);
    }

    D3DPERF_EndEvent();
}

void CActorInstance::BeginDiffuseRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CActorInstance::BeginDiffuseRender **");

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        if (ms->GetPassIndex() == MODEL_SHADER_PASS_SHADOW)
            return;

        ms->SetDiffuseAddColor(m_AddColor);
        ms->SetAlphaBlendValue(m_fAlphaValue);
        ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE);
    }

    D3DPERF_EndEvent();
}

void CActorInstance::EndDiffuseRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CActorInstance::EndDiffuseRender **");

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        if (ms->GetPassIndex() == MODEL_SHADER_PASS_SHADOW)
            return;

        ms->SetDiffuseAddColor(DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f));
        ms->SetAlphaBlendValue(m_fAlphaValue);
        ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE);
    }
    D3DPERF_EndEvent();
}

void CActorInstance::BeginOpacityRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CActorInstance::BeginOpacityRender **");

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        if (ms->GetPassIndex() == MODEL_SHADER_PASS_SHADOW)
            return;

        ms->SetDiffuseAddColor(m_AddColor);
        ms->SetPassIndex(MODEL_SHADER_PASS_OPACITY);
    }
    D3DPERF_EndEvent();
}

void CActorInstance::EndOpacityRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CActorInstance::EndOpacityRender **");

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        if (ms->GetPassIndex() == MODEL_SHADER_PASS_SHADOW)
            return;

        ms->SetDiffuseAddColor(DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f));
        ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE);
    }
    D3DPERF_EndEvent();
}

void CActorInstance::BeginBlendRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CActorInstance::BeginBlendRender **");

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        if (ms->GetPassIndex() == MODEL_SHADER_PASS_SHADOW)
            return;

        ms->SetDiffuseAddColor(m_AddColor);
        ms->SetAlphaBlendValue(m_fAlphaValue);
        ms->SetPassIndex(MODEL_SHADER_PASS_BLEND);
    }
    D3DPERF_EndEvent();
}

void CActorInstance::EndBlendRender()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CActorInstance::EndBlendRender **");

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        if (ms->GetPassIndex() == MODEL_SHADER_PASS_SHADOW)
            return;

        ms->SetDiffuseAddColor(DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f));
        ms->SetAlphaBlendValue(m_fAlphaValue);
        ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE);
    }
    D3DPERF_EndEvent();
}


void CActorInstance::RestoreRenderMode()
{
    // NOTE : This is temporary code. I wanna convert this code to that restore the mode to
    //        model's default setting which had has as like specular or normal. - [levites]
    m_iRenderMode = RENDER_MODE_NORMAL;
    if (m_kBlendAlpha.m_isBlending)
    {
        m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
    }
}

void CActorInstance::SetRenderMode(int iRenderMode)
{
    m_iRenderMode = iRenderMode;
    if (m_kBlendAlpha.m_isBlending)
    {
        m_kBlendAlpha.m_iOldRenderMode = iRenderMode;
    }
}

void CActorInstance::SetAddColor(const DirectX::SimpleMath::Color &c_rColor)
{
    m_AddColor = c_rColor;
    m_AddColor.A(1.0f);
}

void CActorInstance::RenderCollisionData()
{
    static CScreen s_Screen;
    DWORD lighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);

    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    if (m_pAttributeInstance)
    {
        for (uint32_t col = 0; col < GetCollisionInstanceCount(); ++col)
        {
            CBaseCollisionInstance *pInstance = GetCollisionInstanceData(col);
            pInstance->Render();
        }
    }

    Engine::GetDevice().SetDepthEnable(false, false);

    s_Screen.SetColorOperation();
    s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);
    TCollisionPointInstanceList::iterator itor;
    /*itor = m_AttackingPointInstanceList.begin();
    for (; itor != m_AttackingPointInstanceList.end(); ++itor)
    {
        const TCollisionPointInstance & c_rInstance = *itor;
        for (uint32_t i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
        {
            const CDynamicSphereInstance & c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
            s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x,
                                    c_rSphereInstance.v3Position.y,
                                    c_rSphereInstance.v3Position.z,
                                    c_rSphereInstance.fRadius);
        }
    }*/
    s_Screen.SetDiffuseColor(1.0f, (isShow()) ? 1.0f : 0.0f, 0.0f);
    Vector3 center;
    float r;
    GetBoundingSphere(center, r);
    s_Screen.RenderCircle3d(center.x, center.y, center.z, r);

    s_Screen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
    itor = m_DefendingPointInstanceList.begin();
    for (; itor != m_DefendingPointInstanceList.end(); ++itor)
    {
        const TCollisionPointInstance &c_rInstance = *itor;
        for (uint32_t i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
        {
            const CDynamicSphereInstance &c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
            s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x, c_rSphereInstance.v3Position.y,
                                    c_rSphereInstance.v3Position.z, c_rSphereInstance.fRadius);
        }
    }

    s_Screen.SetDiffuseColor(0.0f, 1.0f, 0.0f);
    itor = m_BodyPointInstanceList.begin();
    for (; itor != m_BodyPointInstanceList.end(); ++itor)
    {
        const TCollisionPointInstance &c_rInstance = *itor;
        for (uint32_t i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
        {
            const CDynamicSphereInstance &c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
            s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x, c_rSphereInstance.v3Position.y,
                                    c_rSphereInstance.v3Position.z, c_rSphereInstance.fRadius);
        }
    }

    s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);
    //	if (m_SplashArea.fDisappearingTime > GetLocalTime())
    {
        CDynamicSphereInstanceVector::iterator itor = m_kSplashArea.SphereInstanceVector.begin();
        for (; itor != m_kSplashArea.SphereInstanceVector.end(); ++itor)
        {
            const CDynamicSphereInstance &c_rInstance = *itor;
            s_Screen.RenderCircle3d(c_rInstance.v3Position.x, c_rInstance.v3Position.y, c_rInstance.v3Position.z,
                                    c_rInstance.fRadius);
        }
    }

    Engine::GetDevice().SetDepthEnable(true, true);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, lighting);
}

void CActorInstance::RenderToShadowMap(bool showWeapon)
{
    if (RENDER_MODE_BLEND == m_iRenderMode)
    {
        if (GetAlphaValue() < 0.5f)
            return;
    }

    CGraphicThingInstance::RenderToShadowMap(!IsEmotionMotion());

    if (m_pkHorse)
        m_pkHorse->RenderToShadowMap(showWeapon);
}
