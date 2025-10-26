#ifndef METIN2_SERVER_GAME_SAFEBOX_H
#define METIN2_SERVER_GAME_SAFEBOX_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ItemGrid.hpp"

class CHARACTER;
class CItem;

class CSafebox
{
public:
    CSafebox(CHARACTER *pkChrOwner, int iSize, uint32_t dwGold);
    ~CSafebox();

    void SetWindowMode(uint8_t bWindowMode);
    void TransferFromInventory(uint32_t dwPos, CItem *pkItem);
    void ChangeSize(int iSize);

    bool IsValidPosition(uint32_t dwPos) const;
    bool IsEmpty(uint32_t dwPos, uint8_t bSize) const;
    CItem *FindNextStackable(CItem *item) const;
    int32_t FindEmpty(uint8_t size) const;

    CItem *Get(uint32_t pos) const;
    bool Add(uint32_t dwPos, CItem *pkItem);
    CItem *Remove(uint32_t pos);
    bool MoveItem(uint16_t bCell, uint16_t bDestCell, CountType count);

    void Save() const;

protected:
    CHARACTER *m_pkChrOwner;
    std::unique_ptr<ItemGrid> m_pkGrid;
    int m_iSize;
    int32_t m_lGold;
    uint8_t m_bWindowMode;
};
#endif /* METIN2_SERVER_GAME_SAFEBOX_H */
