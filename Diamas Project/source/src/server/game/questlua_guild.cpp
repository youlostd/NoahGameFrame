#include "DbCacheSocket.hpp"
#include "MeleyLair.h"
#include "char.h"
#include "char_manager.h"
#include "guild.h"
#include "guild_manager.h"
#include "questlua.h"
#include "questmanager.h"
#include "utils.h"

namespace quest
{
//
// "guild" Lua functions
//
int guild_around_ranking_string(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CHARACTER* ch = q.GetCurrentCharacterPtr();
    if (!ch->GetGuild())
        lua_pushstring(L, "");
    else {
        lua_pushstring(L, CGuildManager::instance()
                              .GetAroundRankString(ch->GetGuild()->GetID())
                              .c_str());
    }
    return 1;
}

int guild_high_ranking_string(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CHARACTER* ch = q.GetCurrentCharacterPtr();
    uint32_t dwMyGuild = 0;
    if (ch->GetGuild())
        dwMyGuild = ch->GetGuild()->GetID();

    lua_pushstring(
        L, CGuildManager::instance().GetHighRankString(dwMyGuild).c_str());
    return 1;
}

int guild_get_ladder_point(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CHARACTER* ch = q.GetCurrentCharacterPtr();
    if (!ch->GetGuild()) {
        lua_pushnumber(L, -1);
    } else {
        lua_pushnumber(L, ch->GetGuild()->GetLadderPoint());
    }
    return 1;
}

int guild_get_rank(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CHARACTER* ch = q.GetCurrentCharacterPtr();

    if (!ch->GetGuild()) {
        lua_pushnumber(L, -1);
    } else {
        lua_pushnumber(L, CGuildManager::instance().GetRank(ch->GetGuild()));
    }
    return 1;
}

int guild_is_war(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch->GetGuild() &&
        ch->GetGuild()->UnderWar((uint32_t)lua_tonumber(L, 1)))
        lua_pushboolean(L, true);
    else
        lua_pushboolean(L, false);

    return 1;
}

int guild_name(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CGuild* pkGuild =
        CGuildManager::instance().FindGuild((uint32_t)lua_tonumber(L, 1));

    if (pkGuild)
        lua_pushstring(L, pkGuild->GetName());
    else
        lua_pushstring(L, "");

    return 1;
}

int guild_level(lua_State* L)
{
    luaL_checknumber(L, 1);

    CGuild* pkGuild =
        CGuildManager::instance().FindGuild((uint32_t)lua_tonumber(L, 1));

    if (pkGuild)
        lua_pushnumber(L, pkGuild->GetLevel());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int guild_war_enter(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    CQuestManager& q = CQuestManager::instance();
    CHARACTER* ch = q.GetCurrentCharacterPtr();

    #ifdef ENABLE_NEW_GUILD_WAR
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "minseviye", ch->GetGuild()->GetName()) > ch->GetLevel()) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Maalesef savasa girmek icin leveliniz "
                                       "yetersiz !");
        return 0;
    }

    if (quest::CQuestManager::instance().GetEventFlagBR(
            "suankiplayer", ch->GetGuild()->GetName()) >=
        quest::CQuestManager::instance().GetEventFlagBR(
            "maxplayer", ch->GetGuild()->GetName())) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Maalesef savastaki oyuncu sayisi full "
                                       "durumda !");
        return 0;
    }

    if (quest::CQuestManager::instance().GetEventFlagBR(
            "savasci", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 0) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Bu savasa savasci karakterler "
                                       "katilamaz.");
        return 0;
    }
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "ninja", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Bu savasa ninja karakterler "
                                       "katilamaz.");
        return 0;
    }
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "sura", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 2) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Bu savasa sura karakterler katilamaz.");
        return 0;
    }
    if (quest::CQuestManager::instance().GetEventFlagBR(
            "shaman", ch->GetGuild()->GetName()) > 0 &&
        ch->GetJob() == 3) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Bu savasa shaman karakterler "
                                       "katilamaz.");
        return 0;
    }
#endif

    if (ch->GetGuild())
        ch->GetGuild()->GuildWarEntryAccept((uint32_t)lua_tonumber(L, 1), ch);

    return 0;
}

int guild_get_any_war(lua_State* L)
{
    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch && ch->GetGuild())
        lua_pushnumber(L, ch->GetGuild()->UnderAnyWar());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int guild_get_name(lua_State* L)
{
    if (!lua_isnumber(L, 1)) {
        lua_pushstring(L, "");
        return 1;
    }

    CGuild* pkGuild =
        CGuildManager::instance().FindGuild((uint32_t)lua_tonumber(L, 1));

    if (pkGuild)
        lua_pushstring(L, pkGuild->GetName());
    else
        lua_pushstring(L, "");

    return 1;
}

int guild_get_warp_war_list(lua_State* L)
{
    FBuildLuaGuildWarList f(L);
    CGuildManager::instance().for_each_war(f);
    return 1;
}

int guild_get_member_count(lua_State* L)
{
    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch == nullptr) {
        lua_pushnumber(L, 0);
        return 1;
    }

    CGuild* pGuild = ch->GetGuild();

    if (pGuild == nullptr) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, pGuild->GetMemberCount());

    return 1;
}

int guild_change_master(lua_State* L)
{
    // 리턴값
    //	0 : 입력한 이름이 잘못되었음 ( 문자열이 아님 )
    //	1 : 길드장이 아님
    //	2 : 지정한 이름의 길드원이 없음
    //	3 : 요청 성공
    //	4 : 길드가 없음

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    CGuild* pGuild = ch->GetGuild();

    if (pGuild != nullptr) {
        if (pGuild->GetMasterPID() == ch->GetPlayerID()) {
            if (lua_isstring(L, 1) == false) {
                lua_pushnumber(L, 0);
            } else {
                bool ret = pGuild->ChangeMasterTo(
                    pGuild->GetMemberPID(lua_tostring(L, 1)));

                lua_pushnumber(L, ret == false ? 2 : 3);
            }
        } else {
            lua_pushnumber(L, 1);
        }
    } else {
        lua_pushnumber(L, 4);
    }

    return 1;
}

int guild_change_master_with_limit(lua_State* L)
{
    // 인자
    //  arg0 : 새 길드장 이름
    //  arg1 : 새 길드장 레벨 제한
    //  arg2 : resign_limit 제한 시간
    //  arg3 : be_other_leader 제한 시간
    //  arg4 : be_other_member 제한 시간
    //  arg5 : 캐시템인가 아닌가
    //
    // 리턴값
    //	0 : 입력한 이름이 잘못되었음 ( 문자열이 아님 )
    //	1 : 길드장이 아님
    //	2 : 지정한 이름의 길드원이 없음
    //	3 : 요청 성공
    //	4 : 길드가 없음
    //	5 : 지정한 이름이 온라인이 아님
    //	6 : 지정한 캐릭터 레벨이 기준레벨보다 낮음
    //	7 : 새 길드장이 be_other_leader 제한에 걸림

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    CGuild* pGuild = ch->GetGuild();

    if (pGuild != nullptr) {
        if (pGuild->GetMasterPID() == ch->GetPlayerID()) {
            if (lua_isstring(L, 1) == false) {
                lua_pushnumber(L, 0);
            } else {
                CHARACTER* pNewMaster =
                    g_pCharManager->FindPC(lua_tostring(L, 1));

                if (pNewMaster != nullptr) {
                    if (pNewMaster->GetLevel() < lua_tonumber(L, 2)) {
                        lua_pushnumber(L, 6);
                    } else {
                        int nBeOtherLeader =
                            pNewMaster->GetQuestFlag("change_guild_master.be_"
                                                     "other_leader");
                        CQuestManager::instance().GetPC(ch->GetPlayerID());

                        if (lua_toboolean(L, 6))
                            nBeOtherLeader = 0;

                        if (nBeOtherLeader > get_global_time()) {
                            lua_pushnumber(L, 7);
                        } else {
                            bool ret = pGuild->ChangeMasterTo(
                                pGuild->GetMemberPID(lua_tostring(L, 1)));

                            if (ret == false) {
                                lua_pushnumber(L, 2);
                            } else {
                                lua_pushnumber(L, 3);

                                pNewMaster->SetQuestFlag("change_guild_master."
                                                         "be_other_leader",
                                                         0);
                                pNewMaster->SetQuestFlag("change_guild_master."
                                                         "be_other_member",
                                                         0);
                                pNewMaster->SetQuestFlag(
                                    "change_guild_master.resign_limit",
                                    (int)lua_tonumber(L, 3));

                                ch->SetQuestFlag("change_guild_master.be_other_"
                                                 "leader",
                                                 (int)lua_tonumber(L, 4));
                                ch->SetQuestFlag("change_guild_master.be_other_"
                                                 "member",
                                                 (int)lua_tonumber(L, 5));
                                ch->SetQuestFlag("change_guild_master.resign_"
                                                 "limit",
                                                 0);
                            }
                        }
                    }
                } else {
                    lua_pushnumber(L, 5);
                }
            }
        } else {
            lua_pushnumber(L, 1);
        }
    } else {
        lua_pushnumber(L, 4);
    }

    return 1;
}

int guild_can_use_storage(lua_State* L)
{
    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();

    CGuild* pGuild = ch->GetGuild();

    if (!pGuild) {
        lua_pushnumber(L, 0);
        return 1;
    }

    const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

    if (!m) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, pGuild->HasGradeAuth(m->grade, GUILD_AUTH_USE_GS));

    return 1;
}

int guild_get_master_pid(lua_State* L)
{
    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    CGuild* pGuild = ch->GetGuild();
    if (!pGuild) {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, pGuild->GetMasterPID());
    return 1;
}

void RegisterGuildFunctionTable()
{
    luaL_reg guild_functions[] = {
        {"get_rank", guild_get_rank},
        {"get_ladder_point", guild_get_ladder_point},
        {"high_ranking_string", guild_high_ranking_string},
        {"around_ranking_string", guild_around_ranking_string},
        {"name", guild_name},
        {"level", guild_level},
        {"is_war", guild_is_war},
        {"war_enter", guild_war_enter},
        {"get_any_war", guild_get_any_war},
        {"get_name", guild_get_name},
        {"get_warp_war_list", guild_get_warp_war_list},
        {"get_member_count", guild_get_member_count},
        {"change_master", guild_change_master},
        {"change_master_with_limit", guild_change_master_with_limit},
        {"can_use_storage", guild_can_use_storage},
        {"get_master_pid", guild_get_master_pid},

        {nullptr, nullptr}};

    CQuestManager::instance().AddLuaFunctionTable("guild", guild_functions);
}
} // namespace quest
