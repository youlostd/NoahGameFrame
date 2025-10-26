#pragma once
#include <Config.hpp>
#include <game/GamePacket.hpp>

/*
 *	상점 처리
 *
 *	2003-01-16 anoa	일차 완료
 *	2003-12-26 levites 수정
 *
 *	2012-10-29 rtsummit 새로운 화폐 출현 및 tab 기능 추가로 인한 shop 확장.
 *
 */

class CPythonShop : public CSingleton<CPythonShop>
{
public:
    CPythonShop(void);
    virtual ~CPythonShop(void);

    struct ShopPosition
    {
        ShopPosition()
            : channel(0), mapIndex(0), x(0), y(0)
        {
        };

        int channel, mapIndex, x, y;
    };
    struct ShopTab
    {
        ShopTab();

        uint8_t coinType;
        uint32_t coinVnum;
        std::string name;
        packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
    };

    void Clear();

    void SetItemData(uint32_t slot, const packet_shop_item &item);
    const packet_shop_item *GetItemData(uint32_t slot) const;

    void SetItemData(uint8_t tab, uint32_t slot, const packet_shop_item &item);
    const packet_shop_item *GetItemData(uint8_t tab, uint32_t slot) const;

    void SetTabCount(uint8_t tabCount)
    {
        m_tabCount = tabCount;
    }

    uint8_t GetTabCount()
    {
        return m_tabCount;
    }

    void SetTabCoinType(uint8_t tabIdx, uint8_t coinType);
    void SetTabCoinVnum(uint8_t tabIdx, uint32_t coinType);

    uint8_t GetTabCoinType(uint8_t tabIdx);
    uint32_t GetTabCoinVnum(uint8_t tabIdx);

    void SetTabName(uint8_t tabIdx, const char *name);
    const char *GetTabName(uint8_t tabIdx);

    const ShopTab* GetShopTab(uint8_t tabIdx);

    void Open(bool isPrivateShop, bool isMainPrivateShop);
    void Close();
    bool IsOpen();
    bool IsPrivateShop();
    bool IsMainPlayerPrivateShop();

    void ClearPrivateShopStock();
    void AddPrivateShopItemStock(TItemPos ItemPos, uint8_t byDisplayPos, Gold dwPrice);
    void DelPrivateShopItemStock(TItemPos ItemPos);
    Gold GetPrivateShopItemPrice(TItemPos ItemPos);
    void BuildPrivateShop(const char *sign, const TItemPos &bundleItem);

protected:
    bool m_isShoping;
    bool m_isPrivateShop;
    bool m_isMainPlayerPrivateShop;

    uint8_t m_tabCount;
    ShopTab m_aShoptabs[SHOP_TAB_COUNT_MAX];

    std::unordered_map<TItemPos, TShopItemTable> m_PrivateShopItemStock;
};
