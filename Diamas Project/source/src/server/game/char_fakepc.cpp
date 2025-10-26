#include "main.h"

#ifdef __FAKE_PC__
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "skill.h"
#include "motion.h"
#include "sectree.h"
#include "party.h"

#include <game/GamePacket.hpp>
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>
#include "PacketUtils.hpp"

extern int passes_per_sec;

struct FFindFakePCSkillVictim
{
    FFindFakePCSkillVictim(CHARACTER *pkChr, CSkillProto *pkSkill)
    {
        m_pkChr = pkChr;
        m_pkSkill = pkSkill;
        m_pkVictim = nullptr;
        m_iVictimDistance = -1;
    }

    void operator()(CEntity *ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        auto *pkChr = static_cast<CHARACTER *>(ent);

        if (pkChr->IsDead())
            return;

        if (!m_pkChr->FakePC_IsSupporter())
        {
            if (!pkChr->IsMonster())
                return;

            if (!pkChr->FakePC_IsSkillNeeded(m_pkSkill))
                return;

            int iDist = DISTANCE_SQRT(m_pkChr->GetX() - pkChr->GetX(), m_pkChr->GetY() - pkChr->GetY());
            if (iDist > m_pkSkill->dwTargetRange)
                return;

            if ((pkChr->FakePC_Check() && m_pkVictim && !m_pkVictim->FakePC_Check()) ||
                (m_iVictimDistance == -1 || iDist > m_iVictimDistance))
            {
                m_iVictimDistance = iDist;
                m_pkVictim = pkChr;
            }
        }
        else
        {
            if (!pkChr->IsPC() && !pkChr->FakePC_Check())
                return;

            if (!pkChr->FakePC_IsSkillNeeded(m_pkSkill))
                return;

            if (!IS_SET(m_pkSkill->dwFlag, SKILL_FLAG_ATTACK))
            {
                if (pkChr->IsPC())
                {
                    CHARACTER *pkFakePCOwner = m_pkChr->FakePC_GetOwner();
                    if (pkFakePCOwner != pkChr)
                    {
                        if (!pkFakePCOwner->GetParty() || !pkFakePCOwner->GetParty()->IsMember(pkChr->GetPlayerID()))
                            return;
                    }
                }
                else
                {
                    if (!pkChr->FakePC_Check() || pkChr->FakePC_GetOwner() != m_pkChr->FakePC_GetOwner())
                        return;

                    if (m_pkVictim &&
                        (m_pkVictim->GetJob() != JOB_SHAMAN ||
                         m_pkVictim->GetSkillGroup() != m_pkChr->GetSkillGroup()) &&
                        (m_pkVictim->IsPC() || m_pkVictim->GetLevel() > pkChr->GetLevel()))
                        return;
                }
            }

            int iDist = DISTANCE_SQRT(m_pkChr->GetX() - pkChr->GetX(), m_pkChr->GetY() - pkChr->GetY());
            if (iDist > m_pkSkill->dwTargetRange)
                return;

            if (m_iVictimDistance == -1)
            {
                m_iVictimDistance = iDist;
                m_pkVictim = pkChr;
            }
        }
    }

    CHARACTER *m_pkChr;
    CSkillProto *m_pkSkill;
    CHARACTER *m_pkVictim;
    int m_iVictimDistance;
};

EVENTFUNC(fake_pc_afk_event)
{
    if (event == nullptr)
    {
        SPDLOG_ERROR("%s <Event> Null pointer, %s:%d", __FUNCTION__, __FILE__, __LINE__);
        return 0;
    }

    char_event_info *info = dynamic_cast<char_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_ERROR("fake_pc_afk_event <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;
    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }

    ch->FakePC_Owner_ClearAfkEvent();
    if (ch->FakePC_Owner_DespawnAllSupporter())
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, ("Your fake pcs are despawned because of your inactivity."));

    return 0;
}

void CHARACTER::FakePC_Load(CHARACTER *pkOwner, CItem *pkSpawnItem)
{
    if (m_pkFakePCOwner)
        return;

    // init owner
    pkOwner->FakePC_Owner_AddSpawned(this, pkSpawnItem);
    pkOwner->FakePC_Owner_ResetAfkEvent();

    // init this
    m_pkFakePCOwner = pkOwner;
    m_pkFakePCSpawnItem = pkSpawnItem;

    // set item info
    if (m_pkFakePCSpawnItem)
    {
        // m_pkFakePCSpawnItem->SetItemActive(true);
        m_pkFakePCSpawnItem->StartTimerBasedOnWearExpireEvent();
    }

    // set general
    m_bPKMode = PK_MODE_PROTECT;
    SetLevel(pkOwner->GetLevel());
    SetEmpire(pkOwner->GetEmpire());

    // set name
    const char *szName = pkOwner->FakePC_Owner_GetName();
    if (*szName)
        SetName(szName);
    else
        SetName(((std::string)pkOwner->GetName()) + "'s Ditto");

    // set damage factor
    FakePC_ComputeDamageFactor();

    // copy parts

    // copy affects
    const auto list_pkAffect = pkOwner->GetAffectContainer();
    for (const auto &it : list_pkAffect)
        FakePC_AddAffect(it);

    // compute points
    ComputePoints();
    ComputeBattlePoints();
        std::memcpy(m_pointsInstant.parts, m_pkFakePCOwner->m_pointsInstant.parts, sizeof(m_pointsInstant.parts));

    // start affect + potion event
    StartAffectEvent();
}

void CHARACTER::FakePC_Destroy()
{
    if (!FakePC_Check())
        return;

    // deinit owner
    m_pkFakePCOwner->FakePC_Owner_RemoveSpawned(this);
    m_pkFakePCOwner->FakePC_Owner_ResetAfkEvent();

    // set item info
    if (m_pkFakePCSpawnItem)
    {
        // m_pkFakePCSpawnItem->SetItemActive(false);
        m_pkFakePCSpawnItem->StopTimerBasedOnWearExpireEvent();
    }

#ifdef __ARENA_DITTO_MODE__
    CDittoArenaManager::instance().OnDittoDestroy(this);
#endif
    event_cancel(&m_pkFakePCAfkEvent);
    // clear
    m_pkFakePCOwner = nullptr;
    m_pkFakePCSpawnItem = nullptr;
}

void CHARACTER::FakePC_Owner_ResetAfkEvent()
{
    event_cancel(&m_pkFakePCAfkEvent);

    if (m_set_pkFakePCSpawns.empty())
        return;

    auto *info = AllocEventInfo<char_event_info>();
    info->ch = this;
    event_create(fake_pc_afk_event, info, THECORE_SECS_TO_PASSES(60 * 3));
}

void CHARACTER::FakePC_Owner_ClearAfkEvent() { m_pkFakePCAfkEvent = nullptr; }

bool CHARACTER::FakePC_CanAddAffect(const AffectData &pkAff)
{
    if (pkAff.duration < 60 * 10)
        return false;

    std::unordered_set<uint32_t> set_dwNotAllowedTypes;

    if (set_dwNotAllowedTypes.find(pkAff.type) != set_dwNotAllowedTypes.end())
        return false;

    return true;
}

void CHARACTER::FakePC_AddAffect(const AffectData &data)
{
    if (!FakePC_CanAddAffect(data))
        return;

    AffectData pkNewAff = {};

    auto it = m_map_pkFakePCAffects.find(data);
    if (it == m_map_pkFakePCAffects.end())
    {
        InsertAffect(m_affects, data);

        m_map_pkFakePCAffects.insert(std::pair<AffectData, AffectData>(data, pkNewAff));
        m_map_pkFakePCAffects.insert(std::pair<AffectData, AffectData>(pkNewAff, data));
    }
    else
    {
        pkNewAff = it->second;

        ComputeAffect(pkNewAff, false);
    }

    ComputeAffect(data, true);
}

void CHARACTER::FakePC_RemoveAffect(const AffectData &pkAff)
{
    const auto it = m_map_pkFakePCAffects.find(pkAff);
    if (it == m_map_pkFakePCAffects.end())
        return;

    RemoveAffect(it->second);
}

void CHARACTER::FakePC_Owner_AddSpawned(CHARACTER *pkFakePC, CItem *pkSpawnItem)
{
    m_set_pkFakePCSpawns.insert(pkFakePC);

    if (pkSpawnItem)
        m_map_pkFakePCSpawnItems.insert(std::pair<CItem *, CHARACTER *>(pkSpawnItem, pkFakePC));
}

bool CHARACTER::FakePC_Owner_RemoveSpawned(CHARACTER *pkFakePC)
{
    if (pkFakePC->FakePC_GetOwnerItem())
        m_map_pkFakePCSpawnItems.erase(pkFakePC->FakePC_GetOwnerItem());

    m_set_pkFakePCSpawns.erase(pkFakePC);

    return true;
}

bool CHARACTER::FakePC_Owner_RemoveSpawned(CItem *pkSpawnItem)
{
    auto it = m_map_pkFakePCSpawnItems.find(pkSpawnItem);
    if (it == m_map_pkFakePCSpawnItems.end())
        return false;

    return FakePC_Owner_RemoveSpawned(it->second);
}

CHARACTER *CHARACTER::FakePC_Owner_GetSpawnedByItem(CItem *pkItem)
{
    if (m_set_pkFakePCSpawns.empty())
        return nullptr;

    for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
    {
        if (m_set_pkFakePCSpawn->FakePC_GetOwnerItem() == pkItem)
            return m_set_pkFakePCSpawn;
    }

    return nullptr;
}

CHARACTER *CHARACTER::FakePC_Owner_GetSupporter()
{
    if (m_set_pkFakePCSpawns.empty())
        return nullptr;

    for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
    {
        if (m_set_pkFakePCSpawn->FakePC_IsSupporter())
            return m_set_pkFakePCSpawn;
    }

    return nullptr;
}

CHARACTER *CHARACTER::FakePC_Owner_GetSupporterByItem(CItem *pkItem)
{
    if (m_set_pkFakePCSpawns.empty())
        return nullptr;

    for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
    {
        if (m_set_pkFakePCSpawn->FakePC_IsSupporter() && m_set_pkFakePCSpawn->FakePC_GetOwnerItem() == pkItem)
            return m_set_pkFakePCSpawn;
    }

    return nullptr;
}

uint32_t CHARACTER::FakePC_Owner_CountSummonedByItem() { return m_map_pkFakePCSpawnItems.size(); }

void CHARACTER::FakePC_Owner_AddAffect(const AffectData &pkAff)
{
    if (pkAff.type < 200)
        return;

    if (!FakePC_CanAddAffect(pkAff))
        return;

    for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
        m_set_pkFakePCSpawn->FakePC_AddAffect(pkAff);
}

void CHARACTER::FakePC_Owner_RemoveAffect(const AffectData &pkAff)
{
    for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
        m_set_pkFakePCSpawn->FakePC_RemoveAffect(pkAff);
}

// general apply
void CHARACTER::FakePC_Owner_ApplyPoint(ApplyType bType, ApplyValue lValue)
{
    for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
        m_set_pkFakePCSpawn->ApplyPoint(bType, lValue);
}

// equip / unequip items
void CHARACTER::FakePC_Owner_ItemPoints(CItem *pkItem, bool bAdd)
{
    for (auto pkFakePC : m_set_pkFakePCSpawns)
    {
        pkItem->ModifyPoints(bAdd, pkFakePC);
        pkFakePC->SetImmuneFlag(m_pointsInstant.dwImmuneFlag);
    }
}

// mount
void CHARACTER::FakePC_Owner_MountBuff(bool bAdd)
{
}

void CHARACTER::FakePC_ComputeDamageFactor()
{
    m_fFakePCDamageFactor = 0.5f;
    if (m_pkFakePCSpawnItem)
    {
        for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
        {
            if (m_pkFakePCSpawnItem->GetAttributeType(i) != 0)
            {
                if (m_pkFakePCSpawnItem->GetAttributeValue(i) != 0)
                {
                    if (get_global_time() >= m_pkFakePCSpawnItem->GetAttributeValue(i))
                        continue;
                }

                m_fFakePCDamageFactor += ((float)m_pkFakePCSpawnItem->GetAttributeType(i) / 100.0f);
            }
        }
    }
}

CHARACTER *CHARACTER::FakePC_Owner_Spawn(int lX, int lY, CItem *pkItem, bool bIsEnemy, bool bIsRedPotionEnabled)
{
    if (!IsPC())
    {
        SPDLOG_ERROR("cannot spawn fake pc for mob {}", GetRaceNum());
        return nullptr;
    }

    uint32_t dwRaceNum = MAIN_RACE_MOB_WARRIOR_M + GetRaceNum();
    int lMapIndex = GetMapIndex();

    CHARACTER *pkFakePC = g_pCharManager->SpawnMob(dwRaceNum, lMapIndex, lX, lY, GetZ(), false, -1, false);
    if (!pkFakePC)
    {
        SPDLOG_ERROR("cannot spawn fake pc {} for player {} {}", dwRaceNum, GetPlayerID(), GetName());
        return nullptr;
    }

    pkFakePC->FakePC_Load(this, pkItem);
    pkFakePC->SetHP(pkFakePC->GetMaxHP());
    pkFakePC->SetSP(pkFakePC->GetMaxSP());

    if (bIsEnemy)
    {
        pkFakePC->SetPVPTeam(SHRT_MAX);

        if (auto pkDungeon = GetDungeon())
            pkFakePC->SetDungeon(pkDungeon);
    }

    // if (!bIsRedPotionEnabled)
    //	pkFakePC->DisableHealPotions();

    pkFakePC->Show(lMapIndex, lX, lY);

    return pkFakePC;
}

void CHARACTER::FakePC_Owner_DespawnAll()
{
    if (!g_pCharManager->IsPendingDestroy())
    {
        while (!m_set_pkFakePCSpawns.empty())
            M2_DESTROY_CHARACTER(*(m_set_pkFakePCSpawns.begin()));
    }
    else
    {
        for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
            M2_DESTROY_CHARACTER(m_set_pkFakePCSpawn);
    }
}

bool CHARACTER::FakePC_Owner_DespawnAllSupporter()
{
    // create temporary vector copy of all spawned fake pcs
    std::vector<CHARACTER *> vec_pkSpawns;
    vec_pkSpawns.reserve(m_set_pkFakePCSpawns.size());

    int i = 0;
    for (auto it = m_set_pkFakePCSpawns.begin(); it != m_set_pkFakePCSpawns.end(); ++it, ++i)
        vec_pkSpawns[i] = *it;

    // destroy all fake pc supporter in the vector
    int iCount = 0;
    for (i = 0; i < vec_pkSpawns.size(); ++i)
    {
        CHARACTER *ch = vec_pkSpawns[i];

        if (!ch->FakePC_IsSupporter())
            continue;

        M2_DESTROY_CHARACTER(ch);
        ++iCount;
    }

    return iCount > 0;
}

bool CHARACTER::FakePC_Owner_DespawnByItem(CItem *pkItem)
{
    auto it = m_map_pkFakePCSpawnItems.find(pkItem);
    if (it == m_map_pkFakePCSpawnItems.end())
        return false;

    M2_DESTROY_CHARACTER(it->second);
    return true;
}

void CHARACTER::FakePC_Owner_ForceFocus(CHARACTER *pkVictim)
{
    for (auto pkFakePC : m_set_pkFakePCSpawns)
    {
        if (!pkFakePC->FakePC_IsSupporter() || !pkFakePC->FakePC_CanAttack())
            continue;

        if (pkFakePC->IsDead() || pkFakePC->IsStun())
            continue;

        pkFakePC->BeginFight(pkVictim);
    }
}

uint8_t CHARACTER::FakePC_ComputeComboIndex()
{
    uint8_t bComboSequence = 0;

    if (!GetWear(WEAR_WEAPON))
        return bComboSequence;

    if (GetComboSequence() > 0 && GetComboSequence() <= 3)
    {
        uint32_t dwInterval = get_dword_time() - GetLastComboTime();
        if (dwInterval <= GetValidComboInterval())
            bComboSequence = GetComboSequence();
    }

    return bComboSequence;
}

bool CHARACTER::FakePC_IsSkillNeeded(CSkillProto *pkSkill)
{
    if (pkSkill->dwAffectFlag != 0)
    {
        if (FindAffect(pkSkill->dwVnum) != nullptr)
            return false;
        else
            return !IsGoodAffect(pkSkill->dwVnum);
    }
    else if (pkSkill->bPointOn != POINT_NONE)
    {
        if (pkSkill->bPointOn == POINT_HP)
        {
            if (GetHPPct() > 90)
                return false;
        }
    }

    return true;
}

bool CHARACTER::FakePC_IsBuffSkill(uint32_t dwVnum)
{
    switch (dwVnum)
    {
    case SKILL_HOSIN:
    case SKILL_REFLECT:
    case SKILL_GICHEON:
    case SKILL_JEONGEOP:
    case SKILL_KWAESOK:
    case SKILL_JEUNGRYEOK:
        return true;
    default:
        return false;
    }
    return false;
}

bool CHARACTER::FakePC_UseSkill(CHARACTER *pkTarget)
{
    if (!FakePC_Check())
    {
        SPDLOG_ERROR("cannot use fake pc skills on non-fake pc %s [pid %u vid %u]", GetName(), GetPlayerID(),
                     (uint32_t)GetVID());
        return false;
    }

    if (!GetSectree())
        return false;

    if (!FakePC_CanAttack())
        return false;

    // check animation time
    if (m_dwLastSkillVnum != 0)
    {
        uint32_t dwTimeDif = get_dword_time() - m_dwLastSkillTime;
        if (dwTimeDif < 2000)
            return false;

        CSkillProto *pkSkill = CSkillManager::Instance().Get(m_dwLastSkillVnum);
        if (pkSkill)
        {
            uint32_t dwSkillIndex = 0;
            const uint32_t *dwSkillList = GetUsableSkillList(GetJob(), GetSkillGroup());
            if (dwSkillList)
            {
                for (int i = 0; i < 6; ++i)
                {
                    if (dwSkillList[i] == m_dwLastSkillVnum)
                    {
                        dwSkillIndex = i;
                        break;
                    }
                }
            }

            uint32_t dwDurationMotionKey =
                MakeMotionKey(MOTION_MODE_GENERAL, MOTION_SPECIAL_1 + (GetSkillGroup() - 1) * 6 + dwSkillIndex);
            auto motion = GetMotionManager().Get(GetRaceNum(), dwDurationMotionKey);
            uint32_t duration = motion ? motion->duration : 2000;

            if (dwTimeDif < duration)
                return false;
        }
    }

    // get distance
    int iDistToTarget = 0;
    if (pkTarget)
        iDistToTarget = DISTANCE_APPROX(GetX() - pkTarget->GetX(), GetY() - pkTarget->GetY());

    // check if attr is BANPK
    bool bIsBanPK = false;
    if (GetSectree()->IsAttr(GetX(), GetY(), ATTR_BANPK) ||
        (pkTarget && pkTarget->GetSectree() &&
         pkTarget->GetSectree()->IsAttr(pkTarget->GetX(), pkTarget->GetY(), ATTR_BANPK)))
        bIsBanPK = true;

    const uint32_t *pSkills = GetUsableSkillList(GetJob(), GetSkillGroup());
    if (pSkills)
    {
        // SUPPORT SKILLS
        for (int i = 0; i < 6; ++i)
        {
            // get skill vnum
            uint32_t dwVnum = pSkills[i];
            if (!dwVnum)
                continue;

            // check if skill is support skill
            CSkillProto *pkProto = CSkillManager::instance().Get(dwVnum);
            if (!pkProto || IS_SET(pkProto->dwFlag, SKILL_FLAG_ATTACK))
                continue;

            // check level
            if (GetSkillLevel(dwVnum) <= 0)
                continue;

            // check if the skill is usable (cooltime)
            if (!m_SkillUseInfo[dwVnum].IsCooltimeOver())
                continue;

            // skills with only-display effects aren't used by npcs
            if (pkProto->bPointOn == POINT_NONE)
                continue;

            // check if self needed
            bool bNeedSelf = FakePC_IsSkillNeeded(pkProto);
            if (!bNeedSelf)
            {
                if (!IS_SET(pkProto->dwFlag, SKILL_FLAG_SELFONLY))
                {
                    FFindFakePCSkillVictim f(this, pkProto);
                    GetSectree()->ForEachAround(f);

                    if (f.m_pkVictim != nullptr)
                    {
                        // block buffs for main char
                        // if (f.m_pkVictim == FakePC_GetOwner() && FakePC_IsBuffSkill(dwVnum))
                        // {
                        // continue;
                        // }
                        if (!FakePC_IsBuffSkill(dwVnum) && UseSkill(dwVnum, f.m_pkVictim))
                        {
                            SetRotationToXY(f.m_pkVictim->GetX(), f.m_pkVictim->GetY());
                            FakePC_SendSkillPacket(pkProto);
                            return true;
                        }
                    }
                }

                continue;
            }

            // use skill
            if (UseSkill(dwVnum, this))
            {
                FakePC_SendSkillPacket(pkProto);
                return true;
            }
        }

        // ATTACK SKILLS
        if (pkTarget && !bIsBanPK &&
            (GetComboSequence() != 0 || GetLastComboTime() == 0 || get_dword_time() - GetLastComboTime() >= 1000))
        {
            SetRotationToXY(pkTarget->GetX(), pkTarget->GetY());

            for (int i = 0; i < 6; ++i)
            {
                // get skill vnum
                uint32_t dwVnum = pSkills[i];
                if (!dwVnum)
                    continue;

                // check if skill is attack skill
                CSkillProto *pkProto = CSkillManager::instance().Get(dwVnum);
                if (!pkProto || !IS_SET(pkProto->dwFlag, SKILL_FLAG_ATTACK))
                    continue;

                // check level
                if (GetSkillLevel(dwVnum) <= 0)
                    continue;

                // check if the skill is usable (cooltime)
                if (!m_SkillUseInfo[dwVnum].IsCooltimeOver())
                    continue;

                // check range
                if (!IS_SET(pkProto->dwFlag, SKILL_FLAG_SELFONLY) || IS_SET(pkProto->dwFlag, SKILL_FLAG_SPLASH))
                {
                    uint32_t dwMaxRange = pkProto->dwTargetRange;
                    if (!dwMaxRange)
                        dwMaxRange = pkProto->iSplashRange;
                    if (!dwMaxRange)
                        dwMaxRange = 250;

                    if (iDistToTarget >= dwMaxRange)
                        continue;
                }

                // use skill
                if (UseSkill(dwVnum, pkTarget))
                {
                    FakePC_SendSkillPacket(pkProto);
                    return true;
                }
            }
        }
    }

    return false;
}

void CHARACTER::FakePC_SendSkillPacket(CSkillProto *pkSkill)
{
    GcSkillMotionPacket p;
    p.vid = GetVID();
    p.x = GetX();
    p.y = GetY();
    p.rotation = GetRotation() / 5.0f;
    p.time = get_dword_time();
    p.skillVnum = pkSkill->dwVnum;
    p.skillLevel = GetSkillLevel(pkSkill->dwVnum);
    p.skillGrade = GetSkillMasterType(pkSkill->dwVnum);
    PacketView(m_map_view, static_cast<CEntity*>(this), HEADER_GC_SKILL_MOTION, p);
}

void CHARACTER::FakePC_Owner_ExecFunc(void (CHARACTER::*func)())
{
    for (auto m_set_pkFakePCSpawn : m_set_pkFakePCSpawns)
        (m_set_pkFakePCSpawn->*func)();
}
#endif
