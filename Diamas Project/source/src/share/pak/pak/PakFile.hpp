#ifndef METIN2_SHARE_PAK_PAKFILE_HPP
#define METIN2_SHARE_PAK_PAKFILE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <memory>

#include <pak/VfsFile.hpp>

namespace storm {
class View;
}

METIN2_BEGIN_NS

class PakFile : public VfsFile
{
	public:
		PakFile(std::unique_ptr<uint8_t[]>&& data, uint32_t size);
		virtual ~PakFile();

		uint32_t GetSize() const;

		bool Read(uint32_t offset,
		                  void* buffer,
		                  uint32_t bytes) const;

		bool GetView(uint32_t offset,
		                     storm::View& view,
		                     uint32_t bytes) const;

	private:
		std::unique_ptr<uint8_t[]> m_data;
		uint32_t m_size;
};

METIN2_END_NS

#endif
