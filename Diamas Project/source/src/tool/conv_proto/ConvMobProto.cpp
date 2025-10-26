#include "NpcList.hpp"
#include <game/ProtoReader.hpp>

#include <game/MobTypes.hpp>
#include <game/ProtoXml.hpp>

#include <base/SimpleApp.hpp>
#include <base/DumpVersion.hpp>


#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <vector>
#include <iostream>
#include "spdlog/spdlog.h"

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

		bool m_verbose;

		std::string m_inputPath;
		std::string m_outputPath;
		std::string m_npcListPath;
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
		spdlog::error("args: {0}", e.what());
		return 1;
	}

	NpcList npcList;
	if (!npcList.Load(m_npcListPath.c_str()))
		return 1;

	std::vector<old::TMobTable> mobs;
	if (!LoadMobProto(m_inputPath, mobs))
		return 1;

	if (m_verbose) {
		SPDLOG_INFO("Loaded {0} mobs from {1}",
		          mobs.size(), m_inputPath);
	}

	std::vector<MobProto> newMobs;
	newMobs.reserve(mobs.size());

	for (auto it = mobs.begin(), end = mobs.end(); it != end; ++it) {
		MobProto entry = {};
		if (ConvertMobProto(entry, *it, npcList))
			newMobs.push_back(entry);
	}

	return WriteProtoXml(m_outputPath.c_str(), newMobs, "mob-proto") ? 0 : 1;
}

bool Main::ParseArguments(int argc, const char** argv)
{
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help,h", "produce a help message")
	    ("version,V", "output the version number")
	    ("verbose,v", "be verbose")
	    ("input", po::value<std::string>(&m_inputPath)->required(),
	     "input mob_proto.txt")
	    ("output", po::value<std::string>(&m_outputPath)->required(),
	     "output mob_proto.xml")
	    ("npc-list", po::value<std::string>(&m_npcListPath)->required(),
	     "npclist.csv path")
	;

	po::positional_options_description p;
	p.add("input", 1);
	p.add("output", 1);
	p.add("npc-list", 1);

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

	m_verbose = vm.count("verbose");
	return true;
}

}

METIN2_IMPLEMENT_SIMPLEAPP_MAIN(Main)

METIN2_END_NS
