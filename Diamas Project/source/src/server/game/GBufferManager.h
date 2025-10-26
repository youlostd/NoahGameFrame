#ifndef METIN2_SERVER_GAME_BUFFER_MANAGER_H
#define METIN2_SERVER_GAME_BUFFER_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vector>


class TEMP_BUFFER
{
public:
    TEMP_BUFFER(uint32_t size = 8192, bool forceDelete = false);
    ~TEMP_BUFFER();

    const void *read_peek();
    void *write_peek();

    void write(const void *data, int size);
    void write_proceed(uint32_t size);

    int size();
    void reset();

    buffer *getptr() { return m_buf; }

protected:
    buffer *m_buf;
    bool m_forceDelete;
};

class TempBuffer
{
public:
    void reserve(size_t size) { m_buffer.reserve(size); }

    void write(const void *data, size_t size);

    const void *data() { return m_buffer.data(); }

    size_t size() const { return m_buffer.size(); }

private:
    std::vector<uint8_t> m_buffer;
};

#endif /* METIN2_SERVER_GAME_BUFFER_MANAGER_H */
