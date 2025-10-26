#ifndef METIN2_GAME_BUILDINGCONSTANTS_HPP
#define METIN2_GAME_BUILDINGCONSTANTS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

METIN2_BEGIN_NS

enum BuildingLimits
{
	OBJECT_MATERIAL_MAX_NUM = 5,
};

enum BuildingType
{
	BUILDING_HEADQUARTERS,
	BUILDING_FACILITY,
	BUILDING_OBJECT,
	BUILDING_WALL,
	BUILDING_BUILD_IN,
};

bool GetBuildingTypeString(storm::StringRef& s, uint32_t val);
bool GetBuildingTypeValue(const storm::StringRef& s, uint32_t& val);

METIN2_END_NS

#endif
