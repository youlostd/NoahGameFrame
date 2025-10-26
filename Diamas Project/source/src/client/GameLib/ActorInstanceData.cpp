#include "StdAfx.h"
#include "ActorInstance.h"
#include "RaceManager.h"
#include "ItemManager.h"
#include "RaceData.h"

#include "../eterlib/ResourceManager.h"
#include "../EterGrnLib/Material.h"
#include "../EterLib/Engine.hpp"

uint32_t CActorInstance::GetVirtualID()
{
    return m_dwSelfVID;
}

void CActorInstance::SetVirtualID(uint32_t dwVID)
{
    m_dwSelfVID = dwVID;
}

void CActorInstance::UpdateAttribute()
{
    if (!m_pAttributeInstance)
        return;

    if (!m_bNeedUpdateCollision)
        return;

    m_bNeedUpdateCollision = false;

    const CStaticCollisionDataVector &c_rkVec_ColliData = m_pAttributeInstance->GetObjectPointer()->
        GetCollisionDataVector();
    UpdateCollisionData(&c_rkVec_ColliData);

    m_pAttributeInstance->RefreshObject(GetTransform());
    UpdateHeightInstance(m_pAttributeInstance);
}

void CActorInstance::__CreateAttributeInstance(CAttributeData::Ptr pData)
{
    m_pAttributeInstance = new CAttributeInstance();
    m_pAttributeInstance->Clear();
    m_pAttributeInstance->SetObjectPointer(pData);
}

uint32_t CActorInstance::GetRace()
{
    return m_eRace;
}

bool CActorInstance::SetRace(uint32_t eRace)
{
    m_eRace = eRace;
    m_pkCurRaceData = nullptr;

    const auto raceData = CRaceManager::Instance().GetRaceDataPointer(eRace);
    if (!raceData)
        return false;

    m_pkCurRaceData = raceData.value();

    // Motion id is the base race id
    CGraphicThingInstance::SetMotionID(m_pkCurRaceData->GetRaceIndex());

    auto pAttributeData = m_pkCurRaceData->GetAttributeDataPtr();
    if (pAttributeData)
        __CreateAttributeInstance(pAttributeData);

    // Setup Graphic ResourceData
    __ClearAttachingEffect();

    CGraphicThingInstance::Clear();

    // NOTE : PC만 Part별로 다 생성하게 해준다.
    if (IsPC() || IsBuffBot())
    {
        CGraphicThingInstance::ReserveModelThing(PART_MAX_NUM);
        CGraphicThingInstance::ReserveModelInstance(PART_MAX_NUM);
    }
    else
    {
        CGraphicThingInstance::ReserveModelThing(1);
        CGraphicThingInstance::ReserveModelInstance(1);
    }

    m_pkCurRaceData->LoadMotions();

    return true;
}

void CActorInstance::SetHair(uint32_t eHair)
{
    assert(m_pkCurRaceData && "");
    if (!m_pkCurRaceData)
        return;

    auto pkHair = m_pkCurRaceData->FindHair(eHair);
    if (!pkHair)
    {
        SPDLOG_ERROR("Failed to find hair {} for race {}",
                      eHair, m_eRace);
        return;
    }

    if (pkHair->m_stModelFileName.empty())
    {
        SPDLOG_ERROR("Hair {} for race {}: model path empty",
                      eHair, m_eRace);
        return;
    }

    auto &resMgr = CResourceManager::Instance();
    auto model = resMgr.LoadResource<CGraphicThing>(pkHair->m_stModelFileName);
    if (!model)
    {
        SPDLOG_ERROR("Hair {} for race {}: model {} is null",
                      eHair, m_eRace, pkHair->m_stModelFileName.c_str());
        return;
    }

    RegisterModelThing(PART_HAIR, model);
    SetModelInstance(PART_HAIR, PART_HAIR, 0, PART_MAIN);

    for (const auto &skin : pkHair->m_kVct_kSkin)
    {
        auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(skin.m_stDstFileName);
        if (!r)
        {
            SPDLOG_DEBUG("Hair {} for race {}: skin {} is null",
                          eHair, m_eRace, skin.m_stDstFileName.c_str());
            continue;
        }

        SetMaterialImagePointer(PART_HAIR, skin.m_stSrcFileName.c_str(), r);
    }
}

void CActorInstance::SetShape(uint32_t eShape, float fSpecular)
{
#ifdef WORLD_EDITOR
	if ((IsNPC() || IsPoly() || IsEnemy()))
	{
		fSpecular = 1.0;
	}
#else
    if ((IsNPC() || IsPoly() || IsEnemy()) && Engine::GetSettings().IsShowSpecular())
    {
        fSpecular = 1.0;
    }
#endif
    assert(m_pkCurRaceData && "");

    if (!m_pkCurRaceData)
        return;

    m_eShape = eShape;

    CRaceData::SShape *pkShape = m_pkCurRaceData->FindShape(eShape);
    if (pkShape)
    {
        auto &resMgr = CResourceManager::Instance();

        if (pkShape->m_stModelFileName.empty())
        {
            auto r = m_pkCurRaceData->GetBaseModelThing();
            if (!r)
            {
                SPDLOG_ERROR("Race {} has no base model", m_eRace);
                return;
            }

            RegisterModelThing(0, r);
        }
        else
        {
            auto r = resMgr.LoadResource<CGraphicThing>(pkShape->m_stModelFileName);
            if (!r)
            {
                SPDLOG_ERROR("Failed to load {} for shape {} / race {}",
                              pkShape->m_stModelFileName.c_str(), eShape, m_eRace);
                return;
            }

            RegisterModelThing(0, r);
        }

        SetModelInstance(0, 0, 0);

        if (fSpecular == 0.0f && pkShape->specular != 0.0f)
            fSpecular = pkShape->specular;

        for (const auto &skin : pkShape->m_kVct_kSkin)
        {
            auto r = resMgr.LoadResource<CGraphicImage>(skin.m_stDstFileName.c_str());
            if (!r)
            {
                SPDLOG_ERROR(
                    "Failed to load {} to replace {} for "
                    "shape {} / race {}",
                    skin.m_stDstFileName.c_str(), skin.m_stSrcFileName.c_str(),
                    eShape, m_eRace);
                continue;
            }

            if (fSpecular > 0.0f)
            {
                SMaterialData kMaterialData;
                kMaterialData.pImage = r;
                kMaterialData.isSpecularEnable = true;
                kMaterialData.fSpecularPower = fSpecular;
                kMaterialData.bSphereMapIndex = 0;
                SetMaterialData(skin.m_ePart, skin.m_stSrcFileName.c_str(), kMaterialData);
            }
            else
            {
                SetMaterialImagePointer(skin.m_ePart, skin.m_stSrcFileName.c_str(), r);
            }
        }

        for (const auto &data : pkShape->m_attachingData)
        {
            switch (data.dwType)
            {
            case NRaceData::ATTACHING_DATA_TYPE_EFFECT: {
                if (data.isAttaching)
                    AttachEffectByName(0, data.strAttachingBoneName.c_str(),
                                       data.pEffectData->strFileName.c_str());
                else
                    AttachEffectByName(0, nullptr,
                                       data.pEffectData->strFileName.c_str());
                break;
            }
            }
        }
    }
    else
    {
        if (m_pkCurRaceData && m_pkCurRaceData->IsTree())
        {
            __CreateTree(m_pkCurRaceData->GetTreeFileName());
            return;
        }

        auto model = m_pkCurRaceData->GetBaseModelThing();
        if (!model)
        {
            SPDLOG_ERROR("Failed to load base model for race {}", m_eRace);
            return;
        }

        RegisterModelThing(0, model);
        SetModelInstance(0, 0, 0);
    }

    for (const auto &data : m_pkCurRaceData->GetAttachingData())
    {
        switch (data.dwType)
        {
        case NRaceData::ATTACHING_DATA_TYPE_EFFECT: {
            if (data.isAttaching)
                AttachEffectByName(0, data.strAttachingBoneName.c_str(),
                                   data.pEffectData->strFileName.c_str());
            else
                AttachEffectByName(0, 0,
                                   data.pEffectData->strFileName.c_str());
            break;
        }
        }
    }

    RefreshCollisionAttachments();
}

void CActorInstance::ChangeMaterial(const char *c_szFileName)
{
    assert(m_pkCurRaceData && "");
    if (!m_pkCurRaceData)
        return;

    CRaceData::SShape *pkShape = m_pkCurRaceData->FindShape(m_eShape);
    if (!pkShape)
        return;

    const auto &skins = pkShape->m_kVct_kSkin;
    if (skins.empty())
        return;

    const auto &skin = skins[0];

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    if (!r)
    {
        SPDLOG_ERROR("Failed to load {} to replace {}",
                      c_szFileName, skin.m_stSrcFileName.c_str());
        return;
    }

    SetMaterialImagePointer(skin.m_ePart, skin.m_stSrcFileName.c_str(), r);
}

void CActorInstance::SetSpecularInfo(bool bEnable, int iPart, float fAlpha)
{
    assert(m_pkCurRaceData && "");
    if (!m_pkCurRaceData)
        return;

    CRaceData::SShape *pkShape = m_pkCurRaceData->FindShape(m_eShape);
    if (pkShape->m_kVct_kSkin.empty())
        return;

    std::string filename = pkShape->m_kVct_kSkin[0].m_stSrcFileName;
    StringPath(filename);

    CGraphicThingInstance::SetSpecularInfo(iPart, filename.c_str(), bEnable, fAlpha);
}

void CActorInstance::SetSpecularInfoForce(bool bEnable, int iPart, float fAlpha)
{
    CGraphicThingInstance::SetSpecularInfo(iPart, NULL, bEnable, fAlpha);
}
