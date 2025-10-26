#include "Cache.h"
#include "ClientManager.h"
#include "ItemAwardManager.h"
#include "Main.h"
#include "Persistence.hpp"
#include "PlayerLoader.hpp"
#include "QID.h"
#include <fmt/compile.h>

#include "game/GamePacket.hpp"

extern std::string g_stLocale;
extern int g_test_server;
extern int g_log;

/*
 * PLAYER LOAD
 */
void CClientManager::QUERY_PLAYER_LOAD(CPeer* peer, uint32_t dwHandle,
                                       TPlayerLoadPacket* packet)
{
    // Attempt to grab a cached player here, because the PlayerLoader
    // constructor will create one.
    auto c = m_playerCache.Get(packet->player_id);

    auto loader = new PlayerLoader(*this, dwHandle, peer->GetHandle(),
                                   packet->account_id, packet->player_id);

    if (c) {
        loader->SetTable(c->GetTable());

        // These are deliberately empty so we don't overwrite the data
        // cached in the player table.
        loader->SetSkills({});
        loader->SetQuickslots({});
        loader->SetTitle(c->GetTable().title);

        // Collect our items
        const auto& cachedItems = c->GetItems();

        std::vector<TPlayerItem> items;
        items.reserve(cachedItems.size());

        for (const auto& cachedItem : cachedItems)
            items.push_back(cachedItem->GetData());

        loader->SetItems(std::move(items));
    } else {
        std::string query;
        CreatePlayerLoadQuery(packet->player_id, query);
        CDBManager::instance().ReturnQuery(query, QID_PLAYER, peer->GetHandle(),
                                           loader);

        query.clear();
        CreateSkillLoadQuery(packet->player_id, query);
        CDBManager::instance().ReturnQuery(query, QID_SKILL, peer->GetHandle(),
                                           loader);

        query.clear();
        CreateTitleLoadQuery(packet->player_id, query);
        CDBManager::instance().ReturnQuery(query, QID_TITLE, peer->GetHandle(),
                                           loader);

        query.clear();
        CreateQuickslotLoadQuery(packet->player_id, query);
        CDBManager::instance().ReturnQuery(query, QID_QUICKSLOT,
                                           peer->GetHandle(), loader);

        CDBManager::instance().ReturnQuery(
            fmt::format("SELECT id,`window`+0,pos,count,vnum,trans_vnum,"
                        "seal_date,"
                        "is_gm_owner, is_blocked, price, "
                        "socket0,socket1,socket2,socket3,socket4,socket5,"
                        "attrtype0,attrvalue0,attrtype1,attrvalue1,"
                        "attrtype2,attrvalue2,attrtype3,attrvalue3,"
                        "attrtype4,attrvalue4,attrtype5,attrvalue5,"
                        "attrtype6,attrvalue6 "
                        "FROM item WHERE owner_id={} AND (`window` < {} OR "
                        "`window` = {} OR `window` = {}) ORDER BY pos",
                        packet->player_id, SAFEBOX, DRAGON_SOUL_INVENTORY,
                        EWindows::SWITCHBOT),
            QID_ITEM, peer->GetHandle(), loader);
    }

    CDBManager::instance().ReturnQuery(fmt::format("SELECT "
                                                   "bType,bSubType,bApplyOn,lApplyValue,"
                                                   "lDuration,lSPCost,color "
                                                   "FROM affect WHERE dwPID={}",
                                                   packet->player_id),
                                       QID_AFFECT, peer->GetHandle(), loader);

    CDBManager::instance().ReturnQuery(
        fmt::format("SELECT slot, status, "
                    "attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,"
                    "attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,"
                    "attrtype5,attrvalue5,attrtype6,attrvalue6,"
                    "attrtype0_alt1,attrvalue0_alt1,attrtype1_alt1,attrvalue1_"
                    "alt1,attrtype2_alt1,attrvalue2_alt1,attrtype3_alt1,"
                    "attrvalue3_alt1,attrtype4_alt1,attrvalue4_alt1,attrtype5_"
                    "alt1,attrvalue5_alt1,attrtype6_alt1,attrvalue6_alt1,"
                    "attrtype0_alt2,attrvalue0_alt2,attrtype1_alt2,attrvalue1_"
                    "alt2,attrtype2_alt2,attrvalue2_alt2,attrtype3_alt2,"
                    "attrvalue3_alt2,attrtype4_alt2,attrvalue4_alt2,attrtype5_"
                    "alt2,attrvalue5_alt2,attrtype6_alt2,attrvalue6_alt2"
                    " FROM player_switchbot_data WHERE player_id={}",
                    packet->player_id),
        QID_SWITCHBOT_SLOT_DATA, peer->GetHandle(), loader);

    CDBManager::instance().ReturnQuery(fmt::format("SELECT "
                                                   "dwPID,szName,szState,"
                                                   "lValue FROM quest WHERE "
                                                   "dwPID={} AND lValue<>0",
                                                   packet->player_id),
                                       QID_QUEST, peer->GetHandle(), loader);

    CDBManager::instance().ReturnQuery(
        fmt::format("SELECT messenger_block.other_pid, messenger_block.mode, "
                    "player.name FROM messenger_block JOIN player ON "
                    "player.id = messenger_block.other_pid "
                    "WHERE messenger_block.pid={}",
                    packet->player_id),
        QID_MESSENGER_BLOCK, peer->GetHandle(), loader);
#ifdef ENABLE_BATTLE_PASS
    // Load all missions from table
    char queryStrBP[QUERY_MAX_LEN];
    snprintf(queryStrBP, sizeof(queryStrBP),
             "SELECT player_id, mission_id, battle_pass_id, extra_info, "
             "completed FROM battle_pass WHERE player_id = %d",
             packet->player_id);
    CDBManager::instance().ReturnQuery(queryStrBP, QID_BATTLE_PASS,
                                       peer->GetHandle(), loader);
#endif

    CDBManager::instance().ReturnQuery(fmt::format("SELECT player_id, "
                                                   "mission_id, kill_count, "
                                                   "day, completed FROM "
                                                   "hunting_missions WHERE "
                                                   "player_id = {}",
                                                   packet->player_id),
                                       QID_HUNTING_MISSIONS, peer->GetHandle(),
                                       loader);

    SendDungeonInfoCacheSet(peer, loader);
}

void CClientManager::RESULT_COMPOSITE_PLAYER(CPeer* peer, SQLMsg* pMsg,
                                             uint32_t dwQID)
{
    CQueryInfo* qi = (CQueryInfo*)pMsg->pvUserData;
    auto loader = (PlayerLoader*)qi->pvData;

    MYSQL_RES* pSQLResult = pMsg->Get()->pSQLResult;
    if (!pSQLResult) {
        SPDLOG_ERROR("AID {0}/PID {1}: NULL MYSQL_RES for QID {2}",
                      loader->GetAid(), loader->GetPid(), dwQID);

        switch (dwQID) {
            case QID_PLAYER:
                loader->Fail();
                loader->SetTable({});
                return;

            case QID_SKILL:
                loader->Fail();
                loader->SetSkills({});
                return;

            case QID_QUICKSLOT:
                loader->Fail();
                loader->SetQuickslots({});
                return;

            case QID_ITEM:
                loader->Fail();
                loader->SetItems({});
                return;

            case QID_QUEST:
                loader->Fail();
                loader->SetQuests({});
                return;

            case QID_AFFECT:
                loader->Fail();
                loader->SetAffects({});
                return;

            case QID_SWITCHBOT_SLOT_DATA:
                loader->Fail();
                loader->SetSwitchbotData({});
                return;

            case QID_TITLE:
                loader->Fail();
                loader->SetTitle({});
                return;

            case QID_MESSENGER_BLOCK:
                loader->Fail();
                loader->SetBlocked({});
                return;
#ifdef ENABLE_BATTLE_PASS
            case QID_BATTLE_PASS:
                loader->Fail();
                loader->SetBattlePassMissions({});
                return;
#endif
            case QID_HUNTING_MISSIONS:
                loader->Fail();
                loader->SetHuntingMissions({});
                return;
            case QID_DUNGEON_INFO:
                loader->Fail();
                loader->SetDungeonInfos({});
                return;
        }
    }

    switch (dwQID) {
        case QID_PLAYER:
            SPDLOG_TRACE("{0}: QID_PLAYER", loader->GetPid());
            RESULT_PLAYER_LOAD(peer, pSQLResult, loader);
            break;

        case QID_SKILL:
            SPDLOG_TRACE("{0}: QID_SKILL", loader->GetPid());
            RESULT_SKILL_LOAD(peer, pSQLResult, loader);
            break;

        case QID_QUICKSLOT:
            SPDLOG_TRACE("{0}: QID_QUICKSLOT", loader->GetPid());
            RESULT_QUICKSLOT_LOAD(peer, pSQLResult, loader);
            break;

        case QID_ITEM:
            SPDLOG_TRACE("{0}: QID_ITEM", loader->GetPid());
            RESULT_ITEM_LOAD(peer, pSQLResult, loader);
            break;

#ifdef ENABLE_BATTLE_PASS
        case QID_BATTLE_PASS:
            SPDLOG_TRACE("{0}: QID_BATTLE_PASS", loader->GetPid());
            RESULT_BATTLE_PASS_LOAD(peer, pSQLResult, loader);
            break;
#endif

        case QID_HUNTING_MISSIONS:
            SPDLOG_TRACE("{0}: QID_HUNTING_MISSIONS", loader->GetPid());
            HandleResultHuntingMissionLoad(peer, pSQLResult, loader);
            break;

        case QID_QUEST:
            SPDLOG_TRACE("{0}: QID_QUEST", loader->GetPid());
            RESULT_QUEST_LOAD(peer, pSQLResult, loader);
            break;

        case QID_AFFECT:
            SPDLOG_TRACE("{0}: QID_AFFECT", loader->GetPid());
            RESULT_AFFECT_LOAD(peer, pSQLResult, loader);
            break;

        case QID_SWITCHBOT_SLOT_DATA:
            SPDLOG_TRACE("{0}: QID_SWITCHBOT_SLOT_DATA", loader->GetPid());
            HandleResultSwitchbotDataLoad(peer, pSQLResult, loader);
            break;

        case QID_TITLE:
            SPDLOG_TRACE("{0}: QID_TITLE", loader->GetPid());
            RESULT_TITLE_LOAD(peer, pSQLResult, loader);
            break;
        case QID_MESSENGER_BLOCK:
            SPDLOG_TRACE("{0}: QID_MESSENGER_BLOCK", loader->GetPid());
            RESULT_MESSENGER_BLOCK(peer, pSQLResult, loader);
            break;
        case QID_DUNGEON_INFO:
            RESULT_DUNGEON_INFO_LOAD(peer, pSQLResult, loader);
            break;
    }
}
void CClientManager::RESULT_PLAYER_LOAD(CPeer* peer, MYSQL_RES* pRes,
                                        PlayerLoader* loader)
{
    TPlayerTable tab = {};
    if (!CreatePlayerTableFromRes(pRes, tab)) {
        loader->Fail();
        loader->SetTable(tab);
        return;
    }

    STORM_ASSERT(tab.id == loader->GetPid(), "Sanity");

    SPDLOG_TRACE("Loaded player table for {0}", loader->GetPid());

    loader->SetTable(tab);
}

void CClientManager::RESULT_SKILL_LOAD(CPeer* peer, MYSQL_RES* res,
                                       PlayerLoader* loader)
{
    std::vector<SkillPair> skills;
    CreateSkillTableFromRes(res, skills);

    SPDLOG_TRACE("Loaded {0} skills for {1}", skills.size(), loader->GetPid());

    loader->SetSkills(std::move(skills));
}

void CClientManager::RESULT_QUICKSLOT_LOAD(CPeer* peer, MYSQL_RES* res,
                                           PlayerLoader* loader)
{
    std::vector<QuickslotPair> quickslots;
    CreateQuickslotTableFromRes(res, quickslots);

    SPDLOG_TRACE("Loaded {0} quickslots for {1}", quickslots.size(),
                 loader->GetPid());

    loader->SetQuickslots(std::move(quickslots));
}

void CClientManager::RESULT_ITEM_LOAD(CPeer* peer, MYSQL_RES* pRes,
                                      PlayerLoader* loader)
{
    std::vector<TPlayerItem> items;
    CreateItemTableFromRes(pRes, loader->GetPid(), items);

    SPDLOG_TRACE("Loaded {0} items for {1}", items.size(), loader->GetPid());

    std::vector<CachedItem*> cachedItems;
    cachedItems.reserve(items.size());

    for (const auto& item : items) {
        auto& cachedItem = m_itemCache.ForceGet(item.id);
        cachedItem.SetData(item, false);
        cachedItem.Lock();
        cachedItems.push_back(&cachedItem);
    }

    loader->GetPlayer().SetItems(std::move(cachedItems), false);
    loader->SetItems(std::move(items));
}

void CClientManager::HandleResultSwitchbotDataLoad(CPeer* peer, MYSQL_RES* pRes,
                                                   PlayerLoader* loader)
{
    std::size_t count = mysql_num_rows(pRes);

    std::vector<SwitchBotSlotData> slotData;
    slotData.resize(count);

    for (std::size_t i = 0; i != count; ++i) {
        auto& r = slotData[i];
        MYSQL_ROW row = mysql_fetch_row(pRes);

        auto j = 0;
        str_to_number(r.slot, row[j++]);
        str_to_number(r.status, row[j++]);

        for (auto& tab : r.attr) {
            for (auto& attr : tab) {
                str_to_number(attr.bType, row[j++]);
                str_to_number(attr.lValue, row[j++]);
            }
        }
    }

    loader->SetSwitchbotData(std::move(slotData));
}

void CClientManager::RESULT_AFFECT_LOAD(CPeer* peer, MYSQL_RES* pRes,
                                        PlayerLoader* loader)
{
    std::size_t count = mysql_num_rows(pRes);

    std::vector<AffectData> affects;
    affects.resize(count);

    for (std::size_t i = 0; i != count; ++i) {
        auto& r = affects[i];
        MYSQL_ROW row = mysql_fetch_row(pRes);

        str_to_number(r.type, row[0]);
        str_to_number(r.subType, row[1]);
        str_to_number(r.pointType, row[2]);
        str_to_number(r.pointValue, row[3]);
        str_to_number(r.duration, row[4]);
        str_to_number(r.spCost, row[5]);
        str_to_number(r.color, row[6]);
    }

    loader->SetAffects(std::move(affects));
}

void CClientManager::RESULT_QUEST_LOAD(CPeer* peer, MYSQL_RES* pRes,
                                       PlayerLoader* loader)
{
    std::size_t count = mysql_num_rows(pRes);

    std::vector<TQuestTable> quests;
    quests.resize(count);

    for (std::size_t i = 0; i != count; ++i) {
        auto& r = quests[i];
        MYSQL_ROW row = mysql_fetch_row(pRes);

        str_to_number(r.dwPID, row[0]);
        strlcpy(r.szName, row[1], sizeof(r.szName));
        strlcpy(r.szState, row[2], sizeof(r.szState));
        str_to_number(r.lValue, row[3]);
    }

    loader->SetQuests(std::move(quests));
}

void CClientManager::RESULT_MESSENGER_BLOCK(CPeer* peer, MYSQL_RES* res,
                                            PlayerLoader* loader)
{
    std::size_t count = std::min<std::size_t>(0xffff, mysql_num_rows(res));

    std::vector<ElementLoadBlockedPC> blocked;
    blocked.reserve(count);

    for (std::size_t i = 0; i != count; ++i) {
        ElementLoadBlockedPC r = {};
        MYSQL_ROW row = mysql_fetch_row(res);

        str_to_number(r.pid, row[0]);
        str_to_number(r.data.mode, row[1]);
        strlcpy(r.data.name, row[2], sizeof(r.data.name));
        blocked.emplace_back(r);
    }

    loader->SetBlocked(std::move(blocked));
}

void CClientManager::RESULT_TITLE_LOAD(CPeer* peer, MYSQL_RES* pRes,
                                       PlayerLoader* loader)
{
    uint32_t iNumRows = (uint32_t)mysql_num_rows(pRes);

    if (iNumRows == 0) {
        loader->SetTitle({});
        return;
    }

    if (iNumRows > 1)
        spdlog::error("More than 1 row when loading title ({0}) Loading first "
                      "and ignoring the rest.",
                      loader->GetPid());

    // today_pvp, today_pve, today_other, today_gk, total, last_update
    MYSQL_ROW row = mysql_fetch_row(pRes);

    TPlayerTitle pt = {};
    strncpy(pt.title, row[0], 40);
    storm::ParseNumber(row[1], pt.color);
    loader->SetTitle(pt);
}

#ifdef ENABLE_BATTLE_PASS
void CClientManager::RESULT_BATTLE_PASS_LOAD(CPeer* peer, MYSQL_RES* pRes,
                                             PlayerLoader* loader)
{
    std::size_t count = mysql_num_rows(pRes);

    std::vector<TPlayerBattlePassMission> missions;
    missions.resize(count);

    for (std::size_t i = 0; i != count; ++i) {
        auto& r = missions[i];
        MYSQL_ROW row = mysql_fetch_row(pRes);

        int col = 0;

        str_to_number(r.dwPlayerId, row[col++]);
        str_to_number(r.dwMissionId, row[col++]);
        str_to_number(r.dwBattlePassId, row[col++]);
        str_to_number(r.dwExtraInfo, row[col++]);
        str_to_number(r.bCompleted, row[col++]);
        r.bIsUpdated = 0;
    }
    loader->SetBattlePassMissions(missions);
}

void CClientManager::QUERY_SAVE_BATTLE_PASS(
    CPeer* peer, uint32_t dwHandle, TPlayerBattlePassMission* battlePass)
{
    const auto szQuery =
        fmt::format("REPLACE INTO battle_pass (player_id, mission_id, "
                    "battle_pass_id, extra_info, completed) VALUES ({}, {}, "
                    "{}, {}, {})",
                    battlePass->dwPlayerId, battlePass->dwMissionId,
                    battlePass->dwBattlePassId, battlePass->dwExtraInfo,
                    battlePass->bCompleted ? 1 : 0);
    CDBManager::instance().AsyncQuery(szQuery);
}

bool CClientManager::LoadBattlePassRanking()
{
    const auto query = fmt::format("SELECT player_name, "
                                   "UNIX_TIMESTAMP(finish_time) "
                                   "FROM battle_pass_ranking WHERE "
                                   "battle_pass_id = {} ORDER BY finish_time "
                                   "ASC LIMIT 45;",
                                   m_config.battlepassId);

    std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery(query));
    SQLResult* pRes = pkMsg->Get();

    if (!pRes->uiNumRows)
        return true; // No ranking for battle pass with id %i%

    if (!m_vec_battlePassRanking.empty())
        m_vec_battlePassRanking.clear();

    uint8_t posContor = 1;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(pRes->pSQLResult))) {
        TBattlePassRanking* ranking = new TBattlePassRanking;
        ranking->bPos = posContor;
        strlcpy(ranking->playerName, row[0], sizeof(ranking->playerName));
        str_to_number(ranking->dwFinishTime, row[1]);

        m_vec_battlePassRanking.push_back(ranking);
        posContor++;
    }

    return true;
}

void CClientManager::RequestLoadBattlePassRanking(CPeer* peer,
                                                  uint32_t dwHandle,
                                                  const char* data)
{
    uint32_t dwPlayerID = *(uint32_t*)data;
    data += sizeof(uint32_t);

    uint8_t bIsGlobal = *(uint8_t*)data;
    data += sizeof(uint8_t);

    if (m_vec_battlePassRanking.empty()) {
        uint32_t dwCount = 0;
        TBattlePassRanking pbpTable = {0};

        peer->EncodeHeader(HEADER_DG_BATTLE_PASS_LOAD_RANKING, dwHandle,
                           sizeof(uint32_t) + sizeof(uint8_t) +
                               sizeof(uint32_t) +
                               sizeof(TBattlePassRanking) * dwCount);
        peer->Encode(&dwPlayerID, sizeof(uint32_t));
        peer->Encode(&bIsGlobal, sizeof(uint8_t));
        peer->Encode(&dwCount, sizeof(uint32_t));
        peer->Encode(&pbpTable, sizeof(TBattlePassRanking) * dwCount);
        return;
    } else {
        static std::vector<TBattlePassRanking> sendVector;
        sendVector.resize(m_vec_battlePassRanking.size());

        MYSQL_ROW row;

        for (int i = 0; i < m_vec_battlePassRanking.size(); ++i) {
            TBattlePassRanking* pkRanking = m_vec_battlePassRanking[i];
            TBattlePassRanking& r = sendVector[i];

            r.bPos = pkRanking->bPos;
            strlcpy(r.playerName, pkRanking->playerName, sizeof(r.playerName));
            r.dwFinishTime = pkRanking->dwFinishTime;
        }

        uint32_t dwCount = sendVector.size();

        peer->EncodeHeader(HEADER_DG_BATTLE_PASS_LOAD_RANKING, dwHandle,
                           sizeof(uint32_t) + sizeof(uint8_t) +
                               sizeof(uint32_t) +
                               sizeof(TBattlePassRanking) * dwCount);
        peer->Encode(&dwPlayerID, sizeof(uint32_t));
        peer->Encode(&bIsGlobal, sizeof(uint8_t));
        peer->Encode(&dwCount, sizeof(uint32_t));
        peer->Encode(&sendVector[0], sizeof(TBattlePassRanking) * dwCount);
    }
}

void CClientManager::QUERY_REGISTER_RANKING(
    CPeer* peer, uint32_t dwHandle, TBattlePassRegisterRanking* pRanking)
{
    CDBManager::instance().AsyncQuery(fmt::format("REPLACE INTO battle_pass_ranking (player_name, battle_pass_id, "
             "finish_time) VALUES ('{}', {}, NOW())",
             pRanking->playerName, pRanking->bBattlePassId));
}
#endif

void CClientManager::HandleResultHuntingMissionLoad(CPeer* peer,
                                                    MYSQL_RES* pRes,
                                                    PlayerLoader* loader)
{
    std::size_t count = mysql_num_rows(pRes);

    std::vector<TPlayerHuntingMission> missions;
    missions.resize(count);

    for (std::size_t i = 0; i != count; ++i) {
        auto& r = missions[i];
        MYSQL_ROW row = mysql_fetch_row(pRes);

        int col = 0;

        str_to_number(r.playerId, row[col++]);
        str_to_number(r.missionId, row[col++]);
        str_to_number(r.killCount, row[col++]);
        str_to_number(r.day, row[col++]);
        str_to_number(r.bCompleted, row[col++]);
        r.bIsUpdated = 0;
    }
    loader->SetHuntingMissions(missions);
}

void CClientManager::QuerySaveHuntingMissions(CPeer* peer, uint32_t dwHandle,
                                              TPlayerHuntingMission* mission)
{
    CDBManager::instance().AsyncQuery(
        fmt::format("REPLACE INTO hunting_missions (player_id, mission_id, "
                    "kill_count, day, completed) VALUES ({}, {}, {}, {}, {})",
                    mission->playerId, mission->missionId, mission->killCount,
                    mission->day, mission->bCompleted ? 1 : 0));
}

/*
 * PLAYER SAVE
 */
void CClientManager::QUERY_PLAYER_SAVE(CPeer* peer, uint32_t dwHandle,
                                       TPlayerTable* pkTab)
{
    SPDLOG_TRACE("Saving {0} ({1})", pkTab->id, pkTab->name);

    auto cachedPlayer = m_playerCache.Get(pkTab->id);
    if (!cachedPlayer) {
        spdlog::error("Failed to get cached player for {0}", pkTab->id);

        std::string query;
        CreatePlayerSaveQuery(*pkTab, query);
        CDBManager::instance().ReturnQuery(query, QID_PLAYER_SAVE, 0, NULL);
        return;
    }

    cachedPlayer->SetTable(*pkTab);

    // Lengthen |cachedPlayer|'s lifetime if it's expiring.
    cachedPlayer->Touch();
}

/*
 * PLAYER CREATE
 */
void CClientManager::__QUERY_PLAYER_CREATE(CPeer* peer, uint32_t dwHandle,
                                           TPlayerCreatePacket* packet)
{
    static std::unordered_map<uint32_t, time_t> s_createTimeByAccountID;
    auto it = s_createTimeByAccountID.find(packet->account_id);
    if (it != s_createTimeByAccountID.end()) {
        time_t curtime = time(0);

        if (curtime - it->second < 30) {
            peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 4);
            peer->EncodeDWORD(
                GcCharacterPhaseFailurePacket::CreateType::kTimeLimit);
            return;
        }
    }

    std::unique_ptr<SQLMsg> pMsg0(CDBManager::instance().DirectQuery(
        fmt::format("SELECT id FROM player WHERE slot={} AND account_id={} AND "
                    "deleted=0",
                    packet->slot, packet->account_id)));

    if (pMsg0->Get()->uiNumRows > 0) {
        if (!pMsg0->Get()->pSQLResult) {
            peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 4);
            peer->EncodeDWORD(
                GcCharacterPhaseFailurePacket::CreateType::kEinval);
            return;
        }

        MYSQL_ROW row = mysql_fetch_row(pMsg0->Get()->pSQLResult);

        uint32_t dwPID = 0;
        str_to_number(dwPID, row[0]);
        peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 4);
        peer->EncodeDWORD(
            GcCharacterPhaseFailurePacket::CreateType::kAlreadySlot);
        SPDLOG_INFO("ALREADY EXIST AccountChrIdx {0} ID {1}", packet->slot,
                    dwPID);
        return;
    }

    std::unique_ptr<SQLMsg> pMsg1(CDBManager::instance().DirectQuery(
        fmt::format("SELECT COUNT(*) as count FROM player WHERE name='{}' AND "
                    "deleted = 0",
                    packet->player_table.name)));

    if (pMsg1->Get()->uiNumRows) {
        if (!pMsg1->Get()->pSQLResult) {
            peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 4);
            peer->EncodeDWORD(
                GcCharacterPhaseFailurePacket::CreateType::kEinval);
            return;
        }

        MYSQL_ROW row = mysql_fetch_row(pMsg1->Get()->pSQLResult);

        if (*row[0] != '0') {
            SPDLOG_INFO("ALREADY EXIST name {0}, row[0] {1} query {2}",
                        packet->player_table.name, row[0], queryStr);
            peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 4);
            peer->EncodeDWORD(
                GcCharacterPhaseFailurePacket::CreateType::kAlready);
            return;
        }
    } else {
        peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 4);
        peer->EncodeDWORD(GcCharacterPhaseFailurePacket::CreateType::kEinval);
        return;
    }

    std::string queryString;
    CreatePlayerCreateQuery(*packet, queryString);

    SPDLOG_INFO("PlayerCreate accountid {0} name {1} level {2} gold {3}, st "
                "{4} ht {5} job {6}",
                packet->account_id, packet->player_table.name,
                packet->player_table.level, packet->player_table.gold,
                packet->player_table.st, packet->player_table.ht,
                packet->player_table.job);

    std::unique_ptr<SQLMsg> pMsg2(
        CDBManager::instance().DirectQuery(queryString));

    SPDLOG_TRACE("Create_Player queryLen[{0}]", queryLen);

    if (pMsg2->Get()->uiAffectedRows <= 0) {
        peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 4);
        peer->EncodeDWORD(GcCharacterPhaseFailurePacket::CreateType::kAlready);
        SPDLOG_INFO("ALREADY EXIST3 query: {0} AffectedRows {1}", queryStr,
                    pMsg2->Get()->uiAffectedRows);
        return;
    }

    TPacketDGCreateSuccess pack = {};
    pack.slot = packet->slot;
    pack.player.id = pMsg2->Get()->uiInsertID;
    strlcpy(pack.player.name, packet->player_table.name,
            sizeof(pack.player.name));
    pack.player.byJob = packet->player_table.job;
    pack.player.empire = packet->player_table.empire;
    pack.player.byLevel = 1;
    pack.player.dwPlayMinutes = 0;
    pack.player.byST = (uint8_t)packet->player_table.st;
    pack.player.byHT = (uint8_t)packet->player_table.ht;
    pack.player.byDX = (uint8_t)packet->player_table.dx;
    pack.player.byIQ = (uint8_t)packet->player_table.iq;
    pack.player.parts[PART_MAIN].vnum = packet->player_table.part_base;
    pack.player.parts[PART_HAIR] = packet->player_table.parts[PART_HAIR];
    pack.player.parts[PART_ACCE] = packet->player_table.parts[PART_ACCE];
    pack.player.mapIndex = packet->player_table.lMapIndex;
    pack.player.x = packet->player_table.x;
    pack.player.y = packet->player_table.y;
    peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_SUCCESS, dwHandle,
                       sizeof(TPacketDGCreateSuccess));
    peer->Encode(&pack, sizeof(TPacketDGCreateSuccess));

    s_createTimeByAccountID[packet->account_id] = time(nullptr);
}

/*
 * PLAYER DELETE
 */
void CClientManager::__QUERY_PLAYER_DELETE(CPeer* peer, uint32_t handle,
                                           const GdPlayerDeletePacket& p)
{
    std::unique_ptr<SQLMsg> msg(
        CDBManager::instance().DirectQuery(fmt::format("UPDATE player SET "
                                                       "deleted = 1, "
                                                       "deleted_at=NOW() WHERE "
                                                       "id = {}",
                                                       p.pid)));
    if (msg->Get()->uiAffectedRows == 0 ||
        msg->Get()->uiAffectedRows == (uint32_t)-1) {
        SPDLOG_INFO("PLAYER_DELETE FAILED {0} CANNOT INSERT TO player_deleted",
                    p.pid);
        peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, handle, 4);
        peer->EncodeDWORD(p.pid);
        return;
    }

    auto cachedPlayer = m_playerCache.Get(p.pid);
    if (cachedPlayer) {
        assert(!cachedPlayer->IsLocked() && "You can't be playing anymore.");

        // Grab a copy so we don't refer to the soon-to-be-deleted player cache.
        auto cachedItems = cachedPlayer->GetItems();

        // This unlocks all items
        m_playerCache.Remove(p.pid);

        // Remove all items as well (they end up being deleted as well)
        for (const auto& item : cachedItems) {
            assert(!item->IsLocked() && "You can't be playing anymore.");

            m_itemCache.Remove(item->GetData().id);
        }
    }

    CDBManager::instance().AsyncQuery(
        fmt::format("DELETE FROM player_skill WHERE pid={}", p.pid));
    CDBManager::instance().AsyncQuery(
        fmt::format("DELETE FROM player_quickslot WHERE pid={}", p.pid));
    CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM item WHERE "
                                                  "owner_id={} AND (`window` < "
                                                  "{})",
                                                  p.pid, SAFEBOX));
    CDBManager::instance().AsyncQuery(
        fmt::format("DELETE FROM quest WHERE dwPID={}", p.pid));
    CDBManager::instance().AsyncQuery(
        fmt::format("DELETE FROM affect WHERE dwPID={}", p.pid));
    CDBManager::instance().AsyncQuery(
        fmt::format("DELETE FROM guild_member WHERE pid={}", p.pid));
    CDBManager::instance().AsyncQuery(
        fmt::format("DELETE FROM myshop_pricelist WHERE owner_id={}", p.pid));
    CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM messenger_list "
                                                  "WHERE account='{}' OR "
                                                  "companion='{}'",
                                                  p.name, p.name));
    CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM messenger_block "
                                                  "WHERE pid={} OR "
                                                  "other_pid={}",
                                                  p.pid, p.pid));

    peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_SUCCESS, handle, 4);
    peer->EncodeDWORD(p.pid);
}

void CClientManager::QUERY_ITEM_FLUSH(CPeer* pkPeer, const char* c_pData)
{
    uint32_t dwID = *(uint32_t*)c_pData;

    auto cachedItem = m_itemCache.Get(dwID);
    if (cachedItem && cachedItem->IsLocked()) {
        spdlog::error("Trying to flush+forget locked item {0}", dwID);
        return;
    }

    m_itemCache.Remove(dwID);
}

void CClientManager::HandleQueryAddSwitchbotData(CPeer* peer,
                                                 TPacketGDAddSwitchbotData* p)
{

    SPDLOG_TRACE("HEADER_GD_ADD_SWITCHBOT_DATA PID {0} SLOT {1}", p->pid,
                 p->slot);

    CDBManager::instance().AsyncQuery(fmt::format(
        "REPLACE INTO player_switchbot_data (player_id, slot, status, "
        "attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,"
        "attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,"
        "attrtype6,attrvalue6,attrtype0_alt1,attrvalue0_alt1,attrtype1_alt1,"
        "attrvalue1_alt1,attrtype2_alt1,attrvalue2_alt1,attrtype3_alt1,"
        "attrvalue3_alt1,attrtype4_alt1,attrvalue4_alt1,attrtype5_alt1,"
        "attrvalue5_alt1,attrtype6_alt1,attrvalue6_alt1,attrtype0_alt2,"
        "attrvalue0_alt2,attrtype1_alt2,attrvalue1_alt2,attrtype2_alt2,"
        "attrvalue2_alt2,attrtype3_alt2,attrvalue3_alt2,attrtype4_alt2,"
        "attrvalue4_alt2,attrtype5_alt2,attrvalue5_alt2,attrtype6_alt2,"
        "attrvalue6_alt2) "
        "VALUES({}, {}, {}, {}, "
        "{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{"
        "},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{})",
        p->pid, p->elem.slot, p->elem.status, p->elem.attr[0][0].bType,
        p->elem.attr[0][0].lValue, p->elem.attr[0][1].bType,
        p->elem.attr[0][1].lValue, p->elem.attr[0][2].bType,
        p->elem.attr[0][2].lValue, p->elem.attr[0][3].bType,
        p->elem.attr[0][3].lValue, p->elem.attr[0][4].bType,
        p->elem.attr[0][4].lValue, p->elem.attr[0][5].bType,
        p->elem.attr[0][5].lValue, p->elem.attr[0][6].bType,
        p->elem.attr[0][6].lValue, p->elem.attr[1][0].bType,
        p->elem.attr[1][0].lValue, p->elem.attr[1][1].bType,
        p->elem.attr[1][1].lValue, p->elem.attr[1][2].bType,
        p->elem.attr[1][2].lValue, p->elem.attr[1][3].bType,
        p->elem.attr[1][3].lValue, p->elem.attr[1][4].bType,
        p->elem.attr[1][4].lValue, p->elem.attr[1][5].bType,
        p->elem.attr[1][5].lValue, p->elem.attr[1][6].bType,
        p->elem.attr[1][6].lValue, p->elem.attr[2][0].bType,
        p->elem.attr[2][0].lValue, p->elem.attr[2][1].bType,
        p->elem.attr[2][1].lValue, p->elem.attr[2][2].bType,
        p->elem.attr[2][2].lValue, p->elem.attr[2][3].bType,
        p->elem.attr[2][3].lValue, p->elem.attr[2][4].bType,
        p->elem.attr[2][4].lValue, p->elem.attr[2][5].bType,
        p->elem.attr[2][5].lValue, p->elem.attr[2][6].bType,
        p->elem.attr[2][6].lValue));
}

void CClientManager::HandleQueryRemoveSwitchbotData(
    CPeer* peer, TPacketGDRemoveSwitchbotData* p)
{
    SPDLOG_TRACE("HEADER_GD_REMOVE_AFFECT PID {0} SLOT {1}", p->pid, p->slot);

    CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM "
                                                  "player_switchbot_data WHERE "
                                                  "player_id={} AND slot={}",
                                                  p->pid, p->slot));
}

void CClientManager::QUERY_ADD_AFFECT(CPeer* peer, TPacketGDAddAffect* p)
{

    SPDLOG_TRACE("HEADER_GD_ADD_AFFECT PID {0} TYPE {1}", p->dwPID,
                 p->elem.type);
    auto query = fmt::format("REPLACE INTO affect (dwPID, bType, bSubType, bApplyOn, "
                             "lApplyValue, lDuration, lSPCost, color) "
                             "VALUES({}, {}, {}, {}, {}, {}, {}, {})",
                             p->dwPID, p->elem.type, p->elem.subType, p->elem.pointType,
                             p->elem.pointValue, p->elem.duration,
                             p->elem.spCost, p->elem.color);
    CDBManager::instance().AsyncQuery(query);
}

void CClientManager::QUERY_REMOVE_AFFECT(CPeer* peer, TPacketGDRemoveAffect* p)
{
    SPDLOG_TRACE("HEADER_GD_REMOVE_AFFECT PID {0} TYPE {1} APPLY {2}", p->dwPID,
                 p->dwType, p->bApplyOn);

    CDBManager::instance().AsyncQuery(fmt::format("DELETE FROM affect WHERE "
                                                  "dwPID={} AND bType={} AND "
                                                  "bApplyOn={}",
                                                  p->dwPID, p->dwType,
                                                  p->bApplyOn));
}

void CClientManager::QUERY_HIGHSCORE_REGISTER(CPeer* peer,
                                              TPacketGDHighscore* data)
{
    SPDLOG_TRACE("HEADER_GD_HIGHSCORE_REGISTER: PID {0}", data->dwPID);

    ClientHandleInfo* pi = new ClientHandleInfo(0);
    strlcpy(pi->login, data->szBoard, sizeof(pi->login));
    pi->account_id = (uint32_t)data->lValue;
    pi->player_id = data->dwPID;
    pi->account_index = (data->cDir > 0);

    CDBManager::instance().ReturnQuery(fmt::format("SELECT iValue FROM "
                                                   "highscore WHERE "
                                                   "szBoard='{}' AND dwPID = "
                                                   "{}",
                                                   data->szBoard, data->dwPID),
                                       QID_HIGHSCORE_REGISTER,
                                       peer->GetHandle(), pi);
}

void CClientManager::RESULT_HIGHSCORE_REGISTER(CPeer* pkPeer, SQLMsg* msg)
{
    CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
    ClientHandleInfo* pi = (ClientHandleInfo*)qi->pvData;
    // uint32_t dwHandle = pi->dwHandle;

    char szBoard[21];
    strlcpy(szBoard, pi->login, sizeof(szBoard));
    int value = (int)pi->account_id;

    SQLResult* res = msg->Get();

    if (res->uiNumRows == 0) {
        // 새로운 하이스코어를 삽입
        CDBManager::instance().AsyncQuery(fmt::format("INSERT INTO highscore "
                                                      "VALUES('{}', {}, {})",
                                                      szBoard, pi->player_id,
                                                      value));
    } else {
        if (!res->pSQLResult) {
            delete pi;
            return;
        }

        MYSQL_ROW row = mysql_fetch_row(res->pSQLResult);
        if (row && row[0]) {
            int current_value = 0;
            str_to_number(current_value, row[0]);
            if ((pi->account_index && current_value >= value) ||
                (!pi->account_index && current_value <= value)) {
                value = current_value;
            }

            CDBManager::instance().AsyncQuery(
                fmt::format("REPLACE INTO highscore VALUES('{}', {}, {})",
                            szBoard, pi->player_id, value));
        } else {

            CDBManager::instance().AsyncQuery(
                fmt::format("INSERT INTO highscore VALUES('{}', {}, {})",
                            szBoard, pi->player_id, value));
        }
    }
    // TODO: 이곳에서 하이스코어가 업데이트 되었는지 체크하여 공지를 뿌려야한다.
    delete pi;
}

void CClientManager::SendDungeonInfoCacheSet(CPeer* peer,
                                             PlayerLoader* loader)
{
    // Try to load from the cache first
    TDungeonInfoCacheSet* pHSet = FindDungeonInfoCacheSet(loader->GetPid());
    if (pHSet && pHSet->size() > 0) {
        std::vector<TPlayerDungeonInfo> info;
        info.resize(pHSet->size());

        uint32_t dwCount = 0;

        auto it = pHSet->begin();
        while (it != pHSet->end()) {
            CDungeonInfoCache* c = *it++;
            TPlayerDungeonInfo* p = c->Get();
            info.push_back(*p);
        }
        loader->SetDungeonInfos(info);

    } else {
        CDBManager::instance().ReturnQuery(
            fmt::format("SELECT pid, dungeon_id, UNIX_TIMESTAMP(cooldown_end), "
                 "complete_count, fastest_time, highest_damage "
                 "FROM dungeon_info WHERE pid = {};", loader->GetPid()), QID_DUNGEON_INFO, peer->GetHandle(),
                                       loader);
    }
}

void CClientManager::RESULT_DUNGEON_INFO_LOAD(CPeer* peer, MYSQL_RES* res,
                                              PlayerLoader* loader)
{
    std::size_t count = mysql_num_rows(res);

    std::vector<TPlayerDungeonInfo> dungeonInfos;
    dungeonInfos.resize(count);

    // "SELECT pid, dungeon_id, UNIX_TIMESTAMP(cooldown_end), complete_count,
    // fastest_time, highest_damage "

    for (size_t i = 0; i < count; ++i) {
        MYSQL_ROW row = mysql_fetch_row(res);
        auto& h = dungeonInfos[i];

        int cur = 0;

        str_to_number(h.dwPlayerID, row[cur++]);
        str_to_number(h.bDungeonID, row[cur++]);
        str_to_number(h.dwCooldownEnd, row[cur++]);
        str_to_number(h.wCompleteCount, row[cur++]);
        str_to_number(h.wFastestTime, row[cur++]);
        str_to_number(h.dwHightestDamage, row[cur++]);
    }

    loader->SetDungeonInfos(dungeonInfos);
}

void CClientManager::QUERY_DUNGEON_INFO_SAVE(CPeer* pkPeer, const char* c_pData)
{
    TPlayerDungeonInfo* p = (TPlayerDungeonInfo*)c_pData;
    PutDungeonInfoCache(p, false);
}

void CClientManager::PutDungeonInfoCache(TPlayerDungeonInfo* p, bool bSkipQuery)
{
    TDungeonInfoCacheSet* pSet = GetDungeonInfoCacheSet(p->dwPlayerID);
    CDungeonInfoCache* c = GetDungeonInfoCache(p->dwPlayerID, p->bDungeonID);

    if (!c) {
        c = new CDungeonInfoCache;
        pSet->insert(c);
    }

    c->Put(p, bSkipQuery);
}

CClientManager::TDungeonInfoCacheSet*
CClientManager::FindDungeonInfoCacheSet(uint32_t dwPlayerID)
{
    TDungeonInfoCacheSetPtrMap::iterator it =
        m_map_DungeonInfoSetPtr.find(dwPlayerID);
    if (it == m_map_DungeonInfoSetPtr.end())
        return NULL;

    return it->second;
}

CClientManager::TDungeonInfoCacheSet*
CClientManager::GetDungeonInfoCacheSet(uint32_t dwPlayerID)
{
    TDungeonInfoCacheSet* pSet = FindDungeonInfoCacheSet(dwPlayerID);
    if (!pSet) {
        pSet = new TDungeonInfoCacheSet;
        m_map_DungeonInfoSetPtr.insert(
            TDungeonInfoCacheSetPtrMap::value_type(dwPlayerID, pSet));
    }
    return pSet;
}

CDungeonInfoCache* CClientManager::GetDungeonInfoCache(uint32_t dwPlayerID,
                                                       uint8_t bDungeonID)
{
    TDungeonInfoCacheSetPtrMap::iterator it =
        m_map_DungeonInfoSetPtr.find(dwPlayerID);
    if (it == m_map_DungeonInfoSetPtr.end())
        return NULL;

    TDungeonInfoCacheSet* pSet = it->second;
    TDungeonInfoCacheSet::iterator itSet = pSet->begin();
    while (itSet != pSet->end()) {
        CDungeonInfoCache* pCache = *itSet++;
        if (pCache) {
            if (pCache->Get()->dwPlayerID == dwPlayerID &&
                pCache->Get()->bDungeonID == bDungeonID)
                return pCache;
        }
    }

    return NULL;
}

void CClientManager::UpdateDungeonInfoCache()
{
    for (const auto it : m_map_DungeonInfoSetPtr) {
        TDungeonInfoCacheSet* pSet = it.second;
        if (pSet) {
            TDungeonInfoCacheSet::iterator itSet = pSet->begin();
            while (itSet != pSet->end()) {
                CDungeonInfoCache* pCache = *(itSet++);
                if (pCache->CheckFlushTimeout()) {
                    // if (g_test_server)

                    pCache->Flush();
                }
            }
        }
    }
}

bool CClientManager::InitializeDungeonRanking(bool bIsReload)
{
    if (bIsReload) {
        m_map_DungeonRankMap.clear();
    }

    char query[256];
    std::string strOrderBy[DUNGEON_RANK_TYPE_MAX][2] = {
        {"d.complete_count", "complete_count DESC"},
        {"d.fastest_time", "fastest_time ASC"},
        {"d.highest_damage", "highest_damage DESC"}};

    uint8_t bPos = 0;

    for (int i = DUNGEON_ID_NONE + 1; i < DUNGEON_ID_MAX; i++) {
        for (int j = 0; j < DUNGEON_RANK_TYPE_MAX; j++) {
            // Reuse the query from the start every time
            snprintf(query, sizeof(query),
                     "SELECT p.name, %s "
                     "FROM dungeon_info d INNER JOIN player p ON p.id = d.pid "
                     "WHERE d.dungeon_id = %d AND %s > 0 "
                     "ORDER BY %s;",
                     strOrderBy[j][0].c_str(), i, strOrderBy[j][0].c_str(),
                     strOrderBy[j][1].c_str());

            std::unique_ptr<SQLMsg> pkMsg(
                CDBManager::instance().DirectQuery(query));
            SQLResult* pRes = pkMsg->Get();

            // If no data for this id/type just skip it
            if (!pRes->uiNumRows)
                continue;

            auto pSet = std::make_unique<TDungeonRankSet>();
            pSet->bDungeonID = i;
            pSet->bType = j;
            const auto& [it, b] =m_map_DungeonRankMap.emplace(
                std::make_pair(i, j), std::move(pSet));

            // Reset the pos after every set
            bPos = 0;

            MYSQL_ROW data;
            while ((data = mysql_fetch_row(pRes->pSQLResult)) &&
                   bPos < DUNGEON_RANKING_MAX_NUM) {
                strlcpy(it->second->rankItems[bPos].szName, data[0],
                        sizeof(it->second->rankItems[bPos].szName));
                str_to_number(it->second->rankItems[bPos].dwValue, data[1]);

                bPos++;
            }
        }
    }

    if (bIsReload) {
        for (const auto peer : m_peerList) {
            if (peer->GetChannel()) {
                peer->EncodeHeader(HEADER_DG_DUNGEON_RANKING_LOAD, 0,
                                   sizeof(uint16_t) + sizeof(uint16_t) +
                                       (sizeof(TDungeonRankSet) *
                                        m_map_DungeonRankMap.size()));
                peer->EncodeWORD(sizeof(TDungeonRankSet));
                peer->EncodeWORD(m_map_DungeonRankMap.size());
                for (const auto& it : m_map_DungeonRankMap) {
                    peer->Encode(it.second.get(), sizeof(TDungeonRankSet));
                }
            }
        }
    }

    return true;
}
