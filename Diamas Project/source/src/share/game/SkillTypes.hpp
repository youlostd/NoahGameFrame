#ifndef METIN2_GAME_SKILLTYPES_HPP
#define METIN2_GAME_SKILLTYPES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/SkillConstants.hpp>

#include <xml/Types.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <vector>

METIN2_BEGIN_NS

#pragma pack(push, 1)

struct SkillRequirement
{
	uint32_t skillVnum;
	uint8_t skillLevel;
};

struct SkillProto
{
	static const uint32_t kVersion = 1;

	uint32_t vnum;
	storm::String name;

	uint8_t type;
	uint8_t attrType;
	uint32_t flags;

	storm::String cooltime;

	storm::String levelStep;
	uint8_t maxLevel;
	uint8_t levelLimit;

	storm::String spCost;
	storm::String grandMasterSpCost;

	uint32_t targetRange;
	uint32_t maxHitCount;
	uint32_t splashRange;
	storm::String splashDamage;


	uint8_t point;
	uint8_t affectFlag;
	storm::String value;
	storm::String masterValue;
	storm::String duration;
	storm::String durationSpCost;

	uint8_t point2;
	uint8_t affectFlag2;
	storm::String value2;
	storm::String duration2;

	uint8_t point3;
	uint8_t affectFlag3;
	storm::String value3;
	storm::String duration3;

	std::vector<SkillRequirement> requirements;
};

#pragma pack(pop)

bool ParseProto(SkillProto& entry, const XmlNode* node);
XmlNode* FormatProto(const SkillProto& entry, XmlMemoryPool* pool);

METIN2_END_NS

BOOST_FUSION_ADAPT_STRUCT(
	SkillRequirement,
	skillVnum,
	skillLevel
)

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::SkillProto,
	vnum,
	name,
	type,
	attrType,
	flags,
	cooltime,
	levelStep,
	maxLevel,
	levelLimit,
	spCost,
	grandMasterSpCost,
	targetRange,
	maxHitCount,
	splashRange,
	splashDamage,
	point,
	affectFlag,
	value,
	masterValue,
	duration,
	durationSpCost,
	point2,
	affectFlag2,
	value2,
	duration2,
	point3,
	affectFlag3,
	value3,
	duration3,
	requirements
);

#endif // METIN2_GAME_SKILLTYPES_HPP
