#include "StdAfx.h"

#include "../eterLib/Camera.h"
#include "../eterLib/RenderTargetManager.h"
#include "../eterPythonLib/PythonGraphic.h"
#include "PythonApplication.h"
#include "PythonItemRenderTargetManager.h"
#include "base/Remotery.h"

#include <game/MotionConstants.hpp>

CPythonItemRenderTargetManager::CPythonItemRenderTargetManager()
    : m_pModel(nullptr)
    , m_bgImage(nullptr)
    , m_modelRotation(0)
{
}

CPythonItemRenderTargetManager::~CPythonItemRenderTargetManager()
{
    Destroy();
}

void CPythonItemRenderTargetManager::__Initialize()
{
    m_pModel = nullptr;
    m_modelRotation = 0.0f;
    m_bgImage = nullptr;
}

void CPythonItemRenderTargetManager::Destroy()
{
    m_modelRotation = 0.0f;
}

bool CPythonItemRenderTargetManager::SetModelInstance(
    CInstanceBase::SCreateData& createData)
{
    m_pModel = std::make_unique<CInstanceBase>();
    return m_pModel->Create(createData);
}

void CPythonItemRenderTargetManager::Select(
    CInstanceBase::SCreateData& createData)
{
    m_pModel.reset();
    if (!SetModelInstance(createData)) {
        return;
    }

    if (!m_pModel)
        return;

    m_modelRotation = 0.0f;


    m_pModel->PushLoopMotion(MOTION_WAIT);
    if(m_pModel->IsMountingHorse()) {
        m_pModel->GetHorseInfo().GetActorRef().SetAlwaysRenderAttachingEffect();
    }
    m_pModel->SetAlwaysRenderAttachingEffect();

    m_pModel->SetAlwaysRender(true);
    m_pModel->Refresh(MOTION_WAIT, true);
    m_pModel->SetRotation(m_modelRotation);
    m_pModel->NEW_SetPixelPosition(TPixelPosition(0.0f, 0.0f, 0.0f));
    m_pModel->RefreshWeapon();
    m_pModel->RefreshAcce();

    m_modelHeight = m_pModel->GetGraphicThingInstanceRef().GetHeight();
    m_zoom = 0.0f;
    m_targetY = 0.0f;
    m_fModelZoomSpeed = m_modelHeight * 8.9f / 140.0f;
    m_minZoom = m_modelHeight / 35.0f;

    CCameraManager::instance().SetCurrentCamera(
        CCameraManager::ILLUSTRATION_CAMERA);
    CCamera* pCam = CCameraManager::instance().GetCurrentCamera();
    pCam->SetTargetHeight(m_modelHeight / 2.0f);
    CCameraManager::instance().ResetToPreviousCamera();
}

bool CPythonItemRenderTargetManager::CreateModelBackground(uint32_t width,
                                                           uint32_t height)
{
    if (m_bgImage)
        return false;

    m_bgImage.reset(new CGraphicExpandedImageInstance);

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>("d:/ymir "
                                                                      "work/ui/"
                                                                      "model-"
                                                                      "bg-hd."
                                                                      "png");
    if (!r) {
        SPDLOG_ERROR("Failed to load {0}", "d:/ymir work/ui/model-bg-hd.png");
        return false;
    }

    m_bgImage->SetImagePointer(r);
    m_bgImage->SetScale(float(width) / float(m_bgImage->GetWidth()),
                        float(height) / float(m_bgImage->GetHeight()));

    return true;
}

void CPythonItemRenderTargetManager::RenderBackground()
{
    if (!m_isShow)
        return;

    auto& graphic = CPythonGraphic::Instance();
    auto& rtm = CRenderTargetManager::instance();

    CRenderTargetManager::instance().ClearRenderTarget();
    graphic.SetInterfaceRenderState();

    RECT rectRender;
    if (!rtm.GetRenderTargetRect(CRenderTargetManager::ILLUSTRADED_BG, rectRender))
        return;

    if (!rtm.ChangeRenderTarget(CRenderTargetManager::ILLUSTRADED_BG))
        return;

    if (m_bgImage && !m_bgImage->IsEmpty()) {
        m_bgImage->Render();
    }

    rtm.ResetRenderTarget();
}

void CPythonItemRenderTargetManager::Drag(int32_t x, int32_t y)
{
    if (!m_isShow)
        return;

    if (!m_pModel)
        return;

    m_baseRotation = fmod(m_baseRotation + (x / 50.0f), 360.0f);
}

void CPythonItemRenderTargetManager::Update()
{
    if (!m_isShow)
        return;

    if (!m_pModel)
        return;

    if (m_enableRotation) {
        if (m_modelRotation < 360.0f)
            m_modelRotation += (1.0f / 100.0f * m_rotSpeed);
        else
            m_modelRotation = 0.0f;
    } else {
        m_modelRotation = m_baseRotation;
    }

    m_pModel->Update();
    m_pModel->SetAlwaysRenderAttachingEffect();
    if(m_pModel->IsMountingHorse()) {
        m_pModel->GetHorseInfo().GetActorRef().SetAlwaysRenderAttachingEffect();
    }
    m_pModel->SetRotation(m_modelRotation);
    m_pModel->Transform();
    m_pModel->GetGraphicThingInstanceRef().RotationProcess();
}

void CPythonItemRenderTargetManager::Deform()
{
    rmt_ScopedCPUSample(ITM_Deform, 0);

    if (!m_isShow)
        return;

    if (m_pModel)
        m_pModel->Deform();
}

void CPythonItemRenderTargetManager::RenderModel()
{
    if (!m_isShow)
        return;
    rmt_ScopedCPUSample(ITM_RenderModel, 0);

    auto& graphic = CPythonGraphic::Instance();
    auto& rtm = CRenderTargetManager::instance();

    RECT rectRender;
    if (!rtm.GetRenderTargetRect(CRenderTargetManager::ILLUSTRADED_BG, rectRender))
        return;

    if (!rtm.ChangeRenderTarget(CRenderTargetManager::ILLUSTRADED_BG))
        return;

    if (!m_pModel) {
        rtm.ResetRenderTarget();
        return;
    }

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(34, 50, 47, 0), L"** IRTM RenderModel **");

    CPythonGraphic::Instance().ClearDepthBuffer();

    D3DLIGHT9 Light;
    Light.Type = D3DLIGHT_SPOT;
    Light.Position = {50.0f, 150.0f, 350.0f};
    Light.Direction = {-0.15f, -0.3f, -0.9f};
    Light.Theta = DirectX::XMConvertToRadians(30.0f);
    Light.Phi = DirectX::XMConvertToRadians(45.0f);
    Light.Falloff = 1.0f;
    Light.Attenuation0 = 0.0f;
    Light.Attenuation1 = 0.005f;
    Light.Attenuation2 = 0.0f;
    Light.Diffuse.r = 1.0f;
    Light.Diffuse.g = 1.0f;
    Light.Diffuse.b = 1.0f;
    Light.Diffuse.a = 1.0f;
    Light.Ambient.r = 1.0f;
    Light.Ambient.g = 1.0f;
    Light.Ambient.b = 1.0f;
    Light.Ambient.a = 1.0f;
    Light.Range = 500.0f;

    D3DMATERIAL9 Material;
    Material.Ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    Material.Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    Material.Emissive = {0.1f, 0.1f, 0.1f, 1.0f};

    auto* ms = GrannyState::instance().GetModelShader();

    float fFov = graphic.GetFOV();
    float fAspect = graphic.GetAspect();
    float fNearY = graphic.GetNear();
    float fFarY = graphic.GetFar();

    float fx = rectRender.left;
    float fy = rectRender.top;
    float fWidth = (rectRender.right - rectRender.left);
    float fHeight = (rectRender.bottom - rectRender.top);

    CCameraManager::instance().SetCurrentCamera(
        CCameraManager::ILLUSTRATION_CAMERA);
    CCamera* pCam = CCameraManager::instance().GetCurrentCamera();

    graphic.PushState();

    if (!pCam->IsDraging())
        pCam->SetViewParams(
            Vector3(0.0f, -(m_modelHeight * 8.899999618530273f + m_zoom), 0.0f),
            Vector3(0.0f, m_targetY, 0.0f), Vector3(0.0f, 0.0f, 1.0f));

    CPythonApplication::AppInst().TargetModelCamera();

    graphic.SetPerspective(10.0f, fWidth / fHeight, 100.0f, 15000.0f);

    D3DMATERIAL9 OldMat;
    STATEMANAGER.GetMaterial(&OldMat);

    if (ms) {
        Matrix transViewProj =
            XMMatrixMultiplyTranspose(ms_matView, ms_matProj);
        ms->SetTransformConstant(transViewProj);
        ms->SetMaterial(Material);
        ms->SetLightDirection(*(Vector3*)(&(Light.Direction)));
        ms->SetLightColor(Vector4(Light.Diffuse.r, Light.Diffuse.g,
                                  Light.Diffuse.b, Light.Diffuse.a));
        STATEMANAGER.SetMaterial(&Material);

        ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE);
        ms->SetAlphaBlendValue(1.0);
    }

    m_pModel->Render();
    m_pModel->RenderAttachingEffect();
    if(m_pModel->IsMountingHorse()) {
        m_pModel->GetHorseInfo().GetActorRef().RenderAttachingEffect();
    }
    if (ms) {
        STATEMANAGER.SetMaterial(&OldMat);
        ms->SetMaterial(OldMat);
    }

    CCameraManager::instance().ResetToPreviousCamera();
    graphic.PopState();

    graphic.SetPerspective(fFov, fAspect, fNearY, fFarY);
    if (ms) {
        Matrix transViewProj =
            XMMatrixMultiplyTranspose(ms_matView, ms_matProj);
        ms->SetTransformConstant(transViewProj);
    }
    rtm.ResetRenderTarget();
    // STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, isFogEnable);
    D3DPERF_EndEvent();
}

void CPythonItemRenderTargetManager::SetModelZoom(bool zoom)
{
    if (!m_pModel)
        return;

    if (zoom) {
        m_zoom -= m_fModelZoomSpeed;
        auto minRange = -(m_modelHeight * 8.9f - m_modelHeight * 3.0f);
        m_zoom = std::max<double>(minRange, m_zoom);
    } else {
        m_zoom = m_zoom + m_fModelZoomSpeed;
        auto minRange = 14000.0f - m_modelHeight * 8.9f;
        auto maxRange = m_modelHeight * 8.9f + m_modelHeight * 5.0;
        m_zoom = std::min<double>(m_zoom, minRange);
        m_zoom = std::min<double>(m_zoom, maxRange);
    }
}
