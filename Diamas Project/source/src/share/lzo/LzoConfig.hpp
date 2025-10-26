#ifndef METIN2_LZO_LZOCONFIG_HPP
#define METIN2_LZO_LZOCONFIG_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/Util.hpp>

METIN2_BEGIN_NS

static const uint32_t kLzoObjectFourCc = STORM_MAKEFOURCC('M', 'C', 'O', 'Z');

struct LzoObjectHeader
{
	uint32_t fourCc;

	uint32_t encryptedSize;
	uint32_t compressedSize;
	uint32_t decompressedSize;
};

const char* GetLzoErrorString(int code);

BOOST_FORCEINLINE std::size_t GetMaxLzoCompressedSize(std::size_t size)
{
	return size + size / 16 + 64 + 3;
}

BOOST_FORCEINLINE std::size_t CalculateLzoOverhead(std::size_t size)
{
	const std::size_t overhead = size > 0xbfff ? 0xbfff : size;
	return overhead + size / 16 + 64 + 3;
}

METIN2_END_NS

#endif
