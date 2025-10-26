#pragma once
#include <Config.hpp>
#include "StdAfx.h"
#include "../EterBase/Timer.h"

#ifdef ENABLE_ADMIN_MANAGER
#include <game/GamePacket.hpp>

class CPythonAdmin : public CSingleton<CPythonAdmin>
{
public:
	enum ETeamAllowFlags {
		ALLOW_VIEW_ONLINE_LIST = 1 << 0,
		ALLOW_VIEW_ONLINE_COUNT = 1 << 1,
		ALLOW_GM_TRADE_BLOCK_OPTION = 1 << 2,
		ALLOW_MAPVIEWER = 1 << 3,
		ALLOW_VIEW_MAPVIEWER_NPC = 1 << 4,
		ALLOW_VIEW_MAPVIEWER_STONE = 1 << 5,
		ALLOW_VIEW_MAPVIEWER_MONSTER = 1 << 6,
		ALLOW_VIEW_MAPVIEWER_PLAYER = 1 << 7,
		ALLOW_VIEW_MAPVIEWER_ORE = 1 << 8,
		ALLOW_OBSERVER = 1 << 9,
		ALLOW_VIEW_OBSERVER_GOLD = 1 << 10,
		ALLOW_VIEW_OBSERVER_SKILL = 1 << 11,
		ALLOW_VIEW_OBSERVER_EQUIPMENT = 1 << 12,
		ALLOW_VIEW_OBSERVER_INVENTORY = 1 << 13,
		ALLOW_VIEW_OBSERVER_WHISPER = 1 << 14,
		ALLOW_OBSERVER_CHATBAN = 1 << 15,
		ALLOW_OBSERVER_ACCOUNTBAN = 1 << 16,
		ALLOW_OBSERVER_PERMA_ACCOUNTBAN = 1 << 17,
		ALLOW_BAN = 1 << 18,
		ALLOW_VIEW_BAN_CHAT = 1 << 19,
		ALLOW_VIEW_BAN_CHAT_LOG = 1 << 20,
		ALLOW_BAN_CHAT = 1 << 21,
		ALLOW_VIEW_BAN_ACCOUNT = 1 << 22,
		ALLOW_VIEW_BAN_ACCOUNT_LOG = 1 << 23,
		ALLOW_BAN_ACCOUNT = 1 << 24,
		ALLOW_ITEM = 1 << 25,
		ALLOW_VIEW_ITEM = 1 << 26,
	};

	enum EBanTypes {
		BAN_TYPE_CHAT = 1,
		BAN_TYPE_ACCOUNT = 2,
	};

	enum EBanAccountSearchTypes {
		BAN_ACCOUNT_SEARCH_ACCOUNT,
		BAN_ACCOUNT_SEARCH_PLAYER,
		BAN_ACCOUNT_SEARCH_MAX_NUM,
	};

	enum EItemOwnerTypes {
		ITEM_OWNER_PLAYER,
		ITEM_OWNER_ACCOUNT,
	};

	enum EItemSearchTypes {
		ITEM_SEARCH_IID,
		ITEM_SEARCH_INAME,
		ITEM_SEARCH_PID,
		ITEM_SEARCH_PNAME,
		ITEM_SEARCH_GM_ITEM,
	};

	enum EGeneralPlayerSortOptions {
		GENERAL_ONLINE_PLAYER_SORT_BY_PID,
		GENERAL_ONLINE_PLAYER_SORT_BY_NAME,
		GENERAL_ONLINE_PLAYER_SORT_BY_MAP_INDEX,
		GENERAL_ONLINE_PLAYER_SORT_BY_CHANNEL,
		GENERAL_ONLINE_PLAYER_SORT_BY_EMPIRE,
	};

	typedef struct SWhisperMessage {
		uint32_t							dwSenderPID;
		uint32_t							dwSentTime;
		char							szMessage[512 + 1];
	} TWhisperMessage;
	typedef struct SWhisper {
		uint32_t							dwOtherPID;
		std::string						stOtherName;
		std::vector<TWhisperMessage>	vec_kMessages;
	} TWhisper;

	typedef struct SChatBanPlayerInfo {
		char	szName[CHARACTER_NAME_MAX_LEN + 1];
		uint8_t	bRace;
		uint8_t	bLevel;
		uint16_t	wChatbanCount;
		uint16_t	wAccountbanCount;
		uint32_t	dwDuration;
		uint32_t	dwOnlineTime;

		SChatBanPlayerInfo() : dwDuration(0), dwOnlineTime(0)
		{
		}
		void	SetOnline()
		{
			dwOnlineTime = static_cast<float>(ELTimer_GetMSec() / 1000.0f);
		}
		void	SetOffline()
		{
			if (!IsOnline())
				return;

			dwDuration = GetDuration();
			dwOnlineTime = 0;
		}
		bool	IsOnline() const
		{
			return dwOnlineTime != 0;
		}
		uint32_t	GetDuration() const
		{
			if (!dwDuration)
				return 0;

			if (!dwOnlineTime)
				return dwDuration;

			uint32_t dwTimeOnline = static_cast<float>(ELTimer_GetMSec() / 1000.0f) - dwOnlineTime;
			if (dwTimeOnline >= dwDuration)
				return 0;

			return dwDuration - dwTimeOnline;
		}
		void	SetDuration(uint32_t dwDuration)
		{
			this->dwDuration = dwDuration;

			if (dwOnlineTime)
				dwOnlineTime = static_cast<float>(ELTimer_GetMSec() / 1000.0f);
		}
	} TChatBanPlayerInfo;

	typedef struct SAccountBanInfo {
		char	szLoginName[30 + 1];
		uint32_t	dwPID[PLAYER_PER_ACCOUNT];
		char	szName[PLAYER_PER_ACCOUNT][CHARACTER_NAME_MAX_LEN + 1];
		uint8_t	bRace[PLAYER_PER_ACCOUNT];
		uint8_t	bLevel[PLAYER_PER_ACCOUNT];
		uint16_t	wChatbanCount[PLAYER_PER_ACCOUNT];
		uint16_t	wAccountbanCount;
		uint32_t	dwDuration;
		bool	bIsOnline[PLAYER_PER_ACCOUNT];

		uint32_t	GetDuration() const
		{
			if (!dwDuration)
				return 0;

			if (dwDuration == INT_MAX)
				return dwDuration;

			int iTimeLeft = dwDuration - static_cast<float>(ELTimer_GetMSec() / 1000.0f);
			if (iTimeLeft < 0)
				return 0;

			return iTimeLeft;
		}
		void	SetDuration(uint32_t dwDuration)
		{
			if (!dwDuration)
				this->dwDuration = 0;
			else
				this->dwDuration = static_cast<float>(ELTimer_GetMSec() / 1000.0f) + dwDuration;
		}
	} TAccountBanInfo;

	typedef struct SItemInfo {
		uint8_t		bOwnerType;
		uint32_t		dwOwnerID;
		std::string	stOwnerName;
		TItemPos	kCell;
		uint32_t		dwItemVnum;
		uint8_t		bCount;
		SocketValue		alSockets[ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute	aAttr[ITEM_ATTRIBUTE_MAX_NUM];
		bool		bIsGMItem;
	} TItemInfo;

public:
	CPythonAdmin();
	~CPythonAdmin();

	void	Initialize();
	void	Clear();

	// general
	bool	CanOpenAdminWindow();

	void	SetGameWindow(PyObject* poWindow);

	void	SetAllowFlag(uint32_t dwAllowFlag);
	uint32_t	GetAllowFlag() const;
	bool	HasAllow(uint32_t dwAllowFlag) const;

	bool	AddOnlinePlayer(const TAdminManagerPlayerInfo* c_pPlayerInfo);
	bool	RemoveOnlinePlayer(uint32_t dwPID);

	uint32_t	GetOnlinePlayerCount();
	const TAdminManagerPlayerInfo*	GetOnlinePlayerByIndex(uint32_t dwIndex);
	const TAdminManagerPlayerInfo*	GetOnlinePlayerByPID(uint32_t dwPID);

	void	SortOnlinePlayer(uint8_t bSortType, uint8_t bDirection);
	bool	IsOnlinePlayerSorted() const;
	int		GetOnlinePlayerSortType() const { return m_iOnlinePlayerSortType; }
	uint8_t	GetOnlinePlayerSortDirection() const { return m_bOnlinePlayerDirection; }
	const TAdminManagerPlayerInfo*	GetSortOnlinePlayerByIndex(uint32_t dwIndex);

	void	SetGMItemTradeBlock(bool bIsBlock);
	bool	IsGMItemTradeBlock() const;

	// mapviewer
	void				MapViewer_Clear();

	void				MapViewer_LoadMapInfo(uint32_t mapIndex);
	const std::string&	MapViewer_GetMapName();
	uint32_t MapViewer_GetMapIndex();
	uint32_t				MapViewer_GetSizeX();
	uint32_t				MapViewer_GetSizeY();

	void				MapViewer_AddPlayer(const TAdminManagerMapViewerPlayerInfo& rkPlayer);
	void				MapViewer_ErasePlayer(uint32_t dwPID);
	void				MapViewer_AddMonster(const TAdminManagerMapViewerMobInfo& rkMonster);
	void				MapViewer_EraseMonster(uint32_t dwVID);
	TAdminManagerMapViewerPlayerInfo*							MapViewer_GetPlayerByPID(uint32_t dwPID);
	const std::map<uint32_t, TAdminManagerMapViewerPlayerInfo>&	MapViewer_GetPlayer();
	TAdminManagerMapViewerMobInfo*								MapViewer_GetMonsterByVID(uint32_t dwVID);
	const std::map<uint32_t, TAdminManagerMapViewerMobInfo>&		MapViewer_GetMonster();

	// observer
	void				Observer_Clear(bool bClearMessages = true);
	
	void				Observer_StopRunning();
	bool				Observer_IsRunning() const;
	bool				Observer_IsStoppedByForce() const;
	void				Observer_SetPID(uint32_t dwPID);
	uint32_t				Observer_GetPID() const;
	void				Observer_SetAID(uint32_t dwAID);
	uint32_t				Observer_GetAID() const;
	void				Observer_SetLoginName(const std::string& c_rstLoginName);
	const std::string&	Observer_GetLoginName() const;
	void				Observer_SetRaceNum(uint8_t bRaceNum);
	uint8_t				Observer_GetRaceNum() const;
	void				Observer_LoadMapName(uint32_t mapIndex);
	const std::string&	Observer_GetMapName() const;

	void				Observer_SetSkillGroup(uint8_t bSkillGroup);
	uint8_t				Observer_GetSkillGroup() const;
	void				Observer_SetSkill(uint32_t dwSkillVnum, TPlayerSkill* pkSkillData);
	const TPlayerSkill*	Observer_GetSkill(uint32_t dwSkillVnum) const;
	void				Observer_SetSkillCoolTime(uint32_t dwSkillVnum, uint32_t dwCoolTime);
	uint32_t				Observer_GetSkillCoolTime(uint32_t dwSkillVnum) const;
	uint32_t				Observer_GetSkillElapsedCoolTime(uint32_t dwSkillVnum) const;
	
	void				Observer_SetPoint(uint8_t bType, long long llValue);
	long long			Observer_GetPoint(uint8_t bType, long long llDefaultValue = -1) const;

	void									Observer_SetInventoryItem(const TAdminManagerObserverItemInfo* pItemInfo);
	void									Observer_DelInventoryItem(uint16_t wCell);
	const TAdminManagerObserverItemInfo*	Observer_GetInventoryItem(uint16_t wCell) const;

	void								Observer_AddMessage(bool bIsOtherPIDSender, uint32_t dwOtherPID, uint32_t dwElapsedTime, const char* szMessage, const char* szOtherName = NULL);
	const std::map<uint32_t, TWhisper>&	Observer_GetMessageMap() const;

	void				Observer_SetChatBanTimeLeft(uint32_t dwTimeLeft);
	uint32_t				Observer_GetChatBanTimeLeft() const;

	void				Observer_SetAccountBanTimeLeft(uint32_t dwTimeLeft);
	uint32_t				Observer_GetAccountBanTimeLeft() const;

	// ban
	void				Ban_Clear();

	void				Ban_SetPlayerOnline(uint32_t dwPID);
	void				Ban_SetPlayerOffline(uint32_t dwPID);
	void				Ban_UpdateChatInfo(const TAdminManagerBanClientPlayerInfo* pPlayerInfo);
	void				Ban_SetChatSearchResult(const TAdminManagerBanClientPlayerInfo* pPlayerInfo);
	void				Ban_ClientSetChatSearchResult(uint32_t dwPID);
	void				Ban_UpdateAccountInfo(const TAdminManagerBanClientAccountInfo* pAccountInfo);
	void				Ban_SetAccountSearchResult(const TAdminManagerBanClientAccountInfo* pAccountInfo);
	void				Ban_ClientSetAccountSearchResult(uint32_t dwAID);

	uint32_t				Ban_GetChatPlayerCount();
	const TChatBanPlayerInfo*	Ban_GetChatPlayerByIndex(uint32_t dwIndex, uint32_t& rdwPID);
	const TChatBanPlayerInfo*	Ban_GetChatPlayerByPID(uint32_t dwPID);
	uint32_t						Ban_GetChatPlayerPIDByName(const std::string& c_rstName);
	uint32_t						Ban_GetChatSearchResultPID() const;

	uint32_t				Ban_GetAccountCount();
	const TAccountBanInfo*		Ban_GetAccountByIndex(uint32_t dwIndex, uint32_t& rdwAID);
	const TAccountBanInfo*		Ban_GetAccountByAID(uint32_t dwAID);
	uint32_t						Ban_GetAccountAIDByName(const std::string& c_rstName);
	uint32_t						Ban_GetAccountAIDByPlayerName(const std::string& c_rstName);
	uint32_t						Ban_GetAccountAIDByPID(uint32_t dwPID);
	uint32_t						Ban_GetAccountSearchResultAID() const;

	void				Ban_ClearLogInfo();
	void				Ban_AddLogInfo(const TAdminManagerBanClientLogInfo* pLogInfo, const char* szReason, const char* szProof);
	const TAdminManagerBanLogInfo*	Ban_GetLogInfo(uint32_t dwIndex);
	uint32_t				Ban_GetLogInfoCount() const;
	void				Ban_SetLogInfoType(uint8_t bType);
	uint8_t				Ban_GetLogInfoType() const;

	// item
	void				Item_Clear();

	void				Item_ClearInfo();
	void				Item_AddInfo(TAdminManagerItemInfo* pInfo, const char* szOwnerName);
	uint32_t				Item_GetInfoCount() const;
	TItemInfo*			Item_GetInfoByIndex(uint32_t dwIndex, uint32_t& rdwID);
	TItemInfo*			Item_GetInfoByID(uint32_t dwID);

private:
	PyObject*	m_poGameWindow;
	bool		m_bCanOpenAdminWindow;

	uint32_t	m_dwAllowFlag;

	std::map<uint32_t, TAdminManagerPlayerInfo>	m_map_OnlinePlayer;
	std::vector<TAdminManagerPlayerInfo>		m_vec_OnlinePlayerSorted;
	int		m_iOnlinePlayerSortType;
	uint8_t	m_bOnlinePlayerDirection;
	
	bool	m_bGMItemTradeBlock;

	bool		m_bMapViewer_ShowMap;
	uint32_t	m_mapViewer_MapIndex;
	std::string	m_stMapViewer_MapName;
	uint32_t		m_dwMapViewer_MapSizeX;
	uint32_t		m_dwMapViewer_MapSizeY;
	std::map<uint32_t, TAdminManagerMapViewerPlayerInfo>	m_map_MapViewerPlayer;
	std::map<uint32_t, TAdminManagerMapViewerMobInfo>		m_map_MapViewerMob;

	bool							m_bObserver_Running;
	uint32_t							m_dwObserver_PID;
	uint32_t							m_dwObserver_AID;
	std::string						m_stObserver_LoginName;
	uint8_t							m_bObserver_RaceNum;
	std::string						m_stObserver_MapName;
	uint8_t							m_bObserver_SkillGroup;
	TPlayerSkill					m_akObserver_Skills[SKILL_MAX_NUM];
	uint32_t							m_adwObserver_SkillStartCoolTimes[SKILL_MAX_NUM];
	uint32_t							m_adwObserver_SkillCoolTimes[SKILL_MAX_NUM];
	std::map<uint8_t, int>				m_map_ObserverPoints;
	TAdminManagerObserverItemInfo	m_akObserver_Item[c_Inventory_Count];
	std::map<uint32_t, TWhisper>		m_map_ObserverWhispers;
	uint32_t							m_dwObserverChatBanTimeout;
	uint32_t							m_dwObserverAccountBanTimeout;

	uint32_t									m_dwBanChatSearchPlayerPID;
	TChatBanPlayerInfo						m_kBanChatSearchPlayer;
	std::map<uint32_t, TChatBanPlayerInfo>		m_map_BanChatList;
	std::map<std::string, uint32_t>			m_map_BanChatPIDMap;

	uint32_t									m_dwBanAccountSearchResultAID;
	TAccountBanInfo							m_kBanAccountSearchResult;
	std::map<uint32_t, TAccountBanInfo>		m_map_BanAccountList;
	std::map<std::string, uint32_t>			m_map_BanAccountMapByName;
	std::map<std::string, uint32_t>			m_map_BanAccountMapByPlayerName;
	std::map<uint32_t, uint32_t>					m_map_BanAccountMapByPID;

	uint8_t									m_bBanLogType;
	std::vector<TAdminManagerBanLogInfo>	m_vec_BanLogInfo;

	std::map<uint32_t, TItemInfo>				m_map_ItemInfo;
};
#endif
