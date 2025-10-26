#ifndef METIN2_CLIENT_ETERPACK_PAKARCHIVE_HPP
#define METIN2_CLIENT_ETERPACK_PAKARCHIVE_HPP

#include "PakFormat.hpp"

#include <storm/io/File.hpp>
#include <MappedFile.h>

#include <string>
#include <unordered_map>
#include <utility>

class CMappedFile;
class PakArchive;

typedef std::unordered_map<
	uint64_t,
	std::pair<PakArchive*, PakFileEntry>
> PakFileDict;

class PakArchive
{
	public:
		bool Create(const std::string& filename, PakFileDict& dict);

		bool Get(std::string_view path,
                 const PakFileEntry &entry, std::unique_ptr<uint8_t[]> &data, uint32_t &size);

	private:
		CMappedFile m_file;
};

#endif
