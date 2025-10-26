#include "char.h"
#include "char_manager.h"

#include "config.h"
#include "utils.h"
#include "vector.h"

#include <game/AffectConstants.hpp>
#include <game/GamePacket.hpp>

#include "motion.h"
#include "party.h"
#include "questmanager.h"

#include "BlueDragon.h"
#include "exchange.h"
#include "guild_manager.h"
#include "item_manager.h"
#include "main.h"
#include "mob_manager.h"
#include "sectree_manager.h"
#include "war_map.h"
#include "xmas_event.h"

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

#include "guild.h"
#include "skill.h"

#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>

extern CHARACTER* FindVictim(CHARACTER* pkChr, int iMaxDistance);
#ifdef __FAKE_PC__
extern CHARACTER* FindFakePCVictim(CHARACTER* pkChr);
#endif

namespace
{
class FuncFindChrForFlag
{
    public:
    FuncFindChrForFlag(CHARACTER* pkChr)
        : m_pkChr(pkChr)
        , m_pkChrFind(nullptr)
        , m_iMinDistance(INT_MAX)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        if (ent->IsObserverMode())
            return;

        CHARACTER* pkChr = (CHARACTER*)ent;

        if (!pkChr->IsPC())
            return;

        if (!pkChr->GetGuild())
            return;

        if (pkChr->IsDead())
            return;

        int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkChr->GetX(),
                                    pkChr->GetY() - m_pkChr->GetY());

        if (iDist <= 500 && m_iMinDistance > iDist &&
            !pkChr->FindAffect(AFFECT_WAR_FLAG_1) &&
            !pkChr->FindAffect(AFFECT_WAR_FLAG_2) &&
            !pkChr->FindAffect(AFFECT_WAR_FLAG_3)) {
            // 우리편 깃발일 경우
            if ((uint32_t)m_pkChr->GetPoint(POINT_STAT) ==
                pkChr->GetGuild()->GetID()) {
                CWarMap* pMap = pkChr->GetWarMap();
                uint8_t idx;

                if (!pMap ||
                    !pMap->GetTeamIndex(pkChr->GetGuild()->GetID(), idx))
                    return;

                // 우리편 기지에 깃발이 없을 때만 깃발을 뽑는다. 안그러면 기지에
                // 있는 깃발을 가만히 두고 싶은데도 뽑힐수가 있으므로..
                if (!pMap->IsFlagOnBase(idx)) {
                    m_pkChrFind = pkChr;
                    m_iMinDistance = iDist;
                }
            } else {
                // 상대편 깃발인 경우 무조건 뽑는다.
                m_pkChrFind = pkChr;
                m_iMinDistance = iDist;
            }
        }
    }

    CHARACTER* m_pkChr;
    CHARACTER* m_pkChrFind;
    int m_iMinDistance;
};

class FuncFindChrForFlagBase
{
    public:
    FuncFindChrForFlagBase(CHARACTER* pkChr)
        : m_pkChr(pkChr)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        if (ent->IsObserverMode())
            return;

        CHARACTER* pkChr = (CHARACTER*)ent;

        if (!pkChr->IsPC())
            return;

        CGuild* pkGuild = pkChr->GetGuild();

        if (!pkGuild)
            return;

        int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkChr->GetX(),
                                    pkChr->GetY() - m_pkChr->GetY());

        if (iDist > 500)
            return;

        auto aff = pkChr->FindAffect(AFFECT_WAR_FLAG_1);
        if (!aff)
            aff = pkChr->FindAffect(AFFECT_WAR_FLAG_2);
        if (!aff)
            aff = pkChr->FindAffect(AFFECT_WAR_FLAG_3);

        if (!aff)
            return;

        if ((uint32_t)m_pkChr->GetPoint(POINT_STAT) == pkGuild->GetID() &&
            m_pkChr->GetPoint(POINT_STAT) != aff->pointValue) {
            CWarMap* pMap = pkChr->GetWarMap();
            uint8_t idx;

            if (!pMap || !pMap->GetTeamIndex(pkGuild->GetID(), idx))
                return;

            // if (pMap->IsFlagOnBase(idx))
            {
                uint8_t idx_opp = idx == 0 ? 1 : 0;

                SendGuildWarScore(m_pkChr->GetPoint(POINT_STAT),
                                  aff->pointValue, 1);

                pMap->ResetFlag();

                pMap->Notice("%s 길드가 %s 길드의 깃발을 빼앗았습니다!",
                             pMap->GetGuild(idx)->GetName(),
                             pMap->GetGuild(idx_opp)->GetName());
            }
        }
    }

    CHARACTER* m_pkChr;
};

class FuncFindGuardVictim
{
    public:
    FuncFindGuardVictim(CHARACTER* pkChr, int iMaxDistance)
        : m_pkChr(pkChr)
        , m_iMinDistance(INT_MAX)
        , m_iMaxDistance(iMaxDistance)
        , m_lx(pkChr->GetX())
        , m_ly(pkChr->GetY())
        , m_pkChrVictim(nullptr){};

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER* pkChr = (CHARACTER*)ent;

        // 일단 PC 공격안함
        if (pkChr->IsPC())
            return;

        if (pkChr->IsNPC() && !pkChr->IsMonster())
            return;

        if (pkChr->IsDead())
            return;

        if (pkChr->FindAffect(SKILL_EUNHYUNG) ||
            pkChr->FindAffect(AFFECT_INVISIBILITY) ||
            pkChr->FindAffect(AFFECT_REVIVE_INVISIBLE))
            return;

        // 왜구는 패스
        if (pkChr->GetRaceNum() == 5001)
            return;

#ifdef __FAKE_PC__
        if (pkChr->FakePC_Check())
            return;
#endif

        int iDistance =
            DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

        if (iDistance < m_iMinDistance && iDistance <= m_iMaxDistance) {
            m_pkChrVictim = pkChr;
            m_iMinDistance = iDistance;
        }
    }

    CHARACTER* GetVictim() { return (m_pkChrVictim); }

    private:
    CHARACTER* m_pkChr;

    int m_iMinDistance;
    int m_iMaxDistance;
    long m_lx;
    long m_ly;

    CHARACTER* m_pkChrVictim;
};
} // namespace

// STATE_IDLE_REFACTORING
void CHARACTER::StateIdle()
{
    if (IsStone()) {
        __StateIdle_Stone();
        return;
    }

    if (IsWarp() || IsGoto()) {
        m_dwStateDuration = THECORE_SECS_TO_PASSES(60);
        return;
    }

    if (IsPC())
        return;

    if (!IsMonster()) {
        __StateIdle_NPC();
        return;
    }

    __StateIdle_Monster();
}

void CHARACTER::__StateIdle_Stone()
{
    m_dwStateDuration = THECORE_SECS_TO_PASSES(1);

    uint64_t iPercent = static_cast<uint64_t>(GetHP() * 100) / GetMobTable().dwMaxHP;
    uint32_t dwVnum = Random::get(
        std::min(GetMobTable().sAttackSpeed, GetMobTable().sMovingSpeed),
        std::max(GetMobTable().sAttackSpeed, GetMobTable().sMovingSpeed));

    MonsterChat(fmt::format("Im at {0}%", iPercent));

    if (iPercent <= 10 && GetMaxSP() < 10) {
        SetMaxSP(10);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 100,
                                   GetY() - 100, GetX() + 100, GetY() + 100,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 600,
                                   GetY() - 600, GetX() + 600, GetY() + 600,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 20 && GetMaxSP() < 9) {
        SetMaxSP(9);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 100,
                                   GetY() - 100, GetX() + 100, GetY() + 100,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 600,
                                   GetY() - 600, GetX() + 600, GetY() + 600,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 30 && GetMaxSP() < 8) {
        SetMaxSP(8);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 100,
                                   GetY() - 100, GetX() + 100, GetY() + 100,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 40 && GetMaxSP() < 7) {
        SetMaxSP(7);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 50 && GetMaxSP() < 6) {
        SetMaxSP(6);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 60 && GetMaxSP() < 5) {
        SetMaxSP(5);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 100,
                                   GetY() - 100, GetX() + 100, GetY() + 100,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 70 && GetMaxSP() < 4) {
        SetMaxSP(4);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 100,
                                   GetY() - 100, GetX() + 100, GetY() + 100,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 80 && GetMaxSP() < 3) {
        SetMaxSP(3);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 90 && GetMaxSP() < 2) {
        SetMaxSP(2);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 100,
                                   GetY() - 100, GetX() + 100, GetY() + 100,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    } else if (iPercent <= 99 && GetMaxSP() < 1) {
        SetMaxSP(1);
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);
        g_pCharManager->SelectStone(this);
        g_pCharManager->SpawnGroup(dwVnum, GetMapIndex(), GetX() - 400,
                                   GetY() - 400, GetX() + 400, GetY() + 400,
                                   nullptr, false, nullptr, false, GetVictim());
        g_pCharManager->SelectStone(nullptr);
    }

    UpdatePacket();
}

void CHARACTER::__StateIdle_NPC()
{
    MonsterChat(MONSTER_CHAT_WAIT);
    m_dwStateDuration = THECORE_SECS_TO_PASSES(5);

    // 펫 시스템의 Idle 처리는 기존 거의 모든 종류의 캐릭터들이 공유해서
    // 사용하는 상태머신이 아닌 CPetActor::Update에서 처리함.
    if (IsPet())
        return;

    if (IsGuardNPC()) {
        if (!quest::CQuestManager::instance().GetEventFlag("noguard")) {
            FuncFindGuardVictim f(this, 50000);
            ForEachSeen(f);

            CHARACTER* victim = f.GetVictim();

            if (victim) {
                m_dwStateDuration = THECORE_SECS_TO_PASSES(1) / 2;

                if (CanBeginFight())
                    BeginFight(victim);
            }
        }

        return;
    }

    // CHAOS DUNGEON
    if (GetRaceNum() == 20415) // Chaos Portal time
    {
        if (get_dword_time() > m_dwPlayStartTime) {
            M2_DESTROY_CHARACTER(this);
            return;
        }
    }
    if (!IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE)) {
        //
        // 이 곳 저 곳 이동한다.
        //
        CHARACTER* pkChrProtege = GetProtege();

        if (pkChrProtege) {
            if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(),
                                GetY() - pkChrProtege->GetY()) > 500) {
                if (Follow(pkChrProtege, (float)Random::get(100, 300)))
                    return;
            }
        }

        if (!Random::get(0, 6)) {
            SetRotation((float)Random::get(0, 359)); // 방향은 랜덤으로 설정

            float fx, fy;
            float fDist = (float)Random::get(200, 400);

            GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

            // 느슨한 못감 속성 체크; 최종 위치와 중간 위치가 갈수없다면 가지
            // 않는다.
            if (!(SECTREE_MANAGER::instance().IsMovablePosition(
                      GetMapIndex(), GetX() + (int)fx, GetY() + (int)fy) &&
                  SECTREE_MANAGER::instance().IsMovablePosition(
                      GetMapIndex(), GetX() + (int)fx / 2,
                      GetY() + (int)fy / 2)))
                return;

            SetNowWalking(true);

            Goto(GetX() + (int)fx, GetY() + (int)fy);
            return;
        }
    }
}

void CHARACTER::__StateIdle_Monster()
{
    if (IsStun())
        return;

    if (!CanMove())
        return;

    if (IsCoward()) {
        // 겁쟁이 몬스터는 도망만 다닙니다.
        if (!IsDead())
            CowardEscape();

        return;
    }

    if (IsBerserker())
        if (IsBerserk())
            SetBerserk(false);

    if (IsGodSpeeder())
        if (IsGodSpeed())
            SetGodSpeed(false);

    CHARACTER* victim = GetVictim();

    if (!victim || victim->IsDead()) {
        SetVictim(NULL);
        victim = NULL;
        m_dwStateDuration = THECORE_SECS_TO_PASSES(1);
    }

    if (!victim || victim->IsBuilding()) {
        // 돌 보호 처리
        if (m_pkChrStone) {
            victim = m_pkChrStone->GetNearestVictim(m_pkChrStone);
        }
        // 선공 몬스터 처리
        else if (IsAggressive()) {
            if (GetMapIndex() == 61 &&
                quest::CQuestManager::instance().GetEventFlag("xmas_tree")) {

            } else {

#ifdef __MELEY_LAIR_DUNGEON__
                if (GetRaceNum() == MeleyLair::BOSS_VNUM)
                    victim = FindVictim(this, 40000);
                else
                    victim = FindVictim(this, m_pkMobData->wAggressiveSight);
#else
                victim = FindVictim(this, m_pkMobData->wAggressiveSight);
#endif
            }
        }
    }

    if (victim && !victim->IsDead()) {
        if (CanBeginFight())
            BeginFight(victim);

        return;
    }

    if (IsAggressive() && !victim)
        m_dwStateDuration = THECORE_SECS_TO_PASSES(Random::get(1, 3));
    else
        m_dwStateDuration = THECORE_SECS_TO_PASSES(Random::get(3, 5));

    CHARACTER* pkChrProtege = GetProtege();

    // 보호할 것(돌, 파티장)에게로 부터 멀다면 따라간다.
    if (pkChrProtege) {
        if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(),
                            GetY() - pkChrProtege->GetY()) > 1000) {
            if (Follow(pkChrProtege, (float)Random::get(150, 400))) {
                MonsterLog("[IDLE] Following protege.");
                return;
            }
        }
    }

    //
    // 그냥 왔다리 갔다리 한다.
    //
    if (!IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE)) {
        if (!Random::get(0, 6)) {
            SetRotation((float)Random::get(0, 359)); // 방향은 랜덤으로 설정

            float fx, fy;
            float fDist = (float)Random::get(300, 700);

            GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

            // 느슨한 못감 속성 체크; 최종 위치와 중간 위치가 갈수없다면 가지
            // 않는다.
            if (!(SECTREE_MANAGER::instance().IsMovablePosition(
                      GetMapIndex(), GetX() + (int)fx, GetY() + (int)fy) &&
                  SECTREE_MANAGER::instance().IsMovablePosition(
                      GetMapIndex(), GetX() + (int)fx / 2,
                      GetY() + (int)fy / 2)))
                return;

            Goto(GetX() + (int)fx, GetY() + (int)fy);
            return;
        }
    }

    MonsterChat(MONSTER_CHAT_WAIT);
}
// END_OF_STATE_IDLE_REFACTORING

bool __CHARACTER_GotoNearTarget(CHARACTER* self, CHARACTER* victim)
{
    if (IS_SET(self->GetAIFlag(), AIFLAG_NOMOVE))
        return false;

    switch (self->GetMobBattleType()) {
        case BATTLE_TYPE_RANGE:
        case BATTLE_TYPE_MAGIC:
            if (self->Follow(victim, self->GetMobAttackRange() * 8 / 10))
                return true;
            break;

        default:
            if (self->Follow(victim, self->GetMobAttackRange() * 9 / 10))
                return true;
    }

    return false;
}

void CHARACTER::StateMove()
{

    const auto newPos = GetInterpolatedPosition(get_dword_time());
    const bool finished = newPos.x == m_posDest.x && newPos.y == m_posDest.y;

    Move(newPos.x, newPos.y);

    // GcEffectPacket pack;
    // pack.header = HEADER_GC_EFFECT;
    // storm::CopyStringSafe(pack.filename, "D:/ymir work/effect/etc/!.mse");
    // pack.x = newPos.x;
    // pack.y = newPos.y;
    // PacketView(&pack, sizeof(pack));
    if (IsPC() && (thecore_pulse() & 15) == 0) {
        UpdateSectree();

        if (GetExchange()) {
            CHARACTER* victim = GetExchange()->GetCompany()->GetOwner();
            int iDist = DISTANCE_APPROX(GetX() - victim->GetX(),
                                        GetY() - victim->GetY());

            if (iDist >= EXCHANGE_MAX_DISTANCE) {
                GetExchange()->Cancel();
            }
        }

        if (GetExchange()) {
            CHARACTER* victim = GetExchange()->GetCompany()->GetOwner();
            int iDist = DISTANCE_APPROX(GetX() - victim->GetX(),
                                        GetY() - victim->GetY());

            // 거리 체크
            if (iDist >= EXCHANGE_MAX_DISTANCE) {
                GetExchange()->Cancel();
            }
        }
    }

    if (IsPC()) {
        if (IsWalking() && GetStamina() < GetMaxStamina()) {
            // 5초 후 부터 스테미너 증가
            if (get_dword_time() - GetWalkStartTime() > 5000)
                PointChange(POINT_STAMINA, GetMaxStamina() / 1);
        }

        // 전투 중이면서 뛰는 중이면
        if (!IsWalking() && !IsRiding()) {
            if ((get_dword_time() - GetLastAttackTime()) < 20000) {

                StartAffectEvent();

                if (thecore_pulse() & 1)
                    PointChange(POINT_STAMINA, -STAMINA_PER_STEP);

                StartStaminaConsume();

                if (GetStamina() <= 0) {
                    // 스테미나가 모자라 걸어야함
                    SetStamina(0);
                    SetNowWalking(true);
                    StopStaminaConsume();
                }

            } else if (IsStaminaConsume()) {
                StopStaminaConsume();
            }
        }
    } else {

        // XXX AGGRO
        if (IsMonster() && GetVictim()) {
            CHARACTER* victim = GetVictim();
            UpdateAggrPoint(victim, DAMAGE_TYPE_NORMAL,
                            -(victim->GetLevel() / 3 + 1));

            if (gConfig.testServer) {
                // 몬스터가 적을 쫓아가는 것이면 무조건 뛰어간다.
                SetNowWalking(false);
            }
        }

        if (IsMonster() && GetMobRank() >= MOB_RANK_BOSS && GetVictim()) {
            CHARACTER* victim = GetVictim();

            if (GetRaceNum() == 2191 && Random::get(1, 20) == 1 &&
                get_dword_time() - m_pkMobInst->m_dwLastWarpTime > 1000) {
                float fx, fy;
                GetDeltaByDegree(victim->GetRotation(), 400, &fx, &fy);
                int32_t new_x = victim->GetX() + (int32_t)fx;
                int32_t new_y = victim->GetY() + (int32_t)fy;
                // Can't teleport if it'll end up into safezone
                SECTREE* sectree = GetSectree();
                if (sectree && !sectree->IsAttr(new_x, new_y, ATTR_BANPK)) {

                    SetRotation(GetDegreeFromPositionXY(
                        new_x, new_y, victim->GetX(), victim->GetY()));
                    Show(victim->GetMapIndex(), new_x, new_y, 0, true);
                    GotoState(m_stateBattle);
                    m_dwStateDuration = 1;
                    ResetMobSkillCooltime();
                    m_pkMobInst->m_dwLastWarpTime = get_dword_time();
                    return;
                }
            }

            if (Random::get(0, 3) == 0) {
                if (__CHARACTER_GotoNearTarget(this, victim))
                    return;
            }
        }
    }

    if (finished) {
        if (IsPC()) {
            SPDLOG_DEBUG("Arrive {0} {1} {2}", GetName(), newPos.x, newPos.y);
            GotoState(m_stateIdle);
            StopStaminaConsume();
        } else {
            if (GetVictim() && !IsCoward()) {
                if (!IsState(m_stateBattle))
                    MonsterLog("[BATTLE] I came near and started attacking %s",
                               GetVictim()->GetName());

                GotoState(m_stateBattle);
                m_dwStateDuration = 1;
            } else {
                if (!IsState(m_stateIdle))
                    MonsterLog("[IDLE] I do not have an objective.");

                GotoState(m_stateIdle);

                m_dwStateDuration = THECORE_SECS_TO_PASSES(Random::get(1, 3));
            }
        }
    }
}

#ifdef ENABLE_MELEY_LAIR_DUNGEON

// Meley Test
int Meley_StateBattle(CHARACTER* pChar)
{

    if (gConfig.testServer)
        SPDLOG_ERROR("Meley_StateBattle start 2");

    const int SkillCount = 3;
    int SkillPriority[SkillCount] = {};

    uint32_t dungeonStep = 0;
    auto dungeon = pChar->GetMeley();
    if (dungeon)
        dungeonStep = dungeon->GetDungeonStep();

    switch (dungeonStep) {
        case 1:
            SkillPriority[0] = 0;
            SkillPriority[1] = 1;
            SkillPriority[2] = 2;
            break;
        case 2:
            SkillPriority[0] = 2;
            SkillPriority[1] = 1;
            SkillPriority[2] = 0;
            break;
        case 3:
            SkillPriority[0] = 0;
            SkillPriority[1] = 2;
            SkillPriority[2] = 1;
            break;
        default:
            break;
    }

    for (int i = 0; i < SkillCount; ++i) {
        const int SkillIndex = SkillPriority[i];

        if (Random::get(0, 1)) {
            PIXEL_POSITION pos = MeleyLair::CMgr::instance().GetXYZ();
            if (pos.x)
                pChar->SetRotationToXY(130 * 100, 130 * 100);

            auto key = MakeMotionKey(MOTION_MODE_GENERAL,
                                     MOTION_SPECIAL_1 + SkillIndex);

            auto timeNow = get_dword_time();
            if (pChar->UseMobSkill(SkillIndex)) {

                pChar->SendMovePacket(FUNC_MOB_SKILL, SkillIndex, pChar->GetX(),
                                      pChar->GetY(), 0, timeNow);
            }

            auto motion = GetMotionManager().Get(pChar->GetRaceNum(), key);
            uint32_t duration = motion ? motion->duration : 2000;

            if (gConfig.testServer)
                SPDLOG_ERROR("Meley_StateBattle skill_idx %u", SkillIndex);

            return 0 == duration ? THECORE_SECS_TO_PASSES(1)
                                 : THECORE_MSECS_TO_PASSES(duration);
        }
    }

    return THECORE_SECS_TO_PASSES(1);
}
#endif

void CHARACTER::StateBattle()
{
    if (IsStone()) {
        SPDLOG_ERROR("Stone must not use battle state (name {0})", GetName());
        return;
    }

    if (IsPC())
        return;

    if (!CanMove())
        return;

    if (IsStun())
        return;

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if (GetRaceNum() == (MeleyLair::BOSS_VNUM)) {
        // Meley Special Battle State
        SPDLOG_ERROR("Meley_StateBattle start");
        m_dwStateDuration = Meley_StateBattle(this);
        return;
    }
#endif

    CHARACTER* victim = GetVictim();

    if (IsCoward()) {
        if (IsDead())
            return;

        SetVictim(NULL);

        if (Random::get(1, 50) != 1) {
            GotoState(m_stateIdle);
            m_dwStateDuration = 1;
        } else
            CowardEscape();

        return;
    }

    if (!victim || (victim->IsStun() && IsGuardNPC()) || victim->IsDead()) {
        if (victim && victim->IsDead() && IsAggressive() &&
            (!GetParty() || GetParty()->GetLeader() == this)) {

            CHARACTER* new_victim =
                FindVictim(this, GetRaceNum() != (MeleyLair::BOSS_VNUM)
                                     ? m_pkMobData->wAggressiveSight
                                     : 40000);

            SetVictim(new_victim);
            m_dwStateDuration = THECORE_SECS_TO_PASSES(1);

            if (!new_victim) {
                switch (GetMobBattleType()) {
                    case BATTLE_TYPE_MELEE:
                    case BATTLE_TYPE_SUPER_POWER:
                    case BATTLE_TYPE_SUPER_TANKER:
                    case BATTLE_TYPE_POWER:
                    case BATTLE_TYPE_TANKER: {
                        float fx, fy;
                        float fDist = Random::get(400, 1500);

                        GetDeltaByDegree(Random::get(0, 359), fDist, &fx, &fy);

                        if (SECTREE_MANAGER::instance().IsMovablePosition(
                                victim->GetMapIndex(), victim->GetX() + (int)fx,
                                victim->GetY() + (int)fy) &&
                            SECTREE_MANAGER::instance().IsMovablePosition(
                                victim->GetMapIndex(),
                                victim->GetX() + (int)fx / 2,
                                victim->GetY() + (int)fy / 2)) {
                            float dx = victim->GetX() + fx;
                            float dy = victim->GetY() + fy;

                            SetRotation(GetDegreeFromPosition(dx, dy));

                            if (Goto((int32_t)dx, (int32_t)dy))
                                SPDLOG_INFO("KILL_AND_GO: {0} distance {1}",
                                            GetName(), fDist);
                        }
                    }
                }
            }
            return;
        }

        SetVictim(NULL);

        if (IsGuardNPC())
            Return();

        m_dwStateDuration = THECORE_SECS_TO_PASSES(1);
        return;
    }

    if (CanSummonMonster() && !IsDead() && !IsStun()) {
        CParty* pParty = GetParty();
        const auto leader = pParty->GetLeaderCharacter();

        bool bPct = !Random::get(0, 3);

        if (bPct &&
            pParty->CountMemberByVnum(GetSummonVnum()) < SUMMON_MONSTER_COUNT) {

            MonsterLog("Summoning party!");
            int sx = GetX() - 300;
            int sy = GetY() - 300;
            int ex = GetX() + 300;
            int ey = GetY() + 300;

            CHARACTER* tch = nullptr;

            if (leader && leader->IsPet()) {
                tch = CHARACTER_MANAGER::instance().SpawnMobRange(
                    GetSummonVnum(), GetMapIndex(), sx, sy, ex, ey, true, true,
                    leader->GetScale());
            } else {
                tch = CHARACTER_MANAGER::instance().SpawnMobRange(
                    GetSummonVnum(), GetMapIndex(), sx, sy, ex, ey, true, true);
            }

            if (tch) {
                pParty->Join(tch->GetVID());
                pParty->Link(tch);

                if (leader) {
                    if (leader->IsPet()) {
                        tch->SetPetPawn();
                        tch->SetAttackMob();
                        tch->SetNoAttackChunjo();
                        tch->SetNoAttackShinsu();
                        tch->SetNoAttackJinno();
                        tch->SetScale(leader->GetScale());
                        tch->UpdatePacket();
                        tch->SetAggressive();
                    }
                }
            }
        }
    }

    CHARACTER* pkChrProtege = GetProtege();

    float fDist =
        DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

    if (fDist >= 4000.0f) {
#ifdef ENABLE_MELEY_LAIR_DUNGEON
        bool bPass = true;
        if (GetRaceNum() == (MeleyLair::BOSS_VNUM) && (fDist < 32000.0f))
            bPass = false;

        if (bPass) {
            MonsterLog("Abandoned because of a target too far");
            SetVictim(NULL);
            if (pkChrProtege)
                if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(),
                                    GetY() - pkChrProtege->GetY()) > 1000)
                    Follow(pkChrProtege, Random::get(150, 400));

            return;
        }
#else
        MonsterLog("Abandoned because of a target too far");
        SetVictim(NULL);
        if (pkChrProtege)
            if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(),
                                GetY() - pkChrProtege->GetY()) > 1000)
                Follow(pkChrProtege, Random::get(150, 400));

        return;
#endif
    }

    if (fDist >= GetMobAttackRange() * 1.15) {
        __CHARACTER_GotoNearTarget(this, victim);
        return;
    }

    if (m_pkParty)
        m_pkParty->SendMessage(this, PM_ATTACKED_BY, 0, 0);

    if (2493 == m_pkMobData->dwVnum) {
        m_dwStateDuration = BlueDragon_StateBattle(this);
        return;
    }

    uint32_t dwCurTime = get_dword_time();
    uint32_t dwDuration =
        CalculateDuration(GetLimitPoint(POINT_ATT_SPEED), 2000);

    if ((dwCurTime - m_dwLastAttackTime) < dwDuration) {
        m_dwStateDuration = std::max<int>(
            1, THECORE_SECS_TO_PASSES(dwDuration -
                                      (dwCurTime - m_dwLastAttackTime)) /
                   1000);
        return;
    }

    if (IsBerserker() == true)
        if (GetHPPct() < m_pkMobData->bBerserkPoint)
            if (IsBerserk() != true)
                SetBerserk(true);

    if (IsGodSpeeder() == true)
        if (GetHPPct() < m_pkMobData->bGodSpeedPoint)
            if (IsGodSpeed() != true)
                SetGodSpeed(true);

    if (HasMobSkill()) {
        for (unsigned int iSkillIdx = 0; iSkillIdx < MOB_SKILL_MAX_NUM;
             ++iSkillIdx) {
            if (CanUseMobSkill(iSkillIdx)) {
                SetRotationToXY(victim->GetX(), victim->GetY());

                if (UseMobSkill(iSkillIdx)) {
                    auto key = MakeMotionKey(MOTION_MODE_GENERAL,
                                             MOTION_SPECIAL_1 + iSkillIdx);
                    SendMovePacket(FUNC_MOB_SKILL, iSkillIdx, GetX(), GetY(), 0,
                                   dwCurTime);

                    auto motion = GetMotionManager().Get(GetRaceNum(), key);
                    uint32_t duration = motion ? motion->duration : 2000;
                    m_dwStateDuration = THECORE_MSECS_TO_PASSES(duration);

                    SPDLOG_DEBUG("USE_MOB_SKILL: {0} idx {1} motion {2} "
                                  "duration {3}",
                                  GetName(), iSkillIdx,
                                  MOTION_SPECIAL_1 + iSkillIdx, duration);
                    return;
                }
            }
        }
    }

    if (!Attack(victim, 0, 0)) // If the attack fails? Why did it fail? TODO
        m_dwStateDuration = THECORE_SECS_TO_PASSES(1) / 2;
    else {
        SetRotationToXY(victim->GetX(), victim->GetY());
        SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

        auto key = MakeMotionKey(MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK);
        auto motion = GetMotionManager().Get(GetRaceNum(), key);

        uint32_t duration = motion ? motion->duration : 2000;
        m_dwStateDuration = THECORE_MSECS_TO_PASSES(duration);
    }
}

void CHARACTER::StateFlag()
{
    m_dwStateDuration = THECORE_SECS_TO_PASSES(1) / 2;

    CWarMap* pMap = GetWarMap();

    if (!pMap)
        return;

    FuncFindChrForFlag f(this);
    ForEachSeen(f);

    if (!f.m_pkChrFind)
        return;

    if (NULL == f.m_pkChrFind->GetGuild())
        return;

    char buf[256];
    uint8_t idx;

    if (!pMap->GetTeamIndex(GetPoint(POINT_STAT), idx))
        return;

    uint32_t type = AFFECT_WAR_FLAG_1 + idx;

    f.m_pkChrFind->AddAffect(type, POINT_NONE, GetPoint(POINT_STAT),
                             INFINITE_AFFECT_DURATION, 0, false);

    f.m_pkChrFind->AddAffect(type, POINT_MOV_SPEED,
                             50 - f.m_pkChrFind->GetPoint(POINT_MOV_SPEED),
                             INFINITE_AFFECT_DURATION, 0, false);

    pMap->RemoveFlag(idx);

    std::snprintf(buf, sizeof(buf),
                  LC_TEXT("%s 길드의 깃발을 %s 님이 획득하였습니다."),
                  pMap->GetGuild(idx)->GetName(), f.m_pkChrFind->GetName().c_str());
    pMap->Notice(buf);
}

void CHARACTER::StateFlagBase()
{
    m_dwStateDuration = (uint32_t)THECORE_MSECS_TO_PASSES(500);

    FuncFindChrForFlagBase f(this);
    ForEachSeen(f);
}

void CHARACTER::StateHorse()
{
    const float RESPAWN_DISTANCE = 4500.0f;
    const float START_FOLLOW_DISTANCE = 400.0f;
    const float START_RUN_DISTANCE = 700.0f;
    const int MIN_APPROACH = 150;
    const int MAX_APPROACH = 300;

    uint32_t STATE_DURATION = (uint32_t)THECORE_MSECS_TO_PASSES(500);

    bool bDoMoveAlone = true;
    bool bRun = true;

    if (IsDead())
        return;

    m_dwStateDuration = STATE_DURATION;

    CHARACTER* victim = GetRider();
    if (!victim) {
        M2_DESTROY_CHARACTER(this);
        return;
    }

    m_pkMobInst->m_posLastAttacked = GetXYZ();

    float fDist =
        DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

    if (fDist >= RESPAWN_DISTANCE) {
        float fOwnerRot = victim->GetRotation() *
                          boost::math::constants::pi<float>() / 180.0f;
        float fx = -MIN_APPROACH * cos(fOwnerRot);
        float fy = -MIN_APPROACH * sin(fOwnerRot);
        this->Show(victim->GetMapIndex(), victim->GetX() + fx,
                   victim->GetY() + fy);
    } else if (fDist >= START_FOLLOW_DISTANCE) {
        if (fDist > START_RUN_DISTANCE)
            SetNowWalking(!bRun);

        Follow(victim, Random::get(MIN_APPROACH, MAX_APPROACH));

        m_dwStateDuration = STATE_DURATION;
    } else if (bDoMoveAlone && (get_dword_time() > m_dwLastAttackTime)) {
        m_dwLastAttackTime = get_dword_time() + Random::get(5000, 12000);

        SetRotation(Random::get(0.0f, 359.0f));

        float fx, fy;
        float fRandDist = Random::get(200.0f, 400.0f);

        GetDeltaByDegree(GetRotation(), fRandDist, &fx, &fy);
        if (!(SECTREE_MANAGER::instance().IsMovablePosition(
                  GetMapIndex(), GetX() + (int)fx, GetY() + (int)fy) &&
              SECTREE_MANAGER::instance().IsMovablePosition(
                  GetMapIndex(), GetX() + (int)fx / 2, GetY() + (int)fy / 2)))
            return;

        SetNowWalking(true);
        Goto(GetX() + (int)fx, GetY() + (int)fy);
    }
}
