#include "NpcManager.h"
#include "../EterBase/lzo.h"
#include "../GameLib/RaceManager.h"
#include "StdAfx.h"
#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <game/MobConstants.hpp>
#include <pak/ClientDb.hpp>
#include <pak/Vfs.hpp>

#include "pak/Util.hpp"

bool NpcManager::LoadRaceHeight(const std::string &filename)
{
    auto data = LoadFileToString(GetVfs(), filename);
    if (!data)
        return false;

    if (!GetVfs().Open(filename))
    {
        SPDLOG_DEBUG("NpcManager::LoadRaceHeight(c_szFileName={0}) - Load Error", filename);
        return false;
    }

    CMemoryTextFileLoader kTextFileLoader;
    kTextFileLoader.Bind(data.value());

    CTokenVector args;
    for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
    {
        if (!kTextFileLoader.SplitLineByTab(i, &args))
            continue;

        if (args.size() < 2)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has only {} tokens", filename, i, args.size());
            return false;
        }

        try
        {
            uint32_t vnum = std::stoul(args[0]);
            float height = std::stof(args[1]);
            CRaceManager::Instance().SetRaceHeight(vnum, height);
        }
        catch (const std::invalid_argument &ia)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has invalid token {}", filename, i, args[0]);
            return false;
        }
        catch (const std::out_of_range &oor)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has out of range token {}", filename, i, args[0]);
            return false;
        }
    }

    return true;
}

bool NpcManager::LoadWorldBossInfo(const char *szFileName)
{
    auto data = LoadFileToString(GetVfs(), szFileName);
    if (!data)
        return false;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data.value()))
    {
        return false;
    }

    for (const auto &p : reader.GetChildren())
    {
        const auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;
        const auto grp = static_cast<GroupTextGroup *>(node);

        TWorldBossInfo kWorldBossInfo;
        memset(&kWorldBossInfo, 0, sizeof(TWorldBossInfo));

        // min/max level are optional
        uint32_t day = 0;
        uint32_t hour = 0;
        uint32_t mob = std::numeric_limits<uint32_t>::max();
        GetGroupProperty(grp, "day", kWorldBossInfo.bDay);
        GetGroupProperty(grp, "hour", kWorldBossInfo.bHour);
        GetGroupProperty(grp, "mob", kWorldBossInfo.dwRaceNum);
        GetGroupProperty(grp, "daily", kWorldBossInfo.bIsDaily);

        // get maps
        std::vector<WorldBossPosition> vecMapIndexes;
        std::vector<std::string> vecMapNames;

        for (int k = 1; k < 256; ++k)
        {
            char buf[4];
            snprintf(buf, sizeof(buf), "%d", k);

            const auto tokens = grp->GetTokens(buf);
            if (!tokens)
                break;
            if (tokens->size() > 3)
            {
                vecMapNames.push_back(tokens->at(0));
                uint32_t iMapIndex = 0;
                storm::ParseNumber(tokens->at(1), iMapIndex);
                uint32_t x = 0;
                storm::ParseNumber(tokens->at(2).c_str(), x);
                uint32_t y = 0;
                storm::ParseNumber(tokens->at(3).c_str(), y);

                vecMapIndexes.emplace_back(iMapIndex, x, y);
            }
        }

        // alloc map memory
        kWorldBossInfo.iMapCount = vecMapIndexes.size();

        // save maps
        std::swap(kWorldBossInfo.piMapIndexList, vecMapIndexes);
        std::swap(kWorldBossInfo.pszMapNameList, vecMapNames);

        m_vec_kWorldBossInfo.emplace_back(kWorldBossInfo);
    }

    return true;
}

const std::vector<TWorldBossInfo> &NpcManager::GetWorldBossInfos()
{
    return m_vec_kWorldBossInfo;
}

bool NpcManager::LoadHugeRace(const std::string &filename)
{
    auto data = LoadFileToString(GetVfs(), filename.c_str());
    if (!data)
        return false;

    CMemoryTextFileLoader kTextFileLoader;
    kTextFileLoader.Bind(data.value());

    CTokenVector args;
    for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
    {
        if (!kTextFileLoader.SplitLineByTab(i, &args))
            continue;

        if (args.size() < 1)
        {
            SPDLOG_ERROR("Huge race list '{}' line {} has only {} tokens", filename, i, args.size());
            return false;
        }

        try
        {
            uint32_t vnum = std::stoul(args[0]);
            CRaceManager::Instance().SetHugeRace(vnum);
        }
        catch (const std::invalid_argument &ia)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has invalid token {}", filename, i, args[0]);
            return false;
        }
        catch (const std::out_of_range &oor)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has out of range token {}", filename, i, args[0]);
            return false;
        }
    }

    return true;
}

bool NpcManager::LoadNpcList(const std::string &filename)
{
    auto data = LoadFileToString(GetVfs(), filename.c_str());
    if (!data)
        return false;

    CMemoryTextFileLoader kTextFileLoader;
    kTextFileLoader.Bind(data.value());

    CTokenVector args;
    for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
    {
        if (!kTextFileLoader.SplitLineByTab(i, &args))
            continue;

        if (args.size() < 2)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has only {} tokens", filename, i, args.size());
            return false;
        }

        try
        {
            uint32_t vnum = std::stoul(args[0]);
            CRaceManager::Instance().RegisterRace(vnum, args[1]);
        }
        catch (const std::invalid_argument &ia)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has invalid token {}", filename, i, args[0]);
            return false;
        }
        catch (const std::out_of_range &oor)
        {
            SPDLOG_ERROR("Npc race list '{}' line {} has out of range token {}", filename, i, args[0]);
            return false;
        }
    }

    return true;
}

bool NpcManager::LoadSpeculaSettingFile(const char *c_szFileName)
{
    auto data = LoadFileToString(GetVfs(), c_szFileName);
    if (!data)
        return false;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data.value()))
    {
        return false;
    }

    const auto rareItems = reader.GetList("SpecularSetting");
    if (rareItems != nullptr)
    {
        for (uint32_t i = 0; i < rareItems->GetLineCount(); ++i)
        {
            auto sb = std::string(rareItems->GetLine(i));
            std::vector<std::string> line_parts;
            boost::erase_all(sb, "\"");
            boost::trim(sb);
            boost::split(line_parts, sb, boost::is_any_of("\t "), boost::token_compress_on);

            if ((line_parts.size() < 2))
            {
                SPDLOG_ERROR("Invalid line {0} only has {1} tokens", i, line_parts.size());
                continue;
            }

            try
            {
                const auto vnum = std::stoul(line_parts.at(0));
                const auto speculuar = std::stof(line_parts.at(1));

                m_specularConfig.emplace(vnum, speculuar);
            }
            catch (const std::invalid_argument &ia)
            {
                SPDLOG_ERROR("SpecularSetting Invalid line {} has invalid token {}", i, line_parts.at(0));
                return false;
            }
            catch (const std::out_of_range &oor)
            {
                SPDLOG_ERROR("SpecularSetting Invalid line {} has out of range token {}", i, line_parts.at(0));
                return false;
            }
        }
    }

    return true;
}

bool NpcManager::LoadNonPlayerData(const char *c_szFileName)
{
    std::vector<TMobTable> v;
    if (!LoadClientDb(GetVfs(), "data/mob_proto", v))
        return false;

    for (const auto &mob : v)
    {
        auto &nonPlayerElem = m_protoMap[mob.dwVnum];
        nonPlayerElem.mobTable = mob;
        nonPlayerElem.isSet = false;
        nonPlayerElem.isFiltered = false;
        nonPlayerElem.dropList.clear();
    }

    return true;
}

bool NpcManager::LoadNames(const char *filename)
{
    auto str = LoadFileToString(GetVfs(), filename);
    if (!str)
    {
        SPDLOG_DEBUG("NpcManager::LoadNames(c_szFileName={0}) - Load Error", filename);
        return false;
    }

    CMemoryTextFileLoader kTextFileLoader;
    kTextFileLoader.Bind(str.value());

    CTokenVector args;
    for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
    {
        if (!kTextFileLoader.SplitLineByTab(i, &args))
            continue;

        if (args.size() < 2)
        {
            SPDLOG_ERROR("Mob name list ' {}' line {} has only {} tokens", filename, i, args.size());
            return false;
        }

        try
        {
            uint32_t vnum = std::stoul(args[0]);
            m_nameMap[vnum] = args[1];
        }
        catch (const std::invalid_argument &ia)
        {
            SPDLOG_ERROR("Mob Names '{}' line {} has invalid token {}", filename, i, args[0]);
            return false;
        }
        catch (const std::out_of_range &oor)
        {
            SPDLOG_ERROR("Mob Names '{}' line {} has out of range token {}", filename, i, args[0]);
            return false;
        }
    }

    return true;
}

size_t NpcManager::WikiLoadClassMobs(BYTE bType, WORD fromLvl, WORD toLvl)
{
    m_vecTempMob.clear();
    for (auto it = m_protoMap.begin(); it != m_protoMap.end(); ++it)
    {
        if (it->second.isFiltered && it->second.mobTable.bLevel >= fromLvl && it->second.mobTable.bLevel < toLvl)
        {
            if (bType == 0 && it->second.mobTable.bType == CHAR_TYPE_MONSTER && it->second.mobTable.bRank >= 4)
                m_vecTempMob.push_back(it->first);
            else if (bType == 1 && it->second.mobTable.bType == CHAR_TYPE_MONSTER && it->second.mobTable.bRank < 4)
                m_vecTempMob.push_back(it->first);
            else if (bType == 2 && it->second.mobTable.bType == CHAR_TYPE_STONE)
                m_vecTempMob.push_back(it->first);
        }
    }
    return m_vecTempMob.size();
}

void NpcManager::WikiSetBlacklisted(DWORD vnum)
{
    auto it = m_protoMap.find(vnum);
    if (it != m_protoMap.end())
        it->second.isFiltered = true;
}

std::optional<std::string> NpcManager::GetName(uint32_t dwVnum)
{
    const auto it = m_nameMap.find(dwVnum);
    if (it != m_nameMap.end())
    {
        return it->second;
    }

    auto p = GetTable(dwVnum);
    if (!p)
        return std::nullopt;

    return p->szLocaleName;
}

bool NpcManager::GetInstanceType(uint32_t dwVnum, uint8_t *pbType)
{
    const auto *p = GetTable(dwVnum);

    // dwVnum를 찾을 수 없으면 플레이어 캐릭터로 간주 한다. 문제성 코드 -_- [cronan]
    if (!p)
        return false;

    *pbType = p->bType;

    return true;
}

NpcManager::TWikiInfoTable *NpcManager::GetWikiTable(DWORD dwVnum)
{
    TNonPlayerDataMap::iterator itor = m_protoMap.find(dwVnum);

    if (itor == m_protoMap.end())
        return NULL;

    return &(itor->second);
}

const TMobTable *NpcManager::GetTable(uint32_t dwVnum)
{
    const auto itor = m_protoMap.find(dwVnum);

    if (itor == m_protoMap.end())
        return NULL;

#ifdef INGAME_WIKI
    return &itor->second.mobTable;
#else
    return &itor->second;
#endif
}

uint8_t NpcManager::GetEventType(uint32_t dwVnum)
{
    const TMobTable *p = GetTable(dwVnum);

    if (!p)
    {
        // SPDLOG_DEBUG("NpcManager::GetEventType - Failed to find virtual number\n");
        return ON_CLICK_NONE;
    }

    return p->bOnClickType;
}

uint32_t NpcManager::GetAttElementFlag(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        return 0;
    }

    if (IS_SET(c_pTable->dwRaceFlag, RACE_FLAG_ATT_ELEC))
        return RACE_FLAG_ATT_ELEC;

    if (IS_SET(c_pTable->dwRaceFlag, RACE_FLAG_ATT_FIRE))
        return RACE_FLAG_ATT_FIRE;

    if (IS_SET(c_pTable->dwRaceFlag, RACE_FLAG_ATT_ICE))
        return RACE_FLAG_ATT_ICE;

    if (IS_SET(c_pTable->dwRaceFlag, RACE_FLAG_ATT_WIND))
        return RACE_FLAG_ATT_WIND;

    if (IS_SET(c_pTable->dwRaceFlag, RACE_FLAG_ATT_EARTH))
        return RACE_FLAG_ATT_EARTH;

    if (IS_SET(c_pTable->dwRaceFlag, RACE_FLAG_ATT_DARK))
        return RACE_FLAG_ATT_DARK;

    return 0;
}

#ifdef INGAME_WIKI
DWORD NpcManager::GetVnumByNamePart(const char *c_pszName)
{
    if (m_vecWikiNameSort.size() == 0)
        return 0;

    int iMaxLimit = m_vecWikiNameSort.size() - 1;
    int iMinLimit = 0;
    int iIndex = m_vecWikiNameSort.size() / 2;
    TMobTable *pData = NULL;

    std::string stLowerItemName = c_pszName;
    std::transform(stLowerItemName.begin(), stLowerItemName.end(), stLowerItemName.begin(), ::tolower);

    std::string stLowerNameCurrent;

    int iCompareLen = stLowerItemName.length();
    if (!iCompareLen)
        return 0;

    while (true)
    {
        pData = m_vecWikiNameSort[iIndex];
        const char *c_pszNameNow = pData->szLocaleName.c_str();
        stLowerNameCurrent = c_pszNameNow;
        std::transform(stLowerNameCurrent.begin(), stLowerNameCurrent.end(), stLowerNameCurrent.begin(), ::tolower);

        int iRetCompare = stLowerItemName.compare(0, iCompareLen, stLowerNameCurrent, 0, iCompareLen);
        if (iRetCompare != 0)
        {
            if (iRetCompare < 0) // search item name < current item name
                iMaxLimit = iIndex - 1;
            else // search item name > current item name
                iMinLimit = iIndex + 1;

            if (iMinLimit > iMaxLimit)
                return 0;

            iIndex = iMinLimit + (iMaxLimit - iMinLimit) / 2;
            continue;
        }

        int iOldIndex = iIndex;
        int iLastIndex, iLowSize = stLowerNameCurrent.length(), iHighSize = stLowerNameCurrent.length();
        int iLowIndex = iIndex;
        int iHighIndex = iIndex;

        iLastIndex = iIndex;
        while (iLastIndex > 0)
        {
            int iNewIndex = iLastIndex - 1;
            c_pszNameNow = m_vecWikiNameSort[iNewIndex]->szLocaleName.c_str();
            if (strnicmp(c_pszNameNow, stLowerItemName.c_str(), iCompareLen))
                break;

            iLastIndex = iNewIndex;
            if (strlen(c_pszNameNow) <= iLowSize)
            {
                iLowIndex = iNewIndex;
                iLowSize = strlen(c_pszNameNow);
            }
        }

        iLastIndex = iIndex;
        while (iLastIndex < m_vecWikiNameSort.size() - 1 - 1)
        {
            int iNewIndex = iLastIndex + 1;
            c_pszNameNow = m_vecWikiNameSort[iNewIndex]->szLocaleName.c_str();
            if (strnicmp(c_pszNameNow, stLowerItemName.c_str(), iCompareLen))
                break;

            iLastIndex = iNewIndex;
            if (strlen(c_pszNameNow) < iHighSize)
            {
                iHighIndex = iNewIndex;
                iHighSize = strlen(c_pszNameNow);
            }
        }

        if (iHighSize < iLowSize)
            iIndex = iHighIndex;
        else
            iIndex = iLowIndex;

        pData = m_vecWikiNameSort[iIndex];
        return pData->dwVnum;
    }
}

void NpcManager::BuildWikiSearchList()
{
    m_vecWikiNameSort.clear();
    for (auto it = m_protoMap.begin(); it != m_protoMap.end(); ++it)
        if (it->second.isFiltered)
            m_vecWikiNameSort.push_back(&it->second.mobTable);

    SortMobDataName();
}

void NpcManager::SortMobDataName()
{
    std::qsort(&m_vecWikiNameSort[0], m_vecWikiNameSort.size(), sizeof(m_vecWikiNameSort[0]),
               [](const void *a, const void *b) {
                   TMobTable *pItem1 = *(TMobTable **)(static_cast<const TMobTable *>(a));
                   TMobTable *pItem2 = *(TMobTable **)(static_cast<const TMobTable *>(b));
                   std::string stRealName1 = pItem1->szLocaleName;
                   std::transform(stRealName1.begin(), stRealName1.end(), stRealName1.begin(), ::tolower);
                   std::string stRealName2 = pItem2->szLocaleName;
                   std::transform(stRealName2.begin(), stRealName2.end(), stRealName2.begin(), ::tolower);

                   int iSmallLen = std::min(stRealName1.length(), stRealName2.length());
                   int iRetCompare = stRealName1.compare(0, iSmallLen, stRealName2, 0, iSmallLen);

                   if (iRetCompare != 0)
                       return iRetCompare;

                   if (stRealName1.length() < stRealName2.length())
                       return -1;
                   else if (stRealName2.length() < stRealName1.length())
                       return 1;

                   return 0;
               });
}

#endif

const char *NpcManager::GetMonsterName(uint32_t dwVnum)
{
    // Polymorph fix
    const auto it = m_nameMap.find(dwVnum);
    if (it != m_nameMap.end())
    {
        return it->second.c_str();
    }

    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        static const char *sc_szEmpty = "";
        return sc_szEmpty;
    }

    return c_pTable->szLocaleName.c_str();
}

uint32_t NpcManager::GetScale(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        return 100;
    }

    return MINMAX(1, c_pTable->wScale, 3000);
}

std::optional<std::unordered_map<uint32_t, std::string>> NpcManager::GetEntriesByName(const std::string &name,
                                                                                      int32_t limit)
{
    if (name.empty())
        return std::nullopt;

    int32_t i = 0;
    std::unordered_map<uint32_t, std::string> entries;
    for (const auto &entry : m_nameMap)
    {
        if (strstr(entry.second.c_str(), name.c_str()) != NULL)
        {
            entries.emplace(entry.first, entry.second);
            if (i >= limit)
                break;
            ++i;
        }
    }

    return entries;
}

uint32_t NpcManager::GetMobRank(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        return 0;
    }

    return c_pTable->bRank;
}

uint32_t NpcManager::GetMonsterColor(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
        return 0;

    return c_pTable->dwMonsterColor;
}

void NpcManager::GetMatchableMobList(int iLevel, int iInterval, TMobTableList *pMobTableList)
{
    /*
        pMobTableList->clear();

        TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.begin();
        for (; itor != m_NonPlayerDataMap.end(); ++itor)
        {
            TMobTable * pMobTable = itor->second;

            int iLowerLevelLimit = iLevel-iInterval;
            int iUpperLevelLimit = iLevel+iInterval;

            if ((pMobTable->abLevelRange[0] >= iLowerLevelLimit && pMobTable->abLevelRange[0] <= iUpperLevelLimit) ||
                (pMobTable->abLevelRange[1] >= iLowerLevelLimit && pMobTable->abLevelRange[1] <= iUpperLevelLimit))
            {
                pMobTableList->push_back(pMobTable);
            }
        }
    */
}

uint32_t NpcManager::GetMonsterExp(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        uint32_t dwExp = 0;
        return dwExp;
    }

    return c_pTable->dwExp;
}

float NpcManager::GetMonsterDamageMultiply(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        uint32_t fDamMultiply = 0;
        return fDamMultiply;
    }

    return c_pTable->fDamMultiply;
}

uint32_t NpcManager::GetMonsterST(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        uint32_t bStr = 0;
        return bStr;
    }

    return c_pTable->bStr;
}

uint32_t NpcManager::GetMonsterDX(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
    {
        uint32_t bDex = 0;
        return bDex;
    }

    return c_pTable->bDex;
}

uint16_t NpcManager::GetMobRegenCycle(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
        return 0;

    return c_pTable->bRegenCycle;
}

uint8_t NpcManager::GetMobRegenPercent(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
        return 0;

    return c_pTable->bRegenPercent;
}

uint32_t NpcManager::GetMobGoldMin(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
        return 0;

    return c_pTable->dwGoldMin;
}

uint32_t NpcManager::GetMobGoldMax(uint32_t dwVnum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
        return 0;

    return c_pTable->dwGoldMax;
}

uint32_t NpcManager::GetMobResist(uint32_t dwVnum, uint8_t bResistNum)
{
    const TMobTable *c_pTable = GetTable(dwVnum);
    if (!c_pTable)
        return 0;

    if (bResistNum >= MOB_RESISTS_MAX_NUM)
        return 0;

    return c_pTable->cResists[bResistNum];
}

bool NpcManager::HasSpecularConfig(uint32_t vnum)
{
    auto it = m_specularConfig.find(vnum);
    return it != m_specularConfig.end();
}

float NpcManager::GetSpecularValue(uint32_t vnum)
{
    auto it = m_specularConfig.find(vnum);

    if (it == m_specularConfig.end())
        return 0.0f;

    return it->second;
}

void NpcManager::Clear()
{
}

void NpcManager::Destroy()
{
    m_protoMap.clear();
}

NpcManager::NpcManager()
{
    Clear();
}

NpcManager::~NpcManager(void)
{
    Destroy();
}
