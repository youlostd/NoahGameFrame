#include "../eterLib/StateManager.h"
#include "StdAfx.h"
#include <pak/Vfs.hpp>

#include "MapManager.h"
#include "MapOutdoor.h"

#include "PropertyLoader.h"

#include <storm/io/View.hpp>

#include "base/GroupTextTree.hpp"
#include "pak/Util.hpp"

//////////////////////////////////////////////////////////////////////////
// 기본 함수
//////////////////////////////////////////////////////////////////////////

bool CMapManager::IsMapOutdoor()
{
    if (m_pkMap)
        return true;

    return false;
}

CMapOutdoor &CMapManager::GetMapOutdoorRef()
{
    assert(NULL != m_pkMap);
    return *m_pkMap;
}

CMapManager::CMapManager()
{
    m_pkMap = nullptr;

    m_isFogEnabled = true;
    m_isXmasShow = false;
    m_isSnowModeOption = false;
    m_isSnowTextureModeOption = false;
    m_isNightModeOption = false;

    Initialize();
}

CMapManager::~CMapManager()
{
    Destroy();
}

void CMapManager::Initialize()
{
    __LoadMapInfoVector();
}

void CMapManager::Create()
{
    if (m_pkMap)
    {
        m_pkMap->Clear();
        return;
    }

    m_pkMap = AllocMap();
    assert(NULL != m_pkMap && "CMapManager::Create MAP is NULL");
}

void CMapManager::Destroy()
{
    if (m_pkMap)
    {
        delete m_pkMap;
        m_pkMap = nullptr;
    }
}

void CMapManager::Clear()
{
    if (m_pkMap)
        m_pkMap->Clear();

    m_Forest.Clear();
}

CMapOutdoor *CMapManager::AllocMap()
{
    return new CMapOutdoor;
}

//////////////////////////////////////////////////////////////////////////
// Map
//////////////////////////////////////////////////////////////////////////
void CMapManager::LoadProperty()
{
    CPropertyLoader PropertyLoader;
    PropertyLoader.SetPropertyManager(&m_PropertyManager);
    PropertyLoader.Create("*.*", "Property");
}

bool CMapManager::LoadMap(const int mapIndex, const std::string &c_rstrMapName, float x, float y, float z)
{
    Clear();
    m_pkMap->Leave();
    m_pkMap->SetName(c_rstrMapName);
    m_pkMap->LoadProperty();
    m_pkMap->SetMapIndex(mapIndex);

    if (CMapBase::MAPTYPE_OUTDOOR != m_pkMap->GetType())
    {
        SPDLOG_ERROR("CMapManager::LoadMap() Invalid Map Type");
        return false;
    }

    if (!m_pkMap->Load(x, y, z))
    {
        SPDLOG_ERROR("CMapManager::LoadMap() Outdoor Map Load Failed");
        return false;
    }

    m_pkMap->Enter();
    return true;
}

bool CMapManager::IsMapReady()
{
    if (!m_pkMap)
        return false;

    return m_pkMap->IsReady();
}

bool CMapManager::UnloadMap(const std::string c_strMapName)
{
    assert(m_pkMap && "Tried unloading null map");

    if (c_strMapName != m_pkMap->GetName() && "" != m_pkMap->GetName())
    {
        LogBoxf("%s: Unload Map Failed", c_strMapName.c_str());
        return false;
    }

    Clear();
    return true;
}

bool CMapManager::UpdateMap(float fx, float fy, float fz)
{
    if (!m_pkMap)
        return false;

    return m_pkMap->Update(fx, -fy, fz);
}

void CMapManager::UpdateAroundAmbience(float fx, float fy, float fz)
{
    if (!m_pkMap)
        return;

    m_pkMap->UpdateAroundAmbience(fx, -fy, fz);
}

float CMapManager::GetHeight(float fx, float fy)
{
    if (!m_pkMap)
    {
        SPDLOG_ERROR("CMapManager::GetHeight({0}, {1}) - Access without a map created", fx, fy);
        return 0.0f;
    }

    return m_pkMap->GetHeight(fx, fy);
}

float CMapManager::GetTerrainHeight(float fx, float fy)
{
    if (!m_pkMap)
    {
        SPDLOG_ERROR("CMapManager::GetTerrainHeight({0}, {1}) - Invalid map", fx, fy);
        return 0.0f;
    }

    return m_pkMap->GetTerrainHeight(fx, fy);
}

bool CMapManager::GetWaterHeight(int iX, int iY, long *plWaterHeight)
{
    if (!m_pkMap)
    {
        SPDLOG_ERROR("CMapManager::GetTerrainHeight({0}, {1}) - Invalid map", iX, iY);
        return false;
    }

    return m_pkMap->GetWaterHeight(iX, iY, plWaterHeight);
}

void CMapManager::RefreshPortal()
{
    if (!IsMapReady())
        return;

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        auto pArea = m_pkMap->GetAreaPointer(i);
        if (!pArea)
            continue;

        pArea.value()->RefreshPortal();
    }
}

void CMapManager::ClearPortal()
{
    if (!IsMapReady())
        return;

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        auto pArea = m_pkMap->GetAreaPointer(i);
        if (!pArea)
            continue;

        pArea.value()->ClearPortal();
    }
}

void CMapManager::AddShowingPortalID(int iID)
{
    if (!IsMapReady())
        return;

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        auto pArea = m_pkMap->GetAreaPointer(i);
        if (!pArea)
            continue;

        pArea.value()->AddShowingPortalID(iID);
    }
}

uint32_t CMapManager::GetShadowMapColor(float fx, float fy)
{
    if (!IsMapReady())
        return 0xFFFFFFFF;

    return m_pkMap->GetShadowMapColor(fx, fy);
}

bool CMapManager::isPhysicalCollision(const Vector3 &c_rvCheckPosition)
{
    if (!IsMapReady())
        return false;

    CMapOutdoor &rkMap = GetMapOutdoorRef();
    return rkMap.isAttrOn(c_rvCheckPosition.x, -c_rvCheckPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK);
}

std::vector<int> &CMapManager::GetRenderedSplatNum(int *piPatch, int *piSplat, float *pfSplatRatio)
{
    if (!m_pkMap)
    {
        static std::vector<int> s_emptyVector;
        *piPatch = 0;
        *piSplat = 0;
        return s_emptyVector;
    }

    return m_pkMap->GetRenderedSplatNum(piPatch, piSplat, pfSplatRatio);
}

bool CMapManager::GetNormal(int ix, int iy, Vector3 *pv3Normal)
{
    if (!IsMapReady())
        return false;

    return m_pkMap->GetNormal(ix, iy, pv3Normal);
}

bool CMapManager::isAttrOn(float fX, float fY, uint8_t byAttr)
{
    if (!IsMapReady())
        return false;

    return m_pkMap->isAttrOn(fX, fY, byAttr);
}

bool CMapManager::GetAttr(float fX, float fY, uint8_t *pbyAttr)
{
    if (!IsMapReady())
        return false;

    return m_pkMap->GetAttr(fX, fY, pbyAttr);
}

bool CMapManager::isAttrOn(int iX, int iY, uint8_t byAttr)
{
    if (!IsMapReady())
        return false;

    return m_pkMap->isAttrOn(iX, iY, byAttr);
}

bool CMapManager::GetAttr(int iX, int iY, uint8_t *pbyAttr)
{
    if (!IsMapReady())
        return false;

    return m_pkMap->GetAttr(iX, iY, pbyAttr);
}

void CMapManager::SetTerrainRenderSort(CMapOutdoor::ETerrainRenderSort eTerrainRenderSort)
{
    if (!IsMapReady())
        return;

    m_pkMap->SetTerrainRenderSort(eTerrainRenderSort);
}

void CMapManager::SetTransparentTree(bool bTransparenTree)
{
    if (!IsMapReady())
        return;

    m_pkMap->SetTransparentTree(bTransparenTree);
}

CMapOutdoor::ETerrainRenderSort CMapManager::GetTerrainRenderSort()
{
    if (!IsMapReady())
        return CMapOutdoor::DISTANCE_SORT;

    return m_pkMap->GetTerrainRenderSort();
}

void CMapManager::__LoadMapInfoVector()
{

    auto data = LoadFileToString(GetVfs(), "data/map_config.txt");
    if (!data)
        return;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data.value()))
    {
        return;
    }

    const auto *noSnowMaps = reader.GetList("NoSnowMaps");
    if (noSnowMaps != nullptr)
    {
        for (uint32_t i = 0; i < noSnowMaps->GetLineCount(); ++i)
        {
            auto line = std::string(noSnowMaps->GetLine(i));
            boost::trim(line);
            m_noSnowMap.emplace_back(line);
        }
    }

    const auto *noBoomMaps = reader.GetList("NoBoomMaps");
    if (noBoomMaps != nullptr)
    {
        for (uint32_t i = 0; i < noBoomMaps->GetLineCount(); ++i)
        {
            auto line = std::string(noBoomMaps->GetLine(i));
            boost::trim(line);
            m_noBoomMap.push_back(line);
        }
    }

    const auto *noHorseSkillMaps = reader.GetList("NoHorseSkillMaps");
    if (noHorseSkillMaps != nullptr)
    {
        for (uint32_t i = 0; i < noHorseSkillMaps->GetLineCount(); ++i)
        {
            auto line = std::string(noHorseSkillMaps->GetLine(i));
            boost::trim(line);
            m_noHorseSkillMap.push_back(line);
        }
    }

    const auto *duelMaps = reader.GetList("DuelMaps");
    if (duelMaps != nullptr)
    {
        for (uint32_t i = 0; i < duelMaps->GetLineCount(); ++i)
        {
            auto line = std::string(duelMaps->GetLine(i));
            boost::trim(line);
            uint32_t index;
            storm::ParseNumber(line, index);
            m_duelMapIndices.push_back(index);
        }
    }

    const auto *mapIndexList = reader.GetList("MapIndex");
    if (mapIndexList != nullptr)
    {
        auto lines = mapIndexList->GetLines();

        std::vector<storm::StringRef> args;

        int i = 0;
        for (const auto &line : lines)
        {
            args.clear();
            storm::ExtractArguments(line, args);

            if (args.empty())
                continue;


            if (args.size() < 2)
                continue;

            std::string mapName(args[1]);

            uint32_t mapIndex;
            storm::ParseNumber(args[0], mapIndex);

            float musicVolume = 1.0f;
            std::string musicFileName;
            if (args.size() >= 3)
            {
                musicFileName = args[2];
            }

            if (args.size() == 4)
            {
                storm::ParseNumber(args[3], musicVolume);
            }

            auto strFileName = mapName + "\\Setting.txt";

            CTokenVectorMap stTokenVectorMap;

            if (!LoadMultipleTextData(strFileName.c_str(), stTokenVectorMap))
            {
                SPDLOG_ERROR("An error occured while loading map setting file {}", strFileName);
                continue;
            }

            if (stTokenVectorMap.end() == stTokenVectorMap.find("mapsize"))
            {
                SPDLOG_ERROR("MapOutdoor::LoadSetting(c_szFileName={0}) - FIND 'mapsize' - FAILED", strFileName);
                continue;
            }

            TMapInfo info{};
            info.mapName = mapName;
            storm::ParseNumber(stTokenVectorMap["mapsize"][0], info.sizeX);
            storm::ParseNumber(stTokenVectorMap["mapsize"][1], info.sizeY);
            info.fieldMusicFile = musicFileName;
            info.fieldMusicVolume = musicVolume;

            auto [it, b] = m_mapInfo.insert(std::make_pair(mapIndex, info));
            if(b)
                m_mapInfoByName.emplace(mapName, &it->second);
            m_mapNames.emplace_back(mapName);
        }
    }

    const auto *mapIndexTranslation = reader.GetList("MapIndexTranslation");
    if (mapIndexTranslation != nullptr)
    {
        auto lines = mapIndexTranslation->GetLines();
                std::vector<storm::StringRef> args;

        for (const auto &line : lines)
        {
            args.clear();
            storm::ExtractArguments(line, args);

            if (args.empty())
                continue;

            if (args.size() < 2)
                continue;

            uint32_t mapIndex = 0;
            storm::ParseNumber(args[0], mapIndex);

            auto it = m_mapInfo.find(mapIndex);
            if(it != m_mapInfo.end()) {
                it->second.mapTranslation = args[1];   
            }

        }
    }
}

void CMapManager::SetXMasShowEvent(int iFlag)
{
    m_isXmasShow = iFlag;
}

bool CMapManager::IsXmasShow()
{
    return m_isXmasShow;
}

bool CMapManager::IsSnowModeOption()
{
    return m_isSnowModeOption;
}

void CMapManager::SetSnowModeOption(bool enable)
{
    m_isSnowModeOption = enable;
}

bool CMapManager::IsSnowTextureModeOption()
{
    return m_isSnowTextureModeOption;
}

void CMapManager::SetSnowTextureModeOption(bool enable)
{
    m_isSnowTextureModeOption = enable;
}

void CMapManager::SetNightOption(int iFlag)
{
    m_isNightModeOption = iFlag;
}

bool CMapManager::GetNightOption()
{
    return m_isNightModeOption;
}

void CMapManager::SetFogEnable(bool iFlag)
{
    if (!IsMapReady())
        return;

    auto *envInstance = m_pkMap->GetActiveEnvironmentPtr();
    if (!envInstance)
    {
        SPDLOG_ERROR("CPythonBackground::SetFogEnable(bool enable={0}): No active environment", iFlag);
        return;
    }

    envInstance->SetFogEnable(iFlag);

    m_isFogEnabled = iFlag;
}

bool CMapManager::IsFogEnabled()
{
    return m_isFogEnabled;
}
