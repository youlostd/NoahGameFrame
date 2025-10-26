#include "HuntingManager.hpp"

#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <storm/StringUtil.hpp>

bool HuntingManager::LoadClient(const std::string& data)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data))
        return false;

    return Initialize(reader);
}

bool HuntingManager::LoadServer()
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile("data/hunting_missions.txt"))
        return false;

    return Initialize(reader);
}

bool HuntingManager::Initialize(const GroupTextReader& reader)
{

    for (const auto& p : reader.GetChildren()) {
        auto* const node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        const auto* grp = static_cast<GroupTextGroup*>(node);

        HuntingMission m = {};

        if (!GetGroupProperty(grp, "Vnum", m.id)) {
            SPDLOG_ERROR("{}: Missing property 'Vnum' in Group '{}'",
                         "data/hunting_missions.txt", grp->GetName());
            return false;
        }

        
        if (!GetGroupProperty(grp, "Name", m.name)) {
            SPDLOG_WARN("{}: Missing property 'Name' in Group '{}'",
                         "data/hunting_missions.txt", grp->GetName());
            m.name = "HUNTING_MISSION_UNSET_NAME";
        }

        if (!GetGroupProperty(grp, "NeedCount", m.needCount)) {
            SPDLOG_ERROR("{}: Missing property 'NeedCount' in Group '{}'",
                         "data/hunting_missions.txt", grp->GetName());
            return false;
        }

        if (!GetGroupProperty(grp, "MinLevel", m.minLevel)) {
            SPDLOG_ERROR("{}: Missing property 'MinLevel' in Group '{}'",
                         "data/hunting_missions.txt", grp->GetName());
            return false;
        }

        if (!GetGroupProperty(grp, "Daily", m.isDaily)) {
            m.isDaily = 0;
        }

        const auto* const mobList = grp->GetList("MobVnums");
        if (!mobList) {
            SPDLOG_ERROR("{}: Missing List 'MobVnums' in Group '{}'",
                         "data/hunting_missions.txt", grp->GetName());
            return false;
        }

        std::vector<storm::StringRef> tokens;
        for (const auto& line : mobList->GetLines()) {
            tokens.clear();

            uint32_t mobVnum;
            if(!storm::ParseNumber(line, mobVnum)) {
                SPDLOG_ERROR("{}: Invalid MobVnum in List 'MobVnums' in Group '{}'",
                             "data/hunting_missions.txt", grp->GetName());
                return false;
            }

            m.mobVnums.emplace_back(mobVnum);
        }

        const auto* const rewardList = grp->GetList("Rewards");
        if (!rewardList) {
            SPDLOG_ERROR("{}: Missing List 'Rewards' in Group '{}'",
                         "data/hunting_missions.txt", grp->GetName());
            return false;
        }

        tokens.clear();
        for (const auto& line : rewardList->GetLines()) {
            tokens.clear();
            storm::ExtractArguments(line, tokens);

            if (tokens.size() != 2)
                continue;

            HuntingReward r = {};

            if (tokens[0] == "TREE_POINT") {
                r.vnum = 3;
            } else {
                if (!storm::ParseNumber(tokens[0], r.vnum)) {
                    SPDLOG_ERROR("{}: Invalid vnum in 'Rewards' in Group '{}'",
                                 "data/hunting_missions.txt", grp->GetName());
                    return false;
                }
            }

            if (!storm::ParseNumber(tokens[1], r.count)) {
                SPDLOG_ERROR("{}: Invalid count in 'Rewards' in Group '{}'",
                             "data/hunting_missions.txt", grp->GetName());
                return false;
            }

            m.rewards.emplace_back(r);
        }

        const auto* const requiredList = grp->GetList("Required");
        if (requiredList) {
            for (const auto& line : requiredList->GetLines()) {
                uint32_t reqId = 0;
                if (!storm::ParseNumber(line, reqId)) {
                    SPDLOG_ERROR("{}: Invalid requirement in 'Required' in "
                                 "Group '{}'",
                                 "data/hunting_missions.txt", grp->GetName());
                    return false;
                }

                m.required.emplace_back(reqId);
            }
        }

        m_missions.emplace(m.id, m);
    }

    for (const auto& [id, m2] : m_missions) {
        for(const auto& mobVnum : m2.mobVnums) {
            auto& vec = m_missionsByMob[mobVnum];
            vec.emplace_back(m2);
        }
    }

    for(auto& [mobVnum, vec] : m_missionsByMob) {
        std::sort(vec.begin(), vec.end(),
                  [](const HuntingMission& a, const HuntingMission& b) {
                      return a.needCount < b.needCount;
                  });
    }

    return true;
}

const HuntingMission* HuntingManager::GetHuntingMissionById(GenericVnum id)
{
    if (!m_missions.count(id))
        return nullptr;

    return &m_missions[id];
}

std::vector<HuntingMission>
HuntingManager::GetHuntingMissionByMob(GenericVnum vnum)
{
    if(auto it = m_missionsByMob.find(vnum); it != m_missionsByMob.end()) {
        return it->second;
    } 
        
    return {};
}
