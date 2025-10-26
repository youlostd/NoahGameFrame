#ifndef METIN2_SERVER_GAME_ITEM_MANAGER_H
#define METIN2_SERVER_GAME_ITEM_MANAGER_H

#include "game/MobTypes.hpp"

#include <Config.hpp>
#include <optional>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef M2_USE_POOL
#include "pool.h"
#endif
class CHARACTER;
#include <base/Singleton.hpp>
#include <thecore/utils.hpp>

#include <game/DbPackets.hpp>
#include <base/robin_hood.h>
// special_item_group.txt에서 정의하는 속성 그룹
// type attr로 선언할 수 있다.
// 이 속성 그룹을 이용할 수 있는 것은 special_item_group.txt에서 Special type으로 정의된 그룹에 속한 UNIQUE ITEM이다.
class CSpecialAttrGroup
{
public:
    CSpecialAttrGroup(uint32_t vnum)
        : m_dwVnum(vnum)
    {
    }

    struct CSpecialAttrInfo
    {
        CSpecialAttrInfo(uint32_t _apply_type, uint32_t _apply_value)
            : apply_type(_apply_type), apply_value(_apply_value)
        {
        }

        uint32_t apply_type;
        ApplyValue apply_value;
    };

    uint32_t m_dwVnum;
    std::string m_stEffectFileName;
    std::vector<CSpecialAttrInfo> m_vecAttrs;
};

class CSpecialItemGroup
{
public:
    enum EGiveType
    {
        NONE,
        GOLD,
        EXP,
        MOB,
        SLOW,
        DRAIN_HP,
        POISON,
        MOB_GROUP,
        POLY_MARBLE,
        GIVE_TYPE_MAX_NUM,
    };

    // QUEST 타입은 퀘스트 스크립트에서 vnum.sig_use를 사용할 수 있는 그룹이다.
    //		단, 이 그룹에 들어가기 위해서는 ITEM 자체의 TYPE이 QUEST여야 한다.
    // SPECIAL 타입은 idx, item_vnum, attr_vnum을 입력한다. attr_vnum은 위에 CSpecialAttrGroup의 Vnum이다.
    //		이 그룹에 들어있는 아이템은 같이 착용할 수 없다.
    enum ESIGType { NORMAL, PCT, QUEST, SPECIAL, RAND, ATTR };

    struct CSpecialItemInfo
    {
        uint32_t vnum;
        SocketValue count;
        int rare;

        CSpecialItemInfo(uint32_t _vnum, SocketValue _count, int _rare)
            : vnum(_vnum), count(_count), rare(_rare)
        {
        }
    };

    CSpecialItemGroup(uint32_t vnum, uint8_t type = 0)
        : m_dwVnum(vnum), m_bType(type)
    {
    }

    void AddItem(uint32_t vnum, SocketValue count, int prob, int rare)
    {
        if (!prob)
            return;
        if (!m_vecProbs.empty())
            prob += m_vecProbs.back();
        m_vecProbs.push_back(prob);
        m_vecItems.push_back(CSpecialItemInfo(vnum, count, rare));
    }

    bool IsEmpty() const { return m_vecProbs.empty(); }

    uint8_t GetType() const { return m_bType; }

    // Type Multi, 즉 m_bType == PCT 인 경우,
    // 확률을 더해가지 않고, 독립적으로 계산하여 아이템을 생성한다.
    // 따라서 여러 개의 아이템이 생성될 수 있다.
    // by rtsummit
    int GetMultiIndex(std::vector<int> &idx_vec) const
    {
        idx_vec.clear();
        if (m_bType == PCT)
        {
            int count = 0;
            if (Random::get(1, 100) <= m_vecProbs[0])
            {
                idx_vec.push_back(0);
                count++;
            }
            for (auto i = 1; i < m_vecProbs.size(); i++)
            {
                if (Random::get(1, 100) <= m_vecProbs[i] - m_vecProbs[i - 1])
                {
                    idx_vec.push_back(i);
                    count++;
                }
            }
            return count;
        }
        else
        {
            idx_vec.push_back(GetOneIndex());
            return 1;
        }
    }

    int GetOneIndex() const
    {
        int n = Random::get(1, m_vecProbs.back());
        auto it = lower_bound(m_vecProbs.begin(), m_vecProbs.end(), n);
        return std::distance(m_vecProbs.begin(), it);
    }

    const CSpecialItemInfo &GetRandom() const
    {
        const auto randomIt = Random::get( m_vecItems );
        return *randomIt;
    }

    int GetVnum(int idx) const { return m_vecItems[idx].vnum; }

    SocketValue GetCount(int idx) const { return m_vecItems[idx].count; }

    int GetRarePct(int idx) const { return m_vecItems[idx].rare; }

    bool Contains(uint32_t dwVnum) const
    {
        for (uint32_t i = 0; i < m_vecItems.size(); i++)
        {
            if (m_vecItems[i].vnum == dwVnum)
                return true;
        }
        return false;
    }

    // Group의 Type이 Special인 경우에
    // dwVnum에 매칭되는 AttrVnum을 return해준다.
    uint32_t GetAttrVnum(uint32_t dwVnum) const
    {
        if (CSpecialItemGroup::SPECIAL != m_bType)
            return 0;
        for (auto it = m_vecItems.begin(); it != m_vecItems.end(); ++it)
        {
            if (it->vnum == dwVnum) { return it->count; }
        }
        return 0;
    }

    // Group의 Size를 return해준다.
    int GetGroupSize() const { return m_vecProbs.size(); }

    uint32_t m_dwVnum;
    uint8_t m_bType;
    std::vector<int> m_vecProbs;
    std::vector<CSpecialItemInfo> m_vecItems; // vnum, count
};

class CMobItemGroup
{
public:
    struct SMobItemGroupInfo
    {
        uint32_t dwItemVnum;
        int iCount;
        int iRarePct;

        SMobItemGroupInfo(uint32_t dwItemVnum, int iCount, int iRarePct)
            : dwItemVnum(dwItemVnum),
              iCount(iCount),
              iRarePct(iRarePct)
        {
        }
    };

    CMobItemGroup(uint32_t dwMobVnum, int iKillDrop, const std::string &r_stName)
        : m_dwMobVnum(dwMobVnum),
          m_iKillDrop(iKillDrop),
          m_stName(r_stName)
    {
    }

    int GetKillPerDrop() const { return m_iKillDrop; }

    void AddItem(uint32_t dwItemVnum, int iCount, int iPartPct, int iRarePct)
    {
        if (!m_vecProbs.empty())
            iPartPct += m_vecProbs.back();
        m_vecProbs.push_back(iPartPct);
        m_vecItems.emplace_back(dwItemVnum, iCount, iRarePct);
    }

    // MOB_DROP_ITEM_BUG_FIX
    bool IsEmpty() const { return m_vecProbs.empty(); }

    int GetOneIndex() const
    {
        int n = Random::get(1, m_vecProbs.back());
        auto it = lower_bound(m_vecProbs.begin(), m_vecProbs.end(), n);
        return std::distance(m_vecProbs.begin(), it);
    }

    // END_OF_MOB_DROP_ITEM_BUG_FIX

    const SMobItemGroupInfo &GetOne() const { return m_vecItems[GetOneIndex()]; }

    const std::vector<SMobItemGroupInfo> &GetVector() const { return m_vecItems; }

private:
    uint32_t m_dwMobVnum;
    int m_iKillDrop;
    std::string m_stName;
    std::vector<int> m_vecProbs;
    std::vector<SMobItemGroupInfo> m_vecItems;
};

class CDropItemGroup
{
public:

    struct SDropItemGroupInfo
    {
        uint32_t dwVnum;
        uint32_t dwPct;
        int iCount;

        SDropItemGroupInfo(uint32_t dwVnum, uint32_t dwPct, int iCount)
            : dwVnum(dwVnum), dwPct(dwPct), iCount(iCount)
        {
        }
    };

    CDropItemGroup(uint32_t dwVnum, uint32_t dwMobVnum, const std::string &r_stName)
        : m_dwVnum(dwVnum),
          m_dwMobVnum(dwMobVnum),
          m_stName(r_stName)
    {
    }

    const std::vector<SDropItemGroupInfo> &GetVector() { return m_vec_items; }

    void AddItem(uint32_t dwItemVnum, uint32_t dwPct, int iCount)
    {
        m_vec_items.push_back(SDropItemGroupInfo(dwItemVnum, dwPct, iCount));
    }

    const std::vector<SDropItemGroupInfo> &GetVector() const { return m_vec_items; }

private:
    uint32_t m_dwVnum;
    uint32_t m_dwMobVnum;
    std::string m_stName;
    std::vector<SDropItemGroupInfo> m_vec_items;
};

class CLevelItemGroup
{
public:

    struct SLevelItemGroupInfo
    {
        uint32_t dwVNum;
        uint32_t dwPct;
        int iCount;

        SLevelItemGroupInfo(uint32_t dwVnum, uint32_t dwPct, int iCount)
            : dwVNum(dwVnum), dwPct(dwPct), iCount(iCount)
        {
        }
    };

private :
    uint32_t m_dwLevelLimit;
    std::string m_stName;
    std::vector<SLevelItemGroupInfo> m_vec_items;

public :
    CLevelItemGroup(uint32_t dwLevelLimit)
        : m_dwLevelLimit(dwLevelLimit)
    {
    }

    uint32_t GetLevelLimit() { return m_dwLevelLimit; }

    void AddItem(uint32_t dwItemVnum, uint32_t dwPct, int iCount)
    {
        m_vec_items.emplace_back(dwItemVnum, dwPct, iCount);
    }

    const std::vector<SLevelItemGroupInfo> &GetVector() { return m_vec_items; }
};

class CBuyerThiefGlovesItemGroup
{
    struct SThiefGroupInfo
    {
        uint32_t dwVnum;
        uint32_t dwPct;
        int iCount;

        SThiefGroupInfo(uint32_t dwVnum, uint32_t dwPct, int iCount)
            : dwVnum(dwVnum), dwPct(dwPct), iCount(iCount)
        {
        }
    };

public:
    CBuyerThiefGlovesItemGroup(uint32_t dwVnum, uint32_t dwMobVnum, const std::string &r_stName)
        : m_dwVnum(dwVnum),
          m_dwMobVnum(dwMobVnum),
          m_stName(r_stName)
    {
    }

    const std::vector<SThiefGroupInfo> &GetVector() { return m_vec_items; }

    void AddItem(uint32_t dwItemVnum, uint32_t dwPct, int iCount)
    {
        m_vec_items.push_back(SThiefGroupInfo(dwItemVnum, dwPct, iCount));
    }

private:
    uint32_t m_dwVnum;
    uint32_t m_dwMobVnum;
    std::string m_stName;
    std::vector<SThiefGroupInfo> m_vec_items;
};

class CItemDropInfo
{
public:
    CItemDropInfo(int iLevelStart, int iLevelEnd, int iPercent, uint32_t dwVnum, uint32_t count)
        : m_iLevelStart(iLevelStart)
          , m_iLevelEnd(iLevelEnd)
          , m_iPercent(iPercent)
          , m_dwVnum(dwVnum)
          , m_count(count)
    {
    }

    int m_iLevelStart;
    int m_iLevelEnd;
    int m_iPercent; // 1 ~ 1000
    uint32_t m_dwVnum;
    uint32_t m_count;

    friend bool operator <(const CItemDropInfo &l, const CItemDropInfo &r) { return l.m_iLevelEnd < r.m_iLevelEnd; }
};

class CAttackPetItemDropInfo
{
public:
    CAttackPetItemDropInfo(uint32_t itemVnum, uint32_t mobVnum, int32_t scalePlus, int32_t damagePlus, float dropPct)
        : m_itemVnum(itemVnum), m_mobVnum(mobVnum), m_scalePlus(scalePlus), m_damagePlus(damagePlus), m_dropPct(dropPct)
    {
    }

    uint32_t m_itemVnum = 0;
    uint32_t m_mobVnum = 0;
    int32_t m_scalePlus = 0;
    int32_t m_damagePlus = 0;
    float m_dropPct = 0;

    friend bool operator <(const CAttackPetItemDropInfo &l, const CAttackPetItemDropInfo &r)
    {
        return l.m_mobVnum < r.m_mobVnum;
    }
};

class CLevelPetItemDropInfo
{
public:
    CLevelPetItemDropInfo(uint32_t itemVnum, uint32_t mobVnum, float dropPct)
        : m_itemVnum(itemVnum), m_mobVnum(mobVnum), m_dropPct(dropPct)
    {
    }

    uint32_t m_itemVnum = 0;
    uint32_t m_mobVnum = 0;
    float m_dropPct = 0;

    friend bool operator <(const CLevelPetItemDropInfo &l, const CLevelPetItemDropInfo &r)
    {
        return l.m_mobVnum < r.m_mobVnum;
    }
};

struct SPetEvolInfo
{
    uint32_t evolType;
    uint32_t fromVnum;
    uint32_t toVnum;
};

using TPetEvolInfo = SPetEvolInfo;

class CItem;

class ITEM_MANAGER : public singleton<ITEM_MANAGER>
{
public:
    ITEM_MANAGER();
    virtual ~ITEM_MANAGER();

    bool Initialize();
    void Destroy();
    void Update(); // 매 루프마다 부른다.
    void GracefulShutdown();
    bool ReloadItemProto();

    uint32_t GetNewID();
    bool SetMaxItemID(const ItemIdRange &range); // 최대 고유 아이디를 지정
    bool SetMaxSpareItemID(const ItemIdRange &range);

    // DelayedSave: 어떠한 루틴 내에서 저장을 해야 할 짓을 많이 하면 저장
    // 쿼리가 너무 많아지므로 "저장을 한다" 라고 표시만 해두고 잠깐
    // (예: 1 frame) 후에 저장시킨다.
    void DelayedSave(CItem *item);
    void FlushDelayedSave(CItem *item); // Delayed 리스트에 있다면 지우고 저장한다. 끊김 처리시 사용 됨.
    void SaveSingleItem(CItem *item);

    CItem *CreateItem(uint32_t vnum, CountType count = 1, uint32_t dwID = 0, bool bTryMagic = false, int iRarePct = -1,
                      bool bSkipSave = false);
#ifndef DEBUG_ALLOC
    void DestroyItem(CItem *item);
    bool RemoveMapItem(uint32_t id);
#else
		void DestroyItem(CItem* item, const char* file, size_t line);
#endif
    void RemoveItem(CItem *item, const char *c_pszReason = nullptr); // 사용자로 부터 아이템을 제거

    CItem *Find(uint32_t id);
    CItem *FindByVID(uint32_t vid);
    const TItemTable *GetTable(uint32_t vnum);
    bool GetVnum(const char *c_pszName, uint32_t &r_dwVnum);
    const char *GetName(uint32_t vnum) const;
    bool GetVnumByOriginalName(const char *c_pszName, uint32_t &r_dwVnum);

    bool GetDropPct(CHARACTER *pkChr, CHARACTER *pkKiller, int &iDeltaPercent, int &iRandRange);
    bool GetDropPct(TMobTable *pkChr, int &iDeltaPercent, int &iRandRange);
    bool CreateDropItem(CHARACTER *pkChr, CHARACTER *pkKiller, std::vector<CItem *> &vec_item);

    bool       ReadCommonDropItemFile(const char * c_pszFileName);
    bool       ReadEnhanceTimeExeption(const char * c_pszFileName);
    bool       ReadLevelPetData(const char * c_pszFileName);
    bool       ReadAttackPetDropData(const char * c_pszFileName);
    bool       ReadDoubleItemAttrAllow(const char * c_pszFileName);
    ItemApply  GetLevelPetRandomBonus();
    uint32_t   GetPetNeededExpByLevel(Level lv);
    ApplyValue GetLevelPetBonusPerPoint(ApplyType t);
    ApplyValue GetLevelPetApplyDefault(ApplyType t);
    bool       IsDoubleApplyAllow(uint32_t apply);
    bool       ReadEtcDropItemFile(const char * c_pszFileName, bool isReloading = false);
    bool       ReadPetItemEvolutionTable(const char * c_pszFileName, bool isReloading);
    bool       ReadMonsterDropItemGroup(const char * c_pszFileName, bool isReloading = false);
    bool       IsEnhanceTimeExceptionItem(uint32_t vnum);
    bool       ReadSpecialDropItemFile(const char * c_pszFileName, bool isReloading = false);
    bool       ReadAddSocketBlock(const char* c_pszFileName, bool isReloading);

    uint32_t                      GetRefineFromVnum(uint32_t dwVnum);
    void                          RegisterHyperlink(const unsigned id, const ClientItemData& d);
    std::optional<ClientItemData> GetHyperlinkItemData(uint32_t id);

    static void CopyAllAttrTo(CItem *pkOldItem, CItem *pkNewItem); // pkNewItem으로 모든 속성과 소켓 값들을 목사하는 함수.

    void InitializeDropInfo();
    std::optional<std::reference_wrapper<TVecDropInfo>> FindDropInfo(uint32_t dwVNum);

    const CSpecialItemGroup *GetSpecialItemGroup(uint32_t dwVnum);
    const CSpecialAttrGroup *GetSpecialAttrGroup(uint32_t dwVnum);

    const ItemAttrProto &GetRareItemAttr(uint32_t apply);
    const ItemAttrProto &GetItemAttr(uint32_t apply);

    // CHECK_UNIQUE_GROUP
    int GetSpecialGroupFromItem(uint32_t dwVnum) const
    {
        auto it = m_ItemToSpecialGroup.find(dwVnum);
        return (it == m_ItemToSpecialGroup.end()) ? 0 : it->second;
    }

    // END_OF_CHECK_UNIQUE_GROUP

protected:
    void CreateQuestDropItem(CHARACTER *pkChr, CHARACTER *pkKiller, std::vector<CItem *> &vec_item, int iDeltaPercent,
                             int iRandRange);

protected:
    typedef robin_hood::unordered_map<uint32_t, CItem *> ITEM_VID_MAP;
    typedef robin_hood::unordered_map<uint32_t, std::string> NameMap;
    typedef std::unordered_map<uint32_t, ItemAttrProto> ItemAttrMap;

    using ProtoMap = std::unordered_map<int32_t, TItemTable>;
    ProtoMap m_itemProto;
    NameMap m_itemNames;

    bool LoadNames(const std::string &filename);

    std::map<uint32_t, uint32_t> m_map_ItemRefineFrom;
    int m_iTopOfTable;

#ifdef INGAME_WIKI
    std::map<uint32_t, std::unique_ptr<TWikiItemInfo>> m_wikiInfoMap;
    std::map<uint32_t, std::vector<TWikiItemOriginInfo>> m_itemOriginMap;
#endif

    ITEM_VID_MAP m_VIDMap; ///< m_dwVIDCount 의 값단위로 아이템을 저장한다.
    uint32_t m_dwVIDCount; ///< 이녀석 VID가 아니라 그냥 프로세스 단위 유니크 번호다.
    ItemIdRange m_ItemIDRange;
    ItemIdRange m_ItemIDSpareRange;

    std::unordered_set<CItem *> m_set_pkItemForDelayedSave;
    std::map<uint32_t, CItem *> m_map_pkItemByID;
    std::map<uint32_t, uint32_t> m_map_dwEtcItemDropProb;
    std::map<uint32_t, std::unique_ptr<CDropItemGroup>> m_map_pkDropItemGroup;
    std::map<uint32_t, std::unique_ptr<CSpecialItemGroup>> m_map_pkSpecialItemGroup;
    std::map<uint32_t, std::unique_ptr<CSpecialItemGroup>> m_map_pkQuestItemGroup;
    std::map<uint32_t, std::unique_ptr<CSpecialAttrGroup>> m_map_pkSpecialAttrGroup;
    std::map<uint32_t, std::unique_ptr<CMobItemGroup>> m_map_pkMobItemGroup;
    std::map<uint32_t, std::unique_ptr<CLevelItemGroup>> m_map_pkLevelItemGroup;
    std::unordered_map<uint32_t, TVecDropInfo> m_map_pkDropInfoByVNum;
    std::vector<TPetEvolInfo> m_map_petEvolutionFromToVnum;

    // CHECK_UNIQUE_GROUP
    std::map<uint32_t, int> m_ItemToSpecialGroup;
    // END_OF_CHECK_UNIQUE_GROUP
    std::vector<CItemDropInfo> m_commonDropItem[MOB_RANK_MAX_NUM];
    std::unordered_set<uint32_t> m_enhanceExceptionVnums;
    std::vector<uint32_t> m_doubleApplyTypeAllow;
    std::vector<CAttackPetItemDropInfo> m_attackPetItemDropInfos;
    std::vector<CLevelPetItemDropInfo> m_levelPetItemDropInfos;
    std::unordered_map<Level, uint32_t> m_levelPetExpTable;
    std::vector<ItemApply> m_levelPetAvailableBonus;
    std::unordered_map<ApplyType, float> m_levelPetBonusPerStatPoint;
    std::unordered_map<uint32_t, ClientItemData> m_hyperLinkItems;
    std::vector<ItemVnum> m_blockSocketAdd;

    ItemAttrMap m_itemAttr;
    ItemAttrMap m_itemAttrRare;
private:
    // 독일에서 기존 캐시 아이템과 같지만, 교환 가능한 캐시 아이템을 만든다고 하여,
    // 오리지널 아이템에 교환 금지 플래그만 삭제한 새로운 아이템들을 만들어,
    // 새로운 아이템 대역을 할당하였다.
    // 문제는 새로운 아이템도 오리지널 아이템과 같은 효과를 내야하는데,
    // 서버건, 클라건, vnum 기반으로 되어있어
    // 새로운 vnum을 죄다 서버에 새로 다 박아야하는 안타까운 상황에 맞닿았다.
    // 그래서 새 vnum의 아이템이면, 서버에서 돌아갈 때는 오리지널 아이템 vnum으로 바꿔서 돌고 하고,
    // 저장할 때에 본래 vnum으로 바꿔주도록 한다.
    // 이를 위해 오리지널 vnum과 새로운 vnum을 연결시켜주는 맵을 만듦.
    typedef std::map<uint32_t, uint32_t> TMapDW2DW;
    TMapDW2DW m_map_new_to_ori;

    TMapDW2DW m_map_ori_to_debuffed;

public:
    uint32_t GetMaskVnum(uint32_t dwVnum);

    std::map<uint32_t, std::unique_ptr<CMobItemGroup>> &GetMapMobItemGroup() { return m_map_pkMobItemGroup; }

    std::map<uint32_t, std::unique_ptr<CLevelItemGroup>> &GetMapMobLevelItemGroup() { return m_map_pkLevelItemGroup; }

    std::map<uint32_t, std::unique_ptr<CDropItemGroup>> &GetMapDropItemGroup() { return m_map_pkDropItemGroup; }

    uint32_t GetPetItemEvolvedByVnum(uint32_t evolType, uint32_t vnum);
    uint32_t GetItemDebuffVnum(uint32_t vnum);

    bool ReadItemVnumMaskTable(const char *c_pszFileName);
    bool ReadItemDebuffTable(const char *c_pszFileName);
    bool IsSocketAddBlocked(ItemVnum vnum) { return std::find(m_blockSocketAdd.begin(), m_blockSocketAdd.end(), vnum) != m_blockSocketAdd.end(); }
#ifdef INGAME_WIKI
    void BuildWikiInfoForVnum(uint32_t vnum);
    TWikiItemInfo *GetItemWikiInfo(uint32_t vnum);

    std::vector<TWikiItemOriginInfo> &GetItemOrigin(uint32_t vnum) { return m_itemOriginMap[vnum]; }
#endif

private:
#ifdef M2_USE_POOL
		ObjectPool<CItem> pool_;
#endif

    bool m_isInitialized = false;
};

#ifndef DEBUG_ALLOC
#define M2_DESTROY_ITEM(ptr) ITEM_MANAGER::instance().DestroyItem(ptr)
#else
#define M2_DESTROY_ITEM(ptr) ITEM_MANAGER::instance().DestroyItem(ptr, __FILE__, __LINE__)
#endif

#endif /* METIN2_SERVER_GAME_ITEM_MANAGER_H */
