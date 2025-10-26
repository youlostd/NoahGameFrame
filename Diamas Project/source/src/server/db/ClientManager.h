#ifndef METIN2_SERVER_DB_CLIENTMANAGER_H
#define METIN2_SERVER_DB_CLIENTMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#include <unordered_map>
#include <unordered_map>
#include <map>

#include <game/Types.hpp>
#include <game/BuildingTypes.hpp>

#include "Peer.h"
#include "DBManager.h"
#include "PlayerLoader.hpp"
#include "Cache.h"


#include <net/Acceptor.hpp>
#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/chrono/system_clocks.hpp>

#include "game/MobTypes.hpp"

#include <game/DungeonInfoTypes.hpp>

const char* __GetWarType(int type);


struct Config;
class PlayerLoader;

class CClientManager : public singleton<CClientManager>
{
	friend class CPeer;

public:
	typedef std::list<std::shared_ptr<CPeer>> TPeerList;


	// MYSHOP_PRICE_LIST
	/// 아이템 가격정보 리스트 요청 정보
	/**
	 * first: Peer handle
	 * second: request packet
	 */
	typedef std::pair<uint32_t, uint32_t> TItemPricelistReqInfo;

	// END_OF_MYSHOP_PRICE_LIST

	struct HandleInfo
	{
		uint32_t handle;
	};

	class ClientHandleInfo
	{
	public:
		uint32_t dwHandle = 0;
		uint32_t account_id = 0;
		uint32_t player_id = 0;
		uint8_t account_index = 0;
		char login[LOGIN_MAX_LEN + 1]{};
		char safebox_password[SAFEBOX_PASSWORD_MAX_LEN + 1]{};
		char ip[MAX_HOST_LENGTH + 1];

		TAccountTable* pAccountTable;
		TSafeboxTable* pSafebox;

		ClientHandleInfo(uint32_t argHandle, uint32_t dwPID = 0)
		{
			dwHandle = argHandle;
			pSafebox = nullptr;
			pAccountTable = nullptr;
			player_id = dwPID;
		};
		//µ¶ÀÏ¼±¹°±â´É¿ë »ý¼ºÀÚ
		ClientHandleInfo(uint32_t argHandle, uint32_t dwPID, uint32_t accountId)
		{
			dwHandle = argHandle;
			pSafebox = nullptr;
			pAccountTable = nullptr;
			player_id = dwPID;
			account_id = accountId;
		};

		~ClientHandleInfo()
		{
			if(pSafebox) {
				delete pSafebox;
				pSafebox = nullptr;
			}
		}
	};

public:
	CClientManager(asio::io_service& ioService,
	               const Config& config);
	~CClientManager();

	bool Initialize();
	time_t GetCurrentTime();
	CPeer* GetPeer(uint32_t handle);
	bool HasPeer(CPeer* peer);

	void Quit();

	int GetPlayerDeleteLevelLimit()
	{
		return m_iPlayerDeleteLevelLimit;
	}

	void SetChinaEventServer(bool flag)
	{
		m_bChinaEventServer = flag;
	}

	bool IsChinaEventServer()
	{
		return m_bChinaEventServer;
	}


	void SendAllGuildSkillRechargePacket();


	void SendGuildSkillUsable(uint32_t guild_id, uint32_t dwSkillVnum, bool bUsable);

	template <class Func>
	void for_each_peer(Func&& f);

	CPeer* GetAnyPeer();

	void ForwardPacket(uint8_t header, const void* data, int size, uint8_t bChannel = 0, CPeer* except = nullptr);

	void SendNotice(const char* c_pszFormat, ...);

	void ProcessPacket(CPeer* peer, uint8_t header, uint32_t dwHandle,
                       const char* data, uint32_t dwLength);

private:
	bool LoadItemNames(const std::string& filename);
	bool InitializeTables();
	bool MirrorItemTableIntoDb();
	bool InitializeItemTable();
	bool InitializeItemTableFile();
	bool InitializeBanwordTable();
	bool InitializeLandTable();
	bool InitializeObjectTable();

	void StartDbTimer();
	void StartUpdateTimer();
	void StartHourlyTimer();
	void StartDungeonRankingTimer();

	void AddPeer(asio::ip::tcp::socket socket);
	void RemovePeer(CPeer* pPeer);

	int AnalyzeQueryResult(SQLMsg* msg);

	void GuildCreate(CPeer* peer, uint32_t dwGuildID);
	void GuildSkillUpdate(CPeer* peer, TPacketGuildSkillUpdate* p);
	void GuildExpUpdate(CPeer* peer, TPacketGuildExpUpdate* p);
	void GuildAddMember(CPeer* peer, TPacketGDGuildAddMember* p);
	void GuildChangeGrade(CPeer* peer, TPacketGuild* p);
	void GuildRemoveMember(CPeer* peer, TPacketGuild* p);
	void GuildChangeMemberData(CPeer* peer, TPacketGuildChangeMemberData* p);
	void GuildDisband(CPeer* peer, TPacketGuild* p);
	void GuildWar(CPeer* peer, TPacketGuildWar* p);
	void GuildWarScore(CPeer* peer, TPacketGuildWarScore* p);
	void GuildChangeLadderPoint(TPacketGuildLadderPoint* p);
	void GuildUseSkill(TPacketGuildUseSkill* p);
	void GuildDepositMoney(TPacketGDGuildMoney* p);
	void GuildWithdrawMoney(CPeer* peer, TPacketGDGuildMoney* p);
	void GuildWithdrawMoneyGiveReply(TPacketGDGuildMoneyWithdrawGiveReply* p);
	void UpdateObject(TPacketGDUpdateObject* p);
	void GuildChangeMaster(TPacketChangeGuildMaster* p);

#ifdef ENABLE_GUILD_STORAGE
	void GuildAddItem(TPacketGuildStorageAddItem* p, CPeer* peer);
	void GuildRemoveItem(TPacketGuildStorageRemoveItemRequest* p, CPeer* peer);
	void GuildAddItemRemoveResponse(TPacketGuildStorageAddItem* p, CPeer* peer);
	void GuildMoveItem(TPacketGuildStorageMoveItem* p, CPeer* peer);
	void GuildLoadItems(TPacketRequestGuildItems* p, CPeer* peer);
#endif

	void QUERY_BOOT(CPeer* peer, TPacketGDBoot* p);

	void QUERY_PLAYER_LOAD(CPeer* peer, uint32_t dwHandle, TPlayerLoadPacket*);
	void RESULT_COMPOSITE_PLAYER(CPeer* peer, SQLMsg* pMsg, uint32_t dwQID);
	void RESULT_PLAYER_LOAD(CPeer* peer, MYSQL_RES* pRes, PlayerLoader* loader);
	void RESULT_SKILL_LOAD(CPeer* peer, MYSQL_RES* res, PlayerLoader* loader);
	void RESULT_QUICKSLOT_LOAD(CPeer* peer, MYSQL_RES* res, PlayerLoader* loader);
	void RESULT_ITEM_LOAD(CPeer* peer, MYSQL_RES* pRes, PlayerLoader* loader);
	void HandleResultSwitchbotDataLoad(CPeer* peer, MYSQL_RES* pRes, PlayerLoader* loader);
	void RESULT_QUEST_LOAD(CPeer* peer, MYSQL_RES* pRes, PlayerLoader* loader);
	void RESULT_MESSENGER_BLOCK(CPeer* peer, MYSQL_RES* res, PlayerLoader* loader);
	void RESULT_AFFECT_LOAD(CPeer* peer, MYSQL_RES* pRes, PlayerLoader* loader);
	void RESULT_TITLE_LOAD(CPeer* peer, MYSQL_RES* pRes, PlayerLoader* loader);
#ifdef ENABLE_BATTLE_PASS
	void RESULT_BATTLE_PASS_LOAD(CPeer* peer, MYSQL_RES* pRes, PlayerLoader* loader);
	void QUERY_SAVE_BATTLE_PASS(CPeer* peer, uint32_t dwHandle, TPlayerBattlePassMission* battlePass);
	void QUERY_REGISTER_RANKING(CPeer* peer, uint32_t dwHandle, TBattlePassRegisterRanking* pRanking);
    void HandleResultHuntingMissionLoad(CPeer *peer, MYSQL_RES *pRes, PlayerLoader *loader);
    void QuerySaveHuntingMissions(CPeer *peer, uint32_t dwHandle, TPlayerHuntingMission *mission);

    bool LoadBattlePassRanking();
	void RequestLoadBattlePassRanking(CPeer* peer, uint32_t dwHandle, const char* data);
#endif
	/// °¡°ÝÁ¤º¸ ·Îµå Äõ¸®¿¡ ´ëÇÑ Result Ã³¸®
	/**
	 * @param	peer °¡°ÝÁ¤º¸¸¦ ¿äÃ»ÇÑ Game server ÀÇ peer °´Ã¼ Æ÷ÀÎÅÍ
	 * @param	pMsg Äõ¸®ÀÇ Result ·Î ¹ÞÀº °´Ã¼ÀÇ Æ÷ÀÎÅÍ
	 *
	 * ·ÎµåµÈ °¡°ÝÁ¤º¸ ¸®½ºÆ®¸¦ Ä³½Ã¿¡ ÀúÀåÇÏ°í peer ¿¡°Ô ¸®½ºÆ®¸¦ º¸³»ÁØ´Ù.
	 */
	void RESULT_PRICELIST_LOAD(CPeer* peer, SQLMsg* pMsg);

	void QUERY_PLAYER_SAVE(CPeer* peer, uint32_t dwHandle, TPlayerTable*);

	void __QUERY_PLAYER_CREATE(CPeer* peer, uint32_t dwHandle, TPlayerCreatePacket*);
	void __QUERY_PLAYER_DELETE(CPeer* peer, uint32_t handle, const GdPlayerDeletePacket& p);
	void QUERY_ITEM_FLUSH(CPeer* pkPeer, const char* c_pData);
	void HandleQueryAddSwitchbotData(CPeer* peer, TPacketGDAddSwitchbotData* p);
	void HandleQueryRemoveSwitchbotData(CPeer* peer, TPacketGDRemoveSwitchbotData* p);

	void QUERY_ITEM_SAVE(CPeer* pkPeer, const char* c_pData);
	void QUERY_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData);


	void QUERY_QUEST_SAVE(CPeer* pkPeer, TQuestTable*, uint32_t dwLen);
	void QUERY_ADD_AFFECT(CPeer* pkPeer, TPacketGDAddAffect* p);
	void QUERY_REMOVE_AFFECT(CPeer* pkPeer, TPacketGDRemoveAffect* p);

	void QUERY_SAFEBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle, TSafeboxLoadPacket*, bool bMall);
	void QUERY_SAFEBOX_SAVE(CPeer* pkPeer, TSafeboxTable* pTable);
	void QUERY_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, uint32_t dwHandle, TSafeboxChangeSizePacket* p);
	void QUERY_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, uint32_t dwHandle, TSafeboxChangePasswordPacket* p);

	void RESULT_SAFEBOX_LOAD(CPeer* pkPeer, SQLMsg* msg);
	void RESULT_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg);
	void RESULT_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, SQLMsg* msg);
	void RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(CPeer* pkPeer, SQLMsg* msg);

	void QUERY_SETUP(CPeer* pkPeer, uint32_t dwHandle, const char* c_pData);

	void SendPartyOnSetup(CPeer* peer);

	void QUERY_HIGHSCORE_REGISTER(CPeer* peer, TPacketGDHighscore* data);
	void RESULT_HIGHSCORE_REGISTER(CPeer* pkPeer, SQLMsg* msg);

	void QUERY_FLUSH_CACHE(CPeer* pkPeer, const char* c_pData);

	void QUERY_PARTY_CREATE(CPeer* peer, TPacketPartyCreate* p);
	void QUERY_PARTY_DELETE(CPeer* peer, TPacketPartyDelete* p);
	void QUERY_PARTY_ADD(CPeer* peer, TPacketPartyAdd* p);
	void QUERY_PARTY_REMOVE(CPeer* peer, TPacketPartyRemove* p);
	void QUERY_PARTY_STATE_CHANGE(CPeer* peer, TPacketPartyStateChange* p);
	void QUERY_PARTY_SET_MEMBER_LEVEL(CPeer* peer, TPacketPartySetMemberLevel* p);

	void QUERY_RELOAD_PROTO();

	void AddGuildPriv(TPacketGiveGuildPriv* p);
	void AddEmpirePriv(TPacketGiveEmpirePriv* p);
	void AddCharacterPriv(TPacketGiveCharacterPriv* p);

	void MoneyLog(TPacketMoneyLog* p);

#ifdef ENABLE_GEM_SYSTEM
	void		GemLog(TPacketGemLog* p);
#endif


	void ChargeCash(const TRequestChargeCash* p, uint32_t handle, CPeer* peer);


	void LoadEventFlag();
private:
	void SetEventFlag(TPacketSetEventFlag* p);
	void SendEventFlagsOnSetup(CPeer* peer);

	// °áÈ¥
	void MarriageAdd(TPacketMarriageAdd* p);
	void MarriageUpdate(TPacketMarriageUpdate* p);
	void MarriageRemove(TPacketMarriageRemove* p);

	void WeddingRequest(TPacketWeddingRequest* p);
	void WeddingReady(TPacketWeddingReady* p);
	void WeddingEnd(TPacketWeddingEnd* p);


	// MYSHOP_PRICE_LIST
	// °³ÀÎ»óÁ¡ °¡°ÝÁ¤º¸

	/// ¾ÆÀÌÅÛ °¡°ÝÁ¤º¸ ¸®½ºÆ® ¾÷µ¥ÀÌÆ® ÆÐÅ¶(HEADER_GD_MYSHOP_PRICELIST_UPDATE) Ã³¸®ÇÔ¼ö
	/**
	 * @param [in]	pPacket ÆÐÅ¶ µ¥ÀÌÅÍÀÇ Æ÷ÀÎÅÍ
	 */
	void MyshopPricelistUpdate(const MyShopPriceListHeader* pPacket);

	/// ¾ÆÀÌÅÛ °¡°ÝÁ¤º¸ ¸®½ºÆ® ¿äÃ» ÆÐÅ¶(HEADER_GD_MYSHOP_PRICELIST_REQ) Ã³¸®ÇÔ¼ö
	/**
	 * @param	peer ÆÐÅ¶À» º¸³½ Game server ÀÇ peer °´Ã¼ÀÇ Æ÷ÀÎÅÍ
	 * @param [in]	dwHandle °¡°ÝÁ¤º¸¸¦ ¿äÃ»ÇÑ peer ÀÇ ÇÚµé
	 * @param [in]	dwPlayerID °¡°ÝÁ¤º¸ ¸®½ºÆ®¸¦ ¿äÃ»ÇÑ ÇÃ·¹ÀÌ¾îÀÇ ID
	 */
	void MyshopPricelistRequest(CPeer* peer, uint32_t dwHandle, const uint32_t* p);
	// END_OF_MYSHOP_PRICE_LIST

	// Building
	void CreateObject(TPacketGDCreateObject* p);
	void DeleteObject(uint32_t dwID);
	void UpdateLand(uint32_t* pdw);


	// BLOCK_CHAT
	void BlockChat(TPacketBlockChat* p);
	void HandleAuthSetPinQuery(CPeer* peer, uint32_t handle, const GdAuthSetPinQueryPacket& p);
	// END_OF_BLOCK_CHAT
	void HandleAuthLoginQuery(CPeer* peer, uint32_t handle,
                              const GdAuthLoginQueryPacket& p);

    void CheckHwidBan(const std::string& login, const std::string& hwid,
                      uint32_t peerHandle);

    void HandleAuthRegisterQuery(CPeer* peer, uint32_t handle, const GdAuthRegisterQueryPacket& data);

	void HandleHGuardInfoQuery(CPeer* peer, uint32_t handle, const GdHGuardInfoQueryPacket& p);
	void HandleHwidInfoQuery(CPeer* peer, uint32_t dwHandle, const GdHwidInfoQueryPacket& data);

	void HandleHGuardVerifyQuery(CPeer* peer, uint32_t handle, const GdHGuardValidationQueryPacket& data);

	void HandleAuthLoginResult(CPeer* peer, SQLMsg* msg);
    void HandleAuthRegisterNameCheckResult(CPeer* peer, SQLMsg* msg);
    void HandleAuthRegister(CPeer* peer, SQLMsg* msg);
    void HandleHGuardInfoResult(CPeer* peer, SQLMsg* msg);
	void HandleHGuardVerifyResult(CPeer* peer, SQLMsg* msg);

	
	void HandlePlayerListQuery(CPeer* peer, uint32_t handle,
	                           const GdPlayerListQueryPacket& p);
	void HandlePlayerListResult(CPeer* peer, SQLMsg* msg);

	void HandlePlayerDisconnect(CPeer* peer, const GdPlayerDisconnectPacket& p);
	void HandleChangeName(CPeer* peer, uint32_t handle, GdChangeNamePacket* p);
	void HandleChangeEmpire(CPeer* peer, uint32_t handle, GdChangeEmpirePacket* p);

public:
	/// Forgets the cache entry of |id| if possible.
	void RemoveCachedItem(uint32_t id);

private:
	asio::io_service& m_ioService;
	const Config& m_config;

	Acceptor m_acceptor;
	uint32_t m_nextPeerHandle;
	TPeerList m_peerList;

	CPeer* m_pkAuthPeer;

	asio::basic_waitable_timer<chrono::steady_clock> m_dbProcessTimer;
	asio::basic_waitable_timer<chrono::steady_clock> m_updateTimer;
	asio::basic_waitable_timer<chrono::steady_clock> m_hourlyTimer;
	asio::basic_waitable_timer<chrono::steady_clock> m_dungeonRankingTimer;

	int m_iPlayerDeleteLevelLimit;
	int m_iPlayerDeleteLevelLimitLower;
	bool m_bChinaEventServer;

	std::vector<TMobTable> m_vec_mobTable;
	std::vector<TItemTable> m_vec_itemTable;
	std::map<uint32_t, TItemTable> m_map_itemTableByVnum;
	std::unordered_map<uint32_t, std::string> m_itemNames;

	std::vector<TBanwordTable> m_vec_banwordTable;

	std::vector<TLand> m_vec_kLandTable;
	std::map<uint32_t, std::unique_ptr<TObject>> m_map_pkObjectTable;
#ifdef ENABLE_BATTLE_PASS
	std::vector<TBattlePassRanking *> m_vec_battlePassRanking;
#endif

public:
	CacheManager<CachedPlayer>& GetPlayerCache()
	{
		return m_playerCache;
	}

private:
	CacheManager<CachedPlayer> m_playerCache;
	CacheManager<CachedItem> m_itemCache;

	//CacheManager<CachedPriceList> m_priceListCache;


	struct TPartyInfo
	{
		uint8_t bRole;
		uint8_t bLevel;

		TPartyInfo() : bRole(0), bLevel(0)
		{
		}
	};

	typedef std::map<uint32_t, TPartyInfo> TPartyMember;
	typedef std::map<uint32_t, TPartyMember> TPartyMap;
	typedef std::map<uint8_t, TPartyMap> TPartyChannelMap;
	TPartyMap m_map_pkPartyMap;

	typedef std::map<std::string, long> TEventFlagMap;
	TEventFlagMap m_map_lEventFlag;

	using THwidDataPair = std::pair<std::string, long>;
	using THWidFlagMap = std::multimap<std::string, THwidDataPair>;
	THWidFlagMap m_map_lHwidFlag;

	int m_iCacheFlushCount;
	int m_iCacheFlushCountLimit;

private:
	ItemIdRange m_itemRange{};

public:
	bool InitializeNowItemID();
	uint32_t AllocateItemId();


	//BOOT_LOCALIZATION
	/* ·ÎÄÃ Á¤º¸ ÃÊ±âÈ­ 
	 **/
	bool InitializeLocalization();
	const TItemTable* GetItemTable(uint32_t dwVnum) const;

private:
	std::vector<tLocale> m_vec_Locale;
	//END_BOOT_LOCALIZATION
	//ADMIN_MANAGER

	//ADMIN_MANAGER
	bool __GetAdminInfo(std::vector<tAdminInfo>& rAdminVec);
#ifdef __ADMIN_MANAGER__
	bool __GetAdminConfig(uint32_t adwAdminConfig[GM_MAX_NUM], uint32_t adwAdminManagerConfig[GM_MAX_NUM]);
#else
	bool __GetAdminConfig(uint32_t adwAdminConfig[GM_MAX_NUM]);
#endif
	//END_ADMIN_MANAGER


	//RELOAD_ADMIN
	void ReloadAdmin();
	//END_RELOAD_ADMIN
	void BreakMarriage(CPeer* peer, const char* data);


	void SendSpareItemIDRange(CPeer* peer);

	void UpdateHorseName(TPacketUpdateHorseName* data, CPeer* peer);
	void AckHorseName(uint32_t dwPID, CPeer* peer);

	void UpdateItemName(TPacketUpdateItemName* data, CPeer* peer);
	void AckItemName(uint32_t dwItemID, CPeer* peer);

	void LoadHwidFlag();
	void SetHwidFlag(TPacketSetHwidFlag* p);
	void SendHwidFlagsOnSetup(CPeer* peer);

	void MessengerSetBlock(const PacketGDMessengerSetBlock* block);

	//delete gift notify icon
	void DeleteAwardId(TPacketDeleteAwardID* data);


#ifdef __DUNGEON_FOR_GUILD__
	void GuildDungeon(TPacketGDGuildDungeon* sPacket);
	void GuildDungeonGD(TPacketGDGuildDungeonCD* sPacket);
#endif
public:

	void UnbindPeer(CPeer* peer);
	void EraseItemCache(uint32_t id);

private:

#ifdef __OFFLINE_SHOP__
public:

	typedef std::map<uint32_t, TOfflineShopItem> TMapItemByPosition;

	typedef struct SOfflineShop
	{
		TOfflineShopData kShopData{};

		TMapItemByPosition mapItems;

		std::list<CPeer*> listListenerPeers;

		SOfflineShop()
		{
			this->kShopData.dwOwnerPID = 0;
			memset(this->kShopData.szName, '\0', sizeof(this->kShopData.szName));

			this->kShopData.llGold = 0;

			this->kShopData.lMapIndex = 0;
			this->kShopData.lX = this->kShopData.lY = 0;

			this->kShopData.byChannel = 0;
#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
			this->kShopData.iLeftOpeningTime = 0;
#endif
#if !defined(__OFFLINE_SHOP_OPENING_TIME__)
			this->kShopData.bOpen = false;
#endif

			this->mapItems.clear();

			this->listListenerPeers.clear();
		}

		void ForEachPeer(uint8_t header, const void* data, size_t size, CPeer* except = NULL);
	} TOfflineShop;

	typedef std::unordered_map<uint32_t, TOfflineShop> TMapOfflineShopByOwnerPID;

private:
	TMapOfflineShopByOwnerPID m_mapOfflineShops;

	uint32_t m_dwNextOfflineShopItemID;

private:
	bool LoadOfflineShops();
	bool LoadNextOfflineShopItemID();

	void ReceiveOfflineShopCreatePacket(TPacketGDOfflineShopCreate* pkPacket, uint8_t byChannel);
	void ReceiveOfflineShopUpdateNamePacket(CPeer* pkPeer, TPacketOfflineShopUpdateName* pkPacket);
	void ReceiveOfflineShopUpdatePositionPacket(TPacketGDOfflineShopUpdatePosition* pkPacket);
	void ReceiveOfflineShopAddItemPacket(CPeer* pkPeer, TPacketOfflineShopAddItem* pkPacket);
	void ReceiveOfflineShopMoveItemPacket(CPeer* pkPeer, TPacketOfflineShopMoveItem* pkPacket);
	void ReceiveOfflineShopRemoveItemPacket(CPeer* pkPeer, TPacketOfflineShopRemoveItem* pkPacket);
        void ReceiveOfflineShopBuyItemByIdPacket(CPeer *pkPeer, uint32_t handle, TPacketOfflineShopRemoveItemById* pkPacket);
        void ReceiveOfflineShopRequestBuyRequest(CPeer *peer, uint32_t dwHandle,
                                                 const uint32_t *p);
        void ReceiveOfflineShopRequestItemInfoPacket(CPeer* pkPeer, TPacketOfflineShopRequestItem* pkPacket);

        void ReceiveOfflineShopUpdateGoldPacket(CPeer* pkPeer, TPacketOfflineShopUpdateGold* pkPacket);
	void ReceiveOfflineShopDestroyPacket(CPeer* pkPeer, TPacketOfflineShopDestroy* pkPacket);
	void ReceiveOfflineShopRegisterListenerPacket(CPeer* pkPeer, TPacketGDOfflineShopRegisterListener* pkPacket);
	void ReceiveOfflineShopUnregisterListenerPacket(CPeer* pkPeer, TPacketGDOfflineShopUnregisterListener* pkPacket);
	void ReceiveOfflineShopClosePacket(CPeer* pkPeer, TPacketOfflineShopClose* pkPacket);
        void ReceiveOfflineShopSearchPacket(CPeer *pkPeer, DWORD dwHandle,
                                            TPacketOfflineShopSearch *p);
        void ReceiveOfflineShopOpenPacket(CPeer* pkPeer, TPacketOfflineShopOpen* pkPacket);
#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
	void ReceiveOfflineShopFlushLeftOpeningTimePacket(TPacketGDOfflineShopFlushLeftOpeningTime* pkPacket);
#endif

	TOfflineShop* FindOfflineShop(uint32_t dwOwnerPID);
	void DestroyOfflineShop(uint32_t dwOwnerPID);

	uint32_t GetNextOfflineShopItemID();

public:
	TMapOfflineShopByOwnerPID*	GetOfflineShopMap();
private:
	typedef std::unordered_map<uint32_t, COfflineShopCache*>		TMapOfflineShopCacheByOwnerPID;
	typedef std::unordered_map<uint32_t, COfflineShopItemCache*>	TMapOfflineShopItemCacheByItemID;

private:
	TMapOfflineShopCacheByOwnerPID		m_mapOfflineShopCaches;
	TMapOfflineShopItemCacheByItemID	m_mapOfflineShopItemCaches;
	std::queue<COfflineShopItemCache*>	m_queueOfflineShopItemCachesOrder;

private:
	void	PutOfflineShopCache(TOfflineShop* pkOfflineShop);
	void	PutOfflineShopItemCache(uint32_t dwOwnerPID, TOfflineShopItem* pkOfflineShopItem);

	void	FlushOfflineShopCache(bool bDeleteCache = false);
	void	FlushOfflineShopItemCache(bool bDeleteCache = false);
private:

#endif

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
	//EVENT_MANAGER
private:
	bool		__GetEventInfo(std::vector<TEventInfo> & rEventVec);
	void		ReloadEvents(CPeer*);
	void		AutoEvents(TPacketAutoEvents* p);
	//END_EVENT_MANAGER
#endif

	public:
		typedef std::unordered_set<CDungeonInfoCache *, std::hash<CDungeonInfoCache*> > TDungeonInfoCacheSet;
		typedef std::unordered_map<uint32_t, TDungeonInfoCacheSet *> TDungeonInfoCacheSetPtrMap;

		TDungeonInfoCacheSet * FindDungeonInfoCacheSet(uint32_t dwPlayerID);
		TDungeonInfoCacheSet * GetDungeonInfoCacheSet(uint32_t dwPlayerID);
		
		CDungeonInfoCache * GetDungeonInfoCache(uint32_t dwPlayerID, uint8_t bDungeonID);
		void PutDungeonInfoCache(TPlayerDungeonInfo * p, bool bSkipQuery);

		void UpdateDungeonInfoCache();
		
		void SendDungeonInfoCacheSet(CPeer *peer, PlayerLoader *loader);
	private:
		void	RESULT_DUNGEON_INFO_LOAD(CPeer * peer, MYSQL_RES * pRes, PlayerLoader* loader);
		void 	QUERY_DUNGEON_INFO_SAVE(CPeer * pkPeer, const char * c_pData);
	
		TDungeonInfoCacheSetPtrMap	m_map_DungeonInfoSetPtr;
		
	public:
		typedef std::map<std::pair<uint8_t, uint8_t>, std::unique_ptr<TDungeonRankSet>> TDungeonRankCachePtrMap;
		
		bool 	InitializeDungeonRanking(bool bIsReload = false);
	private:
		TDungeonRankCachePtrMap	m_map_DungeonRankMap;
};

template <class Func>
void CClientManager::for_each_peer(Func&& f)
{
	std::for_each(m_peerList.begin(), m_peerList.end(),
	              [&f](const std::shared_ptr<CPeer>& p)
	              {
		              f(p.get());
	              });
}

#endif /* METIN2_SERVER_DB_CLIENTMANAGER_H */
