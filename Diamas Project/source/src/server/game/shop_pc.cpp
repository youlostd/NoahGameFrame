#include "constants.h"
#include "utils.h"
#include "config.h"
#include "shop.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "CharUtil.hpp"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "db.h"
#include "locale.h"
#include "questmanager.h"

#include <game/PageGrid.hpp>
#include <game/GamePacket.hpp>

#include <storm/Util.hpp>
#include "spdlog/spdlog.h"

PcShop::PcShop()
    : Shop(kShopTypePc)
      , m_owner(nullptr)
      , m_items{}
{
}

PcShop::~PcShop()
{
    for (auto &item : m_items)
    {
        if (!item.item)
            continue;

        STORM_ASSERT(item.item->IsExchanging(), "Item is not being traded");
        item.item->SetExchanging(false);
    }
}

bool PcShop::Create(CHARACTER *owner,
                    const TShopItemTable *itemTable,
                    uint8_t itemCount)
{
    STORM_ASSERT(itemCount <= SHOP_HOST_ITEM_MAX_NUM,
                 "Item table size is fixed");

    m_owner = owner;

    // TODO(tim): Add constants for this
    PagedGrid<bool> grid(5, 9);

    for (int i = 0; i < itemCount; ++i, ++itemTable)
    {
        STORM_ASSERT(itemTable->display_pos >= 0 &&
                     itemTable->display_pos < SHOP_HOST_ITEM_MAX_NUM,
                     "Display position out of range");

        auto item = owner->GetItem(itemTable->pos);
        STORM_ASSERT(nullptr != item, "Argument check");

        auto table = item->GetProto();

        if (!grid.IsEmpty(itemTable->display_pos, table->bSize))
        {
            SPDLOG_ERROR(
                "{1}: PC-Shop position {0} already occupied name {1}",
                itemTable->display_pos, owner->GetName());
            continue;
        }

        grid.PutPos(true, itemTable->display_pos, table->bSize);

        auto &shopItem = m_items[itemTable->display_pos];

        shopItem.item = item;
        shopItem.onDestroy = item->AddOnDestroy([this, item]() { RemoveItem(item); });
        shopItem.price = itemTable->price;

        STORM_ASSERT(!item->IsExchanging(), "Item is already being traded");
        item->SetExchanging(true);
    }

    return true;
}

bool PcShop::AddGuest(CHARACTER *ch, uint32_t ownerVid, bool otherEmpire)
{
    STORM_ASSERT(!ch->GetShop(), "Already shopping");
    ch->SetShop(this);

    ShopGuest guest;
    guest.fromOtherEmpire = otherEmpire;
    m_guestMap.insert(std::make_pair(ch, guest));

    TPacketGCShop pack;
    pack.subheader = SHOP_SUBHEADER_GC_START;

    TPacketGCShopStart pack2{};

    pack2.owner_vid = m_owner->GetVID();

    for (int i = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i)
    {
        const auto &shopItem = m_items[i];
        const auto item = shopItem.item;

        if (!item)
            continue;

        pack2.items[i].display_pos = i;
        pack2.items[i].vnum = item->GetVnum();
        pack2.items[i].price = shopItem.price;
        pack2.items[i].count = item->GetCount();
        pack2.items[i].transVnum = item->GetTransmutationVnum();
        std::memcpy(pack2.items[i].sockets, item->GetSockets(), sizeof(pack2.items[i].sockets));
        std::memcpy(pack2.items[i].attrs, item->GetAttributes(), sizeof(pack2.items[i].attrs));
    }


    pack.actionStart = pack2;
    ch->GetDesc()->Send(HEADER_GC_SHOP, pack);
    return true;
}

/*virtual*/
int PcShop::Buy(CHARACTER *ch, uint16_t pos, CountType amount)
{
    auto it = m_guestMap.find(ch);
    if (it == m_guestMap.end())
        return SHOP_SUBHEADER_GC_END;

    if (pos >= SHOP_HOST_ITEM_MAX_NUM)
        return SHOP_SUBHEADER_GC_END;

    auto &shopItem = m_items[pos];
    auto item = shopItem.item;

    if (!item)
        return SHOP_SUBHEADER_GC_SOLD_OUT;

    Gold price = shopItem.price;

    if (ch->GetGold() < price)
    {
        SPDLOG_TRACE("{0} has not enough gold ({1} < {2})",
                     ch->GetName(), ch->GetGold(), price);

        return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
    }

    int iEmptyPos = ch->GetEmptyInventory(item);
    if (iEmptyPos < 0)
    {
        SPDLOG_DEBUG("{0}'s inventory full (item size {1})",
                      ch->GetName(), item->GetSize());

        return SHOP_SUBHEADER_GC_INVENTORY_FULL;
    }

    ch->ChangeGold(-price);

    //세금 계산
    Gold dwTax = 0;
    int iVal = quest::CQuestManager::instance().GetEventFlag("personal_shop");

    if (0 < iVal)
    {
        if (iVal > 100)
            iVal = 100;

        dwTax = price * iVal / 100;
        price = price - dwTax;
    }
    else
    {
        iVal = 0;
        dwTax = 0;
    }

    // 군주 시스템 : 세금 징수
    m_owner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

    char buf[512];
    if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
    {
        fmt::format_to_n(buf, sizeof(buf), "{} FROM: {} TO: {} PRICE: {}",
                 item->GetName(), ch->GetPlayerID(), m_owner->GetPlayerID(), price);

        LogManager::instance().GoldBarLog(ch->GetPlayerID(), item->GetID(),
                                          SHOP_BUY, buf);

        LogManager::instance().GoldBarLog(m_owner->GetPlayerID(), item->GetID(),
                                          SHOP_SELL, buf);
    }

    STORM_ASSERT(item->IsExchanging(), "Item is not being traded");
    item->SetExchanging(false);

    RemoveItem(pos);

    item->RemoveFromCharacter();
    item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));

    ITEM_MANAGER::instance().FlushDelayedSave(item);

    fmt::format_to_n(buf, sizeof(buf), "{} {}({}) {} {}",
             item->GetName(), m_owner->GetPlayerID(),
             m_owner->GetName().c_str(), price, item->GetCount());

    LogManager::instance().ItemLog(ch, item, "SHOP_BUY", buf);

    fmt::format_to_n(buf, sizeof(buf), "{} {}({}) {} {}",
             item->GetName(), ch->GetPlayerID(),
             ch->GetName().c_str(), price, item->GetCount());

    LogManager::instance().ItemLog(m_owner, item, "SHOP_SELL", buf);

    m_owner->ChangeGold(price);

    if (iVal > 0)
        SendI18nChatPacket(m_owner, CHAT_TYPE_INFO, "판매금액의 %d %% 가 세금으로 나가게됩니다", iVal);

    if (item)
        SPDLOG_DEBUG("SHOP: BUY: name {0} {1}(x {2}):{3} price {4}",
                      ch->GetName(), item->GetName(), item->GetCount(),
                      item->GetID(), price);

    ch->Save();
    return SHOP_SUBHEADER_GC_OK;
}

void PcShop::RemoveItem(const CItem *item)
{
    for (size_t i = 0; i < STORM_ARRAYSIZE(m_items); ++i)
    {
        if (m_items[i].item == item)
            RemoveItem(i);
    }
}

void PcShop::RemoveItem(uint8_t pos)
{
    m_items[pos].item = nullptr;
    m_items[pos].onDestroy.disconnect();
    BroadcastItemRemove(pos);
}

void PcShop::BroadcastItemRemove(uint8_t pos)
{

    TPacketGCShop pack;
    TPacketGCShopUpdateItem pack2;
    
    pack2.pos = pos;
    pack2.item.vnum = 0;
    pack2.item.price = 0;
    pack2.item.count = 0;

    pack.actionUpdate = pack2;

    Broadcast(HEADER_GC_SHOP, pack);
}
