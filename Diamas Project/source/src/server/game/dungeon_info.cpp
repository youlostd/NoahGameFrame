#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include "char.h"
#include "db.h"
#include "lzo_manager.h"
#include "DbCacheSocket.hpp"
#include "char_manager.h"
#include <game/GamePacket.hpp>
#include "questmanager.h"
#include "locale_service.h"
#include "dungeon_info.h"

#include "desc.h"

CDungeonInfo::CDungeonInfo()
{
}

CDungeonInfo::~CDungeonInfo()
{
	bIsInfoLoading = false;
	ClearInfoMap(true);
}

bool CDungeonInfo::ReadDungeonInfoFile(const char * c_pszFileName)
{
	FILE *fp;
	char one_line[256];
    auto delim = "\t\r\n";
	char *token_string;
    auto iCnt = 0;

	TDungeonInfoSet * infoSet = NULL;
	
	if (0 == c_pszFileName || 0 == c_pszFileName[0])
		return false;

	if ((fp = fopen(c_pszFileName, "r")) == 0)
		return false;
	
	while (fgets(one_line, 256, fp))
	{
		if (one_line[0] == '#')
			continue;
		
		token_string = strtok(one_line, delim);
		if (NULL == token_string)
			continue;

        infoSet = new TDungeonInfoSet;

        iCnt = 0;
        while (token_string != NULL)
        {
            switch(iCnt)
            {
                case 0:
                    str_to_number(infoSet->bDungeonID, token_string);
                    break;
                case 4:
                    str_to_number(infoSet->bMinLvl, token_string);
                    break;
                case 5:
                    str_to_number(infoSet->bMaxLvl, token_string);
                    break;
                case 6:
                    str_to_number(infoSet->bMinParty, token_string);
                    break;
                case 7:
                    str_to_number(infoSet->bMaxParty, token_string);
                    break;
                case 8:
                    str_to_number(infoSet->wCooldown, token_string);
                    break;
                case 9:
                    str_to_number(infoSet->wRunTime, token_string);
                    break;
               case 10:
                    str_to_number(infoSet->lEnterMapIndex, token_string);
                    break;
                case 11:
                    str_to_number(infoSet->lEnterX, token_string);
                    break;
                case 12:
                    str_to_number(infoSet->lEnterY, token_string);
                    break;
                case 13:
                    str_to_number(infoSet->dwTicketVnum, token_string);
                    break;
                case 14:
                    str_to_number(infoSet->bTicketCount, token_string);
                    break;

                default:
                    break;
            }
            
            token_string = strtok(NULL, delim);
            iCnt++;
        }

        m_map_DungeonInfoMap.emplace(infoSet->bDungeonID, std::move(infoSet));
	}

	return true;
}

TDungeonInfoSet * CDungeonInfo::GetDungeonInfo(uint8_t bDungeonID)
{
    auto it = m_map_DungeonInfoMap.find(bDungeonID);
	if(it != m_map_DungeonInfoMap.end())
	{
		return it->second.get();
	}
	
	return NULL;
}

bool CDungeonInfo::IsValidDungeon(uint8_t bDungeonID)
{
    auto it = m_map_DungeonInfoMap.find(bDungeonID);
	if(it != m_map_DungeonInfoMap.end())
		return true;

	return false;
}

void CDungeonInfo::DungeonWarpSet(CHARACTER* ch, const TPacketCGDungeonWarp& p)
{
	if(!ch)
		return;
	
	if(!ch->GetDesc())
		return;


    auto it = m_map_DungeonInfoMap.find(p.bDungeonID);
	if(it != m_map_DungeonInfoMap.end())
	{
		if (ch->IsHack() || !ch->CanWarp())
			return;

		if(ch->GetLevel() < it->second->bMinLvl || ch->GetLevel() > it->second->bMaxLvl) {
			SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You level is too high/low for this dungeon.");
            return;
		}
		
		ch->WarpSet(it->second->lEnterMapIndex,  it->second->lEnterX, it->second->lEnterY);
	}
}

// Ranking
void CDungeonInfo::ClearInfoMap(bool bAll)
{

	m_map_DungeonRankMap.clear();
	
	if(bAll)
	{
		m_map_DungeonInfoMap.clear();
	}
}

void CDungeonInfo::AppendRankSet(TDungeonRankSet * pRank)
{
	// sys_err("AppendRankSet %d %d", pRank->bDungeonID, pRank->bType);
	
	auto pSet = std::make_unique<TDungeonRankSet>();
	pSet->bDungeonID = pRank->bDungeonID;
	pSet->bType = pRank->bType;
	std::memcpy(&pSet->rankItems, pRank->rankItems, sizeof(TDungeonRankItem) * DUNGEON_RANKING_MAX_NUM);

	m_map_DungeonRankMap.emplace(std::make_pair(pRank->bDungeonID, pRank->bType), std::move(pSet));
}

void CDungeonInfo::GetRankingSet(CHARACTER* ch, const TPacketCGDungeonRanking& p)
{
	if(!ch)
		return;
	
	if(!ch->GetDesc())
		return;


	TDungeonRankCachePtrMap::iterator it = m_map_DungeonRankMap.find(std::make_pair(p.bDungeonID, p.bRankingType));
	if(it != m_map_DungeonRankMap.end())
	{
		TDungeonRankSet * pSet = it->second.get();
		if(pSet)
		{
			TPacketGCDungeonRanking packet;
			packet.rankingSet.bDungeonID = pSet->bDungeonID;
			packet.rankingSet.bType = pSet->bType;
			std::memcpy(&packet.rankingSet.rankItems, pSet->rankItems, sizeof(TDungeonRankItem) * DUNGEON_RANKING_MAX_NUM);
			ch->GetDesc()->Send(HEADER_GC_DUNGEON_RANKING, packet);
			return;
		}
	}
	
	TPacketGCDungeonRanking packet;
	packet.rankingSet.bDungeonID = p.bDungeonID;
	packet.rankingSet.bType = p.bRankingType;
	memset(&packet.rankingSet.rankItems, 0, sizeof(TDungeonRankItem) * DUNGEON_RANKING_MAX_NUM);
	ch->GetDesc()->Send(HEADER_GC_DUNGEON_RANKING, packet);
}