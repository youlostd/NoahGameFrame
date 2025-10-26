#include "utils.h"
#include "config.h"
#include "questmanager.h"
#include "char.h"
#include "party.h"
#include "xmas_event.h"
#include "char_manager.h"
#include "shop_manager.h"
#include "guild.h"
#include "sectree_manager.h"

namespace quest
{
//
// "npc" lua functions
//
int npc_open_shop(lua_State *L)
{
    int iShopVnum = 0;

    if (lua_gettop(L) == 1)
    {
        if (lua_isnumber(L, 1))
            iShopVnum = (int)lua_tonumber(L, 1);
    }

    if (CQuestManager::instance().GetCurrentNPCCharacterPtr())
        CShopManager::instance().StartShopping(CQuestManager::instance().GetCurrentCharacterPtr(),
                                               CQuestManager::instance().GetCurrentNPCCharacterPtr(), iShopVnum);
    return 0;
}

int npc_get_name(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
    if (npc)
        lua_pushstring(L, npc->GetName().c_str());
    else
        lua_pushstring(L, "");

    return 1;
}

int npc_is_pc(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
    if (npc && npc->IsPC())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int npc_get_empire(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
    if (npc)
        lua_pushnumber(L, npc->GetEmpire());
    else
        lua_pushnumber(L, 0);
    return 1;
}

int npc_get_race(lua_State *L)
{
    lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCRace());
    return 1;
}

int npc_get_guild(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
    CGuild *pGuild = nullptr;
    if (npc)
        pGuild = npc->GetGuild();

    lua_pushnumber(L, pGuild ? pGuild->GetID() : 0);
    return 1;
}

int npc_get_remain_skill_book_count(lua_State *L)
{
    CHARACTER *npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
    if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
    {
        lua_pushnumber(L, 0);
        return 1;
    }
    lua_pushnumber(L, std::max<int>(0, npc->GetPoint(POINT_ATT_GRADE_BONUS)));
    return 1;
}

int npc_dec_remain_skill_book_count(lua_State *L)
{
    CHARACTER *npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
    if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM) { return 0; }
    npc->SetPoint(POINT_ATT_GRADE_BONUS, std::max<int>(0, npc->GetPoint(POINT_ATT_GRADE_BONUS) - 1));
    return 0;
}

int npc_get_remain_hairdye_count(lua_State *L)
{
    CHARACTER *npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
    if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
    {
        lua_pushnumber(L, 0);
        return 1;
    }
    lua_pushnumber(L, std::max<int>(0, npc->GetPoint(POINT_DEF_GRADE_BONUS)));
    return 1;
}

int npc_dec_remain_hairdye_count(lua_State *L)
{
    CHARACTER *npc = CQuestManager::instance().GetCurrentNPCCharacterPtr();
    if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM) { return 0; }
    npc->SetPoint(POINT_DEF_GRADE_BONUS, std::max<int>(0, npc->GetPoint(POINT_DEF_GRADE_BONUS) - 1));
    return 0;
}

int npc_is_quest(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    if (npc)
    {
        const std::string &r_st = q.GetCurrentQuestName();

        if (q.GetQuestIndexByName(r_st) == npc->GetQuestBy())
        {
            lua_pushboolean(L, 1);
            return 1;
        }
    }

    lua_pushboolean(L, 0);
    return 1;
}

int npc_kill(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    ch->SetQuestNPCID(0);
    if (npc) { npc->Dead(); }
    return 0;
}

int npc_purge(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    ch->SetQuestNPCID(0);
    if (npc) { M2_DESTROY_CHARACTER(npc); }
    return 0;
}

int npc_is_near(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    lua_Number dist = 10;

    if (lua_isnumber(L, 1))
        dist = lua_tonumber(L, 1);

    if (ch == nullptr || npc == nullptr) { lua_pushboolean(L, false); }
    else { lua_pushboolean(L, DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist * 100); }

    return 1;
}

int npc_is_near_vid(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        SPDLOG_ERROR("invalid vid");
        lua_pushboolean(L, 0);
        return 1;
    }

    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = g_pCharManager->Find((uint32_t)lua_tonumber(L, 1));
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    lua_Number dist = 10;

    if (lua_isnumber(L, 2))
        dist = lua_tonumber(L, 2);

    if (ch == nullptr || npc == nullptr) { lua_pushboolean(L, false); }
    else { lua_pushboolean(L, DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist * 100); }

    return 1;
}

int npc_unlock(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    if (npc != nullptr)
    {
        if (npc->IsPC())
            return 0;

        if (npc->GetQuestNPCID() == ch->GetPlayerID()) { npc->SetQuestNPCID(0); }
    }
    return 0;
}

int npc_lock(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    if (!npc || npc->IsPC())
    {
        lua_pushboolean(L, true);
        return 1;
    }

    if (npc->GetQuestNPCID() == 0 || npc->GetQuestNPCID() == ch->GetPlayerID())
    {
        npc->SetQuestNPCID(ch->GetPlayerID());
        lua_pushboolean(L, true);
    }
    else { lua_pushboolean(L, false); }

    return 1;
}

int npc_get_leader_vid(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    auto party = npc->GetParty();
    CHARACTER *leader = party->GetLeader();

    if (leader)
        lua_pushnumber(L, leader->GetVID());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int npc_get_vid(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

    lua_pushnumber(L, npc->GetVID());

    return 1;
}

int npc_get_vid_attack_mul(lua_State *L)
{
    //CQuestManager& q = CQuestManager::instance();

    uint32_t vid = (uint32_t)lua_tonumber(L, 1);
    CHARACTER *targetChar = g_pCharManager->Find(vid);

    if (targetChar)
        lua_pushnumber(L, targetChar->GetAttMul());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int npc_set_vid_attack_mul(lua_State *L)
{
    //CQuestManager& q = CQuestManager::instance();

    uint32_t vid = (uint32_t)lua_tonumber(L, 1);
    float attack_mul = (float)lua_tonumber(L, 2);

    CHARACTER *targetChar = g_pCharManager->Find(vid);

    if (targetChar)
        targetChar->SetAttMul(attack_mul);

    return 0;
}

int npc_get_vid_damage_mul(lua_State *L)
{
    //CQuestManager& q = CQuestManager::instance();

    uint32_t vid = (uint32_t)lua_tonumber(L, 1);
    CHARACTER *targetChar = g_pCharManager->Find(vid);

    if (targetChar)
        lua_pushnumber(L, targetChar->GetDamMul());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int npc_set_vid_damage_mul(lua_State *L)
{
    //CQuestManager& q = CQuestManager::instance();

    uint32_t vid = (uint32_t)lua_tonumber(L, 1);
    float damage_mul = (float)lua_tonumber(L, 2);

    CHARACTER *targetChar = g_pCharManager->Find(vid);

    if (targetChar)
        targetChar->SetDamMul(damage_mul);

    return 0;
}

/**
* Gets the level of a given NPC
* If no param is supplied it will return the current when kill npc vnum back
* Edited by MartPwnS 04.01.2014
*/
int npc_get_level(lua_State *L)
{
    uint32_t vid = (uint32_t)lua_tonumber(L, 1);
    if (vid)
    {
        CHARACTER *ch = g_pCharManager->Find((uint32_t)lua_tonumber(L, 1));

        if (!ch)
        {
            lua_pushnumber(L, 0);
            return 0;
        }

        lua_pushnumber(L, ch->GetLevel());
    }
    else
    {
        CQuestManager &q = CQuestManager::instance();
        CHARACTER *npc = q.GetCurrentNPCCharacterPtr();

        if (!npc)
        {
            lua_pushnumber(L, 0);
            return 0;
        }

        lua_pushnumber(L, npc->GetLevel());
    }

    return 1;
}

/**
* Sends an effect to a given npc
* Arguments: vid, effect_filename
* Added by Think on 20.02.2014
*/
int npc_send_effect(lua_State *L)
{
    const lua_Number vid = lua_tonumber(L, 1);
    const char *fname = lua_tostring(L, 2);

    CHARACTER *npc = g_pCharManager->Find((uint32_t)vid);
    if (vid == 0 && !vid)
    {
        CQuestManager &q = CQuestManager::instance();
        npc = q.GetCurrentNPCCharacterPtr();
    }

    if (fname[0] != '\0' && npc)
        npc->SpecificEffectPacket(fname);

    return 0;
}

/**
* Get the local x coordinate of the current quest npc
* Port of the pc function
* 17/May/14 - Think
*/
int npc_get_local_x(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
    if (!npc)
        return 0;

    SECTREE_MAP *pMap = SECTREE_MANAGER::instance().GetMap(npc->GetMapIndex());

    if (pMap)
        lua_pushnumber(L, (npc->GetX()) / 100);
    else
        lua_pushnumber(L, npc->GetX() / 100);

    return 1;
}

/**
* Get the local y coordinate of the current quest npc.
* Port of the pc function
* 17/May/14 - Think
*/
int npc_get_local_y(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
    if (!npc)
        return 0;

    SECTREE_MAP *pMap = SECTREE_MANAGER::instance().GetMap(npc->GetMapIndex());

    if (pMap)
        lua_pushnumber(L, npc->GetY() / 100);
    else
        lua_pushnumber(L, npc->GetY() / 100);

    return 1;
}

void RegisterNPCFunctionTable()
{
    luaL_reg npc_functions[] =
    {
        {"getrace", npc_get_race},
        {"get_race", npc_get_race},
        {"open_shop", npc_open_shop},
        {"get_empire", npc_get_empire},
        {"is_pc", npc_is_pc},
        {"get_name", npc_get_name},
        {"is_quest", npc_is_quest},
        {"kill", npc_kill},
        {"purge", npc_purge},
        {"is_near", npc_is_near},
        {"is_near_vid", npc_is_near_vid},
        {"lock", npc_lock},
        {"unlock", npc_unlock},
        {"get_guild", npc_get_guild},
        {"get_leader_vid", npc_get_leader_vid},
        {"get_vid", npc_get_vid},
        {"get_vid_attack_mul", npc_get_vid_attack_mul},
        {"set_vid_attack_mul", npc_set_vid_attack_mul},
        {"get_vid_damage_mul", npc_get_vid_damage_mul},
        {"set_vid_damage_mul", npc_set_vid_damage_mul},

        // NEW NPC COMMANDS: 
        {"get_level", npc_get_level},
        {"send_effect", npc_send_effect},
        {"get_local_x", npc_get_local_x},
        {"get_local_y", npc_get_local_y},

        // X-mas santa special
        {"get_remain_skill_book_count", npc_get_remain_skill_book_count},
        {"dec_remain_skill_book_count", npc_dec_remain_skill_book_count},
        {"get_remain_hairdye_count", npc_get_remain_hairdye_count},
        {"dec_remain_hairdye_count", npc_dec_remain_hairdye_count},

        {nullptr, nullptr}
    };

    CQuestManager::instance().AddLuaFunctionTable("npc", npc_functions);
}
};
