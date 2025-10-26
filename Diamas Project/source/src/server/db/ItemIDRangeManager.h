// vim:ts=4 sw=4
#ifndef __INC_METIN_II_ITEM_ID_RANGE_MANAGER_H__
#define __INC_METIN_II_ITEM_ID_RANGE_MANAGER_H__
#include <game/Constants.hpp>
#include <game/ItemConstants.hpp>
#include <game/ItemTypes.hpp>

#include <base/Singleton.hpp>

bool DoRangesCollide(const ItemIdRange& a, const ItemIdRange& b);


class CItemIDRangeManager : public singleton<CItemIDRangeManager>
{
public:
	void Boot();

	bool BuildRange(uint32_t min, uint32_t max, ItemIdRange& range);

	bool GetRange(ItemIdRange& range);
	void ReturnRange(const ItemIdRange& range);

private:
	// List of all ranges we can assign
	std::list<ItemIdRange> m_ranges;
};

#endif
