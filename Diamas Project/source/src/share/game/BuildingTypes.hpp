#ifndef METIN2_GAME_BUILDINGTYPES_HPP
#define METIN2_GAME_BUILDINGTYPES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/BuildingConstants.hpp>
#include <game/game_type.h>

#include <xml/Types.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <unordered_map>

METIN2_BEGIN_NS

typedef struct SLand
{
	uint32_t id;
	int32_t mapIndex;
	int32_t x, y;
	int32_t width, height;
	uint32_t guildId;
	uint8_t guildLevelLimit;
	Gold price;
} TLand;

struct ObjectMaterial
{
	uint32_t itemVnum;
	uint32_t count;
};

struct ObjectProto
{
	static const uint32_t kVersion = 2;

	uint32_t vnum;
	uint32_t type;
	std::string name;

	Gold price;
	ObjectMaterial materials[OBJECT_MATERIAL_MAX_NUM];

	int32_t life;
	int32_t region[4];

	uint32_t npcVnum;
	uint32_t groupVnum; // 같은 그룹은 하나만 건설가능
	uint32_t dependOnGroupVnum; // 지어져 있어야하는 그룹

	std::string msmFilename;

	float rotationLimits[3]; // xyz
	bool allowPlacement;
};

typedef struct SObject
{
	uint32_t id;
	uint32_t landId;
	uint32_t vnum;
	int32_t mapIndex;
	int32_t x, y;

	float xRot;
	float yRot;
	float zRot;
	int32_t life;
} TObject;

struct GuildObject
{
  uint32_t x;
  uint32_t y;
  uint32_t vid;
  uint32_t vnum;
  float zRot;
  float xRot;
  float yRot;
};


bool ParseProto(ObjectProto& entry, const XmlNode* node);
XmlNode* FormatProto(const ObjectProto& entry, XmlMemoryPool* pool);

METIN2_END_NS

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::SLand,
	id,
	mapIndex,
	x,
	y,
	width,
	height,
	guildId,
	guildLevelLimit,
	price
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::ObjectMaterial,
	itemVnum,
	count
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::ObjectProto,
	vnum,
	type,
	name,
	price,
	materials,
	life,
	region,
	npcVnum,
	groupVnum,
	dependOnGroupVnum,
	msmFilename,
	rotationLimits,
	allowPlacement
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::SObject,
	id,
	landId,
	vnum,
	mapIndex,
	x,
	y,
	xRot,
	yRot,
	zRot,
	life
);

#endif
