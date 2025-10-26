#ifndef METIN2_CLIENT_MAIN_PYTHONPRIVATESHOPSEARCH_H
#define METIN2_CLIENT_MAIN_PYTHONPRIVATESHOPSEARCH_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "PythonNetworkStream.h"

class CPythonPrivateShopSearch : public CSingleton<CPythonPrivateShopSearch>
{
public:
    using TItemInstanceVector = std::vector<ShopSearchItemData>;

public:
    CPythonPrivateShopSearch();
    virtual ~CPythonPrivateShopSearch();

    void AddItemData(const ShopSearchItemData &rItemData);

    void SetPageInfo(const ShopSearchPageInfo& pageInfo) { m_pageInfo = pageInfo; }
    const ShopSearchPageInfo& GetPageInfo() { return m_pageInfo; }

   void SetShopSearchFilters(const ShopSearchFilter& filters) { m_shopSearchFilter = filters; }
    const ShopSearchFilter& GetShopSearchFilters() const { return m_shopSearchFilter; }

    void ClearItemData();

    uint32_t GetItemDataCount()
    {
        return m_ItemInstanceVector.size();
    }

    uint32_t GetItemDataPtr(uint32_t index, ShopSearchItemData **ppInstance);
    ShopSearchItemData* GetItemData(uint32_t index);

protected:
    TItemInstanceVector m_ItemInstanceVector;
    ShopSearchPageInfo m_pageInfo{};
    ShopSearchFilter m_shopSearchFilter;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONPRIVATESHOPSEARCH_H */
