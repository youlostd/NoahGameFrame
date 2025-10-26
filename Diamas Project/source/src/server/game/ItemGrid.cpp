#include "ItemGrid.hpp"
#include "item.h"

ItemGrid::ItemGrid(uint32_t width, uint32_t height, uint32_t pages, ItemGrid *prev)
    : PagedGrid(width, height, pages, prev)
{
}

bool ItemGrid::Put(CItem *item, uint32_t position) { return PutPos(item, position, item->GetSize()); }

void ItemGrid::Clear(uint32_t position)
{
    assert(position < m_width * m_height * m_pages);

    position = GetBasePos(position);

    auto* item = GetPos(position);
    if (!item)
        return;

    ClearPos(position, item->GetSize());
}

uint32_t ItemGrid::GetBasePos(uint32_t cell) const
{
    auto* item = GetPos(cell);
    if (!item || cell < m_width)
        return cell;

    auto* up = item;
    while (up == item && cell >= m_width)
    {
        cell -= m_width;
        up = GetPos(cell);
    }

    return cell + m_width;
}

CItem *ItemGrid::GetUnique(uint32_t position) const
{
    if (position != GetBasePos(position))
        return nullptr;

    return GetPos(position);
}
