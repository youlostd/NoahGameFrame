#pragma once

#include <game/DbPackets.hpp>
#include <game/Types.hpp>
#include <game/GamePacket.hpp>
#include <base/Singleton.hpp>

class GroupTextReader;
class CHARACTER;

class CBattlePass : public singleton<CBattlePass>
{
public:
    CBattlePass() = default;
    ~CBattlePass() = default;

    bool ReadBattlePassFile();
    bool ReadBattlePassGroup(GroupTextReader &reader);
    bool ReadBattlePassMissions(GroupTextReader &reader);

    static std::string GetMissionNameByType(uint8_t bType);
    std::string GetBattlePassNameByID(uint8_t bID);

    void GetMissionSearchName(uint8_t bMissionType, std::string *, std::string *);

    void BattlePassRequestOpen(CHARACTER *pkChar);
    bool BattlePassMissionGetInfo(uint8_t bBattlePassId, uint8_t bMissionType, uint32_t *dwFirstInfo,
                                  uint32_t *dwSecondInfo);
    void BattlePassRewardMission(CHARACTER *pkChar, uint32_t bMissionType, uint32_t bBattlePassId);

    void BattlePassRequestReward(CHARACTER *pkChar);
    void BattlePassReward(CHARACTER *pkChar);

private:
    typedef std::map<uint8_t, std::string> TMapBattlePassName;
    typedef std::map<std::string, std::vector<TBattlePassRewardItem>> TMapBattlePassReward;
    typedef std::map<std::string, std::vector<TBattlePassMissionInfo>> TMapBattleMissionInfo;

    TMapBattlePassName m_map_battle_pass_name;
    TMapBattlePassReward m_map_battle_pass_reward;
    TMapBattleMissionInfo m_map_battle_pass_mission_info;
};
