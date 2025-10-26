#ifndef METIN2_SERVER_GAME_START_POSITION_H
#define METIN2_SERVER_GAME_START_POSITION_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "locale_service.h"
#include "locale.hpp"

extern char g_nation_name[4][32];
extern uint32_t g_start_position[4][3];
extern long g_start_map[4];
extern uint32_t g_create_position[4][3];
extern uint32_t arena_return_position[4][3];
extern uint32_t g_create_position_wolf[4][2];

inline const char *EMPIRE_NAME(uint8_t e) { return LC_TEXT(g_nation_name[e]); }

inline uint32_t EMPIRE_START_MAP(uint8_t e)
{
    if (1 <= e && e <= 3)
        return g_start_position[e][2];

    return 0;
}

inline uint32_t EMPIRE_START_X(uint8_t e)
{
    if (1 <= e && e <= 3)
        return g_start_position[e][0];

    return 0;
}

inline uint32_t EMPIRE_START_Y(uint8_t e)
{
    if (1 <= e && e <= 3)
        return g_start_position[e][1];

    return 0;
}

inline uint32_t ARENA_RETURN_POINT_MAP(uint8_t e)
{
    if (1 <= e && e <= 3)
        return arena_return_position[e][2];

    return 0;
}

inline uint32_t ARENA_RETURN_POINT_X(uint8_t e)
{
    if (1 <= e && e <= 3)
        return arena_return_position[e][0];

    return 0;
}

inline uint32_t ARENA_RETURN_POINT_Y(uint8_t e)
{
    if (1 <= e && e <= 3)
        return arena_return_position[e][1];

    return 0;
}

inline uint32_t CREATE_START_MAP(uint8_t e)
{
    if (1 <= e && e <= 3)
        return g_create_position[e][2];

    return 0;
}

inline uint32_t CREATE_START_X(uint8_t e, bool isWolf = false)
{
    if (1 <= e && e <= 3)
    {
        if (isWolf)
            return g_create_position_wolf[e][0];

        return g_create_position[e][0];
    }

    return 0;
}

inline uint32_t CREATE_START_Y(uint8_t e, bool isWolf = false)
{
    if (1 <= e && e <= 3)
    {
        if (isWolf)
            return g_create_position_wolf[e][1];

        return g_create_position[e][1];
    }

    return 0;
}
#endif /* METIN2_SERVER_GAME_START_POSITION_H */
