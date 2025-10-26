#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "questmanager.h"
#include "char.h"
#include "party.h"
#include "char_manager.h"
#include "shop_manager.h"
#include "dungeon_info.h"
#ifndef ALUA
#define ALUA(name) int name(lua_State* L)
#endif
namespace quest
{
ALUA(dungeon_info_get_min_level)
// int dungeon_info_get_min_level(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->bMinLvl);
    return 1;
}

ALUA(dungeon_info_get_max_level)
// int dungeon_info_get_max_level(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->bMaxLvl);
    return 1;
}

ALUA(dungeon_info_get_min_party)
// int dungeon_info_get_min_party(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->bMinParty);
    return 1;
}

ALUA(dungeon_info_get_max_party)
// int dungeon_info_get_max_party(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->bMaxParty);
    return 1;
}

ALUA(dungeon_info_get_cooldown)
// int dungeon_info_get_cooldown(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->wCooldown);
    return 1;
}

ALUA(dungeon_info_get_runtime)
// int dungeon_info_get_runtime(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->wRunTime);
    return 1;
}

ALUA(dungeon_info_get_ticket_vnum)
// int dungeon_info_get_ticket_vnum(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->dwTicketVnum);
    return 1;
}

ALUA(dungeon_info_get_ticket_count)
// int dungeon_info_get_ticket_count(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    const TDungeonInfoSet* infoSet =
        CDungeonInfo::instance().GetDungeonInfo((int)lua_tonumber(L, 1));
    if (!infoSet) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, infoSet->bTicketCount);
    return 1;
}

ALUA(dungeon_info_is_info_loaded)
// int dungeon_info_is_info_loaded(lua_State * L)
{
    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (ch) {
        lua_pushboolean(L, ch->IsDungeonInfoLoaded());
        return 1;
    }

    lua_pushboolean(L, false);
    return 1;
}

ALUA(dungeon_info_get_cooldown_end)
// int dungeon_info_get_cooldown_end(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (ch) {
        lua_pushnumber(L, ch->GetDungeonCooldownEnd((int)lua_tonumber(L, 1)));
        return 1;
    }

    lua_pushnumber(L, 0);
    return 1;
}

ALUA(dungeon_info_set_cooldown)
// int dungeon_info_set_cooldown(lua_State * L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
        SPDLOG_ERROR("invalid argument");
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (ch)
        ch->SetDungeonCooldown((int)lua_tonumber(L, 1),
                               (int)lua_tonumber(L, 2));

    return 0;
}

ALUA(dungeon_info_reset_cooldown)
// int dungeon_info_reset_cooldown(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (ch)
        ch->ResetDungeonCooldown((int)lua_tonumber(L, 1));

    return 0;
}

ALUA(dungeon_info_set_ranking)
// int dungeon_info_set_ranking(lua_State * L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isnumber(L, 4)) {
        SPDLOG_ERROR("invalid argument");
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (ch)
        ch->SetDungeonRanking(lua_tonumber(L, 1), lua_tonumber(L, 2),
                              lua_tonumber(L, 3), lua_tonumber(L, 4));

    return 0;
}

ALUA(dungeon_info_is_valid_dungeon)
// int dungeon_info_is_valid_dungeon(lua_State * L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        lua_pushboolean(L, false);
        return 1;
    }

    lua_pushboolean(
        L, CDungeonInfo::instance().IsValidDungeon((int)lua_tonumber(L, 1)));
    return 1;
}

ALUA(dungeon_info_get_dungeon_max_num)
// int dungeon_info_get_dungeon_max_num(lua_State * L)
{
    lua_pushnumber(L, CDungeonInfo::instance().GetDungeonNum());
    return 1;
}

void RegisterDungeonInfoFunctionTable()
{
    luaL_reg dungeon_info_functions[] = {
        {"get_min_level", dungeon_info_get_min_level},
        {"get_max_level", dungeon_info_get_max_level},
        {"get_min_party", dungeon_info_get_min_party},
        {"get_max_party", dungeon_info_get_max_party},
        {"get_cooldown", dungeon_info_get_cooldown},
        {"get_runtime", dungeon_info_get_runtime},
        {"get_ticket_vnum", dungeon_info_get_ticket_vnum},
        {"get_ticket_count", dungeon_info_get_ticket_count},

        {"is_info_loaded", dungeon_info_is_info_loaded},
        {"get_cooldown_end", dungeon_info_get_cooldown_end},
        {"set_cooldown", dungeon_info_set_cooldown},
        {"reset_cooldown", dungeon_info_reset_cooldown},
        {"set_ranking", dungeon_info_set_ranking},

        {"is_valid_dungeon", dungeon_info_is_valid_dungeon},
        {"get_dungeon_max_num", dungeon_info_get_dungeon_max_num},

        {NULL, NULL}};

    CQuestManager::instance().AddLuaFunctionTable("dungeon_info",
                                                  dungeon_info_functions);
}
}; // namespace quest
