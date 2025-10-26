#ifndef METIN2_VFS_VFS_HPP
#define METIN2_VFS_VFS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <pak/VfsFile.hpp>
#include <storm/String.hpp>
#include <memory>

METIN2_BEGIN_NS

class VfsProvider;

// Flags controlling buffering
// Default: automatic (left up to implementation)
static const uint32_t kVfsOpenUnbuffered = 1 << 0;
static const uint32_t kVfsOpenFullyBuffered = 1 << 1;

/// A [read-only] Virtual File System (VFS).
///
/// This class is the user-visible part of the VFS implementation.
/// It delegates its work to VfsProviders, which implement the real VFS.
/// This delegation allows the VFS consumer to be independent of the actual
/// VFS in use and thus makes switching implementations easier.
class Vfs
{
	public:
		void RegisterProvider(VfsProvider* provider);


		bool Exists(const std::string& path);

		/// Open a file for reading.
		std::unique_ptr<VfsFile> Open(std::string_view path,
		                              uint32_t flags = kVfsOpenFullyBuffered);


	private:
		std::vector<VfsProvider*> m_providers;
};

Vfs& GetVfs();
void SetVfs(Vfs* vfs);

METIN2_END_NS

#endif
