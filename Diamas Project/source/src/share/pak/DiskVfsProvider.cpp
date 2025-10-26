#include <pak/DiskVfsProvider.hpp>
#include <pak/DiskVfsFile.hpp>

#include <base/GroupTextTree.hpp>

#include <storm/io/FileUtil.hpp>

#include <pak/VfsProvider.hpp>

METIN2_BEGIN_NS

DiskVfsProvider::DiskVfsProvider()
	: ::VfsProvider("Disk")
{
	// ctor
}

/*virtual*/ bool DiskVfsProvider::LoadConfig(const GroupTextGroup& root)
{
	// No configuration [yet]
	return true;
}

/*virtual*/ bool DiskVfsProvider::DoesFileExist(const storm::String& path)
{
	bsys::error_code ec;
	storm::StatFile(path, nullptr, nullptr, nullptr, nullptr, nullptr, ec);
	return !ec;
}

/*virtual*/ std::unique_ptr<VfsFile> DiskVfsProvider::OpenFile(std::string_view path,
                                                               uint32_t flags)
{
	std::unique_ptr<DiskVfsFile> file(new DiskVfsFile());
	if (!file->Open(path))
		return nullptr;

	return std::unique_ptr<VfsFile>(std::move(file));
}

METIN2_END_NS
