#include "StdAfx.h"

#include "PythonApplication.h"
#include "PythonWikiModelViewManager.h"
#include "../eterLib/Camera.h"
#include "../eterLib/GrpRenderTargetTexture.h"
#include "../eterLib/RenderTargetManager.h"
#include "../eterPythonLib/PythonGraphic.h"
#include "../gameLib/ActorInstance.h"
#include "base/Remotery.h"

CPythonWikiModelView::CPythonWikiModelView(size_t addID)
    : m_pWeaponModel(NULL), m_pModelInstance(NULL), m_bShow(false), m_modelRotation(0.0f), m_modulID(addID),
      m_pyWindow(NULL)
{
    CCameraManager::instance().AddCamera(CCameraManager::CAMERA_MAX + addID);
}

CPythonWikiModelView::~CPythonWikiModelView()
{
    if (m_pModelInstance)
        delete m_pModelInstance;

    if (m_pWeaponModel)
        delete m_pWeaponModel;

    CCameraManager::instance().RemoveCamera(CCameraManager::CAMERA_MAX + m_modulID);
}

void CPythonWikiModelView::ClearInstance()
{
    if (m_pModelInstance)
    {
        delete m_pModelInstance;
        m_pModelInstance = NULL;
    }

    if (m_pWeaponModel)
    {
        delete m_pWeaponModel;
        m_pWeaponModel = NULL;
    }
}

void CPythonWikiModelView::RegisterWindow(UI::CWindow *hWnd)
{
    if (hWnd)
        m_pyWindow = (UI::CRenderTarget *)hWnd;
    else
        m_pyWindow = NULL;
}

void CPythonWikiModelView::SetModelHair(DWORD dwVnum, bool isItem)
{
    if (!m_pWeaponModel)
        return;

    if (!dwVnum)
        isItem = false;
    DWORD eHair;
    if (isItem)
    {
        m_dwHairNum = dwVnum;
        CItemData *pItemData;
        if (!CItemManager::Instance().GetItemDataPointer(dwVnum, &pItemData))
            return;

        eHair = pItemData->GetValue(3);
    }
    else
        eHair = dwVnum;
    auto pRaceData = CRaceManager::Instance().GetRaceDataPointer(m_pWeaponModel->GetMotionID()).value_or(nullptr);
    if (!pRaceData)
        return;

    CRaceData::SHair *pkHair = pRaceData->FindHair(eHair);
    if (pkHair)
    {
        if (!pkHair->m_stModelFileName.empty())
        {
            auto pkHairThing =
                CResourceManager::Instance().LoadResource<CGraphicThing>(pkHair->m_stModelFileName.c_str());
            m_pWeaponModel->RegisterModelThing(PART_HAIR, pkHairThing);
            m_pWeaponModel->SetModelInstance(PART_HAIR, PART_HAIR, 0, PART_MAIN);
        }
#ifdef ENABLE_HAIR_SPECULAR
        float fSpecularPower = 0.0f;

        switch (eHair)
        {
        case 451:
        case 452:
            fSpecularPower = 1.0f;
            break;
        case 702:
        case 703:
        case 481:
        case 482:
        case 483:
        case 484:
        case 723:
            fSpecularPower = 0.75f;
            break;
        case 706:
        case 707:
            fSpecularPower = 0.5f;
            break;
        default:
            break;
        }

        const std::vector<CRaceData::SSkin> &c_rkVct_kSkin = pkHair->m_kVct_kSkin;
        std::vector<CRaceData::SSkin>::const_iterator i;
        for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i)
        {
            const CRaceData::SSkin &c_rkSkinItem = *i;

            CResource *pkRes = CResourceManager::Instance().GetResourcePointer(c_rkSkinItem.m_stDstFileName.c_str());

            if (pkRes)
            {
                if (fSpecularPower > 0.0f)
                {
                    SMaterialData kMaterialData;
                    kMaterialData.pImage = static_cast<CGraphicImage *>(pkRes);
                    kMaterialData.isSpecularEnable = TRUE;
                    kMaterialData.fSpecularPower = fSpecularPower;
                    kMaterialData.bSphereMapIndex = 0;
                    m_pWeaponModel->SetMaterialData(CRaceData::PART_HAIR, c_rkSkinItem.m_stSrcFileName.c_str(),
                                                    kMaterialData);
                }
                else
                {
                    m_pWeaponModel->SetMaterialImagePointer(CRaceData::PART_HAIR, c_rkSkinItem.m_stSrcFileName.c_str(),
                                                            static_cast<CGraphicImage *>(pkRes));
                }
            }
        }
#else
        // m_pWeaponModel->SetMaterialImagePointer(PART_HAIR, c_rkSkinItem.m_stSrcFileName.c_str(),
        // static_cast<CGraphicImage*>(pkRes));
#endif

       // uint32_t dwMotionKey = pRaceData->GetMotionKey(MOTION_MODE_GENERAL, MOTION_WAIT).value_or(-1);

        //m_pWeaponModel->SetMotion(dwMotionKey, 0.0f, 0, 0.0f);
    }
}

void CPythonWikiModelView::SetModelForm(DWORD dwVnum, bool isItem)
{
    if (!m_pWeaponModel)
        return;

    DWORD eShape = dwVnum;
    float fSpecular = 0.0f;
    if (isItem)
    {
        CItemData *pItemData;
        if (!CItemManager::Instance().GetItemDataPointer(dwVnum, &pItemData))
            return;

        eShape = pItemData->GetValue(3);
        fSpecular = pItemData->GetSpecularPowerf();
    }
    auto pRaceData = CRaceManager::Instance().GetRaceDataPointer(m_pWeaponModel->GetMotionID()).value_or(nullptr);
    if (!pRaceData)
        return;

    CRaceData::SShape *pkShape = pRaceData->FindShape(eShape);
    if (pkShape)
    {
        CResourceManager &rkResMgr = CResourceManager::Instance();

        if (pkShape->m_stModelFileName.empty())
        {
            auto pModelThing = pRaceData->GetBaseModelThing();
            m_pWeaponModel->RegisterModelThing(0, pModelThing);
        }
        else
        {
            auto pModelThing = rkResMgr.LoadResource<CGraphicThing>(pkShape->m_stModelFileName.c_str());
            m_pWeaponModel->RegisterModelThing(0, pModelThing);
        }

        m_pWeaponModel->SetModelInstance(0, 0, 0);

        const std::vector<CRaceData::SSkin> &c_rkVct_kSkin = pkShape->m_kVct_kSkin;
        std::vector<CRaceData::SSkin>::const_iterator i;
        for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i)
        {
            const CRaceData::SSkin &c_rkSkinItem = *i;

            auto pkRes = CResourceManager::Instance().LoadResource<CGraphicImage>(c_rkSkinItem.m_stDstFileName.c_str());

            if (pkRes)
            {
                if (fSpecular > 0.0f)
                {
                    SMaterialData kMaterialData;
                    kMaterialData.pImage = pkRes;
                    kMaterialData.isSpecularEnable = TRUE;
                    kMaterialData.fSpecularPower = fSpecular;
                    kMaterialData.bSphereMapIndex = 0;
                    m_pWeaponModel->SetMaterialData(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(),
                                                    kMaterialData);
                }
                else
                {
                    m_pWeaponModel->SetMaterialImagePointer(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(),
                                                            pkRes);
                }
            }
        }
    }
    else
    {
        auto pModelThing = pRaceData->GetBaseModelThing();
        m_pWeaponModel->RegisterModelThing(0, pModelThing);
        m_pWeaponModel->SetModelInstance(0, 0, 0);
    }

    if (m_pWeaponModel->GetMotionID() < 20)
    {
        if (dwVnum != 94135)
            SetModelHair(m_dwHairNum);
        else
        {
            m_pWeaponModel->RegisterModelThing(PART_HAIR, NULL);
            m_pWeaponModel->SetModelInstance(PART_HAIR, PART_HAIR, 0, PART_MAIN);

            //uint32_t dwMotionKey = pRaceData->GetMotionKey(MOTION_MODE_GENERAL, MOTION_WAIT).value_or(-1);

            //m_pWeaponModel->SetMotion(dwMotionKey, 0.0f, 0, 0.0f);
        }
    }
    else
    {
        //uint32_t dwMotionKey = pRaceData->GetMotionKey(MOTION_MODE_GENERAL, MOTION_WAIT).value_or(-1);

        //m_pWeaponModel->SetMotion(dwMotionKey, 0.0f, 0, 0.0f);
    }
}

void CPythonWikiModelView::SetWeaponModel(DWORD dwVnum)
{
    CItemData* pItemData;
    if (!CItemManager::Instance().GetItemDataPointer(dwVnum, &pItemData))
        return;

    if (pItemData->GetType() != ITEM_WEAPON && pItemData->GetType() != ITEM_COSTUME ||
        pItemData->GetType() == ITEM_COSTUME && pItemData->GetSubType() != COSTUME_WEAPON)
        return;

    auto pItemModel = pItemData->GetDropModelThing();
    if (!pItemModel)
        return;

    if (m_pWeaponModel)
        delete m_pWeaponModel;

    if (m_pModelInstance)
    {
        delete m_pModelInstance;
        m_pModelInstance = NULL;
    }

    m_pWeaponModel = new CGraphicThingInstance;
    m_pWeaponModel->Clear();
    m_pWeaponModel->ReserveModelThing(1);
    m_pWeaponModel->ReserveModelInstance(1);
    m_pWeaponModel->RegisterModelThing(0, pItemModel);
    m_pWeaponModel->SetModelInstance(0, 0, 0);
    m_pWeaponModel->SetPosition(0, 0, 0);
    m_pWeaponModel->SetAlwaysRender(true);

    SMaterialData kMaterialData;
    kMaterialData.pImage = NULL;
    kMaterialData.isSpecularEnable = TRUE;
    kMaterialData.fSpecularPower = pItemData->GetSpecularPowerf();
    kMaterialData.bSphereMapIndex = 1;
    m_pWeaponModel->SetMaterialData(0, NULL, kMaterialData);

    CCameraManager::instance().SetCurrentCamera(CCameraManager::CAMERA_MAX + m_modulID);
    CCamera* pCam = CCameraManager::instance().GetCurrentCamera();

    pCam->SetTargetHeight(m_modelHeight / 2.0f);

    CCameraManager::instance().ResetToPreviousCamera();
}

void CPythonWikiModelView::SetModel(DWORD vnum)
{
    auto pRaceData = CRaceManager::Instance().GetRaceDataPointer(vnum);
    if (!pRaceData)
        return;

    pRaceData.value()->LoadMotions();

    if (m_pWeaponModel)
    {
        delete m_pWeaponModel;
        m_pWeaponModel = NULL;
    }

    m_dwHairNum = 0;
    float fSpecular = 0.0f;
    float fTargetHeight = 0.0f;
    BYTE reserveAmmount = 1;
    if (vnum < 20)
    {
        // fSpecular = 100.0f;
        reserveAmmount = PART_MAX_NUM;
        fTargetHeight = 110.0f;
    }
    m_pWeaponModel = new CGraphicThingInstance;
    m_pWeaponModel->Clear();
    m_pWeaponModel->ReserveModelThing(reserveAmmount);
    m_pWeaponModel->ReserveModelInstance(reserveAmmount);
    m_pWeaponModel->SetMotionID(vnum);

    m_pWeaponModel->SetPosition(0, 0, 0);
    m_pWeaponModel->SetAlwaysRender(true);

    float fScale = 1.0f;
    if (vnum < 20)
        fScale = 1.0f;
    else
    {
        const auto dwScale = NpcManager::Instance().GetScale(vnum);
        if (dwScale != 100)
            fScale = static_cast<float>(dwScale / 100.0f);
    }
    m_pWeaponModel->SetScale(fScale, fScale, fScale);
    m_pWeaponModel->SetRotation(0.0f);

    SetModelForm(0, false);
    m_pWeaponModel->Deform(); // otherwise the size would not be correct

    m_modelRotation = 0.0f;

    if (vnum >= 20)
    {
        auto fRaceHeight = CRaceManager::instance().GetRaceHeight(vnum).value_or(0.0f);
        if (fRaceHeight == 0.0f)
        {
            m_pWeaponModel->DeformNoSkin();
            fRaceHeight = m_pWeaponModel->GetHeight();
        }
        m_modelHeight = fRaceHeight;
    }
    m_zoom = 0.0f;
    m_targetY = 0.0f;
    m_fModelZoomSpeed = m_modelHeight * 8.9f / 140.0f;
    m_minZoom = m_modelHeight / 35.0f;

    CCameraManager::instance().SetCurrentCamera(CCameraManager::CAMERA_MAX + m_modulID);
    CCamera *pCam = CCameraManager::instance().GetCurrentCamera();
    pCam->SetTargetHeight(m_modelHeight / 2.0f);
    CCameraManager::instance().ResetToPreviousCamera();
}

void CPythonWikiModelView::Drag(int32_t x, int32_t y)
{
    if (!m_bShow)
        return;

    if (!m_pModelInstance && !m_pWeaponModel)
        return;

    m_baseRotation = fmod(m_baseRotation + (x / 50.0f), 360.0f);
}

void CPythonWikiModelView::SetModelZoom(bool zoom)
{
    if (!m_pModelInstance && !m_pWeaponModel)
        return;

    if (zoom)
    {
        m_zoom -= m_fModelZoomSpeed;
        auto minRange = -(m_modelHeight * 8.9f - m_modelHeight * 3.0f);
        m_zoom = std::max<double>(minRange, m_zoom);
    }
    else
    {
        m_zoom = m_zoom + m_fModelZoomSpeed;
        auto minRange = 14000.0f - m_modelHeight * 8.9f;
        auto maxRange = m_modelHeight * 8.9f + m_modelHeight * 5.0;
        m_zoom = std::min<double>(m_zoom, minRange);
        m_zoom = std::min<double>(m_zoom, maxRange);
    }
}

void CPythonWikiModelView::RenderModel()
{
    if (!m_bShow || m_pyWindow && !m_pyWindow->IsShow())
        return;

    auto &graphic = CPythonGraphic::Instance();
    auto &rtm = CRenderTargetManager::instance();

    RECT rectRender;
    if (!CRenderTargetManager::instance().GetWikiRenderTargetRect(m_modulID, rectRender))
        return;

    if (!CRenderTargetManager::instance().ChangeWikiRenderTarget(m_modulID))
        return;

    if (!m_pModelInstance && !m_pWeaponModel)
    {
        rtm.ResetRenderTarget();
        return;
    }

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(34, 50, 47, 0), L"** Wiki RenderModel **");

    rtm.ClearRenderTarget();
    graphic.ClearDepthBuffer();

    float fFov = graphic.GetFOV();
    float fAspect = graphic.GetAspect();
    float fNearY = graphic.GetNear();
    float fFarY = graphic.GetFar();

    float fx = rectRender.left;
    float fy = rectRender.top;
    float fWidth = (rectRender.right - rectRender.left);
    float fHeight = (rectRender.bottom - rectRender.top);

    CCameraManager::instance().SetCurrentCamera(CCameraManager::CAMERA_MAX + m_modulID);
    CCamera *pCam = CCameraManager::instance().GetCurrentCamera();
    graphic.PushState();

    if (!pCam->IsDraging())
        pCam->SetViewParams(Vector3(0.0f, -(m_modelHeight * 8.899999618530273f + m_zoom), 0.0f),
                            Vector3(0.0f, 0.0f, 95.0f), Vector3(0.0f, 0.0f, 1.0f));

    CPythonApplication::AppInst().TargetModelCamera();

    graphic.SetPerspective(10.0f, fWidth / fHeight, 100.0f, 15000.0f);

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

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        Matrix transViewProj = XMMatrixMultiplyTranspose(CGraphicBase::ms_matView, CGraphicBase::ms_matProj);
        ms->SetTransformConstant(transViewProj);
        ms->SetMaterial(Material);
        ms->SetLightDirection(*(Vector3 *)(&(Light.Direction)));
        ms->SetLightColor(Vector4(Light.Diffuse.r, Light.Diffuse.g, Light.Diffuse.b, Light.Diffuse.a));
        STATEMANAGER.SetMaterial(&Material);

        ms->SetPassIndex(MODEL_SHADER_PASS_DIFFUSE);
        ms->SetAlphaBlendValue(1.0);
    }

    if (m_pModelInstance)
    {
        m_pModelInstance->Render();
        m_pModelInstance->RenderAttachingEffect();

    }
    else if (m_pWeaponModel)
    {

        m_pWeaponModel->RenderWithOneTexture(true);

        m_pWeaponModel->BlendRenderWithOneTexture();
    }

    CCameraManager::instance().ResetToPreviousCamera();
    graphic.PopState();

    graphic.SetPerspective(fFov, fAspect, fNearY, fFarY);

    if (ms)
    {
        Matrix transViewProj = XMMatrixMultiplyTranspose(CGraphicBase::ms_matView, CGraphicBase::ms_matProj);
        ms->SetTransformConstant(transViewProj);
    }

    rtm.ResetRenderTarget();

    D3DPERF_EndEvent();
}

void CPythonWikiModelView::DeformModel()
{
    if (m_bShow && (!m_pyWindow || m_pyWindow->IsShow()))
    {
        if (m_pModelInstance)
            m_pModelInstance->Deform();
        else if (m_pWeaponModel)
            m_pWeaponModel->Deform();
    }
}

void CPythonWikiModelView::UpdateModel()
{
    if (!m_bShow || !m_pModelInstance && !m_pWeaponModel || m_pyWindow && !m_pyWindow->IsShow())
        return;

    CCameraManager::instance().SetCurrentCamera(CCameraManager::CAMERA_MAX + m_modulID);
    CCamera *pCamera = CCameraManager::instance().GetCurrentCamera();
    if (pCamera && pCamera->IsDraging())
    {
        m_modelRotation = 0.0f;
        float heightSave = pCamera->GetTargetHeight();
        pCamera->Update();
        pCamera->SetTargetHeight(heightSave);
    }
    else if (m_enableRotation)
    {
        if (m_modelRotation < 360.0f)
            m_modelRotation += (1.0f / 100.0f * m_rotSpeed);
        else
            m_modelRotation = 0.0f;
    }
    else
    {
        m_modelRotation = m_baseRotation;
    }

    if (m_pModelInstance)
    {
        m_pModelInstance->SetAlwaysRenderAttachingEffect();
        m_pModelInstance->SetRotation(m_modelRotation);
        m_pModelInstance->Transform();
        CActorInstance &rkModelActor = m_pModelInstance->GetGraphicThingInstanceRef();
        rkModelActor.RotationProcess();
    }
    else if (m_pWeaponModel)
    {
        m_pWeaponModel->Update();
        m_pWeaponModel->ChangeMotion(MakeMotionKey(MOTION_MODE_GENERAL, MOTION_WAIT));

        m_pWeaponModel->UpdateTime();
        m_pWeaponModel->Deform();

        m_pWeaponModel->SetRotation(m_modelRotation);
        m_pWeaponModel->Transform();
    }

    CCameraManager::instance().ResetToPreviousCamera();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPythonWikiModelViewManager::CPythonWikiModelViewManager() : m_bShow(false) {}

CPythonWikiModelViewManager::~CPythonWikiModelViewManager()
{
    for (auto it = m_vecModelView.begin(); it != m_vecModelView.end(); ++it)
        delete (*it);
}

CPythonWikiModelView *CPythonWikiModelViewManager::GetModule(DWORD moduleID)
{
    auto find = std::find_if(m_vecModelView.begin(), m_vecModelView.end(),
                             [moduleID](CPythonWikiModelView* model) { return model->GetID() == moduleID; });
    if (find != m_vecModelView.end())
        return *find;

    return nullptr;
}

void CPythonWikiModelViewManager::AddView(DWORD addID)
{
    auto newElem = new CPythonWikiModelView(addID);
    m_vecModelView.insert(std::lower_bound(m_vecModelView.begin(), m_vecModelView.end(), newElem,
                                           [](const CPythonWikiModelView *p1, const CPythonWikiModelView *p2) {
                                               if (p1->GetID() <= p2->GetID())
                                                   return true;
                                               return false;
                                           }),
                          newElem);
}

void CPythonWikiModelViewManager::RemoveView(DWORD addID)
{
    auto find = std::find_if(m_vecModelView.begin(), m_vecModelView.end(),
                             [addID](CPythonWikiModelView* model) { return model->GetID() == addID; });

    if (find != m_vecModelView.end())
    {
        delete (*find);
        m_vecModelView.erase(find);
    }
}

DWORD CPythonWikiModelViewManager::GetFreeID()
{
    // m_vecModelView must be sorted first (its done in AddView)
    DWORD ret = 0;
    for (auto it = m_vecModelView.begin(); it != m_vecModelView.end() && (*it)->GetID() == ret; ++it, ++ret)
        ;
    return ret;
}

void CPythonWikiModelViewManager::DeformModel()
{
    rmt_ScopedCPUSample(WikiModelDeform, 0);

    if (!m_bShow)
        return;

    for (auto it = m_vecModelView.begin(); it != m_vecModelView.end(); ++it)
        (*it)->DeformModel();
}

void CPythonWikiModelViewManager::RenderModel()
{
    if (!m_bShow)
        return;
        rmt_ScopedCPUSample(WTM_RenderModel, 0);

    for (auto it = m_vecModelView.begin(); it != m_vecModelView.end(); ++it)
        (*it)->RenderModel();
}

void CPythonWikiModelViewManager::ClearInstances()
{
    for (auto it = m_vecModelView.begin(); it != m_vecModelView.end(); ++it)
        (*it)->ClearInstance();
}

void CPythonWikiModelViewManager::UpdateModel()
{
    if (!m_bShow)
        return;

    for (auto it = m_vecModelView.begin(); it != m_vecModelView.end(); ++it)
        (*it)->UpdateModel();
}
