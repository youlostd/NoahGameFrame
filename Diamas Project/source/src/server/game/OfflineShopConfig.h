#ifndef METIN2_SERVER_GAME_OFFLINESHOPCONFIG_H
#define METIN2_SERVER_GAME_OFFLINESHOPCONFIG_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

extern uint32_t g_dwOfflineShopKeeperVNum;
extern Gold g_llOfflineShopMaxGold;

extern int g_iOfflineShopSlotWidth;
extern int g_iOfflineShopSlotHeight;

extern std::map<uint8_t, std::vector<long>> g_mapOfflineShopRestrictedMapIndicesByEmpire;
extern bool OfflineShopIsAllowedMap(uint8_t byEmpire, long lMapIndex);

enum EOfflineShopEditingPrivileges
{
    OFFLINE_SHOP_EDITING_PRIVILEGE_DISALLOW,
    OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_BY_NEAR,
    OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_BY_FAR,
    OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS,
    OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_CHANNELS,
    OFFLINE_SHOP_EDITING_PRIVILEGE_ALLOW_ACROSS_MAPS_AND_CHANNELS,
};

extern int g_iOfflineShopByNearMaxDistance;

extern int g_iOfflineShopViewShopAsGuestPrivilege;
extern int g_iOfflineShopViewShopAsOwnerPrivilege;
extern int g_iOfflineShopEditNamePrivilege;
extern int g_iOfflineShopAddItemsPrivilege;
extern int g_iOfflineShopRemoveItemsPrivilege;
extern int g_iOfflineShopMoveItemsPrivilege;
extern int g_iOfflineShopWithdrawGoldPrivilege;

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
//#define OFFLINE_SHOP_MAX_FARM_TIME	24 * 60 * 60
#define OFFLINE_SHOP_MAX_FARM_TIME	60 * 60 * 24 * 365
#define OFFLINE_SHOP_MAX_SPECIAL_TIME	24 * 60 * 60
#endif
#endif /* METIN2_SERVER_GAME_OFFLINESHOPCONFIG_H */
