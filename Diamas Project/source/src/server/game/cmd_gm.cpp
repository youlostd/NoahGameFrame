#include "DbCacheSocket.hpp"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "config.h"
#include "desc_manager.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "sectree_manager.h"
#include "utils.h"
#include <game/GamePacket.hpp>

#include "guild.h"
#include "guild_manager.h"
#include "regen.h"

#include "CharUtil.hpp"
#include "ChatUtil.hpp"
#include "DragonSoul.h"
#include "GArena.h"
#include "GBattle.h"
#include "SITokenParser.h"
#include "building.h"
#include "cube.h"
#include "db.h"
#include "fishing.h"
#include "log.h"
#include "mining.h"
#include "party.h"
#include "priv_manager.h"
#include "questmanager.h"
#include "shop.h"
#include "start_position.h"
#include "threeway_war.h"
#include "vector.h"
#include "xmas_event.h"
#include <game/AffectConstants.hpp>

#include "ItemUtils.h"
#include <cctype>

#include "gm.h"
#include "item.h"

#include "MasterUtil.hpp"
#include "MeleyLair.h"
#include "OXEvent.h"
#include "shop_manager.h"
#include <game/MasterPackets.hpp>

// ADD_COMMAND_SLOW_STUN
enum {
    COMMANDAFFECT_STUN,
    COMMANDAFFECT_SLOW,
};

void Command_ApplyAffect(CHARACTER* ch, const char* argument,
                         const char* affectName, int cmdAffect)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    SPDLOG_INFO(arg1);

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: %s <name>", affectName);
        return;
    }

    CHARACTER* tch = g_pCharManager->FindPC(arg1);
    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "%s is not in same map", arg1);
        return;
    }

    if (tch->GetGMLevel() > ch->GetGMLevel()) {
        ch->ChatPacket(CHAT_TYPE_INFO, "You cannot stun/slow someone that "
                                       "outranks you.");
        return;
    }

    switch (cmdAffect) {
        case COMMANDAFFECT_STUN:
            SkillAttackAffect(tch, 1000, 0, AFFECT_STUN, POINT_NONE, 0, 30,
                              "GM_STUN");
            break;
        case COMMANDAFFECT_SLOW:
            SkillAttackAffect(tch, 1000, 0, AFFECT_SLOW, POINT_MOV_SPEED, -30,
                              30, "GM_SLOW");
            break;
    }

    SPDLOG_INFO("%s %s", arg1, affectName);

    ch->ChatPacket(CHAT_TYPE_INFO, "%s %s", arg1, affectName);
}

// END_OF_ADD_COMMAND_SLOW_STUN

void ShowState(CHARACTER* ch, CHARACTER* tch)
{
    std::stringstream ss;

    ss << tch->GetName() << "'s PID:(" << tch->GetPlayerID() << ")"
       << " VID:(" << (uint32_t)tch->GetVID() << ")"
       << " CH:(" << (int)gConfig.channel << ")"
       << " State: ";

    if (tch->IsPosition(POS_FIGHTING))
        ss << "Battle";
    else if (tch->IsPosition(POS_DEAD))
        ss << "Dead";
    else if (tch->IsPosition(POS_FISHING))
        ss << "Fishing";
    else
        ss << "Standing";

    if (tch->GetShop())
        ss << ", Shop";

    if (tch->GetExchange())
        ss << ", Exchange";

    ch->ChatPacket(CHAT_TYPE_INFO, "%s", ss.str().c_str());

    ss.clear();

    ss << "Coordinate " << tch->GetX() << "x" << tch->GetY() << "("
       << tch->GetX() / 100 << "x" << tch->GetY() / 100 << ")";

    SECTREE* pSec = SECTREE_MANAGER::instance().Get(tch->GetMapIndex(),
                                                    tch->GetX(), tch->GetY());

    if (pSec) {
        ss << "MapIndex " << tch->GetMapIndex() << " Attribute "
           << pSec->GetAttribute(tch->GetX(), tch->GetY())
           << " Local Position (" << (tch->GetX()) / 100 << " x "
           << (tch->GetY()) / 100 << ")";
    }

    ch->ChatPacket(CHAT_TYPE_INFO, "%s", ss.str().c_str());

    if (!tch->IsShop()) {
        if (tch->IsNPC())
            ch->ChatPacket(CHAT_TYPE_INFO, "VNUM %d", tch->GetRaceNum());

        ch->ChatPacket(CHAT_TYPE_INFO, "LEV %d", tch->GetLevel());
        ch->ChatPacket(CHAT_TYPE_INFO, "HP %d/%d", tch->GetHP(),
                       tch->GetMaxHP());
        ch->ChatPacket(CHAT_TYPE_INFO, "SP %d/%d", tch->GetSP(),
                       tch->GetMaxSP());
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("ATT {:G} MAGIC_ATT {:G} SPD {:G} CRIT "
                                   "{:G}% PENE {:G}% ATT_BONUS {:G}%",
                                   tch->GetPoint(POINT_ATT_GRADE),
                                   tch->GetPoint(POINT_MAGIC_ATT_GRADE),
                                   tch->GetPoint(POINT_ATT_SPEED),
                                   tch->GetPoint(POINT_CRITICAL_PCT),
                                   tch->GetPoint(POINT_PENETRATE_PCT),
                                   tch->GetPoint(POINT_ATT_BONUS)));
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("DEF {:G} MAGIC_DEF {:G} BLOCK {:G}% DODGE "
                                   "{:G}% DEF_BONUS {:G}%",
                                   tch->GetPoint(POINT_DEF_GRADE),
                                   tch->GetPoint(POINT_MAGIC_DEF_GRADE),
                                   tch->GetPoint(POINT_BLOCK),
                                   tch->GetPoint(POINT_DODGE),
                                   tch->GetPoint(POINT_DEF_BONUS)));

        SendChatPacket(ch, CHAT_TYPE_INFO, "RESISTANCES:");
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   WARR:{:G}% ASAS:{:G}% SURA:{:G}% "
                                   "SHAM:{:G}%",
                                   tch->GetPoint(POINT_RESIST_WARRIOR),
                                   tch->GetPoint(POINT_RESIST_ASSASSIN),
                                   tch->GetPoint(POINT_RESIST_SURA),
                                   tch->GetPoint(POINT_RESIST_SHAMAN)));
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   SWORD:{:G}% THSWORD:{:G}% DAGGER:{:G}% "
                                   "BELL:{:G}% FAN:{:G}% BOW:{:G}%",
                                   tch->GetPoint(POINT_RESIST_SWORD),
                                   tch->GetPoint(POINT_RESIST_TWOHAND),
                                   tch->GetPoint(POINT_RESIST_DAGGER),
                                   tch->GetPoint(POINT_RESIST_BELL),
                                   tch->GetPoint(POINT_RESIST_FAN),
                                   tch->GetPoint(POINT_RESIST_BOW)));
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   FIRE:{:G}% LIGHT:{:G}% MAGIC:{:G}% "
                                   "WIND:{:G}% CRIT:{:G}% PENE:{:G}%",
                                   tch->GetPoint(POINT_RESIST_FIRE),
                                   tch->GetPoint(POINT_RESIST_ELEC),
                                   tch->GetPoint(POINT_RESIST_MAGIC),
                                   tch->GetPoint(POINT_RESIST_WIND),
                                   tch->GetPoint(POINT_RESIST_CRITICAL),
                                   tch->GetPoint(POINT_RESIST_PENETRATE)));
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   ICE:{:G}% EARTH:{:G}% DARK:{:G}% "
                                   "BOOSTCRIT:{:G}% BOOSTPENE:{:G}%",
                                   tch->GetPoint(POINT_RESIST_ICE),
                                   tch->GetPoint(POINT_RESIST_EARTH),
                                   tch->GetPoint(POINT_RESIST_DARK),
                                   tch->GetPoint(POINT_BOOST_CRITICAL),
                                   tch->GetPoint(POINT_BOOST_PENETRATE)));

        SendChatPacket(ch, CHAT_TYPE_INFO, "MALL:");
        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   ATT:{:G}% DEF:{:G}% EXP:{:G}% ITEMx%d "
                                   "GOLDx%d",
                                   tch->GetPoint(POINT_MALL_ATTBONUS),
                                   tch->GetPoint(POINT_MALL_DEFBONUS),
                                   tch->GetPoint(POINT_MALL_EXPBONUS),
                                   tch->GetPoint(POINT_MALL_ITEMBONUS) / 10,
                                   tch->GetPoint(POINT_MALL_GOLDBONUS) / 10));

        SendChatPacket(ch, CHAT_TYPE_INFO, "BONUS:");
        SendChatPacket(
            ch, CHAT_TYPE_INFO,
            fmt::format("   SKILL:{:G}% NORMAL:{:G}% SKILL_DEF:{:G}% "
                        "NORMAL_DEF:{:G}%",
                        tch->GetPoint(POINT_SKILL_DAMAGE_BONUS),
                        tch->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS),
                        tch->GetPoint(POINT_SKILL_DEFEND_BONUS),
                        tch->GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS)));

        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   HUMAN:{:G}% ANIMAL:{:G}% ORC:{:G}% "
                                   "MILGYO:{:G}% UNDEAD:{:G}%",
                                   tch->GetPoint(POINT_ATTBONUS_HUMAN),
                                   tch->GetPoint(POINT_ATTBONUS_ANIMAL),
                                   tch->GetPoint(POINT_ATTBONUS_ORC),
                                   tch->GetPoint(POINT_ATTBONUS_MILGYO),
                                   tch->GetPoint(POINT_ATTBONUS_UNDEAD)));

        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   DEVIL:{:G}% INSECT:{:G}% FIRE:{:G}% "
                                   "ICE:{:G}% DESERT:{:G}%",
                                   tch->GetPoint(POINT_ATTBONUS_DEVIL),
                                   tch->GetPoint(POINT_ATTBONUS_INSECT),
                                   tch->GetPoint(POINT_ATTBONUS_FIRE),
                                   tch->GetPoint(POINT_ATTBONUS_ICE),
                                   tch->GetPoint(POINT_ATTBONUS_DESERT)));

        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   TREE:{:G}% MOB:{:G}% BOSS:{:G}% METIN: "
                                   "{:G}%",
                                   tch->GetPoint(POINT_ATTBONUS_TRENT),
                                   tch->GetPoint(POINT_ATTBONUS_MONSTER),
                                   tch->GetPoint(POINT_ATTBONUS_BOSS),
                                   tch->GetPoint(POINT_ATTBONUS_METIN)));

        SendChatPacket(ch, CHAT_TYPE_INFO,
                       fmt::format("   WARR:{:G}% ASAS:{:G}% SURA:{:G}% "
                                   "SHAM:{:G}%",
                                   tch->GetPoint(POINT_ATTBONUS_WARRIOR),
                                   tch->GetPoint(POINT_ATTBONUS_ASSASSIN),
                                   tch->GetPoint(POINT_ATTBONUS_SURA),
                                   tch->GetPoint(POINT_ATTBONUS_SHAMAN)));
    }

    if (tch->IsPC()) {
        ch->ChatPacket(CHAT_TYPE_INFO, "PREMIUM:");
        for (int i = 0; i < MAX_PRIV_NUM; ++i) {
            if (tch->GetPremiumRemainSeconds(i) > 0 &&
                strcmp(c_apszPremiumNames[i], "") != 0) {
                ch->ChatPacket(CHAT_TYPE_INFO, "  %s: %d seconds left.",
                               LC_TEXT(c_apszPremiumNames[i]),
                               tch->GetPremiumRemainSeconds(i));
            }
        }

        for (int i = 0; i < MAX_PRIV_NUM; ++i) {
            if (CPrivManager::instance().GetPriv(tch, i)) {
                int iByEmpire = CPrivManager::instance().GetPrivByEmpire(
                                    tch->GetEmpire(), i) +
                                CPrivManager::instance().GetPrivByEmpire(0, i);
                int iByGuild = 0;

                if (tch->GetGuild())
                    iByGuild = CPrivManager::instance().GetPrivByGuild(
                        tch->GetGuild()->GetID(), i);

                int iByPlayer = CPrivManager::instance().GetPrivByCharacter(
                    tch->GetPlayerID(), i);

                if (iByEmpire)
                    ch->ChatPacket(CHAT_TYPE_INFO, "%s for empire : %d",
                                   LC_TEXT(c_apszPrivNames[i]), iByEmpire);

                if (iByGuild)
                    ch->ChatPacket(CHAT_TYPE_INFO, "%s for guild : %d",
                                   LC_TEXT(c_apszPrivNames[i]), iByGuild);

                if (iByPlayer)
                    ch->ChatPacket(CHAT_TYPE_INFO, "%s for player : %d",
                                   LC_TEXT(c_apszPrivNames[i]), iByPlayer);
            }
        }
    }

}

ACMD(do_stun)
{
    Command_ApplyAffect(ch, argument, "stun", COMMANDAFFECT_STUN);
}

ACMD(do_slow)
{
    Command_ApplyAffect(ch, argument, "slow", COMMANDAFFECT_SLOW);
}

ACMD(do_list_affect)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    CHARACTER* tch = ch;
    if (*arg1) {
        tch = g_pCharManager->FindPC(arg1);
        if (!tch)
            tch = ch;
    }

    ch->ChatPacket(CHAT_TYPE_INFO,
                   "-- Affect List of %s -------------------------------",
                   tch->GetName());
    ch->ChatPacket(CHAT_TYPE_INFO, "Type Point Modif Duration Flag");

    for (const auto& affect : tch->GetAffectContainer()) {
        ch->ChatPacket(CHAT_TYPE_INFO, "%4d %5d %5d %f", affect.type,
                       affect.pointType, affect.pointValue, affect.duration);
    }
}

ACMD(do_add_affect)
{
    char arg1[256], arg2[256], arg3[256], arg4[256], arg5[256];
    argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
    argument = two_arguments(argument, arg3, sizeof(arg3), arg4, sizeof(arg4));
    argument = one_argument(argument, arg5, sizeof(arg5));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: /add_affect <name|- for myself> "
                                       "<type> <point> <value> <duration>");
        return;
    }

    const auto tch = arg1[0] == '-' ? ch : g_pCharManager->FindPC(arg1);
    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "%s is not here!", arg1);
        return;
    }

    uint32_t type, pointType, duration;
    int32_t pointValue;
    if (!str_to_number(type, arg2) || !str_to_number(pointValue, arg4) ||
        !str_to_number(duration, arg5)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid parameters");
        return;
    }

    auto success = GetPointTypeValue(arg3, pointType);
    if (!success || (!pointType && !str_to_number(pointType, arg3))) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid point type {}", arg3);
        return;
    }

    tch->AddAffect(type, pointType, pointValue, duration, 0, false);
    ch->ChatPacket(CHAT_TYPE_INFO, "Affect added successfully");
}

ACMD(do_remove_affect)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <type> <point>");
        return;
    }

    bool removed = false;

    uint32_t type = 0;
    uint8_t point = 0;

    if (!str_to_number(type, arg1) || !str_to_number(point, arg2)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid parameters");
        return;
    }

    while (true) {
        auto af = ch->FindAffect(type, point);
        if (!af)
            break;
        ch->RemoveAffect(*af);
        removed = true;
    }

    if (removed)
        ch->ChatPacket(CHAT_TYPE_INFO, "Affect successfully removed.");
    else
        ch->ChatPacket(CHAT_TYPE_INFO, "Not affected by that type and point.");
}

ACMD(do_clear_affect)
{
    ch->ClearAffect();
}

ACMD(do_transfer)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: transfer <name> "
                                       "[block_observer=(0,1)]");
        return;
    }

    uint8_t blockObserver = false;

    if (*arg2 && !str_to_number(blockObserver, arg2)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid parameter for block_observer "
                                       "choose 0 or 1");
        return;
    }

    CHARACTER* tch = g_pCharManager->FindPC(arg1);
    if (!tch) {
        const auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(arg1);
        if (!op) {
            ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
            return;
        }

        if (MeleyLair::CMgr::instance().IsMeleyMap(op->mapIndex)) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
            return;
        }

        GmCharacterWarpPacket p;
        p.pid = op->pid;
        p.mapIndex = ch->GetMapIndex();
        p.x = ch->GetX();
        p.y = ch->GetY();
        p.channel = gConfig.channel;
        DESC_MANAGER::instance().GetMasterSocket()->Send(kGmCharacterWarp, p);
        return;
    }

    if (blockObserver) {
        tch->AddAffect(AFFECT_NON_OBSERVER, POINT_NONE, 0,
                       INFINITE_AFFECT_DURATION, 0, false);
    }

    if (MeleyLair::CMgr::instance().IsMeleyMap(tch->GetMapIndex())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return;
    }

    tch->WarpSet(ch->GetMapIndex(), ch->GetX(), ch->GetY());
}

// LUA_ADD_GOTO_INFO

static std::vector<GotoInfo> gs_vec_gotoInfo;

void CHARACTER_AddGotoInfo(const std::string& c_st_name, uint8_t empire,
                           int mapIndex, uint32_t x, uint32_t y)
{
    GotoInfo newGotoInfo;
    newGotoInfo.st_name = c_st_name;
    newGotoInfo.empire = empire;
    newGotoInfo.mapIndex = mapIndex;
    newGotoInfo.x = x;
    newGotoInfo.y = y;
    gs_vec_gotoInfo.push_back(newGotoInfo);

    if (gConfig.testServer)
        SPDLOG_INFO("AddGotoInfo(name={0}, empire={1}, mapIndex={2}, pos=({3}, "
                    "{4}))",
                    c_st_name.c_str(), empire, mapIndex, x, y);
}

bool FindInString(const char* c_pszFind, const char* c_pszIn)
{
    const char* c = c_pszIn;
    const char* p;

    p = strchr(c, '|');
    int c_pszFindLength = strlen(c_pszFind);

    if (!p)
        return (0 == strncasecmp(c_pszFind, c_pszIn, c_pszFindLength));
    else {
        char sz[64 + 1];
        do {
            strlcpy(sz, c, std::min<int>(sizeof(sz), (p - c) + 1));

            if (!strncasecmp(c_pszFind, sz, c_pszFindLength))
                return true;

            c = p + 1;
        } while ((p = strchr(c, '|')));

        strlcpy(sz, c, sizeof(sz));

        if (!strncasecmp(c_pszFind, sz, c_pszFindLength))
            return true;
    }

    return false;
}

bool CHARACTER_GoToName(CHARACTER* ch, uint8_t empire, int mapIndex,
                        const char* gotoName)
{
    std::vector<GotoInfo>::iterator i;
    for (i = gs_vec_gotoInfo.begin(); i != gs_vec_gotoInfo.end(); ++i) {
        const GotoInfo& c_eachGotoInfo = *i;

        if (mapIndex != 0) {
            if (mapIndex != c_eachGotoInfo.mapIndex)
                continue;
        } else if (!FindInString(gotoName, c_eachGotoInfo.st_name.c_str()))
            continue;

        if (c_eachGotoInfo.empire == 0 || c_eachGotoInfo.empire == empire) {
            int x = c_eachGotoInfo.x * 100;
            int y = c_eachGotoInfo.y * 100;

            ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
            ch->WarpSet(c_eachGotoInfo.mapIndex, x, y);
            ch->Stop();
            return true;
        }
    }
    return false;
}

// END_OF_LUA_ADD_GOTO_INFO

/*
   = {
   { "A1|¿µ¾ÈÀ¾¼º",		0, 1,  4693, 9642 },
   { "A3|ÀÚ¾çÇö",		0, 3,  3608, 8776 },

   { "B1|Á¶¾ÈÀ¾¼º",		0, 21,  557, 1579 },
   { "B3|º¹Á¤Çö",		0, 23, 1385, 2349 },

   { "C1|Æò¹«À¾¼º",		0, 41, 9696, 2784 },
   { "C3|¹Ú¶óÇö",		0, 43, 8731, 2426 },

// Snow
{ "Snow|¼­ÇÑ»ê",		1, 61, 4342, 2906 },
{ "Snow|¼­ÇÑ»ê",		2, 61, 3752, 1749 },
{ "Snow|¼­ÇÑ»ê",		3, 61, 4918, 1736 },

// Flame
{ "Flame|µµ¿°È­Áö|È­¿°",	1, 62, 5994, 7563 },
{ "Flame|µµ¿°È­Áö|È­¿°",	2, 62, 5978, 6222 },
{ "Flame|µµ¿°È­Áö|È­¿°",	3, 62, 7307, 6898 },

// Desert
{ "Desert|¿µºñ»ç¸·|»ç¸·",	1, 63, 2178, 6272 },
{ "Desert|¿µºñ»ç¸·|»ç¸·",	2, 63, 2219, 5027 },
{ "Desert|¿µºñ»ç¸·|»ç¸·",	3, 63, 3440, 5025 },

// Threeway
{ "Three|½Â·æ°î",		1, 64, 4021, 6739 },
{ "Three|½Â·æ°î",		2, 64, 2704, 7399 },
{ "Three|½Â·æ°î",		3, 64, 3213, 8080 },

// ¹Ð±³»ç¿ø
{ "Milgyo|¹Ð±³»ç¿ø",	1, 65, 5536, 1436 },
{ "Milgyo|¹Ð±³»ç¿ø",	2, 65, 5536, 1436 },
{ "Milgyo|¹Ð±³»ç¿ø",	3, 65, 5536, 1436 },

// »ç±ÍÅ¸¿öÀÔ±¸
{ "»ç±ÍÅ¸¿öÀÔ±¸",		1, 65, 5905, 1108 },
{ "»ç±ÍÅ¸¿öÀÔ±¸",		2, 65, 5905, 1108 },
{ "»ç±ÍÅ¸¿öÀÔ±¸",		3, 65, 5905, 1108 },

{ NULL,			0,  0,    0,    0 },
};
 */

ACMD(do_goto)
{
    char arg1[256], arg2[256];
    int x = 0, y = 0, z = 0;

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 && !*arg2) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: goto <x meter> <y meter>");
        return;
    }

    if (isdigit(*arg1) && isdigit(*arg2)) {
        str_to_number(x, arg1);
        str_to_number(y, arg2);

        ch->ChatPacket(CHAT_TYPE_INFO, "You goto ( %d, %d )", x, y);
    } else {
        int mapIndex = 0;
        uint8_t empire = 0;

        if (*arg1 == '#')
            str_to_number(mapIndex, (arg1 + 1));

        if (*arg2 && isdigit(*arg2)) {
            str_to_number(empire, arg2);
            empire = std::clamp<uint8_t>(empire, 1, 3);
        } else
            empire = ch->GetEmpire();

        if (CHARACTER_GoToName(ch, empire, mapIndex, arg1))
            return;

        ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map command syntax: /goto "
                                       "<mapname> [empire]");
        return;
    }

    x *= 100;
    y *= 100;

    ch->Show(ch->GetMapIndex(), x, y, z);
    ch->Stop();
}

ACMD(do_wallhack)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid arguments");
        return;
    }

    uint8_t isWallhack = 0;
    str_to_number(isWallhack, arg1);

    SendChatPacket(ch, CHAT_TYPE_COMMAND,
                   fmt::format("WallHack {0}", isWallhack));
}

ACMD(do_bot_report)
{
    /*
     *const char* szHelp = "Usage: bot_report <character name>";
    CHECK_COMMAND(Parser, 1, szHelp);

    const auto arg1 = Parser.GetTokenString(0);

    auto tch = g_pCharManager->FindPC(arg1);

    if (NULL == tch)
    {
        const CCI* pkCci = P2P_MANAGER::instance().Find(arg1);

        if (NULL != pkCci)
        {
            TPacketGGPlayerReport p;
            p.header = HEADER_GG_PLAYER_REPORT;
            p.dwFromPID = ch->GetPlayerID();
            p.dwTargetPID = pkCci->pid;
            pkCci->pkDesc->Packet(&p, sizeof(TPacketGGFindPosition));
        }
        else
        {
            ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
        }

    }
    else {
        BlankPacket p;
        p.header = HEADER_GC_BOT_REPORT;
        ch->GetDesc()->Packet(&p, sizeof(p));
    }
    */
}

ACMD(do_warp)
{
    char arg1[256], arg2[256], arg3[256];
    one_argument(
        two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3,
        sizeof(arg3));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: warp <character name> | <map "
                                       "index> [<x meter> <y meter>]");
        return;
    }

    auto& dm = DESC_MANAGER::instance();

    int32_t map = 0;
    PIXEL_POSITION pos{};
    auto tch = g_pCharManager->FindPC(arg1);

    if (isdigit(*arg1) && !tch) {
        const auto op = dm.GetOnlinePlayers().Get(arg1);

        if (!op) {
            str_to_number(map, arg1);
            if (isdigit(*arg2) && isdigit(*arg3)) {
                str_to_number(pos.x, arg2);
                str_to_number(pos.y, arg3);
                pos.x *= 100;
                pos.y *= 100;
            } else if (!SECTREE_MANAGER::instance().GetSpawnPositionByMapIndex(
                           map, pos)) {
                ch->ChatPacket(CHAT_TYPE_INFO, "Could not find map %u", map);
                return;
            }
        } else {
            GmCharacterTransferPacket p;
            p.sourcePid = ch->GetPlayerID();
            p.targetPid = op->pid;
            dm.GetMasterSocket()->Send(kGmCharacterTransfer, p);
            return;
        }
    } else if (tch) {
        map = tch->GetMapIndex();
        pos.x = tch->GetX();
        pos.y = tch->GetY();
    } else {
        const auto op = dm.GetOnlinePlayers().Get(arg1);

        if (!op) {
            ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
            return;
        }

        GmCharacterTransferPacket p;
        p.sourcePid = ch->GetPlayerID();
        p.targetPid = op->pid;
        dm.GetMasterSocket()->Send(kGmCharacterTransfer, p);
        return;
    }

    ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", pos.x, pos.y);
    ch->WarpSet(map, pos.x, pos.y);
    ch->Stop();
}

ACMD(do_rewarp)
{
    int x = ch->GetX(), y = ch->GetY();
    int mapIndex = ch->GetMapIndex();
    ch->ChatPacket(CHAT_TYPE_INFO, "You warp to map %d at ( %d, %d )", mapIndex,
                   x, y);
    ch->WarpSet(mapIndex, x, y);
    ch->Stop();
}

ACMD(do_user_rewarp)
{
    const auto now = get_global_time();
    if (const auto questPc =
            quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
        questPc) {
        uint32_t useCycle =
            quest::CQuestManager::instance().GetEventFlag("rewarp_user_cycle");
        if (!useCycle)
            useCycle = 1;
        uint32_t laseUseTime = questPc->GetFlag("RewarpUser.LastUseTime");

        if (laseUseTime + useCycle > now) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You can not use this so fast.");
            return;
        }
    }

    if (!ch->CanWarp()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You cannot perform this action right now.");
        return;
    }

    if (MeleyLair::CMgr::instance().IsMeleyMap(ch->GetMapIndex())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return;
    }
    int x = ch->GetX(), y = ch->GetY();
    int mapIndex = ch->GetMapIndex();
    // ch->ChatPacket(CHAT_TYPE_INFO, "You warp to map %d at ( %d, %d )"
    // ,mapIndex, x, y);
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "Settings applied and reloaded");

    const auto questPc =
        quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
    if (questPc) {
        questPc->SetFlag("RewarpUser.LastUseTime", now);
    }

    ch->WarpSet(mapIndex, x, y);
    ch->Stop();
}

ACMD(do_give_item)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 && !*arg2) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: give_item <player> <item_vnum>");
        return;
    }

    CHARACTER* tch = g_pCharManager->FindPC(arg1);
    if (tch) {
        uint32_t dwVnum;

        if (isNumeric(std::string(arg2)))
            str_to_number(dwVnum, arg2);
        else {
            if (!ITEM_MANAGER::instance().GetVnum(arg2, dwVnum)) {
                ch->ChatPacket(CHAT_TYPE_INFO,
                               "#%u item not exist by that vnum.", dwVnum);
                return;
            }
        }

        int iCount = 1;

        auto item =
            ITEM_MANAGER::instance().CreateItem(dwVnum, iCount, 0, true);
        if (item) {
            tch->AutoGiveItem(item, true);
        } else {
            ch->ChatPacket(CHAT_TYPE_INFO, "No item found by #%u.", dwVnum);
        }
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "Player %s is not on this instance.",
                       arg1);
    }
}

ACMD(do_item)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: item <item vnum>");
        return;
    }

    uint32_t dwVnum;

    if (isNumeric(std::string(arg1)))
        str_to_number(dwVnum, arg1);
    else {
        if (!ITEM_MANAGER::instance().GetVnum(arg1, dwVnum)) {
            ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum.",
                           dwVnum);
            return;
        }
    }

    int iCount = 1;

    if (isNumeric(std::string(arg2))) {
        str_to_number(iCount, arg2);
        iCount = std::clamp<uint32_t>(iCount, 1, GetItemMaxCount(dwVnum));
    }

    auto item = ITEM_MANAGER::instance().CreateItem(dwVnum, iCount, 0, true);
    if (item) {
        ch->AutoGiveItem(item, true);
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "No item found by #%u.", dwVnum);
    }
}

ACMD(do_pet_item)
{
    const char* szHelp = "/pet_item <item vnum> <mob vnum> [scale]";
    CHECK_COMMAND(Parser, 2, szHelp);

    auto itemVnum = Parser.GetTokenInt(0);
    auto mobVnum = Parser.GetTokenInt(1);

    int32_t scale = 0;
    if (Parser.GetTokenNum() == 3)
        scale = Parser.GetTokenInt(2);

    const auto table = ITEM_MANAGER::instance().GetTable(itemVnum);
    if (!table) {
        ch->ChatPacket(CHAT_TYPE_INFO, "An item with this vnum could not be "
                                       "found");
        return;
    }

    if (table->bType != ITEM_TOGGLE || table->bSubType != TOGGLE_PET) {
        ch->ChatPacket(CHAT_TYPE_INFO, "The given item is not a pet seal");
        return;
    }

    CItem* item = ITEM_MANAGER::instance().CreateItem(itemVnum, 1, 0, true);
    if (item) {
        auto resultItem = ch->AutoGiveItem(item, true);
        resultItem->SetSocket(0, mobVnum);
        resultItem->SetSocket(1, scale);
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "No item found by #%u.", itemVnum);
    }
}

ACMD(do_group_random)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: grrandom <group vnum>");
        return;
    }

    uint32_t dwVnum = 0;
    str_to_number(dwVnum, arg1);
    g_pCharManager->SpawnGroupGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500,
                                    ch->GetY() - 500, ch->GetX() + 500,
                                    ch->GetY() + 500);
}

ACMD(do_group)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: group <group vnum>");
        return;
    }

    uint32_t dwVnum = 0;
    str_to_number(dwVnum, arg1);

    if (gConfig.testServer)
        SPDLOG_INFO("COMMAND GROUP SPAWN %u at %u %u %u", dwVnum,
                    ch->GetMapIndex(), ch->GetX(), ch->GetY());

    g_pCharManager->SpawnGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500,
                               ch->GetY() - 500, ch->GetX() + 500,
                               ch->GetY() + 500);
}

ACMD(do_mob_coward)
{
    char arg1[256], arg2[256];
    uint32_t vnum = 0;
    CHARACTER* tch;

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mc <vnum>");
        return;
    }

    const TMobTable* pkMob;

    if (isdigit(*arg1)) {
        str_to_number(vnum, arg1);

        if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
            vnum = 0;
    } else {
        pkMob = CMobManager::Instance().Get(arg1, true);

        if (pkMob)
            vnum = pkMob->dwVnum;
    }

    if (vnum == 0) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No such mob by that vnum");
        return;
    }

    int iCount = 0;

    if (*arg2)
        str_to_number(iCount, arg2);
    else
        iCount = 1;

    iCount = std::min(20, iCount);

    while (iCount--) {
        tch = g_pCharManager->SpawnMobRange(
            vnum, ch->GetMapIndex(), ch->GetX() - Random::get(200, 750),
            ch->GetY() - Random::get(200, 750),
            ch->GetX() + Random::get(200, 750),
            ch->GetY() + Random::get(200, 750), true,
            pkMob->bType == CHAR_TYPE_STONE);
        if (tch)
            tch->SetCoward();
    }
}

ACMD(do_mob_map)
{
    const char* szHelp = "Syntax: mm <vnum> (count)";
    CHECK_COMMAND(Parser, 1, szHelp);

    int Params = Parser.GetTokenNum();

    if (Params == 2) {
        auto mobVnum = Parser.GetTokenInt(0);
        auto count = std::clamp(Parser.GetTokenInt(1), 1, 50);

        for (int i = 0; i <= count; ++i) {
            auto tch = g_pCharManager->SpawnMobRandomPosition(
                mobVnum, ch->GetMapIndex());

            if (tch)
                ch->ChatPacket(CHAT_TYPE_INFO, "%s spawned in %dx%d",
                               tch->GetName(), tch->GetX(), tch->GetY());
            else
                ch->ChatPacket(CHAT_TYPE_INFO, "Spawn failed.");
        }
    } else {
        auto mobVnum = Parser.GetTokenInt(0);
        CHARACTER* tch =
            g_pCharManager->SpawnMobRandomPosition(mobVnum, ch->GetMapIndex());

        if (tch)
            ch->ChatPacket(CHAT_TYPE_INFO, "%s spawned in %dx%d",
                           tch->GetName(), tch->GetX(), tch->GetY());
        else
            ch->ChatPacket(CHAT_TYPE_INFO, "Spawn failed.");
    }
}

ACMD(do_mob_aggresive)
{
    char arg1[256], arg2[256];
    uint32_t vnum = 0;

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
        return;
    }

    const TMobTable* pkMob;

    if (isdigit(*arg1)) {
        str_to_number(vnum, arg1);

        if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
            vnum = 0;
    } else {
        pkMob = CMobManager::Instance().Get(arg1, true);

        if (pkMob)
            vnum = pkMob->dwVnum;
    }

    if (vnum == 0) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No such mob by that vnum");
        return;
    }

    int iCount = 0;

    if (*arg2)
        str_to_number(iCount, arg2);
    else
        iCount = 1;

    iCount = std::min(20, iCount);

    while (iCount--) {
        g_pCharManager->SpawnMobRange(vnum, ch->GetMapIndex(),
                                      ch->GetX() - Random::get(200, 750),
                                      ch->GetY() - Random::get(200, 750),
                                      ch->GetX() + Random::get(200, 750),
                                      ch->GetY() + Random::get(200, 750), true,
                                      pkMob->bType == CHAR_TYPE_STONE, true);
    }
}

ACMD(do_mob)
{
    char arg1[256], arg2[256];
    uint32_t vnum = 0;

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
        return;
    }

    const TMobTable* pkMob = nullptr;

    if (isdigit(*arg1)) {
        str_to_number(vnum, arg1);

        if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
            vnum = 0;
    } else {
        pkMob = CMobManager::Instance().Get(arg1, true);

        if (pkMob)
            vnum = pkMob->dwVnum;
    }

    if (vnum == 0) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No such mob by that vnum");
        return;
    }

    int iCount = 0;

    if (*arg2)
        str_to_number(iCount, arg2);
    else
        iCount = 1;

    if (gConfig.testServer)
        iCount = std::min(400, iCount);
    else
        iCount = std::min(200, iCount);

    while (iCount--) {
        g_pCharManager->SpawnMobRange(vnum, ch->GetMapIndex(),
                                      ch->GetX() - Random::get(600, 1050),
                                      ch->GetY() - Random::get(600, 1050),
                                      ch->GetX() + Random::get(600, 1050),
                                      ch->GetY() + Random::get(600, 1050), true,
                                      pkMob->bType == CHAR_TYPE_STONE);
    }
}

ACMD(do_mob_ld)
{
    char arg1[256], arg2[256], arg3[256], arg4[256];
    uint32_t vnum = 0;

    two_arguments(
        two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3,
        sizeof(arg3), arg4, sizeof(arg4));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
        return;
    }

    const TMobTable* pkMob = nullptr;

    if (isdigit(*arg1)) {
        str_to_number(vnum, arg1);

        if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
            vnum = 0;
    } else {
        pkMob = CMobManager::Instance().Get(arg1, true);

        if (pkMob)
            vnum = pkMob->dwVnum;
    }

    if (vnum == 0) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No such mob by that vnum");
        return;
    }

    int dir = 1;
    long x = 0;
    long y = 0;

    if (*arg2)
        str_to_number(x, arg2);
    if (*arg3)
        str_to_number(y, arg3);
    if (*arg4)
        str_to_number(dir, arg4);

    g_pCharManager->SpawnMob(vnum, ch->GetMapIndex(), x * 100, y * 100,
                             ch->GetZ(), pkMob->bType == CHAR_TYPE_STONE, dir);
}

struct FuncPurge {
    CHARACTER* m_pkGM;
    bool m_bAll;

    FuncPurge(CHARACTER* ch)
        : m_pkGM(ch)
        , m_bAll(false)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER* pkChr = (CHARACTER*)ent;

        if (m_pkGM == pkChr)
            return;

        if (pkChr->IsShop() || pkChr->IsPet() || pkChr->GetRider() ||
            pkChr->IsBuilding() ||
            pkChr->IsPC()) // We dont want to purge shops pets and mounts
                           // buffbots and buildings
            return;

        int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(),
                                    pkChr->GetY() - m_pkGM->GetY());

        if (!m_bAll &&
            iDist >= 1000) // 10¹ÌÅÍ ÀÌ»ó¿¡ ÀÖ´Â °ÍµéÀº purge ÇÏÁö ¾Ê´Â´Ù.
            return;

        SPDLOG_INFO("PURGE: %s %d", pkChr->GetName(), iDist);

        M2_DESTROY_CHARACTER(pkChr);
    }
};

ACMD(do_purge)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    FuncPurge func(ch);

    if (*arg1 && !strcmp(arg1, "map")) {
        g_pCharManager->DestroyCharacterInMap(ch->GetMapIndex());
    } else if (*arg1 && !strcmp(arg1, "all")) {
        func.m_bAll = true;
        ch->ForEachSeen(func);
    } else {
        ch->ForEachSeen(func);
    }
}

ACMD(do_inventory_purge)
{
    for (uint32_t i = 0; i < INVENTORY_MAX_NUM; ++i) {
        if (auto item = ch->GetInventoryItem(i); item) {
            ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
            ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i,
                              std::numeric_limits<uint16_t>::max());
        }
    }

    for (uint32_t i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i) {
        if (auto item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i)); item) {
            ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
        }
    }
}

ACMD(do_equipment_purge)
{
    for (auto i = 0; i < WEAR_MAX_NUM; ++i) {
        if (auto item = ch->GetWear(i); item) {
            ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
            ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i,
                              std::numeric_limits<uint16_t>::max());
        }
    }
}

ACMD(do_state)
{
    char arg1[256];
    CHARACTER* tch;

    one_argument(argument, arg1, sizeof(arg1));

    if (*arg1) {
        if (arg1[0] == '#')
            tch = g_pCharManager->Find(strtoul(arg1 + 1, nullptr, 10));
        else
            tch = g_pCharManager->FindPC(arg1);
    } else
        tch = ch;

    if (!tch)
        return;

    ShowState(ch, tch);
}

ACMD(do_notice)
{
    auto message = fmt::format("{0}: {1}", ch->GetName(), argument);
    BroadcastNotice(message.c_str());
}

ACMD(do_admin_notice)
{
    BroadcastNotice(argument);
}

ACMD(do_whisper_all)
{
    BroadcastWhisperAll(argument);
}

ACMD(do_map_notice)
{
    SendNoticeMap(argument, ch->GetMapIndex(), false);
}

ACMD(do_big_notice_map)
{
    if (!ch)
        return;

    BroadcastBigNotice(argument, ch->GetMapIndex()); // Broadcast to the map
}

ACMD(do_big_notice)
{
    BroadcastBigNotice(argument, -1); // Broadcast to all core maps AND peers
}

ACMD(do_who)
{
    int iTotal;
    int* paiEmpireUserCount;
    int iLocal;

    DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

    ch->ChatPacket(CHAT_TYPE_INFO,
                   "Total users in game %d (%d / %d / %d) (This core %d)",
                   iTotal, paiEmpireUserCount[1], paiEmpireUserCount[2],
                   paiEmpireUserCount[3], iLocal);
}

class user_func
{
    public:
    CHARACTER* m_ch;
    static int count;
    static char str[128];
    static int str_len;

    user_func()
        : m_ch(nullptr)
    {
    }

    void initialize(CHARACTER* ch)
    {
        m_ch = ch;
        str_len = 0;
        count = 0;
        str[0] = '\0';
    }

    void operator()(DescPtr d)
    {
        if (!d->GetCharacter())
            return;

        int len = std::snprintf(str + str_len, sizeof(str) - str_len, "%-16s ",
                                d->GetCharacter()->GetName().c_str());

        if (len < 0 || len >= (int)sizeof(str) - str_len)
            len = (sizeof(str) - str_len) - 1;

        str_len += len;
        ++count;

        if (!(count % 4)) {
            m_ch->ChatPacket(CHAT_TYPE_INFO, str);

            str[0] = '\0';
            str_len = 0;
        }
    }
};

int user_func::count = 0;
char user_func::str[128] = {
    0,
};
int user_func::str_len = 0;

ACMD(do_user)
{
    const DESC_MANAGER::DESC_SET& c_ref_set =
        DESC_MANAGER::instance().GetClientSet();
    user_func func;

    func.initialize(ch);
    std::for_each(c_ref_set.begin(), c_ref_set.end(), func);

    if (func.count % 4)
        ch->ChatPacket(CHAT_TYPE_INFO, func.str);

    ch->ChatPacket(CHAT_TYPE_INFO, "Total %d", func.count);
}

ACMD(do_disconnect)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "ex) /dc <player name>");
        return;
    }

    CHARACTER* tch = g_pCharManager->FindPC(arg1);
    if (tch) {
        tch->GetDesc()->DelayedDisconnect(0, "GM_DC");
    }
    if (!tch) {
        const auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(arg1);
        if (op)
            RelayDisconnect(op->pid);
    }
}

ACMD(do_ban)
{
    // Args
    char arg1[256], arg2[256], arg3[256];

    // Local variables
    const char* szName;
    const char* szReason;
    int iDuration;

    one_argument(
        two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3,
        sizeof(arg3));

    // Invalid syntax
    if (!*arg1 || !*arg2 || !*arg3) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid Syntax, usage: <player name> "
                                       "<time string (1d/1h/1m/1s)> <reason> "
                                       "tip: don't use spaces in the reason, "
                                       "use _");
        return;
    }

    szName = arg1;
    iDuration = parse_time_str(arg2);
    szReason = arg3;

    if (iDuration <= 0) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid ban duration");
        return;
    }

    if (!check_name(szName)) {
        return;
    }
    char reasonEscaped[GUILD_NAME_MAX_LEN * 2 + 1];
    DBManager::instance().EscapeString(reasonEscaped, sizeof(reasonEscaped), szReason,
                                       strlen(szReason));

    std::string query = "UPDATE account.account LEFT JOIN player.player ON "
                        "player.account_id = account.id SET account.availDt = "
                        "FROM_UNIXTIME(UNIX_TIMESTAMP(CURRENT_TIMESTAMP()) + "
                        "{}), account.reason = '{}' WHERE player.name = '{}' AND deleted = 0";
    std::unique_ptr<SQLMsg> msg(DBManager::instance().DirectQuery(
        query.c_str(), iDuration, reasonEscaped, szName));
    if (msg->Get()->uiAffectedRows > 0) {

        CHARACTER* tch = g_pCharManager->FindPC(szName);
        if (tch) {
            tch->GetDesc()->DelayedDisconnect(0, "GM_BAN");
        }
        if (!tch) {
            const auto op =
                DESC_MANAGER::instance().GetOnlinePlayers().Get(arg1);
            if (op)
                RelayDisconnect(op->pid);
        }

        ch->ChatPacket(CHAT_TYPE_INFO,
                       "%s has been banned for %s with reason: %s", szName,
                       arg2, szReason);

        char szNotice[64];
        snprintf(szNotice, sizeof szNotice, "%s has been banned!", szName);

        BroadcastNoticeSpecial(szNotice);
    }
}

ACMD(do_kill)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "ex) /kill <player name>");
        return;
    }

    CHARACTER* tch = g_pCharManager->FindPC(arg1);
    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
        return;
    }

    tch->Dead();
}

#define MISC 0
#define BINARY 1
#define NUMBER 2

const struct set_struct {
    const char* cmd;
    const char type;
} set_fields[] = {{"gold", NUMBER},   {"race", BINARY},
                  {"sex", BINARY},    {"exp", NUMBER},
                  {"max_hp", NUMBER}, {"max_sp", NUMBER},
                  {"skill", NUMBER},  {"alignment", NUMBER},
                  {"align", NUMBER},  {"tree_point", NUMBER},

                  {"\n", MISC}};

ACMD(do_set)
{
    char arg1[256], arg2[256], arg3[256];

    CHARACTER* tch = nullptr;

    int i, len;
    const char* line;

    line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
    one_argument(line, arg3, sizeof(arg3));

    if (!*arg1 || !*arg2 || !*arg3) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: set <name> <field> <value>");
        return;
    }

    tch = g_pCharManager->FindPC(arg1);

    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
        return;
    }

#ifdef __NEW_GAMEMASTER_CONFIG__
    if (tch != ch &&
        !GM::check_allow(ch->GetGMLevel(), GM_ALLOW_MODIFY_OTHERS)) {
        ch->ChatPacket(CHAT_TYPE_INFO, ("You have no rights to modify other "
                                        "players."));
        return;
    }
#endif

    PointValue newVal = 0;

    len = strlen(arg2);

    for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
        if (!strncmp(arg2, set_fields[i].cmd, len))
            break;

    switch (i) {
        case 0: // gold
        {
            Gold gold = 0;
            str_to_number(gold, arg3);
            DBManager::instance().SendMoneyLog(MONEY_LOG_MISC, 3, gold);
            tch->ChangeGold(gold);
            newVal = tch->GetGold();
        } break;

        case 1: // race
            break;

        case 2: // sex
            break;

        case 3: // exp
        {
            PointValue amount = 0;
            str_to_number(amount, arg3);
            tch->PointChange(POINT_EXP, amount, true);
            newVal = tch->GetExp();
        } break;

        case 4: // max_hp
        {
            PointValue amount = 0;
            str_to_number(amount, arg3);
            tch->PointChange(POINT_MAX_HP, amount, true);
            newVal = tch->GetMaxHP();
        } break;

        case 5: // max_sp
        {
            PointValue amount = 0;
            str_to_number(amount, arg3);
            tch->PointChange(POINT_MAX_SP, amount, true);
            newVal = tch->GetMaxSP();
        } break;

        case 6: // active skill point
        {
            PointValue amount = 0;
            str_to_number(amount, arg3);
            tch->PointChange(POINT_SKILL, amount, true);
            newVal = tch->GetPoint(POINT_SKILL);
        } break;

        case 7: // alignment
        case 8: // alignment
        {
            PlayerAlignment amount = 0;
            str_to_number(amount, arg3);
            tch->UpdateAlignment(amount - tch->GetRealAlignment());
            newVal = tch->GetRealAlignment();
        } break;

        case 9: // skill tree points
        {
            PointValue amount = 0;
            str_to_number(amount, arg3);
            tch->PointChange(POINT_SKILLTREE_POINTS, amount, true);
            newVal = tch->GetPoint(POINT_SKILLTREE_POINTS);
        } break;
    }

    if (set_fields[i].type == NUMBER) {
        PointValue amount = 0;
        str_to_number(amount, arg3);
        ch->ChatPacket(CHAT_TYPE_INFO,
                       fmt::format("{}'s {} set to [{:G}]", tch->GetName(),
                                   set_fields[i].cmd, amount)
                           .c_str());
        ch->ChatPacket(CHAT_TYPE_INFO,
                       fmt::format("is now [{:G}]", newVal).c_str());
    }
}

ACMD(do_reset)
{
    ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
    ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
    ch->Save();
}

ACMD(do_advance)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: advance <name> <level>");
        return;
    }

    CHARACTER* tch = g_pCharManager->FindPC(arg1);

    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
        return;
    }

    int level = 0;
    str_to_number(level, arg2);

    tch->ResetPoint(std::clamp<PointValue>(level, 1, gConfig.maxLevel));
}

ACMD(do_respawn)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (*arg1 && !strcasecmp(arg1, "all")) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Respaw everywhere");
        regen_reset(0, 0);
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "Respaw around");
        regen_reset(ch->GetX(), ch->GetY());
    }
}

ACMD(do_safebox_size)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    int size = 0;

    if (*arg1)
        str_to_number(size, arg1);

    if (size > 3 || size < 0)
        size = 0;

    ch->ChatPacket(CHAT_TYPE_INFO, "Safebox size set to %d", size);
    ch->ChangeSafeboxSize(size);
}

ACMD(do_makeguild)
{
    if (ch->GetGuild())
        return;

    CGuildManager& gm = CGuildManager::instance();

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    TGuildCreateParameter cp = {};
    cp.master = ch;
    storm::CopyStringSafe(cp.name, arg1);

    if (!check_name(cp.name)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "적합하지 않은 길드 이름 입니다.");
        return;
    }

    gm.CreateGuild(cp);
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "(%s) 길드가 생성되었습니다. [임시]",
                       cp.name);
}

ACMD(do_deleteguild)
{
    if (ch->GetGuild())
        ch->GetGuild()->RequestDisband(ch->GetPlayerID());
}

ACMD(do_greset)
{
    if (ch->GetGuild())
        ch->GetGuild()->Reset();
}

// REFINE_ROD_HACK_BUG_FIX
ACMD(do_refine_rod)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    uint8_t cell = 0;
    str_to_number(cell, arg1);
    CItem* item = ch->GetInventoryItem(cell);
    if (item)
        fishing::RealRefineRod(ch, item);
}

// END_OF_REFINE_ROD_HACK_BUG_FIX

// REFINE_PICK
ACMD(do_refine_pick)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    uint8_t cell = 0;
    str_to_number(cell, arg1);
    CItem* item = ch->GetInventoryItem(cell);
    if (item) {
        mining::CHEAT_MAX_PICK(ch, item);
        mining::RealRefinePick(ch, item);
    }
}

ACMD(do_max_pick)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    uint8_t cell = 0;
    str_to_number(cell, arg1);
    CItem* item = ch->GetInventoryItem(cell);
    if (item) {
        mining::CHEAT_MAX_PICK(ch, item);
    }
}

// END_OF_REFINE_PICK

ACMD(do_fishing_simul)
{
    char arg1[256];
    char arg2[256];
    char arg3[256];
    argument = one_argument(argument, arg1, sizeof(arg1));
    two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));

    int count = 1000;
    int prob_idx = 0;
    int level = 100;

    ch->ChatPacket(CHAT_TYPE_INFO, "Usage: fishing_simul <level> <prob index> "
                                   "<count>");

    if (*arg1)
        str_to_number(level, arg1);

    if (*arg2)
        str_to_number(prob_idx, arg2);

    if (*arg3)
        str_to_number(count, arg3);

    fishing::Simulation(level, count, prob_idx, ch);
}

ACMD(do_invisibility)
{
    if (ch->FindAffect(AFFECT_INVISIBILITY))
        ch->RemoveAffect(AFFECT_INVISIBILITY);
    else
        ch->AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0,
                      INFINITE_AFFECT_DURATION, 0, true);
}

ACMD(do_event_flag)
{
    char arg1[256];
    char arg2[256];

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!(*arg1) || !(*arg2))
        return;

    int value = 0;
    str_to_number(value, arg2);

    if (!strcmp(arg1, "mob_item") || !strcmp(arg1, "mob_exp") ||
        !strcmp(arg1, "mob_gold") || !strcmp(arg1, "mob_dam") ||
        !strcmp(arg1, "mob_gold_pct") || !strcmp(arg1, "mob_item_buyer") ||
        !strcmp(arg1, "mob_exp_buyer") || !strcmp(arg1, "mob_gold_buyer") ||
        !strcmp(arg1, "mob_gold_pct_buyer"))
        value = std::clamp(value, 0, 1000);

    // quest::CQuestManager::instance().SetEventFlag(arg1, atoi(arg2));
    quest::CQuestManager::instance().RequestSetEventFlag(arg1, value);
    ch->ChatPacket(CHAT_TYPE_INFO, "RequestSetEventFlag %s %d", arg1, value);
    SPDLOG_INFO("RequestSetEventFlag %s %d", arg1, value);
}

ACMD(do_get_event_flag)
{
    // Filter
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    quest::CQuestManager::instance().SendEventFlagList(ch, arg1);
}

ACMD(do_private)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: private <map index>");
        return;
    }

    long lMapIndex;
    long map_index = 0;
    str_to_number(map_index, arg1);
    if ((lMapIndex = SECTREE_MANAGER::instance().CreatePrivateMap(map_index))) {
        ch->SaveExitLocation();

        SECTREE_MAP* pkSectreeMap =
            SECTREE_MANAGER::instance().GetMap(lMapIndex);
        ch->WarpSet(lMapIndex, pkSectreeMap->m_setting.posSpawn.x,
                    pkSectreeMap->m_setting.posSpawn.y);
    } else
        ch->ChatPacket(CHAT_TYPE_INFO, "Can't find map by index %d", map_index);
}

ACMD(do_qf)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    quest::PC* pPC =
        quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
    std::string questname = pPC->GetCurrentQuestName();

    if (!questname.empty()) {
        int value = quest::CQuestManager::Instance().GetQuestStateIndex(
            questname, arg1);

        pPC->SetFlag(questname + ".__status", value);
        pPC->ClearTimer();

        quest::PC::QuestInfoIterator it = pPC->quest_begin();
        unsigned int questindex =
            quest::CQuestManager::instance().GetQuestIndexByName(questname);

        while (it != pPC->quest_end()) {
            if (it->first == questindex) {
                it->second.st = value;
                break;
            }

            ++it;
        }

        ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d",
                       questname.c_str(), arg1, value);
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
    }
}

ACMD(do_book)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    CSkillProto* pkProto;

    if (isdigit(*arg1)) {
        uint32_t vnum = 0;
        str_to_number(vnum, arg1);
        pkProto = CSkillManager::instance().Get(vnum);
    } else
        pkProto = CSkillManager::instance().Get(arg1);

    if (!pkProto) {
        ch->ChatPacket(CHAT_TYPE_INFO, "There is no such a skill.");
        return;
    }

    CItem* item = ch->AutoGiveItem(50300);
    item->SetSocket(0, pkProto->dwVnum);
}

ACMD(do_setskillother)
{
    char arg1[256], arg2[256], arg3[256];
    argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
    one_argument(argument, arg3, sizeof(arg3));

    if (!*arg1 || !*arg2 || !*arg3 || !isdigit(*arg3)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskillother <target> "
                                       "<skillname> <lev>");
        return;
    }

    CHARACTER* tch;

    tch = g_pCharManager->FindPC(arg1);

    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
        return;
    }

#ifdef __NEW_GAMEMASTER_CONFIG__
    if (tch != ch &&
        !GM::check_allow(ch->GetGMLevel(), GM_ALLOW_MODIFY_OTHERS)) {
        ch->ChatPacket(CHAT_TYPE_INFO, ("You have no rights to modify other "
                                        "players."));
        return;
    }
#endif
    CSkillProto* pk;

    if (isdigit(*arg2)) {
        uint32_t vnum = 0;
        str_to_number(vnum, arg2);
        pk = CSkillManager::instance().Get(vnum);
    } else
        pk = CSkillManager::instance().Get(arg2);

    if (!pk) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
        return;
    }

    uint8_t level = 0;
    str_to_number(level, arg3);
    tch->SetSkillLevel(pk->dwVnum, level);
    tch->ComputePoints();
    tch->ComputeMountPoints();
    tch->SkillLevelPacket();
}

ACMD(do_setskill)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2 || !isdigit(*arg2)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskill <name> <lev>");
        return;
    }

    CSkillProto* pk;

    if (isdigit(*arg1)) {
        uint32_t vnum = 0;
        str_to_number(vnum, arg1);
        pk = CSkillManager::instance().Get(vnum);
    }

    else
        pk = CSkillManager::instance().Get(arg1);

    if (!pk) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
        return;
    }

    uint8_t level = 0;
    str_to_number(level, arg2);
    ch->SetSkillLevel(pk->dwVnum, level);
    ch->ComputePoints();
    ch->ComputeMountPoints();
    ch->SkillLevelPacket();
}

ACMD(do_setskillcolor)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2 || !isdigit(*arg2)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskillcolor <name|vnum> "
                                       "<color>");
        return;
    }

    CSkillProto* pk;

    if (isdigit(*arg1)) {
        uint32_t vnum = 0;
        str_to_number(vnum, arg1);
        pk = CSkillManager::instance().Get(vnum);
    }

    else
        pk = CSkillManager::instance().Get(arg1);

    if (!pk) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
        return;
    }

    uint32_t color = 0;
    str_to_number(color, arg2);
    ch->SetSkillColor(pk->dwVnum, color);
    ch->SkillLevelPacket();
}

ACMD(do_set_skill_point)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    int skill_point = 0;
    if (*arg1)
        str_to_number(skill_point, arg1);

    ch->SetPoint(POINT_SKILL, skill_point);
    ch->PointChange(POINT_SKILL, 0);
}

ACMD(do_set_skill_group)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    int skill_group = 0;
    if (*arg1)
        str_to_number(skill_group, arg1);

    ch->SetSkillGroup(skill_group);

    ch->ClearSkill();
    ch->ChatPacket(CHAT_TYPE_INFO, "skill group to %d.", skill_group);
}

ACMD(do_reload)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (*arg1) {
        switch (LOWER(*arg1)) {
            case 'p':
                ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
                db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);
                break;

            case 'q':
                ch->ChatPacket(CHAT_TYPE_INFO, "Reloading quest.");
                quest::CQuestManager::instance().Reload();
                break;

            case 'f':
                fishing::Initialize();
                break;

                // RELOAD_ADMIN
            case 'a':
                ch->ChatPacket(CHAT_TYPE_INFO, "Reloading Admin infomation.");

                TPacketReloadAdmin pack;
                strlcpy(pack.szIP, gConfig.gameIp.c_str(), sizeof(pack.szIP));

                db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, &pack,
                                        sizeof(TPacketReloadAdmin));
                SPDLOG_INFO("Reloading admin infomation.");
                break;
                // END_RELOAD_ADMIN
            case 'c': // cube
                // ·ÎÄÃ ÇÁ·Î¼¼½º¸¸ °»»êÇÑ´Ù.
                Cube_init();
                break;
            default:
                const int FILE_NAME_LEN = 256;
                if (strstr(arg1, "drop")) {
                    ch->ChatPacket(CHAT_TYPE_INFO, "Reloading: ETCDropItem: %s",
                                   "data/etc_drop_item.txt");
                    if (!ITEM_MANAGER::instance().ReadEtcDropItemFile(
                            "data/etc_drop_item.txt", true))
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "failed to reload ETCDropItem: %s",
                                       "data/etc_drop_item.txt");
                    else
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "reload success: ETCDropItem: %s",
                                       "data/etc_drop_item.txt");

                    ch->ChatPacket(CHAT_TYPE_INFO,
                                   "Reloading: SpecialItemGroup: %s",
                                   "data/special_item_group.txt");
                    if (!ITEM_MANAGER::instance().ReadSpecialDropItemFile(
                            "data/special_item_group.txt", true))
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "failed to reload SpecialItemGroup: %s",
                                       "data/special_item_group.txt");
                    else
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "reload success: SpecialItemGroup: %s",
                                       "data/special_item_group.txt");

                    ch->ChatPacket(CHAT_TYPE_INFO,
                                   "Reloading: MOBDropItemFile: %s",
                                   "data/mob_drop_item.txt");
                    if (!ITEM_MANAGER::instance().ReadMonsterDropItemGroup(
                            "data/mob_drop_item.txt", true))
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "failed to reload MOBDropItemFile: %s",
                                       "data/mob_drop_item.txt");
                    else
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "reload success: MOBDropItemFile: %s",
                                       "data/mob_drop_item.txt");
                } else if (strstr(arg1, "group")) {
                    ch->ChatPacket(CHAT_TYPE_INFO, "Reloading: mob groups: %s",
                                   "data/group.txt");
                    if (!CMobManager::instance().LoadGroup("data/group.txt",
                                                           true))
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "failed to reload mob groups: %s",
                                       "data/group.txt");

                    ch->ChatPacket(CHAT_TYPE_INFO,
                                   "Reloading: mob group group: %s",
                                   "data/group_group.txt");
                    if (!CMobManager::instance().LoadGroupGroup(
                            "data/group_group.txt", true))
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "failed to reload mob group group: %s",
                                       "data/group_group.txt");
                } else if (strstr(arg1, "regen")) {
                    SendNoticeMap("Reloading regens!", ch->GetMapIndex(),
                                  false);
                    regen_free_map(ch->GetMapIndex());
                    g_pCharManager->DestroyCharacterInMap(ch->GetMapIndex());
                    regen_reload(ch->GetMapIndex());
                    SendNoticeMap("Regens reloaded!", ch->GetMapIndex(), false);
                } else if (strstr(arg1, "shop")) {
                    CShopManager::instance().Reload();
                }

                break;
        }
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
        db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);
    }
}

ACMD(do_cooltime)
{
    ch->DisableCooltime();
}

ACMD(do_level)
{
    char arg2[256];
    one_argument(argument, arg2, sizeof(arg2));

    if (!*arg2) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: level <level>");
        return;
    }

    int level = 0;
    str_to_number(level, arg2);
    ch->ResetPoint(std::clamp<uint32_t>(level, 1, gConfig.maxLevel));

    ch->ClearSkill();
    ch->ClearSubSkill();
}

ACMD(do_gwlist)
{
    SendI18nChatPacket(ch, CHAT_TYPE_NOTICE, "현재 전쟁중인 길드 입니다");
    CGuildManager::instance().ShowGuildWarList(ch);
}

ACMD(do_stop_guild_war)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2)
        return;

    int id1 = 0, id2 = 0;

    str_to_number(id1, arg1);
    str_to_number(id2, arg2);

    if (!id1 || !id2)
        return;

    if (id1 > id2) {
        std::swap(id1, id2);
    }

    ch->ChatPacket(CHAT_TYPE_TALKING, "%d %d", id1, id2);
    CGuildManager::instance().RequestEndWar(id1, id2);
}

ACMD(do_cancel_guild_war)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    int id1 = 0, id2 = 0;
    str_to_number(id1, arg1);
    str_to_number(id2, arg2);

    if (id1 > id2)
        std::swap(id1, id2);

    CGuildManager::instance().RequestCancelWar(id1, id2);
}

ACMD(do_guild_state)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    CGuild* pGuild = CGuildManager::instance().FindGuildByName(arg1);
    if (pGuild != nullptr) {
        ch->ChatPacket(CHAT_TYPE_INFO, "GuildID: %d", pGuild->GetID());
        ch->ChatPacket(CHAT_TYPE_INFO, "GuildMasterPID: %d",
                       pGuild->GetMasterPID());
        ch->ChatPacket(CHAT_TYPE_INFO, "IsInWar: %d", pGuild->UnderAnyWar());
    } else {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%s: 존재하지 않는 길드 입니다.",
                           arg1);
    }
}

struct FuncDisableRegen {
    int _fDistance;
    PIXEL_POSITION _BasePos;
    bool _bAll;

    FuncDisableRegen(int fDistance, PIXEL_POSITION BasePos, bool bAll)
        : _fDistance(fDistance)
        , _BasePos(BasePos)
        , _bAll(bAll)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER* pkChr = (CHARACTER*)ent;

        float fDist = DISTANCE(_BasePos, pkChr->GetXYZ());
        if (fDist < _fDistance || _bAll) {
            if (pkChr->IsNPC()) {
                pkChr->CanRegenHP(false);
            }
        }
    }
};

struct FuncWeaken {
    int _fDistance;
    PIXEL_POSITION _BasePos;
    bool _bAll;

    FuncWeaken(int fDistance, PIXEL_POSITION BasePos, bool bAll)
        : _fDistance(fDistance)
        , _BasePos(BasePos)
        , _bAll(bAll)
    {
    }

    void operator()(CEntity* ent)
    {
        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER* pkChr = (CHARACTER*)ent;

        float fDist = DISTANCE(_BasePos, pkChr->GetXYZ());
        if (fDist < _fDistance || _bAll) {
            if (pkChr->IsNPC()) {
                pkChr->PointChange(POINT_HP, 1 - pkChr->GetHP());
                pkChr->CanRegenHP(false);
            }
        }
    }
};

ACMD(do_weaken)
{
    const char* szHelp = "/weak range/noregen <all/default:2000>";
    CHECK_COMMAND(Parser, 1, szHelp);

    int Params = Parser.GetTokenNum();
    CMDTOKEN("mob", Parser.GetTokenString(0).c_str())
    {
        int raceNum = 101;
        bool bAll = false;
        if (Params == 2 && SIIsNumber(Parser.GetTokenString(1).c_str())) {
            raceNum = Parser.GetTokenInt(1);
        }

        auto func = [raceNum](CEntity* ent) {
            if (!ent->IsType(ENTITY_CHARACTER))
                return;

            CHARACTER* pkChr = (CHARACTER*)ent;

            if (pkChr->GetRaceNum() == raceNum) {
                pkChr->PointChange(POINT_HP, 1 - pkChr->GetHP());
                pkChr->CanRegenHP(false);
            }
        };

        ch->ForEachSeen(func);
        ch->ChatPacket(CHAT_TYPE_INFO, "weaked %d in range %s", raceNum,
                       Parser.GetTokenString(1).c_str());
    }
    else CMDTOKEN("range", Parser.GetTokenString(0).c_str())
    {
        int Range = 2000;
        bool bAll = false;
        if (Params == 2 && SIIsNumber(Parser.GetTokenString(1).c_str())) {
            Range = Parser.GetTokenInt(1);
        } else if (Params == 2 &&
                   !strcmp(Parser.GetTokenString(1).c_str(), "all")) {
            bAll = true;
        }
        FuncWeaken func(Range, ch->GetXYZ(), bAll);

        ch->ForEachSeen(func);
        ch->ChatPacket(CHAT_TYPE_INFO, "weaked monsters in range %s",
                       Parser.GetTokenString(1).c_str());
    }
    else CMDTOKEN("noregen", Parser.GetTokenString(0).c_str())
    {
        int Range = 2000;
        bool bAll = false;
        if (Params == 2 && SIIsNumber(Parser.GetTokenString(1).c_str())) {
            Range = Parser.GetTokenInt(1);
        } else if (Params == 2 &&
                   !strcmp(Parser.GetTokenString(1).c_str(), "all")) {
            bAll = true;
        }

        FuncDisableRegen func(Range, ch->GetXYZ(), bAll);

        ch->ForEachSeen(func);
        ch->ChatPacket(CHAT_TYPE_INFO,
                       "disabled regen for monsters in range %s",
                       Parser.GetTokenString(1).c_str());
        return;
    }

    COMMANDINFO(szHelp);
}

ACMD(do_getqf)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    CHARACTER* tch;

    if (!*arg1)
        tch = ch;
    else {
        tch = g_pCharManager->FindPC(arg1);

        if (!tch) {
            ch->ChatPacket(CHAT_TYPE_INFO,
                           "There is no such '%s' character online.", arg1);
            return;
        }
    }

    quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

    if (pPC)
        pPC->SendFlagList(ch);
}

ACMD(do_getf)
{
    char arg1[256];
    char arg2[256];

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: getf <filter> [<player name>]");
        ch->ChatPacket(CHAT_TYPE_INFO, "Use the 'getqf' command to output a "
                                       "full list of quest flags.");
        return;
    }

    CHARACTER* tch;

    if (!*arg2)
        tch = ch;
    else {
        tch = g_pCharManager->FindPC(arg2);

        if (!tch) {
            ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
            return;
        }
    }

    quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

    if (pPC)
        pPC->SendFlagList(ch, arg1);
}

ACMD(do_set_state)
{
    char arg1[256];
    char arg2[256];
    char arg3[256];

    // argument = one_argument(argument, arg1, sizeof(arg1));
    one_argument(
        two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3,
        sizeof(arg3));

    if (!*arg1 || !*arg2) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: set_state <quest_name> <state "
                                       "name>");
        return;
    }

    quest::PC* pPC =
        quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
    std::string questname = arg1;
    std::string statename = arg2;

    CHARACTER* tch = ch;
    if (*arg3) {
        tch = g_pCharManager->FindPC(arg3);
        if (!tch) {
            if (ch)
                ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find pc by name %s.",
                               arg3);
            return;
        }
    } else if (!tch)
        return;

#ifdef __NEW_GAMEMASTER_CONFIG__
    if (tch != ch &&
        !GM::check_allow(ch->GetGMLevel(), GM_ALLOW_MODIFY_OTHERS)) {
        if (ch)
            ch->ChatPacket(CHAT_TYPE_INFO, ("You have no rights to modify "
                                            "other players."));
        return;
    }
#endif

    if (!questname.empty()) {
        int value = quest::CQuestManager::Instance().GetQuestStateIndex(
            questname, statename);

        pPC->SetFlag(questname + ".__status", value);
        pPC->ClearTimer();

        quest::PC::QuestInfoIterator it = pPC->quest_begin();
        unsigned int questindex =
            quest::CQuestManager::instance().GetQuestIndexByName(questname);

        while (it != pPC->quest_end()) {
            if (it->first == questindex) {
                it->second.st = value;
                break;
            }

            ++it;
        }

        if (ch)
            ch->ChatPacket(CHAT_TYPE_INFO,
                           "setting quest state flag %s %s %d of %s",
                           questname.c_str(), arg1, value, tch->GetName());
    } else {
        if (ch)
            ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
    }
}

ACMD(do_setqf)
{
    char arg1[256];
    char arg2[256];
    char arg3[256];

    one_argument(
        two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3,
        sizeof(arg3));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setqf <flagname> <value> "
                                       "[<character name>]");
        return;
    }

    CHARACTER* tch = ch;

    if (*arg3)
        tch = g_pCharManager->FindPC(arg3);

    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
        return;
    }

    quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

    if (pPC) {
        int value = 0;
        str_to_number(value, arg2);
        pPC->SetFlag(arg1, value);
        ch->ChatPacket(CHAT_TYPE_INFO, "Quest flag set: %s %d", arg1, value);
    }
}

ACMD(do_delqf)
{
    char arg1[256];
    char arg2[256];

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: delqf <flagname> [<character "
                                       "name>]");
        return;
    }

    CHARACTER* tch = ch;

    if (*arg2)
        tch = g_pCharManager->FindPC(arg2);

    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
        return;
    }

    quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

    if (pPC) {
        if (pPC->DeleteFlag(arg1))
            ch->ChatPacket(CHAT_TYPE_INFO, "Delete success.");
        else
            ch->ChatPacket(CHAT_TYPE_INFO, "Delete failed. Quest flag does not "
                                           "exist.");
    }
}

ACMD(do_forgetme)
{
    ch->ForgetMyAttacker();
}

ACMD(do_aggregate)
{
    ch->AggregateMonster(5000);
}

ACMD(do_attract_ranger)
{
    ch->AttractRanger();
}

ACMD(do_pull_monster)
{
    ch->PullMonster();
}

ACMD(do_polymorph)
{
    char arg1[256], arg2[256];

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
    if (*arg1) {
        if (is_positive_number(std::string(arg1))) {
            uint32_t dwVnum = 0;
            str_to_number(dwVnum, arg1);
            bool bMaintainStat = false;
            if (*arg2) {
                int value = 0;
                str_to_number(value, arg2);
                bMaintainStat = (value > 0);
            }

            ch->SetPolymorph(dwVnum, bMaintainStat);
        }
    }
}

ACMD(do_polymorph_item)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    if (*arg1) {
        uint32_t dwVnum = 0;
        str_to_number(dwVnum, arg1);

        CItem* item = ITEM_MANAGER::instance().CreateItem(70104, 1, 0, true);
        if (item) {
            item->SetSocket(0, dwVnum);
            int iEmptyPos = ch->GetEmptyInventory(item);

            if (iEmptyPos != -1) {
                item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
                LogManager::instance().ItemLog(ch, item, "GM", item->GetName());
            } else {
                M2_DESTROY_ITEM(item);
                ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
            }
        } else {
            ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.",
                           70104);
        }
        // ch->SetPolymorph(dwVnum, bMaintainStat);
    }
}

ACMD(do_priv_empire)
{
    char arg1[256] = {0};
    char arg2[256] = {0};
    char arg3[256] = {0};
    char arg4[256] = {0};
    int empire = 0;
    int type = 0;
    int value = 0;
    int duration = 0;

    const char* line =
        two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2)
        goto USAGE;

    if (!line)
        goto USAGE;

    two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));

    if (!*arg3 || !*arg4)
        goto USAGE;

    str_to_number(empire, arg1);
    str_to_number(type, arg2);
    str_to_number(value, arg3);
    value = std::clamp(value, 0, 1000);
    duration = parse_time_str(arg4);

    if (empire < 0 || 3 < empire)
        goto USAGE;

    if (type < 1 || 5 < type)
        goto USAGE;

    if (value < 0)
        goto USAGE;

    if (duration < 0)
        goto USAGE;

    SPDLOG_INFO("_give_empire_privileage(empire={0}, type={1}, value={2}, "
                "duration={3}) by command",
                empire, type, value, duration);
    CPrivManager::instance().RequestGiveEmpirePriv(empire, type, value,
                                                   duration);
    return;

USAGE:
    ch->ChatPacket(CHAT_TYPE_INFO, "usage : priv_empire <empire> <type> "
                                   "<value> <duration>");
    ch->ChatPacket(CHAT_TYPE_INFO, "  <empire>    0 - 3 (0==all)");
    ch->ChatPacket(CHAT_TYPE_INFO, "  <type>      1:item_drop, 2:gold_drop, "
                                   "3:gold10_drop, 4:exp, 5:double_loot");
    ch->ChatPacket(CHAT_TYPE_INFO, "  <value>     percent (double_loot 1 = "
                                   "active)");
    ch->ChatPacket(CHAT_TYPE_INFO, "  <duration>  ex: 60s or 4h or 10m");
}

ACMD(do_mount_test)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    if (*arg1) {
        uint32_t vnum = 0;
        str_to_number(vnum, arg1);
        ch->MountVnum(vnum);
        ch->ComputePoints();
        ch->ComputeMountPoints();
    }
}

ACMD(do_observer)
{
    ch->SetObserverMode(!ch->IsObserverMode());
}

ACMD(do_socket_item)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (*arg1) {
        uint32_t dwVnum = 0;
        str_to_number(dwVnum, arg1);

        int iSocketCount = 0;
        str_to_number(iSocketCount, arg2);

        if (!iSocketCount || iSocketCount >= ITEM_SOCKET_MAX_NUM)
            iSocketCount = 3;

        if (!dwVnum) {
            if (!ITEM_MANAGER::instance().GetVnum(arg1, dwVnum)) {
                ch->ChatPacket(CHAT_TYPE_INFO,
                               "#%d item not exist by that vnum.", dwVnum);
                return;
            }
        }

        CItem* item = ch->AutoGiveItem(dwVnum);

        if (item) {
            for (int i = 0; i < iSocketCount; ++i)
                item->SetSocket(i, 1);
        } else {
            ch->ChatPacket(CHAT_TYPE_INFO, "#%d cannot create item.", dwVnum);
        }
    }
}

ACMD(do_shop_refund)
{
    std::unique_ptr<SQLMsg> pmsg(
        DBManager::instance().DirectQuery("SELECT owner_id, vnum, count, pos, "
                                          "socket0, socket1, socket2, socket3, "
                                          "socket4, socket5,"
                                          "attrtype0, attrvalue0,"
                                          "attrtype1, attrvalue1,"
                                          "attrtype2, attrvalue2,"
                                          "attrtype3, attrvalue3,"
                                          "attrtype4, attrvalue4,"
                                          "attrtype5, attrvalue5,"
                                          "attrtype6, attrvalue6, price FROM "
                                          "item WHERE owner_id = {} AND window "
                                          "= 'SHOP'",
                                          ch->GetPlayerID()));

    std::vector<TPlayerItem> items;
    items.reserve(pmsg->Get()->uiNumRows);

    for (uint i = 0; i < pmsg->Get()->uiNumRows; ++i) {
        MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
        TPlayerItem t;
        int32_t col = 0;

        str_to_number(t.owner, row[col]);
        str_to_number(t.data.vnum, row[++col]);
        str_to_number(t.data.count, row[++col]);
        str_to_number(t.pos, row[++col]);

        for (int j = 0; j < ITEM_SOCKET_MAX_NUM; ++j) {
            str_to_number(t.data.sockets[j], row[++col]);
        }

        for (int j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; ++j) {
            str_to_number(t.data.attrs[j].bType, row[++col]);
            str_to_number(t.data.attrs[j].sValue, row[++col]);
        }

        str_to_number(t.price, row[++col]);
        items.push_back(t);
    }

    std::unique_ptr<SQLMsg> pmsg2(
        DBManager::instance().DirectQuery("DELETE FROM item WHERE owner_id = "
                                          "{} AND window = 'SHOP';",
                                          ch->GetPlayerID()));
    if (pmsg2 && pmsg2->Get()->uiAffectedRows > 0) {
        for (const auto item : items) {
            auto newItem = ITEM_MANAGER::instance().CreateItem(item.data.vnum,
                                                               item.data.count);
            if (newItem) {
                newItem->SetSockets(item.data.sockets);
                newItem->SetAttributes(item.data.attrs);
                ch->AutoGiveItem(newItem);
            }
        }
    }
}

ACMD(do_xmas)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    int flag = 0;

    if (*arg1)
        str_to_number(flag, arg1);

    switch (subcmd) {
        case SCMD_XMAS_SNOW:
            quest::CQuestManager::instance().RequestSetEventFlag("xmas_snow",
                                                                 flag);
            break;

        case SCMD_XMAS_BOOM:
            quest::CQuestManager::instance().RequestSetEventFlag("xmas_boom",
                                                                 flag);
            break;

        case SCMD_XMAS_SANTA:
            quest::CQuestManager::instance().RequestSetEventFlag("xmas_santa",
                                                                 flag);
            break;
    }
}

// BLOCK_CHAT
ACMD(do_block_chat_list)
{
    if (!ch)
        return;

    DBManager::instance().ReturnQuery(QID_BLOCK_CHAT_LIST, ch->GetPlayerID(),
                                      nullptr,
                                      "SELECT p.name, a.lDuration FROM affect "
                                      "as a, player as p WHERE a.bType = {} "
                                      "AND a.dwPID = p.id",
                                      AFFECT_BLOCK_CHAT);
}

ACMD(do_vote_block_chat)
{
    return;
}

ACMD(do_suspect_fishbot)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    uint32_t pid;
    if (!*arg1 || !str_to_number(pid, arg1)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: suspect_fishbot <pid>");
        return;
    }

    CHARACTER* tch = g_pCharManager->FindByPID(pid);
    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO,
                       "There's no player nearby you going by pid %lu.", pid);
        return;
    }
}

ACMD(do_block_chat)
{
    char arg1[256];
    argument = one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        if (ch)
            ch->ChatPacket(CHAT_TYPE_INFO, "Usage: block_chat <name> <time> (0 "
                                           "to off)");

        return;
    }

    const char* name = arg1;
    int32_t lBlockDuration = parse_time_str(argument);

    if (lBlockDuration < 0) {
        if (ch) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "잘못된 형식의 시간입니다. h, m, s를 붙여서 "
                               "지정해 주십시오.");
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "예) 10s, 10m, 1m 30s");
        }
        return;
    }

    if (lBlockDuration <= 0 && ch->GetGMLevel() < GM_HIGH_WIZARD) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You are not allowed to lift chat bans");
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "ask a high ranking team member.");
        return;
    }

    if (GM::get_level(name) > ch->GetGMLevel()) {
        ch->ChatPacket(CHAT_TYPE_INFO, "You cannot dc someone that outranks "
                                       "you.");
        return;
    }

    SPDLOG_INFO("BLOCK CHAT {0} {1}", name, lBlockDuration);

    const auto tch = g_pCharManager->FindPC(name);
    if (!tch) {
        const auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(name);
        if (op) {
            GmBlockChatPacket p;
            p.pid = op->pid;
            p.duration = lBlockDuration;
            DESC_MANAGER::instance().GetMasterSocket()->Send(kGmBlockChat, p);
        } else {
            TPacketBlockChat p;
            strlcpy(p.szName, name, sizeof(p.szName));
            p.lDuration = lBlockDuration;
            db_clientdesc->DBPacket(HEADER_GD_BLOCK_CHAT,
                                    ch ? ch->GetDesc()->GetHandle() : 0, &p,
                                    sizeof(p));
        }

        if (ch)
            ch->ChatPacket(CHAT_TYPE_INFO, "Chat block requested.");

        return;
    }

    if (tch && ch != tch) {
        if (lBlockDuration == 0) {
            tch->RemoveAffect(AFFECT_BLOCK_CHAT);
        } else {
            tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, lBlockDuration, 0,
                           false);
            SendI18nChatPacket(tch, CHAT_TYPE_INFO,
                               "Chat has been banned on the operator form.");
        }
    }
}

// END_OF_BLOCK_CHAT

// BUILD_BUILDING
ACMD(do_build)
{
    using namespace building;

    char arg1[256], arg2[256], arg3[256], arg4[256];
    const char* line = one_argument(argument, arg1, sizeof(arg1));
    uint8_t GMLevel = ch->GetGMLevel();

    CLand* pkLand = CManager::instance().FindLand(ch->GetMapIndex(), ch->GetX(),
                                                  ch->GetY());

    // NOTE: 조건 체크들은 클라이언트와 서버가 함께 하기 때문에 문제가 있을 때는
    //       메세지를 전송하지 않고 에러를 출력한다.
    if (!pkLand) {
        SPDLOG_ERROR("{0} trying to build on not buildable area.",
                     ch->GetName());
        return;
    }

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax: no command");
        return;
    }

    // 건설 권한 체크
    if (GMLevel == GM_PLAYER) {
        // 플레이어가 집을 지을 때는 땅이 내껀지 확인해야 한다.
        if ((!ch->GetGuild() ||
             ch->GetGuild()->GetID() != pkLand->GetOwner())) {
            SPDLOG_ERROR("{0} trying to build on not owned land.",
                         ch->GetName());
            return;
        }

        // 내가 길마인가?
        if (ch->GetGuild()->GetMasterPID() != ch->GetPlayerID()) {
            SPDLOG_ERROR("{0} trying to build while not the guild master.",
                         ch->GetName());
            return;
        }
    }

    switch (LOWER(*arg1)) {
        case 'c': {
            // /build c vnum x y x_rot y_rot z_rot
            char arg5[256], arg6[256];
            line = one_argument(
                two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2)),
                arg3, sizeof(arg3)); // vnum x y
            one_argument(
                two_arguments(line, arg4, sizeof(arg4), arg5, sizeof(arg5)),
                arg6,
                sizeof(arg6)); // x_rot y_rot z_rot

            if (!*arg1 || !*arg2 || !*arg3 || !*arg4 || !*arg5 || !*arg6) {
                ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
                return;
            }

            uint32_t dwVnum = 0;
            str_to_number(dwVnum, arg1);

            using namespace building;

            const auto t = CManager::instance().GetObjectProto(dwVnum);
            if (!t) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "존재하지 않는 건물입니다.");
                return;
            }

            const uint32_t BUILDING_MAX_PRICE = 100000000;

            if (t->groupVnum) {
                if (pkLand->FindObjectByGroup(t->groupVnum)) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "같이 지을 수 없는 종류의 건물이 지어져 "
                                       "있습니다.");
                    return;
                }
            }

            // 건물 종속성 체크 (이 건물이 지어져 있어야함)
            if (t->dependOnGroupVnum) {
                //		const TObjectProto * dependent =
                // CManager::instance().GetObjectProto(dwVnum); 		if
                // (dependent)
                {
                    // 지어져있는가?
                    if (!pkLand->FindObjectByGroup(t->dependOnGroupVnum)) {
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "건설에 필요한 건물이 지어져 있지 "
                                           "않습니다.");
                        return;
                    }
                }
            }

            if (t->price > BUILDING_MAX_PRICE) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "건물 비용 정보 이상으로 건설 작업에 "
                                   "실패했습니다.");
                return;
            }

            if (ch->GetGold() < t->price) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "건설 비용이 부족합니다.");
                return;
            }

            // 아이템 자재 개수 체크

            int i;
            for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i) {
                uint32_t dwItemVnum = t->materials[i].itemVnum;
                uint32_t dwItemCount = t->materials[i].count;

                if (dwItemVnum == 0)
                    break;

                if ((int)dwItemCount > ch->CountSpecifyItem(dwItemVnum)) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "자재가 부족하여 건설할 수 없습니다.");
                    return;
                }
            }

            float x_rot = atof(arg4);
            float y_rot = atof(arg5);
            float z_rot = atof(arg6);
            // 20050811.myevan.건물 회전 기능 봉인 해제
            /*
               if (x_rot != 0.0f || y_rot != 0.0f || z_rot != 0.0f)
               {
               ch->ChatPacket(CHAT_TYPE_INFO, "건물 회전 기능은 아직 제공되지
               않습니다"); return;
               }
             */

            int32_t map_x = 0;
            str_to_number(map_x, arg2);
            int32_t map_y = 0;
            str_to_number(map_y, arg3);

            bool isSuccess =
                pkLand->RequestCreateObject(dwVnum, ch->GetMapIndex(), map_x,
                                            map_y, x_rot, y_rot, z_rot, true);

            if (!isSuccess) {
                if (gConfig.testServer)
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "건물을 지을 수 없는 위치입니다.");
                return;
            }

            ch->ChangeGold(-t->price);

            // 아이템 자재 사용하기
            {
                int i;
                for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i) {
                    uint32_t dwItemVnum = t->materials[i].itemVnum;
                    uint32_t dwItemCount = t->materials[i].count;

                    if (dwItemVnum == 0)
                        break;

                    SPDLOG_INFO("BUILD: material {0} {1} {2}", i, dwItemVnum,
                                dwItemCount);
                    ch->RemoveSpecifyItem(dwItemVnum, dwItemCount);
                }
            }
        } break;

        case 'u': {
            // /build u vid x y x_rot y_rot z_rot
            char arg5[256], arg6[256];
            line = one_argument(
                two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2)),
                arg3, sizeof(arg3)); // vnum x y
            one_argument(
                two_arguments(line, arg4, sizeof(arg4), arg5, sizeof(arg5)),
                arg6,
                sizeof(arg6)); // x_rot y_rot z_rot

            if (!*arg1 || !*arg2 || !*arg3 || !*arg4 || !*arg5 || !*arg6) {
                ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
                return;
            }

            if (ch->GetGold() < 1000000) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "건설 비용이 부족합니다.");
                return;
            }

            uint32_t dwVid = 0;
            str_to_number(dwVid, arg1);

            using namespace building;

            auto object = pkLand->FindObjectByVID(dwVid);
            if (!object) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "The object you want to update does not "
                                   "exist.");
                return;
            }

            float x_rot = atof(arg4);
            float y_rot = atof(arg5);
            float z_rot = atof(arg6);

            int32_t map_x = 0;
            str_to_number(map_x, arg2);
            int32_t map_y = 0;
            str_to_number(map_y, arg3);

            bool isSuccess = pkLand->RequestUpdateObject(
                object->GetID(), ch->GetMapIndex(), map_x, map_y, x_rot, y_rot,
                z_rot, true);

            if (!isSuccess) {
                if (gConfig.testServer)
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "건물을 지을 수 없는 위치입니다.");
                return;
            }

            ch->ChangeGold(-1000000);
        } break;

        case 'd':
            // build (d)elete ObjectID
            {
                one_argument(line, arg1, sizeof(arg1));

                if (!*arg1) {
                    ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
                    return;
                }

                uint32_t vid = 0;
                str_to_number(vid, arg1);
                pkLand->RequestDeleteObjectByVID(vid);
            }
            break;

            // BUILD_WALL

            // build w n/e/w/s
        case 'w':
            if (GMLevel > GM_PLAYER) {
                int mapIndex = ch->GetMapIndex();

                one_argument(line, arg1, sizeof(arg1));

                SPDLOG_INFO("guild.wall.build map[{0}] direction[{1}]",
                            mapIndex, arg1);

                switch (arg1[0]) {
                    case 's':
                        pkLand->RequestCreateWall(mapIndex, 0.0f);
                        break;
                    case 'n':
                        pkLand->RequestCreateWall(mapIndex, 180.0f);
                        break;
                    case 'e':
                        pkLand->RequestCreateWall(mapIndex, 90.0f);
                        break;
                    case 'w':
                        pkLand->RequestCreateWall(mapIndex, 270.0f);
                        break;
                    default:
                        ch->ChatPacket(CHAT_TYPE_INFO,
                                       "guild.wall.build unknown_direction[%s]",
                                       arg1);
                        SPDLOG_ERROR("guild.wall.build unknown_direction[{0}]",
                                     arg1);
                        break;
                }
            }
            break;

        case 'e':
            if (GMLevel > GM_PLAYER) {
                pkLand->RequestDeleteWall();
            }
            break;

        case 'W':
            // 담장 세우기
            // build (w)all 담장번호 담장크기 대문동 대문서 대문남 대문북

            if (GMLevel > GM_PLAYER) {
                int setID = 0, wallSize = 0;
                char arg5[256], arg6[256];
                line =
                    two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2));
                line =
                    two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));
                two_arguments(line, arg5, sizeof(arg5), arg6, sizeof(arg6));

                str_to_number(setID, arg1);
                str_to_number(wallSize, arg2);

                if (setID != 14105 && setID != 14115 && setID != 14125) {
                    SPDLOG_INFO("BUILD_WALL: wrong wall set id {0}", setID);
                    break;
                } else {
                    bool door_east = false;
                    str_to_number(door_east, arg3);
                    bool door_west = false;
                    str_to_number(door_west, arg4);
                    bool door_south = false;
                    str_to_number(door_south, arg5);
                    bool door_north = false;
                    str_to_number(door_north, arg6);
                    pkLand->RequestCreateWallBlocks(
                        setID, ch->GetMapIndex(), wallSize, door_east,
                        door_west, door_south, door_north);
                }
            }
            break;

        case 'E':
            // 담장 지우기
            // build (e)rase 담장셋ID
            if (GMLevel > GM_PLAYER) {
                one_argument(line, arg1, sizeof(arg1));
                uint32_t id = 0;
                str_to_number(id, arg1);
                pkLand->RequestDeleteWallBlocks(id);
            }
            break;

        default:
            ch->ChatPacket(CHAT_TYPE_INFO, "Invalid command %s", arg1);
            break;
    }
}

// END_OF_BUILD_BUILDING

ACMD(do_clear_quest)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    quest::PC* pPC =
        quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
    pPC->ClearQuest(arg1);
}

ACMD(do_horse_state)
{
    ch->ChatPacket(CHAT_TYPE_INFO, "Horse Information:");
    ch->ChatPacket(CHAT_TYPE_INFO, "    Level  %d", ch->GetHorseLevel());
    ch->ChatPacket(CHAT_TYPE_INFO, "    Health %d/%d (%d%%)",
                   ch->GetHorseHealth(), ch->GetHorseMaxHealth(),
                   ch->GetHorseHealth() * 100 / ch->GetHorseMaxHealth());
    ch->ChatPacket(CHAT_TYPE_INFO, "    Stam   %d/%d (%d%%)",
                   ch->GetHorseStamina(), ch->GetHorseMaxStamina(),
                   ch->GetHorseStamina() * 100 / ch->GetHorseMaxStamina());
}

ACMD(do_horse_level)
{
    const char* szHelp = "Usage: /horse_level <name> <level>";
    CHECK_COMMAND(Parser, 2, szHelp);

    auto name = Parser.GetTokenString(0);
    auto level = Parser.GetTokenInt(1);

    CHARACTER* victim;
    victim = g_pCharManager->FindPC(name);

    if (nullptr == victim) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "존재하지 않는 캐릭터 입니다.");
        return;
    }

#ifdef __NEW_GAMEMASTER_CONFIG__
    if (victim != ch &&
        !GM::check_allow(ch->GetGMLevel(), GM_ALLOW_MODIFY_OTHERS)) {
        if (ch)
            ch->ChatPacket(CHAT_TYPE_INFO, ("You have no rights to modify "
                                            "other players."));
        return;
    }
#endif

    level = std::clamp(level, 0, HORSE_MAX_LEVEL);

    ch->ChatPacket(CHAT_TYPE_INFO, "horse level set (%s: %d)",
                   victim->GetName(), level);

    victim->SetHorseLevel(level);
    victim->ComputePoints();
    victim->ComputeMountPoints();
    victim->SkillLevelPacket();
    return;
}

ACMD(do_horse_ride)
{
    if (!ch->HasToggleMount()) {
        if (ch->IsHorseRiding())
            ch->StopRiding();
        else
            ch->StartRiding();
    }
}

ACMD(do_horse_summon)
{
    ch->HorseSummon(true, true);
}

ACMD(do_horse_unsummon)
{
    ch->HorseSummon(false, true);
}

ACMD(do_horse_set_stat)
{
    char arg1[256], arg2[256];

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (*arg1 && *arg2) {
        int hp = 0;
        str_to_number(hp, arg1);
        int stam = 0;
        str_to_number(stam, arg2);
        ch->UpdateHorseHealth(hp - ch->GetHorseHealth());
        ch->UpdateHorseStamina(stam - ch->GetHorseStamina());
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage : /horse_set_stat <hp> "
                                       "<stamina>");
    }
}

ACMD(do_save_attribute_to_image) // command "/saveati" for alias
{
    char szFileName[256];
    char szMapIndex[256];

    two_arguments(argument, szMapIndex, sizeof(szMapIndex), szFileName,
                  sizeof(szFileName));

    if (!*szMapIndex || !*szFileName) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /saveati <map_index> "
                                       "<filename>");
        return;
    }

    long lMapIndex = 0;
    str_to_number(lMapIndex, szMapIndex);

    /*if (SECTREE_MANAGER::instance().SaveAttributeToImage(lMapIndex,
    szFileName)) ch->ChatPacket(CHAT_TYPE_INFO, "Save done."); else
        ch->ChatPacket(CHAT_TYPE_INFO, "Save failed.");*/
}

ACMD(do_change_attr)
{
    CItem* weapon = ch->GetWear(WEAR_WEAPON);
    if (weapon)
        weapon->ChangeAttribute();
}

ACMD(do_add_attr)
{
    CItem* weapon = ch->GetWear(WEAR_WEAPON);
    if (weapon)
        weapon->AddAttribute();
}

ACMD(do_add_socket)
{
    CItem* weapon = ch->GetWear(WEAR_WEAPON);
    if (weapon)
        weapon->AddSocket();
}

ACMD(do_show_arena_list)
{
    CArenaManager::instance().SendArenaMapListTo(ch);
}

ACMD(do_end_all_duel)
{
    CArenaManager::instance().EndAllDuel();
}

ACMD(do_end_duel)
{
    char szName[256];

    one_argument(argument, szName, sizeof(szName));

    CHARACTER* pChar = g_pCharManager->FindPC(szName);
    if (pChar == nullptr) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "존재하지 않는 캐릭터 입니다.");
        return;
    }

    if (CArenaManager::instance().EndDuel(pChar->GetPlayerID()) == false) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "대련 강제 종료 실패");
    } else {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "대련 강제 종료 성공");
    }
}

ACMD(do_duel)
{
    char szName1[256];
    char szName2[256];
    char szSet[256];
    char szMinute[256];
    char szMap[256];

    int set = 0;
    int minute = 0;
    uint32_t map = 0;

    argument = two_arguments(argument, szName1, sizeof(szName1), szName2,
                             sizeof(szName2));
    auto line = two_arguments(argument, szSet, sizeof(szSet), szMinute,
                              sizeof(szMinute));
    one_argument(line, szMap, sizeof(szMap));

    str_to_number(set, szSet);

    if (set < 0)
        set = 1;
    if (set > 5)
        set = 5;

    if (!str_to_number(minute, szMinute))
        minute = 5;

    if (minute < 5)
        minute = 5;

    if (!str_to_number(map, szMap))
        map = 0;

    CHARACTER* pChar1 = g_pCharManager->FindPC(szName1);
    CHARACTER* pChar2 = g_pCharManager->FindPC(szName2);

    if (pChar1 != nullptr && pChar2 != nullptr) {
        pChar1->RemoveGoodAffect();
        pChar2->RemoveGoodAffect();

        pChar1->RemoveBadAffect();
        pChar2->RemoveBadAffect();

        auto pParty = pChar1->GetParty();
        if (pParty != nullptr) {
            if (pParty->GetMemberCount() == 2) {
                CPartyManager::instance().DeleteParty(pParty);
            } else {
                SendI18nChatPacket(pChar1, CHAT_TYPE_INFO,
                                   "<파티> 파티에서 나가셨습니다.");
                pParty->Quit(pChar1->GetPlayerID());
            }
        }

        pParty = pChar2->GetParty();
        if (pParty != nullptr) {
            if (pParty->GetMemberCount() == 2) {
                CPartyManager::instance().DeleteParty(pParty);
            } else {
                SendI18nChatPacket(pChar2, CHAT_TYPE_INFO,
                                   "<파티> 파티에서 나가셨습니다.");
                pParty->Quit(pChar2->GetPlayerID());
            }
        }

        if (CArenaManager::instance().StartDuel(pChar1, pChar2, set, minute,
                                                map) == true) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "대련이 성공적으로 시작 되었습니다.");
        } else {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "대련 시작에 문제가 있습니다.");
        }
    } else {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "대련자가 없습니다.");
    }
}

ACMD(do_stat_plus_amount)
{
    char szPoint[256];

    one_argument(argument, szPoint, sizeof(szPoint));

    if (*szPoint == '\0')
        return;

    if (ch->IsPolymorphed()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "둔갑 중에는 능력을 올릴 수 없습니다.");
        return;
    }

    int nRemainPoint = ch->GetPoint(POINT_STAT);

    if (nRemainPoint <= 0) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "남은 스탯 포인트가 없습니다.");
        return;
    }

    PointValue nPoint = 0;
    str_to_number(nPoint, szPoint);

    if (nRemainPoint < nPoint) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "남은 스탯 포인트가 적습니다.");
        return;
    }

    if (nPoint < 0) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "값을 잘못 입력하였습니다.");
        return;
    }

    switch (subcmd) {
        case POINT_HT: // Ã¼·Â
            if (nPoint + ch->GetRealPoint(POINT_HT) > gConfig.maxStat) {
                nPoint = gConfig.maxStat - ch->GetRealPoint(POINT_HT);
            }
            break;

        case POINT_IQ: // Áö´É
            if (nPoint + ch->GetRealPoint(POINT_IQ) > gConfig.maxStat) {
                nPoint = gConfig.maxStat - ch->GetRealPoint(POINT_IQ);
            }
            break;

        case POINT_ST: // ±Ù·Â
            if (nPoint + ch->GetRealPoint(POINT_ST) > gConfig.maxStat) {
                nPoint = gConfig.maxStat - ch->GetRealPoint(POINT_ST);
            }
            break;

        case POINT_DX: // ¹ÎÃ¸
            if (nPoint + ch->GetRealPoint(POINT_DX) > gConfig.maxStat) {
                nPoint = gConfig.maxStat - ch->GetRealPoint(POINT_DX);
            }
            break;

        default:
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "명령어의 서브 커맨드가 잘못 되었습니다.");
            return;
            break;
    }

    if (nPoint > 0) {
        ch->SetRealPoint(subcmd, ch->GetRealPoint(subcmd) + nPoint);
        ch->ComputePoints();
        ch->ComputeMountPoints();
        ch->PointChange(subcmd, 0);

        if (subcmd == POINT_IQ)
            ch->PointChange(POINT_MAX_HP, 0);
        else if (subcmd == POINT_HT)
            ch->PointChange(POINT_MAX_SP, 0);

        ch->PointChange(POINT_STAT, -nPoint);
        ch->PointsPacket();
    }
}

struct tTwoPID {
    int pid1;
    int pid2;
};

ACMD(do_break_marriage)
{
    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    tTwoPID pids = {0, 0};

    str_to_number(pids.pid1, arg1);
    str_to_number(pids.pid2, arg2);

    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                       "플레이어 %d 와 플레이어  %d를 파혼시킵니다..",
                       pids.pid1, pids.pid2);
    db_clientdesc->DBPacket(HEADER_GD_BREAK_MARRIAGE, 0, &pids, sizeof(pids));
}

ACMD(do_effect)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    int effect_type = 0;
    str_to_number(effect_type, arg1);
    ch->EffectPacket(effect_type);
}

struct FCountInMap {
    int32_t m_Count[4]{};

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            if (ch && ch->IsPC())
                ++m_Count[ch->GetEmpire()];
        }
    }

    int GetCount(uint8_t bEmpire) { return m_Count[bEmpire]; }
};

ACMD(do_threeway_war_info)
{
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "각제국 진행 정보");
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "선택 맵 정보 성지 %d 통로 %d %d %d",
                       GetSungziMapIndex(), GetPassMapIndex(1),
                       GetPassMapIndex(2), GetPassMapIndex(3));
    ch->ChatPacket(CHAT_TYPE_INFO, "ThreewayPhase %d",
                   g_pThreeWayWar->GetRegenFlag());

    for (int n = 1; n < 4; ++n) {
        SECTREE_MAP* pSecMap =
            SECTREE_MANAGER::instance().GetMap(GetSungziMapIndex());

        FCountInMap c;

        if (pSecMap) {
            pSecMap->for_each(c);
        }

        ch->ChatPacket(CHAT_TYPE_INFO, "%s killscore %d usercount %d",
                       EMPIRE_NAME(n), g_pThreeWayWar->GetKillScore(n),
                       c.GetCount(n));
    }
}

ACMD(do_threeway_war_myinfo)
{
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "나의 삼거리 진행정보");
    ch->ChatPacket(CHAT_TYPE_INFO, "Deadcount %d",
                   g_pThreeWayWar->GetReviveTokenForPlayer(ch->GetPlayerID()));
}

ACMD(do_reset_subskill)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: reset_subskill <name>");
        return;
    }

    CHARACTER* tch = g_pCharManager->FindPC(arg1);

    if (tch == nullptr)
        return;

    tch->ClearSubSkill();
    ch->ChatPacket(CHAT_TYPE_INFO, "Subskill of [%s] was reset",
                   tch->GetName());
}

ACMD(do_flush)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (0 == arg1[0]) {
        ch->ChatPacket(CHAT_TYPE_INFO, "usage : /flush player_id");
        return;
    }

    uint32_t pid = (uint32_t)strtoul(arg1, nullptr, 10);

    db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
    db_clientdesc->Packet(&pid, sizeof(uint32_t));

    ch->ChatPacket(CHAT_TYPE_INFO, "Sent flush request for #%lu.", pid);
}

ACMD(do_eclipse)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (strtol(arg1, nullptr, 10) == 1) {
        quest::CQuestManager::instance().RequestSetEventFlag("eclipse", 1);
    } else {
        quest::CQuestManager::instance().RequestSetEventFlag("eclipse", 0);
    }
}

ACMD(do_event_helper)
{
    char arg1[256];
    int mode = 0;

    one_argument(argument, arg1, sizeof(arg1));
    str_to_number(mode, arg1);

    if (mode == 1) {
        xmas::SpawnEventHelper(true);
        ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Spawn");
    } else {
        xmas::SpawnEventHelper(false);
        ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Delete");
    }
}

struct FMobCounter {
    int nCount;
    int nStoneCount;
    uint32_t iSpecificVnum;

    FMobCounter(uint32_t specificVnum)
    {
        iSpecificVnum = specificVnum;
        nCount = 0;
        nStoneCount = 0;
    }

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* pChar = static_cast<CHARACTER*>(ent);

            if (iSpecificVnum) {
                if (pChar->GetRaceNum() == iSpecificVnum)
                    nCount++;

                return;
            }

            if (pChar->IsMonster()) {
                nCount++;
            }

            if (pChar->IsStone()) {
                nStoneCount++;
            }
        }
    }
};

ACMD(do_get_mob_count)
{
    char arg1[50];

    one_argument(argument, arg1, sizeof(arg1));

    uint32_t specificVnum = 0;
    str_to_number(specificVnum, arg1);

    SECTREE_MAP* pSectree =
        SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());

    if (pSectree == nullptr)
        return;

    FMobCounter f(specificVnum);
    pSectree->for_each(f);

    if (specificVnum) {
        ch->ChatPacket(CHAT_TYPE_INFO, "MapIndex: %d - Count of %lu: %d",
                       ch->GetMapIndex(), specificVnum, f.nCount);
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO,
                       "MapIndex: %d - Mob count: %d, Stone count: %d",
                       ch->GetMapIndex(), f.nCount, f.nStoneCount);
    }
}

ACMD(do_clear_land)
{
    const building::CLand* pLand = building::CManager::instance().FindLand(
        ch->GetMapIndex(), ch->GetX(), ch->GetY());

    if (nullptr == pLand) {
        return;
    }

    ch->ChatPacket(CHAT_TYPE_INFO, "Guild Land(%d) Cleared", pLand->GetID());

    building::CManager::instance().ClearLand(pLand->GetID());
}

ACMD(do_special_item) {}

ACMD(do_set_stat)
{
    char szName[256];
    char szChangeAmount[256];

    two_arguments(argument, szName, sizeof(szName), szChangeAmount,
                  sizeof(szChangeAmount));

    if (szName[0] == '\0' || szChangeAmount[0] == '\0') {
        ch->ChatPacket(CHAT_TYPE_INFO, "usage: <tcon|tint|tstr|tdex> <name> "
                                       "<stat points>");
        return;
    }

    const auto tch = g_pCharManager->FindPC(szName);
    if (!tch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find player(%s)", szName);
        return;
    } else {
#ifdef __NEW_GAMEMASTER_CONFIG__
        if (tch != ch &&
            !GM::check_allow(ch->GetGMLevel(), GM_ALLOW_MODIFY_OTHERS)) {
            ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have no rights to "
                                                   "modify other players."));
            return;
        }
#endif
        if (tch->IsPolymorphed()) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "둔갑 중에는 능력을 올릴 수 없습니다.");
            return;
        }

        if (subcmd != POINT_HT && subcmd != POINT_IQ && subcmd != POINT_ST &&
            subcmd != POINT_DX) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "명령어의 서브 커맨드가 잘못 되었습니다.");
            return;
        }
        int nRemainPoint = tch->GetPoint(POINT_STAT);
        int nCurPoint = tch->GetRealPoint(subcmd);
        int nChangeAmount = 0;
        str_to_number(nChangeAmount, szChangeAmount);
        int nPoint = nCurPoint + nChangeAmount;

        int n = 0;
        switch (subcmd) {
            case POINT_HT:
                if (nPoint < JobInitialPoints[tch->GetJob()].ht) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "Cannot set stat under initial stat.");
                    return;
                }
                n = 0;
                break;
            case POINT_IQ:
                if (nPoint < JobInitialPoints[tch->GetJob()].iq) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "Cannot set stat under initial stat.");
                    return;
                }
                n = 1;
                break;
            case POINT_ST:
                if (nPoint < JobInitialPoints[tch->GetJob()].st) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "Cannot set stat under initial stat.");
                    return;
                }
                n = 2;
                break;
            case POINT_DX:
                if (nPoint < JobInitialPoints[tch->GetJob()].dx) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "Cannot set stat under initial stat.");
                    return;
                }
                n = 3;
                break;
        }

        if (nPoint > gConfig.maxStat) {
            nChangeAmount -= nPoint - gConfig.maxStat;
            nPoint = gConfig.maxStat;
        }

        if (nRemainPoint < nChangeAmount) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "남은 스탯 포인트가 적습니다.");
            return;
        }

        tch->SetRealPoint(subcmd, nPoint);
        tch->SetPoint(subcmd, tch->GetPoint(subcmd) + nChangeAmount);
        tch->PointChange(subcmd, 0);

        tch->PointChange(POINT_STAT, -nChangeAmount);
        tch->ComputePoints();
        tch->ComputeMountPoints();
        const char* stat_name[4] = {"con", "int", "str", "dex"};

        ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s change %d to %d", szName,
                       stat_name[n], nCurPoint, nPoint);
    }
}

ACMD(do_get_item_id_list)
{
    for (int i = 0; i < INVENTORY_MAX_NUM; i++) {
        CItem* item = ch->GetInventoryItem(i);
        if (item != nullptr)
            ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d",
                           item->GetCell(), item->GetName(), item->GetID());
    }
}

ACMD(do_set_socket)
{
    char arg1[256];
    char arg2[256];
    char arg3[256];

    one_argument(
        two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3,
        sizeof(arg3));

    int item_id, socket_num, value;
    if (!str_to_number(item_id, arg1) || !str_to_number(socket_num, arg2) ||
        !str_to_number(value, arg3))
        return;

    CItem* item = ITEM_MANAGER::instance().Find(item_id);
    if (item)
        item->SetSocket(socket_num, value);
}

ACMD(do_can_dead)
{
    if (subcmd)
        ch->SetArmada();
    else
        ch->ResetArmada();
}

ACMD(do_all_skill_master)
{
    ch->SetHorseLevel(SKILL_MAX_LEVEL);
    for (int i = 0; i < SKILL_MAX_NUM; i++) {
        if (true == ch->CanUseSkill(i)) {
            switch (i) {
                case SKILL_COMBO:
                    ch->SetSkillLevel(i, 2);
                    break;
                case SKILL_LANGUAGE1:
                case SKILL_LANGUAGE2:
                case SKILL_LANGUAGE3:
                    ch->SetSkillLevel(i, 20);
                    break;
                case SKILL_HORSE_SUMMON:
                    ch->SetSkillLevel(i, 10);
                    break;
                case SKILL_HORSE:
                    ch->SetSkillLevel(i, HORSE_MAX_LEVEL);
                    break;
                case SKILL_HORSE_WILDATTACK:
                case SKILL_HORSE_CHARGE:
                case SKILL_HORSE_ESCAPE:
                case SKILL_HORSE_WILDATTACK_RANGE:
                    ch->SetSkillLevel(i, 20);
                    break;
                default:
                    ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
                    break;
            }
        } else {
            switch (i) {
                case SKILL_HORSE_WILDATTACK:
                case SKILL_HORSE_CHARGE:
                case SKILL_HORSE_ESCAPE:
                case SKILL_HORSE_WILDATTACK_RANGE:
                    ch->SetSkillLevel(i, 20);
                    break;
            }
        }
    }
    ch->SetHorseLevel(HORSE_MAX_LEVEL);
    ch->ComputePoints();
    ch->ComputeMountPoints();
    ch->SkillLevelPacket();
}

ACMD(do_use_item)
{
    char arg1[256];

    one_argument(argument, arg1, sizeof(arg1));

    int cell = 0;
    str_to_number(cell, arg1);

    CItem* item = ch->GetInventoryItem(cell);
    if (item) {
        ch->UseItem(TItemPos(INVENTORY, cell));
    } else {
        ch->ChatPacket(CHAT_TYPE_INFO, "¾ÆÀÌÅÛÀÌ ¾ø¾î¼­ Âø¿ëÇÒ ¼ö ¾ø¾î.");
    }
}

ACMD(do_dragon_soul)
{
    if (ch->GetLevel() < 150) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You need to be atleast level 150 to use the "
                           "alchemy.");
        return;
    }

    char arg1[24];
    const char* rest = one_argument(argument, arg1, sizeof(arg1));
    switch (arg1[0]) {
        case 'a': {
            one_argument(rest, arg1, sizeof(arg1));
            int deck_idx = 0;
            if (str_to_number(deck_idx, arg1) == false) {
                return;
            }
            ch->DragonSoul_ActivateDeck(deck_idx);
        } break;
        case 'd': {
            ch->DragonSoul_DeactivateAll();
        } break;
    }
}

ACMD(do_ds_list)
{
    for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; i++) {
        TItemPos cell(DRAGON_SOUL_INVENTORY, i);

        CItem* item = ch->GetItem(cell);
        if (item != nullptr)
            ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d",
                           item->GetCell(), item->GetName(), item->GetID());
    }
}

ACMD(do_click_safebox)
{
    ch->SetSafeboxOpenPosition();
    ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}

ACMD(do_click_guildbank)
{
    /*
     *
    CGuild* guild = ch->GetGuild();

    if (!guild) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You cannot open the guild
    storage without a guild."); return;
    }

    if (guild->HasGradeAuth(guild->GetMember(ch->GetPlayerID())->grade,
    GUILD_AUTH_USE_GS))
    {
        ch->GetGuild()->GetStorage()->Open();
        TPacketCGSafeboxSize p;

        p.bHeader = HEADER_GC_GUILDSTORAGE_OPEN;
        p.bSize = CGuildStorage::GUILDSTORAGE_TAB_COUNT;

        ch->GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));
        ch->GetGuild()->GetStorage()->SendItems(ch);
        return;
    }

    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                       "You do not have the permission to use the guild "
                       "storage.");
                         */
}

ACMD(do_ds_qualify)
{
    if (!ch || !ch->IsGM())
        return;

    ch->DragonSoul_GiveQualification();
}

ACMD(do_target_state)
{
    CHARACTER* target = ch->GetTarget();
    if (!target) {
        ch->ChatPacket(CHAT_TYPE_INFO, "No target is selected!");
        return;
    }

    if (target == ch) {
        ch->ChatPacket(CHAT_TYPE_INFO, "If you wish to display your state, you "
                                       "can just run /state");
        return;
    }

    ShowState(ch, target);
}

#include "entity.h"
#include <unordered_map>

EVENTINFO(maintenance_event_data)
{
    int32_t seconds;
    int32_t duration;

    maintenance_event_data()
        : seconds(0)
        , duration(0)
    {
    }
};

EVENTFUNC(maintenance_event)
{
    maintenance_event_data* info =
        static_cast<maintenance_event_data*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("maintenance_event> <Factor> Null pointer");
        return 0;
    }

    int32_t& pSec = info->seconds;
    int32_t iDur = info->duration;

    const auto& c_set_desc = DESC_MANAGER::instance().GetClientSet();
    for (const auto d : c_set_desc) {
        if (d->GetCharacter()) {
            TPacketGCMaintenance p;
            p.seconds = info->seconds;
            p.duration = iDur;
            d->Send(HEADER_GC_MAINTENANCE, p);
        }
    }

    GmMaintenanceBroadcastPacket p;
    p.seconds = info->seconds;
    p.duration = iDur;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmMaintenanceBroadcast,
                                                     p);

    if (pSec > 0) {
        --pSec;
    }

    if (pSec == 0) {
        GmShutdownBroadcastPacket p;
        p.reason = "MAINT";
        p.countdown = 10;
        DESC_MANAGER::instance().GetMasterSocket()->Send(kGmShutdownBroadcast,
                                                         p);
        SPDLOG_ERROR("Accept shutdown command from {0}", p.reason);

        Shutdown(10);
        return 0;
    }

    return THECORE_SECS_TO_PASSES(1);
}

LPEVENT m_pkMantenanceEvent = nullptr;

ACMD(do_maintenance)
{
    if (m_pkMantenanceEvent)
        event_cancel(&m_pkMantenanceEvent);

    if (!ch)
        return;

    if (ch->GetGMLevel() < GM_IMPLEMENTOR) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You have to be an Server Admin.");
        return;
    }

    char arg1[256];
    char arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Usage: maintenance <time> <durration>");
        return;
    }

    int32_t StartSec = parse_time_str(arg1);
    int32_t DurSec = parse_time_str(arg2);

    if (StartSec < 0) {
        if (ch) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "The StartSec cant be below zero.");
        }
        return;
    }

    maintenance_event_data* info = AllocEventInfo<maintenance_event_data>();
    info->seconds = StartSec;
    info->duration = DurSec;

    m_pkMantenanceEvent = event_create(maintenance_event, info, 1);
}

ACMD(do_set_title)
{
    const char* szHelp = "/set_title <title> <color>";
    CHECK_COMMAND(Parser, 2, szHelp);

    std::string colorString = Parser.GetTokenString(1);
    uint32_t color = 0;

    if (!ch->IsGM()) {
        if (!ch->CountSpecifyTypeItem(ITEM_USE, USE_SET_TITLE)) {
            return;
        }
    }

    if (!check_name(colorString.c_str())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "There are invalid characters in your title.");
        return;
    }

    if (!storm::ParseNumber(colorString.c_str(), color)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You cannot use this color in your title.");
        return;
    }

    if (!ch->IsGM()) {
        if (!ch->CountSpecifyTypeItem(ITEM_USE, USE_SET_TITLE)) {
            return;
        }
        auto item = ch->FindSpecifyTypeItem(ITEM_USE, USE_SET_TITLE);
        if (!item)
            return;
        ITEM_MANAGER::instance().RemoveItem(item, "SET_TITLE");
    }

    ch->SetTitle(Parser.GetTokenString(0), color);
}

ACMD(do_get_drop_mob)
{
    const char* szHelp = "/get_drop_mob <item_vnum>";
    CHECK_COMMAND(Parser, 1, szHelp);

    int Params = Parser.GetTokenNum();
    auto firstArg = Parser.GetToken(0);
    uint32_t dwVnum;

    if (is_positive_number(std::string(firstArg)))
        str_to_number(dwVnum, firstArg);
    else {
        if (!ITEM_MANAGER::instance().GetVnum(firstArg, dwVnum)) {
            ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum.",
                           dwVnum);
            return;
        }
    }

    int ItemVnum = dwVnum;

    uint32_t search_vnum = 19;
    std::set<uint32_t> monsters_that_drop;

    const auto& pkDropItemGroup =
        ITEM_MANAGER::instance().GetMapDropItemGroup();
    const auto& pkLevelItemGroup =
        ITEM_MANAGER::instance().GetMapMobLevelItemGroup();
    const auto& pkMobItemGroup = ITEM_MANAGER::instance().GetMapMobItemGroup();

    for (const auto& mobitemgroup : pkDropItemGroup) {
        auto v = mobitemgroup.second->GetVector();

        if (std::find_if(
                v.begin(), v.end(),
                [ItemVnum](const CDropItemGroup::SDropItemGroupInfo& s) {
                    return s.dwVnum == ItemVnum;
                }) != v.end()) {
            monsters_that_drop.insert(mobitemgroup.first);
            break;
        }
    }

    for (auto& mobitemgroup : pkLevelItemGroup) {
        auto v = mobitemgroup.second->GetVector();

        if (std::find_if(
                v.begin(), v.end(),
                [ItemVnum](const CLevelItemGroup::SLevelItemGroupInfo& s) {
                    return s.dwVNum == ItemVnum;
                }) != v.end()) {
            monsters_that_drop.insert(mobitemgroup.first);
            break;
        }
    }

    for (auto& mobitemgroup : pkMobItemGroup) {
        auto v = mobitemgroup.second->GetVector();

        if (std::find_if(v.begin(), v.end(),
                         [ItemVnum](const CMobItemGroup::SMobItemGroupInfo& s) {
                             return s.dwItemVnum == ItemVnum;
                         }) != v.end()) {
            monsters_that_drop.insert(mobitemgroup.first);
            break;
        }
    }

    for (auto& monster : monsters_that_drop) {
        auto pkMob = CMobManager::instance().Get(monster);
        if (pkMob) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "%s droppt das angegebene Item",
                               pkMob->szLocaleName.c_str());
        }
    }
}

ACMD(do_get_drop_item)
{
    const char* szHelp = "/get_drop_item <mob_vnum>";
    CHECK_COMMAND(Parser, 1, szHelp);

    int Params = Parser.GetTokenNum();

    int MobVnum = Parser.GetTokenInt(0);

    std::set<uint32_t> items_that_drop;

    const auto& pkDropItemGroup =
        ITEM_MANAGER::instance().GetMapDropItemGroup();
    const auto& pkLevelItemGroup =
        ITEM_MANAGER::instance().GetMapMobLevelItemGroup();
    const auto& pkMobItemGroup = ITEM_MANAGER::instance().GetMapMobItemGroup();

    auto it = pkDropItemGroup.find(MobVnum);
    if (it != pkDropItemGroup.end()) {
        auto v = it->second->GetVector();

        for (uint32_t i = 0; i < v.size(); ++i) {
            items_that_drop.insert(v[i].dwVnum);
        }
    }

    auto itMobItem = pkMobItemGroup.find(MobVnum);
    if (itMobItem != pkMobItemGroup.end()) {
        auto v = itMobItem->second->GetVector();

        for (uint32_t i = 0; i < v.size(); ++i) {
            items_that_drop.insert(v[i].dwItemVnum);
        }
    }

    auto itLevelItem = pkLevelItemGroup.find(MobVnum);
    if (itLevelItem != pkLevelItemGroup.end()) {
        auto v = itLevelItem->second->GetVector();

        for (uint32_t i = 0; i < v.size(); ++i) {
            items_that_drop.insert(v[i].dwVNum);
        }
    }

    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "Das angegebene Monster droppt:");
    for (auto itemVnum : items_that_drop) {
        auto pkItem = ITEM_MANAGER::instance().GetTable(itemVnum);
        if (pkItem) {
            SendChatPacket(ch, CHAT_TYPE_INFO, pkItem->szLocaleName);
        }
    }
}

ACMD(do_free_regen)
{
    ch->ChatPacket(CHAT_TYPE_INFO, "freeing regens on mapindex %ld",
                   ch->GetMapIndex());
    regen_free_map(ch->GetMapIndex());
    ch->ChatPacket(CHAT_TYPE_INFO, "the regens now FREEEE! :)");
}

ACMD(do_test_chest_drop) {}

ACMD(do_set_attr)
{
    char arg1[256];
    char arg2[256];
    char arg3[256];
    char arg4[256];

    one_argument(three_arguments(argument, arg1, sizeof(arg1), arg2,
                                 sizeof(arg2), arg3, sizeof(arg3)),
                 arg4, sizeof(arg4));

    int inventoryPos, attrPos, attrValue;
    if (!str_to_number(inventoryPos, arg1) || !str_to_number(attrPos, arg2) ||
        !str_to_number(attrValue, arg4)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: set_attr <inventory pos> "
                                       "<attribute pos> <attr name/num> <attr "
                                       "value>");
        return;
    }

    uint32_t applyType = 0;
    GetApplyTypeValue(arg3, applyType);
    if (applyType == 0 && !str_to_number(applyType, arg3)) {
        ch->ChatPacket(CHAT_TYPE_INFO,
                       "The apply name (%s) doesn't match any apply type",
                       arg3);
        return;
    }

    CItem* i = ch->GetInventoryItem(inventoryPos);
    if (!i) {
        ch->ChatPacket(CHAT_TYPE_INFO, "You need to have an item on that "
                                       "inventory position");
        return;
    }

    if (attrPos >= ITEM_ATTRIBUTE_MAX_NUM) {
        ch->ChatPacket(CHAT_TYPE_INFO,
                       "Incorrect attribute position! Valid: [0, %d]",
                       ITEM_ATTRIBUTE_MAX_NUM - 1);
        return;
    }

    i->SetForceAttribute(attrPos, (uint8_t)applyType, attrValue);
}

ACMD(do_copy_attr)
{
    char arg1[256];
    char arg2[256];

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    int fromPos, targetPos;
    if (!str_to_number(fromPos, arg1) || !str_to_number(targetPos, arg2)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: copy_attr <from inventory pos> "
                                       "<to inventory pos> <attr name/num> "
                                       "<attr value>");
        return;
    }

    CItem* from = ch->GetInventoryItem(fromPos);
    CItem* to = ch->GetInventoryItem(targetPos);
    if (!from) {
        ch->ChatPacket(CHAT_TYPE_INFO,
                       "You need to have an item on the %d inventory position",
                       fromPos);
        return;
    }

    if (!to) {
        ch->ChatPacket(CHAT_TYPE_INFO,
                       "You need to have an item on the %d inventory position",
                       targetPos);
        return;
    }

    ITEM_MANAGER::instance().CopyAllAttrTo(from, to);
}

ACMD(do_start_ox_event)
{
    COXEventManager::instance().StartAutomaticOX();
}

ACMD(do_impersonate)
{

    const char* szHelp = "/impersonate <name>";
    CHECK_COMMAND(Parser, 1, szHelp);

    auto name = Parser.GetTokenString(0);

    auto p = std::unique_ptr<SQLMsg>(
        DBManager::instance().DirectQuery("SELECT id, account_id, slot FROM "
                                          "{}.player WHERE name = '{}'",
                                          gConfig.playerDb.name, name));
    if (p && p->Get() && p->Get()->uiNumRows) {

        const auto& row = p->Get()->rows[0];

        TPlayerLoadPacket p2;
        storm::ParseNumber(row[1], p2.account_id);
        storm::ParseNumber(row[0], p2.player_id);
        storm::ParseNumber(row[2], p2.account_index);

        db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD,
                                ch->GetDesc()->GetHandle(), &p2,
                                sizeof(TPlayerLoadPacket));
    }
}

ACMD(do_fly_test)
{

    const char* szHelp = "/fly_test <index>";
    CHECK_COMMAND(Parser, 1, szHelp);

    ch->CreateFly(Parser.GetTokenInt(0), ch);
}

ACMD(do_spawn_copy)
{
}

#ifdef __OFFLINE_SHOP_CLOSE_BY_COMMAND__
#include "OfflineShop.h"
#include "OfflineShopManager.h"

ACMD(do_shop_close)
{
    const char* szHelp = "/shop_close <player>";
    CHECK_COMMAND(Parser, 1, szHelp);
    auto arg1 = Parser.GetTokenString(0);


	auto* pkOfflineShop = COfflineShopManager::instance().FindOfflineShop(arg1.c_str());
	if (!pkOfflineShop)
	{
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "There is no shop by owner name %d.", arg1);
		return;
	}

	pkOfflineShop->CloseShop();
}

ACMD(do_shop_rename)
{
    const char* szHelp = "/shop_rename <player> <new_name>";
    CHECK_COMMAND(Parser, 2, szHelp);

    auto arg1 = Parser.GetTokenString(0);
    auto playerName = Parser.GetTokenString(1);

	COfflineShop* pkOfflineShop = COfflineShopManager::instance().FindOfflineShop(arg1.c_str());
	if (!pkOfflineShop)
	{
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "There is no shop by owner name %d.", arg1);
		return;
	}

	pkOfflineShop->SetName(playerName.c_str());
}
#endif