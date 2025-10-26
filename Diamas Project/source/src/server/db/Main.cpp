#include "Config.h"
#include "Peer.h"
#include "DBManager.h"
#include "ClientManager.h"
#include "GuildManager.h"
#ifdef ENABLE_GUILD_STORAGE
#include "GuildStorageManager.h"
#endif
#include "ItemAwardManager.h"
#include "QID.h"

#include "PrivManager.h"
#include "MoneyLog.h"
#include "Marriage.h"
#include "ItemIDRangeManager.h"

#include <csignal>
#include <storm/memory/NewAllocator.hpp>
#include <thecore/utils.hpp>

#include <iostream>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include "base/Console.hpp"

#ifdef _WIN_VRUNNER_
#include <fstream>
#endif

int g_iPlayerCacheFlushSeconds = 60 * 10;
int g_iItemCacheFlushSeconds = 60 * 10;
int g_iItemPriceListTableCacheFlushSeconds = 540;
int g_iShopCacheFlushSeconds = 360;

#ifndef DEBUG
	void emergency_sig(int sig)
	{
		if (sig == SIGSEGV)
			SPDLOG_INFO( "SIGNAL: SIGSEGV");
		else if (sig == SIGUSR1)
			SPDLOG_INFO( "SIGNAL: SIGUSR1");

		if (sig == SIGSEGV)
			abort();
	}
#endif


class Main
{
public:
	Main();
	~Main();

	int Run(int argc, const char** argv);

private:

	bool Start();
	void Quit();

	void WaitForSignals();
	void OpenLogs();

	static Main* s_main;

	asio::io_service m_ioService;
	asio::signal_set m_signals;
    std::shared_ptr<spdlog::logger> m_logger;

	Config m_config;
	CDBManager m_dbManager;
	CClientManager m_clientManager;
	CGuildManager m_guildManager;
	CPrivManager m_privManager;
	CMoneyLog m_moneyLog;
	ItemAwardManager m_itemAwardManager;
	marriage::CManager m_marriageManager;
#ifdef ENABLE_GUILD_STORAGE
	CGuildStorageManager storageManager;
#endif
	CItemIDRangeManager m_idRangeManager;

protected:
	storm::NewAllocator m_allocator;
};


/*static*/
Main* Main::s_main = nullptr;

Main::Main()
	: m_signals(m_ioService, SIGINT, SIGTERM)
	  , m_config({})
	  , m_clientManager(m_ioService, m_config)
	  , m_guildManager(m_config)
	  , m_itemAwardManager(m_ioService)
{
	storm::SetDefaultAllocator(&m_allocator);

#if !VSTD_PLATFORM_WINDOWS
		m_signals.add(SIGUSR1);
#endif
	OpenLogs();
	s_main = this;
}

Main::~Main()
{
	storm::SetDefaultAllocator(nullptr);

	s_main = nullptr;
}


int Main::Run(int argc, const char** argv)
{
	if(!Start())
		return 1;

	
#ifdef _WIN_VRUNNER_
    DWORD pid = GetCurrentProcessId();

    std::ofstream output("pid.txt");
    if (output.is_open()) {
        output << pid;
        output.close();
    }
#endif

	m_idRangeManager.Boot();

	if(!m_clientManager.Initialize()) {
		spdlog::error("ClientManager initialization failed");
		return false;
	}
	m_guildManager.Initialize();
	m_marriageManager.Initialize();
	m_itemAwardManager.StartReloadTimer();

	WaitForSignals();

	{
#if VSTD_PLATFORM_WINDOWS
		auto scopeHandler = ScopedConsoleCloseHandler(std::bind(&Main::Quit, this));
#endif
		m_ioService.run();
	}

	SPDLOG_INFO("Metin2DBCacheServer Stop");

	signal_setup();


	// MySQL connections aren't managed by our io_service
	// At least, not yet
	m_dbManager.Quit();


	return 0;
}
#define FULL_LOG

void Main::OpenLogs()
{
    auto max_size = 1048576 * 5;
    auto max_files = 3;
    m_logger = spdlog::rotating_logger_st("dbcache", "syslog", max_size,
                                          max_files, true);
    m_logger->set_level(spdlog::level::err);

    try {
        spdlog::set_default_logger(m_logger);
    } catch (const spdlog::spdlog_ex& ex) {
        fmt::print(ex.what());
    }
	m_logger->flush_on(spdlog::level::err);
	spdlog::set_default_logger(m_logger);

}


bool Main::Start()
{
	if(!LoadFile(m_config, "dbcache.config.toml"))
		return false;

	if(!m_dbManager.Connect(SQL_PLAYER, m_config.playerDb))
		return false;

	if(!m_dbManager.Connect(SQL_ACCOUNT, m_config.accountDb))
		return false;

	if(!m_dbManager.Connect(SQL_COMMON, m_config.commonDb))
		return false;

	if(!m_dbManager.Connect(SQL_LOG, m_config.logDb))
		return false;

	if(m_config.playerDeleteLevelMax == 0)
		m_config.playerDeleteLevelMax = PLAYER_MAX_LEVEL_CONST + 1;

	if(m_config.playerCacheFlushTime != 0)
		g_iPlayerCacheFlushSeconds = m_config.playerCacheFlushTime;

	if(m_config.itemCacheFlushTime != 0)
		g_iItemCacheFlushSeconds = m_config.itemCacheFlushTime;

	if(m_config.itemPriceListCacheFlushTime != 0)
		g_iItemPriceListTableCacheFlushSeconds = m_config.itemPriceListCacheFlushTime;

	return true;
}

void Main::Quit()
{
	m_itemAwardManager.Quit();
	m_clientManager.Quit();
	m_signals.cancel();
}

void Main::WaitForSignals()
{
	m_signals.async_wait([this](const boost::system::error_code& ec, int sig)
	{
		if(ec == asio::error::operation_aborted)
			return;

		if(ec) {
			spdlog::error("Waiting for signals {0} failed with {1}",
			              sig, ec);
			return;
		}

		spdlog::trace("Received signal {0}", sig);

		if(sig == SIGINT || sig == SIGTERM) {
			Quit();

			// Stop waiting for signals
			return;
		}

		if(sig == SIGUSR1)
			OpenLogs();

		WaitForSignals();
	});
}

int main(int argc, const char** argv)
{
	return Main().Run(argc, argv);
}


namespace boost
{
	void throw_exception(std::exception const& e)
	{
		std::fprintf(stderr,
		             "Encountered an uncaught exception with message %s",
		             e.what());

		std::abort();
	}
}

namespace storm
{
	bool HandleAssertionFailure(const char* filename, int lineno,
	                            const char* functionName, const char* expr,
	                            const char* msg)
	{
		std::fprintf(stderr,
		             "Encountered an assertion failure at [%s:%d] %s with message: %s\n",
		             filename, lineno, functionName, msg);

		std::abort();
		return true;
	}

	bool HandleFatalFailure(const char* filename, int lineno,
	                        const char* functionName,
	                        const char* msg)
	{
		std::fprintf(stderr,
		             "Encountered a fatal failure at [%s:%d] %s with message: %s\n",
		             filename, lineno, functionName, msg);

		std::abort();
		return true;
	}
}
