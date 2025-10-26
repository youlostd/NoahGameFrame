#include "StdAfx.h"

#include "PythonMyShopDecoManager.h"
#include "../eterLib/Camera.h"
#include "../eterLib/RenderTargetManager.h"
#include "../eterPythonLib/PythonGraphic.h"
#include "PythonApplication.h"
#include <game/MotionConstants.hpp>

CPythonMyShopDecoManager::CPythonMyShopDecoManager()
    : m_pModel(nullptr),
      m_bgImage(nullptr),
      m_modelRotation(0),
      m_isShow(false)
{
    m_mapInstances.clear();
}

CPythonMyShopDecoManager::~CPythonMyShopDecoManager()
{
    Destroy();
}

void CPythonMyShopDecoManager::__Initialize()
{
    m_pModel = nullptr;
    m_isShow = false;
    m_modelRotation = 0.0f;
    m_bgImage = nullptr;
}

void CPythonMyShopDecoManager::Destroy()
{
    m_isShow = false;
    m_modelRotation = 0.0f;
    m_pModel = nullptr;

    stl_wipe_second(m_mapInstances);
}

CInstanceBase *CPythonMyShopDecoManager::GetInstancePtr(uint32_t VirtualID)
{
    auto itor = m_mapInstances.find(VirtualID);

    if (m_mapInstances.end() == itor)
    {
        return nullptr;
    }

    return itor->second;
}

bool CPythonMyShopDecoManager::CreateModelInstance(uint32_t index)
{
    if (GetInstancePtr(index) != nullptr)
        return true;

    CInstanceBase::SCreateData kCreateData{};
    kCreateData.m_bType = 11;
    kCreateData.m_dwRace = index;
    kCreateData.m_scale = 100;

    auto pModel = new CInstanceBase;
    if (pModel->Create(kCreateData))
    {
        m_mapInstances.emplace(index, pModel);
        return true;
    }

    delete (pModel);

    return false;
}

void CPythonMyShopDecoManager::SelectModel(uint32_t index)
{
    if (!index)
    {
        m_pModel = nullptr;
        return;
    }

    if (!CreateModelInstance(index))
    {
        m_pModel = nullptr;
        return;
    }

    m_pModel = GetInstancePtr(index);

    if (!m_pModel)
        return;

    m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
    m_modelRotation = 0.0f;
    m_pModel->Refresh(MOTION_WAIT, true);
    m_pModel->SetLoopMotion(MOTION_WAIT);
    m_pModel->SetAlwaysRender(true);
    m_pModel->SetRotation(m_modelRotation);
    CCameraManager::instance().SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
    CCamera *pCam = CCameraManager::instance().GetCurrentCamera();
    pCam->SetTargetHeight(110.0);
    CCameraManager::instance().ResetToPreviousCamera();
}

bool CPythonMyShopDecoManager::CreateModelBackground(uint32_t width, uint32_t height)
{
    if (m_bgImage)
        return false;

    m_bgImage.reset(new CGraphicImageInstance);
    m_bgImage->SetImagePointer(
        CResourceManager::instance().LoadResource<CGraphicImage>("d:/ymir work/ui/game/myshop_deco/model_view_bg.sub"));
    m_bgImage->SetScale(width / 190.0f, height / 210.0f);

    return true;
}

void CPythonMyShopDecoManager::RenderBackground()
{
    if (!m_isShow)
        return;

    if (!m_bgImage)
        return;

    RECT rectRender;
    if (!CRenderTargetManager::instance().GetRenderTargetRect(CRenderTargetManager::MYSHOP_DECO, rectRender))
        return;

    if (!CRenderTargetManager::instance().ChangeRenderTarget(CRenderTargetManager::MYSHOP_DECO))
        return;

    CRenderTargetManager::instance().ClearRenderTarget();
    m_bgImage->Render();

    CRenderTargetManager::instance().ResetRenderTarget();
}

void CPythonMyShopDecoManager::Update()
{
    if (!m_isShow || !m_pModel)
        return;

    if (m_modelRotation < 360.0f)
        m_modelRotation += 1.0f;
    else
        m_modelRotation = 0.0f;

    m_pModel->SetRotation(m_modelRotation);
    m_pModel->Transform();
    m_pModel->GetGraphicThingInstanceRef().RotationProcess();
}

void CPythonMyShopDecoManager::Deform()
{
    if (!m_isShow)
        return;

    if (m_pModel)
        m_pModel->Deform();
}

void CPythonMyShopDecoManager::RenderModel()
{
    auto &graphic = CPythonGraphic::Instance();
    auto &rtm = CRenderTargetManager::instance();

    RECT rectRender;
    if (!rtm.GetRenderTargetRect(CRenderTargetManager::MYSHOP_DECO, rectRender))
        return;

    if (!rtm.ChangeRenderTarget(CRenderTargetManager::MYSHOP_DECO))
        return;

    if (!m_pModel)
    {
        rtm.ResetRenderTarget();
        return;
    }

    graphic.ClearDepthBuffer();
    //CPythonGraphic::Identity();
    CPythonGraphic::Instance().SetGameRenderState();

    float fFov = graphic.GetFOV();
    float fAspect = graphic.GetAspect();
    float fNearY = graphic.GetNear();
    float fFarY = graphic.GetFar();

    float fx = rectRender.left;
    float fy = rectRender.top;
    float fWidth = (rectRender.right - rectRender.left);
    float fHeight = (rectRender.bottom - rectRender.top);

    CCameraManager::instance().SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
    CCamera *pCam = CCameraManager::instance().GetCurrentCamera();

    graphic.PushState();

    pCam->SetViewParams(
        Vector3(0.0f, -1500.0f, 600.0f),
        Vector3(0.0f, 0.0f, 95.0f),
        Vector3(0.0f, 0.0f, 1.0f)
        );

    CPythonApplication::AppInst().TargetModelCamera();

    graphic.SetPerspective(15.0f, fWidth / fHeight, 150.0, 30000.0);

    m_pModel->Render();
    m_pModel->RenderAttachingEffect();

    CCameraManager::instance().ResetToPreviousCamera();
    graphic.PopState();

    graphic.SetPerspective(fFov, fAspect, fNearY, fFarY);

    rtm.ResetRenderTarget();
}
