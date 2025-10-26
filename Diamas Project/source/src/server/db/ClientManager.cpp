#include <game/BuildingConstants.hpp>
#include <game/BuildingTypes.hpp>
#include <game/Types.hpp>

#include "ClientManager.h"

#include <game/AffectConstants.hpp>

#include "Config.h"
#include "DBManager.h"
#include "GuildManager.h"
#include "ItemAwardManager.h"
#include "ItemIDRangeManager.h"
#include "Main.h"
#include "Marriage.h"
#include "MoneyLog.h"
#include "PrivManager.h"
#include "QID.h"

#include "Cache.h"

#include "Persistence.hpp"
#include <game/grid.h>
#include <sstream>

#include "fmt/format.h"
#include <game/PageGrid.hpp>
#include <storm/Util.hpp>

extern int g_test_server;

int g_item_count = 0;
int g_query_count[2];

CClientManager::CClientManager(asio::io_service& ioService,
                               const Config& config)
    : m_ioService(ioService)
      , m_config(config)
      , m_acceptor(m_ioService)
      , m_nextPeerHandle(0)
      , m_pkAuthPeer(NULL)
      , m_dbProcessTimer(m_ioService)
      , m_updateTimer(m_ioService)
      , m_hourlyTimer(m_ioService)
      , m_dungeonRankingTimer(m_ioService), m_iPlayerDeleteLevelLimit(0)
      , m_iPlayerDeleteLevelLimitLower(0)
      , m_bChinaEventServer(false)
      , m_iCacheFlushCount(0)
      , m_iCacheFlushCountLimit(200)
      , m_dwNextOfflineShopItemID(0)
{
    // We use 1 for |start|, because we need start > max
    m_itemRange.min = 0;
    m_itemRange.max = 0;
    m_itemRange.start = 1;
}

CClientManager::~CClientManager()
{
    STORM_ASSERT(m_peerList.empty(), "Leaked peers");
}

bool CClientManager::Initialize()
{
    if (!InitializeTables()) {
        spdlog::error("Table Initialize FAILED");
        return false;
    }

    CGuildManager::instance().BootReserveWar();

    SPDLOG_INFO("PLAYER_DELETE_LEVEL_LIMIT set to {0}",
                m_config.playerDeleteLevelMax);
    SPDLOG_INFO("PLAYER_DELETE_LEVEL_LIMIT_LOWER set to {0}",
                m_config.playerDeleteLevelMin);

    m_bChinaEventServer = m_config.chinaEventServer;
    SPDLOG_INFO("CHINA_EVENT_SERVER {0}", IsChinaEventServer());

    LoadEventFlag();
    LoadHwidFlag();

    boost::system::error_code ec;
    m_acceptor.Bind(m_config.bindAddr, m_config.bindPort, ec,
                    [this](asio::ip::tcp::socket socket) {
                        SPDLOG_INFO("Accepted connection on {0}", socket);
                        AddPeer(std::move(socket));
                    });

    StartDbTimer();
    StartUpdateTimer();
    StartHourlyTimer();
    //StartDungeonRankingTimer();
    return true;
}

void CClientManager::Quit()
{
    if (!m_peerList.empty())
        SPDLOG_WARN("Game servers should be closed before the DB cache");

    CClientManager::TPeerList peerListCopy = m_peerList;
    for (const auto& p : peerListCopy) {
        if (p)
            p->Disconnect();
    }

    m_acceptor.Close();
    m_dbProcessTimer.cancel();
    m_updateTimer.cancel();
    m_hourlyTimer.cancel();

    //
    // 메인루프 종료처리
    //

    m_playerCache.TryFlushAll();
    m_playerCache.RemoveAll();

    m_itemCache.TryFlushAll();
    m_itemCache.RemoveAll();

#ifdef __OFFLINE_SHOP__
    this->FlushOfflineShopCache(true);
    this->FlushOfflineShopItemCache(true);
#endif
    for (const auto& it : m_map_DungeonInfoSetPtr) {
        TDungeonInfoCacheSet* pSet = it.second;
        if (pSet) {
            TDungeonInfoCacheSet::iterator itSet = pSet->begin();
            while (itSet != pSet->end()) {
                CDungeonInfoCache* pCache = *itSet++;
                pCache->Flush();
                delete pCache;
            }

            pSet->clear();
            delete pSet;
        }
    }
    m_map_DungeonInfoSetPtr.clear();
}

void CClientManager::QUERY_BOOT(CPeer* peer, TPacketGDBoot* p)
{
    const uint8_t bPacketVersion =
        6; // BOOT ÆÐÅ¶ÀÌ ¹Ù²ð¶§¸¶´Ù ¹øÈ£¸¦ ¿Ã¸®µµ·Ï ÇÑ´Ù.

    std::vector<tAdminInfo> vAdmin;
#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
    std::vector<TEventInfo> vEvent;
#endif
    __GetAdminInfo(vAdmin);

    uint32_t adwAdminConfig[GM_MAX_NUM];
    __GetAdminConfig(adwAdminConfig);

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
    __GetEventInfo(vEvent);
#endif
    SPDLOG_INFO("QUERY_BOOT : AdminInfo (Request ServerIp {0}) ", p->szIP);

    uint32_t dwPacketSize =
        sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t) +
        sizeof(uint16_t) + sizeof(TBanwordTable) * m_vec_banwordTable.size() +
        sizeof(uint16_t) + sizeof(uint16_t) +
        sizeof(TLand) * m_vec_kLandTable.size() + sizeof(uint16_t) +
        sizeof(uint16_t) + sizeof(TObject) * m_map_pkObjectTable.size() +
        sizeof(uint16_t) + sizeof(uint16_t) +
        sizeof(TDungeonRankSet) * m_map_DungeonRankMap.size() +

        sizeof(time_t) + sizeof(uint16_t) + sizeof(uint16_t) +
        sizeof(ItemIdRange) * 2 +
        // ADMIN_MANAGER
        sizeof(uint16_t) + sizeof(uint16_t) +
        sizeof(tAdminInfo) * vAdmin.size() + sizeof(adwAdminConfig) +
        // END_ADMIN_MANAGER

        sizeof(uint16_t);

#ifdef __OFFLINE_SHOP__
    dwPacketSize +=
        sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) +
        sizeof(TPacketDGOfflineShopCreate) * this->m_mapOfflineShops.size() +
        sizeof(uint32_t) * this->m_mapOfflineShops.size();
    for (TMapOfflineShopByOwnerPID::iterator it =
             this->m_mapOfflineShops.begin();
         it != this->m_mapOfflineShops.end(); ++it)
        dwPacketSize +=
            sizeof(TOfflineShopItemData) * it->second.mapItems.size();
#endif

    peer->EncodeHeader(HEADER_DG_BOOT, 0, dwPacketSize);
    peer->Encode(&dwPacketSize, sizeof(uint32_t));
    peer->Encode(&bPacketVersion, sizeof(uint8_t));

    SPDLOG_INFO("BOOT: PACKET: {0}", dwPacketSize);
    SPDLOG_INFO("BOOT: VERSION: {0}", bPacketVersion);

    SPDLOG_INFO("sizeof(TBanwordTable) = {0}", sizeof(TBanwordTable));
    SPDLOG_INFO("sizeof(TLand) = {0}", sizeof(TLand));
    SPDLOG_INFO("sizeof(TObject) = {0}", sizeof(TObject));
    // ADMIN_MANAGER
    SPDLOG_INFO("sizeof(tAdminInfo) * vAdmin.size() = {0} * {1} = {2}",
                sizeof(tAdminInfo), vAdmin.size(),
                sizeof(tAdminInfo) * vAdmin.size());
    // END_ADMIN_MANAGER
#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
    // EVENT_MANAGER
    SPDLOG_INFO("sizeof(TEventInfo) = {0} * {1} ",
                sizeof(TEventInfo) * vEvent.size());
    // END_EVENT_MANAGER
#endif

    peer->EncodeWORD(sizeof(TBanwordTable));
    peer->EncodeWORD(m_vec_banwordTable.size());
    if (m_vec_banwordTable.size())
        peer->Encode(&m_vec_banwordTable[0],
                     sizeof(TBanwordTable) * m_vec_banwordTable.size());

    peer->EncodeWORD(sizeof(TLand));
    peer->EncodeWORD(m_vec_kLandTable.size());
    if (m_vec_kLandTable.size())
        peer->Encode(&m_vec_kLandTable[0],
                     sizeof(TLand) * m_vec_kLandTable.size());

    peer->EncodeWORD(sizeof(TObject));
    peer->EncodeWORD(m_map_pkObjectTable.size());

    auto it = m_map_pkObjectTable.begin();

    while (it != m_map_pkObjectTable.end())
        peer->Encode((it++)->second.get(), sizeof(TObject));

    peer->EncodeWORD(sizeof(TDungeonRankSet));
    peer->EncodeWORD(m_map_DungeonRankMap.size());
    for (const auto& it3 : m_map_DungeonRankMap) {
        peer->Encode(it3.second.get(), sizeof(TDungeonRankSet));
    }

    // Auction Boot

    time_t now = time(nullptr);
    peer->Encode(&now, sizeof(time_t));

    ItemIdRange itemRange, itemRangeSpare;

    if (!CItemIDRangeManager::instance().GetRange(itemRange) ||
        !CItemIDRangeManager::instance().GetRange(itemRangeSpare)) {
        // TODO(tim): Handle that
        spdlog::error("Out of item ID ranges");
        return;
    }

    peer->EncodeWORD(sizeof(ItemIdRange));
    peer->EncodeWORD(1);
    peer->Encode(&itemRange, sizeof(ItemIdRange));
    peer->Encode(&itemRangeSpare, sizeof(ItemIdRange));

    peer->SetItemIDRange(itemRange);
    peer->SetSpareItemIDRange(itemRangeSpare);

    // ADMIN_MANAGER
    peer->EncodeWORD(sizeof(tAdminInfo));
    peer->EncodeWORD(vAdmin.size());

    for (size_t n = 0; n < vAdmin.size(); ++n) {
        peer->Encode(&vAdmin[n], sizeof(tAdminInfo));
        SPDLOG_INFO("Admin name {}", vAdmin[n].m_szName);
    }

    peer->Encode(&adwAdminConfig[0], sizeof(adwAdminConfig));

#ifdef __OFFLINE_SHOP__
    peer->EncodeWORD(sizeof(TPacketDGOfflineShopCreate));
    peer->EncodeWORD(sizeof(TOfflineShopItemData));
    peer->EncodeDWORD(this->m_mapOfflineShops.size());
    for (TMapOfflineShopByOwnerPID::iterator it =
             this->m_mapOfflineShops.begin();
         it != this->m_mapOfflineShops.end(); ++it) {
        TPacketDGOfflineShopCreate kPacket;
        memcpy(&kPacket.kShopData, &it->second, sizeof(kPacket.kShopData));

        peer->Encode(&kPacket, sizeof(kPacket));

        peer->EncodeDWORD(it->second.mapItems.size());
        for (TMapItemByPosition::iterator itItems = it->second.mapItems.begin();
             itItems != it->second.mapItems.end(); ++itItems)
            peer->Encode(&(itItems->second.data), sizeof(itItems->second.data));
    }
#endif

    peer->EncodeWORD(0xffff);
}

void CClientManager::SendPartyOnSetup(CPeer* pkPeer)
{
    for (const auto& party : m_map_pkPartyMap) {
        SPDLOG_INFO("PARTY SendPartyOnSetup Party [{0}]", party.first);
        pkPeer->EncodeHeader(HEADER_DG_PARTY_CREATE, 0,
                             sizeof(TPacketPartyCreate));
        pkPeer->Encode(&party.first, sizeof(uint32_t));

        for (auto it_member = party.second.begin();
             it_member != party.second.end(); ++it_member) {
            SPDLOG_INFO("PARTY SendPartyOnSetup Party [{0}] Member [{1}]",
                        party.first, it_member->first);
            pkPeer->EncodeHeader(HEADER_DG_PARTY_ADD, 0,
                                 sizeof(TPacketPartyAdd));
            pkPeer->Encode(&party.first, sizeof(uint32_t));
            pkPeer->Encode(&it_member->first, sizeof(uint32_t));
            pkPeer->Encode(&it_member->second.bRole, sizeof(uint8_t));

            pkPeer->EncodeHeader(HEADER_DG_PARTY_SET_MEMBER_LEVEL, 0,
                                 sizeof(TPacketPartySetMemberLevel));
            pkPeer->Encode(&party.first, sizeof(uint32_t));
            pkPeer->Encode(&it_member->first, sizeof(uint32_t));
            pkPeer->Encode(&it_member->second.bLevel, sizeof(uint8_t));
        }
    }
}

void CClientManager::QUERY_QUEST_SAVE(CPeer* pkPeer, TQuestTable* pTable,
                                      uint32_t dwLen)
{
    if (0 != (dwLen % sizeof(TQuestTable))) {
        spdlog::error("Invalid packet size {0}, sizeof(TQuestTable) == {1}",
                      dwLen, sizeof(TQuestTable));
        return;
    }

    int iSize = dwLen / sizeof(TQuestTable);

    std::string query;

    for (int i = 0; i < iSize; ++i, ++pTable) {
        if (pTable->lValue == 0) {
            query = fmt::format("DELETE FROM quest WHERE dwPID={} AND "
                                "szName='{}' AND szState='{}'",
                                pTable->dwPID, pTable->szName, pTable->szState);
        } else {
            query = fmt::format("REPLACE INTO quest (dwPID, szName, szState, "
                                "lValue) VALUES({}, '{}', '{}', {})",
                                pTable->dwPID, pTable->szName, pTable->szState,
                                pTable->lValue);
        }

        CDBManager::instance().ReturnQuery(query, QID_QUEST_SAVE,
                                           pkPeer->GetHandle(), NULL);
    }
}

void CClientManager::QUERY_SAFEBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle,
                                        TSafeboxLoadPacket* packet, bool bMall)
{
    ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
    strlcpy(pi->safebox_password, packet->szPassword,
            sizeof(pi->safebox_password));
    pi->account_id = packet->dwID;
    pi->account_index = 0;
    pi->ip[0] = bMall ? 1 : 0;
    strlcpy(pi->login, packet->szLogin, sizeof(pi->login));

    auto query = fmt::format("SELECT account_id, size, password FROM safebox "
                             "WHERE account_id={}",
                             packet->dwID);

    spdlog::trace("HEADER_GD_SAFEBOX_LOAD (handle: {0} account.id {1} is_mall "
                  "{2}",
                  dwHandle, packet->dwID, bMall ? 1 : 0);

    CDBManager::instance().ReturnQuery(query, QID_SAFEBOX_LOAD,
                                       pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_LOAD(CPeer* pkPeer, SQLMsg* msg)
{
    CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
    ClientHandleInfo* pi = (ClientHandleInfo*)qi->pvData;
    uint32_t dwHandle = pi->dwHandle;

    // 여기에서 사용하는 account_index는 쿼리 순서를 말한다.
    // 첫번째 패스워드 알아내기 위해 하는 쿼리가 0
    // 두번째 실제 데이터를 얻어놓는 쿼리가 1

    if (pi->account_index == 0) {
        char szSafeboxPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
        strlcpy(szSafeboxPassword, pi->safebox_password,
                sizeof(szSafeboxPassword));

        TSafeboxTable* pSafebox = new TSafeboxTable;
        memset(pSafebox, 0, sizeof(TSafeboxTable));

        SQLResult* res = msg->Get();

        if (res->uiNumRows == 0) {
            if (strcmp("000000", szSafeboxPassword)) {
                pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle,
                                     0);
                delete pi;
                return;
            }
        } else {
            MYSQL_ROW row = mysql_fetch_row(res->pSQLResult);

            // 비밀번호가 틀리면..
            if (((!row[2] || !*row[2]) &&
                 strcmp("000000", szSafeboxPassword)) ||
                ((row[2] && *row[2]) && strcmp(row[2], szSafeboxPassword))) {
                pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle,
                                     0);
                delete pi;
                return;
            }

            if (!row[0])
                pSafebox->dwID = 0;
            else
                str_to_number(pSafebox->dwID, row[0]);

            if (!row[1])
                pSafebox->bSize = 0;
            else
                str_to_number(pSafebox->bSize, row[1]);
            /*
            if (!row[3])
            pSafebox->dwGold = 0;
            else
            pSafebox->dwGold = atoi(row[3]);
            */
            if (pi->ip[0] == 1) {
                pSafebox->bSize = 1;
                SPDLOG_INFO("MALL id[{0}] size[{1}]", pSafebox->dwID,
                            pSafebox->bSize);
            } else
                SPDLOG_INFO("SAFEBOX id[{0}] size[{1}]", pSafebox->dwID,
                            pSafebox->bSize);
        }

        if (0 == pSafebox->dwID)
            pSafebox->dwID = pi->account_id;

        pi->pSafebox = pSafebox;

        const auto query =
            fmt::format("SELECT id,`window`+0,pos,count,vnum,"
                        "trans_vnum, "
                        "seal_date,"
                        "is_gm_owner, is_blocked, price, "
                        "socket0,socket1,socket2,socket3,socket4,socket5,"
                        "attrtype0,attrvalue0,attrtype1,attrvalue1,"
                        "attrtype2,attrvalue2,attrtype3,attrvalue3,"
                        "attrtype4,attrvalue4,attrtype5,attrvalue5,"
                        "attrtype6,attrvalue6 "
                        "FROM item WHERE owner_id={} AND `window`='{}'",
                        pi->account_id, pi->ip[0] == 0 ? "SAFEBOX" : "MALL");

        pi->account_index = 1;

        CDBManager::instance().ReturnQuery(query, QID_SAFEBOX_LOAD,
                                           pkPeer->GetHandle(), pi);
        return;
    }
    if (!pi->pSafebox) {
        spdlog::error("null safebox pointer!");
        delete pi;
        return;
    }

    // 쿼리에 에러가 있었으므로 응답할 경우 창고가 비어있는 것 처럼
    // 보이기 때문에 창고가 아얘 안열리는게 나음
    if (!msg->Get()->pSQLResult) {
        spdlog::error("null safebox result");
        delete pi;
        return;
    }

    static std::vector<TPlayerItem> items;
    CreateItemTableFromRes(msg->Get()->pSQLResult, pi->account_id, items);

    bool bEscape = false;
    PagedGrid<bool> grid(SAFEBOX_PAGE_WIDTH, SAFEBOX_PAGE_HEIGHT, 3);

    auto pSet = ItemAwardManager::instance().GetByLogin(pi->login);
    if (pSet) {
        for (auto& r : items) {
            const auto it = m_map_itemTableByVnum.find(r.data.vnum);
            if (it == m_map_itemTableByVnum.end()) {
                spdlog::error("invalid item vnum {0} in safebox: login {1}",
                              r.data.vnum, pi->login);
                bEscape = true;
                break;
            }

            // so we don't crash loading invalid positions
            if (!grid.IsEmpty(r.pos, it->second.bSize))
                spdlog::error("Invalid item position {0} for item {1} owner "
                              "{2}",
                              r.pos, r.id, r.owner);
            else
                grid.PutPos(true, r.pos, it->second.bSize);
        }
    }

    if (pSet && !bEscape) {
        std::vector<std::pair<uint32_t, uint32_t>> vec_dwFinishedAwardID;

        auto it = pSet->begin();
        while (it != pSet->end()) {
            auto pItemAward = *(it++);
            if (pItemAward->bTaken)
                continue;

            if (pi->ip[0] == 0 && pItemAward->bMall)
                continue;

            if (pi->ip[0] == 1 && !pItemAward->bMall)
                continue;

            auto it = m_map_itemTableByVnum.find(pItemAward->data.vnum);
            if (it == m_map_itemTableByVnum.end()) {
                spdlog::error("invalid item vnum {0} in item_award: login {1}",
                              pItemAward->data.vnum, pi->login);
                continue;
            }

            auto pos = grid.FindBlank(it->second.bSize);
            if (!pos)
                break;

            TPlayerItem item;
            memset(&item, 0, sizeof(TPlayerItem));

            const uint32_t id = AllocateItemId();
            if (0 == id) {
                spdlog::error("UNIQUE ID OVERFLOW!!");
                break;
            }

            const auto safeboxQuery = fmt::format(
                "INSERT INTO item (id, owner_id, window, pos, vnum, "
                "trans_vnum, count, socket0, socket1, socket2, "
                "attrtype0, attrvalue0, "
                "attrtype1, attrvalue1, "
                "attrtype2, attrvalue2, "
                "attrtype3, attrvalue3, "
                "attrtype4, attrvalue4, "
                "attrtype5, attrvalue5, "
                "attrtype6, attrvalue6) "
                "VALUES({}, {}, '{}', {}, {}, {}, {}, {}, {}, {},"
                "{}, {}, "
                "{}, {}, "
                "{}, {}, "
                "{}, {}, "
                "{}, {}, "
                "{}, {}, "
                "{}, {}) ",
                id, pi->account_id, pi->ip[0] == 0 ? "SAFEBOX" : "MALL",
                pos.value(), pItemAward->data.vnum, pItemAward->data.transVnum,
                pItemAward->data.count, pItemAward->data.sockets[0],
                pItemAward->data.sockets[1], pItemAward->data.sockets[2],
                pItemAward->data.attrs[0].bType,
                pItemAward->data.attrs[0].sValue,
                pItemAward->data.attrs[1].bType,
                pItemAward->data.attrs[1].sValue,
                pItemAward->data.attrs[2].bType,
                pItemAward->data.attrs[2].sValue,
                pItemAward->data.attrs[3].bType,
                pItemAward->data.attrs[3].sValue,
                pItemAward->data.attrs[4].bType,
                pItemAward->data.attrs[4].sValue,
                pItemAward->data.attrs[5].bType,
                pItemAward->data.attrs[5].sValue,
                pItemAward->data.attrs[6].bType,
                pItemAward->data.attrs[6].sValue);

            std::unique_ptr<SQLMsg> pmsg(
                CDBManager::instance().DirectQuery(safeboxQuery));
            SQLResult* pRes = pmsg->Get();

            SPDLOG_TRACE("SAFEBOX Query: {0}", safeboxQuery);

            if (pRes->uiAffectedRows == 0 || pRes->uiInsertID == 0 ||
                pRes->uiAffectedRows == (uint32_t)-1)
                break;

            item.id = pmsg->Get()->uiInsertID;
            item.window = pi->ip[0] == 0 ? SAFEBOX : MALL;
            item.pos = pos.value();
            item.data = pItemAward->data;
            item.is_gm_owner = false;
            item.is_blocked = false;
            items.push_back(item);

            vec_dwFinishedAwardID.emplace_back(pItemAward->dwID, item.id);
            grid.PutPos(true, pos.value(), it->second.bSize);
        }

        for (auto& i : vec_dwFinishedAwardID)
            ItemAwardManager::instance().Taken(i.first, i.second);
    }

    pi->pSafebox->wItemCount = items.size();

    pkPeer->EncodeHeader(
        pi->ip[0] == 0 ? HEADER_DG_SAFEBOX_LOAD : HEADER_DG_MALL_LOAD, dwHandle,
        sizeof(TSafeboxTable) + sizeof(TPlayerItem) * items.size());

    pkPeer->Encode(pi->pSafebox, sizeof(TSafeboxTable));

    if (!items.empty())
        pkPeer->Encode(items.data(), sizeof(TPlayerItem) * items.size());

    delete pi;
}

void CClientManager::QUERY_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, uint32_t dwHandle,
                                               TSafeboxChangeSizePacket* p)
{
    ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
    pi->account_index = p->bSize;

    auto query = fmt::format("INSERT INTO safebox (account_id, size) "
                             "VALUES({}, {}) "
                             "ON DUPLICATE KEY UPDATE size=VALUES(size)",
                             p->dwID, p->bSize);

    CDBManager::instance().ReturnQuery(query, QID_SAFEBOX_CHANGE_SIZE,
                                       pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg)
{
    CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
    ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
    uint32_t dwHandle = p->dwHandle;
    uint8_t bSize = p->account_index;

    delete p;

    if (msg->Get()->uiNumRows > 0) {
        pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_SIZE, dwHandle,
                             sizeof(uint8_t));
        pkPeer->EncodeBYTE(bSize);
    }
}

void CClientManager::QUERY_SAFEBOX_CHANGE_PASSWORD(
    CPeer* pkPeer, uint32_t dwHandle, TSafeboxChangePasswordPacket* p)
{
    ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
    strlcpy(pi->safebox_password, p->szNewPassword,
            sizeof(pi->safebox_password));
    strlcpy(pi->login, p->szOldPassword, sizeof(pi->login));
    pi->account_id = p->dwID;

    auto query = fmt::format("SELECT password FROM safebox WHERE account_id={}",
                             p->dwID);
    CDBManager::instance().ReturnQuery(query, QID_SAFEBOX_CHANGE_PASSWORD,
                                       pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, SQLMsg* msg)
{
    CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
    ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
    uint32_t dwHandle = p->dwHandle;

    if (msg->Get()->uiNumRows > 0) {
        MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

        if ((row[0] && *row[0] && !strcasecmp(row[0], p->login)) ||
            ((!row[0] || !*row[0]) && !strcmp("000000", p->login))) {
            char escape_pwd[64];
            CDBManager::instance().EscapeString(escape_pwd, p->safebox_password,
                                                strlen(p->safebox_password));
            auto query = fmt::format("UPDATE safebox SET password='{}' WHERE "
                                     "account_id={}",
                                     escape_pwd, p->account_id);
            CDBManager::instance().ReturnQuery(
                query, QID_SAFEBOX_CHANGE_PASSWORD_SECOND, pkPeer->GetHandle(),
                p);
            return;
        }
    } else {
        char escape_pwd[64];
        CDBManager::instance().EscapeString(escape_pwd, p->safebox_password,
                                            strlen(p->safebox_password));
        auto query = fmt::format("INSERT INTO safebox SET password='{}', "
                                 "account_id={}",
                                 escape_pwd, p->account_id);
        CDBManager::instance().ReturnQuery(
            query, QID_SAFEBOX_CHANGE_PASSWORD_SECOND, pkPeer->GetHandle(), p);
        return;
    }

    delete p;

    // Wrong old password
    pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle,
                         sizeof(uint8_t));
    pkPeer->EncodeBYTE(0);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(CPeer* pkPeer,
                                                           SQLMsg* msg)
{
    CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
    ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
    uint32_t dwHandle = p->dwHandle;
    delete p;

    pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle,
                         sizeof(uint8_t));
    pkPeer->EncodeBYTE(1);
}

// MYSHOP_PRICE_LIST
void CClientManager::RESULT_PRICELIST_LOAD(CPeer* peer, SQLMsg* pMsg)
{
    auto req =
        (TItemPricelistReqInfo*)static_cast<CQueryInfo*>(pMsg->pvUserData)
            ->pvData;

    //
    // DB ¿¡¼­ ·ÎµåÇÑ Á¤º¸¸¦ Cache ¿¡ ÀúÀå
    //

    MyShopItemPriceListTable table;
    table.pid = req->second;
    table.count = 0;

    if (0 != pMsg->Get()->uiNumRows) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)) &&
               table.count < STORM_ARRAYSIZE(table.info)) {
            str_to_number(table.info[table.count].vnum, row[0]);
            str_to_number(table.info[table.count].price, row[1]);
            table.count++;
        }
    }

    //
    // 로드한 데이터를 Game server 에 전송
    //

    MyShopPriceListHeader header;
    header.pid = req->second;
    header.bundleItem = {};
    header.count = table.count;

    size_t sizePriceListSize = sizeof(MyShopPriceInfo) * header.count;

    peer->EncodeHeader(HEADER_DG_MYSHOP_PRICELIST_RES, req->first,
                       sizeof(header) + sizePriceListSize);
    peer->Encode(&header, sizeof(header));
    peer->Encode(table.info, sizePriceListSize);
    delete req;
}

void CClientManager::QUERY_SAFEBOX_SAVE(CPeer* pkPeer, TSafeboxTable* pTable)
{
    auto query = fmt::format("UPDATE safebox SET gold='{}' WHERE account_id={}",
                             pTable->dwGold, pTable->dwID);

    CDBManager::instance().ReturnQuery(query, QID_SAFEBOX_SAVE,
                                       pkPeer->GetHandle(), nullptr);
}

void CClientManager::QUERY_SETUP(CPeer* peer, uint32_t dwHandle,
                                 const char* c_pData)
{
    TPacketGDSetup* p = (TPacketGDSetup*)c_pData;
    c_pData += sizeof(TPacketGDSetup);

    if (p->bAuthServer) {
        SPDLOG_DEBUG("AUTH_PEER ptr {0}", (void*)peer);

        m_pkAuthPeer = peer;
        return;
    }

    peer->SetGameIp(p->gameIp);
    peer->SetGamePort(p->gamePort);
    peer->SetChannel(p->bChannel);

    //
    // ¼Â¾÷ : Á¢¼ÓÇÑ ÇÇ¾î¿¡ ´Ù¸¥ ÇÇ¾îµéÀÌ Á¢¼ÓÇÏ°Ô ¸¸µç´Ù. (P2P ÄÁ³Ø¼Ç »ý¼º)
    //
    SPDLOG_INFO("SETUP: channel {0} game {1}:{2}", peer->GetChannel(),
                p->gameIp, p->gamePort);
    ;

    SendPartyOnSetup(peer);

    for (const auto& other : m_peerList) {
        if (other.get() == peer)
            continue;

        SendPartyOnSetup(other.get());
    }

    CGuildManager::instance().OnSetup(peer);
    CPrivManager::instance().SendPrivOnSetup(peer);
    SendEventFlagsOnSetup(peer);
    SendHwidFlagsOnSetup(peer);
    marriage::CManager::instance().OnSetup(peer);
}

void CClientManager::QUERY_ITEM_SAVE(CPeer* pkPeer, const char* c_pData)
{
    const auto p = (TPlayerItem*)c_pData;

    // SAFEBOX/MALL isn't cached.
    if (p->window == SAFEBOX || p->window == MALL) {
        RemoveCachedItem(p->id);

        std::string query;
        CreateItemSaveQuery(*p, query);

        CDBManager::instance().ReturnQuery(query, QID_ITEM_SAVE,
                                           pkPeer->GetHandle(), NULL);

        return;
    }

    auto& cachedItem = m_itemCache.ForceGet(p->id);
    const auto& oldData = cachedItem.GetData();

    if (oldData.owner != p->owner) {
        if (oldData.owner != 0) {
            auto oldOwner = m_playerCache.Get(oldData.owner);
            if (oldOwner)
                oldOwner->RemoveItem(&cachedItem);
        }

        auto newOwner = m_playerCache.Get(p->owner);
        if (newOwner)
            newOwner->AddItem(&cachedItem);
    }

    cachedItem.SetData(*p);
}

void CClientManager::QUERY_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData)
{
    uint32_t dwID = *(uint32_t*)c_pData;
    c_pData += sizeof(uint32_t);

    uint32_t dwPID = *(uint32_t*)c_pData;

    RemoveCachedItem(dwID);

    auto query = fmt::format("DELETE FROM item WHERE id={}", dwID);

    SPDLOG_DEBUG("HEADER_GD_ITEM_DESTROY: PID {} ID {}", dwPID, dwID);

    if (dwPID == 0)
        CDBManager::instance().AsyncQuery(query);
    else
        CDBManager::instance().ReturnQuery(query, QID_ITEM_DESTROY,
                                           pkPeer->GetHandle(), nullptr);
}

void CClientManager::QUERY_FLUSH_CACHE(CPeer* pkPeer, const char* c_pData)
{
    uint32_t dwPID = *(uint32_t*)c_pData;

    auto cachedPlayer = m_playerCache.ForceGet(dwPID);
    if (cachedPlayer.IsLocked()) {
        spdlog::error("Trying to flush+forget locked player {}", dwPID);
        return;
    }

    cachedPlayer.Flush();
    m_playerCache.Remove(dwPID);
}

void CClientManager::QUERY_RELOAD_PROTO()
{
    if (!InitializeTables()) {
        spdlog::error("QUERY_RELOAD_PROTO: cannot load tables");
        return;
    }

    for (auto i = m_peerList.begin(); i != m_peerList.end(); ++i) {
        auto tmp = *i;

        tmp->EncodeHeader(HEADER_DG_RELOAD_PROTO, 0,
                          sizeof(uint16_t) + sizeof(TBanwordTable) *
                                                 m_vec_banwordTable.size());

        tmp->EncodeWORD(m_vec_banwordTable.size());
        tmp->Encode(&m_vec_banwordTable[0],
                    sizeof(TBanwordTable) * m_vec_banwordTable.size());
    }
}

// ADD_GUILD_PRIV_TIME
/**
 * @version	05/06/08 Bang2ni - Áö¼Ó½Ã°£ Ãß°¡
 */
void CClientManager::AddGuildPriv(TPacketGiveGuildPriv* p)
{
    CPrivManager::instance().AddGuildPriv(p->guild_id, p->type, p->value,
                                          p->duration_sec);
}

void CClientManager::AddEmpirePriv(TPacketGiveEmpirePriv* p)
{
    CPrivManager::instance().AddEmpirePriv(p->empire, p->type, p->value,
                                           p->duration_sec);
}

// END_OF_ADD_GUILD_PRIV_TIME

void CClientManager::AddCharacterPriv(TPacketGiveCharacterPriv* p)
{
    CPrivManager::instance().AddCharPriv(p->pid, p->type, p->value);
}

void CClientManager::MoneyLog(TPacketMoneyLog* p)
{
    CMoneyLog::instance().AddLog(p->type, p->vnum, p->gold);
}

void CClientManager::GuildDepositMoney(TPacketGDGuildMoney* p)
{
    CGuildManager::instance().DepositMoney(p->dwGuild, p->iGold);
}

void CClientManager::GuildWithdrawMoney(CPeer* peer, TPacketGDGuildMoney* p)
{
    CGuildManager::instance().WithdrawMoney(peer, p->dwGuild, p->iGold);
}

void CClientManager::GuildWithdrawMoneyGiveReply(
    TPacketGDGuildMoneyWithdrawGiveReply* p)
{
    CGuildManager::instance().WithdrawMoneyReply(p->dwGuild, p->bGiveSuccess,
                                                 p->iChangeGold);
}

void CClientManager::UpdateObject(TPacketGDUpdateObject* p)
{
    auto query = fmt::format("UPDATE object SET land_id={}, map_index={}, "
                             "x={}, y={}, x_rot={}, y_rot={}, z_rot={} WHERE "
                             "id = {}",
                             p->dwLandID, p->lMapIndex, p->x, p->y, p->xRot,
                             p->yRot, p->zRot, p->dwID);

    std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(query));

    if (pmsg->Get()->uiAffectedRows == 0) {
        spdlog::error("cannot insert object");
        return;
    }

    auto it = m_map_pkObjectTable.find(p->dwID);
    if (it == m_map_pkObjectTable.end())
        return;

    it->second->id = p->dwID;
    it->second->landId = p->dwLandID;
    it->second->mapIndex = p->lMapIndex;
    it->second->x = p->x;
    it->second->y = p->y;
    it->second->xRot = p->xRot;
    it->second->yRot = p->yRot;
    it->second->zRot = p->zRot;
    it->second->life = 0;

    ForwardPacket(HEADER_DG_UPDATE_OBJECT, it->second.get(), sizeof(TObject));
}

void CClientManager::CreateObject(TPacketGDCreateObject* p)
{
    char szQuery[512];

    snprintf(szQuery, sizeof(szQuery),
             "INSERT INTO object (land_id, vnum, map_index, x, y, x_rot, "
             "y_rot, z_rot) VALUES(%u, %u, %d, %d, %d, %f, "
             "%f, %f)",
             p->dwLandID, p->dwVnum, p->lMapIndex, p->x, p->y, p->xRot, p->yRot,
             p->zRot);

    std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(szQuery));

    if (pmsg->Get()->uiInsertID == 0) {
        spdlog::error("cannot insert object");
        return;
    }

    auto pkObj = std::make_unique<TObject>();
    pkObj->id = (uint32_t)pmsg->Get()->uiInsertID;
    pkObj->vnum = p->dwVnum;
    pkObj->landId = p->dwLandID;
    pkObj->mapIndex = p->lMapIndex;
    pkObj->x = p->x;
    pkObj->y = p->y;
    pkObj->xRot = p->xRot;
    pkObj->yRot = p->yRot;
    pkObj->zRot = p->zRot;
    pkObj->life = 0;

    ForwardPacket(HEADER_DG_CREATE_OBJECT, pkObj.get(), sizeof(TObject));

    m_map_pkObjectTable.emplace(pkObj->id, std::move(pkObj));
}

void CClientManager::DeleteObject(uint32_t dwID)
{
    std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(
        fmt::format("DELETE FROM object WHERE id={}", dwID)));

    if (pmsg->Get()->uiAffectedRows == 0 ||
        pmsg->Get()->uiAffectedRows == (uint32_t)-1) {
        spdlog::error("no object by id {0}", dwID);
        return;
    }

    auto it = m_map_pkObjectTable.find(dwID);
    if (it != m_map_pkObjectTable.end()) {
        m_map_pkObjectTable.erase(it);
    }

    ForwardPacket(HEADER_DG_DELETE_OBJECT, &dwID, sizeof(uint32_t));
}

void CClientManager::UpdateLand(uint32_t* pdw)
{
    uint32_t dwID = pdw[0];
    uint32_t dwGuild = pdw[1];

    TLand* p = &m_vec_kLandTable[0];

    uint32_t i;

    for (i = 0; i < m_vec_kLandTable.size(); ++i, ++p) {
        if (p->id == dwID) {
            CDBManager::instance().AsyncQuery(fmt::format(
                "UPDATE land SET guild_id={} WHERE id={}", dwGuild, dwID));

            p->guildId = dwGuild;
            break;
        }
    }

    if (i < m_vec_kLandTable.size())
        ForwardPacket(HEADER_DG_UPDATE_LAND, p, sizeof(TLand));
}

// BLOCK_CHAT
void CClientManager::BlockChat(TPacketBlockChat* p)
{
    std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(
        fmt::format("SELECT id FROM player WHERE name = '{}'", p->szName)));
    SQLResult* pRes = pmsg->Get();

    if (pRes->uiNumRows) {
        MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
        uint32_t pid = strtoul(row[0], nullptr, 10);

        TPacketGDAddAffect pa = {};
        pa.dwPID = pid;
        pa.elem.type = AFFECT_BLOCK_CHAT;
        pa.elem.duration = p->lDuration;
        QUERY_ADD_AFFECT(NULL, &pa);
    }
}

// END_OF_BLOCK_CHAT

void CClientManager::MarriageAdd(TPacketMarriageAdd* p)
{
    SPDLOG_DEBUG("MarriageAdd {} {} {} {}", p->dwPID1, p->dwPID2, p->szName1,
                 p->szName2);
    marriage::CManager::instance().Add(p->dwPID1, p->dwPID2, p->szName1,
                                       p->szName2);
}

void CClientManager::MarriageUpdate(TPacketMarriageUpdate* p)
{
    SPDLOG_DEBUG("MarriageUpdate PID:{} {} LP:{} ST:{}", p->dwPID1, p->dwPID2,
                 p->iLovePoint, p->byMarried);
    marriage::CManager::instance().Update(p->dwPID1, p->dwPID2, p->iLovePoint,
                                          p->byMarried);
}

void CClientManager::MarriageRemove(TPacketMarriageRemove* p)
{
    SPDLOG_DEBUG("MarriageRemove {} {}", p->dwPID1, p->dwPID2);
    marriage::CManager::instance().Remove(p->dwPID1, p->dwPID2);
}

void CClientManager::WeddingRequest(TPacketWeddingRequest* p)
{
    SPDLOG_DEBUG("WeddingRequest {} {}", p->dwPID1, p->dwPID2);
    ForwardPacket(HEADER_DG_WEDDING_REQUEST, p, sizeof(TPacketWeddingRequest));
    // marriage::CManager::instance().RegisterWedding(p->dwPID1, p->szName1,
    // p->dwPID2, p->szName2);
}

void CClientManager::WeddingReady(TPacketWeddingReady* p)
{
    SPDLOG_DEBUG("WeddingReady {} {}", p->dwPID1, p->dwPID2);
    ForwardPacket(HEADER_DG_WEDDING_READY, p, sizeof(TPacketWeddingReady));
    marriage::CManager::instance().ReadyWedding(p->dwMapIndex, p->dwPID1,
                                                p->dwPID2);
}

void CClientManager::WeddingEnd(TPacketWeddingEnd* p)
{
    SPDLOG_DEBUG("WeddingEnd {} {}", p->dwPID1, p->dwPID2);
    marriage::CManager::instance().EndWedding(p->dwPID1, p->dwPID2);
}

// Erase item cache
void CClientManager::EraseItemCache(uint32_t id)
{
    m_itemCache.Remove(id);
}

//
// Ä³½Ã¿¡ °¡°ÝÁ¤º¸°¡ ÀÖÀ¸¸é Ä³½Ã¸¦ ¾÷µ¥ÀÌÆ® ÇÏ°í Ä³½Ã¿¡ °¡°ÝÁ¤º¸°¡ ¾ø´Ù¸é
// ¿ì¼± ±âÁ¸ÀÇ µ¥ÀÌÅÍ¸¦ ·ÎµåÇÑ µÚ¿¡ ±âÁ¸ÀÇ Á¤º¸·Î Ä³½Ã¸¦ ¸¸µé°í »õ·Î ¹ÞÀº
// °¡°ÝÁ¤º¸¸¦ ¾÷µ¥ÀÌÆ® ÇÑ´Ù.
//
void CClientManager::MyshopPricelistUpdate(const MyShopPriceListHeader* p)
{
    if (p->count > SHOP_PRICELIST_MAX_NUM) {
        spdlog::error("count overflow!");
        return;
    }

    std::string query;

    query += fmt::format("DELETE FROM myshop_pricelist WHERE owner_id = {};",
                         p->pid);

    const auto info = (const MyShopPriceInfo*)(p + 1);
    for (uint32_t i = 0; i != p->count; ++i) {
        query += fmt::format("INSERT INTO myshop_pricelist (owner_id, "
                             "item_vnum, price) VALUES({}, {}, {});",
                             p->pid, info->vnum, info->price);
    }

    CDBManager::instance().AsyncQuery(query);
}

// MYSHOP_PRICE_LIST
// Ä³½ÃµÈ °¡°ÝÁ¤º¸°¡ ÀÖÀ¸¸é Ä³½Ã¸¦ ÀÐ¾î ¹Ù·Î Àü¼ÛÇÏ°í Ä³½Ã¿¡ Á¤º¸°¡ ¾øÀ¸¸é DB ¿¡
// Äõ¸®¸¦ ÇÑ´Ù.
//
void CClientManager::MyshopPricelistRequest(CPeer* peer, uint32_t dwHandle,
                                            const uint32_t* p)
{
    SPDLOG_INFO("Query MyShopPricelist handle[{}] pid[{}]", dwHandle, p->pid);
    auto query = fmt::format("SELECT item_vnum, price FROM myshop_pricelist "
                             "WHERE owner_id={}",
                             *p);
    CDBManager::instance().ReturnQuery(query, QID_ITEMPRICE_LOAD,
                                       peer->GetHandle(),
                                       new TItemPricelistReqInfo(dwHandle, *p));
}

// END_OF_MYSHOP_PRICE_LIST

void CClientManager::RemoveCachedItem(uint32_t id)
{
    auto* cachedItem = m_itemCache.Get(id);
    if (cachedItem && (cachedItem->GetData().window < SAFEBOX ||
                       cachedItem->GetData().window == DRAGON_SOUL_INVENTORY ||
                       cachedItem->GetData().window == SWITCHBOT)) {
        auto* cachedPlayer = m_playerCache.Get(cachedItem->GetData().owner);
        if (cachedPlayer)
            cachedPlayer->RemoveItem(cachedItem);
    }

    m_itemCache.Remove(id);
}

void CClientManager::ProcessPacket(CPeer* peer, uint8_t header,
                                   uint32_t dwHandle, const char* data,
                                   uint32_t dwLength)
{
    // STORM_DLOG(Info, "Process {0} on {1} with size {2}", header, dwHandle,
    // dwLength);

    switch (header) {

        case HEADER_GD_BOOT:
            QUERY_BOOT(peer, (TPacketGDBoot*)data);
            break;

        case HEADER_GD_PLAYER_LIST_QUERY:
            HandlePlayerListQuery(peer, dwHandle,
                                  *(GdPlayerListQueryPacket*)data);
            break;

        case HEADER_GD_PLAYER_LOAD:
            SPDLOG_INFO("HEADER_GD_PLAYER_LOAD (handle: {0} length: {1})",
                        dwHandle, dwLength);
            QUERY_PLAYER_LOAD(peer, dwHandle, (TPlayerLoadPacket*)data);
            break;

        case HEADER_GD_PLAYER_SAVE:
            SPDLOG_INFO("HEADER_GD_PLAYER_SAVE (handle: {0} length: {1})",
                        dwHandle, dwLength);
            QUERY_PLAYER_SAVE(peer, dwHandle, (TPlayerTable*)data);
            break;

        case HEADER_GD_PLAYER_DISCONNECT:
            HandlePlayerDisconnect(peer, *(GdPlayerDisconnectPacket*)data);
            break;

        case HEADER_GD_PLAYER_CREATE:
            SPDLOG_INFO("HEADER_GD_PLAYER_CREATE (handle: {0} length: {1})",
                        dwHandle, dwLength);
            __QUERY_PLAYER_CREATE(peer, dwHandle, (TPlayerCreatePacket*)data);
            SPDLOG_INFO("END PLAYER_CREATE");
            break;

        case HEADER_GD_PLAYER_DELETE:
            SPDLOG_INFO("HEADER_GD_PLAYER_DELETE (handle: {0} length: {1})",
                        dwHandle, dwLength);
            __QUERY_PLAYER_DELETE(peer, dwHandle, *(GdPlayerDeletePacket*)data);
            break;

        case HEADER_GD_QUEST_SAVE:
            SPDLOG_INFO("HEADER_GD_QUEST_SAVE (handle: {0} length: {1}",
                        dwHandle, dwLength);
            QUERY_QUEST_SAVE(peer, (TQuestTable*)data, dwLength);
            break;

        case HEADER_GD_SAFEBOX_LOAD:
            QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket*)data, 0);
            break;

        case HEADER_GD_SAFEBOX_SAVE:
            SPDLOG_INFO("HEADER_GD_SAFEBOX_SAVE (handle: {0} length: {1}",
                        dwHandle, dwLength);
            QUERY_SAFEBOX_SAVE(peer, (TSafeboxTable*)data);
            break;

        case HEADER_GD_SAFEBOX_CHANGE_SIZE:
            QUERY_SAFEBOX_CHANGE_SIZE(peer, dwHandle,
                                      (TSafeboxChangeSizePacket*)data);
            break;

        case HEADER_GD_SAFEBOX_CHANGE_PASSWORD:
            QUERY_SAFEBOX_CHANGE_PASSWORD(peer, dwHandle,
                                          (TSafeboxChangePasswordPacket*)data);
            break;

        case HEADER_GD_MALL_LOAD:
            QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket*)data, 1);
            break;

        case HEADER_GD_UPDATE_EMPIRE:
            break;

        case HEADER_GD_SETUP:
            QUERY_SETUP(peer, dwHandle, data);
            break;

        case HEADER_GD_GUILD_CREATE:
            GuildCreate(peer, *(uint32_t*)data);
            break;

        case HEADER_GD_GUILD_SKILL_UPDATE:
            GuildSkillUpdate(peer, (TPacketGuildSkillUpdate*)data);
            break;

        case HEADER_GD_GUILD_EXP_UPDATE:
            GuildExpUpdate(peer, (TPacketGuildExpUpdate*)data);
            break;

        case HEADER_GD_GUILD_ADD_MEMBER:
            GuildAddMember(peer, (TPacketGDGuildAddMember*)data);
            break;

        case HEADER_GD_GUILD_REMOVE_MEMBER:
            GuildRemoveMember(peer, (TPacketGuild*)data);
            break;

        case HEADER_GD_GUILD_CHANGE_GRADE:
            GuildChangeGrade(peer, (TPacketGuild*)data);
            break;

        case HEADER_GD_GUILD_CHANGE_MEMBER_DATA:
            GuildChangeMemberData(peer, (TPacketGuildChangeMemberData*)data);
            break;

        case HEADER_GD_GUILD_DISBAND:
            GuildDisband(peer, (TPacketGuild*)data);
            break;

        case HEADER_GD_GUILD_WAR:
            GuildWar(peer, (TPacketGuildWar*)data);
            break;

        case HEADER_GD_GUILD_WAR_SCORE:
            GuildWarScore(peer, (TPacketGuildWarScore*)data);
            break;

        case HEADER_GD_GUILD_CHANGE_LADDER_POINT:
            GuildChangeLadderPoint((TPacketGuildLadderPoint*)data);
            break;

        case HEADER_GD_GUILD_USE_SKILL:
            GuildUseSkill((TPacketGuildUseSkill*)data);
            break;

#ifdef ENABLE_GUILD_STORAGE
        case HEADER_GD_GUILDSTORAGE_MOVE:
            GuildMoveItem((TPacketGuildStorageMoveItem*)data, peer);
            break;

        case HEADER_GD_GUILDSTORAGE_ADD_ITEM:
            GuildAddItem((TPacketGuildStorageAddItem*)data, peer);
            break;

        case HEADER_GD_GUILDSTORAGE_REMOVE_ITEM:
            GuildRemoveItem((TPacketGuildStorageRemoveItemRequest*)data, peer);
            break;

        case HEADER_GD_GUILDSTORAGE_ADD_ITEM_D:
            GuildAddItemRemoveResponse((TPacketGuildStorageAddItem*)data, peer);
            break;

        case HEADER_GD_REQUEST_GUILD_ITEMS:
            GuildLoadItems((TPacketRequestGuildItems*)data, peer);
            break;
#endif

        case HEADER_GD_FLUSH_CACHE:
            QUERY_FLUSH_CACHE(peer, data);
            break;

        case HEADER_GD_ITEM_SAVE:
            QUERY_ITEM_SAVE(peer, data);
            break;

        case HEADER_GD_ITEM_DESTROY:
            QUERY_ITEM_DESTROY(peer, data);
            break;

        case HEADER_GD_ADD_SWITCHBOT_DATA:
            HandleQueryAddSwitchbotData(peer, (TPacketGDAddSwitchbotData*)data);
            break;

        case HEADER_GD_REMOVE_SWITCHBOT_DATA:
            HandleQueryRemoveSwitchbotData(peer,
                                           (TPacketGDRemoveSwitchbotData*)data);
            break;

        case HEADER_GD_ADD_AFFECT:
            QUERY_ADD_AFFECT(peer, (TPacketGDAddAffect*)data);
            break;

        case HEADER_GD_REMOVE_AFFECT:
            QUERY_REMOVE_AFFECT(peer, (TPacketGDRemoveAffect*)data);
            break;

        case HEADER_GD_HIGHSCORE_REGISTER:
            QUERY_HIGHSCORE_REGISTER(peer, (TPacketGDHighscore*)data);
            break;

        case HEADER_GD_PARTY_CREATE:
            QUERY_PARTY_CREATE(peer, (TPacketPartyCreate*)data);
            break;

        case HEADER_GD_PARTY_DELETE:
            QUERY_PARTY_DELETE(peer, (TPacketPartyDelete*)data);
            break;

        case HEADER_GD_PARTY_ADD:
            QUERY_PARTY_ADD(peer, (TPacketPartyAdd*)data);
            break;

        case HEADER_GD_PARTY_REMOVE:
            QUERY_PARTY_REMOVE(peer, (TPacketPartyRemove*)data);
            break;

        case HEADER_GD_PARTY_STATE_CHANGE:
            QUERY_PARTY_STATE_CHANGE(peer, (TPacketPartyStateChange*)data);
            break;

        case HEADER_GD_PARTY_SET_MEMBER_LEVEL:
            QUERY_PARTY_SET_MEMBER_LEVEL(peer,
                                         (TPacketPartySetMemberLevel*)data);
            break;

        case HEADER_GD_RELOAD_PROTO:
            QUERY_RELOAD_PROTO();
            break;

        case HEADER_GD_CHANGE_NAME:
            HandleChangeName(peer, dwHandle, (GdChangeNamePacket*)data);
            break;

        case HEADER_GD_CHANGE_EMPIRE:
            HandleChangeEmpire(peer, dwHandle, (GdChangeEmpirePacket*)data);
            break;

        case HEADER_GD_AUTH_SET_PIN:
            HandleAuthSetPinQuery(peer, dwHandle,
                                  *(GdAuthSetPinQueryPacket*)data);
            break;

        case HEADER_GD_AUTH_LOGIN:
            HandleAuthLoginQuery(peer, dwHandle,
                                 *(GdAuthLoginQueryPacket*)data);
            break;

        case HEADER_GD_AUTH_REGISTER:
            HandleAuthRegisterQuery(peer, dwHandle,
                                 *(GdAuthRegisterQueryPacket*)data);
            break;

        case HEADER_GD_HGUARD_INFO:
            HandleHGuardInfoQuery(peer, dwHandle,
                                  *(GdHGuardInfoQueryPacket*)data);
            break;

        case HEADER_GD_HWID_INFO:
            HandleHwidInfoQuery(peer, dwHandle, *(GdHwidInfoQueryPacket*)data);
            break;

        case HEADER_GD_HGUARD_VERIFY:
            HandleHGuardVerifyQuery(peer, dwHandle,
                                    *(GdHGuardValidationQueryPacket*)data);
            break;

        case HEADER_GD_REQUEST_GUILD_PRIV:
            AddGuildPriv((TPacketGiveGuildPriv*)data);
            break;

        case HEADER_GD_REQUEST_EMPIRE_PRIV:
            AddEmpirePriv((TPacketGiveEmpirePriv*)data);
            break;

        case HEADER_GD_REQUEST_CHARACTER_PRIV:
            AddCharacterPriv((TPacketGiveCharacterPriv*)data);
            break;

        case HEADER_GD_MONEY_LOG:
            MoneyLog((TPacketMoneyLog*)data);
            break;

        case HEADER_GD_GUILD_DEPOSIT_MONEY:
            GuildDepositMoney((TPacketGDGuildMoney*)data);
            break;

        case HEADER_GD_GUILD_WITHDRAW_MONEY:
            GuildWithdrawMoney(peer, (TPacketGDGuildMoney*)data);
            break;

        case HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY:
            GuildWithdrawMoneyGiveReply(
                (TPacketGDGuildMoneyWithdrawGiveReply*)data);
            break;

        case HEADER_GD_SET_EVENT_FLAG:
            SetEventFlag((TPacketSetEventFlag*)data);
            break;

        case HEADER_GD_SET_HWID_FLAG:
            SetHwidFlag((TPacketSetHwidFlag*)data);
            break;

        case HEADER_GD_CREATE_OBJECT:
            CreateObject((TPacketGDCreateObject*)data);
            break;

        case HEADER_GD_UPDATE_OBJECT:
            UpdateObject((TPacketGDUpdateObject*)data);
            break;

        case HEADER_GD_DELETE_OBJECT:
            DeleteObject(*(uint32_t*)data);
            break;

        case HEADER_GD_UPDATE_LAND:
            UpdateLand((uint32_t*)data);
            break;

        case HEADER_GD_MARRIAGE_ADD:
            MarriageAdd((TPacketMarriageAdd*)data);
            break;

        case HEADER_GD_MARRIAGE_UPDATE:
            MarriageUpdate((TPacketMarriageUpdate*)data);
            break;

        case HEADER_GD_MARRIAGE_REMOVE:
            MarriageRemove((TPacketMarriageRemove*)data);
            break;

        case HEADER_GD_WEDDING_REQUEST:
            WeddingRequest((TPacketWeddingRequest*)data);
            break;

        case HEADER_GD_WEDDING_READY:
            WeddingReady((TPacketWeddingReady*)data);
            break;

        case HEADER_GD_WEDDING_END:
            WeddingEnd((TPacketWeddingEnd*)data);
            break;

            // BLOCK_CHAT
        case HEADER_GD_BLOCK_CHAT:
            BlockChat((TPacketBlockChat*)data);
            break;
            // END_OF_BLOCK_CHAT

            // MYSHOP_PRICE_LIST
        case HEADER_GD_MYSHOP_PRICELIST_UPDATE:
            MyshopPricelistUpdate((MyShopPriceListHeader*)data);
            break;

        case HEADER_GD_MYSHOP_PRICELIST_REQ:
            MyshopPricelistRequest(peer, dwHandle,
                                   (uint32_t*)data);
            break;
            // END_OF_MYSHOP_PRICE_LIST

            // RELOAD_ADMIN
        case HEADER_GD_RELOAD_ADMIN:
            ReloadAdmin();
            break;
            // END_RELOAD_ADMIN

        case HEADER_GD_BREAK_MARRIAGE:
            BreakMarriage(peer, data);
            break;

        case HEADER_GD_REQ_SPARE_ITEM_ID_RANGE:
            SendSpareItemIDRange(peer);
            break;

        case HEADER_GD_REQ_CHANGE_GUILD_MASTER:
            GuildChangeMaster((TPacketChangeGuildMaster*)data);
            break;

        case HEADER_GD_UPDATE_HORSE_NAME:
            UpdateHorseName((TPacketUpdateHorseName*)data, peer);
            break;

        case HEADER_GD_REQ_HORSE_NAME:
            AckHorseName(*(uint32_t*)data, peer);
            break;

        case HEADER_GD_UPDATE_ITEM_NAME:
            UpdateItemName((TPacketUpdateItemName*)data, peer);
            break;

        case HEADER_GD_REQ_ITEM_NAME:
            AckItemName(*(uint32_t*)data, peer);
            break;

        case HEADER_GD_REQUEST_CHARGE_CASH:
            ChargeCash((TRequestChargeCash*)data, dwHandle, peer);
            break;

        case HEADER_GD_MESSENGER_SET_BLOCK:
            MessengerSetBlock(
                reinterpret_cast<const PacketGDMessengerSetBlock*>(data));
            break;
        case HEADER_GD_DUNGEON_INFO_SAVE:
            QUERY_DUNGEON_INFO_SAVE(peer, data);
            break;
        case HEADER_GD_DELETE_AWARDID:
            DeleteAwardId((TPacketDeleteAwardID*)data);
            break;

#ifdef __DUNGEON_FOR_GUILD__
        case HEADER_GD_GUILD_DUNGEON:
            GuildDungeon((TPacketGDGuildDungeon*)data);
            break;
        case HEADER_GD_GUILD_DUNGEON_CD:
            GuildDungeonGD((TPacketGDGuildDungeonCD*)data);
            break;
#endif

#ifdef __OFFLINE_SHOP__
        case HEADER_GD_OFFLINE_SHOP_CREATE:
            this->ReceiveOfflineShopCreatePacket(
                (TPacketGDOfflineShopCreate*)data, peer->GetChannel());
            break;

        case HEADER_GD_OFFLINE_SHOP_NAME:
            this->ReceiveOfflineShopUpdateNamePacket(
                peer, (TPacketOfflineShopUpdateName*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_POSITION:
            this->ReceiveOfflineShopUpdatePositionPacket(
                (TPacketGDOfflineShopUpdatePosition*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_ITEM_ADD:
            this->ReceiveOfflineShopAddItemPacket(
                peer, (TPacketOfflineShopAddItem*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_ITEM_MOVE:
            this->ReceiveOfflineShopMoveItemPacket(
                peer, (TPacketOfflineShopMoveItem*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_ITEM_REMOVE:
            this->ReceiveOfflineShopRemoveItemPacket(
                peer, (TPacketOfflineShopRemoveItem*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_ITEM_BUY_BY_ID:
            this->ReceiveOfflineShopBuyItemByIdPacket(
                peer, dwHandle, (TPacketOfflineShopRemoveItemById*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_ITEM_BUY_REQUEST_BY_ID:
            this->ReceiveOfflineShopRequestBuyRequest(
                peer, dwHandle, (uint32_t*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_SEARCH_REQUEST:
            this->ReceiveOfflineShopSearchPacket(
                peer, dwHandle, (TPacketOfflineShopSearch*)data);
            break;

      case HEADER_GD_OFFLINE_SHOP_GOLD:
            this->ReceiveOfflineShopUpdateGoldPacket(
                peer, (TPacketOfflineShopUpdateGold*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_DESTROY:
            this->ReceiveOfflineShopDestroyPacket(
                peer, (TPacketOfflineShopDestroy*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_REGISTER_LISTENER:
            this->ReceiveOfflineShopRegisterListenerPacket(
                peer, (TPacketGDOfflineShopRegisterListener*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_UNREGISTER_LISTENER:
            this->ReceiveOfflineShopUnregisterListenerPacket(
                peer, (TPacketGDOfflineShopUnregisterListener*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_CLOSE:
            this->ReceiveOfflineShopClosePacket(peer,
                                                (TPacketOfflineShopClose*)data);
            break;

        case HEADER_GD_OFFLINE_SHOP_OPEN:
            this->ReceiveOfflineShopOpenPacket(peer,
                                               (TPacketOfflineShopOpen*)data);
            break;

#if defined(__OFFLINE_SHOP_OPENING_TIME__) ||                                  \
    defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
        case HEADER_GD_OFFLINE_SHOP_FLUSH_LEFT_OPENING_TIME:
            this->ReceiveOfflineShopFlushLeftOpeningTimePacket(
                (TPacketGDOfflineShopFlushLeftOpeningTime*)data);
            break;
#endif
#endif

#ifdef ENABLE_GEM_SYSTEM
        case HEADER_GD_GEM_LOG:
            GemLog((TPacketGemLog*)data);
            break;
#endif

#ifdef ENABLE_BATTLE_PASS
        case HEADER_GD_SAVE_BATTLE_PASS:
            QUERY_SAVE_BATTLE_PASS(peer, dwHandle,
                                   (TPlayerBattlePassMission*)data);
            break;

        case HEADER_GD_REGISTER_BP_RANKING:
            QUERY_REGISTER_RANKING(peer, dwHandle,
                                   (TBattlePassRegisterRanking*)data);
            break;

        case HEADER_GD_BATTLE_PASS_RANKING:
            RequestLoadBattlePassRanking(peer, dwHandle, data);
            break;
#endif
        case HEADER_GD_SAVE_HUNTING_MISSION:
            QuerySaveHuntingMissions(peer, dwHandle,
                                     (TPlayerHuntingMission*)data);
            break;
        default:
            spdlog::error("Unknown header (header: {0} handle: {1} length: "
                          "{2})",
                          header, dwHandle, dwLength);
            break;
    }
}

void CClientManager::AddPeer(asio::ip::tcp::socket socket)
{
    auto p = std::make_shared<CPeer>(std::move(socket), ++m_nextPeerHandle);
    p->Setup();
    m_peerList.push_front(std::move(p));
}

void CClientManager::RemovePeer(CPeer* pPeer)
{

    // Remove the peer from the offline shop listeners to prevent
    // heap use after free bugs
    // Probably would be better to use weak pointers at some point
    for (auto& [ownerPid, offlineShop] : m_mapOfflineShops) {
        offlineShop.listListenerPeers.remove_if(
            [&pPeer](CPeer* p) { return p == pPeer; });
    }

    m_peerList.remove_if(
        [&pPeer](const std::shared_ptr<CPeer>& p) { return p.get() == pPeer; });
}

bool CClientManager::HasPeer(CPeer* peer)
{
    return std::find_if(m_peerList.begin(), m_peerList.end(),
                        [&peer](const std::shared_ptr<CPeer>& p) {
                            return p.get() == peer;
                        }) != m_peerList.end();
}

CPeer* CClientManager::GetPeer(uint32_t handle)
{
    for (const auto& peer : m_peerList) {
        if (peer && peer->GetHandle() == handle)
            return peer.get();
    }

    return nullptr;
}

CPeer* CClientManager::GetAnyPeer()
{
    if (m_peerList.empty())
        return NULL;

    return m_peerList.front().get();
}

// DB Process the results received from the manager.
int CClientManager::AnalyzeQueryResult(SQLMsg* msg)
{
    CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
    CPeer* peer = GetPeer(qi->dwIdent);

    SPDLOG_TRACE("AnalyzeQueryResult {0}", qi->iType);

    switch (qi->iType) {
        case QID_ITEM_AWARD_LOAD:
            ItemAwardManager::instance().Load(msg);
            delete qi;
            return true;

        case QID_GUILD_RANKING:
            CGuildManager::instance().ResultRanking(msg->Get()->pSQLResult);
            break;
    }

    if (!peer) {
        delete qi;
        return true;
    }

    switch (qi->iType) {
        case QID_PLAYER:
        case QID_SKILL:
        case QID_QUICKSLOT:
        case QID_ITEM:
        case QID_QUEST:
        case QID_AFFECT:
#ifdef ENABLE_BATTLE_PASS
        case QID_BATTLE_PASS:
#endif
        case QID_HUNTING_MISSIONS:
        case QID_SWITCHBOT_SLOT_DATA:
        case QID_PET:
        case QID_TITLE:
        case QID_MESSENGER_BLOCK:
        case QID_DUNGEON_INFO:

            RESULT_COMPOSITE_PLAYER(peer, msg, qi->iType);
            break;

        case QID_AUTH_LOGIN:
            HandleAuthLoginResult(peer, msg);
            break;

        case QID_AUTH_REGISTER_NAME_CHECK:
            HandleAuthRegisterNameCheckResult(peer, msg);
            break;
        case QID_AUTH_REGISTER:
            HandleAuthRegister(peer, msg);
            break;

        case QID_GET_HGUARD_INFO:
            HandleHGuardInfoResult(peer, msg);
            break;

        case QID_VALIDATE_HGUARD_CODE:
            HandleHGuardVerifyResult(peer, msg);
            break;

        case QID_SAFEBOX_LOAD:
            RESULT_SAFEBOX_LOAD(peer, msg);
            break;

        case QID_SAFEBOX_CHANGE_SIZE:
            RESULT_SAFEBOX_CHANGE_SIZE(peer, msg);
            break;

        case QID_SAFEBOX_CHANGE_PASSWORD:
            RESULT_SAFEBOX_CHANGE_PASSWORD(peer, msg);
            break;

        case QID_SAFEBOX_CHANGE_PASSWORD_SECOND:
            RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(peer, msg);
            break;

        case QID_HIGHSCORE_REGISTER:
            RESULT_HIGHSCORE_REGISTER(peer, msg);
            break;

        case QID_SAFEBOX_SAVE:
        case QID_ITEM_SAVE:
        case QID_ITEM_DESTROY:
        case QID_QUEST_SAVE:
        case QID_PLAYER_SAVE:
        case QID_ITEM_AWARD_TAKEN:
            break;

        case QID_PLAYER_LIST:
            HandlePlayerListResult(peer, msg);
            break;

        case QID_ITEMPRICE_LOAD:
            RESULT_PRICELIST_LOAD(peer, msg);
            break;

        default:
            SPDLOG_INFO("Unknown query result type {0}: str: {1}", qi->iType,
                        msg->stQuery);
            break;
    }

    delete qi;
    return true;
}

void CClientManager::StartDbTimer()
{
    // TODO(tim): Don't use a timer here!
    m_dbProcessTimer.expires_from_now(chrono::milliseconds(25));
    m_dbProcessTimer.async_wait([this](const boost::system::error_code& ec) {
        if (ec == asio::error::operation_aborted)
            return;

        if (ec) {
            spdlog::error("Failed to wait for db timer: {0}", ec.message());
            return;
        }

        SQLMsg* tmp;
        while ((tmp = CDBManager::instance().PopResult())) {
            AnalyzeQueryResult(tmp);
            delete tmp;
        }

        StartDbTimer();
    });
}

void CClientManager::StartUpdateTimer()
{
    m_updateTimer.expires_from_now(chrono::seconds(1));
    m_updateTimer.async_wait([this](const boost::system::error_code& ec) {
        if (ec == asio::error::operation_aborted)
            return;

        if (ec) {
            spdlog::error("Failed to wait for update timer: {0}", ec.message());
            return;
        }

        CDBManager::instance().ResetCounter();

        m_iCacheFlushCount = 0;

        auto now = chrono::steady_clock::now();
        m_playerCache.Update(now);
        m_itemCache.Update(now);
#ifdef __OFFLINE_SHOP__
        this->FlushOfflineShopCache();
        this->FlushOfflineShopItemCache();
#endif
        UpdateDungeonInfoCache();

        CGuildManager::instance().Update();
        CPrivManager::instance().Update();
        marriage::CManager::instance().Update();

        StartUpdateTimer();
    });
}

void CClientManager::StartHourlyTimer()
{
    m_hourlyTimer.expires_from_now(chrono::hours(1));
    m_hourlyTimer.async_wait([this](const boost::system::error_code& ec) {
        if (ec == asio::error::operation_aborted)
            return;

        if (ec) {
            spdlog::error("Failed to wait for hourly timer: {0}", ec.message());
            return;
        }

#ifdef ENABLE_BATTLE_PASS
        LoadBattlePassRanking();
#endif

        StartHourlyTimer();
    });
}

void CClientManager::StartDungeonRankingTimer()
{
    m_dungeonRankingTimer.expires_from_now(chrono::minutes(10));
    m_dungeonRankingTimer.async_wait([this](const boost::system::error_code& ec) {
        if (ec == asio::error::operation_aborted)
            return;

        if (ec) {
            spdlog::error("Failed to wait for hourly timer: {0}", ec.message());
            return;
        }

#ifdef ENABLE_BATTLE_PASS
        InitializeDungeonRanking(true);
#endif

        StartDungeonRankingTimer();
    });
}

void CClientManager::SendAllGuildSkillRechargePacket()
{
    ForwardPacket(HEADER_DG_GUILD_SKILL_RECHARGE, nullptr, 0);
}

void CClientManager::ForwardPacket(uint8_t header, const void* data, int size,
                                   uint8_t bChannel, CPeer* except)
{
    for (const auto& p : m_peerList) {
        if (p.get() == except)
            continue;

        if (!p->GetChannel())
            continue;

        if (bChannel && p->GetChannel() != bChannel)
            continue;

        p->EncodeHeader(header, 0, size);

        if (size > 0 && data)
            p->Encode(data, size);
    }
}

void CClientManager::SendNotice(const char* c_pszFormat, ...)
{
    char szBuf[255 + 1];
    va_list args;

    va_start(args, c_pszFormat);
    int len = vsnprintf(szBuf, sizeof(szBuf), c_pszFormat, args);
    va_end(args);
    szBuf[len] = '\0';

    ForwardPacket(HEADER_DG_NOTICE, szBuf, len + 1);
}

time_t CClientManager::GetCurrentTime()
{
    return time(nullptr);
}

// ITEM_UNIQUE_ID
bool CClientManager::InitializeNowItemID()
{
    if (!CItemIDRangeManager::instance().GetRange(m_itemRange)) {
        spdlog::error("Out of item ID ranges");
        return false;
    }

    SPDLOG_INFO("Grabbed new item ID range: [{0}, {1}] start {2}",
                m_itemRange.min, m_itemRange.max, m_itemRange.start);
    return true;
}

uint32_t CClientManager::AllocateItemId()
{
    // If we're out of IDs, we'll attempt to get a new range.
    // Usually that only happens on boot.
    if (m_itemRange.start > m_itemRange.max && !InitializeNowItemID())
        return 0;

    return m_itemRange.start++;
}

// BOOT_LOCALIZATION

bool CClientManager::InitializeLocalization()
{
    std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(
        "SELECT mValue, mKey FROM locale", SQL_COMMON));

    if (pMsg->Get()->uiNumRows == 0) {
        spdlog::error("InitializeLocalization() - Locale load query failed");
        return false;
    }

    SPDLOG_INFO("InitializeLocalization() - LoadLocaleTable(count:%d)",
                pMsg->Get()->uiNumRows);

    m_vec_Locale.clear();

    MYSQL_ROW row;

    for (int n = 0; (row = mysql_fetch_row(pMsg->Get()->pSQLResult)) != nullptr;
         ++n) {
        int col = 0;
        tLocale locale;

        strlcpy(locale.szValue, row[col++], sizeof(locale.szValue));
        strlcpy(locale.szKey, row[col++], sizeof(locale.szKey));

        m_vec_Locale.push_back(locale);
    }

    return true;
}

// END_BOOT_LOCALIZATION
// ADMIN_MANAGER
// ADMIN_MANAGER

bool CClientManager::__GetAdminInfo(std::vector<tAdminInfo>& rAdminVec)
{
    std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(
        "SELECT mID,mAccount,mName,mAuthority FROM gmlist", SQL_COMMON));

    if (pMsg->Get()->uiNumRows == 0) {
        spdlog::error("__GetAdminInfo() ==> Failed to fetch gmlist");
        return false;
    }

    MYSQL_ROW row;
    rAdminVec.reserve(pMsg->Get()->uiNumRows);

    while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult))) {
        int idx = 0;
        tAdminInfo Info;

        str_to_number(Info.m_ID, row[idx++]);
        trim_and_lower(row[idx++], Info.m_szAccount, sizeof(Info.m_szAccount));
        strlcpy(Info.m_szName, row[idx++], sizeof(Info.m_szName));
        std::string stAuth = row[idx++];

        if (!stAuth.compare("IMPLEMENTOR"))
            Info.m_Authority = GM_IMPLEMENTOR;
        else if (!stAuth.compare("GOD"))
            Info.m_Authority = GM_GOD;
        else if (!stAuth.compare("HIGH_WIZARD"))
            Info.m_Authority = GM_HIGH_WIZARD;
        else if (!stAuth.compare("LOW_WIZARD"))
            Info.m_Authority = GM_LOW_WIZARD;
        else if (!stAuth.compare("WIZARD"))
            Info.m_Authority = GM_WIZARD;
        else
            continue;

        rAdminVec.push_back(Info);

        SPDLOG_INFO("GM: PID {} Login {} Character {} Authority {}[{}]",
                    Info.m_ID, Info.m_szAccount, Info.m_szName,
                    Info.m_Authority, stAuth.c_str());
    }

    return true;
}

// END_ADMIN_MANAGER
bool CClientManager::__GetAdminConfig(uint32_t pAdminConfig[GM_MAX_NUM])
{
    memset(pAdminConfig, 1, sizeof(uint32_t) * GM_MAX_NUM);

    std::unique_ptr<SQLMsg> pMsg(CDBManager::instance().DirectQuery(
        "SELECT authority, general_allow+0 FROM gmconfig", SQL_COMMON));

    if (pMsg->Get()->uiNumRows == 0) {
        spdlog::error("__GetAdminConfig() ==> Admin config returned no "
                      "results");
        return false;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult))) {
        int col = 0;

        unsigned char bAuthority = GM_MAX_NUM;
        std::string stAuthority = row[col++];
        if (!stAuthority.compare("IMPLEMENTOR"))
            bAuthority = GM_IMPLEMENTOR;
        else if (!stAuthority.compare("GOD"))
            bAuthority = GM_GOD;
        else if (!stAuthority.compare("HIGH_WIZARD"))
            bAuthority = GM_HIGH_WIZARD;
        else if (!stAuthority.compare("LOW_WIZARD"))
            bAuthority = GM_LOW_WIZARD;
        else if (!stAuthority.compare("WIZARD"))
            bAuthority = GM_WIZARD;
        else if (!stAuthority.compare("PLAYER"))
            bAuthority = GM_PLAYER;
        else {
            spdlog::error("unkown authority %s", stAuthority.c_str());
            continue;
        }

        uint32_t dwGeneralAllow = 0;
        if (row[col])
            str_to_number(dwGeneralAllow, row[col]);
        col++;

        pAdminConfig[bAuthority] = dwGeneralAllow;
    }

    return true;
}

// END_ADMIN_MANAGER

void CClientManager::ReloadAdmin()
{
    std::vector<tAdminInfo> vAdmin;
    __GetAdminInfo(vAdmin);

    uint32_t adwAdminConfig[GM_MAX_NUM];
    __GetAdminConfig(adwAdminConfig);

    uint32_t dwPacketSize = sizeof(uint16_t) +
                            sizeof(tAdminInfo) * vAdmin.size() +
                            sizeof(uint32_t) * GM_MAX_NUM;

    for (auto peer : m_peerList) {
        if (!peer->GetChannel())
            continue;

        peer->EncodeHeader(HEADER_DG_RELOAD_ADMIN, 0, dwPacketSize);

        peer->EncodeWORD(vAdmin.size());

        for (auto& n : vAdmin)
            peer->Encode(&n, sizeof(tAdminInfo));

        peer->Encode(&adwAdminConfig[0], sizeof(adwAdminConfig));
    }

    SPDLOG_INFO("ReloadAdmin End");
}

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
bool CClientManager::__GetEventInfo(std::vector<TEventInfo>& rEventVec)
{
    // szIP == NULL ÀÏ°æ¿ì  ¸ðµç¼­¹ö¿¡ ¿î¿µÀÚ ±ÇÇÑÀ» °®´Â´Ù.
    char szQuery[512];
    snprintf(szQuery, sizeof(szQuery),
             "SELECT id, name, period+0, type+0, details, map_index, channel, "
             "start_time, end_time, is_notice FROM events");
    std::unique_ptr<SQLMsg> pMsg(
        CDBManager::instance().DirectQuery(szQuery, SQL_COMMON));

    if (pMsg->Get()->uiNumRows == 0) {
        spdlog::error("__GetEventInfo() ==> DirectQuery failed(%s)", szQuery);
        return false;
    }

    MYSQL_ROW row;
    rEventVec.reserve(pMsg->Get()->uiNumRows);

    while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult))) {
        int idx = 0;
        TEventInfo Info;
        // id, name, type+0, questname, map_index, eventflag, start_time,
        // end_time
        str_to_number(Info.dwID, row[idx++]);
        // trim_and_lower(row[idx++], Info.m_szAccount,
        // sizeof(Info.m_szAccount));
        strlcpy(Info.szName, row[idx++], sizeof(Info.szName));
        str_to_number(Info.bPeriod, row[idx++]);
        str_to_number(Info.bType, row[idx++]);
        strlcpy(Info.szDetails, row[idx++], sizeof(Info.szDetails));
        str_to_number(Info.lMapIndex, row[idx++]);
        str_to_number(Info.bChannel, row[idx++]);
        // strlcpy(Info.sSubType->szQuestFlagName, row[idx++],
        // sizeof(Info.sSubType->szQuestFlagName));
        strlcpy(Info.szStartTime, row[idx++], sizeof(Info.szStartTime));
        str_to_number(Info.dwEndTime, row[idx++]);
        str_to_number(Info.bIsNotice, row[idx]);

        rEventVec.push_back(Info);

        SPDLOG_INFO("EVENT: ID {} NAME {} PERIOD {} TYPE {} MAP_INDEX {} "
                    "START_TIME {} END_TIME {} \n{}\n",
                    Info.dwID, Info.szName, Info.bPeriod, Info.bType,
                    Info.lMapIndex, Info.szStartTime, Info.dwEndTime,
                    Info.szDetails);
    }

    return true;
}
#endif

// BREAK_MARRIAGE
void CClientManager::BreakMarriage(CPeer* peer, const char* data)
{
    uint32_t pid1, pid2;

    pid1 = *(int*)data;
    data += sizeof(int);

    pid2 = *(int*)data;
    data += sizeof(int);

    SPDLOG_INFO("Breaking off a marriage engagement! pid %d and pid %d", pid1,
                pid2);
    marriage::CManager::instance().Remove(pid1, pid2);
}

// END_BREAK_MARIIAGE

const TItemTable* CClientManager::GetItemTable(uint32_t dwVnum) const
{
    auto it = m_map_itemTableByVnum.find(dwVnum);
    if (it == m_map_itemTableByVnum.end())
        return nullptr;
    return &it->second;
}

void CClientManager::SendSpareItemIDRange(CPeer* peer)
{
    peer->SendSpareItemIDRange();
}

// delete gift notify icon
void CClientManager::DeleteAwardId(TPacketDeleteAwardID* data)
{
    // SPDLOG_INFO("data from game server arrived %d",data->dwID);
    std::map<uint32_t, TItemAward*>::iterator it;
    it = ItemAwardManager::Instance().GetMapAward().find(data->dwID);
    if (it != ItemAwardManager::Instance().GetMapAward().end()) {
        std::set<TItemAward*>& kSet =
            ItemAwardManager::Instance()
                .GetMapkSetAwardByLogin()[it->second->szLogin];
        if (kSet.erase(it->second))
            SPDLOG_INFO("erase ItemAward id: %d from cache", data->dwID);
        ItemAwardManager::Instance().GetMapAward().erase(data->dwID);
    } else {
        SPDLOG_INFO("DELETE_AWARDID : could not find the id: %d", data->dwID);
    }
}

void CClientManager::ChargeCash(const TRequestChargeCash* packet,
                                uint32_t handle, CPeer* peer)
{
    std::string query;

    if (packet->bAdd) {

        if (ERequestCharge_Cash == packet->eChargeType)
            query = fmt::format("update account set `cash` = `cash` + {} where "
                                "id = {} limit 1",
                                packet->iAmount, packet->dwAID);
        else if (ERequestCharge_Mileage == packet->eChargeType)
            query = fmt::format("update account set `mileage` = `mileage` + {} "
                                "where id = {} limit 1",
                                packet->iAmount, packet->dwAID);
        else {
            spdlog::error("Invalid request charge type (type : {}, amount : "
                          "{}, aid : {})",
                          packet->eChargeType, packet->iAmount, packet->dwAID);
            return;
        }
    } else {
        if (ERequestCharge_Cash == packet->eChargeType)
            query = fmt::format("update account set `cash` = `cash` - {} where "
                                "id = {} limit 1",
                                packet->iAmount, packet->dwAID);
        else if (ERequestCharge_Mileage == packet->eChargeType)
            query = fmt::format("update account set `mileage` = `mileage` - {} "
                                "where id = {} limit 1",
                                packet->iAmount, packet->dwAID);
        else {
            spdlog::error("Invalid request charge type (type : {}, amount : "
                          "{}, aid : {})",
                          packet->eChargeType, packet->iAmount, packet->dwAID);
            return;
        }
    }

    SPDLOG_DEBUG(query);

    spdlog::error("Request Charge (type : {}, amount : {}, aid : {})",
                  packet->eChargeType, packet->iAmount, packet->dwAID);

    std::unique_ptr<SQLMsg> a(
        CDBManager::Instance().DirectQuery(query, SQL_ACCOUNT));

    peer->EncodeHeader(HEADER_DG_UPDATE_CASH, handle, 0);
}

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
void CClientManager::AutoEvents(TPacketAutoEvents* p)
{
    ForwardPacket(HEADER_DG_AUTO_EVENT, p, sizeof(TPacketAutoEvents));
}
#endif

#ifdef __DUNGEON_FOR_GUILD__
void CClientManager::GuildDungeon(TPacketGDGuildDungeon* sPacket)
{
    CGuildManager::instance().Dungeon(sPacket->dwGuildID, sPacket->bChannel,
                                      sPacket->lMapIndex);
}

void CClientManager::GuildDungeonGD(TPacketGDGuildDungeonCD* sPacket)
{
    CGuildManager::instance().DungeonCooldown(sPacket->dwGuildID,
                                              sPacket->dwTime);
}
#endif

#ifdef ENABLE_GEM_SYSTEM
void CClientManager::GemLog(TPacketGemLog* p)
{
    CGemLog::instance().AddLog(p->type, p->gem);
}
#endif

void CClientManager::MessengerSetBlock(const PacketGDMessengerSetBlock* block)
{
    std::string query;

    if (block->mode)
        query = fmt::format("REPLACE INTO messenger_block VALUES({}, {}, {})",
                            block->pid, block->other_pid, block->mode);
    else
        query = fmt::format("DELETE FROM messenger_block WHERE pid = {} and "
                            "other_pid = {}",
                            block->pid, block->other_pid);

    CDBManager::instance().AsyncQuery(query);
}
