#include "char.h"
#include "char_manager.h"
#include "wedding.h"
#include "questmanager.h"
#include "utils.h"
#include "config.h"

extern int g_nPortalLimitTime;

namespace quest
{
int marriage_engage_to(lua_State *L)
{
    uint32_t vid = (uint32_t)lua_tonumber(L, 1);
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    CHARACTER *ch_you = g_pCharManager->Find(vid);
    if (ch_you)
    {
        marriage::CManager::instance().RequestAdd(ch->GetPlayerID(), ch_you->GetPlayerID(), ch->GetName().c_str(),
                                                  ch_you->GetName().c_str());
    }
    return 0;
}

int marriage_remove(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("{}: is not married cannot remove marriage", ch->GetName());
        return 0;
    }
    marriage::CManager::instance().RequestRemove(ch->GetPlayerID(), pMarriage->GetOther(ch->GetPlayerID()));
    return 0;
}

int marriage_set_to_marriage(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    pMarriage->SetMarried();
    return 0;
}

int marriage_find_married_vid(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    uint32_t vid = 0;
    if (pMarriage)
    {
        CHARACTER *you = g_pCharManager->FindByPID(pMarriage->GetOther(ch->GetPlayerID()));
        if (you)
            vid = you->GetVID();
    }

    lua_pushnumber(L, vid);

    return 1;
}

struct FBuildLuaWeddingMapList
{
    lua_State *L;
    int m_count;

    FBuildLuaWeddingMapList(lua_State *L)
        : L(L), m_count(1) { lua_newtable(L); }

    void operator()(marriage::TMarriage *pMarriage)
    {
        if (!pMarriage->pWeddingInfo)
            return;

        lua_newtable(L);
        lua_pushnumber(L, pMarriage->m_pid1);
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, pMarriage->m_pid2);
        lua_rawseti(L, -2, 2);
        lua_pushstring(L, pMarriage->name1.c_str());
        lua_rawseti(L, -2, 3);
        lua_pushstring(L, pMarriage->name2.c_str());
        lua_rawseti(L, -2, 4);
        lua_rawseti(L, -2, m_count++);
    }
};

int marriage_get_wedding_list(lua_State *L)
{
    marriage::CManager::instance().for_each_wedding(FBuildLuaWeddingMapList(L));
    return 1;
}

int marriage_join_wedding(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
    {
        SPDLOG_ERROR("invalid player id for wedding map");
        return 0;
    }

    uint32_t pid1 = (uint32_t)lua_tonumber(L, 1);
    uint32_t pid2 = (uint32_t)lua_tonumber(L, 2);

    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(pid1);
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    if (pMarriage->GetOther(pid1) != pid2)
    {
        SPDLOG_ERROR("not married %u %u", pid1, pid2);
        return 0;
    }

    //PREVENT_HACK
    if (ch->IsHack())
        return 0;
    //END_PREVENT_HACK
    pMarriage->WarpToWeddingMap(ch->GetPlayerID());
    return 0;
}

int marriage_warp_to_my_marriage_map(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }

    //PREVENT_HACK
    if (ch->IsHack())
        return 0;
    //END_PREVENT_HACK

    pMarriage->WarpToWeddingMap(ch->GetPlayerID());
    return 0;
}

int marriage_end_wedding(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    if (pMarriage->pWeddingInfo)
    {
        // 결혼식 끝내기 요청
        pMarriage->RequestEndWedding();
    }
    return 0;
}

int marriage_wedding_dark(lua_State *L)
{
    if (!lua_isboolean(L, 1))
    {
        SPDLOG_ERROR("invalid argument 1 : must be boolean");
        return 0;
    }
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());

    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    if (pMarriage->pWeddingInfo)
    {
        auto pWedding = marriage::WeddingManager::instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
        if (pWedding)
            pWedding.value()->SetDark(lua_toboolean(L, 1));
    }

    return 0;
}

int marriage_wedding_client_command(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        SPDLOG_ERROR("invalid argument 1 : must be string");
        return 0;
    }

    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    if (pMarriage->pWeddingInfo)
    {
        auto pWedding = marriage::WeddingManager::instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
        if (pWedding)
            pWedding.value()->ShoutInMap(CHAT_TYPE_COMMAND, lua_tostring(L, 1));
    }
    return 0;
}

int marriage_wedding_is_playing_music(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    if (pMarriage->pWeddingInfo)
    {
        auto pWedding = marriage::WeddingManager::instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
        if (pWedding)
            lua_pushboolean(L, pWedding.value()->IsPlayingMusic());
        else
            lua_pushboolean(L, false);
    }

    lua_pushboolean(L, false);
    return 1;
}

int marriage_wedding_music(lua_State *L)
{
    if (!lua_isboolean(L, 1))
    {
        SPDLOG_ERROR("invalid argument 1 : must be boolean");
        return 0;
    }
    if (!lua_isstring(L, 2))
    {
        SPDLOG_ERROR("invalid argument 2 : must be string");
        return 0;
    }

    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    if (pMarriage->pWeddingInfo)
    {
        auto pWedding = marriage::WeddingManager::instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
        if (pWedding)
            pWedding.value()->SetMusic(
                lua_toboolean(L, 1),
                lua_tostring(L, 2)
            );
    }
    return 0;
}

int marriage_wedding_snow(lua_State *L)
{
    if (!lua_isboolean(L, 1))
    {
        SPDLOG_ERROR("invalid argument 1 : must be boolean");
        return 0;
    }
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (!pMarriage)
    {
        SPDLOG_ERROR("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
        return 0;
    }
    if (pMarriage->pWeddingInfo)
    {
        auto pWedding = marriage::WeddingManager::instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
        if (pWedding)
            pWedding.value()->SetSnow(lua_toboolean(L, 1));
    }
    return 0;
}

int marriage_in_my_wedding(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());
    if (pMarriage->pWeddingInfo)
    {
        lua_pushboolean(L, (uint32_t)ch->GetMapIndex() == pMarriage->pWeddingInfo->dwMapIndex);
    }
    else { lua_pushboolean(L, 0); }
    return 1;
}

int marriage_get_married_time(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(ch->GetPlayerID());

    if (!pMarriage)
    {
        SPDLOG_ERROR("trying to get time for not married character");
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, get_global_time() - pMarriage->marry_time);
    return 1;
}

void RegisterMarriageFunctionTable()
{
    luaL_reg marriage_functions[] =
    {
        {"engage_to", marriage_engage_to},
        {"remove", marriage_remove},
        {"find_married_vid", marriage_find_married_vid},
        {"get_wedding_list", marriage_get_wedding_list},
        {"join_wedding", marriage_join_wedding},
        {"set_to_marriage", marriage_set_to_marriage},
        {"end_wedding", marriage_end_wedding},
        {"wedding_dark", marriage_wedding_dark},
        {"wedding_snow", marriage_wedding_snow},
        {"wedding_music", marriage_wedding_music},
        {"wedding_is_playing_music", marriage_wedding_is_playing_music},
        {"wedding_client_command", marriage_wedding_client_command},
        {"in_my_wedding", marriage_in_my_wedding},
        {"warp_to_my_marriage_map", marriage_warp_to_my_marriage_map},
        {"get_married_time", marriage_get_married_time},
        {nullptr, nullptr}
    };
    CQuestManager::instance().AddLuaFunctionTable("marriage", marriage_functions);
}
}
