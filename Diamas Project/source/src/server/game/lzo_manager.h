#ifndef METIN2_SERVER_GAME_LZO_MANAGER_H
#define METIN2_SERVER_GAME_LZO_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <lzo/lzo1x.h>
#include <base/Singleton.hpp>

class LZOManager : public singleton<LZOManager>
{
public:
    LZOManager();
    virtual ~LZOManager();

    bool Compress(const uint8_t *src, size_t srcsize, uint8_t *dest, lzo_uint *puiDestSize);
    bool Decompress(const uint8_t *src, size_t srcsize, uint8_t *dest, lzo_uint *puiDestSize);
    size_t GetMaxCompressedSize(size_t original);

    uint8_t *GetWorkMemory() { return m_workmem; }

private:
    uint8_t *m_workmem;
};

#endif /* METIN2_SERVER_GAME_LZO_MANAGER_H */
