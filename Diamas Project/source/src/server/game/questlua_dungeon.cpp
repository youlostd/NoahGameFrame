#include "DbCacheSocket.hpp"
#include "GBufferManager.h"
#include "char.h"
#include "char_manager.h"
#include "constants.h"
#include "desc_manager.h"
#include "dungeon.h"
#include "party.h"
#include "questlua.h"
#include "questmanager.h"

#include "desc.h"
#include <game/GamePacket.hpp>
#ifndef ALUA
#define ALUA(name) int name(lua_State* L)
#endif
#include <boost/math/constants/constants.hpp>

#ifdef ENABLE_HYDRA_DUNGEON
#include "HydraDungeon.h"
#endif

namespace quest
{
//
// "dungeon" lua functions
//
int dungeon_notice(lua_State* L)
{
    if (!lua_isstring(L, 1))
        return 0;

    ostringstream s;
    create_notice_format_string(L, s);

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->Notice(s.str().c_str());
    return 0;
}

int dungeon_set_mission_message(lua_State* L)
{
    if (!lua_isstring(L, 1))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SetMissionMessage(lua_tostring(L, 1));
    return 0;
}

int dungeon_set_mission_sub_message(lua_State* L)
{
    if (!lua_isstring(L, 1))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SetMissionSubMessage(lua_tostring(L, 1));
    return 0;
}

int dungeon_clear_mission_message(lua_State* L)
{
    if (!lua_isstring(L, 1))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->ClearMissionMessage();
    return 0;
}

int dungeon_set_quest_flag(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();

    FSetQuestFlag f;

    f.flagname =
        q.GetCurrentPC()->GetCurrentQuestName() + "." + lua_tostring(L, 1);
    f.value = static_cast<int>(lua_tonumber(L, 2));

    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->ForEachMember(f);

    return 0;
}

int dungeon_set_flag(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2)) {
        SPDLOG_ERROR("dungeon_set_flag: einval");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        SPDLOG_ERROR("dungeon_set_flag: no dungeon");
        return 0;
    }

    const char* sz = lua_tostring(L, 1);
    int value = static_cast<int>(lua_tonumber(L, 2));
    pDungeon->SetFlag(sz, value);
    return 0;
}

int dungeon_get_flag(lua_State* L)
{
    if (!lua_isstring(L, 1)) {
        SPDLOG_ERROR("dungeon_get_flag: einval");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    const auto name = q.GetCurrentQuestName();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        SPDLOG_ERROR("dungeon_get_flag: no dungeon in {0}", name.c_str());
        return 0;
    }

    const char* sz = lua_tostring(L, 1);
    lua_pushnumber(L, pDungeon->GetFlag(sz));
    return 1;
}

int dungeon_get_flag_from_map_index(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2)) {
        SPDLOG_ERROR("dungeon_get_flag_from_map_index: einval");
        return 0;
    }

    uint32_t dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 2));
    if (!dwMapIndex) {
        lua_pushboolean(L, 0);
        return 1;
    }

    auto* pDungeon =
        CDungeonManager::instance().FindByMapIndex<QuestDungeon>(dwMapIndex);
    if (!pDungeon) {
        SPDLOG_ERROR("dungeon_get_flag_from_map_index: no dungeon");
        return 0;
    }

    const char* sz = lua_tostring(L, 1);
    lua_pushnumber(L, pDungeon->GetFlag(sz));
    return 1;
}

int dungeon_get_map_index(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        SPDLOG_INFO("Dungeon GetMapIndex %d", pDungeon->GetMapIndex());
        lua_pushnumber(L, pDungeon->GetMapIndex());
    } else {
        SPDLOG_ERROR("no dungeon !!!");
        lua_pushnumber(L, 0);
    }

    return 1;
}

int dungeon_get_base_map_index(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        SPDLOG_INFO("Dungeon GetMapIndex {}", pDungeon->GetOriginalMapIndex());
        lua_pushnumber(L, pDungeon->GetOriginalMapIndex());
    } else {
        SPDLOG_ERROR("no dungeon !!!");
        lua_pushnumber(L, 0);
    }

    return 1;
}

int dungeon_regen_file(lua_State* L)
{
    if (!lua_isstring(L, 1)) {
        SPDLOG_ERROR("wrong filename");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnRegen(lua_tostring(L, 1));

    return 0;
}

int dungeon_set_regen_file(lua_State* L)
{
    if (!lua_isstring(L, 1)) {
        SPDLOG_ERROR("wrong filename");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnRegen(lua_tostring(L, 1), false);
    return 0;
}

int dungeon_clear_regen(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();
    if (pDungeon)
        pDungeon->ClearRegen();
    return 0;
}

int dungeon_check_eliminated(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();
    if (pDungeon)
        pDungeon->CheckEliminated();
    return 0;
}

int dungeon_set_exit_all_at_eliminate(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("wrong time");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SetExitAllAtEliminate(
            static_cast<int32_t>(lua_tonumber(L, 1)));

    return 0;
}

int dungeon_set_event_at_eliminate(lua_State* L)
{
    const char* c_pszRegenFile = nullptr;

    if (lua_gettop(L) >= 1)
        c_pszRegenFile = lua_tostring(L, 1);

    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SetEventAtEliminate(c_pszRegenFile);

    return 0;
}

int dungeon_set_warp_at_eliminate(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("wrong time");
        return 0;
    }

    if (!lua_isnumber(L, 2)) {
        SPDLOG_ERROR("wrong map index");
        return 0;
    }

    if (!lua_isnumber(L, 3)) {
        SPDLOG_ERROR("wrong X");
        return 0;
    }

    if (!lua_isnumber(L, 4)) {
        SPDLOG_ERROR("wrong Y");
        return 0;
    }

    const char* c_pszRegenFile = nullptr;

    if (lua_gettop(L) >= 5)
        c_pszRegenFile = lua_tostring(L, 5);

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        SPDLOG_ERROR("dungeon_set_warp_at_eliminate: no dungeon");
        return 0;
    }

    pDungeon->SetWarpAtEliminate(static_cast<int32_t>(lua_tonumber(L, 1)),
                                 static_cast<int32_t>(lua_tonumber(L, 2)),
                                 static_cast<int32_t>(lua_tonumber(L, 3)),
                                 static_cast<int32_t>(lua_tonumber(L, 4)),
                                 c_pszRegenFile);
    return 0;
}

int dungeon_new_jump(lua_State* L)
{
    if (lua_gettop(L) < 3) {
        SPDLOG_ERROR("not enough argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        SPDLOG_ERROR("wrong argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    int32_t lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));

    auto* pDungeon =
        CDungeonManager::instance().Create<QuestDungeon>(lMapIndex);
    if (!pDungeon) {
        SPDLOG_ERROR("cannot create dungeon %d", lMapIndex);
        lua_pushnumber(L, 0);
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    ch->WarpSet(pDungeon->GetMapIndex(),
                static_cast<long>(lua_tonumber(L, 2)) * 100,
                static_cast<long>(lua_tonumber(L, 3)) * 100);

    lua_pushnumber(L, pDungeon->GetMapIndex());
    return 1;
}

int dungeon_new_jump_all(lua_State* L)
{
    if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) ||
        !lua_isnumber(L, 3)) {
        SPDLOG_ERROR("not enough argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    int32_t lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));

    auto* pDungeon =
        CDungeonManager::instance().Create<QuestDungeon>(lMapIndex);
    if (!pDungeon) {
        SPDLOG_ERROR("cannot create dungeon %d", lMapIndex);
        lua_pushnumber(L, 0);
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    pDungeon->JumpAll(ch->GetMapIndex(), static_cast<int>(lua_tonumber(L, 2)),
                      static_cast<int>(lua_tonumber(L, 3)));
    lua_pushnumber(L, pDungeon->GetMapIndex());
    return 1;
}

int dungeon_new_jump_party(lua_State* L)
{
    if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) ||
        !lua_isnumber(L, 3)) {
        SPDLOG_ERROR("not enough argument");
        return 0;
    }

    int32_t lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));

    auto* pDungeon =
        CDungeonManager::instance().Create<QuestDungeon>(lMapIndex);
    if (!pDungeon) {
        SPDLOG_ERROR("cannot create dungeon %d", lMapIndex);
        lua_pushnumber(L, 0);
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (!ch) {
        SPDLOG_ERROR("d.new_jump_party: No character");
        lua_pushnumber(L, 0);
        return 1;
    }

    auto* party = ch->GetParty();
    if (!party) {
        SPDLOG_ERROR("d.new_jump_party: No party for %d:%s", ch->GetPlayerID(),
                     ch->GetName());
        lua_pushnumber(L, 0);
        return 1;
    }

    /*if (ch != party->GetLeader()) {
        SPDLOG_ERROR("d.new_jump_party: %d:%s isn't the leader",
            ch->GetPlayerID(), ch->GetName());
        return 0;
    }*/

    pDungeon->JumpParty(ch->GetParty(), ch->GetMapIndex(),
                        static_cast<int>(lua_tonumber(L, 2)),
                        static_cast<int>(lua_tonumber(L, 3)));
    lua_pushnumber(L, pDungeon->GetMapIndex());
    return 1;
}

int dungeon_jump_all(lua_State* L)
{
    if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    auto* pDungeon = CQuestManager::instance().GetCurrentDungeon();

    if (!pDungeon)
        return 0;

    pDungeon->JumpAll(pDungeon->GetMapIndex(),
                      static_cast<int>(lua_tonumber(L, 1)),
                      static_cast<int>(lua_tonumber(L, 2)));
    return 0;
}

int dungeon_jump_all_delayed(lua_State* L)
{
    if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    auto* pDungeon = CQuestManager::instance().GetCurrentDungeon();

    if (!pDungeon)
        return 0;

    pDungeon->JumpAllDelayed(static_cast<int>(lua_tonumber(L, 1)),
                      static_cast<int>(lua_tonumber(L, 2)), static_cast<int>(lua_tonumber(L, 3)));
    return 0;
}


int dungeon_warp_all(lua_State* L)
{
    if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    auto pDungeon = CQuestManager::instance().GetCurrentDungeon();

    if (!pDungeon)
        return 0;

    pDungeon->WarpAll(pDungeon->GetMapIndex(),
                      static_cast<int>(lua_tonumber(L, 1)),
                      static_cast<int>(lua_tonumber(L, 2)));
    return 0;
}

int dungeon_get_kill_stone_count(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
        lua_pushnumber(L, 0);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        lua_pushnumber(L, pDungeon->GetKillStoneCount());
        return 1;
    }

    lua_pushnumber(L, 0);
    return 1;
}

int dungeon_get_kill_mob_count(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
        lua_pushnumber(L, 0);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        lua_pushnumber(L, pDungeon->GetKillMobCount());
        return 1;
    }

    lua_pushnumber(L, 0);
    return 1;
}

int dungeon_unique_set_maxhp(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->UniqueSetMaxHP(lua_tostring(L, 1),
                                 static_cast<int64_t>(lua_tonumber(L, 2)));

    return 0;
}

int dungeon_unique_set_hp(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->UniqueSetHP(lua_tostring(L, 1),
                              static_cast<int64_t>(lua_tonumber(L, 2)));

    return 0;
}

int dungeon_unique_set_hp_perc(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->UniqueSetHPPerc(lua_tostring(L, 1),
                                  static_cast<int>(lua_tonumber(L, 2)));

    return 0;
}

int dungeon_unique_set_def_grade(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->UniqueSetDefGrade(lua_tostring(L, 1),
                                    static_cast<int>(lua_tonumber(L, 2)));

    return 0;
}

int dungeon_unique_get_hp_perc(lua_State* L)
{
    if (!lua_isstring(L, 1)) {
        lua_pushnumber(L, 0);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        lua_pushnumber(L, pDungeon->GetUniqueHpPerc(lua_tostring(L, 1)));
        return 1;
    }

    lua_pushnumber(L, 0);
    return 1;
}

int dungeon_is_unique_dead(lua_State* L)
{
    if (!lua_isstring(L, 1)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        lua_pushboolean(L, pDungeon->IsUniqueDead(lua_tostring(L, 1)) ? 1 : 0);
        return 1;
    }

    lua_pushboolean(L, 0);
    return 1;
}

int dungeon_purge_unique(lua_State* L)
{
    if (!lua_isstring(L, 1))
        return 0;
    SPDLOG_INFO("QUEST_DUNGEON_PURGE_UNIQUE %s", lua_tostring(L, 1));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->PurgeUnique(lua_tostring(L, 1));

    return 0;
}

int dungeon_purge_area(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isnumber(L, 4))
        return 0;
    SPDLOG_INFO("QUEST_DUNGEON_PURGE_AREA");

    int x1 = lua_tonumber(L, 1);
    int y1 = lua_tonumber(L, 2);
    int x2 = lua_tonumber(L, 3);
    int y2 = lua_tonumber(L, 4);

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    const int mapIndex = pDungeon->GetMapIndex();
    if (0 == mapIndex) {
        SPDLOG_ERROR("_purge_area: cannot get a map index with (%d, %d)", x1,
                     y1);
        return 0;
    }

    SECTREE_MAP* pSectree = SECTREE_MANAGER::instance().GetMap(mapIndex);
    if (nullptr != pSectree) {
        FPurgeArea func(x1, y1, x2, y2,
                        CQuestManager::instance().GetCurrentNPCCharacterPtr());
        pSectree->for_each(func);
    }

    return 0;
}

int dungeon_kill_unique(lua_State* L)
{
    if (!lua_isstring(L, 1))
        return 0;

    SPDLOG_INFO("QUEST_DUNGEON_KILL_UNIQUE %s", lua_tostring(L, 1));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->KillUnique(lua_tostring(L, 1));

    return 0;
}

int dungeon_spawn_stone_door(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
        return 0;

    SPDLOG_INFO("QUEST_DUNGEON_SPAWN_STONE_DOOR %s %s", lua_tostring(L, 1),
                lua_tostring(L, 2));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnStoneDoor(lua_tostring(L, 1), lua_tostring(L, 2));

    return 0;
}

int dungeon_spawn_wooden_door(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
        return 0;

    SPDLOG_INFO("QUEST_DUNGEON_SPAWN_WOODEN_DOOR %s %s", lua_tostring(L, 1),
                lua_tostring(L, 2));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnWoodenDoor(lua_tostring(L, 1), lua_tostring(L, 2));

    return 0;
}

int dungeon_spawn_move_group(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3))
        return 0;

    SPDLOG_INFO("QUEST_DUNGEON_SPAWN_MOVE_GROUP %d %s %s",
                (int)lua_tonumber(L, 1), lua_tostring(L, 2),
                lua_tostring(L, 3));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnMoveGroup(static_cast<uint32_t>(lua_tonumber(L, 1)),
                                 lua_tostring(L, 2), lua_tostring(L, 3));

    return 0;
}

int dungeon_spawn_move_unique(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3) ||
        !lua_isstring(L, 4))
        return 0;

    SPDLOG_INFO("QUEST_DUNGEON_SPAWN_MOVE_UNIQUE %s %d %s %s",
                lua_tostring(L, 1), (int)lua_tonumber(L, 2), lua_tostring(L, 3),
                lua_tostring(L, 4));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnMoveUnique(lua_tostring(L, 1),
                                  static_cast<uint32_t>(lua_tonumber(L, 2)),
                                  lua_tostring(L, 3), lua_tostring(L, 4));

    return 0;
}

int dungeon_spawn_unique(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3))
        return 0;

    SPDLOG_INFO("QUEST_DUNGEON_SPAWN_UNIQUE %s %d %s", lua_tostring(L, 1),
                (int)lua_tonumber(L, 2), lua_tostring(L, 3));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnUnique(lua_tostring(L, 1),
                              static_cast<uint32_t>(lua_tonumber(L, 2)),
                              lua_tostring(L, 3));

    return 0;
}

int dungeon_spawn(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
        return 0;

    SPDLOG_INFO("QUEST_DUNGEON_SPAWN %d %s", (int)lua_tonumber(L, 1),
                lua_tostring(L, 2));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->Spawn(static_cast<uint32_t>(lua_tonumber(L, 1)),
                        lua_tostring(L, 2));

    return 0;
}

int dungeon_set_stage(lua_State* L)
{
    if (!lua_isnumber(L, 1))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SetDungeonStage(lua_tonumber(L, 1));
    return 0;
}

int dungeon_get_stage(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pushnumber(L, pDungeon->GetDungeonStage());
    return 1;
}

int dungeon_get_prev_stage(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        lua_pushnumber(L, -1);
        return 1;
    }

    lua_pushnumber(L, pDungeon->GetPrevDungeonStage());
    return 1;
}

int dungeon_set_unique(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 2));

    if (pDungeon)
        pDungeon->SetUnique(lua_tostring(L, 1), vid);
    return 0;
}

int dungeon_get_unique_vid(lua_State* L)
{
    if (!lua_isstring(L, 1)) {
        lua_pushnumber(L, 0);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        lua_pushnumber(L, pDungeon->GetUniqueVid(lua_tostring(L, 1)));
        return 1;
    }

    lua_pushnumber(L, 0);
    return 1;
}

int dungeon_spawn_mob(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    uint32_t vid = 0;

    if (pDungeon) {
        uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
        int32_t x = static_cast<int32_t>(lua_tonumber(L, 2));
        int32_t y = static_cast<int32_t>(lua_tonumber(L, 3));
        float radius =
            lua_isnumber(L, 4) ? static_cast<float>(lua_tonumber(L, 4)) : 0;
        uint32_t count = (lua_isnumber(L, 5))
                             ? static_cast<uint32_t>(lua_tonumber(L, 5))
                             : 1;

        SPDLOG_INFO("dungeon_spawn_mob %d %d %d", dwVnum, x, y);

        if (count == 0)
            count = 1;

        while (count--) {
            if (radius < 1) {
                CHARACTER* ch = pDungeon->SpawnMob(dwVnum, x, y);
                if (ch && !vid)
                    vid = ch->GetVID();
            } else {
                float angle = Random::get(0, 999) *
                              boost::math::constants::pi<float>() * 2 / 1000;
                float r = Random::get(0, 999) * radius / 1000;

                int32_t nx = x + static_cast<int32_t>(r * cos(angle));
                int32_t ny = y + static_cast<int32_t>(r * sin(angle));

                CHARACTER* ch = pDungeon->SpawnMob(dwVnum, nx, ny);
                if (ch && !vid)
                    vid = ch->GetVID();
            }
        }
    }

    lua_pushnumber(L, vid);
    return 1;
}

int dungeon_spawn_mob_dir(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isnumber(L, 4)) {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    uint32_t vid = 0;

    if (pDungeon) {
        uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
        int32_t x = static_cast<int32_t>(lua_tonumber(L, 2));
        int32_t y = static_cast<int32_t>(lua_tonumber(L, 3));
        uint8_t dir = static_cast<int>(lua_tonumber(L, 4));

        CHARACTER* ch = pDungeon->SpawnMob(dwVnum, x, y, dir);
        if (ch && !vid)
            vid = ch->GetVID();
    }
    lua_pushnumber(L, vid);
    return 1;
}

int dungeon_spawn_mob_ac_dir(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isnumber(L, 4)) {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    uint32_t vid = 0;

    if (pDungeon) {
        uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
        int32_t x = static_cast<int32_t>(lua_tonumber(L, 2));
        int32_t y = static_cast<int32_t>(lua_tonumber(L, 3));
        uint8_t dir = static_cast<int>(lua_tonumber(L, 4));

        CHARACTER* ch = pDungeon->SpawnMob_ac_dir(dwVnum, x, y, dir);
        if (ch && !vid)
            vid = ch->GetVID();
    }
    lua_pushnumber(L, vid);
    return 1;
}

int dungeon_spawn_goto_mob(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isnumber(L, 4))
        return 0;

    int32_t lFromX = static_cast<int32_t>(lua_tonumber(L, 1));
    int32_t lFromY = static_cast<int32_t>(lua_tonumber(L, 2));
    int32_t lToX = static_cast<int32_t>(lua_tonumber(L, 3));
    int32_t lToY = static_cast<int32_t>(lua_tonumber(L, 4));

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SpawnGotoMob(lFromX, lFromY, lToX, lToY);

    return 0;
}

int dungeon_spawn_name_mob(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isstring(L, 4))
        return 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
        int32_t x = static_cast<int32_t>(lua_tonumber(L, 2));
        int32_t y = static_cast<int32_t>(lua_tonumber(L, 3));
        pDungeon->SpawnNameMob(dwVnum, x, y, lua_tostring(L, 4));
    }
    return 0;
}

int dungeon_spawn_group(lua_State* L)
{
    //
    // argument: vnum,x,y,radius,aggressive,count
    //
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
        !lua_isnumber(L, 4) || !lua_isnumber(L, 6)) {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    uint32_t vid = 0;

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        uint32_t group_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
        int32_t local_x = static_cast<int32_t>(lua_tonumber(L, 2)) * 100;
        int32_t local_y = static_cast<int32_t>(lua_tonumber(L, 3)) * 100;
        float radius = static_cast<float>(lua_tonumber(L, 4)) * 100;
        bool bAggressive = lua_toboolean(L, 5);
        uint32_t count = static_cast<uint32_t>(lua_tonumber(L, 6));

        CHARACTER* chRet = pDungeon->SpawnGroup(group_vnum, local_x, local_y,
                                                radius, bAggressive, count);
        if (chRet)
            vid = chRet->GetVID();
    }

    lua_pushnumber(L, vid);
    return 1;
}

int dungeon_join(lua_State* L)
{
    if (lua_gettop(L) < 1 || !lua_isnumber(L, 1)) {
        lua_pushnumber(L, 0);
        return 1;
    }

    int32_t lMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));
    auto pDungeon = CDungeonManager::instance().Create<QuestDungeon>(lMapIndex);
    if (!pDungeon) {
        lua_pushnumber(L, 0);
        return 1;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
        pDungeon->JoinParty(ch->GetParty());
    else if (!ch->GetParty())
        pDungeon->Join(ch);

    lua_pushnumber(L, pDungeon->GetMapIndex());
    return 1;
}

int dungeon_exit(lua_State* L) // 던전에 들어오기 전 위치로 보냄
{
    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    ch->ExitToSavedLocation();
    return 0;
}

int dungeon_exit_all(
    lua_State* L) // 던전에 있는 모든 사람을 던전에 들어오기 전 위치로 보냄
{
    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->ExitAll();

    return 0;
}

int dungeon_exit_all2(
    lua_State* L) // 던전에 있는 모든 사람을 던전에 들어오기 전 위치로 보냄
{
    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->ExitAllDungeon();

    return 0;
}

int dungeon_set_exit_location(lua_State* L)
{
    if (lua_gettop(L) < 3 ||
        (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))) {
        lua_pushnumber(L, 0);
        return 1;
    }
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();
    if (!pDungeon) {
        lua_pushnumber(L, 0);
        return 1;
    }

    pDungeon->SetDungeonExitLocation(static_cast<int32_t>(lua_tonumber(L, 1)),
                                     static_cast<int32_t>(lua_tonumber(L, 2)),
                                     static_cast<int32_t>(lua_tonumber(L, 3)));
    lua_pushnumber(L, 1);

    return 1;
}

struct FSayDungeonByItemGroup {
    const QuestDungeon::ItemGroup* item_group;
    std::string can_enter_ment;
    std::string cant_enter_ment;

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = static_cast<CHARACTER*>(ent);

            if (ch->IsPC()) {
                packet_script p;

                for (auto it = item_group->begin(); it != item_group->end();
                     it++) {
                    if (ch->CountSpecifyItem(it->first) >= it->second) {
                        p.skin = quest::CQuestManager::QUEST_SKIN_NORMAL;
                        p.script = can_enter_ment;
                        ch->GetDesc()->Send(HEADER_GC_SCRIPT, p);
                        return;
                    }
                }

                p.skin = quest::CQuestManager::QUEST_SKIN_NORMAL;
                p.script = cant_enter_ment;

                ch->GetDesc()->Send(HEADER_GC_SCRIPT, p);
            }
        }
    }
};

int dungeon_say_diff_by_item_group(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3)) {
        SPDLOG_INFO("QUEST wrong set flag");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        SPDLOG_ERROR("QUEST : no dungeon");
        return 0;
    }

    SECTREE_MAP* pMap =
        SECTREE_MANAGER::instance().GetMap(pDungeon->GetMapIndex());

    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", pDungeon->GetMapIndex());
        return 0;
    }
    FSayDungeonByItemGroup f;
    SPDLOG_INFO("diff_by_item");

    std::string group_name(lua_tostring(L, 1));
    f.item_group = pDungeon->GetItemGroup(group_name);

    if (f.item_group == nullptr) {
        SPDLOG_ERROR("invalid item group");
        return 0;
    }

    f.can_enter_ment = lua_tostring(L, 2);
    f.can_enter_ment += "[ENTER][ENTER][ENTER][ENTER][DONE]";
    f.cant_enter_ment = lua_tostring(L, 3);
    f.cant_enter_ment += "[ENTER][ENTER][ENTER][ENTER][DONE]";

    pMap->for_each(f);

    return 0;
}

struct FExitDungeonByItemGroup {
    const QuestDungeon::ItemGroup* item_group;

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = static_cast<CHARACTER*>(ent);

            if (ch->IsPC()) {
                for (auto it = item_group->begin(); it != item_group->end();
                     it++) {
                    if (ch->CountSpecifyItem(it->first) >= it->second) {
                        return;
                    }
                }
                auto dungeon = ch->GetDungeon();
                if (dungeon) {
                    dungeon->SkipPlayerSaveDungeonOnce();
                }
                ch->ExitToSavedLocation();
            }
        }
    }
};

int dungeon_exit_all_by_item_group(
    lua_State* L) // 특정 아이템 그룹에 속한 아이템이 없는사람은 강퇴
{
    if (!lua_isstring(L, 1)) {
        SPDLOG_INFO("QUEST wrong set flag");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        SPDLOG_ERROR("QUEST : no dungeon");
        return 0;
    }

    SECTREE_MAP* pMap =
        SECTREE_MANAGER::instance().GetMap(pDungeon->GetMapIndex());

    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", pDungeon->GetMapIndex());
        return 0;
    }
    FExitDungeonByItemGroup f;

    std::string group_name(lua_tostring(L, 1));
    f.item_group = pDungeon->GetItemGroup(group_name);

    if (f.item_group == nullptr) {
        SPDLOG_ERROR("invalid item group");
        return 0;
    }

    pMap->for_each(f);

    return 0;
}

struct FDeleteItemInItemGroup {
    const QuestDungeon::ItemGroup* item_group;

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = static_cast<CHARACTER*>(ent);

            if (ch->IsPC()) {
                for (auto it = item_group->begin(); it != item_group->end();
                     it++) {
                    if (ch->CountSpecifyItem(it->first) >= it->second) {
                        ch->RemoveSpecifyItem(it->first, it->second);
                        return;
                    }
                }
            }
        }
    }
};

int dungeon_delete_item_in_item_group_from_all(
    lua_State* L) // 특정 아이템을 던전 내 pc에게서 삭제.
{
    if (!lua_isstring(L, 1)) {
        SPDLOG_INFO("QUEST wrong set flag");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (!pDungeon) {
        SPDLOG_ERROR("QUEST : no dungeon");
        return 0;
    }

    SECTREE_MAP* pMap =
        SECTREE_MANAGER::instance().GetMap(pDungeon->GetMapIndex());

    if (!pMap) {
        SPDLOG_ERROR("cannot find map by index {}", pDungeon->GetMapIndex());
        return 0;
    }

    FDeleteItemInItemGroup f;

    std::string group_name(lua_tostring(L, 1));
    f.item_group = pDungeon->GetItemGroup(group_name);

    if (f.item_group == nullptr) {
        SPDLOG_ERROR("invalid item group");
        return 0;
    }

    pMap->for_each(f);

    return 0;
}

int dungeon_kill_all(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->KillAll();

    return 0;
}

int dungeon_purge(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->Purge();

    return 0;
}

int dungeon_exit_all_to_start_position(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->ExitAllToStartPosition();

    return 0;
}

int dungeon_count_monster(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        lua_pushnumber(L, pDungeon->CountMonster());
    else {
        SPDLOG_ERROR("not in a dungeon");
        lua_pushnumber(L, LONG_MAX);
    }

    return 1;
}

int dungeon_select(lua_State* L)
{
    const auto dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 1));
    if (dwMapIndex) {
        auto* pDungeon =
            CDungeonManager::instance().FindByMapIndex<QuestDungeon>(
                dwMapIndex);
        if (pDungeon) {
            CQuestManager::instance().SelectDungeon(pDungeon);
            lua_pushboolean(L, 1);
        } else {
            CQuestManager::instance().SelectDungeon(nullptr);
            lua_pushboolean(L, 0);
        }
    } else {
        CQuestManager::instance().SelectDungeon(nullptr);
        lua_pushboolean(L, 0);
    }
    return 1;
}

int dungeon_find(lua_State* L)
{
    const auto dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 1));
    if (dwMapIndex) {
        auto* pDungeon =
            CDungeonManager::instance().FindByMapIndex<QuestDungeon>(
                dwMapIndex);
        if (pDungeon) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

int dungeon_all_near_to(lua_State* L)
{
    auto* pDungeon = CQuestManager::instance().GetCurrentDungeon();

    if (pDungeon != nullptr)
        lua_pushboolean(L, pDungeon->IsAllPCNearTo(
                               static_cast<int>(lua_tonumber(L, 1)),
                               static_cast<int>(lua_tonumber(L, 2)), 30));
    else
        lua_pushboolean(L, false);

    return 1;
}

int dungeon_set_warp_location(lua_State* L)
{
    auto* pDungeon = CQuestManager::instance().GetCurrentDungeon();
    if (pDungeon == nullptr)
        return 0;

    if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) ||
        !lua_isnumber(L, 3))
        return 0;

    FSetWarpLocation f(static_cast<int>(lua_tonumber(L, 1)),
                       static_cast<int>(lua_tonumber(L, 2)),
                       static_cast<int>(lua_tonumber(L, 3)));
    pDungeon->ForEachMember(f);
    return 0;
}

int dungeon_set_item_group(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
        return 0;

    std::string group_name(lua_tostring(L, 1));
    int size = lua_tonumber(L, 2);

    QuestDungeon::ItemGroup item_group;
    for (int i = 0; i < size; i++) {
        if (!lua_isnumber(L, i * 2 + 3) || !lua_isnumber(L, i * 2 + 4))
            return 0;

        item_group.push_back(std::make_pair<uint32_t, int>(
            lua_tonumber(L, i * 2 + 3), lua_tonumber(L, i * 2 + 4)));
    }

    auto* pDungeon = CQuestManager::instance().GetCurrentDungeon();
    if (!pDungeon)
        return 0;

    pDungeon->CreateItemGroup(group_name, item_group);
    return 0;
}

int dungeon_set_quest_flag2(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();

    FSetQuestFlag f;
    if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3)) {
        SPDLOG_ERROR("dungeon_set_quest_flag2: Invalid Argument");
        return 0;
    }

    f.flagname = std::string(lua_tostring(L, 1)) + "." + lua_tostring(L, 2);
    f.value = static_cast<int>(lua_tonumber(L, 3));

    auto* pDungeon = q.GetCurrentDungeon();
    if (pDungeon)
        pDungeon->ForEachMember(f);

    return 0;
}

int dungeon_get_pids(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    uint32_t count = 0;
    const auto pDungeon = q.GetCurrentDungeon();
    if (pDungeon) {
        pDungeon->ForEachMember([&](CHARACTER* ch) {
            lua_pushnumber(L, ch->GetPlayerID());
            count++;
        });
    }

    return count;
}

int dungeon_unique_has_victim(lua_State* L)
{
    if (!lua_isstring(L, 1)) {
        lua_pushboolean(L, false);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        auto ch =
            g_pCharManager->Find(pDungeon->GetUniqueVid(lua_tostring(L, 1)));
        if (ch) {
            lua_pushboolean(L, ch->GetVictim() != nullptr);
            return 1;
        }
    }

    lua_pushboolean(L, false);
    return 1;
}

int dungeon_unique_apply(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        lua_pushboolean(L, false);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        auto ch =
            g_pCharManager->Find(pDungeon->GetUniqueVid(lua_tostring(L, 1)));
        if (ch) {
            ch->ApplyPoint(static_cast<uint32_t>(lua_tonumber(L, 2)),
                           static_cast<ApplyValue>(lua_tonumber(L, 3)));
            lua_pushboolean(L, true);
            return 1;
        }
    }

    lua_pushboolean(L, false);
    return 1;
}

int dungeon_unique_set_invincible(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        lua_pushboolean(L, false);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        auto ch =
            g_pCharManager->Find(pDungeon->GetUniqueVid(lua_tostring(L, 1)));
        if (ch) {
            if (lua_tonumber(L, 2) == 1)
                ch->SetArmada();
            else
                ch->ResetArmada();
            lua_pushboolean(L, true);
            return 1;
        }
    }

    lua_pushboolean(L, false);
    return 1;
}

int dungeon_unique_affect_add(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        lua_pushboolean(L, false);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        auto ch =
            g_pCharManager->Find(pDungeon->GetUniqueVid(lua_tostring(L, 1)));
        if (ch) {
            ch->AddAffect(static_cast<uint32_t>(lua_tonumber(L, 2)),
                          static_cast<int32_t>(lua_tonumber(L, 3)),
                          static_cast<int32_t>(lua_tonumber(L, 4)),
                          static_cast<int32_t>(lua_tonumber(L, 5)), 0, false);
            lua_pushboolean(L, true);
            return 1;
        }
    }

    lua_pushboolean(L, false);
    return 1;
}

int dungeon_unique_remove_affect(lua_State* L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        lua_pushboolean(L, false);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto pDungeon = q.GetCurrentDungeon();

    if (pDungeon) {
        auto ch =
            g_pCharManager->Find(pDungeon->GetUniqueVid(lua_tostring(L, 1)));
        if (ch) {
            ch->RemoveAffect(static_cast<uint32_t>(lua_tonumber(L, 2)));
            lua_pushboolean(L, true);
            return 1;
        }
    }

    lua_pushboolean(L, false);
    return 1;
}

#ifdef ENABLE_HYDRA_DUNGEON
int dungeon_create_hydra(lua_State* L)
{
    lua_pushboolean(L, CHydraDungeonManager::instance().CreateDungeon(
                           CQuestManager::instance().GetCurrentCharacterPtr()));
    return 1;
}
#endif

ALUA(dungeon_set_boss_vnum)
// int dungeon_set_boss_vnum(lua_State* L)
{
    if (!lua_isnumber(L, 1))
        return 1;

    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();
    if (!pDungeon)
        return 1;

    pDungeon->AddBossVnum((int)lua_tonumber(L, 1));
    return 1;
}

ALUA(dungeon_get_boss_max_dmg)
// int dungeon_get_boss_max_dmg(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();
    if (!pDungeon) {
        lua_pushnumber(L, 0);
        return 1;
    }
    if (!lua_isnumber(L, 1)) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, pDungeon->GetMaxDmgOnBoss(lua_tonumber(L, 1)));
    return 1;
}

int dungeon_completed(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->Completed();

    return 0;
}

int dungeon_set_rejoin_pos(lua_State* L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
        lua_pushboolean(L, false);
        return 1;
    }

    CQuestManager& q = CQuestManager::instance();
    auto* pDungeon = q.GetCurrentDungeon();

    if (pDungeon)
        pDungeon->SetRejoinPosition(static_cast<int32_t>(lua_tonumber(L, 1)), static_cast<int32_t>(lua_tonumber(L, 2)));

    return 0;
}


void RegisterDungeonFunctionTable()
{
    luaL_reg dungeon_functions[] = {
        {"join", dungeon_join},
        {"exit", dungeon_exit},
        {"exit_all", dungeon_exit_all},
        {"set_item_group", dungeon_set_item_group},
        {"exit_all_by_item_group", dungeon_exit_all_by_item_group},
        {"say_diff_by_item_group", dungeon_say_diff_by_item_group},
        {"delete_item_in_item_group_from_all", dungeon_delete_item_in_item_group_from_all},
        {"purge", dungeon_purge},
        {"kill_all", dungeon_kill_all},
        {"set_stage", dungeon_set_stage},
        {"get_stage", dungeon_get_stage},
        {"get_prev_stage", dungeon_get_prev_stage},
        {"spawn", dungeon_spawn},
        {"spawn_mob", dungeon_spawn_mob},
        {"spawn_mob_dir", dungeon_spawn_mob_dir},
        {"spawn_mob_ac_dir", dungeon_spawn_mob_ac_dir},
        {"spawn_name_mob", dungeon_spawn_name_mob},
        {"spawn_goto_mob", dungeon_spawn_goto_mob},
        {"spawn_group", dungeon_spawn_group},
        {"spawn_unique", dungeon_spawn_unique},
        {"spawn_move_unique", dungeon_spawn_move_unique},
        {"spawn_move_group", dungeon_spawn_move_group},
        {"spawn_stone_door", dungeon_spawn_stone_door},
        {"spawn_wooden_door", dungeon_spawn_wooden_door},
        {"purge_unique", dungeon_purge_unique},
        {"purge_area", dungeon_purge_area},
        {"kill_unique", dungeon_kill_unique},
        {"is_unique_dead", dungeon_is_unique_dead},
        {"unique_get_hp_perc", dungeon_unique_get_hp_perc},
        {"unique_set_def_grade", dungeon_unique_set_def_grade},
        {"unique_set_hp", dungeon_unique_set_hp},
        {"unique_set_hp_perc", dungeon_unique_set_hp_perc},
        {"unique_set_maxhp", dungeon_unique_set_maxhp},
        {"get_unique_vid", dungeon_get_unique_vid},
        {"get_kill_stone_count", dungeon_get_kill_stone_count},
        {"get_kill_mob_count", dungeon_get_kill_mob_count},
        {"jump_all", dungeon_jump_all},
        {"warp_all", dungeon_warp_all},
        {"new_jump_all", dungeon_new_jump_all},
        {"new_jump_party", dungeon_new_jump_party},
        {"new_jump", dungeon_new_jump},
        {"regen_file", dungeon_regen_file},
        {"set_regen_file", dungeon_set_regen_file},
        {"clear_regen", dungeon_clear_regen},
        {"set_exit_all_at_eliminate", dungeon_set_exit_all_at_eliminate},
        {"set_warp_at_eliminate", dungeon_set_warp_at_eliminate},
        {"set_event_at_eliminate", dungeon_set_event_at_eliminate},
        {"get_map_index", dungeon_get_map_index},
        {"get_base_map_index", dungeon_get_base_map_index},
        {"check_eliminated", dungeon_check_eliminated},
        {"exit_all_to_start_position", dungeon_exit_all_to_start_position},
        {"count_monster", dungeon_count_monster},
        {"setf", dungeon_set_flag},
        {"getf", dungeon_get_flag},
        {"getf_from_map_index", dungeon_get_flag_from_map_index},
        {"set_unique", dungeon_set_unique},
        {"select", dungeon_select},
        {"find", dungeon_find},
        {"notice", dungeon_notice},
        {"setqf", dungeon_set_quest_flag},
        {"all_near_to", dungeon_all_near_to},
        {"set_warp_location", dungeon_set_warp_location},
        {"setqf2", dungeon_set_quest_flag2},
        {"get_pids", dungeon_get_pids},
        {"unique_has_victim", dungeon_unique_has_victim},
        {"unique_apply", dungeon_unique_apply},
        {"unique_set_invincible", dungeon_unique_set_invincible},

        {"unique_affect_add", dungeon_unique_affect_add},
        {"unique_affect_remove", dungeon_unique_remove_affect},

#ifdef ENABLE_HYDRA_DUNGEON
        {"create_hydra", dungeon_create_hydra},
#endif
        {"set_mission_message", dungeon_set_mission_message},
        {"set_mission_sub_message", dungeon_set_mission_sub_message},
        {"clear_mission_message", dungeon_clear_mission_message},

        {"set_exit_location", dungeon_set_exit_location},
        {"exit_dungeon", dungeon_exit_all2},
        {"add_boss_vnum", dungeon_set_boss_vnum},
        {"get_boss_max_dmg", dungeon_get_boss_max_dmg},
        {"completed", dungeon_completed},
        {"set_rejoin_pos", dungeon_set_rejoin_pos},
        {"jump_all_delayed", dungeon_jump_all_delayed},

        {nullptr, nullptr}};

    CQuestManager::instance().AddLuaFunctionTable("d", dungeon_functions);
}
} // namespace quest
