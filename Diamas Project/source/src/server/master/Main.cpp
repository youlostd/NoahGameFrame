#include "Config.hpp"
#include "Server.hpp"


#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "spdlog/spdlog.h"

#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "base/Console.hpp"

namespace po = boost::program_options;

#include <iostream>
#ifdef _WIN_VRUNNER_
#include <fstream>
#endif

namespace
{

class Main 
{
	public:
		Main();

		int Run(int argc, const char** argv);

	private:
		bool ParseArguments(int argc, const char** argv);

		bool Initialize();
		void Quit();

		void WaitForSignals();
		void StartUpdateTimer();
		void OpenLogs();

		asio::io_service m_ioService;
		asio::signal_set m_signals;
		asio::basic_waitable_timer<std::chrono::steady_clock> m_updateTimer;
        std::shared_ptr<spdlog::async_logger> m_logger;

		Server m_server;
};

Main::Main()
	: m_signals(m_ioService, SIGINT, SIGTERM)
	, m_updateTimer(m_ioService)
	, m_server(m_ioService)
{
#if !VSTD_PLATFORM_WINDOWS
	m_signals.add(SIGUSR1);
#endif

	OpenLogs();

}

int Main::Run(int argc, const char** argv)
{
	try {
		if (!ParseArguments(argc, argv))
			return 1;
	} catch (std::exception& e) {
		SPDLOG_ERROR( "args: {0}", e.what());
		return 1;
	}

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

	{
#if VSTD_PLATFORM_WINDOWS
		auto sc = ScopedConsoleCloseHandler([this]()
		{
		    Quit();
		});
#endif

		m_ioService.run();
	}

	return 0;
}

bool Main::ParseArguments(int argc, const char** argv)
{
	vstd::string action;

	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help,h", "produce a help message")
	    ("version,V", "output the version number")
	;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

	if (vm.count("help")) {
		std::cout << desc;
		return false;
	}


	po::notify(vm);
	return true;
}

bool Main::Initialize()
{
	Config c;
	if (!LoadFile(c, "master.config.toml"))
		return false;

	SPDLOG_INFO("Master server");
	SPDLOG_INFO("Attempting to start...");
	m_logger->set_level(static_cast<spdlog::level::level_enum>(c.logLevel));
	m_server.Start(c);
	StartUpdateTimer();
	return true;
}

void Main::Quit()
{
	SPDLOG_INFO("QUIT");

	m_signals.cancel();
	m_server.Quit();
}

void Main::WaitForSignals()
{
	m_signals.async_wait([this] (const boost::system::error_code& ec, int sig) {
		if (ec == boost::asio::error::operation_aborted || ec == boost::asio::error::bad_descriptor)
			return;

		if (ec) {
			SPDLOG_ERROR( "Waiting for signals {0} failed with {1}",
			          sig, ec.message());
			return;
		}

		spdlog::trace( "Received signal {0}", sig);

		if (sig == SIGINT || sig == SIGTERM) {
			Quit();

			// Stop waiting for signals
			return;
		}


		WaitForSignals();
	});
}

void Main::StartUpdateTimer()
{
	m_updateTimer.expires_from_now(std::chrono::seconds(30));
	m_updateTimer.async_wait([this] (const boost::system::error_code& ec) {


		if (ec) {
			return;
		}

		//m_server.GetLoginSessions().Update();
		m_server.GetOnlinePlayers().Update();

		StartUpdateTimer();
	});
}

void Main::OpenLogs()
{
    spdlog::init_thread_pool(8192, 1);
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotating_syserr_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        "syserr", 3, 0, true, 3);
    auto rotating_syslog_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        "syslog", 3, 0, true, 3);
    std::vector<spdlog::sink_ptr> sinks{stdout_sink, rotating_syserr_sink, rotating_syslog_sink};
    m_logger = std::make_shared<spdlog::async_logger>("master", sinks.begin(), sinks.end(),
                                                      spdlog::thread_pool(),
                                                      spdlog::async_overflow_policy::overrun_oldest);

#if defined(FULL_LOG)
    m_logger->set_level(spdlog::level::debug);
    sinks[0]->set_level(spdlog::level::debug); // console. Allow everything.  Default value
    sinks[1]->set_level(spdlog::level::err);   //  syserr
    sinks[2]->set_level(spdlog::level::debug); //  log
#else
		m_logger->set_level(spdlog::level::info);
		sinks[0]->set_level(spdlog::level::trace); // console. Allow everything.  Default value
		sinks[1]->set_level(spdlog::level::err); //  syserr
		sinks[2]->set_level(spdlog::level::info); //  syslog		
#endif

    spdlog::register_logger(m_logger);
    spdlog::set_default_logger(m_logger);
}

extern "C" int main(int argc, const char** argv)
{ 
	return Main().Run(argc, argv); 
}

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
