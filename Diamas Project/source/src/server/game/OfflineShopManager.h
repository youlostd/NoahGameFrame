#ifndef METIN2_SERVER_GAME_OFFLINESHOPMANAGER_H
#define METIN2_SERVER_GAME_OFFLINESHOPMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef __OFFLINE_SHOP__
#include <game/DbPackets.hpp>
#include <game/GamePacket.hpp>
#include <game/MasterPackets.hpp>
#include <base/Singleton.hpp>

class CHARACTER;
class DESC;
class COfflineShop;

using OfflineShopPtr = std::unique_ptr<COfflineShop>;
class COfflineShopManager : public singleton<COfflineShopManager>
{
private:
    typedef std::unordered_map<uint32_t, OfflineShopPtr> TMapOfflineShopByPID;

public:
    COfflineShopManager();
    ~COfflineShopManager();

    void RegisterListener(uint32_t dwShopOwnerPID, bool bNeedInfos);
    void UnregisterListener(uint32_t dwShopOwnerPID);
    const TMapOfflineShopByPID &GetShopMap() const;

    typedef std::pair<TOfflineShopData, std::vector<TOfflineShopItemData>> TDataPair;
    typedef std::vector<TDataPair> TCreationDataVector;
    void CreateOfflineShops(TCreationDataVector &rvecOfflineShops);
    COfflineShop *CreateOfflineShop(TOfflineShopData *pkData, bool bIsRealShop = false);

    COfflineShop *CreateOfflineShop(CHARACTER *pkOwner, const char *c_szName, long lX, long lY);
    COfflineShop *FindOfflineShop(uint32_t dwPID, bool bTryToLoad = false);
    COfflineShop *FindOfflineShop(const char *c_szOwnerName);

    void DestroyOfflineShop(COfflineShop *pkOfflineShop, bool bFromDatabase = false, bool bGlobal = true);
    void Destroy();
    void OpenAllShops();

private:
    TMapOfflineShopByPID m_mapOfflineShops;

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
public:
    void ReceiveP2PFarmOpeningTimePacket(const MgOfflineShopTimePacket &p);
    void LoginOwner(uint32_t dwPID, bool onEnter = false);
    void LogoutOwner(CHARACTER *pkCharacter);
#endif
};
#endif
#endif /* METIN2_SERVER_GAME_OFFLINESHOPMANAGER_H */
