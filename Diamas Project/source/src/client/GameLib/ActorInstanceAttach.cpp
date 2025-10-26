#include "../EffectLib/EffectManager.h"
#include "../EterGrnLib/Material.h"
#include "../EterGrnLib/ModelInstance.h"
#include "ActorInstance.h"
#include "ItemData.h"
#include "ItemManager.h"
#include "RaceData.h"
#include "StdAfx.h"
#include "WeaponTrace.h"
#include <game/Constants.hpp>
#include <game/EffectConstants.hpp>
#include <game/MotionConstants.hpp>

#include "../EterBase/StepTimer.h"
#include "../EterLib/CharacterEffectRegistry.hpp"
#include "../eterBase/Timer.h"

bool USE_WEAPON_SPECULAR = TRUE;

uint32_t CActorInstance::AttachSmokeEffect(uint32_t eSmoke)
{
    if (!m_pkCurRaceData)
        return 0;

    uint32_t dwSmokeEffectID = m_pkCurRaceData->GetSmokeEffectID(eSmoke);
    if (dwSmokeEffectID == 0)
        return 0;

    return AttachEffectByID(0, m_pkCurRaceData->GetSmokeBone().c_str(), dwSmokeEffectID, 0, 0, 1, EFFECT_KIND_SMOKE);
}

bool CActorInstance::__IsLeftHandWeapon(const uint32_t type) const
{
    switch (type)
    {
    case WEAPON_DAGGER:
    case WEAPON_CLAW:
    case WEAPON_BOW:
        return true;

    case WEAPON_FAN:
        return __IsMountingHorse();

    default:
        return false;
    }
}

bool CActorInstance::__IsRightHandWeapon(const uint32_t type) const
{
    if (type == WEAPON_BOW)
        return false;

    return true;
}

bool CActorInstance::__IsWeaponTrace(const uint32_t weaponType) const
{
    switch (weaponType)
    {
    case WEAPON_BELL:
    case WEAPON_FAN:
    case WEAPON_BOW:
        return false;
    default:
        return true;
    }
}

void CActorInstance::AttachWeapon(CItemData *itemData)
{
    if (!itemData)
    {
        RegisterModelThing(PART_WEAPON, NULL);
        SetModelInstance(PART_WEAPON, PART_WEAPON, 0);

        RegisterModelThing(PART_WEAPON_LEFT, NULL);
        SetModelInstance(PART_WEAPON_LEFT, PART_WEAPON_LEFT, 0);
        return;
    }

    __DestroyWeaponTrace();

    uint32_t weaponType = itemData->GetWeaponType();
    if (__IsRightHandWeapon(weaponType))
    {
        AttachWeapon(PART_WEAPON, itemData);
    }
    else
    {
        RegisterModelThing(PART_WEAPON, NULL);
        SetModelInstance(PART_WEAPON, PART_WEAPON, 0);
    }

    if (__IsLeftHandWeapon(weaponType))
    {
        AttachWeapon(PART_WEAPON_LEFT, itemData);
    }
    else
    {
        RegisterModelThing(PART_WEAPON_LEFT, NULL);
        SetModelInstance(PART_WEAPON_LEFT, PART_WEAPON_LEFT, 0);
    }
}

bool CActorInstance::GetAttachingBoneName(uint32_t dwPartIndex, const char **c_pszBoneName)
{
    return m_pkCurRaceData->GetAttachingBoneName(dwPartIndex, c_pszBoneName);
}

void CActorInstance::AttachWeapon(uint32_t part, CItemData *itemData)
{
    assert(itemData && "null pointer");

    const char *szBoneName;
    if (!GetAttachingBoneName(part, &szBoneName))
        return;

    int boneIndex;
    if (!FindBoneIndex(PART_MAIN, szBoneName, &boneIndex))
        return;

    RegisterModelThing(part, itemData->GetModelThing());

    if (!SetModelInstance(part, part, 0))
    {
        SPDLOG_DEBUG("Failed to instantiate weapon model {0} for {1}", itemData->GetModelThingPath(),
                     itemData->GetIndex());
        return;
    }

    AttachModelInstance(PART_MAIN, // attached-to model-instance
                        *this,     // attachment thing-instance
                        part,      // attachment model-instance
                        boneIndex);

    SMaterialData kMaterialData;
    kMaterialData.pImage = NULL;
    kMaterialData.isSpecularEnable = TRUE;
    kMaterialData.fSpecularPower = itemData->GetSpecularPowerf();
    kMaterialData.bSphereMapIndex = 1;
    SetMaterialData(part, nullptr, kMaterialData);

    // Weapon Trace
    if (__IsWeaponTrace(itemData->GetWeaponType()))
    {
        CWeaponTrace *pWeaponTrace = CWeaponTrace::New();
        pWeaponTrace->SetWeaponInstance(this, part, szBoneName);
        m_WeaponTraceVector.push_back(pWeaponTrace);
    }
}

void CActorInstance::AttachAcce(CItemData *itemData)
{
    if (!itemData)
    {
        RegisterModelThing(PART_ACCE, NULL);
        SetModelInstance(PART_ACCE, PART_ACCE, 0);
        return;
    }

    int boneIndex;
    if (!FindBoneIndex(PART_MAIN, "Bip01 Spine2", &boneIndex))
        return;

    RegisterModelThing(PART_ACCE, itemData->GetModelThing());

    SetModelInstance(PART_ACCE, PART_ACCE, 0);
    AttachModelInstance(PART_MAIN, // attached-to model-instance
                        *this,     // attachment thing-instance
                        PART_ACCE, // attachment model-instance
                        boneIndex);

    auto mi = m_modelInstances[PART_ACCE];
    if (!mi)
        return;

    if (itemData->HasScaleData())
    {
        uint32_t Race = GetRace();
        uint32_t Job = GetJobByRace(Race);
        uint32_t Sex = GetSexByRace(Race);

        Vector3 Scale = itemData->GetItemScale(Job, Sex);
        Vector3 ScalePos = itemData->GetItemScalePosition(Job, Sex);

        SetAttachmentScale(Scale);
    }
    else
    {
        SetAttachmentScale(1.0f, 1.0f, 1.0f);
    }

    SMaterialData kMaterialData;
    kMaterialData.pImage = NULL;
    kMaterialData.isSpecularEnable = TRUE;
    kMaterialData.fSpecularPower = itemData->GetSpecularPowerf();
    kMaterialData.bSphereMapIndex = 1;
    SetMaterialData(PART_ACCE, NULL, kMaterialData);
}

void CActorInstance::DettachEffect(uint32_t dwEID)
{
    for (auto i = m_AttachingEffectList.begin(); i != m_AttachingEffectList.end();)
    {
        if (i->dwEffectIndex == dwEID)
        {
            i = m_AttachingEffectList.erase(i);
            CEffectManager::Instance().DestroyEffectInstance(dwEID);
        }
        else
        {
            ++i;
        }
    }
}

uint32_t CActorInstance::AttachEffectByName(uint32_t dwParentPartIndex, const char *c_pszBoneName,
                                            const char *c_pszEffectName, uint32_t addColor, float scale,
                                            uint32_t effectKind)
{
    std::string str;
    uint32_t dwCRC;
    StringPath(c_pszEffectName, str);
    dwCRC = ComputeCrc32(0, str.c_str(), str.length());

    return AttachEffectByID(dwParentPartIndex, c_pszBoneName, dwCRC, nullptr, addColor, scale, effectKind);
}

uint32_t CActorInstance::AttachEffectByID(uint32_t dwParentPartIndex, const char *c_pszBoneName, uint32_t dwEffectID,
                                          Vector3 const *c_pv3Position, uint32_t addColor, float scale,
                                          uint32_t effectKind)
{
    TAttachingEffect ae;
    ae.dwEffectId = dwEffectID;
    ae.iLifeType = EFFECT_LIFE_INFINITE;
    ae.dwEndTime = 0;
    ae.dwModelIndex = dwParentPartIndex;
    ae.dwEffectIndex = CEffectManager::Instance().CreateEffect(dwEffectID, scale, effectKind);
    ae.isAttaching = true;

    const auto e = CEffectManager::Instance().GetEffectInstance(ae.dwEffectIndex);
    if (e)
    {
        e->SetAddColor(addColor);
        e->SetAlwaysRender(true);
    }

    if (c_pv3Position)
        ae.matTranslation = Matrix::CreateTranslation(*c_pv3Position);
    else
        ae.matTranslation = Matrix::Identity;

    if (c_pszBoneName)
    {
        if (0 == std::strcmp(c_pszBoneName, "PART_WEAPON"))
            GetAttachingBoneName(PART_WEAPON, &c_pszBoneName);
        else if (0 == std::strcmp(c_pszBoneName, "PART_WEAPON_LEFT"))
            GetAttachingBoneName(PART_WEAPON_LEFT, &c_pszBoneName);

        int iBoneIndex;
        if (!FindBoneIndex(dwParentPartIndex, c_pszBoneName, &iBoneIndex))
            ae.iBoneIndex = -1;
        else
            ae.iBoneIndex = iBoneIndex;
    }
    else
    {
        ae.iBoneIndex = -1;
    }

    m_AttachingEffectList.push_back(ae);
    return ae.dwEffectIndex;
}

void CActorInstance::RefreshCollisionAttachments()
{
    assert(m_pkCurRaceData);
    if (!m_pkCurRaceData)
    {
        SPDLOG_ERROR("m_pkCurRaceData = NULL");
        return;
    }

    m_BodyPointInstanceList.clear();
    m_DefendingPointInstanceList.clear();

    for (const auto &data : m_pkCurRaceData->GetAttachingData())
    {
        switch (data.dwType)
        {
        case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA: {
            const NRaceData::TCollisionData *c_pCollisionData = data.pCollisionData;

            if (NRaceData::COLLISION_TYPE_ATTACKING == c_pCollisionData->iCollisionType)
                continue;

            TCollisionPointInstance PointInstance;
            if (!CreateCollisionInstancePiece(PART_MAIN, &data, &PointInstance))
                continue;

            switch (c_pCollisionData->iCollisionType)
            {
            case NRaceData::COLLISION_TYPE_ATTACKING:
                SPDLOG_DEBUG("Obsolete attacking collision data encountered");
                break;
            case NRaceData::COLLISION_TYPE_DEFENDING:
                m_DefendingPointInstanceList.push_back(PointInstance);
                break;
            case NRaceData::COLLISION_TYPE_BODY:
                m_BodyPointInstanceList.push_back(PointInstance);
                break;
            }
        }
        }
    }
}

void CActorInstance::SetWeaponTraceTexture(const char *szTextureName)
{
    for (const auto &wt : m_WeaponTraceVector)
        wt->SetTexture(szTextureName);
}

void CActorInstance::UseTextureWeaponTrace()
{
    for (const auto &wt : m_WeaponTraceVector)
        wt->UseTexture();
}

void CActorInstance::UseAlphaWeaponTrace()
{
    for (const auto &wt : m_WeaponTraceVector)
        wt->UseAlpha();
}

void CActorInstance::UpdateAttachingInstances()
{
    auto &mgr = CEffectManager::Instance();

    uint64_t dwCurrentTime = DX::StepTimer::instance().GetTotalMillieSeconds();
    for (auto it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();)
    {
        if (EFFECT_LIFE_WITH_MOTION == it->iLifeType)
        {
            ++it;
            continue;
        }

        if ((EFFECT_LIFE_NORMAL == it->iLifeType && it->dwEndTime < dwCurrentTime) ||
            !mgr.IsAliveEffect(it->dwEffectIndex))
        {
            mgr.DestroyEffectInstance(it->dwEffectIndex);
            it = m_AttachingEffectList.erase(it);
            continue;
        }

        if (!it->isAttaching)
            continue;

        auto* effect = mgr.GetEffectInstance(it->dwEffectIndex);
        if (effect)
        {
            if (it->iBoneIndex == -1)
            {
                Matrix matTransform;
                matTransform = it->matTranslation;
                matTransform *= m_worldMatrix;
                effect->SetGlobalMatrix(matTransform);
            }
            else
            {
                Matrix *pBoneMat;
                if (GetBoneMatrix(it->dwModelIndex, it->iBoneIndex, &pBoneMat))
                {
                    Matrix matTransform;
                    matTransform = *pBoneMat;
                    matTransform *= it->matTranslation;
                    matTransform *= m_worldMatrix;
                    effect->SetGlobalMatrix(matTransform);
                }
                else
                {
                    SPDLOG_ERROR("GetBoneMatrix(modelIndex({0}), boneIndex({1}).NOT_FOUND_BONE", it->dwModelIndex,
                                 it->iBoneIndex);
                }
            }
        }

        ++it;
    }
}

void CActorInstance::RenderAttachingEffect()
{
    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
    {
        const auto instance = mgr.GetEffectInstance(effect.dwEffectIndex);
        if (instance)
            instance->Render();
    }
}

void CActorInstance::SetAlwaysRenderAttachingEffect()
{
    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
    {
        const auto instance = mgr.GetEffectInstance(effect.dwEffectIndex);
        if (instance)
            instance->SetAlwaysRender(true);
    }
}

bool CActorInstance::IsShowMinorAttachingEffect()
{
    auto shownCount = 0;

    const auto infoHit = gCharacterEffectRegistry->FindEffect(EFFECT_HIT);
    const auto infoSelect = gCharacterEffectRegistry->FindEffect(EFFECT_SELECT);
    const auto infoTarget = gCharacterEffectRegistry->FindEffect(EFFECT_TARGET);

    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
    {
        if (infoSelect)
        {
            if (effect.dwEffectId == infoSelect->id)
                continue;
        }

        if (infoHit)
        {
            if (effect.dwEffectId == infoHit->id)
                continue;
        }

        if (infoTarget)
        {
            if (effect.dwEffectId == infoTarget->id)
                continue;
        }

        const auto instance = mgr.GetEffectInstance(effect.dwEffectIndex);
        if (instance && instance->isShow())
            ++shownCount;
    }

    return shownCount > 0;
}

void CActorInstance::ShowAllAttachingEffect()
{
    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
    {
        const auto instance = mgr.GetEffectInstance(effect.dwEffectIndex);
        if (instance)
        {
            instance->DisableForceHide();
            instance->SetActive();
        }
    }
}

void CActorInstance::HideAllAttachingEffect()
{
    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
    {
        const auto instance = mgr.GetEffectInstance(effect.dwEffectIndex);
        if (instance)
        {
            instance->EnableForceHide();
            instance->SetDeactive();
        }
    }
}

void CActorInstance::HideEffect(uint32_t index)
{
    auto &mgr = CEffectManager::Instance();

    const auto instance = mgr.GetEffectInstance(index);
    if (instance)
    {
        instance->EnableForceHide();
        instance->SetDeactive();
    }
}

void CActorInstance::ShowEffect(uint32_t index)
{
    auto &mgr = CEffectManager::Instance();

    const auto instance = mgr.GetEffectInstance(index);
    if (instance)
    {
        instance->DisableForceHide();
        instance->SetActive();
    }
}

void CActorInstance::ShowMinorAttachingEffect()
{
    const auto infoHit = gCharacterEffectRegistry->FindEffect(EFFECT_HIT);
    const auto infoSelect = gCharacterEffectRegistry->FindEffect(EFFECT_SELECT);
    const auto infoTarget = gCharacterEffectRegistry->FindEffect(EFFECT_TARGET);

    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
    {
        if (infoSelect)
        {
            if (effect.dwEffectId == infoSelect->id)
                continue;
        }

        if (infoHit)
        {
            if (effect.dwEffectId == infoHit->id)
                continue;
        }

        if (infoTarget)
        {
            if (effect.dwEffectId == infoTarget->id)
                continue;
        }

        const auto instance = mgr.GetEffectInstance(effect.dwEffectIndex);
        if (instance)
        {
            instance->DisableForceHide();
            instance->Show();
        }
    }
}

void CActorInstance::HideMinorAttachingEffect()
{
    const auto infoHit = gCharacterEffectRegistry->FindEffect(EFFECT_HIT);
    const auto infoSelect = gCharacterEffectRegistry->FindEffect(EFFECT_SELECT);
    const auto infoTarget = gCharacterEffectRegistry->FindEffect(EFFECT_TARGET);

    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
    {
        if (infoSelect)
        {
            if (effect.dwEffectId == infoSelect->id)
                continue;
        }

        if (infoHit)
        {
            if (effect.dwEffectId == infoHit->id)
                continue;
        }

        if (infoTarget)
        {
            if (effect.dwEffectId == infoTarget->id)
                continue;
        }

        const auto instance = mgr.GetEffectInstance(effect.dwEffectIndex);
        if (instance)
        {
            instance->EnableForceHide();
            instance->Hide();
        }
    }
}

void CActorInstance::__ClearAttachingEffect()
{
    auto &mgr = CEffectManager::Instance();
    for (const auto &effect : m_AttachingEffectList)
        mgr.DestroyEffectInstance(effect.dwEffectIndex);

    m_AttachingEffectList.clear();
}
