#ifndef METIN2_BASE_CRC32_HPP
#define METIN2_BASE_CRC32_HPP

#include <Config.hpp>



uint32_t ComputeCrc32(uint32_t previousCrc32, const void* data, uint32_t length);



#endif
