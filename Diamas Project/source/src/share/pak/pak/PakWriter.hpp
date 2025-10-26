#ifndef METIN2_CLIENT_ETERPACK_PAKWRITER_HPP
#define METIN2_CLIENT_ETERPACK_PAKWRITER_HPP

#include "PakFormat.hpp"

#include <string>
#include <vector>
#include <concurrent_vector.h>

#include <storm/crypt/Xtea.hpp>
#include <storm/io/File.hpp>
#include <storm/io/FileLoader.hpp>

class PakFilename;

class PakWriter
{
	public:
		~PakWriter() = default;

		bool Create(const std::string& filename);
		bool Add(const PakFilename& archivedPath,
		         const std::string& diskPath,
		         uint32_t flags);

		bool Save();

	private:
		storm::File m_file;
		concurrency::concurrent_vector<PakFileEntry> m_files;
};

#endif
