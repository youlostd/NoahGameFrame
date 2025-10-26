#include "../EterLib/ResourceManager.h"
#include "../SpeedTreeLib/SpeedTreeForest.h"
#include "../eterBase/Timer.h"
#include "AreaTerrain.h"
#include "MapOutdoor.h"
#include "StdAfx.h"
#include "../SpeedTreeLib/SpeedTreeForestDirectX9.h"

#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <pak/Vfs.hpp>

#include "pak/Util.hpp"

// CAreaLoaderThread CMapOutdoor::ms_AreaLoaderThread;

bool CMapOutdoor::Load(float x, float y, float z)
{
    Destroy();

    auto strFileName = GetMapDataDirectory() + "\\Setting.txt";

    if (!LoadSetting(strFileName.c_str()))
    {
        SPDLOG_ERROR("CMapOutdoor::Load : LoadSetting({0}) Failed", strFileName.c_str());
        return false;
    }

    CreateTerrainPatchProxyList();
    BuildQuadTree();
    LoadWaterTexture();

    m_lOldReadX = -1;

    CSpeedTreeForestDirectX9::Instance().Initialize();

    Update(x, y, z);
    return true;
}

bool CMapOutdoor::isTerrainLoaded(uint16_t wX, uint16_t wY)
{
    for (auto &pTerrain : m_TerrainVector)
    {
        uint16_t usCoordX, usCoordY;
        pTerrain->GetCoordinate(&usCoordX, &usCoordY);

        if (usCoordX == wX && usCoordY == wY)
            return true;
    }
    return false;
}

bool CMapOutdoor::isAreaLoaded(uint16_t wX, uint16_t wY)
{
    for (auto &area : m_AreaVector)
    {
        uint16_t usCoordX, usCoordY;
        area->GetCoordinate(&usCoordX, &usCoordY);

        if (usCoordX == wX && usCoordY == wY)
            return true;
    }
    return false;
}

// ÇöÀç ÁÂÇ¥¸¦ ±â¹ÝÀ¸·Î ÁÖÀ§(ex. 3x3)¿¡ ÀÖ´Â Terrain°ú AreaÆ÷ÀÎÅÍ¸¦
// m_pTerrain°ú m_pArea¿¡ ¿¬°áÇÑ´Ù.
void CMapOutdoor::AssignTerrainPtr()
{
    short sReferenceCoordMinX, sReferenceCoordMaxX, sReferenceCoordMinY, sReferenceCoordMaxY;
    sReferenceCoordMinX = std::max(m_CurCoordinate.m_sTerrainCoordX - LOAD_SIZE_WIDTH, 0);
    sReferenceCoordMaxX = std::min(m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH, m_sTerrainCountX - 1);
    sReferenceCoordMinY = std::max(m_CurCoordinate.m_sTerrainCoordY - LOAD_SIZE_WIDTH, 0);
    sReferenceCoordMaxY = std::min(m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH, m_sTerrainCountY - 1);

    uint32_t i;
    for (i = 0; i < AROUND_AREA_NUM; ++i)
    {
        m_pArea[i] = nullptr;
        m_pTerrain[i] = nullptr;
    }

    for (i = 0; i < m_TerrainVector.size(); ++i)
    {
        auto &pTerrain = m_TerrainVector[i];
        uint16_t usCoordX, usCoordY;
        pTerrain->GetCoordinate(&usCoordX, &usCoordY);

        if (usCoordX >= sReferenceCoordMinX && usCoordX <= sReferenceCoordMaxX && usCoordY >= sReferenceCoordMinY &&
            usCoordY <= sReferenceCoordMaxY)
        {
            m_pTerrain[(usCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 +
                       (usCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH)] = pTerrain.get();
        }
    }

    for (i = 0; i < m_AreaVector.size(); ++i)
    {
        auto &area = m_AreaVector[i];
        uint16_t usCoordX, usCoordY;
        area->GetCoordinate(&usCoordX, &usCoordY);

        if (usCoordX >= sReferenceCoordMinX && usCoordX <= sReferenceCoordMaxX && usCoordY >= sReferenceCoordMinY &&
            usCoordY <= sReferenceCoordMaxY)
        {
            m_pArea[(usCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 +
                    (usCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH)] = area.get();
        }
    }
}

bool CMapOutdoor::LoadArea(uint16_t wAreaCoordX, uint16_t wAreaCoordY)
{
 /*   if (isAreaLoaded(wAreaCoordX, wAreaCoordY))
        return true;
    const uint32_t ulID = static_cast<uint32_t>(wAreaCoordX) * 1000L + static_cast<uint32_t>(wAreaCoordY);
    char szAreaPathName[64 + 1];

    _snprintf(szAreaPathName, sizeof(szAreaPathName), "%s\\%06u\\", GetMapDataDirectory().c_str(), ulID);

    CArea *area = new CArea;
    area->SetMapOutDoor(this);
    area->SetCoordinate(wAreaCoordX, wAreaCoordY);
    m_areaLoader.Request(area);
    */

      if (isAreaLoaded(wAreaCoordX, wAreaCoordY))
        return true;
    const uint32_t ulID = static_cast<uint32_t>(wAreaCoordX) * 1000L + static_cast<uint32_t>(wAreaCoordY);
    char szAreaPathName[64 + 1];

    _snprintf(szAreaPathName, sizeof(szAreaPathName), "%s\\%06u\\", GetMapDataDirectory().c_str(), ulID);

    std::unique_ptr<CArea> area(new CArea());
    area->SetMapOutDoor(this);
    area->SetCoordinate(wAreaCoordX, wAreaCoordY);
    if (!area->Load(szAreaPathName))
        SPDLOG_ERROR(" CMapOutdoor::LoadArea({0}, {1}) LoadShadowMap ERROR", wAreaCoordX, wAreaCoordY);

    m_AreaVector.push_back(std::move(area));
    return true;
}

bool CMapOutdoor::LoadTerrain(uint16_t wTerrainCoordX, uint16_t wTerrainCoordY)
{
    if (isTerrainLoaded(wTerrainCoordX, wTerrainCoordY))
        return true;

    //////////////////////////////////////////////////////////////////////////
    uint32_t dwStartTime = ELTimer_GetMSec();

    const uint32_t ulID = static_cast<uint32_t>(wTerrainCoordX) * 1000L + static_cast<uint32_t>(wTerrainCoordY);
    char filename[256];
    sprintf(filename, "%s\\%06lu\\AreaProperty.txt", GetMapDataDirectory().c_str(), ulID);

    CTokenVectorMap stTokenVectorMap;

    if (!LoadMultipleTextData(filename, stTokenVectorMap))
    {
        SPDLOG_ERROR("CMapOutdoor::LoadTerrain AreaProperty Read Error\n");
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
    {
        SPDLOG_ERROR("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 1\n");
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("areaname"))
    {
        SPDLOG_ERROR("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 2\n");
        return false;
    }

    const std::string &scriptType = stTokenVectorMap["scripttype"][0];
    const std::string &areaName = stTokenVectorMap["areaname"][0];

    if (scriptType != "AreaProperty")
    {
        SPDLOG_ERROR("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 3\n");
        return false;
    }

    std::unique_ptr<CTerrain> pTerrain(new CTerrain());
    pTerrain->Clear();
    pTerrain->SetMapOutDoor(this);

    pTerrain->SetCoordinate(wTerrainCoordX, wTerrainCoordY);

    pTerrain->CopySettingFromGlobalSetting();

    char szRawHeightFieldname[64 + 1];
    char szWaterMapName[64 + 1];
    char szAttrMapName[64 + 1];
    char szShadowTexName[64 + 1];
    char szShadowMapName[64 + 1];
    char szMiniMapTexName[64 + 1];
    char szSplatName[64 + 1];

    _snprintf(szRawHeightFieldname, sizeof(szRawHeightFieldname), "%s\\%06u\\height.raw", GetMapDataDirectory().c_str(),
              ulID);
    _snprintf(szSplatName, sizeof(szSplatName), "%s\\%06u\\tile.raw", GetMapDataDirectory().c_str(), ulID);
    _snprintf(szAttrMapName, sizeof(szAttrMapName), "%s\\%06u\\attr.atr", GetMapDataDirectory().c_str(), ulID);
    _snprintf(szWaterMapName, sizeof(szWaterMapName), "%s\\%06u\\water.wtr", GetMapDataDirectory().c_str(), ulID);
    _snprintf(szShadowTexName, sizeof(szShadowTexName), "%s\\%06u\\shadowmap.dds", GetMapDataDirectory().c_str(), ulID);
    _snprintf(szShadowMapName, sizeof(szShadowMapName), "%s\\%06u\\shadowmap.raw", GetMapDataDirectory().c_str(), ulID);
    _snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds", GetMapDataDirectory().c_str(), ulID);

    if (!pTerrain->LoadWaterMap(szWaterMapName))
        SPDLOG_ERROR(" CMapOutdoor::LoadTerrain({0}, {1}) LoadWaterMap ERROR", wTerrainCoordX, wTerrainCoordY);

    if (!pTerrain->LoadHeightMap(szRawHeightFieldname))
        SPDLOG_ERROR(" CMapOutdoor::LoadTerrain({0}, {1}) LoadHeightMap ERROR", wTerrainCoordX, wTerrainCoordY);

    if (!pTerrain->LoadAttrMap(szAttrMapName))
        SPDLOG_ERROR(" CMapOutdoor::LoadTerrain({0}, {1}) LoadAttrMap ERROR", wTerrainCoordX, wTerrainCoordY);

    if (!pTerrain->RAW_LoadTileMap(szSplatName))
        SPDLOG_ERROR(" CMapOutdoor::LoadTerrain({0}, {1}) RAW_LoadTileMap ERROR", wTerrainCoordX, wTerrainCoordY);

    pTerrain->LoadShadowTexture(szShadowTexName);

    if (!pTerrain->LoadShadowMap(szShadowMapName))
        SPDLOG_ERROR(" CMapOutdoor::LoadTerrain({0}, {1}) LoadShadowMap ERROR", wTerrainCoordX, wTerrainCoordY);

    pTerrain->LoadMiniMapTexture(szMiniMapTexName);
    pTerrain->SetName(areaName);
    pTerrain->CalculateTerrainPatch();

    pTerrain->SetReady();

    SPDLOG_DEBUG("CMapOutdoor::LoadTerrain {0}", ELTimer_GetMSec() - dwStartTime);

    m_TerrainVector.push_back(std::move(pTerrain));

    return true;
}

bool CMapOutdoor::LoadSetting(const char *filename)
{
    auto data = LoadFileToString(GetVfs(), filename);
    if (!data)
        return false;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data.value()))
    {
        SPDLOG_ERROR("Failed to parse config file property/config.txt");
        return false;
    }

    static const std::string_view kRequiredTokens[] = {
        "ScriptType", "ViewRadius", "CellScale", "HeightScale", "MapSize", "TextureSet",
    };

    if (!VerifyRequiredPropertiesPresent(&reader, kRequiredTokens))
    {
        //
        return false;
    }

    if (reader.GetProperty("ScriptType") != "MapSetting")
    {
        SPDLOG_ERROR("{0}: Invalid resource type", filename);
        return false;
    }

    uint8_t terrainVisible = 1;
    if (GetGroupProperty(&reader, "TerrainVisible", terrainVisible))
        SetVisiblePart(PART_TERRAIN, 0 != terrainVisible);

    uint8_t enablePortal = 1;
    if (GetGroupProperty(&reader, "EnablePortal", enablePortal))
        EnablePortal(0 != terrainVisible);

    GetGroupProperty(&reader, "ViewRadius", m_lViewRadius);
    if (0L >= m_lViewRadius)
    {
        SPDLOG_ERROR("{0}: ViewRadius == 0", filename);
        return false;
    }

    GetGroupProperty(&reader, "HeightScale", m_fHeightScale);

    uint32_t cellScale;
    GetGroupProperty(&reader, "CellScale", cellScale);
    if (cellScale != CTerrainImpl::CELLSCALE)
    {
        SPDLOG_ERROR("{0}: Unsupported CellScale", filename);
        return false;
    }

    const auto tokens = reader.GetTokens("MapSize");
    if (!tokens || tokens->size() != 2)
    {
        SPDLOG_ERROR("{0}: Invalid MapSize", filename);
        return false;
    }
    uint16_t w;
    uint16_t h;
    try
    {
        w = std::stoi((*tokens)[0]);
        h = std::stoi((*tokens)[1]);
    }
    catch (...)
    {
        SPDLOG_ERROR("{0}: Invalid MapSize", filename);
        return false;
    }

    SetTerrainCount(w, h);

    std::string textureset = std::string(reader.GetProperty("TextureSet"));

    if (0 != textureset.find_first_of("textureset", 0))
        textureset = "textureset\\" + textureset;

    if (!m_TextureSet.Load(textureset.c_str(), m_fTerrainTexCoordBase))
    {
        SPDLOG_ERROR("{0}: LOAD TEXTURE SET({1}) ERROR", filename, textureset);
        return false;
    }

    std::string snowTextureSet = std::string(reader.GetProperty("TextureSet"));

    if (0 != snowTextureSet.find_first_of("textureset", 0))
        snowTextureSet = "textureset\\" + snowTextureSet;

    if (0 != snowTextureSet.find_first_of("textureset", 0))
    {
        snowTextureSet = "textureset\\snow\\snow_" + snowTextureSet;
    }
    else
    {
        snowTextureSet = "textureset\\snow\\snow_" + snowTextureSet.substr(11);
    }

    if (!m_SnowTextureSet.Load(snowTextureSet.c_str(), m_fTerrainTexCoordBase))
    {
        SPDLOG_DEBUG("{0}: has no snow texture set ({1})", filename, snowTextureSet);
    }

    CTerrain::SetTextureSet(&m_TextureSet);

    const auto legacyEnvPath = std::string(reader.GetProperty("Environment"));
    if (!legacyEnvPath.empty())
    {
        std::string path = GetMapDataDirectory() + "/" + legacyEnvPath;
        if (!RegisterEnvironment(0, path.c_str()))
        {
            path = "d:/ymir work/environment/";
            path += legacyEnvPath;

            if (!RegisterEnvironment(0, path.c_str()) && !RegisterEnvironment(0, legacyEnvPath.c_str()))
            {
                SPDLOG_ERROR("{0}: Failed to load main environment {1}", filename, path);
                return false;
            }
        }

        SetActiveEnvironment(GetEnvironment(0));
    }

    auto props = reader.GetProperties();
    char buf[24];
    int i;
    for (i = 1;; ++i)
    {
        sprintf(buf, "Environment%d", i);

        if (props.find(buf) != props.end())
        {
            auto envPath = std::string(reader.GetProperty(buf));

            std::string path = GetMapDataDirectory() + "/" + envPath;
            if (!RegisterEnvironment(i, path.c_str()))
            {
                path = "d:/ymir work/environment/";
                path += envPath;

                if (!RegisterEnvironment(i, path.c_str()) && !RegisterEnvironment(i, envPath.c_str()))
                {
                    SPDLOG_ERROR("{0}: Failed to load environment index {1} {2}", filename, path, i);
                    return false;
                }
            }
        }
        else
        {
            break;
        }
    }

    if (!RegisterEnvironment(99, "d:/ymir work/environment/moonlight04.msenv"))
    {
        SPDLOG_ERROR("{0}: Failed to load night environment", filename);
        return false;
    }

    // Transform
    m_matBuildingTransparent = Matrix::CreateScale(1.0f / ((float)ms_iWidth), -1.0f / ((float)ms_iHeight), 1.0f);
    m_matBuildingTransparent._41 = 0.5f;
    m_matBuildingTransparent._42 = 0.5f;
    return true;
}

bool CMapOutdoor::ReloadSetting(bool useSnowTexture)
{
    if (useSnowTexture)
        CTerrain::SetTextureSet(&m_SnowTextureSet);
    else
        CTerrain::SetTextureSet(&m_TextureSet);

    return true;
}
