#include <pak/DiskVfsFile.hpp>


#include <limits>
#include <storm/io/View.hpp>

METIN2_BEGIN_NS

/*virtual*/ DiskVfsFile::~DiskVfsFile()
{
	// dtor
}

bool DiskVfsFile::Open(const storm::StringRef& path)
{
	bsys::error_code ec;
	m_file.Open(path, ec,
	            storm::AccessMode::kRead,
	            storm::CreationDisposition::kOpenExisting,
	            storm::ShareMode::kRead);

	if (ec) {
		SPDLOG_TRACE("Failed to open '{0}' for reading with '{1}'",
		           path, ec);
		return false;
	}

	return true;
}

/*virtual*/ uint32_t DiskVfsFile::GetSize() const
{
	const uint64_t size = m_file.GetSize();
	STORM_ASSERT(size < std::numeric_limits<uint32_t>::max(), "Sanity check");

	return static_cast<uint32_t>(size);
}

/*virtual*/ bool DiskVfsFile::Read(uint32_t offset,
                                   void* buffer,
                                   uint32_t bytes) const
{
	bsys::error_code ec;
	auto read = m_file.Read(offset, buffer, bytes, ec);

	if (ec) {
		SPDLOG_TRACE(
		          "Failed to read {0} bytes from file at pos {1} with {2}",
		          bytes, offset, ec);
		return false;
	}

	if (read != bytes) {
		SPDLOG_TRACE(
		          "Could only read {0} (instead of {1}) from file pos {2}",
		          read, bytes, offset);
		return false;
	}

	return true;
}

/*virtual*/ bool DiskVfsFile::GetView(uint32_t offset,
                                      storm::View& view,
                                      uint32_t bytes) const
{
	auto p = view.Initialize(bytes);
	return Read(offset, p, bytes);
}

METIN2_END_NS
