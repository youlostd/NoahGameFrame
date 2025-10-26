#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "questmanager.h"

namespace quest
{
//
// "item" Lua functions
//

int item_get_cell(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();

    if (q.GetCurrentItem()) {
        lua_pushnumber(L, q.GetCurrentItem()->GetCell());
    } else
        lua_pushnumber(L, 0);
    return 1;
}

int item_select_cell(lua_State* L)
{
    lua_pushboolean(L, 0);
    if (!lua_isnumber(L, 1)) {
        return 1;
    }
    uint16_t cell = (uint16_t)lua_tonumber(L, 1);

    CHARACTER* ch = CQuestManager::instance().GetCurrentCharacterPtr();
    CItem* item = ch ? ch->GetInventoryItem(cell) : nullptr;

    if (!item) {
        return 1;
    }

    CQuestManager::instance().UpdateQuestItem(
        CQuestManager::instance().GetCurrentCharacterPtr(), item);
    lua_pushboolean(L, 1);

    return 1;
}

int item_select(lua_State* L)
{
    lua_pushboolean(L, 0);
    if (!lua_isnumber(L, 1)) {
        return 1;
    }
    uint32_t id = (uint32_t)lua_tonumber(L, 1);
    CItem* item = ITEM_MANAGER::instance().Find(id);

    if (!item) {
        return 1;
    }

    CQuestManager::instance().UpdateQuestItem(
        CQuestManager::instance().GetCurrentCharacterPtr(), item);
    lua_pushboolean(L, 1);

    return 1;
}

int item_get_id(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();

    if (q.GetCurrentItem()) {
        lua_pushnumber(L, q.GetCurrentItem()->GetID());
    } else
        lua_pushnumber(L, 0);
    return 1;
}

int item_remove(lua_State* L)
{
    QuestState* st = CQuestManager::instance().GetCurrentState();
    auto* item = ITEM_MANAGER::instance().Find(st->item);
    if (item) {
        if (CQuestManager::instance().GetCurrentCharacterPtr() ==
            item->GetOwner()) {
            ITEM_MANAGER::instance().RemoveItem(item);
            st->item = 0;
        } else {
            SPDLOG_ERROR("Tried to remove invalid item {:p}", (void*)item);
        }
    }

    return 0;
}

int item_equip(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto item = q.GetCurrentItem();
    auto ch = q.GetCurrentCharacterPtr();
    if (item && ch) {
        if(item->GetSectree())
            item->RemoveFromGround();
        ch->EquipItem(item);
    }

    return 0;
}

int item_get_socket(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    if (q.GetCurrentItem() && lua_isnumber(L, 1)) {
        int idx = (int)lua_tonumber(L, 1);
        if (idx < 0 || idx >= ITEM_SOCKET_MAX_NUM)
            lua_pushnumber(L, 0);
        else
            lua_pushnumber(L, q.GetCurrentItem()->GetSocket(idx));
    } else {
        lua_pushnumber(L, 0);
    }
    return 1;
}

int item_get_socket_pct(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    if (q.GetCurrentItem()) {
        lua_pushnumber(L, q.GetCurrentItem()->GetProto()->bGainSocketPct);
    } else {
        lua_pushnumber(L, 0);
    }
    return 1;
}

int item_socket_repair(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto item = q.GetCurrentItem();
    if (!item) {
        lua_pushnumber(L, 0);
        return 1;
    }

    if (!item->IsWeapon() && !item->IsArmor()) {
        lua_pushnumber(L, 0);
        return 1;
    }

    auto socketCount = item->GetProto()->bGainSocketPct;
    for (auto i = 0; i < socketCount; ++i) {
        if (item->GetSocket(i) == 0 || item->GetSocket(i) > 30000)
            item->SetSocket(i, 1, false);
    }
    ITEM_MANAGER::instance().SaveSingleItem(item);

    lua_pushnumber(L, 1);

    return 1;
}

int item_set_socket(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    if (q.GetCurrentItem() && lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
        int idx = (int)lua_tonumber(L, 1);
        int value = (int)lua_tonumber(L, 2);
        if (idx >= 0 && idx < ITEM_SOCKET_MAX_NUM)
            q.GetCurrentItem()->SetSocket(idx, value);
    }
    return 0;
}

int item_get_vnum(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushnumber(L, item->GetVnum());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int item_has_flag(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("flag is not a number.");
        lua_pushboolean(L, 0);
        return 1;
    }

    if (!item) {
        lua_pushboolean(L, 0);
        return 1;
    }

    long lCheckFlag = (long)lua_tonumber(L, 1);
    lua_pushboolean(L, IS_SET(item->GetFlag(), lCheckFlag));

    return 1;
}

int item_get_value(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (!item) {
        lua_pushnumber(L, 0);
        return 1;
    }

    if (!lua_isnumber(L, 1)) {
        SPDLOG_ERROR("index is not a number");
        lua_pushnumber(L, 0);
        return 1;
    }

    int index = (int)lua_tonumber(L, 1);

    if (index < 0 || index >= ITEM_VALUES_MAX_NUM) {
        SPDLOG_ERROR("index(%d) is out of range (0..%d)", index,
                     ITEM_VALUES_MAX_NUM);
        lua_pushnumber(L, 0);
    } else
        lua_pushnumber(L, item->GetValue(index));

    return 1;
}

int item_set_value(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (!item) {
        lua_pushnumber(L, 0);
        return 1;
    }

    if (false ==
        (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))) {
        SPDLOG_ERROR("index is not a number");
        lua_pushnumber(L, 0);
        return 1;
    }

    item->SetForceAttribute((int)lua_tonumber(L, 1),     // index
                            (uint8_t)lua_tonumber(L, 2), // apply type
                            (short)lua_tonumber(L, 3)    // apply value
    );

    return 0;
}

int item_get_name(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushstring(L, item->GetName());
    else
        lua_pushstring(L, "");

    return 1;
}

int item_get_size(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushnumber(L, item->GetSize());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int item_get_count(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushnumber(L, item->GetCount());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int item_set_count(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (!lua_isnumber(L, 1)) {
        lua_pushboolean(L, false);
        return 1;
    }

    auto count = static_cast<CountType>(lua_tonumber(L, 1));
    if (item)
        lua_pushboolean(L, item->SetCount(count));
    else
        lua_pushboolean(L, false);

    return 1;
}

int item_get_type(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushnumber(L, item->GetItemType());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int item_get_sub_type(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushnumber(L, item->GetSubType());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int item_get_refine_vnum(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushnumber(L, item->GetRefinedVnum());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int item_next_refine_vnum(lua_State* L)
{
    uint32_t vnum = 0;
    if (lua_isnumber(L, 1))
        vnum = (uint32_t)lua_tonumber(L, 1);

    const TItemTable* pTable = ITEM_MANAGER::instance().GetTable(vnum);
    if (pTable) {
        lua_pushnumber(L, pTable->dwRefinedVnum);
    } else {
        SPDLOG_ERROR("Cannot find item table of vnum %u", vnum);
        lua_pushnumber(L, 0);
    }
    return 1;
}

int item_get_level(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* item = q.GetCurrentItem();

    if (item)
        lua_pushnumber(L, item->GetRefineLevel());
    else
        lua_pushnumber(L, 0);

    return 1;
}

int item_get_level_limit(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();

    if (q.GetCurrentItem()) {
        if (q.GetCurrentItem()->GetItemType() != ITEM_WEAPON &&
            q.GetCurrentItem()->GetItemType() != ITEM_ARMOR) {
            return 0;
        }
        lua_pushnumber(L, q.GetCurrentItem()->GetLevelLimit());
        return 1;
    }
    return 0;
}

int item_start_realtime_expire(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    CItem* pItem = q.GetCurrentItem();

    if (pItem) {
        pItem->StartRealTimeExpireEvent();
        return 1;
    }

    return 0;
}

int item_copy_and_give_before_remove(lua_State* L)
{
    lua_pushboolean(L, 0);
    if (!lua_isnumber(L, 1))
        return 1;

    uint32_t vnum = (uint32_t)lua_tonumber(L, 1);

    CQuestManager& q = CQuestManager::instance();
    CItem* pItem = q.GetCurrentItem();
    CHARACTER* pChar = q.GetCurrentCharacterPtr();

    CItem* pkNewItem = ITEM_MANAGER::instance().CreateItem(vnum, 1, 0, false);

    if (pkNewItem) {
        ITEM_MANAGER::CopyAllAttrTo(pItem, pkNewItem);
        LogManager::instance().ItemLog(pChar, pkNewItem, "COPY SUCCESS",
                                       pkNewItem->GetName());

        uint16_t wCell = pItem->GetCell();

        ITEM_MANAGER::instance().RemoveItem(pItem, "REMOVE (COPY SUCCESS)");

        pkNewItem->AddToCharacter(pChar, TItemPos(INVENTORY, wCell));
        ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

        // ¼º°ø!
        lua_pushboolean(L, 1);
    }

    return 1;
}

int item_get_attr(lua_State* L)
{
    CQuestManager& q = CQuestManager::instance();
    auto* item = q.GetCurrentItem();

    if (!item) {
        SPDLOG_ERROR("cannot get current item");
        lua_pushnumber(L, 0);
        return 1;
    }

    if (false == lua_isnumber(L, 1)) {
        SPDLOG_ERROR("index is not a number");
        lua_pushnumber(L, 0);
        return 1;
    }

    int index = lua_tonumber(L, 1);
    const TPlayerItemAttribute& attrItem = item->GetAttribute(index);

    lua_pushnumber(L, attrItem.bType);
    lua_pushnumber(L, attrItem.sValue);
    return 2;
}

void RegisterITEMFunctionTable()
{
    luaL_reg item_functions[] = {
        {"get_id", item_get_id},
        {"get_cell", item_get_cell},
        {"select", item_select},
        {"select_cell", item_select_cell},
        {"remove", item_remove},
        {"get_socket", item_get_socket},
        {"set_socket", item_set_socket},
        {"get_socket_pct", item_get_socket_pct},
        {"socket_repair", item_socket_repair},
        {"get_vnum", item_get_vnum},
        {"has_flag", item_has_flag},
        {"get_value", item_get_value},
        {"set_value", item_set_value},
        {"get_name", item_get_name},
        {"get_size", item_get_size},
        {"get_count", item_get_count},
        {"set_count", item_set_count},
        {"get_type", item_get_type},
        {"get_sub_type", item_get_sub_type},
        {"get_refine_vnum", item_get_refine_vnum},
        {"get_level", item_get_level},
        {"next_refine_vnum", item_next_refine_vnum},

        {"get_level_limit", item_get_level_limit},
        {"start_realtime_expire", item_start_realtime_expire},
        {"copy_and_give_before_remove", item_copy_and_give_before_remove},
        {"get_attrib", item_get_attr},
        {"set_attrib", item_set_value},

        {"equip", item_equip},

        {nullptr, nullptr}};
    CQuestManager::instance().AddLuaFunctionTable("item", item_functions);
}
} // namespace quest
