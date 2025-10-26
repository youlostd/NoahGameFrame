#ifndef METIN2_SERVER_GAME_MAP_LOCATION_H
#define METIN2_SERVER_GAME_MAP_LOCATION_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>
#include <base/Singleton.hpp>
#include <base/robin_hood.h>

class CMapLocation : public singleton<CMapLocation>
{
public:
    struct Location
    {
        std::string addr;
        uint16_t port;
    };

    bool Get(int32_t index, std::string &addr, uint16_t &port);

    bool Get(uint8_t channel, int32_t index,
             std::string &addr, uint16_t &port);

    bool Exists(uint8_t channel, int32_t index);

    uint16_t GetPort(uint8_t channel, int32_t index);

    void Insert(uint8_t channel, int32_t index,
                const char *addr, uint16_t port);

  protected:
    struct pairhash
    {
    public:
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U> &x) const
        {
            return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
        }
    };

    robin_hood::unordered_map<std::pair<uint8_t, uint32_t>, Location, pairhash> m_maps;
};
#endif /* METIN2_SERVER_GAME_MAP_LOCATION_H */
