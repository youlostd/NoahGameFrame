#include "ItemList.hpp"

#include <storm/io/TextFileLoader.hpp>
#include <storm/StringUtil.hpp>
#include <storm/Tokenizer.hpp>
#include <SpdLog.hpp>

METIN2_BEGIN_NS

ItemListEntry::ItemListEntry()
	: vnum(0)
{
	// ctor
}

ItemList::ItemList()
{
	// ctor
}

bool ItemList::Load(const storm::StringRef& filename)
{
	storm::TextFileLoader loader(storm::GetDefaultAllocator());

	bsys::error_code ec;
	loader.Load(filename, ec);

	if (ec) {
		spdlog::error("Failed to load '{0}' with '{1}'", filename, ec.message());
		return false;
	}

	const auto file = std::string(loader.GetText(), loader.GetSize());
	std::vector<std::string> lines;
	storm::Tokenize(file,
	                "\r\n",
	                lines);

	int i = 0;
	for (const auto& line : lines) {

		auto s = line;
		storm::TrimAndAssign(s);

		if (s.empty() || s.front() == '#')
			continue;

		if (!ParseLine(s)) {
			spdlog::error("Failed to parse line {0}", line);
			return false;
		}
	}

	return true;
}

const ItemListEntry* ItemList::Get(uint32_t vnum) const
{
	auto it = m_entries.find(vnum);
	if (it != m_entries.end())
		return &it->second;

	return nullptr;
}

bool ItemList::ParseLine(const storm::String& line)
{
	ItemListEntry entry;

	storm::Tokenizer tok(line);
	auto pred = storm::IsChar('\t');

	for (uint32_t i = 0; tok.Next(pred); ++i) {
		switch (i) {
			case 0:
				if (!storm::ParseNumber(tok.GetCurrent(), entry.vnum))
					entry.vnum = 0;
				break;

			case 1: break; // We're not interested in the "type"

			case 2:
				entry.iconFilename = tok.GetCurrent();
				break;

			case 3:
				entry.modelFilename = tok.GetCurrent();
				break;

			default:
				spdlog::error("Too many tokens {0}", i);
				return false;
		}
	}

	if (entry.vnum == 0) {
		spdlog::error("No vnum");
		return false;
	}

	m_entries[entry.vnum] = entry;
	return true;
}

METIN2_END_NS
