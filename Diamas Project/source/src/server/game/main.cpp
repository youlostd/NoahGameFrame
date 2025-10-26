#include "main.h"

#include <game/GamePacket.hpp>
#include <storm/memory/NewAllocator.hpp>

#include <chrono>

#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "db.h"
#include "desc.h"
#include "desc_manager.h"
#include "event.h"
#include "item_manager.h"
#include "log.h"
#include "lzo_manager.h"
#include "messenger_manager.h"
#include "mob_manager.h"
#include "motion.h"
#include "party.h"
#include "pvp.h"
#include "questmanager.h"
#include "regen.h"
#include "sectree_manager.h"
#include "shop_manager.h"
#include "skill.h"

#include "DragonSoul.h"
#include "GArena.h"
#include "GBanWord.h"
#include "GRefineManager.h"
#include "MarkManager.h"
#include "OXEvent.h"
#include "building.h"
#include "cmd.h"
#include "dungeon.h"
#include "fishing.h"
#include "guild_manager.h"
#include "horsename_manager.h"
#include "item_addon.h"
#include "itemname_manager.h"
#include "map_location.h"
#include "marriage.h"
#include "polymorph.h"
#include "priv_manager.h"
#include "target.h"
#include "threeway_war.h"
#include "war_map.h"
#include "wedding.h"

#if defined(WJ_COMBAT_ZONE)
#include "combat_zone.h"
#endif
#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif
#ifdef __OFFLINE_SHOP__
#include "OfflineShopManager.h"
#endif
#ifdef ENABLE_HYDRA_DUNGEON
#include "HydraDungeon.h"
#endif

#ifdef ENABLE_BATTLE_PASS
#include "battle_pass.h"
#endif

#include "ChatUtil.hpp"
#include "DbCacheSocket.hpp"
#include "cube.h"

#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include "DbCacheSocket.hpp"
#include <boost/asio/io_service.hpp>
#include <game/HuntingManager.hpp>

#include "dungeon_info.h"
#include "mining.h"

#include <base/Console.hpp>
#include <csignal>

#ifdef _WIN_VRUNNER_
#include <fstream>
#endif


namespace
{
struct SendDisconnectFunc {
    void operator()(DESC* d)
    {
        if (d->GetCharacter()) {
            if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
                d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit "
                                                                 "Shutdown("
                                                                 "SendDisconnec"
                                                                 "tFunc)");
        }
    }
};

class Main
{
    public:
    Main();
    ~Main();

    int Run(int argc, const char** argv);

    MotionManager& GetMotionManager() { return m_motionManager; }

    LocaleService& GetLocaleService() { return m_localeService; }

    private:
    bool Initialize();
    void Quit();

    void Loop();

    void WaitForSignals();
    void UpdateTick(const boost::system::error_code& ec);
    void OpenLogs();

    void StartUpdateTimer();
    void FlushTick(const boost::system::error_code& ec);
    void StartFlushTimer();

    Remotery* rmt = nullptr;

    std::shared_ptr<spdlog::logger> m_logger;

    asio::io_service m_ioService;
    asio::signal_set m_signals;
    //
    // [tim] The order of these members is very important.
    // They're all accessed using global variables, so dependencies are very
    // fragile -.-
    //

    LocaleService m_localeService;
    LZOManager m_lzoManager;
    CPolymorphUtils m_polymorphUtils;
    CMobManager m_mobManager;
    MotionManager m_motionManager;

    DBManager m_dbManager;
    LogManager m_logManager;
    MessengerManager m_messengerManager;
    CBanwordManager m_banwordManager;

    DESC_MANAGER m_descManager;

    CMapLocation m_mapLocation;

    CHARACTER_MANAGER m_charManager;
    SECTREE_MANAGER m_sectreeManager;

    CShopManager m_shopManager;
    CSkillManager m_skillManager;
    CPartyManager m_partyManager;
    CPVPManager m_pvpManager;
    CPrivManager m_privManager;
    CTargetManager m_targetManager;
    CHorseNameManager m_horseNameManager;
    CItemNameManager m_itemNameManager;
    quest::CQuestManager m_questManager;
#ifdef __OFFLINE_SHOP__
    COfflineShopManager m_offlineShopManager;
#endif
#ifdef ENABLE_BATTLE_PASS
    CBattlePass m_battlePass;
#endif
    CDungeonInfo m_dungeonInfo;

    // [tim] Also controls ground-items => dependency on SECTREE_MANAGER
    ITEM_MANAGER m_itemManager;
    CRefineManager m_refineManager;
    CItemAddonManager m_itemAddonManager;

    CGuildManager m_guildManager;
    CGuildMarkManager m_markManager;
    CWarMapManager m_warMapManager;
    building::CManager m_buildingManager;

    marriage::CManager m_marriageManager;
    marriage::WeddingManager m_weddingManager;

    // Dungeon managers...
    CDungeonManager m_dungeonManager;
    CThreeWayWar m_threewayWar;
    CArenaManager m_arenaManager;
    COXEventManager m_oxEventManager;

#if defined(WJ_COMBAT_ZONE)
    CCombatZoneManager m_combatZoneManager;
#endif

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    MeleyLair::CMgr m_meleyManager;
#endif
#ifdef ENABLE_HYDRA_DUNGEON
    CHydraDungeonManager m_hydraManager;
#endif

    DSManager m_dsManager;
    HuntingManager m_huntingManager;

    asio::high_resolution_timer m_updateTimer;
    asio::high_resolution_timer m_flushTimer;

    protected:
    storm::NewAllocator m_allocator;
};
} // namespace

Main* globalInstance = nullptr;
using namespace std::chrono_literals;

Main::Main()
#ifdef VSTD_PLATFORM_WINDOWS
    : m_signals(m_ioService, SIGTERM, SIGBREAK)
#else
    : m_signals(m_ioService, SIGTERM, SIGHUP)

#endif
    , m_descManager(m_ioService)
    , m_updateTimer(m_ioService, 40000us)
    , m_flushTimer(m_ioService, 60s)

{
    storm::SetDefaultAllocator(&m_allocator);
    rmt_CreateGlobalInstance(&rmt);

    OpenLogs();

    globalInstance = this;

    ilInit();
}

Main::~Main()
{
    globalInstance = nullptr;

    ilShutDown();

    storm::SetDefaultAllocator(nullptr);
    if (m_logger)
        m_logger->flush();

    spdlog::shutdown();

    // Destroy the main instance of Remotery.
    rmt_DestroyGlobalInstance(rmt);
}

int Main::Run(int argc, const char** argv)
{
    if (!Initialize())
        return 1;


#ifdef _WIN_VRUNNER_
    DWORD pid = GetCurrentProcessId();

    std::ofstream output("pid.txt");
    if (output.is_open()) {
        output << pid;
        output.close();
    }
 #endif


    WaitForSignals();

    Loop();

    db_clientdesc.reset();

    SPDLOG_INFO("<shutdown> event_destroy()...");
    event_destroy();

    return 0;
}

void Main::WaitForSignals()
{
    m_signals.async_wait([this](const boost::system::error_code& ec, int sig) {
        if (ec == asio::error::operation_aborted)
            return;

        if (ec) {
            SPDLOG_ERROR("Waiting for signals {0} failed with {1}", sig,
                         ec.message());
            return;
        }

        SPDLOG_INFO("Received signal {0}", sig);
#ifdef VSTD_PLATFORM_WINDOWS
        if (sig == SIGTERM || sig == SIGBREAK) {
#else
        if (sig == SIGTERM || sig == SIGHUP) {
#endif

            Quit();
            return;
        }

        WaitForSignals();
    });
}

void Main::UpdateTick(const boost::system::error_code& ec)
{
    if (ec)
        return;
    rmt_ScopedCPUSample(gUpdateTick, RMTSF_Recursive);

    if (!thecore_is_shutdowned()) {
        ++thecore_heart->pulse;

        event_process(thecore_heart->pulse);
        m_dbManager.Process();

        m_pvpManager.Process();
        m_charManager.Update(thecore_heart->pulse);

        if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 5 + 2))) {
            m_charManager.UpdateWorldBoss();
        }

        if (gConfig.ccuServer) {

            if (!(thecore_heart->pulse %
                  (thecore_heart->passes_per_sec * 3600))) {
                int iTotal;
                int* paiEmpireUserCount;
                int iLocal;
                m_descManager.GetUserCount(iTotal, &paiEmpireUserCount, iLocal);
                m_dbManager.Query("INSERT INTO {}.ccu_log SET count_total = "
                                  "{}, count_red = {}, count_yellow = {}, "
                                  "count_blue = {};",
                                  gConfig.logDb.name, iTotal,
                                  paiEmpireUserCount[1], paiEmpireUserCount[2],
                                  paiEmpireUserCount[3]);
            }
        }
    }

    m_updateTimer.expires_after(40000us);
    StartUpdateTimer();
};

void Main::StartUpdateTimer()
{
    m_updateTimer.async_wait(
        [this](const boost::system::error_code& ec) { UpdateTick(ec); });
}

void Main::FlushTick(const boost::system::error_code& ec)
{
    if (ec) {
        return;
    }

    rmt_ScopedCPUSample(FlushTick, 0);

    rmt_BeginCPUSample(ItemManagerUpdate, 0);
    ITEM_MANAGER::instance().Update();
    rmt_EndCPUSample();
    rmt_BeginCPUSample(ProcessDelayedSave, 0);
    m_charManager.ProcessDelayedSave();
    rmt_EndCPUSample();

    m_descManager.UpdateLocalUserCount();

    m_flushTimer.expires_after(30s);
    StartFlushTimer();
}

void Main::StartFlushTimer()
{
    m_flushTimer.async_wait(
        [this](const boost::system::error_code& ec) { FlushTick(ec); });
}

void Main::Quit()
{
    SPDLOG_INFO("<shutdown> Starting...");

    SPDLOG_INFO("<shutdown> Destroying CArenaManager...");
    m_arenaManager.Destroy();

    SPDLOG_INFO("<shutdown> Destroying COXEventManager...");
    m_oxEventManager.Destroy();

    SPDLOG_INFO("<shutdown> Destroying COfflineShopManager...");
    m_offlineShopManager.Destroy();

    SPDLOG_INFO("<shutdown> Shutting down CHARACTER_MANAGER...");
    m_charManager.GracefulShutdown();

    SPDLOG_INFO("<shutdown> Shutting down ITEM_MANAGER...");
    m_itemManager.GracefulShutdown();

    SPDLOG_INFO("<shutdown> Destroying CShopManager...");
    m_shopManager.Destroy();

    SPDLOG_INFO("<shutdown> Destroying CHARACTER_MANAGER...");
    m_charManager.Destroy();

    SPDLOG_INFO("<shutdown> Destroying quest::CQuestManager...");
    m_questManager.Destroy();

    SPDLOG_INFO("<shutdown> regen_free()...");
    regen_free();

    SPDLOG_INFO("<shutdown> DESC_MANAGER::Quit");
    m_descManager.Quit();
    m_flushTimer.cancel();
    m_updateTimer.cancel();
    m_signals.cancel();

    db_clientdesc->Disconnect();
    m_ioService.stop();
}

void Main::Loop()
{
    SPDLOG_INFO("Entering main loop");
#if VSTD_PLATFORM_WINDOWS
    auto v = ScopedConsoleCloseHandler([this] { Quit(); });
#endif

    StartUpdateTimer();
    StartFlushTimer();

    m_ioService.run();

    SPDLOG_INFO("Exiting main loop");
}

bool Main::Initialize()
{
    if (!LoadConfiguration())
        return false;

    bool ok = thecore_init(gConfig.passesPerSecond);
    if (!ok) {
        SPDLOG_ERROR("thecore_init() failed");
        return false;
    }

    m_logger->set_level(
        static_cast<spdlog::level::level_enum>(gConfig.logLevel));
    m_logger->flush_on(
        static_cast<spdlog::level::level_enum>(gConfig.logLevel));

    // Player DB
    DBManager::instance().Connect(
        gConfig.playerDb.addr.c_str(), gConfig.playerDb.port,
        gConfig.playerDb.user.c_str(), gConfig.playerDb.password.c_str(),
        gConfig.playerDb.name.c_str());

    if (!DBManager::instance().IsConnected()) {
        SPDLOG_ERROR("Failed to connect to PLAYER_SQL");
        return false;
    }
    // Log DB 접속
    LogManager::instance().Connect(
        gConfig.logDb.addr.c_str(), gConfig.logDb.port,
        gConfig.logDb.user.c_str(), gConfig.logDb.password.c_str(),
        gConfig.logDb.name.c_str());

    LogManager::instance().ConnectAsync(
        gConfig.logDb.addr.c_str(), gConfig.logDb.port,
        gConfig.logDb.user.c_str(), gConfig.logDb.password.c_str(),
        gConfig.logDb.name.c_str());

    if (!LogManager::instance().IsConnected()) {
        SPDLOG_ERROR("Failed to connect to LOG_SQL");
        return false;
    }

    LogManager::instance().BootLog(gConfig.hostname.c_str(), gConfig.channel);

    if (gConfig.localeNames.empty()) {
        SPDLOG_ERROR("No locales specified");
        return false;
    }

    for (const auto& name : gConfig.localeNames) {
        SPDLOG_INFO("Loading locale: {0}", name);
        m_localeService.AddLocale(name);
    }

    m_localeService.SetDefaultLocale(gConfig.localeNames[0]);

#ifdef __FreeBSD__
    setproctitle("%s", gConfig.hostname.c_str());
#else
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleTitle(gConfig.hostname.c_str());
#endif
#endif

    uint16_t port;
    storm::ParseNumber(gConfig.gamePort.c_str(), port);

    for (auto mapIndex : gConfig.hostedMaps) {
        CMapLocation::instance().Insert(gConfig.channel, mapIndex,
                                        gConfig.gameIp.c_str(), port);
    }

    interpreter_load_config("data/CMD");

    db_clientdesc = std::make_shared<DbCacheSocket>(m_ioService);
    if (!db_clientdesc->Connect(gConfig.dbAddr, gConfig.dbPort))
        return false;

    m_descManager.Setup(gConfig.gameBindIp, gConfig.gamePort);
    m_descManager.ConnectMaster(gConfig.masterBindIp, gConfig.masterPort);

    if (!m_questManager.Initialize())
        return false;

    m_messengerManager.Initialize();
    m_guildManager.Initialize();
    fishing::Initialize();
    m_oxEventManager.Initialize();
    m_arenaManager.Initialize();
    if (!m_refineManager.Initialize())
        return false;
    if (!m_itemManager.Initialize())
        return false;
    if (!m_mobManager.Initialize())
        return false;
    if (!m_threewayWar.Initialize())
        return false;
    if (!m_motionManager.Initialize())
        return false;
    if (!m_sectreeManager.Initialize())
        return false;
    if (!m_buildingManager.Initialize())
        return false;
    if (!m_shopManager.Initialize())
        return false;
    if (!m_skillManager.Initialize())
        return false;

    if (!m_huntingManager.LoadServer())
        return false;

#ifdef ENABLE_BATTLE_PASS
    if (!m_battlePass.ReadBattlePassFile())
        return false;
#endif
    if (!Cube_init())
        return false;

    m_itemManager.InitializeDropInfo();

    if (!m_charManager.LoadWorldBossInfo("data/world_boss.txt"))
        return false;

    if (!m_charManager.LoadAutoNotices())
        return false;

    if (!m_dungeonInfo.ReadDungeonInfoFile("data/dungeon_info.txt"))
        return false;

    if(!mining::LoadMiningExtraDrops("data/mining_extra_drops.txt"))
        return false;

    return true;
}

#ifdef _DEBUG
#define FULL_LOG
#endif

void Main::OpenLogs()
{

    auto max_size = 1048576 * 5;
    auto max_files = 3;
    m_logger = spdlog::rotating_logger_st("game_server", "syslog", max_size,
                                          max_files, true);
    m_logger->set_level(spdlog::level::err);

    try {
        spdlog::set_default_logger(m_logger);
    } catch (const spdlog::spdlog_ex& ex) {
        fmt::print(ex.what());
    }
}

MotionManager& GetMotionManager()
{
    return globalInstance->GetMotionManager();
}

LocaleService& GetLocaleService()
{
    return globalInstance->GetLocaleService();
}

extern "C" int main(int argc, const char** argv)
{
    auto m = std::make_unique<Main>();
    return m->Run(argc, argv);
}

namespace boost
{
void throw_exception(std::exception const& e)
{
    std::fprintf(stderr, "Encountered an uncaught exception with message %s",
                 e.what());

    std::abort();
}
} // namespace boost

namespace storm
{
bool HandleAssertionFailure(const char* filename, int lineno,
                            const char* functionName, const char* expr,
                            const char* msg)
{
    std::fprintf(stderr,
                 "Encountered an assertion failure at [%s:%d] %s with message: "
                 "%s\n",
                 filename, lineno, functionName, msg);

    std::abort();
    return true;
}

bool HandleFatalFailure(const char* filename, int lineno,
                        const char* functionName, const char* msg)
{
    std::fprintf(stderr,
                 "Encountered a fatal failure at [%s:%d] %s with message: %s\n",
                 filename, lineno, functionName, msg);

    std::abort();
    return true;
}
} // namespace storm
