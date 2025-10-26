#include "PakArchive.hpp"
#include "PakUtil.hpp"

#include <Debug.h>
#include <MappedFile.h>
#include <tea.h>

#include <lz4.h>
#include <storm/io/StreamUtil.hpp>
#include <xxhash/xxhash.h>

bool PakArchive::Create(const std::string &filename, PakFileDict &dict)
{
    if (!m_file.Create(filename))
        return false;

    PakHeader hdr{};
    if (!m_file.Read(&hdr, sizeof(hdr)))
        return false;

    if (hdr.fourCc != PakHeader::kFourCc)
        return false;

    if (hdr.version != PakHeader::kVersion)
        return false;

    m_file.Seek(hdr.fileInfoOffset);

    for (uint32_t i = 0; i != hdr.fileCount; ++i)
    {
        PakFileEntry e{};
        if (!m_file.Read(&e, sizeof(e)))
            return false;

        dict.insert_or_assign(e.filenameHash, std::make_pair(this, e));
    }

    return true;
}

bool PakArchive::Get(std::string_view path, const PakFileEntry &entry, std::unique_ptr<uint8_t[]>& data,
                     uint32_t& size)
{
	size = entry.size;
	data.reset(new uint8_t[entry.size]);

    m_file.Seek(entry.offset);

    if (entry.flags & kFileFlagLz4)
    {
        const auto r =
            LZ4_decompress_safe(reinterpret_cast<const char *>(m_file.GetCurrentSeekPoint()),
                                reinterpret_cast<char *>(data.get()), entry.diskSize, entry.size);

        if (r < 0)
        {
            SPDLOG_ERROR("Failed to decompress {0} with {1}", path, r);
            return false;
        }
    }

    return true;
}
