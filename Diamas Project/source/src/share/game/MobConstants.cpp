#include "MobConstants.hpp"

#include <storm/StringFlags.hpp>

#include <game/Constants.hpp>

typedef storm::StringValueTable<uint32_t> UInt32Table;

UInt32Table kBattleTypes[] = {
	{"MELEE", BATTLE_TYPE_MELEE},
	{"RANGE", BATTLE_TYPE_RANGE},
	{"MAGIC", BATTLE_TYPE_MAGIC},
	{"SPECIAL", BATTLE_TYPE_SPECIAL},
	{"POWER", BATTLE_TYPE_POWER},
	{"TANKER", BATTLE_TYPE_TANKER},
	{"SUPER_POWER", BATTLE_TYPE_SUPER_POWER},
	{"SUPER_TANKER", BATTLE_TYPE_SUPER_TANKER},
};

UInt32Table kMobRanks[] = {
	{"PAWN", MOB_RANK_PAWN},
	{"S_PAWN", MOB_RANK_S_PAWN},
	{"KNIGHT", MOB_RANK_KNIGHT},
	{"S_KNIGHT", MOB_RANK_S_KNIGHT},
	{"BOSS", MOB_RANK_BOSS},
	{"KING", MOB_RANK_KING},
	{"LEGEND", MOB_RANK_LEGENDARY},
};

UInt32Table kMobEnchantTypes[] = {
	{"CURSE", APPLY_CURSE_PCT},
	{"SLOW", APPLY_SLOW_PCT},
	{"POISON", APPLY_POISON_PCT},
	{"STUN", APPLY_STUN_PCT},
	{"CRITICAL", APPLY_CRITICAL_PCT},
	{"PENETRATE", APPLY_PENETRATE_PCT},
};

UInt32Table kMobResistTypes[] = {
	{"SWORD", APPLY_RESIST_SWORD},
	{"TWOHAND", APPLY_RESIST_TWOHAND},
	{"DAGGER", APPLY_RESIST_DAGGER},
	{"BELL", APPLY_RESIST_BELL},
	{"FAN", APPLY_RESIST_FAN},
	{"BOW", APPLY_RESIST_BOW},
	{"FIRE", APPLY_RESIST_FIRE},
	{"ELECT", APPLY_RESIST_ELEC},
	{"MAGIC", APPLY_RESIST_MAGIC},
	{"WIND", APPLY_RESIST_WIND},
	{"POISON", APPLY_POISON_REDUCE},
};

UInt32Table kOnClickTypes[] = {
	{"NONE", ON_CLICK_NONE},
	{"SHOP", ON_CLICK_SHOP},
	{"TALK", ON_CLICK_TALK},
};

UInt32Table kAiFlags[] = {
	{"AGGRESSIVE", AIFLAG_AGGRESSIVE},
	{"NOMOVE", AIFLAG_NOMOVE},
	{"COWARD", AIFLAG_COWARD},
	{"NOATTACKSHINSU", AIFLAG_NOATTACKSHINSU},
	{"NOATTACKJINNO", AIFLAG_NOATTACKJINNO},
	{"NOATTACKCHUNJO", AIFLAG_NOATTACKCHUNJO},
	{"ATTACKMOB", AIFLAG_ATTACKMOB},
	{"BERSERK", AIFLAG_BERSERK},
	{"STONESKIN", AIFLAG_STONESKIN},
	{"GODSPEED", AIFLAG_GODSPEED},
	{"DEATHBLOW", AIFLAG_DEATHBLOW},
	{"REVIVE", AIFLAG_REVIVE},
	{ "HEALER", AIFLAG_HEALER },
	{ "COUNT", AIFLAG_COUNT },
	{ "NORECOVERY", AIFLAG_NORECOVERY },
	{ "REFLECT", AIFLAG_REFLECT },
	{ "FALL", AIFLAG_FALL },
	{ "VIT", AIFLAG_VIT },
	{ "RATTSPEED", AIFLAG_RATTSPEED },
	{ "RCASTSPEED", AIFLAG_RCASTSPEED },
	{ "RHP_REGEN", AIFLAG_RHP_REGEN },
	{ "TIMEVIT", AIFLAG_TIMEVIT },
	{ "NOPUSH", AIFLAG_NOPUSH },
};

UInt32Table kRaceFlags[] = {
	{"ANIMAL", RACE_FLAG_ANIMAL},
	{"UNDEAD", RACE_FLAG_UNDEAD},
	{"DEVIL", RACE_FLAG_DEVIL},
	{"HUMAN", RACE_FLAG_HUMAN},
	{"ORC", RACE_FLAG_ORC},
	{"MILGYO", RACE_FLAG_MILGYO},
	{"INSECT", RACE_FLAG_INSECT},
	{"FIRE", RACE_FLAG_FIRE},
	{"ICE", RACE_FLAG_ICE},
	{"DESERT", RACE_FLAG_DESERT},
	{"TREE", RACE_FLAG_TREE},
	{"ATT_ELEC", RACE_FLAG_ATT_ELEC},
	{"ATT_FIRE", RACE_FLAG_ATT_FIRE},
	{"ATT_ICE", RACE_FLAG_ATT_ICE},
	{"ATT_WIND", RACE_FLAG_ATT_WIND},
	{"ATT_EARTH", RACE_FLAG_ATT_EARTH},
	{"ATT_DARK", RACE_FLAG_ATT_DARK},
	{"CZ", RACE_FLAG_CZ},
	{"SHADOW", RACE_FLAG_SHADOW},


};

bool GetBattleTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kBattleTypes); }

bool GetBattleTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kBattleTypes); }

bool GetMobRankString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kMobRanks); }

bool GetMobRankValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kMobRanks); }

bool GetMobEnchantTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kMobEnchantTypes); }

bool GetMobResistTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kMobResistTypes); }

bool GetOnClickTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kOnClickTypes); }

bool GetOnClickTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kOnClickTypes); }

bool GetAiFlagsString(storm::String& s, uint32_t val)
{ return storm::FormatBitflagWithTable(s, val, kAiFlags); }

bool GetAiFlagsValue(const storm::String& s, uint32_t& val)
{ return storm::ParseBitflagStringWithTable(s, val, kAiFlags); }

bool GetRaceFlagsString(storm::String& s, uint32_t val)
{ return storm::FormatBitflagWithTable(s, val, kRaceFlags); }

bool GetRaceFlagsValue(const storm::String& s, uint32_t& val)
{ return storm::ParseBitflagStringWithTable(s, val, kRaceFlags); }


