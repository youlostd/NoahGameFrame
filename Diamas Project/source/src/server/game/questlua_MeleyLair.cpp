#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "questlua.h"
#include "questmanager.h"
#include "MeleyLair.h"
#include "char.h"

namespace quest
{
int32_t MeleyLair_GetPartecipantsLimit(lua_State *L)
{
    lua_pushnumber(L, (int32_t)(MeleyLair::PARTECIPANTS_LIMIT));
    return 1;
}

int32_t MeleyLair_GetSubMapIndex(lua_State *L)
{
    lua_pushnumber(L, (int32_t)(MeleyLair::SUBMAP_INDEX));
    return 1;
}

int32_t MeleyLair_Enter(lua_State *L)
{
    CHARACTER *pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
    if (pkChar)
    {
        bool bTry = MeleyLair::CMgr::instance().Enter(pkChar);
        lua_pushboolean(L, bTry);
    }
    else
    {
        lua_pushboolean(L, false);
        lua_pushnumber(L, 1);
    }

    return 2;
}

int32_t MeleyLair_IsMeleyMap(lua_State *L)
{
    CHARACTER *pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
    bool bTry = pkChar != nullptr ? MeleyLair::CMgr::instance().IsMeleyMap(pkChar->GetMapIndex()) : false;
    lua_pushboolean(L, bTry);

    return 1;
}

int32_t MeleyLair_Check(lua_State *L)
{
    CHARACTER *pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
    if (pkChar)
        MeleyLair::CMgr::instance().Check(pkChar);

    return 0;
}

int32_t MeleyLair_Leave(lua_State *L)
{
    CHARACTER *pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
    if (pkChar)
        MeleyLair::CMgr::instance().LeaveRequest(pkChar);

    return 0;
}

int32_t MeleyLair_CanGetReward(lua_State *L)
{
    CHARACTER *pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
    bool bTry = pkChar != nullptr ? MeleyLair::CMgr::instance().CanGetReward(pkChar) : false;
    lua_pushboolean(L, bTry);

    return 1;
}

int32_t MeleyLair_Reward(lua_State *L)
{
    if (!lua_isnumber(L, 1))
        return 0;

    uint8_t bReward = (uint8_t)lua_tonumber(L, 1);
    CHARACTER *pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
    if (pkChar)
        MeleyLair::CMgr::instance().Reward(pkChar, bReward);

    return 0;
}

int32_t MeleyLair_OpenRanking(lua_State *L)
{
    MeleyLair::CMgr::instance().OpenRanking(CQuestManager::instance().GetCurrentCharacterPtr());
    return 0;
}

void RegisterMeleyLairFunctionTable()
{
    luaL_reg functions[] =
    {
        {"GetPartecipantsLimit", MeleyLair_GetPartecipantsLimit},
        {"GetSubMapIndex", MeleyLair_GetSubMapIndex},
        {"Enter", MeleyLair_Enter},
        {"IsMeleyMap", MeleyLair_IsMeleyMap},
        {"Check", MeleyLair_Check},
        {"Leave", MeleyLair_Leave},
        {"CanGetReward", MeleyLair_CanGetReward},
        {"Reward", MeleyLair_Reward},
        {"OpenRanking", MeleyLair_OpenRanking},
        {nullptr, nullptr}
    };

    CQuestManager::instance().AddLuaFunctionTable("MeleyLair", functions);
}
}

#endif
