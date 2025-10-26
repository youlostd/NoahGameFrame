#include <sstream>

#include "questmanager.h"
#include "questlua.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "GBufferManager.h"
#include "db.h"

#include "regen.h"
#include <game/AffectConstants.hpp>
#include "guild.h"
#include "guild_manager.h"
#include "sectree_manager.h"
#include "item.h"
#include <boost/math/constants/constants.hpp>
#include "DbCacheSocket.hpp"
#include "locale.hpp"

#ifdef WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

namespace quest
{
using namespace std;

bool RunString(lua_State *L, const std::string &str, const char *name)
{
    int status = luaL_loadbuffer(L, str.data(), str.size(), name);
    if (0 != status)
    {
        SPDLOG_ERROR("%s | status %d", lua_isstring(L, -1) ? lua_tostring(L, -1) : "<no message>", status);
        lua_pop(L, 1);
        return false;
    }

    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (0 != status)
    {
        SPDLOG_ERROR("Failed executing {} with {}:{}", name, status,
                     lua_isstring(L, -1) ? lua_tostring(L, -1) : "<no message>");
        lua_pop(L, 1);
        return false;
    }

    return true;
}

bool RunFile(lua_State *L, const char *filename)
{
    int status = luaL_loadfile(L, filename);
    if (0 != status)
    {
        SPDLOG_ERROR("{0} | status {1}", lua_isstring(L, -1) ? lua_tostring(L, -1) : "<no message>", status);
        lua_pop(L, 1);
        return false;
    }

    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (0 != status)
    {
        SPDLOG_ERROR("Failed executing {0} with {1}:{2}", filename, status,
                     lua_isstring(L, -1) ? lua_tostring(L, -1) : "<no message>");
        lua_pop(L, 1);
        return false;
    }

    return true;
}

std::string ScriptToString(const std::string &locale, const std::string &str)
{
    lua_State *L = CQuestManager::instance().GetLuaState();

    SetGameforgeLocale(L, locale);

    int x = lua_gettop(L);

    std::string code = "return " + str;

    std::string retstr;
    if (RunString(L, code, "ScriptToString") && lua_isstring(L, -1))
        retstr = lua_tostring(L, -1);

    lua_settop(L, x);
    return retstr;
}

void FSetWarpLocation::operator()(CHARACTER *ch)
{
    if (ch->IsPC())
    {
        ch->SetWarpLocation(map_index, x, y);
    }
}

void FSetQuestFlag::operator()(CHARACTER *ch)
{
    if (!ch->IsPC())
        return;

    PC *pPC = CQuestManager::instance().GetPCForce(ch->GetPlayerID());

    if (pPC)
        pPC->SetFlag(flagname, value);
}

bool FPartyCheckFlagLt::operator()(CHARACTER *ch)
{
    if (!ch->IsPC())
        return false;

    PC *pPC = CQuestManager::instance().GetPCForce(ch->GetPlayerID());
    bool returnBool = false;
    if (pPC)
    {
        int flagValue = pPC->GetFlag(flagname);
        if (value > flagValue)
            returnBool = true;
        else
            returnBool = false;
    }

    return returnBool;
}

FPartyChat::FPartyChat(int ChatType, const char *str) : iChatType(ChatType), str(str) {}

void FPartyChat::operator()(CHARACTER *ch) { ch->ChatPacket(iChatType, "%s", str); }

void FPartyClearReady::operator()(CHARACTER *ch) { ch->RemoveAffect(AFFECT_DUNGEON_READY); }

void FSendScriptToEmpire::operator()(CEntity *ent)
{
    if (ent->IsType(ENTITY_CHARACTER))
    {
        CHARACTER *ch = (CHARACTER *)ent;

        if (ch->GetDesc())
        {
            if (ch->GetEmpire() == bEmpire)
                ch->GetDesc()->Send(HEADER_GC_SCRIPT, p);
        }
    }
}

void FPurgeArea::operator()(CEntity *ent)
{
    if (true == ent->IsType(ENTITY_CHARACTER))
    {
        CHARACTER *pChar = static_cast<CHARACTER *>(ent);

        if (pChar == ExceptChar)
            return;

        if (!pChar->IsPet() && (true == pChar->IsMonster() || true == pChar->IsStone()))
        {
            if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
            {
                M2_DESTROY_CHARACTER(pChar);
            }
        }
    }
}

void FWarpEmpire::operator()(CEntity *ent)
{
    if (ent->IsType(ENTITY_CHARACTER))
    {
        CHARACTER *ch = (CHARACTER *)ent;

        if (ch->IsPC() && ch->GetEmpire() == m_bEmpire)
        {
            ch->WarpSet(m_lMapIndexTo, m_x, m_y);
        }
    }
}

FBuildLuaGuildWarList::FBuildLuaGuildWarList(lua_State *lua_state) : L(lua_state), m_count(1)
{
    lua_newtable(lua_state);
}

void FBuildLuaGuildWarList::operator()(uint32_t g1, uint32_t g2)
{
    CGuild *g = CGuildManager::instance().FindGuild(g1);

    if (!g)
        return;

    if (g->GetGuildWarType(g2) == GUILD_WAR_TYPE_FIELD)
        return;

    if (g->GetGuildWarState(g2) != GUILD_WAR_ON_WAR)
        return;

    lua_newtable(L);
    lua_pushnumber(L, g1);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, g2);
    lua_rawseti(L, -2, 2);
    lua_rawseti(L, -2, m_count++);
}

bool IsScriptTrue(const char *code, int size)
{
    if (size == 0)
        return true;

    lua_State *L = CQuestManager::instance().GetLuaState();
    int x = lua_gettop(L);
    int errcode = lua_dobuffer(L, code, size, "IsScriptTrue");
    int bStart = lua_toboolean(L, -1);
    if (errcode)
    {
        char buf[100];
        std::snprintf(buf, sizeof(buf), "LUA ScriptRunError (code:%%d src:[%%%ds])", size);
        SPDLOG_ERROR(buf, errcode, code);
    }
    lua_settop(L, x);
    return bStart != 0;
}

void create_notice_format_string(lua_State *L, ostringstream &s)
{
    int n = lua_gettop(L);

    s << lua_tostring(L, 1);
    if (n != 1)
        s << "#";

    for (int i = 2; i <= n; ++i)
    {
        if (lua_isstring(L, i))
        {
            s << lua_tostring(L, i);
            if (i != n)
                s << ";";
        }
        else if (lua_isnumber(L, i))
        {
            s << fmt::format("{}", lua_tonumber(L, i));
            if (i != n)
                s << ";";
        }
    }
}

void combine_lua_string(lua_State *L, ostringstream &s)
{
    char buf[32];

    int n = lua_gettop(L);
    int i;

    for (i = 1; i <= n; ++i)
    {
        if (lua_isstring(L, i))
            // printf("%s\n",lua_tostring(L,i));
            s << lua_tostring(L, i);
        else if (lua_isnumber(L, i))
        {
            std::snprintf(buf, sizeof(buf), "%.14g\n", lua_tonumber(L, i));
            s << buf;
        }
    }
}

bool SetGameforgeLocale(lua_State *L, const std::string &locale)
{
    lua_getglobal(L, "__locale");
    lua_pushstring(L, locale.c_str());
    lua_rawget(L, -2);

    if (lua_isnil(L, -1))
    {
        SPDLOG_ERROR("Locale {} has no translation", locale.c_str());
        return false;
    }

    lua_setglobal(L, "gameforge");
    lua_pop(L, 1);
    return true;
}

int highscore_show(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    const char *pszBoardName = lua_tostring(L, 1);
    uint32_t mypid = q.GetCurrentCharacterPtr()->GetPlayerID();
    bool bOrder = (int)lua_tonumber(L, 2) != 0 ? true : false;

    DBManager::instance().ReturnQuery(QID_HIGHSCORE_SHOW, mypid, nullptr,
                                      "SELECT h.pid, p.name, h.value FROM highscore as h, player as p WHERE h.board = "
                                      "'{}' AND h.pid = p.id ORDER BY h.value {} LIMIT 10",
                                      pszBoardName, bOrder ? "DESC" : "");
    return 0;
}

int highscore_register(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();

    TPacketGDHighscore p;
    p.dwPID = q.GetCurrentCharacterPtr()->GetPlayerID();
    p.lValue = (int32_t)lua_tonumber(L, 2);
    strlcpy(p.szBoard, lua_tostring(L, 1), sizeof(p.szBoard));
    char cDir;
    p.cDir = (bool)lua_tonumber(L, 3);

    db_clientdesc->DBPacket(HEADER_GD_HIGHSCORE_REGISTER, 0, &p, sizeof(TPacketGDHighscore));

    return 0;
}

//
// "member" Lua functions
//
int member_chat(lua_State *L)
{
    ostringstream s;
    combine_lua_string(L, s);
    CQuestManager::Instance().GetCurrentPartyMember()->ChatPacket(CHAT_TYPE_TALKING, "%s", s.str().c_str());
    return 0;
}

int member_clear_ready(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentPartyMember();
    ch->RemoveAffect(AFFECT_DUNGEON_READY);
    return 0;
}

int member_set_ready(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentPartyMember();
    ch->AddAffect(AFFECT_DUNGEON_READY, POINT_NONE, 0, 65535, 0, true);
    return 0;
}

int mob_spawn(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
    {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    uint32_t mob_vnum = (uint32_t)lua_tonumber(L, 1);
    long local_x = (long)lua_tonumber(L, 2) * 100;
    long local_y = (long)lua_tonumber(L, 3) * 100;
    float radius = (float)lua_tonumber(L, 4) * 100;
    bool bAggressive = lua_toboolean(L, 5);
    uint32_t count = (lua_isnumber(L, 6)) ? (uint32_t)lua_tonumber(L, 6) : 1;
    bool noReward = lua_toboolean(L, 7);
    const char *mobName = lua_tostring(L, 8);

    if (count == 0)
        count = 1;
    else if (count > 10)
    {
        SPDLOG_ERROR("count bigger than 10");
        count = 10;
    }

    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    uint32_t dwQuestIdx = CQuestManager::instance().GetCurrentPC()->GetCurrentQuestIndex();

    bool ret = false;
    CHARACTER *mob = nullptr;

    while (count--)
    {
        for (int loop = 0; loop < 8; ++loop)
        {
            double angle = Random::get(0, 999) * boost::math::constants::pi<double>() * 2 / 1000;
            double r = Random::get(0, 999) * radius / 1000;

            int32_t x = local_x + (int32_t)(r * cos(angle));
            int32_t y = local_y + (int32_t)(r * sin(angle));

            mob = g_pCharManager->SpawnMob(mob_vnum, ch->GetMapIndex(), x, y, 0);

            if (mob)
                break;
        }

        if (mob)
        {
            if (bAggressive)
                mob->SetAggressive();

            if (noReward)
                mob->SetNoRewardFlag();

            mob->SetQuestBy(dwQuestIdx);

            if (mobName)
            {
                mob->SetName(mobName);
            }

            if (!ret)
            {
                ret = true;
                lua_pushnumber(L, (uint32_t)mob->GetVID());
            }
        }
    }

    if (!ret)
        lua_pushnumber(L, 0);

    return 1;
}

int mob_spawn_group(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 6))
    {
        SPDLOG_ERROR("invalid argument");
        lua_pushnumber(L, 0);
        return 1;
    }

    uint32_t group_vnum = (uint32_t)lua_tonumber(L, 1);
    long local_x = (long)lua_tonumber(L, 2) * 100;
    long local_y = (long)lua_tonumber(L, 3) * 100;
    double radius = (double)lua_tonumber(L, 4) * 100;
    bool bAggressive = lua_toboolean(L, 5);
    uint32_t count = (uint32_t)lua_tonumber(L, 6);

    if (count == 0)
        count = 1;
    else if (count > 10)
    {
        SPDLOG_ERROR("count bigger than 10");
        count = 10;
    }

    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    uint32_t dwQuestIdx = CQuestManager::instance().GetCurrentPC()->GetCurrentQuestIndex();

    bool ret = false;
    CHARACTER *mob = nullptr;

    while (count--)
    {
        for (int loop = 0; loop < 8; ++loop)
        {
            double angle = Random::get(0, 999) * boost::math::constants::pi<double>() * 2 / 1000;
            double r = Random::get(0, 999) * radius / 1000;

            int32_t x = local_x + (int32_t)(r * cos(angle));
            int32_t y = local_y + (int32_t)(r * sin(angle));

            mob = g_pCharManager->SpawnGroup(group_vnum, ch->GetMapIndex(), x, y, x, y, nullptr, bAggressive);

            if (mob)
                break;
        }

        if (mob)
        {
            mob->SetQuestBy(dwQuestIdx);

            if (!ret)
            {
                ret = true;
                lua_pushnumber(L, (uint32_t)mob->GetVID());
            }
        }
    }

    if (!ret)
        lua_pushnumber(L, 0);

    return 1;
}

int mob_spawn_for_me(lua_State *L)
{
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
    {
        SPDLOG_ERROR("invalid argument");
        return 0;
    }

    uint32_t mob_vnum = (uint32_t)lua_tonumber(L, 1);
    long local_x = (long)lua_tonumber(L, 2) * 100;
    long local_y = (long)lua_tonumber(L, 3) * 100;
    float radius = (float)lua_tonumber(L, 4) * 100;
    bool bAggressive = lua_toboolean(L, 5);
    uint32_t count = (lua_isnumber(L, 6)) ? (uint32_t)lua_tonumber(L, 6) : 1;
    const char *mobName = lua_tostring(L, 7);

    if (count == 0)
        count = 1;
    else if (count > 10)
    {
        SPDLOG_ERROR("Can't spawn more than 10 mobs at a time.");
        count = 10;
    }

    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
    SECTREE_MAP *pMap = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());
    if (pMap == nullptr)
    {
        return 0;
    }
    uint32_t dwQuestIdx = CQuestManager::instance().GetCurrentPC()->GetCurrentQuestIndex();

    bool ret = false;
    CHARACTER *mob = nullptr;

    while (count--)
    {
        for (int loop = 0; loop < 8; ++loop)
        {
            double angle = Random::get(0, 999) * boost::math::constants::pi<double>() * 2 / 1000;
            double r = Random::get(0, 999) * radius / 1000;

            long x = local_x + (long)(r * cos(angle));
            long y = local_y + (long)(r * sin(angle));

            // Spawns a monster without displaying it
            mob = g_pCharManager->SpawnMob(mob_vnum, ch->GetMapIndex(), x, y, 0, false, -1, false);

            if (mob)
            {
                mob->SetXYZ(x, y, 0);
                break;
            }
        }

        if (mob)
        {
            if (bAggressive)
                mob->SetAggressive();

            mob->SetQuestBy(dwQuestIdx);

            if (mobName)
            {
                mob->SetName(mobName);
            }

            uint32_t pid = ch->GetPlayerID();
            mob->SetRequirementFunction([pid](CHARACTER *player) -> bool {
                if (!pid && !player)
                    return false;

                return pid == player->GetPlayerID();
            });

            if (!mob->Show(ch->GetMapIndex(), mob->GetX(), mob->GetY(), mob->GetZ(), false))
            {
                M2_DESTROY_CHARACTER(mob);
                SPDLOG_INFO("Reindeer: cannot show monster");

                lua_pushnumber(L, 0);
                return 1;
            }

            if (!ret)
            {
                ret = true;
                lua_pushnumber(L, (uint32_t)mob->GetVID());
            }
        }
    }

    if (!ret)
        lua_pushnumber(L, 0);

    return 1;
}

int mob_set_no_reward(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        SPDLOG_ERROR("Invalid argument, expected integer");
        lua_pushboolean(L, false);
        return 1;
    }

    uint32_t v = (uint32_t)lua_tonumber(L, 1);
    if (!v)
    {
        SPDLOG_ERROR("Invalid vid, expected vid != 0");
        lua_pushboolean(L, false);
        return 1;
    }

    CHARACTER *mob = g_pCharManager->Find(v);

    if (!mob)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    mob->SetNoRewardFlag();
    lua_pushboolean(L, true);
    return 1;
}

int mob_set_no_move(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        SPDLOG_ERROR("Invalid argument, expected integer");
        lua_pushboolean(L, false);
        return 1;
    }

    uint32_t v = (uint32_t)lua_tonumber(L, 1);
    if (!v)
    {
        SPDLOG_ERROR("Invalid vid, expected vid != 0");
        lua_pushboolean(L, false);
        return 1;
    }

    CHARACTER *mob = g_pCharManager->Find(v);

    if (!mob)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    mob->SetNoMoveFlag();
    lua_pushboolean(L, true);
    return 1;
}

int mob_remove(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        SPDLOG_ERROR("Invalid argument, expected integer");
        lua_pushboolean(L, false);
        return 1;
    }

    uint32_t v = (uint32_t)lua_tonumber(L, 1);
    if (!v)
    {
        SPDLOG_ERROR("Invalid vid, expected vid != 0");
        lua_pushboolean(L, false);
        return 1;
    }

    CHARACTER *mob = g_pCharManager->Find(v);

    if (!mob)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    mob->Dead(nullptr, true);
    lua_pushboolean(L, true);
    return 1;
}

//
// global Lua functions
//
//
// Registers Lua function table
//
void CQuestManager::AddLuaFunctionTable(const char *c_pszName, luaL_reg *preg)
{
    lua_newtable(L);

    while ((preg->name))
    {
        lua_pushstring(L, preg->name);
        lua_pushcfunction(L, preg->func);
        lua_rawset(L, -3);
        preg++;
    }

    lua_setglobal(L, c_pszName);
}

void CQuestManager::BuildStateIndexToName(const char *questName)
{
    int x = lua_gettop(L);
    lua_getglobal(L, questName);

    if (lua_isnil(L, -1))
    {
        SPDLOG_ERROR("QUEST wrong quest state file for quest %s", questName);
        lua_settop(L, x);
        return;
    }

    for (lua_pushnil(L); lua_next(L, -2);)
    {
        if (lua_isstring(L, -2) && lua_isnumber(L, -1))
        {
            lua_pushvalue(L, -2);
            lua_rawset(L, -4);
        }
        else
        {
            lua_pop(L, 1);
        }
    }

    lua_settop(L, x);
}

/**
 * @version 05/06/08	Bang2ni - __get_guildid_byname 스크립트 함수 등록
 */
bool CQuestManager::InitializeLua()
{
    L = lua_open();

    luaopen_base(L);
    luaopen_table(L);
    luaopen_string(L);
    luaopen_math(L);
    // TEMP
    luaopen_io(L);
    luaopen_debug(L);

    RegisterAffectFunctionTable();
    RegisterBuildingFunctionTable();
    RegisterDungeonFunctionTable();
    RegisterGameFunctionTable();
    RegisterGuildFunctionTable();
    RegisterHorseFunctionTable();
    RegisterITEMFunctionTable();
    RegisterMarriageFunctionTable();
    RegisterNPCFunctionTable();
    RegisterPartyFunctionTable();
    RegisterPCFunctionTable();
    RegisterQuestFunctionTable();
    RegisterTargetFunctionTable();
    RegisterArenaFunctionTable();
    RegisterForkedFunctionTable();
    RegisterOXEventFunctionTable();
    RegisterDragonSoulFunctionTable();
#ifdef ENABLE_MELEY_LAIR_DUNGEON
    RegisterMeleyLairFunctionTable();
#endif
    RegisterDungeonInfoFunctionTable();

    {
        luaL_reg member_functions[] = {{"chat", member_chat},
                                       {"set_ready", member_set_ready},
                                       {"clear_ready", member_clear_ready},
                                       {nullptr, nullptr}};

        AddLuaFunctionTable("member", member_functions);
    }

    {
        luaL_reg highscore_functions[] = {
            {"register", highscore_register}, {"show", highscore_show}, {nullptr, nullptr}};

        AddLuaFunctionTable("highscore", highscore_functions);
    }

    {
        luaL_reg mob_functions[] = {{"spawn", mob_spawn},
                                    {"spawn_group", mob_spawn_group},
                                    {"spawn_for_me", mob_spawn_for_me},
                                    {"set_no_reward", mob_set_no_reward},
                                    {"set_no_move", mob_set_no_move},
                                    {"remove", mob_remove},

                                    {nullptr, nullptr}};

        AddLuaFunctionTable("mob", mob_functions);
    }

    //
    // global namespace functions
    //
    RegisterGlobalFunctionTable(L);

    const char *settingsFileName = "data/settings.lua";
    const char *questlibFileName = "data/quest/questlib.lua";
    const char *questLocaleFileName = "data/quest/locale.lua";

    if (!RunFile(L, settingsFileName))
        return false;

    if (!RunFile(L, questlibFileName))
        return false;

    const auto &defaultLocale = GetLocaleService().GetDefaultLocale();

    if (!RunFile(L, fmt::format("{}/translate.lua", defaultLocale.path).c_str()))
        return false;

    lua_newtable(L);
    lua_setglobal(L, "__locale");

    for (const auto &p : GetLocaleService().GetLocales())
    {
        const auto &locale = p.second;

        if (!RunFile(L, fmt::format("{}/translate.lua", locale.path).c_str()))
            return false;

        // Legacy locale file
        // TODO: remove!
        if (!RunFile(L, questLocaleFileName))
            return false;

        // Copy |gameforge| to |__locale.<locale name>|
        lua_getglobal(L, "__locale");
        lua_pushstring(L, locale.name.c_str());
        lua_getglobal(L, "gameforge");
        lua_rawset(L, -3);
    }

    const auto stateDir = "data/quest/object/state/"s;
    auto questIndex = 0;
    if (fs::exists(stateDir))
    {
        for (const auto &entry : fs::directory_iterator(stateDir))
        {
            if (fs::is_regular_file(entry))
            {
                questIndex++;
                auto questName = entry.path().filename().generic_string();
                RegisterQuest(questName, ++questIndex);

                const std::string file = stateDir + questName;
                if (!RunFile(L, file.c_str()))
                    return false;

                BuildStateIndexToName(questName.c_str());
            }
        }
    }

    lua_setgcthreshold(L, 0);

    lua_newtable(L);
    lua_setglobal(L, "__codecache");
    return true;
}

void CQuestManager::GotoSelectState(QuestState &qs)
{
    if (!lua_checkstack(qs.co, 1))
    {
        SPDLOG_ERROR("Failed to grow the stack");
        return;
    }

    if (!lua_istable(qs.co, -1))
    {
        SPDLOG_ERROR("expected a table, got {0}", lua_typename(qs.co, -1));
        return;
    }

    int n = luaL_getn(qs.co, -1);

    std::ostringstream os;
    os << "[QUESTION  resume;" + fmt::to_string(qs.iIndex);

    for (int i = 1; i <= n; i++)
    {
        lua_rawgeti(qs.co, -1, i);

        if (lua_isstring(qs.co, -1))
        {
            os << "|" << i << ";" << lua_tostring(qs.co, -1);
        }
        else
        {
            SPDLOG_ERROR("select table contains a {0} != string", lua_typename(qs.co, -1));
            return;
        }

        lua_pop(qs.co, 1);
    }

    os << "]";

    AddScript(os.str().c_str());
    qs.suspend_state = SUSPEND_STATE_SELECT;
    SPDLOG_DEBUG("{}", m_strScript.c_str());
    SendScript();
}

EVENTINFO(confirm_timeout_event_info)
{
    uint32_t dwWaitPID;
    uint32_t dwReplyPID;

    confirm_timeout_event_info() : dwWaitPID(0), dwReplyPID(0) {}
};

EVENTFUNC(confirm_timeout_event)
{
    confirm_timeout_event_info *info = static_cast<confirm_timeout_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("confirm_timeout_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *chWait = g_pCharManager->FindByPID(info->dwWaitPID);
    CHARACTER *chReply = nullptr; // CHARACTER_MANAGER::info().FindByPID(info->dwReplyPID);

    if (chReply)
    {
        // 시간 지나면 알아서 닫힘
    }

    if (chWait)
    {
        CQuestManager::instance().Confirm(info->dwWaitPID, CONFIRM_TIMEOUT);
    }

    return 0;
}

void CQuestManager::GotoConfirmState(QuestState &qs)
{
    qs.suspend_state = SUSPEND_STATE_CONFIRM;
    uint32_t dwVID = (uint32_t)lua_tonumber(qs.co, -3);
    const char *szMsg = lua_tostring(qs.co, -2);
    int iTimeout = (int)lua_tonumber(qs.co, -1);

    SPDLOG_INFO("GotoConfirmState vid {} msg '{}', timeout {}", dwVID, szMsg, iTimeout);

    // 1. 상대방에게 확인창 띄움
    // 2. 나에게 확인 기다린다고 표시하는 창 띄움
    // 3. 타임아웃 설정 (타임아웃 되면 상대방 창 닫고 나에게도 창 닫으라고 보냄)

    // 1
    // 상대방이 없는 경우는 그냥 상대방에게 보내지 않는다. 타임아웃에 의해서 넘어가게됨
    CHARACTER *ch = g_pCharManager->Find(dwVID);
    if (ch && ch->IsPC())
    {
        ch->ConfirmWithMsg(szMsg, iTimeout, GetCurrentCharacterPtr()->GetPlayerID());
    }

    // 2
    GetCurrentPC()->SetConfirmWait((ch && ch->IsPC()) ? ch->GetPlayerID() : 0);
    ostringstream os;
    os << "[CONFIRM_WAIT resume;" << qs.iIndex << "|timeout;" << iTimeout << "]";
    AddScript(os.str());
    SendScript();

    // 3
    confirm_timeout_event_info *info = AllocEventInfo<confirm_timeout_event_info>();

    info->dwWaitPID = GetCurrentCharacterPtr()->GetPlayerID();
    info->dwReplyPID = (ch && ch->IsPC()) ? ch->GetPlayerID() : 0;

    event_create(confirm_timeout_event, info, THECORE_SECS_TO_PASSES(iTimeout));
}

void CQuestManager::GotoSelectItemState(QuestState &qs)
{
    qs.suspend_state = SUSPEND_STATE_SELECT_ITEM;
    AddScript("[SELECT_ITEM resume;" + fmt::to_string(qs.iIndex) + "]");
    SendScript();
}

void CQuestManager::GotoInputState(QuestState &qs)
{
    qs.suspend_state = SUSPEND_STATE_INPUT;
    AddScript("[INPUT resume;" + fmt::to_string(qs.iIndex) + "]");
    SendScript();

    // 시간 제한을 검
    // event_create(input_timeout_event, dwEI, THECORE_SECS_TO_PASSES(iTimeout));
}

void CQuestManager::GotoPauseState(QuestState &qs)
{
    qs.suspend_state = SUSPEND_STATE_PAUSE;
    AddScript("[NEXT resume;" + fmt::to_string(qs.iIndex) + "]");
    SendScript();
}

void CQuestManager::GotoEndState(QuestState &qs)
{
    AddScript("[DONE]");
    SendScript();
}

//
// * OpenState
//
// The beginning of script
//

QuestState CQuestManager::OpenState(const string &quest_name, int state_index, const std::string &locale, uint32_t itemId)
{
    QuestState qs;
    qs.args = 0;
    qs.st = state_index;
    qs.co = lua_newthread(L);
    qs.ico = lua_ref(L, 1 /*qs.co*/);
    qs.locale = locale;
    qs.item = itemId;

    return qs;
}

//
// * RunState
//
// decides script to wait for user input, or finish
//
bool CQuestManager::RunState(QuestState &qs)
{
    ClearError();

    m_CurrentRunningState = &qs;

    if (!SetGameforgeLocale(qs.co, qs.locale))
        return false;

    int ret = lua_resume(qs.co, qs.args);

    if (ret == 0)
    {
        if (lua_gettop(qs.co) == 0)
        {
            // end of quest
            GotoEndState(qs);
            return false;
        }

        // If we got here, we're going to try and start a suspended state.
        // Prevent more than one concurrent waiting state. If suspended state is 0,
        // we allow other quests to override it since that's the state for the
        // temporary chat quest that generates the select when clicking on NPCs.
        PC *pPC = GetCurrentPC();
        QuestState *curSuspendedState = pPC->GetSuspendedState();
        if (curSuspendedState && curSuspendedState->iIndex != 0 && curSuspendedState->iIndex != qs.iIndex)
        {
            const std::string qNameReq = CQuestManager::instance().GetQuestNameByIndex(qs.iIndex);
            const std::string qNameSuspended = CQuestManager::instance().GetQuestNameByIndex(curSuspendedState->iIndex);
            SPDLOG_ERROR("Denied suspended state for quest {} (state {}), as {} (state {}) already has one.",
                         qNameReq.c_str(), qs.st, qNameSuspended.c_str(), curSuspendedState->st);
        }
        else
        {
            pPC->SetSuspendedState(&qs);

            if (!strcmp(lua_tostring(qs.co, 1), "select"))
            {
                GotoSelectState(qs);
                return true;
            }

            if (!strcmp(lua_tostring(qs.co, 1), "wait"))
            {
                GotoPauseState(qs);
                return true;
            }

            if (!strcmp(lua_tostring(qs.co, 1), "input"))
            {
                GotoInputState(qs);
                return true;
            }

            if (!strcmp(lua_tostring(qs.co, 1), "confirm"))
            {
                GotoConfirmState(qs);
                return true;
            }

            if (!strcmp(lua_tostring(qs.co, 1), "select_item"))
            {
                GotoSelectItemState(qs);
                return true;
            }
        }
    }
    else
    {
        if (lua_gettop(qs.co) > 0)
        {
            SPDLOG_ERROR("LUA_ERROR: {}", lua_tostring(qs.co, -1));

            // Extract traceback
            if (lua_gettop(qs.co) > 1)
            {
                //luaTraceback(qs.co);
                const char *err = lua_tostring(qs.co, -1);
                SPDLOG_ERROR(err);
            }
        }
        else
        {
            SPDLOG_ERROR("LUA_ERROR: no information available.");
        }
    }

    WriteRunningStateToSyserr();
    SetError();

    GotoEndState(qs);
    return false;
}

//
// * CloseState
//
// makes script end
//
void CQuestManager::CloseState(QuestState &qs) const
{
    if (qs.co)
    {
        lua_unref(L, qs.ico);
        qs.co = nullptr;
    }
}
} // namespace quest
