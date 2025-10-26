#include "DbCacheSocket.hpp"
#include "item_manager.h"
#include <game/length.h>
#include <game/DbPackets.hpp>

uint32_t ITEM_MANAGER::GetNewID()
{
    assert(m_ItemIDRange.start != 0);

    if (m_ItemIDRange.start > m_ItemIDRange.max)
    {
        if (m_ItemIDSpareRange == kNullRange)
        {
            // TODO(tim): This is a good place for an exception
            SPDLOG_ERROR("ItemIDRange: FATAL ERROR!!! no more item id");
            thecore_shutdown();
            return 0;
        }

        SPDLOG_INFO(
            "ItemIDRange: Switching to SpareRange %d ~ %d %d",
            m_ItemIDSpareRange.min, m_ItemIDSpareRange.max,
            m_ItemIDSpareRange.start);

        db_clientdesc->DBPacket(HEADER_GD_REQ_SPARE_ITEM_ID_RANGE, 0,
                                &m_ItemIDRange.max, sizeof(uint32_t));

        SetMaxItemID(m_ItemIDSpareRange);
        m_ItemIDSpareRange = kNullRange;
    }

    return m_ItemIDRange.start++;
}

bool ITEM_MANAGER::SetMaxItemID(const ItemIdRange &range)
{
    if (range == kNullRange)
    {
        SPDLOG_ERROR("Item ID range is invalid");
        return false;
    }

    m_ItemIDRange = range;

    SPDLOG_INFO("ItemIDRange: {} ~ {} {}",
                m_ItemIDRange.min, m_ItemIDRange.max, m_ItemIDRange.start);
    return true;
}

bool ITEM_MANAGER::SetMaxSpareItemID(const ItemIdRange &range)
{
    if (range == kNullRange)
    {
        SPDLOG_ERROR("Spare item ID range is invalid");
        return false;
    }

    m_ItemIDSpareRange = range;

    SPDLOG_INFO(
        "ItemIDRange: New Spare ItemID Range {} ~ {} {}",
        m_ItemIDSpareRange.min, m_ItemIDSpareRange.max,
        m_ItemIDSpareRange.start);
    return true;
}
