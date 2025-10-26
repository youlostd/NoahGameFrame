#include <game/BuildingConstants.hpp>

#include <storm/StringFlags.hpp>

METIN2_BEGIN_NS

typedef storm::StringValueTable<uint32_t> UInt32Table;

UInt32Table kBuildingTypes[] = {
	{"HEADQUARTERS", BUILDING_HEADQUARTERS},
	{"FACILITY", BUILDING_FACILITY},
	{"OBJECT", BUILDING_OBJECT},
	{"WALL", BUILDING_WALL},
	{"BUILD_IN", BUILDING_BUILD_IN},
};

bool GetBuildingTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kBuildingTypes); }

bool GetBuildingTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kBuildingTypes); }

METIN2_END_NS

