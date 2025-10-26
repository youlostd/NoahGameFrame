#ifndef METIN2_VFS_DISKVFSFILE_HPP
#define METIN2_VFS_DISKVFSFILE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <pak/VfsFile.hpp>

#include <storm/io/File.hpp>

METIN2_BEGIN_NS

class DiskVfsFile : public VfsFile
{
	public:
		virtual ~DiskVfsFile();

		bool Open(const storm::StringRef& path);

		virtual uint32_t GetSize() const;

		virtual bool Read(uint32_t offset,
		                  void* buffer,
		                  uint32_t bytes) const;

		virtual bool GetView(uint32_t offset,
		                     storm::View& view,
		                     uint32_t bytes) const;

	private:
		mutable storm::File m_file;
};

METIN2_END_NS

#endif
