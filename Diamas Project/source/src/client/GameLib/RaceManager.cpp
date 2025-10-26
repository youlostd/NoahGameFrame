#include "RaceManager.h"
#include "RaceMotionData.h"
#include "StdAfx.h"
#include <fstream>
#include <game/MotionConstants.hpp>
#include <pak/Vfs.hpp>

#include "pak/Util.hpp"

CRaceData *CRaceManager::__LoadRaceData(uint32_t dwRaceIndex)
{
    auto it = m_racePaths.find(dwRaceIndex);
    if (m_racePaths.end() == it)
        return nullptr;

    std::string &path = it->second;
    if (path.empty())
        return nullptr;

    const auto it2 = m_pathToData.find(path);
    if (it2 != m_pathToData.end())
        return it2->second.get();

    std::unique_ptr<CRaceData> pRaceData = std::make_unique<CRaceData>();
    pRaceData->SetRace(dwRaceIndex);

    if (!pRaceData->LoadRaceData(path.c_str()))
    {
        SPDLOG_ERROR("CRaceManager::RegisterRacePath(race={0}).LoadRaceData({1})", dwRaceIndex, path.c_str());
        return nullptr;
    }

    __LoadRaceMotionList(*pRaceData, CFileNameHelper::GetPath(path));

    auto rawPtr = pRaceData.get();
    m_pathToData.insert(std::make_pair(path, std::move(pRaceData)));
    return rawPtr;
}

bool CRaceManager::__LoadRaceMotionList(CRaceData &rkRaceData, const std::string &path)
{
    std::string motionListFilename(path);
    motionListFilename += rkRaceData.GetMotionListFileName();

    auto data = LoadFileToString(GetVfs(), motionListFilename.c_str());
    if (!data)
        return false;

    CMemoryTextFileLoader kTextFileLoader;
    kTextFileLoader.Bind(data.value());

    rkRaceData.RegisterMotionMode(MOTION_MODE_GENERAL);

    char szMode[256];
    char szType[256];
    char szFile[256];
    int nPercent = 0;
    bool isSpawn = false;

    static std::string stSpawnMotionFileName;
    static std::string stMotionFileName;

    stSpawnMotionFileName = "";
    stMotionFileName = "";

    UINT uLineCount = kTextFileLoader.GetLineCount();
    for (UINT uLineIndex = 0; uLineIndex < uLineCount; ++uLineIndex)
    {
        const std::string &c_rstLine = kTextFileLoader.GetLineString(uLineIndex);
        sscanf(c_rstLine.c_str(), "%s %s %s %d", szMode, szType, szFile, &nPercent);

        uint32_t motionType = MOTION_NONE;
        if (!GetMotionValue(szType, motionType))
            continue;

        stMotionFileName.assign(path.data(), path.size());
        stMotionFileName += szFile;

        rkRaceData.RegisterMotionData(MOTION_MODE_GENERAL, motionType, stMotionFileName.c_str(), nPercent);

        switch (motionType)
        {
        case MOTION_SPAWN:
            isSpawn = true;
            break;
        case MOTION_DAMAGE:
            stSpawnMotionFileName = stMotionFileName;
            break;
        }
    }

    if (!isSpawn && stSpawnMotionFileName != "")
    {
        rkRaceData.RegisterMotionData(MOTION_MODE_GENERAL, MOTION_SPAWN, stSpawnMotionFileName.c_str(), nPercent);
    }

    rkRaceData.RegisterNormalAttack(MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK);
    return true;
}

void CRaceManager::RegisterRace(uint32_t race, const std::string &path)
{
    auto r = m_racePaths.insert(std::make_pair(race, path));
    if (!r.second)
        SPDLOG_DEBUG("Race {0} already registered", race);
}

void CRaceManager::CreateRace(uint32_t dwRaceIndex)
{
    if (m_RaceDataMap.end() != m_RaceDataMap.find(dwRaceIndex))
    {
        SPDLOG_ERROR("RaceManager::CreateRace : Race {0} already created", dwRaceIndex);
        return;
    }

    auto pRaceData = std::make_unique<CRaceData>();
    pRaceData->SetRace(dwRaceIndex);

    m_RaceDataMap.insert(std::make_pair(dwRaceIndex, pRaceData.get()));
    m_createdRaces.push_back(std::move(pRaceData));

    SPDLOG_DEBUG("CRaceManager::CreateRace(dwRaceIndex={0})", dwRaceIndex);
}

void CRaceManager::SelectRace(uint32_t dwRaceIndex)
{
    TRaceDataIterator itor = m_RaceDataMap.find(dwRaceIndex);
    if (m_RaceDataMap.end() == itor)
    {
        assert(!"Failed to select race data!");
        return;
    }

    m_pSelectedRaceData = itor->second;
}

auto CRaceManager::RegisterCacheMotionData(uint8_t motionMode, uint16_t motionIndex, const std::string &filename,
                                           uint8_t weight) -> void
{
    auto pRaceData = GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    pRaceData->RegisterMotionData(motionMode, motionIndex, GetFullPathFileName(filename.c_str()), weight);
}

CRaceData *CRaceManager::GetSelectedRaceDataPointer()
{
    return m_pSelectedRaceData;
}

std::optional<CRaceData *> CRaceManager::GetRaceDataPointer(uint32_t dwRaceIndex)
{
    auto itor = m_RaceDataMap.find(dwRaceIndex);
    if (m_RaceDataMap.end() == itor)
    {
        CRaceData *pRaceData = __LoadRaceData(dwRaceIndex);
        if (pRaceData)
        {
            m_RaceDataMap.insert(std::make_pair(dwRaceIndex, pRaceData));
            return pRaceData;
        }

        SPDLOG_DEBUG("CRaceManager::GetRaceDataPointer: cannot load data by dwRaceIndex {0}", dwRaceIndex);
        return std::nullopt;
    }

    return itor->second;
}

bool CRaceManager::IsHugeRace(uint32_t race)
{
    auto it = std::find(m_hugeRace.begin(), m_hugeRace.end(), race);

    if (it == m_hugeRace.end())
        return false;

    return true;
}

void CRaceManager::SetHugeRace(uint32_t race)
{
    auto it = std::find(m_hugeRace.begin(), m_hugeRace.end(), race);

    if (it != m_hugeRace.end())
        return;

    m_hugeRace.push_back(race);
}

std::optional<float> CRaceManager::GetRaceHeight(uint32_t race)
{
    auto it = m_raceHeight.find(race);

    if (it == m_raceHeight.end())
        return std::nullopt;

    return it->second;
}

void CRaceManager::SetRaceHeight(uint32_t race, float height)
{
    auto it = m_raceHeight.find(race);

    if (it != m_raceHeight.end())
        return;

    m_raceHeight[race] = height;
}

bool CRaceManager::PreloadRace(uint32_t dwRaceIndex)
{
    auto itor = m_RaceDataMap.find(dwRaceIndex);
    if (m_RaceDataMap.end() == itor)
    {
        CRaceData *pRaceData = __LoadRaceData(dwRaceIndex);
        if (pRaceData)
        {
            m_RaceDataMap.insert(std::make_pair(dwRaceIndex, pRaceData));
            return true;
        }

        SPDLOG_DEBUG("CRaceManager::PreloadRace : cannot load data by dwRaceIndex {0}", dwRaceIndex);
        return false;
    }

    return true;
}

void CRaceManager::SetPathName(const std::string &c_szPathName)
{
    m_strPathName = c_szPathName;
}

const char *CRaceManager::GetFullPathFileName(const char *c_szFileName)
{
    static std::string s_stFileName;

    if (c_szFileName[0] != '.')
    {
        s_stFileName = m_strPathName;
        s_stFileName += c_szFileName;
    }
    else
    {
        s_stFileName = c_szFileName;
    }

    return s_stFileName.c_str();
}

void CRaceManager::__Initialize()
{
    m_pSelectedRaceData = NULL;
}

void CRaceManager::__DestroyRaceDataMap()
{
    m_pathToData.clear();
    m_createdRaces.clear();
}

void CRaceManager::Destroy()
{
    __DestroyRaceDataMap();

    __Initialize();
}

CRaceManager::CRaceManager()
{
    __Initialize();
}

CRaceManager::~CRaceManager()
{
    Destroy();
}
