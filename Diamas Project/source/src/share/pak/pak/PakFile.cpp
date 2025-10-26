#include "PakFile.hpp"

#include <cstring>
#include <storm/io/View.hpp>

METIN2_BEGIN_NS

PakFile::PakFile(std::unique_ptr<uint8_t[]>&& data, uint32_t size)
    : m_data(std::move(data))
    , m_size(size)
{
    // ctor
}

/*virtual*/ PakFile::~PakFile()
{
    // dtor
}

uint32_t PakFile::GetSize() const
{
    return m_size;
}

bool PakFile::Read(uint32_t offset, void* buffer, uint32_t bytes) const
{
    if (offset + bytes <= m_size) {
        std::memcpy(buffer, m_data.get() + offset, bytes);
        return true;
    }

    return false;
}

bool PakFile::GetView(uint32_t offset, storm::View& view, uint32_t bytes) const
{
    if (offset + bytes <= m_size) {
        view.Initialize(m_data.get() + offset);
        return true;
    }

    return false;
}

METIN2_END_NS
