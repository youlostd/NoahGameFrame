#include "StdAfx.h"
#include "ActorInstance.h"
#include "RaceData.h"
#include "FlyHandler.h"

#include <game/MotionConstants.hpp>
#include "../eterGrnLib/Motion.h"
#include <game/MotionTypes.hpp>

UINT CActorInstance::__GetMotionType()
{
    if (!m_pkCurRaceMotionData)
        return MOTION_TYPE_NONE;

    return m_pkCurRaceMotionData->GetType();
}

void CActorInstance::__MotionEventProcess(bool isPC)
{
    if (isAttacking())
    {
        uint32_t dwNextFrame = uint32_t(GetAttackingElapsedTime() * g_fGameFPS);
        for (; m_kCurMotNode.dwcurFrame < dwNextFrame; ++m_kCurMotNode.dwcurFrame)
        {
            MotionEventProcess();
            SoundEventProcess(!isPC);
        }
    }
    else
    {
        MotionEventProcess();
        SoundEventProcess(!isPC);

        ++m_kCurMotNode.dwcurFrame;
    }
}

void CActorInstance::MotionProcess(bool isPC)
{
    __MotionEventProcess(isPC);
    CurrentMotionProcess();
    ReservingMotionProcess();
}

void CActorInstance::HORSE_MotionProcess(bool isPC)
{
    __MotionEventProcess(isPC);

    if (MOTION_TYPE_LOOP == m_kCurMotNode.iMotionType)
        if (m_kCurMotNode.dwcurFrame >= m_kCurMotNode.dwFrameCount)
            m_kCurMotNode.dwcurFrame = 0;
}

void CActorInstance::ReservingMotionProcess()
{
    if (m_MotionDeque.empty())
        return;

    TReservingMotionNode &rReservingMotionNode = m_MotionDeque.front();

    float fCurrentTime = GetLocalTime();
    if (rReservingMotionNode.fStartTime > fCurrentTime)
        return;

    uint32_t dwNextMotionIndex = MakeMotionId(rReservingMotionNode.dwMotionKey).index;
    switch (dwNextMotionIndex)
    {
    case MOTION_STAND_UP:
    case MOTION_STAND_UP_BACK:
        if (IsFaint())
        {
            //Tracenf("일어서려고 했으나 기절중");

            SetEndStopMotion();

            // 이후의 모션 전부 1초씩 딜레이
            TMotionDeque::iterator itor = m_MotionDeque.begin();
            for (; itor != m_MotionDeque.end(); ++itor)
            {
                TReservingMotionNode &rNode = *itor;
                rNode.fStartTime += 1.0f;
            }
            return;
        }
        break;
    }

    SCurrentMotionNode kPrevMotionNode = m_kCurMotNode;

    EMotionPushType iMotionType = rReservingMotionNode.iMotionType;
    float fSpeedRatio = rReservingMotionNode.fSpeedRatio;
    float fBlendTime = rReservingMotionNode.fBlendTime;
    uint32_t dwMotionKey = rReservingMotionNode.dwMotionKey;

    m_MotionDeque.pop_front();

    uint32_t dwCurrentMotionIndex = MakeMotionId(dwMotionKey).index;
    switch (dwCurrentMotionIndex)
    {
    case MOTION_STAND_UP:
    case MOTION_STAND_UP_BACK:
        if (IsDead())
        {
            //Tracenf("일어서려고 했으나 사망");
            // 예전 데이터로 복구
            m_kCurMotNode = kPrevMotionNode;
            __ClearMotion();

            // 이전 동작 마지막 상태 유지
            SetEndStopMotion();
            return;
        }
        break;
    }

    int iLoopCount;
    if (MOTION_TYPE_ONCE == iMotionType)
        iLoopCount = 1;
    else
        iLoopCount = 0;

    SSetMotionData kSetMotData;
    kSetMotData.dwMotKey = dwMotionKey;
    kSetMotData.fBlendTime = fBlendTime;
    kSetMotData.fSpeedRatio = fSpeedRatio;
    kSetMotData.iLoopCount = iLoopCount;

    uint32_t dwRealMotionKey = __SetMotion(kSetMotData);

    if (0 == dwRealMotionKey)
        return;

    float fDurationTime = GetMotionDuration(dwRealMotionKey) / fSpeedRatio;
    float fStartTime = rReservingMotionNode.fStartTime;
    float fEndTime = fStartTime + fDurationTime;

    if (dwRealMotionKey == 16777473)
    {
        int bp = 0;
        bp++;
    }

    m_kCurMotNode.uSkill = 0;
    m_kCurMotNode.iMotionType = iMotionType;
    m_kCurMotNode.fSpeedRatio = fSpeedRatio;
    m_kCurMotNode.fStartTime = fStartTime;
    m_kCurMotNode.fEndTime = fEndTime;
    m_kCurMotNode.dwMotionKey = dwRealMotionKey;
    m_kCurMotNode.dwcurFrame = 0;
    m_kCurMotNode.dwFrameCount = fDurationTime / (1.0f / g_fGameFPS);
    m_kCurMotNode.addColor = 0;
}

void CActorInstance::CurrentMotionProcess()
{
    if (MOTION_TYPE_LOOP == m_kCurMotNode.iMotionType) // 임시다. 최종적인 목표는 Once도 절대로 넘어가선 안된다. - [levites]
        if (m_kCurMotNode.dwcurFrame >= m_kCurMotNode.dwFrameCount)
            m_kCurMotNode.dwcurFrame = 0;

    if (IsDead())
        return;

    if (!m_MotionDeque.empty())
        return;

    float fCurrentTime = GetLocalTime();
    uint32_t dwMotionIndex = MakeMotionId(m_kCurMotNode.dwMotionKey).index;
    bool isLooping = false;

    // 끝났다면 Playing Flag를 끈다
    if (m_pkCurRaceMotionData && m_pkCurRaceMotionData->IsLoopMotion())
    {
        if (m_kCurMotNode.iLoopCount > 1 || m_kCurMotNode.iLoopCount == -1)
        {
            if (fCurrentTime - m_kCurMotNode.fStartTime > m_pkCurRaceMotionData->GetLoopEndTime())
            {
                m_kCurMotNode.dwcurFrame = uint32_t(m_pkCurRaceMotionData->GetLoopStartTime() * g_fGameFPS);
                __SetLocalTime(m_kCurMotNode.fStartTime + m_pkCurRaceMotionData->GetLoopStartTime());
                if (-1 != m_kCurMotNode.iLoopCount)
                    --m_kCurMotNode.iLoopCount;

                isLooping = true;
            }
        }
        else if (!m_kQue_kFlyTarget.empty())
        {
            if (!m_kBackupFlyTarget.IsObject())
            {
                m_kBackupFlyTarget = m_kFlyTarget;
            }

            if (fCurrentTime - m_kCurMotNode.fStartTime > m_pkCurRaceMotionData->GetLoopEndTime())
            {
                m_kCurMotNode.dwcurFrame = uint32_t(m_pkCurRaceMotionData->GetLoopStartTime() * g_fGameFPS);
                __SetLocalTime(m_kCurMotNode.fStartTime + m_pkCurRaceMotionData->GetLoopStartTime());

                SetFlyTarget(m_kQue_kFlyTarget.front());
                m_kQue_kFlyTarget.pop_front();

                isLooping = true;
            }
        }
    }

    if (!isLooping)
    {
        if (fCurrentTime > m_kCurMotNode.fEndTime)
        {
            if (m_kBackupFlyTarget.IsValidTarget())
            {
                m_kFlyTarget = m_kBackupFlyTarget;
                m_kBackupFlyTarget.Clear();
            }

            ////////////////////////////////////////////

            if (MOTION_TYPE_ONCE == m_kCurMotNode.iMotionType)
            {
                switch (dwMotionIndex)
                {
                case MOTION_DAMAGE_FLYING:
                case MOTION_DAMAGE_FLYING_BACK:
                case MOTION_DEAD:
                case MOTION_INTRO_SELECTED:
                case MOTION_INTRO_NOT_SELECTED:
                    m_kCurMotNode.fEndTime += 3.0f;
                    SetEndStopMotion();
                    break;
                default:
                    InterceptLoopMotion(MOTION_WAIT);
                    break;
                }
            }
            else if (MOTION_TYPE_LOOP == m_kCurMotNode.iMotionType)
            {
                if (MOTION_WAIT == dwMotionIndex)
                {
                    PushLoopMotion(MOTION_WAIT, 0.5f);
                }
            }
        }
    }
}

void CActorInstance::SetMotionMode(int iMotionMode)
{
    if (IsPoly())
        iMotionMode = MOTION_MODE_GENERAL;

    m_wcurMotionMode = iMotionMode;
}

int CActorInstance::GetMotionMode()
{
    return m_wcurMotionMode;
}

void CActorInstance::SetMotionLoopCount(int iCount)
{
    assert(iCount >= -1 && iCount < 100);
    m_kCurMotNode.iLoopCount = iCount;
}

void CActorInstance::PushMotion(EMotionPushType iMotionType, uint32_t dwMotionKey, float fBlendTime, float fSpeedRatio)
{
    if (!CheckMotionThingIndex(dwMotionKey, m_dwMotionID))
    {
        SPDLOG_ERROR("Not found want to using motion : {0} {1} {2}",
                      MakeMotionId(dwMotionKey).index, m_dwMotionID, m_eRace);
        return;
    }

    TReservingMotionNode MotionNode;

    MotionNode.iMotionType = iMotionType;
    MotionNode.dwMotionKey = dwMotionKey;
    MotionNode.fStartTime = GetLastMotionTime(fBlendTime);
    MotionNode.fBlendTime = fBlendTime;
    MotionNode.fSpeedRatio = fSpeedRatio;
    MotionNode.fDuration = GetMotionDuration(dwMotionKey);

    m_MotionDeque.push_back(MotionNode);
}

bool CActorInstance::InterceptOnceMotion(uint32_t dwMotion, float fBlendTime, UINT uSkill, float fSpeedRatio,
                                         uint32_t addColor)
{
    if (m_eActorType == TYPE_PC)
    {
        storm::StringRef motion;
        GetMotionString(motion, dwMotion);
        spdlog::trace("InterceptOnceMotion {} m_eRace = {}", motion, m_eRace);
    }
    if (IsStone()) {
        return false;
    }
    return InterceptMotion(MOTION_TYPE_ONCE, dwMotion, fBlendTime, uSkill, fSpeedRatio, addColor);
}

bool CActorInstance::InterceptLoopMotion(uint32_t dwMotion, float fBlendTime)
{
    if (m_eActorType == TYPE_PC)
    {
        storm::StringRef motion;
        GetMotionString(motion, dwMotion);
        spdlog::trace("InterceptLoopMotion {} m_eRace = {}", motion, m_eRace);
    }
    return InterceptMotion(MOTION_TYPE_LOOP, dwMotion, fBlendTime);
}

void CActorInstance::SetLoopMotion(uint32_t dwMotion, float fBlendTime, float fSpeedRatio)
{
    if (!m_pkCurRaceData)
    {
        //SPDLOG_WARN("CActorInstance::SetLoopMotion(dwMotion={0}, fBlendTime={1}, fSpeedRatio={2}) - No RaceData",
        //	dwMotion, fBlendTime, fSpeedRatio);
        return;
    }

    uint32_t dwMotionKey;
    if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, dwMotion, &dwMotionKey))
    {
        //SPDLOG_ERROR("CActorInstance::SetLoopMotion(dwMotion={0}, fBlendTime={1}, fSpeedRatio={2}) - GetMotionKey(m_wcurMotionMode={3}, dwMotion={4}, &MotionKey) ERROR",
        //	dwMotion, fBlendTime, fSpeedRatio, m_wcurMotionMode, dwMotion);
        return;
    }

    __ClearMotion();

    SSetMotionData kSetMotData;
    kSetMotData.dwMotKey = dwMotionKey;
    kSetMotData.fBlendTime = fBlendTime;
    kSetMotData.fSpeedRatio = fSpeedRatio;

    uint32_t dwRealMotionKey = __SetMotion(kSetMotData);

    if (0 == dwRealMotionKey)
        return;

    m_kCurMotNode.iMotionType = MOTION_TYPE_LOOP;
    m_kCurMotNode.fStartTime = GetLocalTime();
    m_kCurMotNode.dwMotionKey = dwRealMotionKey;
    m_kCurMotNode.fEndTime = 0.0f;
    m_kCurMotNode.fSpeedRatio = fSpeedRatio;
    m_kCurMotNode.dwcurFrame = 0;
    m_kCurMotNode.dwFrameCount = GetMotionDuration(dwRealMotionKey) / (1.0f / g_fGameFPS);
    m_kCurMotNode.uSkill = 0;
}

// 리턴값 == SetMotion의 리턴값 == 실제로 애니메이션 데이터를 플레이 했느냐?
bool CActorInstance::InterceptMotion(EMotionPushType iMotionType, uint16_t wMotion, float fBlendTime, UINT uSkill,
                                     float fSpeedRatio, uint32_t addColor)
{
    if (!m_pkCurRaceData)
    {
        SPDLOG_WARN(
            "CActorInstance::InterceptMotion(iMotionType={0}, wMotion={1}, fBlendTime={2}) - m_pkCurRaceData=NULL",
            iMotionType, wMotion, fBlendTime);
        return false;
    }

    uint32_t dwMotionKey;
    if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, wMotion, &dwMotionKey))
    {
        SPDLOG_WARN(
            "CActorInstance::InterceptMotion(iLoopType={0}, wMotionMode={1}, wMotion={2}, fBlendTime={3}) - GetMotionKey(m_wcurMotionMode={4}, wMotion={5}, &MotionKey) ERROR",
            iMotionType, m_wcurMotionMode, wMotion, fBlendTime, m_wcurMotionMode, wMotion);
        return false;
    }

    __ClearMotion();

    int iLoopCount;
    if (MOTION_TYPE_ONCE == iMotionType)
        iLoopCount = 1;
    else
        iLoopCount = 0;

    SSetMotionData kSetMotData;
    kSetMotData.dwMotKey = dwMotionKey;
    kSetMotData.fBlendTime = fBlendTime;
    kSetMotData.iLoopCount = iLoopCount;
    kSetMotData.fSpeedRatio = fSpeedRatio;
    kSetMotData.uSkill = uSkill;

    uint32_t dwRealMotionKey = __SetMotion(kSetMotData);

    if (0 == dwRealMotionKey)
        return false;

    if (m_pFlyEventHandler)
    {
        if (__IsNeedFlyTargetMotion())
        {
            m_pFlyEventHandler->OnSetFlyTarget();
        }
    }

    assert(NULL != m_pkCurRaceMotionData);

    if (dwRealMotionKey == 17665)
    {
        static int bp = 0;
        ++bp;
    }

    // FIX : 위에서 호출한 __SetMotion 함수 내에서 랜덤으로 다른 모션을 선택할 수도 있기 때문에 dwMotionKey값은 유효하지 않고
    // 따라서 해당 키로 산출한 duration은 유효하지 않음. 당연히 현재 play중인 모션의 시간을 구해야 함.. -_-;;
    // float fDuration=GetMotionDuration(dwMotionKey)/fSpeedRatio;
    float fDuration = GetMotionDuration(dwRealMotionKey) / fSpeedRatio;

    m_kCurMotNode.iMotionType = iMotionType;
    m_kCurMotNode.fStartTime = GetLocalTime();
    m_kCurMotNode.fEndTime = m_kCurMotNode.fStartTime + fDuration;
    m_kCurMotNode.dwMotionKey = dwRealMotionKey;
    m_kCurMotNode.dwcurFrame = 0;
    m_kCurMotNode.dwFrameCount = fDuration / (1.0f / g_fGameFPS);

    m_kCurMotNode.uSkill = uSkill;
    m_kCurMotNode.fSpeedRatio = fSpeedRatio;
    m_kCurMotNode.addColor = addColor;

    return true;
}

bool CActorInstance::PushOnceMotion(uint32_t dwMotion, float fBlendTime, float fSpeedRatio)
{
    if (!m_pkCurRaceData)
        return false;

    assert(m_pkCurRaceData);

    uint32_t MotionKey;
    if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, dwMotion, &MotionKey))
        return false;

    PushMotion(MOTION_TYPE_ONCE, MotionKey, fBlendTime, fSpeedRatio);
    return true;
}

bool CActorInstance::PushLoopMotion(uint32_t dwMotion, float fBlendTime, float fSpeedRatio)
{
    assert(m_pkCurRaceData);
    if (!m_pkCurRaceData)
        return false;

    if (m_eActorType == TYPE_PC)
    {
        storm::StringRef motion;
        GetMotionString(motion, dwMotion);
        spdlog::trace("PushLoopMotion {} m_eRace = {}", motion, m_eRace);
    }

    uint32_t MotionKey;
    if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, dwMotion, &MotionKey))
        return false;

    PushMotion(MOTION_TYPE_LOOP, MotionKey, fBlendTime, fSpeedRatio);
    return true;
}

uint16_t CActorInstance::__GetCurrentMotionIndex()
{
    return MakeMotionId(m_kCurMotNode.dwMotionKey).index;
}

uint32_t CActorInstance::__GetCurrentMotionKey()
{
    return m_kCurMotNode.dwMotionKey;
}

bool CActorInstance::IsUsingSkill()
{
    uint32_t dwCurMotionIndex = __GetCurrentMotionIndex();

    if (dwCurMotionIndex >= MOTION_SKILL && dwCurMotionIndex < MOTION_SKILL_END)
        return TRUE;

    switch (dwCurMotionIndex)
    {
    case MOTION_SPECIAL_1:
    case MOTION_SPECIAL_2:
    case MOTION_SPECIAL_3:
    case MOTION_SPECIAL_4:
    case MOTION_SPECIAL_5:
    case MOTION_SPECIAL_6:
        return TRUE;
    }

    return FALSE;
}

bool CActorInstance::IsMining()
{
    if (!m_pkCurRaceMotionData)
        return FALSE;

    if (__GetCurrentMotionIndex() == MOTION_DIG)
        return TRUE;

    return FALSE;
}

bool CActorInstance::IsFishing()
{
    if (!m_pkCurRaceMotionData)
        return FALSE;

    if (__GetCurrentMotionIndex() == MOTION_FISHING_WAIT ||
        __GetCurrentMotionIndex() == MOTION_FISHING_REACT ||
        __GetCurrentMotionIndex() == MOTION_FISHING_THROW)
        return TRUE;

    return FALSE;
}

bool CActorInstance::CanCancelSkill()
{
    assert(IsUsingSkill());
    return m_pkCurRaceMotionData->IsCancelEnableSkill();
}

bool CActorInstance::isLock()
{
    uint32_t dwCurMotionIndex = __GetCurrentMotionIndex();

    // Locked during attack
    switch (dwCurMotionIndex)
    {
    case MOTION_NORMAL_ATTACK:
    case MOTION_COMBO_ATTACK_1:
    case MOTION_COMBO_ATTACK_2:
    case MOTION_COMBO_ATTACK_3:
    case MOTION_COMBO_ATTACK_4:
    case MOTION_COMBO_ATTACK_5:
    case MOTION_COMBO_ATTACK_6:
    case MOTION_COMBO_ATTACK_7:
    case MOTION_COMBO_ATTACK_8:
    case MOTION_SPECIAL_1:
    case MOTION_SPECIAL_2:
    case MOTION_SPECIAL_3:
    case MOTION_SPECIAL_4:
    case MOTION_SPECIAL_5:
    case MOTION_SPECIAL_6:
    case MOTION_FISHING_THROW:
    case MOTION_FISHING_WAIT:
    case MOTION_FISHING_STOP:
    case MOTION_FISHING_REACT:
    case MOTION_FISHING_CATCH:
    case MOTION_FISHING_FAIL:
    case MOTION_CLAP:
    case MOTION_DANCE_1:
    case MOTION_DANCE_2:
    case MOTION_DANCE_3:
    case MOTION_DANCE_4:
    case MOTION_DANCE_5:
    case MOTION_DANCE_6:
    case MOTION_DANCE_7:

    case MOTION_EMOTION_PUSH_UP:
    case MOTION_EMOTION_DANCE_7:
    case MOTION_EMOTION_EXERCISE:
    case MOTION_EMOTION_DOZE:
    case MOTION_EMOTION_SELFIE:
    case MOTION_EMOTION_CHARGING:
    case MOTION_EMOTION_NOSAY:
    case MOTION_EMOTION_WEATHER_1:
    case MOTION_EMOTION_WEATHER_2:
    case MOTION_EMOTION_WEATHER_3:
    case MOTION_EMOTION_HUNGRY:
    case MOTION_EMOTION_SIREN:
    case MOTION_EMOTION_LETTER:
    case MOTION_EMOTION_CALL:
    case MOTION_EMOTION_CELEBRATION:
    case MOTION_EMOTION_ALCOHOL:
    case MOTION_EMOTION_BUSY:
    case MOTION_THROW_MONEY:
    case MOTION_CONGRATULATION:
    case MOTION_FORGIVE:
    case MOTION_ANGRY:
    case MOTION_ATTRACTIVE:
    case MOTION_SAD:
    case MOTION_SHY:
    case MOTION_CHEERUP:
    case MOTION_BANTER:
    case MOTION_JOY:
    case MOTION_CHEERS_1:
    case MOTION_CHEERS_2:
    case MOTION_KISS_WITH_WARRIOR:
    case MOTION_KISS_WITH_ASSASSIN:
    case MOTION_KISS_WITH_SURA:
    case MOTION_KISS_WITH_SHAMAN:
    case MOTION_FRENCH_KISS_WITH_WARRIOR:
    case MOTION_FRENCH_KISS_WITH_ASSASSIN:
    case MOTION_FRENCH_KISS_WITH_SURA:
    case MOTION_FRENCH_KISS_WITH_SHAMAN:
    case MOTION_SLAP_HIT_WITH_WARRIOR:
    case MOTION_SLAP_HIT_WITH_ASSASSIN:
    case MOTION_SLAP_HIT_WITH_SURA:
    case MOTION_SLAP_HIT_WITH_SHAMAN:
    case MOTION_SLAP_HURT_WITH_WARRIOR:
    case MOTION_SLAP_HURT_WITH_ASSASSIN:
    case MOTION_SLAP_HURT_WITH_SURA:
    case MOTION_SLAP_HURT_WITH_SHAMAN:
        return TRUE;
        break;
    }

    // Locked during using skill
    if (IsUsingSkill())
    {
        if (m_pkCurRaceMotionData->IsCancelEnableSkill())
            return FALSE;

        return TRUE;
    }

    return FALSE;
}

float CActorInstance::GetLastMotionTime(float fBlendTime)
{
    if (m_MotionDeque.empty())
    {
        if (MOTION_TYPE_ONCE == m_kCurMotNode.iMotionType)
            return (m_kCurMotNode.fEndTime - fBlendTime);

        return GetLocalTime();
    }

    TReservingMotionNode &rMotionNode = m_MotionDeque[m_MotionDeque.size() - 1];
    return rMotionNode.fStartTime + rMotionNode.fDuration - fBlendTime;
}

float CActorInstance::GetMotionDuration(uint32_t dwMotionKey)
{
    CGraphicThing::Ptr pMotion;
    if (!GetMotionThingPointer(m_dwMotionID, dwMotionKey, &pMotion))
    {
        auto id = MakeMotionId(dwMotionKey);
        SPDLOG_WARN("CActorInstance::GetMotionDuration - Cannot get motion: {0} / {1}, {2} race {3}",
                     id.mode, id.index, id.subIndex, m_dwMotionID);
        return 0.0f;
    }

    if (0 == pMotion->GetMotionCount())
    {
        auto id = MakeMotionId(dwMotionKey);
        SPDLOG_DEBUG("CActorInstance::GetMotionDuration - Invalid Motion Key : {0}, {1}, {2} race {3}",
                      id.mode, id.index, id.subIndex, m_dwMotionID);
        return 0.0f;
    }

    auto pGrannyMotion = pMotion->GetMotionPointer(0);
    return pGrannyMotion->GetDuration();
}
#pragma optimize( "", off )
uint32_t CActorInstance::GetRandomMotionKey(uint32_t dwMotionKey)
{
    // NOTE : 자주 호출 되는 부분은 아니지만 어느 정도의 최적화 여지가 있음 - [levites]
    // FIXME : 처음에 선택된 모션이 없는 것에 대한 처리가 되어 있지 않다.
    auto id = MakeMotionId(dwMotionKey);
    uint16_t wMode = id.mode;
    uint16_t wIndex = id.index;

    const CRaceData::TMotionVector *c_pMotionVector;
    if (m_pkCurRaceData->GetMotionVectorPointer(wMode, wIndex, &c_pMotionVector))
    {
        if (c_pMotionVector->size() > 1)
        {
            std::vector<int> prob;
            for (const auto &c_rMotion : *c_pMotionVector)
            {
                prob.push_back(c_rMotion.byPercentage);
            }

            std::discrete_distribution<> d(prob.begin(), prob.end());
            auto subIndex = d(GetRand());

            return MakeMotionKey(wMode, wIndex, subIndex);
        }
    }

    return dwMotionKey;
}

void CActorInstance::PreAttack()
{
}

void CActorInstance::__ClearMotion()
{
    __HideWeaponTrace();

    m_MotionDeque.clear();
    m_kCurMotNode.dwcurFrame = 0;
    m_kCurMotNode.dwFrameCount = 0;
    m_kCurMotNode.uSkill = 0;
    m_kCurMotNode.iLoopCount = 0;
    m_kCurMotNode.iMotionType = MOTION_TYPE_NONE;
    m_kCurMotNode.addColor = 0;
}

uint32_t CActorInstance::__SetMotion(const SSetMotionData &c_rkSetMotData, uint32_t dwRandMotKey)
{
    uint32_t dwMotKey = dwRandMotKey;

    if (dwMotKey == 0)
        dwMotKey = GetRandomMotionKey(c_rkSetMotData.dwMotKey);

    auto id = MakeMotionId(dwMotKey);

    if (IsDead())
    {
        if (id.index != MOTION_DAMAGE_FLYING &&
            id.index != MOTION_DAMAGE_FLYING_BACK &&
            id.index != MOTION_DEAD &&
            id.index != MOTION_DEAD_BACK)
            return 0;
    }

    if (IsUsingSkill())
        __OnStop();

    if (__IsStandUpMotion())
        __OnStop();

    if (__IsMoveMotion())
    {
        if (id.index == MOTION_DAMAGE || id.index == MOTION_DAMAGE_BACK || id.index == MOTION_DAMAGE_FLYING || id.index
            == MOTION_DAMAGE_FLYING_BACK)
        {
            if (!m_isMain)
            {
                return false;
            }
        }

        if (id.index != MOTION_RUN &&
            id.index != MOTION_WALK &&
            !__IsMovingSkill(c_rkSetMotData.uSkill))
        {
            __OnStop();
        }
    }
    else
    {
        if (id.index == MOTION_RUN || __IsMovingSkill(c_rkSetMotData.uSkill))
        {
            __OnMove();
        }
    }

    // NOTE : 스킬 사용중 사라지는 문제를 위한 안전 장치 - [levites]
    if (__IsHiding())
        __ShowEvent();

    if (-1 != m_iFishingEffectID)
    {
        CEffectManager &rkEftMgr = CEffectManager::Instance();
        rkEftMgr.DeactiveEffectInstance(m_iFishingEffectID);

        m_iFishingEffectID = -1;
    }

    if (m_pkHorse)
    {
        auto dwChildMotKey = MakeMotionKey(m_pkHorse->m_wcurMotionMode,
                                           id.index,
                                           id.subIndex);

        if (MOTION_DEAD == id.index)
            CGraphicThingInstance::ChangeMotion(dwMotKey, c_rkSetMotData.iLoopCount, c_rkSetMotData.fSpeedRatio);
        else
            CGraphicThingInstance::SetMotion(dwMotKey, c_rkSetMotData.fBlendTime, c_rkSetMotData.iLoopCount,
                                             c_rkSetMotData.fSpeedRatio);

        m_pkHorse->SetMotion(dwChildMotKey, c_rkSetMotData.fBlendTime, c_rkSetMotData.iLoopCount,
                             c_rkSetMotData.fSpeedRatio);
        m_pkHorse->__BindMotionData(dwChildMotKey);

        if (c_rkSetMotData.iLoopCount)
            m_pkHorse->m_kCurMotNode.iMotionType = MOTION_TYPE_ONCE; // 무조건 이전 모션 타입으로 설정되고 있었음
        else
            m_pkHorse->m_kCurMotNode.iMotionType = MOTION_TYPE_LOOP;

        m_pkHorse->m_kCurMotNode.dwFrameCount = m_pkHorse->GetMotionDuration(dwChildMotKey) / (1.0f / g_fGameFPS);
        m_pkHorse->m_kCurMotNode.dwcurFrame = 0;
        m_pkHorse->m_kCurMotNode.dwMotionKey = dwChildMotKey;
        m_pkHorse->m_kCurMotNode.addColor = 0;
    }
    else
    {
        CGraphicThingInstance::SetMotion(dwMotKey, c_rkSetMotData.fBlendTime, c_rkSetMotData.iLoopCount,
                                         c_rkSetMotData.fSpeedRatio);
    }

    __HideWeaponTrace();

    if (__BindMotionData(dwMotKey))
    {
        int iLoopCount = __GetLoopCount();
        SetMotionLoopCount(iLoopCount);

        if (__CanAttack())
        {
            // 여기서 공격 모션일 경우의 처리를 합니다 - [levites]
            __ShowWeaponTrace();

            m_HitDataMap.clear();
            //PreAttack();
        }

        if (__IsComboAttacking())
        {
            if (!__CanNextComboAttack())
            {
                // 2004.11.19.myevan.동물 변신시 이부분에서 바로 리셋되어 다음동작 안나온다
                m_dwcurComboIndex = 0; // 콤보 리셋 - [levites]

                // NOTE : ClearCombo() 를 수행해서는 안된다.
                //        콤보 다음에 스킬을 이어서 사용할 경우 m_pkCurRaceMotionData까지 초기화 되어 버린다.
                //Tracef("MotionData에 콤보 데이타가 들어 있지 않습니다.\n");
            }
        }
    }

    return dwMotKey;
}

bool CActorInstance::__BindMotionData(uint32_t dwMotionKey)
{
    if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey, &m_pkCurRaceMotionData))
    {
        SPDLOG_WARN("Failed to bind motion.");
        m_pkCurRaceMotionData = NULL;
        m_dwcurComboIndex = 0;
        return false;
    }

    return true;
}

int CActorInstance::__GetLoopCount()
{
    if (!m_pkCurRaceMotionData)
    {
        SPDLOG_ERROR("CActorInstance::__GetLoopCount() - m_pkCurRaceMotionData==NULL");
        return -1;
    }

    return m_pkCurRaceMotionData->GetLoopCount();
}

bool CActorInstance::__CanAttack()
{
    if (!m_pkCurRaceMotionData)
    {
        SPDLOG_ERROR("CActorInstance::__CanAttack() - m_pkCurRaceMotionData==NULL");
        return false;
    }

    if (!m_pkCurRaceMotionData->isAttackingMotion())
        return false;

    return true;
}

bool CActorInstance::__CanNextComboAttack()
{
    if (!m_pkCurRaceMotionData)
    {
        SPDLOG_ERROR("CActorInstance::__CanNextComboAttack() - m_pkCurRaceMotionData==NULL");
        return false;
    }

    if (!m_pkCurRaceMotionData->IsComboInputTimeData())
        return false;

    return true;
}

bool CActorInstance::__IsComboAttacking()
{
    if (0 == m_dwcurComboIndex)
        return false;

    return true;
}

bool CActorInstance::__IsNeedFlyTargetMotion()
{
    if (!m_pkCurRaceMotionData)
        return true;

    for (uint32_t i = 0; i < m_pkCurRaceMotionData->GetMotionEventDataCount(); ++i)
    {
        const CRaceMotionData::TMotionEventData *c_pData;
        if (!m_pkCurRaceMotionData->GetMotionEventDataPointer(i, &c_pData))
            continue;

        if (c_pData->iType == MOTION_EVENT_TYPE_WARP)
            return true;

        if (c_pData->iType == MOTION_EVENT_TYPE_FLY)
            return true;

        if (c_pData->iType == MOTION_EVENT_TYPE_EFFECT_TO_TARGET)
            return true;

        if (c_pData->iType == MOTION_EVENT_TYPE_RELATIVE_MOVE_ON)
            return true;
    }

    return false;
}

bool CActorInstance::__HasMotionFlyEvent()
{
    if (!m_pkCurRaceMotionData)
        return true;

    for (uint32_t i = 0; i < m_pkCurRaceMotionData->GetMotionEventDataCount(); ++i)
    {
        const CRaceMotionData::TMotionEventData *c_pData;
        if (!m_pkCurRaceMotionData->GetMotionEventDataPointer(i, &c_pData))
            continue;

        if (c_pData->iType == MOTION_EVENT_TYPE_FLY)
            return true;

        if (c_pData->iType == MOTION_EVENT_TYPE_RELATIVE_MOVE_ON)
            return true;
    }
    return false;
}

bool CActorInstance::__IsWaitMotion()
{
    return (__GetMotionType() == MOTION_TYPE_WAIT);
}

bool CActorInstance::__IsMoveMotion()
{
    return (__GetMotionType() == MOTION_TYPE_MOVE);
}

bool CActorInstance::__IsAttackMotion()
{
    return (__GetMotionType() == MOTION_TYPE_ATTACK);
}

bool CActorInstance::__IsComboAttackMotion()
{
    return (__GetMotionType() == MOTION_TYPE_COMBO);
}

bool CActorInstance::__IsDamageMotion()
{
    return (__GetMotionType() == MOTION_TYPE_DAMAGE);
}

bool CActorInstance::__IsKnockDownMotion()
{
    return (__GetMotionType() == MOTION_TYPE_KNOCKDOWN);
}

bool CActorInstance::__IsEmotionMotion()
{
    return (__GetMotionType() == MOTION_TYPE_EMOTION);
}

bool CActorInstance::__IsEventMotion()
{
    return (__GetMotionType() == MOTION_TYPE_EVENT);
}

bool CActorInstance::__IsDieMotion()
{
    if (__IsKnockDownMotion())
        return true;

    return (__GetMotionType() == MOTION_TYPE_DIE);
}

bool CActorInstance::__IsStandUpMotion()
{
    return (__GetMotionType() == MOTION_TYPE_STANDUP);
}
