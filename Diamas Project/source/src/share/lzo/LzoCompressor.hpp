#ifndef METIN2_LZO_LZOOBJECTWRITER_HPP
#define METIN2_LZO_LZOOBJECTWRITER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/memory/AbstractAllocator.hpp>
#include <storm/StringFwd.hpp>

#include <boost/noncopyable.hpp>

METIN2_BEGIN_NS

class LzoCompressor : boost::noncopyable
{
	public:
		LzoCompressor(const storm::Allocator& allocator);

		bool Compress(const void* src, std::size_t srcSize,
		              void* dst, std::size_t& dstSize,
		              int level = 10);

	private:
		storm::Allocator m_allocator;
};

METIN2_END_NS

#endif
