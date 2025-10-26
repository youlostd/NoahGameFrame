#ifndef METIN2_GAME_CUBECONSTANTS_HPP
#define METIN2_GAME_CUBECONSTANTS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

enum ECubeResultCategory
{
	CUBE_CATEGORY_POTION,
	CUBE_CATEGORY_WEAPON,
	CUBE_CATEGORY_ARMOR,
	CUBE_CATEGORY_ACCESSORY,
	CUBE_CATEGORY_ETC,
};

bool GetCubeCategoryString(storm::StringRef& s, uint32_t val);
bool GetCubeCategoryValue(const storm::StringRef& s, uint32_t& val);



#endif // METIN2_GAME_CUBECONSTANTS_HPP
