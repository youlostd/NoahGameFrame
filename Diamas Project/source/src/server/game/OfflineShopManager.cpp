#include <game/MasterPackets.hpp>
#include "desc_manager.h"

#ifdef __OFFLINE_SHOP__
#include "OfflineShopManager.h"
#include "OfflineShop.h"
#include "OfflineShopConfig.h"
#include "char.h"
#include "DbCacheSocket.hpp"
#include "char_manager.h"
#include "config.h"
#include "sectree_manager.h"

uint32_t g_dwOfflineShopKeeperVNum = 30008;
Gold g_llOfflineShopMaxGold = LLONG_MAX;

int g_iOfflineShopSlotWidth = 10;
int g_iOfflineShopSlotHeight = 9;

std::map<uint8_t, std::vector<long>> g_mapOfflineShopRestrictedMapIndicesByEmpire = {
    {EMPIRE_A, {1, 3, 21, 23, 41, 43}},
    {EMPIRE_B, {1, 3, 21, 23, 41, 43}},
    {EMPIRE_C, {1, 3, 21, 23, 41, 43}},
};

bool OfflineShopIsAllowedMap(uint8_t byEmpire, long lMapIndex)
{
    if (g_mapOfflineShopRestrictedMapIndicesByEmpire.empty())
        return true;

    std::map<uint8_t, std::vector<long>>::iterator itMap = g_mapOfflineShopRestrictedMapIndicesByEmpire.find(byEmpire);
    if (itMap != g_mapOfflineShopRestrictedMapIndicesByEmpire.end())
    {
        for (auto itVec = itMap->second.begin(); itVec != itMap->second.end(); ++itVec)
        {
            if ((*itVec) == lMapIndex)
            {
                return true;
            }
        }
    }

    return false;
}

int g_iOfflineShopByNearMaxDistance = 2500;

int g_iOfflineShopViewShopAsGuestPrivilege = OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_BY_FAR;
int g_iOfflineShopViewShopAsOwnerPrivilege = OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS;
int g_iOfflineShopEditNamePrivilege = OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS;
int g_iOfflineShopAddItemsPrivilege = OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS;
int g_iOfflineShopRemoveItemsPrivilege = OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS;
int g_iOfflineShopMoveItemsPrivilege = OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS;
int g_iOfflineShopWithdrawGoldPrivilege = OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS;

COfflineShopManager::COfflineShopManager() { }

COfflineShopManager::~COfflineShopManager() {}

void COfflineShopManager::RegisterListener(uint32_t dwShopOwnerPID, bool bNeedInfos)
{
    TPacketGDOfflineShopRegisterListener kPacket;
    kPacket.dwOwnerPID = dwShopOwnerPID;
    kPacket.bNeedOfflineShopInfos = bNeedInfos;

    db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_REGISTER_LISTENER, 0, &kPacket, sizeof(kPacket));
}

void COfflineShopManager::UnregisterListener(uint32_t dwShopOwnerPID)
{
    TPacketGDOfflineShopUnregisterListener kPacket;
    kPacket.dwOwnerPID = dwShopOwnerPID;

    db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_UNREGISTER_LISTENER, 0, &kPacket, sizeof(kPacket));
}

const COfflineShopManager::TMapOfflineShopByPID &COfflineShopManager::GetShopMap() const { return m_mapOfflineShops; }

void COfflineShopManager::CreateOfflineShops(TCreationDataVector &rvecOfflineShops)
{
    while (rvecOfflineShops.size() > 0)
    {
        TDataPair kData = rvecOfflineShops.back();
        rvecOfflineShops.pop_back();

        TOfflineShopData &rkData = kData.first;
        std::vector<TOfflineShopItemData> &rvecItems = kData.second;

        if (rkData.byChannel != gConfig.channel || !SECTREE_MANAGER::instance().GetMap(rkData.lMapIndex))
        {
            continue;
        }

        if (rvecItems.size() < 1 && rkData.llGold < 1)
        {
            TPacketOfflineShopDestroy kPacket;
            kPacket.dwOwnerPID = rkData.dwOwnerPID;

            db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_DESTROY, 0, &kPacket, sizeof(kPacket));
            continue;
        }

        COfflineShop *pkOfflineShop = COfflineShopManager::instance().CreateOfflineShop(&rkData, true);
        if (!pkOfflineShop)
        {
            continue;
        }

        COfflineShopManager::instance().RegisterListener(pkOfflineShop->GetOwnerPID(), false);

        while (!rvecItems.empty())
        {
            pkOfflineShop->AddItem(&rvecItems.back());
            rvecItems.pop_back();
        }
    }
}

COfflineShop *COfflineShopManager::CreateOfflineShop(TOfflineShopData *pkData, bool bIsRealShop)
{
    COfflineShop *pkOfflineShop = FindOfflineShop(pkData->dwOwnerPID);
    if (pkOfflineShop)
        return pkOfflineShop;

    pkOfflineShop = new COfflineShop(pkData);
    pkOfflineShop->SetRealShop(bIsRealShop);
    m_mapOfflineShops.insert(TMapOfflineShopByPID::value_type(pkData->dwOwnerPID, pkOfflineShop));

    return pkOfflineShop;
}

COfflineShop *COfflineShopManager::CreateOfflineShop(CHARACTER *pkOwner, const char *c_szName, long lX, long lY)
{
    if (!pkOwner || !pkOwner->IsPC())
        return nullptr;

    if (FindOfflineShop(pkOwner->GetPlayerID()) != nullptr)
        return nullptr;

    if (pkOwner->GetLevel() < 55)
    {
        SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, "You are not allowed to create offline shops yet.");
        return nullptr;
    }

    if (!OfflineShopIsAllowedMap(pkOwner->GetEmpire(), pkOwner->GetMapIndex()))
    {
        SendI18nChatPacket(pkOwner, CHAT_TYPE_INFO, "You are not allowed to create a shop on this map.");
        return nullptr;
    }
    {
        TPacketGDOfflineShopCreate kPacket;
        kPacket.dwOwnerPID = pkOwner->GetPlayerID();
        storm::CopyStringSafe(kPacket.szOwnerName, pkOwner->GetName());
        strlcpy(kPacket.szName, c_szName, sizeof(kPacket.szName));

        kPacket.lMapIndex = pkOwner->GetMapIndex();
        kPacket.lX = lX;
        kPacket.lY = lY;

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_CREATE, 0, &kPacket, sizeof(kPacket));
    }

    OfflineShopPtr pkOfflineShop = std::make_unique<COfflineShop>(pkOwner->GetPlayerID(), pkOwner->GetName().c_str(),
                                                                  c_szName, pkOwner->GetMapIndex(), lX, lY);
    auto *rawPtr = pkOfflineShop.get();
    pkOfflineShop->SetChannel(gConfig.channel);
    m_mapOfflineShops.emplace(pkOwner->GetPlayerID(), std::move(pkOfflineShop));

    return rawPtr;
}

COfflineShop *COfflineShopManager::FindOfflineShop(uint32_t dwPID, bool bTryToLoad)
{
    auto it = m_mapOfflineShops.find(dwPID);
    if (it == m_mapOfflineShops.end())
    {
        if (bTryToLoad)
            RegisterListener(dwPID, true);

        return nullptr;
    }

    return it->second.get();
}

COfflineShop *COfflineShopManager::FindOfflineShop(const char *c_szOwnerName)
{
    for (auto it = m_mapOfflineShops.begin(); it != m_mapOfflineShops.end(); ++it)
    {
        if (!strcmp(it->second->GetOwnerName(), c_szOwnerName))
            return it->second.get();
    }

    return nullptr;
}

void COfflineShopManager::DestroyOfflineShop(COfflineShop *pkOfflineShop, bool bFromDatabase, bool bGlobal)
{
    if (!bFromDatabase)
    {
        if (bGlobal)
        {
            TPacketOfflineShopDestroy kPacket;
            kPacket.dwOwnerPID = pkOfflineShop->GetOwnerPID();

            db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_DESTROY, 0, &kPacket, sizeof(kPacket));
        }
        else
        {
            UnregisterListener(pkOfflineShop->GetOwnerPID());
        }
    }

    pkOfflineShop->StopCloseEvent();
    m_mapOfflineShops.erase(pkOfflineShop->GetOwnerPID());
    
}

void COfflineShopManager::Destroy()
{
    for (auto it = m_mapOfflineShops.begin(); it != m_mapOfflineShops.end(); ++it)
    {
        UnregisterListener(it->second->GetOwnerPID());
    }

     m_mapOfflineShops.clear();
}

void COfflineShopManager::OpenAllShops()
{
    for (auto it = m_mapOfflineShops.begin(); it != m_mapOfflineShops.end(); ++it)
    {
        if (it->second->IsOpened())
        {
            it->second->SpawnKeeper();

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
            it->second->StartCloseEvent();
#endif
        }
    }
}

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
void COfflineShopManager::ReceiveP2PFarmOpeningTimePacket(const MgOfflineShopTimePacket &p)
{
    switch (p.type)
    {
    case OFFLINE_SHOP_FARM_OPENING_TIME_INFORMATION_LOGIN: {
        CHARACTER *pkCharacter = g_pCharManager->FindByPID(p.ownerPid);
        if (!pkCharacter)
            return;

        pkCharacter->SetOfflineShopFarmedOpeningTime(
            std::max(p.openingTime - pkCharacter->GetOfflineShopSpecialOpeningTime(), 0));
        pkCharacter->SetOfflineShopSpecialOpeningTime(p.openingTime - pkCharacter->GetOfflineShopFarmedOpeningTime());
        pkCharacter->SendLeftOpeningTimePacket();
    }
    break;

    case OFFLINE_SHOP_FARM_OPENING_TIME_INFORMATION_LOGOUT: {
        COfflineShop *pkOfflineShop = FindOfflineShop(p.ownerPid);
        if (!pkOfflineShop)
            return;

        pkOfflineShop->SetOpeningTime(p.openingTime);
        pkOfflineShop->StartCloseEvent();
    }
    break;

    default:
        break;
    }
}

void COfflineShopManager::LoginOwner(uint32_t dwPID, bool onEnter)
{
    COfflineShop *pkOfflineShop = FindOfflineShop(dwPID);
    if (!pkOfflineShop || !pkOfflineShop->IsRealShop())
        return;

    if (!onEnter)
    {
        CHARACTER *pkCharacter = g_pCharManager->FindByPID(dwPID);
        if (!pkCharacter)
        {
            SPDLOG_ERROR("LoginOwner got called without logged in owner (PID: {})!", dwPID);
            return;
        }

        pkCharacter->SetOfflineShopFarmedOpeningTime(
            std::max(pkOfflineShop->GetLeftOpeningTime() - pkCharacter->GetOfflineShopSpecialOpeningTime(), 0));
        pkCharacter->SetOfflineShopSpecialOpeningTime(pkOfflineShop->GetLeftOpeningTime() -
                                                      pkCharacter->GetOfflineShopFarmedOpeningTime());
        pkCharacter->SendLeftOpeningTimePacket();
    }
    else
    {
        GmOfflineShopTimePacket p{};
        p.type = OFFLINE_SHOP_FARM_OPENING_TIME_INFORMATION_LOGIN;
        p.ownerPid = pkOfflineShop->GetOwnerPID();
        p.openingTime = pkOfflineShop->GetLeftOpeningTime();
        DESC_MANAGER::instance().GetMasterSocket()->Send(kGmOfflineShopTime, p);
    }

    pkOfflineShop->StopCloseEvent();
    pkOfflineShop->SetOpeningTime(0);

    if (!pkOfflineShop->IsOpened())
        pkOfflineShop->OpenShop(true);
}

void COfflineShopManager::LogoutOwner(CHARACTER *pkCharacter)
{
    if (!pkCharacter)
        return;

    COfflineShop *pkOfflineShop = FindOfflineShop(pkCharacter->GetPlayerID());
    if (pkOfflineShop)
    {
        if (pkOfflineShop->IsRealShop())
        {
            pkOfflineShop->SetOpeningTime(pkCharacter->GetOfflineShopFarmedOpeningTime() +
                                          pkCharacter->GetOfflineShopSpecialOpeningTime());
            pkOfflineShop->StartCloseEvent();
        }
        else
        {
            GmOfflineShopTimePacket p{};
            p.type = OFFLINE_SHOP_FARM_OPENING_TIME_INFORMATION_LOGOUT;
            p.ownerPid = pkOfflineShop->GetOwnerPID();
            p.openingTime =
                pkCharacter->GetOfflineShopFarmedOpeningTime() + pkCharacter->GetOfflineShopSpecialOpeningTime();
            DESC_MANAGER::instance().GetMasterSocket()->Send(kGmOfflineShopTime, p);
        }
    }
}
#endif
#endif
