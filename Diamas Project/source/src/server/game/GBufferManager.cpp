#include "GBufferManager.h"

TEMP_BUFFER::TEMP_BUFFER(uint32_t size, bool forceDelete)
    : m_forceDelete(forceDelete)
{
    if (forceDelete)
        size = std::max<uint32_t>(size, 1024 * 128);

    m_buf = buffer_new(size);
    STORM_ASSERT(m_buf, "buffer_new() failed");
}

TEMP_BUFFER::~TEMP_BUFFER() { buffer_delete(m_buf); }

const void *TEMP_BUFFER::read_peek() { return buffer_read_peek(m_buf); }

void *TEMP_BUFFER::write_peek() { return buffer_write_peek(m_buf); }

void TEMP_BUFFER::write(const void *data, int size) { buffer_write(m_buf, data, size); }

void TEMP_BUFFER::write_proceed(uint32_t size) { buffer_write_proceed(m_buf, size); }

int TEMP_BUFFER::size() { return buffer_size(m_buf); }

void TEMP_BUFFER::reset() { buffer_reset(m_buf); }

void TempBuffer::write(const void *data, size_t size)
{
    const auto packetBytes = static_cast<const uint8_t *>(data);
    m_buffer.insert(m_buffer.end(), packetBytes, packetBytes + size);
}
