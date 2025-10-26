#include <base/ResponseFile.hpp>

#include <storm/io/FileUtil.hpp>
#include <storm/StringUtil.hpp>
#include <storm/Tokenizer.hpp>
#include <vstl/string.hpp>



std::pair<std::string, std::string> AtOptionParser(const std::string& s)
{
	if ('@' == s[0])
		return std::make_pair("response-file", s.substr(1));
	else
		return std::pair<std::string, std::string>();
}

bool ParseResponseFile(const std::string_view& filename,
                       std::vector<std::string>& args)
{
	bsys::error_code ec;
	storm::String content;

	storm::ReadFileToString(filename, content, ec);
	if (ec) {
		spdlog::error("Failed to open {0} with {1}",
		          filename, ec);
		return false;
	}

	storm::Tokenizer tok(content);
	for (const auto pred = storm::IsChar('\n'); tok.Next(pred); ) {
		auto line(tok.GetCurrent());

		storm::TrimAndAssign(line);

		// Filter empty lines and comments early on.
		if (line.empty() || line.front() == '#')
			return false;

		args.emplace_back(line.data(), line.size());
	}

	return true;
}


