#include "StdAfx.h"
#include "PythonPlayer.h"
#include "PythonTextTail.h"
#include "PythonNetworkStream.h"
#include "InstanceBase.h"
#include "PythonChat.h"
#include "PythonGuild.h"

#include "../GameLib/ItemManager.h"

#include <game/AffectConstants.hpp>
#include "PythonBackground.h"
#include "../eterBase/Timer.h"
#include "PythonApplication.h"

int32_t CPythonPlayer::CanDetachMetin(uint32_t vnum, TItemPos targetCell) const
{
    // Scroll
    CItemData *pScrollItemData = CItemManager::Instance().GetProto(vnum);
    if (!pScrollItemData)
    {
        return DETACH_METIN_CANT;
    }
    int iScrollType = pScrollItemData->GetType();
    int iScrollSubType = pScrollItemData->GetSubType();
    if (iScrollType != ITEM_USE)
    {
        return DETACH_METIN_CANT;
    }
    if (iScrollSubType != USE_DETACHMENT)
    {
        return DETACH_METIN_CANT;
    }

    // Target Item
    int iTargetItemIndex = CPythonPlayer::Instance().GetItemIndex(targetCell);

    CItemData *pTargetItemData = CItemManager::Instance().GetProto(iTargetItemIndex);
    if (!pTargetItemData)
    {
        return DETACH_METIN_CANT;
    }
    //int iTargetType = pTargetItemData->GetType();
    //int iTargetSubType = pTargetItemData->GetSubType();

    if (pTargetItemData->IsFlag(ITEM_FLAG_REFINEABLE))
    {
        for (int iSlotCount = 0; iSlotCount < METIN_SOCKET_COUNT; ++iSlotCount)
            if (CPythonPlayer::Instance().GetItemMetinSocket(targetCell, iSlotCount) > 2)
            {
                return DETACH_METIN_OK;
            }
    }

    return DETACH_METIN_CANT;
}

bool CPythonPlayer::FindSkillSlotIndexBySkillIndex(uint32_t dwSkillIndex, uint32_t *pdwSkillSlotIndex)
{
    for(auto it = m_Skill.cbegin(); it != m_Skill.cend(); ++it)
    {
        auto& rkSkillInst = (*it).second;
        if (dwSkillIndex == rkSkillInst.dwIndex)
        {
            *pdwSkillSlotIndex = it->first;
            return true;
        }
    }

    return false;
}

void CPythonPlayer::ChangeCurrentSkillNumberOnly(uint32_t dwSlotIndex)
{
    TSkillInstance &rkSkillInst = m_Skill[dwSlotIndex];

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
        return;

    if (!pSkillData->IsCanUseSkill())
        return;

    if (!__IsRightButtonSkillMode())
    {
        if (!__IsTarget())
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NEED_TARGET");
            return;
        }

        ClickSkillSlot(dwSlotIndex);
    }
    else
    {
        m_dwcurSkillSlotIndex = dwSlotIndex;
        m_ppyGameWindow.attr("ChangeCurrentSkill")(dwSlotIndex);
    }
}

void CPythonPlayer::ClickSkillSlot(uint32_t dwSlotIndex)
{
    TSkillInstance &rkSkillInst = m_Skill[dwSlotIndex];

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
        return;

    if (CPythonSkill::SKILL_TYPE_GUILD == pSkillData->byType)
    {
        UseGuildSkill(dwSlotIndex);
        return;
    }

    if (!pSkillData->IsCanUseSkill())
        return;

    if (pSkillData->IsStandingSkill())
    {
        if (pSkillData->IsToggleSkill())
        {
            if (IsSkillActive(dwSlotIndex))
            {
                CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
                if (!pkInstMain)
                    return;
                if (pkInstMain->IsUsingSkill())
                    return;

                gPythonNetworkStream->SendUseSkillPacket(rkSkillInst.dwIndex);
                return;
            }
        }

        __UseSkill(dwSlotIndex);
        return;
    }

    if (m_dwcurSkillSlotIndex == dwSlotIndex)
    {
        __UseSkill(m_dwcurSkillSlotIndex);
        return;
    }

    if (!__IsRightButtonSkillMode())
    {
        __UseSkill(dwSlotIndex);
    }
    else
    {
        m_dwcurSkillSlotIndex = dwSlotIndex;
        m_ppyGameWindow.attr("ChangeCurrentSkill")(dwSlotIndex);
    }
}

bool CPythonPlayer::__CheckSkillUsable(uint32_t dwSlotIndex)
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;


    TSkillInstance &rkSkillInst = m_Skill[dwSlotIndex];

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
        return false;

    //TODO: NoHorseSkillMaps
    const auto isNoHorseSkillMap = CPythonBackground::instance().IsOnBlockedHorseSkillMap();

    if (isNoHorseSkillMap && pkInstMain->IsMountingHorse() && pSkillData->IsHorseSkill())
    {
        m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "BLOCKED");
        return false;
    }

    if (pkInstMain->IsMountingHorse())
    {
        if (!pSkillData->IsHorseSkill())
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NOT_HORSE_SKILL");
            return false;
        }
    }

    if (pSkillData->IsHorseSkill())
    {
        if (!pkInstMain->IsMountingHorse())
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "HAVE_TO_RIDE");
            return false;
        }
    }

    // 2004.07.26.levites - 안전지대에서 공격 못하도록 수정
    // NOTE : 공격 스킬은 안전지대에서 사용하지 못합니다 - [levites]
    if (pSkillData->IsAttackSkill())
    {
        if (pkInstMain->IsInSafe())
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "IN_SAFE");
            return false;
        }
    }

    // NOTE : 패시브 스킬은 사용하지 못합니다 - [levites]
    if (!pSkillData->IsCanUseSkill())
        return false;
    //	if (CPythonSkill::SKILL_TYPE_PASSIVE == pSkillData->byType)
    //		return false;

    // NOTE : [Only Assassin] 빈병이 있는지 체크 합니다.
    if (pSkillData->IsNeedEmptyBottle())
    {
        if (!__HasItem(27995))
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NEED_EMPTY_BOTTLE");
            return false;
        }
    }

    // NOTE : [Only Assassin] 독병이 있는지 체크 합니다.
    if (pSkillData->IsNeedPoisonBottle())
    {
        if (!__HasItem(27996))
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NEED_POISON_BOTTLE");
            return false;
        }
    }

    // NOTE : 낚시 중일때는 스킬을 사용하지 못합니다.
    if (pkInstMain->IsFishingMode())
    {
        m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "REMOVE_FISHING_ROD");
        return false;
    }

    // NOTE : 레벨 체크
    if (m_sysIsLevelLimit)
    {
        if (rkSkillInst.iLevel <= 0)
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NOT_YET_LEARN");
            return false;
        }
    }

    // NOTE : 들고 있는 무기 체크
    if (!pSkillData->CanUseWeaponType(pkInstMain->GetWeaponType()))
    {
        m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NOT_MATCHABLE_WEAPON");
        return false;
    }

    if (!pSkillData->IsHorseSkill()) // HORSE 스킬 중에 화살을 쓰지 않는 스킬이 있기 때문에
    {
        if (__CheckShortArrow(rkSkillInst, *pSkillData))
            return false;

        // NOTE : 활이 필요할 경우 화살 개수 체크
        if (pSkillData->IsNeedBow())
        {
            if (!__HasEnoughArrow())
                return false;
        }
    }

    if (__CheckDashAffect(*pkInstMain))
    {
        if (!pSkillData->IsChargeSkill())
        {
            if (__CheckRestSkillCoolTime(dwSlotIndex))
            {
                m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "WAIT_COOLTIME");
                return false;
            }
        }
    }
    else
    {
        if (__CheckRestSkillCoolTime(dwSlotIndex))
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "WAIT_COOLTIME");
            return false;
        }

        if (__CheckShortLife(rkSkillInst, *pSkillData))
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NOT_ENOUGH_HP");
            return false;
        }

        if (__CheckShortMana(rkSkillInst, *pSkillData))
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NOT_ENOUGH_SP");
            return false;
        }
    }

    return true;
}

bool CPythonPlayer::__CheckShortArrow(TSkillInstance &rkSkillInst, CPythonSkill::TSkillData &rkSkillData)
{
    if (!rkSkillData.IsNeedBow())
        return false;

    if (__HasEnoughArrow())
        return false;

    return true;
}

bool CPythonPlayer::__CheckShortMana(TSkillInstance &rkSkillInst, CPythonSkill::TSkillData &rkSkillData)
{
    if (131 == rkSkillInst.dwIndex)
        return false;

    int iNeedSP = rkSkillData.GetNeedSP(rkSkillInst.fcurEfficientPercentage);
    PointValue icurSP = GetStatus(POINT_SP);

    // NOTE : ToggleSkill 이 아닌데 소모 SP 가 0 이다.
    if (!rkSkillData.IsToggleSkill())
    {
        if (iNeedSP == 0)
        {
            return true;
        }
    }

    if (rkSkillData.CanUseIfNotEnough())
    {
        if (icurSP <= 0)
            return true;
    }
    else
    {
        if (-1 != iNeedSP)
            if (iNeedSP > icurSP)
                return true;
    }

    return false;
}

bool CPythonPlayer::__CheckShortLife(TSkillInstance &rkSkillInst, CPythonSkill::TSkillData &rkSkillData)
{
    if (!rkSkillData.IsUseHPSkill())
        return false;

    int64_t dwNeedHP = rkSkillData.GetNeedSP(rkSkillInst.fcurEfficientPercentage);
    if (dwNeedHP <= GetStatus(POINT_HP))
        return false;

    return true;
}

bool CPythonPlayer::__CheckRestSkillCoolTime(uint32_t dwSlotIndex)
{
    if (!m_sysIsCoolTime)
        return false;

    const auto fElapsedTime = DX::StepTimer::Instance().GetTotalSeconds() - m_Skill[dwSlotIndex].
                              fLastUsedTime;
    if (fElapsedTime >= m_Skill[dwSlotIndex].fCoolTime)
        return false;

    return true;
}

bool CPythonPlayer::__CheckDashAffect(CInstanceBase &rkInstMain)
{
    return rkInstMain.HasAffect(AFFECT_DASH);
}

void CPythonPlayer::__UseCurrentSkill()
{
    __UseSkill(m_dwcurSkillSlotIndex);
}

uint32_t CPythonPlayer::__GetSkillTargetRange(CPythonSkill::TSkillData &rkSkillData)
{
    return rkSkillData.GetTargetRange() + GetStatus(POINT_BOW_DISTANCE) * 100;
}

bool CPythonPlayer::__ProcessEnemySkillTargetRange(CInstanceBase &rkInstMain, CInstanceBase &rkInstTarget,
                                                   CPythonSkill::TSkillData &rkSkillData, uint32_t dwSkillSlotIndex)
{
    uint32_t dwSkillTargetRange = __GetSkillTargetRange(rkSkillData);
    float fSkillTargetRange = float(dwSkillTargetRange);
    if (fSkillTargetRange <= 0.0f)
        return true;

    // #0000806: [M2EU] 수룡에게 무사(나한군) 탄환격 스킬 사용 안됨	
    float fTargetDistance = rkInstMain.GetDistance(&rkInstTarget);

    if (CRaceManager::instance().IsHugeRace(rkInstTarget.GetRace()))
    {
        fTargetDistance -= 200.0f; // TEMP: 일단 하드 코딩 처리. 정석적으로는 바운드 스피어를 고려해야함
    }

    if (fTargetDistance >= fSkillTargetRange)
    {
        if (rkSkillData.IsChargeSkill())
        {
            if (!__IsReservedUseSkill(dwSkillSlotIndex))
                __SendUseSkill(dwSkillSlotIndex, 0);
        }

        __ReserveUseSkill(rkInstTarget.GetVirtualID(), dwSkillSlotIndex, dwSkillTargetRange);

        return false;
    }

    // 2004.07.05.myevan. 궁신탄영 사용시 맵에 끼임. 사용하기전 갈수 있는곳 체크
    TPixelPosition kPPosTarget;
    rkInstTarget.NEW_GetPixelPosition(&kPPosTarget);

    if (CPythonBackground::Instance().IsBlock(kPPosTarget.x, kPPosTarget.y))
    {
        m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "CANNOT_APPROACH");
        return false;
    }

    return true;
}

bool CPythonPlayer::__CanUseSkill()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    if (IsObserverMode())
        return false;

    return pkInstMain->CanUseSkill();
}

bool CPythonPlayer::__UseSkill(uint32_t dwSlotIndex)
{
    // PrivateShop
    if (IsOpenPrivateShop())
    {
        return true;
    }

    if (!__CanUseSkill())
    {
        return false;
    }

    TSkillInstance &rkSkillInst = m_Skill[dwSlotIndex];

    if (__CheckSpecialSkill(rkSkillInst.dwIndex))
    {
        return true;
    }

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
    {
        SPDLOG_DEBUG("CPythonPlayer::__UseSkill(dwSlotIndex={0}) There is no skill data", dwSlotIndex);
        return false;
    }

    if (pSkillData->IsToggleSkill())
    {
        if (IsSkillActive(dwSlotIndex))
        {
            gPythonNetworkStream->SendUseSkillPacket(rkSkillInst.dwIndex, dwSlotIndex);
            return false;
        }
    }

    if (!__CheckSkillUsable(dwSlotIndex))
    {
        return false;
    }

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
    {
        SPDLOG_DEBUG("CPythonPlayer::__UseSkill(dwSlotIndex={0}) There is no main player", dwSlotIndex);
        return false;
    }

    if (pkInstMain->IsUsingSkill())
    {
        return false;
    }

    CInstanceBase *pkInstTarget = NULL;

    // NOTE : 타겟이 필요한 경우
    if (pSkillData->IsNeedTarget() ||
        pSkillData->CanChangeDirection() ||
        pSkillData->IsAutoSearchTarget())
    {
        if (pSkillData->IsNeedCorpse())
            pkInstTarget = __GetDeadTargetInstancePtr();
        else
            pkInstTarget = __GetAliveTargetInstancePtr();

        // 현재 타겟이 없으면..
        if (!pkInstTarget)
        {
            // 업데이트하고..
            __ChangeTargetToPickedInstance();

            // 다시 얻어낸다.
            if (pSkillData->IsNeedCorpse())
                pkInstTarget = __GetDeadTargetInstancePtr();
            else
                pkInstTarget = __GetAliveTargetInstancePtr();
        }

        if (pkInstTarget)
        {
            if (pSkillData->IsOnlyForAlliance())
            {
                if (pkInstMain == pkInstTarget)
                {
                    if (!pSkillData->CanUseForMe())
                    {
                        m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "CANNOT_USE_SELF");
                        return false;
                    }
                }
                else if (!pkInstMain->IsAttackableInstance(*pkInstTarget) && pkInstTarget->IsPC())
                {
                    uint32_t dwSkillRange = __GetSkillTargetRange(*pSkillData);

                    if (dwSkillRange > 0)
                    {
                        float fDistance = pkInstMain->GetDistance(pkInstTarget);
                        if (fDistance >= float(dwSkillRange))
                        {
                            __ReserveUseSkill(pkInstTarget->GetVirtualID(), dwSlotIndex, dwSkillRange);
                            return false;
                        }
                    }
                }
                else
                {
                    if (pSkillData->CanUseForMe())
                    {
                        pkInstTarget = pkInstMain;

                        SPDLOG_DEBUG(
                            " [ALERT] 동료에게 사용하는 기술임에도 적에게 타겟팅 되어있어서 자신에게로 재설정\n");
                    }
                    else
                    {
                        m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "ONLY_FOR_ALLIANCE");
                        return false;
                    }
                }
            }
            else
            {
                if (pkInstTarget->IsInSafe())
                {
                    m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(),
                                                             "CANNOT_ATTACK_ENEMY_IN_SAFE_AREA");
                    return false;
                }

                if (pkInstMain->IsAttackableInstance(*pkInstTarget))
                {
                    if (!__ProcessEnemySkillTargetRange(*pkInstMain, *pkInstTarget, *pSkillData, dwSlotIndex))
                        return false;
                }
                else
                {
                    m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "CANNOT_ATTACK");
                    return false;
                }
            }

            pkInstMain->SetFlyTargetInstance(*pkInstTarget);

            if (pkInstMain != pkInstTarget)
            {
                if (pkInstMain->IsFlyTargetObject())
                {
                    pkInstMain->NEW_LookAtFlyTarget();
                }
                else
                {
                    pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);
                }
            }
        }
        else
        {
            if (pSkillData->IsAutoSearchTarget())
            {
                if (pkInstMain->NEW_GetFrontInstance(&pkInstTarget, 2000.0f))
                {
                    SetTarget(pkInstTarget->GetVirtualID());
                    if (!__ProcessEnemySkillTargetRange(*pkInstMain, *pkInstTarget, *pSkillData, dwSlotIndex))
                        return false;
                }
                else
                {
                    m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NEED_TARGET");
                    return false;
                }
            }

            if (pSkillData->CanUseForMe())
            {
                pkInstTarget = pkInstMain;
                pkInstMain->SetFlyTargetInstance(*pkInstMain);
                SPDLOG_DEBUG(" [ALERT] 타겟이 없어서 플레이어에게 사용합니다\n");
            }
            else if (pSkillData->IsNeedCorpse())
            {
                m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "ONLY_FOR_CORPSE");
                return false;
            }
            else
            {
                m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NEED_TARGET");
                return false;
            }
        }
    }

    if (pSkillData->CanChangeDirection())
    {
        uint32_t dwPickedActorID;
        TPixelPosition kPPosPickedGround;

        if (pkInstTarget && pkInstTarget != pkInstMain)
        {
            pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);
        }
        else if (__GetPickedActorID(&dwPickedActorID))
        {
            CInstanceBase *pkInstVictim = NEW_FindActorPtr(dwPickedActorID);
            if (pkInstVictim)
                pkInstMain->NEW_LookAtDestInstance(*pkInstVictim);
        }
        else if (__GetPickedGroundPos(&kPPosPickedGround))
        {
            pkInstMain->NEW_LookAtDestPixelPosition(kPPosPickedGround);
        }
        else
        {
            SPDLOG_DEBUG("CPythonPlayer::__UseSkill({0}) - 화면 기준 방향 설정을 해야함", dwSlotIndex);
        }
    }

    // 관격술 처리
    uint32_t dwTargetMaxCount = pSkillData->GetTargetCount(rkSkillInst.fcurEfficientPercentage);
    uint32_t dwRange = __GetSkillTargetRange(*pSkillData);
    if (dwTargetMaxCount > 0 && pkInstTarget)
    {
        uint32_t dwTargetCount = 1;
        std::vector<CInstanceBase *> kVct_pkInstTarget;

        if (pSkillData->IsFanRange())
        {
            if (pkInstMain->NEW_GetInstanceVectorInFanRange(float(dwRange), *pkInstTarget, &kVct_pkInstTarget))
            {
                std::vector<CInstanceBase *>::iterator i;
                for (i = kVct_pkInstTarget.begin(); i != kVct_pkInstTarget.end(); ++i)
                {
                    if (dwTargetCount >= dwTargetMaxCount)
                        break;

                    CInstanceBase *pkInstEach = *i;

                    if (pkInstTarget != pkInstEach && !pkInstEach->IsDead())
                    {
                        pkInstMain->AddFlyTargetInstance(*pkInstEach);
                        gPythonNetworkStream->SendAddFlyTargetingPacket(pkInstEach->GetVirtualID(),
                                                                        pkInstEach->GetGraphicThingInstanceRef().
                                                                        OnGetFlyTargetPosition());

                        dwTargetCount++;
                    }
                }
            }
        }
        else if (pSkillData->IsCircleRange())
        {
            if (pkInstMain->NEW_GetInstanceVectorInCircleRange(float(dwRange), &kVct_pkInstTarget))
            {
                std::vector<CInstanceBase *>::iterator i;
                for (i = kVct_pkInstTarget.begin(); i != kVct_pkInstTarget.end(); ++i)
                {
                    if (dwTargetCount >= dwTargetMaxCount)
                        break;

                    CInstanceBase *pkInstEach = *i;

                    if (pkInstTarget != pkInstEach && !pkInstEach->IsDead())
                    {
                        pkInstMain->AddFlyTargetInstance(*pkInstEach);
                        gPythonNetworkStream->SendAddFlyTargetingPacket(pkInstEach->GetVirtualID(),
                                                                        pkInstEach->GetGraphicThingInstanceRef().
                                                                        OnGetFlyTargetPosition());

                        dwTargetCount++;
                    }
                }
            }
        }

        if (dwTargetCount < dwTargetMaxCount)
        {
            while (dwTargetCount < dwTargetMaxCount)
            {
                TPixelPosition kPPosDst;
                pkInstMain->NEW_GetRandomPositionInFanRange(*pkInstTarget, &kPPosDst);

                kPPosDst.x = kPPosDst.x;
                kPPosDst.y = -kPPosDst.y;

                pkInstMain->AddFlyTargetPosition(kPPosDst);
                gPythonNetworkStream->SendAddFlyTargetingPacket(0, kPPosDst);

                dwTargetCount++;
            }
        }
    }

    /////

    // NOTE : 멀리서 적을 클릭해놓고 스킬을 쓰면 스킬을 쓴뒤 바로 적을 공격하는 문제를 수정하기 위한 코드 - [levites]
    __ClearReservedAction();
    /////

    if (!pSkillData->IsNoMotion())
    {
        uint32_t dwMotionIndex = pSkillData->GetSkillMotionIndex(rkSkillInst.iGrade);
        uint32_t dwLoopCount = pSkillData->GetMotionLoopCount(rkSkillInst.fcurEfficientPercentage);
        if (!pkInstMain->NEW_UseSkill(rkSkillInst.dwIndex, dwMotionIndex, dwLoopCount,
                                      pSkillData->IsMovingSkill() ? true : false,
                                      rkSkillInst.color))

        {
            SPDLOG_DEBUG("CPythonPlayer::__UseSkill({0}) - pkInstMain->NEW_UseSkill - ERROR", dwSlotIndex);
            return false;
        }
    }

    uint32_t dwTargetVID = pkInstTarget ? pkInstTarget->GetVirtualID() : 0;

    __SendUseSkill(dwSlotIndex, dwTargetVID);
    return true;
}

void CPythonPlayer::__SendUseSkill(uint32_t dwSkillSlotIndex, uint32_t dwTargetVID)
{
    TSkillInstance &rkSkillInst = m_Skill[dwSkillSlotIndex];

    gPythonNetworkStream->SendUseSkillPacket(rkSkillInst.dwIndex, dwTargetVID);

    __RunCoolTime(dwSkillSlotIndex);
}

uint8_t CPythonPlayer::__GetSkillType(uint32_t dwSkillSlotIndex)
{
    TSkillInstance &rkSkillInst = m_Skill[dwSkillSlotIndex];

    CPythonSkill::TSkillData *pkSkillData;
    CPythonSkill &rkPythonSkill = CPythonSkill::Instance();
    if (!rkPythonSkill.GetSkillData(rkSkillInst.dwIndex, &pkSkillData))
    {
        //SPDLOG_ERROR("CPythonPlayer::__GetSkillType(dwSkillSlotIndex=%d) - NOT CHECK", dwSkillSlotIndex);
        return 0;
    }
    return pkSkillData->GetType();
}

void CPythonPlayer::__RunCoolTime(uint32_t dwSkillSlotIndex)
{
    TSkillInstance &rkSkillInst = m_Skill[dwSkillSlotIndex];

    CPythonSkill::TSkillData *pkSkillData;
    if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pkSkillData))
    {
        SPDLOG_ERROR("CPythonPlayer::__SendUseSkill(dwSkillSlotIndex={0}) - NOT CHECK", dwSkillSlotIndex);
        return;
    }

    CPythonSkill::TSkillData &rkSkillData = *pkSkillData;

    rkSkillInst.fCoolTime = rkSkillData.GetSkillCoolTime(rkSkillInst.fcurEfficientPercentage);
    rkSkillInst.fLastUsedTime = DX::StepTimer::Instance().GetTotalSeconds();

    m_ppyGameWindow.attr("RunUseSkillEvent")(dwSkillSlotIndex, rkSkillInst.fCoolTime);
}

bool CPythonPlayer::__HasEnoughArrow() const
{
    const auto itemData = CItemManager::instance().GetProto(GetItemIndex({EQUIPMENT, WEAR_ARROW}));
    if (itemData)
    {
        return ITEM_WEAPON == itemData->GetType() && (WEAPON_ARROW == itemData->GetSubType()
                                                      || WEAPON_QUIVER == itemData->GetSubType());
    }

    return false;
}

bool CPythonPlayer::__HasItem(uint32_t dwItemID)
{
    for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
    {
        if (dwItemID == GetItemIndex(TItemPos(INVENTORY, i)))
            return true;
    }

    for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
    {
        if (dwItemID == GetItemIndex(TItemPos(DRAGON_SOUL_INVENTORY, i)))
            return true;
    }

    return false;
}

extern const uint32_t c_iSkillIndex_Tongsol;
extern const uint32_t c_iSkillIndex_Fishing;
extern const uint32_t c_iSkillIndex_Mining;
extern const uint32_t c_iSkillIndex_Making;
extern const uint32_t c_iSkillIndex_Combo;
extern const uint32_t c_iSkillIndex_Language1;
extern const uint32_t c_iSkillIndex_Language2;
extern const uint32_t c_iSkillIndex_Language3;
extern const uint32_t c_iSkillIndex_Polymorph;

void CPythonPlayer::UseGuildSkill(uint32_t dwSkillSlotIndex)
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;
    if (!pkInstMain->CanUseSkill())
        return;

    ///////////////////////////////////////////////////////////////////////////////////////////////


    TSkillInstance &rkSkillInst = m_Skill[dwSkillSlotIndex];

    uint32_t dwSkillIndex = rkSkillInst.dwIndex;

    CPythonSkill::TSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
        return;

    if (__CheckRestSkillCoolTime(dwSkillSlotIndex))
    {
        m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "WAIT_COOLTIME");
        return;
    }

    if (pSkillData->IsOnlyForGuildWar())
    {
        if (!CPythonGuild::Instance().IsDoingGuildWar())
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "ONLY_FOR_GUILD_WAR");
            return;
        }
    }

    uint32_t dwMotionIndex = pSkillData->GetSkillMotionIndex();
    if (!pkInstMain->NEW_UseSkill(dwSkillIndex, dwMotionIndex, 1, false, rkSkillInst.color))

    {
        SPDLOG_DEBUG("CPythonPlayer::UseGuildSkill({0}) - pkInstMain->NEW_UseSkill - ERROR", dwSkillIndex);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////

    gPythonNetworkStream->SendGuildUseSkillPacket(dwSkillIndex, 0);
    __RunCoolTime(dwSkillSlotIndex);
}

void CPythonPlayer::SetComboSkillFlag(bool bFlag)
{
    uint32_t dwSlotIndex;
    if (!GetSkillSlotIndex(c_iSkillIndex_Combo, &dwSlotIndex))
    {
        SPDLOG_DEBUG("CPythonPlayer::SetComboSkillFlag(killIndex={0}) - Can't Find Slot Index\n", c_iSkillIndex_Combo);
        return;
    }

    if (bFlag)
    {
        ActivateSkillSlot(dwSlotIndex);
    }
    else
    {
        DeactivateSkillSlot(dwSlotIndex);
    }

    int iLevel = GetSkillLevel(dwSlotIndex);
    if (iLevel <= 0)
    {
        SPDLOG_DEBUG("CPythonPlayer::SetComboSkillFlag(skillIndex={0}, skillLevel={1}) - Invalid Combo Skill Level\n",
                     c_iSkillIndex_Combo, iLevel);
        return;
    }

    iLevel = std::min(iLevel, 2);

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    if (bFlag)
    {
        pkInstMain->SetComboType(iLevel);
    }
    else
    {
        pkInstMain->SetComboType(0);
    }
}

bool CPythonPlayer::__CheckSpecialSkill(uint32_t dwSkillIndex)
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    // Fishing
    if (c_iSkillIndex_Fishing == dwSkillIndex)
    {
        if (pkInstMain->IsFishingMode())
        {
            NEW_Fishing();
        }
        else
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "EQUIP_FISHING_ROD");
        }
        return true;
    }

        // Combo
    else if (c_iSkillIndex_Combo == dwSkillIndex)
    {
        uint32_t dwSlotIndex;
        if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
            return false;

        int iLevel = GetSkillLevel(dwSlotIndex);
        if (iLevel > 0)
        {
            gPythonNetworkStream->SendUseSkillPacket(dwSkillIndex);
        }
        else
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NOT_YET_LEARN");
        }

        return true;
    }

    return false;
}
