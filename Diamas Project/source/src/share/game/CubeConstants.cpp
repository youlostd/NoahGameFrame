#include <game/CubeConstants.hpp>

#include <storm/StringFlags.hpp>

typedef storm::StringValueTable<uint32_t> UInt32Table;

UInt32Table kCubeCategories[] = {
	{"POTION", CUBE_CATEGORY_POTION},
	{"WEAPON", CUBE_CATEGORY_WEAPON},
	{"ARMOR", CUBE_CATEGORY_ARMOR},
	{"ACCESSORY", CUBE_CATEGORY_ACCESSORY},
	{"ETC", CUBE_CATEGORY_ETC},

};

bool GetCubeCategoryString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kCubeCategories); }

bool GetCubeCategoryValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kCubeCategories); }
