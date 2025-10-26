#include "PakFilename.hpp"
#include "PakFormat.hpp"

#include <xxhash/xxhash.h>
#include <boost/algorithm/string.hpp>
#include <cstring>

namespace
{

char ToLowerAscii(char ch)
{
	if (ch <= 'Z' && ch >= 'A')
		return ch - ('Z' - 'z');

	return ch;
}

}

PakFilename::PakFilename(const std::string& path)
{
	Set(path.data(), path.length());
}

PakFilename::PakFilename(std::string_view path)
{
	Set(path.data(), path.length());
}

PakFilename::PakFilename(const char* path)
{
	Set(path, std::strlen(path));
}

PakFilename& PakFilename::operator=(const std::string& path)
{
	Set(path.data(), path.length());
	return *this;
}

PakFilename& PakFilename::operator=(std::string_view path)
{
	Set(path.data(), path.length());
	return *this;
}

PakFilename& PakFilename::operator=(const char* path)
{
	Set(path, std::strlen(path));
	return *this;
}

void PakFilename::Set(const char* path, uint32_t length)
{
	m_path.resize(length);

	for (uint32_t i = 0; i < length; ++i) {
		uint8_t ch = path[i];
		if (ch == '\\')
			ch = '/';
		else
			ch = ToLowerAscii(ch);

		m_path[i] = ch;
	}

	boost::trim(m_path);

	m_hash = XXH64(m_path.data(), length, kFilenameMagic);
}

uint64_t PakFilename::GetHash() const
{
	return m_hash;
}

const std::string& PakFilename::GetPath() const
{
	return m_path;
}
