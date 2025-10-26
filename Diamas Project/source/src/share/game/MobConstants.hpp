#ifndef METIN2_GAME_MOBCONSTANTS_HPP
#define METIN2_GAME_MOBCONSTANTS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>



enum EMobRank
{
	MOB_RANK_PAWN,
	MOB_RANK_S_PAWN,
	MOB_RANK_KNIGHT,
	MOB_RANK_S_KNIGHT,
	MOB_RANK_BOSS,
	MOB_RANK_KING,
	MOB_RANK_LEGENDARY,
	MOB_RANK_MAX_NUM
};

enum EBattleType
{
	BATTLE_TYPE_MELEE,
	BATTLE_TYPE_RANGE,
	BATTLE_TYPE_MAGIC,
	BATTLE_TYPE_SPECIAL,
	BATTLE_TYPE_POWER,
	BATTLE_TYPE_TANKER,
	BATTLE_TYPE_SUPER_POWER,
	BATTLE_TYPE_SUPER_TANKER,
	BATTLE_TYPE_MAX_NUM
};

enum EOnClickEvents
{
	ON_CLICK_NONE,
	ON_CLICK_SHOP,
	ON_CLICK_TALK,
	ON_CLICK_MAX_NUM
};

enum ERaceFlags
{
	RACE_FLAG_ANIMAL	= (1 << 0),
	RACE_FLAG_UNDEAD	= (1 << 1),
	RACE_FLAG_DEVIL		= (1 << 2),
	RACE_FLAG_HUMAN		= (1 << 3),
	RACE_FLAG_ORC		= (1 << 4),
	RACE_FLAG_MILGYO	= (1 << 5),
	RACE_FLAG_INSECT	= (1 << 6),
	RACE_FLAG_FIRE		= (1 << 7),
	RACE_FLAG_ICE		= (1 << 8),
	RACE_FLAG_DESERT	= (1 << 9),
	RACE_FLAG_TREE		= (1 << 10),
	RACE_FLAG_ATT_ELEC	= (1 << 11),
	RACE_FLAG_ATT_FIRE	= (1 << 12),
	RACE_FLAG_ATT_ICE	= (1 << 13),
	RACE_FLAG_ATT_WIND	= (1 << 14),
	RACE_FLAG_ATT_EARTH	= (1 << 15),
	RACE_FLAG_ATT_DARK	= (1 << 16),
	RACE_FLAG_CZ	= (1 << 17),
	RACE_FLAG_SHADOW	= (1 << 18),

	RACE_FLAG_MAX_NUM	= 18,

};

enum EAIFlags
{
	AIFLAG_AGGRESSIVE = (1 << 0),
	AIFLAG_NOMOVE = (1 << 1),
	AIFLAG_COWARD = (1 << 2),
	AIFLAG_NOATTACKSHINSU = (1 << 3),
	AIFLAG_NOATTACKJINNO = (1 << 4),
	AIFLAG_NOATTACKCHUNJO = (1 << 5),
	AIFLAG_ATTACKMOB = (1 << 6),
	AIFLAG_BERSERK = (1 << 7),
	AIFLAG_STONESKIN = (1 << 8),
	AIFLAG_GODSPEED = (1 << 9),
	AIFLAG_DEATHBLOW = (1 << 10),
	AIFLAG_REVIVE = (1 << 11),
	AIFLAG_HEALER = (1 << 12),
	AIFLAG_COUNT = (1 << 13), // Attack Count instead of damage
	AIFLAG_NORECOVERY = (1 << 14), // Disables recovery
	AIFLAG_REFLECT = (1 << 15), // 20% Reflect on attacks
	AIFLAG_FALL = (1 << 16), // Gives affect AFFECT_MOUNT_FALL to players
	AIFLAG_VIT = (1 << 17), // Unknown
	AIFLAG_RATTSPEED = (1 << 18), // Gives affect AFFECT_ATT_SPEED_SLOW to players
	AIFLAG_RCASTSPEED = (1 << 19), // Gives affect AFFECT_REDUCE_CAST_SPEED to players
	AIFLAG_RHP_REGEN = (1 << 20), // Gives affect AFFECT_NO_RECOVERY to players
	AIFLAG_TIMEVIT = (1 << 21), // Unknown
	AIFLAG_NOPUSH = (1 << 22),

};


enum EMobResists
{
	MOB_RESIST_SWORD,
	MOB_RESIST_TWOHAND,
	MOB_RESIST_DAGGER,
	MOB_RESIST_BELL,
	MOB_RESIST_FAN,
	MOB_RESIST_BOW,
	MOB_RESIST_FIRE,
	MOB_RESIST_ELECT,
	MOB_RESIST_MAGIC,
	MOB_RESIST_WIND,
	MOB_RESIST_POISON,
	MOB_RESIST_BLEEDING,
	MOB_RESISTS_MAX_NUM
};


enum EMobEnchants
{
	MOB_ENCHANT_CURSE,
	MOB_ENCHANT_SLOW,
	MOB_ENCHANT_POISON,
	MOB_ENCHANT_STUN,
	MOB_ENCHANT_CRITICAL,
	MOB_ENCHANT_PENETRATE,
	MOB_ENCHANTS_MAX_NUM
};

bool GetBattleTypeString(storm::StringRef& s, uint32_t val);
bool GetBattleTypeValue(const storm::StringRef& s, uint32_t& val);

bool GetMobRankString(storm::StringRef& s, uint32_t val);
bool GetMobRankValue(const storm::StringRef& s, uint32_t& val);

bool GetMobEnchantTypeString(storm::StringRef& s, uint32_t val);
bool GetMobEnchantTypeValue(const storm::StringRef& s, uint32_t& val);

bool GetMobResistTypeString(storm::StringRef& s, uint32_t val);
bool GetMobResistTypeValue(const storm::StringRef& s, uint32_t& val);

bool GetOnClickTypeString(storm::StringRef& s, uint32_t val);
bool GetOnClickTypeValue(const storm::StringRef& s, uint32_t& val);

bool GetAiFlagsString(storm::String& s, uint32_t val);
bool GetAiFlagsValue(const storm::String& s, uint32_t& val);

bool GetRaceFlagsString(storm::String& s, uint32_t val);
bool GetRaceFlagsValue(const storm::String& s, uint32_t& val);



#endif // METIN2_GAME_MOBCONSTANTS_HPP
