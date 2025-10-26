
#include "safebox.h"
#include <game/GamePacket.hpp>
#include "char.h"
#include "DbCacheSocket.hpp"
#include "item.h"
#include "item_manager.h"
#include "ItemUtils.h"
#include "log.h"
#include "desc.h"

void SafeboxItemPacket(CHARACTER *owner, CItem *item, uint8_t windowMode, uint16_t pos)
{
    GcItemSetPacket pack;
    pack.pos = TItemPos(windowMode, pos);
    pack.data.id = item->GetID();
    pack.data.vnum = item->GetVnum();
    pack.data.count = item->GetCount();
    pack.data.nSealDate = item->GetSealDate();
    pack.data.transVnum = item->GetTransmutationVnum();
    std::memcpy(pack.data.sockets, item->GetSockets(), sizeof(pack.data.sockets));
    std::memcpy(pack.data.attrs, item->GetAttributes(), sizeof(pack.data.attrs));

    owner->GetDesc()->Send(windowMode == SAFEBOX ? HEADER_GC_SAFEBOX_SET : HEADER_GC_MALL_SET, pack);
}

CSafebox::CSafebox(CHARACTER *pkChrOwner, int iSize, uint32_t dwGold)
    : m_pkChrOwner(pkChrOwner)
      , m_pkGrid(iSize
                     ? new ItemGrid(SAFEBOX_PAGE_WIDTH, SAFEBOX_PAGE_HEIGHT,
                                    iSize)
                     : nullptr)
      , m_iSize(iSize)
      , m_lGold(dwGold)
      , m_bWindowMode(SAFEBOX)
{
    assert(m_pkChrOwner != NULL);
}

CSafebox::~CSafebox()
{
    for (auto i = 0u; i < m_pkGrid->GetSize(); ++i)
    {
        auto* item = m_pkGrid->GetUnique(i);
        if (item)
        {
            item->SetSkipSave(true);
            ITEM_MANAGER::instance().FlushDelayedSave(item);

            m_pkGrid->Clear(i);
            item->RemoveFromCharacter();

            M2_DESTROY_ITEM(item);
        }
    }
}

void CSafebox::SetWindowMode(uint8_t bWindowMode) { m_bWindowMode = bWindowMode; }

void CSafebox::TransferFromInventory(uint32_t dwPos, CItem *pkItem)
{
    if (!IsValidPosition(dwPos))
    {
        SPDLOG_ERROR("SAFEBOX: item on wrong position at %d (size of grid = %lu)", dwPos, m_pkGrid->GetSize());
        return;
    }

    if (IsEmpty(dwPos, pkItem->GetSize()))
    {
        // Position is empty, we can just add
        // Remove it from the player's inventory first.
        CHARACTER *ch = pkItem->GetOwner();
        pkItem->RemoveFromCharacter();
        if (!pkItem->IsDragonSoul())
            ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, dwPos, std::numeric_limits<uint16_t>::max());

        // Move to safebox
        Add(dwPos, pkItem);

        char szHint[128];
        std::snprintf(szHint, sizeof(szHint), "%d", pkItem->GetCount());
        LogManager::instance().ItemLog(pkItem->GetOwner(), pkItem, "SAFEBOX PUT", szHint);
    }
    else
    {
        CItem *targetItem = Get(dwPos);
        if (targetItem && CanStack(targetItem, pkItem))
        {
            // We can stack.
            CountType count = std::min<CountType>(GetItemMaxCount(targetItem) - targetItem->GetCount(),
                                                  pkItem->GetCount());

            if (count > 0)
            {
                // Log it (we won't be able to log it afterwards if orig item is destroyed)
                char szHint[128];
                std::snprintf(szHint, sizeof(szHint), "%u [+%d]", targetItem->GetCount(), count);
                LogManager::instance().ItemLog(targetItem->GetOwner(), targetItem, "SAFEBOX STACK TO", szHint);

                std::snprintf(szHint, sizeof(szHint), "%u [-%d]", pkItem->GetCount(), count);
                LogManager::instance().ItemLog(pkItem->GetOwner(), pkItem, "SAFEBOX STACK FROM", szHint);

                //Update the counts
                pkItem->SetCount(pkItem->GetCount() - count);
                targetItem->SetCount(targetItem->GetCount() + count);

                //Update the client
                SafeboxItemPacket(m_pkChrOwner, targetItem, m_bWindowMode, dwPos);

                SPDLOG_TRACE("SAFEBOX: TRANSFER STACK %s -> %dx %lu @ pos %d (stacked: %d)", m_pkChrOwner->GetName(),
                             targetItem->GetCount(), targetItem->GetVnum(), (uint8_t)dwPos, count);
            }
        }
    }
}

void CSafebox::ChangeSize(int iSize)
{
    // If the current size is greater than the argument, the size is assumed.
    if (m_iSize >= iSize)
        return;

    m_iSize = iSize;

    auto* newGrid = new ItemGrid(SAFEBOX_PAGE_WIDTH, SAFEBOX_PAGE_HEIGHT,
                                m_iSize, m_pkGrid.get());
    m_pkGrid.reset(newGrid);
}

bool CSafebox::IsValidPosition(uint32_t dwPos) const
{
    if (!m_pkGrid)
        return false;

    if (dwPos >= m_pkGrid->GetSize())
        return false;

    return true;
}

bool CSafebox::IsEmpty(uint32_t dwPos, uint8_t bSize) const
{
    if (!m_pkGrid)
        return false;

    return m_pkGrid->IsEmpty(dwPos, bSize);
}

CItem *CSafebox::FindNextStackable(CItem *item) const
{
    for (int i = 0; i < m_pkGrid->GetSize(); ++i)
    {
        auto* other = m_pkGrid->GetUnique(i);
        if (other)
        {
            if (CanStack(item, other))
            {
                CountType count = std::min<CountType>(GetItemMaxCount(other) - other->GetCount(), item->GetCount());
                if (count > 0)
                    return other;
            }
        }
    }

    return nullptr;
}

int32_t CSafebox::FindEmpty(uint8_t size) const { return m_pkGrid->FindBlank(size).value_or(-1); }

CItem *CSafebox::Get(uint32_t pos) const
{
    if (pos >= m_pkGrid->GetSize())
        return nullptr;

    return m_pkGrid->GetPos(pos);
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool CSafebox::Add(uint32_t dwPos, CItem *pkItem)
{
    if (!IsValidPosition(dwPos))
    {
        SPDLOG_ERROR("SAFEBOX: item on wrong position at %u (size of grid = %u)", dwPos, m_pkGrid->GetSize());
        return false;
    }

    pkItem->SetPosition({m_bWindowMode, static_cast<uint16_t>(dwPos)});
    pkItem->SetOwner(m_pkChrOwner);
    pkItem->Save(); // Force a save
    ITEM_MANAGER::instance().FlushDelayedSave(pkItem);

    m_pkGrid->Put(pkItem, dwPos);

    SafeboxItemPacket(m_pkChrOwner, pkItem, m_bWindowMode, dwPos);

    SPDLOG_TRACE("SAFEBOX: ADD %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
    return true;
}

CItem *CSafebox::Remove(uint32_t pos)
{
    auto* pkItem = Get(pos);
    if (!pkItem)
        return nullptr;

    m_pkGrid->Clear(pos);
    pkItem->RemoveFromCharacter();

    TPacketGCItemDel pack;
    pack.pos = TItemPos(m_bWindowMode, pos);
    m_pkChrOwner->GetDesc()->Send(m_bWindowMode == SAFEBOX ? HEADER_GC_SAFEBOX_DEL : HEADER_GC_MALL_DEL, pack);
    SPDLOG_TRACE("SAFEBOX: REMOVE {} {} count {}", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
    return pkItem;
}

bool CSafebox::MoveItem(uint16_t bCell, uint16_t bDestCell, CountType count)
{
    CItem *item;

    if (!IsValidPosition(bCell) || !IsValidPosition(bDestCell))
        return false;

    if (!((item = Get(bCell))))
        return false;

    if (item->IsExchanging())
        return false;

    if (item->GetCount() < count)
        return false;

    {
        auto* targetItem = Get(bDestCell);
        if (targetItem)
        {
            if (!CanStack(item, targetItem))
                //Can't stack, and since there's an item there already, we can't do anything else.
                return false;

            if (count == 0)
                count = item->GetCount();

            count = std::min<CountType>(GetItemMaxCount(targetItem) - targetItem->GetCount(), count);

            if (count > 0)
            {
                const auto newOriginCount = item->GetCount() - count;
                if (newOriginCount <= 0)
                    Remove(bCell);

                //Update the counts
                item->SetCount(newOriginCount);
                targetItem->SetCount(targetItem->GetCount() + count);

                //Update the client
                SafeboxItemPacket(m_pkChrOwner, targetItem, m_bWindowMode, bDestCell);
                if (newOriginCount > 0)
                    SafeboxItemPacket(m_pkChrOwner, item, m_bWindowMode, bCell);
                //Only do this if item was not destroyed

                SPDLOG_TRACE("SAFEBOX: STACK %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell,
                             targetItem->GetName(), targetItem->GetCount());
            }

            return true;
        }

        if (!IsEmpty(bDestCell, item->GetSize()))
            return false;

        m_pkGrid->Clear(bCell);

        if (!m_pkGrid->Put(item, bDestCell))
        {
            m_pkGrid->Put(item, bCell);
            return false;
        }
        else
        {
            m_pkGrid->Clear(bDestCell);
            m_pkGrid->Put(item, bCell);
        }

        SPDLOG_TRACE("SAFEBOX: MOVE %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell,
                     item->GetName(), item->GetCount());

        Remove(bCell);
        Add(bDestCell, item);
    }

    return true;
}

void CSafebox::Save() const
{
    TSafeboxTable t{};

    t.dwID = m_pkChrOwner->GetDesc()->GetAccountTable().id;
    t.dwGold = m_lGold;

    db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_SAVE, 0, &t, sizeof(TSafeboxTable));
    SPDLOG_TRACE("SAFEBOX: SAVE %s", m_pkChrOwner->GetName());
}
