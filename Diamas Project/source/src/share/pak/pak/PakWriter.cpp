#include "PakWriter.hpp"
#include "PakFilename.hpp"
#include "PakUtil.hpp"

#include <MappedFile.h>
#include <Debug.h>
#include <tea.h>

#include <xxhash/xxhash.h>

#include <lz4.h>
#include <lz4hc.h>

bool PakWriter::Create(const std::string& filename)
{
	bsys::error_code ec;
	m_file.Open(filename, ec,
	            storm::AccessMode::kWrite,
	            storm::CreationDisposition::kCreateAlways,
	            storm::ShareMode::kNone,
	            storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} with {1}", filename, ec.message());
		return false;
	}

	m_file.Seek(storm::Whence::kBegin, sizeof(PakHeader), ec);

	if (ec) {
		spdlog::error("Failed to open/seek {0} with {1}", filename, ec.message());
		return false;
	}

	return true;
	
}

bool PakWriter::Add(const PakFilename& archivedPath,
                    const std::string& diskPath,
                    uint32_t flags)
{
	storm::FileLoader src(storm::GetDefaultAllocator());

	bsys::error_code ec;
	src.Load(diskPath, ec);

	if (ec) {
		spdlog::error("Failed to open {0} with {1}", diskPath, ec.message());
		return false;
	}

	if (src.GetSize() == 0) {
		spdlog::error("Cannot add zero-sized files");
		return false;
	}


	PakFileEntry entry{};
	entry.filenameHash = archivedPath.GetHash();
	entry.flags = flags;
	entry.offset = m_file.Seek(storm::Whence::kCurrent, 0, ec);
	entry.size = src.GetSize();
	entry.diskSize = entry.size;

	TempOrOutputBuffer diskData(src.GetData());

	if (entry.flags & kFileFlagLz4) {
		const auto bound = LZ4_compressBound(entry.size);
		diskData.MakeTemporaryBuffer(bound);

		const auto w = LZ4_compress_HC(reinterpret_cast<const char*>(src.GetData()),
		                        reinterpret_cast<char*>(diskData.ptr),
		                        entry.size, bound, 0);
		if (0 == w)
			return false;

		entry.diskSize = w;
	}


	m_file.Write(diskData.ptr, entry.diskSize, ec);

	if (ec) {
		spdlog::error("Failed to write {0} bytes with {1}",
		          entry.diskSize, ec.message());
		return false;
	}
	

	m_files.push_back(entry);
	return true;
}

bool PakWriter::Save()
{
	PakHeader hdr{};
	hdr.fourCc = PakHeader::kFourCc;
	hdr.version = PakHeader::kVersion;
	bsys::error_code ec;
	hdr.fileInfoOffset = m_file.Seek(storm::Whence::kCurrent, 0, ec);
	
	hdr.fileCount = m_files.size();

	for (const auto& f : m_files) {
		m_file.Write(&f, sizeof(f), ec);
		if (ec)
			return false;
	}

	m_file.Write(0, &hdr, sizeof(hdr), ec);
	if (ec) return false;

	return true;
}
