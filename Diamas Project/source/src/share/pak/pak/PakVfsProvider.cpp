#include "PakVfsProvider.hpp"
#include "PakArchive.hpp"
#include "PakFile.hpp"

#include <base/GroupTextTree.hpp>


#include <xxhash.h>

METIN2_BEGIN_NS

PakVfsProvider::PakVfsProvider()
	: VfsProvider("Pak")
{
	// ctor
}

bool PakVfsProvider::AddArchive(const std::string& path)
{
    auto pack = std::make_unique<PakArchive>();

    if (!pack->Create(path, m_files))
    {
        SPDLOG_WARN("Failed to load PakArchive {0}", path.c_str());
        return false;
    }

    m_paks.push_back(std::move(pack));
    return true;
}

/*virtual*/ bool PakVfsProvider::DoesFileExist(const storm::String& path)
{
	return m_files.find(XXH64(path.data(), path.size(), kFilenameMagic)) != m_files.end();
}

/*virtual*/ std::unique_ptr<VfsFile> PakVfsProvider::OpenFile(std::string_view path,
                                                               uint32_t flags)
{
	const auto it = m_files.find(XXH64(path.data(), path.size(), kFilenameMagic));
	if (it != m_files.end()) {
		SPDLOG_TRACE("Found archive entry for {0}", path);

		const auto& entry = it->second;
		STORM_ASSERT(entry.first, "No archive bound?");

		std::unique_ptr<uint8_t[]> data;
		uint32_t size;

		if (entry.first->Get(path, entry.second, data, size))
			return std::make_unique<PakFile>(std::move(data), size);
	} else {
		SPDLOG_TRACE("Failed to find archive entry for {0}", path);
	}

	return nullptr;
}

METIN2_END_NS
