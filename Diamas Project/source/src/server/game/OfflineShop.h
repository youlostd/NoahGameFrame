#ifndef METIN2_SERVER_GAME_OFFLINESHOP_H
#define METIN2_SERVER_GAME_OFFLINESHOP_H

#include <Config.hpp>


#include "char.h"
#include "char_manager.h"
#include "desc.h"
#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef __OFFLINE_SHOP__
#include <game/DbPackets.hpp>

class CItem;
class CGrid;

class COfflineShop
{
private:
    typedef std::map<uint32_t, TOfflineShopItemData> TMapItemByPosition;
    typedef std::unordered_set<uint32_t> TSetViewers;

public:
    COfflineShop(uint32_t dwOwnerPID, const char *c_szOwnerName, const char *c_szName, long lMapIndex, long lX,
                 long lY);
    COfflineShop(TOfflineShopData *pkData);
    ~COfflineShop();

    void AddItem(TOfflineShopItemData *pkItem);
    void AddItem(CItem *pkItem, uint32_t dwPosition, Gold llPrice);
    void RemoveItem(uint32_t dwPosition, bool bFromDatabase = false);
    void RemoveItemByOwner(uint32_t dwPosition, TItemPos *pkInventoryPosition = nullptr);
    void MoveItem(uint32_t dwCurrentPosition, uint32_t dwNewPosition, bool bFromDatabase = false);
    int GetItemCount() const;

    void AddViewer(CHARACTER *pkCharacter);
    void RemoveViewer(CHARACTER *pkCharacter);
    void RemoveViewerFromSet(CHARACTER *pkCharacter);

    void BuyItem(CHARACTER *pkBuyer, uint32_t dwPosition, TItemPos *pkInventoryPosition = nullptr);

    Gold GetGold() const;
    void SetGold(Gold llGold, bool bFromDatabase = false);
    void WithdrawGold(CHARACTER *pkOwner, Gold llGold);

    void SetName(const char *c_szName, bool bFromDatabase = false);
    const char *GetName() const;

    void SetChannel(uint8_t byChannel);
    uint8_t GetChannel() const;
    void SetPosition(long lMapIndex, long lX, long lY, bool bFromDatabase = false);
    long GetMapIndex() const;
    long GetX() const;
    long GetY() const;
    bool IsNear(CHARACTER *pkCharacter) const;

    bool SpawnKeeper();
    void DespawnKeeper();

    uint32_t GetOwnerPID() const;
    const char *GetOwnerName() const;

    void OpenShop(bool bSendToDatabase = false);
#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    void CloseShop(bool bFromDatabase = false, bool bEndedByEvent = false);
#else
	void		CloseShop(bool bFromDatabase = false);
#endif
    CHARACTER *GetShopKeeper() const;

    void SetRealShop(bool bIsRealShop);
    bool IsRealShop() const;

    bool IsOpened() const;

#ifdef __OFFLINE_SHOP_CLOSE_ITEMS_TO_INVENTORY__
    bool CanRemoveAllItems(CHARACTER *pkOwner);
    bool RemoveAllItems(CHARACTER *pkOwner);
#endif

public:
    void SendGoldPacket(CHARACTER *pkOwner) const;
    const TMapItemByPosition &GetItemMap() const;

private:
    TOfflineShopItemData *FindItem(uint32_t dwPosition);

    template<typename T>
    void SendPacketToViewer(PacketId id, const T& p);

private:
    uint32_t m_dwOwnerPID;
    char m_szOwnerName[CHARACTER_NAME_MAX_LEN + 1] = {};
    std::string m_szName;

    TMapItemByPosition m_mapItems;
    CGrid *m_pkItemGrid;

    TSetViewers m_setViewers;

    Gold m_llGold;

    CHARACTER *m_pkShopKeeper;

    uint8_t m_byChannel;
    long m_lMapIndex;
    long m_lX, m_lY;

    bool m_bIsRealShop;

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
public:
    void SetOpeningTime(int iSeconds);
    int GetLeftOpeningTime() const;
    void StartCloseEvent();
    void StopCloseEvent();

#ifndef __OFFLINE_SHOP_FARM_OPENING_TIME__
	void	SendLeftOpeningTimePacket(CHARACTER* pkOwner);
#endif

private:
    int m_iOpeningTime;
    LPEVENT m_pkCloseEvent;
#endif
#if !defined(__OFFLINE_SHOP_OPENING_TIME__)
private:
    bool m_bOpen;

public:
    void SetOpeningState(bool bOpen);
    void SendOpeningStatePacket(CHARACTER *pkCharacter) const;
#endif

};

template <typename T>
void COfflineShop::SendPacketToViewer(PacketId id, const T& p) {

    for (const auto &viewerPid : m_setViewers)
    {
        if (const auto viewer = g_pCharManager->FindByPID(viewerPid); viewer)
        {
            if (const auto d = viewer->GetDesc(); d)
                d->Send(id, p);
        }
    }

}
#endif
#endif /* METIN2_SERVER_GAME_OFFLINESHOP_H */
