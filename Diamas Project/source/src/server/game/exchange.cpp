#include "exchange.h"
#include "DbCacheSocket.hpp"
#include "DragonSoul.h"
#include "GBufferManager.h"
#include "char.h"
#include "db.h"
#include "desc.h"
#include "gm.h"
#include "item.h"
#include "item_manager.h"
#include "locale_service.h"
#include "log.h"

#include "utils.h"
#include <game/GamePacket.hpp>
#include "ItemUtils.h"

#if defined(WJ_COMBAT_ZONE)
#include "combat_zone.h"
#endif

// 교환 패킷
void SendExchangeArgPacket(CHARACTER *ch, uint8_t sub_header, bool is_me, uint64_t arg1)
{
    if (!ch->GetDesc())
        return;

    packet_exchange p;
    p.sub_header = sub_header;
    p.is_me = is_me;
    p.arg = arg1;

    ch->GetDesc()->Send(HEADER_GC_EXCHANGE, p);
}

void SendExchangeItemPacket(CHARACTER* ch, bool is_me, uint8_t pos, CItem* item) {
    packet_exchange p;
    p.sub_header = EXCHANGE_SUBHEADER_GC_ITEM_ADD;
    p.is_me      = is_me;

    ClientItemData d;
    d.id          = item->GetID();
    d.vnum        = item->GetVnum();
    d.transVnum   = item->GetTransmutationVnum();
    d.count       = item->GetCount();
    d.nSealDate   = item->GetSealDate();
    d.highlighted = 0;
    std::memcpy(&d.sockets, item->GetSockets(), sizeof(d.sockets));
    std::memcpy(&d.attrs, item->GetAttributes(), sizeof(d.attrs));
    p.itemData = d;
    p.itemPos  = TItemPos(RESERVED_WINDOW, pos);
    ch->GetDesc()->Send(HEADER_GC_EXCHANGE, p);
}

void SendExchangeSubheader(CHARACTER *ch, uint8_t sub_header)
{
    if (!ch->GetDesc())
        return;

    packet_exchange p;
    p.sub_header = sub_header;

    ch->GetDesc()->Send(HEADER_GC_EXCHANGE, p);
}

void SendExchangeItemPosPacket(CHARACTER *ch, uint8_t sub_header, bool is_me, TItemPos pos)
{
    if (!ch->GetDesc())
        return;

    packet_exchange p;
    p.sub_header = sub_header;
    p.is_me = is_me;
    p.itemPos = pos;

    ch->GetDesc()->Send(HEADER_GC_EXCHANGE, p);
}

// 교환을 시작
bool CHARACTER::ExchangeStart(CHARACTER *victim)
{
    if (this == victim) // 자기 자신과는 교환을 못한다.
        return false;

    if (IsObserverMode() && !IsGM())
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "관전 상태에서는 교환을 할 수 없습니다.");
        return false;
    }

    if (victim->IsNPC())
        return false;

#ifdef __NEW_GAMEMASTER_CONFIG__
    if ((!GM::check_allow(GetGMLevel(), GM_ALLOW_EXCHANGE_TO_GM) && victim->IsGM()) ||
        (!GM::check_allow(GetGMLevel(), GM_ALLOW_EXCHANGE_TO_PLAYER) && !victim->IsGM()))
    {
        if (!GM::check_allow(GetGMLevel(), GM_ALLOW_EXCHANGE_TO_GM) && victim->IsGM())
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "You are not allowed to trade to gamemasters.");
        else if (!GM::check_allow(GetGMLevel(), GM_ALLOW_EXCHANGE_TO_PLAYER) && !victim->IsGM())
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "You are not allowed to trade to players.");
        return false;
    }
#endif

    // PREVENT_TRADE_WINDOW
    if (IsOpenSafebox() || GetShopOwner() || GetMyShop() || IsCubeOpen())
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "다른 거래창이 열려있을경우 거래를 할수 없습니다.");
        return false;
    }

    if (victim->IsOpenSafebox() || victim->GetShopOwner() || victim->GetMyShop() || victim->IsCubeOpen())
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "상대방이 다른 거래중이라 거래를 할수 없습니다.");
        return false;
    }
    // END_PREVENT_TRADE_WINDOW

    int iDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

    // 거리 체크
    if (iDist >= EXCHANGE_MAX_DISTANCE)
        return false;

    if (GetExchange())
        return false;

    if (victim->GetExchange())
    {
        SendExchangeSubheader(this, EXCHANGE_SUBHEADER_GC_ALREADY);
        return false;
    }

    if (victim->IsBlockMode(BLOCK_EXCHANGE, this))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "상대방이 교환 거부 상태입니다.");
        return false;
    }

    SetExchange(new CExchange(this));
    victim->SetExchange(new CExchange(victim));

    victim->GetExchange()->SetCompany(GetExchange());
    GetExchange()->SetCompany(victim->GetExchange());

    SetExchangeTime();
    victim->SetExchangeTime();

    SendExchangeArgPacket(victim, EXCHANGE_SUBHEADER_GC_START, 0, GetVID());
    SendExchangeArgPacket(this, EXCHANGE_SUBHEADER_GC_START, 0, victim->GetVID());

    return true;
}

CExchange::CExchange(CHARACTER *pOwner)
    : m_pCompany(nullptr), m_pOwner(pOwner), m_bAccept(false), m_gold(0), m_grid(EXCHANGE_WIDTH, EXCHANGE_HEIGHT)
{
    pOwner->SetExchange(this);
}

CExchange::~CExchange()
{
}


bool CExchange::AddItem(TItemPos item_pos, uint8_t pos)
{
    assert(m_pOwner != NULL && GetCompany());

    if (!item_pos.IsValidItemPosition())
        return false;

    // 장비는 교환할 수 없음
    if (item_pos.IsEquipPosition())
        return false;

    CItem *item = m_pOwner->GetItem(item_pos);
    if (!item)
        return false;

    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
    {
        SendI18nChatPacket(m_pOwner, CHAT_TYPE_INFO, "아이템을 건네줄 수 없습니다.");
        return false;
    }

    if (item->IsSealed())
    {
        SendI18nChatPacket(m_pOwner, CHAT_TYPE_INFO, "ITEM_IS_SEALED_CANNOT_DO");
        return false;
    }

    if (item->isLocked())
        return false;

    // 이미 교환창에 추가된 아이템인가?
    if (item->IsExchanging())
    {
        SPDLOG_INFO("EXCHANGE under exchanging");
        return false;
    }

    if (!m_grid.IsEmpty(pos, item->GetSize()))
    {
        auto newPos = FindEmpty(item->GetSize());
        if (newPos != -1) { pos = newPos; }
        else
        {
            SPDLOG_INFO("EXCHANGE not empty item_pos {0} {1} {2}", pos, 1, item->GetSize());
            return false;
        }
    }

    Accept(false);
    GetCompany()->Accept(false);

    if (!m_grid.Put(item, pos))
    {
        SPDLOG_INFO("EXCHANGE put failed pos {0} size {1}", pos, item->GetSize());
        return false;
    }

    item->SetExchanging(true);

    m_onDestroy[pos] = item->AddOnDestroy([this, pos] { RemoveItem(pos); });

    SendExchangeItemPacket(m_pOwner, true, pos, item);
    SendExchangeItemPacket(GetCompany()->GetOwner(), false, pos, item);


    SPDLOG_TRACE("EXCHANGE AddItem success {0} pos({1}, {2}) {3}", item->GetVnum(), item_pos.window_type, item_pos.cell,
                 pos);
    return true;
}

int32_t CExchange::FindEmpty(uint8_t size) const { return m_grid.FindBlank(size).value_or(-1); }

bool CExchange::RemoveItem(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return false;

    CItem *item = m_grid.GetPos(pos);
    if (!item)
        return false;

    m_onDestroy[pos].disconnect();

    item->SetExchanging(false);
    m_grid.Clear(pos);

    SendExchangeItemPosPacket(GetOwner(), EXCHANGE_SUBHEADER_GC_ITEM_DEL, true, TItemPos(RESERVED_WINDOW, pos));
    SendExchangeItemPosPacket(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ITEM_DEL, false, TItemPos(RESERVED_WINDOW, pos));

    Accept(false);
    GetCompany()->Accept(false);
    return true;
}

bool CExchange::AddGold(Gold gold)
{
    if (gold <= 0)
        return false;

    if (GetOwner()->GetGold() < gold)
    {
        // 가지고 있는 돈이 부족.
       SendExchangeSubheader(GetOwner(), EXCHANGE_SUBHEADER_GC_LESS_GOLD);

        return false;
    }

    if (m_gold > 0)
        return false;

    Accept(false);
    GetCompany()->Accept(false);

    m_gold = gold;

    SendExchangeArgPacket(GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, true, m_gold);
    SendExchangeArgPacket(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, false, m_gold);
    return true;
}

bool CExchange::Check(int *piItemCount)
{
    if (GetOwner()->GetGold() < m_gold)
        return false;

    uint32_t item_count = 0;

    for (uint32_t i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
    {
        CItem *item = m_grid.GetUnique(i);
        if (!item)
            continue;

        if (!item->GetPosition().IsValidItemPosition())
            return false;

        if (item->GetOwner() != GetOwner())
            return false;

        ++item_count;
    }

    *piItemCount = item_count;
    return true;
}

bool CExchange::CheckSpace()
{
    PagedGrid<bool> inventoryGrid(5,
                                  9,
                                  INVENTORY_PAGE_COUNT);

    static std::vector<uint16_t> s_vDSGrid(DRAGON_SOUL_INVENTORY_MAX_NUM);
    bool bDSInitialized = false;

    CHARACTER *partner = GetCompany()->GetOwner();

    for (uint32_t i = 0; i < inventoryGrid.GetSize(); ++i)
    {
        auto* item = partner->GetInventoryItem(i);
        if (!item)
            continue;

        inventoryGrid.PutPos(true, i, item->GetSize());
    }
	std::vector<uint16_t> exceptDSCells;
	exceptDSCells.clear();

    for (uint32_t i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
    {
        CItem *item = m_grid.GetUnique(i);
        if (!item)
            continue;

        if (item->IsDragonSoul())
        {
			const auto foundCell = partner->GetEmptyDragonSoulInventoryWithExceptions(item, exceptDSCells);
			if (foundCell != -1)
			{
				exceptDSCells.push_back(gsl::narrow_cast<uint16_t>(foundCell));
				continue;
			}

			return false;
        }
        else
        {
            std::optional<uint32_t> pos = std::nullopt;
            switch (item->GetItemType())
            {
            case ITEM_MATERIAL:
                pos = inventoryGrid.FindBlankBetweenPages(item->GetSize(), NORMAL_INVENTORY_MAX_PAGE,
                                                          MATERIAL_INVENTORY_MAX_PAGE);
                break;

            case ITEM_COSTUME:
                pos = inventoryGrid.FindBlankBetweenPages(item->GetSize(), MATERIAL_INVENTORY_MAX_PAGE,
                                                          COSTUME_INVENTORY_MAX_PAGE);
                break;
            default:
                pos = inventoryGrid.FindBlankBetweenPages(item->GetSize(), NORMAL_INVENTORY_MIN_PAGE,
                                                          NORMAL_INVENTORY_MAX_PAGE);
                break;
            }

            if (!pos.has_value())
                return false;

            inventoryGrid.PutPos(true, pos.value(), item->GetSize());
        }
    }

    return true;
}

// 교환 끝 (아이템과 돈 등을 실제로 옮긴다)
bool CExchange::Done(uint32_t tradeID, bool firstPlayer)
{
    CHARACTER *victim = GetCompany()->GetOwner();

    for (uint32_t i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
    {
        auto item = m_grid.GetUnique(i);
        if (!item)
            continue;
        m_grid.Clear(i);

        int empty_pos;
        if (item->IsDragonSoul())
            empty_pos = victim->GetEmptyDragonSoulInventory(item);
        else
            empty_pos = victim->GetEmptyInventory(item);

        if (empty_pos < 0)
        {
            SPDLOG_ERROR("No space in inventory left {0} <-> {1} item {2}", m_pOwner->GetName(), victim->GetName(),
                         item->GetVnum());
            continue;
        }

        m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

        item->RemoveFromCharacter();
        if (item->IsDragonSoul())
            item->AddToCharacter(victim, TItemPos(DRAGON_SOUL_INVENTORY, empty_pos));
        else
            item->AddToCharacter(victim, TItemPos(INVENTORY, empty_pos));

        ITEM_MANAGER::instance().FlushDelayedSave(item);

        item->SetExchanging(false);
        LogManager::instance().ExchangeItemLog(tradeID, item, firstPlayer ? "A" : "B");
    }

    if (m_gold)
    {
        GetOwner()->ChangeGold(-m_gold);
        GetOwner()->Save();
        victim->ChangeGold(m_gold);
        victim->Save();

        if (m_gold > 1000)
        {
            LogManager::instance().CharLog(victim, m_gold, "EXCHANGE_GOLD_TAKE",
                                           fmt::format("{} {}", GetOwner()->GetPlayerID(), GetOwner()->GetName()).
                                           c_str());
            LogManager::instance().CharLog(GetOwner(), m_gold, "EXCHANGE_GOLD_GIVE",
                                           fmt::format("{} {}", victim->GetPlayerID(), victim->GetName()).c_str());
        }
    }
    return true;
}

bool CExchange::Accept(bool bAccept)
{
    if (m_bAccept == bAccept)
        return true;

    m_bAccept = bAccept;

    // 둘 다 동의 했으므로 교환 성립
    if (m_bAccept && GetCompany()->m_bAccept)
    {
        int iItemCount;

        CHARACTER *victim = GetCompany()->GetOwner();

        // PREVENT_PORTAL_AFTER_EXCHANGE
        GetOwner()->SetExchangeTime();
        victim->SetExchangeTime();
        // END_PREVENT_PORTAL_AFTER_EXCHANGE

        // exchange_check 에서는 교환할 아이템들이 제자리에 있나 확인하고,
        // 엘크도 충분히 있나 확인한다, 두번째 인자로 교환할 아이템 개수
        // 를 리턴한다.
        if (!Check(&iItemCount))
        {
            SendI18nChatPacket(GetOwner(), CHAT_TYPE_INFO, "돈이 부족하거나 아이템이 제자리에 없습니다.");
            SendI18nChatPacket(victim, CHAT_TYPE_INFO, "상대방의 돈이 부족하거나 아이템이 제자리에 없습니다.");
            Cancel();
            return false;
        }

        // 리턴 받은 아이템 개수로 상대방의 소지품에 남은 자리가 있나 확인한다.
        if (!CheckSpace())
        {
            SendI18nChatPacket(GetOwner(), CHAT_TYPE_INFO, "상대방의 소지품에 빈 공간이 없습니다.");
            SendI18nChatPacket(victim, CHAT_TYPE_INFO, "소지품에 빈 공간이 없습니다.");
            Cancel();
            return false;
        }

        // 상대방도 마찬가지로..
        if (!GetCompany()->Check(&iItemCount))
        {
            SendI18nChatPacket(victim, CHAT_TYPE_INFO, "돈이 부족하거나 아이템이 제자리에 없습니다.");
            SendI18nChatPacket(GetOwner(), CHAT_TYPE_INFO, "상대방의 돈이 부족하거나 아이템이 제자리에 없습니다.");
            Cancel();
            return false;
        }

        if (!GetCompany()->CheckSpace())
        {
            SendI18nChatPacket(victim, CHAT_TYPE_INFO, "상대방의 소지품에 빈 공간이 없습니다.");
            SendI18nChatPacket(GetOwner(), CHAT_TYPE_INFO, "소지품에 빈 공간이 없습니다.");
            Cancel();
            return false;
        }

        uint32_t tradeID =
            LogManager::instance().ExchangeLog(EXCHANGE_TYPE_TRADE, GetOwner()->GetPlayerID(), victim->GetPlayerID(),
                                               GetOwner()->GetX(), GetOwner()->GetY(), m_gold, GetCompany()->m_gold);

        if (Done(tradeID, true))
        {
            if (m_gold) // 돈이 있을 떄만 저장
                GetOwner()->Save();

            if (GetCompany()->Done(tradeID, false))
            {
                if (GetCompany()->m_gold) // 돈이 있을 때만 저장
                    victim->Save();

                // INTERNATIONAL_VERSION
                SendI18nChatPacket(GetOwner(), CHAT_TYPE_INFO, "%s 님과의 교환이 성사 되었습니다.", victim->GetName());
                SendI18nChatPacket(victim, CHAT_TYPE_INFO, "%s 님과의 교환이 성사 되었습니다.", GetOwner()->GetName());
                // END_OF_INTERNATIONAL_VERSION
            }
        }

        Cancel();
        return false;
    }
    else
    {
        // 아니면 accept에 대한 패킷을 보내자.
        SendExchangeArgPacket(GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, true, m_bAccept);
        SendExchangeArgPacket(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, false, m_bAccept);
        return true;
    }
}

void CExchange::Cancel()
{
    SendExchangeSubheader(GetOwner(), EXCHANGE_SUBHEADER_GC_END);
    GetOwner()->SetExchange(nullptr);

    for (uint32_t i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
    {
        auto item = m_grid.GetUnique(i);
        if (item)
            item->SetExchanging(false);
    }

    if (GetCompany())
    {
        GetCompany()->SetCompany(nullptr);
        GetCompany()->Cancel();
    }

    delete this;
}
