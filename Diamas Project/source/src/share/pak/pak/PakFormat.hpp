#ifndef METIN2_CLIENT_ETERPACK_PAKFORMAT_HPP
#define METIN2_CLIENT_ETERPACK_PAKFORMAT_HPP

#include <cstdint>

#define PAK_MAKEFOURCC(b0, b1, b2, b3) \
	(uint32_t(uint8_t(b0)) | (uint32_t(uint8_t(b1)) << 8) | \
	(uint32_t(uint8_t(b2)) << 16) | (uint32_t(uint8_t(b3)) << 24))

enum FileFlags
{
	kFileFlagLz4 = 1 << 0,
};

// generated using: random.randint(0, 1 << 64)
static const uint64_t kFilenameMagic = 4639460977058815519ull;
static const uint64_t kFilenameKeyMagic1 = 1085645127720199622ull;
static const uint64_t kFilenameKeyMagic2 = 4478567896021961570ull;

#pragma pack(push, 1)
struct PakHeader
{
	static const uint32_t kFourCc = PAK_MAKEFOURCC('P', 'A', 'K', ' ');
	static const uint32_t kVersion = 1;

	uint32_t fourCc;
	uint32_t version;

	// PakFileEntry array
	uint32_t fileInfoOffset;
	uint32_t fileCount;
};

struct PakFileEntry
{
	// XXH64 hash of the "normalized" filename
	// see: PakFilename::Set
	uint64_t filenameHash;

	// see FileFlags
	uint32_t flags;

	// Offset of the file - relative to the archive's beginning.
	uint32_t offset;

	// Size of the file - on disk (i.e compressed, etc.)
	uint32_t diskSize;

	// Size of the unpacked file
	uint32_t size;
};
#pragma pack(pop)

#endif
