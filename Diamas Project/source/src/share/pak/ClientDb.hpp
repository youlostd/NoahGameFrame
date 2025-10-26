#ifndef METIN2_VFS_CLIENTDB_HPP
#define METIN2_VFS_CLIENTDB_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <pak/Vfs.hpp>

#include <base/ClientDb.hpp>

#include <storm/io/View.hpp>

METIN2_BEGIN_NS

template <class T>
bool LoadClientDb(Vfs& vfs, std::string_view filename,
                  std::vector<T>& data)
{
	auto fp = GetVfs().Open(filename, kVfsOpenFullyBuffered);
	if (!fp) {
		SPDLOG_ERROR("Failed to open {0}", filename);
		return false;
	}

	const auto size = fp->GetSize();

	storm::View v(storm::GetDefaultAllocator());
	if (!fp->GetView(0, v, size)) {
		SPDLOG_ERROR("{0}: Failed to view {1} bytes", filename, size);
		return false;
	}

	return LoadClientDb<T>(asio::const_buffer(v.GetData(), size), data);
}

METIN2_END_NS

#endif
