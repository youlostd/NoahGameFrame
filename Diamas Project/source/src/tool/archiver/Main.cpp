#include "Main.hpp"

#include <base/SimpleApp.hpp>
#include <base/DumpVersion.hpp>

#include <pak/PakWriter.hpp>
#include <pak/PakFilename.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <ppl.h>
#include <concurrent_vector.h>
#include <ppltasks.h>
#include <ppltaskscheduler.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "XmlUtil.hpp"

	using namespace concurrency;

METIN2_BEGIN_NS

namespace
{

	class Main : public SimpleApp
	{
	public:
		Main();
		void OpenLogs();

		int Run(int argc, const char** argv);
		task<void> ParseXMLFiles();
	private:
		bool ParseArguments(int argc, const char** argv);

		bool m_verbose;
		std::vector<std::string> m_xmls;
	};

	// Observes all exceptions that occurred in all tasks in the given range.
template<class T, class InIt> 
void observe_all_exceptions(InIt first, InIt last) 
{
    std::for_each(first, last, [](concurrency::task<T> t)
    {
        t.then([](concurrency::task<T> previousTask)
        {
            try
            {
                previousTask.get();
            }
            // Although you could catch (...), this demonstrates how to catch specific exceptions. Your app
            // might handle different exception types in different ways.
            catch (...)
            {
            	SPDLOG_INFO("An exception occured");
                // Swallow the exception.
            }
        });
    });
}

}

Main::Main()
	: m_verbose(false)
{
	OpenLogs();
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

int Main::Run(int argc, const char** argv)
{
	try {
		if (!ParseArguments(argc, argv))
			return 1;
	}
	catch (std::exception& e) {
		SPDLOG_ERROR("args: {0}", e.what());
		return 1;
	}

	ParseXMLFiles().then([](task<void> previousTask)
    {
        try
        {
            previousTask.get();
        }
        // Although cancellation is not part of this example, we recommend this pattern for cases that do.
        catch (const task_canceled&)
        {
            // Your app might show a message to the user, or handle the error in some other way.
        }

        SPDLOG_INFO("Done");
    }).wait();


	return 0;
}

bool Main::ParseArguments(int argc, const char** argv)
{
	std::string action;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce a help message")
		("version,V", "output the version number")
		("verbose,v", "produce verbose output")
		("xmls,X",
			po::value< std::vector<std::string> >(&m_xmls)->multitoken(),
			"XML files");

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

	if (vm.count("help")) {
		SPDLOG_INFO(desc);
		return false;
	}

	if (vm.count("version")) {
		DumpVersion();
		return false;
	}

	po::notify(vm);

	if (vm.count("verbose"))
		m_verbose = true;

	if (!vm.count("xmls")) {
		SPDLOG_ERROR("No xml files given");
		return false;
	}

	return true;
}

task<void> Main::ParseXMLFiles()
{


	std::vector<task<void>> tasks;
	tasks.reserve(m_xmls.size());

	for (auto& xmlFile : m_xmls)
	{
		tasks.emplace_back(create_task([&]() -> boost::property_tree::ptree
			{
				SPDLOG_INFO("Parsing {}", xmlFile);
			
				boost::property_tree::ptree tree;
				read_xml(xmlFile, tree);
				return tree;
			}).then([](boost::property_tree::ptree result)
				{

					for (auto& vt : result.get_child("ScriptFile"))
					{
						if (vt.first == "CreateEterPack") {

							std::vector<task<void>> subtasks;

							
							const auto path = vt.second.get<std::string>("<xmlattr>.ArchivePath");
							SPDLOG_INFO("Creating {}", path);

							PakWriter write;

							if (write.Create(path)) {
								for (auto& elem : vt.second) {
									if (elem.first != "File") {
										continue;
									}
									
									const PakFilename archivedPath(elem.second.get<std::string>("<xmlattr>.ArchivedPath"));

									write.Add(archivedPath, elem.second.data(), kFileFlagLz4);
								}
							}

							write.Save();
						}
						else if (vt.first == "CreateEterPackXml") {
							SPDLOG_INFO("Creating make xml");

							const auto input = vt.second.get<std::string>("<xmlattr>.Input");
							const auto separator = input.find(':');
							std::string source, prefix;
							if (separator != std::string::npos) {
								source = input.substr(0, separator);
								prefix = input.substr(separator + 1, std::string::npos);
							}
							else {
								source = input;
							}

							std::vector<std::string> ignores;
							ignores.reserve(vt.second.size());
							std::vector<std::string> adds;
							adds.reserve(vt.second.size());
							std::vector<std::pair<std::string, std::string>> patches;
							patches.reserve(vt.second.size());

							for (auto& j : vt.second) {
								if (j.first == "Ignore") {
									ignores.push_back(j.second.get<std::string>("<xmlattr>.Pattern")); // <Ignore Pattern="[a-zA-Z0-9]+.png" />
								}
								else if (j.first == "Add") {
									adds.push_back(j.second.get<std::string>("<xmlattr>.Pattern")); // <Add><![CDATA[Path]]></Add>
								}
								else if (j.first == "Patch") {
									patches.push_back(std::make_pair<std::string, std::string>(j.second.get<std::string>("<xmlattr>.Search"),
										j.second.get<std::string>("<xmlattr>.Replace"))); // <Add><![CDATA[Path]]></Add>
								}
							}

							XmlGenerator gen;
							gen(vt.second.get<std::string>("<xmlattr>.XmlPath"),
								source,
								prefix,
								vt.second.get<std::string>("<xmlattr>.ArchivePath"),
								ignores,
								patches,
								adds);
						}
					}

				}));

	}
	
	return when_all(begin(tasks), end(tasks)).then([tasks](task<void> previousTask)
    {
        task_status status = completed;
		try {
			status = previousTask.wait();
		} catch (const std::exception& ex) {
			SPDLOG_INFO(ex.what());
		}

        // TODO: If other exception types might happen, add catch handlers here.

        // Ensure that we observe all exceptions.
        observe_all_exceptions<void>(begin(tasks), end(tasks));

        // Cancel any continuations that occur after this task if any previous task was canceled.
        // Although cancellation is not part of this example, we recommend this pattern for cases that do.
        if (status == canceled)
        {
            cancel_current_task();
        }
    });

}


METIN2_IMPLEMENT_SIMPLEAPP_MAIN(Main)

METIN2_END_NS
