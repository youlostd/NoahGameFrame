#ifndef METIN2_GAME_PROTOBINARY_HPP
#define METIN2_GAME_PROTOBINARY_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/ClientDb.hpp>

#include <storm/String.hpp>

#include <vector>

METIN2_BEGIN_NS

template <typename T>
struct ProtoClientDbTraits
{
	ProtoClientDbTraits(const std::vector<T>& items)
		: items(items)
	{
		// ctor
	}

	uint32_t GetVersion() const { return T::kVersion; }
	uint32_t GetSize() const { return items.size(); }

	template <class OutputStream>
	bool Serialize(OutputStream& stream)
	{
		bsys::error_code ec;
		WriteExact(stream, items.data(), items.size() * sizeof(T), ec);

		if (ec) {
			spdlog::error("Failed to write {0} items", items.size());
			return false;
		}

		return true;
	}

	const std::vector<T>& items;
};

template <typename T>
bool WriteSimpleProto(const storm::StringRef& filename,
                      const std::vector<T>& data)
{
	return SaveClientDb<T>(filename, data);
}

METIN2_END_NS

#endif
