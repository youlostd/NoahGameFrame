#ifndef METIN2_CLIENT_GAMELIB_WIKIMANAGER_HPP
#define METIN2_CLIENT_GAMELIB_WIKIMANAGER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Types.hpp>
using OriginMap = std::unordered_map<GenericVnum, std::string>;

class WikiManager : public CSingleton<WikiManager>
{



  public:
    
    WikiManager();
    virtual ~WikiManager();
        bool LoadWikiConfig(const std::string &filename);

    [[nodiscard]] const std::vector<GenericVnum> &GetItemWhitelist() const { return m_itemWhitelist; }
    [[nodiscard]] const std::vector<GenericVnum> &GetMobWhitelist() const { return m_mobWhitelist; }
    [[nodiscard]] const OriginMap &GetItemOriginMap() const { return m_itemOriginMap; }
    [[nodiscard]] const OriginMap &GetMobOriginMap() const { return m_mobOriginMap; }
    [[nodiscard]] const std::vector<GenericVnum> &GetBossChests() const { return m_bossChests; }
    [[nodiscard]] const std::vector<GenericVnum> &GetEventChests() const { return m_eventChests; }
    [[nodiscard]] const std::vector<GenericVnum> &GetAltChests() const { return m_altChests; }
    [[nodiscard]] const std::vector<GenericVnum> &GetCostumeWeapons() const { return m_costumeWeapons; }
    [[nodiscard]] const std::vector<GenericVnum> &GetCostumeArmors() const { return m_costumeArmors; }
    [[nodiscard]] const std::vector<GenericVnum> &GetCostumeHairs() const { return m_costumeHairs; }
    [[nodiscard]] const std::vector<GenericVnum> &GetCostumeWings() const { return m_costumeWings; }
    [[nodiscard]] const std::vector<GenericVnum> &GetCostumeShinings() const { return m_costumeShinings; }

  private:
    std::vector<GenericVnum> m_itemWhitelist;
    std::vector<GenericVnum> m_mobWhitelist;
    std::unordered_map<GenericVnum, std::string> m_itemOriginMap;
    std::unordered_map<GenericVnum, std::string> m_mobOriginMap;
    std::vector<GenericVnum> m_bossChests;
    std::vector<GenericVnum> m_eventChests;
    std::vector<GenericVnum> m_altChests;
    std::vector<GenericVnum> m_costumeWeapons;
    std::vector<GenericVnum> m_costumeArmors;
    std::vector<GenericVnum> m_costumeHairs;
    std::vector<GenericVnum> m_costumeWings;
    std::vector<GenericVnum> m_costumeShinings;
};

#endif /* METIN2_CLIENT_GAMELIB_WIKIMANAGER_HPP */
