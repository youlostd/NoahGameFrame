#include "battle_pass.h"

#include "ChatUtil.hpp"
#include "DbCacheSocket.hpp"
#include "GBufferManager.h"
#include "char.h"
#include "db.h"
#include "desc.h"
#include "desc_manager.h"
#include "party.h"
#include "questlua.h"
#include "questmanager.h"
#include <game/GamePacket.hpp>

#include <string>

#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <game/AffectConstants.hpp>

#include <storm/StringFlags.hpp>

const std::string g_astMissionType[MISSION_TYPE_MAX] = {"",
                                                        "MONSTER_KILL",
                                                        "PLAYER_KILL",
                                                        "MONSTER_DAMAGE",
                                                        "PLAYER_DAMAGE",
                                                        "USE_ITEM",
                                                        "SELL_ITEM",
                                                        "CRAFT_ITEM",
                                                        "REFINE_ITEM",
                                                        "DESTROY_ITEM",
                                                        "COLLECT_ITEM",
                                                        "FRY_FISH",
                                                        "CATCH_FISH",
                                                        "SPENT_YANG",
                                                        "FARM_YANG",
                                                        "COMPLETE_DUNGEON",
                                                        "COMPLETE_MINIGAME",
                                                        "BOSS_KILL",
                                                        "METIN_KILL",
                                                        "REACH_PLAYTIME"};

storm::StringValueTable<uint8_t> kMissionTypes[] = {
    {
        "MONSTER_KILL",
        MONSTER_KILL,
    },
    {
        "PLAYER_KILL",
        PLAYER_KILL,
    },
    {
        "MONSTER_DAMAGE",
        MONSTER_DAMAGE,
    },
    {
        "PLAYER_DAMAGE",
        PLAYER_DAMAGE,
    },
    {
        "USE_ITEM",
        USE_ITEM,
    },
    {
        "SELL_ITEM",
        SELL_ITEM,
    },
    {
        "CRAFT_ITEM",
        CRAFT_ITEM,
    },
    {
        "REFINE_ITEM",
        REFINE_ITEM,
    },
    {
        "DESTROY_ITEM",
        DESTROY_ITEM,
    },
    {
        "COLLECT_ITEM",
        COLLECT_ITEM,
    },
    {
        "FRY_FISH",
        FRY_FISH,
    },
    {
        "CATCH_FISH",
        CATCH_FISH,
    },
    {
        "SPENT_YANG",
        SPENT_YANG,
    },
    {
        "FARM_YANG",
        FARM_YANG,
    },
    {
        "COMPLETE_DUNGEON",
        COMPLETE_DUNGEON,
    },
    {
        "COMPLETE_MINIGAME",
        COMPLETE_MINIGAME,
    },
    {
        "BOSS_KILL",
        BOSS_KILL,
    },
    {
        "METIN_KILL",
        METIN_KILL,
    },
    {
        "REACH_PLAYTIME",
        REACH_PLAYTIME,
    },

};

bool GetBattlePassMissionTypeString(storm::String& s, uint8_t val)
{
    return storm::FormatValueWithTable(s, val, kMissionTypes);
}

bool GetBattlePassMissionTypeValue(const storm::StringRef& s, uint8_t& val)
{
    return storm::ParseStringWithTable(s, val, kMissionTypes);
}

bool CBattlePass::ReadBattlePassFile()
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile("data/battle_pass.txt")) {
        SPDLOG_ERROR("Failed to load battle pass configuration file "
                     "\'data/battle_pass.txt\'");
        return false;
    }

    if (!ReadBattlePassGroup(reader))
        return false;

    if (!ReadBattlePassMissions(reader))
        return false;

    return true;
}

bool CBattlePass::ReadBattlePassGroup(GroupTextReader& reader)
{
    std::string stName;

    const auto* pGroupNode = reader.GetList("BattlePass");

    if (nullptr == pGroupNode) {
        SPDLOG_ERROR("battle_pass.txt need BattlePass list.");
        return false;
    }

    const int n = pGroupNode->GetLineCount();
    if (0 == n) {
        SPDLOG_ERROR("List BattlePass is Empty.");
        return false;
    }

    std::unordered_set<int32_t> setIDs;

    std::vector<storm::StringRef> tokens;
    for (const auto& line : pGroupNode->GetLines()) {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() != 2)
            continue;

        uint32_t battlePassId = 0;
        storm::ParseNumber(tokens[0], battlePassId);

        setIDs.insert(battlePassId);

        m_map_battle_pass_name.emplace(battlePassId, tokens[1]);
    }

    return true;
}

bool CBattlePass::ReadBattlePassMissions(GroupTextReader& reader)
{
    auto it = m_map_battle_pass_name.begin();
    while (it != m_map_battle_pass_name.end()) {
        auto battlePassName = (it++)->second;

        const auto* pGroupNode = reader.GetGroup(battlePassName.c_str());

        if (nullptr == pGroupNode) {
            SPDLOG_ERROR("data/battle_pass.txt: Needs Group '{}'.",
                         battlePassName.c_str());
            return false;
        }

        const auto n = pGroupNode->GetChildren().size();
        if (n < 2) {
            SPDLOG_ERROR("Group {} need to have at least one grup for Reward "
                         "and one Mission. Row: {}",
                         battlePassName.c_str(), n);
            return false;
        }

        {
            const auto* pChild = pGroupNode->GetList("reward");
            if (!pChild) {
                SPDLOG_ERROR("In Group {}, Reward group is not defined.",
                             battlePassName.c_str());
                return false;
            }

            std::vector<TBattlePassRewardItem> rewardVector;
            std::vector<storm::StringRef> tokens;
            for (const auto& line : pChild->GetLines()) {
                tokens.clear();
                storm::ExtractArguments(line, tokens);

                if (tokens.size() != 2)
                    continue;

                TBattlePassRewardItem itemReward = {};
                storm::ParseNumber(tokens[0], itemReward.dwVnum);
                storm::ParseNumber(tokens[1], itemReward.bCount);
                rewardVector.push_back(itemReward);
            }

            m_map_battle_pass_reward.insert(TMapBattlePassReward::value_type(
                battlePassName.c_str(), rewardVector));
        }

        std::vector<TBattlePassMissionInfo> missionInfoVector;

        for (auto i = 1; i < n; i++) {
            auto missionGroupName = fmt::format("Mission_{}", i);
            const auto* pChild = pGroupNode->GetGroup(missionGroupName);
            if (!pChild) {
                SPDLOG_ERROR("In Group {}, {} subgroup is not defined.",
                             battlePassName.c_str(), missionGroupName);
                return false;
            }

            std::string stMissionSearch[] = {"", ""};
            const auto bAlreadySearched = false;
            const uint8_t bRewardContor = 0;
            TBattlePassMissionInfo missionInfo = {};

            missionInfo.bMissionType = MISSION_TYPE_NONE;
            auto mt = pChild->GetProperty("Type");
            if (!mt.empty()) {
                if (!storm::ParseStringWithTable(mt, missionInfo.bMissionType,
                                                 kMissionTypes)) {
                    SPDLOG_ERROR("Group {0}: Unknown mission type '{1}'",
                                 pChild->GetName(),
                                 std::string(mt.data(), mt.length()));
                    return false;
                }
            }

            if (!bAlreadySearched) {
                GetMissionSearchName(missionInfo.bMissionType,
                                     &stMissionSearch[0], &stMissionSearch[1]);
            }

            for (auto k = 0; k < 2; k++) {
                if (!stMissionSearch[k].empty()) {
                    if (!GetGroupProperty(pChild, stMissionSearch[k],
                                          missionInfo.dwMissionInfo[k])) {
                        SPDLOG_ERROR("In Group '{}' and subgroup '{}' required "
                                     "property '{}' does not exist.",
                                     battlePassName.c_str(), missionGroupName,
                                     stMissionSearch[k].c_str());
                        return false;
                    }
                }
            }

            const auto* rewards = pChild->GetList("rewards");
            if (!rewards) {
                SPDLOG_ERROR("In Group {}, Reward group is not defined.",
                             battlePassName.c_str());
                return false;
            }

            std::vector<TBattlePassRewardItem> rewardVector;
            std::vector<storm::StringRef> tokens;
            const auto& lines = rewards->GetLines();
            for (auto i = 0; i < lines.size(); ++i) {
                const auto& line = lines[i];
                tokens.clear();
                storm::ExtractArguments(line, tokens);

                if (tokens.size() != 2)
                    continue;

                if (i > 2)
                    continue;

                storm::ParseNumber(tokens[0],
                                   missionInfo.aRewardList[i].dwVnum);
                storm::ParseNumber(tokens[1],
                                   missionInfo.aRewardList[i].bCount);
            }

            missionInfoVector.push_back(missionInfo);
        }

        m_map_battle_pass_mission_info.insert(TMapBattleMissionInfo::value_type(
            battlePassName.c_str(), missionInfoVector));
    }

    return true;
}

std::string CBattlePass::GetMissionNameByType(uint8_t bType)
{
    for (auto i = 0; i < MISSION_TYPE_MAX; i++) {
        if (i == bType)
            return g_astMissionType[i];
    }

    return "";
}

std::string CBattlePass::GetBattlePassNameByID(uint8_t bID)
{
    const auto it = m_map_battle_pass_name.find(bID);

    if (it == m_map_battle_pass_name.end()) {
        return "";
    }

    return it->second;
}

void CBattlePass::GetMissionSearchName(uint8_t bMissionType,
                                       std::string* st_name_1,
                                       std::string* st_name_2)
{
    switch (bMissionType) {
        case MONSTER_KILL:
        case BOSS_KILL:
        case METIN_KILL:
        case USE_ITEM:
        case SELL_ITEM:
        case CRAFT_ITEM:
        case REFINE_ITEM:
        case DESTROY_ITEM:
        case COLLECT_ITEM:
            *st_name_1 = "Vnum";
            *st_name_2 = "Count";
            break;

        case PLAYER_KILL:
            *st_name_1 = "MinLevel";
            *st_name_2 = "Count";
            break;

        case MONSTER_DAMAGE:
            *st_name_1 = "Vnum";
            *st_name_2 = "Value";
            break;

        case PLAYER_DAMAGE:
            *st_name_1 = "MinLevel";
            *st_name_2 = "Value";
            break;

        case FRY_FISH:
        case CATCH_FISH:
        case REACH_PLAYTIME:
            *st_name_1 = "";
            *st_name_2 = "Count";
            break;

        case SPENT_YANG:
        case FARM_YANG:
            *st_name_1 = "";
            *st_name_2 = "Value";
            break;

        case COMPLETE_DUNGEON:
        case COMPLETE_MINIGAME:
            *st_name_1 = "Id";
            *st_name_2 = "Count";
            break;

        default:
            *st_name_1 = "";
            *st_name_2 = "";
            break;
    }
}

void CBattlePass::BattlePassRequestOpen(CHARACTER* pkChar)
{
    SPDLOG_ERROR("BattlePassRequestOpen");

    if (!pkChar) {
        SPDLOG_ERROR("pkChar null");
        return;
    }

    if (!pkChar->GetDesc()) {
        SPDLOG_ERROR("pkChar GetDesc null");
        return;
    }

    if (!pkChar->IsLoadedBattlePass()) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "The battle pass has not been loaded yet, try again "
                           "in 5 seconds.");
        return;
    }

    const auto bBattlePassId = pkChar->GetBattlePassId();
    if (!bBattlePassId) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO, "No battle pass active.");
        return;
    }

    const auto it = m_map_battle_pass_name.find(bBattlePassId);

    if (it == m_map_battle_pass_name.end()) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "Could not find data for battle pass id %d",
                           bBattlePassId);
        return;
    }

    const auto battlePassName = it->second;
    const auto itInfo = m_map_battle_pass_mission_info.find(battlePassName);

    if (itInfo == m_map_battle_pass_mission_info.end()) {
        pkChar->ChatPacket(CHAT_TYPE_INFO,
                           "Nu exista nici un battle pass cu acest nume. "
                           "Debug: %s",
                           battlePassName.c_str());
        SPDLOG_ERROR("%s", battlePassName.c_str());
        return;
    }

    const auto itReward = m_map_battle_pass_reward.find(battlePassName);
    if (itReward == m_map_battle_pass_reward.end()) {
        pkChar->ChatPacket(CHAT_TYPE_INFO,
                           "Nu exista reward pentru acest battle pass. Debug: "
                           "%s",
                           battlePassName.c_str());
        return;
    }

    const auto rewardInfo = itReward->second;
    auto missionInfo = itInfo->second;

    for (auto& i : missionInfo) {
        i.dwMissionInfo[2] =
            pkChar->GetMissionProgress(i.bMissionType, bBattlePassId);
    }

    if (!missionInfo.empty()) {
        GcBattlePassPacket packet;
        packet.missions = missionInfo;
        packet.rewards = rewardInfo;
        pkChar->GetDesc()->Send(HEADER_GC_BATTLE_PASS_OPEN, packet);
    }
}

void CBattlePass::BattlePassRewardMission(CHARACTER* pkChar,
                                          uint32_t bMissionType,
                                          uint32_t bBattlePassId)
{
    if (!pkChar)
        return;

    if (!pkChar->GetDesc())
        return;

    if (!bBattlePassId) {
        pkChar->ChatPacket(CHAT_TYPE_INFO, "Nu ai nici un battle pass activ.");
        return;
    }

    const auto it = m_map_battle_pass_name.find(bBattlePassId);

    if (it == m_map_battle_pass_name.end()) {
        pkChar->ChatPacket(CHAT_TYPE_INFO,
                           "Nu exista nici un battle pass cu acest id. Debug: "
                           "%d",
                           bBattlePassId);
        return;
    }

    const auto battlePassName = it->second;
    const auto itInfo = m_map_battle_pass_mission_info.find(battlePassName);

    if (itInfo == m_map_battle_pass_mission_info.end()) {
        pkChar->ChatPacket(CHAT_TYPE_INFO,
                           "Nu exista nici un battle pass cu acest nume. "
                           "Debug: %s",
                           battlePassName.c_str());
        return;
    }

    auto missionInfo = itInfo->second;

    for (auto i = 0; i < missionInfo.size(); i++) 
    {
        if (missionInfo[i].bMissionType == bMissionType) 
        {
            for (auto j = 0; j < MISSION_REWARD_COUNT; j++) 
            {
                if (missionInfo[i].aRewardList[j].dwVnum && missionInfo[i].aRewardList[j].bCount > 0)
                    pkChar->AutoGiveItem(missionInfo[i].aRewardList[j].dwVnum,
                                         missionInfo[i].aRewardList[j].bCount);
            }

            break;
        }
    }
}

void CBattlePass::BattlePassRequestReward(CHARACTER* pkChar)
{
    if (!pkChar)
        return;

    if (!pkChar->GetDesc())
        return;

    const auto bBattlePassId = pkChar->GetBattlePassId();
    if (!bBattlePassId) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "No valid battle pass found on your character.");
        return;
    }

    if (pkChar->GetBattlePassState() == 1) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "You already received the reward for this battle "
                           "pass");
        return;
    }

    const auto it = m_map_battle_pass_name.find(bBattlePassId);
    if (it == m_map_battle_pass_name.end()) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "Could not find the battle pass name.");
        return;
    }

    const auto battlePassName = it->second;
    const auto itInfo = m_map_battle_pass_mission_info.find(battlePassName);

    if (itInfo == m_map_battle_pass_mission_info.end()) {
               SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "Could not load battle pass mission info.");
                return;

    }

    auto missionInfo = itInfo->second;

    auto bIsCompleted = true;
    for (auto i = 0; i < missionInfo.size(); i++) {
        if (!pkChar->IsCompletedMission(missionInfo[i].bMissionType)) {
            bIsCompleted = false;
            std::string s;
            if(GetBattlePassMissionTypeString(s, missionInfo[i].bMissionType)) {
                      SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "Mission type %s is not completed.", s.c_str());
            }
         
            break;
        }
    }

    if (bIsCompleted) {
        BroadcastNoticeSpecial(fmt::format("Player %s successfully finished "
                                           "Battle Pass %s.#{};{}",
                                           pkChar->GetName(),
                                           battlePassName.c_str())
                                   .c_str());
        BattlePassReward(pkChar);
    } else {
               SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "Your battle pass is not completed yet.");
        }
}

void CBattlePass::BattlePassReward(CHARACTER* pkChar)
{
    if (!pkChar)
        return;

    if (!pkChar->GetDesc())
        return;

    const auto bBattlePassId = pkChar->GetBattlePassId();
    if (!bBattlePassId) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "No valid battle pass found on your character.");
        return;
    }

    const auto it = m_map_battle_pass_name.find(bBattlePassId);

    if (it == m_map_battle_pass_name.end()) {
        SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "No valid battle pass found on your character.");
        return;
    }

    const auto battlePassName = it->second;
    const auto itReward = m_map_battle_pass_reward.find(battlePassName);
    if (itReward == m_map_battle_pass_reward.end()) {
                SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "There is no reward for this battle pass.  %s",
                           battlePassName.c_str());

        return;
    }

    auto* pAffect = pkChar->FindAffect(AFFECT_BATTLE_PASS, POINT_NONE);
    if (!pAffect) {
                SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "Could not find battle pass data.");
        return;
    }

    pAffect->color = 1;

    auto rewardInfo = itReward->second;

    for (auto i = 0; i < rewardInfo.size(); i++) {
        pkChar->AutoGiveItem(rewardInfo[i].dwVnum, rewardInfo[i].bCount);
    }

    TBattlePassRegisterRanking ranking;
    ranking.bBattlePassId = bBattlePassId;
    storm::CopyStringSafe(ranking.playerName, pkChar->GetName());
    db_clientdesc->DBPacket(HEADER_GD_REGISTER_BP_RANKING, 0, &ranking,
                            sizeof(TBattlePassRegisterRanking));

    SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "You received your reward and were registered in the ranking.");
    SendI18nChatPacket(pkChar, CHAT_TYPE_INFO,
                           "The ranking refreshes every 2 hours.");
}

bool CBattlePass::BattlePassMissionGetInfo(uint8_t bBattlePassId,
                                           uint8_t bMissionType,
                                           uint32_t* dwFirstInfo,
                                           uint32_t* dwSecondInfo)
{
    const auto it = m_map_battle_pass_name.find(bBattlePassId);
    if (it == m_map_battle_pass_name.end())
        return false;

    const auto battlePassName = it->second;
    const auto itInfo = m_map_battle_pass_mission_info.find(battlePassName);

    if (itInfo == m_map_battle_pass_mission_info.end())
        return false;

    auto missionInfo = itInfo->second;

    for (auto i = 0; i < missionInfo.size(); i++) {
        if (missionInfo[i].bMissionType == bMissionType) {
            *dwFirstInfo = missionInfo[i].dwMissionInfo[0];
            *dwSecondInfo = missionInfo[i].dwMissionInfo[1];
            return true;
        }
    }

    return false;
}
