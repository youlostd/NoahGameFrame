#include "questlua.h"
#include "questmanager.h"
#include "DbCacheSocket.hpp"
#include "char.h"
#include "item_manager.h"
#include "item.h"
#include "cmd.h"
#include <game/GamePacket.hpp>
#include "guild.h"
#ifdef ENABLE_GUILD_STORAGE
#include "guild_storage.h"
#endif
#include "desc.h"

#undef sys_err

extern ACMD(do_in_game_mall);

namespace quest
{
int game_set_event_flag(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();

    if (lua_isstring(L, 1) && lua_isnumber(L, 2))
        q.RequestSetEventFlag(lua_tostring(L, 1), (int)lua_tonumber(L, 2));

    return 0;
}

int game_get_event_flag(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();

    if (lua_isstring(L, 1))
        lua_pushnumber(L, q.GetEventFlag(lua_tostring(L, 1)));
    else
        lua_pushnumber(L, 0);

    return 1;
}

int game_set_hwid_flag(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();

    if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3))
        return 0;

    auto hwid = lua_tostring(L, 1);
    auto flag = lua_tostring(L, 2);
    long value = lua_tonumber(L, 3);

    q.SetHwidFlag(hwid, flag, value);

    return 0;
}

int game_get_hwid_flag(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();

    if (lua_isstring(L, 1) && lua_isstring(L, 2))
        lua_pushnumber(L, q.GetHwidFlag(lua_tostring(L, 1), lua_tostring(L, 2)));
    else
        lua_pushnumber(L, 0);

    return 1;
}

int game_request_make_guild(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    DESC *d = q.GetCurrentCharacterPtr()->GetDesc();
    if (d)
    {
        BlankPacket p;
        d->Send(HEADER_GC_REQUEST_MAKE_GUILD, p);
    }
    return 0;
}

#ifdef ENABLE_GUILD_STORAGE
	int game_open_guild_storage(lua_State* /*L*/)
	{
		CQuestManager& q = CQuestManager::instance();
		CHARACTER* ch = q.GetCurrentCharacterPtr();
		if (!ch->GetGuild())
			return 0;
		ch->GetGuild()->GetStorage()->Open();
		TPacketCGSafeboxSize p;

		p.bHeader = HEADER_GC_GUILDSTORAGE_OPEN;
		p.bSize = CGuildStorage::GUILDSTORAGE_TAB_COUNT;

		ch->GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));
		ch->GetGuild()->GetStorage()->SendItems(ch);
		//ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenGuildStorage");

		return 0;
	}
#endif

int game_get_safebox_level(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();
    lua_pushnumber(L, q.GetCurrentCharacterPtr()->GetSafeboxSize() / SAFEBOX_PAGE_SIZE);
    return 1;
}

int game_set_safebox_level(lua_State *L)
{
    CQuestManager &q = CQuestManager::instance();

    //q.GetCurrentCharacterPtr()->ChangeSafeboxSize(3*(int)lua_tonumber(L,-1));
    TSafeboxChangeSizePacket p;
    p.dwID = q.GetCurrentCharacterPtr()->GetDesc()->GetAccountTable().id;
    p.bSize = SAFEBOX_PAGE_SIZE * 15;
    db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_SIZE, q.GetCurrentCharacterPtr()->GetDesc()->GetHandle(), &p,
                            sizeof(p));

    return 0;
}

int game_open_safebox(lua_State * /*L*/)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    ch->SetSafeboxOpenPosition();
    ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
    return 0;
}

#ifdef ENABLE_GEM_SYSTEM
	int game_open_gem_shop(lua_State*)
	{
		CQuestManager& q = CQuestManager::instance();
		CHARACTER* ch = q.GetCurrentCharacterPtr();
		//if (ch->CheckItemsFull() == false)
//{
//	//ch->UpdateItemsGayaMarker();
//	//ch->InfoGayaMarker();
//	//ch->StartCheckTimeMarket();
//}
//else{
//	//ch->CheckTimeW();
//	//ch->InfoGayaMarker();
//	//ch->StartCheckTimeMarket();

//	//	ch->StartCheckTimeMarket();

		//}
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenGemShop");
		return 0;
	}
#endif

int game_open_mall(lua_State * /*L*/)
{
    CQuestManager &q = CQuestManager::instance();
    CHARACTER *ch = q.GetCurrentCharacterPtr();
    ch->SetSafeboxOpenPosition();
    ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
    return 0;
}

int game_drop_item(lua_State *L)
{
    //
    // Syntax: game.drop_item(50050, 1)
    //
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    uint32_t item_vnum = (uint32_t)lua_tonumber(L, 1);
    int count = (int)lua_tonumber(L, 2);
    long x = ch->GetX();
    long y = ch->GetY();

    CItem *item = ITEM_MANAGER::instance().CreateItem(item_vnum, count);

    if (!item)
    {
        SPDLOG_ERROR("cannot create item vnum {} count {}", item_vnum, count);
        return 0;
    }

    PIXEL_POSITION pos;
    pos.x = x + Random::get(-200, 200);
    pos.y = y + Random::get(-200, 200);

    item->AddToGround(ch->GetMapIndex(), pos);
    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
        item->StartDestroyEvent(gConfig.itemGroundTimeLong);
    } else {
        item->StartDestroyEvent(gConfig.itemGroundTime);
    }

    return 0;
}

int game_drop_item_with_ownership(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    CItem *item = nullptr;
    switch (lua_gettop(L))
    {
    case 1:
        item = ITEM_MANAGER::instance().CreateItem((uint32_t)lua_tonumber(L, 1));
        break;
    case 2:
    case 3:
        item = ITEM_MANAGER::instance().CreateItem((uint32_t)lua_tonumber(L, 1), (int)lua_tonumber(L, 2));
        break;
    default:
        return 0;
    }

    if (item == nullptr) { return 0; }

    if (lua_isnumber(L, 3))
    {
        int sec = (int)lua_tonumber(L, 3);
        if (sec <= 0) { item->SetOwnership(ch); }
        else { item->SetOwnership(ch, sec); }
    }
    else
        item->SetOwnership(ch);

    PIXEL_POSITION pos;
    pos.x = ch->GetX() + Random::get(-200, 200);
    pos.y = ch->GetY() + Random::get(-200, 200);

    item->AddToGround(ch->GetMapIndex(), pos);
    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
        item->StartDestroyEvent(gConfig.itemGroundTimeLong);
    } else {
        item->StartDestroyEvent(gConfig.itemGroundTime);
    }

    return 0;
}

int game_web_mall(lua_State *L)
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    if (ch != nullptr) { do_in_game_mall(ch, const_cast<char *>(""), 0, 0); }
    return 0;
}

void RegisterGameFunctionTable()
{
    luaL_reg game_functions[] =
    {
        {"get_safebox_level", game_get_safebox_level},
        {"request_make_guild", game_request_make_guild},
        {"set_safebox_level", game_set_safebox_level},

#ifdef ENABLE_GEM_SYSTEM
			{ "OpenGemShop", game_open_gem_shop },
#endif

        {"open_safebox", game_open_safebox},
        {"open_mall", game_open_mall},
        {"get_event_flag", game_get_event_flag},
        {"set_hwid_flag", game_set_hwid_flag},
        {"get_hwid_flag", game_get_hwid_flag},
        {"set_event_flag", game_set_event_flag},
        {"drop_item", game_drop_item},
        {"drop_item_with_ownership", game_drop_item_with_ownership},
        {"open_web_mall", game_web_mall},
#ifdef ENABLE_GUILD_STORAGE
			{ "open_guild_storage",			game_open_guild_storage },
#endif
        {nullptr, nullptr}
    };

    CQuestManager::instance().AddLuaFunctionTable("game", game_functions);
}
}
