#ifndef METIN2_SERVER_GAME_CONFIG_H
#define METIN2_SERVER_GAME_CONFIG_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <algorithm>

enum
{
    ADDRESS_MAX_LEN = 15
};

struct DbConfig
{
    std::string addr;
    uint16_t port{};
    std::string user;
    std::string password;
    std::string name;
};

struct LevelLimit
{
    uint32_t min;
    uint32_t max;
};

struct Config
{
    uint8_t channel = 1;

    int32_t passesPerSecond = 25;
    uint32_t pingTimeout = 2 * 60; // s
    uint32_t saveTimeout = 60;     // s
    int32_t logLevel = spdlog::level::warn;

    uint8_t markMinLevel = 3;
    bool testServer = false;
    bool damageDebug = false;
    bool authServer = false;
    bool guildMarkServer = false;
    bool worldBossMaster = false;
    bool noMoreUser = false;
    bool disbaleEmotionMask = true;
    bool disablePvP = false;
    bool disableRegen = false;
    bool disableSkills = false;
    bool disableWandering = false;
    bool disableCharCreation = false;
    bool disableRareAttr = false;
    bool disableRandomMasterSkills = true;
    bool disableSkillBookStepLeveling = true;
    bool enableEmotionWithoutMask = true;
    bool enableSameGenderEmotions = true;
    bool enableAlwaysBook = true;
    bool enableSwitchbot = true;
    bool ccuServer = false;

    uint8_t shopTaxes = 3;
    uint8_t dragonSoulMinLevel = 100;
    int32_t maxLevelStats = 150;
    uint32_t pkProtectLevel = 15;
    uint32_t maxLevel = 170;
    uint32_t minOfflineMessageLevel = 105;
    int64_t maxGold = 1000000000000000ULL;
    uint32_t portalLimitTime = 1;
    uint32_t maxStatPoint = 270;
    uint32_t viewRange = 5000;
    uint32_t userLimit = 0;
    uint32_t busyUserCount = 650;
    uint32_t fullUserCount = 1200;
    uint32_t speedHackLimitCount = 50;
    uint32_t speedHackLimitBonus = 80;
    uint32_t syncHackLimitCount = 10;
    uint32_t syncHackLimitTime = 100;
    uint32_t shoutLevelLimit = 10;
    uint32_t maxStat = 150;
    uint32_t skillBookDelay = 3600;
    uint32_t itemOwnershipTime = 30;
    uint32_t itemOwnershipTimeLong = 300;
    uint32_t itemGroundTime = 30;
    uint32_t itemGroundTimeLong = 300;

    uint32_t pcMaxMoveSpeed = 250;
    uint32_t pcMaxHitRange = 300;
    uint32_t pcMaxAttackSpeed = 170;
    uint32_t mobMaxMoveSpeed = 1000;
    uint32_t mobMaxAttackSpeed = 1000;
    uint32_t gmMaxMoveSpeed = 400;

    uint8_t buffBotIq = 174;
    uint8_t buffBotSkillLevel = 50;

    std::string gamePort = "5000";

    std::string dbAddr = "127.0.0.1";
    std::string dbPort = "15000";

    std::string gameBindIp = "127.0.0.1";
    std::string gameIp = "127.0.0.1";

    std::string masterBindIp = "127.0.0.1";
    std::string masterIp = "127.0.0.1";

    std::string masterPort = "10000";

    std::string sqlAddr = "127.0.0.1";
    std::string hostname = "Game";
    std::string version = "1512176040";
    std::string ishopSas = "!#b9nGZp";
    std::string ishopUrl = "shop.hp";
    std::string hguardUrl = "hguard.hp";

    uint32_t serverId = 1;

    DbConfig accountDb;
    DbConfig playerDb;
    DbConfig commonDb;
    DbConfig logDb;

    std::vector<uint32_t> blockedHorseSkillMaps;
    std::vector<uint32_t> blockedMountMaps;
    std::vector<uint32_t> blockedPetMaps;
    std::vector<uint32_t> blockedLevelPetMaps;

    std::vector<uint32_t> hostedMaps;
    std::vector<uint32_t> pvpDisabledMaps;
    std::vector<uint32_t> instantRestartMaps;
    std::vector<uint32_t> allowSashMaps;
    std::vector<uint32_t> disallowSashMaps;
    std::vector<uint32_t> blockSkillColorMaps;
    std::vector<uint32_t> restartWithAffectsMaps;
    std::vector<uint32_t> duelOnlyMaps;
    std::vector<uint32_t> hackCheckedMaps;
    std::vector<uint32_t> sureRemoveGoodDisabledMaps;
    std::vector<uint32_t> disableVoteBuffMaps;



    std::vector<std::string> adminpageAddrs;
    std::vector<std::string> localeNames;
    std::unordered_map<uint32_t, LevelLimit> mapLevelLimit;

    std::string adminpagePassword;

    bool IsBlockedHorseSkillMap(uint32_t index)
    {
        return std::find(blockedHorseSkillMaps.begin(), blockedHorseSkillMaps.end(), index) !=
               blockedHorseSkillMaps.end();
    }

    bool IsBlockedMountMap(uint32_t index)
    {
        return std::find(blockedMountMaps.begin(), blockedMountMaps.end(), index) != blockedMountMaps.end();
    }

    bool IsBlockedPetMap(uint32_t index)
    {
        return std::find(blockedPetMaps.begin(), blockedPetMaps.end(), index) != blockedPetMaps.end();
    }

    bool IsBlockedLevelPetMap(uint32_t index)
    {
        return std::find(blockedLevelPetMaps.begin(), blockedLevelPetMaps.end(), index) != blockedLevelPetMaps.end();
    }

    bool IsHostingMap(uint32_t index)
    {
        return std::find(hostedMaps.begin(), hostedMaps.end(), index) != hostedMaps.end();
    }

    bool IsPvPDisabledMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(pvpDisabledMaps.begin(), pvpDisabledMaps.end(), index) != pvpDisabledMaps.end();
    }

    bool IsAllowSashMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(allowSashMaps.begin(), allowSashMaps.end(), index) != allowSashMaps.end();
    }

    bool IsDisallowSashMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(disallowSashMaps.begin(), disallowSashMaps.end(), index) != disallowSashMaps.end();
    }

    bool IsBlockedSkillColorMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(blockSkillColorMaps.begin(), blockSkillColorMaps.end(), index) != blockSkillColorMaps.end();
    }

    bool IsRestartWithAffectsMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(restartWithAffectsMaps.begin(), restartWithAffectsMaps.end(), index) != restartWithAffectsMaps.end();
    }

    bool IsSuraRemoveGoodDisabledMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(sureRemoveGoodDisabledMaps.begin(), sureRemoveGoodDisabledMaps.end(), index) != sureRemoveGoodDisabledMaps.end();
    }

    bool IsDuelOnlyMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(duelOnlyMaps.begin(), duelOnlyMaps.end(), index) != duelOnlyMaps.end();
    }

    bool IsHackCheckedMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(hackCheckedMaps.begin(), hackCheckedMaps.end(), index) != hackCheckedMaps.end();
    }
    bool IsVoteBuffDisableMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(disableVoteBuffMaps.begin(), disableVoteBuffMaps.end(), index) != disableVoteBuffMaps.end();
    }

    bool HasDisallowSashMaps() const { return !disallowSashMaps.empty(); }

    bool HasAllowSashMaps() const { return !allowSashMaps.empty(); }

    bool IsInstantRestartMap(uint32_t index)
    {
        if (index > 10000)
            index /= 10000;

        return std::find(instantRestartMaps.begin(), instantRestartMaps.end(), index) != instantRestartMaps.end();
    }
};

extern Config gConfig;
extern bool g_bNoMoreClient;
void map_allow_copy(int32_t *pl, int size);

bool LoadConfiguration();

bool IsEmptyAdminPage();
bool IsAdminPage(const char *ip);
void ClearAdminPages();

#endif /* METIN2_SERVER_GAME_CONFIG_H */
