#include "DbCacheSocket.hpp"
#include "GBufferManager.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "constants.h"
#include "db.h"
#include "desc.h"
#include "desc_manager.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "mob_manager.h"
#include "questmanager.h"
#include "shop.h"
#include "utils.h"

#include <game/GamePacket.hpp>
#include <game/PageGrid.hpp>

#include "battle_pass.h"
#include "spdlog/spdlog.h"

NpcShop::NpcShop()
    : Shop(kShopTypeNpc)
{
    // ctor
}

NpcShop::~NpcShop()
{
    // dtor
}

void NpcShop::AddShopTable(const TShopTableEx& shopTable)
{
    m_vec_shopTabs.emplace_back(shopTable);
}

bool NpcShop::AddGuest(CHARACTER* ch, uint32_t ownerVid, bool otherEmpire)
{
    STORM_ASSERT(!ch->GetShop(), "Already shopping");
    ch->SetShop(this);

    ShopGuest guest{};
    guest.fromOtherEmpire = otherEmpire;
    m_guestMap.emplace(ch, guest);
    auto desc = ch->GetDesc();

    TPacketGCShop pack;
    TPacketGCShopStartEx pack2 = {};

    std::vector<TPacketGCShopStartEx::TSubPacketShopTab> tabs;

    pack.subheader = SHOP_SUBHEADER_GC_START_EX;
    pack2.owner_vid = ownerVid;
    pack2.shop_tab_count = m_vec_shopTabs.size();

    for (const auto& shop_tab : m_vec_shopTabs) {
        TPacketGCShopStartEx::TSubPacketShopTab pack_tab = {};

        pack_tab.coin_type = shop_tab.coinType;
        pack_tab.coin_vnum = shop_tab.coinVnum;

        pack_tab.name = shop_tab.name;

        for (uint8_t i = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i) {
            pack_tab.items[i].vnum = shop_tab.items[i].vnum;
            pack_tab.items[i].count = shop_tab.items[i].count;

            switch (shop_tab.coinType) {
                case SHOP_COIN_TYPE_GOLD:
                    pack_tab.items[i].price = shop_tab.items[i].price;
                    break;

                case SHOP_COIN_TYPE_SECONDARY_COIN:
                    pack_tab.items[i].price = shop_tab.items[i].price;
                    break;

                default:
                    pack_tab.items[i].price = shop_tab.items[i].price;
                    break;
            }

            pack_tab.items[i].transVnum = 0;

            std::memset(pack_tab.items[i].attrs, 0,
                        sizeof(pack_tab.items[i].attrs));

            std::memset(pack_tab.items[i].sockets, 0,
                        sizeof(pack_tab.items[i].sockets));
        }

        tabs.emplace_back(pack_tab);
    }

    pack2.tabs = tabs;
    pack.actionStartEx = pack2;

    desc->Send(HEADER_GC_SHOP, pack);
    return true;
}

bool NpcShop::CheckFreeInventory(CHARACTER* ch, uint16_t pos, CountType amount,
                                 const TItemTable* item)
{
    PagedGrid<bool> inventoryGrid(5, 9, INVENTORY_PAGE_COUNT);
    for (uint32_t i = 0; i < inventoryGrid.GetSize(); ++i) {
        auto* inventoryItem = ch->GetInventoryItem(i);
        if (!inventoryItem)
            continue;

        inventoryGrid.PutPos(true, i, inventoryItem->GetSize());
    }

    for (int i = 0; i < amount; ++i) {
        std::optional<uint32_t> newPos = std::nullopt;
        switch (item->bType) {
            case ITEM_MATERIAL:
                newPos = inventoryGrid.FindBlankBetweenPages(
                    item->bSize, NORMAL_INVENTORY_MAX_PAGE,
                    MATERIAL_INVENTORY_MAX_PAGE);
                break;

            case ITEM_COSTUME:
                newPos = inventoryGrid.FindBlankBetweenPages(
                    item->bSize, MATERIAL_INVENTORY_MAX_PAGE,
                    COSTUME_INVENTORY_MAX_PAGE);
                break;
            default:
                newPos = inventoryGrid.FindBlankBetweenPages(
                    item->bSize, NORMAL_INVENTORY_MIN_PAGE,
                    NORMAL_INVENTORY_MAX_PAGE);
                break;
        }

        if (!newPos.has_value()) {
            return false;
        }

        inventoryGrid.PutPos(true, newPos.value(), item->bSize);
    }
    return true;
}

int NpcShop::Buy(CHARACTER* ch, uint16_t pos, CountType amount)
{
    uint8_t tabIdx = pos / SHOP_HOST_ITEM_MAX_NUM;
    uint8_t slotPos = pos % SHOP_HOST_ITEM_MAX_NUM;

    if (tabIdx >= GetTabCount()) {
        SPDLOG_WARN("ShopEx::Buy : invalid position {0} : {1}", pos,
                    ch->GetName());
        return SHOP_SUBHEADER_GC_INVALID_POS;
    }

    SPDLOG_DEBUG("ShopEx::Buy : name {0} pos {1}", ch->GetName(), pos);

    auto it = m_guestMap.find(ch);
    if (it == m_guestMap.end())
        return SHOP_SUBHEADER_GC_END;

    TShopTableEx& shopTab = m_vec_shopTabs[tabIdx];
    TShopItemTable& r_item = shopTab.items[slotPos];

    auto dwPrice = r_item.price * amount;

    switch (shopTab.coinType) {
        case SHOP_COIN_TYPE_GOLD: {
            if (ch->GetGold() < dwPrice) {
                SPDLOG_TRACE("ShopEx::Buy : Not enough money : {0} has {1}, "
                             "price {2}",
                             ch->GetName(), ch->GetGold(), dwPrice);
                return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
            }

            break;
        }

        case SHOP_COIN_TYPE_SECONDARY_COIN: {
            if (shopTab.coinVnum != 0) {
                int count = ch->CountSpecifyItem(shopTab.coinVnum);
                if (count < dwPrice) {
                    SPDLOG_TRACE("ShopEx::Buy : Not enough {} : {} has {}, "
                                 "price {}",
                                 shopTab.coinVnum, ch->GetName(), count,
                                 dwPrice);
                    return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX;
                }
            } else {
                int count = ch->CountSpecifyTypeItem(ITEM_SECONDARY_COIN);
                if (count < dwPrice) {
                    SPDLOG_TRACE("ShopEx::Buy : Not enough myeongdojun : {0} "
                                 "has {1}, price {2}",
                                 ch->GetName(), count, dwPrice);
                    return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX;
                }
            }
        } break;

        case SHOP_COIN_TYPE_CASH: {
            int count = ch->GetCash("cash");
            if (count < dwPrice) {
                SPDLOG_TRACE("ShopEx::Buy : Not enough cash : {0} has {1}, "
                             "price {2}",
                             ch->GetName(), count, dwPrice);
                return SHOP_SUBHEADER_GC_NOT_ENOUGH_CASH;
            }
        } break;
    }

    const auto* item = ITEM_MANAGER::instance().GetTable(r_item.vnum);
    if (!item)
        return SHOP_SUBHEADER_GC_SOLD_OUT;

    std::string itemName = item->szLocaleName;

    if (!CheckFreeInventory(ch, pos, amount, item)) {
        return SHOP_SUBHEADER_GC_INVENTORY_FULL;
    }

    switch (shopTab.coinType) {
        case SHOP_COIN_TYPE_GOLD: {
#ifdef ENABLE_BATTLE_PASS
            uint8_t bBattlePassId = ch->GetBattlePassId();
            if (bBattlePassId) {
                uint32_t dwYangCount, dwNotUsed;
                if (CBattlePass::instance().BattlePassMissionGetInfo(
                        bBattlePassId, SPENT_YANG, &dwNotUsed, &dwYangCount)) {
                    if (ch->GetMissionProgress(SPENT_YANG, bBattlePassId) <
                        dwYangCount)
                        ch->UpdateMissionProgress(SPENT_YANG, bBattlePassId,
                                                  dwPrice, dwYangCount);
                }
            }
#endif

            ch->ChangeGold(-dwPrice);
        }

        break;
        case SHOP_COIN_TYPE_SECONDARY_COIN:
            if (shopTab.coinVnum != 0)
                ch->RemoveSpecifyItem(shopTab.coinVnum, dwPrice);
            else
                ch->RemoveSpecifyTypeItem(ITEM_SECONDARY_COIN, -1, dwPrice);
            break;
        case SHOP_COIN_TYPE_CASH:
            ch->ChangeCash(dwPrice, "cash", false);
            break;
    }

    for (int i = 0; i < amount; ++i) {
        auto* newItem = ch->AutoGiveItem(r_item.vnum, r_item.count);
        ITEM_MANAGER::instance().FlushDelayedSave(newItem);

        if (shopTab.coinType != SHOP_COIN_TYPE_CASH) {

            LogManager::instance().ItemLog(ch, newItem, "BUY",
                                           fmt::format("{}:{}:{}:{}",
                                                       shopTab.coinType,
                                                       dwPrice, itemName, i + 1)
                                               .c_str());
        } else {
            LogManager::instance().ItemshopLog(ch->GetPlayerID(), r_item.vnum,
                                               ch->GetDesc()->GetAid(), dwPrice,
                                               r_item.count);
        }
    }

    DBManager::instance().SendMoneyLog(MONEY_LOG_SHOP, r_item.vnum,
                                       -static_cast<int64_t>(dwPrice));

    SPDLOG_DEBUG("ShopEx: BUY: name {0} {1}(x {2}) price {3} amount {2}",
                 ch->GetName(), itemName, r_item.count, dwPrice, amount);

    ch->Save();
    return SHOP_SUBHEADER_GC_OK;
}
