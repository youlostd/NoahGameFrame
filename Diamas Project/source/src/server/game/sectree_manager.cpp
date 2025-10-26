#include "config.h"
#include "utils.h"
#include "sectree_manager.h"
#include "regen.h"
#include "lzo_manager.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "GBufferManager.h"
#include <game/GamePacket.hpp>
#include "start_position.h"
#include "roundalgo.h"
#include "vector.h"
#include <lzo/lzo1x.h>
#include <storm/Util.hpp>

#include <game/attribute.h>
#include <thecore/stdafx.h>

SECTREE_MAP::SECTREE_MAP()
{
}

SECTREE_MAP::SECTREE_MAP(const SECTREE_MAP &r)
{
    m_setting = r.m_setting;

    for (const auto &p : r.map_)
    {
        auto tree = std::make_unique<SECTREE>();

        tree->m_id.coord = p.second->m_id.coord;
        tree->CloneAttribute(p.second.get());

        map_.emplace(p.first, std::move(tree));
    }

    Build();
}

void SECTREE_MAP::Add(uint32_t key, std::unique_ptr<SECTREE> sectree)
{
    auto r = map_.emplace(key, std::move(sectree));

    if (!r.second)
    {
        SPDLOG_ERROR("Duplicate sectree for {0} on {1}",
                     key, m_setting.iIndex);
    }
}

SECTREE *SECTREE_MAP::Find(uint32_t dwPackage) const
{
    const auto it = map_.find(dwPackage);
    if (it == map_.end())
        return nullptr;

    return it->second.get();
}

SECTREE *SECTREE_MAP::Find(uint32_t x, uint32_t y) const
{
    SECTREEID id;
    id.coord.x = x / SECTREE_SIZE;
    id.coord.y = y / SECTREE_SIZE;
    return Find(id.package);
}

void SECTREE_MAP::Build()
{
    struct neighbor_coord_s
    {
        int x;
        int y;
    } neighbor_coord[8] = {
            {-SECTREE_SIZE, 0},
            {SECTREE_SIZE, 0},
            {0, -SECTREE_SIZE},
            {0, SECTREE_SIZE},
            {-SECTREE_SIZE, SECTREE_SIZE},
            {SECTREE_SIZE, -SECTREE_SIZE},
            {-SECTREE_SIZE, -SECTREE_SIZE},
            {SECTREE_SIZE, SECTREE_SIZE},
        };

    //
    // 모든 sectree에 대해 주위 sectree들 리스트를 만든다.
    //

    for (auto &p : map_)
    {
        const auto &tree = p.second;

        // Bind default attribute in case we didn't load one...
        if (!tree->GetAttributePtr())
            tree->BindAttribute(new CAttribute(SECTREE_SIZE / CELL_SIZE,
                                               SECTREE_SIZE / CELL_SIZE));

        tree->m_neighbor_list.push_back(tree.get()); // 자신을 넣는다.

        SPDLOG_TRACE("{0}x{1}", tree->m_id.coord.x, tree->m_id.coord.y);

        int x = tree->m_id.coord.x * SECTREE_SIZE;
        int y = tree->m_id.coord.y * SECTREE_SIZE;

        for (uint32_t i = 0; i < STORM_ARRAYSIZE(neighbor_coord); ++i)
        {
            auto tree2 = Find(x + neighbor_coord[i].x, y + neighbor_coord[i].y);
            if (tree2)
            {
                SPDLOG_TRACE("   {0} {1}x{2}", i, tree2->m_id.coord.x, tree2->m_id.coord.y);
                tree->m_neighbor_list.push_back(tree2);
            }
        }
    }
}

bool SECTREE_MAP::IsRegionValid(uint32_t x1, uint32_t y1,
                                uint32_t x2, uint32_t y2) const
{
    if (x1 > x2)
        std::swap(x1, x2);

    if (y1 > y2)
        std::swap(y1, y2);

    for (uint32_t y = y1; y <= y2; y += SECTREE_SIZE)
    {
        for (uint32_t x = x1; x <= x2; x += SECTREE_SIZE)
        {
            auto tree = Find(x, y);
            if (!tree)
                return false;
        }
    }

    return true;
}

bool SECTREE_MAP::HasAttrInRegion(uint32_t x1, uint32_t y1,
                                  uint32_t x2, uint32_t y2,
                                  uint32_t attr) const
{
    if (x1 > x2)
        std::swap(x1, x2);

    if (y1 > y2)
        std::swap(y1, y2);

    for (uint32_t y = y1; y <= y2; y += CELL_SIZE)
    {
        for (uint32_t x = x1; x <= x2; x += CELL_SIZE)
        {
            auto tree = Find(x, y);
            if (!tree)
                continue;

            if (tree->IsAttr(x, y, attr))
                return true;
        }
    }

    return false;
}

SECTREE_MAP *SECTREE_MANAGER::GetMap(int32_t lMapIndex)
{
    auto it = m_map_pkSectree.find(lMapIndex);
    if (it == m_map_pkSectree.end())
        return nullptr;

    return it->second.get();
}

SECTREE *SECTREE_MANAGER::Get(uint32_t dwIndex, uint32_t package)
{
    SECTREE_MAP *pkSectreeMap = GetMap(dwIndex);
    if (!pkSectreeMap)
        return nullptr;

    return pkSectreeMap->Find(package);
}

SECTREE *SECTREE_MANAGER::Get(uint32_t dwIndex, uint32_t x, uint32_t y)
{
    SECTREEID id;
    id.coord.x = x / SECTREE_SIZE;
    id.coord.y = y / SECTREE_SIZE;
    return Get(dwIndex, id.package);
}

bool SECTREE_MANAGER::Initialize()
{
    FILE *fp = fopen("data/map/index", "r");
    if (nullptr == fp)
        return false;

    char buf[256 + 1];
    uint32_t line = 0;

    while (fgets(buf, 256, fp))
    {
        ++line;

        if (!strncmp(buf, "//", 2) || *buf == '#' || *buf == '\n' || *buf == '\r')
            continue;

        char szMapName[256];
        int iIndex;

        if (sscanf(buf, " %d %s ", &iIndex, szMapName) != 2)
        {
            SPDLOG_ERROR("Map-list line {0} is malformed", line);
            continue;
        }

        char szFilename[256];
        std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/Setting.txt", szMapName);

        TMapSetting setting;
        setting.iIndex = iIndex;

        if (!LoadSettingFile(iIndex, szFilename, setting))
        {
            SPDLOG_ERROR("can't load file {0} in LoadSettingFile", szFilename);
            fclose(fp);
            return 0;
        }

        std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/Town.txt", szMapName);
        if (!LoadMapRegion(szFilename, setting, szMapName))
        {
            SPDLOG_ERROR("can't load file {0} in LoadMapRegion", szFilename);
            fclose(fp);
            return 0;
        }

        // 먼저 이 서버에서 이 맵의 몬스터를 스폰해야 하는가 확인 한다.
        if (gConfig.IsHostingMap(iIndex))
        {
            auto pkMapSectree = BuildSectreeFromSetting(setting);
            m_map_pkSectree.emplace(iIndex, std::unique_ptr<SECTREE_MAP>(pkMapSectree));

            std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/server_attr", szMapName);
            LoadAttribute(pkMapSectree, szFilename, setting);
            std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/", szMapName);
            regen_register_map(szFilename, setting.iIndex);
            pkMapSectree->Build();

            std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/regen.txt", szMapName);
            regen_load(szFilename, setting.iIndex);

            std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/npc.txt", szMapName);
            regen_load(szFilename, setting.iIndex);

            std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/boss.txt", szMapName);
            regen_load(szFilename, setting.iIndex);

            std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/stone.txt", szMapName);
            regen_load(szFilename, setting.iIndex);

            std::snprintf(szFilename, sizeof(szFilename), "data/map/%s/dungeon.txt", szMapName);
            LoadDungeon(iIndex, szFilename);
        }
    }

    fclose(fp);

    return 1;
}

// -----------------------------------------------------------------------------
// Setting.txt 로 부터 SECTREE 만들기
// -----------------------------------------------------------------------------
int SECTREE_MANAGER::LoadSettingFile(int32_t lMapIndex, const char *c_pszSettingFileName, TMapSetting &r_setting)
{
    memset(&r_setting, 0, sizeof(TMapSetting));

    FILE *fp = fopen(c_pszSettingFileName, "r");
    if (!fp)
    {
        SPDLOG_ERROR("cannot open file: {0}", c_pszSettingFileName);
        return 0;
    }

    char buf[256], cmd[256];
    int iWidth = 0, iHeight = 0;

    while (fgets(buf, 256, fp))
    {
        sscanf(buf, " %s ", cmd);

        if (!strcasecmp(cmd, "MapSize")) { sscanf(buf, " %s %d %d ", cmd, &iWidth, &iHeight); }
        else if (!strcasecmp(cmd, "CellScale")) { sscanf(buf, " %s %d ", cmd, &r_setting.iCellScale); }
    }

    fclose(fp);

    if ((iWidth == 0 && iHeight == 0) || r_setting.iCellScale == 0)
    {
        SPDLOG_ERROR("Invalid Settings file: {0}", c_pszSettingFileName);
        return 0;
    }

    r_setting.iIndex = lMapIndex;
    r_setting.iWidth = (r_setting.iCellScale * 128 * iWidth);
    r_setting.iHeight = (r_setting.iCellScale * 128 * iHeight);
    return 1;
}

SECTREE_MAP *SECTREE_MANAGER::BuildSectreeFromSetting(TMapSetting &r_setting)
{
    SECTREE_MAP *pkMapSectree = new SECTREE_MAP;
    pkMapSectree->m_setting = r_setting;

    int w = vstd::round_up<int32_t>(r_setting.iWidth, SECTREE_SIZE);
    int h = vstd::round_up<int32_t>(r_setting.iHeight, SECTREE_SIZE);

    for (int x = 0; x != w; x += SECTREE_SIZE)
    {
        for (int y = 0; y != h; y += SECTREE_SIZE)
        {
            auto tree = std::make_unique<SECTREE>();

            tree->m_id.coord.x = x / SECTREE_SIZE;
            tree->m_id.coord.y = y / SECTREE_SIZE;

            // Temporary is necessary since we move |tree|
            const auto package = tree->m_id.package;
            pkMapSectree->Add(package, std::move(tree));
        }
    }

    return pkMapSectree;
}

void SECTREE_MANAGER::LoadDungeon(int iIndex, const char *c_pszFileName)
{
    FILE *fp = fopen(c_pszFileName, "r");
    if (!fp)
        return;

    int count = 0; // for debug

    while (!feof(fp))
    {
        char buf[1024];

        if (nullptr == fgets(buf, 1024, fp))
            break;

        if (buf[0] == '#' || (buf[0] == '/' && buf[1] == '/'))
            continue;

        int x, y, sx, sy, dir;
        char position_name[1024];

        sscanf(buf, " %s %d %d %d %d %d ",
               position_name, &x, &y, &sx, &sy, &dir);

        x -= sx;
        y -= sy;
        sx *= 2;
        sy *= 2;
        sx += x;
        sy += y;

        m_map_pkArea[iIndex].emplace(position_name, TAreaInfo(x, y, sx, sy, dir));

        count++;
    }

    fclose(fp);

    SPDLOG_TRACE("Dungeon Position Load [%d]%d count %d", iIndex, c_pszFileName, count);
}

// Fix me
// 현재 Town.txt에서 x, y를 그냥 받고, 그걸 이 코드 내에서 base 좌표를 더해주기 때문에
// 다른 맵에 있는 타운으로 절대 이동할 수 없게 되어있다.
// 앞에 map이라거나, 기타 다른 식별자가 있으면,
// 다른 맵의 타운으로도 이동할 수 있게 하자.
// by rtsummit
bool SECTREE_MANAGER::LoadMapRegion(const char *c_pszFileName, TMapSetting &r_setting, const char *c_pszMapName)
{
    FILE *fp = fopen(c_pszFileName, "r");

    if (!fp)
        return false;

    int iX = 0, iY = 0;
    PIXEL_POSITION pos[3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    fscanf(fp, " %d %d ", &iX, &iY);

    int iEmpirePositionCount = fscanf(fp, " %d %d %d %d %d %d ",
                                      &pos[0].x, &pos[0].y,
                                      &pos[1].x, &pos[1].y,
                                      &pos[2].x, &pos[2].y);

    fclose(fp);

    TMapRegion region;

    region.index = r_setting.iIndex;
    region.width = r_setting.iWidth;
    region.height = r_setting.iHeight;

    region.strMapName = c_pszMapName;

    region.posSpawn.x = (iX * 100);
    region.posSpawn.y = (iY * 100);
    region.posSpawn.z = 0;

    r_setting.posSpawn = region.posSpawn;

    SPDLOG_TRACE("LoadMapRegion {0} - {1}x{2}, town {3} {4}",
                region.index,
                region.width,
                region.height,
                region.posSpawn.x,
                region.posSpawn.y);

    if (iEmpirePositionCount == 6)
    {
        region.bEmpireSpawnDifferent = true;

        for (int i = 0; i < 3; i++)
        {
            region.posEmpire[i].x = (pos[i].x * 100);
            region.posEmpire[i].y = (pos[i].y * 100);
        }
    }
    else { region.bEmpireSpawnDifferent = false; }

    m_vec_mapRegion.push_back(region);
    return true;
}

bool SECTREE_MANAGER::LoadAttribute(SECTREE_MAP *pkMapSectree, const char *c_pszFileName, TMapSetting &r_setting)
{
    FILE *fp = fopen(c_pszFileName, "rb");
    if (!fp)
    {
        SPDLOG_ERROR("SECTREE_MANAGER::LoadAttribute : cannot open {0}", c_pszFileName);
        return false;
    }

    int iWidth, iHeight;

    fread(&iWidth, sizeof(int32_t), 1, fp);
    fread(&iHeight, sizeof(int32_t), 1, fp);

    int maxMemSize = LZOManager::instance().GetMaxCompressedSize(
        sizeof(uint32_t) * (SECTREE_SIZE / CELL_SIZE) * (SECTREE_SIZE / CELL_SIZE));

    unsigned int uiSize;

#ifndef _MSC_VER
	uint8_t abComp[maxMemSize];
#else
    uint8_t *abComp = new uint8_t[maxMemSize];
#endif

    uint32_t *attr = new uint32_t[maxMemSize];

    for (int y = 0; y < iHeight; ++y)
    {
        for (int x = 0; x < iWidth; ++x)
        {
            // UNION 으로 좌표를 합쳐만든 uint32_t값을 아이디로 사용한다.
            SECTREEID id;
            id.coord.x = x;
            id.coord.y = y;

            SECTREE *tree = pkMapSectree->Find(id.package);

            // SERVER_ATTR_LOAD_ERROR
            if (tree == nullptr)
            {
               //SPDLOG_CRITICAL(
               //    "FATAL ERROR! LoadAttribute({0}) - cannot find sectree(package={1}, coord=({2}, {3}), map_index={4})",
               //    c_pszFileName, id.package, id.coord.x, id.coord.y, r_setting.iIndex);
                SPDLOG_CRITICAL("ERROR_ATTR_POS({0}, {1}) attr_size({2}, {3})", x, y, iWidth, iHeight);
                SPDLOG_CRITICAL("CHECK! 'Setting.txt' and 'server_attr' MAP_SIZE!!");

                pkMapSectree->DumpAllToSysErr();

                delete[] attr;
#ifdef _MSC_VER
                delete[] abComp;
#endif
                return false;
            }
            // END_OF_SERVER_ATTR_LOAD_ERROR

            if (tree->m_id.package != id.package)
            {
                SPDLOG_ERROR("returned tree id mismatch! return {0}, request {1}",
                             tree->m_id.package, id.package);
                fclose(fp);

                delete[] attr;
#ifdef _MSC_VER
                delete[] abComp;
#endif
                return false;
            }

            fread(&uiSize, sizeof(int32_t), 1, fp);
            fread(abComp, sizeof(int8_t), uiSize, fp);

            //LZOManager::instance().Decompress(abComp, uiSize, (uint8_t *) tree->GetAttributePointer(), &uiDestSize);
            lzo_uint uiDestSize = sizeof(uint32_t) * maxMemSize;
            LZOManager::instance().Decompress(abComp, uiSize, (uint8_t *)attr, &uiDestSize);

            if (uiDestSize != sizeof(uint32_t) * (SECTREE_SIZE / CELL_SIZE) * (SECTREE_SIZE / CELL_SIZE))
            {
               //SPDLOG_ERROR("SECTREE_MANAGER::LoadAttribte : {0} : {1} {2} size mismatch! {3}",
               //             c_pszFileName, tree->m_id.coord.x, tree->m_id.coord.y, uiDestSize);
                fclose(fp);

                delete[] attr;
#ifdef _MSC_VER
                delete[] abComp;
#endif
                return false;
            }

            tree->BindAttribute(new CAttribute(attr, SECTREE_SIZE / CELL_SIZE, SECTREE_SIZE / CELL_SIZE));
        }
    }

    fclose(fp);

    delete[] attr;
#ifdef _MSC_VER
    delete[] abComp;
#endif
    return true;
}

bool SECTREE_MANAGER::GetRecallPositionByEmpire(int iMapIndex, uint8_t bEmpire, PIXEL_POSITION &r_pos)
{
    std::vector<TMapRegion>::iterator it = m_vec_mapRegion.begin();

    // 10000을 넘는 맵은 인스턴스 던전에만 한정되어있다.
    if (iMapIndex >= 10000) { iMapIndex /= 10000; }

    while (it != m_vec_mapRegion.end())
    {
        TMapRegion &rRegion = *(it++);

        if (rRegion.index == iMapIndex)
        {
            if (rRegion.bEmpireSpawnDifferent && bEmpire >= 1 && bEmpire <= 3)
                r_pos = rRegion.posEmpire[bEmpire - 1];
            else
                r_pos = rRegion.posSpawn;

            return true;
        }
    }

    return false;
}

bool SECTREE_MANAGER::GetCenterPositionOfMap(int32_t lMapIndex, PIXEL_POSITION &r_pos)
{
    std::vector<TMapRegion>::iterator it = m_vec_mapRegion.begin();

    while (it != m_vec_mapRegion.end())
    {
        TMapRegion &rRegion = *(it++);

        if (rRegion.index == lMapIndex)
        {
            r_pos.x = rRegion.width / 2;
            r_pos.y = rRegion.height / 2;
            r_pos.z = 0;
            return true;
        }
    }

    return false;
}

bool SECTREE_MANAGER::GetSpawnPositionByMapIndex(int32_t lMapIndex, PIXEL_POSITION &r_pos)
{
    if (lMapIndex > 10000)
        lMapIndex /= 10000;
    std::vector<TMapRegion>::iterator it = m_vec_mapRegion.begin();

    while (it != m_vec_mapRegion.end())
    {
        TMapRegion &rRegion = *(it++);

        if (lMapIndex == rRegion.index)
        {
            r_pos = rRegion.posSpawn;
            return true;
        }
    }

    return false;
}

const TMapRegion *SECTREE_MANAGER::FindRegionByPartialName(const char *szMapName)
{
    std::vector<TMapRegion>::iterator it = m_vec_mapRegion.begin();

    while (it != m_vec_mapRegion.end())
    {
        TMapRegion &rRegion = *(it++);

        //if (rRegion.index == lMapIndex)
        //return &rRegion;
        if (rRegion.strMapName.find(szMapName))
            return &rRegion; // 캐싱 해서 빠르게 하자
    }

    return nullptr;
}

const TMapRegion *SECTREE_MANAGER::GetMapRegion(int32_t lMapIndex)
{
    std::vector<TMapRegion>::iterator it = m_vec_mapRegion.begin();

    while (it != m_vec_mapRegion.end())
    {
        TMapRegion &rRegion = *(it++);

        if (rRegion.index == lMapIndex)
            return &rRegion;
    }

    return nullptr;
}

bool SECTREE_MANAGER::IsMovablePosition(int32_t lMapIndex, int32_t x, int32_t y)
{
    SECTREE *tree = Get(lMapIndex, x, y);
    if (!tree)
        return false;

    return (!tree->IsAttr(x, y, ATTR_BLOCK | ATTR_OBJECT));
}

bool SECTREE_MANAGER::GetMovablePosition(int32_t lMapIndex, int32_t x, int32_t y, PIXEL_POSITION &pos)
{
    int i = 0;

    do
    {
        int32_t dx = x + aArroundCoords[i].x;
        int32_t dy = y + aArroundCoords[i].y;

        SECTREE *tree = Get(lMapIndex, dx, dy);

        if (!tree)
            continue;

        if (!tree->IsAttr(dx, dy, ATTR_BLOCK | ATTR_OBJECT))
        {
            pos.x = dx;
            pos.y = dy;
            return true;
        }
    }
    while (++i < ARROUND_COORD_MAX_NUM);

    pos.x = x;
    pos.y = y;
    return false;
}

bool SECTREE_MANAGER::GetValidLocation(int32_t lMapIndex, int32_t x, int32_t y,
                                       int32_t &r_lValidMapIndex,
                                       PIXEL_POSITION &r_pos, uint8_t empire)
{
    SECTREE_MAP *pkSectreeMap = GetMap(lMapIndex);
    if (!pkSectreeMap)
    {
        if (lMapIndex >= 10000) { return GetValidLocation(lMapIndex / 10000, x, y, r_lValidMapIndex, r_pos); }
        else
        {
            SPDLOG_ERROR("cannot find sectree_map by map index {}", lMapIndex);
            return false;
        }
    }

    r_lValidMapIndex = lMapIndex;

    if (pkSectreeMap->Find(x, y))
    {
        r_pos.x = x;
        r_pos.y = y;
        return true;
    }

    if (GetRecallPositionByEmpire(lMapIndex, empire, r_pos))
        return true;

    SPDLOG_ERROR("invalid location (map index %d %d x %d)", lMapIndex, x, y);
    return false;
}

bool SECTREE_MANAGER::GetRandomLocation(int32_t lMapIndex, PIXEL_POSITION &r_pos, uint32_t dwCurrentX,
                                        uint32_t dwCurrentY, int iMaxDistance)
{
    SECTREE_MAP *pkSectreeMap = GetMap(lMapIndex);
    if (!pkSectreeMap)
        return false;

    uint32_t x, y;

    std::vector<TMapRegion>::iterator it = m_vec_mapRegion.begin();

    while (it != m_vec_mapRegion.end())
    {
        TMapRegion &rRegion = *(it++);

        if (rRegion.index != lMapIndex)
            continue;

        int i = 0;

        while (i++ < 100)
        {
            x = Random::get(50, rRegion.width - 50);
            y = Random::get(50, rRegion.height - 50);

            if (iMaxDistance != 0)
            {
                int d;

                d = abs((float)dwCurrentX - x);

                if (d > iMaxDistance)
                {
                    if (x < dwCurrentX)
                        x = dwCurrentX - iMaxDistance;
                    else
                        x = dwCurrentX + iMaxDistance;
                }

                d = abs((float)dwCurrentY - y);

                if (d > iMaxDistance)
                {
                    if (y < dwCurrentY)
                        y = dwCurrentY - iMaxDistance;
                    else
                        y = dwCurrentY + iMaxDistance;
                }
            }

            SECTREE *tree = pkSectreeMap->Find(x, y);

            if (!tree)
                continue;

            if (tree->IsAttr(x, y, ATTR_BLOCK | ATTR_OBJECT))
                continue;

            r_pos.x = x;
            r_pos.y = y;
            return true;
        }
    }

    return false;
}

int32_t SECTREE_MANAGER::CreatePrivateMap(int32_t lMapIndex)
{
    if (lMapIndex >= 10000) // 10000번 이상의 맵은 없다. (혹은 이미 private 이다)
        return 0;

    SECTREE_MAP *pkMapSectree = GetMap(lMapIndex);
    if (!pkMapSectree)
    {
        SPDLOG_ERROR("Cannot find map index {}", lMapIndex);
        return 0;
    }

    // <Factor> Circular private map indexing
    int32_t base = lMapIndex * 10000;
    int index_cap = 10000;
    PrivateIndexMapType::iterator it = next_private_index_map_.find(lMapIndex);
    if (it == next_private_index_map_.end())
        it = next_private_index_map_.insert(PrivateIndexMapType::value_type(lMapIndex, 0)).first;

    int i, next_index = it->second;
    for (i = 0; i < index_cap; ++i)
    {
        if (GetMap(base + next_index) == nullptr)
            break; // available

        if (++next_index >= index_cap)
            next_index = 0;
    }

    if (i == index_cap)
    {
        // No available index
        return 0;
    }

    int32_t lNewMapIndex = base + next_index;
    if (++next_index >= index_cap)
        next_index = 0;

    it->second = next_index;

    m_map_pkSectree.emplace(lNewMapIndex, std::make_unique<SECTREE_MAP>(*pkMapSectree));

    SPDLOG_TRACE("Creating private map {} (original {})", lNewMapIndex, lMapIndex);
    return lNewMapIndex;
}

struct FDestroyPrivateMapEntity
{
    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER))
        {
            auto ch = static_cast<CHARACTER *>(ent);
            SPDLOG_TRACE("{}: Removing from private map", ch->GetName());

            auto sec = ch->GetSectree();
            if (sec)
                sec->RemoveEntity(ent);

            if (ch->IsToggleMount())
                return;

            if (ch->GetDesc()) { DESC_MANAGER::instance().DestroyDesc(ch->GetDesc()); }
            else
                M2_DESTROY_CHARACTER(ch);
        }
        else if (ent->IsType(ENTITY_ITEM))
        {
            auto item = static_cast<CItem *>(ent);
            SPDLOG_TRACE("PRIVATE_MAP: removing item {}", item->GetName());

            M2_DESTROY_ITEM(item);
        }
        else
            SPDLOG_ERROR("PRIVAE_MAP: trying to remove unknown entity {0}", ent->GetEntityType());
    }
};

void SECTREE_MANAGER::DestroyPrivateMap(int32_t lMapIndex)
{
    if (lMapIndex < 10000) // private map 은 인덱스가 10000 이상 이다.
        return;

    SECTREE_MAP *pkMapSectree = GetMap(lMapIndex);
    if (!pkMapSectree)
        return;

    // Removes everything currently on this map
    // WARNING:
    // May be in this map but not in any Sectree
    // So you can't delete here, so the pointer can break
    // need to be handled separately
    // TODO: Figure out a better way to handle this.
    FDestroyPrivateMapEntity f;
    pkMapSectree->for_each(f);

    m_map_pkSectree.erase(lMapIndex);

    SPDLOG_TRACE("PRIVATE_MAP: {} destroyed", lMapIndex);
}

const TAreaMap *SECTREE_MANAGER::GetDungeonArea(int32_t mapIndex)
{
    auto it = m_map_pkArea.find(mapIndex);
    if (it == m_map_pkArea.end())
        return nullptr;

    return &it->second;
}

void SECTREE_MANAGER::SendNPCPosition(CHARACTER *ch)
{
    DESC *d = ch->GetDesc();
    if (!d)
        return;

    int32_t lMapIndex = ch->GetMapIndex();

    if (m_mapNPCPosition[lMapIndex].empty())
        return;

    TEMP_BUFFER buf;
    TPacketGCNPCPosition p;

    TNPCPosition np{};

    for (auto it = m_mapNPCPosition[lMapIndex].begin(); it != m_mapNPCPosition[lMapIndex].end(); ++it)
    {
        np.bType = it->bType;
        np.vnum = it->vnum;
        np.name = it->name;
        np.x = it->x;
        np.y = it->y;
        p.positions.emplace_back(np);
    }

    if(!p.positions.empty())
        d->Send(HEADER_GC_NPC_POSITION, p);
}

void SECTREE_MANAGER::InsertNPCPosition(int32_t lMapIndex, uint32_t vnum, uint8_t bType, std::string szName, int32_t x,
                                        int32_t y)
{
    m_mapNPCPosition[lMapIndex].push_back(npc_info(bType, vnum, szName, x, y));
}

uint8_t SECTREE_MANAGER::GetEmpireFromMapIndex(int32_t lMapIndex)
{
    if (lMapIndex >= 1 && lMapIndex <= 20)
        return 1;

    if (lMapIndex >= 21 && lMapIndex <= 40)
        return 2;

    if (lMapIndex >= 41 && lMapIndex <= 60)
        return 3;

    if (lMapIndex == 184 || lMapIndex == 185)
        return 1;

    if (lMapIndex == 186 || lMapIndex == 187)
        return 2;

    if (lMapIndex == 188 || lMapIndex == 189)
        return 3;

    switch (lMapIndex)
    {
    case 190:
        return 1;
    case 191:
        return 2;
    case 192:
        return 3;
    }

    return 0;
}

class FRemoveIfAttr
{
public:
    FRemoveIfAttr(SECTREE *pkTree, uint32_t dwAttr)
        : m_pkTree(pkTree), m_dwCheckAttr(dwAttr)
    {
    }

    void operator ()(CEntity *entity)
    {
        if (!m_pkTree->IsAttr(entity->GetX(), entity->GetY(), m_dwCheckAttr))
            return;

        if (entity->IsType(ENTITY_ITEM)) { M2_DESTROY_ITEM((CItem*)entity); }
        else if (entity->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *ch = (CHARACTER *)entity;

            if (ch->IsPC())
            {
                PIXEL_POSITION pos;

                if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
                    ch->WarpSet(ch->GetMapIndex(), pos.x, pos.y);
                else
                    ch->WarpSet(EMPIRE_START_MAP(ch->GetEmpire()), EMPIRE_START_X(ch->GetEmpire()),
                                EMPIRE_START_Y(ch->GetEmpire()));
            }
            else
                ch->Dead();
        }
    }

    SECTREE *m_pkTree;
    uint32_t m_dwCheckAttr;
};

bool SECTREE_MANAGER::ForAttrRegion(int32_t mapIndex, int32_t sx, int32_t sy,
                                    int32_t ex, int32_t ey,
                                    float xRot, float yRot, float zRot,
                                    uint32_t attr, EAttrRegionMode mode)
{
    auto map = GetMap(mapIndex);
    if (!map)
    {
        SPDLOG_ERROR("Cannot find SECTREE_MAP by map index: {}", mapIndex);
        return mode == ATTR_REGION_MODE_CHECK;
    }

    //SPDLOG_DEBUG("1: {0} {1} ~ {2} {3} - {4} {5} {6}", sx, sy, ex, ey, xRot, yRot, zRot);

    RotateRegion(sx, sy, ex, ey, xRot, yRot, zRot);

    //SPDLOG_DEBUG("2: {0} {1} ~ {2} {3}", sx, sy, ex, ey);

    //
    // 영역의 좌표를 Cell 의 크기에 맞춰 확장한다.
    //

    sx -= sx % CELL_SIZE;
    sy -= sy % CELL_SIZE;
    ex += CELL_SIZE - (ex % CELL_SIZE);
    ey += CELL_SIZE - (ey % CELL_SIZE);

    //SPDLOG_DEBUG("3: {0} {1} ~ {2} {3}", sx, sy, ex, ey);

    for (uint32_t y = sy; y <= ey; y += CELL_SIZE)
    {
        for (uint32_t x = sx; x <= ex; x += CELL_SIZE)
        {
            auto tree = map->Find(x, y);
            if (!tree)
                continue;

            switch (mode)
            {
            case ATTR_REGION_MODE_SET:
                SPDLOG_TRACE("SET %d on %d %d", attr, x, y);
                tree->SetAttribute(x, y, attr);
                break;

            case ATTR_REGION_MODE_REMOVE:
                tree->RemoveAttribute(x, y, attr);
                break;

            case ATTR_REGION_MODE_CHECK:
                if (tree->IsAttr(x, y, attr))
                    return true;
                break;

            default:
                SPDLOG_ERROR("Unknown region mode %d", mode);
                break;
            }
        }
    }

    return mode != ATTR_REGION_MODE_CHECK;
}

struct FPurgeMonsters
{
    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER) == true)
        {
            CHARACTER *lpChar = (CHARACTER *)ent;

            if (lpChar->IsMonster() == true && !lpChar->IsPet() && !lpChar->IsToggleMount())
            {
                M2_DESTROY_CHARACTER(lpChar);
            }
        }
    }
};

void SECTREE_MANAGER::PurgeMonstersInMap(int32_t lMapIndex)
{
    SECTREE_MAP *sectree = SECTREE_MANAGER::instance().GetMap(lMapIndex);
    if (sectree != nullptr)
    {
        struct FPurgeMonsters f;

        sectree->for_each(f);
    }
}

struct FPurgeStones
{
    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER) == true)
        {
            CHARACTER *lpChar = (CHARACTER *)ent;

            if (lpChar->IsStone() == true) { M2_DESTROY_CHARACTER(lpChar); }
        }
    }
};

void SECTREE_MANAGER::PurgeStonesInMap(int32_t lMapIndex)
{
    SECTREE_MAP *sectree = SECTREE_MANAGER::instance().GetMap(lMapIndex);
    if (sectree != nullptr)
    {
        struct FPurgeStones f;

        sectree->for_each(f);
    }
}

struct FPurgeNPCs
{
    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER) == true)
        {
            CHARACTER *lpChar = (CHARACTER *)ent;

            if (lpChar->IsNPC() == true && !lpChar->IsPet() && !lpChar->IsToggleMount())
            {
                M2_DESTROY_CHARACTER(lpChar);
            }
        }
    }
};

void SECTREE_MANAGER::PurgeNPCsInMap(int32_t lMapIndex)
{
    SECTREE_MAP *sectree = SECTREE_MANAGER::instance().GetMap(lMapIndex);
    if (sectree != nullptr)
    {
        struct FPurgeNPCs f;

        sectree->for_each(f);
    }
}

struct FCountMonsters
{
    std::map<VID, VID> m_map_Monsters;

    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER) == true)
        {
            CHARACTER *lpChar = (CHARACTER *)ent;

            if (lpChar->IsMonster() == true) { m_map_Monsters[lpChar->GetVID()] = lpChar->GetVID(); }
        }
    }
};

size_t SECTREE_MANAGER::GetMonsterCountInMap(int32_t lMapIndex)
{
    SECTREE_MAP *sectree = SECTREE_MANAGER::instance().GetMap(lMapIndex);
    if (sectree != nullptr)
    {
        struct FCountMonsters f;

        sectree->for_each(f);

        return f.m_map_Monsters.size();
    }

    return 0;
}

struct FCountSpecifiedMonster
{
    uint32_t SpecifiedVnum;
    size_t cnt;

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    bool bForceVnum;

    FCountSpecifiedMonster(uint32_t id, bool force)
        : SpecifiedVnum(id), cnt(0), bForceVnum(force)
    {
    }
#else
	FCountSpecifiedMonster(uint32_t id) : SpecifiedVnum(id), cnt(0) {}
#endif

    void operator()(CEntity *ent)
    {
        if (true == ent->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *pChar = static_cast<CHARACTER *>(ent);

            if (true == pChar->IsStone() || (bForceVnum) && (pChar->IsNPC()))
            {
                if (pChar->GetMobTable().dwVnum == SpecifiedVnum)
                    cnt++;
            }
        }
    }
};

#ifdef ENABLE_MELEY_LAIR_DUNGEON
size_t SECTREE_MANAGER::GetMonsterCountInMap(long lMapIndex, uint32_t dwVnum, bool bForceVnum)
#else
size_t SECTREE_MANAGER::GetMonsterCountInMap(long lMapIndex, uint32_t dwVnum)
#endif
{
    SECTREE_MAP *sectree = SECTREE_MANAGER::instance().GetMap(lMapIndex);
    if (nullptr != sectree)
    {
#ifdef ENABLE_MELEY_LAIR_DUNGEON
        struct FCountSpecifiedMonster f(dwVnum, bForceVnum);
#else
		struct FCountSpecifiedMonster f(dwVnum);
#endif

        sectree->for_each(f);

        return f.cnt;
    }

    return 0;
}
