#ifndef METIN2_TOOL_CONVPROTO_NPCLIST_HPP
#define METIN2_TOOL_CONVPROTO_NPCLIST_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Types.hpp>

#include <storm/String.hpp>

#include <unordered_map>

METIN2_BEGIN_NS

class NpcList
{
	public:
		NpcList();

		bool Load(const storm::StringRef& filename);

		const std::string* Get(uint32_t vnum) const;

	private:
		typedef std::unordered_map<uint32_t, std::string> NpcMap;

		NpcMap m_entries;
};

METIN2_END_NS

#endif
