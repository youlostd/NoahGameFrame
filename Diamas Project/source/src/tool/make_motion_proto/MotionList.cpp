#include "MotionList.hpp"

#include <game/MotionTypes.hpp>

#include <storm/io/TextFileLoader.hpp>
#include <storm/Tokenizer.hpp>
#include <storm/StringUtil.hpp>
#include <SpdLog.hpp>


bool MotionList::Load(const storm::StringRef& filename)
{
	storm::TextFileLoader loader(storm::GetDefaultAllocator());

	bsys::error_code ec;
	loader.Load(filename, ec);

	if (ec) {
		SPDLOG_ERROR("Failed to load motion-list '{0}' with '{1}'",
		          filename, ec.message());
		return false;
	}


	std::vector<std::string> lines;
	storm::Tokenize(std::string_view(loader.GetText()),
	                "\r\n",
	                lines);

	std::vector<storm::StringRef> args;

	int i = 0;
	for (const auto& line : lines) {

		args.clear();
		storm::ExtractArguments(line, args);

		// Simply ignore empty lines...
		if (args.empty())
			continue;

		if (args.size() != 4) {
			SPDLOG_WARN(
			          "Motion list '{0}' line {1} has only {2} token(s)",
			          filename, i, args.size());
			continue;
		}

		uint32_t mode;
		if (!GetMotionModeValue(args[0], mode)) {
			SPDLOG_WARN(
			          "Motion list '{0}' line {1} has invalid mode {2}",
			          filename, i, args[0]);
			continue;
		}

		uint32_t index;
		if (!GetMotionValue(args[1], index)) {
			SPDLOG_WARN(
			          "Motion list '{0}' line {1} has invalid index {2}",
			          filename, i, args[1]);
			continue;
		}

		const auto key = MakeMotionKey(mode, index);
		m_motionFiles[key].push_back(std::string(args[2]));
	}

	return true;
}

const std::vector<storm::String>* MotionList::GetMotions(uint32_t key) const
{
	// This function retrieves all sub-motions.
	STORM_ASSERT(MakeMotionId(key).subIndex == 0, "No sub indexing allowed");

	const auto it = m_motionFiles.find(key);
	if (it != m_motionFiles.end())
		return &it->second;

	return nullptr;
}

const storm::String* MotionList::GetMotion(uint32_t key) const
{
	const auto id = MakeMotionId(key);

	const auto& mot = GetMotions(MakeMotionKey(id.mode, id.index));
	if (mot && id.subIndex < mot->size())
		return &(*mot)[id.subIndex];

	return nullptr;
}

