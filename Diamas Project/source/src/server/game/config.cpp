#include <sstream>
#ifndef _WIN32
#include <ifaddrs.h>
#endif
#include <algorithm>

#include "utils.h"
#include "log.h"
#include "desc_manager.h"
#include "char.h"
#include "config.h"
#include "db.h"
#include <boost/algorithm/clamp.hpp>
#include <boost/algorithm/string.hpp>
#include "SITokenParser.h"
#include <storm/StringUtil.hpp>
#include <toml.hpp>

Config gConfig;

static void ParseIpList(const std::string &str,
                        std::string &bindIp,
                        std::string &ip)
{
    std::vector<storm::StringRef> args;
    storm::ExtractArguments(str, args);

    bindIp.assign(args[0].data(), args[0].length());

    if (args.size() > 1)
        ip.assign(args[1].data(), args[1].length());
    else
        ip = bindIp;
}

bool LoadMainConfig(const std::string &configName)
{
    const auto data = toml::parse(configName);

    try
    {
        if (data.contains("channel"))
            gConfig.channel = data.at("channel").as_integer();

        if (data.contains("log-level"))
            gConfig.logLevel = toml::get<toml::integer>(data.at("log-level"));

        if (data.contains("passes-per-second"))
            gConfig.passesPerSecond = toml::get<toml::integer>(data.at("passes-per-second"));

        if (data.contains("ping-timeout"))
            gConfig.pingTimeout = toml::get<toml::integer>(data.at("ping-timeout"));

        if (data.contains("save-timeout"))
            gConfig.saveTimeout = toml::get<toml::integer>(data.at("save-timeout"));

        if (data.contains("mark-min-level"))
            gConfig.markMinLevel = toml::get<toml::integer>(data.at("mark-min-level"));

        if (data.contains("test-server"))
            gConfig.testServer = toml::get<toml::boolean>(data.at("test-server"));

        if (data.contains("ccu-server"))
            gConfig.ccuServer = toml::get<toml::boolean>(data.at("ccu-server"));

        if (data.contains("world-boss-master"))
            gConfig.worldBossMaster = toml::get<toml::boolean>(data.at("world-boss-master"));

        if (data.contains("auth-server"))
            gConfig.authServer = toml::get<toml::boolean>(data.at("auth-server"));

        if (data.contains("mark-server"))
            gConfig.guildMarkServer = toml::get<toml::boolean>(data.at("mark-server"));

        if (data.contains("no-more-user"))
            gConfig.noMoreUser = toml::get<toml::boolean>(data.at("no-more-user"));

        if (data.contains("disable-emotion-mask"))
            gConfig.disbaleEmotionMask = toml::get<toml::boolean>(data.at("disable-emotion-mask"));

        if (data.contains("disable-pvp"))
            gConfig.disablePvP = toml::get<toml::boolean>(data.at("disable-pvp"));

        if (data.contains("disable-regen"))
            gConfig.disableRegen = toml::get<toml::boolean>(data.at("disable-regen"));

        if (data.contains("disable-skills"))
            gConfig.disableSkills = toml::get<toml::boolean>(data.at("disable-skills"));

        if (data.contains("disable-wandering"))
            gConfig.disableWandering = toml::get<toml::boolean>(data.at("disable-wandering"));

        if (data.contains("disable-char-creation"))
            gConfig.disableCharCreation = toml::get<toml::boolean>(data.at("disable-char-creation"));

        if (data.contains("disable-rare-attr"))
            gConfig.disableRareAttr = toml::get<toml::boolean>(data.at("disable-rare-attr"));

        if (data.contains("disable-random-master-skills"))
            gConfig.disableRandomMasterSkills = toml::get<toml::boolean>(data.at("disable-random-master-skills"));

        if (data.contains("disable-skillbook-steps"))
            gConfig.disableSkillBookStepLeveling = toml::get<toml::boolean>(data.at("disable-skillbook-steps"));

        if (data.contains("enable-gay-emotions"))
            gConfig.enableSameGenderEmotions = toml::get<toml::boolean>(data.at("enable-gay-emotions"));

        if (data.contains("enable-always-book-success"))
            gConfig.enableAlwaysBook = toml::get<toml::boolean>(data.at("enable-always-book-success"));

        if (data.contains("enable-switchbot"))
            gConfig.enableSwitchbot = toml::get<toml::boolean>(data.at("enable-switchbot"));

        if (data.contains("shop-tax"))
            gConfig.shopTaxes = toml::get<toml::integer>(data.at("shop-tax"));

        if (data.contains("max-level-stats"))
            gConfig.maxLevelStats = toml::get<toml::integer>(data.at("max-level-stats"));

        if (data.contains("pk-protect-level"))
            gConfig.pkProtectLevel = toml::get<toml::integer>(data.at("pk-protect-level"));

        if (data.contains("dragon-soul-min-level"))
            gConfig.dragonSoulMinLevel = toml::get<toml::integer>(data.at("dragon-soul-min-level"));

        if (data.contains("max-level"))
            gConfig.maxLevel = toml::get<toml::integer>(data.at("max-level"));

        if (data.contains("min-offline-message-level"))
            gConfig.minOfflineMessageLevel = toml::get<toml::integer>(data.at("min-offline-message-level"));

        if (data.contains("max-gold"))
            gConfig.maxGold = toml::get<toml::integer>(data.at("max-gold"));

        if (data.contains("max-stat-points"))
            gConfig.maxStatPoint = toml::get<toml::integer>(data.at("max-stat-points"));

        if (data.contains("view-range"))
            gConfig.viewRange = toml::get<toml::integer>(data.at("view-range"));

        if (data.contains("user-limit"))
            gConfig.userLimit = toml::get<toml::integer>(data.at("user-limit"));

        if (data.contains("busy-user-count"))
            gConfig.busyUserCount = toml::get<toml::integer>(data.at("busy-user-count"));

        if (data.contains("full-user-count"))
            gConfig.fullUserCount = toml::get<toml::integer>(data.at("full-user-count"));

        if (data.contains("speedhack-limit-count"))
            gConfig.speedHackLimitCount = toml::get<toml::integer>(data.at("speedhack-limit-count"));

        if (data.contains("speedhack-limit-bonus"))
            gConfig.speedHackLimitBonus = toml::get<toml::integer>(data.at("speedhack-limit-bonus"));

        if (data.contains("synchack-limit-count"))
            gConfig.syncHackLimitCount = toml::get<toml::integer>(data.at("synchack-limit-count"));

        if (data.contains("synchack-limit-time"))
            gConfig.syncHackLimitTime = toml::get<toml::integer>(data.at("synchack-limit-time"));

        if (data.contains("shout-level-limit"))
            gConfig.shoutLevelLimit = toml::get<toml::integer>(data.at("shout-level-limit"));

        if (data.contains("max-stat"))
            gConfig.maxStat = toml::get<toml::integer>(data.at("max-stat"));

        if (data.contains("skillbook-delay"))
            gConfig.skillBookDelay = toml::get<toml::integer>(data.at("skillbook-delay"));

        if (data.contains("item-ownership-time"))
            gConfig.itemOwnershipTime = toml::get<toml::integer>(data.at("item-ownership-time"));

        if (data.contains("item-ownership-time-long"))
            gConfig.itemOwnershipTimeLong = toml::get<toml::integer>(data.at("item-ownership-time-long"));

        if (data.contains("item-ground-time"))
            gConfig.itemGroundTime = toml::get<toml::integer>(data.at("item-ground-time"));

        if (data.contains("item-ground-time-long"))
            gConfig.itemGroundTimeLong = toml::get<toml::integer>(data.at("item-ground-time-long"));

        if (data.contains("pc-max-move-speed"))
            gConfig.pcMaxMoveSpeed = toml::get<toml::integer>(data.at("pc-max-move-speed"));

        if (data.contains("pc-max-hit-range"))
            gConfig.pcMaxHitRange = toml::get<toml::integer>(data.at("pc-max-hit-range"));

        if (data.contains("pc-max-att-speed"))
            gConfig.pcMaxAttackSpeed = toml::get<toml::integer>(data.at("pc-max-att-speed"));

        if (data.contains("mob-max-move-speed"))
            gConfig.mobMaxMoveSpeed = toml::get<toml::integer>(data.at("mob-max-move-speed"));

        if (data.contains("mob-max-att-speed"))
            gConfig.mobMaxAttackSpeed = toml::get<toml::integer>(data.at("mob-max-att-speed"));

        if (data.contains("gm-max-move-speed"))
            gConfig.gmMaxMoveSpeed = toml::get<toml::integer>(data.at("gm-max-move-speed"));

        if (data.contains("buffbot-int"))
            gConfig.buffBotIq = toml::get<toml::integer>(data.at("buffbot-int"));

        if (data.contains("buffbot-skill-level"))
            gConfig.buffBotSkillLevel = toml::get<toml::integer>(data.at("buffbot-skill-level"));

        if (data.contains("game-port"))
            gConfig.gamePort = toml::get<std::string>(data.at("game-port"));

        if (data.contains("db-addr"))
            gConfig.dbAddr = toml::get<toml::string>(data.at("db-addr"));

        if (data.contains("db-port"))
            gConfig.dbPort = toml::get<std::string>(data.at("db-port"));

        if (data.contains("ip"))
        {
            std::string ips = toml::get<std::string>(data.at("ip"));
            ParseIpList(ips.c_str(), gConfig.gameBindIp, gConfig.gameIp);
        }

        if (data.contains("master-ip"))
        {
            std::string ips = toml::get<std::string>(data.at("master-ip"));
            ParseIpList(ips.c_str(), gConfig.masterBindIp, gConfig.masterIp);
        }

        if (data.contains("master-port"))
            gConfig.masterPort = toml::get<std::string>(data.at("master-port"));

        if (data.contains("version")) { gConfig.version = toml::get<std::string>(data.at("version")); }

        if (data.contains("hostname")) { gConfig.hostname = toml::get<std::string>(data.at("hostname")); }

        if (data.contains("ishop-sas")) { gConfig.ishopSas = toml::get<std::string>(data.at("ishop-sas")); }

        if (data.contains("ishop-url")) { gConfig.ishopUrl = toml::get<std::string>(data.at("ishop-url")); }

        if (data.contains("hguard-domain")) { gConfig.hguardUrl = toml::get<std::string>(data.at("hguard-domain")); }

        if (data.contains("server-id")) { gConfig.serverId = toml::get<toml::integer>(data.at("server-id")); }

        if (data.contains("admin-page-password"))
        {
            gConfig.adminpagePassword = toml::get<std::string>(data.at("admin-page-password"));
        }

        if (data.contains("hosted-maps"))
        {
            gConfig.hostedMaps = toml::get<std::vector<uint32_t>>(data.at("hosted-maps"));
        }

        if (data.contains("blocked-horse-skill-maps"))
        {
            gConfig.blockedHorseSkillMaps = toml::get<std::vector<uint32_t>>(data.at("blocked-horse-skill-maps"));
        }

        if (data.contains("blocked-mount-maps"))
        {
            gConfig.blockedMountMaps = toml::get<std::vector<uint32_t>>(data.at("blocked-mount-maps"));
        }

        if (data.contains("blocked-pet-maps"))
        {
            gConfig.blockedPetMaps = toml::get<std::vector<uint32_t>>(data.at("blocked-pet-maps"));
        }

        if (data.contains("blocked-level-pet-maps"))
        {
            gConfig.blockedLevelPetMaps = toml::get<std::vector<uint32_t>>(data.at("blocked-level-pet-maps"));
        }

        if (data.contains("pvp-disabled-maps"))
        {
            gConfig.pvpDisabledMaps = toml::get<std::vector<uint32_t>>(data.at("pvp-disabled-maps"));
        }

        if (data.contains("allow-sash-maps"))
        {
            gConfig.allowSashMaps = toml::get<std::vector<uint32_t>>(data.at("allow-sash-maps"));
        }

        if (data.contains("disallow-sash-maps"))
        {
            gConfig.disallowSashMaps = toml::get<std::vector<uint32_t>>(data.at("disallow-sash-maps"));
        }

        if (data.contains("block-skill-color-maps"))
        {
            gConfig.blockSkillColorMaps = toml::get<std::vector<uint32_t>>(data.at("block-skill-color-maps"));
        }

        if (data.contains("restart-with-affects-maps"))
        {
            gConfig.restartWithAffectsMaps = toml::get<std::vector<uint32_t>>(data.at("restart-with-affects-maps"));
        }

        if (data.contains("duel-only-maps"))
        {
            gConfig.duelOnlyMaps = toml::get<std::vector<uint32_t>>(data.at("duel-only-maps"));
        }
        if (data.contains("check-hack-maps"))
        {
            gConfig.hackCheckedMaps = toml::get<std::vector<uint32_t>>(data.at("check-hack-maps"));
        }

        if (data.contains("disable-remove-good-maps")) {
            gConfig.sureRemoveGoodDisabledMaps = toml::get<std::vector<uint32_t>>(data.at("disable-remove-good-maps"));
        }

        if (data.contains("disable-votebuff-maps")) {
            gConfig.disableVoteBuffMaps = toml::get<std::vector<uint32_t>>(data.at("disable-votebuff-maps"));
        }
 
        if (!gConfig.allowSashMaps.empty() && !gConfig.disallowSashMaps.empty())
        {
            SPDLOG_CRITICAL("allow-sash-maps and disallow-sash-maps cannot be filled at the same time");
            return false;
        }

        if (data.contains("instant-restart-maps"))
        {
            gConfig.instantRestartMaps = toml::get<std::vector<uint32_t>>(data.at("instant-restart-maps"));
        }

        if (data.contains("admin-page-ips"))
        {
            gConfig.adminpageAddrs = toml::get<std::vector<std::string>>(data.at("admin-page-ips"));
        }

        if (data.contains("supported-locales"))
        {
            gConfig.localeNames = toml::get<std::vector<std::string>>(data.at("supported-locales"));
        }

        if (data.contains("map-level-limit"))
        {
            const auto mapLevelLimit = toml::find(data, "map-level-limit");

            const auto levelLimitUpper = toml::get<std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>
            >(mapLevelLimit);
            for (const auto &elem : levelLimitUpper)
            {
                gConfig.mapLevelLimit[std::get<0>(elem)] = {std::get<1>(elem), std::get<2>(elem)};
            }
        }
    }
    catch (const toml::syntax_error &err)
    {
        SPDLOG_CRITICAL(err.what());
        return false;
    }catch (const toml::type_error &err)
    {
        SPDLOG_CRITICAL(err.what());
        return false;
    }

    return true;
}

void LoadDbElement(const toml::value data, DbConfig &db, std::string dbName)
{
    if (data.count(dbName) != 0)
    {
        const auto tab = toml::get<toml::table>(data.at(dbName));
        db.user = toml::get<std::string>(tab.at("user"));
        db.addr = toml::get<std::string>(tab.at("host"));
        db.port = toml::get_or(tab.at("port"), 3306);
        db.password = toml::get<std::string>(tab.at("password"));
        db.name = toml::get<std::string>(tab.at("name"));
    }
}

bool LoadDbConfig()
{
    try
    {
        const auto data = toml::parse("config.db.toml");
        LoadDbElement(data, gConfig.playerDb, "player");
        LoadDbElement(data, gConfig.commonDb, "common");
        LoadDbElement(data, gConfig.logDb, "log");
        LoadDbElement(data, gConfig.accountDb, "account");

        return true;
    }
    catch (...) { return false; }
}

void LoadCommandPrivileges()
{
    FILE *fp;
    char buf[256];

    if ((fp = fopen("CMD", "r")))
    {
        uint32_t lineno = 1;
        while (fgets(buf, 256, fp))
        {
            ++lineno;
            char cmd[32], levelname[32];
            int level;

            two_arguments(buf, cmd, sizeof(cmd), levelname, sizeof(levelname));

            if (!*cmd || !*levelname)
            {
                fprintf(
                    stderr, "CMD syntax error: <cmd> <DISABLE | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD> at line %d\n",
                    lineno);
                exit(1);
            }

            if (!strcasecmp(levelname, "LOW_WIZARD"))
                level = GM_LOW_WIZARD;
            else if (!strcasecmp(levelname, "WIZARD"))
                level = GM_WIZARD;
            else if (!strcasecmp(levelname, "HIGH_WIZARD"))
                level = GM_HIGH_WIZARD;
            else if (!strcasecmp(levelname, "GOD"))
                level = GM_GOD;
            else if (!strcasecmp(levelname, "IMPLEMENTOR"))
                level = GM_IMPLEMENTOR;
            else if (!strcasecmp(levelname, "DISABLE"))
                level = GM_IMPLEMENTOR + 1;
            else
            {
                fprintf(stderr, "CMD syntax error: <cmd> <DISABLE | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD>\n");
                exit(1);
            }

            interpreter_set_privilege(cmd, level);
        }

        fclose(fp);
    }
}

bool LoadConfiguration()
{
    LoadCommandPrivileges();

    try
    {
        return LoadDbConfig() && LoadMainConfig("config.toml") && LoadMainConfig("config.core.toml") && LoadMainConfig(
                   "config.global.toml");
    }
    catch (const std::exception& e)
    {
        fmt::print(e.what());
        return false;
    }
}

bool g_bNoMoreClient = false;

void map_allow_copy(int32_t *pl, int size)
{
    int iCount = 0;

    auto it = gConfig.hostedMaps.begin();
    while (it != gConfig.hostedMaps.end())
    {
        int i = *(it++);
        *(pl++) = i;

        if (++iCount > size)
            break;
    }
}

bool IsEmptyAdminPage() { return gConfig.adminpageAddrs.empty(); }

bool IsAdminPage(const char *ip)
{
    return std::find(gConfig.adminpageAddrs.begin(), gConfig.adminpageAddrs.end(), ip) != gConfig.adminpageAddrs.end();
}

void ClearAdminPages() { gConfig.adminpageAddrs.clear(); }
