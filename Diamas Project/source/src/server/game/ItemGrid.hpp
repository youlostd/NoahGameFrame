#ifndef METIN2_SERVER_GAME_ITEMGRID_HPP
#define	METIN2_SERVER_GAME_ITEMGRID_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "game/ItemTypes.hpp"

#include <game/PageGrid.hpp>

class CItem;

class ItemGrid : public PagedGrid<CItem *>
{
public:
    ItemGrid(uint32_t width, uint32_t height, uint32_t pages = 1,
             ItemGrid *prev = nullptr);

    bool Put(CItem *item, uint32_t cell);
    void Clear(uint32_t cell);

    uint32_t GetBasePos(uint32_t cell) const;
    CItem *GetUnique(uint32_t cell) const;
};

#endif
