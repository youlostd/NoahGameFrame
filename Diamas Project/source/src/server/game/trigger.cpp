#include "utils.h"
#include "config.h"
#include "char.h"
#include "sectree_manager.h"
#include "GBattle.h"
#include <game/AffectConstants.hpp>
#include "shop_manager.h"
#include "skill.h"

int OnClickShop(TRIGGERPARAM);
int OnClickTalk(TRIGGERPARAM);

int OnIdleDefault(TRIGGERPARAM);
int OnAttackDefault(TRIGGERPARAM);

typedef struct STriggerFunction
{
    int (*func)(TRIGGERPARAM);
} TTriggerFunction;

TTriggerFunction OnClickTriggers[ON_CLICK_MAX_NUM] =
{
    {nullptr,},     // ON_CLICK_NONE,
    {OnClickShop,}, // ON_CLICK_SHOP,
    {nullptr,},     // ON_CLICK_TALK,
};

void CHARACTER::AssignTriggers(const TMobTable *table)
{
    if (table->bOnClickType >= ON_CLICK_MAX_NUM)
    {
        SPDLOG_ERROR("%s has invalid OnClick value %d", GetName(), table->bOnClickType);
        abort();
    }

    m_triggerOnClick.bType = table->bOnClickType;
    m_triggerOnClick.pFunc = OnClickTriggers[table->bOnClickType].func;
}

/*
 * ON_CLICK
 */
int OnClickShop(TRIGGERPARAM)
{
    CShopManager::instance().StartShopping(causer, ch);
    return 1;
}

/*
 * ¸ó½ºÅÍ AI ÇÔ¼öµéÀ» BattleAI Å¬·¡½º·Î ¼öÁ¤
 */
int OnIdleDefault(TRIGGERPARAM)
{
    if (ch->OnIdle())
        return THECORE_SECS_TO_PASSES(1);

    return THECORE_SECS_TO_PASSES(1);
}

class FuncFindMobVictim
{
public:
    FuncFindMobVictim(CHARACTER *pkChr, int iMaxDistance)
        : m_pkChr(pkChr),
          m_iMinDistance(std::numeric_limits<int>::min()),
          m_iMaxDistance(iMaxDistance),
          m_lx(pkChr->GetX()),
          m_ly(pkChr->GetY()),
          m_pkChrVictim(nullptr)
    {
    };

    bool operator ()(CEntity *ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return false;

        CHARACTER *pkChr = static_cast<CHARACTER *>(ent);

        if (pkChr->IsNPC())
        {
            if (!pkChr->IsMonster() || !m_pkChr->IsAttackMob() || m_pkChr->IsAggressive())
                return false;

#ifdef __FAKE_PC__
            if (pkChr->FakePC_Check())
            {
                if (!m_pkChr->FakePC_Check())
                    return false;

                if (m_pkChr->GetPVPTeam() == pkChr->GetPVPTeam())
                    return false;
            }
#endif
        }

#ifdef __FAKE_PC__
        if ((pkChr->FakePC_IsSupporter() && !pkChr->FakePC_CanAttack()) || (
                m_pkChr->FakePC_IsSupporter() && !m_pkChr->FakePC_CanAttack()))
            return false;

        if (pkChr->IsPC() && m_pkChr->FakePC_Check() && m_pkChr->FakePC_GetOwner() == pkChr)
            return false;
#endif

        if (pkChr->IsPet())
            return false;

        if (pkChr->IsPetPawn())
            return false;

        if (pkChr->IsBuffBot())
            return false;

        if (pkChr->IsToggleMount())
            return false;

        if (pkChr->IsDead() || pkChr->IsObserverMode())
            return false;

        if (pkChr->FindAffect(SKILL_EUNHYUNG) ||
            pkChr->FindAffect(AFFECT_INVISIBILITY) ||
            pkChr->FindAffect(AFFECT_REVIVE_INVISIBLE))
            return false;

        if (pkChr->FindAffect(SKILL_TERROR) && !m_pkChr->IsImmune(IMMUNE_TERROR)) // 공포 처리
        {
            if (pkChr->GetLevel() >= m_pkChr->GetLevel())
                return false;
        }

        if (m_pkChr->IsNoAttackShinsu())
        {
            if (pkChr->GetEmpire() == 1)
                return false;
        }

        if (m_pkChr->IsNoAttackChunjo())
        {
            if (pkChr->GetEmpire() == 2)
                return false;
        }

        if (m_pkChr->IsNoAttackJinno())
        {
            if (pkChr->GetEmpire() == 3)
                return false;
        }

        int iDistance = DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

        if (iDistance < m_iMinDistance && iDistance <= m_iMaxDistance)
        {
            m_pkChrVictim = pkChr;
            m_iMinDistance = iDistance;
        }
        return true;
    }

    CHARACTER *GetVictim() { return m_pkChrVictim; }

private:
    CHARACTER *m_pkChr;

    int m_iMinDistance;
    int m_iMaxDistance;
    int32_t m_lx;
    int32_t m_ly;

    CHARACTER *m_pkChrVictim;
};

CHARACTER *FindVictim(CHARACTER *pkChr, int iMaxDistance)
{
    FuncFindMobVictim f(pkChr, iMaxDistance);
    pkChr->ForEachSeen(f);
    return f.GetVictim();
}

#ifdef __FAKE_PC__
class FuncFindFakePCVictim
{
public:
    FuncFindFakePCVictim(CHARACTER *pkChr, CHARACTER *pkMainPC)
        : m_pkChr(pkChr),
          m_pkMainPC(pkMainPC),
          m_lx(pkChr->GetX()),
          m_ly(pkChr->GetY()),
          m_lmainx(pkMainPC->GetX()),
          m_lmainy(pkMainPC->GetY()),
          m_lDistance(-1),
          m_pkChrVictim(nullptr)
    {
    }

    bool operator ()(CEntity *ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return false;

        CHARACTER *pkChr = (CHARACTER *)ent;

        if (!pkChr->IsMonster())
            return false;

        if (pkChr->IsDead())
            return false;

        if (pkChr->GetVictim() != m_pkMainPC && pkChr->GetVictim() != m_pkChr)
            return false;

#ifdef __FAKE_BUFF__
		if (pkChr->FakeBuff_Check())
			return false;
#endif

        // focus range / magic types
        if (m_pkChrVictim)
        {
            if (m_pkChrVictim->GetVictim() == m_pkMainPC && pkChr->GetVictim() == m_pkChr)
                return false;

            if (m_pkChrVictim->GetVictim() == pkChr->GetVictim())
            {
                if ((m_pkChrVictim->GetMobBattleType() == BATTLE_TYPE_RANGE || m_pkChrVictim->GetMobBattleType() ==
                     BATTLE_TYPE_MAGIC) &&
                    (pkChr->GetMobBattleType() != BATTLE_TYPE_RANGE && pkChr->GetMobBattleType() != BATTLE_TYPE_MAGIC))
                    return false;
            }
        }

        int iDistanceToPlayer = DISTANCE_APPROX(m_lmainx - pkChr->GetX(), m_lmainy - pkChr->GetY());
        if (iDistanceToPlayer > 2500)
            return false;

        int iDistance = DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

        if (m_lDistance < 0 || iDistance < m_lDistance)
        {
            m_lDistance = iDistance;
            m_pkChrVictim = pkChr;
        }

        return true;
    }

    CHARACTER *GetVictim() { return (m_pkChrVictim); }

private:
    CHARACTER *m_pkChr;
    CHARACTER *m_pkMainPC;

    int m_lx;
    int m_ly;
    int m_lmainx;
    int m_lmainy;
    int m_lDistance;

    CHARACTER *m_pkChrVictim;
};

CHARACTER *FindFakePCVictim(CHARACTER *pkChr)
{
    if (!pkChr->FakePC_Check())
    {
        SPDLOG_ERROR("{} {} is no fake pc [char type {}]", pkChr->GetPlayerID(), pkChr->GetName(),
                     pkChr->GetCharType());
        return nullptr;
    }

    CHARACTER *pkOwner = pkChr->FakePC_GetOwner();
    if (!pkOwner)
    {
        SPDLOG_ERROR("cannot get spawner for fake pc {}", pkChr->GetName());
        return nullptr;
    }

    FuncFindFakePCVictim f(pkChr, pkOwner);
    if (pkChr->GetSectree() != nullptr) { pkChr->GetSectree()->ForEachAround(f); }
    return f.GetVictim();
}
#endif
