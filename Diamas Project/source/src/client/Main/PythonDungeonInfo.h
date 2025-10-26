#pragma once
#include <game/DungeonInfoTypes.hpp>

class CPythonDungeonInfo : public CSingleton<CPythonDungeonInfo>
{
	public:
		enum EDungeonType
		{
			DUNGEON_TYPE_UNKNOWN,
			DUNGEON_TYPE_PRIVATE,
			DUNGEON_TYPE_PUBLIC, // Need to register and everyone can join until party is full
		};
		
		enum EDungeonOrganization
		{
			DUNGEON_ORG_UNKNOWN,
			DUNGEON_ORG_SINGLE,
			DUNGEON_ORG_PARTY,
			DUNGEON_ORG_HYBRID, // Single or Party
		};
		
		enum EDungeonTableTokenType
		{
			TABLE_TOKEN_TYPE_ID,
			TABLE_TOKEN_TYPE_NAME,
			TABLE_TOKEN_TYPE_TYPE,
			TABLE_TOKEN_TYPE_ORGANIZATION,
			TABLE_TOKEN_TYPE_MIN_LEVEL,
			TABLE_TOKEN_TYPE_MAX_LEVEL,
			TABLE_TOKEN_TYPE_MIN_PARTY,
			TABLE_TOKEN_TYPE_MAX_PARTY,
			TABLE_TOKEN_TYPE_COOLDOWN,
			TABLE_TOKEN_TYPE_RUNTIME,
			TABLE_TOKEN_TYPE_ENTER_MAP_INDEX,
			TABLE_TOKEN_TYPE_ENTER_X,
			TABLE_TOKEN_TYPE_ENTER_Y,
			TABLE_TOKEN_TYPE_TICKET_VNUM,
			TABLE_TOKEN_TYPE_TICKET_COUNT,
			TABLE_TOKEN_TYPE_ST_RACEFLAG,
			TABLE_TOKEN_TYPE_RES_RACEFLAG,
			TABLE_TOKEN_TYPE_MAX_NUM,
		};
		
		typedef struct SDungeonInfoData
		{
			BYTE bID; // Max number of dungeons will be 255
			std::string strName;
			
			BYTE bType;
			BYTE bOrganization;
			
			BYTE bMinLevel;
			BYTE bMaxLevel;
			
			BYTE bMinParty;
			BYTE bMaxParty;	

			WORD wCooldown;
			WORD wRunTime;

			long lEnterMapIndex;
			long lEnterX;
			long lEnterY;
			
			DWORD dwTicketVnum;
			BYTE bTicketCount;

			std::string strStRaceFlag;
			std::string strResRaceFlag;
		} TDungeonInfoData;
		
		typedef std::map<DWORD, TDungeonInfoData> TDungeonInfoDataMap;

	public:
		CPythonDungeonInfo();
		virtual ~CPythonDungeonInfo();

		void Destroy(bool bIsPartial = false);
		bool LoadDungeonInfo(const char * c_szFileName);

		bool IsValidDungeon(BYTE bID);
		const TDungeonInfoData * GetDungeonByID(BYTE bID) const;
		
		void ClearInfo();
		void AppendDungeonInfo(const TPlayerDungeonInfo & info);
		const TPlayerDungeonInfo * GetDungeonInfo(BYTE bID) const;
		void UpdateDungeonCooldown(BYTE bDungeonID, DWORD dwCooldownEnd);
		
		bool IsNeedRequestInfo() { return m_bNeedServerRequest; }
		void SetNoNeedRequestInfo() { m_bNeedServerRequest = false; }
	protected:
		TDungeonInfoDataMap m_DungeonInfoMap;
		std::vector<TPlayerDungeonInfo> m_dungeonInfo;
		bool m_bNeedServerRequest;
		
	// Ranking code
	public:
		bool IsRankingRequestInfo(BYTE bDungeonID, BYTE bRankingType) { return m_bRankingServerRequest[bDungeonID][bRankingType]; }
		void SetNoRankingRequestInfo(BYTE bDungeonID, BYTE bRankingType) { m_bRankingServerRequest[bDungeonID][bRankingType] = false; }
		
		void AppendDungeonRankingSet(const TDungeonRankSet & set);
		const TDungeonRankSet * GetDungeonRankingSet(BYTE bDungeonID, BYTE bRankingType) const;
	protected:
		bool m_bRankingServerRequest[DUNGEON_ID_MAX][DUNGEON_RANK_TYPE_MAX];
		
		std::vector<TDungeonRankSet> m_dungeonRankingInfo;
};