#include "../eterLib/StateManager.h"
#include "../eterlib/Camera.h"
#include "StdAfx.h"

#include "../EterGrnLib/GrannyState.hpp"
#include "../EterLib/Engine.hpp"
#include "MapOutdoor.h"
#include "../EterGrnLib/ModelShader.h"

static int recreate = false;

void CMapOutdoor::SetShadowTextureSize(uint16_t size)
{
    if (m_wShadowMapSize == size)
        return;

    ReleaseCharacterShadowTexture();
    m_wShadowMapSize = size;
    CreateCharacterShadowTexture();
}

void CMapOutdoor::CreateCharacterShadowTexture()
{
    assert(m_wShadowMapSize != 0 && "sanity check");

    m_ShadowMapViewport.X = 1;
    m_ShadowMapViewport.Y = 1;
    m_ShadowMapViewport.Width = m_wShadowMapSize - 2;
    m_ShadowMapViewport.Height = m_wShadowMapSize - 2;
    m_ShadowMapViewport.MinZ = 0.0f;
    m_ShadowMapViewport.MaxZ = 1.0f;

    if (FAILED(ms_lpd3dDevice->CreateTexture(m_wShadowMapSize, m_wShadowMapSize, 1, D3DUSAGE_RENDERTARGET,
                                             D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lpCharacterShadowMapTexture, NULL)))
    {
        SPDLOG_ERROR("Unable to create Character Shadow render target texture");
        return;
    }

    if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wShadowMapSize, m_wShadowMapSize, D3DFMT_D24X8,
                                                         D3DMULTISAMPLE_NONE, 0, TRUE,
                                                         &m_lpCharacterShadowMapColorSurface, NULL)))
    {
        SPDLOG_ERROR("Unable to create Character Shadow depth Surface");
        return;
    }
}

void CMapOutdoor::ReleaseCharacterShadowTexture()
{
    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetShadowTexture((LPDIRECT3DTEXTURE9)0);
    }
    M2_SAFE_RELEASE(m_lpCharacterShadowMapTexture, 0);
    M2_SAFE_RELEASE(m_lpCharacterShadowMapColorSurface, 0);
}

DWORD dwLightEnable = false;
bool CMapOutdoor::BeginRenderCharacterShadowToTexture()
{
    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetShadowTexture((LPDIRECT3DTEXTURE9)0);
    }

    Matrix matLightView, matLightProj;

    CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pCurrentCamera)
        return false;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CMapOutdoor::BeginRenderCharacterShadowToTexture **");

    if (!m_lpCharacterShadowMapTexture)
        CreateCharacterShadowTexture();

    Vector3 v3Target = pCurrentCamera->GetTarget();

    Vector3 v3Eye(v3Target.x - 1.732f * 1250.0f, v3Target.y - 1250.0f, v3Target.z + 2.0f * 1.732f * 1250.0f);

    matLightView = Matrix::CreateLookAt(v3Eye, v3Target, Vector3::Backward);

    matLightProj = Matrix::CreateOrthographic(2550.0f, 2550.0f, 1.0f, 15000.0f);

    STATEMANAGER.SaveTransform(D3DTS_VIEW, &matLightView);
    STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matLightProj);

    Matrix mViewToLightProj;
    mViewToLightProj = ms_matView;
    mViewToLightProj = mViewToLightProj.Invert();
    mViewToLightProj = mViewToLightProj * matLightView;
    mViewToLightProj = mViewToLightProj * matLightProj;
    // Compute the texture matrix
    float fBias = -0.0005f;

    float fTexOffs = 0.5 + (0.5 / (float)m_wShadowMapSize);
    Matrix matTexAdj(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, fTexOffs, fTexOffs,
                         fBias, 1.0f);

    Matrix shadowMapMatrix = mViewToLightProj * matTexAdj;

    if (ms)
    {
        ms->SetShadowTexMatrix(shadowMapMatrix);
        ms->SetViewToLightProjection(mViewToLightProj);

        Matrix transViewProj = XMMatrixMultiplyTranspose(matLightView, matLightProj);

        ms->SetTransformConstant(transViewProj);
    }

    bool bSuccess = true;

    // Backup Device Context
    if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface)))
    {
        SPDLOG_ERROR("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Render Target");
        bSuccess = false;
    }

    if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface)))
    {
        SPDLOG_ERROR("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Depth Surface");
        bSuccess = false;
    }

    IDirect3DSurface9 *pSurface = NULL;
    HRESULT hr = m_lpCharacterShadowMapTexture->GetSurfaceLevel(0, &pSurface);
    if (FAILED(hr))
        return false;

    if (FAILED(ms_lpd3dDevice->SetDepthStencilSurface(m_lpCharacterShadowMapColorSurface)))
    {
        SPDLOG_ERROR("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map Render Target");
        bSuccess = false;
    }

    if (FAILED(ms_lpd3dDevice->SetRenderTarget(0, pSurface)))
    {
        SPDLOG_ERROR("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map Render Target");
        bSuccess = false;
    }
    pSurface->Release();
    hr = ms_lpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0);
    // Engine::GetDevice().Clear(true, true, false);
    if (FAILED(ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF),
                                     1.0f, 0)))
    {
        SPDLOG_ERROR("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Clear Render Target");
        bSuccess = false;
    }

    if (FAILED(ms_lpd3dDevice->GetViewport(&m_BackupViewport)))
    {
        SPDLOG_ERROR("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Viewport");
        bSuccess = false;
    }

    if (FAILED(ms_lpd3dDevice->SetViewport(&m_ShadowMapViewport)))
    {
        SPDLOG_ERROR("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map viewport");
        bSuccess = false;
    }

    D3DPERF_EndEvent();
    if (ms)
    {
        ms->SetPassIndex(MODEL_SHADER_PASS_SHADOW);
    }

    return bSuccess;
}

void CMapOutdoor::EndRenderCharacterShadowToTexture()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CMapOutdoor::EndRenderCharacterShadowToTexture **");

    ms_lpd3dDevice->SetViewport(&m_BackupViewport);

    ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);
    ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);

    M2_SAFE_RELEASE(m_lpBackupRenderTargetSurface, 0);
    M2_SAFE_RELEASE(m_lpBackupDepthSurface, 0);

    STATEMANAGER.RestoreTransform(D3DTS_VIEW);
    STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {

        Matrix transViewProj = XMMatrixMultiplyTranspose(ms_matView, ms_matProj);
        ms->SetTransformConstant(transViewProj);
        ms->SetShadowTexture(m_lpCharacterShadowMapTexture);
    }

    // Restore Device Context

    D3DPERF_EndEvent();
}
