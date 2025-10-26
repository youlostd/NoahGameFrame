#ifndef METIN2_SERVER_GAME_BLUEDRAGON_BINDER_H
#define METIN2_SERVER_GAME_BLUEDRAGON_BINDER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

enum BLUEDRAGON_STONE_EFFECT
{
    DEF_BONUS = 1,
    ATK_BONUS = 2,
    REGEN_TIME_BONUS = 3,
    REGEN_PECT_BONUS = 4,
};

extern unsigned int BlueDragon_GetRangeFactor(const char *key, const int val);
extern unsigned int BlueDragon_GetSkillFactor(const size_t cnt, ...);
extern unsigned int BlueDragon_GetIndexFactor(const char *container, const size_t idx, const char *key);

#endif /* METIN2_SERVER_GAME_BLUEDRAGON_BINDER_H */
