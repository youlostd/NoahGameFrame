#ifndef METIN2_SERVER_GAME_QUESTLUA_H
#define METIN2_SERVER_GAME_QUESTLUA_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "quest.h"
#include <game/GamePacket.hpp>
#include <net/Type.hpp>
#include "GBufferManager.h"

class CEntity;
class CHARACTER;

namespace quest
{
extern void RegisterPCFunctionTable();
extern void RegisterNPCFunctionTable();
extern void RegisterTargetFunctionTable();
extern void RegisterAffectFunctionTable();
extern void RegisterBuildingFunctionTable();
extern void RegisterMarriageFunctionTable();
extern void RegisterITEMFunctionTable();
extern void RegisterDungeonFunctionTable();
extern void RegisterQuestFunctionTable();
extern void RegisterPartyFunctionTable();
extern void RegisterHorseFunctionTable();
extern void RegisterGuildFunctionTable();
extern void RegisterGameFunctionTable();
extern void RegisterArenaFunctionTable();
extern void RegisterGlobalFunctionTable(lua_State *L);
extern void RegisterForkedFunctionTable();
extern void RegisterOXEventFunctionTable();
extern void RegisterBattleArenaFunctionTable();
extern void RegisterDragonSoulFunctionTable();
#ifdef ENABLE_MELEY_LAIR_DUNGEON
extern void RegisterMeleyLairFunctionTable();
#endif
extern void RegisterDungeonInfoFunctionTable();

extern void combine_lua_string(lua_State *L, std::ostringstream &s);
extern void create_notice_format_string(lua_State *L, std::ostringstream &s);

bool SetGameforgeLocale(lua_State *L, const std::string &locale);

struct FSetWarpLocation
{
    long map_index;
    long x;
    long y;

    FSetWarpLocation(long _map_index, long _x, long _y) : map_index(_map_index), x(_x), y(_y) {}

    void operator()(CHARACTER *ch);
};

struct FSetQuestFlag
{
    std::string flagname;
    int value;

    void operator()(CHARACTER *ch);
};

struct FPartyCheckFlagLt
{
    std::string flagname;
    int value;

    bool operator()(CHARACTER *ch);
};

struct FPartyChat
{
    int iChatType;
    const char *str;

    FPartyChat(int ChatType, const char *str);
    void operator()(CHARACTER *ch);
};

struct FPartyClearReady
{
    void operator()(CHARACTER *ch);
};

struct FSendScriptToEmpire
{
    GcScriptPacket p;
    uint8_t bEmpire;

    void operator()(CEntity *ent);
};

struct FPurgeArea
{
    int x1, y1, x2, y2;
    CHARACTER *ExceptChar;

    FPurgeArea(int a, int b, int c, int d, CHARACTER *p) : x1(a), y1(b), x2(c), y2(d), ExceptChar(p) {}

    void operator()(CEntity *ent);
};

struct FWarpEmpire
{
    uint8_t m_bEmpire;
    long m_lMapIndexTo;
    long m_x;
    long m_y;

    void operator()(CEntity *ent);
};

EVENTINFO(warp_all_to_map_my_empire_event_info)
{
    uint8_t m_bEmpire;
    long m_lMapIndexFrom;
    long m_lMapIndexTo;
    long m_x;
    long m_y;

    warp_all_to_map_my_empire_event_info() : m_bEmpire(0), m_lMapIndexFrom(0), m_lMapIndexTo(0), m_x(0), m_y(0) {}
};

EVENTFUNC(warp_all_to_map_my_empire_event);

struct FBuildLuaGuildWarList
{
    lua_State *L;
    int m_count;

    FBuildLuaGuildWarList(lua_State *L);
    void operator()(uint32_t g1, uint32_t g2);
};
} // namespace quest

#endif /* METIN2_SERVER_GAME_QUESTLUA_H */
