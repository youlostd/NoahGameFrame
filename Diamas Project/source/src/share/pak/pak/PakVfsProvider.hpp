#ifndef METIN2_PAK_PAKVFSPROVIDER_HPP
#define METIN2_PAK_PAKVFSPROVIDER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "PakArchive.hpp"

#include <pak/VfsProvider.hpp>

#include <storm/String.hpp>

#include <vector>
#include <memory>

METIN2_BEGIN_NS

class PakVfsProvider : public VfsProvider
{
	public:
		PakVfsProvider();
        bool AddArchive(const std::string& path);
		bool LoadConfig(const GroupTextGroup& root) override { return true; }
		virtual bool DoesFileExist(const storm::String& path);

		virtual std::unique_ptr<VfsFile> OpenFile(std::string_view path,
		                                          uint32_t flags);

	private:
		PakFileDict m_files;
		std::vector<std::unique_ptr<PakArchive>> m_paks;
};

METIN2_END_NS

#endif
