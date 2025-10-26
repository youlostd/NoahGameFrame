#include <sstream>

#include "desc.h"
#include "party.h"
#include "char.h"
#include "questlua.h"
#include "questmanager.h"
#include <game/GamePacket.hpp>
#include "char_manager.h"

namespace quest
{
using namespace std;

//
// "party" Lua functions
//
int party_clear_ready(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty())
    {
        FPartyClearReady f;
        ch->GetParty()->ForEachNearMember(f);
    }
    return 0;
}

struct FRunCinematicSender
{
    std::string data;
    struct packet_script pack;

    FRunCinematicSender(const char *str)
    {
        data = "[RUN_CINEMA value;";
        data += str;
        data += "]";

        pack.skin = CQuestManager::QUEST_SKIN_CINEMATIC;
        pack.script = data;
    }

    void operator()(CHARACTER *ch)
    {
        SPDLOG_TRACE("CINEMASEND_TRY %s", ch->GetName());

        if (ch->GetDesc())
        {
            SPDLOG_TRACE("CINEMASEND %s", ch->GetName());
            ch->GetDesc()->Send(HEADER_GC_SCRIPT, pack);
        }
    }
};

int party_run_cinematic(lua_State *L)
{
    if (!lua_isstring(L, 1))
        return 0;

    SPDLOG_TRACE("RUN_CINEMA %s", lua_tostring(L, 1));
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty())
    {
        FRunCinematicSender f(lua_tostring(L, 1));

        ch->GetParty()->Update();
        ch->GetParty()->ForEachNearMember(f);
    }

    return 0;
}

struct FCinematicSender
{
    struct ::packet_script packet_script;

    FCinematicSender(const char *str)
    {
        packet_script.skin = CQuestManager::QUEST_SKIN_CINEMATIC;
        packet_script.script = str;
    }

    void operator()(CHARACTER *ch)
    {

        if (ch->GetDesc())
        {
            ch->GetDesc()->Send(HEADER_GC_SCRIPT, packet_script);
        }
    }
};

int party_show_cinematic(lua_State *L)
{
    if (!lua_isstring(L, 1))
        return 0;

    SPDLOG_TRACE("CINEMA %s", lua_tostring(L, 1));
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty())
    {
        FCinematicSender f(lua_tostring(L, 1));

        ch->GetParty()->Update();
        ch->GetParty()->ForEachNearMember(f);
    }
    return 0;
}

int party_get_near_count(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty())
        lua_pushnumber(L, ch->GetParty()->GetNearMemberCount());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int party_syschat(lua_State *L)
{
    auto pParty = CQuestManager::Instance().GetCurrentCharacterPtr()->GetParty();

    if (pParty)
    {
        ostringstream s;
        combine_lua_string(L, s);

        FPartyChat f(CHAT_TYPE_INFO, s.str().c_str());

        pParty->ForEachOnlineMember(f);
    }

    return 0;
}

int party_is_leader(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

int party_is_party(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (!ch)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, ch->GetParty() ? 1 : 0);
    return 1;
}

int party_get_leader_pid(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    if (ch && ch->GetParty()) { lua_pushnumber(L, ch->GetParty()->GetLeaderPID()); }
    else { lua_pushnumber(L, -1); }
    return 1;
}

int party_chat(lua_State *L)
{
    auto pParty = CQuestManager::Instance().GetCurrentCharacterPtr()->GetParty();

    if (pParty)
    {
        ostringstream s;
        combine_lua_string(L, s);

        FPartyChat f(CHAT_TYPE_TALKING, s.str().c_str());

        pParty->ForEachOnlineMember(f);
    }

    return 0;
}

int party_is_map_member_flag_lt(lua_State *L)
{
    if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    CQuestManager &q = CQuestManager::Instance();
    auto pParty = q.GetCurrentCharacterPtr()->GetParty();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    PC *pPC = q.GetCurrentPC();

    const char *sz = lua_tostring(L, 1);

    if (pParty)
    {
        FPartyCheckFlagLt f;
        f.flagname = pPC->GetCurrentQuestName() + "." + sz;
        f.value = (int)rint(lua_tonumber(L, 2));

        bool returnBool = pParty->ForEachOnMapMemberBool(f, ch->GetMapIndex());
        lua_pushboolean(L, returnBool);
    }

    return 1;
}

int party_set_flag(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty() && lua_isstring(L, 1) && lua_isnumber(L, 2))
        ch->GetParty()->SetFlag(lua_tostring(L, 1), (int)lua_tonumber(L, 2));

    return 0;
}

int party_get_flag(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (!ch->GetParty() || !lua_isstring(L, 1))
        lua_pushnumber(L, 0);
    else
        lua_pushnumber(L, ch->GetParty()->GetFlag(lua_tostring(L, 1)));

    return 1;
}

int party_set_quest_flag(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();

    FSetQuestFlag f;

    f.flagname = q.GetCurrentPC()->GetCurrentQuestName() + "." + lua_tostring(L, 1);
    f.value = (int)rint(lua_tonumber(L, 2));

    CHARACTER *ch = q.GetCurrentCharacterPtr();

    if (ch->GetParty())
        ch->GetParty()->ForEachOnlineMember(f);
    else
        f(ch);

    return 0;
}

struct FGiveBuff
{
    uint32_t type;
    uint8_t pointType;
    int32_t pointValue;
    int32_t duration;
    int32_t spCost;
    bool override;
    bool isCube;

    FGiveBuff(uint32_t type, uint8_t pointType, int32_t pointValue,
              int32_t duration, int32_t spCost, bool override,
              bool isCube = false)
        : type(type)
          , pointType(pointType)
          , pointValue(pointValue)
          , duration(duration)
          , spCost(spCost)
          , override(override)
          , isCube(isCube)
    {
        // ctor
    }

    void operator()(CHARACTER *ch) { ch->AddAffect(type, pointType, pointValue, duration, spCost, override, isCube); }
};

// 파티 단위로 버프 주는 함수.
// 같은 맵에 있는 파티원만 영향을 받는다.
int party_give_buff(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) ||
        !lua_isnumber(L, 5) || !lua_isnumber(L, 6) || !lua_isboolean(L, 7) || !lua_isboolean(L, 8))
    {
        lua_pushboolean(L, false);
        return 1;
    }

    uint32_t type = lua_tonumber(L, 1);
    uint8_t pointType = lua_tonumber(L, 2);
    int32_t pointValue = lua_tonumber(L, 3);
    uint32_t flag = lua_tonumber(L, 4);
    int32_t duration = lua_tonumber(L, 5);
    int32_t spCost = lua_tonumber(L, 6);
    bool bOverride = lua_toboolean(L, 7);
    bool IsCube = lua_toboolean(L, 8);

    FGiveBuff f(type, pointType, pointValue, duration, spCost, bOverride, IsCube);
    if (ch->GetParty())
        ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());
    else
        f(ch);

    lua_pushboolean(L, true);
    return 1;
}

struct FPartyPIDCollector
{
    std::vector<uint32_t> vecPIDs;

    FPartyPIDCollector()
    {
    }

    void operator ()(CHARACTER *ch) { vecPIDs.push_back(ch->GetPlayerID()); }
};

int party_get_member_pids(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    auto pParty = ch->GetParty();
    if (nullptr == pParty)
    {
        lua_pushnumber(L, ch->GetPlayerID());
        return 1;
    }
    FPartyPIDCollector f;
    pParty->ForEachOnMapMember(f, ch->GetMapIndex());

    for (std::vector<uint32_t>::iterator it = f.vecPIDs.begin(); it != f.vecPIDs.end(); ++it)
    {
        lua_pushnumber(L, *it);
    }
    return f.vecPIDs.size();
}

struct FPartyVIDCollector
{
    std::vector<uint32_t> vecVIDs;

    FPartyVIDCollector()
    {
    }

    void operator ()(CHARACTER *ch) { vecVIDs.push_back(ch->GetVID()); }
};

int party_get_member_vids(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    auto pParty = ch->GetParty();
    if (nullptr == pParty) { return 0; }

    FPartyVIDCollector f;
    pParty->ForEachOnMapMember(f, ch->GetMapIndex());

    for (std::vector<uint32_t>::iterator it = f.vecVIDs.begin(); it != f.vecVIDs.end(); ++it)
    {
        lua_pushnumber(L, *it);
    }
    return f.vecVIDs.size();
}

/*
* Removes a player from the group also deletes the group if members = 2 or user is leader.
* @Author MartPwnS 11.05.2014 17:50
*/
int party_remove_player(lua_State *L)
{
    CHARACTER *ch;
    CQuestManager &q = CQuestManager::instance();
    if (lua_isnumber(L, 1))
    {
        uint32_t pid = (uint32_t)lua_tonumber(L, 1);
        ch = g_pCharManager->FindByPID(pid);
    }
    else
        ch = q.GetCurrentCharacterPtr();

    if (!ch)
        return 0; // Supplied pid doesn? exist or GetCurrentCharacterPTR failed (unlikely)

    auto pParty = ch->GetParty();
    if (!pParty) // Player has no party
        return 0;

    if (pParty->GetMemberCount() == 2 || pParty->GetLeaderPID() == ch->GetPlayerID())
        CPartyManager::instance().DeleteParty(pParty);
    else
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<파티> 파티에서 추방당하셨습니다.");
        pParty->Quit(ch->GetPlayerID());
    }

    return 1;
}

int party_get_max_level(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty())
        lua_pushnumber(L, ch->GetParty()->GetMemberMaxLevel());
    else
        lua_pushnumber(L, 1);

    return 1;
}

int party_get_min_level(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetParty())
        lua_pushnumber(L, ch->GetParty()->GetMemberMinLevel());
    else
        lua_pushnumber(L, 1);

    return 1;
}

void RegisterPartyFunctionTable()
{
    luaL_reg party_functions[] =
    {
        {"is_leader", party_is_leader},
        {"is_party", party_is_party},
        {"get_leader_pid", party_get_leader_pid},
        {"setf", party_set_flag},
        {"getf", party_get_flag},
        {"setqf", party_set_quest_flag},
        {"chat", party_chat},
        {"syschat", party_syschat},
        {"get_near_count", party_get_near_count},
        {"show_cinematic", party_show_cinematic},
        {"run_cinematic", party_run_cinematic},
        {"get_max_level", party_get_max_level},
        {"get_min_level", party_get_min_level},
        {"clear_ready", party_clear_ready},
        {"give_buff", party_give_buff},
        {"is_map_member_flag_lt", party_is_map_member_flag_lt},
        {"get_member_pids", party_get_member_pids}, // 파티원들의 pid를 return
        {"get_member_vids", party_get_member_vids},
        {"remove_player", party_remove_player},
        {nullptr, nullptr}
    };

    CQuestManager::instance().AddLuaFunctionTable("party", party_functions);
}
}
