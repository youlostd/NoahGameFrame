#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "sectree.h"
#include "questmanager.h"
#include <storm/StringUtil.hpp>
#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <base/ClientDb.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

CMobInstance::CMobInstance()
{
    m_dwLastAttackedTime = get_dword_time();
    m_dwLastWarpTime = get_dword_time();
}

bool CMobManager::ReloadMobProto()
{
	
    ProtoMap tempProtoMap;
	
    std::vector<TMobTable> v;
    if (!LoadClientDbFromFile("data/mob_proto_server", v))
        return false;

    for (const auto &mob : v)
    {
        tempProtoMap.emplace(mob.dwVnum, mob);

        if (mob.bType == CHAR_TYPE_NPC || mob.bType == CHAR_TYPE_WARP || mob.bType == CHAR_TYPE_GOTO)
            g_pCharManager->RegisterRaceNum(mob.dwVnum);

        quest::CQuestManager::instance().RegisterNPCVnum(mob.dwVnum);
    }

	g_pCharManager->for_each([&](CHARACTER *ch) {
		if(ch->IsPC())
            return;
		
		auto it = tempProtoMap.find(ch->GetRaceNum());
		if(it == tempProtoMap.end()) {
			ch->SetProto(nullptr);
			return;
		}
		
        ch->SetProto(&it->second);
		ch->ComputePoints();
    });
	m_protoMap.clear();
	m_protoMap.swap(tempProtoMap);
	
	m_mobNames.clear();
	if (!LoadNames(GetLocaleService().GetDefaultLocale().path + "/mob_names.txt"))
        return false;
	
    return true;
}

bool CMobManager::Initialize()
{
    m_noName = "NONAME";
    m_protoMap.clear();
    m_mobNames.clear();

    std::vector<TMobTable> v;
    if (!LoadClientDbFromFile("data/mob_proto_server", v))
        return false;

    for (const auto &mob : v)
    {
        m_protoMap.emplace(mob.dwVnum, mob);

        if (mob.bType == CHAR_TYPE_NPC ||
            mob.bType == CHAR_TYPE_WARP ||
            mob.bType == CHAR_TYPE_GOTO)
            g_pCharManager->RegisterRaceNum(mob.dwVnum);

        quest::CQuestManager::instance().RegisterNPCVnum(mob.dwVnum);
    }

    if (!LoadNames(GetLocaleService().GetDefaultLocale().path + "/mob_names.txt"))
        return false;

    if (!LoadGroup("data/group.txt"))
    {
        SPDLOG_ERROR("cannot load group.txt");
        return false;
    }

    if (!LoadGroupGroup("data/group_group.txt"))
    {
        SPDLOG_ERROR("cannot load group_group.txt");
        return false;
    }

    if (!ReadMobExpLevelLimitConfig("data/mob_exp_limit.txt"))
    {
        SPDLOG_ERROR("cannot load data/mob_exp_limit.txt");
        return false;
    }

    if (!ReadMountSpeedLimitConfig("data/mount_speed_limit.txt"))
    {
        SPDLOG_ERROR("cannot load data/mount_speed_limit.txt");
        return false;
    }

    return true;
}

const TMobTable *CMobManager::Get(uint32_t dwVnum)
{
    auto it = m_protoMap.find(dwVnum);
    if (it == m_protoMap.end())
        return nullptr;

    return &it->second;
}

const std::string &CMobManager::GetName(uint32_t dwVnum) const
{
    auto it = m_mobNames.find(dwVnum);
    if (it == m_mobNames.end()) { return m_noName; }
    return it->second;
}

const TMobTable *CMobManager::Get(const std::string &name, bool isAbbrev) const
{
    auto f = [&](const robin_hood::pair<uint32_t, std::string> &p) -> bool
    {
        if (isAbbrev)
        {
            return p.second.compare(0, p.second.size(),
                                    name.data(), name.size()) == 0;
        }

        return boost::starts_with(p.second, name);
    };

    const auto it = std::find_if(m_mobNames.begin(),
                                 m_mobNames.end(),
                                 f);

    if (it != m_mobNames.end())
        return &m_protoMap.find(it->first)->second;

    return nullptr;
}

bool CMobManager::MobHasExpLevelLimit(uint32_t vnum)
{
    const auto it = m_map_mobExpLimit.find(vnum);

    return it != m_map_mobExpLimit.end();
}

MobExpLimitPair CMobManager::GetMobExpLevelLimit(uint32_t vnum)
{
    const auto it = m_map_mobExpLimit.find(vnum);

    if (it == m_map_mobExpLimit.end()) { return {}; }

    return (*it).second;
}

bool CMobManager::MountHasSpeedLimit(uint32_t vnum) const
{
    const auto it = m_mapMountSpeedLimit.find(vnum);

    return it != m_mapMountSpeedLimit.end();
}

uint32_t CMobManager::GetMountSpeedLimit(uint32_t vnum) const
{
    const auto it = m_mapMountSpeedLimit.find(vnum);

    if (it == m_mapMountSpeedLimit.end()) { return 250; }

    return (*it).second;
}

uint32_t CMobManager::GetGroupFromGroupGroup(uint32_t dwVnum)
{
    auto it = m_map_pkMobGroupGroup.find(dwVnum);

    if (it == m_map_pkMobGroupGroup.end())
        return 0;

    return it->second->GetMember();
}

CMobGroup *CMobManager::GetGroup(uint32_t dwVnum)
{
    const auto it = m_map_pkMobGroup.find(dwVnum);

    if (it == m_map_pkMobGroup.end())
        return nullptr;

    return it->second;
}

bool CMobManager::ReadMountSpeedLimitConfig(const char *c_pszFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load mount speed limit data file {0}",
                     c_pszFileName);
        return false;
    }

    const auto list = reader.GetList("MountSpeedLimit");
    if (!list) { return false; }

    if (list)
    {
        std::vector<storm::StringRef> tokens;
        int32_t lineNum = 1;
        for (const auto &line : list->GetLines())
        {
            tokens.clear();
            storm::ExtractArguments(line, tokens);

            if (tokens.size() != 2)
            {
                SPDLOG_WARN(
                    "{0}:{1}: Items group line has only {2} tokens but needs 2",
                    c_pszFileName, lineNum, tokens.size());
                continue;
            }

            uint32_t mobVnum;
            if (!storm::ParseNumber(tokens[0], mobVnum))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse vnum",
                    c_pszFileName, lineNum);
                continue;
            }

            uint32_t limit;
            if (!storm::ParseNumber(tokens[1], limit))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse vnum",
                    c_pszFileName, lineNum);
                continue;
            }

            m_mapMountSpeedLimit.emplace(mobVnum, limit);
            ++lineNum;
        }
    }

    return true;
}

bool CMobManager::ReadMobExpLevelLimitConfig(const char *c_pszFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load common drop file %s", c_pszFileName);
        return false;
    }

    for (const auto &p : reader.GetChildren())
    {
        const auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;
        const auto grp = static_cast<GroupTextGroup *>(node);

        // min/max level are optional
        uint32_t vnum = 0;
        uint32_t min = 0;
        uint32_t max = std::numeric_limits<uint32_t>::max();
        GetGroupProperty(grp, "Vnum", vnum);
        GetGroupProperty(grp, "Min", min);
        GetGroupProperty(grp, "Max", max);

        m_map_mobExpLimit.emplace(vnum, std::make_pair(min, max));
    }

    return true;
}

bool CMobManager::LoadGroupGroup(const char *c_pszFileName, bool isReloading)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
        return false;

    std::string stName;

    std::map<uint32_t, CMobGroupGroup *> tempLoader;
    if (isReloading)
        SPDLOG_INFO("RELOADING group group: %s", c_pszFileName);

    for (const auto &p : reader.GetChildren())
    {
        const auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        const auto *grp = static_cast<GroupTextGroup *>(node);

        int iVnum;
        if (!GetGroupProperty(grp, "Vnum", iVnum))
        {
            SPDLOG_ERROR("{}: Missing property 'Vnum' in Group '{}'", c_pszFileName, grp->GetName());
            return false;
        }

        CMobGroupGroup *pkGroup = new CMobGroupGroup(iVnum);

        for (int k = 1; k < 256; ++k)
        {
            const auto *pTok = grp->GetTokens(fmt::format("{:d}", k));

            if (pTok)
            {
                uint32_t dwMobVnum = 0;
                str_to_number(dwMobVnum, pTok->at(0).c_str());

                // ADD_MOB_GROUP_GROUP_PROB
                int prob = 1;
                if (pTok->size() > 1)
                    str_to_number(prob, pTok->at(1).c_str());
                // END_OF_ADD_MOB_GROUP_GROUP_PROB

                if (dwMobVnum)
                    pkGroup->AddMember(dwMobVnum, prob);

                continue;
            }

            break;
        }

        if (isReloading)
            tempLoader.insert(std::make_pair((uint32_t)iVnum, pkGroup));
        else
            m_map_pkMobGroupGroup.insert(std::make_pair((uint32_t)iVnum, pkGroup));
    }
    if (isReloading)
    {
        for (auto &it : m_map_pkMobGroupGroup)
            delete (it.second);
        m_map_pkMobGroupGroup.clear();
        for (auto &it : tempLoader) { m_map_pkMobGroupGroup[it.first] = it.second; }
    }
    return true;
}

bool CMobManager::LoadNames(const std::string &filename)
{
    bsys::error_code ec;
    std::string file;

    storm::ReadFileToString(filename, file, ec);

    if (ec)
    {
        SPDLOG_ERROR("Failed to load mob name list '{0}' with '{1}'",
                     filename.c_str(), ec);
        return false;
    }

    std::vector<std::string> lines;
    storm::Tokenize(file,
                    "\r\n",
                    lines);

    std::vector<storm::String> args;

    int i = 0;
    for (const auto &line : lines)
    {
        args.clear();
        storm::Tokenize(boost::trim_copy(line),
                        "\t",
                        args);

        ++i;

        if (args.empty())
            continue;

        if (args.size() != 2)
        {
            SPDLOG_ERROR(
                "Mob name list '{0}' line {1} has only {2} tokens",
                filename, i, args.size());
            return false;
        }

        uint32_t vnum;
        if (!storm::ParseNumber(args[0], vnum))
        {
            SPDLOG_ERROR(
                "Mob name list '{0}' line {1} vnum is invalid",
                filename, i);
            return false;
        }

        m_mobNames[vnum] = std::string(args[1]);
    }

    return true;
}

bool CMobManager::LoadGroup(const char *c_pszFileName, bool isReloading)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
        return false;

    std::string stName;
    std::map<uint32_t, CMobGroup *> tempLoader;
    if (isReloading)
        SPDLOG_INFO("RELOADING groups: %s", c_pszFileName);

    for (const auto &p : reader.GetChildren())
    {
        auto *node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;
        // Allowed - see type check above
        auto *grp = static_cast<GroupTextGroup *>(node);

        stName = std::string(grp->GetName().data(), grp->GetName().length());

        int iVnum;
        if (!GetGroupProperty(grp, "Vnum", iVnum))
        {
            SPDLOG_ERROR("{}: Missing property 'Vnum' in Group '{}'", c_pszFileName, grp->GetName());
            return false;
        }

        const auto *tokens = grp->GetTokens("Leader");
        if (!tokens || tokens->size() != 2)
        {
            SPDLOG_ERROR("{}: Invalid size for Property 'Leader' in Group '{}'", c_pszFileName, grp->GetName());
            return false;
        }

        auto *pkGroup = new CMobGroup;

        pkGroup->Create(iVnum, stName);
        uint32_t vnum = 0;
        storm::ParseNumber(tokens->at(1), vnum);
        pkGroup->AddMember(vnum);

        SPDLOG_INFO("GROUP: {0} {1}", iVnum, stName.c_str());
        SPDLOG_INFO("               {0} {1}", tokens->at(0).c_str(), tokens->at(1).c_str());

        for (int k = 1; k < 256; ++k)
        {
            const auto *pTok = grp->GetTokens(fmt::format("{:d}", k));

            if (pTok)
            {
                SPDLOG_INFO("               {0} {1}", pTok->at(0).c_str(), pTok->at(1).c_str());
                uint32_t vnum = 0;
                str_to_number(vnum, pTok->at(1).c_str());
                pkGroup->AddMember(vnum);
                continue;
            }

            break;
        }

        if (isReloading)
            tempLoader.emplace(iVnum, pkGroup);
        else
            m_map_pkMobGroup.emplace(iVnum, pkGroup);
    }

    if (isReloading)
    {
        for (auto &it : m_map_pkMobGroup)
            delete (it.second);
        m_map_pkMobGroup.clear();
        for (auto &it : tempLoader) { m_map_pkMobGroup[it.first] = it.second; }
    }
    return true;
}
