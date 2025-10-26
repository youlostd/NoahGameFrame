#include "../effectLib/EffectManager.h"
#include "../milesLib/SoundManager.h"
#include "StdAfx.h"

#include "ActorInstance.h"
#include "RaceData.h"
#include <game/MotionConstants.hpp>

#include "../EterBase/StepTimer.h"
#include "../eterBase/Timer.h"
#include "RaceManager.h"

void CActorInstance::SetBattleHitEffect(uint32_t dwID)
{
    m_dwBattleHitEffectID = dwID;
}

void CActorInstance::SetBattleAttachEffect(uint32_t dwID)
{
    m_dwBattleAttachEffectID = dwID;
}

bool CActorInstance::CanAct()
{
    if (IsDead())
        return false;

    if (IsStun())
        return false;

    if (IsParalysis())
        return false;

    if (IsFaint())
        return false;

    if (IsSleep())
        return false;

    return true;
}

bool CActorInstance::CanUseSkill()
{
    if (!CanAct())
        return false;

    uint32_t dwCurMotionIndex = __GetCurrentMotionIndex();

    // Locked during attack
    switch (dwCurMotionIndex) {
        case MOTION_FISHING_THROW:
        case MOTION_FISHING_WAIT:
        case MOTION_FISHING_STOP:
        case MOTION_FISHING_REACT:
        case MOTION_FISHING_CATCH:
        case MOTION_FISHING_FAIL:
            return true;
            break;
    }

    // Locked during using skill
    if (IsUsingSkill()) {
        if (m_pkCurRaceMotionData->IsCancelEnableSkill())
            return true;

        return false;
    }

    return true;
}

bool CActorInstance::CanMove()
{
    if (!CanAct())
        return false;

    if (isLock())
        return false;

    return true;
}

bool CActorInstance::CanAttack()
{
    if (!CanAct())
        return false;

    if (IsUsingSkill()) {
        if (!CanCancelSkill())
            return false;
    }

    return true;
}

bool CActorInstance::CanFishing()
{
    if (!CanAct())
        return false;

    if (IsUsingSkill())
        return false;

    switch (__GetCurrentMotionIndex()) {
        case MOTION_WAIT:
        case MOTION_WALK:
        case MOTION_RUN:
            break;
        default:
            return false;
    }

    return true;
}

bool CActorInstance::IsClickableDistanceDestInstance(CActorInstance& rkInstDst,
                                                     float fDistance)
{
    TPixelPosition kPPosSrc;
    GetPixelPosition(&kPPosSrc);

    Vector3 kD3DVct3Src(kPPosSrc);

    TCollisionPointInstanceList& rkLstkDefPtInst =
        rkInstDst.m_DefendingPointInstanceList;
    TCollisionPointInstanceList::iterator i;

    for (i = rkLstkDefPtInst.begin(); i != rkLstkDefPtInst.end(); ++i) {
        CDynamicSphereInstanceVector& rkVctkDefSphere =
            (*i).SphereInstanceVector;

        CDynamicSphereInstanceVector::iterator j;
        for (j = rkVctkDefSphere.begin(); j != rkVctkDefSphere.end(); ++j) {
            CDynamicSphereInstance& rkSphere = (*j);

            float fMovDistance =
                Vector3::Distance(rkSphere.v3Position, kD3DVct3Src);
            float fAtkDistance = rkSphere.fRadius + fDistance;

            if (fAtkDistance > fMovDistance)
                return true;
        }
    }

    return false;
}

void CActorInstance::InputNormalAttackCommand(float fDirRot)
{
    if (!__CanInputNormalAttackCommand())
        return;

    m_fAtkDirRot = fDirRot;
    NormalAttack(m_fAtkDirRot);
}

bool CActorInstance::InputComboAttackCommand(float fDirRot)
{
    m_fAtkDirRot = fDirRot;

    if (m_isPreInput)
        return false;

    /////////////////////////////////////////////////////////////////////////////////

    // Process Input
    if (0 == m_dwcurComboIndex) {
        __RunNextCombo();
        return true;
    } else if (m_pkCurRaceMotionData->IsComboInputTimeData()) {
        // 동작 경과 시간
        float fElapsedTime = GetAttackingElapsedTime();

        // 이미 입력 한계 시간이 지났다면..
        if (fElapsedTime > m_pkCurRaceMotionData->GetComboInputEndTime()) {
            // SPDLOG_DEBUG("입력 한계 시간 지남");
            if (IsBowMode())
                m_isNextPreInput = true;
            return false;
        }

        if (fElapsedTime > m_pkCurRaceMotionData
                               ->GetNextComboTime()) // 콤보 발동 시간 이 후라면
        {
            // SPDLOG_DEBUG("다음 콤보 동작");
            // args : BlendingTime
            __RunNextCombo();
            return true;
        } else if (fElapsedTime >
                   m_pkCurRaceMotionData
                       ->GetComboInputStartTime()) // 선 입력 시간 범위 라면..
        {
            // SPDLOG_DEBUG("선 입력 설정");
            m_isPreInput = true;
            return false;
        }
    } else {
        float fElapsedTime = GetAttackingElapsedTime();
        if (fElapsedTime > m_pkCurRaceMotionData->GetMotionDuration() *
                               0.9f) // 콤보 발동 시간 이 후라면
        {
            // SPDLOG_DEBUG("다음 콤보 동작");
            // args : BlendingTime
            __RunNextCombo();
            return true;
        }
    }
    // Process Input

    return false;
}

void CActorInstance::ComboProcess()
{
    // If combo is on action
    if (0 != m_dwcurComboIndex) {
        if (!m_pkCurRaceMotionData) {
            SPDLOG_ERROR("Attacking motion data is NULL! : {0}",
                         m_dwcurComboIndex);
            __ClearCombo();
            return;
        }

        float fElapsedTime = GetAttackingElapsedTime();

        // Process PreInput
        if (m_isPreInput) {
            if (fElapsedTime > m_pkCurRaceMotionData->GetNextComboTime()) {
                __RunNextCombo();
                m_isPreInput = false;

                return;
            }
        }
    } else {
        m_isPreInput = FALSE;

        if (!IsUsingSkill()) // m_isNextPreInput´Â È°¸ðµå ÀÏ¶§¸¸ »ç¿ëÇÏ´Â º¯¼ö
            if (m_isNextPreInput) // È°ÀÏ¶§¸¸ ½ºÅ³ÀÌ Äµ½½ µÇ´Â°Ç ÀÌ°÷ ¶§¹®ÀÓ
            {
                __RunNextCombo();
                m_isNextPreInput = FALSE;
            }
    }
}

void CActorInstance::__RunNextCombo()
{
    ++m_dwcurComboIndex;
    ///////////////////////////

    uint16_t wComboIndex = m_dwcurComboIndex;
    uint16_t wComboType = __GetCurrentComboType();

    if (wComboIndex == 0) {
        SPDLOG_ERROR("CActorInstance::__RunNextCombo(wComboType={0}, "
                     "wComboIndex={1})",
                     wComboType, wComboIndex);
        return;
    }

    uint32_t dwComboArrayIndex = wComboIndex - 1;

    CRaceData::TComboData* pComboData;

    if (!m_pkCurRaceData->GetComboDataPointer(m_wcurMotionMode, wComboType,
                                              &pComboData)) {
        SPDLOG_ERROR("CActorInstance::__RunNextCombo(wComboType={0}, "
                     "wComboIndex={1}) - "
                     "m_pkCurRaceData->GetComboDataPointer(m_wcurMotionMode={2}"
                     ", &pComboData) == NULL",
                     wComboType, wComboIndex, m_wcurMotionMode);
        return;
    }

    if (dwComboArrayIndex >= pComboData->ComboIndexVector.size()) {
        SPDLOG_ERROR("CActorInstance::__RunNextCombo(wComboType={0}, "
                     "wComboIndex={1}) - (dwComboArrayIndex=%d) >= "
                     "(pComboData->ComboIndexVector.size()={2})",
                     wComboType, wComboIndex, dwComboArrayIndex,
                     pComboData->ComboIndexVector.size());
        return;
    }

    uint16_t wcurComboMotionIndex =
        pComboData->ComboIndexVector[dwComboArrayIndex];
    ComboAttack(wcurComboMotionIndex, m_fAtkDirRot, 0.1f);

    ////////////////////////////////
    // ÄÞº¸°¡ ³¡³µ´Ù¸é
    if (m_dwcurComboIndex == pComboData->ComboIndexVector.size()) {
        __OnEndCombo();
    }
}

void CActorInstance::__OnEndCombo()
{
    if (__IsMountingHorse()) {
        m_dwcurComboIndex = 1;
    }

    // Do not initialize the combo here.
    // The place where the combo is initialized is when the last combo is over
    // and the motion is automatically returned to the wait.
}

void CActorInstance::__ClearCombo()
{
    m_dwcurComboIndex = 0;
    m_isPreInput = FALSE;
    m_pkCurRaceMotionData = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CActorInstance::isAttacking()
{
    if (isNormalAttacking())
        return TRUE;

    if (isComboAttacking())
        return TRUE;

    if (IsSplashAttacking())
        return TRUE;

    return FALSE;
}

bool CActorInstance::isValidAttacking()
{
    if (!m_pkCurRaceMotionData)
        return FALSE;

    if (!m_pkCurRaceMotionData->isAttackingMotion())
        return FALSE;

    const NRaceData::TMotionAttackData* c_pData =
        m_pkCurRaceMotionData->GetMotionAttackDataPointer();
    float fElapsedTime = GetAttackingElapsedTime();
    NRaceData::THitDataContainer::const_iterator itor =
        c_pData->HitDataContainer.begin();
    for (; itor != c_pData->HitDataContainer.end(); ++itor) {
        const NRaceData::THitData& c_rHitData = *itor;
        if (fElapsedTime > c_rHitData.fAttackStartTime &&
            fElapsedTime < c_rHitData.fAttackEndTime)
            return TRUE;
    }

    return TRUE;
}

bool CActorInstance::CanCheckAttacking()
{
    if (isAttacking())
        return true;

    return false;
}

bool CActorInstance::__IsInSplashTime()
{
    if (m_kSplashArea.fDisappearingTime > GetLocalTime())
        return true;

    return false;
}

bool CActorInstance::isNormalAttacking()
{
    if (!m_pkCurRaceMotionData)
        return FALSE;

    if (!m_pkCurRaceMotionData->isAttackingMotion())
        return FALSE;

    const NRaceData::TMotionAttackData* c_pData =
        m_pkCurRaceMotionData->GetMotionAttackDataPointer();
    if (NRaceData::MOTION_TYPE_NORMAL != c_pData->iMotionType)
        return FALSE;

    return TRUE;
}

bool CActorInstance::isComboAttacking()
{
    if (!m_pkCurRaceMotionData)
        return FALSE;

    if (!m_pkCurRaceMotionData->isAttackingMotion())
        return FALSE;

    const NRaceData::TMotionAttackData* c_pData =
        m_pkCurRaceMotionData->GetMotionAttackDataPointer();
    if (NRaceData::MOTION_TYPE_COMBO != c_pData->iMotionType)
        return FALSE;

    return TRUE;
}

bool CActorInstance::IsSplashAttacking()
{
    if (!m_pkCurRaceMotionData)
        return FALSE;

    if (m_pkCurRaceMotionData->HasSplashMotionEvent())
        return TRUE;

    return FALSE;
}

bool CActorInstance::__IsMovingSkill(uint16_t wSkillNumber)
{
    enum {
        HORSE_DASH_SKILL_NUMBER = 137,
    };

    return HORSE_DASH_SKILL_NUMBER == wSkillNumber;
}

bool CActorInstance::IsActEmotion()
{
    uint32_t dwCurMotionIndex = __GetCurrentMotionIndex();
    switch (dwCurMotionIndex) {
        case MOTION_FRENCH_KISS_START + 0:
        case MOTION_FRENCH_KISS_START + 1:
        case MOTION_FRENCH_KISS_START + 2:
        case MOTION_FRENCH_KISS_START + 3:
        case MOTION_KISS_START + 0:
        case MOTION_KISS_START + 1:
        case MOTION_KISS_START + 2:
        case MOTION_KISS_START + 3:
            return TRUE;
    }

    return FALSE;
}

bool CActorInstance::IsUsingMovingSkill()
{
    return __IsMovingSkill(m_kCurMotNode.uSkill);
}

uint32_t CActorInstance::GetComboIndex()
{
    return m_dwcurComboIndex;
}

float CActorInstance::GetAttackingElapsedTime()
{
    return (GetLocalTime() - m_kCurMotNode.fStartTime) *
           m_kCurMotNode.fSpeedRatio;
    //	return (GetLocalTime() - m_kCurMotNode.fStartTime) * __GetAttackSpeed();
}

bool CActorInstance::__CanInputNormalAttackCommand()
{
    if (IsWaiting())
        return true;

    if (isNormalAttacking()) {
        float fElapsedTime = GetAttackingElapsedTime();

        if (fElapsedTime > m_pkCurRaceMotionData->GetMotionDuration() * 0.9f)
            return true;
    }

    return false;
}

bool CActorInstance::NormalAttack(float fDirRot, float fBlendTime)
{
    uint16_t wMotionIndex;
    if (!m_pkCurRaceData->GetNormalAttackIndex(m_wcurMotionMode, &wMotionIndex))
        return FALSE;

    BlendRotation(fDirRot, fBlendTime);
    SetAdvancingRotation(fDirRot);
    InterceptOnceMotion(wMotionIndex, 0.1f, 0, __GetAttackSpeed());

    __OnAttack(wMotionIndex);

    NEW_SetAtkPixelPosition(NEW_GetCurPixelPositionRef());

    return TRUE;
}

bool CActorInstance::ComboAttack(uint32_t dwMotionIndex, float fDirRot,
                                 float fBlendTime)
{
    BlendRotation(fDirRot, fBlendTime);
    SetAdvancingRotation(fDirRot);

    InterceptOnceMotion(dwMotionIndex, fBlendTime, 0, __GetAttackSpeed());

    __OnAttack(dwMotionIndex);

    NEW_SetAtkPixelPosition(NEW_GetCurPixelPositionRef());

    return TRUE;
}

void CActorInstance::__ProcessMotionEventAttackSuccess(uint32_t dwMotionKey,
                                                       uint8_t byEventIndex,
                                                       CActorInstance& rVictim)
{
    CRaceMotionData* pMotionData;

    if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey, &pMotionData))
        return;

    if (byEventIndex >= pMotionData->GetMotionEventDataCount())
        return;

    const CRaceMotionData::TMotionAttackingEventData* pMotionEventData;
    if (!pMotionData->GetMotionAttackingEventDataPointer(byEventIndex,
                                                         &pMotionEventData))
        return;

    const Vector3& c_rv3VictimPos = rVictim.GetPositionVectorRef();
    __ProcessDataAttackSuccess(dwMotionKey, pMotionEventData->AttackData,
                               rVictim, c_rv3VictimPos);
}

void CActorInstance::__ProcessMotionAttackSuccess(uint32_t dwMotionKey,
                                                  CActorInstance& rVictim)
{
    CRaceMotionData* c_pMotionData;

    if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey, &c_pMotionData))
        return;

    const Vector3& c_rv3VictimPos = rVictim.GetPositionVectorRef();
    __ProcessDataAttackSuccess(dwMotionKey,
                               c_pMotionData->GetMotionAttackDataReference(),
                               rVictim, c_rv3VictimPos);
}

uint32_t CActorInstance::__GetOwnerVID() const
{
    return m_dwOwnerVID;
}

#ifdef ENHANCED_FLY_FIX
float CActorInstance::__GetOwnerTime() const
{
    return GetLocalTime() - m_fOwnerBaseTime;
}
#endif

bool CActorInstance::__CanPushDestActor(CActorInstance& rkActorDst)
{
    if (rkActorDst.IsBuilding() || rkActorDst.IsDoor() ||
        rkActorDst.IsStone() || rkActorDst.IsNPC() ||
        rkActorDst.HasFlag(kActorFlagCannotPush))
        return false;

    if (CRaceManager::instance().IsHugeRace(rkActorDst.GetRace()))
        return false;

    if (rkActorDst.IsStun())
        return true;

    if (rkActorDst.__GetOwnerVID() != GetVirtualID())
        return false;

#ifdef ENHANCED_FLY_FIX
    if (rkActorDst.__GetOwnerTime() > 3.0f)
        return false;
#endif

    return true;
}

bool IS_PARTY_HUNTING_RACE(unsigned int vnum)
{
    return true;

}
#ifdef ENABLED_SLIDING_SYNC_FOR_JOB_INSTANCE
struct TargetRaceNum {
    float WARRIOR;
    float ASSASSIN;
    float SURA;
    float SHAMAN;

    TargetRaceNum(float warrior, float assassin, float sura, float shaman)
        : WARRIOR(warrior)
        , ASSASSIN(assassin)
        , SURA(sura)
        , SHAMAN(shaman)
    {
    }
};

std::unordered_map<uint32_t, TargetRaceNum> race_slipping_rate_map{
    //      Saldıran                           Saldırılan
    {MAIN_RACE_WARRIOR_M, TargetRaceNum(0.5f, 1.0f, 1.0f, 1.0f)},
    {MAIN_RACE_WARRIOR_W, TargetRaceNum(0.5f, 1.0f, 1.0f, 1.0f)},
    {MAIN_RACE_SHAMAN_M, TargetRaceNum(1.0f, 1.0f, 1.0f, 1.0f)},
    {MAIN_RACE_SHAMAN_W, TargetRaceNum(1.0f, 1.0f, 1.0f, 1.0f)},
    {MAIN_RACE_ASSASSIN_M, TargetRaceNum(1.0f, 1.0f, 1.0f, 1.0f)},
    {MAIN_RACE_ASSASSIN_W, TargetRaceNum(1.0f, 1.0f, 1.0f, 1.0f)},
    {MAIN_RACE_SURA_M, TargetRaceNum(1.0f, 1.0f, 1.0f, 1.0f)},
    {MAIN_RACE_SURA_W, TargetRaceNum(1.0f, 1.0f, 1.0f, 1.0f)},
};
#endif

void CActorInstance::__ProcessDataAttackSuccess(
    uint32_t motionKey, const NRaceData::TAttackData& c_rAttackData,
    CActorInstance& rVictim, const Vector3& c_rv3Position, UINT uiSkill,
    bool isSendPacket)
{
    if (NRaceData::HIT_TYPE_NONE == c_rAttackData.iHittingType)
        return;

    InsertDelay(c_rAttackData.fStiffenTime);

    Vector3 victimPos = rVictim.GetPosition();

    // Only do this for skills, normal hits behave are a bit odd and it's
    // probably better without
    if (uiSkill)
        rVictim.GetBlendingPosition(&victimPos);

    Vector3 pushDest(victimPos);

if (__CanPushDestActor(rVictim) && c_rAttackData.fExternalForce > 0.0f) {
        __PushCircle(rVictim);
#ifdef ENABLED_SLIDING_SYNC_FOR_JOB_INSTANCE
        const Vector3& kVictimPos = rVictim.GetPosition();
        auto externalForceRatio = 1.0f;
        
    if (auto it = race_slipping_rate_map.find(GetRace()); it != race_slipping_rate_map.end())
      {
            switch (rVictim.GetRace())
            {
              case MAIN_RACE_WARRIOR_M:
              case MAIN_RACE_WARRIOR_W:
                   externalForceRatio = it ->second.WARRIOR;
                break;
    
              case MAIN_RACE_ASSASSIN_M:
              case MAIN_RACE_ASSASSIN_W:
                   externalForceRatio = it->second.ASSASSIN;
                break;

              case MAIN_RACE_SHAMAN_M:
              case MAIN_RACE_SHAMAN_W:
                   externalForceRatio = it->second.SHAMAN;
                  break;

              case MAIN_RACE_SURA_M:
              case MAIN_RACE_SURA_W:
                   externalForceRatio = it->second.SURA;
                  break;
              default: break;
            }
          
      }
        rVictim.m_PhysicsObject.IncreaseExternalForce(
            kVictimPos, c_rAttackData.fExternalForce * externalForceRatio);

#else
        if (__CanPushDestActor(rVictim) &&
            c_rAttackData.fExternalForce > 0.0f) {
            __PushCircle(rVictim);

            // VICTIM_COLLISION_TEST
            const Vector3& kVictimPos = rVictim.GetPosition();
            rVictim.m_PhysicsObject.IncreaseExternalForce(
                kVictimPos,
                c_rAttackData.fExternalForce); //*nForceRatio/100.0f);
            rVictim.m_PhysicsObject.GetLastPosition(&pushDest);
            // VICTIM_COLLISION_TEST_END
            pushDest += victimPos;
        }
#endif

    }

    // Invisible Time
    if (IS_PARTY_HUNTING_RACE(rVictim.GetRace())) {
        if (uiSkill) // ÆÄÆ¼ »ç³É ¸ó½ºÅÍ¶óµµ ½ºÅ³ÀÌ¸é ¹«Àû½Ã°£ Àû¿ë
            rVictim.m_fInvisibleTime =
                DX::StepTimer::Instance().GetTotalSeconds() +
                c_rAttackData.fInvisibleTime;

        if (m_isMain)
            // #0000794: [M2KR] Æú¸®¸ðÇÁ - ¹ë·±½Ì ¹®Á¦ Å¸ÀÎ °ø°Ý¿¡ ÀÇÇÑ ¹«Àû
            // Å¸ÀÓÀº °í·ÁÇÏÁö ¾Ê°í ÀÚ½Å °ø°Ý¿¡ ÀÇÇÑ°Í¸¸ Ã¼Å©ÇÑ´Ù
            rVictim.m_fInvisibleTime =
                DX::StepTimer::Instance().GetTotalSeconds() +
                c_rAttackData.fInvisibleTime;
    } else // ÆÄÆ¼ »ç³É ¸ó½ºÅÍ°¡ ¾Æ´Ò °æ¿ì¸¸ Àû¿ë
    {
        rVictim.m_fInvisibleTime = DX::StepTimer::Instance().GetTotalSeconds() +
                                   c_rAttackData.fInvisibleTime;
    }

    // Stiffen Time
    rVictim.InsertDelay(c_rAttackData.fStiffenTime);

    // Hit Effect
    Vector3 vec3Effect(rVictim.m_x, rVictim.m_y, rVictim.m_z);

    // #0000780: [M2KR] 수룡 타격구 문제
    if (CRaceManager::instance().IsHugeRace(rVictim.GetRace()))
        vec3Effect = c_rv3Position;

    const Vector3& v3Pos = GetPosition();

    float roll = std::atan2(-vec3Effect.x + v3Pos.x, +vec3Effect.y - v3Pos.y);

    // 2004.08.03.myevan.In the case of a building or door, the impact effect is not visible 
    if (rVictim.IsBuilding() || rVictim.IsDoor()) {
        Vector3 vec3Delta = vec3Effect - v3Pos;
        vec3Delta.Normalize();
        vec3Delta *= 30.0f;
        vec3Delta += v3Pos;

        if (m_dwBattleHitEffectID) {
            auto& mgr = CEffectManager::Instance();
            auto index =
                mgr.CreateEffect(m_dwBattleHitEffectID, 1, EFFECT_KIND_BATTLE);

            Matrix mat = Matrix::CreateTranslation(vec3Delta);

            auto* effect = mgr.GetEffectInstance(index);
            if (effect)
                effect->SetGlobalMatrix(mat);
        }
    } else {
        if (m_dwBattleHitEffectID) {
            auto& mgr = CEffectManager::Instance();
            auto index = mgr.CreateEffect(m_dwBattleHitEffectID);

            Matrix mat = Matrix::CreateRotationZ(XMConvertToRadians(roll));
            mat._41 = vec3Effect.x;
            mat._42 = vec3Effect.y;
            mat._43 = vec3Effect.z;

            auto* effect = mgr.GetEffectInstance(index);
            if (effect)
                effect->SetGlobalMatrix(mat);
        }

        if (m_dwBattleAttachEffectID)
            rVictim.AttachEffectByID(0, NULL, m_dwBattleAttachEffectID, nullptr,
                                     0, 1, EFFECT_KIND_BATTLE);
    }

    if (rVictim.IsBuilding()) {
        // 2004.08.03.ºôµùÀÇ °æ¿ì Èçµé¸®¸é ÀÌ»óÇÏ´Ù
    } else if (rVictim.IsStone() || rVictim.IsDoor()) {
        __HitStone(rVictim);
    } else {
        ///////////
        // Motion
        if (NRaceData::HIT_TYPE_GOOD == c_rAttackData.iHittingType ||
            rVictim.IsResistFallen()) {
            __HitGood(rVictim);
        } else if (NRaceData::HIT_TYPE_GREAT == c_rAttackData.iHittingType) {
            __HitGreate(rVictim);
        } else {
            SPDLOG_ERROR("Unknown AttackingData.iHittingType {0}",
                         c_rAttackData.iHittingType);
        }
    }

    __OnHit(motionKey, uiSkill, rVictim, pushDest, isSendPacket);
}

void CActorInstance::OnShootDamage()
{
    if (IsStun()) {
        Die();
    } else {
        __Shake(100);

        if (!isLock() && !__IsKnockDownMotion() && !__IsStandUpMotion()) {
            if (InterceptOnceMotion(MOTION_DAMAGE))
                PushLoopMotion(MOTION_WAIT);
        }
    }
}

void CActorInstance::__Shake(uint32_t dwDuration)
{
    uint32_t dwCurTime = ELTimer_GetMSec();
    m_dwShakeTime = dwCurTime + dwDuration;
}

void CActorInstance::ShakeProcess()
{
    if (m_dwShakeTime) {
        Vector3 v3Pos(0.0f, 0.0f, 0.0f);

        uint32_t dwCurTime = ELTimer_GetMSec();

        if (m_dwShakeTime < dwCurTime) {
            m_dwShakeTime = 0;
        } else {
            int nShakeSize = 10;

            switch (rand() % 2) {
                case 0:
                    v3Pos.x += rand() % nShakeSize;
                    break;
                case 1:
                    v3Pos.x -= rand() % nShakeSize;
                    break;
            }

            switch (rand() % 2) {
                case 0:
                    v3Pos.y += rand() % nShakeSize;
                    break;
                case 1:
                    v3Pos.y -= rand() % nShakeSize;
                    break;
            }

            switch (rand() % 2) {
                case 0:
                    v3Pos.z += rand() % nShakeSize;
                    break;
                case 1:
                    v3Pos.z -= rand() % nShakeSize;
                    break;
            }
        }

        m_worldMatrix._41 += v3Pos.x;
        m_worldMatrix._42 += v3Pos.y;
        m_worldMatrix._43 += v3Pos.z;
    }
}

void CActorInstance::__HitStone(CActorInstance& rVictim)
{
    if (rVictim.IsStun()) {
        rVictim.Die();
    } else {
        rVictim.__Shake(100);
    }
}

void CActorInstance::__HitGood(CActorInstance& rVictim)
{
    if (rVictim.IsKnockDown())
        return;

    if (rVictim.IsStun()) {
        rVictim.Die();
    } else {
        rVictim.__Shake(100);

        if (!rVictim.isLock()) {
            float fRotRad = DirectX::XMConvertToRadians(GetRotation());
            float fVictimRotRad =
                DirectX::XMConvertToRadians(rVictim.GetRotation());

            Vector2 v2Normal(sin(fRotRad), cos(fRotRad));
            Vector2 v2VictimNormal(sin(fVictimRotRad), cos(fVictimRotRad));

            v2Normal.Normalize();
            v2VictimNormal.Normalize();

            float fScalar = v2Normal.Dot(v2VictimNormal);

            if (fScalar < 0.0f) {
                if (rVictim.InterceptOnceMotion(MOTION_DAMAGE))
                    rVictim.PushLoopMotion(MOTION_WAIT);
            } else {
                if (rVictim.InterceptOnceMotion(MOTION_DAMAGE_BACK))
                    rVictim.PushLoopMotion(MOTION_WAIT);
                else if (rVictim.InterceptOnceMotion(MOTION_DAMAGE))
                    rVictim.PushLoopMotion(MOTION_WAIT);
            }
        }
    }
}

void CActorInstance::__HitGreate(CActorInstance& rVictim)
{
    // DISABLE_KNOCKDOWN_ATTACK
    if (rVictim.IsKnockDown())
        return;
    if (rVictim.__IsStandUpMotion())
        return;
    // END_OF_DISABLE_KNOCKDOWN_ATTACK
    rVictim.__Shake(100);

    if (rVictim.IsUsingSkill())
        return;

    float fRotRad = DirectX::XMConvertToRadians(GetRotation());
    float fVictimRotRad = DirectX::XMConvertToRadians(rVictim.GetRotation());

    Vector2 v2Normal(sin(fRotRad), cos(fRotRad));
    Vector2 v2VictimNormal(sin(fVictimRotRad), cos(fVictimRotRad));

    v2Normal.Normalize();
    v2VictimNormal.Normalize();

    float fScalar = v2Normal.Dot(v2VictimNormal);

    if (rVictim.IsStun()) {
        if (fScalar < 0.0f) {
            rVictim.InterceptOnceMotion(MOTION_DAMAGE_FLYING);
        } else {
            if (!rVictim.InterceptOnceMotion(MOTION_DAMAGE_FLYING_BACK))
                rVictim.InterceptOnceMotion(MOTION_DAMAGE_FLYING);
        }

        rVictim.m_isRealDead = true;
    } else {
        if (fScalar < 0.0f) {
            if (rVictim.InterceptOnceMotion(MOTION_DAMAGE_FLYING)) {
                if (rVictim.PushOnceMotion(MOTION_STAND_UP))
                    rVictim.PushLoopMotion(MOTION_WAIT);
            }
        } else {
            if (!rVictim.InterceptOnceMotion(MOTION_DAMAGE_FLYING_BACK)) {
                if (rVictim.InterceptOnceMotion(MOTION_DAMAGE_FLYING)) {
                    if (rVictim.PushOnceMotion(MOTION_STAND_UP))
                        rVictim.PushLoopMotion(MOTION_WAIT);
                }
            } else {
                if (rVictim.PushOnceMotion(MOTION_STAND_UP_BACK))
                    rVictim.PushLoopMotion(MOTION_WAIT);
            }
        }
    }
}

void CActorInstance::SetBlendingPosition(const TPixelPosition& c_rPosition,
                                         float fBlendingTime)
{
    // return;
    TPixelPosition Position;

    Position.x = c_rPosition.x - m_x;
    Position.y = c_rPosition.y - m_y;
    Position.z = 0;

    m_PhysicsObject.SetLastPosition(Position, fBlendingTime);
}

void CActorInstance::ResetBlendingPosition()
{
    m_PhysicsObject.Initialize();
}

void CActorInstance::GetBlendingPosition(TPixelPosition* pPosition)
{
    if (m_PhysicsObject.isBlending()) {
        m_PhysicsObject.GetLastPosition(pPosition);
        pPosition->x += m_x;
        pPosition->y += m_y;
        pPosition->z += m_z;
    } else {
        pPosition->x = m_x;
        pPosition->y = m_y;
        pPosition->z = m_z;
    }
}

void CActorInstance::__PushCircle(CActorInstance& rVictim)
{
    const TPixelPosition& c_rkPPosAtk = NEW_GetAtkPixelPositionRef();

    const Vector3 v3SrcPos(c_rkPPosAtk.x, -c_rkPPosAtk.y, c_rkPPosAtk.z);

    const Vector3& c_rv3SrcPos = v3SrcPos;
    const Vector3& c_rv3DstPos = rVictim.GetPosition();

    Vector3 v3Direction;
    v3Direction.x = c_rv3DstPos.x - c_rv3SrcPos.x;
    v3Direction.y = c_rv3DstPos.y - c_rv3SrcPos.y;
    v3Direction.z = 0.0f;
    v3Direction.Normalize();

    rVictim.__SetFallingDirection(v3Direction.x, v3Direction.y);
}

void CActorInstance::__PushDirect(CActorInstance& rVictim)
{

    rVictim.__SetFallingDirection(
        cosf(DirectX::XMConvertToRadians(m_fcurRotation + 270.0f)),
        sinf(DirectX::XMConvertToRadians(m_fcurRotation + 270.0f)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CActorInstance::__isInvisible()
{
    if (IsDead())
        return true;

    if (DX::StepTimer::Instance().GetTotalSeconds() >= m_fInvisibleTime)
        return false;

    return true;
}

void CActorInstance::__SetFallingDirection(float fx, float fy)
{
    m_PhysicsObject.SetDirection(Vector3(fx, fy, 0.0f));
}
