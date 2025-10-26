#include "WikiManager.hpp"

#include <base/GroupTextTree.hpp>

#include "pak/Util.hpp"

WikiManager::WikiManager() {}

WikiManager::~WikiManager() {}

bool WikiManager::LoadWikiConfig(const std::string &filename)
{
    auto data = LoadFileToString(GetVfs(), filename.c_str());
    if (!data)
        return false;

    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadString(data.value()))
    {
        return false;
    }

    const auto *itemWhitelist = reader.GetList("ItemWhitelist");
    if (itemWhitelist != nullptr)
    {
        for (uint32_t i = 0; i < itemWhitelist->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(itemWhitelist->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_itemWhitelist.push_back(vnum);
        }
    }

    const auto *mobWhiteList = reader.GetList("MobWhitelist");
    if (mobWhiteList != nullptr)
    {
        for (uint32_t i = 0; i < mobWhiteList->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(mobWhiteList->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_mobWhitelist.push_back(vnum);
        }
    }

    const auto *bossChestVnums = reader.GetList("BossChestVnums");
    if (bossChestVnums != nullptr)
    {
        for (uint32_t i = 0; i < bossChestVnums->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(bossChestVnums->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_bossChests.push_back(vnum);
        }
    }

    const auto *eventChestVnums = reader.GetList("EventChestVnums");
    if (eventChestVnums != nullptr)
    {
        for (uint32_t i = 0; i < eventChestVnums->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(eventChestVnums->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_eventChests.push_back(vnum);
        }
    }

    const auto *altChestVnums = reader.GetList("AltChestVnums");
    if (altChestVnums != nullptr)
    {
        for (uint32_t i = 0; i < altChestVnums->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(altChestVnums->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_altChests.push_back(vnum);
        }
    }

    const auto *costumeWeaponVnum = reader.GetList("CostumeWeaponVnum");
    if (costumeWeaponVnum != nullptr)
    {
        for (uint32_t i = 0; i < costumeWeaponVnum->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(costumeWeaponVnum->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_costumeWeapons.push_back(vnum);
        }
    }

    const auto *costumeArmorVnum = reader.GetList("CostumeArmorVnum");
    if (costumeArmorVnum != nullptr)
    {
        for (uint32_t i = 0; i < costumeArmorVnum->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(costumeArmorVnum->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_costumeArmors.push_back(vnum);
        }
    }

    const auto *costumeHairVnum = reader.GetList("CostumeHairVnum");
    if (costumeHairVnum != nullptr)
    {
        for (uint32_t i = 0; i < costumeHairVnum->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(costumeHairVnum->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_costumeHairs.push_back(vnum);
        }
    }

    const auto *costumeWingVnum = reader.GetList("CostumeWingVnum");
    if (costumeWingVnum != nullptr)
    {
        for (uint32_t i = 0; i < costumeWingVnum->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(costumeWingVnum->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_costumeWings.push_back(vnum);
        }
    }

    const auto *costumeShiningVnum = reader.GetList("CostumeShiningVnum");
    if (costumeShiningVnum != nullptr)
    {
        for (uint32_t i = 0; i < costumeShiningVnum->GetLineCount(); ++i)
        {
            uint32_t vnum = 0;
            if (!storm::ParseNumber(costumeShiningVnum->GetLine(i), vnum))
            {
                SPDLOG_ERROR("Could not pare rare item on line {}", i);
                vnum = 0;
            }
            m_costumeShinings.push_back(vnum);
        }
    }

    const auto *itemOriginMap = reader.GetList("ItemOriginMap");
    if (!itemOriginMap)
    {
        SPDLOG_ERROR("Missing ItemOriginMap List in '{}'", filename);
        return false;
    }

    std::vector<storm::StringRef> tokens;
    for (const auto &line : itemOriginMap->GetLines())
    {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() != 2)
        {
            SPDLOG_ERROR("{0}: ItemOriginMap group line has only {1} tokens but needs 2", filename, tokens.size());
            continue;
        }

        uint32_t vnum = 0;
        if (!storm::ParseNumber(tokens[0], vnum))
        {
            vnum = 0;
        }

        m_itemOriginMap.emplace(vnum, tokens[1]);

    }

    const auto *mobOriginMap = reader.GetList("MobOriginMap");
    if (!mobOriginMap)
    {
        SPDLOG_ERROR("Missing MobOriginMap List in '{}'", filename);
        return false;
    }

    tokens.clear();
    for (const auto &line : mobOriginMap->GetLines())
    {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() != 2)
        {
            SPDLOG_ERROR("{0}: mobOriginMap group line has only {1} tokens but needs 2", filename, tokens.size());
            continue;
        }

        uint32_t vnum = 0;
        if (!storm::ParseNumber(tokens[0], vnum))
        {
            vnum = 0;
        }

        m_mobOriginMap.emplace(vnum, tokens[1]);

    }

    return true;
}
