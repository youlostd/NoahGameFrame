#include <pak/Vfs.hpp>
#include <optional>

METIN2_BEGIN_NS

std::optional<std::string> LoadFileToString(Vfs& vfs,
                      const storm::String& filename)
{
	std::string s;
	auto fp = vfs.Open(filename, kVfsOpenFullyBuffered);
	if (!fp)
		return std::nullopt;

	const auto size = fp->GetSize();

	s.resize(size);
    if(fp->Read(0, &*s.begin(), size))
		return s;

	return std::nullopt;
}

METIN2_END_NS
