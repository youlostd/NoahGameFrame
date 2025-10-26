#include "ItemList.hpp"
#include "NpcList.hpp"

#include <game/Constants.hpp>
#include <game/ItemTypes.hpp>
#include <game/MobTypes.hpp>

#include <storm/StringUtil.hpp>

#include <cstdio>
#include <cstring>
#include "spdlog/spdlog.h"
#include <game/Format.hpp>

METIN2_BEGIN_NS

namespace
{

bool ConvertMobType(MobProto& entry, const old::TMobTable& table)
{
	entry.charType = table.bType;

	/*switch (table.bType) {
		case old::CHAR_TYPE_MONSTER: entry.charType = CHAR_TYPE_MONSTER; break;
		case old::CHAR_TYPE_NPC: entry.charType = CHAR_TYPE_NPC; break;
		case old::CHAR_TYPE_STONE: entry.charType = CHAR_TYPE_STONE; break;
		case old::CHAR_TYPE_WARP: entry.charType = CHAR_TYPE_WARP; break;
		case old::CHAR_TYPE_DOOR: entry.charType = CHAR_TYPE_DOOR; break;
		case old::CHAR_TYPE_BUILDING: entry.charType = CHAR_TYPE_BUILDING; break;
		case old::CHAR_TYPE_PC: entry.charType = CHAR_TYPE_PC; break;
		case old::CHAR_TYPE_POLYMORPH_PC: entry.charType = CHAR_TYPE_POLYMORPH_PC; break;
		case old::CHAR_TYPE_HORSE: entry.charType = CHAR_TYPE_HORSE; break;
		case old::CHAR_TYPE_GOTO: entry.charType = CHAR_TYPE_GOTO; break;
		case old::CHAR_TYPE_PET: break;

		default:
			spdlog::error("Failed to convert char-type {0}", table.bType);
			return false;
	}*/

	return true;
}

bool ConvertMobRank(MobProto& entry, const old::TMobTable& table)
{
	switch (table.bRank) {
		case old::MOB_RANK_PAWN: entry.rank = MOB_RANK_PAWN; break;
		case old::MOB_RANK_S_PAWN: entry.rank = MOB_RANK_S_PAWN; break;
		case old::MOB_RANK_KNIGHT: entry.rank = MOB_RANK_KNIGHT; break;
		case old::MOB_RANK_S_KNIGHT: entry.rank = MOB_RANK_S_KNIGHT; break;
		case old::MOB_RANK_BOSS: entry.rank = MOB_RANK_BOSS; break;
		case old::MOB_RANK_KING: entry.rank = MOB_RANK_KING; break;
		case old::MOB_RANK_MAX_NUM: entry.rank = MOB_RANK_MAX_NUM; break;

		default:
			spdlog::error("Failed to convert mob rank {0}", table.bRank);
			return false;
	}

	return true;
}

bool ConvertMobEmpire(MobProto& entry, const old::TMobTable& table)
{
	switch (table.bEmpire) {
		case 0: entry.empire = EMPIRE_NONE; break;
		case 1: entry.empire = EMPIRE_SHINSU; break;
		case 2: entry.empire = EMPIRE_CHUNJO; break;
		case 3: entry.empire = EMPIRE_JINNO; break;

		default:
			spdlog::error("Failed to convert mob empire {0}", table.bEmpire);
			return false;
	}

	return true;
}

bool ConvertMobOnClickType(MobProto& entry, const old::TMobTable& table)
{
	switch (table.bOnClickType) {
		case old::ON_CLICK_NONE: entry.onClickType = ON_CLICK_NONE; break;
		case old::ON_CLICK_SHOP: entry.onClickType = ON_CLICK_SHOP; break;
		case old::ON_CLICK_TALK: entry.onClickType = ON_CLICK_TALK; break;
		case old::ON_CLICK_MAX_NUM: entry.onClickType = ON_CLICK_MAX_NUM; break;

		default:
			spdlog::error("Failed to convert mob on-lick type {0}", table.bOnClickType);
			return false;
	}

	return true;
}

bool ConvertMobBattleType(MobProto& entry, const old::TMobTable& table)
{
	switch (table.bBattleType) {
		case old::BATTLE_TYPE_MELEE: entry.battleType = BATTLE_TYPE_MELEE; break;
		case old::BATTLE_TYPE_RANGE: entry.battleType = BATTLE_TYPE_RANGE; break;
		case old::BATTLE_TYPE_MAGIC: entry.battleType = BATTLE_TYPE_MAGIC; break;
		case old::BATTLE_TYPE_SPECIAL: entry.battleType = BATTLE_TYPE_SPECIAL; break;
		case old::BATTLE_TYPE_POWER: entry.battleType = BATTLE_TYPE_POWER; break;
		case old::BATTLE_TYPE_TANKER: entry.battleType = BATTLE_TYPE_TANKER; break;
		case old::BATTLE_TYPE_SUPER_POWER: entry.battleType = BATTLE_TYPE_SUPER_POWER; break;
		case old::BATTLE_TYPE_SUPER_TANKER: entry.battleType = BATTLE_TYPE_SUPER_TANKER; break;
		case old::BATTLE_TYPE_MAX_NUM: entry.battleType = BATTLE_TYPE_MAX_NUM; break;

		default:
			spdlog::error("Failed to convert mob battle type {0}", table.bBattleType);
			return false;
	}

	return true;
}

void ConvertMobAiFlags(MobProto& entry, const old::TMobTable& table)
{
	entry.aiFlags = table.dwAIFlag;
}

void ConvertMobRaceFlags(MobProto& entry, const old::TMobTable& table)
{
	entry.raceFlags = table.dwRaceFlag;
}

void ConvertMobImmuneFlags(MobProto& entry, const old::TMobTable& table)
{
	if (table.dwImmuneFlag & old::IMMUNE_STUN)
		entry.immuneFlags |= IMMUNE_STUN;
	if (table.dwImmuneFlag & old::IMMUNE_SLOW)
		entry.immuneFlags |= IMMUNE_SLOW;
	if (table.dwImmuneFlag & old::IMMUNE_FALL)
		entry.immuneFlags |= IMMUNE_FALL;
	if (table.dwImmuneFlag & old::IMMUNE_CURSE)
		entry.immuneFlags |= IMMUNE_CURSE;
	if (table.dwImmuneFlag & old::IMMUNE_POISON)
		entry.immuneFlags |= IMMUNE_POISON;
	if (table.dwImmuneFlag & old::IMMUNE_TERROR)
		entry.immuneFlags |= IMMUNE_TERROR;
	if (table.dwImmuneFlag & old::IMMUNE_REFLECT)
		entry.immuneFlags |= IMMUNE_REFLECT;
}

}

bool ConvertMobProto(MobProto& entry, const old::TMobTable& table,
                     NpcList& npcList)
{
	entry.vnum = table.dwVnum;

	if (!ConvertMobType(entry, table) ||
	    !ConvertMobRank(entry, table) ||
	    !ConvertMobEmpire(entry, table) ||
	    !ConvertMobOnClickType(entry, table) ||
	    !ConvertMobBattleType(entry, table))
		return false;

	entry.level = table.bLevel;

	ConvertMobAiFlags(entry, table);
	ConvertMobRaceFlags(entry, table);
	ConvertMobImmuneFlags(entry, table);

	entry.minGold = table.dwGoldMin;
	entry.maxGold = table.dwGoldMax;
	entry.exp = table.dwExp;
	entry.dropItemVnum = table.dwDropItemVnum;
	entry.polymorphItemVnum = table.dwPolymorphItemVnum;

	entry.hp = table.dwMaxHP;
	entry.regenCycle = table.bRegenCycle;
	entry.regenPct = table.bRegenPercent;

	entry.st = table.bStr;
	entry.ht = table.bCon;
	entry.dx = table.bDex;
	entry.iq = table.bInt;
	entry.def = table.wDef;
	entry.moveSpeed = table.sMovingSpeed;
	entry.attackSpeed = table.sAttackSpeed;
	entry.attackRange = table.wAttackRange;
	entry.mountCapacity = table.bMountCapacity;

	entry.minDamage = table.dwDamageRange[0];
	entry.maxDamage = table.dwDamageRange[1];
	entry.damMultiply = table.fDamMultiply;

	entry.resurrectionVnum = table.dwResurrectionVnum;
	entry.summonVnum = table.dwSummonVnum;
	entry.drainSp = table.dwDrainSP;
	entry.aggressiveSight = table.wAggressiveSight;
	entry.aggressiveHpPct = table.bAggresiveHPPct;
	entry.berserkPoint = table.bBerserkPoint;
	entry.stoneSkinPoint = table.bStoneSkinPoint;
	entry.godSpeedPoint = table.bGodSpeedPoint;
	entry.deathBlowPoint = table.bDeathBlowPoint;
	entry.revivePoint = table.bRevivePoint;

	if (table.cEnchants[old::MOB_ENCHANT_CURSE])
		entry.applies.push_back({APPLY_CURSE_PCT, table.cEnchants[old::MOB_ENCHANT_CURSE]});

	if (table.cEnchants[old::MOB_ENCHANT_SLOW])
		entry.applies.push_back({APPLY_SLOW_PCT, table.cEnchants[old::MOB_ENCHANT_SLOW]});

	if (table.cEnchants[old::MOB_ENCHANT_POISON])
		entry.applies.push_back({APPLY_POISON_PCT, table.cEnchants[old::MOB_ENCHANT_POISON]});

	if (table.cEnchants[old::MOB_ENCHANT_STUN])
		entry.applies.push_back({APPLY_STUN_PCT, table.cEnchants[old::MOB_ENCHANT_STUN]});

	if (table.cEnchants[old::MOB_ENCHANT_CRITICAL])
		entry.applies.push_back({APPLY_CRITICAL_PCT, table.cEnchants[old::MOB_ENCHANT_CRITICAL]});

	if (table.cEnchants[old::MOB_ENCHANT_PENETRATE])
		entry.applies.push_back({APPLY_PENETRATE_PCT, table.cEnchants[old::MOB_ENCHANT_PENETRATE]});

	if (table.cResists[old::MOB_RESIST_SWORD])
		entry.applies.push_back({APPLY_RESIST_SWORD, table.cResists[old::MOB_RESIST_SWORD]});

	if (table.cResists[old::MOB_RESIST_TWOHAND])
		entry.applies.push_back({APPLY_RESIST_TWOHAND, table.cResists[old::MOB_RESIST_TWOHAND]});

	if (table.cResists[old::MOB_RESIST_DAGGER])
		entry.applies.push_back({APPLY_RESIST_DAGGER, table.cResists[old::MOB_RESIST_DAGGER]});

	if (table.cResists[old::MOB_RESIST_BELL])
		entry.applies.push_back({APPLY_RESIST_BELL, table.cResists[old::MOB_RESIST_BELL]});

	if (table.cResists[old::MOB_RESIST_FAN])
		entry.applies.push_back({APPLY_RESIST_FAN, table.cResists[old::MOB_RESIST_FAN]});

	if (table.cResists[old::MOB_RESIST_BOW])
		entry.applies.push_back({APPLY_RESIST_BOW, table.cResists[old::MOB_RESIST_BOW]});

	if (table.cResists[old::MOB_RESIST_FIRE])
		entry.applies.push_back({APPLY_RESIST_FIRE, table.cResists[old::MOB_RESIST_FIRE]});

	if (table.cResists[old::MOB_RESIST_ELECT])
		entry.applies.push_back({APPLY_RESIST_ELEC, table.cResists[old::MOB_RESIST_ELECT]});

	if (table.cResists[old::MOB_RESIST_MAGIC])
		entry.applies.push_back({APPLY_RESIST_MAGIC, table.cResists[old::MOB_RESIST_MAGIC]});

	if (table.cResists[old::MOB_RESIST_WIND])
		entry.applies.push_back({APPLY_RESIST_WIND, table.cResists[old::MOB_RESIST_WIND]});

	if (table.cResists[old::MOB_RESIST_POISON])
		entry.applies.push_back({APPLY_POISON_REDUCE, table.cResists[old::MOB_RESIST_POISON]});

	entry.color = table.dwMonsterColor;

	const auto* msm = npcList.Get(entry.vnum);
	if (msm) {
		entry.msmFilename = *msm;
	}

	for (int i = 0; i < MOB_SKILL_MAX_NUM && i < old::MOB_SKILL_MAX_NUM; ++i) {
		entry.skills[i].vnum = table.Skills[i].dwVnum;
		entry.skills[i].level = table.Skills[i].bLevel;
	}

	return true;
}

METIN2_END_NS
