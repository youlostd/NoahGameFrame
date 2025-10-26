#include "spdlog/spdlog.h"

#include "GrannyRuntime.hpp"
#include "MotionManager.hpp"
#include "XmlWriter.hpp"

#include <base/SimpleApp.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>



namespace
{

class Main : public SimpleApp
{
	public:
		Main();

		int Run(int argc, const char** argv);

	private:
		bool ParseArguments(int argc, const char** argv);


		GrannyRuntimeInitializer m_grannyInit;
		MotionManager m_motionManager;

		storm::String m_vfsConfig;
		storm::String m_npcList;
		storm::String m_output;
		bool m_verbose;
};

Main::Main()
	: m_verbose(false)
{
}

int Main::Run(int argc, const char** argv)
{
	try {
		if (!ParseArguments(argc, argv))
			return 1;
	} catch (std::exception& e) {
		SPDLOG_ERROR("args: {0}", e.what());
		return 1;
	}


	if (!m_motionManager.Load(m_npcList))
		return 2;

	m_motionManager.UpdateAll();

	if (!WriteMotionProto(m_output, m_motionManager.GetRaces()))
		return 3;

	return 0;
}

bool Main::ParseArguments(int argc, const char** argv)
{
	std::string action;

	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help,h", "produce a help message")
	    ("verbose,v", "produce verbose output")
	    ("npc-list,n", po::value<std::string>(&m_npcList)->required(),
	     "path of the mob_proto.xml file")
	    ("output,o", po::value<std::string>(&m_output)->required(),
	     "path of the output xml file")
	;

	po::positional_options_description p;
	p.add("npc-list", 1);
	p.add("output", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).
	          positional(p).run(), vm);

	if (vm.count("help")) {
		std::cout << desc;
		return false;
	}

	po::notify(vm);

	if (vm.count("verbose"))
		m_verbose = true;

	return true;
}

METIN2_IMPLEMENT_SIMPLEAPP_MAIN(Main)

}


