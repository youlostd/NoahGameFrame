#ifndef METIN2_SERVER_GAME_XMAS_EVENT_H
#define METIN2_SERVER_GAME_XMAS_EVENT_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CHARACTER;

namespace xmas
{
enum
{
    MOB_SANTA_VNUM = 20031,
    //»êÅ¸ 
    MOB_SANTA_STAY_VNUM = 20126,
    //This one doesn't warp around
    MOB_XMAS_TREE_VNUM = 20032,
    MOB_XMAS_FIRWORK_SELLER_VNUM = 9004,

    SNOWMAN_MAIN = 41483,
    SNOWMAN_HAIR = 5068,

    XMAS_2014_PRESENT_VNUM = 51100,
};

void ProcessEventFlag(const std::string &name, int prev_value, int value);
void SpawnSanta(long lMapIndex, int iTimeGapSec);
void SpawnEventHelper(bool spawn);
bool IsSeason();

//Return whether the current NPC can be replaced by a snowman
bool IsSnowmanForPlayer(CHARACTER *ch, uint32_t raceNum);
}
#endif /* METIN2_SERVER_GAME_XMAS_EVENT_H */
