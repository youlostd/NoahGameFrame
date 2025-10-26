#ifndef METIN2_SERVER_GAME_SHOP_MANAGER_H
#define METIN2_SERVER_GAME_SHOP_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <memory>
#include <base/Singleton.hpp>

#include "shop.h"

class CHARACTER;

class CShopManager : public singleton<CShopManager>
{
public:
    bool Initialize();
    bool Reload();
    void Destroy();

    NpcShop *Get(uint32_t dwVnum);
    NpcShop *GetByNPCVnum(uint32_t dwVnum);

    bool StartShopping(CHARACTER *pkChr, CHARACTER *pkShopKeeper, int iShopVnum = 0);
    void StopShopping(CHARACTER *ch);

    void Buy(CHARACTER *ch, uint16_t pos, CountType amount = 1);
    void Sell(CHARACTER *ch, TItemPos bCell, CountType bCount = 0);

    PcShop *CreatePCShop(CHARACTER *ch,
                         const TShopItemTable *table,
                         uint8_t itemCount);

    PcShop *FindPCShop(uint32_t dwVID);
    void DestroyPCShop(CHARACTER *ch);

private:
    typedef std::unordered_map<uint32_t, std::unique_ptr<NpcShop>> NpcShopMap;
    typedef std::unordered_map<uint32_t, std::unique_ptr<PcShop>> PcShopMap;

    NpcShopMap m_map_pkShop;
    NpcShopMap m_map_pkShopByNPCVnum;
    PcShopMap m_map_pkShopByPC;

    bool ReadShopTableEx(const storm::StringRef &filename);
};
#endif /* METIN2_SERVER_GAME_SHOP_MANAGER_H */
