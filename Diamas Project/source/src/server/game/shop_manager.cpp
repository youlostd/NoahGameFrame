#include "shop_manager.h"
#include "shop.h"

#include "utils.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "db.h"
#include "DbCacheSocket.hpp"

#include <storm/StringUtil.hpp>
#include <storm/StringFlags.hpp>
#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <game/PageGrid.hpp>
#include <game/GamePacket.hpp>
#include <game/ItemTypes.hpp>

#include "battle_pass.h"

namespace
{
enum SortDirection
{
    kShopSortNone,
    kShopSortAsc,
    kShopSortDesc,
    kShopSortVnumAsc,
    kShopSortVnumDesc,
};

storm::StringValueTable<uint32_t> kShopCoinTypes[] = {
    {"GOLD", SHOP_COIN_TYPE_GOLD},
    {"SECONDARY_COIN", SHOP_COIN_TYPE_SECONDARY_COIN},
    {"CASH", SHOP_COIN_TYPE_CASH},

};

storm::StringValueTable<uint32_t> kShopSortDirections[] = {
    {"NONE", kShopSortNone},
    {"ASC", kShopSortAsc},
    {"DESC", kShopSortDesc},
    {"VNUM_ASC", kShopSortVnumAsc},
    {"VNUM_DESC", kShopSortVnumDesc},
};

bool CompareShopItemVnum(const SShopItemTable &lhs, const SShopItemTable &rhs) { return lhs.vnum < rhs.vnum; }

bool ConvertToShopItemTable(GroupTextGroup *grp, TShopTableEx &shopTable)
{
    if (!GetGroupProperty(grp, "Vnum", shopTable.dwVnum))
    {
        SPDLOG_ERROR("Group {0}: missing 'Vnum'", grp->GetName());
        return false;
    }
    auto name = grp->GetProperty("Name");
    shopTable.name = std::string(name.data(), name.length());
    if (shopTable.name.empty())
    {
        SPDLOG_ERROR("Group {0}: missing 'Name'", grp->GetName());
        return false;
    }

    if (shopTable.name.length() >= SHOP_TAB_NAME_MAX)
    {
        SPDLOG_ERROR("Group {0}: Shop name too long", grp->GetName());
        return false;
    }

    const auto coinType = grp->GetProperty("CoinType");
    if (!coinType.empty())
    {
        if (!storm::ParseStringWithTable(coinType,
                                         shopTable.coinType,
                                         kShopCoinTypes))
        {
            SPDLOG_ERROR("Group {0}: Unknown coin-type '{1}'",
                         grp->GetName(), std::string(coinType.data(), coinType.length()));
            return false;
        }
    }
    else { shopTable.coinType = SHOP_COIN_TYPE_GOLD; }

    if (shopTable.coinType == SHOP_COIN_TYPE_SECONDARY_COIN)
    {
        const auto coinVnum = grp->GetProperty("CoinVnum");
        if (!coinVnum.empty())
        {
            if (!storm::ParseNumber(coinVnum, shopTable.coinVnum))
            {
                SPDLOG_ERROR("Group {0}: Invalid coin-vnum '{1}'",
                             grp->GetName(), std::string(coinVnum.data(), coinVnum.length()));
                return false;
            }
        }
    }

    auto itemList = grp->GetList("Items");
    if (!itemList)
    {
        SPDLOG_ERROR("Group {0}: no items", grp->GetName());
        return false;
    }

    std::vector<TShopItemTable> shopItems(itemList->GetLineCount());

    std::vector<storm::StringRef> tokens;
    for (uint32_t i = 0; i < itemList->GetLineCount(); ++i)
    {
        tokens.clear();
        storm::ExtractArguments(itemList->GetLine(i), tokens);

        if (tokens.size() != 2 && tokens.size() != 3)
        {
            SPDLOG_ERROR("Group {0}: item #{0} is invalid", i);
            continue;
        }

        if (!storm::ParseNumber(tokens[0], shopItems[i].vnum))
        {
            SPDLOG_ERROR("Entry #{0} vnum is invalid", i);
            continue;
        }

        if (!storm::ParseNumber(tokens[1], shopItems[i].count))
        {
            SPDLOG_ERROR("Entry #{0} count is invalid", i);
            continue;
        }

        if (tokens.size() == 3)
        {
            if (!storm::ParseNumber(tokens[2], shopItems[i].price))
            {
                SPDLOG_ERROR("Entry #{0} gold is invalid", i);
                continue;
            }
        }
        else
        {
            auto table = ITEM_MANAGER::instance().GetTable(shopItems[i].vnum);
            if (!table)
            {
                SPDLOG_ERROR("Group {0}: vnum {1} doesn't exist",
                             grp->GetName(), shopItems[i].vnum);

                return false;
            }

            shopItems[i].price = table->dwGold * shopItems[i].count;
        }
    }

    uint32_t direction = kShopSortNone;
    const auto sort = grp->GetProperty("Sort");
    if (!sort.empty())
    {
        if (!storm::ParseStringWithTable(sort,
                                         direction,
                                         kShopSortDirections))
        {
            SPDLOG_ERROR("Group {0}: Unknown sort direction '{1}'",
                         grp->GetName(), std::string(sort.data(), sort.length()));
            return false;
        }
    }

    if (direction == kShopSortVnumAsc)
        std::sort(shopItems.begin(), shopItems.end(), CompareShopItemVnum);
    else if (direction == kShopSortVnumDesc)
        std::sort(shopItems.rbegin(), shopItems.rend(), CompareShopItemVnum);

    std::memset(&shopTable.items[0], 0, sizeof(shopTable.items));

    PagedGrid<int> grid(10, 9);

    for (const auto &item : shopItems)
    {
        auto table = ITEM_MANAGER::instance().GetTable(item.vnum);
        if (!table)
        {
            SPDLOG_ERROR("Group {0}: vnum {1} doesn't exist",
                         grp->GetName(), item.vnum);

            return false;
        }

        if (auto pos = grid.FindBlank(table->bSize); pos)
        {
            grid.PutPos(table->dwVnum, pos.value(), table->bSize);
            shopTable.items[pos.value()] = item;
        }
        else
        {
            SPDLOG_ERROR("Group {0}: vnum {1} doesn't fit",
                         grp->GetName(), item.vnum);
        }
    }

    shopTable.byItemCount = shopItems.size();
    return true;
}
}

bool CShopManager::Initialize() { return ReadShopTableEx("data/shop_table_ex.txt"); }
bool CShopManager::Reload()
{

    if (!m_map_pkShop.empty())
    {
        for (auto it = m_map_pkShop.begin(); it != m_map_pkShop.end(); it++)
        {
            it->second->RemoveAllGuests();
        }
    }

    m_map_pkShop.clear();
    m_map_pkShopByNPCVnum.clear();

    return ReadShopTableEx("data/shop_table_ex.txt");
}




void CShopManager::Destroy() { m_map_pkShop.clear(); }

NpcShop *CShopManager::Get(uint32_t dwVnum)
{
    auto it = m_map_pkShop.find(dwVnum);
    if (it == m_map_pkShop.end())
        return nullptr;

    return it->second.get();
}

NpcShop *CShopManager::GetByNPCVnum(uint32_t dwVnum)
{
    auto it = m_map_pkShopByNPCVnum.find(dwVnum);
    if (it == m_map_pkShopByNPCVnum.end())
        return nullptr;

    return it->second.get();
}

/*
 * 인터페이스 함수들
 */

// 상점 거래를 시작
bool CShopManager::StartShopping(CHARACTER *pkChr, CHARACTER *pkChrShopKeeper, int iShopVnum)
{
    /* if (pkChr->GetShop() || pkChr->GetShopOwner() == pkChrShopKeeper)
    {
        StopShopping(pkChr);
        SendI18nChatPacket(pkChr, CHAT_TYPE_INFO, "You already have a shop opened.");
        return false;
    }
*/
    if (pkChrShopKeeper->IsPC())
        return false;

    //PREVENT_TRADE_WINDOW
    if (pkChr->IsOpenSafebox() || pkChr->GetExchange() ||
        pkChr->GetMyShop() || pkChr->IsCubeOpen())
    {
        SendI18nChatPacket(pkChr, CHAT_TYPE_INFO, "다른 거래창이 열린상태에서는 상점거래를 할수 가 없습니다.");
        return false;
    }
    //END_PREVENT_TRADE_WINDOW

    auto distance = DISTANCE_APPROX(pkChr->GetX() - pkChrShopKeeper->GetX(),
                                    pkChr->GetY() - pkChrShopKeeper->GetY());

    if (distance >= SHOP_MAX_DISTANCE)
    {
        SPDLOG_DEBUG("SHOP: TOO_FAR: {0} distance {1}", pkChr->GetName(), distance);
        return false;
    }

    NpcShop *pkShop;

    if (iShopVnum)
        pkShop = Get(iShopVnum);
    else
        pkShop = GetByNPCVnum(pkChrShopKeeper->GetRaceNum());

    if (!pkShop)
    {
        SPDLOG_DEBUG("SHOP: NO SHOP");
        return false;
    }

    bool bOtherEmpire = false;

    if (pkChr->GetEmpire() != pkChrShopKeeper->GetEmpire())
        bOtherEmpire = true;

    pkShop->AddGuest(pkChr, pkChrShopKeeper->GetVID(), bOtherEmpire);
    pkChr->SetShopOwner(pkChrShopKeeper);
    SPDLOG_DEBUG("SHOP: START: {0}", pkChr->GetName());
    return true;
}

PcShop *CShopManager::FindPCShop(uint32_t dwVID)
{
    auto it = m_map_pkShopByPC.find(dwVID);
    if (it == m_map_pkShopByPC.end())
        return nullptr;

    return it->second.get();
}

PcShop *CShopManager::CreatePCShop(CHARACTER *ch,
                                   const TShopItemTable *table,
                                   uint8_t itemCount)
{
    if (FindPCShop(ch->GetVID()))
        return nullptr;

    auto pkShop = std::make_unique<PcShop>();
    pkShop->Create(ch, table, itemCount);

    const auto shopPtr = pkShop.get();
    m_map_pkShopByPC.emplace(ch->GetVID(), std::move(pkShop));
    return shopPtr;
}

void CShopManager::DestroyPCShop(CHARACTER *ch)
{
    auto it = m_map_pkShopByPC.find(ch->GetVID());
    if (it == m_map_pkShopByPC.end())
        return;

    //PREVENT_ITEM_COPY;
    ch->SetMyShopTime();
    //END_PREVENT_ITEM_COPY

    m_map_pkShopByPC.erase(it);
}

// 상점 거래를 종료
void CShopManager::StopShopping(CHARACTER *ch)
{
    auto shop = ch->GetShop();
    if (!shop)
        return;

    //PREVENT_ITEM_COPY;
    ch->SetMyShopTime();
    //END_PREVENT_ITEM_COPY

    shop->RemoveGuest(ch);
    SPDLOG_TRACE("SHOP: END: {0}", ch->GetName());
}

// 아이템 구입
void CShopManager::Buy(CHARACTER *ch, uint16_t pos, CountType amount)
{
    // TODO(tim): This should be impossible - if yes, remove it.
    if (!ch->GetShop() || !ch->GetShopOwner())
        return;

    if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(),
                        ch->GetY() - ch->GetShopOwner()->GetY()) > SHOP_MAX_DISTANCE)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "상점과의 거리가 너무 멀어 물건을 살 수 없습니다.");
        return;
    }

    auto pkShop = ch->GetShop();

    //PREVENT_ITEM_COPY
    ch->SetMyShopTime();
    //END_PREVENT_ITEM_COPY

    int ret = pkShop->Buy(ch, pos, amount);
    if (SHOP_SUBHEADER_GC_OK != ret)
    {
        if (SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX == ret)
        {
            auto shop = static_cast<NpcShop *>(pkShop);
            uint8_t tabIdx = pos / SHOP_HOST_ITEM_MAX_NUM;
            auto coinVnum = shop->GetTabCoinVnum(tabIdx);

            
            TPacketGCShop pack;
            pack.actionNothEnough = coinVnum;
            ch->GetDesc()->Send(HEADER_GC_SHOP, pack);
            return;
        }

        TPacketGCShop pack;
        pack.subheader = ret;
        ch->GetDesc()->Send(HEADER_GC_SHOP, pack);
    }
}

void CShopManager::Sell(CHARACTER *ch, TItemPos bCell, CountType bCount)
{
    if (!ch->GetShop())
        return;

    if (!ch->GetShopOwner())
        return;

    if (!ch->CanHandleItem())
        return;

    if (ch->GetShop()->GetType() != kShopTypeNpc)
        return;

    if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(),
                        ch->GetY() - ch->GetShopOwner()->GetY()) > SHOP_MAX_DISTANCE)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "상점과의 거리가 너무 멀어 물건을 팔 수 없습니다.");
        return;
    }

    if(!bCell.IsValidItemPosition())
        return;

    auto* item = ch->GetItem(bCell);
    if (!item)
        return;

    if (item->IsEquipped())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "착용 중인 아이템은 판매할 수 없습니다.");
        return;
    }

    if (item->isLocked())
        return;

    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
        return;

    if (item->IsSealed())
        return;

    item->Lock(true);

    if (bCount == 0 || bCount > item->GetCount())
        bCount = item->GetCount();

    Gold price = item->GetShopBuyPrice();

    if (IS_SET(item->GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
    {
        if (price == 0)
            price = bCount;
        else
            price = bCount / price;
    }
    else { price *= bCount; }

    price /= 5;

    //세금 계산
    Gold dwTax = 0;
    int iVal = 3;

    dwTax = price * 3 / 100;
    price -= dwTax;

    SPDLOG_TRACE("Sell Item price id {0} {1} itemid {2}", ch->GetPlayerID(), ch->GetName(), item->GetID());

    const Gold nTotalMoney = static_cast<Gold>(ch->GetGold()) + static_cast<Gold>(price);
    if (ch->GetAllowedGold() <= nTotalMoney)
    {
        SPDLOG_ERROR("[OVERFLOW_GOLD] id {0} name {1} gold {2} >= ", ch->GetPlayerID(), ch->GetName(), ch->GetGold(),
                     ch->GetAllowedGold());
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "20억냥이 초과하여 물품을 팔수 없습니다.");
        return;
    }

    // 20050802.myevan.상점 판매 로그에 아이템 ID 추가
    SPDLOG_INFO("SHOP: SELL: {0} item name: {1}(x{2}):{3} price: {4}", ch->GetName(), item->GetName(), bCount,
                item->GetID(), price);

    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "판매금액의 %d %% 가 세금으로 나가게됩니다", 3);

    DBManager::instance().SendMoneyLog(MONEY_LOG_SHOP, item->GetVnum(), price);

#ifdef ENABLE_BATTLE_PASS
    uint8_t bBattlePassId = ch->GetBattlePassId();
    if (bBattlePassId)
    {
        uint32_t dwItemVnum, dwUseCount;
        if (CBattlePass::instance().BattlePassMissionGetInfo(bBattlePassId, SELL_ITEM, &dwItemVnum, &dwUseCount))
        {
            if (dwItemVnum == item->GetVnum() &&  ch->GetMissionProgress(SELL_ITEM, bBattlePassId) < dwUseCount)
                 ch->UpdateMissionProgress(SELL_ITEM, bBattlePassId, bCount, dwUseCount);
        }
    }
#endif


    if (bCount == item->GetCount())
        ITEM_MANAGER::instance().RemoveItem(item, "SELL");
    else if(item->SetCount(item->GetCount() - bCount)) {
        item->Lock(false);
    }

    ch->ChangeGold(price);
}

bool CShopManager::ReadShopTableEx(const storm::StringRef &filename)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(filename))
    {
        SPDLOG_ERROR("Failed to load config file {0}", std::string(filename.data(), filename.length()));
        return false;
    }

    std::unordered_map<
        std::string,
        TShopTableEx
    > nameToTable;

    for (const auto &p : reader.GetChildren())
    {
        auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        // Allowed - see type check above
        auto grp = static_cast<GroupTextGroup *>(node);

        const std::string name = std::string(p.first.data(), p.first.length());

        TShopTableEx table;
        table.name = name;

        if (!ConvertToShopItemTable(grp, table))
        {
            SPDLOG_ERROR("Cannot read shop {0}.", name);
            return false;
        }

        if (!nameToTable.emplace(name, table).second)
        {
            SPDLOG_ERROR("Duplicate shop with name {0}", name);
            continue;
        }

        auto it = m_map_pkShop.find(table.dwVnum);
        if (m_map_pkShop.end() != it)
        {
            SPDLOG_ERROR("Duplicate shop with vnum {0}", table.dwVnum);
            continue;
        }

        auto shop = std::make_unique<NpcShop>();
        shop->AddShopTable(table);
        m_map_pkShop.emplace(table.dwVnum, std::move(shop));
    }

    //
    // NPC <> Shop mapping:
    //
    // Format:
    // List ShopNPC
    // {
    //     100 MyTestShop
    // }
    //

    auto shopNpc = reader.GetList("ShopNPC");
    if (!shopNpc)
    {
        SPDLOG_ERROR("{0} is missing 'ShopNPC' mapping", std::string(filename.data(), filename.length()));
        return false;
    }

    std::vector<storm::StringRef> tokens;
    for (uint32_t i = 0; i < shopNpc->GetLineCount(); ++i)
    {
        tokens.clear();
        storm::ExtractArguments(shopNpc->GetLine(i), tokens);

        if (tokens.size() != 2)
        {
            SPDLOG_ERROR("ShopNPC entry #{0} is invalid", i);
            continue;
        }

        uint32_t npcVnum;
        if (!storm::ParseNumber(tokens[0], npcVnum))
        {
            SPDLOG_ERROR("Entry #{0} vnum is invalid", i);
            continue;
        }

        auto name = std::string(tokens[1].data(), tokens[1].length());

        const auto it = nameToTable.find(name);
        if (it == nameToTable.end())
        {
            SPDLOG_ERROR("No shop with name {0}", std::string(tokens[1].data(), tokens[1].length()));
            continue;
        }

        auto [it2, result] = m_map_pkShopByNPCVnum.try_emplace(npcVnum, lazy_convert_construct([]
        {
            return std::make_unique<NpcShop>();
        }));
        it2->second->AddShopTable(it->second);
    }

    return true;
}
