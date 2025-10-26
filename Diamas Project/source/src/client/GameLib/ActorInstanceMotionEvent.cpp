#include "StdAfx.h"
#include "../EffectLib/EffectManager.h"
#include "../milesLib/SoundManager.h"

#include "ActorInstance.h"
#include "FlyingObjectManager.h"
#include "FlyingInstance.h"
#include "GameEventManager.h"

#include "FlyHandler.h"
#include <game/MotionConstants.hpp>
#include <game/EffectConstants.hpp>

void CActorInstance::MotionEventProcess()
{
    if (!m_pkCurRaceMotionData)
        return;

    for (uint32_t i = 0; i < m_pkCurRaceMotionData->GetMotionEventDataCount(); ++i)
    {
        const CRaceMotionData::TMotionEventData *c_pData;
        if (!m_pkCurRaceMotionData->GetMotionEventDataPointer(i, &c_pData))
            continue;

        MotionEventProcess(m_kCurMotNode.dwcurFrame, i, c_pData);
    }
}

void CActorInstance::SoundEventProcess(bool bCheckFrequency)
{
    if (!m_pkCurRaceMotionData)
        return;

    auto* sounds = m_pkCurRaceMotionData->GetSoundInstanceVectorPointer();
    UpdateSoundInstance(m_kCurMotNode.dwcurFrame, *sounds,
                        m_x, m_y, m_z, bCheckFrequency);
}

void CActorInstance::MotionEventProcess(uint32_t dwcurFrame, int iIndex,
                                        const CRaceMotionData::TMotionEventData *c_pData)
{
    if (c_pData->dwFrame != dwcurFrame)
        return;

    switch (c_pData->iType)
    {
    case MOTION_EVENT_TYPE_EFFECT:
        ProcessMotionEventEffectEvent(c_pData);
        break;

    case MOTION_EVENT_TYPE_EFFECT_TO_TARGET:
        ProcessMotionEventEffectToTargetEvent(c_pData);
        break;

    case MOTION_EVENT_TYPE_SCREEN_WAVING:
        CGameEventManager::Instance().ProcessEventScreenWaving(
            this, (const CRaceMotionData::TScreenWavingEventData *)c_pData);
        break;

    case MOTION_EVENT_TYPE_SPECIAL_ATTACKING:
        ProcessMotionEventSpecialAttacking(iIndex, c_pData);
        break;

    case MOTION_EVENT_TYPE_SOUND:
        ProcessMotionEventSound(c_pData);
        break;

    case MOTION_EVENT_TYPE_FLY:
        ProcessMotionEventFly(c_pData);
        break;

    case MOTION_EVENT_TYPE_CHARACTER_SHOW:
        __ShowEvent();
        break;

    case MOTION_EVENT_TYPE_CHARACTER_HIDE:
        __HideEvent();
        break;

    case MOTION_EVENT_TYPE_RELATIVE_MOVE_ON:
        ProcessMotionEventRelativeMoveOn(c_pData);
        break;

    case MOTION_EVENT_TYPE_RELATIVE_MOVE_OFF:
        ProcessMotionEventRelativeMoveOff(c_pData);
        break;

    case MOTION_EVENT_TYPE_WARP:
#ifndef WORLD_EDITOR
        ProcessMotionEventWarp(c_pData);
#endif
        break;
    }
}

void CActorInstance::__ShowEvent()
{
    m_isHiding = FALSE;
    RestoreRenderMode();
    SetAlphaValue(1.0f);
}

void CActorInstance::__HideEvent()
{
    m_isHiding = TRUE;
    SetBlendRenderMode();
    SetAlphaValue(0.0f);
}

bool CActorInstance::__IsHiding()
{
    return m_isHiding;
}

void CActorInstance::ProcessMotionEventEffectEvent(const CRaceMotionData::TMotionEventData *c_pData)
{
    if (MOTION_EVENT_TYPE_EFFECT != c_pData->iType)
        return;

    const CRaceMotionData::TMotionEffectEventData *c_pEffectData = (const CRaceMotionData::TMotionEffectEventData *)
        c_pData;

    auto &mgr = CEffectManager::Instance();

    if (c_pEffectData->isIndependent)
    {
        auto index = mgr.CreateEffect(c_pEffectData->dwEffectIndex, 1, EFFECT_KIND_MOTION_EFFECT);

	Matrix matLocalPosition;
	matLocalPosition = Matrix::CreateTranslation(c_pEffectData->v3EffectPosition.x, c_pEffectData->v3EffectPosition.y, c_pEffectData->v3EffectPosition.z);

	Matrix matWorld;
	matWorld = matLocalPosition;
	matWorld *= m_worldMatrix;

        auto* effect = mgr.GetEffectInstance(index);
        if (effect)
        {
            effect->SetGlobalMatrix(matWorld);
            effect->SetAddColor(m_kCurMotNode.addColor);
        }

        return;
    }

    if (!c_pEffectData->isAttaching)
    {
        AttachEffectByID(0, NULL, c_pEffectData->dwEffectIndex,
                         &c_pEffectData->v3EffectPosition,
                         m_kCurMotNode.addColor, 1, EFFECT_KIND_MOTION_EFFECT);
        return;
    }

    if (c_pEffectData->isFollowing)
    {
        AttachEffectByID(0,
                         c_pEffectData->strAttachingBoneName.c_str(),
                         c_pEffectData->dwEffectIndex,
                         &c_pEffectData->v3EffectPosition,
                         m_kCurMotNode.addColor, 1, EFFECT_KIND_MOTION_EFFECT);
        return;
    }

    int iBoneIndex;
    uint32_t dwPartIndex = 0;
    if (FindBoneIndex(dwPartIndex, c_pEffectData->strAttachingBoneName.c_str(), &iBoneIndex))
    {
        Matrix *pBoneMat;
        GetBoneMatrix(dwPartIndex, iBoneIndex, &pBoneMat);

        Matrix matLocalPosition = Matrix::CreateTranslation(c_pEffectData->v3EffectPosition);

        /////////////////////////////////////////////////////////////////////
        Matrix matWorld;
        matWorld = *pBoneMat;
        matWorld *= matLocalPosition;
        matWorld *= m_worldMatrix;
        /////////////////////////////////////////////////////////////////////

        auto index = mgr.CreateEffect(c_pEffectData->dwEffectIndex, 1, EFFECT_KIND_MOTION_EFFECT);

        auto* effect = mgr.GetEffectInstance(index);
        if (effect)
        {
            effect->SetGlobalMatrix(matWorld);
            effect->SetAddColor(m_kCurMotNode.addColor);
        }
    }
}

void CActorInstance::ProcessMotionEventEffectToTargetEvent(const CRaceMotionData::TMotionEventData *c_pData)
{
    if (MOTION_EVENT_TYPE_EFFECT_TO_TARGET != c_pData->iType)
        return;

    const CRaceMotionData::TMotionEffectToTargetEventData *c_pEffectToTargetData = (const
        CRaceMotionData::TMotionEffectToTargetEventData *)c_pData;

    auto &mgr = CEffectManager::Instance();

    if (c_pEffectToTargetData->isFishingEffect)
    {
        if (m_iFishingEffectID)
            mgr.DestroyEffectInstance(m_iFishingEffectID);

        m_iFishingEffectID = mgr.CreateEffect(c_pEffectToTargetData->dwEffectIndex, 1,
                                              EFFECT_KIND_MOTION_EFFECT_TO_TARGET);

        Matrix matWorld = Matrix::CreateTranslation(c_pEffectToTargetData->v3EffectPosition);

        auto* effect = mgr.GetEffectInstance(m_iFishingEffectID);
        if (effect)
        {
            effect->SetGlobalMatrix(matWorld);
            effect->SetAddColor(m_kCurMotNode.addColor);
        }

        return;
    }

    if (!m_kFlyTarget.IsValidTarget())
        return;

    if (c_pEffectToTargetData->isFollowing && IsFlyTargetObject())
    {
        auto pTargetInstance = (CActorInstance *)m_kFlyTarget.GetFlyTarget();
        Vector3 v3Position(c_pEffectToTargetData->v3EffectPosition.x,
                               c_pEffectToTargetData->v3EffectPosition.y,
                               c_pEffectToTargetData->v3EffectPosition.z);
        pTargetInstance->AttachEffectByID(0, NULL,
                                          c_pEffectToTargetData->dwEffectIndex,
                                          &v3Position, m_kCurMotNode.addColor, 1, EFFECT_KIND_MOTION_EFFECT_TO_TARGET);
    }
    else
    {
        const Vector3 &c_rv3FlyTarget = m_kFlyTarget.GetFlyTargetPosition();
        Vector3 v3Position(c_rv3FlyTarget.x + c_pEffectToTargetData->v3EffectPosition.x,
                               c_rv3FlyTarget.y + c_pEffectToTargetData->v3EffectPosition.y,
                               c_rv3FlyTarget.z + c_pEffectToTargetData->v3EffectPosition.z);

        const auto index = mgr.CreateEffect(c_pEffectToTargetData->dwEffectIndex, 1,
                                            EFFECT_KIND_MOTION_EFFECT_TO_TARGET);

        Matrix matWorld = Matrix::CreateTranslation(v3Position);

        auto* effect = mgr.GetEffectInstance(index);
        if (effect)
        {
            effect->SetGlobalMatrix(matWorld);
            effect->SetAddColor(m_kCurMotNode.addColor);
        }
    }
}

void CActorInstance::ProcessMotionEventSpecialAttacking(int iMotionEventIndex,
                                                        const CRaceMotionData::TMotionEventData *c_pData)
{
    if (MOTION_EVENT_TYPE_SPECIAL_ATTACKING != c_pData->iType)
        return;

    const CRaceMotionData::TMotionAttackingEventData *c_pAttackingData = (const
        CRaceMotionData::TMotionAttackingEventData *)c_pData;

    float fRadian = DirectX::XMConvertToRadians(270.0f + 360.0f - GetRotation());
    m_kSplashArea.isEnableHitProcess = c_pAttackingData->isEnableHitProcess;
    m_kSplashArea.uSkill = m_kCurMotNode.uSkill;
    m_kSplashArea.MotionKey = m_kCurMotNode.dwMotionKey;
    m_kSplashArea.fDisappearingTime = GetLocalTime() + c_pAttackingData->fDurationTime;
    m_kSplashArea.c_pAttackingEvent = c_pAttackingData;
    m_kSplashArea.HittedInstanceMap.clear();

    m_kSplashArea.SphereInstanceVector.clear();
    m_kSplashArea.SphereInstanceVector.resize(c_pAttackingData->CollisionData.SphereDataVector.size());
    for (uint32_t i = 0; i < c_pAttackingData->CollisionData.SphereDataVector.size(); ++i)
    {
        const TSphereData &c_rSphereData = c_pAttackingData->CollisionData.SphereDataVector[i].GetAttribute();
        CDynamicSphereInstance &rSphereInstance = m_kSplashArea.SphereInstanceVector[i];

        auto fScale = GetScale().x;

        rSphereInstance.fRadius = c_rSphereData.fRadius * fScale;
        //rSphereInstance.v3Advance = Vector3(0.0f, 0.0f, 0.0f);

        rSphereInstance.v3Position.x = m_x + (c_rSphereData.v3Position.x * fScale) * sinf(fRadian) + c_rSphereData.
                                       v3Position.y * cosf(fRadian);
        rSphereInstance.v3Position.y = m_y + (c_rSphereData.v3Position.x * fScale) * cosf(fRadian) - c_rSphereData.
                                       v3Position.y * sinf(fRadian);
        rSphereInstance.v3Position.z = m_z + (c_rSphereData.v3Position.z * fScale);
        rSphereInstance.v3LastPosition = rSphereInstance.v3Position;
    }
}

void CActorInstance::ProcessMotionEventSound(const CRaceMotionData::TMotionEventData *c_pData)
{
    if (MOTION_EVENT_TYPE_SOUND != c_pData->iType)
        return;

    const CRaceMotionData::TMotionSoundEventData *c_pSoundData = (const CRaceMotionData::TMotionSoundEventData *)
        c_pData;

    SPDLOG_ERROR("PLAY SOUND: {0}", c_pSoundData->strSoundFileName.c_str());
    CSoundManager::Instance().PlaySound3D(m_x, m_y, m_z, c_pSoundData->strSoundFileName.c_str());
}

void CActorInstance::ProcessMotionEventFly(const CRaceMotionData::TMotionEventData *c_pData)
{
    if (MOTION_EVENT_TYPE_FLY != c_pData->iType)
    {
        return;
    }

    const CRaceMotionData::TMotionFlyEventData *c_pFlyData = (const CRaceMotionData::TMotionFlyEventData *)c_pData;

    if (m_kFlyTarget.IsValidTarget())
    {
        CFlyingManager &rfm = CFlyingManager::Instance();

        Vector3 v3Start(c_pFlyData->v3FlyPosition);
        v3Start += m_v3Position;

        if (c_pFlyData->isAttaching)
        {
            Matrix *pBoneMat;
            int iBoneIndex;
            uint32_t dwPartIndex = 0;

            if (FindBoneIndex(dwPartIndex, c_pFlyData->strAttachingBoneName.c_str(), &iBoneIndex))
                if (GetBoneMatrix(dwPartIndex, iBoneIndex, &pBoneMat))
                {
                    v3Start.x += pBoneMat->_41;
                    v3Start.y += pBoneMat->_42;
                    v3Start.z += pBoneMat->_43;
                }
        }

        CFlyingInstance *pInstance = nullptr;

        if (m_eRace != MAIN_RACE_ASSASSIN_M && m_eRace != MAIN_RACE_ASSASSIN_W)
        {
            pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
        }
        else
        {
            if (isNormalAttacking())
            {
                if (m_quiverEffectID)
                {
                    pInstance = rfm.CreateFlyingIndexedInstanceFlyTarget(m_quiverEffectID, v3Start, m_kFlyTarget, true);
                }
                else
                {
                    pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
                }
            }
            else if (__IsMountingHorse())
            {
                if (m_quiverEffectID)
                {
                    pInstance = rfm.CreateFlyingIndexedInstanceFlyTarget(m_quiverEffectID, v3Start, m_kFlyTarget, true);
                }
                else
                {
                    pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
                }
            }
            else
            {
                pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
            }
        }

        if (pInstance)
        {
            pInstance->SetEventHandler(m_pFlyEventHandler);
            pInstance->SetOwner(this);
            pInstance->SetSkillIndex(m_kCurMotNode.uSkill);
            pInstance->SetAddColor(m_kCurMotNode.addColor);
        }

        if (m_pFlyEventHandler)
        {
            m_pFlyEventHandler->OnShoot(m_kCurMotNode.dwMotionKey,
                                        m_kCurMotNode.uSkill);
        }
    }
    else
    {
        //TraceError("ActorInstance::ProcessMotionEventFly No Target");
    }
}

void CActorInstance::ProcessMotionEventWarp(const CRaceMotionData::TMotionEventData *c_pData)
{
    //if (CRaceMotionData::MOTION_EVENT_TYPE_WARP != c_pData->iType)
    //	return;

    // FIXME : TMotionWarpEventData·Î »¬ º¯¼ö - [levites]
    static const float sc_fDistanceFromTarget = 270.0f;

    if (m_kFlyTarget.IsValidTarget())
    {
        Vector3 v3MainPosition(m_x, m_y, m_z);
        const Vector3 &c_rv3TargetPosition = __GetFlyTargetPosition();

        Vector3 v3Distance = c_rv3TargetPosition - v3MainPosition;
        v3Distance.Normalize();

        TPixelPosition DestPixelPosition = c_rv3TargetPosition - (v3Distance * sc_fDistanceFromTarget);

        SetPixelPosition(DestPixelPosition);
        LookAt(c_rv3TargetPosition.x, c_rv3TargetPosition.y);

        __OnWarp();
    }
    else
    {
        //TraceError("ActorInstance::ProcessMotionEventFly No Target");
    }
}

void CActorInstance::ProcessMotionEventRelativeMoveOn(const CRaceMotionData::TMotionEventData *c_pData)
{
    if (MOTION_EVENT_TYPE_RELATIVE_MOVE_ON != c_pData->iType)
        return;

    if (!m_kFlyTarget.IsValidTarget())
    {
        SPDLOG_ERROR("ProcessMotionEventRelativeMoveOn without target");
        return;
    }
    const auto c_pRelativeMoveOnData = static_cast<const CRaceMotionData::TMotionRelativeMoveOnEventData *>(c_pData);

    // FIXME : TMotionWarpEventData로 뺄 변수 - [levites]
    static const float sc_fDistanceFromTarget = 270.0f;

    Vector3 v3MainPosition(m_x, m_y, m_z);
    const Vector3 &c_rv3TargetPosition = __GetFlyTargetPosition();

    const float distance = Vector3::Distance(c_rv3TargetPosition, v3MainPosition);

    m_moveFactor = (distance - sc_fDistanceFromTarget) / c_pRelativeMoveOnData->fVelocity;
}

void CActorInstance::ProcessMotionEventRelativeMoveOff(const CRaceMotionData::TMotionEventData *c_pData)
{
    m_moveFactor = 1.0f;
}
