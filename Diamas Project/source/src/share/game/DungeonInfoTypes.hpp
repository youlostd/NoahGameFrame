#ifndef METIN2_GAME_DUNGEONINFOTYPES_HPP
#define METIN2_GAME_DUNGEONINFOTYPES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#include <game/Constants.hpp>
#include <game/DungeonInfoConstants.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

typedef struct SPlayerDungeonInfo
{
	uint32_t	dwPlayerID;
	uint8_t 	bDungeonID;
	
	uint32_t	dwCooldownEnd;
	uint16_t	wCompleteCount;
	uint16_t	wFastestTime;
	uint32_t	dwHightestDamage;
} TPlayerDungeonInfo;

BOOST_FUSION_ADAPT_STRUCT(
    TPlayerDungeonInfo,
	dwPlayerID,
	bDungeonID,
	dwCooldownEnd,
	wCompleteCount,
	wFastestTime,
	dwHightestDamage
)

typedef struct SDungeonRankItem
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t dwValue;
} TDungeonRankItem;

BOOST_FUSION_ADAPT_STRUCT(
    TDungeonRankItem,
	szName,
	dwValue
)

typedef struct SDungeonRankSet
{
	uint8_t bDungeonID;
	uint8_t bType;
	TDungeonRankItem rankItems[DUNGEON_RANKING_MAX_NUM];
} TDungeonRankSet;

BOOST_FUSION_ADAPT_STRUCT(
    TDungeonRankSet,
	bDungeonID,
	bType,
	rankItems
)
typedef struct SDungeonInfoSet
{
	uint8_t bDungeonID;
	uint8_t bMinLvl, bMaxLvl;
	uint8_t bMinParty, bMaxParty;
	uint16_t wCooldown, wRunTime;
	uint32_t lEnterMapIndex, lEnterX, lEnterY;
	uint32_t dwTicketVnum;
	uint8_t bTicketCount;
} TDungeonInfoSet;


BOOST_FUSION_ADAPT_STRUCT(
    TDungeonInfoSet,
	bDungeonID,
	bMinLvl,
	bMaxLvl,

	bMinParty,
	bMaxParty,

	wCooldown,
	wRunTime,

	lEnterMapIndex, lEnterX, lEnterY,
	dwTicketVnum,
	bTicketCount

)
#endif