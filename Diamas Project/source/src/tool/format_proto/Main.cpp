#include "Main.hpp"

#include <base/SimpleApp.hpp>
#include <base/DumpVersion.hpp>


#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>

METIN2_BEGIN_NS

namespace
{

class Main : public SimpleApp
{
	public:
		Main();

		int Run(int argc, const char** argv);

	private:
		bool ParseArguments(int argc, const char** argv);

		ActionRegistrator* GetAction(const std::string& name);

		ActionRegistrator* m_action;
		std::string m_inputPath;
		std::string m_outputPath;
		bool m_verbose;
};

static ActionRegistrator* first = nullptr;

Main::Main()
	: m_action(nullptr)
	, m_verbose(false)
{
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

	if (m_action)
		return m_action->fn(m_inputPath, m_outputPath, m_verbose) ? 0 : 1;

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
	    ("action", po::value<std::string>(&action)->required(),
	     "action name")
	    ("input", po::value<std::string>(&m_inputPath)->required(),
	     "path of the input file")
	    ("output", po::value<std::string>(&m_outputPath)->required(),
	     "path of the output file")
	;

	po::positional_options_description p;
	p.add("action", 1);
	p.add("input", 1);
	p.add("output", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).
	          positional(p).run(), vm);

	if (vm.count("help")) {
		std::cout << desc;
		return false;
	}

	if (vm.count("version")) {
		DumpVersion();
		return false;
	}

	po::notify(vm);

	if (vm.count("verbose"))
		m_verbose = true;

	m_action = GetAction(action);
	if (!m_action) {
		spdlog::error("No such action: {0}", action);
		std::cout << desc;
		return false;
	}

	return true;
}

ActionRegistrator* Main::GetAction(const std::string& name)
{
	for (auto a = first; a; a = a->next) {
		if (a->name == name)
			return a;
	}

	return nullptr;
}

}

ActionRegistrator::ActionRegistrator(const std::string& name,
                                     FormatProtoFunction fn)
	: name(name)
	, fn(fn)
	, next(first)
{
	first = this;
}

METIN2_IMPLEMENT_SIMPLEAPP_MAIN(Main)

METIN2_END_NS
