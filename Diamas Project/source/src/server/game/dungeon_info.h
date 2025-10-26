#ifndef __DUNGEON_INFO_H__
#define __DUNGEON_INFO_H__

#include <base/Singleton.hpp>

#include <game/DungeonInfoTypes.hpp>
#include <game/GamePacket.hpp>

class CHARACTER;

class CDungeonInfo : public singleton<CDungeonInfo>
{
	public:
		CDungeonInfo();
		virtual ~CDungeonInfo();
		
		typedef std::map<std::pair<uint8_t, uint8_t>, std::unique_ptr<TDungeonRankSet>> TDungeonRankCachePtrMap;

		bool	IsInfoLoading() { return bIsInfoLoading; }
		void	SetInfoLoading(bool bFlag) { bIsInfoLoading = bFlag; }
		
		void 	ClearInfoMap(bool bAll = false);
		void 	AppendRankSet(TDungeonRankSet * pRank);
		
		void 	GetRankingSet(CHARACTER* ch, const TPacketCGDungeonRanking& p);
		
		void 	DungeonWarpSet(CHARACTER* ch, const TPacketCGDungeonWarp& p);
	private:
		TDungeonRankCachePtrMap	m_map_DungeonRankMap;
		bool bIsInfoLoading = false;
		
	public:
		typedef std::map<uint8_t, std::unique_ptr<TDungeonInfoSet>> TDungeonInfoPtrMap;
		
		bool ReadDungeonInfoFile(const char * c_pszFileName);
		TDungeonInfoSet * GetDungeonInfo(uint8_t bDungeonID);
		
		bool IsValidDungeon(uint8_t bDungeonID);
		size_t GetDungeonNum() const { return m_map_DungeonInfoMap.size(); }
	private:
		TDungeonInfoPtrMap m_map_DungeonInfoMap;
};
#endif
