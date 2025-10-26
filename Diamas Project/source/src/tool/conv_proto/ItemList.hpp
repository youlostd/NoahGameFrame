#ifndef METIN2_TOOL_CONVPROTO_ITEMLIST_HPP
#define METIN2_TOOL_CONVPROTO_ITEMLIST_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Types.hpp>

#include <storm/String.hpp>

#include <unordered_map>

METIN2_BEGIN_NS

struct ItemListEntry
{
	ItemListEntry();

	uint32_t vnum;
	std::string iconFilename;
	std::string modelFilename;
};

class ItemList
{
	public:
		ItemList();

		bool Load(const storm::StringRef& filename);

		const ItemListEntry* Get(uint32_t vnum) const;

	private:
		typedef std::unordered_map<uint32_t, ItemListEntry> ItemMap;

		bool ParseLine(const storm::String& line);

		ItemMap m_entries;
};

METIN2_END_NS

#endif
