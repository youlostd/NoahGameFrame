#include "lzo.h"

#include <storm/crypt/Xtea.hpp>
#include <storm/Util.hpp>

#include <lzo/lzo1x.h>

METIN2_BEGIN_NS

CLZObject::CLZObject()
    : m_header(nullptr)
      , m_in(nullptr)
{
    // ctor
}

bool CLZObject::BeginDecompress(const void *in)
{
    auto header = static_cast<const Header *>(in);
    if (header->fourCc != kFourCc)
    {
        SPDLOG_ERROR("LZObject: not a valid data");
        return false;
    }

    if (header->realSize == 0)
    {
        SPDLOG_ERROR("LZObject: zero size invalid");
        return false;
    }

    m_header = header;
    m_in = static_cast<const uint8_t *>(in) + sizeof(Header) + sizeof(uint32_t);

    m_buffer.reset(new uint8_t[header->realSize]);
    memset(m_buffer.get(), 0, header->realSize);
    return true;
}

class DecryptBuffer
{
public:
    enum
    {
        LOCAL_BUF_SIZE = 8 * 1024,
    };

    DecryptBuffer(unsigned size)
    {
        if (size >= LOCAL_BUF_SIZE)
            m_buf = new uint8_t[size];
        else
            m_buf = m_local_buf;
    }

    ~DecryptBuffer()
    {
        if (m_local_buf != m_buf)
            delete [] m_buf;
    }

    uint8_t *GetBufferPtr()
    {
        return m_buf;
    }

private:
    uint8_t *m_buf;
    uint8_t m_local_buf[LOCAL_BUF_SIZE];
};

bool CLZObject::Decompress(const uint32_t *key)
{
    lzo_uint uiSize;

    if (m_header->encryptedSize)
    {
        DecryptBuffer buf(m_header->encryptedSize);

        storm::DecryptXtea(buf.GetBufferPtr(),
                           m_in - sizeof(uint32_t),
                           key,
                           m_header->encryptedSize);

        if (*(uint32_t *)buf.GetBufferPtr() != kFourCc)
        {
            SPDLOG_ERROR("LZObject: key incorrect");
            return false;
        }

        int r = lzo1x_decompress(buf.GetBufferPtr() + sizeof(uint32_t),
                                 m_header->compressedSize,
                                 m_buffer.get(),
                                 &uiSize,
                                 nullptr);
        if (LZO_E_OK != r)
        {
            SPDLOG_ERROR(
                "lzo1x_decompress failed with {0} for {1} bytes decrypted data",
                r, m_header->compressedSize);
            return false;
        }
    }
    else
    {
        uiSize = m_header->realSize;

        int r = lzo1x_decompress(m_in, m_header->compressedSize,
                                 m_buffer.get(), &uiSize, nullptr);
        if (LZO_E_OK != r)
        {
            SPDLOG_ERROR(
                "lzo1x_decompress failed with {0} for {1} bytes",
                r, m_header->compressedSize);
            return false;
        }
    }

    if (uiSize != m_header->realSize)
    {
        SPDLOG_ERROR("Size mismatch {0} != {1}", uiSize, m_header->realSize);
        return false;
    }

    return true;
}

uint32_t CLZObject::GetSize() const
{
    assert(m_header);
    return m_header->realSize;
}

METIN2_END_NS
