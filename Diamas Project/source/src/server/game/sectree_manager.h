#ifndef METIN2_SERVER_GAME_SECTREE_MANAGER_H
#define METIN2_SERVER_GAME_SECTREE_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "sectree.h"
#include "vid.h"
#include <game/length.h>
#include <base/robin_hood.h>

// Generic finder for all players in a given sectree
struct FCharacterFinder
{
    std::vector<CHARACTER *> *playerList;

    FCharacterFinder(std::vector<CHARACTER *> &list) { playerList = &list; }

    void operator()(CEntity *ent)
    {
        if (ent && ent->IsType(ENTITY_CHARACTER))
            playerList->push_back((CHARACTER *)ent);
    }
};

class CHARACTER;

typedef struct SMapRegion
{
    int32_t index;
    int32_t width, height;
    PIXEL_POSITION posSpawn;

    bool bEmpireSpawnDifferent;
    PIXEL_POSITION posEmpire[3];

    std::string strMapName;
} TMapRegion;

struct TAreaInfo
{
    int32_t sx, sy, ex, ey, dir;

    TAreaInfo(int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t dir)
        : sx(sx)
          , sy(sy)
          , ex(ex)
          , ey(ey)
          , dir(dir)
    {
    }
};

struct npc_info
{
    uint8_t bType;
    uint32_t vnum;
    std::string name;
    int32_t x, y;

    npc_info(uint8_t bType, uint32_t vnum, std::string name, int32_t x, int32_t y)
        : bType(bType)
          , vnum(vnum), name(name)
          , x(x)
          , y(y)
    {
    }
};

typedef robin_hood::unordered_map<std::string, TAreaInfo> TAreaMap;

typedef struct SSetting
{
    int32_t iIndex;
    int32_t iCellScale;
    int32_t iWidth;
    int32_t iHeight;

    PIXEL_POSITION posSpawn;
} TMapSetting;

class SECTREE_MAP
{
public:
    typedef robin_hood::unordered_map<uint32_t, std::unique_ptr<SECTREE>> MapType;

    SECTREE_MAP();
    SECTREE_MAP(const SECTREE_MAP &r);

    void Add(uint32_t key, std::unique_ptr<SECTREE> sectree);

    SECTREE *Find(uint32_t dwPackage) const;
    SECTREE *Find(uint32_t x, uint32_t y) const;
    void Build();
    bool IsRegionValid(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) const;
    bool HasAttrInRegion(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t attr) const;

    TMapSetting m_setting{};

    template <typename Func>
    void for_each(Func &&rfunc)
    {
        // <Factor> Using snapshot copy to avoid side-effects
        // TODO: Attempt to remove this soon!
        FCollectEntity collector;
        for (auto &it : map_)
            it.second->for_each_entity(collector);

        collector.ForEach(rfunc);
    }

    void DumpAllToSysErr()
    {
        for (auto &i : map_)
        {
            SPDLOG_ERROR("SECTREE {0}({1}, {2})", i.first,
                         i.first & 0xffff, i.first >> 16);
        }
    }

private:
    MapType map_;
};

enum EAttrRegionMode
{
    ATTR_REGION_MODE_SET,
    ATTR_REGION_MODE_REMOVE,
    ATTR_REGION_MODE_CHECK,
};

class SECTREE_MANAGER : public singleton<SECTREE_MANAGER>
{
public:

    SECTREE_MAP *GetMap(int32_t lMapIndex);
    SECTREE *Get(uint32_t dwIndex, uint32_t package);
    SECTREE *Get(uint32_t dwIndex, uint32_t x, uint32_t y);

    template <typename Func>
    void for_each(int iMapIndex, Func &&rfunc)
    {
        SECTREE_MAP *pSecMap = GetMap(iMapIndex);
        if (pSecMap)
            pSecMap->for_each(rfunc);
    }

    bool Initialize();

    int LoadSettingFile(int32_t lIndex, const char *c_pszSettingFileName, TMapSetting &r_setting);
    bool LoadMapRegion(const char *c_pszFileName, TMapSetting &r_Setting, const char *c_pszMapName);
    SECTREE_MAP *BuildSectreeFromSetting(TMapSetting &r_setting);
    bool LoadAttribute(SECTREE_MAP *pkMapSectree, const char *c_pszFileName, TMapSetting &r_setting);
    void LoadDungeon(int iIndex, const char *c_pszFileName);
    bool GetValidLocation(int32_t lMapIndex, int32_t x, int32_t y, int32_t &r_lValidMapIndex, PIXEL_POSITION &r_pos,
                          uint8_t empire = 0);
    bool GetSpawnPositionByMapIndex(int32_t lMapIndex, PIXEL_POSITION &r_pos);
    bool GetRecallPositionByEmpire(int iMapIndex, uint8_t bEmpire, PIXEL_POSITION &r_pos);

    const TMapRegion *GetMapRegion(int32_t lMapIndex);
    const TMapRegion *FindRegionByPartialName(const char *szMapName);

    bool GetMovablePosition(int32_t lMapIndex, int32_t x, int32_t y, PIXEL_POSITION &pos);
    bool IsMovablePosition(int32_t lMapIndex, int32_t x, int32_t y);
    bool GetCenterPositionOfMap(int32_t lMapIndex, PIXEL_POSITION &r_pos);
    bool GetRandomLocation(int32_t lMapIndex, PIXEL_POSITION &r_pos, uint32_t dwCurrentX = 0, uint32_t dwCurrentY = 0,
                           int iMaxDistance = 0);

    int32_t CreatePrivateMap(int32_t lMapIndex); // returns new private map index, returns 0 when fail
    void DestroyPrivateMap(int32_t lMapIndex);

    const TAreaMap *GetDungeonArea(int32_t lMapIndex);
    void SendNPCPosition(CHARACTER *ch);
    void InsertNPCPosition(int32_t lMapIndex, uint32_t vnum, uint8_t bType, std::string szName, int32_t x, int32_t y);

    uint8_t GetEmpireFromMapIndex(int32_t lMapIndex);

    void PurgeMonstersInMap(int32_t lMapIndex);
    void PurgeStonesInMap(int32_t lMapIndex);
    void PurgeNPCsInMap(int32_t lMapIndex);
    size_t GetMonsterCountInMap(int32_t lMapIndex);
#ifdef ENABLE_MELEY_LAIR_DUNGEON
    size_t GetMonsterCountInMap(long lMapIndex, uint32_t dwVnum, bool bForceVnum = false);
#else
	size_t	GetMonsterCountInMap(long lMapIndex, uint32_t dwVnum);
#endif

    /// 영역에 대해 Sectree 의 Attribute 에 대해 특정한 처리를 수행한다.
    /**
    * @param [in]	mapIndex 적용할 Map index
    * @param [in]	sx 사각형 영역의 가장 왼쪽 좌표
    * @param [in]	sy 사각형 영역의 가장 위쪽 좌표
    * @param [in]	ex 사각형 영역의 가장 오른쪽 좌표
    * @param [in]	ey 사각형 영역의 가장 아랫쪽 좌표
    * @param [in]	lRotate 영역에 대해 회전할 각
    * @param [in]	attr 적용할 Attribute
    * @param [in]	mode Attribute 에 대해 처리할 type
    */
    bool ForAttrRegion(int32_t mapIndex, int32_t sx, int32_t sy, int32_t ex, int32_t ey, float xRot, float yRot,
                       float zRot, uint32_t attr, EAttrRegionMode mode);

private:

    /// 직각의 사각형 영역에 대해 Sectree 의 Attribute 에 대해 특정한 처리를 수행한다.
    /**
    * @param [in]	lMapIndex 적용할 Map index
    * @param [in]	lCX 사각형 영역의 가장 왼쪽 Cell 의 좌표
    * @param [in]	lCY 사각형 영역의 가장 위쪽 Cell 의 좌표
    * @param [in]	lCW 사각형 영역의 Cell 단위 폭
    * @param [in]	lCH 사각형 영역의 Cell 단위 높이
    * @param [in]	lRotate 회전할 각(직각)
    * @param [in]	dwAttr 적용할 Attribute
    * @param [in]	mode Attribute 에 대해 처리할 type
    */
    bool ForAttrRegionRightAngle(int32_t lMapIndex, int32_t lCX, int32_t lCY, int32_t lCW, int32_t lCH, int32_t lRotate,
                                 uint32_t dwAttr, EAttrRegionMode mode);

    /// 직각 이외의 사각형 영역에 대해 Sectree 의 Attribute 에 대해 특정한 처리를 수행한다.
    /**
    * @param [in]	lMapIndex 적용할 Map index
    * @param [in]	lCX 사각형 영역의 가장 왼쪽 Cell 의 좌표
    * @param [in]	lCY 사각형 영역의 가장 위쪽 Cell 의 좌표
    * @param [in]	lCW 사각형 영역의 Cell 단위 폭
    * @param [in]	lCH 사각형 영역의 Cell 단위 높이
    * @param [in]	lRotate 회전할 각(직각 이외의 각)
    * @param [in]	dwAttr 적용할 Attribute
    * @param [in]	mode Attribute 에 대해 처리할 type
    */
    bool ForAttrRegionFreeAngle(int32_t lMapIndex, int32_t lCX, int32_t lCY, int32_t lCW, int32_t lCH, int32_t lRotate,
                                uint32_t dwAttr, EAttrRegionMode mode);

    /// 한 Cell 의 Attribute 에 대해 특정한 처리를 수행한다.
    /**
    * @param [in]	lMapIndex 적용할 Map index
    * @param [in]	lCX 적용할 Cell 의 X 좌표
    * @param [in]	lCY 적용할 Cell 의 Y 좌표
    * @param [in]	dwAttr 적용할 Attribute
    * @param [in]	mode Attribute 에 대해 처리할 type
    */
    bool ForAttrRegionCell(int32_t lMapIndex, int32_t lCX, int32_t lCY, uint32_t dwAttr, EAttrRegionMode mode);

    robin_hood::unordered_map<uint32_t, std::unique_ptr<SECTREE_MAP>> m_map_pkSectree;
    robin_hood::unordered_map<int, TAreaMap> m_map_pkArea;
    std::vector<TMapRegion> m_vec_mapRegion;
    robin_hood::unordered_map<uint32_t, std::vector<npc_info>> m_mapNPCPosition;

    // <Factor> Circular private map indexing
    typedef robin_hood::unordered_map<int32_t, int> PrivateIndexMapType;
    PrivateIndexMapType next_private_index_map_;
};

#endif /* METIN2_SERVER_GAME_SECTREE_MANAGER_H */
