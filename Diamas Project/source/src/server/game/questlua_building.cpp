#include "config.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "char.h"
#include "guild.h"
#include "db.h"
#include "building.h"

namespace quest
{
//
// "building" Lua functions
//
int building_get_land_id(lua_State *L)
{
    using namespace building;

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
    {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    CLand *pkLand = CManager::instance().FindLand((int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2),
                                                  (int)lua_tonumber(L, 3));
    lua_pushnumber(L, pkLand ? pkLand->GetID() : 0);
    return 1;
}

int building_get_land_info(lua_State *L)
{
    Gold price = 1000000000;
    int owner = 0;
    int level_limit = 0;

    if (lua_isnumber(L, 1))
    {
        using namespace building;

        CLand *pkLand = CManager::instance().FindLand((uint32_t)lua_tonumber(L, 1));

        if (pkLand)
        {
            const TLand &t = pkLand->GetData();

            price = t.price;
            owner = t.guildId;
            level_limit = t.guildLevelLimit;
        }
    }
    else
        SPDLOG_ERROR("invalid argument");

    lua_pushnumber(L, price);
    lua_pushnumber(L, owner);
    lua_pushnumber(L, level_limit);
    return 3;
}

int building_set_land_owner(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
    {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    using namespace building;

    CLand *pkLand = CManager::instance().FindLand((uint32_t)lua_tonumber(L, 1));

    if (pkLand)
    {
        if (pkLand->GetData().guildId == 0)
            pkLand->SetOwner((uint32_t)lua_tonumber(L, 2));
    }

    return 0;
}

int building_has_land(lua_State *L)
{
    using namespace building;

    if (!lua_isnumber(L, 1))
    {
        SPDLOG_ERROR("invalid argument");
        lua_pushboolean(L, true);
        return 1;
    }

    /*
    if (CManager::instance().FindLandByGuild((uint32_t) lua_tonumber(L, 1)))
        lua_pushboolean(L, true);
    else
        lua_pushboolean(L, false);
    */

    std::unique_ptr<SQLMsg> pmsg(
        DBManager::instance().DirectQuery("SELECT COUNT(*) FROM land WHERE guild_id = {}",
                                          (uint32_t)lua_tonumber(L, 1)));

    if (pmsg->Get()->uiNumRows > 0)
    {
        MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

        int count = 0;
        str_to_number(count, row[0]);

        if (count == 0) { lua_pushboolean(L, false); }
        else { lua_pushboolean(L, true); }
    }
    else { lua_pushboolean(L, true); }

    return 1;
}

int building_reconstruct(lua_State *L)
{
    using namespace building;

    uint32_t dwNewBuilding = (uint32_t)lua_tonumber(L, 1);

    CQuestManager &q = CQuestManager::instance();

    CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
    if (!npc)
        return 0;

    CGuild *pGuild = npc->GetGuild();
    if (!pGuild)
        return 0;

    CLand *pLand = CManager::instance().FindLandByGuild(pGuild->GetID());
    if (!pLand)
        return 0;

    CObject *pObject = pLand->FindObjectByNPC(npc);
    if (!pObject)
        return 0;

    pObject->Reconstruct(dwNewBuilding);

    return 0;
}

void RegisterBuildingFunctionTable()
{
    luaL_reg functions[] =
    {
        {"get_land_id", building_get_land_id},
        {"get_land_info", building_get_land_info},
        {"set_land_owner", building_set_land_owner},
        {"has_land", building_has_land},
        {"reconstruct", building_reconstruct},
        {nullptr, nullptr}
    };

    CQuestManager::instance().AddLuaFunctionTable("building", functions);
}
}
