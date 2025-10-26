#ifndef METIN2_VFS_DISKVFSPROVIDER_HPP
#define METIN2_VFS_DISKVFSPROVIDER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <pak/VfsProvider.hpp>

METIN2_BEGIN_NS

class GroupTextGroup;
class VfsFile;

class DiskVfsProvider : public VfsProvider
{
	public:
		DiskVfsProvider();

		virtual bool LoadConfig(const GroupTextGroup& root);

		virtual bool DoesFileExist(const storm::String& path);

		virtual std::unique_ptr<VfsFile> OpenFile(std::string_view path,
		                                          uint32_t flags);
};

METIN2_END_NS

#endif
