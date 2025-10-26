#ifndef METIN2_CLIENT_MAIN_PYTHONSAFEBOX_H
#define METIN2_CLIENT_MAIN_PYTHONSAFEBOX_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "PythonNetworkStream.h"

class CPythonSafeBox : public CSingleton<CPythonSafeBox>
{
public:
    enum
    {
        SAFEBOX_SLOT_X_COUNT = 5,
        SAFEBOX_SLOT_Y_COUNT = 9,
        SAFEBOX_PAGE_SIZE = SAFEBOX_SLOT_X_COUNT * SAFEBOX_SLOT_Y_COUNT,
    };

    typedef std::vector<ClientItemData> TItemInstanceVector;

public:
    CPythonSafeBox();
    virtual ~CPythonSafeBox();

    void OpenSafeBox(int iSize);
    void SetItemData(uint32_t dwSlotIndex, const ClientItemData &rItemData);
    void DelItemData(uint32_t dwSlotIndex);

    void SetMoney(uint32_t dwMoney);
    uint32_t GetMoney();

    bool GetSlotItemID(uint32_t dwSlotIndex, uint32_t *pdwItemID);

    int GetCurrentSafeBoxSize();
    bool GetItemDataPtr(uint32_t dwSlotIndex, ClientItemData **ppInstance);
    ClientItemData *GetItemData(uint32_t dwSlotIndex);

    // MALL
    void OpenMall(int iSize);
    void SetMallItemData(uint32_t dwSlotIndex, const ClientItemData &rItemData);
    void DelMallItemData(uint32_t dwSlotIndex);
    bool GetMallItemDataPtr(uint32_t dwSlotIndex, ClientItemData **ppInstance);
    ClientItemData *GetMallItemData(uint32_t dwSlotIndex);
    bool GetSlotMallItemID(uint32_t dwSlotIndex, uint32_t *pdwItemID);
    uint32_t GetMallSize();

    void SetOpen(bool isOpen)
    {
        m_isOpen = isOpen;
    }

    bool IsOpen() const
    {
        return m_isOpen;
    }

protected:
    TItemInstanceVector m_ItemInstanceVector;
    TItemInstanceVector m_MallItemInstanceVector;
    uint32_t m_dwMoney;
    bool m_isOpen = false;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONSAFEBOX_H */
