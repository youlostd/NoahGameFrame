#include "char.h"
#include "char_manager.h"
#include "DbCacheSocket.hpp"
#include <game/AffectConstants.hpp>
#include <game/GamePacket.hpp>
#include "config.h"
#include "db.h"
#include "desc.h"
#include "desc_manager.h"
#include "GArena.h"
#include "GRefineManager.h"
#include "dungeon.h"
#include "fishing.h"
#include "item_manager.h"
#include "log.h"
#include "marriage.h"
#include "mob_manager.h"
#include "party.h"
#include "polymorph.h"
#include "questmanager.h"
#include "skill.h"
#include "start_position.h"
#include "utils.h"
#include "vector.h"
#include "war_map.h"

#include "safebox.h"
#include "shop.h"

#include "DragonSoul.h"
#include "OfflineShopConfig.h"
#include "buff_on_attributes.h"

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

#ifdef ENABLE_BATTLE_PASS
#include "battle_pass.h"
#endif

#include "OXEvent.h"

#include "ItemUtils.h"
#include "TextTagUtil.hpp"
#include "gm.h"

#include <stack>

#include "item.h"

const uint8_t g_aBuffOnAttrPoints[] = {
    // POINT_ENERGY
    //,
    POINT_COSTUME_ATTR_BONUS};

extern uint32_t GetHairPartByJob(uint32_t hair, uint8_t job); // char.cpp

struct FFindStone {
    std::map<uint32_t, CHARACTER*> m_mapStone;

    void operator()(CEntity* pEnt)
    {
        if (pEnt->IsType(ENTITY_CHARACTER) == true) {
            CHARACTER* pChar = (CHARACTER*)pEnt;

            if (pChar->IsStone() == true) {
                m_mapStone[(uint32_t)pChar->GetVID()] = pChar;
            }
        }
    }
};

struct FFindEarthDragon {
    CHARACTER* m_Dragon = nullptr;

    void operator()(CEntity*& pEnt)
    {
        if (pEnt->IsType(ENTITY_CHARACTER) == true) {
            CHARACTER* pChar = (CHARACTER*)pEnt;

            if (pChar->IsMonster() && 2496 == pChar->GetMobTable().dwVnum) {
                m_Dragon = pChar;
            }
        }
    }
};

// item socket 복사 -- by mhh
static void FN_copy_item_socket(CItem* dest, CItem* src)
{
    for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
        dest->SetSocket(i, src->GetSocket(i));
    }
}

/////////////////////////////////////////////////////////////////////////////
// ITEM HANDLING
/////////////////////////////////////////////////////////////////////////////
bool CHARACTER::CanHandleItem(bool bSkipCheckRefine, bool bSkipObserver)
{
    if (!bSkipObserver && !IsGM())
        if (m_bIsObserver)
            return false;

    if (IsShop())
        return false;

    if (!bSkipCheckRefine)
        if (m_bUnderRefine)
            return false;

    if (IsCubeOpen() || DragonSoul_RefineWindow_IsOpen())
        return false;

    if (IsAcceWindowOpen())
        return false;

    if (IsWarping())
        return false;

#ifdef __OFFLINE_SHOP__
    if (this->GetViewingOfflineShop()
#ifdef __OFFLINE_SHOP_USE_BUNDLE__
        || this->GetOfflineShopLastOpeningTime()
#endif
    )
        return false;
#endif

    return true;
}

CItem* CHARACTER::GetInventoryItem(const uint16_t& wCell) const
{
    return GetItem(TItemPos(INVENTORY, wCell));
}

CItem* CHARACTER::GetItem(const TItemPos& Cell) const
{
    if (!IsValidItemPosition(Cell))
        return nullptr;

    const uint16_t pos = Cell.cell;
    const uint8_t window_type = Cell.window_type;

    switch (window_type) {
        case INVENTORY:
            if (pos >= m_windows.inventory.GetSize()) {
                SPDLOG_ERROR("Invalid item cell {0} in inventory window", pos);
                return nullptr;
            }

            return m_windows.inventory.GetUnique(pos);

        case EQUIPMENT:
            if (pos >= WEAR_MAX_NUM) {
                SPDLOG_ERROR("Invalid item cell {0} in equipment window", pos);
                return nullptr;
            }

            return m_pointsInstant.wear[pos];

        case SWITCHBOT:
            if (pos >= SWITCHBOT_SLOT_COUNT) {
                SPDLOG_ERROR("Invalid item cell {0} in switchbot window", pos);
                return nullptr;
            }

            return m_pointsInstant.switchbot[pos];

        case BELT_INVENTORY:
            if (pos >= m_windows.belt.GetSize()) {
                SPDLOG_ERROR("Invalid item cell {0} in belt window", pos);
                return nullptr;
            }

            return m_windows.belt.GetUnique(pos);

        case DRAGON_SOUL_INVENTORY:
            if (pos >= DRAGON_SOUL_INVENTORY_MAX_NUM) {
                SPDLOG_ERROR("CHARACTER::GetInventoryItem: invalid DS item "
                             "cell "
                             "{}",
                             pos);
                return nullptr;
            }
            return m_pointsInstant.pDSItems[pos];

        default:
            SPDLOG_ERROR("Invalid window type {}", pos);
            return nullptr;
    }
    return nullptr;
}

CItem* CHARACTER::GetItem_NEW(const TItemPos& Cell) const
{
    CItem* cellItem = GetItem(Cell);
    if (cellItem)
        return cellItem;

    // There's no item in this cell, but that does not mean there is not an item
    // which currently uses up this cell.
    uint16_t bCell = Cell.cell;
    uint8_t bPage = bCell / (INVENTORY_PAGE_SIZE);

    int j = -2;
    for (int j = -2; j < 0; ++j) {
        auto p = bCell + (5 * j);

        if (p / (INVENTORY_PAGE_SIZE) != bPage)
            continue;

        if (p >= INVENTORY_MAX_NUM) // Eeh just for the sake of...
            continue;

        CItem* curCellItem = GetItem(TItemPos(INVENTORY, p));
        if (!curCellItem)
            continue;

        if (p + (curCellItem->GetSize() - 1) * 5 <
            Cell.cell) // Doesn't reach Cell.cell
            continue;

        return curCellItem;
    }

    return nullptr;
}

void CHARACTER::SetItem(const TItemPos& Cell, CItem* item)
{
    if (Cell.window_type == EQUIPMENT) {
        SetWear(Cell.cell, item);
        return;
    }

    if (Cell.window_type == SWITCHBOT) {
        SetSwitchbotItem(Cell.cell, item);
        return;
    }

    if (Cell.window_type == DRAGON_SOUL_INVENTORY) {
        CItem* current = m_pointsInstant.pDSItems[Cell.cell];

        if (current && !item) {
            current->SetOwner(nullptr);

            // Grid Clear
            if (Cell.cell < DRAGON_SOUL_INVENTORY_MAX_NUM) {
                for (int i = 0; i < current->GetSize(); ++i) {
                    int p = Cell.cell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

                    if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                        continue;

                    if (m_pointsInstant.pDSItems[p] &&
                        m_pointsInstant.pDSItems[p] != current)
                        continue;

                    m_pointsInstant.wDSItemGrid[p] = 0;
                }
            } else
                m_pointsInstant.wDSItemGrid[Cell.cell] = 0;
            // Grid Clear End
            m_pointsInstant.pDSItems[Cell.cell] = item;

            // Tell the client the item got removed.
            // This has to happen last.
            TPacketGCItemDel pack;
            pack.pos = Cell;
            GetDesc()->Send(HEADER_GC_ITEM_DEL, pack);
        } else if (!current && item) {
            STORM_ASSERT(!item->GetOwner(), "Already owned");

            // Grid put
            if (Cell.cell >= DRAGON_SOUL_INVENTORY_MAX_NUM) {
                SPDLOG_ERROR("CHARACTER::SetItem: invalid DS item cell {}",
                             Cell.cell);
                return;
            }

            if (Cell.cell < DRAGON_SOUL_INVENTORY_MAX_NUM) {
                for (int i = 0; i < item->GetSize(); ++i) {
                    int p = Cell.cell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

                    if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                        continue;

                    // wCell + 1 로 하는 것은 빈곳을 체크할 때 같은
                    // 아이템은 예외처리하기 위함
                    m_pointsInstant.wDSItemGrid[p] = Cell.cell + 1;
                }
            } else
                m_pointsInstant.wDSItemGrid[Cell.cell] = Cell.cell + 1;
            // Grid put end
            m_pointsInstant.pDSItems[Cell.cell] = item;

            item->SetOwner(this);
            item->SetPosition(Cell);
            item->UpdatePacket();
        } else {
            SPDLOG_ERROR("SetItem cannot overwrite items (cell: {0})",
                         Cell.cell);
            return;
        }

        return;
    }

    auto* grid = m_windows.Get(Cell.window_type);
    if (!grid) {
        SPDLOG_ERROR("Invalid window {0}", Cell.window_type);
        return;
    }

    auto* current = grid->GetUnique(Cell.cell);
    if (current && !item) {
        STORM_ASSERT(current->GetOwner() == this, "Sanity check");
        STORM_ASSERT(current->GetWindow() == Cell.window_type, "Sanity check");

        current->SetOwner(nullptr);
        grid->Clear(Cell.cell);

        //auto* newItem = FindSpecifyItem(current->GetVnum());
        //if (newItem) {
        //    // We dont want to get to else for dragon soul items
        //    if (!newItem->IsDragonSoul())
        //        ChainQuickslotItem(newItem->GetCell(), QUICKSLOT_TYPE_ITEM,
        //                           Cell.cell);
        //} else {
            SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, 255);
        //}

        // Tell the client the item got removed.
        // This has to happen last.
        TPacketGCItemDel pack;
        pack.pos = Cell;
        GetDesc()->Send(HEADER_GC_ITEM_DEL, pack);
    } else if (!current && item) {
        STORM_ASSERT(!item->GetOwner(), "Already owned");

        if (!grid->Put(item, Cell.cell)) {
            SPDLOG_ERROR("Failed to put item to {0}:{1}", Cell.window_type,
                         Cell.cell);
            return;
        }

        item->SetOwner(this);
        item->SetPosition(Cell);
        item->UpdatePacket();
    } else {
        SPDLOG_ERROR("SetItem cannot overwrite items (cell: {0})", Cell.cell);
        return;
    }
}

CItem* CHARACTER::GetWear(uint16_t bCell) const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->GetWear(bCell);
#endif
    // > WEAR_MAX_NUM : 용혼석 슬롯들.
    if (bCell >= WEAR_MAX_NUM) {
        SPDLOG_ERROR("CHARACTER::GetWear: invalid wear cell {}", bCell);
        return nullptr;
    }

    return m_pointsInstant.wear[bCell];
}

CItem* CHARACTER::GetSwitchbotItem(uint16_t bCell) const
{
    // > WEAR_MAX_NUM : 용혼석 슬롯들.
    if (bCell >= SWITCHBOT_SLOT_COUNT) {
        SPDLOG_ERROR("CHARACTER::GetSwitchbotItem: invalid switchbot cell {}",
                     bCell);
        return nullptr;
    }

    return m_pointsInstant.switchbot[bCell];
}

void CHARACTER::SetSwitchbotItem(uint16_t bCell, CItem* item)
{
    if (bCell >= SWITCHBOT_SLOT_COUNT) {
        SPDLOG_ERROR("CHARACTER::SetSwitchbotItem: invalid item cell {0}",
                     bCell);
        return;
    }

    const auto current = m_pointsInstant.switchbot[bCell];

    if (current && !item) {
        STORM_ASSERT(current->GetOwner() == this, "Sanity check");
        STORM_ASSERT(current->GetWindow() == SWITCHBOT, "Sanity check");
        STORM_ASSERT(current->GetCell() == bCell, "Sanity check");

        m_pointsInstant.switchbot[bCell] = nullptr;

        current->SetOwner(nullptr);

        // Tell the client the item got removed.
        // This has to happen last.
        TPacketGCItemDel pack;
        pack.pos = TItemPos(SWITCHBOT, bCell);
        GetDesc()->Send(HEADER_GC_ITEM_DEL, pack);
    } else if (!current && item) {
        STORM_ASSERT(!item->GetOwner(), "Already owned");

        item->SetPosition(TItemPos(SWITCHBOT, bCell));
        item->SetOwner(this);

        m_pointsInstant.switchbot[bCell] = item;

        item->UpdatePacket();
    } else if (current && item) {
        SPDLOG_ERROR("SetSwitchbotItem cannot overwrite items (cell: {0})",
                     bCell);
    }
}

void CHARACTER::SetWear(uint16_t bCell, CItem* item)
{
    if (bCell >= WEAR_MAX_NUM) {
        SPDLOG_ERROR("CHARACTER::SetWear: invalid item cell {0}", bCell);
        return;
    }

    auto* current = m_pointsInstant.wear[bCell];

    if (current && !item) {
        STORM_ASSERT(current->GetOwner() == this, "Sanity check");
        STORM_ASSERT(current->GetWindow() == EQUIPMENT, "Sanity check");
        STORM_ASSERT(current->GetCell() == bCell, "Sanity check");

        if (current->IsDragonSoul()) {
            DSManager::instance().DeactivateDragonSoul(current);
#ifdef ENABLE_DS_SET
            DragonSoul_HandleSetBonus();
#endif
        } else {
            current->ModifyPoints(false);
#ifdef __FAKE_PC__
            FakePC_Owner_ItemPoints(current, false);
#endif
        }
        events::Item::OnUnequip(this, current);

        BuffOnAttr_RemoveBuffsFromItem(current);

        m_pointsInstant.wear[bCell] = nullptr;

        current->SetOwner(nullptr);

        // Tell the client the item got removed.
        // This has to happen last.
        TPacketGCItemDel pack;
        pack.pos = TItemPos(EQUIPMENT, bCell);
        GetDesc()->Send(HEADER_GC_ITEM_DEL, pack);
    } else if (!current && item) {
        STORM_ASSERT(!item->GetOwner(), "Already owned");

        item->SetPosition(TItemPos(EQUIPMENT, bCell));
        item->SetOwner(this);

        m_pointsInstant.wear[bCell] = item;

        BuffOnAttr_AddBuffsFromItem(item);

        events::Item::OnEquip(this, item);
        if (item->IsDragonSoul()) {
            DSManager::instance().ActivateDragonSoul(item);
#ifdef ENABLE_DS_SET
            DragonSoul_HandleSetBonus();
#endif
        } else {
            item->ModifyPoints(true);
#ifdef __FAKE_PC__
            FakePC_Owner_ItemPoints(item, true);
#endif
        }

        item->UpdatePacket();
    } else if (current && item) {
        SPDLOG_ERROR("SetWear cannot overwrite items (cell: {0})", bCell);
        return;
    }

    /*if (bCell == WEAR_WEAPON) {
        // 귀검 사용 시 벗는 것이라면 효과를 없애야 한다.
        if (FindAffect(SKILL_GWIGEOM))
            RemoveAffect(SKILL_GWIGEOM);

        if (FindAffect(SKILL_GEOMKYUNG))
            RemoveAffect(SKILL_GEOMKYUNG);
    }*/

    ComputeBattlePoints();
    UpdatePacket();
}

void CHARACTER::ClearItem()
{
    for (uint32_t i = 0; i < m_windows.inventory.GetSize(); ++i) {
        auto* item = m_windows.inventory.GetPos(i);
        if (!item)
            continue;

        item->SetSkipSave(true);
        ITEM_MANAGER::instance().FlushDelayedSave(item);

        item->RemoveFromCharacter();
        M2_DESTROY_ITEM(item);

        SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
        m_windows.inventory.Clear(i);
    }

    for (uint32_t i = 0; i < WEAR_MAX_NUM; ++i) {
        auto* item = m_pointsInstant.wear[i];
        if (!item)
            continue;

        item->SetSkipSave(true);
        ITEM_MANAGER::instance().FlushDelayedSave(item);

        item->RemoveFromCharacter();
        M2_DESTROY_ITEM(item);

        m_pointsInstant.wear[i] = nullptr;
    }

    for (uint32_t i = 0; i < SWITCHBOT_SLOT_COUNT; ++i) {
        auto* item = m_pointsInstant.switchbot[i];
        if (!item)
            continue;

        item->SetSkipSave(true);
        ITEM_MANAGER::instance().FlushDelayedSave(item);

        item->RemoveFromCharacter();
        M2_DESTROY_ITEM(item);

        m_pointsInstant.switchbot[i] = nullptr;
    }

    for (uint32_t i = 0; i < m_windows.belt.GetSize(); ++i) {
        auto* item = m_windows.belt.GetPos(i);
        if (!item)
            continue;

        item->SetSkipSave(true);
        ITEM_MANAGER::instance().FlushDelayedSave(item);

        item->RemoveFromCharacter();
        M2_DESTROY_ITEM(item);

        m_windows.belt.Clear(i);
    }

    int i;
    CItem* item;
    for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i) {
        if ((item = GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i)))) {
            item->SetSkipSave(true);
            ITEM_MANAGER::instance().FlushDelayedSave(item);

            item->RemoveFromCharacter();
            M2_DESTROY_ITEM(item);
        }
    }
}

bool CHARACTER::IsEmptyItemGrid(TItemPos Cell, uint8_t bSize,
                                const CItem* except) const
{
    const ItemGrid* grid = m_windows.Get(Cell.window_type);
    if (!grid) {
        SPDLOG_ERROR("Invalid window type {0}", Cell.window_type);
        return false;
    }

    return grid->IsEmpty(Cell.cell, bSize, const_cast<CItem**>(&except));
}

bool CHARACTER::IsEmptyItemGridDS(const TItemPos& Cell, uint8_t bSize,
                                  int iExceptionCell) const
{
    switch (Cell.window_type) {
        case DRAGON_SOUL_INVENTORY: {
            uint16_t wCell = Cell.cell;
            if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                return false;

            // bItemCell은 0이 false임을 나타내기 위해 + 1 해서 처리한다.
            // 따라서 iExceptionCell에 1을 더해 비교한다.
            iExceptionCell++;

            if (m_pointsInstant.wDSItemGrid[wCell]) {
                if (m_pointsInstant.wDSItemGrid[wCell] == iExceptionCell) {
                    if (bSize == 1)
                        return true;

                    int j = 1;

                    do {
                        uint16_t p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

                        if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                            return false;

                        if (m_pointsInstant.wDSItemGrid[p])
                            if (m_pointsInstant.wDSItemGrid[p] !=
                                iExceptionCell)
                                return false;
                    } while (++j < bSize);

                    return true;
                }
                return false;
            }

            // 크기가 1이면 한칸을 차지하는 것이므로 그냥 리턴
            if (1 == bSize)
                return true;
            int j = 1;

            do {
                uint16_t p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

                if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                    return false;

                if (m_pointsInstant.wDSItemGrid[p])
                    if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
                        return false;
            } while (++j < bSize);

            return true;
        }
    }
    return false;
}

bool CHARACTER::IsEmptyItemGridSpecialDS(const TItemPos& Cell, uint8_t bSize,
                                         int iExceptionCell,
                                         std::vector<uint16_t>& vec) const
{
    if (find(vec.begin(), vec.end(), Cell.cell) != vec.end()) {
        return false;
    }

    switch (Cell.window_type) {
        case DRAGON_SOUL_INVENTORY: {
            auto wCell = Cell.cell;
            if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                return false;

            // bItemCell은 0이 false임을 나타내기 위해 + 1 해서 처리한다.
            // 따라서 iExceptionCell에 1을 더해 비교한다.
            iExceptionCell++;

            if (m_pointsInstant.wDSItemGrid[wCell]) {
                if (m_pointsInstant.wDSItemGrid[wCell] == iExceptionCell) {
                    if (bSize == 1)
                        return true;

                    int j = 1;

                    do {
                        uint16_t p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

                        if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                            return false;

                        if (m_pointsInstant.wDSItemGrid[p])
                            if (m_pointsInstant.wDSItemGrid[p] !=
                                iExceptionCell)
                                return false;
                    } while (++j < bSize);

                    return true;
                }
                return false;
            }

            // 크기가 1이면 한칸을 차지하는 것이므로 그냥 리턴
            if (1 == bSize)
                return true;
            int j = 1;

            do {
                uint16_t p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

                if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
                    return false;

                if (m_pointsInstant.wDSItemGrid[p])
                    if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
                        return false;
            } while (++j < bSize);

            return true;
        }
    }
    return false;
}

int CHARACTER::GetEmptyInventory(uint8_t size) const
{
    return m_windows.inventory.FindBlank(size).value_or(-1);
}

int CHARACTER::GetEmptyInventory(uint8_t size, uint8_t type) const
{
    switch (type) {
        case ITEM_MATERIAL:
            return m_windows.inventory
                .FindBlankBetweenPages(size, NORMAL_INVENTORY_MAX_PAGE,
                                       MATERIAL_INVENTORY_MAX_PAGE)
                .value_or(-1);
        case ITEM_COSTUME:
            return m_windows.inventory
                .FindBlankBetweenPages(size, MATERIAL_INVENTORY_MAX_PAGE,
                                       COSTUME_INVENTORY_MAX_PAGE)
                .value_or(-1);
        default:
            return m_windows.inventory
                .FindBlankBetweenPages(size, NORMAL_INVENTORY_MIN_PAGE,
                                       NORMAL_INVENTORY_MAX_PAGE)
                .value_or(-1);
    }
}

int CHARACTER::GetEmptyInventory(const CItem* item) const
{
    return GetEmptyInventory(item->GetSize(), item->GetItemType());
}
int CHARACTER::GetEmptyCostumeInventoryCount(uint8_t size) const
{
    int emptyCount = 0;
    for (int i = INVENTORY_PAGE_SIZE * MATERIAL_INVENTORY_MAX_PAGE; i < INVENTORY_PAGE_SIZE * COSTUME_INVENTORY_MAX_PAGE; ++i) {
        if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
            ++emptyCount;
    }

    return emptyCount;
}

int CHARACTER::GetEmptyMaterialInventoryCount(uint8_t size) const
{
    int emptyCount = 0;
    for (int i = INVENTORY_PAGE_SIZE * NORMAL_INVENTORY_MAX_PAGE; i < INVENTORY_PAGE_SIZE * MATERIAL_INVENTORY_MAX_PAGE; ++i) {
        if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
            ++emptyCount;
    }

    return emptyCount;
}
int CHARACTER::GetEmptyNormalInventoryCount(uint8_t size) const
{
    int emptyCount = 0;
    for (int i = 0; i < INVENTORY_PAGE_SIZE * NORMAL_INVENTORY_MAX_PAGE; ++i) {
        if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
            ++emptyCount;
    }

    return emptyCount;
}

int CHARACTER::GetEmptyInventoryCount(uint8_t size) const
{
    int emptyCount = 0;
    for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
        if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
            ++emptyCount;
    }

    return emptyCount;
}

int CHARACTER::GetEmptyDragonSoulInventory(const TItemTable* pItemTable) const
{
    if (nullptr == pItemTable || pItemTable->bType != ITEM_DS)
        return -1;
    if (!DragonSoul_IsQualified()) {
        return -1;
    }
    uint8_t bSize = pItemTable->bSize;
    uint16_t wBaseCell = DSManager::instance().GetBasePosition(pItemTable);

    if (std::numeric_limits<uint16_t>::max() == wBaseCell)
        return -1;

    for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
        if (IsEmptyItemGridDS(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell),
                              bSize))
            return i + wBaseCell;

    return -1;
}

int CHARACTER::GetEmptyDragonSoulInventory(const CItem* pItem,
                                           int iExceptCell /*= -1*/) const
{
    if (nullptr == pItem || !pItem->IsDragonSoul())
        return -1;
    if (!DragonSoul_IsQualified()) {
        return -1;
    }
    uint8_t bSize = pItem->GetSize();
    uint16_t wBaseCell = DSManager::instance().GetBasePosition(pItem);

    if (std::numeric_limits<uint16_t>::max() == wBaseCell)
        return -1;

    for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
        if (IsEmptyItemGridDS(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell),
                              bSize, iExceptCell))
            return i + wBaseCell;

    return -1;
}

int CHARACTER::GetEmptyDragonSoulInventoryWithExceptions(
    CItem* pItem, std::vector<uint16_t>& vec /*= -1*/) const
{
    if (nullptr == pItem || !pItem->IsDragonSoul())
        return -1;
    if (!DragonSoul_IsQualified()) {
        return -1;
    }
    uint8_t bSize = pItem->GetSize();
    uint16_t wBaseCell = DSManager::instance().GetBasePosition(pItem);

    if (std::numeric_limits<uint16_t>::max() == wBaseCell)
        return -1;

    for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
        if (IsEmptyItemGridSpecialDS(
                TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell), bSize, -1, vec))
            return i + wBaseCell;

    return -1;
}

void CHARACTER::CopyDragonSoulItemGrid(
    std::vector<uint16_t>& vDragonSoulItemGrid) const
{
    vDragonSoulItemGrid.resize(DRAGON_SOUL_INVENTORY_MAX_NUM);

    copy(m_pointsInstant.wDSItemGrid,
         m_pointsInstant.wDSItemGrid + DRAGON_SOUL_INVENTORY_MAX_NUM,
         vDragonSoulItemGrid.begin());
}

int CHARACTER::CountEmptyInventory() const
{
    int count = 0;

    for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
        if (GetInventoryItem(i))
            count += GetInventoryItem(i)->GetSize();

    return (INVENTORY_MAX_NUM - count);
}

void TransformRefineItem(CItem* pkOldItem, CItem* pkNewItem)
{
    // ACCESSORY_REFINE
    if (pkOldItem->IsAccessoryForSocket()) {
        for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
            pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
        }
        // pkNewItem->StartAccessorySocketExpireEvent();
    }
    // END_OF_ACCESSORY_REFINE
    else {
        // 여기서 깨진석이 자동적으로 청소 됨
        for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
            if (!pkOldItem->GetSocket(i))
                break;
            pkNewItem->SetSocket(i, 1);
        }

        // 소켓 설정
        int slot = 0;

        for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
            long socket = pkOldItem->GetSocket(i);

            if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
                pkNewItem->SetSocket(slot++, socket);
        }
    }

    // 매직 아이템 설정
    pkOldItem->CopyAttributeTo(pkNewItem);
}

void NotifyRefineSuccess(CHARACTER* ch, CItem* item, const char* way,
                         int32_t refineType)
{
    if (nullptr != ch && item != nullptr) {
        ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineSuceeded");
        LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(),
                                         item->GetID(), item->GetRefineLevel(),
                                         1, way);
    }
}

void NotifyRefineFail(CHARACTER* ch, CItem* item, const char* way, int success,
                      int32_t refineType)
{
    if (nullptr != ch && nullptr != item) {
        ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailed");

        LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(),
                                         item->GetID(), item->GetRefineLevel(),
                                         success, way);
    }
}

void CHARACTER::SetRefineNPC(CHARACTER* ch)
{
    if (ch != nullptr) {
        m_dwRefineNPCVID = ch->GetVID();
    } else {
        m_dwRefineNPCVID = 0;
    }
}

struct FindBlacksmith {
    CHARACTER* m_ch;
    bool found;
    bool m_throughGuild;
    int MAX_BLACKSMITH_DIST =
        7500; // Around 5000 is the max distance, but to have a bit of room.

    FindBlacksmith(CHARACTER* ch, bool throughGuild)
    {
        m_ch = ch;
        found = false;
        m_throughGuild = throughGuild;
    }

    void operator()(CEntity* ent)
    {
        if (found) // Skip all the next entities - we already got what we wanted
            return;

        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = (CHARACTER*)ent;
            if (ch->IsNPC()) {
                uint32_t vnum = ch->GetMobTable().dwVnum;
                switch (vnum) {
                    case BLACKSMITH_WEAPON_MOB:
                    case BLACKSMITH_ARMOR_MOB:
                    case BLACKSMITH_ACCESSORY_MOB: {
                        if (!m_throughGuild) // We can ignore proximity to guild
                            // blacksmiths if we are not using
                            // guild
                            return;

                        int dist = DISTANCE_APPROX(m_ch->GetX() - ch->GetX(),
                                                   m_ch->GetY() - ch->GetY());
                        if (gConfig.testServer)
                            SPDLOG_INFO("Guild blacksmith found within %d "
                                        "distance "
                                        "units.",
                                        dist);

                        if (dist < MAX_BLACKSMITH_DIST) {
                            found = true;
                        }

                        break;
                    }

                    case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
                    case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
                    case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
                    case BLACKSMITH_MOB:
                    case BLACKSMITH2_MOB: {
                        if (m_throughGuild) // Poximity to non-guild blacksmiths
                            // is irrelevant if we are using
                            // guild
                            return;

                        int dist = DISTANCE_APPROX(m_ch->GetX() - ch->GetX(),
                                                   m_ch->GetY() - ch->GetY());
                        if (gConfig.testServer)
                            SPDLOG_INFO("Blacksmith (%lu) found within %d "
                                        "distance "
                                        "units.",
                                        vnum, dist);

                        if (dist < MAX_BLACKSMITH_DIST) {
                            found = true;
                        }

                        break;
                    }
                }
            }
        }
    }
};

bool CHARACTER::DoRefine(CItem* item, int32_t refineType)
{
    if (!CanHandleItem(true)) {
        ClearRefineMode();
        return false;
    }

    if (gConfig.testServer)
        if (m_dwRefineNPCVID)
            SPDLOG_INFO("Refine npcVID is %lu", m_dwRefineNPCVID);

    FindBlacksmith f(this, IsRefineThroughGuild());
    ForEachSeen(f);

    if (!f.found) {
        LogManager::instance().HackLog("REFINE_FAR_BLACKSMITH", this);
        // We let continue - GMs will decide/ban.
    }

    if (item->IsSealed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You cannot refine a sealed item");
        return false;
    }

    //개량 시간제한 : upgrade_refine_scroll.quest 에서 개량후 5분이내에 일반
    //개량을 진행할수 없음
    if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") !=
        0) {
        if (get_global_time() <
            quest::CQuestManager::instance().GetEventFlag("update_refine_"
                                                          "time") +
                (60 * 5)) {
            SPDLOG_INFO("can't refine %d %s", GetPlayerID(), GetName());
            return false;
        }
    }

    const auto* prt =
        CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

    if (!prt)
        return false;

    uint32_t result_vnum = item->GetRefinedVnum();

    // REFINE_COST
    Gold cost = ComputeRefineFee(prt->cost);

    int RefineChance = GetQuestFlag("main_quest_lv7.refine_chance");
    if (RefineChance > 0 && item->CheckItemUseLevel(20) &&
        item->GetItemType() == ITEM_WEAPON) {
        cost = 0;
    }
    // END_OF_REFINE_COST

    if (result_vnum == 0) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "더 이상 개량할 수 없습니다.");
        return false;
    }

    if (item->GetItemType() == ITEM_USE && item->GetSubType() == USE_TUNING)
        return false;

    const auto pProto =
        ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

    if (!pProto) {
        SPDLOG_ERROR("DoRefine NOT GET ITEM PROTO %d", item->GetRefinedVnum());
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "이 아이템은 개량할 수 없습니다.");
        return false;
    }

    // REFINE_COST
    if (GetGold() < cost) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "개량을 하기 위한 돈이 부족합니다.");
        return false;
    }

    if (refineType != REFINE_TYPE_MONEY_ONLY) {
        for (auto material : prt->materials) {
            if (CountSpecifyItem(material.vnum, item) < material.count) {
                if (gConfig.testServer) {
                    ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d",
                               material.vnum,
                               CountSpecifyItem(material.vnum),
                               material.count);
                }
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "개량을 하기 위한 재료가 부족합니다.");
                return false;
            }
        }

        for (auto material : prt->materials)
            RemoveSpecifyItem(material.vnum, material.count, item);
    }

    int prob = Random::get(1, 100);
    int itemProb = prt->prob;

    if (IsRefineThroughGuild() || refineType == REFINE_TYPE_MONEY_ONLY)
        prob -= 10;

    // END_OF_REFINE_COST

    for (auto enhance_material : prt->enhance_materials) {
        if (CountSpecifyItem(enhance_material.vnum, item) >= enhance_material.count) {
            itemProb = std::min(100, itemProb + enhance_material.prob);

            RemoveSpecifyItem(enhance_material.vnum,
                              enhance_material.count,
                              item);

            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "Your refine chance was increased by %d%%",
                               enhance_material.prob);
        }
    }

    // Reduce the free upgrades left (if it is a valid free upgrade)
    if (RefineChance > 0 && cost == 0) {
        SetQuestFlag("main_quest_lv7.refine_chance", RefineChance - 1);
    }
    // End of free upgrade

    // Labor day refine chance increase only for normal blacksmith. T#154
    // <MartPwnS>
    if (refineType != REFINE_TYPE_MONEY_ONLY &&
        quest::CQuestManager::instance().GetEventFlag("labor_day") == 1) {
        itemProb = std::min(100, itemProb + 5); // +5% increase
    }
    // End of labor day refine chance increase

    if (prob <= itemProb) {
        // 성공! 모든 아이템이 사라지고, 같은 속성의 다른 아이템 획득
        CItem* pkNewItem =
            ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

        if (pkNewItem) {
            int wCell = -1;
            if (!item->IsStackable())
                wCell = item->GetCell();
            else
                wCell = GetEmptyInventory(pkNewItem);

            if (wCell == -1) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "Not enough space in your inventory");
                return false;
            }

#ifdef ENABLE_BATTLE_PASS
            uint8_t bBattlePassId = GetBattlePassId();
            if (bBattlePassId) {
                uint32_t dwItemVnum, dwCount;
                if (CBattlePass::instance().BattlePassMissionGetInfo(
                        bBattlePassId, REFINE_ITEM, &dwItemVnum, &dwCount)) {
                    if (dwItemVnum == item->GetVnum() &&
                        GetMissionProgress(REFINE_ITEM, bBattlePassId) <
                            dwCount)
                        UpdateMissionProgress(REFINE_ITEM, bBattlePassId, 1,
                                              dwCount);
                }
            }
#endif

            ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
            LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS",
                                           pkNewItem->GetName());

            // DETAIL_REFINE_LOG
            NotifyRefineSuccess(this, item,
                                IsRefineThroughGuild() ? "GUILD" : "POWER",
                                refineType);
            DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE,
                                               item->GetVnum(), -cost);
            auto isStillExist = item->SetCount(item->GetCount() - 1);
            // LogManager::instance().ItemLog(this, item, "REFINE",
            //                               "REMOVE (REFINE SUCCESS)");
            // END_OF_DETAIL_REFINE_LOG
            if (isStillExist) {
                const auto givenItem = AutoGiveItem(pkNewItem);
                if (givenItem) {
                    pkNewItem = givenItem;
                } else {
                    pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
                }
            } else {
                pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
            }

            ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

            SPDLOG_INFO("Refine Success %d", cost);
            SPDLOG_INFO("PayPee %d", cost);
            PayRefineFee(cost);
            SPDLOG_INFO("PayPee End %d", cost);
        } else {
            // DETAIL_REFINE_LOG
            // 아이템 생성에 실패 -> 개량 실패로 간주
            SPDLOG_ERROR("DoRefine: Cannot create result item {}", result_vnum);
            NotifyRefineFail(this, item,
                             IsRefineThroughGuild() ? "GUILD" : "POWER", 0,
                             refineType);
            // END_OF_DETAIL_REFINE_LOG
        }
    } else {
        // 실패! 모든 아이템이 사라짐.
        DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(),
                                           -cost);
        NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER",
                         0, refineType);
        item->SetCount(item->GetCount() - 1);
        // LogManager::instance().ItemLog(this, item, "REFINE",
        //                              "REMOVE (REFINE FAIL)");

        PayRefineFee(cost);
    }

    return true;
}

enum enum_RefineScrolls {
    CHUKBOK_SCROLL = 0,
    HYUNIRON_CHN = 1,
    // 중국에서만 사용
    YONGSIN_SCROLL = 2,
    MUSIN_SCROLL = 3,
    YAGONG_SCROLL = 4,
    MEMO_SCROLL = 5,
    BDRAGON_SCROLL = 6,
    YAGHAN_STONE = 7,
    JOHEUN_STONE = 8,
    KEUN_STONE = 9,
};

bool CHARACTER::DoRefineWithScroll(CItem* item, int32_t refineType)
{
    if (!CanHandleItem(true)) {
        ClearRefineMode();
        return false;
    }

    if (item->IsSealed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You cannot define a sealed item");
        return false;
    }

    ClearRefineMode();

    //개량 시간제한 : upgrade_refine_scroll.quest 에서 개량후 5분이내에 일반
    //개량을 진행할수 없음
    if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") !=
        0) {
        if (get_global_time() <
            quest::CQuestManager::instance().GetEventFlag("update_refine_"
                                                          "time") +
                (60 * 5)) {
            SPDLOG_INFO("can't refine %d %s", GetPlayerID(), GetName());
            return false;
        }
    }

    const auto* prt =
        CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

    if (!prt)
        return false;

    CItem* pkItemScroll;

    // 개량서 체크
    if (m_iRefineAdditionalCell < 0)
        return false;

    pkItemScroll = GetInventoryItem(m_iRefineAdditionalCell);

    if (!pkItemScroll)
        return false;

    if (!(pkItemScroll->GetItemType() == ITEM_USE &&
          pkItemScroll->GetSubType() == USE_TUNING))
        return false;

    if (pkItemScroll->GetVnum() == item->GetVnum())
        return false;

    // DD-only stones. Not this type of refining!
    if (pkItemScroll->GetValue(0) == YAGHAN_STONE ||
        pkItemScroll->GetValue(0) == JOHEUN_STONE ||
        pkItemScroll->GetValue(0) == KEUN_STONE)
        return false;

    uint32_t result_vnum = item->GetRefinedVnum();
    uint32_t result_fail_vnum = item->GetRefineFromVnum();

    if (result_vnum == 0) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "더 이상 개량할 수 없습니다.");
        return false;
    }

    // MUSIN_SCROLL
    if (pkItemScroll->GetValue(0) == MUSIN_SCROLL) {
        if (item->GetRefineLevel() >= 4) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 개량서로 더 이상 개량할 수 없습니다.");
            return false;
        }
    }
    // END_OF_MUSIC_SCROLL

    else if (pkItemScroll->GetValue(0) == MEMO_SCROLL) {
        if (item->GetRefineLevel() != pkItemScroll->GetValue(1)) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 개량서로 개량할 수 없습니다.");
            return false;
        }
    } else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL) {
        if (item->GetItemType() != ITEM_METIN || item->GetRefineLevel() != 4) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 아이템으로 개량할 수 없습니다.");
            return false;
        }
    }

    const auto* pProto =
        ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

    if (!pProto) {
        SPDLOG_ERROR("DoRefineWithScroll NOT GET ITEM PROTO %d",
                     item->GetRefinedVnum());
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "이 아이템은 개량할 수 없습니다.");
        return false;
    }

    // Check level limit in korea only

    if (GetGold() < prt->cost) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "개량을 하기 위한 돈이 부족합니다.");
        return false;
    }

    for (int i = 0; i < prt->materials.size(); ++i) {
        if (CountSpecifyItem(prt->materials[i].vnum, item) <
            prt->materials[i].count) {
            if (gConfig.testServer) {
                ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d",
                           prt->materials[i].vnum,
                           CountSpecifyItem(prt->materials[i].vnum, item),
                           prt->materials[i].count);
            }
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "개량을 하기 위한 재료가 부족합니다.");
            return false;
        }
    }

    for (int i = 0; i < prt->materials.size(); ++i)
        RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count, item);

    int prob = Random::get(1, 100);
    int success_prob = prt->prob;

    for (int i = 0; i < prt->enhance_materials.size(); ++i) {
        if (CountSpecifyItem(prt->enhance_materials[i].vnum, item) >=
            prt->enhance_materials[i].count) {
            success_prob =
                std::min(100, success_prob + prt->enhance_materials[i].prob);

            RemoveSpecifyItem(prt->enhance_materials[i].vnum,
                              prt->enhance_materials[i].count, item);

            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "Your refine chance was increased by %d%%",
                               prt->enhance_materials[i].prob);
        }
    }

    bool bDestroyWhenFail = false;

    const char* szRefineType = "SCROLL";

    if (pkItemScroll->GetValue(0) == HYUNIRON_CHN ||
        pkItemScroll->GetValue(0) == YONGSIN_SCROLL ||
        pkItemScroll->GetValue(0) == YAGONG_SCROLL) {
        const uint8_t hyuniron_prob[9] = {100, 75, 65, 55, 45, 40, 35, 25, 20};
        // const char yagong_prob[9] = { 100, 100, 90, 80, 70, 60, 50, 30, 20 };
        // MartPwnS not in use

        if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL) {
            success_prob =
                hyuniron_prob[std::clamp(item->GetRefineLevel(), 0, 8)];
        } else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL) {
            success_prob = std::clamp(prt->prob + 10, 1, 100);

            if (gConfig.testServer)
                ChatPacket(CHAT_TYPE_INFO,
                           "Original prob %d%%, Master Compass raise to %d%%",
                           prt->prob, success_prob);
        }

        if (gConfig.testServer) {
            ChatPacket(CHAT_TYPE_INFO,
                       "[Test server] Success_Prob %d, RefineLevel %d ",
                       success_prob, item->GetRefineLevel());
        }

        if (pkItemScroll->GetValue(0) == HYUNIRON_CHN) {
            bDestroyWhenFail = true;
        }

        // DETAIL_REFINE_LOG
        if (pkItemScroll->GetValue(0) == HYUNIRON_CHN) {
            szRefineType = "HYUNIRON";
        } else if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL) {
            szRefineType = "GOD_SCROLL";
        } else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL) {
            szRefineType = "YAGONG_SCROLL";
        }
        // END_OF_DETAIL_REFINE_LOG
    }

    // DETAIL_REFINE_LOG
    if (pkItemScroll->GetValue(0) ==
        MUSIN_SCROLL) // 무신의 축복서는 100% 성공 (+4까지만)
    {
        success_prob = 100;

        szRefineType = "MUSIN_SCROLL";
    }
    // END_OF_DETAIL_REFINE_LOG
    else if (pkItemScroll->GetValue(0) == MEMO_SCROLL) {
        success_prob = 100;
        szRefineType = "MEMO_SCROLL";
    } else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL) {
        success_prob = 80;
        szRefineType = "BDRAGON_SCROLL";
    }

    // if(pkItemScroll->GetVnum() == 70060)
    //{
    //	success_prob = 100;
    //	szRefineType = "EPIC_SCROLL";
    //}
    if (pkItemScroll->GetValue(3) == 0)
        pkItemScroll->SetCount(pkItemScroll->GetCount() - 1);

    if (prob <= success_prob) {
        // 성공! 모든 아이템이 사라지고, 같은 속성의 다른 아이템 획득
        CItem* pkNewItem =
            ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

        if (pkNewItem) {
            int wCell = -1;
            if (item->IsStackable() && item->GetCount() > 1)
                wCell = GetEmptyInventory(pkNewItem);
            else
                wCell = item->GetCell();

            if (wCell == -1) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "Not enough space in your inventory");
                return false;
            }

#ifdef ENABLE_BATTLE_PASS
            uint8_t bBattlePassId = GetBattlePassId();
            if (bBattlePassId) {
                uint32_t dwItemVnum, dwCount;
                if (CBattlePass::instance().BattlePassMissionGetInfo(
                        bBattlePassId, REFINE_ITEM, &dwItemVnum, &dwCount)) {
                    if (dwItemVnum == item->GetVnum() &&
                        GetMissionProgress(REFINE_ITEM, bBattlePassId) <
                            dwCount)
                        UpdateMissionProgress(REFINE_ITEM, bBattlePassId, 1,
                                              dwCount);
                }
            }
#endif

            ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
            LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS",
                                           pkNewItem->GetName());

            NotifyRefineSuccess(this, item, szRefineType, refineType);
            DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE,
                                               item->GetVnum(), -prt->cost);

            if (item->IsStackable())
                item->SetCount(item->GetCount() - 1);
            else
                ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE "
                                                          "SUCCESS)");

            pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
            ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
            PayRefineFee(prt->cost);
        } else {
            // 아이템 생성에 실패 -> 개량 실패로 간주
            SPDLOG_ERROR("DoRefineWithScroll: Cannot create result item {}",
                         result_vnum);
            NotifyRefineFail(this, item, szRefineType, 0, refineType);
        }
    } else if (!bDestroyWhenFail && result_fail_vnum) {
        // 실패! 모든 아이템이 사라지고, 같은 속성의 낮은 등급의 아이템 획득
        CItem* pkNewItem =
            ITEM_MANAGER::instance().CreateItem(result_fail_vnum, 1, 0, false);

        if (pkNewItem) {
            int wCell = -1;
            if (item->IsStackable())
                wCell = GetEmptyInventory(pkNewItem);
            else
                wCell = item->GetCell();

            if (wCell == -1) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "Not enough space in your inventory");
                return false;
            }

#ifdef ENABLE_BATTLE_PASS
            uint8_t bBattlePassId = GetBattlePassId();
            if (bBattlePassId) {
                uint32_t dwItemVnum, dwCount;
                if (CBattlePass::instance().BattlePassMissionGetInfo(
                        bBattlePassId, REFINE_ITEM, &dwItemVnum, &dwCount)) {
                    if (dwItemVnum == item->GetVnum() &&
                        GetMissionProgress(REFINE_ITEM, bBattlePassId) <
                            dwCount)
                        UpdateMissionProgress(REFINE_ITEM, bBattlePassId, 1,
                                              dwCount);
                }
            }
#endif

            ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
            LogManager::instance().ItemLog(this, pkNewItem, "REFINE FAIL",
                                           pkNewItem->GetName());

            DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE,
                                               item->GetVnum(), -prt->cost);
            NotifyRefineFail(this, item, szRefineType, -1, refineType);
            if (item->IsStackable()) {
                item->SetCount(item->GetCount() - 1);
            } else {
                ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE "
                                                          "FAIL)");
            }
            pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
            ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

            PayRefineFee(prt->cost);
        } else {
            // 아이템 생성에 실패 -> 개량 실패로 간주
            SPDLOG_ERROR("DoRefineWithScroll: Cannot create failure item {}",
                         result_fail_vnum);
            NotifyRefineFail(this, item, szRefineType, 0, refineType);
        }
    } else {
        NotifyRefineFail(this, item, szRefineType, 0,
                         refineType); // 개량시 아이템 사라지지 않음

        PayRefineFee(prt->cost);
    }

    return true;
}

bool CHARACTER::RefineInformation(uint16_t bCell, uint8_t bType,
                                  int iAdditionalCell)
{
    if (bCell > INVENTORY_MAX_NUM)
        return false;

    CItem* item = GetInventoryItem(bCell);

    if (!item)
        return false;

    // REFINE_COST
    if (bType == REFINE_TYPE_MONEY_ONLY && !GetQuestFlag("deviltower_zone.can_"
                                                         "refine")) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "사귀 타워 완료 보상은 한번까지 사용가능합니다.");
        return false;
    }
    // END_OF_REFINE_COST

    TPacketGCRefineInformation p;
    p.pos = bCell;
    p.src_vnum = item->GetVnum();
    p.result_vnum = item->GetRefinedVnum();
    p.type = bType;

    if (p.result_vnum == 0) {
        SPDLOG_ERROR("RefineInformation p.result_vnum == 0");
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "이 아이템은 개량할 수 없습니다.");
        return false;
    }

    if (item->GetItemType() == ITEM_USE && item->GetSubType() == USE_TUNING) {
        if (bType == 0) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 아이템은 이 방식으로는 개량할 수 없습니다.");
            return false;
        }
        CItem* itemScroll = GetInventoryItem(iAdditionalCell);
        if (!itemScroll || item->GetVnum() == itemScroll->GetVnum()) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "같은 개량서를 합칠 수는 없습니다.");
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "축복의 서와 현철을 합칠 수 있습니다.");
            return false;
        }
    }

    CRefineManager& rm = CRefineManager::instance();

    const auto* prt = rm.GetRefineRecipe(item->GetRefineSet());

    if (!prt) {
        SPDLOG_ERROR("RefineInformation NOT GET REFINE SET %d",
                     item->GetRefineSet());
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "이 아이템은 개량할 수 없습니다.");
        return false;
    }

    // REFINE_COST

    // MAIN_QUEST_LV7
    p.cost = ComputeRefineFee(prt->cost);

    if (GetQuestFlag("main_quest_lv7.refine_chance") > 0) {
        if (!item->CheckItemUseLevel(20) ||
            item->GetItemType() != ITEM_WEAPON) {
            ChatPacket(CHAT_TYPE_COMMAND, "FreeUpgradeNotApplicable");
        } else {
            p.cost = 0;
        }
    }

    // END_MAIN_QUEST_LV7
    p.prob = prt->prob;
    if (bType == REFINE_TYPE_MONEY_ONLY) {
        p.materials.clear();
        p.enhance_mat = prt->enhance_materials;
    } else {
        p.materials = prt->materials;
        p.enhance_mat = prt->enhance_materials;
    }
    // END_OF_REFINE_COST

    GetDesc()->Send(HEADER_GC_REFINE_INFORMATION, p);

    SetRefineMode(iAdditionalCell);
    return true;
}

bool CHARACTER::RefineItem(CItem* pkItem, CItem* pkTarget)
{
    if (!CanHandleItem())
        return false;

    if (pkItem->GetSubType() == USE_TUNING) {
        if (pkItem->GetValue(0) == MUSIN_SCROLL)
            RefineInformation(pkTarget->GetCell(), REFINE_TYPE_MUSIN,
                              pkItem->GetCell());
        // END_OF_MUSIN_SCROLL
        else if (pkItem->GetValue(0) == HYUNIRON_CHN)
            RefineInformation(pkTarget->GetCell(), REFINE_TYPE_HYUNIRON,
                              pkItem->GetCell());
        else if (pkItem->GetValue(0) == BDRAGON_SCROLL) {
            if (pkTarget->GetRefineSet() != 702)
                return false;
            RefineInformation(pkTarget->GetCell(), REFINE_TYPE_BDRAGON,
                              pkItem->GetCell());
        } else {
            if (pkTarget->GetRefineSet() == 501)
                return false;
            RefineInformation(pkTarget->GetCell(), REFINE_TYPE_SCROLL,
                              pkItem->GetCell());
        }
    } else if (pkItem->GetSubType() == USE_DETACHMENT &&
               IS_SET(pkTarget->GetFlag(), ITEM_FLAG_REFINEABLE)) {
        LogManager::instance().ItemLog(this, pkTarget, "USE_DETACHMENT",
                                       pkTarget->GetName());

        bool bHasMetinStone = false;

        for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++) {
            long socket = pkTarget->GetSocket(i);
            if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM) {
                bHasMetinStone = true;
                break;
            }
        }

        if (bHasMetinStone) {
            for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
                long socket = pkTarget->GetSocket(i);
                if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM) {
                    AutoGiveItem(socket);
                    // TItemTable* pTable =
                    // ITEM_MANAGER::instance().GetTable(pkTarget->GetSocket(i));
                    // pkTarget->SetSocket(i, pTable->alValues[2]);
                    // 깨진돌로 대체해준다
                    pkTarget->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
                }
            }
            pkItem->SetCount(pkItem->GetCount() - 1);
            return true;
        }
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "빼낼 수 있는 메틴석이 없습니다.");
        return false;
    }

    return false;
}

#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
bool CHARACTER::RemoveItem(const TItemPos& Cell)
{
    CItem* item;

    if (!CanHandleItem()) {
        if (DragonSoul_RefineWindow_IsOpen())
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "CANNOT_MOVE_ITEM_IN_DS_REFINE_WINDOW");
        return false;
    }

    if (IsDead())
        return false;

    if (!IsValidItemPosition(Cell) || !((item = GetItem(Cell))))
        return false;

    if (item->IsExchanging())
        return false;

    if (item->IsSealed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, ("ITEM_IS_SEALED_CANNOT_DO"));
        return false;
    }

    if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DESTROY)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, ("ITEM_CANNOT_BE_DESTROYED"));
        return false;
    }

    if (true == item->isLocked())
        return false;

    if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning())
        return false;

    SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell,
                  std::numeric_limits<uint16_t>::max()); // Quickslot 에서 지움

#ifdef ENABLE_BATTLE_PASS
    uint8_t bBattlePassId = GetBattlePassId();
    if (bBattlePassId) {
        uint32_t dwItemVnum, dwUseCount;
        if (CBattlePass::instance().BattlePassMissionGetInfo(
                bBattlePassId, DESTROY_ITEM, &dwItemVnum, &dwUseCount)) {
            if (dwItemVnum == item->GetVnum() &&
                GetMissionProgress(DESTROY_ITEM, bBattlePassId) < dwUseCount)
                UpdateMissionProgress(DESTROY_ITEM, bBattlePassId, 1,
                                      dwUseCount);
        }
    }
#endif

    g_pLogManager->ItemDestroyLog(this, item);
    ITEM_MANAGER::instance().RemoveItem(item, "DESTROY");

    return true;
}
#endif

bool CHARACTER::GiveRecallItem(CItem* item)
{
    int idx = GetMapIndex();
    int iEmpireByMapIndex = -1;

    if (idx < 20)
        iEmpireByMapIndex = 1;
    else if (idx < 40)
        iEmpireByMapIndex = 2;
    else if (idx < 60)
        iEmpireByMapIndex = 3;
    else if (idx < 10000)
        iEmpireByMapIndex = 0;

    // Maps where player can't save their scroll position
    switch (idx) {
        case 66:
        case 216:
            iEmpireByMapIndex = -1;
            break;
    }

    if (iEmpireByMapIndex == -1) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "기억해 둘 수 없는 위치 입니다.");
        return false;
    }

    int pos;

    if (item->GetCount() == 1) // 아이템이 하나라면 그냥 셋팅.
    {
        item->SetSocket(0, GetX());
        item->SetSocket(1, GetY());
        item->SetSocket(2, GetMapIndex());
    } else if ((pos = GetEmptyInventory(item)) !=
               -1) // 그렇지 않다면 다른 인벤토리 슬롯을 찾는다.
    {
        CItem* item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), 1);

        if (nullptr != item2) {
            item2->SetSocket(0, GetX());
            item2->SetSocket(1, GetY());
            item2->SetSocket(2, GetMapIndex());
            item2->AddToCharacter(this, TItemPos(INVENTORY, pos));

            if(item->SetCount(item->GetCount() - 1)) {
                ITEM_MANAGER::instance().FlushDelayedSave(item);
            }
            item2->Save();
            ITEM_MANAGER::instance().FlushDelayedSave(item2);
        }
    } else {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "소지품에 빈 공간이 없습니다.");
        return false;
    }

    return true;
}

void CHARACTER::ProcessRecallItem(CItem* item)
{
    if (MeleyLair::CMgr::instance().IsMeleyMap(GetMapIndex())) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return;
    }

    int idx = item->GetSocket(2);

    int iEmpireByMapIndex = -1;

    if (idx < 20)
        iEmpireByMapIndex = 1;
    else if (idx < 40)
        iEmpireByMapIndex = 2;
    else if (idx < 60)
        iEmpireByMapIndex = 3;
    else if (idx < 10000)
        iEmpireByMapIndex = 0;

    switch (idx) {
        case 66:
        case 216:
            iEmpireByMapIndex = -1;
            break;
            // 악룡군도 일때
        case 301:
        case 302:
        case 303:
        case 304: {
            if (GetLevel() < 90) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "아이템의 레벨 제한보다 레벨이 낮습니다.");
                return;
            }
            break;
        }
    }

    if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "기억된 위치가 타제국에 속해 있어서 귀환할 수 "
                           "없습니다.");
        item->SetSocket(0, 0);
        item->SetSocket(1, 0);
        item->SetSocket(2, 0);
    } else {
        SPDLOG_TRACE("Recall: %s %d %d -> %d %d", GetName(), GetX(), GetY(),
                     item->GetSocket(0), item->GetSocket(1));
        WarpSet(item->GetSocket(2), item->GetSocket(0), item->GetSocket(1));
        // item->SetCount(item->GetCount() - 1);
    }
}

void CHARACTER::__OpenPrivateShop()
{
#ifdef __NEW_GAMEMASTER_CONFIG__
    if (!GM::check_allow(GetGMLevel(), GM_ALLOW_CREATE_PRIVATE_SHOP)) {
        ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot do this with this "
                                           "gamemaster rank."));
        return;
    }
#endif

    ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
}

// MYSHOP_PRICE_LIST
void CHARACTER::SendMyShopPriceListCmd(uint32_t dwItemVnum, Gold dwItemPrice)
{
    SendChatPacket(
        this, CHAT_TYPE_COMMAND,
        fmt::format("MyShopPriceList {} {}", dwItemVnum, dwItemPrice));
}

//
// DB 캐시로 부터 받은 리스트를 User 에게 전송하고 상점을 열라는 커맨드를
// 보낸다.
//
void CHARACTER::UseSilkBotaryReal(const MyShopPriceListHeader* p)
{
    const MyShopPriceInfo* pInfo = (const MyShopPriceInfo*)(p + 1);

    if (!p->count)
        // 가격 리스트가 없다. dummy 데이터를 넣은 커맨드를 보내준다.
        SendMyShopPriceListCmd(1, 0);
    else {
        for (int idx = 0; idx < p->count; idx++)
            SendMyShopPriceListCmd(pInfo[idx].vnum, pInfo[idx].price);
    }

    __OpenPrivateShop();
}

//
// 이번 접속 후 처음 상점을 Open 하는 경우 리스트를 Load 하기 위해 DB 캐시에
// 가격정보 리스트 요청 패킷을 보낸다. 이후부터는 바로 상점을 열라는 응답을
// 보낸다.
//
void CHARACTER::UseSilkBotary(void)
{
    if (m_bNoOpenedShop) {
        uint32_t dwPlayerID = GetPlayerID();
        db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_REQ,
                                GetDesc()->GetHandle(), &dwPlayerID,
                                sizeof(uint32_t));
        m_bNoOpenedShop = false;
    } else {
        __OpenPrivateShop();
    }
}

void CHARACTER::UseKashmirBotary(void)
{
    ChatPacket(CHAT_TYPE_COMMAND, "OpenMyShopDecoWnd");
}

// END_OF_MYSHOP_PRICE_LIST

void CHARACTER::OpenPrivateShop()
{
    ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
}

int CalculateConsume(CHARACTER* ch)
{
    static const int WARP_NEED_LIFE_PERCENT = 30;
    static const int WARP_MIN_LIFE_PERCENT = 10;
    // CONSUME_LIFE_WHEN_USE_WARP_ITEM
    int consumeLife = 0;
    {
        // CheckNeedLifeForWarp
        const int curLife = ch->GetHP();
        const int needPercent = WARP_NEED_LIFE_PERCENT;
        const int needLife = ch->GetMaxHP() * needPercent / 100;
        if (curLife < needLife) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "남은 생명력 양이 모자라 사용할 수 없습니다.");
            return -1;
        }

        consumeLife = needLife;

        // CheckMinLifeForWarp: 독에 의해서 죽으면 안되므로 생명력 최소량는
        // 남겨준다
        const int minPercent = WARP_MIN_LIFE_PERCENT;
        const int minLife = ch->GetMaxHP() * minPercent / 100;
        if (curLife - needLife < minLife)
            consumeLife = curLife - minLife;

        if (consumeLife < 0)
            consumeLife = 0;
    }
    // END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM
    return consumeLife;
}

int CalculateConsumeSP(CHARACTER* lpChar)
{
    static const int NEED_WARP_SP_PERCENT = 30;

    const int curSP = lpChar->GetSP();
    const int needSP = lpChar->GetMaxSP() * NEED_WARP_SP_PERCENT / 100;

    if (curSP < needSP) {
        SendI18nChatPacket(lpChar, CHAT_TYPE_INFO,
                           "남은 정신력 양이 모자라 사용할 수 없습니다.");
        return -1;
    }

    return needSP;
}

bool CHARACTER::HandleUseItemMetin(CItem* item, CItem* item2)
{
    if (!item2)
        return false;
    if (item2->IsExchanging())
        return false;

    if (item2->GetItemType() == ITEM_PICK)
        return false;
    if (item2->GetItemType() == ITEM_ROD)
        return false;

    int i;

    for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
        uint32_t dwVnum;

        if ((dwVnum = item2->GetSocket(i)) <= 2)
            continue;

        const auto p = ITEM_MANAGER::instance().GetTable(dwVnum);

        if (!p)
            continue;

        if (item->GetValue(5) == p->alValues[5]) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "같은 종류의 메틴석은 여러개 부착할 수 "
                               "없습니다.");
            return false;
        }
    }

    if (item2->GetItemType() == ITEM_ARMOR) {
        if (!IS_SET(item->GetWearFlag(), WEARABLE_BODY) ||
            !IS_SET(item2->GetWearFlag(), WEARABLE_BODY)) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 메틴석은 장비에 부착할 수 "
                               "없습니다.");
            return false;
        }
    } else if (item2->GetItemType() == ITEM_WEAPON) {
        if (!IS_SET(item->GetWearFlag(), WEARABLE_WEAPON)) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이 메틴석은 무기에 부착할 수 "
                               "없습니다.");
            return false;
        }
    } else {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "부착할 수 있는 슬롯이 없습니다.");
        return false;
    }

    for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
        if (item2->GetSocket(i) >= 1 && item2->GetSocket(i) <= 2 &&
            item2->GetSocket(i) >= item->GetValue(2)) {
            // 석 확률
            const auto successRate = 100;
            if (Random::get(1, 100) <= successRate) // 80 % Stone percentage
            {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "메틴석 부착에 성공하였습니다.");
                item2->SetSocket(i, item->GetVnum());
            } else {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "메틴석 부착에 실패하였습니다.");
                item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
            }

            LogManager::instance().ItemLog(this, item2, "SOCKET",
                                           item->GetName());
            item->SetCount(item->GetCount() - 1);
            break;
        }

    if (i == ITEM_SOCKET_MAX_NUM)
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "부착할 수 있는 슬롯이 없습니다.");

    return true;
}

bool CHARACTER::HandleItemUsePotionNoDelay(CItem* item)
{
    if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true) {
        if (quest::CQuestManager::instance().GetEventFlag("arena_potion_"
                                                          "limit") > 0) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "대련장에서 사용하실 수 "
                               "없습니다.");
            return false;
        }

        switch (item->GetVnum()) {
            case 71018:
            case 71019:
            case 71020:
                if (quest::CQuestManager::instance().GetEventFlag(
                        "arena_potion_limit_"
                        "count") < 10000) {
                    if (m_nPotionLimit <= 0) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "사용 제한량을 "
                                           "초과하였습니다.");
                        return false;
                    }
                }
                break;

            default:
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "대련장에서 사용하실 수 "
                                   "없습니다.");
                return false;
        }
    }

    bool used = false;

    if (item->GetValue(0) != 0) // HP 절대값 회복
    {
        if (GetHP() < GetMaxHP()) {
            PointChange(POINT_HP, item->GetValue(0) *
                                      (100 + GetPoint(POINT_POTION_BONUS)) /
                                      100);
            EffectPacket(SE_HPUP_RED);
            used = true;
        }
    }

    if (item->GetValue(1) != 0) // SP 절대값 회복
    {
        if (GetSP() < GetMaxSP()) {
            PointChange(POINT_SP, item->GetValue(1) *
                                      (100 + GetPoint(POINT_POTION_BONUS)) /
                                      100);
            EffectPacket(SE_SPUP_BLUE);
            used = true;
        }
    }

    if (item->GetValue(3) != 0) // HP % 회복
    {
        if (GetHP() < GetMaxHP()) {
            PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
            EffectPacket(SE_HPUP_RED);
            used = true;
        }
    }

    if (item->GetValue(4) != 0) // SP % 회복
    {
        if (GetSP() < GetMaxSP()) {
            PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
            EffectPacket(SE_SPUP_BLUE);
            used = true;
        }
    }

    if (used) {
        if (item->GetVnum() == 50085 || item->GetVnum() == 50086) {
            if (gConfig.testServer)
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "월병 또는 종자 를 "
                                   "사용하였습니다");
            SetUseSeedOrMoonBottleTime();
        }

        if (GetWarMap())
            GetWarMap()->UsePotion(this, item);

        m_nPotionLimit--;

        // RESTRICT_USE_SEED_OR_MOONBOTTLE
        item->SetCount(item->GetCount() - 1);
        // END_RESTRICT_USE_SEED_OR_MOONBOTTLE
    }
    return true;
}

bool CHARACTER::HandleItemUsePotion(CItem* item)
{
    if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true) {
        if (quest::CQuestManager::instance().GetEventFlag("arena_potion_"
                                                          "limit") > 0) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "대련장에서 사용하실 수 "
                               "없습니다.");
            return false;
        }

        switch (item->GetVnum()) {
            case 27001:
            case 27002:
            case 27003:
            case 27004:
            case 27005:
            case 27006:
                if (quest::CQuestManager::instance().GetEventFlag(
                        "arena_potion_limit_"
                        "count") < 10000) {
                    if (m_nPotionLimit <= 0) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "사용 제한량을 "
                                           "초과하였습니다.");
                        return false;
                    }
                }
                break;

            default:
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "대련장에서 사용하실 수 "
                                   "없습니다.");
                return false;
        }
    }

    if (item->GetValue(1) != 0) {
        if (GetPoint(POINT_SP_RECOVERY) + GetSP() >= GetMaxSP()) {
            return false;
        }

        PointChange(POINT_SP_RECOVERY,
                    item->GetValue(1) *
                        std::min<PointValue>(
                            200, (100 + GetPoint(POINT_POTION_BONUS))) /
                        100);
        StartAffectEvent();
        EffectPacket(SE_SPUP_BLUE);
    }

    if (item->GetValue(0) != 0) {
        if (GetPoint(POINT_HP_RECOVERY) + GetHP() >= GetMaxHP()) {
            return false;
        }

        PointChange(POINT_HP_RECOVERY,
                    item->GetValue(0) *
                        std::min<PointValue>(
                            200, (100 + GetPoint(POINT_POTION_BONUS))) /
                        100);
        StartAffectEvent();
        EffectPacket(SE_HPUP_RED);
    }

    if (GetWarMap())
        GetWarMap()->UsePotion(this, item);

    item->SetCount(item->GetCount() - 1);
    m_nPotionLimit--;
    return true;
}

bool CHARACTER::HandleItemUseAbilityUp(CItem* item)
{
    if (FindAffect(AFFECT_BLEND, item->GetValue(0))) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "CANT_USE_WATER_AND_BLEND_ITEM_"
                           "TOGETHER");
        return false;
    }
    switch (item->GetValue(0)) {
        case APPLY_MOV_SPEED:
            AddAffect(AFFECT_MOV_SPEED, POINT_MOV_SPEED, item->GetValue(2),
                      item->GetValue(1), 0, true);
            break;

        case APPLY_ATT_SPEED:
            AddAffect(AFFECT_ATT_SPEED, POINT_ATT_SPEED, item->GetValue(2),
                      item->GetValue(1), 0, true);
            break;

        case APPLY_STR:
            AddAffect(AFFECT_STR, POINT_ST, item->GetValue(2), 0,
                      item->GetValue(1), 0, true);
            break;

        case APPLY_DEX:
            AddAffect(AFFECT_DEX, POINT_DX, item->GetValue(2), 0,
                      item->GetValue(1), 0, true);
            break;

        case APPLY_CON:
            AddAffect(AFFECT_CON, POINT_HT, item->GetValue(2), 0,
                      item->GetValue(1), 0, true);
            break;

        case APPLY_INT:
            AddAffect(AFFECT_INT, POINT_IQ, item->GetValue(2), 0,
                      item->GetValue(1), 0, true);
            break;

        case APPLY_CAST_SPEED:
            AddAffect(AFFECT_CAST_SPEED, POINT_CASTING_SPEED, item->GetValue(2),
                      0, item->GetValue(1), 0, true);
            break;

        case APPLY_ATT_GRADE_BONUS:
            AddAffect(AFFECT_ATT_GRADE, POINT_ATT_GRADE_BONUS,
                      item->GetValue(2), 0, item->GetValue(1), 0, true);
            break;

        case APPLY_DEF_GRADE_BONUS:
            AddAffect(AFFECT_DEF_GRADE, POINT_DEF_GRADE_BONUS,
                      item->GetValue(2), 0, item->GetValue(1), 0, true);
            break;
    }
    EffectPacketByPointType(this, GetApplyPoint(item->GetValue(0)));
    if (GetWarMap())
        GetWarMap()->UsePotion(this, item);

    item->SetCount(item->GetCount() - 1);
    return true;
}

auto PetExpChange(CItem* petItem, int amount) -> void
{
    auto petLevel = petItem->GetSocket(ITEM_SOCKET_LEVEL_PET_LEVEL);
    auto exp = petItem->GetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT);
    auto next_exp = ITEM_MANAGER::instance().GetPetNeededExpByLevel(petLevel);

    PointValue expBalance = 0;

    if (amount < 0) {
        petItem->SetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT,
                           exp - std::min<PointValue>(exp, std::abs(amount)));
        exp = petItem->GetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT);
    } else if (exp + amount >= next_exp) {
        // The amount of EXP have left after the level-up
        expBalance = (exp + amount) - next_exp;

        amount = next_exp - exp;
        petItem->SetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT, 0);
        exp = next_exp;
    } else {
        petItem->SetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT, exp + amount);
        exp = petItem->GetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT);
    }

    if (exp >= next_exp) {
    }

    if (expBalance)
        PetExpChange(petItem, expBalance);
}

bool CHARACTER::HandleItemUseLevelPetFood(CItem* item, CItem* item2)
{
    if (!item2)
        return false;

    if (item2->IsEquipped())
        return false;

    if (item2->IsExchanging())
        return false;

    if (item2->isLocked())
        return false;

    if (item2->GetItemType() != ITEM_TOGGLE ||
        item2->GetSubType() != TOGGLE_LEVEL_PET) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "This food is only for pets");
        return false;
    }

    auto petLevel = item2->GetSocket(ITEM_SOCKET_LEVEL_PET_LEVEL);
    auto needCount = item2->GetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT);

    if (petLevel == LEVEL_PET_MAX_LEVEL) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "The pet already has reached its maximum level");
        return false;
    }

    auto foodItemMinLevel = item->GetValue(0);
    auto foodItemMaxLevel = item->GetValue(1);

    if (petLevel < foodItemMinLevel) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "This food can only be used for pet level %d to %d",
                           foodItemMinLevel, foodItemMaxLevel);
        return false;
    }

    if (petLevel >= foodItemMaxLevel) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "This food can only be used for pet level %d to %d",
                           foodItemMinLevel, foodItemMaxLevel);
        return false;
    }

    auto leftNeedCount = needCount - item->GetCount(); // 5 - 2 = 3

    if (leftNeedCount <= 0) {
        item2->SetSocket(ITEM_SOCKET_LEVEL_PET_LEVEL, ++petLevel);
        item2->SetSocket(
            ITEM_SOCKET_LEVEL_NEED_COUNT,
            ITEM_MANAGER::instance().GetPetNeededExpByLevel(petLevel));
        item2->SetSocket(ITEM_SOCKET_LEVEL_STAT_POINT,
                         item2->GetSocket(ITEM_SOCKET_LEVEL_STAT_POINT) +
                             LEVEL_PET_STAT_POINT_PER_LEVEL);

        const auto attrCount = item2->GetAttributeCount();
        if (attrCount < 7) {
            if (petLevel == 10 || petLevel == 30 || petLevel == 60 ||
                petLevel == 100 || petLevel == 150) {
                auto a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();

                while (item2->HasAttr(a.type)) {
                    a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();
                }

                item2->SetForceAttribute(attrCount, a.type, a.value);
            }
            //
        }

        item->SetCount(item->GetCount() - needCount);
    } else {
        item2->SetSocket(ITEM_SOCKET_LEVEL_NEED_COUNT, leftNeedCount);
        item->SetCount(0);
    }

    ITEM_MANAGER::instance().SaveSingleItem(item2);

    return true;
}

bool CHARACTER::HandleItemUseChangePetAttr(CItem* item, CItem* item2)
{
    if (!item2)
        return false;

    if (item2->IsEquipped())
        return false;

    if (item2->IsExchanging())
        return false;

    if (item2->GetItemType() != ITEM_TOGGLE ||
        item2->GetSubType() != TOGGLE_LEVEL_PET)
        return false;

    if (item2->isLocked())
        return false;

    auto petLevel = item2->GetSocket(ITEM_SOCKET_LEVEL_PET_LEVEL);

    item2->SetSocket(ITEM_SOCKET_LEVEL_STAT_POINT,
                     LEVEL_PET_STAT_POINT_PER_LEVEL * petLevel);
    auto attrCount = item2->GetAttributeCount();
    item2->ClearAttribute();
    for (int i = 0; i < attrCount; ++i) {
        auto a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();

        while (item2->HasAttr(a.type)) {
            a = ITEM_MANAGER::instance().GetLevelPetRandomBonus();
        }

        item2->SetForceAttribute(i, a.type, a.value);
    }

    ITEM_MANAGER::instance().SaveSingleItem(item2);
    item->SetCount(item->GetCount() - 1);

    return true;
}

bool CHARACTER::HandleItemUseEnhanceTime(CItem* item, CItem* item2)
{
    if (!item2)
        return false;

    if (item2->IsEquipped())
        return false;

    if (item2->IsExchanging())
        return false;

    if (ITEM_MANAGER::instance().IsEnhanceTimeExceptionItem(item2->GetVnum()))
        return false;

    if (item2->GetCount() > 1) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You cannot enhance the time of "
                           "this item while it is stacked.");
        return false;
    }

    // Type Checks
    auto type = item->GetValue(0);
    auto subType = item->GetValue(1);
    auto addTime = item->GetValue(2);

    if (type != item2->GetItemType() && type != -1) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You cannot enhance the time of "
                           "this item with this item.");
        return false;
    }

    if (subType != item2->GetSubType() && subType != -1) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You cannot enhance the time of "
                           "this item with this item.");
        return false;
    }

    // Time type check

    uint8_t timeType = LIMIT_NONE;

    auto limit = item2->FindLimit(LIMIT_REAL_TIME);
    if (limit)
        timeType = limit->bType;

    limit = item2->FindLimit(LIMIT_REAL_TIME_START_FIRST_USE);
    if (limit)
        timeType = limit->bType;

    limit = item2->FindLimit(LIMIT_TIMER_BASED_ON_WEAR);
    if (limit)
        timeType = limit->bType;

    if (timeType == LIMIT_NONE) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "You cannot enhance the time of "
                           "this item.");
        return false;
    }

    uint32_t socketType = ITEM_SOCKET_REALTIME_EXPIRE;

    switch (timeType) {
        case LIMIT_REAL_TIME:
            socketType = ITEM_SOCKET_REALTIME_EXPIRE;
            break;
        case LIMIT_REAL_TIME_START_FIRST_USE:
            socketType = ITEM_SOCKET_REALTIME_EXPIRE;
            break;
        case LIMIT_TIMER_BASED_ON_WEAR:
            socketType = ITEM_SOCKET_WEAR_REMAIN_SEC;
            break;
    }

    item2->SetSocket(socketType, item2->GetSocket(socketType) + addTime);
    ITEM_MANAGER::instance().SaveSingleItem(item2);

    item->SetCount(item->GetCount() - 1);
    return true;
}

bool CHARACTER::HandleItemUseTalisman(CItem* item)
{
    const int TOWN_PORTAL = 1;
    const int MEMORY_PORTAL = 2;

    // gm_guild_build, oxevent 맵에서 귀환부 귀환기억부 를
    // 사용못하게 막음
    if (GetMapIndex() == 200 || COXEventManager::IsEventMap(GetMapIndex())) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "현재 위치에서 사용할 수 없습니다.");
        return false;
    }

    if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "대련 중에는 이용할 수 없는 "
                           "물품입니다.");
        return false;
    }

    if (MeleyLair::CMgr::instance().IsMeleyMap(GetMapIndex())) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return false;
    }

    if (m_pkWarpEvent) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "이동할 준비가 되어있음으로 "
                           "귀환부를 사용할수 없습니다");
        return false;
    }

    // CONSUME_LIFE_WHEN_USE_WARP_ITEM
    int consumeLife = CalculateConsume(this);

    if (consumeLife < 0)
        return false;
    // END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

    if (item->GetValue(0) == TOWN_PORTAL) // 귀환부
    {
        if (item->GetSocket(0) == 0) {
            if (!GetDungeon())
                if (!GiveRecallItem(item))
                    return false;

            PIXEL_POSITION posWarp;

            if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(
                    GetMapIndex(), GetEmpire(), posWarp)) {
                // CONSUME_LIFE_WHEN_USE_WARP_ITEM
                PointChange(POINT_HP, -consumeLife, false);
                // END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

                WarpSet(GetMapIndex(), posWarp.x, posWarp.y);
            } else {
                SPDLOG_ERROR("CHARACTER::UseItem : cannot find "
                             "spawn position (name %s, %d x %d)",
                             GetName(), GetX(), GetY());
            }
        } else {
            if (gConfig.testServer)
                SendI18nChatPacket(this, CHAT_TYPE_INFO, "원래 위치로 복귀");

            ProcessRecallItem(item);
        }
    } else if (item->GetValue(0) == MEMORY_PORTAL) // 귀환기억부
    {
        if (item->GetSocket(0) == 0) {
            if (GetDungeon()) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "던전 안에서는 %s%s 사용할 "
                                   "수 없습니다.",
                                   TextTag::itemname(item->GetVnum()).c_str(),
                                   "");
                return false;
            }

            if (!GiveRecallItem(item))
                return false;
        } else {
            // CONSUME_LIFE_WHEN_USE_WARP_ITEM
            PointChange(POINT_HP, -consumeLife, false);
            // END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

            ProcessRecallItem(item);
        }
    }
    return true;
}

bool CHARACTER::HandleUseItemUse(CItem* item, CItem* item2)
{
    // USE_CRAFTING_POTION
    if (item->GetVnum() > 50800 && item->GetVnum() <= 50820) {
        if (gConfig.testServer)
            SPDLOG_INFO("ADD addtional effect : vnum(%d) subtype(%d)",
                        item->GetOriginalVnum(), item->GetSubType());

        auto affect_type = AFFECT_EXP_BONUS_EURO_FREE;
        ApplyType apply_type = GetApplyPoint(item->GetValue(0));
        ApplyValue apply_value = item->GetValue(2);
        auto apply_duration = item->GetValue(1);

        switch (item->GetSubType()) {
            case USE_ABILITY_UP: {
                if (FindAffect(affect_type, apply_type)) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "이미 효과가 걸려 있습니다.");
                    return false;
                }
                if (FindAffect(AFFECT_BLEND, item->GetValue(0))) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "CANT_USE_WATER_AND_BLEND_ITEM_"
                                       "TOGETHER");
                    return false;
                }

                AddAffect(affect_type, apply_type, apply_value,
                                  apply_duration, 0, true, true);
                EffectPacketByPointType(this, apply_type);
            }

                if (GetWarMap())
                    GetWarMap()->UsePotion(this, item);

                item->SetCount(item->GetCount() - 1);
                break;

            case USE_AFFECT: {
                if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE,
                               GetApplyPoint(item->GetValue(1)))) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "이미 효과가 걸려 있습니다.");
                } else {
                    AddAffect(AFFECT_EXP_BONUS_EURO_FREE,
                              GetApplyPoint(item->GetValue(1)),
                              item->GetValue(2), item->GetValue(3), 0, false,
                              true);
                    item->SetCount(item->GetCount() - 1);
                }
            } break;

            case USE_POTION_NODELAY: {
                if (CArenaManager::instance().IsArenaMap(GetMapIndex()) ==
                    true) {
                    if (quest::CQuestManager::instance().GetEventFlag("arena_"
                                                                      "potion_"
                                                                      "limit") >
                        0) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "대련장에서 사용하실 수 "
                                           "없습니다.");
                        return false;
                    }

                    switch (item->GetVnum()) {
                        case 71018:
                        case 71019:
                        case 71020:
                            if (quest::CQuestManager::instance().GetEventFlag(
                                    "arena_potion_limit_"
                                    "count") < 10000) {
                                if (m_nPotionLimit <= 0) {
                                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                       "사용 제한량을 "
                                                       "초과하였습니다.");
                                    return false;
                                }
                            }
                            break;

                        default:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "대련장에서 사용하실 수 "
                                               "없습니다.");
                            return false;
                            break;
                    }
                }

                bool used = false;

                if (item->GetValue(0) != 0) // HP 절대값 회복
                {
                    if (GetHP() < GetMaxHP()) {
                        PointChange(POINT_HP,
                                    item->GetValue(0) *
                                        (100 + GetPoint(POINT_POTION_BONUS)) /
                                        100);
                        EffectPacket(SE_HPUP_RED);
                        used = true;
                    }
                }

                if (item->GetValue(1) != 0) // SP 절대값 회복
                {
                    if (GetSP() < GetMaxSP()) {
                        PointChange(POINT_SP,
                                    item->GetValue(1) *
                                        (100 + GetPoint(POINT_POTION_BONUS)) /
                                        100);
                        EffectPacket(SE_SPUP_BLUE);
                        used = true;
                    }
                }

                if (item->GetValue(3) != 0) // HP % 회복
                {
                    if (GetHP() < GetMaxHP()) {
                        PointChange(POINT_HP,
                                    item->GetValue(3) * GetMaxHP() / 100);
                        EffectPacket(SE_HPUP_RED);
                        used = true;
                    }
                }

                if (item->GetValue(4) != 0) // SP % 회복
                {
                    if (GetSP() < GetMaxSP()) {
                        PointChange(POINT_SP,
                                    item->GetValue(4) * GetMaxSP() / 100);
                        EffectPacket(SE_SPUP_BLUE);
                        used = true;
                    }
                }

                if (used) {
                    if (item->GetVnum() == 50085 || item->GetVnum() == 50086) {
                        if (gConfig.testServer)
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "월병 또는 종자 를 "
                                               "사용하였습니다");
                        SetUseSeedOrMoonBottleTime();
                    }

                    if (GetWarMap())
                        GetWarMap()->UsePotion(this, item);

                    m_nPotionLimit--;

                    // RESTRICT_USE_SEED_OR_MOONBOTTLE
                    item->SetCount(item->GetCount() - 1);
                    // END_RESTRICT_USE_SEED_OR_MOONBOTTLE
                }
            } break;
        }

        return true;
    }

    if (item->GetVnum() >= 27863 && item->GetVnum() <= 27883) {
        if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "대련 중에는 이용할 수 없는 "
                               "물품입니다.");
            return false;
        }
    }

    if (gConfig.testServer) {
        SPDLOG_INFO("USE_ITEM {} Type {} SubType {} vnum {}", item->GetName(),
                    item->GetItemType(), item->GetSubType(),
                    item->GetOriginalVnum());
    }

    switch (item->GetSubType()) {
        case USE_TIME_CHARGE_PER: {
            CItem* pDestItem = item2;
            if (nullptr == pDestItem) {
                return false;
            }
            // 우선 용혼석에 관해서만 하도록 한다.
            if (pDestItem->IsDragonSoul()) {
                int ret;
                std::string buf;
                if (item->GetVnum() == DRAGON_HEART_VNUM) {
                    ret = pDestItem->GiveMoreTime_Per((float)item->GetSocket(
                        ITEM_SOCKET_CHARGING_AMOUNT_IDX));
                } else {
                    ret = pDestItem->GiveMoreTime_Per(
                        (float)item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
                }
                if (ret > 0) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "%d초 만큼 충전되었습니다.", ret);
                    item->SetCount(item->GetCount() - 1);

                    return true;
                }

                SendI18nChatPacket(this, CHAT_TYPE_INFO, "충전할 수 없습니다.");
                return false;
            }
            return false;
        } break;
        case USE_TIME_CHARGE_FIX: {
            CItem* pDestItem = item2;
            if (nullptr == pDestItem) {
                return false;
            }
            // 우선 용혼석에 관해서만 하도록 한다.
            if (pDestItem->IsDragonSoul()) {
                int ret = pDestItem->GiveMoreTime_Fix(
                    item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
                char buf[128];
                if (ret) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "%d초 만큼 충전되었습니다.", ret);
                    sprintf(buf, "Increase %ds by item{VN:%u VAL%d:%ld}", ret,
                            item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX,
                            item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
                    LogManager::instance().ItemLog(this, item,
                                                   "DS_CHARGING_SUCCESS", buf);
                    item->SetCount(item->GetCount() - 1);
                    return true;
                }
                SendI18nChatPacket(this, CHAT_TYPE_INFO, "충전할 수 없습니다.");
                sprintf(buf, "No change by item{VN:%u VAL%d:%ld}",
                        item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX,
                        item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
                LogManager::instance().ItemLog(this, item, "DS_CHARGING_FAILED",
                                               buf);
                return false;
            }
            return false;
        } break;

        case USE_SPECIAL:
        case USE_CHEST:
            switch (item->GetVnum()) {
                case 478133:
                case 478134:
                case 478135:
                case 478136:
                case 478137:
                case 478138:
                case 478139:
                case 478140:
                case 478141:
                case 478142:
                case 478143:
                case 478144:
                case 478145:
                case 478146:
                case 478147:
                case 478148:
                case 478149:
                case 478150:
                case 478151:
                case 47832: {
                    if (!item2)
                        return false;

                    auto evolType = item->GetValue(0);

                    const auto evolvedVnum =
                        ITEM_MANAGER::instance().GetPetItemEvolvedByVnum(
                            evolType, item2->GetVnum());

                    if (!evolvedVnum) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "This is not the correct evolution "
                                           "item for this item or it has none");
                        return false;
                    }

                    auto evolvedItem = AutoGiveItem(evolvedVnum);
                    item2->CopySocketTo(evolvedItem);
                    evolvedItem->UpdatePacket();

                    ITEM_MANAGER::instance().RemoveItem(item2);
                    item->SetCount(item->GetCount() - 1);
                } break;

                case 56093: {
                    SetTitle("", 0);
                    Save();
                    item->SetCount(item->GetCount() - 1);
                } break;

                case 72325: {
                    if (!item2)
                        return false;

                    if (item2->GetTransmutationVnum() == 0)
                        return false;

                    item2->SetTransmutationVnum(0);
                    ITEM_MANAGER::instance().FlushDelayedSave(item2);
                    item->SetCount(item->GetCount() - 1);
                    item2->UpdatePacket();
                } break;

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
                case 60004:
                case 60005: {
                    if (item->GetVnum() == 60004) {
                        ChatPacket(CHAT_TYPE_COMMAND, "OpenPShopSearchD"
                                                      "ialog");
                    } else {
                        ChatPacket(CHAT_TYPE_COMMAND, "OpenPShopSearchD"
                                                      "ialogCash");
                    }
                } break;
#endif
                    //크리스마스 란주
                case ITEM_NOG_POCKET: {
                    /*
                                    란주능력치 : item_proto value 의미
                                    이동속도  value 1
                                    공격력	  value 2
                                    경험치    value 3
                                    지속시간  value 0 (단위 초)

                                    */
                    if (FindAffect(AFFECT_NOG_ABILITY)) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이미 효과가 걸려 "
                                           "있습니다.");
                        return false;
                    }
                    long time = item->GetValue(0);
                    long moveSpeedPer = item->GetValue(1);
                    long attPer = item->GetValue(2);
                    long expPer = item->GetValue(3);
                    EffectPacket(SE_DXUP_PURPLE);
                    AddAffect(AFFECT_NOG_ABILITY, POINT_MOV_SPEED, moveSpeedPer,
                              time, 0, true, true);
                    AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_ATTBONUS, attPer,
                              time, 0, true, true);
                    AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_EXPBONUS, expPer,
                              time, 0, true, true);
                    item->SetCount(item->GetCount() - 1);
                } break;

                    //라마단용 사탕
                case ITEM_RAMADAN_CANDY: {
                    /*
                                    사탕능력치 : item_proto value 의미
                                    이동속도  value 1
                                    공격력	  value 2
                                    경험치    value 3
                                    지속시간  value 0 (단위 초)

                                    */
                    long time = item->GetValue(0);
                    long moveSpeedPer = item->GetValue(1);
                    long attPer = item->GetValue(2);
                    long expPer = item->GetValue(3);
                    EffectPacket(SE_DXUP_PURPLE);

                    AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MOV_SPEED,
                              moveSpeedPer, time, 0, true, true);
                    AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_ATTBONUS,
                              attPer, time, 0, true, true);
                    AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_EXPBONUS,
                              expPer, time, 0, true, true);
                    item->SetCount(item->GetCount() - 1);
                } break;
                case ITEM_MARRIAGE_RING: {
                    marriage::TMarriage* pMarriage =
                        marriage::CManager::instance().Get(GetPlayerID());
                    if (pMarriage) {
                        if (pMarriage->ch1 != nullptr) {
                            if (CArenaManager::instance().IsArenaMap(
                                    pMarriage->ch1->GetMapIndex()) == true) {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "대련 중에는 이용할 "
                                                   "수 없는 "
                                                   "물품입니다.");
                                break;
                            }
#if defined(WJ_COMBAT_ZONE)
                            if (CCombatZoneManager::Instance().IsCombatZoneMap(
                                    pMarriage->ch1->GetMapIndex())) {
                                ChatPacket(CHAT_TYPE_INFO, LC_TEXT("cz_cannot_"
                                                                   "use_"
                                                                   "marriage_"
                                                                   "ring"));
                                break;
                            }
#endif
                        }

                        if (pMarriage->ch2 != nullptr) {
                            if (CArenaManager::instance().IsArenaMap(
                                    pMarriage->ch2->GetMapIndex()) == true) {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "대련 중에는 이용할 "
                                                   "수 없는 "
                                                   "물품입니다.");
                                break;
                            }
#if defined(WJ_COMBAT_ZONE)
                            if (CCombatZoneManager::Instance().IsCombatZoneMap(
                                    pMarriage->ch2->GetMapIndex())) {
                                ChatPacket(CHAT_TYPE_INFO, LC_TEXT("cz_cannot_"
                                                                   "use_"
                                                                   "marriage_"
                                                                   "ring"));
                                break;
                            }
#endif
                        }
#ifndef INSTINCT_BUILD
                        int consumeSP = CalculateConsumeSP(this);

                        if (consumeSP < 0)
                            return false;

                        PointChange(POINT_SP, -consumeSP, false);
#endif
                        WarpToPID(pMarriage->GetOther(GetPlayerID()));
                    } else
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "결혼 상태가 아니면 "
                                           "결혼반지를 사용할 수 "
                                           "없습니다.");
                } break;

                    //기존 용기의 망토
                case UNIQUE_ITEM_CAPE_OF_COURAGE:

                    if (!CanUseBraveryCape())
                        return false;

                    // EffectPacket(SE_CAPE_EQIP);
                    AggregateMonster(5000);
                    item->SetCount(item->GetCount() - 1);

                    UpdateBraveryCapePulse();
                    break;

                case UNIQUE_ITEM_CAPE_OF_COURAGE_PREMIUM:
                    if (!CanUseBraveryCapePremium())
                        return false;

                    // EffectPacket(SE_CAPE_EQIP);
                    AggregateMonster(2500);

                    UpdateBraveryCapePulsePremium();
                    break;

                case UNIQUE_ITEM_CAPE_OF_COURAGE_PREMIUM2:
                    if (!CanUseBraveryCapePremium())
                        return false;

                    // EffectPacket(SE_CAPE_EQIP);
                    AggregateMonster(10000);

                    UpdateBraveryCapePulsePremium();
                    break;

                case UNIQUE_ITEM_WHITE_FLAG:
                    ForgetMyAttacker();
                    item->SetCount(item->GetCount() - 1);
                    break;

                case UNIQUE_ITEM_TREASURE_BOX:
                    break;

                case 30093:
                case 30094:
                case 30095:
                case 30096:
                    // 복주머니
                    {
                        const int MAX_BAG_INFO = 26;
                        static struct LuckyBagInfo {
                            uint32_t count;
                            int prob;
                            uint32_t vnum;
                        } b1[MAX_BAG_INFO] = {
                            {1000, 302, 1},   {10, 150, 27002}, {10, 75, 27003},
                            {10, 100, 27005}, {10, 50, 27006},  {10, 80, 27001},
                            {10, 50, 27002},  {10, 80, 27004},  {10, 50, 27005},
                            {1, 10, 50300},   {1, 6, 92},       {1, 2, 132},
                            {1, 6, 1052},     {1, 2, 1092},     {1, 6, 2082},
                            {1, 2, 2122},     {1, 6, 3082},     {1, 2, 3122},
                            {1, 6, 5052},     {1, 2, 5082},     {1, 6, 7082},
                            {1, 2, 7122},     {1, 1, 11282},    {1, 1, 11482},
                            {1, 1, 11682},    {1, 1, 11882},
                        };

                        LuckyBagInfo* bi = nullptr;
                        bi = b1;

                        int pct = Random::get(1, 1000);

                        int i;
                        for (i = 0; i < MAX_BAG_INFO; i++) {
                            if (pct <= bi[i].prob)
                                break;
                            pct -= bi[i].prob;
                        }
                        if (i >= MAX_BAG_INFO)
                            return false;

                        if (bi[i].vnum == 50300) {
                            // 스킬수련서는 특수하게 준다.
                            GiveRandomSkillBook();
                        } else if (bi[i].vnum == 1) {
                            ChangeGold(1000);
                        } else {
                            AutoGiveItem(bi[i].vnum, bi[i].count);
                        }
                        ITEM_MANAGER::instance().RemoveItem(item);
                    }
                    break;

                case 27989: // 영석감지기
                case 76006: // 선물용 영석감지기
                {
                    SECTREE_MAP* pMap =
                        SECTREE_MANAGER::instance().GetMap(GetMapIndex());

                    if (pMap != nullptr) {
                        item->SetSocket(0, item->GetSocket(0) + 1);

                        FFindStone f;

                        // <Factor> SECTREE::for_each ->
                        // SECTREE::for_each_entity
                        pMap->for_each(f);

                        if (f.m_mapStone.size() > 0) {
                            std::map<uint32_t, CHARACTER*>::iterator stone =
                                f.m_mapStone.begin();

                            uint32_t max = UINT_MAX;
                            CHARACTER* pTarget = stone->second;

                            while (stone != f.m_mapStone.end()) {
                                uint32_t dist = (uint32_t)DISTANCE(
                                    GetXYZ(), stone->second->GetXYZ());

                                if (dist != 0 && max > dist) {
                                    max = dist;
                                    pTarget = stone->second;
                                }
                                ++stone;
                            }

                            if (pTarget != nullptr) {
                                int val = 3;

                                if (max < 10000)
                                    val = 2;
                                else if (max < 70000)
                                    val = 1;

                                ChatPacket(CHAT_TYPE_COMMAND,
                                           "StoneDetect %u %d %d",
                                           (uint32_t)GetVID(), val,
                                           (int)GetDegreeFromPositionXY(
                                               GetX(), pTarget->GetY(),
                                               pTarget->GetX(), GetY()));
                            } else {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "감지기를 작용하였으나 "
                                                   "감지되는 "
                                                   "영석이 없습니다.");
                            }
                        } else {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "감지기를 작용하였으나 "
                                               "감지되는 영석이 "
                                               "없습니다.");
                        }

                        if (item->GetSocket(0) >= 6) {
                            ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0",
                                       (uint32_t)GetVID());
                            ITEM_MANAGER::instance().RemoveItem(item);
                        }
                    }
                    break;
                } break;

                case 27987:
                    /**
                     * Clam percentage calculation
                     */
                    {
                        item->SetCount(item->GetCount() - 1);

                        int32_t chanceFail =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "fai"
                                                                          "l");
                        int32_t chanceNothing =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "non"
                                                                          "e");
                        int32_t chanceWhite =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "whit"
                                                                          "e");
                        int32_t chanceBlue =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "blu"
                                                                          "e");
                        int32_t chanceRed =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "re"
                                                                          "d");
                        int32_t chanceNewAkami1 =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "new_"
                                                                          "akam"
                                                                          "i_"
                                                                          "1");
                        int32_t chanceNewAkami2 =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "new_"
                                                                          "akam"
                                                                          "i_"
                                                                          "2");
                        int32_t chanceNewAkami3 =
                            quest::CQuestManager::instance().GetEventFlag("clam"
                                                                          "_cha"
                                                                          "nce_"
                                                                          "new_"
                                                                          "akam"
                                                                          "i_"
                                                                          "3");

                        int32_t r = Random::get(1, 1000);

                        if (r < chanceFail) // 55% -> 59%
                        {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "CLAM_LIGHT_FAIL");
                            AutoGiveItem(27990);
                        } else if (r < chanceNothing) // 15% -> 17%
                        {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "CLAM_HARD_FAIL");
                        } else if (r < chanceWhite) // 13% -> 9%
                        {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "CLAM_WHITE_PEARL");
                            AutoGiveItem(27992);
                        } else if (r < chanceBlue) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "CLAM_BLUE_PEARL");
                            AutoGiveItem(27993);
                        } else if (r < chanceRed) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "CLAM_BLOODRED_PEARL");
                            AutoGiveItem(27994);
                        } else if (r < chanceNewAkami1) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "new_pearl_akami2_1");
                            AutoGiveItem(71239);
                        } else if (r < chanceNewAkami2) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "new_pearl_akami2_2");
                            AutoGiveItem(71244);
                        } else if (r < chanceNewAkami3) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "new_pearl_akami2_3");
                            AutoGiveItem(71241);
                        } else {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "CLAM_HARD_FAIL");
                        }
                    }
                    break;

                case 71013: // 축제용폭죽
                    CreateFly(Random::get<int>(FLY_FIREWORK1, FLY_FIREWORK6),
                              this);
                    item->SetCount(item->GetCount() - 1);
                    break;

                case 50100: // 폭죽
                case 50101:
                case 50102:
                case 50103:
                case 50104:
                case 50105:
                case 50106: {
                    if (!CanUseFireworks())
                        return false;

                    CreateFly(
                        (uint8_t)(item->GetVnum() - 50100 + FLY_FIREWORK1),
                        this);
                    item->SetCount(item->GetCount() - 1);
                    UpdateFireworksPulse();
                }

                break;

                case 50200: // 보따리
#if defined(__OFFLINE_SHOP__) && defined(__OFFLINE_SHOP_USE_BUNDLE__)
                    this->OfflineShopStartCreate();
#else
                    __OpenPrivateShop();
#endif
                    break;

                case fishing::FISH_MIND_PILL_VNUM:
                    AddAffect(AFFECT_FISH_MIND_PILL, POINT_NONE, 0, 20 * 60, 0,
                              true);
                    item->SetCount(item->GetCount() - 1);
                    break;

                case 50301: // 통솔력 수련서
                case 50302:
                case 50303: {
                    if (IsPolymorphed() == true) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "둔갑 중에는 능력을 올릴 수 "
                                           "없습니다.");
                        return false;
                    }

                    int lv = GetSkillLevel(SKILL_LEADERSHIP);

                    if (lv < item->GetValue(0)) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 책은 너무 어려워 "
                                           "이해하기가 힘듭니다.");
                        return false;
                    }

                    if (lv >= item->GetValue(1)) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 책은 아무리 봐도 도움이 "
                                           "될 것 같지 않습니다.");
                        return false;
                    }

                    if (LearnSkillByBook(SKILL_LEADERSHIP)) {
                        item->SetCount(item->GetCount() - 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        SetSkillNextReadTime(SKILL_LEADERSHIP,
                                             get_global_time() + iReadDelay);
                    }
                } break;

                case 50304: // 연계기 수련서
                case 50305:
                case 50306: {
                    if (IsPolymorphed()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변신중에는 책을 읽을수 "
                                           "없습니다.");
                        return false;
                    }
                    if (GetSkillLevel(SKILL_COMBO) == 0 && GetLevel() < 30) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "레벨 30이 되기 전에는 "
                                           "습득할 수 있을 것 같지 "
                                           "않습니다.");
                        return false;
                    }

                    if (GetSkillLevel(SKILL_COMBO) == 1 && GetLevel() < 50) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "레벨 50이 되기 전에는 "
                                           "습득할 수 있을 것 같지 "
                                           "않습니다.");
                        return false;
                    }

                    if (GetSkillLevel(SKILL_COMBO) >= 2) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "연계기는 더이상 수련할 수 "
                                           "없습니다.");
                        return false;
                    }

                    int iPct = item->GetValue(0);

                    if (LearnSkillByBook(SKILL_COMBO, iPct)) {
                        item->SetCount(item->GetCount() - 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        SetSkillNextReadTime(SKILL_COMBO,
                                             get_global_time() + iReadDelay);
                    }
                } break;
                case 50311: // 언어 수련서
                case 50312:
                case 50313: {
                    if (IsPolymorphed()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변신중에는 책을 읽을수 "
                                           "없습니다.");
                        return false;
                    }
                    uint32_t dwSkillVnum = item->GetValue(0);
                    int iPct =
                        std::clamp<PointValue>(item->GetValue(1), 0, 100);
                    if (GetSkillLevel(dwSkillVnum) >= 20 ||
                        dwSkillVnum - SKILL_LANGUAGE1 + 1 == GetEmpire()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이미 완벽하게 알아들을 수 "
                                           "있는 언어이다.");
                        return false;
                    }

                    if (LearnSkillByBook(dwSkillVnum, iPct)) {
                        item->SetCount(item->GetCount() - 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        SetSkillNextReadTime(dwSkillVnum,
                                             get_global_time() + iReadDelay);
                    }
                } break;

                case 50061: // 일본 말 소환 스킬 수련서
                {
                    if (IsPolymorphed()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변신중에는 책을 읽을수 "
                                           "없습니다.");
                        return false;
                    }
                    uint32_t dwSkillVnum = item->GetValue(0);
                    int iPct = std::clamp<int>(item->GetValue(1), 0, 100);

                    if (GetSkillLevel(dwSkillVnum) >= 10) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더 이상 수련할 수 "
                                           "없습니다.");
                        return false;
                    }

                    if (LearnSkillByBook(dwSkillVnum, iPct)) {
                        item->SetCount(item->GetCount() - 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        SetSkillNextReadTime(dwSkillVnum,
                                             get_global_time() + iReadDelay);
                    }
                } break;

                case 50314:
                case 50315:
                case 50316: // 변신 수련서
                case 50569:
                case 50570:
                case 50571: // 증혈 수련서
                case 50572:
                case 50573:
                case 50574: // 철통 수련서
                case 50575:
                case 50576:
                case 50577: {
                    if (IsPolymorphed() == true) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "둔갑 중에는 능력을 올릴 수 "
                                           "없습니다.");
                        return false;
                    }

                    int iSkillLevelLowLimit = item->GetValue(0);
                    int iSkillLevelHighLimit = item->GetValue(1);
                    int iPct = std::clamp<int>(item->GetValue(2), 0, 100);
                    int iLevelLimit = item->GetValue(3);
                    uint32_t dwSkillVnum = 0;

                    switch (item->GetVnum()) {
                        case 50314:
                        case 50315:
                        case 50316:
                            dwSkillVnum = SKILL_POLYMORPH;
                            break;

                        case 50569:
                        case 50570:
                        case 50571:
                            dwSkillVnum = SKILL_ADD_HP;
                            break;

                        case 50572:
                        case 50573:
                        case 50574:
                            dwSkillVnum = SKILL_RESIST_PENETRATE;
                            break;

                        case 50575:
                        case 50576:
                        case 50577:
                            dwSkillVnum = SKILL_ADD_MONSTER;
                            break;

                        default:
                            return false;
                    }

                    if (0 == dwSkillVnum)
                        return false;

                    if (GetLevel() < iLevelLimit) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 책을 읽으려면 레벨을 더 "
                                           "올려야 합니다.");
                        return false;
                    }

                    if (GetSkillLevel(dwSkillVnum) >= 40) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더 이상 수련할 수 "
                                           "없습니다.");
                        return false;
                    }

                    if (GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 책은 너무 어려워 "
                                           "이해하기가 힘듭니다.");
                        return false;
                    }

                    if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 책으로는 더 이상 수련할 "
                                           "수 없습니다.");
                        return false;
                    }

                    if (LearnSkillByBook(dwSkillVnum, iPct)) {
                        item->SetCount(item->GetCount() - 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        SetSkillNextReadTime(dwSkillVnum,
                                             get_global_time() + iReadDelay);
                    }
                } break;

                case 50902: {
                    if (IsPolymorphed()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변신중에는 책을 읽을수 "
                                           "없습니다.");
                        return false;
                    }
                    uint32_t dwSkillVnum = SKILL_CREATE;
                    int iPct = std::clamp<int>(item->GetValue(1), 0, 100);

                    if (GetSkillLevel(dwSkillVnum) >= 40) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더 이상 수련할 수 "
                                           "없습니다.");
                        return false;
                    }

                    if (LearnSkillByBook(dwSkillVnum, iPct)) {
                        item->SetCount(item->GetCount() - 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        SetSkillNextReadTime(dwSkillVnum,
                                             get_global_time() + iReadDelay);

                        if (gConfig.testServer) {
                            ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] "
                                                       "Success to "
                                                       "learn skill ");
                        }
                    } else {
                        if (gConfig.testServer) {
                            ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] "
                                                       "Failed to "
                                                       "learn skill ");
                        }
                    }
                } break;

                    // MINING
                case ITEM_MINING_SKILL_TRAIN_BOOK: {
                    if (IsPolymorphed()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변신중에는 책을 읽을수 "
                                           "없습니다.");
                        return false;
                    }
                    uint32_t dwSkillVnum = SKILL_MINING;
                    int iPct = std::clamp<int>(item->GetValue(1), 0, 100);

                    if (GetSkillLevel(dwSkillVnum) >= 40) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더 이상 수련할 수 "
                                           "없습니다.");
                        return false;
                    }

                    if (LearnSkillByBook(dwSkillVnum, iPct)) {
                        item->SetCount(item->GetCount() - 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        SetSkillNextReadTime(dwSkillVnum,
                                             get_global_time() + iReadDelay);
                    }
                } break;
                    // END_OF_MINING

                case ITEM_HORSE_SKILL_TRAIN_BOOK: {
                    if (IsPolymorphed()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변신중에는 책을 읽을수 "
                                           "없습니다.");
                        return false;
                    }
                    uint32_t dwSkillVnum = SKILL_HORSE;
                    int iPct = std::clamp<int>(item->GetValue(1), 0, 100);

                    if (GetLevel() < 50) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "아직 승마 스킬을 수련할 수 "
                                           "있는 레벨이 아닙니다.");
                        return false;
                    }

                    if (!!quest::CQuestManager::instance().GetEventFlag("no_"
                                                                        "read_"
                                                                        "dela"
                                                                        "y") &&
                        get_global_time() < GetSkillNextReadTime(dwSkillVnum)) {
                        if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY)) {
                            // 주안술서 사용중에는 시간 제한 무시
                            RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "주안술서를 통해 "
                                               "주화입마에서 "
                                               "빠져나왔습니다.");
                        } else {
                            SkillLearnWaitMoreTimeMessage(
                                GetSkillNextReadTime(dwSkillVnum) -
                                get_global_time());
                            return false;
                        }
                    }

                    if (GetPoint(POINT_HORSE_SKILL) >= 20 ||
                        GetSkillLevel(SKILL_HORSE_WILDATTACK) +
                                GetSkillLevel(SKILL_HORSE_CHARGE) +
                                GetSkillLevel(SKILL_HORSE_ESCAPE) >=
                            60 ||
                        GetSkillLevel(SKILL_HORSE_WILDATTACK_RANGE) +
                                GetSkillLevel(SKILL_HORSE_CHARGE) +
                                GetSkillLevel(SKILL_HORSE_ESCAPE) >=
                            60) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더 이상 승마 수련서를 읽을 "
                                           "수 없습니다.");
                        return false;
                    }

                    if (Random::get(1, 100) <= iPct) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "승마 수련서를 읽어 승마 "
                                           "스킬 포인트를 얻었습니다.");
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "얻은 포인트로는 승마 "
                                           "스킬의 레벨을 올릴 수 "
                                           "있습니다.");
                        PointChange(POINT_HORSE_SKILL, 1);

                        int iReadDelay = Random::get<int>(SKILLBOOK_DELAY_MIN,
                                                          SKILLBOOK_DELAY_MAX);
                        if (!gConfig.testServer)
                            SetSkillNextReadTime(
                                dwSkillVnum, get_global_time() + iReadDelay);
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "승마 수련서 이해에 "
                                           "실패하였습니다.");
                    }

                    item->SetCount(item->GetCount() - 1);
                } break;

                case 70102: // 선두
                {
                    if (GetAlignment() >= 0)
                        return false;

                    const int delta =
                        std::min<int>(-GetAlignment(), item->GetValue(0));

                    SPDLOG_INFO("%s ALIGNMENT ITEM %d", GetName(), delta);

                    UpdateAlignment(delta);
                    item->SetCount(item->GetCount() - 1);

                    if (delta / 10 > 0) {
                        SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                           "마음이 맑아지는군. 가슴을 "
                                           "짓누르던 무언가가 좀 "
                                           "가벼워진 느낌이야.");
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "선악치가 %d 증가하였습니다.",
                                           delta / 10);
                    }
                } break;

                case 71107: // 천도복숭아
                case 39032: // 천도복숭아
                case 771107: {
                    int val = item->GetValue(0);
                    const int hourInterval = 60 * 60 * item->GetValue(1);
                    auto questPc =
                        quest::CQuestManager::instance().GetPC(GetPlayerID());
                    const auto lastUseTime = questPc->GetFlag("mythical_peach."
                                                              "last_use_time");

                    if (get_global_time() - lastUseTime < hourInterval) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "아직 사용할 수 없습니다.");
                        return false;
                    }

                    if (val > 0) {
                        if (GetAlignment() == 10000000) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "선악치를 더 이상 올릴 수 "
                                               "없습니다.");
                            return false;
                        }
                    } else {
                        if (GetAlignment() == -10000000) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "선악치를 더 이상 올릴 수 "
                                               "없습니다.");
                            return false;
                        }
                    }

                    const auto oldAlignment = GetAlignment() / 10;
                    UpdateAlignment(val * 10);

                    item->SetCount(item->GetCount() - 1);
                    questPc->SetFlag("mythical_peach.last_use_time",
                                     get_global_time());

                    SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                       "마음이 맑아지는군. 가슴을 "
                                       "짓누르던 무언가가 좀 가벼워진 "
                                       "느낌이야.");
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "선악치가 %d 증가하였습니다.", val);

                    LogManager::instance().CharLog(
                        this, val, "MYTHICAL_PEACH",
                        fmt::format("{} {}", oldAlignment, GetAlignment() / 10)
                            .c_str());
                } break;

                case 39046:
                case 90000: {
                    if (!item2)
                        return false;
                    if (item2->IsExchanging() == true)
                        return false;

                    if (item2->GetSocket(1) != 0) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "ACCE_CLEAR_ITEM_CAN_ONLY_"
                                           "BE_USE_ON_ACCE_WITH_"
                                           "ABSORBED_ITEM");
                    }

                    item2->ClearAttribute();
                    item2->SetSocket(1, 0);
                } break;
                case 71109: // 탈석서
                {
                    if (!item2)
                        return false;
                    if (item2->IsExchanging() == true)
                        return false;

                    if (item2->GetSocketCount() == 0)
                        return false;

                    switch (item2->GetItemType()) {
                        case ITEM_WEAPON:
                            break;
                        case ITEM_ARMOR:
                            switch (item2->GetSubType()) {
                                case ARMOR_EAR:
                                case ARMOR_WRIST:
                                case ARMOR_NECK:
                                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                       "빼낼 영석이 없습니다");
                                    return false;
                            }
                            break;

                        default:
                            return false;
                    }

                    std::stack<SocketValue> socket;

                    for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
                        socket.push(item2->GetSocket(i));

                    int idx = ITEM_SOCKET_MAX_NUM - 1;

                    while (!socket.empty()) {
                        if (socket.top() > 2 &&
                            socket.top() != ITEM_BROKEN_METIN_VNUM)
                            break;

                        idx--;
                        socket.pop();
                    }

                    if (socket.empty()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "빼낼 영석이 없습니다");
                        return false;
                    }

                    CItem* pItemReward = AutoGiveItem(socket.top());

                    if (pItemReward != nullptr) {
                        item2->SetSocket(idx, 1);

                        LogManager::instance().ItemLog(
                            this, item, "USE_DETACHMENT_ONE",
                            fmt::format("{}({}) {}({})", item2->GetName(),
                                        item2->GetID(), pItemReward->GetName(),
                                        pItemReward->GetID())
                                .c_str());
                    }
                } break;

                case ITEM_NEW_YEAR_GREETING_VNUM: {
                    uint32_t dwBoxVnum = ITEM_NEW_YEAR_GREETING_VNUM;
                    std::vector<uint32_t> dwVnums;
                    std::vector<uint32_t> dwCounts;
                    std::vector<CItem*> item_gets;
                    int count = 0;

                    if (GiveItemFromSpecialItemGroup(
                            dwBoxVnum, dwVnums, dwCounts, item_gets, count)) {
                        item->SetCount(item->GetCount() - 1);
                    }
                } break;

                case ITEM_VALENTINE_ROSE:
                case ITEM_VALENTINE_CHOCOLATE: {
                    uint32_t dwBoxVnum = item->GetVnum();
                    std::vector<uint32_t> dwVnums;
                    std::vector<uint32_t> dwCounts;
                    std::vector<CItem*> item_gets(0);
                    int count = 0;

                    if ((item->GetVnum() == ITEM_VALENTINE_ROSE &&
                         SEX_MALE == GetSexByRace(GetRaceNum())) ||
                        (item->GetVnum() == ITEM_VALENTINE_CHOCOLATE &&
                         SEX_FEMALE == GetSexByRace(GetRaceNum()))) {
                        // 성별이 맞지않아 쓸 수 없다.
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "성별이 맞지않아 이 "
                                           "아이템을 열 수 없습니다.");
                        return false;
                    }

                    if (GiveItemFromSpecialItemGroup(
                            dwBoxVnum, dwVnums, dwCounts, item_gets, count))
                        item->SetCount(item->GetCount() - 1);
                } break;

                case ITEM_WHITEDAY_CANDY:
                case ITEM_WHITEDAY_ROSE: {
                    uint32_t dwBoxVnum = item->GetVnum();
                    std::vector<uint32_t> dwVnums;
                    std::vector<uint32_t> dwCounts;
                    std::vector<CItem*> item_gets(
                        0); // c++11 compatibility change
                    int count = 0;

                    // TODO: This is already taken care of by CheckItemSex check
                    // if this is needed
                    if ((item->GetVnum() == ITEM_WHITEDAY_CANDY &&
                         SEX_MALE == GetSexByRace(GetRaceNum())) ||
                        (item->GetVnum() == ITEM_WHITEDAY_ROSE &&
                         SEX_FEMALE == GetSexByRace(GetRaceNum()))) {
                        // 성별이 맞지않아 쓸 수 없다.
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "성별이 맞지않아 이 "
                                           "아이템을 열 수 없습니다.");
                        return false;
                    }

                    if (GiveItemFromSpecialItemGroup(
                            dwBoxVnum, dwVnums, dwCounts, item_gets, count))
                        item->SetCount(item->GetCount() - 1);
                } break;

                case ITEM_GIVE_STAT_RESET_COUNT_VNUM: {
                    PointChange(POINT_STAT_RESET_COUNT, 1);
                    item->SetCount(item->GetCount() - 1);
                } break;

                case 50107: {
                    if (!CanUseFireworks())
                        return false;
                    EffectPacket(SE_CHINA_FIREWORK);
                    // 스턴 공격을 올려준다

                    item->SetCount(item->GetCount() - 1);
                    UpdateFireworksPulse();
                } break;

                case 50108: {
                    if (!CanUseFireworks())
                        return false;
                    if (CArenaManager::instance().IsArenaMap(GetMapIndex()) ==
                        true) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "대련 중에는 이용할 수 없는 "
                                           "물품입니다.");
                        return false;
                    }

                    EffectPacket(SE_SPIN_TOP);
                    item->SetCount(item->GetCount() - 1);
                    UpdateFireworksPulse();
                } break;

                case ITEM_WONSO_BEAN_VNUM:
                    PointChange(POINT_HP, GetMaxHP() - GetHP());
                    item->SetCount(item->GetCount() - 1);
                    break;

                case ITEM_WONSO_SUGAR_VNUM:
                    PointChange(POINT_SP, GetMaxSP() - GetSP());
                    item->SetCount(item->GetCount() - 1);
                    break;

                case ITEM_WONSO_FRUIT_VNUM:
                    PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());
                    item->SetCount(item->GetCount() - 1);
                    break;

                case 71092: // Transformation dismantling
                {
                    if (m_pkChrTarget != nullptr) {
                        if (m_pkChrTarget->IsPolymorphed()) {
                            m_pkChrTarget->SetPolymorph(0);
                            m_pkChrTarget->RemoveAffect(AFFECT_POLYMORPH);
                        }
                    } else {
                        if (IsPolymorphed()) {
                            SetPolymorph(0);
                            RemoveAffect(AFFECT_POLYMORPH);
                        }
                    }
                } break;

                    // Item Sealing TODO: USE_SEAL_ITEM
                case 50263: {
                    if (!item2)
                        return false;

                    if (!item2->IsSealAble()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "Item is not sealable");
                        return false;
                    }

                    if (item2->GetSealDate() == -1) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "Item already sealed");
                        return false;
                    }

                    item2->SetSealDate(-1);

                    TPacketGCSeal pack;
                    pack.pos = TItemPos(item2->GetWindow(), item2->GetCell());
                    pack.action = 0;
                    GetDesc()->Send(HEADER_GC_SEAL, pack);

                    ITEM_MANAGER::Instance().SaveSingleItem(item2);

                    item->SetCount(item->GetCount() - 1);

                    SendI18nChatPacket(this, CHAT_TYPE_INFO, "ITEM_SEALED");
                } break;
                    // TODO: USE_UNSEAL_ITEM
                case 50264: {
                    // Unbind Item

                    if (!item2)
                        return false;

                    if (!item2->IsSealAble())
                        return false;

                    if (item2->GetSealDate() == 0 ||
                        item2->GetSealDate() > time(nullptr)) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "ITEM_IS_NOT_SEALED");
                        return false;
                    }

                    if (item2->GetSealDate() == -1) {
                        item2->SetSealDate(time(nullptr) + (60 * 60 * 24 * 1));
                        item->SetCount(item->GetCount() - 1);
                        ITEM_MANAGER::Instance().SaveSingleItem(item2);

                        TPacketGCSeal pack = {};
                        pack.pos =
                            TItemPos(item2->GetWindow(), item2->GetCell());
                        pack.action = 1;
                        GetDesc()->Send(HEADER_GC_SEAL, pack);
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "ALREADY_IN_UNSEAL_MODE");
                        return false;
                    }
                } break;
            }

            break;

        case USE_CLEAR: {
            RemoveBadAffect();
            item->SetCount(item->GetCount() - 1);
        } break;

        case USE_INVISIBILITY: {
            AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, 300, 0, true);
            item->SetCount(item->GetCount() - 1);
        } break;

        case USE_POTION_NODELAY: {
            return HandleItemUsePotionNoDelay(item);
        }
        case USE_POTION: {
            return HandleItemUsePotion(item);
        }

        case USE_POTION_CONTINUE: {
            if (item->GetValue(0) != 0) {
                AddAffect(AFFECT_HP_RECOVER_CONTINUE, POINT_HP_RECOVER_CONTINUE,
                          item->GetValue(0), 0, item->GetValue(2), 0, true);
            } else if (item->GetValue(1) != 0) {
                AddAffect(AFFECT_SP_RECOVER_CONTINUE, POINT_SP_RECOVER_CONTINUE,
                          item->GetValue(1), 0, item->GetValue(2), 0, true);
            } else
                return false;
        }

            if (GetWarMap())
                GetWarMap()->UsePotion(this, item);

            item->SetCount(item->GetCount() - 1);
            break;

        case USE_ABILITY_UP: {
            return HandleItemUseAbilityUp(item);
        }

            /*
             * Enhances the time of a existing real_time item
             * limited by value0 type and value1 subtype of the target
             * item value2 holds the time it should add
             */

        case USE_ENHANCE_TIME: {
            return HandleItemUseEnhanceTime(item, item2);
        }

        case USE_LEVEL_PET_FOOD: {
            return HandleItemUseLevelPetFood(item, item2);
        }

        case USE_TALISMAN: {
            return HandleItemUseTalisman(item);
        }

        case USE_TUNING:
        case USE_DETACHMENT: {
            if (!item2)
                return false;

            if (item2->IsExchanging())
                return false;

            if (item->IsSealed()) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "ITEM_IS_SEALED_CANNOT_DO");
                return false;
            }

#if defined(SHELIA_BUILD)
            if (item2->GetVnum() >= 28030 &&
                item2->GetVnum() <= 28346) // 영석+3
            {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "Cannot use this for stones");
                return false;
            }
#endif

            RefineItem(item, item2);
        } break;

            //  ACCESSORY_REFINE & ADD/CHANGE_ATTRIBUTES
        case USE_PUT_INTO_BELT_SOCKET:
        case USE_PUT_INTO_RING_SOCKET:
        case USE_PUT_INTO_ACCESSORY_SOCKET:
        case USE_ADD_ACCESSORY_SOCKET:
        case USE_MAKE_ACCESSORY_SOCKET_PERM:
        case USE_CLEAN_SOCKET:
        case USE_CHANGE_ATTRIBUTE:
        case USE_CHANGE_COSTUME_ATTR:
        case USE_RESET_COSTUME_ATTR:
        case USE_CHANGE_ATTRIBUTE2:
        case USE_ADD_ATTRIBUTE:
        case USE_ADD_ATTRIBUTE2:
        case USE_ADD_ATTRIBUTE_RARE:
        case USE_ADD_ATTRIBUTE_PERM:
        case USE_CHANGE_ATTRIBUTE_RARE:
        case USE_CHANGE_ATTRIBUTE_PERM:
        case USE_LEVEL_PET_CHANGE_ATTR: {
            if (!item2)
                return false;

            if (!CanModifyItemAttr(item, item2, this)) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "속성을 변경할 수 없는 "
                                   "아이템입니다.");
                return false;
            }

            switch (item->GetSubType()) {
                case USE_CLEAN_SOCKET: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    int i;
                    for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
                        if (item2->GetSocket(i) == ITEM_BROKEN_METIN_VNUM)
                            break;
                    }

                    if (i == ITEM_SOCKET_MAX_NUM) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "청소할 석이 박혀있지 "
                                           "않습니다.");
                        return false;
                    }

                    int j = 0;

                    for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i) {
                        if (item2->GetSocket(i) != ITEM_BROKEN_METIN_VNUM &&
                            item2->GetSocket(i) != 0)
                            item2->SetSocket(j++, item2->GetSocket(i));
                    }

                    for (; j < ITEM_SOCKET_MAX_NUM; ++j) {
                        if (item2->GetSocket(j) > 0)
                            item2->SetSocket(j, 1);
                    }
                    {
                        LogManager::instance().ItemLog(
                            this, item, "CLEAN_SOCKET",
                            fmt::format("{}", item2->GetID()).c_str());
                    }

                    item->SetCount(item->GetCount() - 1);
                } break;

                case USE_LEVEL_PET_CHANGE_ATTR:
                    return HandleItemUseChangePetAttr(item, item2);

                case USE_CHANGE_ATTRIBUTE: {

                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->GetAttributeCount() == 0) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변경할 속성이 없습니다.");
                        return false;
                    }

                    if (item->GetSubType() == USE_CHANGE_ATTRIBUTE2) {
                        int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = {0, 0, 30,
                                                                      40, 3};

                        item2->ChangeAttribute(aiChangeProb);
                    } else {
                        // 연재경 특수처리
                        // 절대로 연재가 추가 안될거라 하여 하드 코딩함.
                        if (item->GetVnum() == 71151 ||
                            item->GetVnum() == 76023) {
                            if ((item2->GetItemType() == ITEM_WEAPON) ||
                                (item2->GetItemType() == ITEM_ARMOR)) {
                                bool bCanUse = true;
                                for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i) {
                                    if (item2->GetLimitType(i) == LIMIT_LEVEL &&
                                        item2->GetLimitValue(i) > 40) {
                                        bCanUse = false;
                                        break;
                                    }
                                }
                                if (false == bCanUse) {
                                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                       "적용 레벨보다 높아 "
                                                       "사용이 "
                                                       "불가능합니다.");
                                    break;
                                }
                            } else {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "무기와 갑옷에만 "
                                                   "사용 가능합니다.");
                                break;
                            }
                        }
                        item2->ChangeAttribute();
                    }

                    item->SetCount(item->GetCount() - 1);
                } break;

                case USE_CHANGE_ATTRIBUTE_PERM: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->GetAttributeCount() == 0) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변경할 속성이 없습니다.");
                        return false;
                    }

                    if (item->GetSubType() == USE_CHANGE_ATTRIBUTE2) {
                        int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = {0, 0, 30,
                                                                      40, 3};

                        item2->ChangeAttribute(aiChangeProb);
                    } else {
                        // 연재경 특수처리
                        // 절대로 연재가 추가 안될거라 하여 하드 코딩함.
                        if (item->GetVnum() != 71151 &&
                            item->GetVnum() != 76023) {
                        } else {
                            if (item2->GetItemType() != ITEM_WEAPON &&
                                item2->GetItemType() != ITEM_ARMOR) {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "무기와 갑옷에만 "
                                                   "사용 가능합니다.");
                                break;
                            } else {
                                bool bCanUse = true;
                                for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i) {
                                    if (item2->GetLimitType(i) == LIMIT_LEVEL &&
                                        item2->GetLimitValue(i) > 40) {
                                        bCanUse = false;
                                        break;
                                    }
                                }
                                if (false == bCanUse) {
                                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                       "적용 레벨보다 높아 "
                                                       "사용이 "
                                                       "불가능합니다.");
                                    break;
                                }
                            }
                        }
                        item2->ChangeAttribute();
                    }
                } break;

                case USE_ADD_ATTRIBUTE: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->GetAttributeCount() < 5) {
                        // 연재가 특수처리
                        // 절대로 연재가 추가 안될거라 하여 하드 코딩함.
                        if (item->GetVnum() == 71152 ||
                            item->GetVnum() == 76024) {
                            if ((item2->GetItemType() == ITEM_WEAPON) ||
                                (item2->GetItemType() == ITEM_ARMOR)) {
                                bool bCanUse = true;
                                for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i) {
                                    if (item2->GetLimitType(i) == LIMIT_LEVEL &&
                                        item2->GetLimitValue(i) > 40) {
                                        bCanUse = false;
                                        break;
                                    }
                                }
                                if (false == bCanUse) {
                                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                       "적용 레벨보다 높아 "
                                                       "사용이 "
                                                       "불가능합니다.");
                                    break;
                                }
                            } else {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "무기와 갑옷에만 "
                                                   "사용 가능합니다.");
                                break;
                            }
                        }

                        aiItemAttributeAddPercent[0] = 100;
                        aiItemAttributeAddPercent[1] = 100;
                        aiItemAttributeAddPercent[2] = 100;
                        aiItemAttributeAddPercent[3] = 100;
                        aiItemAttributeAddPercent[4] = 100;

                        if (Random::get(1, 100) <=
                            aiItemAttributeAddPercent
                                [item2->GetAttributeCount()]) {
                            item2->AddAttribute();
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "속성 추가에 "
                                               "성공하였습니다.");

                            int iAddedIdx = item2->GetAttributeCount() - 1;
                            LogManager::instance().ItemLog(
                                GetPlayerID(),
                                item2->GetAttributeType(iAddedIdx),
                                item2->GetAttributeValue(iAddedIdx),
                                item->GetID(), "ADD_ATTRIBUTE_SUCCESS",
                                fmt::format("{}", item2->GetID()).c_str(),
                                GetDesc()->GetHostName().c_str(),
                                item->GetOriginalVnum());
                        } else {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "속성 추가에 "
                                               "실패하였습니다.");
                        }

                        item->SetCount(item->GetCount() - 1);
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더이상 이 아이템을 "
                                           "이용하여 속성을 추가할 수 "
                                           "없습니다.");
                    }
                } break;

                case USE_ADD_ATTRIBUTE_PERM: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->GetAttributeCount() < 5) {
                        for (int i = 0; i < 5; ++i)
                            item2->AddAttribute();
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더이상 이 아이템을 "
                                           "이용하여 속성을 추가할 수 "
                                           "없습니다.");
                    }
                } break;

                case USE_ADD_ATTRIBUTE2: {

                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    // 축복의 구슬
                    // 재가비서를 통해 속성을 4개 추가 시킨 아이템에
                    // 대해서 하나의 속성을 더 붙여준다.
                    if (item2->GetAttributeSetIndex() == -1) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "속성을 변경할 수 없는 "
                                           "아이템입니다.");
                        return false;
                    }

                    // 속성이 이미 4개 추가 되었을 때만 속성을 추가
                    // 가능하다.
                    if (item2->GetAttributeCount() == 4) {
                        aiItemAttributeAddPercent[4] = 100;

                        if (Random::get(1, 100) <=
                            aiItemAttributeAddPercent[4]) {
                            item2->AddAttribute();
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "속성 추가에 "
                                               "성공하였습니다.");

                            int iAddedIdx = item2->GetAttributeCount() - 1;
                            LogManager::instance().ItemLog(
                                GetPlayerID(),
                                item2->GetAttributeType(iAddedIdx),
                                item2->GetAttributeValue(iAddedIdx),
                                item->GetID(), "ADD_ATTRIBUTE2_SUCCESS",
                                fmt::format("{}", item2->GetID()).c_str(),
                                GetDesc()->GetHostName().c_str(),
                                item->GetOriginalVnum());
                        } else {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "속성 추가에 "
                                               "실패하였습니다.");
                        }

                        item->SetCount(item->GetCount() - 1);
                    } else if (item2->GetAttributeCount() == 5) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더 이상 이 아이템을 "
                                           "이용하여 속성을 추가할 수 "
                                           "없습니다.");
                    } else if (item2->GetAttributeCount() < 4) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "먼저 재가비서를 이용하여 "
                                           "속성을 추가시켜 주세요.");
                    } else {
                        // wtf ?!
                        SPDLOG_ERROR("ADD_ATTRIBUTE2 : Item has wrong "
                                     "AttributeCount(%d)",
                                     item2->GetAttributeCount());
                    }
                } break;

                case USE_CHANGE_COSTUME_ATTR: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;
                    if (item2->GetProto()->bAlterToMagicItemPct != 100) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "You cannot change the attributes "
                                           "of this item.");
                        return false;
                    }
                    if (item2->GetAttributeCount() == 0) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변경할 속성이 없습니다.");
                        return false;
                    }

                    item2->ChangeAttribute();
                    item->SetCount(item->GetCount() - 1);
                } break;

                case USE_RESET_COSTUME_ATTR: {

                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->GetProto()->bAlterToMagicItemPct != 100) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "You cannot add attributes to this "
                                           "item.");
                        return false;
                    }
                    item2->ClearAttribute();
                    item2->AlterToMagicItem(Random::get(40, 50),
                                            Random::get(10, 15));
                    item->SetCount(item->GetCount() - 1);

                } break;

                case USE_MAKE_ACCESSORY_SOCKET_PERM: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (IS_SET(item2->GetFlag(), ITEM_FLAG_TIER_4)) {
                        if (item->GetValue(0) == 0) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "You can not make sockets on "
                                               "tier 4 or higher permanent "
                                               "with this item.");
                            return false;
                        }
                    }

                    if (item2->IsAccessoryForSocket()) {
                        if (item2->GetAccessorySocketGrade() <
                            ITEM_ACCESSORY_SOCKET_MAX_NUM) {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "You can only make sockets "
                                               "permanent if all slots are "
                                               "filled");
                            return false;
                        }

                        if (item2->GetAccessorySocketMinGrade() <
                            ITEM_ACCESSORY_SOCKET_MAX_NUM) {
                            if (Random::get(1, 100) <= 100) {
                                item2->SetAccessorySocketMinGrade(
                                    item2->GetAccessorySocketMinGrade() + 1);
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "Successfully made one "
                                                   "socket permanent");
                                LogManager::instance().ItemLog(
                                    this, item, "PERM_SOCKET_SUCCESS",
                                    fmt::format("{}", item2->GetID()).c_str());
                            } else {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "Failed to make one socket "
                                                   "permanent");
                                LogManager::instance().ItemLog(
                                    this, item, "PERM_SOCKET_FAIL",
                                    fmt::format("{}", item2->GetID()).c_str());
                            }

                            item->SetCount(item->GetCount() - 1);
                        } else {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "Already every socket "
                                               "permanent");
                        }
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 아이템으로 소켓을 "
                                           "추가할 수 없는 "
                                           "아이템입니다.");
                    }
                } break;

                case USE_ADD_ACCESSORY_SOCKET: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;
                    if (item2->IsAccessoryForSocket()) {
                        if (item2->GetAccessorySocketMaxGrade() <
                            ITEM_ACCESSORY_SOCKET_MAX_NUM) {
                            if (Random::get(1, 100) <= 100) {
                                item2->SetAccessorySocketMaxGrade(
                                    item2->GetAccessorySocketMaxGrade() + 1);
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "소켓이 성공적으로 "
                                                   "추가되었습니다.");
                                LogManager::instance().ItemLog(
                                    this, item, "ADD_SOCKET_SUCCESS",
                                    fmt::format("{}", item2->GetID()).c_str());
                            } else {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "소켓 추가에 "
                                                   "실패하였습니다.");
                                LogManager::instance().ItemLog(
                                    this, item, "ADD_SOCKET_FAIL",
                                    fmt::format("{}", item2->GetID()).c_str());
                            }

                            item->SetCount(item->GetCount() - 1);
                        } else {
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "이 액세서리에는 더이상 "
                                               "소켓을 추가할 공간이 "
                                               "없습니다.");
                        }
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 아이템으로 소켓을 "
                                           "추가할 수 없는 "
                                           "아이템입니다.");
                    }
                } break;

                case USE_PUT_INTO_BELT_SOCKET:
                case USE_PUT_INTO_ACCESSORY_SOCKET: {
                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->IsAccessoryForSocket() &&
                        item->CanPutInto(item2)) {
                        if (item2->GetAccessorySocketGrade() <
                            item2->GetAccessorySocketMaxGrade()) {
                            if (Random::get(1, 100) <=
                                aiAccessorySocketPutPct
                                    [item2->GetAccessorySocketGrade()]) {
                                item2->SetAccessorySocketGrade(
                                    item2->GetAccessorySocketGrade() + 1);
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "장착에 "
                                                   "성공하였습니다.");
                                LogManager::instance().ItemLog(
                                    this, item, "PUT_SOCKET_SUCCESS",
                                    fmt::format("{}", item2->GetID()).c_str());
                            } else {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "장착에 "
                                                   "실패하였습니다.");
                                LogManager::instance().ItemLog(
                                    this, item, "PUT_SOCKET_FAIL",
                                    fmt::format("{}", item2->GetID()).c_str());
                            }

                            item->SetCount(item->GetCount() - 1);
                        } else {
                            if (item2->GetAccessorySocketMaxGrade() == 0)
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "먼저 다이아몬드로 "
                                                   "악세서리에 소켓을 "
                                                   "추가해야합니다.");
                            else if (item2->GetAccessorySocketMaxGrade() <
                                     ITEM_ACCESSORY_SOCKET_MAX_NUM) {
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "이 액세서리에는 "
                                                   "더이상 장착할 "
                                                   "소켓이 없습니다.");
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "다이아몬드로 "
                                                   "소켓을 "
                                                   "추가해야합니다.");
                            } else
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "이 액세서리에는 더이상 "
                                                   "보석을 "
                                                   "장착할 수 없습니다.");
                        }
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "이 아이템을 장착할 수 "
                                           "없습니다.");
                    }
                } break;

                case USE_ADD_ATTRIBUTE_RARE: // 진재가
                {
                    if (!item2) {
                        return false;
                    }

                    if (item2->IsEquipped()) {
                        return false;
                    }

                    if (!item2->GetPosition().IsDefaultInventoryPosition())
                        return false;

                    if (item2->IsSealed())
                        return false;

                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->AddRareAttribute()) {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "성공적으로 속성이 추가 "
                                           "되었습니다");

                        int iAddedIdx = item2->GetRareAttrCount() + 4;

                        LogManager::instance().ItemLog(
                            GetPlayerID(), item2->GetAttributeType(iAddedIdx),
                            item2->GetAttributeValue(iAddedIdx), item->GetID(),
                            "ADD_RARE_ATTR",
                            fmt::format("{}", item2->GetID()).c_str(),
                            GetDesc()->GetHostName().c_str(),
                            item->GetOriginalVnum());

                        item->SetCount(item->GetCount() - 1);
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "더 이상 이 아이템으로 "
                                           "속성을 추가할 수 없습니다");
                    }
                } break;

                case USE_CHANGE_ATTRIBUTE_RARE: // 진재경
                {
                    if (!item2)
                        return false;

                    if (item2->IsEquipped())
                        return false;

                    if (item2->GetItemType() == ITEM_TOGGLE &&
                        item2->GetSubType() == TOGGLE_LEVEL_PET)
                        return false;

                    if (item2->ChangeRareAttribute()) {
                        item->SetCount(item->GetCount() - 1);
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "변경 시킬 속성이 없습니다");
                    }
                } break;
            }
        } break;
            //  END_OF_ACCESSORY_REFINE & END_OF_ADD_ATTRIBUTES &
            //  END_OF_CHANGE_ATTRIBUTES

        case USE_BAIT: {
            if (m_pkFishingEvent) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "낚시 중에 미끼를 갈아끼울 수 "
                                   "없습니다.");
                return false;
            }

            CItem* weapon = GetWear(WEAR_WEAPON);

            if (!weapon || weapon->GetItemType() != ITEM_ROD)
                return false;

            if (weapon->GetSocket(2)) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "이미 꽂혀있던 미끼를 빼고 %s를 "
                                   "끼웁니다.",
                                   TextTag::itemname(item->GetVnum()).c_str());
            } else {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "낚시대에 %s를 미끼로 끼웁니다.",
                                   TextTag::itemname(item->GetVnum()).c_str());
            }

            weapon->SetSocket(2, item->GetValue(0));

            if (item->GetVnum() == 27802)
                item->SetCount(item->GetCount() - 1);
        } break;

        case USE_MOVE:
        case USE_TREASURE_BOX:
        case USE_MONEYBAG:
            break;

        case USE_AFFECT: {
            if (FindAffect(item->GetValue(0),
                           GetApplyPoint(item->GetValue(1)))) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "이미 효과가 걸려 있습니다.");
            } else {
                EffectPacketByPointType(this, GetApplyPoint(item->GetValue(1)));
                AddAffect(item->GetValue(0), GetApplyPoint(item->GetValue(1)),
                          item->GetValue(2), item->GetValue(3), 0, false);
                item->SetCount(item->GetCount() - 1);
            }
        } break;

        case USE_CREATE_STONE:
            AutoGiveItem(Random::get(28000, 28013));
            item->SetCount(item->GetCount() - 1);
            break;

            // 물약 제조 스킬용 레시피 처리
        case USE_RECIPE: {
            CItem* pSource1 = FindSpecifyItem(item->GetValue(1));
            CountType sourceCount1 = item->GetValue(2);

            CItem* pSource2 = FindSpecifyItem(item->GetValue(3));
            CountType sourceCount2 = item->GetValue(4);

            if (sourceCount1 != 0) {
                if (pSource1 == nullptr) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "물약 조합을 위한 재료가 "
                                       "부족합니다.");
                    return false;
                }
            }

            if (sourceCount2 != 0) {
                if (pSource2 == nullptr) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "물약 조합을 위한 재료가 "
                                       "부족합니다.");
                    return false;
                }
            }

            if (pSource1 != nullptr) {
                if (pSource1->GetCount() < sourceCount1) {
                    SendI18nChatPacket(
                        this, CHAT_TYPE_INFO, "재료(%s)가 부족합니다.",
                        TextTag::itemname(pSource1->GetVnum()).c_str());
                    return false;
                }

                pSource1->SetCount(pSource1->GetCount() - sourceCount1);
            }

            if (pSource2 != nullptr) {
                if (pSource2->GetCount() < sourceCount2) {
                    SendI18nChatPacket(
                        this, CHAT_TYPE_INFO, "재료(%s)가 부족합니다.",
                        TextTag::itemname(pSource2->GetVnum()).c_str());
                    return false;
                }

                pSource2->SetCount(pSource2->GetCount() - sourceCount2);
            }

            CItem* pBottle = FindSpecifyItem(50901);

            if (!pBottle || pBottle->GetCount() < 1) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO, "빈 병이 모자릅니다.");
                return false;
            }

            pBottle->SetCount(pBottle->GetCount() - 1);

            if (Random::get(1, 100) > item->GetValue(5)) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "물약 제조에 실패했습니다.");
                return false;
            }

            AutoGiveItem(item->GetValue(0));
        } break;
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
        case USE_OFFLINE_SHOP_FARMED_TIME: {
            if (GetOfflineShopFarmedOpeningTime() + item->GetValue(0) >
                OFFLINE_SHOP_MAX_FARM_TIME) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot increase your farmed shop time "
                                   "this much");
                return false;
            }

            this->SetOfflineShopFarmedOpeningTime(
                this->GetOfflineShopFarmedOpeningTime() + item->GetValue(0));
            this->SendLeftOpeningTimePacket();
            item->SetCount(item->GetCount() - 1);
        } break;
        case USE_OFFLINE_SHOP_SPECIAL_TIME: {
            if (GetOfflineShopSpecialOpeningTime() + item->GetValue(0) >
                OFFLINE_SHOP_MAX_SPECIAL_TIME) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot increase your special shop time "
                                   "this much");
                return false;
            }

            this->SetOfflineShopSpecialOpeningTime(
                this->GetOfflineShopSpecialOpeningTime() + item->GetValue(0));
            this->SendLeftOpeningTimePacket();
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "Your special opening time for the offline shop "
                               "increased by %d hours.",
                               item->GetValue(0) / 60 / 60);
            item->SetCount(item->GetCount() - 1);

        } break;
#endif
        case USE_SET_TITLE: {
            ChatPacket(CHAT_TYPE_COMMAND, "ShowTitleInputDialog");
        } break;

        case USE_ADD_SOCKETS: {
            if (!item2)
                return false;

            if (item == item2)
                return false;

            if (item2->GetItemType() != ITEM_WEAPON &&
                (item2->GetItemType() != ITEM_ARMOR ||
                 item2->GetSubType() != ARMOR_BODY)) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot add sockets to this item.");
                return false;
            }
            
            if (ITEM_MANAGER::instance().IsSocketAddBlocked(item2->GetVnum())) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot add sockets to this item.");
                return false;
            }
            
            auto socketCount = item2->GetProto()->bGainSocketPct;
            if (socketCount < 3) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot add sockets to an item with "
                                   "less than 3 sockets.");
                return false;
            }

            if (item2->GetSocket(3) == 0) {
                item2->SetSocket(3, 1);
                item->SetCount(item->GetCount() - 1);
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You have successfully added a socket to "
                                   "this item.");
                return true;
            } else {
                if (item2->GetSocket(4) == 0) {
                    item2->SetSocket(4, 1);
                    item->SetCount(item->GetCount() - 1);
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "Successfully added a socket to this "
                                       "item");
                    return true;
                } else {
                    if (item2->GetSocket(5) == 0) {
                        item2->SetSocket(5, 1);
                        item->SetCount(item->GetCount() - 1);
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "Successfully added a socket to "
                                           "this item");
                        return true;
                    } else {
                        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                           "This item already cannot gain more "
                                           "sockets.");
                        return false;
                    }
                }
            }
        } break;

#ifdef ENABLE_BATTLE_PASS
        case USE_BATTLEPASS: {
            std::unique_ptr<SQLMsg> pmsg(DBManager::instance().DirectQuery(
                "SELECT * FROM battle_pass_ranking WHERE player_name = '{}' "
                "AND battle_pass_id = {};",
                GetName(), item->GetValue(0)));

            if (pmsg->Get()->uiNumRows > 0) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You have already completed this battle "
                                   "pass.");
                return false;
            }

            int iSeconds = GetSecondsTillNextMonth();

            if (iSeconds < 0) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "System error, please contact an "
                                   "administrator.");
                return false;
            }

            auto* affect = FindAffect(AFFECT_BATTLE_PASS);

            if (affect && static_cast<int>(affect->pointValue) ==
                              static_cast<int>(item->GetValue(0))) {

                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You already have this battle pass enabled "
                                   "on this character.");
                return false;
            } else {
                m_dwBattlePassEndTime = get_global_time() + iSeconds;
                RemoveAffect(AFFECT_BATTLE_PASS);
                AddAffect(AFFECT_BATTLE_PASS, POINT_NONE, item->GetValue(0),
                          iSeconds, 0, true);
                ITEM_MANAGER::instance().RemoveItem(item);
            }
        } break;
#endif
    }

    return true;
}

bool CHARACTER::HandleUseItemSkillbook(CItem* item)
{
    if (IsPolymorphed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "변신중에는 책을 읽을수 없습니다.");
        return false;
    }

    uint32_t dwVnum = 0;

    if (item->GetVnum() == 50300) {
        dwVnum = item->GetSocket(0);
    } else {
        // 새로운 수련서는 value 0 에 스킬 번호가 있으므로 그것을 사용.
        dwVnum = item->GetValue(0);
    }

    if (0 == dwVnum) {
        item->SetCount(item->GetCount() - 1);
        return false;
    }

    if (true == LearnSkillByBook(dwVnum)) {
        item->SetCount(item->GetCount() - 1);

        int iReadDelay =
            Random::get<int>(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

        SetSkillNextReadTime(dwVnum, get_global_time() + iReadDelay);
    }
    return true;
}

bool CHARACTER::HandleItemUseCampfire(CItem* item)
{
    if (!CanUseCampfire())
        return false;

    if (CWarMapManager::instance().IsWarMap(GetMapIndex()))
        return false;

    float fx, fy;
    GetDeltaByDegree(GetRotation(), 100.0f, &fx, &fy);

    SECTREE* tree = SECTREE_MANAGER::instance().Get(
        GetMapIndex(), (long)(GetX() + fx), (long)(GetY() + fy));

    if (!tree) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "모닥불을 피울 수 없는 지점입니다.");
        return false;
    }

    if (tree->IsAttr((long)(GetX() + fx), (long)(GetY() + fy), ATTR_WATER)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "물 속에 모닥불을 피울 수 없습니다.");
        return false;
    }

    CHARACTER* campfire = g_pCharManager->SpawnMob(
        fishing::CAMPFIRE_MOB, GetMapIndex(), (long)(GetX() + fx),
        (long)(GetY() + fy), 0, false, Random::get(0, 359));

    char_event_info* info = AllocEventInfo<char_event_info>();

    info->ch = campfire;

    campfire->m_despawnEvent =
        event_create(despawn_event, info, THECORE_SECS_TO_PASSES(40));

    item->SetCount(item->GetCount() - 1);
    UpdateCampfirePulse();
    return true;
}

bool CHARACTER::UseItemEx(CItem* item, CItem* item2)
{
    if (CArenaManager::instance().IsLimitedItem(GetMapIndex(),
                                                item->GetVnum()) == true) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "대련 중에는 이용할 수 없는 물품입니다.");
        return false;
    }

    const auto itemId = item->GetID();

#ifdef ENABLE_BATTLE_PASS
    uint8_t bBattlePassId = GetBattlePassId();
    if (bBattlePassId) {
        uint32_t dwItemVnum, dwUseCount;
        if (CBattlePass::instance().BattlePassMissionGetInfo(
                bBattlePassId, USE_ITEM, &dwItemVnum, &dwUseCount)) {
            if (dwItemVnum == item->GetVnum() &&
                GetMissionProgress(USE_ITEM, bBattlePassId) < dwUseCount)
                UpdateMissionProgress(USE_ITEM, bBattlePassId, 1, dwUseCount);
        }
    }
#endif

    if (item->GetItemType() == ITEM_QUEST && IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_APPLICABLE) && item2) {
        if (item->GetSIGVnum() == 0) {
            quest::CQuestManager::instance().UseItemToItem(GetPlayerID(), item->GetVnum(),
                                                     item2);
          return true;
        }
    }

    switch (item->GetItemType()) {
        case ITEM_POLYMORPH:
            return ItemProcess_Polymorph(item);

        case ITEM_QUEST: {
            if (GetArena() != nullptr || IsObserverMode() == true) {
                if (item->GetVnum() == 50051 || item->GetVnum() == 50052 ||
                    item->GetVnum() == 50053) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "대련 중에는 이용할 수 없는 "
                                       "물품입니다.");
                    return false;
                }
            }

            if (!IS_SET(item->GetFlag(),
                        ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)) {
                if (item->GetSIGVnum() == 0) {
                    quest::CQuestManager::instance().UseItem(GetPlayerID(),
                                                             item, false);
                } else {
                    quest::CQuestManager::instance().SIGUse(
                        GetPlayerID(), item->GetSIGVnum(), item, false);
                }
            }
        } break;

        case ITEM_CAMPFIRE: {
            if (!HandleItemUseCampfire(item))
                return false;
        } break;

        case ITEM_COSTUME:
        case ITEM_WEAPON:
        case ITEM_ARMOR:
        case ITEM_ROD:
        case ITEM_RING:
        case ITEM_TALISMAN:
        case ITEM_BELT:
        case ITEM_PICK:
        case ITEM_SPECIAL_DS:
        case ITEM_UNIQUE:
        case ITEM_DS: {
            if (!item->IsEquipped())
                EquipItem(item);
            else
                UnequipItem(item);

            break;
        }

        case ITEM_FISH: {
            if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "대련 중에는 이용할 수 없는 물품입니다.");
                return false;
            }

            if (item->GetSubType() == FISH_ALIVE)
                fishing::UseFish(this, item);
        } break;

        case ITEM_TREASURE_BOX: {
            return false;
        }

        case ITEM_TREASURE_KEY: {
            if (!item2)
                return false;

            if (item2->IsExchanging())
                return false;

            if (item2->GetItemType() != ITEM_TREASURE_BOX) {
                SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                   "열쇠로 여는 물건이 아닌것 같다.");
                return false;
            }

            if (item->GetValue(0) == item2->GetValue(0)) {
                // SendI18nChatPacket(this, CHAT_TYPE_TALKING, "열쇠는 맞으나
                // 아이템 주는 부분 구현이 안되었습니다.");
                uint32_t dwBoxVnum = item2->GetVnum();
                std::vector<uint32_t> dwVnums;
                std::vector<uint32_t> dwCounts;
                std::vector<CItem*> item_gets(0); // c++11 compatibility change
                int count = 0;

                if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts,
                                                 item_gets, count)) {
                    ITEM_MANAGER::instance().RemoveItem(item);
                    ITEM_MANAGER::instance().RemoveItem(item2);

                    for (int i = 0; i < count; i++) {
                        switch (dwVnums[i]) {
                            case CSpecialItemGroup::GOLD:
                                break;
                            case CSpecialItemGroup::EXP:
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "상자에서 부터 신비한 빛이 "
                                                   "나옵니다.");
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "%d의 경험치를 "
                                                   "획득했습니다.",
                                                   dwCounts[i]);
                                break;
                            case CSpecialItemGroup::MOB:
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "상자에서 몬스터가 "
                                                   "나타났습니다!");
                                break;
                            case CSpecialItemGroup::SLOW:
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "상자에서 나온 빨간 연기를 "
                                                   "들이마시자 움직이는 속도가 "
                                                   "느려졌습니다!");
                                break;
                            case CSpecialItemGroup::DRAIN_HP:
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "상자가 갑자기 "
                                                   "폭발하였습니다! 생명력이 "
                                                   "감소했습니다.");
                                break;
                            case CSpecialItemGroup::POISON:
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "상자에서 나온 녹색 연기를 "
                                                   "들이마시자 독이 온몸으로 "
                                                   "퍼집니다!");
                                break;
                            case CSpecialItemGroup::MOB_GROUP:
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "상자에서 몬스터가 "
                                                   "나타났습니다!");
                                break;
                            case CSpecialItemGroup::POLY_MARBLE:
                                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                                   "There was a poly marble in "
                                                   "the chest!");
                                break;
                            default:
                                if (item_gets[i]) {
                                    if (dwCounts[i] > 1)
                                        SendI18nChatPacket(
                                            this, CHAT_TYPE_INFO,
                                            "상자에서 %s 가 %d 개 나왔습니다.",
                                            TextTag::itemname(
                                                item_gets[i]->GetVnum())
                                                .c_str(),
                                            dwCounts[i]);
                                    else
                                        SendI18nChatPacket(
                                            this, CHAT_TYPE_INFO,
                                            "상자에서 %s 가 나왔습니다.",
                                            TextTag::itemname(
                                                item_gets[i]->GetVnum())
                                                .c_str());
                                }
                        }
                    }
                } else {
                    SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                       "열쇠가 맞지 않는 것 같다.");
                    return false;
                }
            } else {
                SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                   "열쇠가 맞지 않는 것 같다.");
                return false;
            }
        } break;

        case ITEM_GIFTBOX: {
            auto emptyInven = GetEmptyNormalInventoryCount(3);
            if (emptyInven < 2) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You need at least 6 free inventory slots "
                                   "to open a chest.");
                return false;
            }
            emptyInven = GetEmptyCostumeInventoryCount(3);
            if (emptyInven < 2) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You need at least 6 free costume inventory slots "
                                   "to open a chest.");
                return false;
            }
            emptyInven = GetEmptyMaterialInventoryCount(3);
            if (emptyInven < 2) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You need at least 6 free material inventory slots "
                                   "to open a chest.");
                return false;
            }
            uint32_t dwBoxVnum = item->GetVnum();
            std::vector<uint32_t> dwVnums;
            std::vector<uint32_t> dwCounts;
            std::vector<CItem*> item_gets(0); // c++11 compatibility change
            int count = 0;

            if (dwBoxVnum > 51500 && dwBoxVnum < 52000) // 용혼원석들
            {
                if (!(this->DragonSoul_IsQualified())) {
                    SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                       "먼저 용혼석 퀘스트를 완료하셔야 "
                                       "합니다.");
                    return false;
                }
            }

            if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts,
                                             item_gets, count)) {
                item->SetCount(item->GetCount() - 1);

                for (int i = 0; i < count; i++) {
                    switch (dwVnums[i]) {
                        case CSpecialItemGroup::GOLD:
                        case CSpecialItemGroup::EXP:
                            break;
                        case CSpecialItemGroup::MOB:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 몬스터가 "
                                               "나타났습니다!");
                            break;
                        case CSpecialItemGroup::SLOW:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 나온 빨간 연기를 "
                                               "들이마시자 움직이는 속도가 "
                                               "느려졌습니다!");
                            break;
                        case CSpecialItemGroup::DRAIN_HP:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자가 갑자기 폭발하였습니다! "
                                               "생명력이 감소했습니다.");
                            break;
                        case CSpecialItemGroup::POISON:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 나온 녹색 연기를 "
                                               "들이마시자 독이 온몸으로 "
                                               "퍼집니다!");
                            break;
                        case CSpecialItemGroup::MOB_GROUP:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 몬스터가 "
                                               "나타났습니다!");
                            break;
                        case CSpecialItemGroup::POLY_MARBLE:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "There was a poly marble in the "
                                               "chest!");
                            break;
                        default:
                            if (item_gets[i]) {
                                if (dwCounts[i] > 1)
                                    SendI18nChatPacket(
                                        this, CHAT_TYPE_INFO,
                                        "상자에서 %s 가 %d 개 나왔습니다.",
                                        TextTag::itemname(
                                            item_gets[i]->GetVnum())
                                            .c_str(),
                                        dwCounts[i]);
                                else
                                    SendI18nChatPacket(
                                        this, CHAT_TYPE_INFO,
                                        "상자에서 %s 가 "
                                        "나왔습니다.",
                                        TextTag::itemname(
                                            item_gets[i]->GetVnum())
                                            .c_str());
                            }
                    }
                }
            } else {
                SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                   "아무것도 얻을 수 없었습니다.");
                return false;
            }
        } break;

        case ITEM_SKILLFORGET: {
            if (!item->GetSocket(0)) {
                ITEM_MANAGER::instance().RemoveItem(item);
                return false;
            }

            uint32_t dwVnum = item->GetSocket(0);

            if (SkillLevelDown(dwVnum)) {
                ITEM_MANAGER::instance().RemoveItem(item);
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "스킬 레벨을 내리는데 성공하였습니다.");
            } else
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "스킬 레벨을 내릴 수 없습니다.");
        } break;

        case ITEM_SKILLBOOK: {
            if (!HandleUseItemSkillbook(item))
                return false;
        } break;

        case ITEM_USE: {
            if (!HandleUseItemUse(item, item2))
                return false;
        } break;

        case ITEM_METIN: {
            if (!HandleUseItemMetin(item, item2))
                return false;
        } break;

        case ITEM_AUTOUSE:
        case ITEM_MATERIAL:
        case ITEM_SPECIAL:
        case ITEM_TOOL:
        case ITEM_LOTTERY:
            break;

        case ITEM_TOTEM: {
            if (!item->IsEquipped())
                EquipItem(item);
        } break;

        case ITEM_GACHA: {
            uint32_t dwBoxVnum = item->GetVnum();
            std::vector<uint32_t> dwVnums;
            std::vector<uint32_t> dwCounts;
            std::vector<CItem*> item_gets(0);
            int count = 0;

            if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts,
                                             item_gets, count)) {
                long lOpenCount = item->GetSocket(0);
                uint16_t wCount = lOpenCount / item->GetLimitValue(1),
                         wRest = (lOpenCount % item->GetLimitValue(1));
                item->SetSocket(0, item->GetSocket(0) - 1);
                if (wRest == 1) {
                    if (IS_SET(item->GetFlag(), ITEM_FLAG_STACKABLE) &&
                        !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
                        item->GetCount() > 1) {
                        item->SetCount(item->GetCount() - 1);
                    } else {
                        ITEM_MANAGER::instance().RemoveItem(item);
                    }
                }

                for (int i = 0; i < count; i++) {
                    switch (dwVnums[i]) {
                        case CSpecialItemGroup::GOLD:
                            break;
                        case CSpecialItemGroup::EXP:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 부터 신비한 빛이 "
                                               "나옵니다.");
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "%d의 경험치를 획득했습니다.",
                                               dwCounts[i]);
                            break;
                        case CSpecialItemGroup::MOB:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 몬스터가 "
                                               "나타났습니다!");
                            break;
                        case CSpecialItemGroup::SLOW:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 나온 빨간 연기를 "
                                               "들이마시자 움직이는 속도가 "
                                               "느려졌습니다!");
                            break;
                        case CSpecialItemGroup::DRAIN_HP:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자가 갑자기 "
                                               "폭발하였습니다! 생명력이 "
                                               "감소했습니다.");
                            break;
                        case CSpecialItemGroup::POISON:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 나온 녹색 연기를 "
                                               "들이마시자 독이 온몸으로 "
                                               "퍼집니다!");
                            break;
                        case CSpecialItemGroup::MOB_GROUP:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "상자에서 몬스터가 "
                                               "나타났습니다!");
                            break;
                        case CSpecialItemGroup::POLY_MARBLE:
                            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                               "There was a poly marble in the "
                                               "chest!");
                            break;
                        default:
                            if (item_gets[i]) {
                                if (dwCounts[i] > 1)
                                    SendI18nChatPacket(
                                        this, CHAT_TYPE_INFO,
                                        "상자에서 %s 가 %d 개 "
                                        "나왔습니다.",
                                        TextTag::itemname(
                                            item_gets[i]->GetVnum())
                                            .c_str(),
                                        dwCounts[i]);
                                else
                                    SendI18nChatPacket(
                                        this, CHAT_TYPE_INFO,
                                        "상자에서 %s 가 "
                                        "나왔습니다.",
                                        TextTag::itemname(
                                            item_gets[i]->GetVnum())
                                            .c_str());
                            }
                    }
                }
            } else {
                SendI18nChatPacket(this, CHAT_TYPE_TALKING,
                                   "아무것도 얻을 수 "
                                   "없었습니다.");
                return false;
            }
        } break;

        case ITEM_EXTRACT: {
            CItem* pDestItem = item2;
            if (nullptr == pDestItem) {
                return false;
            }
            switch (item->GetSubType()) {
                case EXTRACT_DRAGON_SOUL:
                    if (pDestItem->IsDragonSoul()) {
                        return DSManager::instance().PullOut(this, NPOS,
                                                             pDestItem, item);
                    }
                    return false;
                case EXTRACT_DRAGON_HEART:
                    if (pDestItem->IsDragonSoul()) {
                        return DSManager::instance().ExtractDragonHeart(
                            this, pDestItem, item);
                    }
                    return false;
                default:
                    return false;
            }
        } break;

        case ITEM_TOGGLE:
            if (!events::Item::OnUse(this, item))
                return false;
            break;
        case ITEM_NONE:
            SPDLOG_ERROR("Item type NONE {]", item->GetName());
            break;

        default:
            SPDLOG_INFO("UseItemEx: Unknown type {} {}", item->GetName(),
                        item->GetItemType());
            return false;
    }

    // The item might have been destroyed by some of the code above.
    // Try to find it again. If that fails, we simply don't signal the event.
    // TODO: Refactor everything!
    item = ITEM_MANAGER::instance().Find(itemId);

    // Only signal the Use event if EquipItem didn't.
    if (item && !item->IsEquipped())
        item->OnUse();

    return true;
}

int g_nPortalLimitTime = 10;

bool IsNotLimitedByPulseItem(const CItem* item)
{
    switch (item->GetItemType()) {
        case ITEM_USE: {
            switch (item->GetSubType()) {
                case USE_CHANGE_ATTRIBUTE:
                case USE_CHANGE_COSTUME_ATTR:
                case USE_RESET_COSTUME_ATTR:
                case USE_CHANGE_ATTRIBUTE2:
                case USE_ADD_ATTRIBUTE:
                case USE_ADD_ATTRIBUTE2:
                case USE_ADD_ATTRIBUTE_RARE:
                case USE_ADD_ATTRIBUTE_PERM:
                case USE_CHANGE_ATTRIBUTE_RARE:
                case USE_CHANGE_ATTRIBUTE_PERM:
                case USE_TUNING:
                    return true;
                default:
                    break;
            }
        } break;
        default:
            break;
    }

    return false;
}

bool CHARACTER::UseItem(const TItemPos& Cell, const TItemPos& DestCell,
                        bool skipTimeChecks)
{
    if (!CanHandleItem())
        return false;

    if (!IsValidItemPosition(Cell))
        return false;

    CItem* item = GetItem(Cell);
    if (!item)
        return false;

    if (!IsNextUseItemPule() && !IsNotLimitedByPulseItem(item) &&
        !skipTimeChecks)
        return false;

    if (skipTimeChecks && item->GetItemType() != ITEM_GIFTBOX)
        return false;

    // We don't want to use it if we are dragging it over another item of the
    // same type...
    auto* destItem = GetItem(DestCell);
    if (destItem && item != destItem && destItem->IsStackable() &&
        !IS_SET(destItem->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
        destItem->GetVnum() == item->GetVnum()) {
        if (MoveItem(Cell, DestCell, 0))
            return false;
    }

    SPDLOG_TRACE("{0}: USE_ITEM {1} (inven {2}, cell: {3})", GetName(),
                 item->GetName(), Cell.window_type, Cell.cell);

    if (item->IsExchanging())
        return false;

    if (IsStun())
        return false;

    if (!CheckItemLimits(this, item, true))
        return false;

    if (!CheckItemRace(this, item)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "군직이 맞지않아 이 아이템을 사용할 수 없습니다.");
        return false;
    }

    if (!CheckItemSex(this, item)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "성별이 맞지않아 이 아이템을 사용할 수 없습니다.");
        return false;
    }

    if (!CheckItemEmpire(this, item)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "This item cannot be used by people from your "
                           "empire");
        return false;
    }

    const auto itemVnum = item->GetVnum();
    auto now = get_global_time();

    // PREVENT_TRADE_WINDOW
    if (IsSummonItem(itemVnum)) {
        if (!IsSummonableZone(GetMapIndex())) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "사용할수 없습니다.");
            return false;
        }

        // 경혼반지 사용지 상대방이 SUMMONABLE_ZONE에 있는가는 WarpToPC()에서
        // 체크
        if (IsHack())
            return false;
        // END_PREVENT_ITEM_COPY

        if (const auto questPc =
                quest::CQuestManager::instance().GetPC(GetPlayerID());
            questPc) {
            uint32_t summonItemUseCycle =
                quest::CQuestManager::instance().GetEventFlag("summon_item_use_"
                                                              "cycle");
            if (!summonItemUseCycle)
                summonItemUseCycle = 1;
            uint32_t lastSummonItemUse = questPc->GetFlag("SummonItem."
                                                          "LastUseTime");

            if (lastSummonItemUse + summonItemUseCycle > now) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You can not use this so fast.");
                return false;
            }
        }

        //귀환부 거리체크
        if (item->GetVnum() != ITEM_MARRIAGE_RING) {
            PIXEL_POSITION posWarp;

            int x = 0;
            int y = 0;
            int mapIndex = 0;

            double nDist = 0;
            const double nDistant = 5000.0;
            //귀환기억부
            if (item->GetVnum() == 22010) {
                mapIndex = item->GetSocket(2);
                x = item->GetSocket(0);
                y = item->GetSocket(1);
            }
            //귀환부
            else if (item->GetVnum() == 22000) {
                SECTREE_MANAGER::instance().GetRecallPositionByEmpire(
                    GetMapIndex(), GetEmpire(), posWarp);

                if (item->GetSocket(0) == 0) {
                    mapIndex = GetMapIndex();
                    x = posWarp.x;
                    y = posWarp.y;
                } else {
                    mapIndex = item->GetSocket(2);
                    x = item->GetSocket(0);
                    y = item->GetSocket(1);
                }
            }

            nDist = sqrt(pow((float)x, 2) + pow((float)y, 2));

            if (nDistant > nDist && GetMapIndex() == mapIndex) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "이동 되어질 위치와 너무 가까워 귀환부를 "
                                   "사용할수 없습니다.");
                if (gConfig.testServer)
                    ChatPacket(CHAT_TYPE_INFO, "PossibleDistant %f nNowDist %f",
                               nDistant, nDist);
                return false;
            }
        }
    }

    //보따리 비단 사용시 거래창 제한 체크
    if (item->GetVnum() == 50200 || item->GetVnum() == 71049) {
        if (GetExchange() || IsShop() || GetShopOwner() || IsOpenSafebox() ||
            IsCubeOpen()) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "거래창,창고 등을 연 상태에서는 "
                               "보따리,비단보따리를 사용할수 없습니다.");
            return false;
        }
    }
    // END_PREVENT_TRADE_WINDOW

    CItem* item2 = nullptr;

    if (IsValidItemPosition(DestCell) && !DestCell.IsEquipPosition()) {
        item2 = GetItem(DestCell);

        if (item2 && item2->IsExchanging())
            return false;
    }

    const uint32_t vnum = item->GetVnum();

    bool ret = false;

    if (IS_SET(item->GetFlag(), ITEM_FLAG_LOG)) {
        uint32_t vid = item->GetVID();
        CountType oldCount = item->GetCount();

        std::string hint = item->GetName();
        ret = UseItemEx(item, item2);

        if (nullptr == ITEM_MANAGER::instance().FindByVID(vid)) {
            LogManager::instance().ItemLog(this, vid, vnum, "REMOVE",
                                           hint.c_str());
        } else if (oldCount != item->GetCount()) {
            hint += fmt::format(" {}", oldCount - 1);
            LogManager::instance().ItemLog(this, vid, vnum, "USE_ITEM",
                                           hint.c_str());
        }
    } else {
        ret = UseItemEx(item, item2);
    }

    if (ret) {
        if (IsSummonItem(itemVnum)) {
            auto* questPc =
                quest::CQuestManager::instance().GetPC(GetPlayerID());
            if (questPc) {
                questPc->SetFlag("SummonItem.LastUseTime", now);
            }
        }
    }

    UpdateUseItemPulse();
    return ret;
}

bool CHARACTER::DropItem(const TItemPos& Cell, CountType count)
{
    constexpr bool enableDropItems = false;

    if constexpr (!enableDropItems) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "Dropping items is disabled use destroy and "
                           "exchange instead.");
        return false;
    } else {
        CItem* item = nullptr;

        if (!CanHandleItem()) {
            if (DragonSoul_RefineWindow_IsOpen())
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "강화창을 연 상태에서는 아이템을 옮길 수 "
                                   "없습니다.");
            return false;
        }

        if (IsDead())
            return false;

        if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
            return false;

        if (item->IsExchanging())
            return false;

        if (item->IsSealed()) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "ITEM_IS_SEALED_CANNOT_DO");
            return false;
        }

#ifdef __NEW_GAMEMASTER_CONFIG__
        if ((!item->IsGMOwner() &&
             !GM::check_allow(GetGMLevel(), GM_ALLOW_DROP_PLAYER_ITEM)) ||
            (item->IsGMOwner() &&
             !GM::check_allow(GetGMLevel(), GM_ALLOW_DROP_GM_ITEM))) {
            ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot do this with this "
                                               "gamemaster rank."));
            return false;
        }
#endif

        if (true == item->isLocked())
            return false;

        if (quest::CQuestManager::instance()
                .GetPCForce(GetPlayerID())
                ->IsRunning() == true)
            return false;

        if (IS_SET(item->GetAntiFlag(),
                   ITEM_ANTIFLAG_DROP | ITEM_ANTIFLAG_GIVE) ||
            !CanTradeRestrictedItem(item, this)) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "버릴 수 없는 아이템입니다.");
            return false;
        }

        if (count == 0 || count > item->GetCount())
            count = item->GetCount();

        SyncQuickslot(
            QUICKSLOT_TYPE_ITEM, Cell.cell,
            std::numeric_limits<uint16_t>::max()); // Quickslot 에서 지움

        CItem* pkItemToDrop;

        if (count == item->GetCount()) {
            item->RemoveFromCharacter();
            pkItemToDrop = item;
        } else {
            if (count == 0) {
                if (gConfig.testServer)
                    SPDLOG_INFO("[DROP_ITEM] drop item count == 0");
                return false;
            }

            item->SetCount(item->GetCount() - count);
            ITEM_MANAGER::instance().FlushDelayedSave(item);

            pkItemToDrop =
                ITEM_MANAGER::instance().CreateItem(item->GetVnum(), count);

            // copy item socket -- by mhh
            FN_copy_item_socket(pkItemToDrop, item);

            char szBuf[51 + 1];
            std::snprintf(szBuf, sizeof(szBuf), "%u %u", pkItemToDrop->GetID(),
                          pkItemToDrop->GetCount());
            LogManager::instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
        }

        PIXEL_POSITION pxPos = GetXYZ();

        if (pkItemToDrop->AddToGround(GetMapIndex(), pxPos)) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "떨어진 아이템은 3분 후 사라집니다.");

            pkItemToDrop->StartDestroyEvent(gConfig.itemGroundTime);

            ITEM_MANAGER::instance().FlushDelayedSave(pkItemToDrop);

            char szHint[32 + 1];
            std::snprintf(szHint, sizeof(szHint), "%s %u %u",
                          pkItemToDrop->GetName(), pkItemToDrop->GetCount(),
                          pkItemToDrop->GetOriginalVnum());
            LogManager::instance().ItemLog(this, pkItemToDrop, "DROP", szHint);
            // Motion(MOTION_PICKUP);
        }

        return true;
    }
}

bool CHARACTER::DropGold(Gold gold)
{
    SendI18nChatPacket(this, CHAT_TYPE_INFO, "YOU_CANNOT_DROP_GOLD");
    return false;
}

bool CHARACTER::SwapItems(const TItemPos& Cell, TItemPos& DestCell, CItem* item)
{
    if (!DestCell.IsDefaultInventoryPosition() ||
        !Cell.IsDefaultInventoryPosition())
        // Only this kind of swapping on inventory
        return false;

    auto targetItem = GetItem_NEW(DestCell);
    if (targetItem &&
        targetItem->GetVID() == item->GetVID()) // Can't swap over my own slots
        return false;

    DestCell = TItemPos(INVENTORY, targetItem->GetCell());

    if (!CanModifyItem(item) || !CanModifyItem(targetItem))
        return false;

    const uint8_t basePage = DestCell.cell / (INVENTORY_PAGE_SIZE);
    std::map<uint16_t, CItem*> moveItemMap;
    auto sizeLeft = item->GetSize();

    for (uint16_t i = 0; i < item->GetSize(); ++i) {
        const uint16_t cellNumber = DestCell.cell + i * 5;

        const uint8_t cPage = cellNumber / (INVENTORY_PAGE_SIZE);
        if (basePage != cPage)
            return false;

        if (auto mvItem = GetItem(TItemPos(INVENTORY, cellNumber)); mvItem) {
            if (mvItem->GetSize() > item->GetSize())
                return false;

            if (mvItem->IsExchanging())
                return false;

            moveItemMap.emplace(Cell.cell + i * 5, mvItem);
            sizeLeft -= mvItem->GetSize();

            if (mvItem->GetSize() > 1)
                i += mvItem->GetSize() -
                     1; // Skip checking the obviously used cells
        } else {
            sizeLeft -= 1; // Empty slot
        }
    }

    if (sizeLeft != 0)
        return false;

    // This map will hold cell positions for syncing the quickslots
    // afterwards
    std::map<uint8_t, uint16_t> syncCells; // Quickslot pos -> Target cell.

    // Let's remove the original item
    syncCells.emplace(
        GetQuickslotPosition(QUICKSLOT_TYPE_ITEM, item->GetCell()),
        DestCell.cell);
    item->RemoveFromCharacter();
    item->AddToCharacter(this, DestCell);

    for (auto& [toCellNumber, mvItem] : moveItemMap) {
        syncCells.emplace(
            GetQuickslotPosition(QUICKSLOT_TYPE_ITEM, mvItem->GetCell()),
            toCellNumber);

        mvItem->RemoveFromCharacter();
        mvItem->AddToCharacter(this, TItemPos(INVENTORY, toCellNumber));
    }

    // Sync quickslots only after all is set
    for (auto& [qCell, tCell] : syncCells) {
        TQuickslot qs = {QUICKSLOT_TYPE_ITEM, tCell};
        SetQuickslot(qCell, qs);
    }

    return true;
}

bool CHARACTER::SplitItem(const TItemPos& Cell, CountType count)
{
    if (count == 0 || count < 0)
        return false;

    if (!IsValidItemPosition(Cell))
        return false;

    if (!CanHandleItem())
        return false;

    CItem* item = GetItem(Cell);
    if (!item)
        return false;

    if (!CanModifyItem(item) || !CanRemove(this, item))
        return false;

    constexpr bool enabled = true;
    if(enabled) {
            
        const auto itemCount = item->GetCount();
        const auto itemVnum = item->GetVnum();
        const auto splitCount = itemCount / count;

        for (int i = 0; i < splitCount; ++i) {
            auto emptyInven = GetEmptyInventory(item);
            if (emptyInven == -1)
                break;

            auto* itemNew = ITEM_MANAGER::instance().CreateItem(itemVnum, count);
            FN_copy_item_socket(itemNew, item);
            LogManager::instance().ItemLog(
                this, item, "ITEM_SPLIT_MULT",
                fmt::format("{} {} {} {} ", itemNew->GetID(), itemNew->GetCount(),
                            item->GetCount(),
                            item->GetCount() + itemNew->GetCount())
                    .c_str());

            item->SetCount(item->GetCount() - count);

            itemNew->AddToCharacter(this, TItemPos(INVENTORY, emptyInven));
            ITEM_MANAGER::instance().FlushDelayedSave(itemNew);
        }
    }

    return true;
}

bool CHARACTER::MoveItem(const TItemPos& Cell, TItemPos DestCell,
                         CountType count)
{
    if (!IsValidItemPosition(Cell) || !IsValidItemPosition(DestCell))
        return false;

    if (!CanHandleItem())
        return false;

    CItem* item = GetItem(Cell);
    if (!item)
        return false;

    if (!CanModifyItem(item) || !CanRemove(this, item))
        return false;

    if (item->GetCount() < count)
        return false;

    CItem* item2 = GetItem(DestCell);

    // Do not allow Toggle items in the other inventory categories
    if (!DestCell.IsNormalInventoryPosition()) {
        if (item && item->GetItemType() == ITEM_TOGGLE)
            return false;
    }

    if (DestCell.IsEquipPosition()) {
        // 장비일 경우 한 곳만 검사해도 된다.
        if (item2) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "이미 장비를 착용하고 있습니다.");
            return false;
        }

        if (!IsEquipmentCell(item, DestCell.cell))
            return false;

        // We pass |item| as |except| here, since the user might decide
        // to move an equipped ring/unique item to a different ring/unique slot.
        if (!CanEquipItem(this, item, item))
            return false;

        item->RemoveFromCharacter();
        item->AddToCharacter(this, DestCell);
        return true;
    }

    if (DestCell.IsSwitchbotPosition()) {
        if (item2) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "슬롯이 이미 사용 중입니다");
            return false;
        }

        if (item->GetAttributeSetIndex() == -1) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "속성을 변경할 수 없는 "
                               "아이템입니다.");
            return false;
        }

        item->RemoveFromCharacter();
        item->AddToCharacter(this, DestCell);
        return true;
    }

    // 이미 착용중인 아이템을 다른 곳으로 옮기는 경우, '장책 해제' 가능한 지
    // 확인하고 옮김
    if (Cell.IsEquipPosition()) {
        if (!CanUnequipItem(this, item))
            return false;

        if (DestCell.window_type == EWindows::DRAGON_SOUL_INVENTORY) {
            if (!IsEmptyItemGridDS(DestCell, item->GetSize(), item->GetCell()))
                return UnequipItem(item);
        } else {
            if (!IsEmptyItemGrid(DestCell, item->GetSize(), item))
                return UnequipItem(item);
        }
    }

    if (item->IsDragonSoul()) {
        if (item->IsEquipped()) {
            return UnequipItem(item);
        }
        if (DestCell.window_type != DRAGON_SOUL_INVENTORY) {
            return false;
        }

        if (!DSManager::instance().IsValidCellForThisItem(item, DestCell))
            return false;
    }
    // 용혼석이 아닌 아이템은 용혼석 인벤에 들어갈 수 없다.
    else if (DRAGON_SOUL_INVENTORY == DestCell.window_type)
        return false;

    if (item2 && item2 != item && CanStack(item, item2)) {
        // 합칠 수 있는 아이템의 경우
        if (!CanModifyItem(item2))
            return false;

        SPDLOG_INFO("{0}: ITEM_STACK {1} ({2}:{3} count {7}) -> ({4}:{5}) "
                    "count {6}",
                    GetName(), item->GetName(), Cell.window_type, Cell.cell,
                    DestCell.window_type, DestCell.cell, count,
                    item->GetCount());

        Stack(item, item2, count);
        return true;
    }

    if (DestCell.window_type == EWindows::DRAGON_SOUL_INVENTORY) {
        if (!IsEmptyItemGridDS(DestCell, item->GetSize()) && count != 0 &&
            count != item->GetCount()) {
            {
                return SwapItems(Cell, DestCell, item);
            }
        }
    } else {
        if (!IsEmptyItemGrid(DestCell, item->GetSize(), item) && count != 0 &&
            count != item->GetCount())
        // It's not empty - Let's try swapping.
        {
            return SwapItems(Cell, DestCell, item);
        }
    }

    if (count == 0 || count == item->GetCount() || !item->IsStackable()) {
        if (DestCell.window_type == EWindows::DRAGON_SOUL_INVENTORY) {
            if (!IsEmptyItemGridDS(DestCell, item->GetSize(), item->GetCell()))
                return false;
        } else {
            if (!IsEmptyItemGrid(DestCell, item->GetSize(), item))
                return false;
        }

        SPDLOG_INFO("{0}: ITEM_MOVE {1} (window: {2}, cell : {3}) -> "
                    "(window:{4}, cell {5}) count {6}",
                    GetName(), item->GetName(), Cell.window_type, Cell.cell,
                    DestCell.window_type, DestCell.cell, count);

        item->RemoveFromCharacter();
        item->AddToCharacter(this, DestCell);

        if (INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
            SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, DestCell.cell);
    } else if (count < item->GetCount()) {
        if (DestCell.window_type == EWindows::DRAGON_SOUL_INVENTORY) {
            if (!IsEmptyItemGridDS(DestCell, item->GetSize()))
                return false;
        } else {
            if (!IsEmptyItemGrid(DestCell, item->GetSize()))
                return false;
        }

        if (count == 0 || count < 0)
            return false;

        SPDLOG_INFO("{0}: ITEM_SPLIT {1} (window: {2}, cell : {3}) -> "
                    "(window:{4}, cell {5}) count {6}",
                    GetName(), item->GetName(), Cell.window_type, Cell.cell,
                    DestCell.window_type, DestCell.cell, count);

        item->SetCount(item->GetCount() - count);
        auto itemNew =
            ITEM_MANAGER::instance().CreateItem(item->GetVnum(), count);

        FN_copy_item_socket(itemNew, item);

        itemNew->AddToCharacter(this, DestCell);

        LogManager::instance().ItemLog(
            this, item, "ITEM_SPLIT",
            fmt::format("{} {} {} {} ", itemNew->GetID(), itemNew->GetCount(),
                        item->GetCount(),
                        item->GetCount() + itemNew->GetCount())
                .c_str());
    }

    return true;
}

namespace NPartyPickupDistribute
{
struct FFindOwnership {
    uint32_t pid;
    CHARACTER* owner;

    FFindOwnership(uint32_t pid)
        : pid(pid)
        , owner(nullptr)
    {
        // ctor
    }

    void operator()(CHARACTER* ch)
    {
        if (ch->GetPlayerID() == pid)
            owner = ch;
    }
};

struct FCountNearMember {
    int total;
    int x, y;
    int32_t map;

    FCountNearMember(CHARACTER* center)
        : total(0)
        , x(center->GetX())
        , y(center->GetY())
        , map(center->GetMapIndex())
    {
    }

    void operator()(CHARACTER* ch)
    {
        if (map == ch->GetMapIndex() &&
            DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <=
                PARTY_DEFAULT_RANGE)
            total += 1;
    }
};

struct FMoneyDistributor {
    int total;
    CHARACTER* c;
    int x, y;
    int32_t map;
    Gold iMoney;

    FMoneyDistributor(CHARACTER* center, Gold iMoney)
        : total(0)
        , c(center)
        , x(center->GetX())
        , y(center->GetY())
        , map(center->GetMapIndex())
        , iMoney(iMoney)
    {
    }

    void operator()(CHARACTER* ch)
    {
        if (ch != c && map == ch->GetMapIndex() &&
            DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <=
                PARTY_DEFAULT_RANGE) {
            ch->ChangeGold(iMoney);
        }
    }
};
} // namespace NPartyPickupDistribute

void CHARACTER::GiveGold(Gold iAmount)
{
    if (iAmount <= 0)
        return;

    SPDLOG_INFO("GIVE_GOLD: {} {}", GetName(), iAmount);

#ifdef ENABLE_BATTLE_PASS
    uint8_t bBattlePassId = GetBattlePassId();
    if (bBattlePassId) {
        uint32_t dwYangCount, dwNotUsed;
        if (CBattlePass::instance().BattlePassMissionGetInfo(
                bBattlePassId, FARM_YANG, &dwNotUsed, &dwYangCount)) {
            if (GetMissionProgress(FARM_YANG, bBattlePassId) < dwYangCount)
                UpdateMissionProgress(FARM_YANG, bBattlePassId, iAmount,
                                      dwYangCount);
        }
    }
#endif

    if (!GetParty()) {
        ChangeGold(iAmount);
        return;
    }

    auto pParty = GetParty();

    // 파티가 있는 경우 나누어 가진다.
    Gold dwTotal = iAmount;
    Gold dwMyAmount = dwTotal;

    NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
    pParty->ForEachOnlineMember(funcCountNearMember);

    if (funcCountNearMember.total > 1) {
        Gold dwShare = dwTotal / funcCountNearMember.total;
        dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

        NPartyPickupDistribute::FMoneyDistributor funcMoneyDist(this, dwShare);

        pParty->ForEachOnlineMember(funcMoneyDist);
    }

    ChangeGold(dwMyAmount);
}

bool CHARACTER::PickupItem(uint32_t vid, uint32_t time)
{
    auto item = ITEM_MANAGER::instance().FindByVID(vid);
    if (!item || !item->GetSectree())
        return false;

#ifdef ENABLE_BATTLE_PASS
    bool bIsBattlePass = item->HaveOwnership();
#endif

    const auto dist =
        DISTANCE_APPROX(GetX() - item->GetX(), GetY() - item->GetY());

    if (dist >= 600)
        return false;

    item->SetLastOwnerPid(0);
    item->WasMine(false);

    const auto ownerPid = item->GetOwnerPid();
    CHARACTER* owner = nullptr;

    if (ownerPid == 0 || ownerPid == GetPlayerID()) {
        owner = this;
    } else {
        if (!GetParty() || IS_SET(item->GetAntiFlag(),
                                  ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_DROP))
            return false;

        NPartyPickupDistribute::FFindOwnership f(ownerPid);
        GetParty()->ForEachOnlineMember(f);

        if (!f.owner && GetParty()->IsMember(ownerPid))
            owner = this;
        else if (f.owner)
            owner = f.owner;
        else
            return false;
    }

    if (item->GetItemType() == ITEM_ELK) {
        owner->GiveGold(item->GetCount());

        item->RemoveFromGround();
        M2_DESTROY_ITEM(item);

        owner->Save();
        return true;
    }

    CountType count = item->GetCount();
    if (item->IsStackable()) {
        for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
            CItem* item2 = owner->GetInventoryItem(i);
            if (!item2)
                continue;

            if (!CanStack(item, item2))
                continue;

            CountType count2 = std::min<CountType>(
                GetItemMaxCount(item2) - item2->GetCount(), count);
            count -= count2;

            item2->SetCount(item2->GetCount() + count2);

            if (count == 0) {
                M2_DESTROY_ITEM(item);
                item = item2;
                break;
            }
        }

        // Only update the count if the original item isn't gone already.
        if (0 != count)
            item->SetCount(count);
    }

    if (0 != count) {
        if (item->IsDragonSoul()) {
            int iEmptyCell = owner->GetEmptyDragonSoulInventory(item);
            if (iEmptyCell == -1) {
                SendI18nChatPacket(owner, CHAT_TYPE_INFO,
                                   "소지하고 있는 아이템이 너무 많습니다.");
                return false;
            }

            item->RemoveFromGround();
            item->AddToCharacter(owner,
                                 TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
        } else {
            int iEmptyCell = owner->GetEmptyInventory(item);
            if (iEmptyCell == -1) {
                SPDLOG_DEBUG("No empty inventory pid {} size {} itemid {}",
                             owner->GetPlayerID(), item->GetSize(),
                             item->GetID());

                SendI18nChatPacket(owner, CHAT_TYPE_INFO,
                                   "소지하고 있는 아이템이 너무 많습니다.");
                return false;
            }

            item->RemoveFromGround();
            item->AddToCharacter(owner, TItemPos(INVENTORY, iEmptyCell));
        }
    }

#ifdef ENABLE_BATTLE_PASS
    if (bIsBattlePass) {
        uint8_t bBattlePassId = GetBattlePassId();
        if (bBattlePassId) {
            uint32_t dwItemVnum, dwCount;
            if (CBattlePass::instance().BattlePassMissionGetInfo(
                    bBattlePassId, COLLECT_ITEM, &dwItemVnum, &dwCount)) {
                if (dwItemVnum == item->GetVnum() &&
                    GetMissionProgress(COLLECT_ITEM, bBattlePassId) < dwCount)
                    UpdateMissionProgress(COLLECT_ITEM, bBattlePassId,
                                          item->GetCount(), dwCount);
            }
        }
    }
#endif

    LogManager::instance().ItemLog(owner, item, "GET",
                                   fmt::format("{} {} {}", item->GetName(),
                                               item->GetCount(),
                                               item->GetVnum())
                                       .c_str());

    if (owner == this) {
        SendItemPickupInfo(this, item->GetVnum(), item->GetCount());
    } else {
        SendI18nChatPacket(owner, CHAT_TYPE_INFO,
                           "아이템 획득: %s 님으로부터 %s", GetName(),
                           TextTag::itemname(item->GetVnum()).c_str());

        SendI18nChatPacket(this, CHAT_TYPE_INFO, "아이템 전달: %s 님에게 %s",
                           owner->GetName(),
                           TextTag::itemname(item->GetVnum()).c_str());
    }

    if (item->GetItemType() == ITEM_QUEST)
        quest::CQuestManager::instance().PickupItem(owner->GetPlayerID(), item);

    return true;
}

#define VERIFY_MSG(exp, msg)                                                   \
    if (true == (exp)) {                                                       \
        ChatPacket(CHAT_TYPE_INFO, LC_TEXT(msg));                              \
        return false;                                                          \
    }

bool CHARACTER::SwapItem(TItemPos cell, TItemPos destCell)
{
    if (!CanHandleItem())
        return false;

    // 같은 CELL 인지 검사
    if (cell.cell == destCell.cell)
        return false;

    CItem* item1 = GetItem(cell);
    CItem* item2 = GetItem(destCell);

    if (!item1 || !item2)
        return false;

    if (item1 == item2) {
        SPDLOG_ERROR("SWAP_DUPLICATE_ITEM name {0} src {1} dst {2}", m_stName,
                     cell.cell, destCell.cell);
        return false;
    }

    const auto item1Pos = item1->GetPosition();
    const auto item2Pos = item2->GetPosition();

    if (!CanModifyItem(item1) || !CanModifyItem(item2))
        return false;

    if (!CanRemove(this, item1) || !CanRemove(this, item2))
        return false;

    if (!CanAdd(this, item1, item2Pos, item2) ||
        !CanAdd(this, item2, item1Pos, item1))
        return false;

    item1->RemoveFromCharacter();
    item2->RemoveFromCharacter();

    item1->AddToCharacter(this, item2Pos);
    item2->AddToCharacter(this, item1Pos);
    return true;
}

bool CHARACTER::UnequipItem(CItem* item, bool unequipFromPlayer /*= false*/)
{
    if (!CanUnequipItem(this, item))
        return false;

    int pos;
    if (item->IsDragonSoul())
        pos = GetEmptyDragonSoulInventory(item);
    else
        pos = GetEmptyInventory(item);

    if (-1 == pos)
        return false;

    item->RemoveFromCharacter();
    if (item->IsDragonSoul()) {
        item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, pos));
    } else
        item->AddToCharacter(this, TItemPos(INVENTORY, pos));

    CheckMaximumPoints();
#ifdef __FAKE_PC__
    FakePC_Owner_ExecFunc(&CHARACTER::CheckMaximumPoints);
#endif
    return true;
}

//
// @version	05/07/05 Bang2ni - Skill 사용후 1.5 초 이내에 장비 착용 금지
//
bool CHARACTER::EquipItem(CItem* item, int iCandidateCell)
{
    if (!CanModifyItem(item) || !CanEquipItem(this, item))
        return false;

    int iWearCell = FindEquipmentCell(this, item);
    if (iWearCell < 0)
        return false;

    // 착용할 곳에 아이템이 있다면,
    auto oldWear = GetWear(iWearCell);
    if (oldWear) {
        STORM_ASSERT(item != oldWear, "Sanity");

        if (item == oldWear) {
            SPDLOG_ERROR("EquipItem: Duplicate for {0}:{1} / {2}:{3}",
                         item->GetWindow(), item->GetCell(), EQUIPMENT,
                         iWearCell);
            return false;
        }

        if (!CanUnequipItem(this, oldWear))
            return false;

        auto oldWearSwapPos = item->GetPosition();
        if (item->IsDragonSoul()) {
            if (!IsEmptyItemGridDS(oldWearSwapPos, oldWear->GetSize(),
                                   item->GetCell())) {
                int iEmptyCell = GetEmptyDragonSoulInventory(oldWear);
                if (iEmptyCell == -1)
                    return false;

                oldWearSwapPos = TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell);
            }
        } else {
            if (!IsEmptyItemGrid(oldWearSwapPos, oldWear->GetSize(), item)) {
                int iEmptyCell = GetEmptyInventory(oldWear);
                if (iEmptyCell == -1)
                    return false;

                oldWearSwapPos = TItemPos(INVENTORY, iEmptyCell);
            }
        }

        item->RemoveFromCharacter();
        oldWear->RemoveFromCharacter();

        SetWear(iWearCell, item);
        oldWear->AddToCharacter(this, oldWearSwapPos);
    } else {
        uint16_t bOldCell = item->GetCell();

        item->RemoveFromCharacter();

        SetWear(iWearCell, item);
        SyncQuickslot(QUICKSLOT_TYPE_ITEM, bOldCell, iWearCell);
    }

    // UpdateEquipSkillUsePulse();
    return true;

    /*
     *
#if defined(WJ_COMBAT_ZONE)
    if (!CCombatZoneManager::instance().CanUseItem(this, item))
        return false;
#endif

    if (iWearCell == WEAR_WEAPON) {
        if (item->GetItemType() == ITEM_WEAPON) {
            CItem* costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
            if (costumeWeapon &&
                costumeWeapon->GetValue(3) != item->GetSubType() &&
                !UnequipItem(costumeWeapon)) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot unequip the costume weapon. Not "
                                   "enough space.");
                return false;
            }
        } else {
            CItem* costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
            if (costumeWeapon && !UnequipItem(costumeWeapon)) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot unequip the costume weapon. Not "
                                   "enough space.");
                return false;
            }
        }
    } else if (iWearCell == WEAR_COSTUME_WEAPON) {
        if (item->GetItemType() == ITEM_COSTUME &&
            item->GetSubType() == COSTUME_WEAPON) {
            CItem* pkWeapon = GetWear(WEAR_WEAPON);
            if (!pkWeapon || pkWeapon->GetItemType() != ITEM_WEAPON ||
                item->GetValue(3) != pkWeapon->GetSubType()) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "You cannot equip the costume weapon. Wrong "
                                   "equipped weapon.");
                return false;
            }
        }
    }
    // 용혼석 특수 처리
    if (item->IsDragonSoul()) {
        // 같은 타입의 용혼석이 이미 들어가 있다면 착용할 수 없다.
        // 용혼석은 swap을 지원하면 안됨.
        if (GetInventoryItem(INVENTORY_MAX_NUM + iWearCell)) {
            ChatPacket(CHAT_TYPE_INFO, "이미 같은 종류의 용혼석을 착용하고 "
                                       "있습니다.");
            return false;
        }

        if (!item->EquipTo(this, iWearCell)) {
            return false;
        }
    }
    // 용혼석이 아님.
    else {
        // 착용할 곳에 아이템이 있다면,
        if (GetWear(iWearCell) &&
            !IS_SET(GetWear(iWearCell)->GetFlag(), ITEM_FLAG_IRREMOVABLE)) {
            // 이 아이템은 한번 박히면 변경 불가. swap 역시 완전 불가
            if (item->GetWearFlag() == WEARABLE_ABILITY)
                return false;

            if (!SwapItem((uint16_t)item->GetCell(),
                          (uint16_t)(INVENTORY_MAX_NUM + iWearCell))) {
                return false;
            }
        } else {
            uint8_t bOldCell = (uint8_t)item->GetCell();

            if (item->EquipTo(this, iWearCell)) {
                SyncQuickslot(QUICKSLOT_TYPE_ITEM, bOldCell, iWearCell);
            }
        }
    }

    if (true == item->IsEquipped()) {
        // 아이템 최초 사용 이후부터는 사용하지 않아도 시간이 차감되는 방식
        // 처리.
        if (-1 != item->GetProto()->cLimitRealTimeFirstUseIndex) {
            // 한 번이라도 사용한 아이템인지 여부는 Socket1을 보고 판단한다.
            // (Socket1에 사용횟수 기록)
            if (0 == item->GetSocket(1)) {
                // 사용가능시간은 Default 값으로 Limit Value 값을 사용하되,
                // Socket0에 값이 있으면 그 값을 사용하도록 한다. (단위는 초)
                long duration =
                    (0 != item->GetSocket(0))
                        ? item->GetSocket(0)
                        : item->GetProto()
                              ->aLimits[(uint8_t)item->GetProto()
                                            ->cLimitRealTimeFirstUseIndex]
                              .lValue;

                if (0 == duration)
                    duration = 60 * 60 * 24 * 7;

                item->SetSocket(0, time(nullptr) + duration);
                item->StartRealTimeExpireEvent();
            }

            item->SetSocket(1, item->GetSocket(1) + 1);
        }

        if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
            ShowAlignment(false);
    }

    UpdateEquipPulse();
    UpdateEquipSkillUsePulse();
    return true;	 */
}

void CHARACTER::BuffOnAttr_AddBuffsFromItem(CItem* pItem)
{
    unsigned int sizeOf =
        sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]);
    for (unsigned int i = 0; i < sizeOf; i++) {
        TMapBuffOnAttrs::iterator it =
            m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
        if (it != m_map_buff_on_attrs.end()) {
            it->second->AddBuffFromItem(pItem);
        }
    }
}

void CHARACTER::BuffOnAttr_RemoveBuffsFromItem(CItem* pItem)
{
    unsigned int sizeOf =
        sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]);
    for (unsigned int i = 0; i < sizeOf; i++) {
        auto it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
        if (it != m_map_buff_on_attrs.end()) {
            it->second->RemoveBuffFromItem(pItem);
        }
    }
}

void CHARACTER::BuffOnAttr_ClearAll()
{
    for (auto& [attr, buff] : m_map_buff_on_attrs) {
        if (buff) {
            buff->Initialize();
        }
    }
}

void CHARACTER::BuffOnAttr_ValueChange(uint8_t bType, uint8_t bOldValue,
                                       uint8_t bNewValue)
{
    auto it = m_map_buff_on_attrs.find(bType);

    if (0 == bNewValue) {
        if (m_map_buff_on_attrs.end() == it)
            return;
        it->second->Off();
        return;
    }

    if (0 != bOldValue) {
        if (m_map_buff_on_attrs.end() == it)
            return;

        it->second->ChangeBuffValue(bNewValue);
        return;
    }

    std::unique_ptr<CBuffOnAttributes> pBuff;
    if (m_map_buff_on_attrs.end() == it) {
        switch (bType) {
                /*case POINT_ENERGY: {
                        static uint8_t abSlot[] = {WEAR_BODY,  WEAR_HEAD,
                   WEAR_FOOTS, WEAR_WRIST, WEAR_WEAPON, WEAR_NECK, WEAR_EAR,
                   WEAR_SHIELD}; static std::vector<uint8_t> vec_slots( abSlot,
                   abSlot + _countof(abSlot)); pBuff = new
                   CBuffOnAttributes(this, bType, &vec_slots); } break;*/
            case POINT_COSTUME_ATTR_BONUS: {
                static uint8_t abSlot[] = {WEAR_COSTUME_BODY, WEAR_COSTUME_HAIR,
                                           WEAR_COSTUME_WEAPON};
                static std::vector<uint8_t> vec_slots(
                    abSlot, abSlot + _countof(abSlot));
                pBuff = std::make_unique<CBuffOnAttributes>(this, bType,
                                                            &vec_slots);
            } break;
            default:
                break;
        }
        auto [it, success] =
            m_map_buff_on_attrs.emplace(bType, std::move(pBuff));
        if (success) {
            it->second->On(bNewValue);
        }
        return;
    }

    it->second->On(bNewValue);
}

CItem* CHARACTER::FindArrow() const
{
    for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
        if (GetInventoryItem(i) &&
            GetInventoryItem(i)->GetItemType() == ITEM_WEAPON &&
            GetInventoryItem(i)->GetSubType() == WEAPON_ARROW)
            return GetInventoryItem(i);

    return nullptr;
}

CItem* CHARACTER::GetSwitcherByAttributeIndex(int setIndex, bool rare) const
{
    switch (setIndex) {
        case ATTRIBUTE_SET_WEAPON:
        case ATTRIBUTE_SET_BODY:
        case ATTRIBUTE_SET_WRIST:
        case ATTRIBUTE_SET_FOOTS:
        case ATTRIBUTE_SET_NECK:
        case ATTRIBUTE_SET_HEAD:
        case ATTRIBUTE_SET_SHIELD:
        case ATTRIBUTE_SET_EAR: {
            if (rare) {
                return FindSpecifyTypeItem(ITEM_USE, USE_CHANGE_ATTRIBUTE_RARE);
            }

            if (auto* item =
                    FindSpecifyTypeItem(ITEM_USE, USE_CHANGE_ATTRIBUTE_PERM);
                item)
                return item;

            if (auto* item =
                    FindSpecifyTypeItem(ITEM_USE, USE_CHANGE_ATTRIBUTE);
                item)
                return item;
        } break;

        case ATTRIBUTE_SET_COSTUME_HAIR:
        case ATTRIBUTE_SET_COSTUME_BODY:
        case ATTRIBUTE_SET_COSTUME_WEAPON:
            if (rare)
                return nullptr;

            return FindSpecifyTypeItem(ITEM_USE, USE_CHANGE_COSTUME_ATTR);

        default:
            return nullptr;
    }

    return nullptr;
}

CItem* CHARACTER::FindQuiver() const
{
    for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
        if (GetInventoryItem(i) &&
            GetInventoryItem(i)->GetItemType() == ITEM_WEAPON &&
            GetInventoryItem(i)->GetSubType() == WEAPON_QUIVER)
            return GetInventoryItem(i);

    return nullptr;
}

CItem* CHARACTER::FindSpecifyItem(uint32_t vnum) const
{
    for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
        if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum)
            return GetInventoryItem(i);

    return nullptr;
}

CItem* CHARACTER::FindItemByID(uint32_t id) const
{
    for (int i = 0; i < m_windows.inventory.GetSize(); ++i) {
        if (nullptr != GetInventoryItem(i) &&
            GetInventoryItem(i)->GetID() == id)
            return GetInventoryItem(i);
    }

    for (int i = 0; i < m_windows.belt.GetSize(); ++i) {
        CItem* item = GetItem(TItemPos(BELT_INVENTORY, i));
        if (item && item->GetID() == id)
            return item;
    }

    return nullptr;
}

int CHARACTER::CountSpecifyItem(uint32_t vnum, CItem* except) const
{
    CountType count = 0;

    for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
        CItem* item = GetInventoryItem(i);
        if(except && except == item)
            continue;
        if (item && item->GetVnum() == vnum) {
            count += item->GetCount();
        }
    }

    return count;
}

void CHARACTER::RemoveSpecifyItem(uint32_t vnum, CountType count, CItem* except)
{
    if (0 == count)
        return;

    for (UINT i = 0; i < INVENTORY_MAX_NUM; ++i) {
        if (nullptr == GetInventoryItem(i))
            continue;

        if(except && except == GetInventoryItem(i))
            continue;

        if (GetInventoryItem(i)->GetVnum() != vnum)
            continue;

        if ((vnum >= 80003 && vnum <= 80007) ||
            (vnum >= 80018 && vnum <= 80020))
            LogManager::instance().GoldBarLog(GetPlayerID(),
                                              GetInventoryItem(i)->GetID(),
                                              QUEST, "RemoveSpecifyItem");

        if (count >= GetInventoryItem(i)->GetCount()) {
            count -= GetInventoryItem(i)->GetCount();
            GetInventoryItem(i)->SetCount(0);

            if (0 == count)
                return;
        } else {
            GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() -
                                          count);
            return;
        }
    }

    // 예외처리가 약하다.
    if (count)
        SPDLOG_INFO("CHARACTER::RemoveSpecifyItem cannot remove enough item "
                    "vnum "
                    "%u, still remain %d",
                    vnum, count);
}

int CHARACTER::CountSpecifyTypeItem(uint8_t type, int8_t subtype) const
{
    CountType count = 0;

    for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
        CItem* pItem = GetInventoryItem(i);

        if (subtype != -1) {
            if (pItem != nullptr && pItem->GetItemType() == type &&
                pItem->GetSubType() == subtype) {
                count += pItem->GetCount();
            }
        } else {
            if (pItem != nullptr && pItem->GetItemType() == type) {
                count += pItem->GetCount();
            }
        }
    }

    return count;
}

CItem* CHARACTER::FindSpecifyTypeItem(uint8_t type, int8_t subtype) const
{
    for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
        CItem* pItem = GetInventoryItem(i);

        if (subtype != -1) {
            if (pItem != nullptr && pItem->GetItemType() == type &&
                pItem->GetSubType() == subtype) {
                return pItem;
            }
        } else {
            if (pItem != nullptr && pItem->GetItemType() == type) {
                return pItem;
            }
        }
    }

    return nullptr;
}

void CHARACTER::RemoveSpecifyTypeItem(uint8_t type, int8_t subtype,
                                      CountType count)
{
    if (0 == count)
        return;

    for (UINT i = 0; i < INVENTORY_MAX_NUM; ++i) {
        auto item = GetInventoryItem(i);
        if (!item)
            continue;

        if (subtype != -1) {
            if (item->GetItemType() != type && item->GetSubType() != subtype)
                continue;
        } else {
            if (item->GetItemType() != type)
                continue;
        }

        if (count >= item->GetCount()) {
            count -= item->GetCount();
            item->SetCount(0);

            if (0 == count)
                return;
        } else {
            item->SetCount(item->GetCount() - count);
            return;
        }
    }
}

CItem* CHARACTER::AutoGiveItem(CItem* item, bool longOwnerShip, bool allowDrop)
{
    if (!item)
        return nullptr;

    if (item->GetOwner()) {
        SPDLOG_ERROR("item {}'s owner exists!", item->GetID());
        return nullptr;
    }

    bool bGiven = false;

    CountType count = item->GetCount();
    if (item->IsStackable()) {
        for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
            CItem* item2 = GetInventoryItem(i);
            if (!item2)
                continue;

            if (!CanStack(item, item2))
                continue;

            CountType count2 = std::min<CountType>(
                GetItemMaxCount(item2) - item2->GetCount(), count);
            count -= count2;

            item2->SetCount(item2->GetCount() + count2);

            if (count == 0) {
                M2_DESTROY_ITEM(item);
                item = item2;
                bGiven = true;
                break;
            }
        }

        // Only update the count if the original item isn't gone already.
        if (0 != count)
            item->SetCount(count);
    }

    if (!bGiven) {
        int cell = item->IsDragonSoul() ? GetEmptyDragonSoulInventory(item)
                                        : GetEmptyInventory(item);

        if (cell != -1) {
            if (item->IsDragonSoul()) {
                item->AddToCharacter(this,
                                     TItemPos(DRAGON_SOUL_INVENTORY, cell));
            } else {
                item->AddToCharacter(this, TItemPos(INVENTORY, cell));
            }

            bGiven = true;
        }
    }

    if (bGiven) {
        LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());
    } else if (allowDrop) {
        item->RemoveFromCharacter();
        item->AddToGround(GetMapIndex(), GetXYZ());

        if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP) || longOwnerShip) {
            item->StartDestroyEvent(gConfig.itemGroundTimeLong);
            item->SetOwnership(this, gConfig.itemOwnershipTimeLong);
        }
        else {
            item->StartDestroyEvent(gConfig.itemGroundTime);
            item->SetOwnership(this, gConfig.itemOwnershipTime);
        }

        LogManager::instance().ItemLog(this, item, "SYSTEM_DROP",
                                       item->GetName());
    }

    return item;
}

CItem* CHARACTER::AutoGiveItem(uint32_t dwItemVnum, CountType count,
                               int iRarePct, bool bMsg)
{
    const auto* p = ITEM_MANAGER::instance().GetTable(dwItemVnum);
    if (!p)
        return nullptr;

    if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT)) {
        if (count < p->alValues[1])
            count = p->alValues[1];
    }

    DBManager::instance().SendMoneyLog(MONEY_LOG_DROP, dwItemVnum, count);

    auto* item = ITEM_MANAGER::instance().CreateItem(dwItemVnum, count, 0, true);
    if (!item) {
        SPDLOG_ERROR("AutoGiveItem: Cannot create item by vnum {} (name: {})",
                     dwItemVnum, GetName());
        return nullptr;
    }

    if (bMsg)
        SendItemPickupInfo(this, item->GetVnum(), item->GetCount());

    if (item->IsStackable()) {
        for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
            auto* item2 = GetInventoryItem(i);
            if (!item2)
                continue;

            if (!CanStack(item, item2) || !CanModifyItem(item2))
                continue;

            const auto count2 = std::min<CountType>(
                GetItemMaxCount(item2) - item2->GetCount(), count);
            count -= count2;

            item2->SetCount(item2->GetCount() + count2);

            if (item2->GetItemType() == ITEM_QUEST)
                quest::CQuestManager::instance().PickupItem(GetPlayerID(),
                                                            item2);

            if (count == 0) {
                M2_DESTROY_ITEM(item);
                return item2;
            }
        }
    }

    item->SetCount(count);

    int iEmptyCell;
    if (item->IsDragonSoul()) {
        iEmptyCell = GetEmptyDragonSoulInventory(item);
    } else
        iEmptyCell = GetEmptyInventory(item);

    if (iEmptyCell != -1) {
        if (item->IsDragonSoul())
            item->AddToCharacter(this,
                                 TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
        else
            item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell));

        LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());

        if (item->GetItemType() == ITEM_USE &&
            item->GetSubType() == USE_POTION) {
            TQuickslot* pSlot;

            if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE) {
                TQuickslot slot;
                slot.type = QUICKSLOT_TYPE_ITEM;
                slot.pos = iEmptyCell;
                SetQuickslot(0, slot);
            }
        }

        if (item->GetItemType() == ITEM_QUEST)
            quest::CQuestManager::instance().PickupItem(GetPlayerID(), item);
    } else {
        item->RemoveFromCharacter();
        item->AddToGround(GetMapIndex(), GetXYZ());

        if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP)) {
            item->StartDestroyEvent(gConfig.itemGroundTimeLong);
            item->SetOwnership(this, gConfig.itemOwnershipTimeLong);
        }
        else {
            item->StartDestroyEvent(gConfig.itemGroundTime);
            item->SetOwnership(this, gConfig.itemOwnershipTime);
        }

        LogManager::instance().ItemLog(this, item, "SYSTEM_DROP",
                                       item->GetName());
    }

    return item;
}

bool CHARACTER::GiveItem(CHARACTER* victim, const TItemPos& Cell)
{
    if (!CanHandleItem())
        return false;

    CItem* item = GetItem(Cell);

    if (item && !item->IsExchanging()) {
        if (victim->CanReceiveItem(this, item)) {
            victim->ReceiveItem(this, item);
            return true;
        }
    }

    return false;
}

bool CHARACTER::CanReceiveItem(CHARACTER* from, CItem* item,
                               bool ignoreDist /*= false*/) const
{
    if (IsPC())
        return false;

    // TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX
    if (!ignoreDist &&
        DISTANCE_APPROX(GetX() - from->GetX(), GetY() - from->GetY()) > 2000)
        return false;
    // END_OF_TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX

    switch (GetRaceNum()) {
        case fishing::CAMPFIRE_MOB:
            if (item->GetItemType() == ITEM_FISH &&
                (item->GetSubType() == FISH_ALIVE ||
                 item->GetSubType() == FISH_DEAD))
                return true;
            break;

        case fishing::FISHER_MOB:
            if (item->GetItemType() == ITEM_ROD)
                return true;
            break;

            // BUILDING_NPC
        case BLACKSMITH_WEAPON_MOB:
        case DEVILTOWER_BLACKSMITH_WEAPON_MOB: {
            if (item->GetItemType() == ITEM_WEAPON && item->GetRefinedVnum())
                return true;
            return false;
        } break;

        case BLACKSMITH_ARMOR_MOB:
        case DEVILTOWER_BLACKSMITH_ARMOR_MOB: {
            if (item->GetItemType() == ITEM_ARMOR &&
                (item->GetSubType() == ARMOR_BODY ||
                 item->GetSubType() == ARMOR_SHIELD ||
                 item->GetSubType() == ARMOR_HEAD) &&
                item->GetRefinedVnum())
                return true;
            return false;
        } break;

        case BLACKSMITH_ACCESSORY_MOB:
        case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB: {
            if (item->GetItemType() == ITEM_ARMOR &&
                !(item->GetSubType() == ARMOR_BODY ||
                  item->GetSubType() == ARMOR_SHIELD ||
                  item->GetSubType() == ARMOR_HEAD) &&
                item->GetRefinedVnum())
                return true;
            return false;
        } break;
            // END_OF_BUILDING_NPC

        case BLACKSMITH_MOB: {
            if (item->GetRefinedVnum() && item->GetRefineSet() < 500) {
                return true;
            }
            return false;
        }

        case BLACKSMITH2_MOB: {
            if (item->GetRefineSet() >= 500) {
                return true;
            }
            return false;
        }

        case ALCHEMIST_MOB:
            if (item->GetRefinedVnum())
                return true;
            break;

        case 20101:
        case 20102:
        case 20103:
            // 초급 말
            {
                // 초급 말
                if (item->GetVnum() == ITEM_REVIVE_HORSE_1) {
                    if (!IsDead()) {
                        SendI18nChatPacket(from, CHAT_TYPE_INFO,
                                           "죽지 않은 말에게 선초를 먹일 수 "
                                           "없습니다.");
                        return false;
                    }
                    return true;
                }
                if (item->GetVnum() == ITEM_HORSE_FOOD_1) {
                    if (IsDead()) {
                        SendI18nChatPacket(from, CHAT_TYPE_INFO,
                                           "죽은 말에게 사료를 먹일 수 "
                                           "없습니다.");
                        return false;
                    }
                    return true;
                }
                if (item->GetVnum() == ITEM_HORSE_FOOD_2 ||
                    item->GetVnum() == ITEM_HORSE_FOOD_3) {
                    return false;
                }
            }
            break;
        case 20104:
        case 20105:
        case 20106:
            // 중급 말
            {
                // 중급 말
                if (item->GetVnum() == ITEM_REVIVE_HORSE_2) {
                    if (!IsDead()) {
                        SendI18nChatPacket(from, CHAT_TYPE_INFO,
                                           "죽지 않은 말에게 선초를 먹일 수 "
                                           "없습니다.");
                        return false;
                    }
                    return true;
                }
                if (item->GetVnum() == ITEM_HORSE_FOOD_2) {
                    if (IsDead()) {
                        SendI18nChatPacket(from, CHAT_TYPE_INFO,
                                           "죽은 말에게 사료를 먹일 수 "
                                           "없습니다.");
                        return false;
                    }
                    return true;
                }
                if (item->GetVnum() == ITEM_HORSE_FOOD_1 ||
                    item->GetVnum() == ITEM_HORSE_FOOD_3) {
                    return false;
                }
            }
            break;
        case 20107:
        case 20108:
        case 20109:
            // 고급 말
            {
                // 고급 말
                if (item->GetVnum() == ITEM_REVIVE_HORSE_3) {
                    if (!IsDead()) {
                        SendI18nChatPacket(from, CHAT_TYPE_INFO,
                                           "죽지 않은 말에게 선초를 먹일 수 "
                                           "없습니다.");
                        return false;
                    }
                    return true;
                }
                if (item->GetVnum() == ITEM_HORSE_FOOD_3) {
                    if (IsDead()) {
                        SendI18nChatPacket(from, CHAT_TYPE_INFO,
                                           "죽은 말에게 사료를 먹일 수 "
                                           "없습니다.");
                        return false;
                    }
                    return true;
                }
                if (item->GetVnum() == ITEM_HORSE_FOOD_1 ||
                    item->GetVnum() == ITEM_HORSE_FOOD_2) {
                    return false;
                }
            }
            break;
    }

    // if (IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_GIVE))
    {
        return true;
    }

    return false;
}

void CHARACTER::ReceiveItem(CHARACTER* from, CItem* item)
{
    if (IsPC())
        return;

    switch (GetRaceNum()) {
#ifdef ENABLE_MELEY_LAIR_DUNGEON
        case MeleyLair::STATUE_VNUM: {
            if (MeleyLair::CMgr::instance().IsMeleyMap(from->GetMapIndex()))
                MeleyLair::CMgr::instance().OnKillStatue(item, from, this);
        } break;
#endif
        case fishing::CAMPFIRE_MOB:
            if (item->GetItemType() == ITEM_FISH &&
                (item->GetSubType() == FISH_ALIVE ||
                 item->GetSubType() == FISH_DEAD))
                fishing::Grill(from, item);
            else {
                // TAKE_ITEM_BUG_FIX
                from->SetQuestNPCID(GetVID());
                // END_OF_TAKE_ITEM_BUG_FIX
                quest::CQuestManager::instance().TakeItem(from->GetPlayerID(),
                                                          GetRaceNum(), item);
            }
            break;

        default:
            SPDLOG_DEBUG("TakeItem %s %d %s", from->GetName(), GetRaceNum(),
                         item->GetName());
            from->SetQuestNPCID(GetVID());
            quest::CQuestManager::instance().TakeItem(from->GetPlayerID(),
                                                      GetRaceNum(), item);
            break;
    }
}

bool CHARACTER::IsEquipUniqueItem(uint32_t dwItemVnum) const
{
    {
        CItem* u = GetWear(WEAR_UNIQUE1);

        if (u && u->GetVnum() == dwItemVnum)
            return true;
    }
    {
        CItem* u = GetWear(WEAR_UNIQUE2);

        if (u && u->GetVnum() == dwItemVnum)
            return true;
    }

    return false;
}

// CHECK_UNIQUE_GROUP
bool CHARACTER::IsEquipUniqueGroup(uint32_t dwGroupVnum) const
{
    {
        CItem* u = GetWear(WEAR_UNIQUE1);

        if (u && u->GetSpecialGroup() == (int)dwGroupVnum)
            return true;
    }
    {
        CItem* u = GetWear(WEAR_UNIQUE2);

        if (u && u->GetSpecialGroup() == (int)dwGroupVnum)
            return true;
    }

    return false;
}

// END_OF_CHECK_UNIQUE_GROUP

void CHARACTER::SetRefineMode(int iAdditionalCell)
{
    m_iRefineAdditionalCell = iAdditionalCell;
    m_bUnderRefine = true;
}

void CHARACTER::ClearRefineMode()
{
    m_bUnderRefine = false;
    SetRefineNPC(nullptr);
}

bool CHARACTER::GiveItemFromSpecialItemGroup(
    uint32_t dwGroupNum, std::vector<uint32_t>& dwItemVnums,
    std::vector<uint32_t>& dwItemCounts, std::vector<CItem*>& item_gets,
    int& count)
{
    const CSpecialItemGroup* pGroup =
        ITEM_MANAGER::instance().GetSpecialItemGroup(dwGroupNum);

    if (!pGroup) {
        SPDLOG_ERROR("cannot find special item group {}", dwGroupNum);
        return false;
    }

    if (pGroup->GetType() == CSpecialItemGroup::RAND) {
        auto [vnum, count, rarePct] = pGroup->GetRandom();

        SPDLOG_INFO("Opened random group got {0}, {1}, {2}", vnum, count,
                    rarePct);

        bool bSuccess = false;

        switch (vnum) {
            case CSpecialItemGroup::GOLD:
                ChangeGold(static_cast<Gold>(count));
                LogManager::instance().CharLog(this, count, "TREASURE_GOLD",
                                               "");

                bSuccess = true;
                break;
            case CSpecialItemGroup::EXP: {
                PointChange(POINT_EXP, count);
                LogManager::instance().CharLog(this, count, "TREASURE_EXP", "");
                bSuccess = true;
            } break;

            case CSpecialItemGroup::MOB: {
                SPDLOG_INFO("CSpecialItemGroup::MOB %d", count);
                int x = GetX() + Random::get(-500, 500);
                int y = GetY() + Random::get(-500, 500);

                CHARACTER* ch = g_pCharManager->SpawnMob(count, GetMapIndex(),
                                                         x, y, 0, true, -1);
                if (ch)
                    ch->SetAggressive();
                bSuccess = true;
            } break;
            case CSpecialItemGroup::SLOW: {
                SPDLOG_INFO("CSpecialItemGroup::SLOW %d", -(int)count);
                AddAffect(AFFECT_SLOW, POINT_MOV_SPEED, -(int)count, 300, 0,
                          true);
                bSuccess = true;
            } break;
            case CSpecialItemGroup::DRAIN_HP: {
                int64_t iDropHP = GetMaxHP() * count / 100;
                SPDLOG_INFO("CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
                iDropHP = std::min(iDropHP, GetHP() - 1);
                SPDLOG_INFO("CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
                PointChange(POINT_HP, -iDropHP);
                bSuccess = true;
            } break;
            case CSpecialItemGroup::POISON: {
                AttackedByPoison(nullptr);
                bSuccess = true;
            } break;

            case CSpecialItemGroup::MOB_GROUP: {
                int sx = GetX() - Random::get(300, 500);
                int sy = GetY() - Random::get(300, 500);
                int ex = GetX() + Random::get(300, 500);
                int ey = GetY() + Random::get(300, 500);
                g_pCharManager->SpawnGroup(count, GetMapIndex(), sx, sy, ex, ey,
                                           nullptr, true);

                bSuccess = true;
            } break;
            case CSpecialItemGroup::POLY_MARBLE: {
                auto* item_get = ITEM_MANAGER::instance().CreateItem(70104, 1);

                if (item_get) {
                    item_get->SetSocket(0, count, false);
                    item_get = AutoGiveItem(item_get, true);
                    if (item_get)
                        bSuccess = true;
                }
            } break;
            default: {
                auto* item_get = AutoGiveItem(vnum, count, rarePct);

                if (item_get) {
                    item_gets.push_back(item_get);
                    bSuccess = true;
                }
            } break;
        }

        dwItemVnums.push_back(vnum);
        dwItemCounts.push_back(count);

        return bSuccess;
    }

    std::vector<int> idxes;
    int n = pGroup->GetMultiIndex(idxes);

    bool bSuccess = false;

    for (int i = 0; i < n; i++) {
        bSuccess = false;
        int idx = idxes[i];
        uint32_t dwVnum = pGroup->GetVnum(idx);
        SocketValue dwCount = pGroup->GetCount(idx);
        int iRarePct = pGroup->GetRarePct(idx);
        CItem* item_get = nullptr;
        switch (dwVnum) {
            case CSpecialItemGroup::GOLD:
                ChangeGold(static_cast<Gold>(dwCount));
                LogManager::instance().CharLog(this, dwCount, "TREASURE_GOLD",
                                               "");

                bSuccess = true;
                break;
            case CSpecialItemGroup::EXP: {
                PointChange(POINT_EXP, dwCount);
                LogManager::instance().CharLog(this, dwCount, "TREASURE_EXP",
                                               "");
                bSuccess = true;
            } break;

            case CSpecialItemGroup::MOB: {
                SPDLOG_INFO("CSpecialItemGroup::MOB %d", dwCount);
                int x = GetX() + Random::get(-500, 500);
                int y = GetY() + Random::get(-500, 500);

                CHARACTER* ch = g_pCharManager->SpawnMob(dwCount, GetMapIndex(),
                                                         x, y, 0, true, -1);
                if (ch)
                    ch->SetAggressive();
                bSuccess = true;
            } break;
            case CSpecialItemGroup::SLOW: {
                SPDLOG_INFO("CSpecialItemGroup::SLOW %d", -(int)dwCount);
                AddAffect(AFFECT_SLOW, POINT_MOV_SPEED, -(int)dwCount, 300, 0,
                          true);
                bSuccess = true;
            } break;
            case CSpecialItemGroup::DRAIN_HP: {
                int64_t iDropHP = GetMaxHP() * dwCount / 100;
                SPDLOG_INFO("CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
                iDropHP = std::min(iDropHP, GetHP() - 1);
                SPDLOG_INFO("CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
                PointChange(POINT_HP, -iDropHP);
                bSuccess = true;
            } break;
            case CSpecialItemGroup::POISON: {
                AttackedByPoison(nullptr);
                bSuccess = true;
            } break;

            case CSpecialItemGroup::MOB_GROUP: {
                int sx = GetX() - Random::get(300, 500);
                int sy = GetY() - Random::get(300, 500);
                int ex = GetX() + Random::get(300, 500);
                int ey = GetY() + Random::get(300, 500);
                g_pCharManager->SpawnGroup(dwCount, GetMapIndex(), sx, sy, ex,
                                           ey, nullptr, true);

                bSuccess = true;
            } break;
            case CSpecialItemGroup::POLY_MARBLE: {
                item_get = ITEM_MANAGER::instance().CreateItem(70104, 1);

                if (item_get) {
                    item_get->SetSocket(0, dwCount, false);
                    item_get = AutoGiveItem(item_get, true);
                    if (item_get)
                        bSuccess = true;
                }
            } break;
            default: {
                item_get = AutoGiveItem(dwVnum, dwCount, iRarePct);

                if (item_get) {
                    bSuccess = true;
                }
            } break;
        }

        if (bSuccess) {
            dwItemVnums.push_back(dwVnum);
            dwItemCounts.push_back(dwCount);
            item_gets.push_back(item_get);
            count++;
        } else {
            return false;
        }
    }
    return bSuccess;
}

bool CHARACTER::ItemProcess_Polymorph(CItem* item)
{
#if defined(WJ_COMBAT_ZONE)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(GetMapIndex())) {
        ChatPacket(CHAT_TYPE_INFO, LC_TEXT("cz_cannot_use_polymorph_item"));
        return false;
    }
#endif

    if (IsPolymorphed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "이미 둔갑중인 상태입니다.");
        return false;
    }

    if (true == IsRiding()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "둔갑할 수 없는 상태입니다.");
        return false;
    }

    if (CPolymorphUtils::Instance().IsOnPolymorphMapBlacklist(GetMapIndex())) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "둔갑에 실패 하였습니다");
        return false;
    }

    uint32_t dwVnum = item->GetSocket(0);

    if (dwVnum == 0) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "잘못된 둔갑 아이템입니다.");
        item->SetCount(item->GetCount() - 1);
        return false;
    }

    const auto* pMob = CMobManager::instance().Get(dwVnum);

    if (pMob == nullptr) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "잘못된 둔갑 아이템입니다.");
        item->SetCount(item->GetCount() - 1);
        return false;
    }

    switch (item->GetVnum()) {
        case 70104:
        case 70105:
        case 70106:
        case 70107: {
            // 둔갑구 처리
            SPDLOG_INFO("USE_POLYMORPH_BALL PID(%d) vnum(%d)", GetPlayerID(),
                        dwVnum);

            // 레벨 제한 체크
            /*int iPolymorphLevelLimit = MAX(0, 20 - GetLevel() * 3 / 10);
            if (pMob->bLevel >= GetLevel() + iPolymorphLevelLimit) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "나보다 너무 높은 레벨의 몬스터로는 변신 할 "
                                   "수 없습니다.");
                return false;
            }*/

            int iDuration =
                GetSkillLevel(POLYMORPH_SKILL_ID) == 0
                    ? 5
                    : (5 + (5 + GetSkillLevel(POLYMORPH_SKILL_ID) / 40 * 25));
            iDuration *= 60;

            uint32_t dwBonus =
                (2 + GetSkillLevel(POLYMORPH_SKILL_ID) / 40) * 100;

            AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, dwVnum, iDuration, 0,
                      true);
            AddAffect(AFFECT_POLYMORPH, POINT_ATT_BONUS, dwBonus, iDuration, 0,
                      false);

            item->SetCount(item->GetCount() - 1);
        } break;

        case 50322: {
            // 보류

            // 둔갑서 처리
            // 소켓0                소켓1           소켓2
            // 둔갑할 몬스터 번호   수련정도        둔갑서 레벨
            SPDLOG_INFO("USE_POLYMORPH_BOOK: {}({}) vnum({})", GetName(),
                        GetPlayerID(), dwVnum);

            if (CPolymorphUtils::instance().PolymorphCharacter(this, item,
                                                               pMob))
                CPolymorphUtils::instance().UpdateBookPracticeGrade(this, item);
        } break;

        default:
            SPDLOG_ERROR("POLYMORPH invalid item passed PID({}) vnum({})",
                         GetPlayerID(), item->GetOriginalVnum());
            return false;
    }

    return true;
}

bool CHARACTER::CanDoCube() const
{
    if (m_bIsObserver && !IsGM())
        return false;
    if (GetShop())
        return false;
    if (GetMyShop())
        return false;
    if (IsWarping())
        return false;

    return true;
}

bool CHARACTER::IsValidItemPosition(const TItemPos& Pos) const
{
    uint8_t window_type = Pos.window_type;
    uint16_t cell = Pos.cell;

    switch (window_type) {
        case RESERVED_WINDOW:
        case DESTROYED_WINDOW:
            return false;

        case INVENTORY:
        case BELT_INVENTORY:
            if (m_windows.Get(window_type))
                return cell < m_windows.Get(window_type)->GetSize();
            else
                return false;

        case EQUIPMENT:
            return cell < WEAR_MAX_NUM;

        case SWITCHBOT:
            return cell < SWITCHBOT_SLOT_COUNT;

        case DRAGON_SOUL_INVENTORY:
            return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);

        case SAFEBOX: {
            if (nullptr != m_pkSafebox)
                return m_pkSafebox->IsValidPosition(cell);
            return false;
        }

        case MALL: {
            if (nullptr != m_pkMall)
                return m_pkMall->IsValidPosition(cell);
            return false;
        }

        default:
            return false;
    }
}

int32_t GetAcceRefineGrade(uint32_t vnum)
{
    if (vnum >= 85001 && vnum <= 85004)
        return vnum - 85000;

    if (vnum >= 85005 && vnum <= 85008)
        return vnum - 85004;

    if (vnum >= 85011 && vnum <= 85014)
        return vnum - 85010;

    if (vnum >= 85015 && vnum <= 85018)
        return vnum - 85014;

    return -1;
}

void CHARACTER::AcceRefineCheckin(uint8_t acceWindowCell,
                                  const TItemPos& inventoryCell,
                                  uint8_t windowType)
{
    if (GetAcceWindowType() != windowType) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("INVALID_ACCE_WINDOW_HACKER_QST"));
        return;
    }

    if (GetAcceWindowType() >= ACCE_SLOT_TYPE_MAX) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("INVALID_ACCE_WINDOW_HACKER_QST"));
        return;
    }

    if (GetExchange() || IsShop() || GetShopOwner() || IsOpenSafebox() ||
        IsCubeOpen())
        return;

    if (!IsValidItemPosition(inventoryCell))
        return;

    if (GetAcceSlot(acceWindowCell) != INVENTORY_MAX_NUM) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_SLOT_ALREADY_IN_USE");
        return;
    }

    auto InventoryItem = GetItem(inventoryCell);

    if (InventoryItem == nullptr)
        return;

    if (InventoryItem->IsEquipped()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "ACCE_CANNOT_USE_EQUIPPED_ITEMS");
        return;
    }

    if (InventoryItem->IsSealed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, ("ITEM_IS_SEALED_CANNOT_DO"));
        return;
    }

    if (InventoryItem->GetCell() >= INVENTORY_MAX_NUM &&
        IS_SET(InventoryItem->GetFlag(), ITEM_FLAG_IRREMOVABLE)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("SAFEBOX__CANNOT_MOVE_ITEM_INTO"));
        return;
    }

    if (true == InventoryItem->isLocked()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("SAFEBOX__ITEM_CANNOT_BE_STORED"));
        return;
    }

    auto OterSlotItem = GetAcceItemBySlot(
        acceWindowCell == ACCE_SLOT_RIGHT ? ACCE_SLOT_LEFT : ACCE_SLOT_RIGHT);

    if (OterSlotItem != nullptr) {
        if (InventoryItem->GetID() == OterSlotItem->GetID())
            return;
    }

    if (windowType == ACCE_SLOT_TYPE_COMBINE) {
        AcceRefineCheckinCombine(acceWindowCell, inventoryCell, InventoryItem,
                                 OterSlotItem);
    } else if (windowType == ACCE_SLOT_TYPE_ABSORB) {
        AcceRefineCheckinAbsorb(acceWindowCell, inventoryCell, InventoryItem,
                                OterSlotItem);
    }
}

void CHARACTER::AcceRefineCheckinCombine(uint8_t acceWindowCell,
                                         const TItemPos& invenPos,
                                         CItem* InventoryItem, CItem* OtherItem)
{
    if (InventoryItem->GetItemType() != ITEM_COSTUME &&
        InventoryItem->GetSubType() != COSTUME_ACCE) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, ("ACCE_COMBINE_NO_ACCE_ITEM"));
        return;
    }

    if (OtherItem) {
        int32_t InventoryItemGrade =
            GetAcceRefineGrade(InventoryItem->GetVnum());
        int32_t OtherItemGrade = GetAcceRefineGrade(OtherItem->GetVnum());

        if (OtherItemGrade != -1) {
            if (InventoryItemGrade != OtherItemGrade) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   ("ACCE_COMBINE_CANNOT_COMBINE_DIFFERENT_"
                                    "GRADE"));
                return;
            }
        }
    }

    SetAcceSlot(acceWindowCell, invenPos.cell);

    if (!IsEmptyAcceSlot(ACCE_SLOT_LEFT)) {
        CItem* pkAcce = GetItem(
            TItemPos(INVENTORY, m_pointsInstant.pAcceSlots[ACCE_SLOT_LEFT]));

        TPacketGCAcce pack;
        pack.subheader = ACCE_SUBHEADER_GC_SET_ITEM;

        GcItemSetPacket pack_sub;
        pack_sub.pos =
            TItemPos(ACCEREFINE, static_cast<uint16_t>(ACCE_SLOT_RESULT));
        pack_sub.data.id = pkAcce->GetID();

        pack_sub.data.vnum = pkAcce->GetRefinedVnum() ? pkAcce->GetRefinedVnum()
                                                      : pkAcce->GetVnum();
        pack_sub.data.count = pkAcce->GetCount();
        pack_sub.data.nSealDate = pkAcce->GetSealDate();

        std::memcpy(pack_sub.data.sockets, pkAcce->GetSockets(),
                    sizeof(pack_sub.data.sockets));
        std::memcpy(pack_sub.data.attrs, pkAcce->GetAttributes(),
                    sizeof(pack_sub.data.attrs));
        pack.itemSet = pack_sub;
        GetDesc()->Send(HEADER_GC_ACCE, pack);
    }

    TPacketGCAcce pack;
    pack.subheader = ACCE_SUBHEADER_GC_SET_ITEM;

    GcItemSetPacket pack_sub;
    pack_sub.data.id = InventoryItem->GetID();

    pack_sub.pos = TItemPos(ACCEREFINE, static_cast<uint16_t>(acceWindowCell));
    pack_sub.data.vnum = InventoryItem->GetVnum();
    pack_sub.data.count = InventoryItem->GetCount();
    pack_sub.data.nSealDate = InventoryItem->GetSealDate();

    std::memcpy(pack_sub.data.sockets, InventoryItem->GetSockets(),
                sizeof(pack_sub.data.sockets));
    std::memcpy(pack_sub.data.attrs, InventoryItem->GetAttributes(),
                sizeof(pack_sub.data.attrs));
    pack.itemSet = pack_sub;
    GetDesc()->Send(HEADER_GC_ACCE, pack);
}

void CHARACTER::AcceRefineCheckinAbsorb(uint8_t acceWindowCell,
                                        const TItemPos& invenPos,
                                        CItem* InventoryItem, CItem* OtherItem)
{
    if (acceWindowCell == ACCE_SLOT_RIGHT) {
        if (InventoryItem->GetItemType() == ITEM_COSTUME)
            return;

        if (InventoryItem->GetItemType() != ITEM_WEAPON &&
            InventoryItem->GetItemType() != ITEM_ARMOR)
            return;

        if (InventoryItem->GetItemType() == ITEM_ARMOR &&
            InventoryItem->GetSubType() != ARMOR_BODY)
            return;

        if (InventoryItem->GetItemType() == ITEM_WEAPON &&
            InventoryItem->GetSubType() == WEAPON_ARROW)
            return;

        if (InventoryItem->GetItemType() == ITEM_WEAPON &&
            InventoryItem->GetSubType() == WEAPON_QUIVER)
            return;
    } else if (acceWindowCell == ACCE_SLOT_LEFT) {
        if (InventoryItem->GetItemType() != ITEM_COSTUME &&
            InventoryItem->GetSubType() != COSTUME_ACCE)
            return;

        if (InventoryItem->GetSocket(ACCE_SOCKET_ITEM_VNUM) != 0) {
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               ("ACCE_ALREADY_ABSORBED_BONUS"));
            return;
        }
    }

    SetAcceSlot(acceWindowCell, invenPos.cell);

    if (!IsEmptyAcceSlot(ACCE_SLOT_RIGHT) && !IsEmptyAcceSlot(ACCE_SLOT_LEFT)) {
        CItem* pkAcce = GetItem(
            TItemPos(INVENTORY, m_pointsInstant.pAcceSlots[ACCE_SLOT_LEFT]));
        CItem* pkWeaponToAbsorb = GetItem(
            TItemPos(INVENTORY, m_pointsInstant.pAcceSlots[ACCE_SLOT_RIGHT]));

        TPacketGCAcce pack;
        pack.subheader = ACCE_SUBHEADER_GC_SET_ITEM;

        GcItemSetPacket pack_sub;
        pack_sub.data.id = pkAcce->GetID();

        pack_sub.pos =
            TItemPos(ACCEREFINE, static_cast<uint16_t>(ACCE_SLOT_RESULT));
        pack_sub.data.vnum = pkAcce->GetVnum();
        pack_sub.data.count = pkAcce->GetCount();
        pack_sub.data.nSealDate = pkAcce->GetSealDate();

        SocketValue newSockets[ITEM_SOCKET_MAX_NUM];
        std::memcpy(newSockets, pkAcce->GetSockets(), sizeof(newSockets));
        newSockets[1] = pkWeaponToAbsorb->GetVnum();

        std::memcpy(pack_sub.data.sockets, newSockets,
                    sizeof(pack_sub.data.sockets));
        std::memcpy(pack_sub.data.attrs, pkWeaponToAbsorb->GetAttributes(),
                    sizeof(pack_sub.data.attrs));
        pack.itemSet = pack_sub;
        GetDesc()->Send(HEADER_GC_ACCE, pack);
    }

    TPacketGCAcce pack;
    pack.subheader = ACCE_SUBHEADER_GC_SET_ITEM;

    GcItemSetPacket pack_sub;

    pack_sub.pos = TItemPos(ACCEREFINE, static_cast<uint16_t>(acceWindowCell));
    pack_sub.data.vnum = InventoryItem->GetVnum();
    pack_sub.data.id = InventoryItem->GetID();

    pack_sub.data.count = InventoryItem->GetCount();
    pack_sub.data.nSealDate = InventoryItem->GetSealDate();

    std::memcpy(pack_sub.data.sockets, InventoryItem->GetSockets(),
                sizeof(pack_sub.data.sockets));
    std::memcpy(pack_sub.data.attrs, InventoryItem->GetAttributes(),
                sizeof(pack_sub.data.attrs));

    pack.itemSet = pack_sub;
    GetDesc()->Send(HEADER_GC_ACCE, pack);
}

void CHARACTER::AcceRefineCheckout(uint8_t acceSlot)
{
    if (m_pointsInstant.pAcceSlots[acceSlot] == INVENTORY_MAX_NUM) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_SLOT_ALREADY_EMPTY");
        return;
    }

    m_pointsInstant.pAcceSlots[acceSlot] = INVENTORY_MAX_NUM;

    DESC* d = GetDesc();

    struct SPacketGCClearAcceSlot {
        uint16_t cell;
    };

    TPacketGCAcce p;
    p.subheader = ACCE_SUBHEADER_GC_CLEAR_SLOT;
    p.cell = acceSlot;
    GetDesc()->Send(HEADER_GC_ACCE, p);
}

void CHARACTER::ChanceRefine(CItem* BaseItem, CItem* MaterialItem,
                             uint64_t cost)
{
    return;
#ifdef ENABLE_ACCE_SECOND_SYSTEM_MIN_CHANCE
    if (BaseItem->GetSocket(ACCE_SOCKET_DRAIN_PCT) <
        ENABLE_ACCE_SECOND_SYSTEM_MIN_CHANCE) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "ACCE_YOUR_BASE_ITEM_NEEDS_ATLEAST_%d_PERCENT_"
                           "ABSORB",
                           ENABLE_ACCE_SECOND_SYSTEM_MIN_CHANCE);
        return;
    }
#endif

    CItem* ResultItem =
        ITEM_MANAGER::instance().CreateItem(BaseItem->GetVnum(), 1, 0, false);

    if (!ResultItem) {
        SPDLOG_ERROR("AcceRefine: Cannot create item {}", BaseItem->GetVnum());
        AcceRefineClear();
        return;
    }

    MaterialItem->RemoveFromCharacter();
    uint16_t ResultCell = BaseItem->GetCell();
    BaseItem->RemoveFromCharacter();

    BaseItem->CopyAttributeTo(ResultItem);
    BaseItem->CopySocketTo(ResultItem);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d({300, 200, 60, 30, 10, 5});
    uint32_t percentageAdd = d(gen);

    ResultItem->SetSocket(
        0, std::clamp<uint32_t>(
               BaseItem->GetSocket(ACCE_SOCKET_DRAIN_PCT) + percentageAdd,
               BaseItem->GetSocket(ACCE_SOCKET_DRAIN_PCT), 25u));

    ITEM_MANAGER::instance().RemoveItem(MaterialItem, "ACCE_MAT");
    ITEM_MANAGER::instance().RemoveItem(BaseItem, "ACCE_BASE");

    ResultItem->AddToCharacter(this, TItemPos(INVENTORY, ResultCell));
    ITEM_MANAGER::instance().FlushDelayedSave(ResultItem);

    PayRefineFee(cost);
    AcceRefineClear();

    if (percentageAdd > 0) {
        g_pLogManager->ItemLog(this, ResultItem, "ACCE CHANCE SUCCESS",
                               ResultItem->GetName());

        EffectPacket(SE_ACCE_ABSORB);
        SendI18nChatPacket(this, CHAT_TYPE_INFO, ("ACCE_SUCCESS"));
    } else {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_NO_PERCENT_ADDED");
    }
}

void CHARACTER::AcceCombine()
{
    return;
    // Check if one of the slots is empty, we cannot continue in this case
    if (IsEmptyAcceSlot(ACCE_SLOT_LEFT) || IsEmptyAcceSlot(ACCE_SLOT_RIGHT)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "ACCE_NEED_TWO_ITEMS_TO_COMBINE");
        return;
    }

    // Get the items and check for valid pointers
    CItem* BaseItem = GetAcceItemBySlot(ACCE_SLOT_LEFT);
    CItem* MaterialItem = GetAcceItemBySlot(ACCE_SLOT_RIGHT);

    if (!BaseItem || !MaterialItem)
        return;

    // Both item need to be Acce Items
    if (!BaseItem->IsCostumeAcce() || !MaterialItem->IsCostumeAcce()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "ACCE_COMBINE_CANNOT_COMBINE_NON_ACCE_ITEMS");
        return;
    }

    // The items need to have the same grade
    if (BaseItem->GetRefinedVnum() != MaterialItem->GetRefinedVnum()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "ACCE_COMBINE_CANNOT_COMBINE_DIFFERENT_GRADE");
        return;
    }

    // We cannot use equipped items
    if (BaseItem->IsEquipped() || MaterialItem->IsEquipped()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "ACCE_CANNOT_USE_EQUIPPED_ITEMS");
        return;
    }

    // ResultVnum of the operation based on the BaseItem refined Vnum and
    // RefineCost based on the BaseItem Buy price
    auto ResultVnum = BaseItem->GetRefinedVnum();
    auto RefineCost = BaseItem->GetProto()->dwGold;

    // Refine table
    auto prt =
        CRefineManager::instance().GetRefineRecipe(BaseItem->GetRefineSet());

    if (!prt && ResultVnum)
        return;

    // Check if we have enough gold to continue
    if (GetGold() < RefineCost) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_REFINE_NOT_ENOUGH_GOLD");
        return;
    }

    // When the ResultVnum is zero we are going to try to upgrade the chance of
    // success
    if (ResultVnum == 0) {
        ChanceRefine(BaseItem, MaterialItem, RefineCost);
        return;
    }

    // Probability check ex: 50% > 30% -> Failed
    // when the check fails Remove the MaterialItem
    // Pay the refine fee and clear the refine window
    if (Random::get(1, 100) > prt->prob) {
        ITEM_MANAGER::instance().RemoveItem(MaterialItem, "ACCE_COMB_FAIL");
        AcceRefineCheckout(ACCE_SLOT_RIGHT);
        PayRefineFee(RefineCost);
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_COMBINE_FAILED");
        return;
    }

    // Create a new item in the manager
    CItem* ResultItem =
        ITEM_MANAGER::instance().CreateItem(ResultVnum, 1, 0, false);

    if (!ResultItem) {
        SPDLOG_ERROR("AcceCombine: Cannot create item {}", ResultVnum);
        AcceRefineClear();
        return;
    }

    uint16_t ResultCell = BaseItem->GetCell();
    // Remove both items from the character
    BaseItem->RemoveFromCharacter();
    MaterialItem->RemoveFromCharacter();

    // Copy the attributes from the BaseItem to the ResultItem
    if (BaseItem->GetAttributeCount() > 0)
        BaseItem->CopyAttributeTo(ResultItem);
    else if (MaterialItem->GetAttributeCount() > 0)
        MaterialItem->CopyAttributeTo(ResultItem);
    // The ResultItem is the last step
    // it has a dynamic absorb rate
    // so we use a discrete_distribution
    // to generate this with some influence
    if (!ResultItem->GetRefinedVnum()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> d(
            {0, 40, 70, 90, 100, 150, 100, 130, 160, 80, 30, 20});

        ResultItem->SetSocket(0, BaseItem->GetProto()->aApplies[0].lValue + 1 +
                                     d(gen));
    }

    // Try to add the result item to the character
    if (ResultItem->AddToCharacter(this, TItemPos(INVENTORY, ResultCell))) {
        // Save the result item
        ITEM_MANAGER::instance().FlushDelayedSave(ResultItem);

        // Remove BaseItem and MaterialItem the player has the result Item now
        ITEM_MANAGER::instance().RemoveItem(MaterialItem, "ACCE_MAT");
        ITEM_MANAGER::instance().RemoveItem(BaseItem, "ACCE_BASE");

        // Log a successful AcceRefine
        g_pLogManager->ItemLog(this, ResultItem, "ACCE COMB SUCCESS",
                               ResultItem->GetName());

        // Pay the refine fee
        PayRefineFee(RefineCost);

        // Clear the acce window
        AcceRefineClear();

        // Notify the player
        EffectPacket(SE_ACCE_ABSORB);
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_COMBINE_SUCCESS");
    } else {
        // Clear the acce window
        AcceRefineClear();
        // Something went wrong notify the player and log
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_COMBINE_ERROR_ASK_TEAM");
        g_pLogManager->ItemLog(this, ResultItem, "ACCEFAIL",
                               ResultItem->GetName());
        ITEM_MANAGER::instance().RemoveItem(ResultItem, "ACCEFAIL");
    }
}

uint16_t CHARACTER::GetAcceSlot(int32_t index)
{
    if (index >= ACCE_SLOT_MAX_NUM)
        return INVENTORY_MAX_NUM;

    return m_pointsInstant.pAcceSlots[index];
}

void CHARACTER::SetAcceSlot(int32_t index, uint16_t slotIndex)
{
    if (index >= ACCE_SLOT_MAX_NUM)
        return;

    m_pointsInstant.pAcceSlots[index] = slotIndex;
}

CItem* CHARACTER::GetAcceItemBySlot(int32_t index)
{
    if (index >= ACCE_SLOT_MAX_NUM)
        return nullptr;

    return GetItem(TItemPos(INVENTORY, GetAcceSlot(index)));
}

bool CHARACTER::IsEmptyAcceSlot(int32_t index) const
{
    if (index >= ACCE_SLOT_MAX_NUM)
        return false;

    return m_pointsInstant.pAcceSlots[index] == INVENTORY_MAX_NUM;
}

bool CHARACTER::AcceAbsorb()
{
    // Stop if one of the slots is empty
    if (IsEmptyAcceSlot(ACCE_SLOT_LEFT)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_NO_ACCE_ITEM_TO_ABSORB");
        return false;
    }

    if (IsEmptyAcceSlot(ACCE_SLOT_RIGHT)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "ACCE_NO_ACCE_ITEM_TO_ABSORB_FROM");
        return false;
    }

    // Get the items
    auto AcceItem = GetAcceItemBySlot(ACCE_SLOT_LEFT);
    auto ItemToAbsorb = GetAcceItemBySlot(ACCE_SLOT_RIGHT);

    // Check for valid item pointers
    if (!ItemToAbsorb || !AcceItem) {
        AcceRefineClear();
        return false;
    }

    // We cannot use Equipped Items
    if (ItemToAbsorb->IsEquipped() || AcceItem->IsEquipped()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("ACCE_CANNOT_ABSORB_EQUIPPED_ITEM"));
        AcceRefineClear();
        return false;
    }

    // Check if the Acce Item is actually valid
    // TODO
    // if (!CItemVnumHelper::IsAcceItem(AcceItem->GetVnum())) {
    //	AcceRefineCheckout(ACCE_SLOT_LEFT);
    //	return false;
    //}

    // Only weapons and the main armor is allowed for the absorbtion proccess
    if (!ItemToAbsorb->IsWeapon() && !ItemToAbsorb->IsArmorBody()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("ACCE_CANNOT_ABSORB_WRONG_ITEM"));
        AcceRefineClear();
        return false;
    }

    uint16_t TargetCell = AcceItem->GetCell();

    // Remove Acce Item and Item to Absorb from the Character
    AcceItem->RemoveFromCharacter();
    ItemToAbsorb->RemoveFromCharacter();

    // Copy the attributes from the ItemToAbsorb to the Acce Item also set
    // socket 1 to the ItemToAbsorb Vnum
    ItemToAbsorb->CopyAttributeTo(AcceItem);
    AcceItem->SetSocket(ACCE_SOCKET_ITEM_VNUM, ItemToAbsorb->GetVnum());

    // Add the AcceItem back to the character
    AcceItem->AddToCharacter(this, TItemPos(INVENTORY, TargetCell));

    // Remove the ItemToAbsorb
    ITEM_MANAGER::instance().RemoveItem(ItemToAbsorb, "ACCE_ABSORBED");

    // Clear the refine window
    AcceRefineClear();

    // Tell the player we it was a success
    SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_SUCCESS_ABSORB");
    EffectPacket(SE_ACCE_ABSORB);

    return true;
}

void CHARACTER::AcceRefineAccept(int32_t windowType)
{
    if (ACCE_SLOT_TYPE_ABSORB == windowType)
        AcceAbsorb();
    else if (ACCE_SLOT_TYPE_COMBINE == windowType)
        AcceCombine();
}

void CHARACTER::AcceRefineClear()
{
    TPacketGCAcce pack = {};
    pack.subheader = CHANGELOOK_SUBHEADER_GC_CLEAR_ALL;

    GetDesc()->Send(HEADER_GC_ACCE, pack);

    m_pointsInstant.pAcceSlots[ACCE_SLOT_RESULT] = INVENTORY_MAX_NUM;
    m_pointsInstant.pAcceSlots[ACCE_SLOT_LEFT] = INVENTORY_MAX_NUM;
    m_pointsInstant.pAcceSlots[ACCE_SLOT_RIGHT] = INVENTORY_MAX_NUM;
}

void CHARACTER::ChangeLookCheckin(uint8_t pos, const TItemPos& itemPosition)
{
    if (pos >= CHANGELOOK_SLOT_MAX) {
        return;
    }

    if (GetExchange() || IsShop() || GetShopOwner() || IsOpenSafebox() ||
        IsCubeOpen())
        return;

    if (!IsValidItemPosition(itemPosition))
        return;

    if (m_pointsInstant.changeLookSlots[pos] != INVENTORY_MAX_NUM) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "SLOT_ALREADY_IN_USE");
        return;
    }

    auto item = GetItem(itemPosition);

    if (item == nullptr)
        return;

    if (item->IsEquipped()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "CANNOT_USE_EQUIPPED_ITEMS");
        return;
    }

    if (item->IsSealed()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, ("ITEM_IS_SEALED_CANNOT_DO"));
        return;
    }

    if (item->GetCell() >= INVENTORY_MAX_NUM &&
        IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("SAFEBOX__CANNOT_MOVE_ITEM_INTO"));
        return;
    }

    if (true == item->isLocked()) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("SAFEBOX__ITEM_CANNOT_BE_STORED"));
        return;
    }

    if(IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_CHANGELOOK)) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           ("<Changelook> This item cannot be used."));
        return;
    }

    auto otherSlotPos = pos == CHANGELOOK_SLOT_RIGHT ? CHANGELOOK_SLOT_LEFT
                                                     : CHANGELOOK_SLOT_RIGHT;
    auto OterSlotItem = GetItem(
        TItemPos(INVENTORY, m_pointsInstant.changeLookSlots[otherSlotPos]));

    if (OterSlotItem != nullptr) {
        if (item->GetID() == OterSlotItem->GetID())
            return;

        if (item->GetItemType() != OterSlotItem->GetItemType())
            return;

        if (item->GetSubType() != OterSlotItem->GetSubType())
            return;
    }

    m_pointsInstant.changeLookSlots[pos] = item->GetCell();

    GcChangeLookPacket pack{};
    pack.subheader = CHANGELOOK_SUBHEADER_GC_SET_ITEM;

    GcItemSetPacket pack_sub = {};

    pack_sub.pos = TItemPos(CHANGELOOK, static_cast<uint16_t>(pos));
    pack_sub.data.vnum = item->GetVnum();
    pack_sub.data.count = item->GetCount();
    pack_sub.data.nSealDate = item->GetSealDate();
    pack_sub.data.transVnum = item->GetTransmutationVnum();
    pack_sub.data.id = item->GetID();
    std::memcpy(pack_sub.data.sockets, item->GetSockets(),
                sizeof(pack_sub.data.sockets));
    std::memcpy(pack_sub.data.attrs, item->GetAttributes(),
                sizeof(pack_sub.data.attrs));
    const auto origPos = item->GetPosition();

    pack.itemSet = pack_sub;
    pack.pos = origPos;
    GetDesc()->Send(HEADER_GC_CHANGELOOK, pack);
}

void CHARACTER::ChangeLookCheckout(uint8_t pos)
{
    if (m_pointsInstant.changeLookSlots[pos] == INVENTORY_MAX_NUM) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "ACCE_SLOT_ALREADY_EMPTY");
        return;
    }

    m_pointsInstant.changeLookSlots[pos] = INVENTORY_MAX_NUM;

    DESC* d = GetDesc();

    GcChangeLookPacket p;
    p.pos = TItemPos(RESERVED_WINDOW, pos);
    p.subheader = CHANGELOOK_SUBHEADER_GC_CLEAR_SLOT;
    d->Send(HEADER_GC_CHANGELOOK, p);
}

void CHARACTER::ChangeLookAccept()
{
    if (m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_LEFT] ==
            INVENTORY_MAX_NUM ||
        m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_RIGHT] ==
            INVENTORY_MAX_NUM) {
        return;
    }

    auto itemLeft = GetItem(TItemPos(
        INVENTORY, m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_LEFT]));
    if (!itemLeft)
        return;

    auto itemRight = GetItem(TItemPos(
        INVENTORY, m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_RIGHT]));
    if (!itemRight)
        return;

    if (GetGold() < 50'000'000) {
        SendI18nChatPacket(this, CHAT_TYPE_INFO,
                           "개량을 하기 위한 돈이 부족합니다.");
        return;
    }

    itemRight->RemoveFromCharacter();

    itemLeft->SetTransmutationVnum(itemRight->GetVnum());
    ITEM_MANAGER::instance().FlushDelayedSave(itemLeft);

    itemLeft->UpdatePacket();

    ChangeGold(-50'000'000);

    M2_DESTROY_ITEM(itemRight);

    ChangeLookCancel();
}

void CHARACTER::ChangeLookCancel()
{
    GcChangeLookPacket pack = {};
    pack.subheader = CHANGELOOK_SUBHEADER_GC_CLEAR_ALL;
    GetDesc()->Send(HEADER_GC_CHANGELOOK, pack);

    m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_LEFT] = INVENTORY_MAX_NUM;
    m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_RIGHT] = INVENTORY_MAX_NUM;
    m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_SPECIAL_ITEM] =
        INVENTORY_MAX_NUM;
}

CHARACTER::Windows::Windows()
    : inventory(INVENTORY_PAGE_WIDTH, INVENTORY_PAGE_HEIGHT,
                INVENTORY_PAGE_COUNT)
    , belt(BELT_INVENTORY_WIDTH, BELT_INVENTORY_HEIGHT)
{
    // ctor
}

const ItemGrid* CHARACTER::Windows::Get(uint8_t type) const
{
    switch (type) {
        case INVENTORY:
            return &inventory;
        case BELT_INVENTORY:
            return &belt;
    }

    return nullptr;
}

ItemGrid* CHARACTER::Windows::Get(uint8_t type)
{
    switch (type) {
        case INVENTORY:
            return &inventory;
        case BELT_INVENTORY:
            return &belt;
    }

    return nullptr;
}

void CHARACTER::LevelPetOpen(const TItemPos& itemPos)
{
    auto* item = GetItem(itemPos);
    if (!item)
        return;

    if (item->GetItemType() != ITEM_TOGGLE ||
        item->GetSubType() != TOGGLE_LEVEL_PET)
        return;

    if (item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE))
        return;

    if (!item->GetPosition().IsDefaultInventoryPosition())
        return;

    if (m_pointsInstant.pOpenLevelPet)
        return;

    item->Lock(true);
    m_pointsInstant.pOpenLevelPet = item;
    ChatPacket(CHAT_TYPE_COMMAND, "LevelPetOpen %d", item->GetCell());
}

void CHARACTER::LevelPetClose()
{
    if (!m_pointsInstant.pOpenLevelPet)
        return;

    m_pointsInstant.pOpenLevelPet->Lock(false);
    m_pointsInstant.pOpenLevelPet = nullptr;

    ChatPacket(CHAT_TYPE_COMMAND, "LevelPetClose");
}

void CHARACTER::LevelPetPlusAttr(uint8_t index)
{
    if (!m_pointsInstant.pOpenLevelPet)
        return;

    auto* item = m_pointsInstant.pOpenLevelPet;

    if (index > item->GetAttributeCount())
        return;

    const auto& attr = item->GetAttribute(index);
    const auto addPoint =
        ITEM_MANAGER::instance().GetLevelPetBonusPerPoint(attr.bType);
    const auto defaultPoint =
        ITEM_MANAGER::instance().GetLevelPetApplyDefault(attr.bType);

    const int32_t bonusLevel = static_cast<int32_t>((attr.sValue - defaultPoint) / addPoint);

    if (gConfig.testServer) {
        ChatPacket(CHAT_TYPE_INFO, "Bonus Level %d", bonusLevel);
        ChatPacket(CHAT_TYPE_INFO, "Add Point %f", addPoint);
        ChatPacket(CHAT_TYPE_INFO, "Default Point %f", defaultPoint);
        ChatPacket(CHAT_TYPE_INFO, "Current Point %f", attr.sValue);
    }

    if (bonusLevel >= 149)
        return;

    const auto statPoints = item->GetSocket(ITEM_SOCKET_LEVEL_STAT_POINT);
    if (statPoints < 1)
        return;

    item->SetForceAttribute(index, attr.bType, attr.sValue + addPoint);
    item->SetSocket(ITEM_SOCKET_LEVEL_STAT_POINT, statPoints - 1);
    item->Save();
    item->UpdatePacket();
}
