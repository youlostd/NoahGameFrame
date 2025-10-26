#ifndef CRC32_H
#define CRC32_H


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <cstring>
#include <cstdint>


using crc_t = uint32_t;

crc_t get_crc32(const char * buffer, size_t count);

#define CRC32(buf) get_crc32(buf, std::strlen(buf))


#endif
