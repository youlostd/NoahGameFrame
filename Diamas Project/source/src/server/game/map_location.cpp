#include "map_location.h"
#include "sectree_manager.h"
#include "config.h"

bool CMapLocation::Get(int32_t index, std::string &addr, uint16_t &port)
{
    if (Get(gConfig.channel, index, addr, port))
        return true;

    if (gConfig.channel == SPECIAL_CHANNEL_EVENT)
        return Get(SPECIAL_CHANNEL_DEFAULT, index, addr, port);

    if (Get(SPECIAL_CHANNEL_EVENT, index, addr, port))
        return true;

    return false;
}

bool CMapLocation::Get(uint8_t channel, int32_t index,
                       std::string &addr, uint16_t &port)
{
    if (index == 0)
    {
        SPDLOG_INFO("Invalid map index {0} specified", index);
        return false;
    }

    if (index > 10000)
        index /= 10000;

    const auto it = m_maps.find(std::make_pair(channel, index));
    if (m_maps.end() == it)
    {
        SPDLOG_INFO("Failed to find map {0} on {1}", index, channel);

        for (const auto &p : m_maps)
        {
            SPDLOG_INFO("{0} {1}: ip {2} {3}",
                        p.first.first, p.second.addr, p.second.port);
        }

        return false;
    }

    addr = it->second.addr;
    port = it->second.port;
    return true;
}

bool CMapLocation::Exists(uint8_t channel, int32_t index)
{
    if (index == 0)
    {
        SPDLOG_INFO("CMapLocation::Get - Error MapIndex[%d]", index);
        return false;
    }

    auto it = m_maps.find(std::make_pair(channel, index));

    if (m_maps.end() == it)
        return false;

    return true;
}

uint16_t CMapLocation::GetPort(uint8_t channel, int32_t index)
{
    if (index == 0)
    {
        SPDLOG_INFO("CMapLocation::Get - Error MapIndex[%d]", index);
        return false;
    }

    auto it = m_maps.find(std::make_pair(channel, index));

    if (m_maps.end() == it)
    {
        SPDLOG_INFO("CMapLocation::Get - Error MapIndex[{0}]", index);
        for (auto i = m_maps.begin(); i != m_maps.end(); ++i)
        {
            SPDLOG_INFO("Map({0}): Server({1}:{2}}", i->first.second, i->second.addr, i->second.port);
        }
        return false;
    }

    return it->second.port;
}

void CMapLocation::Insert(uint8_t channel, int32_t index,
                          const char *addr, uint16_t port)
{
    SPDLOG_INFO("Map {0}:{1} -> {2}:{3}", channel, index, addr, port);

    Location l = {};
    l.addr = addr;
    l.port = port;
    m_maps.emplace(std::make_pair(channel, index), l);
}
