#include <game/DbPackets.hpp>

#include "questmanager.h"
#ifdef __OFFLINE_SHOP__
#include "input.h"
#include "OfflineShop.h"
#include "OfflineShopManager.h"
#include "OfflineShopConfig.h"
#include "config.h"
#include "desc.h"
#include "item.h"
#include "char.h"

#include "char_manager.h"
#include <cctype>

static bool OfflineShopCheckConfig(int iConfigValue, CHARACTER *pkCharacter, COfflineShop *pkOfflineShop)
{
    switch (iConfigValue)
    {
    case OFFLINE_SHOP_EDITING_PRIVILEGE_DISALLOW:
        return false;
    case OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_BY_NEAR:
        if (!pkOfflineShop->IsNear(pkCharacter))
        {
            SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "You have to be near the shop.");
            return false;
        }
        break;
    case OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_BY_FAR:
        if (pkCharacter->GetMapIndex() != pkOfflineShop->GetMapIndex() ||
            pkOfflineShop->GetChannel() != gConfig.channel)
        {
            SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "You have to be near the shop.");
            return false;
        }
        break;
    case OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS:
        if (pkOfflineShop->GetChannel() != gConfig.channel)
        {
            SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "You have to be on the same map and channel as your shop.");
            return false;
        }
        break;
    case OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_CHANNELS:
        if (pkOfflineShop->GetMapIndex() != pkCharacter->GetMapIndex())
        {
            SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "You have to be on the same map as your shop.");
            return false;
        }
        break;
    case OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS:
        return true;
    }

    return true;
}

static bool IsValidName(const std::string &name)
{
    return std::find_if(name.begin(), name.end(), [](char c) { return !(c >= 0x20 && c <= 0x7E); }) == name.end();
}

void CInputMain::ReceiveOfflineShopPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShop& p)
{
    switch (p.bySubHeader)
    {
    case CREATE_MY_SHOP: {
        if (pkCharacter->GetViewingOfflineShop())
        {
            pkCharacter->GetViewingOfflineShop()->RemoveViewer(pkCharacter);
            pkCharacter->GetViewingOfflineShop()->RemoveViewerFromSet(pkCharacter);
            pkCharacter->SetViewingOfflineShop(nullptr);
        }

        if (pkCharacter->GetMyOfflineShop())
        {
            pkCharacter->GetMyOfflineShop()->AddViewer(pkCharacter);
            return;
        }
        else
        {
            TPacketGCOfflineShop kPacket;
            kPacket.bySubHeader = OPEN_MY_SHOP;
            pkCharacter->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP, kPacket);
        }
    }
    break;

    case CLOSE_MY_SHOP: {
        if (!pkCharacter->GetMyOfflineShop())
            return;

#ifdef __OFFLINE_SHOP_CLOSE_ITEMS_TO_INVENTORY__
        COfflineShop *pkOfflineShop = pkCharacter->GetMyOfflineShop();

        if (!pkOfflineShop)
            return;

        Gold llGold = pkOfflineShop->GetGold();
        if (!pkOfflineShop->RemoveAllItems(pkCharacter))
        {
            SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO,
                               "You need to have inventory space for all the items in your shop in order to close it.");
            return;
        }

        if (llGold > 0)
            pkOfflineShop->WithdrawGold(pkCharacter, llGold);
#else
				pkCharacter->GetMyOfflineShop()->CloseShop();
#endif
    }
    break;

    case CLOSE_OTHER_SHOP: {
        if (!pkCharacter->GetViewingOfflineShop())
            return;

        pkCharacter->GetViewingOfflineShop()->RemoveViewer(pkCharacter);
        pkCharacter->GetViewingOfflineShop()->RemoveViewerFromSet(pkCharacter);
        pkCharacter->SetViewingOfflineShop(nullptr);
    }
    break;

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
    case CANCEL_CREATE_MY_SHOP: {
        if (pkCharacter->IsBuildingOfflineShop()) { pkCharacter->SetBuildingOfflineShop(false); }
    }
    break;
#endif
    }
}

struct SOfflineShopItemInfo
{
    CItem *pkItem;
    uint32_t dwDisplayPosition;
    Gold llPrice;
};

int CInputMain::ReceiveOfflineShopCreatePacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopCreate& p)
{
    if (pkCharacter->GetExchange() || pkCharacter->IsOpenSafebox() ||
        pkCharacter->GetShopOwner() || pkCharacter->IsCubeOpen())
    {
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "You cannot open a storage if another Window is already open.");
        return true;
    }

    auto pkOfflineShop = pkCharacter->GetMyOfflineShop();
    if ((pkOfflineShop && pkOfflineShop->GetItemCount() > 0) || !pkCharacter->IsBuildingOfflineShop())
        return true;

    if (p.dwItemCount < 1)
        return true;

    std::string name(p.szShopName);
    if (!IsValidName(name))
    {
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO,
                           "The shop name cannot contain language specific special characters.");
        return true;
    }


    std::vector<SOfflineShopItemInfo> s_vecItems;
    s_vecItems.clear();
    s_vecItems.resize(p.items.size());

    

    auto llTotalGold = pkCharacter->GetGold() + (pkOfflineShop ? pkOfflineShop->GetGold() : 0);

    for (unsigned int i = 0; i < p.items.size(); ++i)
    {
        const auto& c_pkItemInfo = p.items[i];
        CItem *pkItem = pkCharacter->GetItem(c_pkItemInfo.kInventoryPosition);
        if (!pkItem)
            continue;

        if (pkItem->IsExchanging() ||
            pkItem->IsEquipped() || pkItem->IsSealed() ||
            IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_MYSHOP))
        {
            SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO,
                               "You tried to put at least one item into the shop which can not be traded yet.");
            return true;
        }
        else if (c_pkItemInfo.llPrice < 1)
        {
            SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "The lowest price an item can have is 1 gold.");
            return true;
        }

        SOfflineShopItemInfo &rkItemInfo = s_vecItems[i];
        rkItemInfo.pkItem = pkItem;
        rkItemInfo.dwDisplayPosition = c_pkItemInfo.dwDisplayPosition;
        rkItemInfo.llPrice = c_pkItemInfo.llPrice;

        llTotalGold += rkItemInfo.llPrice;
    }

    if (llTotalGold > pkCharacter->GetAllowedGold())
    {
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO,
                           "You can not open your shop because you would earn more than %lld gold.",
                           pkCharacter->GetAllowedGold());
        return true;
    }

    auto lX = pkCharacter->GetX();
    auto lY = pkCharacter->GetY();

    if (!pkOfflineShop)
    {
        pkOfflineShop = COfflineShopManager::instance().CreateOfflineShop(pkCharacter, p.szShopName.c_str(), lX, lY);
        if (!pkOfflineShop)
            return true;
    }
    else
    {
        pkOfflineShop->SetChannel(gConfig.channel);
        pkOfflineShop->SetName(p.szShopName.c_str());
        pkOfflineShop->SetPosition(pkCharacter->GetMapIndex(), lX, lY);
    }

    COfflineShopManager::instance().RegisterListener(pkOfflineShop->GetOwnerPID(), false);

    for (auto &s_vecItem : s_vecItems)
    {
        pkOfflineShop->AddItem(s_vecItem.pkItem, s_vecItem.dwDisplayPosition, s_vecItem.llPrice);
    }

    pkCharacter->SetMyOfflineShop(pkOfflineShop);

    pkOfflineShop->SetOpeningState(true);
    pkOfflineShop->OpenShop();

    if (p.bIsShowEditor)
        pkOfflineShop->AddViewer(pkCharacter);

    return true;
}

void CInputMain::ReceiveOfflineShopNamePacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopName &p)
{
    COfflineShop *pkOfflineShop = pkCharacter->GetMyOfflineShop();
    if (!pkOfflineShop)
        return;

    if (!OfflineShopCheckConfig(g_iOfflineShopEditNamePrivilege, pkCharacter, pkOfflineShop))
        return;

    if (!IsValidName(p.szShopName))
    {
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO,
                           "The shop name cannot contain language specific special characters.");
        return;
    }

    if(p.szShopName.length() < 3 || p.szShopName.length() > 32) {
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO,
                           "The shop name must be atleast 3 characters and at maximum 32 characters long.");
        return;
    }

#ifdef __OFFLINE_SHOP_CHANGE_NAME_DELAY__
    static int s_iDelayTime = 1 * 60 * 60;
    int iTimeDiff = pkCharacter->GetQuestFlag("offline_shop.last_name_change") + s_iDelayTime - (int)time(nullptr);
    if (iTimeDiff > 0)
    {
        int iHours = iTimeDiff / 3600;
        int iMinutes = iTimeDiff % 3600 / 60;
        int iSeconds = iTimeDiff % 60;
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO,
                           "You can only change your shop name once every hour (%02d:%02d:%02d).", iHours, iMinutes,
                           iSeconds);
        return;
    }

    pkCharacter->SetQuestFlag("offline_shop.last_name_change", time(nullptr));

#endif

    pkOfflineShop->SetName(p.szShopName.c_str());
}

#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
void CInputMain::ReceiveOfflineShopPositionPacket(CHARACTER* pkCharacter, TPacketCGOfflineShopPosition* pkData)
{
	COfflineShop* pkOfflineShop = pkCharacter->GetMyOfflineShop();
	if (!pkOfflineShop)
		return;

	pkOfflineShop->SetPosition(pkCharacter->GetMapIndex(), ((long)p.fX), ((long)p.fY));
}
#endif

void CInputMain::ReceiveOfflineShopAddItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopAddItem& p)
{
    COfflineShop *pkOfflineShop = pkCharacter->GetMyOfflineShop();
    if (!pkOfflineShop)
        return;

    if (!OfflineShopCheckConfig(g_iOfflineShopAddItemsPrivilege, pkCharacter, pkOfflineShop))
        return;

    CItem *pkItem = pkCharacter->GetItem(p.kInventoryPosition);
    if (!pkItem)
        return;

    pkOfflineShop->AddItem(pkItem, p.dwDisplayPosition, p.llPrice);
}

void CInputMain::ReceiveOfflineShopMoveItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopMoveItem& p)
{
    COfflineShop *pkOfflineShop = pkCharacter->GetMyOfflineShop();
    if (!pkOfflineShop)
        return;

    if (!OfflineShopCheckConfig(g_iOfflineShopMoveItemsPrivilege, pkCharacter, pkOfflineShop))
        return;

    pkOfflineShop->MoveItem(p.dwOldDisplayPosition, p.dwNewDisplayPosition);
}

void CInputMain::ReceiveOfflineShopRemoveItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopRemoveItem& p)
{
    COfflineShop *pkOfflineShop = pkCharacter->GetMyOfflineShop();
    if (!pkOfflineShop)
        return;

    if (!OfflineShopCheckConfig(g_iOfflineShopRemoveItemsPrivilege, pkCharacter, pkOfflineShop))
        return;

    if (!p.kInventoryPosition.IsDefaultInventoryPosition())
        pkOfflineShop->RemoveItemByOwner(p.dwDisplayPosition);
    else
        pkOfflineShop->RemoveItemByOwner(p.dwDisplayPosition, const_cast<TItemPos*>(&p.kInventoryPosition));
}

void CInputMain::ReceiveOfflineShopBuyItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopBuyItem& p)
{
    COfflineShop *pkOfflineShop = pkCharacter->GetViewingOfflineShop();
    if (!pkOfflineShop)
        return;

    pkOfflineShop->BuyItem(pkCharacter, p.dwDisplayPosition);
}

void CInputMain::ReceiveOfflineShopWithdrawGoldPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopWithdrawGold& p)
{
    COfflineShop *pkOfflineShop = pkCharacter->GetMyOfflineShop();
    if (!pkOfflineShop)
        return;

    if (!OfflineShopCheckConfig(g_iOfflineShopWithdrawGoldPrivilege, pkCharacter, pkOfflineShop))
        return;

    pkOfflineShop->WithdrawGold(pkCharacter, p.llGold);
}

void CInputMain::ReceiveOfflineShopClickShopPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopClickShop& p)
{
    COfflineShop *pkOfflineShop = COfflineShopManager::instance().FindOfflineShop(p.dwShopOwnerPID);
    if (!pkOfflineShop)
        return;
    
    if (!OfflineShopCheckConfig(
            pkCharacter->GetPlayerID() == pkOfflineShop->GetOwnerPID()
                ? g_iOfflineShopViewShopAsOwnerPrivilege
                : g_iOfflineShopViewShopAsGuestPrivilege,
            pkCharacter,
            pkOfflineShop)
    )
        return;

    pkOfflineShop->AddViewer(pkCharacter);
}

void CInputMain::ReceiveOfflineShopReopenPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopReopen& p)
{
    if (pkCharacter->GetLevel() < 55)
    {
        SendI18nChatPacket(pkCharacter, CHAT_TYPE_INFO, "You are not allowed to create offline shops yet.");
        return;
    }

    COfflineShop *pkOfflineShop = pkCharacter->GetMyOfflineShop();
    if (!pkOfflineShop)
        return;

    if (pkOfflineShop->IsOpened())
        return;

#ifdef __OFFLINE_SHOP_OPENING_TIME__
	pkOfflineShop->SetOpeningTime(p.iOpeningTime);
#endif
    pkOfflineShop->OpenShop(true);
}
#endif
