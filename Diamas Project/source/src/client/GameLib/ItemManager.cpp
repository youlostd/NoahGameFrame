#include "StdAfx.h"
#include <pak/Vfs.hpp>
#include "../eterLib/ResourceManager.h"

#include "ItemManager.h"
#include "../EterBase/lzo.h"
#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>
#include <game/Constants.hpp>
#include <game/ItemTypes.hpp>

#include <optional>
#include <fmt/printf.h>
#include <pak/ClientDb.hpp>

#include "pak/Util.hpp"

bool CItemManager::SelectItemData(uint32_t dwIndex)
{
    const auto f = m_ItemMap.find(dwIndex);

    if (m_ItemMap.end() == f)
    {
        int n = m_vec_ItemRange.size();
        for (int i = 0; i < n; i++)
        {
            CItemData *p = m_vec_ItemRange[i];
            const TItemTable *pTable = p->GetTable();
            if ((pTable->dwVnum < dwIndex) && dwIndex < (pTable->dwVnum + pTable->dwVnumRange))
            {
                m_pSelectedItemData = p;
                return TRUE;
            }
        }
        return FALSE;
    }

    m_pSelectedItemData = f->second.get();

    return TRUE;
}

CItemData *CItemManager::GetSelectedItemDataPointer()
{
    return m_pSelectedItemData;
}

bool CItemManager::GetItemDataPointer(uint32_t dwItemID, CItemData **ppItemData)
{
    if (0 == dwItemID)
        return FALSE;

    auto f = m_ItemMap.find(dwItemID);

    if (m_ItemMap.end() == f)
    {
        int n = m_vec_ItemRange.size();
        for (int i = 0; i < n; i++)
        {
            CItemData *p = m_vec_ItemRange[i];
            const TItemTable *pTable = p->GetTable();
            if ((pTable->dwVnum < dwItemID) && dwItemID < (pTable->dwVnum + pTable->dwVnumRange))
            {
                *ppItemData = p;
                return TRUE;
            }
        }
        return FALSE;
    }

    *ppItemData = f->second.get();

    return TRUE;
}

std::optional<std::vector<CItemData *>> CItemManager::GetProtoEntriesByName(const std::string &name, int32_t limit)
{
    if (name.empty())
        return std::nullopt;

    int32_t i = 0;
    std::vector<CItemData *> entries;
    for (const auto &entry : m_ItemMap)
    {
        if (strstr(entry.second.get()->GetName(), name.c_str()) != NULL)
        {
            entries.emplace_back(entry.second.get());
            if (i >= limit)
                break;
            ++i;
        }
    }

    return entries;
}

std::vector<CItemData *> CItemManager::GetRefinedFromItems(uint32_t vnum)
{
    std::vector<CItemData *> entries;
    for (const auto &entry : m_ItemMap)
    {
        if (entry.second.get()->GetRefinedVnum() == vnum)
        {
            entries.emplace_back(entry.second.get());

        }
    }
    return entries;
}

CItemData *CItemManager::GetProto(int dwItemID)
{
    if (0 == dwItemID)
        return FALSE;

    TItemMap::iterator f = m_ItemMap.find(dwItemID);

    if (m_ItemMap.end() == f)
    {
        int n = m_vec_ItemRange.size();
        for (int i = 0; i < n; i++)
        {
            CItemData *p = m_vec_ItemRange[i];
            const TItemTable *pTable = p->GetTable();
            if ((pTable->dwVnum < dwItemID) && dwItemID < (pTable->dwVnum + pTable->dwVnumRange))
            {
                return p;
            }
        }
        return nullptr;
    }

    return f->second.get();
}

CItemData *CItemManager::MakeItemData(uint32_t dwIndex)
{
    auto f = m_ItemMap.find(dwIndex);
    if (m_ItemMap.end() == f)
    {
        auto itemData = std::make_unique<CItemData>();

        auto rawPtr = itemData.get();
        m_ItemMap.insert(std::make_pair(dwIndex, std::move(itemData)));
        return rawPtr;
    }

    return f->second.get();
}

////////////////////////////////////////////////////////////////////////////////////////
// Load Item Scale
bool CItemManager::LoadItemScale(const char *szItemScale)
{
    auto fp = GetVfs().Open(szItemScale, kVfsOpenFullyBuffered);
    if (!fp)
        return false;

    const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
    fp->GetView(0, data, size);

    CMemoryTextFileLoader textFileLoader;
    textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(data.GetData()), size));

    CTokenVector TokenVector;
    for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
            continue;

        if ((TokenVector.size() < 6))
        {
            SPDLOG_ERROR(" CItemManager::LoadItemList({}) - LoadItemScale in {}\n", szItemScale, i);
            continue;
        }

        const std::string &c_rstrID = TokenVector[0];
        const std::string &c_rstrJob = TokenVector[1];
        const std::string &c_rstrSex = TokenVector[2];
        const std::string &c_rstrX = TokenVector[3];
        const std::string &c_rstrY = TokenVector[4];
        const std::string &c_rstrZ = TokenVector[5];

        std::string c_rstrPosX;
        std::string c_rstrPosY;
        std::string c_rstrPosZ;

        uint32_t dwItemVNum = atoi(c_rstrID.c_str());
        float xScale, yScale, zScale;
        storm::ParseNumber(c_rstrX, xScale);
        storm::ParseNumber(c_rstrY, yScale);
        storm::ParseNumber(c_rstrZ, zScale);

        xScale *= 0.01f;
        yScale *= 0.01f;
        zScale *= 0.01f;

        uint32_t dwJob{};
        if (c_rstrJob == "JOB_WARRIOR")
        {
            dwJob = NRaceData::JOB_WARRIOR;
        }

        if (c_rstrJob == "JOB_ASSASSIN")
        {
            dwJob = NRaceData::JOB_ASSASSIN;
        }

        if (c_rstrJob == "JOB_SURA")
        {
            dwJob = NRaceData::JOB_SURA;
        }

        if (c_rstrJob == "JOB_SHAMAN")
        {
            dwJob = NRaceData::JOB_SHAMAN;
        }

        if (c_rstrJob == "JOB_WOLFMAN")
        {
            dwJob = NRaceData::JOB_WOLFMAN;
        }

        uint32_t dwSex = c_rstrSex[0] == 'F' ? SEX_FEMALE : SEX_MALE;

        for (int j = 0; j <= 4; ++j) // 4
        {
            CItemData *pItemData = MakeItemData(dwItemVNum + j);
            pItemData->SetItemTableScaleData(dwJob, dwSex, xScale, yScale, zScale, 0.0f, 0.0f, 0.0f);
        }
    }
    return true;
}



bool CItemManager::LoadRareItems(const std::string &filename)
{
    auto fp = GetVfs().Open(filename);
    if (!fp)
        return false;

    const auto size = fp->GetSize();

    storm::View view(storm::GetDefaultAllocator());
    fp->GetView(0, view, size);

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    std::string_view data(reinterpret_cast<const char *>(view.GetData()), size);

    if (!reader.LoadString(data))
    {
        return false;
    }

    const auto rareItems = reader.GetList("RareItems");
    if (rareItems != nullptr)
    {
        for (uint32_t i = 0; i < rareItems->GetLineCount(); ++i)
        {
            try
            {
                uint32_t vnum = 0;
                if (!storm::ParseNumber(rareItems->GetLine(i), vnum))
                {
                    SPDLOG_ERROR("Could not pare rare item on line {}", i);
                }
                m_rareItems.push_back(vnum);
            }
            catch (const std::invalid_argument &ia)
            {
                SPDLOG_ERROR("Rare item list '{}' line {} has invalid token {}", filename, i, rareItems->GetLine(i));
                return false;
            }
            catch (const std::out_of_range &oor)
            {
                SPDLOG_ERROR("Rare item list '{}' line {} has out of range token {}", filename, i,
                             rareItems->GetLine(i));
                return false;
            }
        }
    }

    return true;
}

bool CItemManager::IsRareItem(uint32_t dwVirtualId)
{
    const auto it = std::find(m_rareItems.begin(), m_rareItems.end(), dwVirtualId);
    if (it == m_rareItems.end())
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////
// Load Item Table

bool CItemManager::LoadItemList(const char *c_szFileName)
{
    auto fp = GetVfs().Open(c_szFileName);
    if (!fp)
        return false;

    const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
    fp->GetView(0, data, size);

    CMemoryTextFileLoader textFileLoader;
    textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(data.GetData()), size));

    CTokenVector TokenVector;
    for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
            continue;

        if (!(TokenVector.size() == 3 || TokenVector.size() == 4))
        {
            SPDLOG_ERROR(" CItemManager::LoadItemList({}) - StrangeLine in {}\n", c_szFileName, i);
            continue;
        }

        const std::string &c_rstrID = TokenVector[0];
        // const std::string & c_rstrType = TokenVector[1];
        const std::string &c_rstrIcon = TokenVector[2];

        uint32_t dwItemVNum = atoi(c_rstrID.c_str());

        auto pItemData = MakeItemData(dwItemVNum);

        if (4 == TokenVector.size())
        {
            const auto &c_rstrModelFileName = TokenVector[3];
            pItemData->SetDefaultItemData(c_rstrIcon, c_rstrModelFileName);
        }
        else
        {
            pItemData->SetDefaultItemData(c_rstrIcon);
        }
    }

    return true;
}

bool CItemManager::LoadDragonSoulTable(const char *filename)
{
    auto text = LoadFileToString(GetVfs(), filename);
    if (!text)
    {
        SPDLOG_WARN("Could not load {}", filename);
        return false;
    }
    return m_dsTable.ReadDragonSoulTableMemory(text.value());
}

const std::string &__SnapString(const std::string &c_rstSrc, std::string &rstTemp)
{
    UINT uSrcLen = c_rstSrc.length();
    if (uSrcLen < 2)
        return c_rstSrc;

    if (c_rstSrc[0] != '"')
        return c_rstSrc;

    UINT uLeftCut = 1;

    UINT uRightCut = uSrcLen;
    if (c_rstSrc[uSrcLen - 1] == '"')
        uRightCut = uSrcLen - 1;

    rstTemp = c_rstSrc.substr(uLeftCut, uRightCut - uLeftCut);
    return rstTemp;
}

bool CItemManager::LoadItemAttrProtos()
{
    std::vector<ItemAttrProto> va;
    if (!LoadClientDb<ItemAttrProto>(GetVfs(), "data/item_attr_proto", va))
        return false;

    m_itemAttr.clear();

    for (const auto &item : va)
        m_itemAttr.emplace(item.apply, item);

    va.clear();

    if (!LoadClientDb<ItemAttrProto>(GetVfs(), "data/item_attr_rare_proto", va))
        return false;

    m_itemAttrRare.clear();

    for (const auto &item : va)
        m_itemAttrRare.emplace(item.apply, item);

    return true;
}

const ItemAttrProto &CItemManager::GetRareItemAttr(uint32_t apply)
{
    return m_itemAttrRare[apply];
}

const ItemAttrProto &CItemManager::GetItemAttr(uint32_t apply)
{
    return m_itemAttr[apply];
}

bool CItemManager::LoadItemDesc(const char *c_szFileName)
{
    auto fp = GetVfs().Open(c_szFileName);
    if (!fp)
        return false;

    const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
    fp->GetView(0, data, size);

    CMemoryTextFileLoader textFileLoader;
    textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(data.GetData()), size));

    CTokenVector args;
    for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLineByTab(i, &args))
            continue;

        if (args.size() != 2 && args.size() != 3 && args.size() != 4)
        {
            SPDLOG_ERROR("{}:{}: expected 2/3/4 tokens, got {}", c_szFileName, i, args.size());
            continue;
        }

        try
        {
            uint32_t vnum = std::stoul(args[0]);
            const auto it = m_ItemMap.find(vnum);
            if (m_ItemMap.end() == it)
            {
                SPDLOG_WARN("{}:{}: vnum {} doesn't exist", c_szFileName, i, vnum);
                continue;
            }

            it->second->SetName(args[1]);

            if (args.size() > 2)
                it->second->SetDescription(args[2]);

            if (args.size() > 3)
                it->second->SetSummary(args[3]);
        }
        catch (const std::invalid_argument &ia)
        {
            SPDLOG_ERROR("{}:{}: has invalid token {}", c_szFileName, i, args[0]);
            return false;
        }
        catch (const std::out_of_range &oor)
        {
            SPDLOG_ERROR("{}:{}: has out of range token {}", c_szFileName, i, args[0]);
            return false;
        }
    }

    return true;
}

uint32_t GetHashCode(const char *pString)
{
    uint32_t i, len;
    uint32_t ch;
    uint32_t result;

    len = strlen(pString);
    result = 5381;
    for (i = 0; i < len; i++)
    {
        ch = (uint32_t)pString[i];
        result = ((result << 5) + result) + ch; // hash * 33 + ch
    }

    return result;
}

void CItemManager::LoadProtoEntry(std::map<uint32_t, uint32_t> itemNameMap, TItemTable table)
{
    uint32_t dwVnum = table.dwVnum;
    CItemData *pItemData;

    auto f = m_ItemMap.find(dwVnum);
    if (m_ItemMap.end() == f)
    {
        auto szName = fmt::sprintf("icon/item/%05d.tga", dwVnum);

        if (!GetVfs().Exists(szName))
        {
            const auto itVnum = itemNameMap.find(GetHashCode(table.szName));

            if (itVnum != itemNameMap.end())
                szName = fmt::sprintf("icon/item/%05d.tga", itVnum->second);
            else
                szName = fmt::sprintf("icon/item/%05d.tga", dwVnum - dwVnum % 10);

            if (!GetVfs().Exists(szName))
            {
                szName = "icon/item/noexist.tga";
            }
        }
        else
        {
            szName = "icon/item/noexist.png";
        }

        pItemData = MakeItemData(dwVnum);
        pItemData->SetDefaultItemData(szName);
    }
    else
    {
        pItemData = f->second.get();
    }

    if (itemNameMap.find(GetHashCode(table.szName)) == itemNameMap.end())
        itemNameMap.emplace(GetHashCode(table.szName), table.dwVnum);

    pItemData->SetItemTableData(table);
    if (0 != table.dwVnumRange)
    {
        m_vec_ItemRange.push_back(pItemData);
    }
}

bool CItemManager::LoadItemTable(const char *c_szFileName)
{
    std::vector<TItemTable> v;
    if (!LoadClientDb(GetVfs(), c_szFileName, v))
        return false;

    std::map<uint32_t, uint32_t> itemNameMap;

    for (const auto &item : v)
    {
        LoadProtoEntry(itemNameMap, item);
    }

    return true;
}

const std::vector<LevelPetDisplayedFood>& CItemManager::GetLevelPetDisplayFoods() const {
    return m_levelPetDisplayedFoods;
}

const std::vector<std::tuple<uint32_t, uint32_t>>& CItemManager::GetLevelPetMobs() const {
    return m_levelPetMobs;
}

bool CItemManager::ReadLevelPetData(const char *c_pszFileName)
{
    auto fp = GetVfs().Open(c_pszFileName);
    if (!fp)
        return false;

    const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
    fp->GetView(0, data, size);

    auto file = std::string(reinterpret_cast<const char*>(data.GetData()), size);

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(file))
    {
        SPDLOG_ERROR("Failed to load level pet data file {0}",
                     c_pszFileName);
        return false;
    }


    std::vector<storm::StringRef> tokens;
    int32_t lineNum = 1;
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

    const auto* bonusPerStatPointList = reader.GetList("BonusPerStatPoint");
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
            m_levelPetMobs.emplace_back(std::make_tuple(vnum, mobVnum));
            ++lineNum;
        }
    }

    const auto* levelToDisplayedFood = reader.GetList("LevelToDisplayedFood");
    if (!levelToDisplayedFood)
    {
        SPDLOG_ERROR("Missing NeedCount List in {}", c_pszFileName);
        return false;
    }

    tokens.clear();
    lineNum = 1;

    LevelPetDisplayedFood lpdf = {};

    for (const auto &line : levelToDisplayedFood->GetLines())
    {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() != 2)
        {
            SPDLOG_ERROR("{0}:{1}: LevelToDisplayedFood group line has only {2} tokens but needs 2", c_pszFileName,
                         lineNum, tokens.size());
            continue;
        }

        std::vector<std::string> v;
        storm::Tokenize(tokens[0], "-", v);
        if (v.size() != 2)
        {
            SPDLOG_ERROR("{0}:{1}: LevelToDisplayedFood group token 1 has only {2} tokens but needs 2", c_pszFileName,
                         lineNum, v.size());
            continue;
        }

        if (!storm::ParseNumber(v[0], lpdf.min))
        {
            SPDLOG_ERROR("{0}:{1}: Invalid min value", c_pszFileName, lineNum);
            continue;
        }

        if (!storm::ParseNumber(v[1], lpdf.max))
        {
            SPDLOG_ERROR("{0}:{1}: Invalid max value", c_pszFileName, lineNum);
            continue;
        }

        if (!storm::ParseNumber(tokens[1], lpdf.vnum))
        {
            SPDLOG_ERROR("{0}:{1}: Could not parse mob vnum", c_pszFileName, lineNum);
            continue;
        }

        m_levelPetDisplayedFoods.push_back(lpdf);
    }



    return true;
}

#ifdef ENABLE_SHINING_SYSTEM
bool CItemManager::LoadShiningTable(const char *szShiningTable)
{
    auto fp = GetVfs().Open(szShiningTable);
    if (!fp)
        return false;

    const auto size = fp->GetSize();

    storm::View view(storm::GetDefaultAllocator());
    fp->GetView(0, view, size);

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    std::string_view data(reinterpret_cast<const char *>(view.GetData()), size);

    if (!reader.LoadString(data))
    {
        return false;
    }

    for (const auto &p : reader.GetChildren())
    {
        auto node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        auto grp = static_cast<GroupTextGroup *>(node);

        const auto strType = grp->GetProperty("Type");
        if (strType.empty())
        {
            SPDLOG_ERROR("LoadShiningTable : Syntax error {0} : no type (armor|weapon), node {1}", szShiningTable,
                         p.first);
            return false;
        }

        std::string vnumString;
        if (!GetGroupProperty(grp, "Vnum", vnumString))
        {
            SPDLOG_ERROR("LoadShiningTable : Syntax error {0} : no vnum, node {1}", szShiningTable, p.first);
            return false;
        }

        uint8_t persistent;
        if (!GetGroupProperty(grp, "Persistent", persistent))
        {
            SPDLOG_ERROR("LoadShiningTable : Syntax error {0} : no persistent, node {1}", szShiningTable, p.first);
            return false;
        }

        CItemData::TItemShiningTable tab;
        tab.persistent = persistent;

        if (strType == "weapon")
        {
            GetGroupProperty(grp, "GenericEffect", tab.effects[WEAPON_NONE]);
            GetGroupProperty(grp, "Sword", tab.effects[WEAPON_SWORD]);
            GetGroupProperty(grp, "Dagger", tab.effects[WEAPON_DAGGER]);
            GetGroupProperty(grp, "Bow", tab.effects[WEAPON_BOW]);
            GetGroupProperty(grp, "TwoHand", tab.effects[WEAPON_TWO_HANDED]);
            GetGroupProperty(grp, "Bell", tab.effects[WEAPON_BELL]);
            GetGroupProperty(grp, "Fan", tab.effects[WEAPON_FAN]);
            GetGroupProperty(grp, "Claw", tab.effects[WEAPON_CLAW]);
        }
        else
        {
            if (!GetGroupProperty(grp, "GenericEffect", tab.effects[WEAPON_NONE]))
            {
                SPDLOG_ERROR("Did not find GenericEffect in node {0}", p.first);
            }
            GetGroupProperty(grp, "BoneName", tab.boneName);
            GetGroupProperty(grp, "Scale", tab.scale);
        }

        const auto pos = vnumString.find('~');
        if (std::string::npos == pos)
        {
            auto pItemData = MakeItemData(std::stoi(vnumString));
            if (pItemData)
                pItemData->SetItemShiningTableData(tab);
        }
        else
        {
            auto startVnumStr(vnumString.substr(0, pos));
            auto endVnumStr(vnumString.substr(pos + 1));

            const uint32_t startVnum = atoi(startVnumStr.c_str());
            const uint32_t endVnum = atoi(endVnumStr.c_str());

            auto vnum = startVnum;

            while (vnum <= endVnum)
            {
                auto pItemData = MakeItemData(vnum);
                if (pItemData)
                    pItemData->SetItemShiningTableData(tab);

                ++vnum;
            }
        }
    }

    return true;
}

#endif

#ifdef INGAME_WIKI
bool CItemManager::IsFilteredAntiflag(CItemData *itemData, DWORD raceFilter)
{
    if (raceFilter != 0)
    {
        if (!itemData->IsAntiFlag(ITEM_ANTIFLAG_SHAMAN) && raceFilter & ITEM_ANTIFLAG_SHAMAN)
            return false;

        if (!itemData->IsAntiFlag(ITEM_ANTIFLAG_SURA) && raceFilter & ITEM_ANTIFLAG_SURA)
            return false;

        if (!itemData->IsAntiFlag(ITEM_ANTIFLAG_ASSASSIN) && raceFilter & ITEM_ANTIFLAG_ASSASSIN)
            return false;

        if (!itemData->IsAntiFlag(ITEM_ANTIFLAG_WARRIOR) && raceFilter & ITEM_ANTIFLAG_WARRIOR)
            return false;
    }

    return true;
}

size_t CItemManager::WikiLoadClassItems(BYTE classType, DWORD raceFilter)
{
    m_tempItemVec.clear();

    for (TItemMap::iterator it = m_ItemMap.begin(); it != m_ItemMap.end(); ++it)
    {
        if (!it->second->IsValidImage() || it->first < 10 || !it->second->IsWhitelisted())
            // || it->second->GetIconFileName().find("D:\\ymir work\\ui\\game\\quest\\questicon\\level_") !=
            // std::string::npos)
            continue;

        switch (classType)
        {
        case 0: // weapon
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_WEAPON &&
                !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 1: // body
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_ARMOR && it->second->GetSubType() == ARMOR_BODY &&
                !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 2:
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_ARMOR && it->second->GetSubType() == ARMOR_EAR &&
                !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 3:
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_ARMOR && it->second->GetSubType() == ARMOR_FOOTS &&
                !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 4:
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_ARMOR && it->second->GetSubType() == ARMOR_HEAD &&
                !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 5:
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_ARMOR && it->second->GetSubType() == ARMOR_NECK &&
                !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 6:
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_ARMOR &&
                it->second->GetSubType() == ARMOR_SHIELD && !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 7:
            if (it->first % 10 == 0 && it->second->GetType() == ITEM_ARMOR && it->second->GetSubType() == ARMOR_WRIST &&
                !IsFilteredAntiflag(it->second.get(), raceFilter))
                m_tempItemVec.push_back(it->first);
            break;
        case 8: // chests
            if (it->second->GetType() == ITEM_GIFTBOX)
                m_tempItemVec.push_back(it->first);
            break;
        case 9: // belts
            if (it->second->GetType() == ITEM_BELT)
                m_tempItemVec.push_back(it->first);
            break;
        case 10: // talisman
            if (it->second->GetType() == ITEM_TALISMAN)
                m_tempItemVec.push_back(it->first);
            break;
        case 11: // rings
            if (it->second->GetType() == ITEM_RING)
                m_tempItemVec.push_back(it->first);
            break;
        case 12: // elexier
            if (it->second->GetType() == ITEM_TOGGLE && it->second->GetSubType() == TOGGLE_AFFECT)
                m_tempItemVec.push_back(it->first);
            break;
        }
    }

    return m_tempItemVec.size();
}

DWORD CItemManager::WikiSearchItem(std::string subStr)
{
    m_tempItemVec.clear();

    for (auto i = m_ItemMap.begin(); i != m_ItemMap.end(); i++)
    {
        const TItemTable *tbl = i->second->GetTable();
        if (!i->second->IsValidImage() || i->first < 10 || tbl->dwVnum < 10 ||
            !GetVfs().Exists(i->second->GetIconFileName().c_str()))
            continue;

        char tempName[25];
        memcpy(tempName, tbl->szLocaleName, sizeof(tempName));
        for (int j = 0; j < sizeof(tempName); j++)
            tempName[j] = tolower(tempName[j]);

        std::string tempString = tempName;
        if (tempString.find(subStr) != std::string::npos)
            m_tempItemVec.push_back(i->first);
    }

    return m_tempItemVec.size();
}
#endif

void CItemManager::RegisterHyperlinkItem(uint32_t id, const ClientItemData& data) {
    m_hyperlinkItems.insert_or_assign(id, data);
}

std::optional<ClientItemData> CItemManager::GetHyperlinkItemData(uint32_t id) {
    if(m_hyperlinkItems.find(id) == m_hyperlinkItems.end())
        return std::nullopt;

    return m_hyperlinkItems[id];
}

void CItemManager::Destroy()
{
    m_ItemMap.clear();
}

CItemManager::CItemManager() : m_pSelectedItemData(NULL) {}

CItemManager::~CItemManager()
{
    Destroy();
}
