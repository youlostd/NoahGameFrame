#include "constants.h"
#include "config.h"
#include "log.h"

#include "char.h"
#include "desc.h"
#include "item.h"
#include "guild.h"
#include "db.h"

static char __escape_hint[1024];

LogManager::LogManager()
    : m_bIsConnect(false)
{
}

bool LogManager::ConnectAsync(const char *host, const int port, const char *user, const char *pwd, const char *db)
{
    if (m_sql.Setup(host, user, pwd, db, "utf8", false, port))
        m_bIsConnect = true;

    return m_bIsConnect;
}

bool LogManager::Connect(const char *host, const int port, const char *user, const char *pwd, const char *db)
{
    m_bIsConnect = m_sql_direct.Setup(host, user, pwd, db, "utf8", true, port);
    if (!m_bIsConnect)
        fprintf(stderr, "cannot open direct sql connection to host: %s user: %s db: %s\n", host, user, db);

    return m_bIsConnect;
}

bool LogManager::IsConnected() { return m_bIsConnect; }

uint32_t LogManager::ExchangeLog(int type, uint32_t dwPID1, uint32_t dwPID2, long x, long y, Gold goldA /*=0*/,
                                 Gold goldB /*=0*/)
{
    std::unique_ptr<SQLMsg> msg(DirectQuery(
        "INSERT INTO exchanges (type, playerA, playerB, goldA, goldB, x, y, date) VALUES ({}, {}, {}, {}, {}, {}, {}, NOW())",
        type, dwPID1, dwPID2, goldA, goldB, x, y));

    if (!msg || msg->Get()->uiAffectedRows == 0 || msg->Get()->uiAffectedRows == (my_ulonglong)-1)
    {
        SPDLOG_ERROR("An sql error occured while logging a trade.");
        return 0;
    }

    return (uint32_t)msg->Get()->uiInsertID;
}

void LogManager::ExchangeItemLog(uint32_t tradeID, CItem *item, const char *player)
{
    if (!tradeID)
    {
        SPDLOG_ERROR("Lost trade due to mysql error (tradeID = 0)");
        return;
    }

    Query("INSERT INTO exchange_items"
          "(trade_id, `toPlayer`, `item_id`, `vnum`, count, socket0, socket1, socket2, socket3, socket4, socket5,"
          " attrtype0, attrtype1, attrtype2, attrtype3, attrtype4, attrtype5, attrtype6,"
          " attrvalue0, attrvalue1, attrvalue2, attrvalue3, attrvalue4, attrvalue5, attrvalue6,"
          " date)"
          "VALUES ({}, '{}', {}, {}, {}, {}, {}, {}, {}, {}, {},"
          "{}, {}, {}, {}, {}, {}, {},"
          "{}, {}, {}, {}, {}, {}, {},"
          "NOW())"
          , tradeID, player, item->GetID(), item->GetVnum(), item->GetCount()
          , item->GetSocket(0), item->GetSocket(1), item->GetSocket(2), item->GetSocket(3), item->GetSocket(4),
          item->GetSocket(5)
          , item->GetAttributeType(0), item->GetAttributeType(1), item->GetAttributeType(2), item->GetAttributeType(3)
          , item->GetAttributeType(4), item->GetAttributeType(5), item->GetAttributeType(6), item->GetAttributeValue(0),
          item->GetAttributeValue(1)
          , item->GetAttributeValue(2), item->GetAttributeValue(3), item->GetAttributeValue(4),
          item->GetAttributeValue(5), item->GetAttributeValue(6)
    );
}

void LogManager::ItemLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dwItemID, const char *c_pszText,
                         const char *c_pszHint, const char *c_pszIP, uint32_t dwVnum)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHint, strlen(c_pszHint));
    Query(
        "INSERT INTO item_log (item_id, vnum, owner, x, y, action, info, ip) VALUES ({}, {}, {}, {}, {},'{}','{}','{}')",
        dwItemID, dwVnum, dwPID, x, y, c_pszText, __escape_hint, c_pszIP);
}

void LogManager::GuildStorageLog(CHARACTER *ch, CGuild *pGuild, CItem *pItem, const char *action, uint32_t page)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), action, strlen(action));

    Query("INSERT INTO guild_storage_log(guild_storage_log.guild, guild_storage_log.member, guild_storage_log.action,"
          " guild_storage_log.time, guild_storage_log.itemvnum, guild_storage_log.itemid,"
          " guild_storage_log.count, guild_storage_log.socket0, guild_storage_log.socket1,"
          " guild_storage_log.socket2, guild_storage_log.socket3, guild_storage_log.socket4,"
          " guild_storage_log.socket5, guild_storage_log.attrtype0, guild_storage_log.attrvalue0,"
          " guild_storage_log.attrtype1, guild_storage_log.attrvalue1, guild_storage_log.attrtype2,"
          " guild_storage_log.attrvalue2, guild_storage_log.attrtype3, guild_storage_log.attrvalue3,"
          " guild_storage_log.attrtype4, guild_storage_log.attrvalue4, guild_storage_log.attrtype5,"
          " guild_storage_log.attrvalue5, guild_storage_log.attrtype6, guild_storage_log.attrvalue6,"
          " guild_storage_log.site) VALUES ('{}', '{}', '{}',"
          " NOW(), '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}');",
          pGuild->GetID(), ch->GetPlayerID(), __escape_hint,
          pItem->GetVnum(), pItem->GetID(), pItem->GetCount(),
          pItem->GetSocket(0), pItem->GetSocket(1), pItem->GetSocket(2),
          pItem->GetSocket(3), pItem->GetSocket(4), pItem->GetSocket(5),
          pItem->GetAttributeType(0), pItem->GetAttributeValue(0), pItem->GetAttributeType(1),
          pItem->GetAttributeValue(1), pItem->GetAttributeType(2), pItem->GetAttributeValue(2),
          pItem->GetAttributeType(3), pItem->GetAttributeValue(3), pItem->GetAttributeType(4),
          pItem->GetAttributeValue(4), pItem->GetAttributeType(5), pItem->GetAttributeValue(5),
          pItem->GetAttributeType(6), pItem->GetAttributeValue(6), page);
}

void LogManager::GuildStorageLog(uint32_t playerId, CGuild *pGuild, TPlayerItem pItem, const char *action,
                                 uint32_t page)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), action, strlen(action));

    Query("INSERT INTO guild_storage_log(guild_storage_log.guild, guild_storage_log.member, guild_storage_log.action,"
          " guild_storage_log.time, guild_storage_log.itemvnum, guild_storage_log.itemid,"
          " guild_storage_log.count, guild_storage_log.socket0, guild_storage_log.socket1,"
          " guild_storage_log.socket2, guild_storage_log.socket3, guild_storage_log.socket4,"
          " guild_storage_log.socket5, guild_storage_log.attrtype0, guild_storage_log.attrvalue0,"
          " guild_storage_log.attrtype1, guild_storage_log.attrvalue1, guild_storage_log.attrtype2,"
          " guild_storage_log.attrvalue2, guild_storage_log.attrtype3, guild_storage_log.attrvalue3,"
          " guild_storage_log.attrtype4, guild_storage_log.attrvalue4, guild_storage_log.attrtype5,"
          " guild_storage_log.attrvalue5, guild_storage_log.attrtype6, guild_storage_log.attrvalue6,"
          " guild_storage_log.site) VALUES ('{}', '{}', '{}',"
          " NOW(), '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}', '{}', '{}',"
          " '{}');",
          pGuild->GetID(), playerId,
          __escape_hint, pItem.data.vnum, pItem.id,
          pItem.data.count, pItem.data.sockets[0], pItem.data.sockets[1],
          pItem.data.sockets[2], pItem.data.sockets[3], pItem.data.sockets[4],
          pItem.data.sockets[5], pItem.data.attrs[0].bType, pItem.data.attrs[0].sValue,
          pItem.data.attrs[1].bType, pItem.data.attrs[1].sValue, pItem.data.attrs[2].bType,
          pItem.data.attrs[2].sValue, pItem.data.attrs[3].bType, pItem.data.attrs[3].sValue,
          pItem.data.attrs[4].bType, pItem.data.attrs[4].sValue, pItem.data.attrs[5].bType,
          pItem.data.attrs[5].sValue, pItem.data.attrs[6].bType, pItem.data.attrs[6].sValue,
          page);
}

void LogManager::ItemLog(CHARACTER *ch, CItem *item, const char *c_pszText, const char *c_pszHint)
{
    if (nullptr == ch || nullptr == item) { return; }

    ItemLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), item->GetID(),
            nullptr == c_pszText ? "" : c_pszText,
            c_pszHint, ch->GetDesc() ? ch->GetDesc()->GetHostName().c_str() : "",
            item->GetOriginalVnum());
}

void LogManager::ItemLog(CHARACTER *ch, int itemID, int itemVnum, const char *c_pszText, const char *c_pszHint)
{
    ItemLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), itemID, c_pszText, c_pszHint,
            ch->GetDesc() ? ch->GetDesc()->GetHostName().c_str() : "", itemVnum);
}

void LogManager::CharLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dwValue, const char *c_pszText,
                         const char *c_pszHint, const std::string &c_pszIP)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHint, strlen(c_pszHint));

    Query(
        "INSERT INTO log (type, time, who, x, y, what, how, hint, ip) VALUES('CHARACTER', NOW(), {}, {}, {}, {}, '{}', '{}', '{}')",
        dwPID, x, y, dwValue, c_pszText, __escape_hint, c_pszIP);
}

void LogManager::CharLog(CHARACTER *ch, uint32_t dw, const char *c_pszText, const char *c_pszHint)
{
    if (ch)
        CharLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), dw, c_pszText, c_pszHint,
                ch->GetDesc() ? ch->GetDesc()->GetHostName().c_str() : "");
    else
        CharLog(0, 0, 0, dw, c_pszText, c_pszHint, "");
}

void LogManager::LoginLog(bool isLogin, uint32_t dwAccountID, uint32_t dwPID, uint8_t bLevel, uint8_t bJob,
                          uint32_t dwPlayTime)
{
    Query(
        "INSERT loginlog (type, time, channel, account_id, pid, level, job, playtime) VALUES ({}, NOW(), {}, {}, {}, {}, {}, {})",
        isLogin ? "'LOGIN'" : "'LOGOUT'", gConfig.channel, dwAccountID, dwPID, bLevel, bJob, dwPlayTime);
}

void LogManager::MoneyLog(uint8_t type, uint32_t vnum, Gold gold)
{
    if (type == MONEY_LOG_RESERVED || type >= MONEY_LOG_TYPE_MAX_NUM)
    {
        SPDLOG_ERROR("TYPE ERROR: type {} vnum {} gold {}", type, vnum, gold);
        return;
    }

    Query("INSERT money_log VALUES (NOW(), {}, {}, {})", type, vnum, gold);
}

void LogManager::ItemshopLog(uint32_t pid, uint32_t vnum, uint32_t aid, Gold price, CountType count)
{
    Query("INSERT cash_shop_log  (time, pid, aid, vnum, price, count)  VALUES (NOW(), {}, {}, {}, {}, {});", pid, aid, vnum, price, count);
}

#ifdef ENABLE_GEM_SYSTEM
void LogManager::GemLog(uint8_t type, int gem)
{

	Query("INSERT gem_log%s VALUES (NOW(), %d, %d)", get_table_postfix(), type, gem);
}
#endif

void LogManager::HackLog(const char *c_pszHackName, CHARACTER *ch)
{
    if (ch->GetDesc())
    {
        m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHackName, strlen(c_pszHackName));

        Query(
            "INSERT INTO hack_log (time, login, name, ip, server, why, hwid) VALUES(NOW(), '{}', '{}', '{}', '{}', '{}', '{}')",
            ch->GetDesc()->GetLogin().c_str(), ch->GetName(), ch->GetDesc()->GetHostName(), gConfig.hostname.c_str(),
            __escape_hint, ch->GetDesc()->GetHWIDHash().c_str());
    }
}

void LogManager::HackCRCLog(const char *c_pszHackName, const char *c_pszLogin, const char *c_pszName,
                            const char *c_pszIP, uint32_t dwCRC)
{
    Query(
        "INSERT INTO hack_crc_log (time, login, name, ip, server, why, crc) VALUES(NOW(), '%s', '%s', '%s', '%s', '%s', %u)",
        c_pszLogin, c_pszName, c_pszIP, gConfig.hostname.c_str(), c_pszHackName, dwCRC);
}

void LogManager::GoldBarLog(uint32_t dwPID, uint32_t dwItemID, GOLDBAR_HOW eHow, const char *c_pszHint)
{
    std::string how;

    switch (eHow)
    {
    case PERSONAL_SHOP_BUY:
        how = ("'BUY'");
        break;

    case PERSONAL_SHOP_SELL:
        how = ("'SELL'");
        break;

    case SHOP_BUY:
        how = ("'SHOP_BUY'");
        break;

    case SHOP_SELL:
        how = ("'SHOP_SELL'");
        break;

    case EXCHANGE_TAKE:
        how = ("'EXCHANGE_TAKE'");
        break;

    case EXCHANGE_GIVE:
        how = ("'EXCHANGE_GIVE'");
        break;

    case QUEST:
        how = ("'QUEST'");
        break;

    default:
        how = ("''");
        break;
    }

    Query("INSERT INTO goldlog (date, time, pid, what, how, hint) VALUES(CURDATE(), CURTIME(), {}, {}, {}, '{}')",
          dwPID, dwItemID, how.c_str(), c_pszHint);
}

void LogManager::CubeLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum, uint32_t item_uid, int item_count,
                         bool success)
{
    Query("INSERT INTO cube (pid, time, x, y, item_vnum, item_uid, item_count, success) "
          "VALUES({}, NOW(), {}, {}, {}, {}, {}, {})",
          dwPID, x, y, item_vnum, item_uid, item_count, success ? 1 : 0);
}

void LogManager::WhisperLog(uint32_t fromPID, uint32_t toPID, const char *message)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), message, strlen(message));
    // Message mysql_real_escape_string Output = __escape_hint;
    Query("INSERT INTO whisper_log (`time`, `from`, `to`, `message`) VALUES(NOW(), {}, {}, '{}');", fromPID, toPID,
          __escape_hint);
}

void LogManager::ChatLog(CHARACTER *ch, uint8_t type, const char *what)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), what, strlen(what));

    Query(
        "INSERT INTO chat_log (`pid`, `channel`, `map_index`, `x`, `y`, `type`, `what`, `when`) VALUES({}, {}, {}, {}, {}, {}, '{}', NOW())",
        ch->GetPlayerID(), gConfig.channel, ch->GetMapIndex(), ch->GetX(), ch->GetY(), type, __escape_hint);
}

void LogManager::SpeedHackLog(uint32_t pid, uint32_t x, uint32_t y, int hack_count)
{
    Query("INSERT INTO speed_hack (pid, time, x, y, hack_count) "
          "VALUES({}, NOW(), {}, {}, {})",
          pid, x, y, hack_count);
}

void LogManager::ChangeNameLog(uint32_t pid, const char *old_name, const char *new_name, const char *ip)
{
    Query("INSERT INTO change_name (pid, old_name, new_name, time, ip) "
          "VALUES({}, '{}', '{}', NOW(), '{}') ",
          pid, old_name, new_name, ip);
}

void LogManager::GMCommandLog(uint32_t dwPID, const std::string &szName, const char *szIP, uint8_t byChannel,
                              const char *szCommand)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), szCommand, strlen(szCommand));

    Query("INSERT INTO command_log (userid, server, ip, port, username, command, date ) "
          "VALUES({}, 999, '{}', {}, '{}', '{}', NOW()) ",
          dwPID, szIP, byChannel, szName, __escape_hint);
}

void LogManager::RefineLog(uint32_t pid, const char *item_name, uint32_t item_id, int item_refine_level, int is_success,
                           const char *how)
{
    m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), item_name, strlen(item_name));

    Query(
        "INSERT INTO refinelog (pid, item_name, item_id, step, time, is_success, setType) VALUES({}, '{}', {}, {}, NOW(), {}, '{}')",
        pid, __escape_hint, item_id, item_refine_level, is_success, how);
}

void LogManager::LevelLog(CHARACTER *pChar, unsigned int level, unsigned int playhour)
{
    uint32_t aid = 0;

    if (nullptr != pChar->GetDesc()) { aid = pChar->GetDesc()->GetAccountTable().id; }

    Query("REPLACE INTO levellog (name, level, time, account_id, pid, playtime) VALUES('{}', {}, NOW(), {}, {}, {})",
          pChar->GetName(), level, aid, pChar->GetPlayerID(), playhour);
}

void LogManager::BootLog(const char *c_pszHostName, uint8_t bChannel)
{
    Query("INSERT INTO bootlog (time, hostname, channel) VALUES(NOW(), '{}', {})",
          c_pszHostName, bChannel);
}

void LogManager::FishLog(uint32_t dwPID, int prob_idx, int fish_id, int fish_level, uint32_t dwMiliseconds,
                         uint32_t dwVnum, uint32_t dwValue)
{
    Query("INSERT INTO fish_log VALUES(NOW(), {}, {}, {}, {}, {}, {}, {})",
          dwPID,
          prob_idx,
          fish_id,
          fish_level,
          dwMiliseconds,
          dwVnum,
          dwValue);
}

void LogManager::QuestRewardLog(const char *c_pszQuestName, uint32_t dwPID, uint32_t dwLevel, int iValue1, int iValue2)
{
    Query("INSERT INTO quest_reward_log VALUES('{}',{},{},2,{},{},NOW())",
          c_pszQuestName,
          dwPID,
          dwLevel,
          iValue1,
          iValue2);
}

void LogManager::DetailLoginLog(bool isLogin, CHARACTER *ch)
{
    if (nullptr == ch->GetDesc())
        return;

    if (true == isLogin)
    {
        Query("INSERT INTO loginlog2(type, is_gm, login_time, channel, account_id, pid, ip, hwid) "
              "VALUES('INVALID', {}, NOW(), {}, {}, {}, inet_aton('{}'), '{}')",
              ch->IsGM() ? "'Y'" : "'N'",
              gConfig.channel,
              ch->GetDesc()->GetAccountTable().id,
              ch->GetPlayerID(),
              ch->GetDesc()->GetHostName(),
              ch->GetDesc()->GetHWIDHash().c_str());
    }
    else
    {
        Query(
            "UPDATE loginlog2 SET type='VALID', logout_time=NOW(), playtime=TIMEDIFF(logout_time,login_time) WHERE account_id={} AND pid={} ORDER BY id DESC LIMIT 1",
            ch->GetDesc()->GetAccountTable().id, ch->GetPlayerID());
    }
}

void LogManager::DragonSlayLog(uint32_t dwGuildID, uint32_t dwDragonVnum, uint32_t dwStartTime, uint32_t dwEndTime)
{
    Query("INSERT INTO dragon_slay_log VALUES( {}, {}, FROM_UNIXTIME({}), FROM_UNIXTIME({}) )",
          dwGuildID, dwDragonVnum, dwStartTime, dwEndTime);
}

void LogManager::GuildEloLog(uint32_t dwGuild1, uint32_t dwGuild2, uint8_t obtainedElo, uint8_t lostElo,
                             uint8_t gameMode)
{
    Query("INSERT INTO guildwar_log (winner, loser, obtained_elo, lost_elo, gamemode) VALUES({}, {}, {}, {}, {})",
          dwGuild1,
          dwGuild2,
          obtainedElo,
          lostElo,
          gameMode
    );
}

#ifdef ENABLE_MELEY_LAIR_DUNGEON
void LogManager::MeleyLog(uint32_t dwGuildID, uint32_t dwPartecipants, uint32_t dwTime)
{
    std::unique_ptr<SQLMsg> pMsg(
        DBManager::Instance().DirectQuery("SELECT partecipants, time FROM log.meley_dungeon WHERE guild_id={};",
                                          dwGuildID));
    if (pMsg->Get()->uiNumRows == 0)
        Query("INSERT INTO meley_dungeon (guild_id, partecipants, time, date) VALUES({}, {}, {}, NOW())", dwGuildID,
              dwPartecipants, dwTime);
    else
    {
        uint32_t dwPartecipantsR = 0, dwTimeR = 0;
        MYSQL_ROW mRow;
        while (nullptr != (mRow = mysql_fetch_row(pMsg->Get()->pSQLResult)))
        {
            int iCur = 0;
            str_to_number(dwPartecipantsR, mRow[iCur++]);
            str_to_number(dwTimeR, mRow[iCur]);
        }

        if (((dwTimeR == dwTime) && (dwPartecipantsR < dwPartecipants)) || dwTimeR > dwTime)
            Query("UPDATE meley_dungeon SET partecipants={}, time={}, date=NOW() WHERE guild_id={};", dwPartecipants,
                  dwTime, dwGuildID);
    }
}
#endif

void LogManager::ItemDestroyLog(CHARACTER *ch, CItem *item)
{
    Query("INSERT INTO destroy_log"
          "(`owner`,`old_item_id`, `vnum`, count, socket0, socket1, socket2,"
          " attrtype0, attrtype1, attrtype2, attrtype3, attrtype4, attrtype5, attrtype6,"
          " attrvalue0, attrvalue1, attrvalue2, attrvalue3, attrvalue4, attrvalue5, attrvalue6,"
          " date)"
          "VALUES ("
          "{}, {},{},{},"
          "{},{},{},"
          "{},{},{},{},"
          "{},{},{},{},{},"
          "{},{},{},{},{},"
          "NOW())"
          , item->GetLastOwnerPID(), item->GetID(), item->GetVnum(), item->GetCount()
          , item->GetSocket(0), item->GetSocket(1), item->GetSocket(2)
          , item->GetAttributeType(0), item->GetAttributeType(1), item->GetAttributeType(2), item->GetAttributeType(3)
          , item->GetAttributeType(4), item->GetAttributeType(5), item->GetAttributeType(6), item->GetAttributeValue(0),
          item->GetAttributeValue(1)
          , item->GetAttributeValue(2), item->GetAttributeValue(3), item->GetAttributeValue(4),
          item->GetAttributeValue(5), item->GetAttributeValue(6)
    );
}

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
void LogManager::AutoEventLog(uint32_t dwEventID, const char * c_pszEventName, uint8_t bChannel, long lMapIndex, uint32_t dwEventTimeLeft, const char * c_pszHint)
{
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHint, strlen(c_pszHint));

	Query("INSERT INTO events_log%s (time, id, name, channel, map_index, remain_time, hint) VALUES(NOW(), %u, '%s', %d, %ld, %u, '%s')", get_table_postfix(), dwEventID, c_pszEventName, bChannel, lMapIndex, dwEventTimeLeft, __escape_hint);

}
#endif

#ifdef __OFFLINE_SHOP_LOGGING__
enum EOfflineShopLogTypes
{
    OFFLINE_SHOP_LOG_TYPE_ADD_ITEM,
    OFFLINE_SHOP_LOG_TYPE_REMOVE_ITEM,
    OFFLINE_SHOP_LOG_TYPE_BUY_ITEM,
    OFFLINE_SHOP_LOG_TYPE_WITHDRAW_GOLD,
    OFFLINE_SHOP_LOG_TYPE_OTHER
};

void LogManager::OfflineShopLogAddItem(uint32_t dwOwnerPID, uint32_t dwVNum, uint8_t byCount, Gold llPrice)
{
    Query("INSERT INTO offline_shop (type, owner_pid, vnum, count, gold) VALUES({}, {}, {}, {}, {})",
          OFFLINE_SHOP_LOG_TYPE_ADD_ITEM + 1,
          dwOwnerPID,
          dwVNum,
          byCount,
          llPrice);
}

void LogManager::OfflineShopLogRemoveItem(uint32_t dwOwnerPID, uint32_t dwVNum, uint8_t byCount)
{
    Query("INSERT INTO offline_shop (type, owner_pid, vnum, count) VALUES({}, {}, {}, {})",
          OFFLINE_SHOP_LOG_TYPE_REMOVE_ITEM + 1,
          dwOwnerPID,
          dwVNum,
          byCount);
}

void LogManager::OfflineShopLogBuyItem(uint32_t dwOwnerPID, uint32_t dwBuyerPID, uint32_t dwVNum, uint8_t byCount,
                                       Gold llPrice)
{
    Query("INSERT INTO offline_shop (type, owner_pid, buyer_pid, vnum, count, gold) VALUES({}, {}, {}, {}, {}, {})",
          OFFLINE_SHOP_LOG_TYPE_BUY_ITEM + 1,
          dwOwnerPID,
          dwBuyerPID,
          dwVNum,
          byCount,
          llPrice);
}

void LogManager::OfflineShopLogWithdrawGold(uint32_t dwOwnerPID, Gold llWithrawedGold)
{
    Query("INSERT INTO offline_shop (type, owner_pid, gold) VALUES({}, {}, {})",
          OFFLINE_SHOP_LOG_TYPE_WITHDRAW_GOLD + 1,
          dwOwnerPID,
          llWithrawedGold);
}
#endif
