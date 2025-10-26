#include "StdAfx.h"
#include "SnowEnvironment.h"

#include "../EterBase/StepTimer.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/ResourceManager.h"
#include "SnowParticle.h"
#include "../eterBase/Timer.h"
#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"

void CSnowEnvironment::Enable()
{
    if (!m_bSnowEnable)
    {
        Create();
    }

    m_bSnowEnable = TRUE;
}

void CSnowEnvironment::Disable() { m_bSnowEnable = FALSE; }

void CSnowEnvironment::Update(const Vector3 &c_rv3Pos)
{
    if (!m_bSnowEnable)
    {
        if (m_kVct_pkParticleSnow.empty())
            return;
    }

    m_v3Center = c_rv3Pos;
}

void CSnowEnvironment::Deform()
{
    if (!m_bSnowEnable)
    {
        if (m_kVct_pkParticleSnow.empty())
            return;
    }

    const Vector3 &c_rv3Pos = m_v3Center;

    float fElapsedTime = DX::StepTimer::Instance().GetElapsedSeconds();

    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pCamera)
        return;

    const Vector3 &c_rv3View = pCamera->GetView();

    Vector3 v3ChangedPos = c_rv3View * 3500.0f + c_rv3Pos;
    v3ChangedPos.z = c_rv3Pos.z;

    auto itor = m_kVct_pkParticleSnow.begin();
    for (; itor != m_kVct_pkParticleSnow.end();)
    {
        auto pSnow = (*itor).get();
        pSnow->Update(fElapsedTime, v3ChangedPos);

        if (!pSnow->IsActivate())
        {
            itor = m_kVct_pkParticleSnow.erase(itor);
        }
        else
        {
            ++itor;
        }
    }

    if (m_bSnowEnable)
    {
        for (int p = 0; p < std::min<uint32_t>(10, m_dwParticleMaxNum - m_kVct_pkParticleSnow.size()); ++p)
        {
            auto pSnowParticle = std::make_unique<CSnowParticle>();
            pSnowParticle->Init(v3ChangedPos);
            m_kVct_pkParticleSnow.emplace_back(std::move(pSnowParticle));
        }
    }
}

void CSnowEnvironment::__BeginBlur()
{
    if (!m_bBlurEnable)
        return;

    ms_lpd3dDevice->GetRenderTarget(0, &m_lpOldSurface);
    ms_lpd3dDevice->GetDepthStencilSurface(&m_lpOldDepthStencilSurface);
    ms_lpd3dDevice->SetRenderTarget(0, m_lpSnowRenderTargetSurface);
    ms_lpd3dDevice->SetDepthStencilSurface(m_lpSnowDepthSurface);
    ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);

    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
}

void CSnowEnvironment::__ApplyBlur()
{
    if (!m_bBlurEnable)
        return;

    //			{
    //				STATEMANAGER.SetRenderState( D3DRS_ALPHABLENDENABLE,   true );
    //				STATEMANAGER.SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    //				STATEMANAGER.SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    //				STATEMANAGER.SetRenderState( D3DRS_COLORVERTEX ,true);
    //				STATEMANAGER.SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE , D3DMCS_COLOR1 );
    //				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    //				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    //				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    //				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    //				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    //				STATEMANAGER.SetTextureStageState(0,  D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    //				uint32_t	alphaColor = 0xFFFFFF | ((uint32_t)(0.6f*255.0f) << 24);
    //
    //				BlurVertex V[4] = { BlurVertex(Vector3(0.0f,0.0f,0.0f),1.0f,		alphaColor, 0,0) ,
    //									BlurVertex(Vector3(wTextureSize,0.0f,0.0f),1.0f,		alphaColor, 1,0) ,
    //									BlurVertex(Vector3(0.0f,wTextureSize,0.0f),1.0f,		alphaColor, 0,1) ,
    //									BlurVertex(Vector3(wTextureSize,wTextureSize,0.0f),1.0f,	alphaColor, 1,1) };
    //				//누적 블러 텍스쳐를 찍는다.
    //				STATEMANAGER.SetTexture(0,m_lpAccumTexture);
    //				Engine::GetDevice().SetFvF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE|D3DFVF_TEX1);
    //				Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,V,sizeof(BlurVertex));
    //			}
    //
    //			{
    //				STATEMANAGER.SetRenderTarget(m_lpAccumRenderTargetSurface, m_lpAccumDepthSurface);
    //
    //				BlurVertex V[4] = { BlurVertex(Vector3(0.0f,0.0f,0.0f),1.0f,		0xFFFFFF, 0,0) ,
    //									BlurVertex(Vector3(wTextureSize,0.0f,0.0f),1.0f,		0xFFFFFF, 1,0) ,
    //									BlurVertex(Vector3(0.0f,wTextureSize,0.0f),1.0f,		0xFFFFFF, 0,1) ,
    //									BlurVertex(Vector3(wTextureSize,wTextureSize,0.0f),1.0f,	0xFFFFFF, 1,1) };
    //
    //				STATEMANAGER.SetTexture(0,m_lpSnowTexture);
    //				STATEMANAGER.SetRenderState( D3DRS_ALPHABLENDENABLE,   false);
    //				STATEMANAGER.SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE|D3DFVF_TEX1);
    //				Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,V,sizeof(BlurVertex));
    //			}

    ///////////////
    {
        ms_lpd3dDevice->SetRenderTarget(0, m_lpOldSurface);
        ms_lpd3dDevice->SetDepthStencilSurface(m_lpOldDepthStencilSurface);

        STATEMANAGER.SetTexture(0, m_lpSnowTexture);
        STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, true);

        D3DSURFACE_DESC desc;
        m_lpOldSurface->GetDesc(&desc);
        float sx = (float)desc.Width;
        float sy = (float)desc.Height;
        M2_SAFE_RELEASE_CHECK(m_lpOldSurface);
        M2_SAFE_RELEASE_CHECK(m_lpOldDepthStencilSurface);

        BlurVertex V[4] = {BlurVertex(Vector3(0.0f, 0.0f, 0.0f), 1.0f, 0xFFFFFF, 0, 0),
                           BlurVertex(Vector3(sx, 0.0f, 0.0f), 1.0f, 0xFFFFFF, 1, 0),
                           BlurVertex(Vector3(0.0f, sy, 0.0f), 1.0f, 0xFFFFFF, 0, 1),
                           BlurVertex(Vector3(sx, sy, 0.0f), 1.0f, 0xFFFFFF, 1, 1)};

        Engine::GetDevice().SetFvF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(BlurVertex));
    }
}

void CSnowEnvironment::Render()
{

    if (!m_bSnowEnable)
    {
        if (m_kVct_pkParticleSnow.empty())
            return;
    }

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CSnowEnvironment::__BeginBlur **");
    __BeginBlur();
    D3DPERF_EndEvent();

    uint32_t dwParticleCount = std::min<size_t>(m_dwParticleMaxNum, m_kVct_pkParticleSnow.size());

    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pCamera)
        return;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CSnowEnvironment::Render **");

    const auto &c_rv3Up = pCamera->GetUp();
    const auto &c_rv3Cross = pCamera->GetCross();

    if (!isValid(m_pVB))
        return;

    if (!isValid(m_pIB))
        return;

    auto pv3Verticies =
        static_cast<SParticleVertex *>(Engine::GetDevice().LockVertexBuffer(m_pVB, D3DLOCK_DISCARD, 0, dwParticleCount * 4 ));
    if (pv3Verticies)
    {
        for (int i = 0; i < m_kVct_pkParticleSnow.size(); i++)
        {
          auto *pSnow = m_kVct_pkParticleSnow[i].get();
            if (pSnow)
            {
              pSnow->SetCameraVertex(c_rv3Up, c_rv3Cross);
              pSnow->GetVerticies(pv3Verticies[i * 4 + 0], pv3Verticies[i * 4 + 1], pv3Verticies[i * 4 + 2],
                                  pv3Verticies[i * 4 + 3]);
            }
        }
    }
    Engine::GetDevice().UnlockVertexBuffer(m_pVB);

    Engine::GetDevice().SetDepthEnable(true, false);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTexture(1, NULL);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    m_pImageInstance->GetGraphicImagePointer()->GetTextureReference().SetTextureStage(0);
    Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_TEX1);
    Engine::GetDevice().SetVertexBuffer(m_pVB, 0, 0);
    Engine::GetDevice().SetIndexBuffer(m_pIB);

    Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dwParticleCount * 4, 0, dwParticleCount * 2);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    Engine::GetDevice().SetDepthEnable(true, true);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
    D3DPERF_EndEvent();

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CSnowEnvironment::__ApplyBlur **");
    __ApplyBlur();
    D3DPERF_EndEvent();
}

bool CSnowEnvironment::__CreateBlurTexture()
{
    if (!m_bBlurEnable)
        return true;

    if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET,
                                             D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpSnowTexture, NULL)))
        return false;
    if (FAILED(m_lpSnowTexture->GetSurfaceLevel(0, &m_lpSnowRenderTargetSurface)))
        return false;
    if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16,
                                                         D3DMULTISAMPLE_NONE, 0, false, &m_lpSnowDepthSurface, NULL)))
        return false;

    if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET,
                                             D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpAccumTexture, NULL)))
        return false;
    if (FAILED(m_lpAccumTexture->GetSurfaceLevel(0, &m_lpAccumRenderTargetSurface)))
        return false;
    if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16,
                                                         D3DMULTISAMPLE_NONE, 0, false, &m_lpAccumDepthSurface, NULL)))
        return false;

    return true;
}

bool CSnowEnvironment::__CreateGeometry()
{

    m_pVB = Engine::GetDevice().CreateVertexBuffer(m_dwParticleMaxNum * 4 * sizeof(SParticleVertex), sizeof(SParticleVertex), nullptr,
                                                   D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY);

    const uint16_t c_awFillRectIndices[6] = {
        0, 2, 1, 2, 3, 1,
    };
    m_pIB = Engine::GetDevice().CreateIndexBuffer(m_dwParticleMaxNum * 6, D3DFMT_INDEX16, nullptr, D3DUSAGE_WRITEONLY);

    if (isValid(m_pIB))
    {
        uint16_t *dstIndices = (uint16_t *)Engine::GetDevice().LockIndexBuffer(m_pIB);
        for (int i = 0; i < m_dwParticleMaxNum; ++i)
        {
            for (int j = 0; j < 6; ++j)
            {
                dstIndices[i * 6 + j] = i * 4 + c_awFillRectIndices[j];
            }
        }
        Engine::GetDevice().UnlockIndexBuffer(m_pIB);
    }

    return true;
}

bool CSnowEnvironment::Create()
{
    Destroy();

    if (!__CreateBlurTexture())
        return false;

    if (!__CreateGeometry())
        return false;

    CGraphicImage::Ptr pImage =
        CResourceManager::Instance().LoadResource<CGraphicImage>("d:/ymir work/special/snow.dds");
    m_pImageInstance = std::make_unique<CGraphicImageInstance>("d:/ymir work/special/snow.dds");
    m_pImageInstance->SetImagePointer(pImage);

    return true;
}

void CSnowEnvironment::Destroy()
{
    M2_SAFE_RELEASE_CHECK(m_lpSnowTexture);
    M2_SAFE_RELEASE_CHECK(m_lpSnowRenderTargetSurface);
    M2_SAFE_RELEASE_CHECK(m_lpSnowDepthSurface);
    M2_SAFE_RELEASE_CHECK(m_lpAccumTexture);
    M2_SAFE_RELEASE_CHECK(m_lpAccumRenderTargetSurface);
    M2_SAFE_RELEASE_CHECK(m_lpAccumDepthSurface);
    Engine::GetDevice().DeleteIndexBuffer(m_pIB);
    Engine::GetDevice().DeleteVertexBuffer(m_pVB);
    

    m_kVct_pkParticleSnow.clear();

    m_pImageInstance.reset();

    __Initialize();
}

void CSnowEnvironment::__Initialize()
{
    m_bSnowEnable = FALSE;
    m_lpSnowTexture = NULL;
    m_lpSnowRenderTargetSurface = NULL;
    m_lpSnowDepthSurface = NULL;
    m_lpAccumTexture = NULL;
    m_lpAccumRenderTargetSurface = NULL;
    m_lpAccumDepthSurface = NULL;
    m_pIB = ENGINE_INVALID_HANDLE;
    m_pVB = ENGINE_INVALID_HANDLE;

    m_pImageInstance = NULL;

    m_kVct_pkParticleSnow.reserve(m_dwParticleMaxNum);
}

CSnowEnvironment::CSnowEnvironment()
{
    m_bBlurEnable = FALSE;
    m_dwParticleMaxNum = 3000;
    m_wBlurTextureSize = 512;

    __Initialize();
}

CSnowEnvironment::~CSnowEnvironment() { Destroy(); }
