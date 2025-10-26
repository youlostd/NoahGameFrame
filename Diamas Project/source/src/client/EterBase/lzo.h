#ifndef METIN2_VFS_ETERBASE_LZO_HPP
#define METIN2_VFS_ETERBASE_LZO_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/Util.hpp>

#include <memory>

#include <windows.h>

METIN2_BEGIN_NS

class CLZObject
{
	public:
		#pragma pack(4)
		struct Header
		{
			uint32_t fourCc;
			uint32_t encryptedSize; // 암호화된 크기
			uint32_t compressedSize; // 압축된 데이터 크기
			uint32_t realSize; // 실제 데이터 크기
		};
		#pragma pack()

		static const uint32_t kFourCc = STORM_MAKEFOURCC('M', 'C', 'O', 'Z');

		CLZObject();

		bool BeginDecompress(const void* in);

		bool Decompress(const uint32_t* key = nullptr);

		const Header& GetHeader() const { return *m_header; }
		uint8_t* GetBuffer() { return m_buffer.get(); }
		uint32_t GetSize() const;

	private:
		std::unique_ptr<uint8_t[]> m_buffer;

		const Header* m_header;
		const uint8_t* m_in;
};

METIN2_END_NS

#endif
