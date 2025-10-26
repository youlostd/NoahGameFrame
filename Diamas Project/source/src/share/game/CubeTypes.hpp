#ifndef METIN2_GAME_CUBETYPES_HPP
#define METIN2_GAME_CUBETYPES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#include <game/CubeConstants.hpp>

#include <game/Types.hpp>

#include <xml/Types.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <vector>

METIN2_BEGIN_NS

#pragma pack(push, 1)

struct CubeItem
{
	ItemVnum vnum;
	CountType count;
	uint8_t transferBonus;
	uint8_t removeAtFailure;
};

struct CubeProto
{
	static const uint32_t kVersion = 1;

	uint32_t vnum;
	uint8_t category;
	uint8_t probability;
	Gold cost;

	std::vector<CubeItem> items;
	CubeItem reward;
};

#pragma pack(pop)

bool ParseProto(CubeProto& entry, const XmlNode* node);
XmlNode* FormatProto(const CubeProto& entry, XmlMemoryPool* pool);

METIN2_END_NS

BOOST_FUSION_ADAPT_STRUCT(
	CubeItem,
	vnum,
	count,
	transferBonus,
	removeAtFailure
);

BOOST_FUSION_ADAPT_STRUCT(
	CubeProto,
	vnum,
	category,
	probability,
	cost,
	items,
	reward
);


#endif // METIN2_GAME_CUBETYPES_HPP
