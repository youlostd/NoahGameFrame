#ifndef METIN2_GAME_WARPMANAGER_HPP
#define METIN2_GAME_WARPMANAGER_HPP

#include "base/GroupTextTree.hpp"

#include <storm/Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

struct WarpInfo
{
    uint16_t mapIndex;
    uint32_t x;
    uint32_t y;
    uint8_t minLevel;
    uint8_t maxLevel;
};

class WarpManager
{
public:
    void LoadList(const GroupTextGroup *grp, const GroupTextList *list);
    bool LoadWarpTable(const std::string& filename);

private:
    std::unordered_map<uint64_t, WarpInfo> m_allWarps;
};


#endif