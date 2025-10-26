#include "TextTagUtil.hpp"
#include "desc_manager.h"
#include <game/MasterPackets.hpp>

#ifdef __OFFLINE_SHOP__
#include "DbCacheSocket.hpp"
#include "OfflineShop.h"
#include "OfflineShopConfig.h"
#include "OfflineShopManager.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "desc.h"
#include "item.h"
#include "item_manager.h"
#include "PacketUtils.hpp"
#include "sectree_manager.h"
#include "utils.h"
#include <game/grid.h>

#ifdef __OFFLINE_SHOP_LOGGING__
#include "log.h"
#endif

#ifdef __OFFLINE_SHOP_CLOSE_ITEMS_TO_INVENTORY__
#include "DragonSoul.h"
#endif

COfflineShop::COfflineShop(uint32_t dwOwnerPID, const char *c_szOwnerName, const char *c_szName, long lMapIndex,
                           long lX, long lY)
    : m_byChannel(gConfig.channel), m_iOpeningTime(0)
{
    m_dwOwnerPID = dwOwnerPID;
    strlcpy(m_szOwnerName, c_szOwnerName, sizeof(m_szOwnerName));
    m_szName = c_szName;

    m_mapItems.clear();
    m_pkItemGrid = new CGrid(g_iOfflineShopSlotWidth, g_iOfflineShopSlotHeight);

    m_setViewers.clear();

    m_llGold = 0;

    m_pkShopKeeper = nullptr;
    m_lMapIndex = lMapIndex;
    m_lX = lX;
    m_lY = lY;

    m_bIsRealShop = true;

#ifdef __OFFLINE_SHOP_OPENING_TIME__
    m_iOpeningTime = 0;
    m_pkCloseEvent = NULL;
#else
    m_bOpen = false;
#endif
}

COfflineShop::COfflineShop(TOfflineShopData *pkData)
{
    m_dwOwnerPID = pkData->dwOwnerPID;
    strlcpy(m_szOwnerName, pkData->szOwnerName, sizeof(m_szOwnerName));
    m_szName = pkData->szName;

    m_mapItems.clear();
    m_pkItemGrid = new CGrid(g_iOfflineShopSlotWidth, g_iOfflineShopSlotHeight);

    m_setViewers.clear();

    m_llGold = pkData->llGold;

    m_pkShopKeeper = nullptr;
    m_byChannel = pkData->byChannel;
    m_lMapIndex = pkData->lMapIndex;
    m_lX = pkData->lX;
    m_lY = pkData->lY;

    m_bIsRealShop = true;

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    m_iOpeningTime = pkData->iLeftOpeningTime;
    m_pkCloseEvent = nullptr;
#endif
#if !defined(__OFFLINE_SHOP_OPENING_TIME__)
    m_bOpen = pkData->bOpen;
#endif
}

COfflineShop::~COfflineShop()
{
    for (const auto &viewerPid : m_setViewers)
    {
        if (const auto viewer = g_pCharManager->FindByPID(viewerPid); viewer)
        {
            RemoveViewer(viewer);
            viewer->SetViewingOfflineShop(nullptr);
        }
    }

    m_setViewers.clear();

    DespawnKeeper();

    CHARACTER *pkOwner = g_pCharManager->FindByPID(GetOwnerPID());
    if (pkOwner)
        pkOwner->SetMyOfflineShop(nullptr);

#ifdef __OFFLINE_SHOP_OPENING_TIME__
    StopCloseEvent();
#endif

    delete (m_pkItemGrid);
}

void COfflineShop::AddItem(TOfflineShopItemData *pkItem)
{
    if (!pkItem)
        return;

    const auto* pkItemTable = ITEM_MANAGER::instance().GetTable(pkItem->info.vnum);
    if (!pkItemTable)
    {
        SPDLOG_ERROR("No item table for item {} of shop of player {} at position {}.", pkItem->info.vnum, GetOwnerPID(),
                     pkItem->dwPosition);
        return;
    }
    else if (!m_pkItemGrid->IsEmpty(pkItem->dwPosition, 1, pkItemTable->bSize))
    {
        SPDLOG_ERROR("Item overlapping of item {}  at position {}  in shop of player {}.", pkItem->info.vnum,
                     pkItem->dwPosition, GetOwnerPID());
        return;
    }

    m_pkItemGrid->Put(pkItem->dwPosition, 1, pkItemTable->bSize);

    TOfflineShopItemData kShopItem;
    memcpy(&kShopItem, pkItem, sizeof(kShopItem));

    m_mapItems.insert(TMapItemByPosition::value_type(kShopItem.dwPosition, kShopItem));
    {
        TPacketGCOfflineShopAddItem kPacket;
        kPacket.data = kShopItem.info;

        kPacket.dwDisplayPosition = kShopItem.dwPosition;
        kPacket.llPrice = kShopItem.llPrice;

        SendPacketToViewer(HEADER_GC_OFFLINE_SHOP_ITEM_ADD, kPacket);
    }
}

void COfflineShop::AddItem(CItem *pkItem, uint32_t dwPosition, Gold llPrice)
{
    if (!pkItem || !pkItem->GetOwner() || pkItem->GetOwner()->GetPlayerID() != GetOwnerPID() ||
        pkItem->IsExchanging() || pkItem->isLocked() || pkItem->IsEquipped())
        return;

    if (pkItem->GetItemType() == ITEM_TOGGLE)
    {
        if (pkItem->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE))
        {
            SendI18nChatPacket(pkItem->GetOwner(), CHAT_TYPE_INFO, "You cannot put active items into your shop.");
            return;
        }
    }

    if (pkItem->IsSealed() || IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_MYSHOP))
    {
        SendI18nChatPacket(pkItem->GetOwner(), CHAT_TYPE_INFO,
                           "You tried to put at least one item into the shop which can not be traded yet.");
        return;
    }

    if (llPrice < 1)
    {
        SendI18nChatPacket(pkItem->GetOwner(), CHAT_TYPE_INFO, "The price of the item has to be at least 1 gold.");
        return;
    }

    if (!m_pkItemGrid->IsEmpty(dwPosition, 1, pkItem->GetSize()))
    {
        SendI18nChatPacket(pkItem->GetOwner(), CHAT_TYPE_INFO, "You can not add the item at position %u.", dwPosition);
        return;
    }

#ifdef __OFFLINE_SHOP_LOGGING__
    LogManager::instance().OfflineShopLogAddItem(GetOwnerPID(), pkItem->GetVnum(), pkItem->GetCount(), llPrice);
#endif

    TOfflineShopItemData kShopItem;
    kShopItem.info.vnum = pkItem->GetVnum();
    kShopItem.info.id = pkItem->GetID();

    kShopItem.info.transVnum = pkItem->GetTransmutationVnum();

    kShopItem.info.count = pkItem->GetCount();
    memcpy(&kShopItem.info.sockets, pkItem->GetSockets(), sizeof(kShopItem.info.sockets));
    memcpy(&kShopItem.info.attrs, pkItem->GetAttributes(), sizeof(kShopItem.info.attrs));
#ifdef __ITEM_RUNE__
    kShopItem.dwRune = pkItem->GetRune();
#endif
    kShopItem.dwPosition = dwPosition;
    kShopItem.llPrice = llPrice;

    pkItem->RemoveFromCharacter();

    m_mapItems.insert(TMapItemByPosition::value_type(kShopItem.dwPosition, kShopItem));
    m_pkItemGrid->Put(kShopItem.dwPosition, 1, pkItem->GetSize());

    ITEM_MANAGER::instance().DestroyItem(pkItem);
    {
        TPacketGCOfflineShopAddItem kPacket;
        kPacket.data = kShopItem.info;

        kPacket.dwDisplayPosition = kShopItem.dwPosition;
        kPacket.llPrice = kShopItem.llPrice;

        SendPacketToViewer(HEADER_GC_OFFLINE_SHOP_ITEM_ADD, kPacket);
    }
    {
        TPacketOfflineShopAddItem kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();
        memcpy(&kPacket.kItem, &kShopItem, sizeof(kShopItem));

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_ITEM_ADD, 0, &kPacket, sizeof(kPacket));
    }
}

void COfflineShop::RemoveItem(uint32_t dwPosition, bool bFromDatabase)
{
    TOfflineShopItemData *pkItem = FindItem(dwPosition);
    if (!pkItem)
    {
        SPDLOG_ERROR("Tried to remove item from empty position {} in shop of player {}!", dwPosition, GetOwnerPID());
        return;
    }

    const auto* pkItemTable = ITEM_MANAGER::instance().GetTable(pkItem->info.vnum);
    if (!pkItemTable)
        SPDLOG_ERROR("No item table for item {} of shop of player {} at position {}.", pkItem->info.vnum, GetOwnerPID(),
                     dwPosition);
    else
        m_pkItemGrid->Get(dwPosition, 1, pkItemTable->bSize);

    m_mapItems.erase(dwPosition);
    {
        TPacketGCOfflineShopRemoveItem kPacket;
        kPacket.dwDisplayPosition = dwPosition;

        SendPacketToViewer(HEADER_GC_OFFLINE_SHOP_ITEM_REMOVE, kPacket);
    }
    if (!bFromDatabase)
    {
        TPacketOfflineShopRemoveItem kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();
        kPacket.dwPosition = dwPosition;

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_ITEM_REMOVE, 0, &kPacket, sizeof(kPacket));
    }

    if (GetItemCount() < 1)
    {
        if (GetGold() < 1)
            COfflineShopManager::instance().DestroyOfflineShop(this);
        else
            CloseShop();
    }
}

void COfflineShop::RemoveItemByOwner(uint32_t dwPosition, TItemPos *pkInventoryPosition)
{
    TOfflineShopItemData *pkItemInfo = FindItem(dwPosition);
    if (!pkItemInfo)
    {
        SPDLOG_ERROR("Tried to remove item from empty position %u in shop of player %u!", dwPosition, GetOwnerPID());
        return;
    }

    CHARACTER *pkOwner = g_pCharManager->FindByPID(GetOwnerPID());
    if (!pkOwner)
        return;

    if (pkInventoryPosition)
    {
        const auto* pkItemTable = ITEM_MANAGER::instance().GetTable(pkItemInfo->info.vnum);
        if (!pkItemTable)
        {
            SPDLOG_ERROR("There is no item table for item {}.", pkItemInfo->info.vnum);
            return;
        }

        if (!pkOwner->IsEmptyItemGrid(*pkInventoryPosition, pkItemTable->bSize))
            return;

        if (pkItemTable->bType == ITEM_DS)
        {
            if (DRAGON_SOUL_INVENTORY != pkInventoryPosition->window_type)
            {
                SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, "<Storages> No movement possible.");
                return;
            }

            if (!DSManager::instance().IsValidCellForThisItem(pkItemTable, *pkInventoryPosition))
            {
                const int iCell = pkOwner->GetEmptyDragonSoulInventory(pkItemTable);
                if (iCell < 0)
                {
                    SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, "<Storages> No movement possible.");
                    return;
                }
                pkInventoryPosition->cell = iCell;
            }
        }
        else
        {
            if (DRAGON_SOUL_INVENTORY == pkInventoryPosition->window_type)
            {
                SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, ("<Storages> No movement possible."));
                return;
            }
        }
    }

    CItem *pkItem = ITEM_MANAGER::instance().CreateItem(pkItemInfo->info.vnum, pkItemInfo->info.count);
    if (!pkItem)
        return;

    pkItem->SetSockets(pkItemInfo->info.sockets);
    pkItem->SetAttributes(pkItemInfo->info.attrs);
    pkItem->SetTransmutationVnum(pkItemInfo->info.transVnum);
#ifdef __ITEM_RUNE__
    pkItem->SetRune(pkItemInfo->dwRune);
#endif

    if (pkInventoryPosition)
        pkItem->AddToCharacter(pkOwner, *pkInventoryPosition);
    else
        pkOwner->AutoGiveItem(pkItem);

#ifdef __OFFLINE_SHOP_LOGGING__
    LogManager::instance().OfflineShopLogRemoveItem(GetOwnerPID(), pkItemInfo->info.vnum, pkItemInfo->info.count);
#endif

    RemoveItem(dwPosition);
}

void COfflineShop::MoveItem(uint32_t dwCurrentPosition, uint32_t dwNewPosition, bool bFromDatabase)
{
    TOfflineShopItemData *pkItem;
    if (!((pkItem = FindItem(dwCurrentPosition))) || FindItem(dwNewPosition))
        return;

    TOfflineShopItemData kNewItem;
    memcpy(&kNewItem, pkItem, sizeof(kNewItem));
    kNewItem.dwPosition = dwNewPosition;

    m_mapItems.erase(dwCurrentPosition);
    m_mapItems.emplace(dwNewPosition, kNewItem);

    const auto* pkItemTable = ITEM_MANAGER::instance().GetTable(kNewItem.info.vnum);
    if (!pkItemTable)
    {
        SPDLOG_ERROR("No item table for item {} in shop of player {}.", kNewItem.info.vnum, GetOwnerPID());
    }
    else
    {
        m_pkItemGrid->Get(dwCurrentPosition, 1, pkItemTable->bSize);
        m_pkItemGrid->Put(dwNewPosition, 1, pkItemTable->bSize);
    }
    {
        TPacketGCOfflineShopMoveItem kPacket;
        kPacket.dwOldDisplayPosition = dwCurrentPosition;
        kPacket.dwNewDisplayPosition = dwNewPosition;

        SendPacketToViewer(HEADER_GC_OFFLINE_SHOP_ITEM_MOVE, kPacket);
    }
    if (!bFromDatabase)
    {
        TPacketOfflineShopMoveItem kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();
        kPacket.dwOldPosition = dwCurrentPosition;
        kPacket.dwNewPosition = dwNewPosition;

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_ITEM_MOVE, 0, &kPacket, sizeof(kPacket));
    }
}

int COfflineShop::GetItemCount() const { return m_mapItems.size(); }

void COfflineShop::AddViewer(CHARACTER *pkCharacter)
{
    if (!pkCharacter || !pkCharacter->GetDesc())
        return;

    if (pkCharacter->GetExchange() || pkCharacter->IsOpenSafebox() || pkCharacter->GetShop())
    {
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "You can not open a shop while you are trading.");
        return;
    }

    if (pkCharacter->GetPlayerID() != GetOwnerPID())
    {
        if (!IsOpened())
            return;
    }

    if (pkCharacter->GetViewingOfflineShop() && pkCharacter->GetViewingOfflineShop() != this)
    {
        pkCharacter->GetViewingOfflineShop()->RemoveViewer(pkCharacter);
        m_setViewers.erase(pkCharacter->GetPlayerID());
        pkCharacter->SetViewingOfflineShop(nullptr);
    }
    {
        TPacketGCOfflineShop kPacket;
        kPacket.bySubHeader = CLEAR_SHOP;

        pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP, kPacket);
    }
    {
        for (auto it = m_mapItems.begin(); it != m_mapItems.end(); ++it)
        {
            TPacketGCOfflineShopAddItem kPacket;
            kPacket.data = it->second.info;

            kPacket.dwDisplayPosition = it->second.dwPosition;
            kPacket.llPrice = it->second.llPrice;

            pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP_ITEM_ADD, kPacket);
        }
    }
    if (pkCharacter->GetPlayerID() == GetOwnerPID())
    {
        SendGoldPacket(pkCharacter);
        {
            TPacketGCOfflineShopName kPacket;
            kPacket.szName = GetName();

            pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP_NAME, kPacket);
        }

#ifdef __OFFLINE_SHOP_OPENING_TIME__
        SendLeftOpeningTimePacket(pkCharacter);
#else
        SendOpeningStatePacket(pkCharacter);
#endif

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
        {
            TPacketGCOfflineShopEditorPositionInfo kPacket;
            kPacket.lMapIndex = GetMapIndex();
            kPacket.lX = GetX();
            kPacket.lY = GetY();
            kPacket.byChannel = GetChannel();

            pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP_EDITOR_POSITION_INFO, kPacket);
        }
#endif
    }
    {
        TPacketGCOfflineShop kPacket;
        kPacket.bySubHeader = (pkCharacter->GetPlayerID() == GetOwnerPID()) ? OPEN_MY_SHOP : OPEN_SHOP;

        pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP, kPacket);
    }

    pkCharacter->SetViewingOfflineShop(this);
    if(pkCharacter && pkCharacter->IsGM())
    {
        pkCharacter->ChatPacket(CHAT_TYPE_INFO, "<GM> The owner of this OfflineShop is called '%s'", GetOwnerName());   
    }
    m_setViewers.emplace(pkCharacter->GetPlayerID());
}

void COfflineShop::RemoveViewer(CHARACTER *pkCharacter)
{
    if (!pkCharacter || pkCharacter->GetViewingOfflineShop() != this)
        return;
    {
        TPacketGCOfflineShop kPacket;
        kPacket.bySubHeader = CLOSE_SHOP;

        pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP, kPacket);
    }
}

void COfflineShop::RemoveViewerFromSet(CHARACTER *pkCharacter)
{
    const auto it = m_setViewers.find(pkCharacter->GetPlayerID());
    if (it != m_setViewers.end())
    {
        m_setViewers.erase(it);
    }
}

void COfflineShop::BuyItem(CHARACTER *pkBuyer, uint32_t dwPosition, TItemPos *pkInventoryPosition)
{
    TOfflineShopItemData *pkItemInfo = FindItem(dwPosition);
    if (!IsOpened() || !pkItemInfo || !pkBuyer || pkBuyer->GetViewingOfflineShop() != this)
        return;

    if ((GetGold() + pkItemInfo->llPrice) > g_llOfflineShopMaxGold)
    {
        SendI18nChatPacket(pkBuyer, CHAT_TYPE_INFO, ("This shop has too much money. Wait until it gets withdrawed."));
        return;
    }

    if (pkBuyer->GetGold() < pkItemInfo->llPrice)
    {
        SendI18nChatPacket(pkBuyer, CHAT_TYPE_INFO, ("You don't have enough money!"));
        return;
    }

    const auto* pkItemTable = ITEM_MANAGER::instance().GetTable(pkItemInfo->info.vnum);
    if (!pkItemTable)
    {
        SPDLOG_ERROR("There is no item table for item {}.", pkItemInfo->info.vnum);
        return;
    }

    if (pkInventoryPosition)
    {

        if (pkItemTable->bType == ITEM_DS)
        {
            if (!pkBuyer->DragonSoul_IsQualified())
            {
                SendI18nChatPacket(pkBuyer, CHAT_TYPE_INFO,
                                   ("You are not qualified to buy dragon stone alchemy items."));
                return;
            }

            if (!pkBuyer->IsEmptyItemGridDS(*pkInventoryPosition, pkItemTable->bSize))
                return;

            if (DRAGON_SOUL_INVENTORY != pkInventoryPosition->window_type)
            {
                SendI18nChatPacket(pkBuyer, CHAT_TYPE_INFO, ("<Storages> No movement possible."));
                return;
            }

            if (!DSManager::instance().IsValidCellForThisItem(pkItemTable, *pkInventoryPosition))
            {
                const int iCell = pkBuyer->GetEmptyDragonSoulInventory(pkItemTable);
                if (iCell < 0)
                {
                    SendI18nChatPacket(pkBuyer, CHAT_TYPE_INFO, ("<Storages> No movement possible."));
                    return;
                }
                pkInventoryPosition->cell = iCell;
            }
        }
        else
        {
            if (!pkBuyer->IsEmptyItemGrid(*pkInventoryPosition, pkItemTable->bSize))
                return;

            if (DRAGON_SOUL_INVENTORY == pkInventoryPosition->window_type)
            {
                SendI18nChatPacket(pkBuyer, CHAT_TYPE_INFO, ("<Storages> No movement possible."));
                return;
            }
        }
    }
    else
    {
        if (pkItemTable->bType == ITEM_DS)
        {
            if (!pkBuyer->DragonSoul_IsQualified())
            {
                SendI18nChatPacket(pkBuyer, CHAT_TYPE_INFO,
                                   ("You are not qualified to buy dragon stone alchemy items."));
                return;
            }
        }
    }

    CItem *pkItem = ITEM_MANAGER::instance().CreateItem(pkItemInfo->info.vnum, pkItemInfo->info.count);
    if (!pkItem)
    {
        SPDLOG_ERROR("Failed to create item!");
        return;
    }

    pkItem->SetSkipSave(true);
    pkItem->SetSockets(pkItemInfo->info.sockets);
    pkItem->SetAttributes(pkItemInfo->info.attrs);
    pkItem->SetTransmutationVnum(pkItemInfo->info.transVnum);

#ifdef __ITEM_RUNE__
    pkItem->SetRune(pkItemInfo->dwRune);
#endif
    pkItem->SetSkipSave(false);

    SetGold(GetGold() + pkItemInfo->llPrice);
    pkBuyer->ChangeGold(-pkItemInfo->llPrice);

    if (pkInventoryPosition)
        pkItem->AddToCharacter(pkBuyer, *pkInventoryPosition);
    else
        pkBuyer->AutoGiveItem(pkItem);

#ifdef __OFFLINE_SHOP_LOGGING__
    LogManager::instance().OfflineShopLogBuyItem(GetOwnerPID(), pkBuyer->GetPlayerID(), pkItemInfo->info.vnum,
                                                 pkItemInfo->info.count, pkItemInfo->llPrice);
#endif
    {
        CHARACTER *pkOwner = g_pCharManager->FindByPID(GetOwnerPID());
        if (pkOwner)
        {
            SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, ("You sold %dx %s for %lld Yang."), pkItemInfo->info.count,
                               TextTag::itemname(pkItemInfo->info.vnum), pkItemInfo->llPrice);
        }
        else
        {
            GmOfflineShopBuyInfoPacket p;
            p.ownerPid = GetOwnerPID();
            p.vnum = pkItemInfo->info.vnum;
            p.count = pkItemInfo->info.count;
            p.price = pkItemInfo->llPrice;
            DESC_MANAGER::instance().GetMasterSocket()->Send(kGmOfflineShopBuyInfo, p);
        }
    }

    RemoveItem(dwPosition);
}

Gold COfflineShop::GetGold() const { return m_llGold; }

void COfflineShop::SetGold(Gold llGold, bool bFromDatabase)
{
    m_llGold = llGold;
    {
        for (const auto &viewerPid : m_setViewers)
        {
            if (viewerPid == GetOwnerPID())
            {
                auto* viewer = g_pCharManager->FindByPID(viewerPid);
                if (viewer)
                {
                    SendGoldPacket(viewer);
                }
                break;
            }
        }
    }
    if (!bFromDatabase)
    {
        TPacketOfflineShopUpdateGold kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();
        kPacket.llGold = GetGold();

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_GOLD, 0, &kPacket, sizeof(kPacket));
    }

    if (GetGold() < 1 && GetItemCount() < 1)
        COfflineShopManager::instance().DestroyOfflineShop(this);
}

void COfflineShop::WithdrawGold(CHARACTER *pkOwner, Gold llGold)
{
    if (!pkOwner || pkOwner->GetPlayerID() != GetOwnerPID() || llGold < 1)
        return;

    if (llGold > GetGold())
    {
        SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, ("The shop doesn't have enough money."));
        return;
    }

    if ((pkOwner->GetGold() + llGold) > static_cast<Gold>(gConfig.maxGold))
    {
        SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, ("You have too much gold in your inventory."));
        return;
    }

    pkOwner->ChangeGold(llGold);
    pkOwner->Save();
    g_pCharManager->FlushDelayedSave(pkOwner);
    SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, ("You've withdrawn %lld Yang from your shop sales stash."), llGold);

#ifdef __OFFLINE_SHOP_LOGGING__
    LogManager::instance().OfflineShopLogWithdrawGold(GetOwnerPID(), llGold);
#endif

    SetGold(GetGold() - llGold);
}

void COfflineShop::SetName(const char *c_szName, bool bFromDatabase)
{
    m_szName = c_szName;
    {
        if (GetShopKeeper())
        {
            TPacketGCOfflineShopAdditionalInfo kPacket;
            kPacket.dwVID = GetShopKeeper()->GetVID();
            kPacket.szShopName = GetName();
            kPacket.dwShopOwnerPID = GetOwnerPID();

            PacketAround(GetShopKeeper()->GetViewMap(), GetShopKeeper(), HEADER_GC_OFFLINE_SHOP_ADDITIONAL_INFO,
                         kPacket);
        }

        for (const auto &viewerPid : m_setViewers)
        {
            if (viewerPid != GetOwnerPID())
                continue;

            TPacketGCOfflineShopName kPacket;
            kPacket.szName = GetName();

            if (auto *viewer = g_pCharManager->FindByPID(viewerPid); viewer)
            {
                if (auto d = viewer->GetDesc(); d)
                    d->Send(HEADER_GC_OFFLINE_SHOP_NAME, kPacket);
            }

            break;
        }
    }
    if (!bFromDatabase)
    {
        TPacketOfflineShopUpdateName kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();
        strlcpy(kPacket.szName, c_szName, sizeof(kPacket.szName));

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_NAME, 0, &kPacket, sizeof(kPacket));
    }
}

const char *COfflineShop::GetName() const { return m_szName.c_str(); }

void COfflineShop::SetChannel(uint8_t byChannel) { m_byChannel = byChannel; }

uint8_t COfflineShop::GetChannel() const { return m_byChannel; }

void COfflineShop::SetPosition(long lMapIndex, long lX, long lY, bool bFromDatabase)
{
    m_lMapIndex = lMapIndex;
    m_lX = lX;
    m_lY = lY;

    if (GetShopKeeper())
    {
        DespawnKeeper();
        if (!SpawnKeeper())
            CloseShop();
    }
    if (!bFromDatabase)
    {
        TPacketGDOfflineShopUpdatePosition kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();
        kPacket.lMapIndex = GetMapIndex();
        kPacket.lX = GetX();
        kPacket.lY = GetY();

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_POSITION, 0, &kPacket, sizeof(kPacket));
    }
}

long COfflineShop::GetMapIndex() const { return m_lMapIndex; }

long COfflineShop::GetX() const { return m_lX; }

long COfflineShop::GetY() const { return m_lY; }

bool COfflineShop::IsNear(CHARACTER *pkCharacter) const
{
    if (!pkCharacter || GetChannel() != gConfig.channel || GetMapIndex() != pkCharacter->GetMapIndex())
        return false;

    if (!GetShopKeeper())
        return true;

    return (DISTANCE_APPROX(pkCharacter->GetX() - GetShopKeeper()->GetX(),
                            pkCharacter->GetY() - GetShopKeeper()->GetY()) <= g_iOfflineShopByNearMaxDistance);
}

bool COfflineShop::SpawnKeeper()
{
    if (!IsRealShop() || GetChannel() != gConfig.channel || GetItemCount() < 1)
        return false;

    if (!SECTREE_MANAGER::instance().GetMap(GetMapIndex()))
    {
        SPDLOG_ERROR("Not existent map %d", GetMapIndex());
        return false;
    }

    if (GetShopKeeper())
        g_pCharManager->DestroyCharacter(GetShopKeeper());

    const std::string strName{GetOwnerName()};

    m_pkShopKeeper = g_pCharManager->SpawnMob(g_dwOfflineShopKeeperVNum, GetMapIndex(), GetX(), GetY(), 0, false, 0,
                                              false, false, 100, true);
    GetShopKeeper()->SetCharType(CHAR_TYPE_SHOP);
    GetShopKeeper()->SetName(strName);
    GetShopKeeper()->SetMyOfflineShop(this);
    GetShopKeeper()->Show(GetMapIndex(), GetX(), GetY(), 0, true);

    return true;
}

void COfflineShop::DespawnKeeper()
{
    if (GetShopKeeper())
        g_pCharManager->DestroyCharacter(GetShopKeeper());

    m_pkShopKeeper = nullptr;
}

uint32_t COfflineShop::GetOwnerPID() const { return m_dwOwnerPID; }

const char *COfflineShop::GetOwnerName() const { return m_szOwnerName; }

void COfflineShop::OpenShop(bool bSendToDatabase)
{
    if (IsOpened()
#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
        && GetShopKeeper()
#endif
    )
        return;
    if (bSendToDatabase)
    {
        TPacketOfflineShopOpen kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();
#ifdef __OFFLINE_SHOP_OPENING_TIME__
        kPacket.iOpeningTime = GetLeftOpeningTime();
#endif

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_OPEN, 0, &kPacket, sizeof(kPacket));
    }

    SpawnKeeper();

#ifdef __OFFLINE_SHOP_OPENING_TIME__
    StartCloseEvent();
#else
    SetOpeningState(true);
#endif
}

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
void COfflineShop::CloseShop(bool bFromDatabase, bool bEndedByEvent)
#else
void COfflineShop::CloseShop(bool bFromDatabase)
#endif
{
    if (!IsOpened()
#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
        && !bEndedByEvent
#endif
    )
        return;

    if (!bFromDatabase)
    {
        TPacketOfflineShopClose kPacket;
        kPacket.dwOwnerPID = GetOwnerPID();

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_CLOSE, 0, &kPacket, sizeof(kPacket));
    }

    const auto ownerPid = GetOwnerPID();
    for (const auto viewerPid : m_setViewers)
    {
        if (viewerPid == GetOwnerPID())
            continue;

        if (const auto viewer = g_pCharManager->FindByPID(viewerPid); viewer)
        {
            RemoveViewer(viewer);
            viewer->SetViewingOfflineShop(nullptr);
        }
    }

    for (auto i = m_setViewers.begin(), last = m_setViewers.end(); i != last;)
    {
        if ((*i) != ownerPid)
        {
            i = m_setViewers.erase(i);
        }
        else
        {
            ++i;
        }
    }

    DespawnKeeper();

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    if (bEndedByEvent)
    {
        m_pkCloseEvent = nullptr;

#ifndef __OFFLINE_SHOP_FARM_OPENING_TIME__
        {
            CHARACTER *pkOwner = g_pCharManager->FindByPID(GetOwnerPID());
            if (pkOwner)
                SendLeftOpeningTimePacket(pkOwner);
        }
#endif
    }
    else
    {
        StopCloseEvent();
    }
#endif

#ifndef __OFFLINE_SHOP_OPENING_TIME__
    SetOpeningState(false);
#endif

    if (GetGold() < 1 && GetItemCount() < 1)
        COfflineShopManager::instance().DestroyOfflineShop(this);
}

CHARACTER *COfflineShop::GetShopKeeper() const { return m_pkShopKeeper; }

void COfflineShop::SetRealShop(bool bIsRealShop) { m_bIsRealShop = bIsRealShop; }

bool COfflineShop::IsRealShop() const { return m_bIsRealShop; }

bool COfflineShop::IsOpened() const
{
#if defined(__OFFLINE_SHOP_OPENING_TIME__)
    return (GetLeftOpeningTime() > 0);
#else
    return m_bOpen;
#endif
}

#ifdef __OFFLINE_SHOP_CLOSE_ITEMS_TO_INVENTORY__
bool COfflineShop::CanRemoveAllItems(CHARACTER *pkOwner)
{
    if (!pkOwner)
        return false;

    static CGrid s_aGrids[INVENTORY_PAGE_COUNT] = {
        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT), CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),
        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT), CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),
        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),

        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT), CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),
        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT), CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),
        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),

        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT), CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),
        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT), CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),
        CGrid(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT),
    };

    static int s_iSlotsPerPage = INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT;

    for (int i = 0; i < INVENTORY_PAGE_COUNT; ++i)
    {
        CGrid &kGrid = s_aGrids[i];

        kGrid.Clear();

        for (int j = (s_iSlotsPerPage * i); j < (s_iSlotsPerPage * (i + 1)); ++j)
        {
            CItem *pkItem = pkOwner->GetInventoryItem(j);
            if (!pkItem)
                continue;

            kGrid.Put(j, 1, pkItem->GetSize());
        }
    }

    static std::vector<uint16_t> s_vecDragonSoulGrid(DRAGON_SOUL_INVENTORY_MAX_NUM);
    pkOwner->CopyDragonSoulItemGrid(s_vecDragonSoulGrid);

    for (auto it = m_mapItems.begin(); it != m_mapItems.end(); ++it)
    {
        const auto* pkItemTable = ITEM_MANAGER::instance().GetTable(it->second.info.vnum);
        if (!pkItemTable)
            continue;

        if (pkItemTable->bType == ITEM_DS)
        {
            if (!pkOwner->DragonSoul_IsQualified())
            {
                return false;
            }

            bool bExistEmptySpace = false;
            const auto wBasePos = DSManager::instance().GetBasePosition(pkItemTable);
            if (wBasePos >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                return false;

            for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; i++)
            {
                uint16_t wPos = wBasePos + i;
                if (0 == s_vecDragonSoulGrid[wBasePos])
                {
                    bool bEmpty = true;
                    for (int j = 1; j < pkItemTable->bSize; j++)
                    {
                        if (s_vecDragonSoulGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM])
                        {
                            bEmpty = false;
                            break;
                        }
                    }
                    if (bEmpty)
                    {
                        for (int j = 0; j < pkItemTable->bSize; j++)
                        {
                            s_vecDragonSoulGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM] = wPos + 1;
                        }
                        bExistEmptySpace = true;
                        break;
                    }
                }
                if (bExistEmptySpace)
                    break;
            }
            if (!bExistEmptySpace)
                return false;
        }
        else
        {
            int iPos = -1;

            for (int i = 0; i < INVENTORY_PAGE_COUNT; ++i)
            {
                CGrid &kGrid = s_aGrids[i];

                iPos = kGrid.FindBlank(1, pkItemTable->bSize);

                if (iPos >= 0)
                {
                    kGrid.Put(iPos, 1, pkItemTable->bSize);
                    break;
                }
            }

            if (iPos < 0)
                return false;
        }
    }

    return true;
}

bool COfflineShop::RemoveAllItems(CHARACTER *pkOwner)
{
    if (!pkOwner || !CanRemoveAllItems(pkOwner))
        return false;

    TMapItemByPosition mapItemsTmp(m_mapItems);
    for (auto it = mapItemsTmp.begin(); it != mapItemsTmp.end(); ++it)
    {
        TOfflineShopItemData *pkItemInfo = &(it->second);

        CItem *pkItem = ITEM_MANAGER::instance().CreateItem(pkItemInfo->info.vnum, pkItemInfo->info.count);
        if (!pkItem)
            return false;

        pkItem->SetSockets(pkItemInfo->info.sockets);
        pkItem->SetAttributes(pkItemInfo->info.attrs);
        pkItem->SetTransmutationVnum(pkItemInfo->info.transVnum);

#ifdef __ITEM_RUNE__
        pkItem->SetRune(pkItemInfo->dwRune);
#endif

        pkOwner->AutoGiveItem(pkItem);

        RemoveItem(it->first);
    }

    return true;
}
#endif

void COfflineShop::SendGoldPacket(CHARACTER *pkOwner) const
{
    if (!pkOwner)
        return;

    TPacketGCOfflineShopGold kPacket;
    kPacket.llGold = GetGold();

    pkOwner->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP_GOLD, kPacket);
}

const COfflineShop::TMapItemByPosition &COfflineShop::GetItemMap() const { return m_mapItems; }

TOfflineShopItemData *COfflineShop::FindItem(uint32_t dwPosition)
{
    auto it = m_mapItems.find(dwPosition);
    if (it == m_mapItems.end())
        return nullptr;

    return &it->second;
}

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
EVENTINFO(offline_shop_close_info)
{
    COfflineShop *pkOfflineShop;
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    uint32_t dwBufferSeconds;
#endif

    offline_shop_close_info()
        : pkOfflineShop(nullptr), dwBufferSeconds{}
    {
    }
};

EVENTFUNC(offline_shop_close_event)
{
    offline_shop_close_info *pkInfo = dynamic_cast<offline_shop_close_info *>(event->info);
    if (!pkInfo)
    {
        SPDLOG_ERROR("offline_shop_close_event :: info is a null pointer!");
        return 0;
    }

    if (!pkInfo->pkOfflineShop)
        return 0;

    if (!pkInfo->pkOfflineShop->IsRealShop())
    {
        SPDLOG_ERROR("Event ticked for managing purpose mirrored offline shop (Owner: %d)!",
                     pkInfo->pkOfflineShop->GetOwnerPID());
        return 0;
    }

    if (pkInfo->dwBufferSeconds > 0)
    {
        pkInfo->dwBufferSeconds -= 1;
    }
    else
    {
        const int iLeftSeconds = std::max(0, pkInfo->pkOfflineShop->GetLeftOpeningTime());
        if (iLeftSeconds % (10 * 60) == 0)
        {
            TPacketGDOfflineShopFlushLeftOpeningTime kPacket;
            kPacket.dwOwnerPID = pkInfo->pkOfflineShop->GetOwnerPID();
            kPacket.iLeftOpeningTime = iLeftSeconds;

            db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_FLUSH_LEFT_OPENING_TIME, 0, &kPacket, sizeof(kPacket));
        }

        if (iLeftSeconds <= 0)
        {
            pkInfo->pkOfflineShop->CloseShop(false, true);
            return 0;
        }
        else
        {
            pkInfo->pkOfflineShop->SetOpeningTime(iLeftSeconds - 1);
        }
    }

    return THECORE_SECS_TO_PASSES(1);
}

void COfflineShop::SetOpeningTime(int iSeconds) { m_iOpeningTime = iSeconds; }

int COfflineShop::GetLeftOpeningTime() const { return m_iOpeningTime; }

void COfflineShop::StartCloseEvent()
{
    if (!IsRealShop())
        return;

    offline_shop_close_info *pkInfo = AllocEventInfo<offline_shop_close_info>();

    pkInfo->pkOfflineShop = this;
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    pkInfo->dwBufferSeconds = 10;
#endif

    m_pkCloseEvent = event_create(offline_shop_close_event, pkInfo, THECORE_SECS_TO_PASSES(1));

#ifndef __OFFLINE_SHOP_FARM_OPENING_TIME__
    {
        CHARACTER *pkOwner = g_pCharManager->FindByPID(GetOwnerPID());
        if (pkOwner)
            SendLeftOpeningTimePacket(pkOwner);
    }
#endif
}

void COfflineShop::StopCloseEvent()
{
    if (m_pkCloseEvent)
        event_cancel(&m_pkCloseEvent);

    m_pkCloseEvent = nullptr;

#ifndef __OFFLINE_SHOP_FARM_OPENING_TIME__
    SetOpeningTime(0);
    {
        CHARACTER *pkOwner = g_pCharManager->FindByPID(GetOwnerPID());
        if (pkOwner)
            SendLeftOpeningTimePacket(pkOwner);
    }
#endif
}

#ifndef __OFFLINE_SHOP_FARM_OPENING_TIME__
void COfflineShop::SendLeftOpeningTimePacket(CHARACTER *pkOwner)
{
    if (!pkOwner)
        return;

    TPacketGCOfflineShopLeftOpeningTime kPacket;
    kPacket.byHeader = HEADER_GC_OFFLINE_SHOP_LEFT_OPENING_TIME;
    kPacket.iLeftOpeningTime = GetLeftOpeningTime();

    pkOwner->GetDesc()->Packet(&kPacket, sizeof(kPacket));
}
#endif
#endif
#if !defined(__OFFLINE_SHOP_OPENING_TIME__)
void COfflineShop::SetOpeningState(bool bOpen)
{
    m_bOpen = bOpen;
    {
        CHARACTER *pkOwner = g_pCharManager->FindByPID(GetOwnerPID());
        if (pkOwner)
            SendOpeningStatePacket(pkOwner);
    }
}

void COfflineShop::SendOpeningStatePacket(CHARACTER *pkCharacter) const
{
    TPacketGCOfflineShop kPacket;
    kPacket.bySubHeader = IsOpened() ? OPEN_MY_SHOP_FOR_OTHERS : CLOSE_MY_SHOP_FOR_OTHERS;

    pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP, kPacket);
}
#endif

#endif
