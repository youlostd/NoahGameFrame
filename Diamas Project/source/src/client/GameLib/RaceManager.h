#ifndef METIN2_CLIENT_GAMELIB_RACEMANAGER_H
#define METIN2_CLIENT_GAMELIB_RACEMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "RaceData.h"
#include <optional>

class CRaceManager : public CSingleton<CRaceManager>
{
  public:
    typedef std::unordered_map<uint32_t, CRaceData *> TRaceDataMap;
    typedef TRaceDataMap::iterator TRaceDataIterator;

  public:
    CRaceManager();
    ~CRaceManager();

    void Create();
    void Destroy();

    void RegisterRace(uint32_t race, const std::string &path);

    void SetPathName(const std::string &c_szPathName);
    const char *GetFullPathFileName(const char *c_szFileName);

    // Handling
    auto CreateRace(uint32_t dwRaceIndex) -> void;
    auto SelectRace(uint32_t dwRaceIndex) -> void;
    auto RegisterCacheMotionData(uint8_t motionMode, uint16_t motionIndex, const std::string &filename,
                                 uint8_t weight = 100) -> void;
    auto GetSelectedRaceDataPointer() -> CRaceData *;
    // Handling

    auto GetRaceDataPointer(uint32_t dwRaceIndex) -> std::optional<CRaceData *>;
    auto IsHugeRace(uint32_t race) -> bool;
    void SetHugeRace(uint32_t race);
    std::optional<float> GetRaceHeight(uint32_t race);
    void SetRaceHeight(uint32_t race, float height);
    bool PreloadRace(uint32_t dwRaceIndex);

  private:
    CRaceData *__LoadRaceData(uint32_t dwRaceIndex);
    bool __LoadRaceMotionList(CRaceData &rkRaceData, const std::string &path);

    void __Initialize();
    void __DestroyRaceDataMap();

    // vnum -> CRaceData*
    // Note: Ownership of the contained instances lies somewhere else.
    TRaceDataMap m_RaceDataMap;

    // This vector owns CRaceData instances created via CreateRace(vnum),
    // since they cannot be stored in m_pathToData (MSM path unknown).
    std::vector<std::unique_ptr<CRaceData>> m_createdRaces;

    // msm -> CRaceData*
    // This map owns CRaceData instances registered via RegisterRace.
    std::unordered_map<std::string, std::unique_ptr<CRaceData>, std::hash<std::string>> m_pathToData;

    std::unordered_map<uint32_t, float> m_raceHeight;
    std::unordered_map<uint32_t, std::string> m_racePaths;
    std::vector<uint32_t> m_hugeRace;

    std::string m_strPathName;
    CRaceData *m_pSelectedRaceData;
};
#endif /* METIN2_CLIENT_GAMELIB_RACEMANAGER_H */
