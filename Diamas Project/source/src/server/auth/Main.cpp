#include "Config.hpp"
#include "Server.hpp"



#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include "base/Console.hpp"

namespace po = boost::program_options;

#include <iostream>
#include <SpdLog.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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
		void OpenLogs();

		asio::io_service m_ioService;
		asio::signal_set m_signals;

		Server m_server;
};

Main::Main()
	: m_signals(m_ioService, SIGINT, SIGTERM)
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
		spdlog::error("args: {0}", e.what());
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
	ScopedConsoleCloseHandler(std::bind(&Main::Quit, this));
#endif

		m_ioService.run();
	}

	return 0;
}

bool Main::ParseArguments(int argc, const char** argv)
{
	std::string action;

	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help,h", "produce a help message")
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
	Config c{};

	if (!LoadFile(c, "auth.config.toml"))
		return false;

	SPDLOG_INFO("Auth server");
	SPDLOG_INFO("Attempting to start...");
    spdlog::set_level(static_cast<spdlog::level::level_enum>(c.logLevel));
    spdlog::flush_on(static_cast<spdlog::level::level_enum>(c.logLevel));

	m_server.Start(c);

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
			spdlog::error("Waiting for signals {0} failed with {1}",
				sig, ec.message());
			return;
		}

		spdlog::trace("Received signal {0}", sig);

		if (sig == SIGINT || sig == SIGTERM) {
			Quit();

			// Stop waiting for signals
			return;
		}

		if (sig == SIGUSR1) {
			m_server.PrintPendingLogins();

			// Stop waiting for signals
			return;
		}


		WaitForSignals();
	});
}

void Main::OpenLogs()
{
	spdlog::init_thread_pool(8192, 1);
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
	auto rotating_syserr_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("syserr.txt", 1024 * 1024 * 10, 10);
	auto rotating_syslog_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("syslog.txt", 1024 * 1024 * 10, 10);
	std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_syserr_sink, rotating_syslog_sink };
	auto logger = std::make_shared<spdlog::async_logger>("auth", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
			spdlog::register_logger(logger);


#define FULL_LOG
#if defined(FULL_LOG)
	logger->set_level(spdlog::level::trace);
	sinks[0]->set_level(spdlog::level::trace); // console. Allow everything.  Default value
	sinks[1]->set_level(spdlog::level::err); //  syserr
	sinks[2]->set_level(spdlog::level::trace); //  log
#else
		logger->set_level(spdlog::level::info);
		sinks[0]->set_level(spdlog::level::trace); // console. Allow everything.  Default value
		sinks[1]->set_level(spdlog::level::err); //  syserr
		sinks[2]->set_level(spdlog::level::info); //  syserr		
#endif
	logger->flush_on(spdlog::level::err); 
		spdlog::set_default_logger(logger);
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
