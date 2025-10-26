#ifndef METIN2_CLIENT_ETERPACK_PAKFILENAME_HPP
#define METIN2_CLIENT_ETERPACK_PAKFILENAME_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <string>
#include <cstdint>

// Simple filename wrapper class
class PakFilename
{
	public:
		PakFilename() = default;
		PakFilename(const std::string& path);
		PakFilename(std::string_view path);
		PakFilename(const char* path);

		PakFilename& operator=(const std::string& path);
	PakFilename& operator=(std::string_view path);
	PakFilename& operator=(const char* path);

		void Set(const char* path, uint32_t length);

		uint64_t GetHash() const;
		const std::string& GetPath() const;

	private:
		std::string m_path;
		uint64_t m_hash{};
};

#endif
