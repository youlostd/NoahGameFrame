#ifndef METIN2_CLIENT_GAMELIB_ITEMMANAGER_H
#define METIN2_CLIENT_GAMELIB_ITEMMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ItemData.h"
#include <game/DragonSoulTable.hpp>
#include <game/ItemTypes.hpp>
#include <optional>

class CItemManager : public CSingleton<CItemManager>
{
  public:
    enum EItemDescCol
    {
        ITEMDESC_COL_VNUM,
        ITEMDESC_COL_NAME,
        ITEMDESC_COL_DESC,
        ITEMDESC_COL_SUMM,
        ITEMDESC_COL_NUM,
    };

  public:
    typedef std::map<uint32_t, std::unique_ptr<CItemData>> TItemMap;
    typedef std::map<std::string, CItemData *> TItemNameMap;
    typedef std::unordered_map<uint32_t, ItemAttrProto> ItemAttrMap;

#ifdef INGAME_WIKI
    typedef std::vector<CItemData *> TItemVec;
    typedef std::vector<DWORD> TItemNumVec;
#endif
  public:
    CItemManager();
    virtual ~CItemManager();

    void Destroy();

    bool SelectItemData(uint32_t dwIndex);
    CItemData *GetSelectedItemDataPointer();

    bool GetItemDataPointer(uint32_t dwItemID, CItemData **ppItemData);
    std::optional<std::vector<CItemData *>> GetProtoEntriesByName(const std::string &name, int32_t limit = 5);
    std::vector<CItemData *> GetRefinedFromItems(uint32_t vnum);
    CItemData *GetProto(int dwItemID);
    const ItemAttrProto &GetRareItemAttr(uint32_t apply);
    const ItemAttrProto &GetItemAttr(uint32_t apply);
    /////
    bool                                               LoadItemDesc(const char * c_szFileName);
    bool                                               LoadItemList(const char * c_szFileName);
    void                                               LoadProtoEntry(std::map<uint32_t, uint32_t> itemNameMap, TItemTable table);
    bool                                               LoadItemTable(const char * c_szFileName);
    const std::vector<LevelPetDisplayedFood> &         GetLevelPetDisplayFoods() const;
    const std::vector<std::tuple<uint32_t, uint32_t>>& GetLevelPetMobs() const;
    bool                                               ReadLevelPetData(const char * c_pszFileName);
    CItemData *                                        MakeItemData(uint32_t dwIndex);
    bool                                               LoadItemScale(const char * szItemScale);
    bool                                               LoadDragonSoulTable(const char * filename);
    bool                                               LoadItemAttrProtos();

#ifdef ENABLE_SHINING_SYSTEM

    bool LoadShiningTable(const char *szShiningTable);

#endif

    DragonSoulTable *GetDragonSoulTable() { return &m_dsTable; }
    bool LoadRareItems(const std::string &filename);
    bool IsRareItem(uint32_t dwVirtualId);

  protected:
    TItemMap m_ItemMap;
    std::vector<CItemData *> m_vec_ItemRange;
    DragonSoulTable m_dsTable;
    std::vector<uint32_t> m_rareItems;
    CItemData *m_pSelectedItemData;
    ItemAttrMap m_itemAttr;
    ItemAttrMap m_itemAttrRare;
    std::vector<ItemApply> m_levelPetAvailableBonus;
    std::vector<std::tuple<uint32_t, uint32_t>> m_levelPetMobs;
    std::unordered_map<ApplyType, float> m_levelPetBonusPerStatPoint;
    std::vector<LevelPetDisplayedFood> m_levelPetDisplayedFoods;
public:
    void RegisterHyperlinkItem(uint32_t id, const ClientItemData& data);
    std::optional<ClientItemData> GetHyperlinkItemData(uint32_t id);
private:
    std::unordered_map<uint32_t, ClientItemData> m_hyperlinkItems;
	
#ifdef INGAME_WIKI
  protected:
    TItemNumVec m_tempItemVec;

  public:
    size_t WikiLoadClassItems(BYTE classType, DWORD raceFilter);
    TItemNumVec *WikiGetLastItems() { return &m_tempItemVec; }

    void WikiAddVnumToBlacklist(DWORD vnum)
    {
        auto it = m_ItemMap.find(vnum);
        if (it != m_ItemMap.end())
            it->second->SetBlacklisted(true);
    };

    DWORD WikiSearchItem(std::string subStr);

  private:
    bool IsFilteredAntiflag(CItemData *itemData, DWORD raceFilter);
#endif
};
#endif /* METIN2_CLIENT_GAMELIB_ITEMMANAGER_H */
