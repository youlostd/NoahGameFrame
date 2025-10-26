#include "NpcList.hpp"

#include <storm/io/TextFileLoader.hpp>
#include <storm/StringUtil.hpp>
#include <storm/Tokenizer.hpp>
#include "spdlog/spdlog.h"

METIN2_BEGIN_NS

NpcList::NpcList()
{
	// ctor
}

bool NpcList::Load(const storm::StringRef& filename)
{
	storm::TextFileLoader loader(storm::GetDefaultAllocator());

	bsys::error_code ec;
	loader.Load(filename, ec);

	if (ec) {
		spdlog::error("Failed to load NPC list '{0}' with {1}",
		          filename, ec.message());
		return false;
	}

	const auto file = std::string(loader.GetText(), loader.GetSize());
	std::vector<std::string> lines;
	storm::Tokenize(file,
	                "\r\n",
	                lines);

	std::vector<storm::String> args;

	int i = 0;
	for (const auto& line : lines) {
		args.clear();
		storm::Tokenize(line,
		                "\t",
		                args);

		++i;

		if (args.empty())
			continue;

		if (args.size() != 2) {
			spdlog::error(
			          "NPC List '{0}' line {1} has only {2} tokens",
			          filename, i, args.size());
			return false;
		}

		uint32_t vnum;
		if (!storm::ParseNumber(args[0], vnum)) {
			spdlog::error(
			          "NPC List '{0}' line {1} vnum is invalid",
			          filename, i);
			return false;
		}

		m_entries[vnum] = args[1];
	}

	return true;
}

const std::string* NpcList::Get(uint32_t vnum) const
{
	auto it = m_entries.find(vnum);
	if (it != m_entries.end())
		return &it->second;

	return nullptr;
}

METIN2_END_NS
