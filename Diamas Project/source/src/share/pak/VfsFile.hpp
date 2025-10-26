#ifndef METIN2_VFS_VFSFILE_HPP
#define METIN2_VFS_VFSFILE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/io/View.hpp>

METIN2_BEGIN_NS

/// An opened VFS file.
///
/// This class represents the user-visible interface
/// for an opened file. It provides several methods
/// to read the file's (meta)data.
class VfsFile
{
	public:
		virtual ~VfsFile() {}

		virtual uint32_t GetSize() const = 0;

		virtual bool Read(uint32_t offset,
		                  void* buffer, uint32_t bytes) const = 0;

		virtual bool GetView(uint32_t offset, storm::View& view,
		                     uint32_t bytes) const = 0;
};

METIN2_END_NS

#endif
