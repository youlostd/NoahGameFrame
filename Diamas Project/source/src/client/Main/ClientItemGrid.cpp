#include "StdAfx.h"
#include "ClientItemGrid.hpp"


ClientItemGrid::ClientItemGrid(uint32_t width, uint32_t height, uint32_t pages, ClientItemGrid* prev)
	: PagedGrid(width, height, pages, prev)
{
}

bool ClientItemGrid::Put(const ClientItemDataAndSize& item, uint32_t position)
{
	if(!m_grid)
		return false;

	return PutPos(item, position, item.size);
}

void ClientItemGrid::Clear(uint32_t position)
{
	assert(position < m_width * m_height * m_pages);
	if(!m_grid)
		return;

	position = GetBasePos(position);

	const auto& item = GetPos(position);
	if(!item.vnum)
		return;


	ClearPos(position, item.size);
}

uint32_t ClientItemGrid::GetBasePos(uint32_t cell) const
{
	const auto& item = GetPos(cell);
	if (!item || cell < m_width)
		return cell;

	auto up = item;
	while (up == item && cell >= m_width) {
		cell -= m_width;
		up = GetPos(cell);
	}

	return cell + m_width;
}

const ClientItemData* ClientItemGrid::GetUnique(uint32_t position) const
{
	if (position != GetBasePos(position))
		return nullptr;

	return &GetPos(position);
}

void ClientItemGrid::ClearGrid()
{
	PagedGrid<ClientItemDataAndSize>::Clear();
}


