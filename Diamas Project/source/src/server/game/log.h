#ifndef METIN2_SERVER_GAME_LOG_H
#define METIN2_SERVER_GAME_LOG_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <mysql/AsyncSQL.h>
#include <game/Types.hpp>
#include <game/DbPackets.hpp>
#include <base/Singleton.hpp>

class CHARACTER;
class CItem;
class CGuild;

enum GOLDBAR_HOW
{
    PERSONAL_SHOP_BUY = 1,
    PERSONAL_SHOP_SELL = 2,
    SHOP_BUY = 3,
    SHOP_SELL = 4,
    EXCHANGE_TAKE = 5,
    EXCHANGE_GIVE = 6,
    QUEST = 7,
};

enum ExchangeType //Matches Enum in table
{
    EXCHANGE_TYPE_SHOP = 1,
    EXCHANGE_TYPE_TRADE = 2,
};

class LogManager : public singleton<LogManager>
{
private:
    enum EBanLogTypes
    {
        BAN_LOG_TYPE_CHAT = 1,
        BAN_LOG_TYPE_ACCOUNT = 2,
    };

public:
    LogManager();

    bool IsConnected();

    bool Connect(const char *host, const int port, const char *user, const char *pwd, const char *db);
    bool ConnectAsync(const char *host, const int port, const char *user, const char *pwd, const char *db);

    uint32_t ExchangeLog(int type, uint32_t dwPID1, uint32_t dwPID2, long x, long y, Gold goldA = 0, Gold goldB = 0);
    void ExchangeItemLog(uint32_t tradeID, CItem *item, const char *player);

    void ItemLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dwItemID, const char *c_pszText,
                 const char *c_pszHint, const char *c_pszIP, uint32_t dwVnum);
    void GuildStorageLog(CHARACTER *ch, CGuild *pGuild, CItem *pItem, const char *action, uint32_t page);
    void GuildStorageLog(uint32_t playerId, CGuild *pGuild, TPlayerItem pItem, const char *action, uint32_t page);
    void GuildStorageLog(CGuild *pGuild, CItem *pItem, const char *action);
    void ItemLog(CHARACTER *ch, CItem *item, const char *c_pszText, const char *c_pszHint);
    void ItemLog(CHARACTER *ch, int itemID, int itemVnum, const char *c_pszText, const char *c_pszHint);

    void CharLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dw, const char *c_pszText, const char *c_pszHint,
                 const std::string &c_pszIP);
    void CharLog(CHARACTER *ch, uint32_t dw, const char *c_pszText, const char *c_pszHint);
    void WhisperLog(uint32_t fromPID, uint32_t toPID, const char *message);
    void ChatLog(CHARACTER *ch, uint8_t type, const char *what);
    void LoginLog(bool isLogin, uint32_t dwAccountID, uint32_t dwPID, uint8_t bLevel, uint8_t bJob,
                  uint32_t dwPlayTime);
    void MoneyLog(uint8_t type, uint32_t vnum, Gold gold);
    void ItemshopLog(uint32_t pid, uint32_t vnum, uint32_t aid, Gold price,
                 CountType count);

#ifdef ENABLE_GEM_SYSTEM
	void		GemLog(uint8_t type, int gem);
#endif

    void HackLog(const char *c_pszHackName, CHARACTER *ch);
    void HackCRCLog(const char *c_pszHackName, const char *c_pszLogin, const char *c_pszName, const char *c_pszIP,
                    uint32_t dwCRC);
    void GoldBarLog(uint32_t dwPID, uint32_t dwItemID, GOLDBAR_HOW eHow, const char *c_pszHint);
    void CubeLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum, uint32_t item_uid, int item_count,
                 bool success);
    void GMCommandLog(uint32_t dwPID, const std::string &, const char *szIP, uint8_t byChannel, const char *szCommand);
    void SpeedHackLog(uint32_t pid, uint32_t x, uint32_t y, int hack_count);
    void ChangeNameLog(uint32_t pid, const char *old_name, const char *new_name, const char *ip);
    void RefineLog(uint32_t pid, const char *item_name, uint32_t item_id, int item_refine_level, int is_success,
                   const char *how);
    void LevelLog(CHARACTER *pChar, unsigned int level, unsigned int playhour);
    void BootLog(const char *c_pszHostName, uint8_t bChannel);
    void FishLog(uint32_t dwPID, int prob_idx, int fish_id, int fish_level, uint32_t dwMiliseconds,
                 uint32_t dwVnum = false, uint32_t dwValue = 0);
    void QuestRewardLog(const char *c_pszQuestName, uint32_t dwPID, uint32_t dwLevel, int iValue1, int iValue2);
    void DetailLoginLog(bool isLogin, CHARACTER *ch);
    void DragonSlayLog(uint32_t dwGuildID, uint32_t dwDragonVnum, uint32_t dwStartTime, uint32_t dwEndTime);
    void GuildEloLog(uint32_t dwGuild1, uint32_t dwGuild2, uint8_t obtainedElo, uint8_t lostElo, uint8_t gameMode);
#ifdef ENABLE_MELEY_LAIR_DUNGEON
    void MeleyLog(uint32_t dwGuildID, uint32_t dwPartecipants, uint32_t dwTime);
#endif
    void ItemDestroyLog(CHARACTER *ch, CItem *item);
#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
	void		AutoEventLog(uint32_t dwEventID, const char * c_pszEventName, uint8_t bChannel, long lMapIndex, uint32_t dwEventTimeLeft, const char * c_pszHint);
#endif
#ifdef __OFFLINE_SHOP_LOGGING__
    void OfflineShopLogAddItem(uint32_t dwOwnerPID, uint32_t dwVNum, uint8_t byCount, Gold llPrice);
    void OfflineShopLogRemoveItem(uint32_t dwOwnerPID, uint32_t dwVNum, uint8_t byCount);
    void OfflineShopLogBuyItem(uint32_t dwOwnerPID, uint32_t dwBuyerPID, uint32_t dwVNum, uint8_t byCount,
                               Gold llPrice);
    void OfflineShopLogWithdrawGold(uint32_t dwOwnerPID, Gold llWithrawedGold);
#endif

private:

    template <class ... Args> void Query(const char* c_pszFormat, Args ... arg);
    template <typename... Args>
    SQLMsg *DirectQuery(const char *c_pszFormat, const Args &... arg);

    CAsyncSQL m_sql;
    CAsyncSQL m_sql_direct;
    bool m_bIsConnect;
};

template <typename... Args>
inline void LogManager::Query(const char *c_pszFormat, Args... arg)
{
    m_sql.AsyncQuery(fmt::format(c_pszFormat, std::forward<Args>(arg)...));
}

template <typename ...Args>
inline SQLMsg *LogManager::DirectQuery(const char *c_pszFormat, const Args & ...arg)
{
    return m_sql_direct.DirectQuery(fmt::format(c_pszFormat, arg...));
}

#define g_pLogManager	singleton<LogManager>::InstancePtr()

#endif /* METIN2_SERVER_GAME_LOG_H */
