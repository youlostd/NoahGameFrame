#include "char.h"

#include "constants.h"
#include "config.h"
#include "char_manager.h"
#include "MeleyLair.h"

#include <game/AffectConstants.hpp>

// 독
const int poison_damage_rate[MOB_RANK_MAX_NUM] =
{
    80, 50, 40, 30, 25, 1
};

int GetPoisonDamageRate(CHARACTER *ch)
{
    int iRate;

    if (ch->IsPC())
        iRate = 50;
    else
        iRate = poison_damage_rate[ch->GetMobRank()];

    iRate = std::max<int>(0, iRate - ch->GetPoint(POINT_POISON_REDUCE));
    return iRate;
}

const int bleeding_damage_rate[MOB_RANK_MAX_NUM] =
{
    80, 50, 40, 30, 25, 1
};

int GetBleedingDamageRate(CHARACTER *ch)
{
    int iRate;

    if (ch->IsPC())
        iRate = 50;
    else
        iRate = bleeding_damage_rate[ch->GetMobRank()];

    iRate = std::max<int>(0, iRate - ch->GetPoint(POINT_BLEEDING_REDUCE));
    return iRate;
}

EVENTINFO(TPoisonEventInfo)
{
    DynamicCharacterPtr ch;
    int count;
    uint32_t attacker_pid;

    TPoisonEventInfo()
        : ch()
          , count(0)
          , attacker_pid(0)
    {
    }
};

EVENTFUNC(poison_event)
{
    TPoisonEventInfo *info = static_cast<TPoisonEventInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("poison_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;

    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }
    CHARACTER *pkAttacker = g_pCharManager->FindByPID(info->attacker_pid);

    int64_t dam = ch->GetMaxHP() * GetPoisonDamageRate(ch) / 1000;
    if (gConfig.testServer)
        ch->ChatPacket(CHAT_TYPE_NOTICE, "Poison Damage %d", dam);

    if (dam != 0)
    {
        if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_POISON))
        {
            ch->m_pkPoisonEvent = nullptr;
            return 0;
        }
    }

    --info->count;

    if (info->count)
        return THECORE_SECS_TO_PASSES(3);

    ch->m_pkPoisonEvent = nullptr;
    return 0;
}

EVENTINFO(TFireEventInfo)
{
    DynamicCharacterPtr ch;
    int count;
    int amount;
    uint32_t attacker_pid;

    TFireEventInfo()
        : ch()
          , count(0)
          , amount(0)
          , attacker_pid(0)
    {
    }
};

EVENTFUNC(fire_event)
{
    TFireEventInfo *info = static_cast<TFireEventInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("fire_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;
    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }
    CHARACTER *pkAttacker = g_pCharManager->FindByPID(info->attacker_pid);

    int dam = info->amount;
    if (gConfig.testServer)
        ch->ChatPacket(CHAT_TYPE_NOTICE, "Fire Damage %d", dam);

    if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_FIRE))
    {
        ch->m_pkFireEvent = nullptr;
        return 0;
    }

    --info->count;

    if (info->count)
        return THECORE_SECS_TO_PASSES(3);
    else
    {
        ch->m_pkFireEvent = nullptr;
        return 0;
    }
}


/*

LEVEL에 의한..

+8   0%
+7   5%
+6  10%
+5  30%
+4  50%
+3  70%
+2  80%
+1  90%
+0 100%
-1 100%
-2 100%
-3 100%
-4 100%
-5 100%
-6 100%
-7 100%
-8 100%

*/

static int poison_level_adjust[9] =
{
    100, 90, 80, 70, 50, 30, 10, 5, 0
};

EVENTINFO(TBleedingEventInfo)
{
    DynamicCharacterPtr ch;
    int count;
    uint32_t attacker_pid;

    TBleedingEventInfo()
        : ch()
          , count(0)
          , attacker_pid(0)
    {
    }
};

EVENTFUNC(bleeding_event)
{
    TBleedingEventInfo *info = static_cast<TBleedingEventInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("poison_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;

    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }
    CHARACTER *pkAttacker = g_pCharManager->FindByPID(info->attacker_pid);

    int64_t dam = ch->GetMaxHP() * GetBleedingDamageRate(ch) / 1000;
    if (gConfig.testServer)
        ch->ChatPacket(CHAT_TYPE_NOTICE, "Bleeding Damage %d", dam);

    if (dam != 0)
    {
        if (ch->Damage(pkAttacker, dam, DAMAGE_TYPE_POISON))
        {
            ch->m_pkBleedEvent = nullptr;
            return 0;
        }
    }

    --info->count;

    if (info->count)
        return THECORE_SECS_TO_PASSES(3);

    ch->m_pkBleedEvent = nullptr;
    return 0;
}



void CHARACTER::AttackedByFire(CHARACTER *pkAttacker, int amount, int count)
{
    if (m_pkFireEvent)
        return;

    if (GetRaceNum() == MeleyLair::STATUE_VNUM)
        return;

    AddAffect(AFFECT_FIRE, POINT_NONE, 0, count * 3 + 1, 0, true);

    TFireEventInfo *info = AllocEventInfo<TFireEventInfo>();

    info->ch = this;
    info->count = count;
    info->amount = amount;
    info->attacker_pid = pkAttacker->GetPlayerID();

    m_pkFireEvent = event_create(fire_event, info, 1);
}

void CHARACTER::AttackedByPoison(CHARACTER *pkAttacker)
{
    if (m_pkPoisonEvent)
        return;

    if (FindAffect(AFFECT_POISON))
        return;

    if (GetRaceNum() == MeleyLair::STATUE_VNUM)
        return;

    if (m_bHasPoisoned && !IsPC()) // Monster takes poison only once.
        return;

    if (pkAttacker && pkAttacker->GetLevel() < GetLevel())
    {
        int delta = GetLevel() - pkAttacker->GetLevel();

        if (delta > 8)
            delta = 8;

        if (Random::get(1, 100) > poison_level_adjust[delta])
            return;
    }

    /*if (IsImmune(IMMUNE_POISON))
    return;*/

    // Poison Rolling Failure, Poisoned!
    m_bHasPoisoned = true;

    AddAffect(AFFECT_POISON, POINT_NONE, 0, POISON_LENGTH + 1, 0, true);

    TPoisonEventInfo *info = AllocEventInfo<TPoisonEventInfo>();

    info->ch = this;
    info->count = 10;
    info->attacker_pid = pkAttacker ? pkAttacker->GetPlayerID() : 0;

    m_pkPoisonEvent = event_create(poison_event, info, 1);

    if (gConfig.testServer && pkAttacker)
    {
        pkAttacker->ChatPacket(CHAT_TYPE_INFO, "POISON %s -> %s", pkAttacker->GetName(), GetName());
    }
}

void CHARACTER::AttackedByBleeding(CHARACTER *pkAttacker)
{
    if (m_pkBleedEvent)
        return;

    if (FindAffect(AFFECT_BLEEDING))
        return;

    if (GetRaceNum() == MeleyLair::STATUE_VNUM)
        return;

    if (pkAttacker && pkAttacker->GetLevel() < GetLevel())
    {
        int delta = GetLevel() - pkAttacker->GetLevel();

        if (delta > 8)
            delta = 8;

        if (Random::get(1, 100) > poison_level_adjust[delta])
            return;
    }

    AddAffect(AFFECT_BLEEDING, POINT_NONE, 0, BLEEDING_LENGTH + 1, 0, true);

    TBleedingEventInfo *info = AllocEventInfo<TBleedingEventInfo>();

    info->ch = this;
    info->count = 10;
    info->attacker_pid = pkAttacker ? pkAttacker->GetPlayerID() : 0;

    m_pkBleedEvent = event_create(bleeding_event, info, 1);

    if (gConfig.testServer && pkAttacker)
    {
        pkAttacker->ChatPacket(CHAT_TYPE_INFO, "BLEEDING %s -> %s", pkAttacker->GetName(), GetName());
    }
}

void CHARACTER::RemoveFire()
{
    RemoveAffect(AFFECT_FIRE);
    event_cancel(&m_pkFireEvent);
}

void CHARACTER::RemovePoison()
{
    RemoveAffect(AFFECT_POISON);
    event_cancel(&m_pkPoisonEvent);
}

void CHARACTER::RemoveBleeding()
{
    RemoveAffect(AFFECT_BLEEDING);
    event_cancel(&m_pkBleedEvent);
}


bool CHARACTER::IsImmune(uint32_t dwImmuneFlag) { return IS_SET(m_pointsInstant.dwImmuneFlag, dwImmuneFlag); }
