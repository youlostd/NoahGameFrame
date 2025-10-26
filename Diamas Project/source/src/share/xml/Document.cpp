#include "Document.hpp"

#include <storm/io/File.hpp>
#include <SpdLog.hpp>


XmlDocument::LoadResult XmlDocument::LoadBuffer(char* buffer, bool keepComments)
{
	FillLineOffsets(buffer);

	try {
		if (keepComments)
			parse<rapidxml::parse_comment_nodes |
			      rapidxml::parse_trim_whitespace>(buffer);
		else
			parse<rapidxml::parse_trim_whitespace>(buffer);
	} catch (rapidxml::parse_error& e) {
		return {true, e.what(), e.where<char>() - buffer};
	}

	return {false, nullptr, 0};
}

XmlDocument::LoadResult XmlDocument::LoadFile(const std::string_view& filename,
                                              bool keepComments)
{
	boost::system::error_code ec;
	storm::File fp;

	fp.Open(filename, ec,
	        storm::AccessMode::kRead,
	        storm::CreationDisposition::kOpenExisting,
	        storm::ShareMode::kRead,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} with {1}", filename, ec);
		return {true, nullptr, 0};
	}

	const auto size = fp.GetSize();
	m_content.resize(size);

	fp.Read(&*m_content.begin(), size, ec);

	if (ec) {
		spdlog::error("Failed to read {0} bytes from {1} with {2}",
		          size, filename, ec);
		return {true, nullptr, 0};
	}

	return LoadBuffer(&*m_content.begin(), keepComments);
}

std::string_view XmlDocument::GetLine(uint32_t line) const
{
	if (line == 0)
		return std::string_view();

	// Remove the 1 we add in GetLineFromOffset()
	--line;

	const std::ptrdiff_t first = line == 0 ? 0 : 1 + m_lineOffsets[line - 1];
	const std::ptrdiff_t last = line == m_lineOffsets.size() ?
	                             m_content.size() : m_lineOffsets[line];

	return std::string_view(m_content.data() + first, last - first);
}

uint32_t XmlDocument::GetLineFromOffset(std::ptrdiff_t offset) const
{
	const auto it = std::lower_bound(m_lineOffsets.begin(),
	                                 m_lineOffsets.end(),
	                                 offset);
	return 1 + (it - m_lineOffsets.begin());
}

void XmlDocument::FillLineOffsets(const std::string_view& data)
{
	for (std::size_t i = 0, len = data.size(); i != len; ++i) {
		if (data[i] == '\n')
			m_lineOffsets.push_back(i);
	}
}

void LogError(const XmlDocument& doc,
              const XmlDocument::LoadResult& res,
              const std::string_view& filename)
{
	if (!res.message) {
		spdlog::error("Unknown error while parsing: {0}", filename);
		return;
	}

	uint32_t line = 0;
	if (res.failureOffset != 0)
		line = doc.GetLineFromOffset(res.failureOffset);

	spdlog::error("{0}:{1}: {2}", filename, line, res.message);

	const auto lineContent = doc.GetLine(line);
	if (!lineContent.empty()) {
		SPDLOG_INFO("{0}", lineContent);

		// Print a point marker
		const auto lineOffset = lineContent.data() - doc.GetContent().data();
		const auto markerOffset = res.failureOffset - lineOffset;

		std::string pointer;
		pointer.reserve(markerOffset);

		if (markerOffset != 0)
			pointer.append(markerOffset - 1, ' ');

		pointer += '^';

		SPDLOG_INFO(pointer);
	}
}

