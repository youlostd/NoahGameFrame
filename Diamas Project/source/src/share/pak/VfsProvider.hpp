#ifndef METIN2_VFS_VFSPROVIDER_HPP
#define METIN2_VFS_VFSPROVIDER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <memory>

METIN2_BEGIN_NS

class GroupTextGroup;
class VfsFile;

/// A @c VfsProvider provides the actual VFS implementation.
///
/// The user-visible VFS consists of several VfsProviders,
/// which usually implement different VFS types (BIG, EterPack, ZIP, Raw Disk).
class VfsProvider
{
	public:
		VfsProvider(const storm::StringRef& name);
		virtual ~VfsProvider() {}

		const storm::StringRef& GetName() const;

		virtual bool LoadConfig(const GroupTextGroup& root) = 0;

		virtual bool DoesFileExist(const storm::String& path) = 0;

		virtual std::unique_ptr<VfsFile> OpenFile(std::string_view path,
		                                          uint32_t flags) = 0;

	private:
		storm::StringRef m_name;
};

BOOST_FORCEINLINE VfsProvider::VfsProvider(const storm::StringRef& name)
	: m_name(name)
{
	// ctor
}

BOOST_FORCEINLINE const storm::StringRef& VfsProvider::GetName() const
{ return m_name; }

METIN2_END_NS

#endif
