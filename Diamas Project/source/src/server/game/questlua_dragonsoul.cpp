#include "config.h"
#include "questmanager.h"
#include "char.h"

namespace quest
{
int ds_open_refine_window(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (nullptr == ch)
    {
        SPDLOG_ERROR("NULL POINT ERROR");
        return 0;
    }
    if (ch->DragonSoul_IsQualified())
        ch->DragonSoul_RefineWindow_Open(true);

    return 0;
}

int ds_give_qualification(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (nullptr == ch)
    {
        SPDLOG_ERROR("NULL POINT ERROR");
        return 0;
    }
    ch->DragonSoul_GiveQualification();

    return 0;
}

int ds_is_qualified(lua_State *L)
{
    const CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (nullptr == ch)
    {
        SPDLOG_ERROR("NULL POINT ERROR");
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, ch->DragonSoul_IsQualified());
    return 1;
}

void RegisterDragonSoulFunctionTable()
{
    luaL_reg ds_functions[] =
    {
        {"open_refine_window", ds_open_refine_window},
        {"give_qualification", ds_give_qualification},
        {"is_qualified", ds_is_qualified},
        {nullptr, nullptr}
    };

    CQuestManager::instance().AddLuaFunctionTable("ds", ds_functions);
}
};
