#include "char.h"
#include "desc.h"

#include <game/GamePacket.hpp>

/////////////////////////////////////////////////////////////////////////////
// QUICKSLOT HANDLING
/////////////////////////////////////////////////////////////////////////////
void CHARACTER::SyncQuickslot(uint8_t bType, uint16_t bOldPos,
                              uint16_t bNewPos) // bNewPos == std::numeric_limits<uint16_t>::max() -> delete
{
    if (bOldPos == bNewPos)
        return;

    for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
    {
        if (m_quickslot[i].type == bType && m_quickslot[i].pos == bOldPos)
        {
            if (bNewPos == std::numeric_limits<uint16_t>::max())
                DelQuickslot(i);
            else
            {
                TQuickslot slot;

                slot.type = bType;
                slot.pos = bNewPos;

                SetQuickslot(i, slot);
            }
        }
    }
}

int CHARACTER::GetQuickslotPosition(uint8_t bType, uint16_t inventoryPos)
{
    for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
    {
        if (m_quickslot[i].type == bType && m_quickslot[i].pos == inventoryPos)
            return i;
    }

    return -1;
}

bool CHARACTER::GetQuickslot(uint8_t pos, TQuickslot **ppSlot)
{
    if (pos >= QUICKSLOT_MAX_NUM)
        return false;

    *ppSlot = &m_quickslot[pos];
    return true;
}

bool CHARACTER::SetQuickslot(uint8_t pos, const TQuickslot &rSlot)
{
    struct packet_quickslot_add pack_quickslot_add;

    if (pos >= QUICKSLOT_MAX_NUM)
        return false;

    if (rSlot.type >= QUICKSLOT_TYPE_MAX_NUM)
        return false;

    for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
    {
        if (rSlot.type == 0)
            continue;

        if (m_quickslot[i].type == rSlot.type && m_quickslot[i].pos == rSlot.pos)
            DelQuickslot(i);
    }

    TItemPos srcCell(INVENTORY, rSlot.pos);

    switch (rSlot.type)
    {
    case QUICKSLOT_TYPE_ITEM:
        if (false == srcCell.IsDefaultInventoryPosition() && false == srcCell.IsBeltInventoryPosition())
            return false;

        break;

    case QUICKSLOT_TYPE_SKILL:
        if ((int)rSlot.pos >= SKILL_MAX_NUM)
            return false;

        break;

    case QUICKSLOT_TYPE_COMMAND:
        break;

    default:
        return false;
    }

    m_quickslot[pos] = rSlot;

    if (GetDesc())
    {
        pack_quickslot_add.pos = pos;
        pack_quickslot_add.slot = m_quickslot[pos];

        GetDesc()->Send(HEADER_GC_QUICKSLOT_ADD, pack_quickslot_add);
    }

    return true;
}

bool CHARACTER::DelQuickslot(uint8_t pos)
{
    struct packet_quickslot_del pack_quickslot_del;

    if (pos >= QUICKSLOT_MAX_NUM)
        return false;

    m_quickslot[pos] = {};

    pack_quickslot_del.pos = pos;

    GetDesc()->Send(HEADER_GC_QUICKSLOT_DEL, pack_quickslot_del);
    return true;
}

bool CHARACTER::SwapQuickslot(uint8_t a, uint8_t b)
{
    struct packet_quickslot_swap pack_quickslot_swap;
    TQuickslot quickslot;

    if (a >= QUICKSLOT_MAX_NUM || b >= QUICKSLOT_MAX_NUM)
        return false;

    // Äü ½½·Ô ÀÚ¸®¸¦ ¼­·Î ¹Ù²Û´Ù.
    quickslot = m_quickslot[a];

    m_quickslot[a] = m_quickslot[b];
    m_quickslot[b] = quickslot;

    pack_quickslot_swap.pos = a;
    pack_quickslot_swap.pos_to = b;

    GetDesc()->Send(HEADER_GC_QUICKSLOT_SWAP, pack_quickslot_swap);
    return true;
}

void CHARACTER::ChainQuickslotItem(uint16_t itemCell, uint8_t bType, uint16_t bOldPos)
{
    for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
    {
        if (m_quickslot[i].type == bType && m_quickslot[i].pos == bOldPos)
        {
            TQuickslot slot;
            slot.type = bType;
            slot.pos = itemCell;

            SetQuickslot(i, slot);

            break;
        }
    }
}
