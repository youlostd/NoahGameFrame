#ifndef METIN2_CLIENT_MAIN_PYTHONOFFLINESHOP_H
#define METIN2_CLIENT_MAIN_PYTHONOFFLINESHOP_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef __OFFLINE_SHOP__
#include <game/GamePacket.hpp>
#include "GameType.h"
#include "Locale.h"

class CPythonOfflineShop : public CSingleton<CPythonOfflineShop>
{
public:
    CPythonOfflineShop();
    virtual ~CPythonOfflineShop();
public:
   
    void SendCreateMyShopPacket();
    void SendCloseMyShopPacket();
    void SendCloseOtherShopPacket();

    void SendCreatePacket(
        const char *c_szShopName,
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
		    float fX, float fY,
#endif
        bool bIsShowEditor,
        std::vector<TOfflineShopCGItemInfo> *pvecItems
#ifdef __OFFLINE_SHOP_OPENING_TIME__
		    , int iOpeningTime
#endif
        );

    void SendShopNamePacket(const char *c_szShopName);
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
		void	SendShopPositionPacket(float fX, float fY);
#endif
    void SendAddItemPacket(TItemPos *pInventoryPosition, uint32_t dwDisplayPosition, LONGLONG llPrice);
    void SendMoveItemPacket(uint32_t dwOldDisplayPosition, uint32_t dwNewDisplayPosition);
    void SendRemoveItemPacket(uint32_t dwDisplayPosition, TItemPos *pkInventoryPosition);
    void SendBuyItemPacket(uint32_t dwDisplayPosition, TItemPos *pkInventoryPosition);
    void SendWithdrawGoldPacket(LONGLONG llGold);

    void SendClickShopPacket(uint32_t dwShopOwnerPID);

#ifdef __OFFLINE_SHOP_OPENING_TIME__
		void	SendReopenShopPacket(int iOpeningTime);
#else
    void SendReopenShopPacket();
#endif

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
    void SendCancelCreateShopPacket();
#endif
public:

    bool ReceiveOfflineShopPacket(const TPacketGCOfflineShop &p);
    bool ReceiveAdditionalInfoPacket(const TPacketGCOfflineShopAdditionalInfo &p);
    bool ReceiveAddItemPacket(const TPacketGCOfflineShopAddItem &kPacket);
    bool ReceiveMoveItemPacket(const TPacketGCOfflineShopMoveItem &kPacket);
    bool ReceiveRemoveItemPacket(const TPacketGCOfflineShopRemoveItem &kPacket);
    bool ReceiveGoldPacket(const TPacketGCOfflineShopGold &kPacket);
    bool ReceiveNamePacket(const TPacketGCOfflineShopName &kPacket);
#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    bool ReceiveLeftOpeningTimePacket(const TPacketGCOfflineShopLeftOpeningTime &kPacket);
#endif
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    bool ReceiveOfflineShopEditorPositionInfoPacket(const TPacketGCOfflineShopEditorPositionInfo &kPacket);
#endif
public:
    void SetPythonHandler(PyObject *poHandler);

private:
    PyObject *m_poHandler;
};
#endif
#endif /* METIN2_CLIENT_MAIN_PYTHONOFFLINESHOP_H */
