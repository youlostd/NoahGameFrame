#include <game/SkillConstants.hpp>

#include <storm/StringFlags.hpp>

#include <boost/algorithm/clamp.hpp>

METIN2_BEGIN_NS

typedef storm::StringValueTable<uint32_t> UInt32Table;

UInt32Table kSkillTypes[] = {
	{"SECONDARY", SKILL_TYPE_SECONDARY},
	{"WARRIOR", SKILL_TYPE_WARRIOR},
	{"ASSASSIN", SKILL_TYPE_ASSASSIN},
	{"SURA", SKILL_TYPE_SURA},
	{"SHAMAN", SKILL_TYPE_SHAMAN},
	{"WOLFMAN", SKILL_TYPE_WOLFMAN},
	{"HORSE", SKILL_TYPE_HORSE},
	{"COMMON", SKILL_TYPE_COMMON},
	{"SKILLTREE", SKILL_TYPE_SKILLTREE},
};

UInt32Table kSkillAttrTypes[] = {
	{"NONE", SKILL_ATTR_TYPE_NONE},
	{"NORMAL", SKILL_ATTR_TYPE_NORMAL},
	{"MELEE", SKILL_ATTR_TYPE_MELEE},
	{"RANGE", SKILL_ATTR_TYPE_RANGE},
	{"MAGIC", SKILL_ATTR_TYPE_MAGIC},
};

UInt32Table kSkillFlags[] = {
	{"ATTACK", SKILL_FLAG_ATTACK},
	{"USE_MELEE_DAMAGE", SKILL_FLAG_USE_MELEE_DAMAGE},
	{"COMPUTE_ATTGRADE", SKILL_FLAG_COMPUTE_ATTGRADE},
	{"SELFONLY", SKILL_FLAG_SELFONLY},
	{"USE_MAGIC_DAMAGE", SKILL_FLAG_USE_MAGIC_DAMAGE},
	{"USE_HP_AS_COST", SKILL_FLAG_USE_HP_AS_COST},
	{"COMPUTE_MAGIC_DAMAGE", SKILL_FLAG_COMPUTE_MAGIC_DAMAGE},
	{"SPLASH", SKILL_FLAG_SPLASH},
	{"GIVE_PENALTY", SKILL_FLAG_GIVE_PENALTY},
	{"USE_ARROW_DAMAGE", SKILL_FLAG_USE_ARROW_DAMAGE},
	{"PENETRATE", SKILL_FLAG_PENETRATE},
	{"IGNORE_TARGET_RATING", SKILL_FLAG_IGNORE_TARGET_RATING},
	{"SLOW", SKILL_FLAG_SLOW},
	{"STUN", SKILL_FLAG_STUN},
	{"HP_ABSORB", SKILL_FLAG_HP_ABSORB},
	{"SP_ABSORB", SKILL_FLAG_SP_ABSORB},
	{"FIRE_CONT", SKILL_FLAG_FIRE_CONT},
	{"REMOVE_BAD_AFFECT", SKILL_FLAG_REMOVE_BAD_AFFECT},
	{"REMOVE_GOOD_AFFECT", SKILL_FLAG_REMOVE_GOOD_AFFECT},
	{"CRUSH", SKILL_FLAG_CRUSH},
	{"POISON", SKILL_FLAG_POISON},
	{"TOGGLE", SKILL_FLAG_TOGGLE},
	{"DISABLE_BY_POINT_UP", SKILL_FLAG_DISABLE_BY_POINT_UP},
	{"CRUSH_LONG", SKILL_FLAG_CRUSH_LONG},
	{"WIND", SKILL_FLAG_WIND},
	{"ELEC", SKILL_FLAG_ELEC},
	{"FIRE", SKILL_FLAG_FIRE},
	{"BLEEDING", SKILL_FLAG_BLEEDING},
	{"PARTY", SKILL_FLAG_PARTY},
	{"PARTY_AND_ALL", SKILL_FLAG_PARTY_AND_ALL},
};

static const uint32_t kSkillPowerByLevel[SKILL_MAX_LEVEL + 1] = {
    0, //
    5, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 50, // normal
    52, 54, 56, 58, 60, 63, 66, 69, 72, 82, // master
    85, 88, 91, 94, 98, 102, 106, 110, 115, // grand master
    125, 126, 128, 130, 132, 134, 137, 140, 143, 146,  // perfect master
    150, // legendary master
};

bool GetSkillTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kSkillTypes); }

bool GetSkillTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kSkillTypes); }

bool GetSkillAttrTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kSkillAttrTypes); }

bool GetSkillAttrTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kSkillAttrTypes); }

bool GetSkillFlagsString(storm::String& s, uint32_t val)
{ return storm::FormatBitflagWithTable(s, val, kSkillFlags); }

bool GetSkillFlagsValue(const storm::String& s, uint32_t& val)
{ return storm::ParseBitflagStringWithTable(s, val, kSkillFlags); }

uint32_t GetSkillPowerByLevel(uint32_t level)
{
	return kSkillPowerByLevel[std::clamp<uint32_t>(level, 0, SKILL_MAX_LEVEL)];
}

uint32_t GetMasterTypeFromLevel(uint32_t level)
{
	if (level >= 50)
		return SKILL_LEGENDARY_MASTER;

	if (level >= 40)
		return SKILL_PERFECT_MASTER;

	if (level >= 30)
		return SKILL_GRAND_MASTER;

	if (level >= 20)
		return SKILL_MASTER;

	return SKILL_NORMAL;
}

uint32_t GetRelativeSkillLevel(uint32_t grade, uint32_t level)
{
	switch (grade) {
		case SKILL_NORMAL:
			return level;

		case SKILL_MASTER:
			return level - 20 + 1;

		case SKILL_GRAND_MASTER:
			return level - 30 + 1;

		case SKILL_PERFECT_MASTER:
			return level - 40 + 1;
			
		case SKILL_LEGENDARY_MASTER:
			return level - 50 + 1;
	}

	return level;
}

METIN2_END_NS
