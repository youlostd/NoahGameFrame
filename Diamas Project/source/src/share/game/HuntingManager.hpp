#ifndef METIN2_SERVER_GAME_GHUNTINGMANAGER_HPP
#define METIN2_SERVER_GAME_GHUNTINGMANAGER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <SpdLog.hpp>
#include <base/GroupTextTree.hpp>

#include <game/Types.hpp>

#include <base/Singleton.hpp>

class HuntingManager : public singleton<HuntingManager>
{
    using MissionMap = std::unordered_map<uint32_t, HuntingMission>;
    using MobMissionMap = std::unordered_map<uint32_t, std::vector<HuntingMission>>;

  public:
    bool Initialize(const GroupTextReader& reader);
    bool LoadServer();
    bool LoadClient(const std::string &data);
    const HuntingMission *GetHuntingMissionById(GenericVnum id);
    std::vector<HuntingMission> GetHuntingMissionByMob(GenericVnum vnum);

    const MissionMap &GetMissions() const { return m_missions; }

  private:
    MissionMap m_missions;
    MobMissionMap m_missionsByMob;
};

#endif
