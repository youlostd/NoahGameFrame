#ifndef METIN2_SERVER_GAME_REGEN_H
#define METIN2_SERVER_GAME_REGEN_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "dungeon.h"

enum
{
    REGEN_TYPE_MOB,
    REGEN_TYPE_GROUP,
    REGEN_TYPE_EXCEPTION,
    REGEN_TYPE_GROUP_GROUP,
    REGEN_TYPE_ANYWHERE,
    REGEN_TYPE_MAX_NUM
};

typedef struct regen
{
    long lMapIndex;
    int type;
    int sx, sy, ex, ey;
    uint8_t z_section;

    uint8_t direction;

    uint32_t time;

    int max_count;
    int count;
    int vnum;

    bool is_aggressive;
    bool is_random_spawn;

    LPEVENT event;

    size_t id; // to help dungeon regen identification

    regen()
        : lMapIndex(0),
          type(0),
          sx(0), sy(0), ex(0), ey(0),
          z_section(0),
          direction(0),
          time(0),
          max_count(0),
          count(0),
          vnum(0),
          is_aggressive(0),
          event(nullptr),
          id(0)
    {
    }
} REGEN;

EVENTINFO(regen_event_info)
{
    regen *regen;

    regen_event_info()
        : regen(nullptr)
    {
    }
};

typedef regen_event_info REGEN_EVENT_INFO;

typedef struct regen_exception
{
    int sx, sy, ex, ey;
    uint8_t z_section;
} REGEN_EXCEPTION;

class CDungeon;

EVENTINFO(dungeon_regen_event_info)
{
    regen *regen;
    CDungeon::IdType dungeon_id;

    dungeon_regen_event_info()
        : regen(nullptr)
          , dungeon_id(0)
    {
    }
};

extern bool regen_load(const char *filename, long lMapIndex);
extern void regen_free_map(long lMapIndex);
extern void regen_reload(long lMapIndex);
extern void regen_register_map(const char *szBaseName, long lMapIndex);
extern bool is_valid_regen(regen *currRegen);
extern bool regen_do(const char *filename, long lMapIndex, CDungeon *pDungeon, bool bOnce = true);
extern bool regen_load_in_file(const char *filename, long lMapIndex);
extern void regen_free();

extern bool is_regen_exception(long x, long y);
extern void regen_reset(int x, int y);

#endif /* METIN2_SERVER_GAME_REGEN_H */
