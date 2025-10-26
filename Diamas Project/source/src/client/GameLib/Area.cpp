#include "StdAfx.h"

#include "../EffectLib/EffectManager.h"
#include "../EterBase/Timer.h"
#include "../EterLib/ResourceManager.h"
#include "../EterLib/StateManager.h"
#include "../SpeedTreeLib/SpeedTreeForestDirectX9.h"

#include "Area.h"

#include <StepTimer.h>

#include "Property.h"
#include "PropertyManager.h"

#include <boost/algorithm/string.hpp>
#include <pak/Vfs.hpp>
#include <unordered_set>

#include "MapOutdoor.h"

void CArea::SObjectData::InitializeRotation()
{
    m_fYaw = m_fPitch = m_fRoll = 0.0f;
}

CArea::SObjectInstance::~SObjectInstance()
{
    if (pTree)
        CSpeedTreeForestDirectX9::Instance().DeleteInstance(pTree);

    if (pThingInstance)
        CGraphicThingInstance::Delete(pThingInstance);

    if (effect)
        CEffectInstance::Delete(effect);
}

CArea::SObjectInstance::SObjectInstance(SObjectInstance &&other)
    : dwType(std::move(other.dwType)), map(other.map), pTree(std::move(other.pTree)),
      isShadowFlag(std::move(other.isShadowFlag)), pThingInstance(std::move(other.pThingInstance)),
      pAttributeInstance(std::move(other.pAttributeInstance)), effect(std::move(other.effect)),
      pAmbienceInstance(std::move(other.pAmbienceInstance)), pDungeonBlock(std::move(other.pDungeonBlock))
{
    other.dwType = PROPERTY_TYPE_NONE;
    other.pTree = nullptr;
    other.pThingInstance = nullptr;
    other.effect = nullptr;
}

CArea::SObjectInstance &CArea::SObjectInstance::operator=(SObjectInstance &&other)
{
    dwType = std::move(other.dwType);
    pTree = std::move(other.pTree);
    isShadowFlag = std::move(other.isShadowFlag);
    pThingInstance = std::move(other.pThingInstance);
    pAttributeInstance = std::move(other.pAttributeInstance);
    effect = std::move(other.effect);
    pAmbienceInstance = std::move(other.pAmbienceInstance);
    pDungeonBlock = std::move(other.pDungeonBlock);
    map = other.map;

    other.dwType = PROPERTY_TYPE_NONE;
    other.pTree = nullptr;
    other.pThingInstance = nullptr;
    other.effect = nullptr;

    return *this;
}

void CArea::__UpdateAniThingList()
{
    {
        CGraphicThingInstance *pkThingInst;

        TThingInstanceVector::iterator i = m_ThingCloneInstaceVector.begin();
        while (i != m_ThingCloneInstaceVector.end())
        {
            pkThingInst = *i++;
            if (pkThingInst->isShow())
            {
                pkThingInst->UpdateLODLevel();
            }
        }
    }

    {
        CGraphicThingInstance *pkThingInst;

        TThingInstanceVector::iterator i = m_AniThingCloneInstanceVector.begin();
        while (i != m_AniThingCloneInstanceVector.end())
        {
            pkThingInst = *i++;
            pkThingInst->Update();
        }
    }
}

void CArea::__UpdateEffectList()
{
    for (auto i = m_effectInstances.begin(); i != m_effectInstances.end();)
    {
        auto pEffectInstance = i->second;

        pEffectInstance->Update();

        if (pEffectInstance->isAlive())
        {
            ++i;
            continue;
        }

        i->first->effect = nullptr;

        i = m_effectInstances.erase(i);
        CEffectInstance::Delete(pEffectInstance);
    }
}

void CArea::Update()
{
    __UpdateAniThingList();
}

void CArea::UpdateAroundAmbience(float fX, float fY, float fZ)
{
    // Ambience
    TAmbienceInstanceVector::iterator i;
    for (i = m_AmbienceCloneInstanceVector.begin(); i != m_AmbienceCloneInstanceVector.end(); ++i)
    {
        TAmbienceInstance *pInstance = *i;
        pInstance->__Update(fX, fY, fZ);
    }
}

struct CArea_LessEffectInstancePtrRenderOrder
{
    bool operator()(CEffectInstance *pkLeft, CEffectInstance *pkRight)
    {
        return pkLeft->LessRenderOrder(pkRight);
    }
};

struct CArea_FEffectInstanceRender
{
    inline void operator()(CEffectInstance *pkEftInst)
    {
        pkEftInst->Render();
    }
};

void CArea::RenderEffect()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CArea::RenderEffect **");

    __UpdateEffectList();

    STATEMANAGER.SetTexture(0, NULL);
    STATEMANAGER.SetTexture(1, NULL);

    static std::vector<CEffectInstance *> effects;
    effects.reserve(m_effectInstances.size());
    effects.clear();

    for (auto &inst : m_effectInstances)
    {
        if (inst.second->isShow())
            effects.push_back(inst.second);
    }

    std::sort(std::execution::par, effects.begin(), effects.end(), CArea_LessEffectInstancePtrRenderOrder());
    std::for_each(effects.begin(), effects.end(), CArea_FEffectInstanceRender());

    D3DPERF_EndEvent();
}

void CArea::CollectRenderingObject(std::vector<CGraphicThingInstance *> &rkVct_pkOpaqueThingInst)
{
    for (auto &i : m_ThingCloneInstaceVector)
    {
        auto pkThingInst = i;
        if (pkThingInst->isShow())
        {
            if (!pkThingInst->HaveBlendThing())
                rkVct_pkOpaqueThingInst.push_back(i);
        }
    }
}

void CArea::CollectBlendRenderingObject(std::vector<CGraphicThingInstance *> &rkVct_pkBlendThingInst)
{
    TThingInstanceVector::iterator i;
    for (i = m_ThingCloneInstaceVector.begin(); i != m_ThingCloneInstaceVector.end(); ++i)
    {
        CGraphicThingInstance *pkThingInst = *i;
        if (pkThingInst->isShow())
        {
            if (pkThingInst->HaveBlendThing())
                rkVct_pkBlendThingInst.push_back(*i);
        }
    }
}

void CArea::Render()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 255, 0, 0), L"** CArea::Render **");

    {
        CGraphicThingInstance *pkThingInst;

        TThingInstanceVector::iterator i = m_AniThingCloneInstanceVector.begin();
        while (i != m_AniThingCloneInstanceVector.end())
        {
            pkThingInst = *i++;
            pkThingInst->Deform();
        }
    }

    CGraphicThingInstance *pkThingInst;

    TThingInstanceVector::iterator i = m_ThingCloneInstaceVector.begin();

    while (i != m_ThingCloneInstaceVector.end())
    {
        pkThingInst = *i++;
        pkThingInst->Render();
    }

    D3DPERF_EndEvent();
}

void CArea::RenderCollision()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 255, 0, 0), L"** Area RenderCollision **");
    uint32_t i;

    STATEMANAGER.SetTexture(0, NULL);
    STATEMANAGER.SetTexture(1, NULL);
    DWORD lighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);

    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, false);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xff000000);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    for (i = 0; i < GetObjectInstanceCount(); i++)
    {
        const TObjectInstance *po;
        if ((po = GetObjectInstancePointer(i)))
        {
            if (po->pTree && po->pTree->isShow())
            {
                uint32_t j;
                for (j = 0; j < po->pTree->GetCollisionInstanceCount(); j++)
                {
                    po->pTree->GetCollisionInstanceData(j)->Render();
        }
            }
        if (po->pThingInstance && po->pThingInstance->isShow())
        {
            uint32_t j;
            for (j = 0; j < po->pThingInstance->GetCollisionInstanceCount(); j++)
            {
                po->pThingInstance->GetCollisionInstanceData(j)->Render();
            }
        }
        if (po->pDungeonBlock && po->pDungeonBlock->isShow())
        {
            uint32_t j;
            for (j = 0; j < po->pDungeonBlock->GetCollisionInstanceCount(); j++)
            {
                po->pDungeonBlock->GetCollisionInstanceData(j)->Render();
            }
        }
    }
    }

    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

    D3DPERF_EndEvent();
}

void CArea::RenderAmbience()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 255, 0, 0), L"** RenderAmbience **");

    DWORD dwColorArg1, dwColorOp;
    STATEMANAGER.GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorArg1);
    STATEMANAGER.GetTextureStageState(0, D3DTSS_COLOROP, &dwColorOp);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    TAmbienceInstanceVector::iterator itor = m_AmbienceCloneInstanceVector.begin();
    for (; itor != m_AmbienceCloneInstanceVector.end(); ++itor)
    {
        TAmbienceInstance *pInstance = *itor;
        pInstance->Render();
    }
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, dwColorArg1);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, dwColorOp);
    D3DPERF_EndEvent();
}

void CArea::RenderDungeon()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 255, 0, 0), L"** Render Dungeon **");


    for (const auto &clone : m_DungeonBlockCloneInstanceVector)
        clone->Render();


    D3DPERF_EndEvent();
}

void CArea::RenderBoundingSpheres()
{
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, false);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, false);

    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xff000000);

    auto f = [](TObjectInstance& instance) {
        CGraphicObjectInstance* o = nullptr;

        if (instance.pTree)
            o = instance.pTree;
        else if (instance.pThingInstance)
            o = instance.pThingInstance;
        else if (instance.pDungeonBlock)
            o = instance.pDungeonBlock.get();
        else
            return;

        //if (!o->isShow())
        //	return;

        Vector3 center;
        float radius;
        o->GetBoundingSphere(center, radius);

        CScreen s;
        s.RenderSphere(NULL, center.x, center.y, center.z, radius, D3DFILL_WIREFRAME);
    };

    std::for_each(m_ObjectInstanceVector.begin(),
        m_ObjectInstanceVector.end(),
        f);

    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, true);
}

void CArea::Refresh()
{
    m_ThingCloneInstaceVector.clear();
    m_DungeonBlockCloneInstanceVector.clear();
    m_AniThingCloneInstanceVector.clear();
    m_ShadowThingCloneInstaceVector.clear();
    m_AmbienceCloneInstanceVector.clear();

    for (auto &pObjectInstance : m_ObjectInstanceVector)
    {
        if (PROPERTY_TYPE_TREE == pObjectInstance.dwType)
        {
            if (pObjectInstance.pTree)
            {
                pObjectInstance.pTree->UpdateBoundingSphere();
                pObjectInstance.pTree->UpdateCollisionData();
            }
        }
        else if (PROPERTY_TYPE_BUILDING == pObjectInstance.dwType)
        {
            pObjectInstance.pThingInstance->Update();
            pObjectInstance.pThingInstance->Transform();
            pObjectInstance.pThingInstance->Show();
            pObjectInstance.pThingInstance->Deform();
            m_ThingCloneInstaceVector.push_back(pObjectInstance.pThingInstance);

            pObjectInstance.pThingInstance->BuildBoundingSphere();
            pObjectInstance.pThingInstance->UpdateBoundingSphere();

            if (pObjectInstance.pThingInstance->IsMotionThing())
            {
                m_AniThingCloneInstanceVector.push_back(pObjectInstance.pThingInstance);
                pObjectInstance.pThingInstance->SetMotion(0);
            }

            if (pObjectInstance.isShadowFlag)
            {
                m_ShadowThingCloneInstaceVector.push_back(pObjectInstance.pThingInstance);
            }

            if (pObjectInstance.pAttributeInstance)
            {
                pObjectInstance.pThingInstance->UpdateCollisionData(
                    &pObjectInstance.pAttributeInstance->GetObjectPointer()->GetCollisionDataVector());
                pObjectInstance.pAttributeInstance->RefreshObject(pObjectInstance.pThingInstance->GetTransform());
                pObjectInstance.pThingInstance->UpdateHeightInstance(pObjectInstance.pAttributeInstance.get());
            }
        }
        else if (PROPERTY_TYPE_EFFECT == pObjectInstance.dwType)
        {
        }
        else if (PROPERTY_TYPE_AMBIENCE == pObjectInstance.dwType)
        {
            m_AmbienceCloneInstanceVector.push_back(pObjectInstance.pAmbienceInstance.get());
        }
        else if (PROPERTY_TYPE_DUNGEON_BLOCK == pObjectInstance.dwType)
        {
            pObjectInstance.pDungeonBlock->Update();
            pObjectInstance.pDungeonBlock->Transform();
            pObjectInstance.pDungeonBlock->Deform();

            pObjectInstance.pDungeonBlock->BuildBoundingSphere();
            pObjectInstance.pDungeonBlock->UpdateBoundingSphere();

            m_DungeonBlockCloneInstanceVector.push_back(pObjectInstance.pDungeonBlock.get());

            if (pObjectInstance.pAttributeInstance)
            {
                pObjectInstance.pDungeonBlock->UpdateCollisionData(
                    &pObjectInstance.pAttributeInstance->GetObjectPointer()->GetCollisionDataVector());
                pObjectInstance.pAttributeInstance->RefreshObject(pObjectInstance.pDungeonBlock->GetTransform());
                pObjectInstance.pDungeonBlock->UpdateHeightInstance(pObjectInstance.pAttributeInstance.get());
            }
        }
    }
}

void CArea::__Load_BuildObjectInstances()
{
    const auto count = m_ObjectDataVector.size();
    std::sort(m_ObjectDataVector.begin(), m_ObjectDataVector.end(), ObjectDataComp());

    m_ObjectInstanceVector.clear();
    m_ObjectInstanceVector.resize(count);

    for (std::size_t i = 0; i != count; ++i)
    {
        m_ObjectInstanceVector[i].map = m_pOwnerOutdoorMap;
        __SetObjectInstance(&m_ObjectInstanceVector[i], &m_ObjectDataVector[i]);
    }

    Refresh();
}

void CArea::__SetObjectInstance(TObjectInstance *pObjectInstance, const TObjectData *c_pData)
{
    CProperty *pProperty;
    if (!CPropertyManager::Instance().Get(c_pData->dwCRC, &pProperty))
        return;

    const char *c_szPropertyType;

    if (!pProperty->GetString("PropertyType", &c_szPropertyType))
        return;

    switch (GetPropertyType(c_szPropertyType))
    {
    case PROPERTY_TYPE_TREE:
        __SetObjectInstance_SetTree(pObjectInstance, c_pData, pProperty);
        break;

    case PROPERTY_TYPE_BUILDING:
        __SetObjectInstance_SetBuilding(pObjectInstance, c_pData, pProperty);
        break;

    case PROPERTY_TYPE_EFFECT:
        __SetObjectInstance_SetEffect(pObjectInstance, c_pData, pProperty);
        break;

    case PROPERTY_TYPE_AMBIENCE:
        __SetObjectInstance_SetAmbience(pObjectInstance, c_pData, pProperty);
        break;

    case PROPERTY_TYPE_DUNGEON_BLOCK:
        __SetObjectInstance_SetDungeonBlock(pObjectInstance, c_pData, pProperty);
        break;
    default:
        break;
    }
}

void CArea::__SetObjectInstance_SetEffect(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                          CProperty *pProperty)
{
    TPropertyEffect Data;
    if (!PropertyEffectStringToData(pProperty, &Data))
        return;

    const auto effectID = ComputeCrc32(0, Data.strFileName.c_str(), Data.strFileName.size());

    auto &rem = CEffectManager::Instance();

    CEffectData *data;
    if (!rem.GetEffectData(effectID, &data))
    {
        if (!rem.RegisterEffect(Data.strFileName.c_str()))
        {
            SPDLOG_ERROR("CArea::SetEffect effect register error {0}", Data.strFileName.c_str());
            return;
        }

        rem.GetEffectData(effectID, &data);
    }

    pObjectInstance->dwType = PROPERTY_TYPE_EFFECT;

    auto* pEffectInstance = CEffectInstance::New();
    pEffectInstance->SetEffectDataPointer(data);

    Matrix mat = Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(c_pData->m_fYaw), DirectX::XMConvertToRadians(c_pData->m_fPitch),
                                   DirectX::XMConvertToRadians(c_pData->m_fRoll));

    mat._41 = c_pData->Position.x;
    mat._42 = c_pData->Position.y;
    mat._43 = c_pData->Position.z + c_pData->m_fHeightBias;

    pEffectInstance->SetGlobalMatrix(mat);
    pObjectInstance->effect = pEffectInstance;
    m_effectInstances.emplace(pObjectInstance, pEffectInstance);
}

void CArea::__SetObjectInstance_SetTree(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                        CProperty *pProperty)
{
    const char *c_szTreeName;
    if (!pProperty->GetString("TreeFile", &c_szTreeName))
        return;

    Vector3 pos(c_pData->Position);
    pos.z += c_pData->m_fHeightBias;

    Quaternion rot = Quaternion::CreateFromYawPitchRoll(c_pData->m_fYaw, c_pData->m_fPitch, c_pData->m_fRoll);

    pObjectInstance->SetTree(pos, rot, c_pData->dwCRC, c_szTreeName);
}

void CArea::TObjectInstance::SetTree(const Vector3 &pos, const Quaternion &rot, uint32_t dwTreeCRC,
                                     const char *c_szTreeName)
{
    if (map)
    {
        auto &rkForest = CSpeedTreeForestDirectX9::Instance();
        pTree = rkForest.CreateInstance(pos.x, pos.y, pos.z, dwTreeCRC, c_szTreeName);
        if (!pTree)
            return;

        dwType = PROPERTY_TYPE_TREE;
    }
    else
    {
    }
}

void CArea::__SetObjectInstance_SetBuilding(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                            CProperty *pProperty)
{
    TPropertyBuilding Data;
    if (!PropertyBuildingStringToData(pProperty, &Data))
        return;

    CResourceManager &rkResMgr = CResourceManager::Instance();

    auto pThing = rkResMgr.LoadResource<CGraphicThing>(Data.strFileName);
    if (!pThing)
    {
        SPDLOG_ERROR("Property {0}: Failed to load {1} for area {2}/{3}", pProperty->GetFileName(), Data.strFileName,
                     m_wX, m_wY);
        return;
    }

    const int iModelCount = pThing->GetModelCount();
    const int iMotionCount = pThing->GetMotionCount();

    pObjectInstance->dwType = PROPERTY_TYPE_BUILDING;
    pObjectInstance->pThingInstance = CGraphicThingInstance::New();
    pObjectInstance->pThingInstance->Initialize();
    pObjectInstance->pThingInstance->ReserveModelThing(iModelCount);
    pObjectInstance->pThingInstance->ReserveModelInstance(iModelCount);
    pObjectInstance->pThingInstance->RegisterModelThing(0, pThing);
    for (int j = 0; j < PORTAL_ID_MAX_NUM; ++j)
        if (0 != c_pData->abyPortalID[j])
            pObjectInstance->pThingInstance->SetPortal(j, c_pData->abyPortalID[j]);

    for (int i = 0; i < iModelCount; ++i)
        pObjectInstance->pThingInstance->SetModelInstance(i, 0, i);

    if (iMotionCount)
    {
        CGraphicThingInstance::RegisterMotionThing(pObjectInstance->pThingInstance->GetMotionID(), 0, pThing);
    }

    pObjectInstance->pThingInstance->SetPosition(c_pData->Position.x, c_pData->Position.y,
                                                 c_pData->Position.z + c_pData->m_fHeightBias);
    pObjectInstance->pThingInstance->SetRotation(c_pData->m_fYaw, c_pData->m_fPitch, c_pData->m_fRoll);
    pObjectInstance->isShadowFlag = Data.isShadowFlag;
    pObjectInstance->pThingInstance->RegisterBoundingSphere();
    pObjectInstance->pThingInstance->Show();
    __LoadAttribute(pObjectInstance, Data.strAttributeDataFileName.c_str());
}

void CArea::__SetObjectInstance_SetAmbience(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                            CProperty *pProperty)
{
    pObjectInstance->pAmbienceInstance.reset(new TAmbienceInstance);
    if (!PropertyAmbienceStringToData(pProperty, &pObjectInstance->pAmbienceInstance->AmbienceData))
        return;

    pObjectInstance->dwType = PROPERTY_TYPE_AMBIENCE;

    auto pAmbienceInstance = pObjectInstance->pAmbienceInstance.get();
    pAmbienceInstance->fx = c_pData->Position.x;
    pAmbienceInstance->fy = c_pData->Position.y;
    pAmbienceInstance->fz = c_pData->Position.z + c_pData->m_fHeightBias;
    pAmbienceInstance->dwRange = c_pData->dwRange;
    pAmbienceInstance->fMaxVolumeAreaPercentage = c_pData->fMaxVolumeAreaPercentage;

    if ("ONCE" == pAmbienceInstance->AmbienceData.strPlayType)
    {
        pAmbienceInstance->Update = &TAmbienceInstance::UpdateOnceSound;
    }
    else if ("STEP" == pAmbienceInstance->AmbienceData.strPlayType)
    {
        pAmbienceInstance->Update = &TAmbienceInstance::UpdateStepSound;
    }
    else if ("LOOP" == pAmbienceInstance->AmbienceData.strPlayType)
    {
        pAmbienceInstance->Update = &TAmbienceInstance::UpdateLoopSound;
    }
}

void CArea::__SetObjectInstance_SetDungeonBlock(TObjectInstance *pObjectInstance, const TObjectData *c_pData,
                                                CProperty *pProperty)
{
    TPropertyDungeonBlock Data;
    if (!PropertyDungeonBlockStringToData(pProperty, &Data))
        return;

    pObjectInstance->dwType = PROPERTY_TYPE_DUNGEON_BLOCK;
    pObjectInstance->pDungeonBlock.reset(new CDungeonBlock());
    pObjectInstance->pDungeonBlock->Load(Data.strFileName.c_str());
    pObjectInstance->pDungeonBlock->SetPosition(c_pData->Position.x, c_pData->Position.y,
                                                c_pData->Position.z + c_pData->m_fHeightBias);
    pObjectInstance->pDungeonBlock->SetRotation(c_pData->m_fYaw, c_pData->m_fPitch, c_pData->m_fRoll);
    pObjectInstance->pDungeonBlock->Update();
    pObjectInstance->pDungeonBlock->BuildBoundingSphere();
    pObjectInstance->pDungeonBlock->RegisterBoundingSphere();
    for (int j = 0; j < PORTAL_ID_MAX_NUM; ++j)
        if (0 != c_pData->abyPortalID[j])
            pObjectInstance->pDungeonBlock->SetPortal(j, c_pData->abyPortalID[j]);
    __LoadAttribute(pObjectInstance, Data.strAttributeDataFileName.c_str());
}

void CArea::__LoadAttribute(TObjectInstance *pObjectInstance, const char *c_szAttributeFileName)
{
    CAttributeInstance *pAttrInstance = new CAttributeInstance;
    auto pAttributeData = CResourceManager::Instance().LoadResource<CAttributeData>(c_szAttributeFileName);

    if (!pAttributeData)
    {
        std::string attrFileName(c_szAttributeFileName);
        boost::algorithm::to_lower(attrFileName);
        const bool bIsDungeonObject = (std::string::npos != attrFileName.find("/dungeon/")) ||
                                      (std::string::npos != attrFileName.find("\\dungeon\\"));

        pAttributeData = new CAttributeData();

        // NOTE: dungeon 오브젝트는 Dummy Collision을 자동으로 생성하지 않도록 함 (던전의 경우 더미 컬리전때문에 문제가
        // 된 경우가 수차례 있었음. 이렇게 하기로 그래픽 팀과 협의 완료)
        if (!bIsDungeonObject)
        {
            if (NULL != pObjectInstance && NULL != pObjectInstance->pThingInstance)
            {
                CGraphicThingInstance *object = pObjectInstance->pThingInstance;
                Vector3 v3Min, v3Max;

                object->GetBoundingAABB(v3Min, v3Max);

                CStaticCollisionData collision;
                collision.dwType = COLLISION_TYPE_OBB;
                collision.quatRotation = Quaternion::CreateFromYawPitchRoll(object->GetYaw(), object->GetPitch(),
                                                   object->GetRoll());
                strcpy(collision.szName, "DummyCollisionOBB");
                collision.v3Position = (v3Min + v3Max) * 0.5f;

                Vector3 vDelta = (v3Max - v3Min);
                collision.fDimensions[0] = vDelta.x * 0.5f;
                collision.fDimensions[1] = vDelta.y * 0.5f;
                collision.fDimensions[2] = vDelta.z * 0.5f;

                pAttributeData->AddCollisionData(collision);
            }
        }
    }

    pAttrInstance->SetObjectPointer(pAttributeData);
    pObjectInstance->pAttributeInstance.reset(pAttrInstance);
}

bool CArea::Load(const char *c_szPathName)
{
    Clear();

    std::string strObjectDataFileName = c_szPathName + std::string("AreaData.txt");
    std::string strAmbienceDataFileName = c_szPathName + std::string("AreaAmbienceData.txt");

    __Load_LoadObject(strObjectDataFileName.c_str());
    __Load_LoadAmbience(strAmbienceDataFileName.c_str());
    __Load_BuildObjectInstances();

    return true;
}

bool CArea::__Load_LoadObject(const char *c_szFileName)
{
    CTokenVectorMap stTokenVectorMap;

    if (!LoadMultipleTextData(c_szFileName, stTokenVectorMap))
    {
        SPDLOG_ERROR(" CArea::Load File Load {} ERROR", c_szFileName);
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("areadatafile"))
    {
        SPDLOG_ERROR(" CArea::__LoadObject File Format {} ERROR 1", c_szFileName);
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("objectcount"))
    {
        SPDLOG_ERROR(" CArea::__LoadObject File Format {} ERROR 2", c_szFileName);
        return false;
    }

    const std::string &c_rstrCount = stTokenVectorMap["objectcount"][0];

    uint32_t dwCount = atoi(c_rstrCount.c_str());

    char szObjectName[32 + 1];

    for (uint32_t i = 0; i < dwCount; ++i)
    {
        _snprintf(szObjectName, sizeof(szObjectName), "object%03d", i);

        if (stTokenVectorMap.end() == stTokenVectorMap.find(szObjectName))
            continue;

        const CTokenVector &rVector = stTokenVectorMap[szObjectName];

        const auto &c_rstrxPosition = rVector[0];
        const auto &c_rstryPosition = rVector[1];
        const auto &c_rstrzPosition = rVector[2];
        const auto &c_rstrCRC = rVector[3];

        TObjectData ObjectData;
        ZeroMemory(&ObjectData, sizeof(ObjectData));
        storm::ParseNumber(c_rstrxPosition, ObjectData.Position.x);
        storm::ParseNumber(c_rstryPosition, ObjectData.Position.y);
        storm::ParseNumber(c_rstrzPosition, ObjectData.Position.z);
        storm::ParseNumber(c_rstrCRC, ObjectData.dwCRC);

        // 20041217.myevan.·ÎÅ×ÀÌ¼Ç °ø½Ä º¯°æ
        ObjectData.InitializeRotation(); // ObjectData.m_fYaw = ObjectData.m_fPitch = ObjectData.m_fRoll = 0;
        if (rVector.size() > 4)
        {
            std::string::size_type s = rVector[4].find('#');
            if (s != rVector[4].npos)
            {
                ObjectData.m_fYaw = atoi(rVector[4].substr(0, s - 1).c_str());
                int p = s + 1;
                s = rVector[4].find('#', p);
                ObjectData.m_fPitch = atoi(rVector[4].substr(p, s - 1 - p + 1).c_str());
                ObjectData.m_fRoll = atoi(rVector[4].substr(s + 1).c_str());
            }
            else
            {
                ObjectData.m_fYaw = 0.0f;
                ObjectData.m_fPitch = 0.0f;
                ObjectData.m_fRoll = atoi(rVector[4].c_str());
            }
        }

        ObjectData.m_fHeightBias = 0.0f;
        if (rVector.size() > 5)
        {
            storm::ParseNumber(rVector[5], ObjectData.m_fHeightBias);
        }

        if (rVector.size() > 6)
        {
            for (auto portalIdx = 0; portalIdx < std::min<int>(rVector.size() - 6, PORTAL_ID_MAX_NUM); ++portalIdx)
            {
                ObjectData.abyPortalID[portalIdx] = atoi(rVector[6 + portalIdx].c_str());
            }
        }

        // If data is not inside property, then delete it.
        CProperty *pProperty;
        if (!CPropertyManager::Instance().Get(ObjectData.dwCRC, &pProperty))
        {
            SPDLOG_ERROR("{0} Object{1}: Failed to find object with CRC {2}", c_szFileName, i, ObjectData.dwCRC);
            continue;
        }

        m_ObjectDataVector.push_back(ObjectData);
    }

    return true;
}

bool CArea::__Load_LoadAmbience(const char *c_szFileName)
{
    CTokenVectorMap stTokenVectorMap;

    if (!LoadMultipleTextData(c_szFileName, stTokenVectorMap))
    {
        SPDLOG_ERROR(" CArea::Load File Load {} ERROR", c_szFileName);
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("areaambiencedatafile"))
    {
        SPDLOG_ERROR(" CArea::__LoadAmbience File Format {} ERROR 1", c_szFileName);
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("objectcount"))
    {
        SPDLOG_ERROR(" CArea::__LoadAmbience File Format {} ERROR 2", c_szFileName);
        return false;
    }

    const std::string &c_rstrCount = stTokenVectorMap["objectcount"][0];

    uint32_t dwCount = atoi(c_rstrCount.c_str());

    char szObjectName[32 + 1];

    for (uint32_t i = 0; i < dwCount; ++i)
    {
        _snprintf(szObjectName, sizeof(szObjectName), "object%03d", i);

        if (stTokenVectorMap.end() == stTokenVectorMap.find(szObjectName))
            continue;

        const CTokenVector &rVector = stTokenVectorMap[szObjectName];

        const std::string &c_rstrxPosition = rVector[0];
        const std::string &c_rstryPosition = rVector[1];
        const std::string &c_rstrzPosition = rVector[2];
        const std::string &c_rstrCRC = rVector[3];
        const std::string &c_rstrRange = rVector[4];

        TObjectData ObjectData;
        ZeroMemory(&ObjectData, sizeof(ObjectData));
        storm::ParseNumber(c_rstrxPosition, ObjectData.Position.x);
        storm::ParseNumber(c_rstryPosition, ObjectData.Position.y);
        storm::ParseNumber(c_rstrzPosition, ObjectData.Position.z);
        storm::ParseNumber(c_rstrCRC, ObjectData.dwCRC);
        storm::ParseNumber(c_rstrRange, ObjectData.dwRange);

        // 20041217.myevan.·ÎÅ×ÀÌ¼Ç ÃÊ±âÈ­
        ObjectData.InitializeRotation();
        ObjectData.m_fHeightBias = 0.0f;
        ObjectData.fMaxVolumeAreaPercentage = 0.0f;

        if (rVector.size() >= 6)
        {
            const std::string &c_rstrPercentage = rVector[5];
            storm::ParseNumber(c_rstrPercentage, ObjectData.fMaxVolumeAreaPercentage);
        }

        // If data is not inside property, then delete it.
        CProperty *pProperty;
        if (!CPropertyManager::Instance().Get(ObjectData.dwCRC, &pProperty))
        {
            SPDLOG_ERROR("{0} Object{1}: Failed to find object with CRC {2}", c_szFileName, i, ObjectData.dwCRC);
            continue;
        }

        m_ObjectDataVector.push_back(ObjectData);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

bool CArea::CheckObjectIndex(uint32_t dwIndex) const
{
    if (dwIndex >= m_ObjectDataVector.size())
        return false;

    return true;
}

uint32_t CArea::GetObjectDataCount()
{
    return m_ObjectDataVector.size();
}

bool CArea::GetObjectDataPointer(uint32_t dwIndex, const TObjectData **ppObjectData) const
{
    if (!CheckObjectIndex(dwIndex))
    {
        assert(!"Setting Object Index is corrupted!");
        return false;
    }

    *ppObjectData = &m_ObjectDataVector[dwIndex];
    return true;
}

uint32_t CArea::GetObjectInstanceCount() const
{
    return m_ObjectInstanceVector.size();
}

const CArea::TObjectInstance *CArea::GetObjectInstancePointer(const uint32_t &dwIndex) const
{
    if (dwIndex >= m_ObjectInstanceVector.size())
        return nullptr;

    return &m_ObjectInstanceVector[dwIndex];
}

void CArea::EnablePortal(bool bFlag)
{
    if (m_bPortalEnable == bFlag)
        return;

    m_bPortalEnable = bFlag;
}

void CArea::ClearPortal()
{
    m_kSet_ShowingPortalID.clear();
}

void CArea::AddShowingPortalID(int iNum)
{
    m_kSet_ShowingPortalID.insert(iNum);
}

void CArea::RefreshPortal()
{
    static std::unordered_set<TObjectInstance *> kSet_ShowingObjectInstance;
    kSet_ShowingObjectInstance.clear();
    for (uint32_t i = 0; i < m_ObjectDataVector.size(); ++i)
    {
        TObjectData &rData = m_ObjectDataVector[i];
        TObjectInstance &pInstance = m_ObjectInstanceVector[i];

        for (unsigned char byPortalID : rData.abyPortalID)
        {
            if (0 == byPortalID)
                break;

            if (m_kSet_ShowingPortalID.end() == m_kSet_ShowingPortalID.find(byPortalID))
                continue;

            kSet_ShowingObjectInstance.insert(&pInstance);
            break;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    m_ThingCloneInstaceVector.clear();
    m_DungeonBlockCloneInstanceVector.clear();

    for (auto &pObjectInstance : m_ObjectInstanceVector)
    {
        if (m_bPortalEnable)
        {
            if (kSet_ShowingObjectInstance.end() == kSet_ShowingObjectInstance.find(&pObjectInstance))
                continue;
        }

        if (PROPERTY_TYPE_BUILDING == pObjectInstance.dwType)
        {
            assert(pObjectInstance.pThingInstance);
            m_ThingCloneInstaceVector.push_back(pObjectInstance.pThingInstance);
        }
        else if (PROPERTY_TYPE_DUNGEON_BLOCK == pObjectInstance.dwType)
        {
            assert(pObjectInstance.pDungeonBlock);
            m_DungeonBlockCloneInstanceVector.push_back(pObjectInstance.pDungeonBlock.get());
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void CArea::Clear()
{
    // Clones
    m_ThingCloneInstaceVector.clear();
    m_DungeonBlockCloneInstanceVector.clear();
    m_AniThingCloneInstanceVector.clear();
    m_ShadowThingCloneInstaceVector.clear();
    m_AmbienceCloneInstanceVector.clear();
    m_effectInstances.clear();

    // Real Instances
    m_ObjectDataVector.clear();
    m_ObjectInstanceVector.clear();

    m_bPortalEnable = false;
    ClearPortal();
}

//////////////////////////////////////////////////////////////////////////
// Coordination 관련
void CArea::GetCoordinate(unsigned short *usCoordX, unsigned short *usCoordY)
{
    *usCoordX = m_wX;
    *usCoordY = m_wY;
}

void CArea::SetCoordinate(const unsigned short &usCoordX, const unsigned short &usCoordY)
{
    m_wX = usCoordX;
    m_wY = usCoordY;
}

//////////////////////////////////////////////////////////////////////////

void CArea::SetMapOutDoor(CMapOutdoor *pOwnerOutdoorMap)
{
    m_pOwnerOutdoorMap = pOwnerOutdoorMap;
}

CArea::CArea()
{
    m_wX = m_wY = 0xFF;
}

CArea::~CArea()
{
    Clear();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CArea::TAmbienceInstance::__Update(float fxCenter, float fyCenter, float fzCenter)
{
    if (0 == dwRange)
        return;

    (this->*Update)(fxCenter, fyCenter, fzCenter);
}

void CArea::TAmbienceInstance::UpdateOnceSound(float fxCenter, float fyCenter, float fzCenter)
{
    float fDistance = sqrtf((fx - fxCenter) * (fx - fxCenter) + (fy - fyCenter) * (fy - fyCenter) +
                            (fz - fzCenter) * (fz - fzCenter));
    if (uint32_t(fDistance) < dwRange)
    {
        if (!sample)
        {
            if (AmbienceData.AmbienceSoundVector.empty())
                return;

            const char *c_szFileName = AmbienceData.AmbienceSoundVector[0].c_str();
            sample = CSoundManager::Instance().PlayAmbienceSound3D(fx, fy, fz, c_szFileName);
        }
    }
    else
    {
        // This stops the sound immediately, do we want that?
        sample.reset();
    }
}

void CArea::TAmbienceInstance::UpdateStepSound(float fxCenter, float fyCenter, float fzCenter)
{
    float fDistance = sqrtf((fx - fxCenter) * (fx - fxCenter) + (fy - fyCenter) * (fy - fyCenter) +
                            (fz - fzCenter) * (fz - fzCenter));
    if (uint32_t(fDistance) < dwRange)
    {
        float fcurTime = DX::StepTimer::instance().GetTotalSeconds();

        if (fcurTime > fNextPlayTime)
        {
            if (AmbienceData.AmbienceSoundVector.empty())
                return;

            const char *c_szFileName = AmbienceData.AmbienceSoundVector[0].c_str();
            sample = CSoundManager::Instance().PlayAmbienceSound3D(fx, fy, fz, c_szFileName);

            fNextPlayTime = DX::StepTimer::instance().GetTotalSeconds();
            fNextPlayTime += AmbienceData.fPlayInterval + Random::get(0.0f, AmbienceData.fPlayIntervalVariation);
        }
    }
    else
    {
        // This stops the sound immediately, do we want that?
        sample.reset();
        fNextPlayTime = 0.0f;
    }
}

void CArea::TAmbienceInstance::UpdateLoopSound(float fxCenter, float fyCenter, float fzCenter)
{
    float fDistance = sqrtf((fx - fxCenter) * (fx - fxCenter) + (fy - fyCenter) * (fy - fyCenter) +
                            (fz - fzCenter) * (fz - fzCenter));
    if (uint32_t(fDistance) < dwRange)
    {
        if (!sample)
        {
            if (AmbienceData.AmbienceSoundVector.empty())
                return;

            const char *c_szFileName = AmbienceData.AmbienceSoundVector[0].c_str();
            sample = CSoundManager::Instance().PlayAmbienceSound3D(fx, fy, fz, c_szFileName, 0);
        }

        if (sample)
            sample->SetVolume(__GetVolumeFromDistance(fDistance));
    }
    else
    {
        sample.reset();
    }
}

float CArea::TAmbienceInstance::__GetVolumeFromDistance(float fDistance)
{
    float fMaxVolumeAreaRadius = float(dwRange) * fMaxVolumeAreaPercentage;
    if (fMaxVolumeAreaRadius <= 0.0f)
        return 1.0f;
    if (fDistance <= fMaxVolumeAreaRadius)
        return 1.0f;

    return 1.0f - ((fDistance - fMaxVolumeAreaRadius) / (dwRange - fMaxVolumeAreaRadius));
}

void CArea::TAmbienceInstance::Render()
{
    float fBoxSize = 10.0f;
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xff00ff00);
    RenderCube(fx - fBoxSize, fy - fBoxSize, fz - fBoxSize, fx + fBoxSize, fy + fBoxSize, fz + fBoxSize);
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
    RenderSphere(NULL, fx, fy, fz, float(dwRange) * fMaxVolumeAreaPercentage, D3DFILL_POINT);
    RenderSphere(NULL, fx, fy, fz, float(dwRange), D3DFILL_POINT);
    RenderCircle2d(fx, fy, fz, float(dwRange) * fMaxVolumeAreaPercentage);
    RenderCircle2d(fx, fy, fz, float(dwRange));

    for (int i = 0; i < 4; ++i)
    {
        float fxAdd = cosf(float(i) * DirectX::XM_PI / 4.0f) * float(dwRange) / 2.0f;
        float fyAdd = sinf(float(i) * DirectX::XM_PI / 4.0f) * float(dwRange) / 2.0f;

        if (i % 2)
        {
            fxAdd /= 2.0f;
            fyAdd /= 2.0f;
        }

        RenderLine2d(fx + fxAdd, fy + fyAdd, fx - fxAdd, fy - fyAdd, fz);
    }
}

bool CArea::SAmbienceInstance::Picking()
{
    return CGraphicCollisionObject::IntersectSphere(Vector3(fx, fy, fz), dwRange);
}

CArea::SAmbienceInstance::SAmbienceInstance()
{
    fx = 0.0f;
    fy = 0.0f;
    fz = 0.0f;
    dwRange = 0;
    fNextPlayTime = 0.0f;
}
