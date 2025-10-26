#include "item_manager.h"

#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "DbCacheSocket.hpp"
#include "db.h"
#include "skill.h"
#include "questmanager.h"
#include "item.h"

#include <storm/StringUtil.hpp>
#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <storm/StringFlags.hpp>
#include <fstream>
#include <absl/strings/str_split.h>
#include <absl/strings/str_join.h>
#include "mob_manager.h"

bool ITEM_MANAGER::ReadCommonDropItemFile(const char *c_pszFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load common drop file {0}", c_pszFileName);
        return false;
    }

    for (const auto &p : reader.GetChildren())
    {
        uint32_t rank;
        if (!GetMobRankValue(p.first, rank))
        {
            SPDLOG_ERROR("Failed to load group {0} - no valid mob rank", p.first.c_str());
            return false;
        }

        auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        auto grp = static_cast<GroupTextGroup *>(node);
        for (const auto &lc : grp->GetChildren())
        {
            if (lc.second->GetType() != GroupTextNode::kGroup)
                continue;

            auto lnode = static_cast<GroupTextGroup *>(lc.second);

            // min/max level are optional
            uint32_t min = 0;
            uint32_t max = std::numeric_limits<uint32_t>::max();
            GetGroupProperty(lnode, "Min", min);
            GetGroupProperty(lnode, "Max", max);

            auto items = lnode->GetList("Items");
            if (!items)
            {
                SPDLOG_ERROR("Failed to load common drop - {0} has no Items list (rank {1})", lc.first.c_str(),
                             p.first.c_str());
                return false;
            }

            std::vector<storm::StringRef> tokens;
            for (const auto &line : items->GetLines())
            {
                tokens.clear();
                storm::ExtractArguments(line, tokens);

                if (tokens.size() != 3)
                {
                    SPDLOG_ERROR(
                        "Failed to load common drop - Items group"
                        " of {0} (rank {1}) doesnt have 3 tokens, but {2}",
                        lc.first.data(), p.first.data(), tokens.size());
                    continue;
                }

                uint32_t vnum;
                if (!storm::ParseNumber(tokens[0], vnum))
                {
                    SPDLOG_ERROR(
                        fmt::format("Vnum in group {0} (rank {1}) is not a number ('{2}')",
                            lc.first.data(), p.first.data(), tokens[0].data()).c_str());
                    continue;
                }

                uint32_t count;
                if (!storm::ParseNumber(tokens[1], count))
                {
                    SPDLOG_ERROR(fmt::format(
                        "Count in group {0} (rank {1}) is not a number ('{2}')",
                        lc.first.data(), p.first.data(), tokens[1].data()).c_str());
                    continue;
                }

                float pct;
                if (!storm::ParseNumber(tokens[2], pct))
                {
                    SPDLOG_ERROR(fmt::format(
                        "Pct in group {0} (rank {1}) is not a number ('{2}')",
                        lc.first.data(), p.first.data(), tokens[2].data()).c_str());
                    continue;
                }

                pct *= 10000;
                m_commonDropItem[rank].push_back(CItemDropInfo(min, max, pct, vnum, count));
            }
        }
    }

    for (int i = 0; i < MOB_RANK_MAX_NUM; ++i)
    {
        auto &v = m_commonDropItem[i];
        std::sort(v.begin(), v.end());

        SPDLOG_INFO(fmt::format("CommonItemDrop rank {0}", i).c_str());

        for (const auto &c : v)
        {
            SPDLOG_INFO(fmt::format("CommonItemDrop {0} {1} {2} {3}",
                c.m_iLevelStart, c.m_iLevelEnd,
                c.m_iPercent, c.m_dwVnum).c_str());
        }
    }

    return true;
}

bool ITEM_MANAGER::ReadEnhanceTimeExeption(const char *c_pszFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load enhance time exception file {0}", c_pszFileName);
        return false;
    }

    const auto list = reader.GetList("EnhanceTimeBlocked");
    if (list)
    {
        const auto &lines = list->GetLines();
        for (const auto &line : lines)
        {
            uint32_t vnum;
            if (!storm::ParseNumber(line, vnum))
            {
                SPDLOG_ERROR(fmt::format("Vnum in List EnhanceTimeBlocked is not a number {0}",
                    line.c_str()).c_str());
                continue;
            }

            m_enhanceExceptionVnums.emplace(vnum);
        }
    }

    for (const auto vnum : m_enhanceExceptionVnums)
        SPDLOG_INFO("EnhanceTimeBlocked {0}", vnum);

    return true;
}

bool ITEM_MANAGER::ReadLevelPetData(const char *c_pszFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load level pet data file {0}",
                     c_pszFileName);
        return false;
    }

    const auto dropList = reader.GetList("PetDropData");
    if (!dropList)
    {
        SPDLOG_ERROR("Missing PetDropData List in {}", c_pszFileName);
        return false;
    }

    if (dropList)
    {
        std::vector<storm::StringRef> tokens;
        int32_t lineNum = 1;
        for (const auto &line : dropList->GetLines())
        {
            tokens.clear();
            storm::ExtractArguments(line, tokens);

            if (tokens.size() != 3)
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Items group line has only {2} tokens but needs 3",
                    c_pszFileName, lineNum, tokens.size());
                continue;
            }

            uint32_t vnum;
            if (!storm::ParseNumber(tokens[0], vnum))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse vnum",
                    c_pszFileName, lineNum);
                continue;
            }

            uint32_t mobVnum;
            if (!storm::ParseNumber(tokens[1], mobVnum))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse mob vnum",
                    c_pszFileName, lineNum);
                continue;
            }

            float pct;
            if (!storm::ParseNumber(tokens[2], pct))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse percentage",
                    c_pszFileName, lineNum);
                continue;
            }

            pct *= 10000;
            m_levelPetItemDropInfos.emplace_back(vnum, mobVnum, pct);
            ++lineNum;
        }
    }

    const auto needCountList = reader.GetList("NeedCount");
    if (!needCountList)
    {
        SPDLOG_ERROR("Missing NeedCount List in {}", c_pszFileName);
        return false;
    }

    std::vector<storm::StringRef> tokens;
    int32_t lineNum = 1;
    for (const auto &line : needCountList->GetLines())
    {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() != 2)
        {
            SPDLOG_ERROR(
                "{0}:{1}: NeedCount group line has only {2} tokens but needs 2",
                c_pszFileName, lineNum, tokens.size());
            continue;
        }

        Level level;
        if (!storm::ParseNumber(tokens[0], level))
        {
            SPDLOG_ERROR(
                "{0}:{1}: Could not parse vnum",
                c_pszFileName, lineNum);
            continue;
        }

        uint32_t needCount;
        if (!storm::ParseNumber(tokens[1], needCount))
        {
            SPDLOG_ERROR(
                "{0}:{1}: Could not parse mob vnum",
                c_pszFileName, lineNum);
            continue;
        }

        m_levelPetExpTable.emplace(level, needCount);
    }

    const auto *availableApply = reader.GetList("AvailablePetBonus");
    if (!availableApply)
    {
        SPDLOG_ERROR("Missing AvailablePetBonus List in {}", c_pszFileName);
        return false;
    }

    tokens.clear();
    lineNum = 1;
    for (const auto &line : availableApply->GetLines())
    {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() != 2)
        {
            SPDLOG_ERROR(
                "{0}:{1}: NeedCount group line has only {2} tokens but needs 2",
                c_pszFileName, lineNum, tokens.size());
            continue;
        }
        ItemApply a;

        uint32_t apply;
        if (!GetApplyTypeValue(tokens[0], apply))
        {
            SPDLOG_ERROR(
                "{0}:{1}: Could not parse apply type",
                c_pszFileName, lineNum);
            continue;
        }
        a.type = apply;

        if (!storm::ParseNumber(tokens[1], a.value))
        {
            SPDLOG_ERROR(
                "{0}:{1}: Could not parse mob vnum",
                c_pszFileName, lineNum);
            continue;
        }

        m_levelPetAvailableBonus.push_back(a);
    }

    const auto bonusPerStatPointList = reader.GetList("BonusPerStatPoint");
    if (!bonusPerStatPointList)
    {
        SPDLOG_ERROR("Missing NeedCount List in {}", c_pszFileName);
        return false;
    }

    tokens.clear();
    lineNum = 1;
    for (const auto &line : bonusPerStatPointList->GetLines())
    {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() != 2)
        {
            SPDLOG_ERROR(
                "{0}:{1}: NeedCount group line has only {2} tokens but needs 2",
                c_pszFileName, lineNum, tokens.size());
            continue;
        }

        uint32_t apply;
        if (!GetApplyTypeValue(tokens[0], apply))
        {
            SPDLOG_ERROR(
                "{0}:{1}: Could not parse apply type",
                c_pszFileName, lineNum);
            continue;
        }

        float pointPer;
        if (!storm::ParseNumber(tokens[1], pointPer))
        {
            SPDLOG_ERROR(
                "{0}:{1}: Could not parse mob vnum",
                c_pszFileName, lineNum);
            continue;
        }

        m_levelPetBonusPerStatPoint.emplace(apply, pointPer);
    }

    return true;
}

bool ITEM_MANAGER::ReadAttackPetDropData(const char *c_pszFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load attack pet drop data file {0}",
                     c_pszFileName);
        return false;
    }

    const auto list = reader.GetList("AttackPetData");
    if (!list) { return false; }

    if (list)
    {
        std::vector<storm::StringRef> tokens;
        int32_t lineNum = 1;
        for (const auto &line : list->GetLines())
        {
            tokens.clear();
            storm::ExtractArguments(line, tokens);

            if (tokens.size() != 5)
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Items group line has only {2} tokens but needs 5",
                    c_pszFileName, lineNum, tokens.size());
                continue;
            }

            uint32_t vnum;
            if (!storm::ParseNumber(tokens[0], vnum))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse vnum",
                    c_pszFileName, lineNum);
                continue;
            }

            uint32_t mobVnum;
            if (!storm::ParseNumber(tokens[1], mobVnum))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse mob vnum",
                    c_pszFileName, lineNum);
                continue;
            }

            int32_t scalePlus;
            if (!storm::ParseNumber(tokens[2], scalePlus))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse scale plus value",
                    c_pszFileName, lineNum);
                continue;
            }

            int32_t damagePlus;
            if (!storm::ParseNumber(tokens[3], damagePlus))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse damage plus value",
                    c_pszFileName, lineNum);
                continue;
            }

            float pct;
            if (!storm::ParseNumber(tokens[4], pct))
            {
                SPDLOG_ERROR(
                    "{0}:{1}: Could not parse percentage",
                    c_pszFileName, lineNum);
                continue;
            }

            pct *= 10000;
            m_attackPetItemDropInfos.emplace_back(vnum, mobVnum, scalePlus, damagePlus, pct);
            ++lineNum;
        }
    }

    return true;
}

bool ITEM_MANAGER::ReadDoubleItemAttrAllow(const char *c_pszFileName)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load item attr double allow file {0}",
                     c_pszFileName);
        return false;
    }

    const auto list = reader.GetList("DoubleAttrAllow");
    if (list)
    {
        const auto &lines = list->GetLines();
        for (const auto &line : lines)
        {
            const auto applyType = GetApplyTypeValueByString(line.c_str());
            if (applyType == -1)
                continue;

            m_doubleApplyTypeAllow.emplace_back(applyType);
        }
    }

    for (const auto apply : m_doubleApplyTypeAllow)
        SPDLOG_INFO("DoubleAttrAllow {0}", apply);

    return true;
}

ItemApply ITEM_MANAGER::GetLevelPetRandomBonus() { return *Random::get(m_levelPetAvailableBonus); }

uint32_t ITEM_MANAGER::GetPetNeededExpByLevel(Level lv) { return m_levelPetExpTable[lv]; }

ApplyValue ITEM_MANAGER::GetLevelPetBonusPerPoint(ApplyType t) { return m_levelPetBonusPerStatPoint[t]; }

ApplyValue ITEM_MANAGER::GetLevelPetApplyDefault(ApplyType t)
{
    auto it = std::find_if(m_levelPetAvailableBonus.begin(), m_levelPetAvailableBonus.end(), [t](ItemApply &ia)
    {
        return ia.type == t;
    });

    if (it == m_levelPetAvailableBonus.end())
        return 0.0f;

    return (*it).value;
}

bool ITEM_MANAGER::IsDoubleApplyAllow(uint32_t apply)
{
    return std::find(m_doubleApplyTypeAllow.begin(),
                     m_doubleApplyTypeAllow.end(), apply) != m_doubleApplyTypeAllow.end();
}

bool ITEM_MANAGER::IsEnhanceTimeExceptionItem(uint32_t vnum)
{
    return (m_enhanceExceptionVnums.find(vnum) != m_enhanceExceptionVnums.end());
}

storm::StringValueTable<uint32_t> kSpecialItemGroupType[] = {
    {"Normal", CSpecialItemGroup::NORMAL},
    {"Pct", CSpecialItemGroup::PCT},
    {"Quest", CSpecialItemGroup::QUEST},
    {"Special", CSpecialItemGroup::SPECIAL},
    {"Rand", CSpecialItemGroup::RAND},
    {"Attr", CSpecialItemGroup::ATTR},
};

storm::StringValueTable<uint32_t> kSpecialItemGroupAwardType[] = {
    {"gold", CSpecialItemGroup::GOLD},
    {"exp", CSpecialItemGroup::EXP},
    {"mob", CSpecialItemGroup::MOB},
    {"slow", CSpecialItemGroup::SLOW},
    {"drain_hp", CSpecialItemGroup::DRAIN_HP},
    {"group", CSpecialItemGroup::MOB_GROUP},
    {"poly", CSpecialItemGroup::POLY_MARBLE},
};

bool ITEM_MANAGER::ReadSpecialDropItemFile(const char *c_pszFileName, bool isReloading)
{
    std::map<uint32_t, std::unique_ptr<CSpecialAttrGroup>> tempSpecAttr;
    std::map<uint32_t, std::unique_ptr<CSpecialItemGroup>> tempSpecItem;
    std::map<uint32_t, std::unique_ptr<CSpecialItemGroup>> tempSpecItemQuest;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load config file {0}", c_pszFileName);
        return false;
    }

    for (const auto &p : reader.GetChildren())
    {
        auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        // Allowed - see type check above
        auto grp = static_cast<GroupTextGroup *>(node);

        GenericVnum iVnum;
        if (!GetGroupProperty(grp, "vnum", iVnum))
        {
            SPDLOG_ERROR("Group {0}: missing 'Vnum'", grp->GetName());
            return false;
        }
        uint32_t type = CSpecialItemGroup::NORMAL;
        const auto groupType = grp->GetProperty("type");
        if (!groupType.empty())
        {
            if (!storm::ParseStringWithTable(groupType,
                                             type,
                                             kSpecialItemGroupType))
            {
                SPDLOG_ERROR("Group {0}: Unknown type '{1}'",
                             grp->GetName(), std::string(groupType.data(), groupType.length()));
                return false;
            }
        }
        else { type = CSpecialItemGroup::NORMAL; }

        if (type == CSpecialItemGroup::QUEST)
            quest::CQuestManager::instance().RegisterNPCVnum(iVnum);

        if (type != CSpecialItemGroup::ATTR)
        {
            auto pkGroup = std::make_unique<CSpecialItemGroup>(iVnum, type);

            auto items = grp->GetList("Items");
            if (items)
            {
                std::vector<storm::StringRef> tokens;
                int32_t lineNum = 1;
                for (const auto &line : items->GetLines())
                {
                    tokens.clear();
                    storm::ExtractArguments(line, tokens);

                    if (tokens.size() < 3)
                    {
                        SPDLOG_WARN(
                            "ReadSpecialDropItemFile Ignoring line {} in Group {} invalid token count atleast 3 tokens required",
                            lineNum, p.first);
                        continue;
                    }

                    uint32_t dwVnum = 0;

                    std::string name(tokens[0].data(), tokens[0].size());

                    if (!GetVnumByOriginalName(name.c_str(), dwVnum))
                    {
                        dwVnum = CSpecialItemGroup::NONE;
                        if (!name.empty())
                        {
                            if (storm::ParseStringWithTable(name,
                                                             dwVnum,
                                                             kSpecialItemGroupAwardType))
                            {
                                
                            } else {
                                 if (!storm::ParseNumber(name, dwVnum))
                                {
                                    SPDLOG_ERROR("Group {0}: Invalid vnum {1}", grp->GetName(), name);
                                    return false;
                                }
                                if (!ITEM_MANAGER::instance().GetTable(dwVnum))
                                {
                                    SPDLOG_ERROR("Group {0}: Item {1} does not exist", grp->GetName(), name);
                                    return false;
                                }
                            }
                        }
                    }

                    SocketValue iCount = 0;
                    storm::ParseNumber(tokens[1], iCount);

                    int iProb = 0;
                    storm::ParseNumber(tokens[2], iProb);

                    int iRarePct = 0;
                    if (tokens.size() > 3) { storm::ParseNumber(tokens[3], iRarePct); }

                    pkGroup->AddItem(dwVnum, iCount, iProb, iRarePct);
                    m_itemOriginMap[dwVnum].emplace_back(TWikiItemOriginInfo{static_cast<GenericVnum>(iVnum), false});

                    // CHECK_UNIQUE_GROUP
                    if (iVnum < 30000) { m_ItemToSpecialGroup[dwVnum] = iVnum; }
                    // END_OF_CHECK_UNIQUE_GROUP
                }
            }

            if (CSpecialItemGroup::QUEST == type)
            {
                if (isReloading)
                    tempSpecItemQuest.emplace(iVnum, std::move(pkGroup));
                else
                    m_map_pkQuestItemGroup.emplace(iVnum, std::move(pkGroup));
            }
            else
            {
                if (isReloading)
                    tempSpecItem.emplace(iVnum, std::move(pkGroup));
                else
                    m_map_pkSpecialItemGroup.emplace(iVnum, std::move(pkGroup));
            }
        }
        else
        {
            auto pkGroup = std::make_unique<CSpecialAttrGroup>(iVnum);

            auto items = grp->GetList("Items");
            if (items)
            {
                std::vector<storm::StringRef> tokens;
                int32_t lineNum = 1;
                for (const auto &line : items->GetLines())
                {
                    tokens.clear();
                    storm::ExtractArguments(line, tokens);

                    if (tokens.size() < 2)
                    {
                        SPDLOG_WARN(
                            "ReadSpecialDropItemFile Ignoring line {} in Group {} invalid token count 2 tokens required",
                            lineNum, p.first);
                        continue;
                    }

                    uint32_t apply_type = 0;
                    int apply_value = 0;

                    if (!storm::ParseNumber(tokens[0], apply_type))
                    {
                        if (!GetApplyTypeValue(tokens[0], apply_type) || 0 == apply_type)
                        {
                            SPDLOG_ERROR("Invalid APPLY_TYPE {} in Special Item Group Vnum {}", tokens[0], iVnum);
                            return false;
                        }
                    }
                    storm::ParseNumber(tokens[1], apply_value);
                    if (apply_type > MAX_APPLY_NUM)
                    {
                        SPDLOG_ERROR("Invalid APPLY_TYPE {} in Special Item Group Vnum {}", apply_type, iVnum);
                        return false;
                    }
                    pkGroup->m_vecAttrs.emplace_back(apply_type, apply_value);
                }

                GetGroupProperty(grp, "effect", pkGroup->m_stEffectFileName);

                if (isReloading)
                    tempSpecAttr.emplace(iVnum, std::move(pkGroup));
                else
                    m_map_pkSpecialAttrGroup.emplace(iVnum, std::move(pkGroup));
            }
        }
    }

    if (isReloading)
    {
        m_map_pkQuestItemGroup.clear();
        m_map_pkSpecialItemGroup.clear();
        m_map_pkSpecialAttrGroup.clear();

        m_map_pkSpecialAttrGroup.swap(tempSpecAttr);
        m_map_pkQuestItemGroup.swap(tempSpecItemQuest);
        m_map_pkSpecialItemGroup.swap(tempSpecItem);
    }
    return true;
}

bool ITEM_MANAGER::ReadAddSocketBlock(const char *c_pszFileName, bool isReloading)
{

    std::ifstream inf(c_pszFileName);

    if (!inf.is_open())
    {
        SPDLOG_ERROR("ArenaManager cannot open 'arena_forbidden_items.txt'.");
        return false;
    }

    std::string ln;
    while (getline(inf, ln))
    {
        if (ln.empty())
            continue;

        if (absl::StrContains(ln, "~"))
        {
            std::vector<absl::string_view> v = absl::StrSplit(ln, '~'); // Can also use ","
            if (v.size() < 2)
                continue;

            uint32_t from;
            if (!absl::SimpleAtoi(v[0], &from))
                continue;

            uint32_t to;
            if (!absl::SimpleAtoi(v[1], &to))
                continue;

            if (from > to)
                to = from;

            for (int i = from; i <= to; ++i) { m_blockSocketAdd.push_back(i); }
        }
        else
        {
            uint32_t iVnum;
            if(storm::ParseNumber(ln, iVnum)) {
                m_blockSocketAdd.push_back(iVnum);
            } 
        }
    }

    return true;
}
bool ITEM_MANAGER::ReadEtcDropItemFile(const char *c_pszFileName, bool isReloading)
{
    FILE *fp = fopen(c_pszFileName, "r");

    if (!fp)
    {
        SPDLOG_ERROR("Cannot open %s", c_pszFileName);
        return false;
    }

    char buf[512];

    int lines = 0;

    while (fgets(buf, 512, fp))
    {
        ++lines;

        if (!*buf || *buf == '\n')
            continue;

        char szItemName[256];
        float fProb = 0.0f;

        strlcpy(szItemName, buf, sizeof(szItemName));
        char *cpTab = strrchr(szItemName, '\t');

        if (!cpTab)
            continue;

        *cpTab = '\0';
        storm::ParseNumber(cpTab + 1, fProb);

        if (!*szItemName || fProb == 0.0f)
            continue;

        uint32_t dwItemVnum;
        if (!storm::ParseNumber(szItemName, dwItemVnum))
        {
            SPDLOG_ERROR("Invalid vnum %s", szItemName);
            fclose(fp);
            return false;
        }

        m_map_dwEtcItemDropProb[dwItemVnum] = (uint32_t)(fProb * 10000.0f);
        SPDLOG_INFO("ETC_DROP_ITEM: %s prob %f", szItemName, fProb);
    }

    fclose(fp);
    return true;
}

bool ITEM_MANAGER::ReadPetItemEvolutionTable(const char *c_pszFileName, bool isReloading)
{
    std::ifstream file(c_pszFileName);

    if (!file.is_open())
    {
        SPDLOG_ERROR("Cannot open {0}", c_pszFileName);
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line[0] == '#')
            continue;

        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of("\t"));

        if (tokens.size() != 3)
            continue;

        uint32_t fromVnum;
        if (!storm::ParseNumber(tokens[0].c_str(), fromVnum))
        {
            SPDLOG_ERROR("Invalid vnum {0}", tokens[0]);
            return false;
        }

        uint32_t toVnum;
        if (!storm::ParseNumber(tokens[1].c_str(), toVnum))
        {
            SPDLOG_ERROR("Invalid vnum {0}", tokens[1]);
            return false;
        }

        uint32_t evolType;
        if (!storm::ParseNumber(tokens[2].c_str(), evolType))
        {
            SPDLOG_ERROR("Invalid vnum {0}", tokens[2]);
            return false;
        }

        m_map_petEvolutionFromToVnum.push_back({evolType, fromVnum, toVnum});
        SPDLOG_INFO("PET_EVOLUTION: from {0} to {1} with type {2} ", fromVnum, toVnum, evolType);
    }

    return true;
}

bool ITEM_MANAGER::ReadMonsterDropItemGroup(const char *c_pszFileName, bool isReloading)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(c_pszFileName))
    {
        SPDLOG_ERROR("Failed to load config file {0}", c_pszFileName);
        return false;
    }

    std::map<uint32_t, std::unique_ptr<CMobItemGroup>> temMobItemGr;
    std::map<uint32_t, std::unique_ptr<CDropItemGroup>> tempDropItemGr;
    std::map<uint32_t, std::unique_ptr<CLevelItemGroup>> tempLevelItemGr;

    if (isReloading) { SPDLOG_INFO("RELOADING MonsterDrop"); }

    for (const auto &p : reader.GetChildren())
    {
        auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        // Allowed - see type check above
        auto grp = static_cast<GroupTextGroup *>(node);

        GenericVnum iMobVnum = 0;
        int iKillDrop = 0;
        int iLevelLimit = 0;

        const auto strType = grp->GetProperty("type");

        if (strType.empty())
        {
            SPDLOG_ERROR("ReadMonsterDropItemGroup : Syntax error %s : no type (kill|drop), node %s", c_pszFileName,
                         p.first);
            return false;
        }

        if (!GetGroupProperty(grp, "mob", iMobVnum))
        {
            SPDLOG_ERROR("ReadMonsterDropItemGroup : Syntax error %s : no mob vnum, node %s", c_pszFileName, p.first);
            return false;
        }

        if (strType == "kill")
        {
            if (!GetGroupProperty(grp, "kill_drop", iKillDrop))
            {
                SPDLOG_ERROR("ReadMonsterDropItemGroup : Syntax error %s : no kill drop count, node %s", c_pszFileName,
                             p.first);
                return false;
            }
        }
        else { iKillDrop = 1; }

        if (strType == "limit")
        {
            if (!GetGroupProperty(grp, "level_limit", iLevelLimit))
            {
                SPDLOG_ERROR("ReadMonsterDropItemGroup : Syntax error %s : no level_limit, node %s", c_pszFileName,
                             p.first);
                return false;
            }
        }
        else { iLevelLimit = 0; }

        if (iKillDrop == 0) { continue; }

        if (strType == "kill")
        {
            auto pkGroup = std::make_unique<CMobItemGroup>(iMobVnum, iKillDrop, p.first);

            auto items = grp->GetList("Items");
            if (items)
            {
                std::vector<storm::StringRef> tokens;
                int32_t lineNum = 1;
                for (const auto &line : items->GetLines())
                {
                    tokens.clear();
                    storm::ExtractArguments(line, tokens);

                    if (tokens.size() < 3)
                    {
                        SPDLOG_WARN("LoadMobDropItem Ignoring line {} in Group {} invalid token count", lineNum,
                                    p.first);
                        continue;
                    }

                    uint32_t vnum = 0;
                    if (is_number(tokens[0]))
                        storm::ParseNumber(tokens[0], vnum);

                    if (!vnum)
                    {
                        GetVnumByOriginalName(tokens[0].data(), vnum);
                        if (!ITEM_MANAGER::instance().GetTable(vnum))
                        {
                            SPDLOG_ERROR("ReadMonsterDropItemGroup : there is no item {0} : node {1} : vnum {2}",
                                         tokens[0], p.first, vnum);
                            return false;
                        }
                    }

                    uint32_t count = 0;
                    storm::ParseNumber(tokens[1], count);

                    if (!count)
                    {
                        SPDLOG_ERROR(
                            "ReadMonsterDropItemGroup : there is no count for item {0} : node {1} : vnum {2}, count {3}",
                            tokens[0], p.first, vnum, count);
                        return false;
                    }

                    uint32_t pct = 0;
                    storm::ParseNumber(tokens[2], pct);

                    if (!pct)
                    {
                        SPDLOG_ERROR(
                            "ReadMonsterDropItemGroup : there is no drop percent for item {0} : node {1} : vnum {2}, count {3}, pct {4}",
                            tokens[0], p.first, pct);
                    }

                    uint32_t rarePct = 0;
                    if (tokens.size() == 4) { storm::ParseNumber(tokens[3], rarePct); }

                    pkGroup->AddItem(vnum, count, pct, rarePct);

                    auto tbl = GetItemWikiInfo(vnum);
                    if (tbl && !tbl->dwOrigin)
                        tbl->dwOrigin = iMobVnum;

                    m_itemOriginMap[vnum].emplace_back(TWikiItemOriginInfo{static_cast<GenericVnum>(iMobVnum), true});
                    CMobManager::instance().GetMobWikiInfo(iMobVnum).push_back(vnum);

                    ++lineNum;
                }
            }
            else
            {
                SPDLOG_ERROR("ReadMonsterDropItemGroup : Group {} is missing a Items list", p.first);
                return false;
            }

            if (isReloading)
                temMobItemGr.emplace(iMobVnum, std::move(pkGroup));
            else
                m_map_pkMobItemGroup.emplace(iMobVnum, std::move(pkGroup));

            return true;
        }
        else if (strType == "drop")
        {
            std::unique_ptr<CDropItemGroup> pkGroup;
            bool bNew = true;
            if (isReloading)
            {
                auto it = tempDropItemGr.find(iMobVnum);
                if (it == tempDropItemGr.end()) { pkGroup = std::make_unique<CDropItemGroup>(0, iMobVnum, p.first); }
                else
                {
                    bNew = false;
                    pkGroup = std::move(it->second);
                }
            }
            else
            {
                auto it = m_map_pkDropItemGroup.find(iMobVnum);
                if (it == m_map_pkDropItemGroup.end())
                {
                    pkGroup = std::make_unique<CDropItemGroup>(0, iMobVnum, p.first);
                }
                else
                {
                    bNew = false;
                    pkGroup = std::move(it->second);
                }
            }

            auto items = grp->GetList("Items");
            if (items)
            {
                std::vector<storm::StringRef> tokens;
                int32_t lineNum = 1;
                for (const auto &line : items->GetLines())
                {
                    tokens.clear();
                    storm::ExtractArguments(line, tokens);

                    if (tokens.size() < 3)
                    {
                        SPDLOG_WARN("LoadMobDropItem Ignoring line {} in Group {} invalid token count", lineNum,
                                    p.first);
                        continue;
                    }

                    uint32_t vnum = 0;
                    if (is_number(tokens[0]))
                        storm::ParseNumber(tokens[0], vnum);

                    if (!vnum)
                    {
                        GetVnumByOriginalName(tokens[0].data(), vnum);
                        if (!ITEM_MANAGER::instance().GetTable(vnum))
                        {
                            SPDLOG_ERROR("ReadMonsterDropItemGroup : there is no item {0} : node {1} : vnum {2}",
                                         tokens[0], p.first, vnum);
                            return false;
                        }
                    }

                    uint32_t count = 0;
                    storm::ParseNumber(tokens[1], count);

                    if (!count)
                    {
                        SPDLOG_ERROR(
                            "ReadMonsterDropItemGroup : there is no count for item {0} : node {1} : vnum {2}, count {3}",
                            tokens[0], p.first, vnum, count);
                        return false;
                    }

                    double pct = 0.0;
                    if (!storm::ParseNumber(tokens[2], pct))
                    {
                        SPDLOG_ERROR(
                            "ReadMonsterDropItemGroup : Invalid drop percent for item {0} : node {1} : vnum {2}, count {3}, pct {4}",
                            tokens[0], p.first, tokens[2]);
                        return false;
                    }

                    uint32_t dwPct = (uint32_t)(10000.0 * pct);
                    pkGroup->AddItem(vnum, dwPct, count);

                    auto *tbl = GetItemWikiInfo(vnum);
                    if (tbl && !tbl->dwOrigin)
                        tbl->dwOrigin = iMobVnum;
                    TWikiItemOriginInfo oi = {iMobVnum, true};

                    m_itemOriginMap[vnum].emplace_back(oi);
                    CMobManager::instance().GetMobWikiInfo(iMobVnum).push_back(vnum);
                }
            }
            else
            {
                SPDLOG_ERROR("ReadMonsterDropItemGroup : Group {} is missing a Items list", p.first);
                return false;
            }

            if (bNew)
            {
                if (isReloading)
                    tempDropItemGr.emplace(iMobVnum, std::move(pkGroup));
                else
                    m_map_pkDropItemGroup.emplace(iMobVnum, std::move(pkGroup));
            }
        }
        else if (strType == "limit")
        {
            auto pkLevelItemGroup = std::make_unique<CLevelItemGroup>(iLevelLimit);

            auto items = grp->GetList("Items");
            if (items)
            {
                std::vector<storm::StringRef> tokens;
                int32_t lineNum = 1;
                for (const auto &line : items->GetLines())
                {
                    tokens.clear();
                    storm::ExtractArguments(line, tokens);

                    if (tokens.size() < 3)
                    {
                        SPDLOG_WARN("LoadMobDropItem Ignoring line {} in Group {} invalid token count", lineNum,
                                    p.first);
                        continue;
                    }

                    uint32_t vnum = 0;
                    if (is_number(tokens[0]))
                        storm::ParseNumber(tokens[0], vnum);

                    if (!vnum)
                    {
                        GetVnumByOriginalName(tokens[0].data(), vnum);
                        if (!ITEM_MANAGER::instance().GetTable(vnum))
                        {
                            SPDLOG_ERROR("ReadMonsterDropItemGroup : there is no item {0} : node {1} : vnum {2}",
                                         tokens[0], p.first, vnum);
                            return false;
                        }
                    }

                    uint32_t count = 0;
                    storm::ParseNumber(tokens[1], count);

                    if (!count)
                    {
                        SPDLOG_ERROR(
                            "ReadMonsterDropItemGroup : there is no count for item {0} : node {1} : vnum {2}, count {3}",
                            tokens[0], p.first, vnum, count);
                        return false;
                    }

                    double pct = 0.0;
                    if (!storm::ParseNumber(tokens[2], pct))
                    {
                        SPDLOG_ERROR(
                            "ReadMonsterDropItemGroup : Invalid drop percent for item {0} : node {1} : vnum {2}, count {3}, pct {4}",
                            tokens[0], p.first, tokens[2]);
                        return false;
                    }

                    uint32_t dwPct = (uint32_t)(10000.0 * pct);

                    pkLevelItemGroup->AddItem(vnum, dwPct, count);
                    TWikiItemOriginInfo oi = {iMobVnum, true};
                    m_itemOriginMap[vnum].emplace_back(oi);
                    auto tbl = GetItemWikiInfo(vnum);
                    if (tbl && !tbl->dwOrigin)
                        tbl->dwOrigin = iMobVnum;
                    CMobManager::instance().GetMobWikiInfo(iMobVnum).push_back(vnum);
                }
            }
            else
            {
                SPDLOG_ERROR("ReadMonsterDropItemGroup : Group {} is missing a Items list", p.first);
                return false;
            }

            if (isReloading)
                tempLevelItemGr.emplace(iMobVnum, std::move(pkLevelItemGroup));
            else
                m_map_pkLevelItemGroup.emplace(iMobVnum, std::move(pkLevelItemGroup));
        }
        else
        {
            SPDLOG_ERROR("ReadMonsterDropItemGroup : Syntax error %s : invalid type %s (kill|drop), node %s",
                         c_pszFileName, strType, p.first.c_str());
            return false;
        }
    }

    if (isReloading)
    {
        m_map_pkLevelItemGroup.clear();
        m_map_pkDropItemGroup.clear();
        m_map_pkMobItemGroup.clear();

        m_map_pkLevelItemGroup.swap(tempLevelItemGr);
        m_map_pkDropItemGroup.swap(tempDropItemGr);
        m_map_pkMobItemGroup.swap(temMobItemGr);
    }
    return true;
}

bool ITEM_MANAGER::ReadItemVnumMaskTable(const char *c_pszFileName)
{
    FILE *fp = fopen(c_pszFileName, "r");
    if (!fp) { return false; }

    int ori_vnum, new_vnum;
    while (fscanf(fp, "%u %u", &ori_vnum, &new_vnum) != EOF)
    {
        m_map_new_to_ori.insert(TMapDW2DW::value_type(new_vnum, ori_vnum));
    }
    fclose(fp);
    return true;
}

bool ITEM_MANAGER::ReadItemDebuffTable(const char *c_pszFileName)
{
    FILE *fp = fopen(c_pszFileName, "r");
    if (!fp) { return false; }

    int ori_vnum, new_vnum;
    while (fscanf(fp, "%u %u", &ori_vnum, &new_vnum) != EOF) { m_map_ori_to_debuffed.emplace(ori_vnum, new_vnum); }
    fclose(fp);
    return true;
}
