#include "Cache.h"
#include "ClientManager.h"
#include "Main.h"
#ifdef __OFFLINE_SHOP__

void CClientManager::TOfflineShop::ForEachPeer(uint8_t header, const void* data,
                                               size_t size, CPeer* except)
{
    for (const auto& listListenerPeer : listListenerPeers) {
        if (!listListenerPeer)
            continue;

        if (listListenerPeer->IsClosed())
            continue;

        if (listListenerPeer == except)
            continue;

        if (!listListenerPeer->GetChannel())
            continue;

        listListenerPeer->EncodeHeader(header, 0, size);

        if (size > 0 && data)
            listListenerPeer->Encode(data, size);
    }
}

bool CClientManager::LoadOfflineShops()
{
    char szQuery[QUERY_MAX_LEN];

    snprintf(szQuery, sizeof(szQuery),
             "SELECT "
             "o.pid, "
             "o.name, "
             "o.gold, "
             "o.channel, "
             "o.map_index, "
             "o.x, "
             "o.y, "
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
             "(p.offline_shop_farmed_opening_time + "
             "p.offline_shop_special_opening_time),"
#endif
             "o.open, "
             "p.name "
             "FROM "
             "offline_shop AS o "
             "INNER JOIN player AS p ON o.pid = p.id");

    std::unique_ptr<SQLMsg> pMsgGeneralData(
        CDBManager::instance().DirectQuery(szQuery));

    auto* pResultGeneralData = pMsgGeneralData->Get();
    if (pResultGeneralData->uiNumRows > 0) {
        MYSQL_ROW rowGeneralData;
        while ((rowGeneralData =
                    mysql_fetch_row(pResultGeneralData->pSQLResult))) {
            auto iCol = 0;

            TOfflineShop kShop;
            str_to_number(kShop.kShopData.dwOwnerPID, rowGeneralData[iCol++]);
            strlcpy(kShop.kShopData.szName, rowGeneralData[iCol++],
                    sizeof(kShop.kShopData.szName));
            str_to_number(kShop.kShopData.llGold, rowGeneralData[iCol++]);
            str_to_number(kShop.kShopData.byChannel, rowGeneralData[iCol++]);
            str_to_number(kShop.kShopData.lMapIndex, rowGeneralData[iCol++]);
            str_to_number(kShop.kShopData.lX, rowGeneralData[iCol++]);
            str_to_number(kShop.kShopData.lY, rowGeneralData[iCol++]);

#if defined(__OFFLINE_SHOP_OPENING_TIME__) ||                                  \
    defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
            str_to_number(kShop.kShopData.iLeftOpeningTime,
                          rowGeneralData[iCol++]);
#endif
#if !defined(__OFFLINE_SHOP_OPENING_TIME__)
            auto iOpen = 0;
            str_to_number(iOpen, rowGeneralData[iCol++]);
            kShop.kShopData.bOpen = (iOpen != 0);
#endif

            strlcpy(kShop.kShopData.szOwnerName, rowGeneralData[iCol++],
                    sizeof(kShop.kShopData.szOwnerName));
            snprintf(szQuery, sizeof(szQuery),
                     "SELECT "
                     "id, vnum, trans_vnum, count, pos, "
                     "socket0, socket1, socket2, "
                     "socket3, socket4, socket5, "
                     "attrtype0, attrvalue0, "
                     "attrtype1, attrvalue1, "
                     "attrtype2, attrvalue2, "
                     "attrtype3, attrvalue3, "
                     "attrtype4, attrvalue4, "
                     "attrtype5, attrvalue5, "
                     "attrtype6, attrvalue6"
                     ", price FROM offline_shop_items WHERE pid=%d",
                     kShop.kShopData.dwOwnerPID);
            std::unique_ptr<SQLMsg> pMsgItems(
                CDBManager::instance().DirectQuery(szQuery));

            auto pResultItems = pMsgItems->Get();
            if (pResultItems->uiNumRows > 0) {
                MYSQL_ROW rowItems;
                while ((rowItems = mysql_fetch_row(pResultItems->pSQLResult))) {
                    auto iCol2 = 0;

                    TOfflineShopItem kItem;
                    memset(&kItem, 0, sizeof(kItem));
                    str_to_number(kItem.dwID, rowItems[iCol2]);
                    str_to_number(kItem.data.info.id, rowItems[iCol2++]);
                    str_to_number(kItem.data.info.vnum, rowItems[iCol2++]);
                    str_to_number(kItem.data.info.transVnum, rowItems[iCol2++]);
                    str_to_number(kItem.data.info.count, rowItems[iCol2++]);
                    str_to_number(kItem.data.dwPosition, rowItems[iCol2++]);
                    for (auto i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
                        str_to_number(kItem.data.info.sockets[i],
                                      rowItems[iCol2++]);
                    }
                    for (auto i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i) {
                        str_to_number(kItem.data.info.attrs[i].bType,
                                      rowItems[iCol2++]);
                        str_to_number(kItem.data.info.attrs[i].sValue,
                                      rowItems[iCol2++]);
                    }

                    str_to_number(kItem.data.llPrice, rowItems[iCol2++]);

                    kShop.mapItems.emplace(kItem.data.dwPosition, kItem);
                }
            }

            this->m_mapOfflineShops.insert(
                TMapOfflineShopByOwnerPID::value_type(
                    kShop.kShopData.dwOwnerPID, kShop));
        }
    }

    return true;
}

bool CClientManager::LoadNextOfflineShopItemID()
{
    char szQuery[QUERY_MAX_LEN];

    snprintf(szQuery, sizeof(szQuery),
             "SELECT IFNULL(MAX(id), 0) FROM offline_shop_items WHERE id <= "
             "3000000000");
    std::unique_ptr<SQLMsg> pkMsgItemIDData(
        CDBManager::instance().DirectQuery(szQuery));

    auto pkResultItemIDData = pkMsgItemIDData->Get();
    if (pkResultItemIDData->uiNumRows < 1) {
        spdlog::error("FATAL :: Failed to load the greatest item ID for "
                      "offline shop items!");
        return false;
    }

    auto pkRowItemIDData = mysql_fetch_row(pkResultItemIDData->pSQLResult);
    if (!pkRowItemIDData) {
        spdlog::error("FATAL :: Failed to load the greatest item ID for "
                      "offline shop items!");
        return false;
    }

    str_to_number(this->m_dwNextOfflineShopItemID, pkRowItemIDData[0]);

    return true;
}

void CClientManager::ReceiveOfflineShopCreatePacket(
    TPacketGDOfflineShopCreate* pkPacket, uint8_t byChannel)
{
    if (this->FindOfflineShop(pkPacket->dwOwnerPID)) {
        spdlog::error("FATAL :: Player %d already has an offline shop! "
                      "DESTROYED!",
                      pkPacket->dwOwnerPID);
        this->DestroyOfflineShop(pkPacket->dwOwnerPID);
    }

    TOfflineShop kShop;
    kShop.kShopData.dwOwnerPID = pkPacket->dwOwnerPID;
    strlcpy(kShop.kShopData.szOwnerName, pkPacket->szOwnerName,
            sizeof(kShop.kShopData.szOwnerName));
    strncpy(kShop.kShopData.szName, pkPacket->szName,
            sizeof(kShop.kShopData.szName));

    kShop.kShopData.lMapIndex = pkPacket->lMapIndex;
    kShop.kShopData.lX = pkPacket->lX;
    kShop.kShopData.lY = pkPacket->lY;

    kShop.kShopData.byChannel = byChannel;
    kShop.kShopData.bOpen =
        true; // Lets assume after creation the shop is opened

    this->m_mapOfflineShops.emplace(pkPacket->dwOwnerPID, kShop);

    this->PutOfflineShopCache(&kShop);
}

void CClientManager::ReceiveOfflineShopUpdateNamePacket(
    CPeer* pkPeer, TPacketOfflineShopUpdateName* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    strncpy(pkOfflineShop->kShopData.szName, pkPacket->szName,
            sizeof(pkOfflineShop->kShopData.szName));

    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_NAME, pkPacket,
                               sizeof(TPacketOfflineShopUpdateName), pkPeer);

    this->PutOfflineShopCache(pkOfflineShop);
}

void CClientManager::ReceiveOfflineShopUpdatePositionPacket(
    TPacketGDOfflineShopUpdatePosition* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    pkOfflineShop->kShopData.lMapIndex = pkPacket->lMapIndex;
    pkOfflineShop->kShopData.lX = pkPacket->lX;
    pkOfflineShop->kShopData.lY = pkPacket->lY;

    this->PutOfflineShopCache(pkOfflineShop);
}

void CClientManager::ReceiveOfflineShopAddItemPacket(
    CPeer* pkPeer, TPacketOfflineShopAddItem* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    if (pkOfflineShop->mapItems.find(pkPacket->kItem.dwPosition) !=
        pkOfflineShop->mapItems.end())
        return;

    TOfflineShopItem kItem;
    kItem.dwID = this->GetNextOfflineShopItemID();
    memcpy(&kItem.data, &pkPacket->kItem, sizeof(kItem.data));
    kItem.data.info.id = kItem.dwID;
    pkOfflineShop->mapItems.emplace(pkPacket->kItem.dwPosition, kItem);

    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_ITEM_ADD, pkPacket,
                               sizeof(TPacketOfflineShopAddItem), pkPeer);

    this->PutOfflineShopItemCache(pkOfflineShop->kShopData.dwOwnerPID, &kItem);
}

void CClientManager::ReceiveOfflineShopMoveItemPacket(
    CPeer* pkPeer, TPacketOfflineShopMoveItem* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    auto it = pkOfflineShop->mapItems.find(pkPacket->dwOldPosition);
    if (it == pkOfflineShop->mapItems.end() ||
        pkOfflineShop->mapItems.find(pkPacket->dwNewPosition) !=
            pkOfflineShop->mapItems.end())
        return;

    it->second.data.dwPosition = pkPacket->dwNewPosition;

    pkOfflineShop->mapItems.emplace(pkPacket->dwNewPosition, it->second);

    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_ITEM_MOVE, pkPacket,
                               sizeof(TPacketOfflineShopMoveItem), pkPeer);

    this->PutOfflineShopItemCache(pkOfflineShop->kShopData.dwOwnerPID,
                                  &it->second);

    pkOfflineShop->mapItems.erase(pkPacket->dwOldPosition);
}

void CClientManager::ReceiveOfflineShopRemoveItemPacket(
    CPeer* pkPeer, TPacketOfflineShopRemoveItem* pkPacket)
{
    auto* pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    auto it = pkOfflineShop->mapItems.find(pkPacket->dwPosition);
    if (it != pkOfflineShop->mapItems.end()) {
        it->second.data.info.vnum = 0;
        this->PutOfflineShopItemCache(pkOfflineShop->kShopData.dwOwnerPID,
                                      &it->second);
    }

    pkOfflineShop->mapItems.erase(pkPacket->dwPosition);

    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_ITEM_REMOVE, pkPacket,
                               sizeof(TPacketOfflineShopRemoveItem), pkPeer);
}

template <class K, class T, class H, class P, class A, class Predicate>
void erase_if(std::unordered_map<K, T, H, P, A>& c, Predicate pred)
{
    for (auto i = c.begin(), last = c.end(); i != last;)
        if (pred(*i))
            i = c.erase(i);
        else
            ++i;
}
template <class K, class T, class C, class A, class Predicate>
void erase_if(std::map<K, T, C, A>& c, Predicate pred)
{
    for (auto i = c.begin(), last = c.end(); i != last;)
        if (pred(*i))
            i = c.erase(i);
        else
            ++i;
}
void CClientManager::ReceiveOfflineShopBuyItemByIdPacket(
    CPeer* pkPeer, uint32_t handle, TPacketOfflineShopRemoveItemById* pkPacket)
{
    auto* pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    uint8_t status = 0;


    Gold price = 0;
    TPacketOfflineShopSendItemInfo p2;
    p2.dwOwnerPID = pkOfflineShop->kShopData.dwOwnerPID;

    TPacketOfflineShopRemoveItem p;
    p.dwOwnerPID = pkOfflineShop->kShopData.dwOwnerPID;

    for (auto& [pos, item] : pkOfflineShop->mapItems) {
        if (item.dwID != pkPacket->itemId || item.data.info.vnum == 0)
            continue;

        p2.kItem = item.data;
        p2.itemId = item.dwID;
        item.data.info.vnum = 0;
        price = item.data.llPrice;

        p.dwPosition = pos;
        this->PutOfflineShopItemCache(pkOfflineShop->kShopData.dwOwnerPID,
                                      &item);
        status = 1;
    }

    if(price < 1)
        status = 0;


    if (status) {
        erase_if(pkOfflineShop->mapItems,
                 [pkPacket](const TMapItemByPosition::value_type& val) {
                     return val.second.dwID == pkPacket->itemId;
                 });
        pkOfflineShop->kShopData.llGold += price;

        TPacketOfflineShopUpdateGold updateGold;
        updateGold.llGold = pkOfflineShop->kShopData.llGold;
        updateGold.dwOwnerPID = pkOfflineShop->kShopData.dwOwnerPID;
        ForwardPacket(HEADER_DG_OFFLINE_SHOP_GOLD, &updateGold, sizeof(TPacketOfflineShopUpdateGold));

        ForwardPacket(HEADER_DG_OFFLINE_SHOP_ITEM_REMOVE, &p,
                                   sizeof(TPacketOfflineShopRemoveItem));



        this->PutOfflineShopCache(pkOfflineShop);

    }

    pkPeer->EncodeHeader(HEADER_DG_OFFLINE_SHOP_BUY_DONE, handle,
                         sizeof(uint8_t) + sizeof(p2));
    pkPeer->Encode(&status, sizeof(uint8_t));
    pkPeer->Encode(&p2, sizeof(p2));
}

void CClientManager::ReceiveOfflineShopRequestBuyRequest(CPeer* peer,
                                                         uint32_t dwHandle,
                                                         const uint32_t* p)
{
    uint8_t found = false;
    for (auto& [ownerId, pkOfflineShop] : m_mapOfflineShops) {
        for (auto& [pos, item] : pkOfflineShop.mapItems) {
            if (item.dwID == *p) {
                TPacketOfflineShopSendItemInfo itemInfo;
                itemInfo.itemId = item.dwID;
                itemInfo.kItem = item.data;
                itemInfo.dwOwnerPID = ownerId;
                found = true;
                peer->EncodeHeader(HEADER_DG_OFFLINE_SHOP_ITEM_INFO, dwHandle,
                                   sizeof(uint8_t) +
                                       sizeof(TPacketOfflineShopSendItemInfo));
                peer->Encode(&found, sizeof(uint8_t));
                peer->Encode(&itemInfo, sizeof(itemInfo));
                return;
            }
        }
    }
    peer->EncodeHeader(HEADER_DG_OFFLINE_SHOP_ITEM_INFO, dwHandle,
                       sizeof(uint8_t));
    peer->Encode(&found, sizeof(uint8_t));
}

void CClientManager::ReceiveOfflineShopRequestItemInfoPacket(
    CPeer* pkPeer, TPacketOfflineShopRequestItem* pkPacket)
{

    auto* pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    TPacketOfflineShopSendItemInfo p;
    bool found = false;
    for (auto& [pos, item] : pkOfflineShop->mapItems) {
        if (item.dwID == pkPacket->itemId) {
            p.kItem = item.data;
            found = true;
        }
    }
    if (!found)
        p.kItem = {};
    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_ITEM_INFO, &p,
                               sizeof(TPacketOfflineShopSendItemInfo), pkPeer);
}

void CClientManager::ReceiveOfflineShopUpdateGoldPacket(
    CPeer* pkPeer, TPacketOfflineShopUpdateGold* pkPacket)
{
    auto* pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    pkOfflineShop->kShopData.llGold = pkPacket->llGold;

    ForwardPacket(HEADER_DG_OFFLINE_SHOP_GOLD, pkPacket,
                               sizeof(TPacketOfflineShopUpdateGold), 0, pkPeer);

    this->PutOfflineShopCache(pkOfflineShop);
}

void CClientManager::ReceiveOfflineShopDestroyPacket(
    CPeer* pkPeer, TPacketOfflineShopDestroy* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_DESTROY, pkPacket,
                               sizeof(TPacketOfflineShopDestroy), pkPeer);

    pkOfflineShop->kShopData.byChannel = 0;
    this->PutOfflineShopCache(pkOfflineShop);

    for (auto itItem = pkOfflineShop->mapItems.begin();
         itItem != pkOfflineShop->mapItems.end(); ++itItem) {
        itItem->second.data.info.vnum = 0;
        this->PutOfflineShopItemCache(pkOfflineShop->kShopData.dwOwnerPID,
                                      &itItem->second);
    }

    this->DestroyOfflineShop(pkPacket->dwOwnerPID);
}

void CClientManager::ReceiveOfflineShopRegisterListenerPacket(
    CPeer* pkPeer, TPacketGDOfflineShopRegisterListener* pkPacket)
{
    auto* pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    if (std::find(pkOfflineShop->listListenerPeers.begin(),
                  pkOfflineShop->listListenerPeers.end(),
                  pkPeer) != pkOfflineShop->listListenerPeers.end())
        return;

    if (pkPacket->bNeedOfflineShopInfos) {
        pkPeer->EncodeHeader(
            HEADER_DG_OFFLINE_SHOP_LOAD, 0,
            sizeof(TPacketDGOfflineShopCreate) + sizeof(uint32_t) +
                sizeof(TOfflineShopItemData) * pkOfflineShop->mapItems.size());

        TPacketDGOfflineShopCreate kPacket;
        memcpy(&kPacket.kShopData, &pkOfflineShop->kShopData,
               sizeof(kPacket.kShopData));

        pkPeer->Encode(&kPacket, sizeof(kPacket));

        pkPeer->EncodeDWORD(pkOfflineShop->mapItems.size());
        for (auto& [pos, item] : pkOfflineShop->mapItems)
            pkPeer->Encode(&(item.data), sizeof(item.data));
    }

    pkOfflineShop->listListenerPeers.push_back(pkPeer);
}

void CClientManager::ReceiveOfflineShopUnregisterListenerPacket(
    CPeer* pkPeer, TPacketGDOfflineShopUnregisterListener* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    auto it = std::remove(pkOfflineShop->listListenerPeers.begin(),
                          pkOfflineShop->listListenerPeers.end(), pkPeer);
    if (it != pkOfflineShop->listListenerPeers.end())
        pkOfflineShop->listListenerPeers.resize(
            pkOfflineShop->listListenerPeers.size() - 1);
}

void CClientManager::ReceiveOfflineShopClosePacket(
    CPeer* pkPeer, TPacketOfflineShopClose* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_CLOSE, pkPacket,
                               sizeof(TPacketOfflineShopClose), pkPeer);
    pkOfflineShop->kShopData.bOpen = false;

    this->PutOfflineShopCache(pkOfflineShop);
}

void CClientManager::ReceiveOfflineShopSearchPacket(CPeer* pkPeer,
                                                    DWORD dwHandle,
                                                    TPacketOfflineShopSearch* p)
{

    // TODO: Query cache
    std::vector<ShopSearchItemData> searchResults;

    for (const auto& [pid, offlineShop] : m_mapOfflineShops) {
        if (offlineShop.kShopData.bOpen == false)
            continue;

        if (offlineShop.kShopData.byChannel == 0)
            continue;

        if (offlineShop.mapItems.empty())
            continue;

        for (const auto& [pos, item] : offlineShop.mapItems) {
            if (item.data.info.vnum == 0)
                continue;

            const auto* proto = GetItemTable(item.data.info.vnum);
            if (!proto)
                continue;

            ShopSearchItemData entry2;
            entry2.data = item.data;
            entry2.ownerVid = 0;
            storm::CopyStringSafe(entry2.ownerName,
                                  offlineShop.kShopData.szOwnerName);
            entry2.channel = offlineShop.kShopData.byChannel;
            entry2.mapIndex = offlineShop.kShopData.lMapIndex;

            bool add = false;

            if (p->vnumFilter != -1) {
                if (p->vnumFilter == item.data.info.vnum)
                    add = true;
                if (p->vnumExtraFilter != -1 && proto->bType == ITEM_TOGGLE &&
                    proto->bSubType == TOGGLE_LEVEL_PET) {
                    if (item.data.info.transVnum == p->vnumExtraFilter) {
                        add = true;
                    } else {
                        add = false;
                    }
                }
            } else {
                if (p->filterType != -1) {
                    switch (p->filterType) {
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
                    if (p->itemType == -1) {
                        add = true;
                    } else if (p->itemType > -1 &&
                               proto->bType == p->itemType) {
                        if (p->itemSubType > -1 &&
                            proto->bSubType == p->itemSubType) {
                            add = true;
                        } else if (p->itemSubType == -1) {
                            add = true;
                        }
                    }
                }
            }

            if (add) {
                searchResults.push_back(entry2);
            }
        }
    }

    pkPeer->EncodeHeader(
        HEADER_DG_OFFLINE_SHOP_SEARCH_RESULT, dwHandle,
        sizeof(uint32_t) + (sizeof(ShopSearchItemData) * searchResults.size()));
    pkPeer->EncodeDWORD(searchResults.size());
    pkPeer->Encode(&searchResults[0],
                   sizeof(ShopSearchItemData) * searchResults.size());
}

void CClientManager::ReceiveOfflineShopOpenPacket(
    CPeer* pkPeer, TPacketOfflineShopOpen* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    pkOfflineShop->ForEachPeer(HEADER_DG_OFFLINE_SHOP_OPEN, pkPacket,
                               sizeof(TPacketOfflineShopOpen), pkPeer);
    pkOfflineShop->kShopData.bOpen = true;

    this->PutOfflineShopCache(pkOfflineShop);
}

#if defined(__OFFLINE_SHOP_OPENING_TIME__) ||                                  \
    defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
void CClientManager::ReceiveOfflineShopFlushLeftOpeningTimePacket(
    TPacketGDOfflineShopFlushLeftOpeningTime* pkPacket)
{
    auto pkOfflineShop = this->FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop)
        return;

    pkOfflineShop->kShopData.iLeftOpeningTime = pkPacket->iLeftOpeningTime;

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    TPlayerTable* pkPlayerTable = NULL;

    const auto pc =
        CClientManager::instance().GetPlayerCache().Get(pkPacket->dwOwnerPID);
    if (!pc) {
        char szQuery[QUERY_MAX_LEN];

        snprintf(szQuery, sizeof(szQuery),
                 "UPDATE player AS p SET "
                 "p.offline_shop_farmed_opening_time = IF(%d > "
                 "p.offline_shop_special_opening_time, %d - "
                 "p.offline_shop_special_opening_time, 0), "
                 "p.offline_shop_special_opening_time = (%d - "
                 "p.offline_shop_farmed_opening_time) "
                 "WHERE id = %d",
                 pkOfflineShop->kShopData.iLeftOpeningTime,
                 pkOfflineShop->kShopData.iLeftOpeningTime,
                 pkOfflineShop->kShopData.iLeftOpeningTime,
                 pkOfflineShop->kShopData.dwOwnerPID);

        CDBManager::instance().AsyncQuery(szQuery, SQL_PLAYER);

    } else {
        auto tab = pc->GetTable();

        tab.iOfflineShopFarmedOpeningTime =
            std::max(pkOfflineShop->kShopData.iLeftOpeningTime -
                         tab.iOfflineShopSpecialOpeningTime,
                     0);
        tab.iOfflineShopSpecialOpeningTime =
            pkOfflineShop->kShopData.iLeftOpeningTime -
            tab.iOfflineShopFarmedOpeningTime;

        pc->SetTable(tab, false);
    }

#else
    this->PutOfflineShopCache(pkOfflineShop);
#endif
}
#endif

CClientManager::TOfflineShop*
CClientManager::FindOfflineShop(uint32_t dwOwnerPID)
{
    auto it = this->m_mapOfflineShops.find(dwOwnerPID);
    if (it == this->m_mapOfflineShops.end())
        return NULL;

    return &(it->second);
}

void CClientManager::DestroyOfflineShop(uint32_t dwOwnerPID)
{
    if (!this->FindOfflineShop(dwOwnerPID))
        return;

    this->m_mapOfflineShops.erase(dwOwnerPID);
}

uint32_t CClientManager::GetNextOfflineShopItemID()
{
    return ++this->m_dwNextOfflineShopItemID;
}

CClientManager::TMapOfflineShopByOwnerPID* CClientManager::GetOfflineShopMap()
{
    return &(this->m_mapOfflineShops);
}

void CClientManager::PutOfflineShopCache(TOfflineShop* pkOfflineShop)
{
    COfflineShopCache* pkOfflineShopCache;

    auto it =
        this->m_mapOfflineShopCaches.find(pkOfflineShop->kShopData.dwOwnerPID);
    if (it == this->m_mapOfflineShopCaches.end()) {
        pkOfflineShopCache = new COfflineShopCache();
        this->m_mapOfflineShopCaches.insert(
            TMapOfflineShopCacheByOwnerPID::value_type(
                pkOfflineShop->kShopData.dwOwnerPID, pkOfflineShopCache));
    } else {
        pkOfflineShopCache = it->second;
    }

    pkOfflineShopCache->Put(&pkOfflineShop->kShopData);
}

void CClientManager::PutOfflineShopItemCache(
    uint32_t dwOwnerPID, TOfflineShopItem* pkOfflineShopItem)
{
    COfflineShopItemCache* pkOfflineShopItemCache = NULL;

    auto it = this->m_mapOfflineShopItemCaches.find(pkOfflineShopItem->dwID);
    if (it == this->m_mapOfflineShopItemCaches.end()) {
        pkOfflineShopItemCache = new COfflineShopItemCache();
        this->m_mapOfflineShopItemCaches.insert(
            TMapOfflineShopItemCacheByItemID::value_type(
                pkOfflineShopItem->dwID, pkOfflineShopItemCache));
    } else {
        pkOfflineShopItemCache = it->second;
    }

    pkOfflineShopItemCache->Put(pkOfflineShopItem, dwOwnerPID);
}

void CClientManager::FlushOfflineShopCache(bool bDeleteCache)
{
    for (auto it = this->m_mapOfflineShopCaches.begin();
         it != this->m_mapOfflineShopCaches.end();) {
        if (bDeleteCache || it->second->CheckFlushTimeout()) {
            it->second->Flush();

            if (bDeleteCache || it->second->ShopWereDeleted()) {
                delete (it->second);
                this->m_mapOfflineShopCaches.erase(it++);
                continue;
            }
        }

        ++it;
    }
}

void CClientManager::FlushOfflineShopItemCache(bool bDeleteCache)
{
    for (auto it = this->m_mapOfflineShopItemCaches.begin();
         it != this->m_mapOfflineShopItemCaches.end();) {
        if (bDeleteCache || it->second->CheckFlushTimeout()) {
            it->second->Flush();

            if (bDeleteCache || it->second->ItemWereDeleted()) {
                delete (it->second);
                this->m_mapOfflineShopItemCaches.erase(it++);
                continue;
            }
        }

        ++it;
    }
}
#endif