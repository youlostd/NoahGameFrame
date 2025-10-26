#ifndef METIN2_SERVER_GAME_SHOP_HPP
#define METIN2_SERVER_GAME_SHOP_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <boost/signals2/connection.hpp>
#include <game/ItemTypes.hpp>

#include "char.h"
#include "desc.h"
#include "net/Type.hpp"

class CHARACTER;
class CItem;

enum { SHOP_MAX_DISTANCE = 1000 };

enum ShopType {
    kShopTypeNpc,
    kShopTypePc,
};

class Shop
{
    public:
    Shop(uint32_t type);
    virtual ~Shop();

    // 게스트 추가/삭제
    virtual bool AddGuest(CHARACTER* ch, uint32_t ownerVid,
                          bool otherEmpire) = 0;
    void RemoveGuest(CHARACTER* ch);
    void RemoveAllGuests();

    // 물건 구입
    virtual int Buy(CHARACTER* ch, uint16_t pos, CountType amount = 1) = 0;

    uint32_t GetType() const;

    protected:
    struct ShopGuest {
        bool fromOtherEmpire;
    };

    typedef std::unordered_map<CHARACTER*, ShopGuest> GuestMap;

    template <typename T> void Broadcast(PacketId id, const T& p) const;

    uint32_t m_type;
    GuestMap m_guestMap;
};

template <typename T> void Shop::Broadcast(PacketId id, const T& p) const
{
    std::for_each(m_guestMap.begin(), m_guestMap.end(),
                  [id, p](const GuestMap::value_type& v) {
                      STORM_ASSERT(v.first->GetDesc(), "Not a player");
                      v.first->GetDesc()->Send(id, p);
                  });
}

class PcShop : public Shop
{
    public:
    PcShop();
    virtual ~PcShop();

    bool Create(CHARACTER* owner, const TShopItemTable* itemTable,
                uint8_t itemCount);

    virtual bool AddGuest(CHARACTER* ch, uint32_t ownerVid, bool otherEmpire);

    virtual int Buy(CHARACTER* ch, uint16_t pos, CountType amount = 1);

    void RemoveItem(const CItem* item);
    void RemoveItem(uint8_t pos);

    private:
    struct ShopItem {
        CItem* item;
        boost::signals2::scoped_connection onDestroy;
        Gold price;
    };

    void BroadcastItemRemove(uint8_t pos);

    CHARACTER* m_owner;
    ShopItem m_items[SHOP_HOST_ITEM_MAX_NUM];
};

class NpcShop : public Shop
{
    public:
    NpcShop();
    virtual ~NpcShop();

    void AddShopTable(const TShopTableEx& shopTable);

    virtual bool AddGuest(CHARACTER* ch, uint32_t ownerVid, bool otherEmpire);
    bool CheckFreeInventory(CHARACTER* ch, uint16_t pos, CountType amount,
                           const TItemTable* item);

    virtual int Buy(CHARACTER* ch, uint16_t pos, CountType amount = 1);

    size_t GetTabCount() { return m_vec_shopTabs.size(); }

    uint32_t GetTabCoinVnum(uint32_t tab)
    {
        return m_vec_shopTabs[tab].coinVnum;
    }

    private:
    std::vector<TShopTableEx> m_vec_shopTabs;
};

#endif
