#include "DbCacheSocket.hpp"
#include "OfflineShop.h"
#include "OfflineShopManager.h"
#include "char.h"
#include "desc.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "questmanager.h"


#include <slice.hpp>

void CHARACTER::ShopSearch_ItemSearch(const int32_t vnum, const int32_t extra)
{
    if (m_shopSearchVnumFilter != vnum)
        m_shopSearchSettingsChanged = true;
    if (m_shopSearchVnumExtraFilter != extra)
        m_shopSearchSettingsChanged = true;

    m_shopSearchVnumFilter = vnum;
    m_shopSearchVnumExtraFilter = extra;
    m_shopSearchFilterType = -1;
    m_shopSearchItemSubType = -1;
    m_shopSearchItemType = -1;
    if (!m_shopSearchSettingsChanged) {
        ShopSearch_SetPage(1);
    } else {
        ShopSearch_UpdateResults();
    }
}

void CHARACTER::ShopSearch_OpenFilter(const int32_t filterType)
{
    if (m_shopSearchFilterType != filterType)
        m_shopSearchSettingsChanged = true;

    m_shopSearchFilterType = filterType;
    m_shopSearchItemSubType = -1;
    m_shopSearchItemType = -1;
    m_shopSearchVnumFilter = -1;
    m_shopSearchVnumExtraFilter = -1;

    if (!m_shopSearchSettingsChanged) {
        ShopSearch_SetPage(1);
    } else {
        ShopSearch_UpdateResults();
    }
}

void CHARACTER::ShopSearch_UpdateResults()
{
    /* const auto& offlineShops = COfflineShopManager::instance().GetShopMap();
     if (offlineShops.empty()) {
         SendI18nChatPacket(this, CHAT_TYPE_INFO,
                            ("PRIVATE_SHOP_SEARCH_NO_SHOPS_FOUND"));
         return;
     }

     m_shopSearchData.clear();
     for (const auto& offlineShop : offlineShops) {

         auto* shop = offlineShop.second.get();

         if (!shop->IsOpened())
             continue;

         if (shop->GetItemCount() < 1)
             continue;

         auto* keeper = shop->GetShopKeeper();
         if (!keeper)
             continue;

         const auto keeperVid = keeper->GetVID();
         const auto* ownerName = shop->GetOwnerName();

         const auto& items = shop->GetItemMap();

         for (const auto& entry : items) {
             const auto item = entry.second;

             if (item.info.vnum == 0)
                 continue;

             const auto* proto =
                 ITEM_MANAGER::instance().GetTable(item.info.vnum);
             if (!proto)
                 continue;

             ShopSearchItemData entry2;
             entry2.data = item;
             entry2.ownerVid = keeperVid;
             entry2.ownerName = ownerName;

             bool add = false;

             if (m_shopSearchVnumFilter != -1) {
                 if (m_shopSearchVnumFilter == item.info.vnum)
                     add = true;
                 if (m_shopSearchVnumExtraFilter != -1 &&
                     proto->bType == ITEM_TOGGLE &&
                     proto->bSubType == TOGGLE_LEVEL_PET) {
                     if (item.info.transVnum == m_shopSearchVnumExtraFilter) {
                         add = true;
                     } else {
                         add = false;
                     }
                 }
             } else {
                 if (m_shopSearchFilterType != -1) {
                     switch (m_shopSearchFilterType) {
                         case ShopSearchFilters::Everything:
                             add = true;
                             break;
                         case ShopSearchFilters::Weapons:
                             add = proto->bType == ITEM_WEAPON;
                             break;
                         case ShopSearchFilters::Armor:
                             add = (proto->bType == ITEM_ARMOR) ||
                                   proto->bType == ITEM_BELT ||
                                   proto->bType == ITEM_RING ||
                                   proto->bType == ITEM_TALISMAN ||
                                   (proto->bType == ITEM_TOGGLE &&
                                    proto->bSubType == TOGGLE_AFFECT);
                             break;
                         case ShopSearchFilters::Costume:
                             add = proto->bType == ITEM_COSTUME;
                             break;
                         case ShopSearchFilters::PetsAndMounts:
                             add = (proto->bType == ITEM_TOGGLE &&
                                    proto->bSubType == TOGGLE_PET) ||
                                   (proto->bType == ITEM_TOGGLE &&
                                    proto->bSubType == TOGGLE_MOUNT) ||
                                   (proto->bType == ITEM_TOGGLE &&
                                    proto->bSubType == TOGGLE_LEVEL_PET) ||
                                   (proto->bType == ITEM_USE &&
                                    proto->bSubType == USE_LEVEL_PET_FOOD);
                             break;
                         case ShopSearchFilters::DragonSoul:
                             add = proto->bType == ITEM_DS;
                             break;
                         case ShopSearchFilters::Chests:
                             add = proto->bType == ITEM_GIFTBOX;
                             break;
                         case ShopSearchFilters::Useful:
                             add = proto->bType == ITEM_QUEST ||
                                   proto->bType == ITEM_USE ||
                                   proto->bType == ITEM_SKILLBOOK ||
                                   proto->bType == ITEM_FISH;
                             break;
                         default:
                             add = false;
                             break;
                     }
                 } else {
                     if (m_shopSearchItemType == -1) {
                         add = true;
                     } else if (m_shopSearchItemType > -1 &&
                                proto->bType == m_shopSearchItemType) {
                         if (m_shopSearchItemSubType > -1 &&
                             proto->bSubType == m_shopSearchItemSubType) {
                             add = true;
                         } else if (m_shopSearchItemSubType == -1) {
                             add = true;
                         }
                     }
                 }
             }

             if (add) {
                 m_shopSearchData.push_back(entry2);
             }
         }
     }

     m_shopSearchPageCount = ceilf(static_cast<float>(m_shopSearchData.size()) /
                                   static_cast<float>(m_shopSearchPerPage));*/
    quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetPlayerID());

    if (!pPC)
        return;

    uint32_t dwShopSearchSecCycle = 1; // 1 sec
    uint32_t dwNowSec = get_global_time();
    uint32_t dwLastShopSearchAttrSec = pPC->GetFlag("ShopSearch."
                                                    "LastShopSearchSec");

 /*if (dwLastShopSearchAttrSec + dwShopSearchSecCycle > dwNowSec) {
        SendI18nChatPacket(
            this, CHAT_TYPE_INFO, ("PRIVATE_SHOP_SEARCH_NEED_WAIT_%d_LEFT_(%d)"),
            dwShopSearchSecCycle,
            dwShopSearchSecCycle - (dwNowSec - dwLastShopSearchAttrSec));
        return;
    }
*/ 
    pPC->SetFlag("ShopSearch.LastShopSearchSec", dwNowSec);

    TPacketOfflineShopSearch p;
    p.filterType = m_shopSearchFilterType;
    p.itemType = m_shopSearchItemType;
    p.itemSubType = m_shopSearchItemSubType;
    p.vnumFilter = m_shopSearchVnumFilter;
    p.vnumExtraFilter = m_shopSearchVnumExtraFilter;
    db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_SEARCH_REQUEST,
                            GetDesc()->GetHandle(), &p,
                            sizeof(TPacketOfflineShopSearch));
}

void CHARACTER::ShopSearch_OpenCategory(const int32_t itemType,
                                        const int32_t itemSubType)
{
    if (itemType != m_shopSearchItemType) {
        m_shopSearchItemType = itemType;
        m_shopSearchSettingsChanged = true;
    }

    if (itemSubType != m_shopSearchItemSubType) {
        m_shopSearchItemSubType = itemSubType;
        m_shopSearchSettingsChanged = true;
    }

    m_shopSearchFilterType = -1;
    m_shopSearchVnumFilter = -1;
    m_shopSearchVnumExtraFilter = -1;

    if (!m_shopSearchSettingsChanged) {
        ShopSearch_SetPage(1);
    } else {
        ShopSearch_UpdateResults();
    }
}

void CHARACTER::ShopSearch_SetPage(const uint16_t page)
{
    m_shopSearchCurrentPage = page;
    m_shopSearchCurrentPageData.clear();
    const auto start = (m_shopSearchCurrentPage - 1) * m_shopSearchPerPage;
    const auto end = ((m_shopSearchCurrentPage - 1) * m_shopSearchPerPage) +
                     m_shopSearchPerPage;
    const auto items = iter::slice(m_shopSearchData, start, end);
    for (const auto& item : items)
        m_shopSearchCurrentPageData.emplace_back(item);

    ShopSearchPageInfo sspi;
    sspi.page = m_shopSearchCurrentPage;
    sspi.pageCount = m_shopSearchPageCount;
    sspi.perPage = m_shopSearchPerPage;

    GcShopSearchPacket p;
    p.setItems = m_shopSearchCurrentPageData;
    p.pageInfo = sspi;
    GetDesc()->Send(HEADER_GC_SHOPSEARCH, p);
}

void CHARACTER::ShopSearch_BuyItem(const uint32_t itemId)
{
    db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_ITEM_BUY_REQUEST_BY_ID,
                            GetDesc()->GetHandle(), &itemId, sizeof(uint32_t));
}

void CHARACTER::ShopSearch_HandleBuyItemFromDb(
    const TPacketOfflineShopSendItemInfo& itemInfo)
{
    const auto& price = itemInfo.kItem.llPrice;
    if (GetGold() < price) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You don't have enough money!");
        return;
    }

    const auto* pkItemTable =
        ITEM_MANAGER::instance().GetTable(itemInfo.kItem.info.vnum);
    if (!pkItemTable) {
        SPDLOG_ERROR("There is no item table for item {}.",
                     itemInfo.kItem.info.vnum);
        return;
    }

    if (pkItemTable->bType == ITEM_DS) {
        if (!DragonSoul_IsQualified()) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               ("You are not qualified to buy dragon stone "
                                "alchemy items."));
            return;
        }
    }

#ifdef __OFFLINE_SHOP_LOGGING__
    LogManager::instance().OfflineShopLogBuyItem(
        itemInfo.dwOwnerPID, GetPlayerID(), itemInfo.kItem.info.vnum,
        itemInfo.kItem.info.count, price);
#endif

    TPacketOfflineShopRemoveItemById kPacket;
    kPacket.dwOwnerPID = itemInfo.dwOwnerPID;
    kPacket.itemId = itemInfo.itemId;

    db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_ITEM_BUY_BY_ID,
                            GetDesc()->GetHandle(), &kPacket, sizeof(kPacket));
}

void CHARACTER::ShopSearch_HandleBuyCompleteFromDb(
    const TPacketOfflineShopSendItemInfo& itemInfo)
{
    const auto& price = itemInfo.kItem.llPrice;

    CItem* pkItem = ITEM_MANAGER::instance().CreateItem(
        itemInfo.kItem.info.vnum, itemInfo.kItem.info.count);
    if (!pkItem) {
        SPDLOG_ERROR("Failed to create item!");
        return;
    }

    pkItem->SetSkipSave(true);
    pkItem->SetSockets(itemInfo.kItem.info.sockets);
    pkItem->SetAttributes(itemInfo.kItem.info.attrs);
    pkItem->SetTransmutationVnum(itemInfo.kItem.info.transVnum);
    pkItem->SetSkipSave(false);
    AutoGiveItem(pkItem, true, true);
    ChangeGold(-price);

    ShopSearch_UpdateResults();
    m_shopSearchSettingsChanged = false;
}

void CHARACTER::ShopSearch_SetResults(
    const std::vector<ShopSearchItemData>& items)
{
    m_shopSearchData.clear();
    m_shopSearchData = items;
    if(m_shopSearchData.empty())
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "No items found.");

    m_shopSearchPageCount = ceilf(static_cast<float>(m_shopSearchData.size()) /
                                  static_cast<float>(m_shopSearchPerPage));
    if (m_shopSearchSettingsChanged) {
        ShopSearch_SetPage(1);

    } else {
        ShopSearch_SetPage(m_shopSearchCurrentPage);
    }
    m_shopSearchSettingsChanged = false;
}
