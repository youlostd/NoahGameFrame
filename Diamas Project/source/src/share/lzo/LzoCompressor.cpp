#include <lzo/LzoCompressor.hpp>
#include <lzo/LzoConfig.hpp>

#include <storm/memory/Allocate.hpp>
#include <storm/String.hpp>

#include <lzo/lzopro/lzo1x.h>
#include <SpdLog.hpp>
#include <cstring>
#include <lzo/lzoconf.h>

METIN2_BEGIN_NS

namespace
{

// malloc/free callbacks required for lzopro_lzo1[xy]_99_compress()
lzo_voidp __LZO_CDECL LzoAlloc(lzo_callback_p self, lzo_uint items, lzo_uint size)
{
	auto allocator = static_cast<storm::Allocator*>(self->user1);
	return allocator->Allocate(items * size, 1);
}

void __LZO_CDECL LzoFree(lzo_callback_p self, lzo_voidp ptr)
{
	auto allocator = static_cast<storm::Allocator*>(self->user1);
	allocator->Deallocate(ptr);
}

}

LzoCompressor::LzoCompressor(const storm::Allocator& allocator)
	: m_allocator(allocator)
{
	// ctor
}

bool LzoCompressor::Compress(const void* src, std::size_t srcSize,
                             void* dst, std::size_t& dstSize,
                             int level)
{
	STORM_ASSERT(level >= 1 && level <= 10,
	             "Compression level has to be in [1, 10]");

	lzo_callback_t callbacks;
	std::memset(&callbacks, 0, sizeof(callbacks));
	callbacks.nalloc = LzoAlloc;
	callbacks.nfree = LzoFree;
	callbacks.user1 = &m_allocator;

	lzo_uint compressedSize = dstSize;
	int r = lzopro_lzo1x_99_compress(static_cast<const lzo_bytep>(src),
	                                 srcSize,
	                                 static_cast<lzo_bytep>(dst),
	                                 &compressedSize,
	                                 &callbacks,
	                                 level);

	if (r == LZO_E_OK) {
		dstSize = compressedSize;
		return true;
	} else {
		spdlog::error("lzopro_lzo1x_99_compress failed with '{0}' for "
		          "{1} bytes of input data",
		          GetLzoErrorString(r), srcSize);
		return false;
	}
}

METIN2_END_NS
