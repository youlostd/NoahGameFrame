#ifndef METIN2_CLIENT_MAIN_CLIENTITEMGRID_HPP
#define METIN2_CLIENT_MAIN_CLIENTITEMGRID_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "game/ItemTypes.hpp"

#include <game/PageGrid.hpp>

struct ClientItemDataAndSize : ClientItemData
{
    ClientItemDataAndSize() = default;

    explicit ClientItemDataAndSize(const ClientItemData &base) : ClientItemData{base}
    {
    }
    uint8_t size;
};

class ClientItemGrid : public PagedGrid<ClientItemDataAndSize>
{
  public:
    ClientItemGrid(uint32_t width, uint32_t height, uint32_t pages = 1, ClientItemGrid *prev = nullptr);

    bool Put(const ClientItemDataAndSize &item, uint32_t cell);
    void Clear(uint32_t cell);

    uint32_t GetBasePos(uint32_t cell) const;
    const ClientItemData *GetUnique(uint32_t cell) const;

    void ClearGrid();
};

#endif
