#ifndef METIN2_GAME_MOBTYPES_HPP
#define METIN2_GAME_MOBTYPES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/MobConstants.hpp>
#include <game/Constants.hpp>
#include <game/Types.hpp>
#include <game/ItemTypes.hpp>

#include <xml/Types.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <unordered_map>
#include <vector>

METIN2_BEGIN_NS

#pragma pack(push, 1)
typedef struct SMobSkillLevel
{
	uint32_t dwVnum;
	uint8_t bLevel;
} TMobSkillLevel;

typedef struct SMobTable
{
	static const uint32_t kVersion = 2;

	uint32_t       dwVnum;
	std::string szName;
	std::string szLocaleName;

	uint8_t        bType;                  // Monster, NPC
	uint8_t        bRank;                  // PAWN, KNIGHT, KING
	uint8_t        bBattleType;            // MELEE, etc..
	uint8_t        bLevel;                 // Level
	uint16_t	wScale;

	Gold       dwGoldMin;
	Gold       dwGoldMax;
	uint32_t       dwExp;
	int64_t       dwMaxHP;
	uint16_t        bRegenCycle;
	uint8_t        bRegenPercent;
	uint16_t        wDef;

	uint32_t       dwAIFlag;
	uint32_t       dwRaceFlag;
	uint32_t       dwImmuneFlag;

	uint8_t        bStr, bDex, bCon, bInt;
	uint32_t       dwDamageRange[2];

	short       sAttackSpeed;
	short       sMovingSpeed;
	uint8_t        bAggresiveHPPct;
	uint16_t        wAggressiveSight;
	uint16_t        wAttackRange;

	char        cEnchants[MOB_ENCHANTS_MAX_NUM];
	char        cResists[MOB_RESISTS_MAX_NUM];

	uint32_t       dwResurrectionVnum;
	uint32_t       dwDropItemVnum;

	uint8_t        bMountCapacity;
	uint8_t        bOnClickType;

	uint8_t        bEmpire;
	float       fDamMultiply;
	uint32_t       dwSummonVnum;
	uint32_t       dwDrainSP;
	uint32_t		dwMonsterColor;
	uint32_t       dwPolymorphItemVnum;

	TMobSkillLevel	Skills[MOB_SKILL_MAX_NUM];

	uint8_t		bBerserkPoint;
	uint8_t		bStoneSkinPoint;
	uint8_t		bGodSpeedPoint;
	uint8_t		bDeathBlowPoint;
	uint8_t		bRevivePoint;
	uint32_t	despawnSeconds;
} TMobTable;


#pragma pack(pop)


METIN2_END_NS

BOOST_FUSION_ADAPT_STRUCT(
	TMobSkillLevel,
	dwVnum,
	bLevel);

BOOST_FUSION_ADAPT_STRUCT(
	TMobTable,
	dwVnum,
	szName,
	szLocaleName,
	bType,
	bRank,
	bBattleType,
	bLevel,
	wScale,
	dwGoldMin,
	dwGoldMax,
	dwExp,
	dwMaxHP,
	bRegenCycle,
	bRegenPercent,
	wDef,
	dwAIFlag,
	dwRaceFlag,
	dwImmuneFlag,
	bStr,
	bDex,
	bCon,
	bInt,
	dwDamageRange,
	sAttackSpeed,
	sMovingSpeed,
	bAggresiveHPPct,
	wAggressiveSight,
	wAttackRange,
	cEnchants,
	cResists,
	dwResurrectionVnum,
	dwDropItemVnum,
	bMountCapacity,
	bOnClickType,
	bEmpire,
	fDamMultiply,
	dwSummonVnum,
	dwDrainSP,
	dwMonsterColor,
	dwPolymorphItemVnum,
	Skills,
	bBerserkPoint,
	bStoneSkinPoint,
	bGodSpeedPoint,
	bDeathBlowPoint,
	bRevivePoint,
	despawnSeconds);

#endif // METIN2_GAME_MOBTYPES_HPP
