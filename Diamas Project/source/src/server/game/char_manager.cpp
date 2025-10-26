#include "char_manager.h"
#include "char.h"
#include "desc.h"

#include "utils.h"

#include "ChatUtil.hpp"
#include "base/GroupTextTree.hpp"
#include "base/GroupTextTreeUtil.hpp"
#include "mob_manager.h"
#include "party.h"
#include "regen.h"

#include "DbCacheSocket.hpp"
#include "config.h"
#include "db.h"
#include "dungeon.h"
#include "game/MasterPackets.hpp"
#include "map_location.h"
#include "mining.h"
#include "questlua.h"
#include "questmanager.h"
#include "shop.h"
#include "xmas_event.h"

#include <absl/strings/match.h>
#include <absl/strings/str_split.h>
#include <fstream>

CHARACTER_MANAGER::CHARACTER_MANAGER()
    : m_iVIDCount(0), m_pkChrSelectedStone(nullptr), m_bUsePendingDestroy(false)
{
    RegisterRaceNum(xmas::MOB_XMAS_FIRWORK_SELLER_VNUM);
    RegisterRaceNum(xmas::MOB_SANTA_VNUM);
    RegisterRaceNum(xmas::MOB_XMAS_TREE_VNUM);

    m_iMobItemRate = 100;
    m_iMobDamageRate = 100;
    m_iMobGoldAmountRate = 100;
    m_iMobGoldDropRate = 100;
    m_iMobExpRate = 100;

    m_iMobItemRatePremium = 100;
    m_iMobGoldAmountRatePremium = 100;
    m_iMobGoldDropRatePremium = 100;
    m_iMobExpRatePremium = 100;

    m_iUserDamageRate = 100;
    m_iUserDamageRatePremium = 100;
}

void CHARACTER_MANAGER::Destroy()
{
    for(auto& [fst, ch] : m_map_pkChrByVID) {
        M2_DESTROY_CHARACTER(ch);
    }
    m_map_pkChrByVID.clear();
}

void CHARACTER_MANAGER::GracefulShutdown()
{
    auto it = m_map_pkPCChr.begin();

    while (it != m_map_pkPCChr.end())
        (it++)->second->Disconnect("GracefulShutdown");
}

uint32_t CHARACTER_MANAGER::AllocVID()
{
    ++m_iVIDCount;
    return m_iVIDCount;
}

CHARACTER *CHARACTER_MANAGER::CreateCharacter(std::string stName, uint32_t dwPID /*= 0*/)
{
    uint32_t dwVID = AllocVID();

#ifdef M2_USE_POOL
    CHARACTER *ch = pool_.Construct();
#else
    auto ch = new CHARACTER;
#endif
    ch->Create(stName, dwVID, dwPID != 0);

    m_map_pkChrByVID.emplace(dwVID, ch);

    if (dwPID)
    {
        m_map_pkPCChr.emplace(stName, ch);
        m_map_pkChrByPID.emplace(dwPID, ch);
    }

    return (ch);
}

#ifndef DEBUG_ALLOC
void CHARACTER_MANAGER::DestroyCharacter(CHARACTER *ch)
#else
void CHARACTER_MANAGER::DestroyCharacter(CHARACTER *ch, const char *file, size_t line)
#endif
{
    if (!ch)
        return;

    // <Factor> Check whether it has been already deleted or not.
    auto it = m_map_pkChrByVID.find(ch->GetVID());
    if (it == m_map_pkChrByVID.end())
    {
        return; // prevent duplicated destruction
    }

    if (m_bUsePendingDestroy)
    {
        m_set_pkChrPendingDestroy.insert(ch);
        return;
    }

    m_map_pkChrByVID.erase(it);

    if (true == ch->IsPC())
    {
        const auto itpCCh = m_map_pkPCChr.find(ch->GetName());
        if (m_map_pkPCChr.end() != itpCCh)
            m_map_pkPCChr.erase(itpCCh);
    }

    // Shops and possible fake character copies (non-PC) hold the player ID as GetPlayerID(), but we should't delete the
    // actual player from here.
    if (ch->GetPlayerID() && ch->IsPC())
    {
        if (auto it2 = m_map_pkChrByPID.find(ch->GetPlayerID()); m_map_pkChrByPID.end() != it2)
        {
            m_map_pkChrByPID.erase(it2);
        }
    }

    UnregisterRaceNumMap(ch);

    RemoveFromStateList(ch);

    delete ch;
}

CHARACTER *CHARACTER_MANAGER::Find(uint32_t dwVID)
{
    auto it = m_map_pkChrByVID.find(dwVID);

    if (m_map_pkChrByVID.end() == it)
        return nullptr;

    // <Factor> Added sanity check
    CHARACTER *found = it->second;
    if (found != nullptr && dwVID != (uint32_t)found->GetVID())
    {
        SPDLOG_ERROR("[CHARACTER_MANAGER::Find] <Factor> {} != {}", dwVID, (uint32_t)found->GetVID());
        return nullptr;
    }
    return found;
}

CHARACTER *CHARACTER_MANAGER::Find(const VID &vid)
{
    CHARACTER *tch = Find((uint32_t)vid);

    if (!tch || tch->GetVID() != vid)
        return nullptr;

    return tch;
}

void CHARACTER_MANAGER::RegisterKill(uint32_t pid, uint64_t hwid)
{
    if (m_map_pidToHwidsKilled.count(pid)) { m_map_pidToHwidsKilled[pid].emplace_back(hwid); }
    else
    {
        m_map_pidToHwidsKilled.emplace(pid, std::vector<uint64_t>());
        m_map_pidToHwidsKilled[pid].emplace_back(hwid);
    }
}

bool CHARACTER_MANAGER::HasPlayerKilledHwid(uint32_t pid, uint64_t hwid)
{
    if (!m_map_pidToHwidsKilled.count(pid))
        return false;

    for (auto &killedHwid : m_map_pidToHwidsKilled[pid])
    {
        if (killedHwid == hwid)
            return true;
    }
    return false;
}

CHARACTER *CHARACTER_MANAGER::FindByPID(uint32_t dwPID)
{
    auto it = m_map_pkChrByPID.find(dwPID);

    if (m_map_pkChrByPID.end() == it)
        return nullptr;

    // <Factor> Added sanity check
    CHARACTER *found = it->second;
    if (found != nullptr && dwPID != found->GetPlayerID())
    {
        SPDLOG_ERROR("[CHARACTER_MANAGER::FindByPID] <Factor> {} != {}", dwPID, found->GetPlayerID());
        return nullptr;
    }
    return found;
}

CHARACTER *CHARACTER_MANAGER::FindPC(std::string stName)
{
    const auto it = m_map_pkPCChr.find(stName);
    if (it == m_map_pkPCChr.end())
        return nullptr;

    // <Factor> Added sanity check
    const auto found = it->second;
    if (found != nullptr && strncasecmp(stName.c_str(), found->GetName().c_str(), CHARACTER_NAME_MAX_LEN) != 0)
    {
        SPDLOG_ERROR("[CHARACTER_MANAGER::FindPC] <Factor> %s != %s", stName.c_str(), found->GetName());
        return nullptr;
    }
    return found;
}

CHARACTER *CHARACTER_MANAGER::SpawnMobRandomPosition(uint32_t dwVnum, long lMapIndex)
{
    // 왜구 스폰할지말지를 결정할 수 있게함
    {
        if (dwVnum == 5001 && !quest::CQuestManager::instance().GetEventFlag("japan_regen"))
        {
            SPDLOG_DEBUG("WAEGU[5001] regen disabled.");
            return nullptr;
        }
    }

    // 해태를 스폰할지 말지를 결정할 수 있게 함
    {
        if (dwVnum == 5002 && !quest::CQuestManager::instance().GetEventFlag("newyear_mob"))
        {
            SPDLOG_DEBUG("HAETAE (new-year-mob) [5002] regen disabled.");
            return nullptr;
        }
    }

    // 광복절 이벤트
    {
        if (dwVnum == 5004 && !quest::CQuestManager::instance().GetEventFlag("independence_day"))
        {
            SPDLOG_DEBUG("INDEPENDECE DAY [5004] regen disabled.");
            return nullptr;
        }
    }

    const auto *pkMob = CMobManager::instance().Get(dwVnum);

    if (!pkMob)
    {
        SPDLOG_ERROR("no mob data for vnum %d", dwVnum);
        return nullptr;
    }

    if (!gConfig.IsHostingMap(lMapIndex))
    {
        SPDLOG_ERROR("not allowed map %d", lMapIndex);
        return nullptr;
    }

    SECTREE_MAP *pkSectreeMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);
    if (pkSectreeMap == nullptr) { return nullptr; }

    int i;
    int32_t x = 0, y = 0;
    for (i = 0; i < 2000; i++)
    {
        x = Random::get(1, (pkSectreeMap->m_setting.iWidth / 100) - 1) * 100;
        y = Random::get(1, (pkSectreeMap->m_setting.iHeight / 100) - 1) * 100;

        SECTREE *tree = pkSectreeMap->Find(x, y);
        if (!tree)
            continue;

        uint32_t dwAttr = tree->GetAttribute(x, y);

        if (IS_SET(dwAttr, ATTR_BLOCK | ATTR_OBJECT))
            continue;

        if (IS_SET(dwAttr, ATTR_BANPK))
            continue;

        break;
    }

    if (i == 2000)
    {
        SPDLOG_INFO("cannot find valid location");
        return nullptr;
    }

    CHARACTER *ch = g_pCharManager->CreateCharacter(pkMob->szLocaleName);
    if (!ch)
    {
        SPDLOG_INFO("SpawnMobRandomPosition: cannot create new character");
        return nullptr;
    }

    ch->SetProto(pkMob);

    // if mob is npc with no empire assigned, assign to empire of map
    if (pkMob->bType == CHAR_TYPE_NPC)
        if (ch->GetEmpire() == 0)
            ch->SetEmpire(SECTREE_MANAGER::instance().GetEmpireFromMapIndex(lMapIndex));

    ch->SetRotation(Random::get(0.0f, 360.0f));

    if (!ch->Show(lMapIndex, x, y, 0, false))
    {
        M2_DESTROY_CHARACTER(ch);
        SPDLOG_ERROR("SpawnMobRandomPosition: cannot show monster");
        return nullptr;
    }

    SPDLOG_DEBUG("spawn {}[{}] random position at {} {} (time: {})", ch->GetName(), dwVnum, x, y,
                  static_cast<int>(get_global_time()));
    return (ch);
}

CHARACTER *CHARACTER_MANAGER::SpawnMob(uint32_t dwVnum, long lMapIndex, long x, long y, long z, bool bSpawnMotion,
                                       int iRot, bool bShow, bool bAggressive, uint16_t scale, bool ignoreMobChecks)
{
    const auto *pkMob = CMobManager::instance().Get(dwVnum);
    if (!pkMob)
    {
        SPDLOG_ERROR("SpawnMob: No monster data for vnum {}", dwVnum);
        return nullptr;
    }

    auto isException =
    (pkMob->bType == CHAR_TYPE_NPC || pkMob->bType == CHAR_TYPE_WARP || pkMob->bType == CHAR_TYPE_GOTO ||
     pkMob->bType == CHAR_TYPE_PET || pkMob->bType == CHAR_TYPE_ATTACK_PET || pkMob->bType == CHAR_TYPE_SHOP);
    if (dwVnum >= MAIN_RACE_MOB_WARRIOR_M && dwVnum <= MAIN_RACE_MOB_MAX_NUM)
        isException = true;

#ifdef ENABLE_HYDRA_DUNGEON
    if (lMapIndex == HYDRA_RUN_MAP_INDEX ||
        (lMapIndex >= HYDRA_RUN_MAP_INDEX * 10000 && lMapIndex < (HYDRA_RUN_MAP_INDEX + 1) * 10000))
        isException = true;
#endif

    if (!isException && !ignoreMobChecks)
    {
        SECTREE *tree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);
        if (!tree)
        {
            SPDLOG_INFO("No Sectree for spawn at {} {} Monster Vnum {} Map Id {}", x, y, dwVnum, lMapIndex);
            return nullptr;
        }

        uint32_t dwAttr = tree->GetAttribute(x, y);

        bool is_set;
        if (mining::IsVeinOfOre(dwVnum))
            is_set = IS_SET(dwAttr, ATTR_BLOCK);
        else
            is_set = IS_SET(dwAttr, ATTR_BLOCK | ATTR_OBJECT);

        if (is_set)
        {
            // SPAWN_BLOCK_LOG
            static bool s_isLog = quest::CQuestManager::instance().GetEventFlag("spawn_block_log");
            static uint32_t s_nextTime = get_global_time() + 10000;

            uint32_t curTime = get_global_time();
            if (curTime > s_nextTime)
            {
                s_nextTime = curTime;
                s_isLog = quest::CQuestManager::instance().GetEventFlag("spawn_block_log");
            }

            if (s_isLog)
                SPDLOG_INFO("SpawnMob: BLOCKED position for spawn {0} {1} at {2} {3} (attr {4})", pkMob->szName,
                        dwVnum, x, y, dwAttr);
            // END_OF_SPAWN_BLOCK_LOG
            return nullptr;
        }

        if (IS_SET(dwAttr, ATTR_BANPK))
        {
            SPDLOG_INFO("SpawnMob: BAN_PK position for mob spawn {0} {1} at {2} {3}", pkMob->szName, dwVnum, x, y);
            return nullptr;
        }
    }

    SECTREE *sectree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);
    if (!sectree)
    {
        SPDLOG_INFO("SpawnMob: cannot create monster at non-exist sectree %d x %d (map %d)", x, y, lMapIndex);
        return nullptr;
    }

    CHARACTER *ch = g_pCharManager->CreateCharacter(pkMob->szLocaleName);
    if (!ch)
    {
        SPDLOG_INFO("SpawnMob: cannot create new character");
        return nullptr;
    }

    if (iRot == -1)
        iRot = Random::get(0, 360);

    ch->SetProto(pkMob);

    // if mob is npc with no empire assigned, assign to empire of map
    if (pkMob->bType == CHAR_TYPE_NPC)
        if (ch->GetEmpire() == 0)
            ch->SetEmpire(SECTREE_MANAGER::instance().GetEmpireFromMapIndex(lMapIndex));

    ch->SetRotation(iRot);

	if(scale != 100)
		ch->SetScale(scale);

    if (bShow && !ch->Show(lMapIndex, x, y, z, bSpawnMotion, bAggressive)) //-__-""
    {
        M2_DESTROY_CHARACTER(ch);
        SPDLOG_INFO("SpawnMob: cannot show monster");
        return nullptr;
    }

    return (ch);
}

CHARACTER *CHARACTER_MANAGER::SpawnMobRange(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey,
                                            bool bIsException, bool bSpawnMotion, bool bAggressive, uint16_t scale)
{
    const auto *pkMob = CMobManager::instance().Get(dwVnum);

    if (!pkMob)
        return nullptr;

    if (pkMob->bType == CHAR_TYPE_STONE) // 돌은 무조건 SPAWN 모션이 있다.
        bSpawnMotion = false; //bSpawnMotion false olmalı

    int i = 16;

    while (i--)
    {
        int x = Random::get(sx, ex);
        int y = Random::get(sy, ey);
        /*
           if (bIsException)
           if (is_regen_exception(x, y))
           continue;
         */
        CHARACTER *ch = SpawnMob(dwVnum, lMapIndex, x, y, 0, bSpawnMotion, -1, true, bAggressive, scale);
        if (ch)
        {
            SPDLOG_DEBUG("MOB_SPAWN: {0}({1}):{2} {3}x{4}", ch->GetName(), (uint32_t)ch->GetVID(), bAggressive ? 1 : 0,
                          ch->GetX(), ch->GetY());
            return (ch);
        }
    }

    return nullptr;
}

void CHARACTER_MANAGER::SelectStone(CHARACTER *pkChr) { m_pkChrSelectedStone = pkChr; }

bool CHARACTER_MANAGER::SpawnMoveGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, int tx, int ty,
                                       regen *pkRegen, bool bAggressive_)
{
#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if (!dwVnum)
        return false;
#endif

    CMobGroup *pkGroup = CMobManager::Instance().GetGroup(dwVnum);

    if (!pkGroup)
    {
        SPDLOG_ERROR("NOT_EXIST_GROUP_VNUM({0}) Map({0}) ", dwVnum, lMapIndex);
        return false;
    }

    CHARACTER *pkChrMaster = nullptr;
    CParty *pkParty = nullptr;

    const std::vector<uint32_t> &c_rdwMembers = pkGroup->GetMemberVector();

    bool bSpawnedByStone = false;
    bool bAggressive = bAggressive_;

    if (m_pkChrSelectedStone)
    {
        bSpawnedByStone = true;
        if (m_pkChrSelectedStone->GetDungeon())
            bAggressive = true;
    }

    for (uint32_t i = 0; i < c_rdwMembers.size(); ++i)
    {
        CHARACTER *tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone);

        if (!tch)
        {
            if (i == 0) // 못만든 몬스터가 대장일 경우에는 그냥 실패
                return false;

            continue;
        }

        sx = tch->GetX() - Random::get(300, 500);
        sy = tch->GetY() - Random::get(300, 500);
        ex = tch->GetX() + Random::get(300, 500);
        ey = tch->GetY() + Random::get(300, 500);

        if (m_pkChrSelectedStone)
            tch->SetStone(m_pkChrSelectedStone);
        else if (pkParty)
        {
            pkParty->Join(tch->GetVID());
            pkParty->Link(tch);
        }
        else if (!pkChrMaster)
        {
            pkChrMaster = tch;
            pkChrMaster->SetRegen(pkRegen);

            pkParty = CPartyManager::instance().CreateParty(pkChrMaster);
        }
        if (bAggressive)
            tch->SetAggressive();

        tch->Goto(tx, ty);
    }

    return true;
}

bool CHARACTER_MANAGER::SpawnGroupGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, regen *pkRegen,
                                        bool bAggressive_, CDungeon *pDungeon)
{
#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if (!dwVnum)
        return false;
#endif

    const uint32_t dwGroupID = CMobManager::Instance().GetGroupFromGroupGroup(dwVnum);

    if (dwGroupID != 0) { return SpawnGroup(dwGroupID, lMapIndex, sx, sy, ex, ey, pkRegen, bAggressive_, pDungeon); }
    else
    {
        SPDLOG_ERROR("Spawn group group does not exist ({}) MAP({})", dwVnum, lMapIndex);
        return false;
    }
}

CHARACTER *CHARACTER_MANAGER::SpawnGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey,
                                         regen *pkRegen, bool bAggressive_, CDungeon *pDungeon, bool bRandom,
                                         CHARACTER *aggressiveTo)
{
#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if (!dwVnum)
        return nullptr;
#endif

    CMobGroup *pkGroup = CMobManager::Instance().GetGroup(dwVnum);

    if (!pkGroup)
    {
        SPDLOG_ERROR("SpawnGroup: Group does not exist ({}) Map({}) ", dwVnum, lMapIndex);
        return nullptr;
    }

    CHARACTER *pkChrMaster = nullptr;
    CParty *pkParty = nullptr;

    const std::vector<uint32_t> &c_rdwMembers = pkGroup->GetMemberVector();

    bool bSpawnedByStone = false;
    bool bAggressive = bAggressive_;

    if (m_pkChrSelectedStone)
    {
        bSpawnedByStone = true;

        if (m_pkChrSelectedStone->GetDungeon())
            bAggressive = true;
    }

    CHARACTER *chLeader = nullptr;

    for (uint32_t i = 0; i < c_rdwMembers.size(); ++i)
    {
        CHARACTER *tch;
        if (bRandom && i == 0)
            tch = SpawnMobRandomPosition(c_rdwMembers[i], lMapIndex);
        else
            tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone);

        if (!tch)
        {
            if (i == 0) // 못만든 몬스터가 대장일 경우에는 그냥 실패
                return nullptr;

            continue;
        }

        if (i == 0)
            chLeader = tch;

        tch->SetDungeon(pDungeon);

        sx = tch->GetX() - Random::get(300, 500);
        sy = tch->GetY() - Random::get(300, 500);
        ex = tch->GetX() + Random::get(300, 500);
        ey = tch->GetY() + Random::get(300, 500);

        if (m_pkChrSelectedStone)
            tch->SetStone(m_pkChrSelectedStone);
        else if (pkParty)
        {
            pkParty->Join(tch->GetVID());
            pkParty->Link(tch);
        }
        else if (!pkChrMaster)
        {
            pkChrMaster = tch;
            pkChrMaster->SetRegen(pkRegen);

            pkParty = CPartyManager::instance().CreateParty(pkChrMaster);
        }

        if (bAggressive)
            tch->SetAggressive();

        if (aggressiveTo)
            tch->SetVictim(aggressiveTo);
    }

    return chLeader;
}

void CHARACTER_MANAGER::Update(int iPulse)
{
        rmt_ScopedCPUSample(CMUpdate, 0);

    BeginPendingDestroy();

    bool resetChatCounter = !(iPulse % THECORE_SECS_TO_PASSES(5));

    std::for_each(m_map_pkPCChr.begin(), m_map_pkPCChr.end(),
                  [resetChatCounter, iPulse](const NAME_MAP::value_type &v)
                  {
                      CHARACTER *ch = v.second;

                      if (resetChatCounter)
                      {
                          ch->ResetChatCounter();
                          ch->ResetWhisperCounter();
                      }

                      ch->UpdateCharacter(iPulse);
                  });

    std::for_each(m_set_pkChrState.begin(), m_set_pkChrState.end(),
                  [iPulse](CHARACTER *ch) { ch->UpdateStateMachine(iPulse); });

    // Update to Santa
    {
        const auto chars = GetCharactersByRaceNum(xmas::MOB_SANTA_VNUM);
        for (auto* ch : chars)
            ch->UpdateStateMachine(iPulse);
    }

    // Record mob hunting counts once an hour
    if (0 == (iPulse % THECORE_SECS_TO_PASSES(3600)))
    {
        for (auto it = m_map_dwMobKillCount.begin(); it != m_map_dwMobKillCount.end(); ++it)
            DBManager::instance().SendMoneyLog(MONEY_LOG_MONSTER_KILL, it->first, it->second);

        m_map_dwMobKillCount.clear();
    }

    if (0 == (iPulse % THECORE_SECS_TO_PASSES(3600)))
    {
        const auto randomChat = Random::get(m_hourlyNotices);
        for (const auto &message : *(randomChat)) { SendNotice(message.c_str()); }
    }

    // The test server counts the number of characters every 60 seconds
    if (0 == (iPulse % THECORE_SECS_TO_PASSES(60))) {
        SPDLOG_INFO("CHARACTER COUNT vid {0} pid {1}", m_map_pkChrByVID.size(), m_map_pkChrByPID.size());
    }

    // To delayed DestroyCharacter
    FlushPendingDestroy();
}

void CHARACTER_MANAGER::ProcessDelayedSave()
{
    auto it = m_set_pkChrForDelayedSave.begin();
    while (it != m_set_pkChrForDelayedSave.end())
    {
        CHARACTER *pkChr = *it++;
        pkChr->SaveReal();
    }

    m_set_pkChrForDelayedSave.clear();
}

bool CHARACTER_MANAGER::AddToStateList(CHARACTER *ch)
{
    assert(ch != NULL);
    if (auto it = m_set_pkChrState.find(ch); it == m_set_pkChrState.end())
    {
        m_set_pkChrState.insert(ch);
        return true;
    }

    return false;
}

void CHARACTER_MANAGER::RemoveFromStateList(CHARACTER *ch)
{
    if (auto it = m_set_pkChrState.find(ch); it != m_set_pkChrState.end())
    {
        m_set_pkChrState.erase(it);
    }
}

void CHARACTER_MANAGER::DelayedSave(CHARACTER *ch) { m_set_pkChrForDelayedSave.insert(ch); }

bool CHARACTER_MANAGER::FlushDelayedSave(CHARACTER *ch)
{
    auto it = m_set_pkChrForDelayedSave.find(ch);

    if (it == m_set_pkChrForDelayedSave.end())
        return false;

    m_set_pkChrForDelayedSave.erase(it);
    ch->SaveReal();
    return true;
}

void CHARACTER_MANAGER::RegisterForMonsterLog(CHARACTER *ch) { m_set_pkChrMonsterLog.insert(ch); }

void CHARACTER_MANAGER::UnregisterForMonsterLog(CHARACTER *ch) { m_set_pkChrMonsterLog.erase(ch); }



void CHARACTER_MANAGER::KillLog(uint32_t dwVnum)
{
    const uint32_t SEND_LIMIT = 10000;

    auto it = m_map_dwMobKillCount.find(dwVnum);

    if (it == m_map_dwMobKillCount.end())
        m_map_dwMobKillCount.insert(std::make_pair(dwVnum, 1));
    else
    {
        ++it->second;

        if (it->second > SEND_LIMIT)
        {
            DBManager::instance().SendMoneyLog(MONEY_LOG_MONSTER_KILL, it->first, it->second);
            m_map_dwMobKillCount.erase(it);
        }
    }
}

void CHARACTER_MANAGER::RegisterRaceNum(uint32_t dwVnum)
{
    m_map_pkChrByRaceNum.emplace(std::piecewise_construct, std::forward_as_tuple(dwVnum), std::forward_as_tuple());
}

void CHARACTER_MANAGER::RegisterRaceNumMap(CHARACTER *ch)
{
    uint32_t vnum = ch->GetRaceNum();
    const auto it = m_map_pkChrByRaceNum.find(vnum);
    if (it == m_map_pkChrByRaceNum.end())
        return;

    it->second.insert(ch);
}

void CHARACTER_MANAGER::UnregisterRaceNumMap(CHARACTER *ch)
{
    uint32_t dwVnum = ch->GetRaceNum();

    const auto it = m_map_pkChrByRaceNum.find(dwVnum);
    if (it != m_map_pkChrByRaceNum.end())
        it->second.erase(ch);
}

CharacterSetSnapshot CHARACTER_MANAGER::GetCharactersByRaceNum(uint32_t dwRaceNum)
{
    const auto it = m_map_pkChrByRaceNum.find(dwRaceNum);
    if (it == m_map_pkChrByRaceNum.end())
        return CharacterSetSnapshot();

    return CharacterSetSnapshot(it->second);
}

int CHARACTER_MANAGER::CountCharactersByRaceNum(uint32_t dwRaceNum)
{
    auto it = m_map_pkChrByRaceNum.find(dwRaceNum);

    if (it == m_map_pkChrByRaceNum.end())
        return 0;

    return it->second.size();
}

#define FIND_JOB_WARRIOR_0 (1 << 3)
#define FIND_JOB_WARRIOR_1 (1 << 4)
#define FIND_JOB_WARRIOR_2 (1 << 5)
#define FIND_JOB_WARRIOR (FIND_JOB_WARRIOR_0 | FIND_JOB_WARRIOR_1 | FIND_JOB_WARRIOR_2)
#define FIND_JOB_ASSASSIN_0 (1 << 6)
#define FIND_JOB_ASSASSIN_1 (1 << 7)
#define FIND_JOB_ASSASSIN_2 (1 << 8)
#define FIND_JOB_ASSASSIN (FIND_JOB_ASSASSIN_0 | FIND_JOB_ASSASSIN_1 | FIND_JOB_ASSASSIN_2)
#define FIND_JOB_SURA_0 (1 << 9)
#define FIND_JOB_SURA_1 (1 << 10)
#define FIND_JOB_SURA_2 (1 << 11)
#define FIND_JOB_SURA (FIND_JOB_SURA_0 | FIND_JOB_SURA_1 | FIND_JOB_SURA_2)
#define FIND_JOB_SHAMAN_0 (1 << 12)
#define FIND_JOB_SHAMAN_1 (1 << 13)
#define FIND_JOB_SHAMAN_2 (1 << 14)
#define FIND_JOB_SHAMAN (FIND_JOB_SHAMAN_0 | FIND_JOB_SHAMAN_1 | FIND_JOB_SHAMAN_2)
#define FIND_JOB_WOLFMAN_0 (1 << 15)
#define FIND_JOB_WOLFMAN_1 (1 << 16)
#define FIND_JOB_WOLFMAN_2 (1 << 17)
#define FIND_JOB_WOLFMAN (FIND_JOB_WOLFMAN_0 | FIND_JOB_WOLFMAN_1 | FIND_JOB_WOLFMAN_2)
//
// (job+1)*3+(skill_group)
//
CHARACTER *CHARACTER_MANAGER::FindSpecifyPC(unsigned int uiJobFlag, long lMapIndex, CHARACTER *except, int iMinLevel,
                                            int iMaxLevel)
{
    CHARACTER *chFind = nullptr;
    int n = 0;

    for (auto it = m_map_pkChrByPID.begin(); it != m_map_pkChrByPID.end(); ++it)
    {
        CHARACTER *ch = it->second;

        if (ch == except)
            continue;

        if (ch->GetLevel() < iMinLevel)
            continue;

        if (ch->GetLevel() > iMaxLevel)
            continue;

        if (ch->GetMapIndex() != lMapIndex)
            continue;

        if (uiJobFlag)
        {
            unsigned int uiChrJob = (1 << ((ch->GetJob() + 1) * 3 + ch->GetSkillGroup()));

            if (!IS_SET(uiJobFlag, uiChrJob))
                continue;
        }

        if (!chFind || Random::get(1, ++n) == 1)
            chFind = ch;
    }

    return chFind;
}

int CHARACTER_MANAGER::GetMobItemRate(CHARACTER *ch)
{
    if (ch && ch->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0)
        return m_iMobItemRatePremium;
    return m_iMobItemRate;
}

int CHARACTER_MANAGER::GetMobDamageRate(CHARACTER *ch) { return m_iMobDamageRate; }

int CHARACTER_MANAGER::GetMobGoldAmountRate(CHARACTER *ch)
{
    if (!ch)
        return m_iMobGoldAmountRate;

    if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
        return m_iMobGoldAmountRatePremium;
    return m_iMobGoldAmountRate;
}

int CHARACTER_MANAGER::GetMobGoldDropRate(CHARACTER *ch)
{
    if (!ch)
        return m_iMobGoldDropRate;

    if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
        return m_iMobGoldDropRatePremium;
    return m_iMobGoldDropRate;
}

int CHARACTER_MANAGER::GetMobExpRate(CHARACTER *ch)
{
    if (!ch)
        return m_iMobExpRate;

    if (ch && ch->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
        return m_iMobExpRatePremium;
    return m_iMobExpRate;
}

int CHARACTER_MANAGER::GetUserDamageRate(CHARACTER *ch)
{
    if (!ch)
        return m_iUserDamageRate;

    if (ch && ch->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
        return m_iUserDamageRatePremium;

    return m_iUserDamageRate;
}

void CHARACTER_MANAGER::SendScriptToMap(long lMapIndex, const std::string &s)
{
    SECTREE_MAP *pSecMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);

    if (nullptr == pSecMap)
        return;

    packet_script p;
    p.skin = 1;
    p.script = s;

    pSecMap->for_each([p](CEntity* ent) {
    if (ent->IsType(ENTITY_CHARACTER))
    {
        CHARACTER *ch = (CHARACTER *)ent;

        if (ch->GetDesc()) { ch->GetDesc()->Send(HEADER_GC_SCRIPT, p); }
    }
});
}

bool CHARACTER_MANAGER::BeginPendingDestroy()
{
    // Begin 이 후에 Begin을 또 하는 경우에 Flush 하지 않는 기능 지원을 위해
    // 이미 시작되어있으면 false 리턴 처리
    if (m_bUsePendingDestroy)
        return false;

    m_bUsePendingDestroy = true;
    return true;
}

void CHARACTER_MANAGER::FlushPendingDestroy()
{
    m_bUsePendingDestroy = false; // Flags must be set first before actual Destroy processing

    if (!m_set_pkChrPendingDestroy.empty())
    {
        auto it = m_set_pkChrPendingDestroy.begin(), end = m_set_pkChrPendingDestroy.end();

        for (; it != end; ++it)
            M2_DESTROY_CHARACTER(*it);

        m_set_pkChrPendingDestroy.clear();
    }
}

void CHARACTER_MANAGER::DestroyCharacterInMap(long lMapIndex)
{
    std::vector<CHARACTER *> tempVec;
    for (auto &it : m_map_pkChrByVID)
    {
        CHARACTER *pkChr = it.second;
        if (pkChr && pkChr->GetMapIndex() == lMapIndex && pkChr->IsNPC() && !pkChr->IsPet() &&
            pkChr->GetRider() == nullptr && !pkChr->IsToggleMount()) { tempVec.push_back(pkChr); }
    }
    for (auto &it : tempVec) { DestroyCharacter(it); }
}

bool CHARACTER_MANAGER::LoadAutoNotices()
{
    std::ifstream inf("data/auto_notice.txt");

    if (!inf.is_open())
    {
        SPDLOG_ERROR("LoadAutoNotices cannot open 'auto_notice.txt'.");
        return false;
    }

    std::string ln;
    while (getline(inf, ln))
    {
        if (ln.empty())
            continue;
        std::vector<std::string> temp;
        if (absl::StrContains(ln, "[ENTER]"))
        {
            std::vector<std::string> v = absl::StrSplit(ln, "[ENTER]");
            m_hourlyNotices.emplace_back(v);
        }
        else
        {
            temp.push_back(ln);
            m_hourlyNotices.emplace_back(temp);
        }
    }
    return true;
}

#ifdef __WORLD_BOSS__
bool CHARACTER_MANAGER::LoadWorldBossInfo(const char *szFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(szFileName)) { return false; }

    for (const auto &p : reader.GetChildren())
    {
        const auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;
        const auto grp = static_cast<GroupTextGroup *>(node);

        TWorldBossInfo kWorldBossInfo = {};

        // min/max level are optional
        uint32_t day = 0;
        uint32_t hour = 0;
        uint32_t mob = std::numeric_limits<uint32_t>::max();
        GetGroupProperty(grp, "day", kWorldBossInfo.bDay);
        GetGroupProperty(grp, "hour", kWorldBossInfo.bHour);
        GetGroupProperty(grp, "mob", kWorldBossInfo.dwRaceNum);
        GetGroupProperty(grp, "daily", kWorldBossInfo.bIsDaily);

        // get maps
        std::vector<WorldBossPosition> vecMapIndexes;
        std::vector<std::string> vecMapNames;

        for (int k = 1; k < 256; ++k)
        {
            char buf[4];
            snprintf(buf, sizeof(buf), "%d", k);

            const auto tokens = grp->GetTokens(buf);
            if (!tokens)
                break;
            if (tokens->size() > 3)
            {
                vecMapNames.push_back(tokens->at(0));
                int iMapIndex = 0;
                str_to_number(iMapIndex, tokens->at(1).c_str());
                int x = 0;
                str_to_number(x, tokens->at(2).c_str());
                int y = 0;
                str_to_number(y, tokens->at(3).c_str());

                vecMapIndexes.emplace_back(iMapIndex, x, y);
            }
        }

        // alloc map memory
        kWorldBossInfo.iMapCount = vecMapIndexes.size();

        // save maps
        kWorldBossInfo.piMapIndexList.swap(vecMapIndexes);
        kWorldBossInfo.pszMapNameList.swap(vecMapNames);

        m_vec_kWorldBossInfo.emplace_back(kWorldBossInfo);
    }

    return true;
}

void CHARACTER_MANAGER::UpdateWorldBoss()
{
    if (!gConfig.worldBossMaster)
    {
        return;
    }

    std::time_t ti = std::time(nullptr);
    auto* pTimeInfo = std::localtime(&ti);

    for (int i = 0; i < m_vec_kWorldBossInfo.size(); ++i)
    {
        TWorldBossInfo &rkInfo = m_vec_kWorldBossInfo[i];
        if (rkInfo.bIsDaily)
        {
            if (rkInfo.bHour == pTimeInfo->tm_hour && rkInfo.iLastSpawnHour != pTimeInfo->tm_hour)
            {
                if (SpawnWorldBoss(i))
                    rkInfo.iLastSpawnHour = pTimeInfo->tm_hour;
            } else if(rkInfo.iLastSpawnHour != pTimeInfo->tm_hour) {
                    rkInfo.iLastSpawnHour = -1;
            }
        }
        else
        {
            if (rkInfo.bDay == pTimeInfo->tm_wday && rkInfo.bHour >= pTimeInfo->tm_hour &&
                rkInfo.iLastSpawnDay != pTimeInfo->tm_yday)
            {
                if (SpawnWorldBoss(i))
                    rkInfo.iLastSpawnDay = pTimeInfo->tm_yday;
            } else if(rkInfo.iLastSpawnDay != pTimeInfo->tm_yday) {
                    rkInfo.iLastSpawnDay = -1;
            }
        }
    }
}

bool CHARACTER_MANAGER::SpawnWorldBoss(int iIndex, int iMapIndex, uint32_t x, uint32_t y)
{
    TWorldBossInfo &rkInfo = m_vec_kWorldBossInfo[iIndex];

    int iMapNumber = -1;
    if (!iMapIndex)
    {
        iMapNumber = Random::get<int32_t>(0, rkInfo.iMapCount - 1);
        iMapIndex = rkInfo.piMapIndexList[iMapNumber].mapIndex;
        x = rkInfo.piMapIndexList[iMapNumber].x;
        y = rkInfo.piMapIndexList[iMapNumber].y;
    }

    if (!gConfig.IsHostingMap(iMapIndex))
    {
        std::string lAddr;
        uint16_t wPort;
        if (!CMapLocation::instance().Get(iMapIndex, lAddr, wPort))
        {
            SPDLOG_ERROR("cannot spawn world boss - no map location for map index {}", iMapIndex);
            return false;
        }

        GmWorldBossPacket p;
        p.bossIndex = iIndex;
        p.mapIndex = iMapIndex;
        p.x = x;
        p.y = y;
        DESC_MANAGER::instance().GetMasterSocket()->Send(kGmWorldBoss, p);

        if (iMapNumber >= 0)
        {
            BroadcastNoticeSpecial(
                fmt::format("WorldBoss [%s] haritasında ortaya çıktı.#{}", rkInfo.pszMapNameList[iMapNumber]).c_str());
        }

        return true;
    }

	DestroyCharacterInMap(iMapIndex);
	
    auto pkMob = g_pCharManager->SpawnMob(rkInfo.dwRaceNum, iMapIndex, x, y, 0);
    if (!pkMob) { SPDLOG_ERROR("cannot spawn world boss on map {}", iMapIndex); }
    else
    {
        if (iMapNumber >= 0)
        {
            BroadcastNoticeSpecial(
                fmt::format("WorldBoss [%s] haritasında ortaya çıktı.#{}", rkInfo.pszMapNameList[iMapNumber]).c_str());
        }

        SPDLOG_INFO("Spawn WORLD_BOSS {} on map {} position {}, {}", rkInfo.dwRaceNum, iMapIndex, pkMob->GetX(),
                    pkMob->GetY());
    }

    return true;
}
#endif

CharacterSnapshotGuard::CharacterSnapshotGuard()
    : m_hasPendingOwnership(g_pCharManager->BeginPendingDestroy())
{
    // ctor
}

CharacterSnapshotGuard::~CharacterSnapshotGuard() noexcept
{
    if (m_hasPendingOwnership)
        g_pCharManager->FlushPendingDestroy();
}

CharacterSetSnapshot::CharacterSetSnapshot()
    : m_chars(nullptr)
{
    // ctor
}

CharacterSetSnapshot::CharacterSetSnapshot(const std::unordered_set<CHARACTER *> &chars)
    : m_chars(&chars)
{
    // ctor
}

std::unordered_set<CHARACTER *>::const_iterator CharacterSetSnapshot::begin() const
{
    if (m_chars)
        return m_chars->begin();

    return {};
}

std::unordered_set<CHARACTER *>::const_iterator CharacterSetSnapshot::end() const
{
    if (m_chars)
        return m_chars->end();

    return {};
}
