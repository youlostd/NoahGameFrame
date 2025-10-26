#include <pak/Vfs.hpp>
#include <pak/VfsProvider.hpp>

#include <base/GroupTextTree.hpp>


METIN2_BEGIN_NS

namespace
{

static Vfs* globalInstance = nullptr;

}

void Vfs::RegisterProvider(VfsProvider* provider)
{
	m_providers.push_back(provider);
}



bool Vfs::Exists(const storm::String& path)
{
	std::string output;
	const auto len = path.length();
	output.resize(len);

	if (len == 0)
		return nullptr;

	storm::CopyAndLowercaseFilename(&output[0], path.data(), len);

	for (VfsProvider* prov : m_providers) {
		if (prov->DoesFileExist(output))
			return true;
	}

	return false;
}

std::unique_ptr<VfsFile> Vfs::Open(std::string_view path,
                                   uint32_t flags)
{
	std::string output;
	const auto len = path.length();
	output.resize(len);

	if (len == 0)
		return nullptr;

	storm::CopyAndLowercaseFilename(&output[0], path.data(), len);

	for (VfsProvider* prov : m_providers) {
		auto file = prov->OpenFile(output, flags);
		if (file)
			return file;
	}

	return nullptr;
}


Vfs& GetVfs()
{
	STORM_ASSERT(globalInstance, "NULL Vfs");
	return *globalInstance;
}

void SetVfs(Vfs* vfs)
{
	globalInstance = vfs;
}

METIN2_END_NS
