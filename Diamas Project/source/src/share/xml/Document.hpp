#ifndef METIN2_XML_DOCUMENT_HPP
#define METIN2_XML_DOCUMENT_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <xml/Types.hpp>

#include <string_view>
#include <rapidxml.hpp>

#include <vector>

/// @file Types.hpp
/// Defines basic XML types.
///
/// @{



class XmlDocument : public rapidxml::xml_document<char>
{
	public:
		struct LoadResult
		{
			bool failed;
			const char* message;
			std::ptrdiff_t failureOffset;

			operator bool() const { return !failed; }
		};

		LoadResult LoadBuffer(char* buffer, bool keepComments);
		LoadResult LoadFile(const std::string_view& path,
		                    bool keepComments = false);

		std::string_view GetLine(uint32_t line) const;
		uint32_t GetLineFromOffset(std::ptrdiff_t offset) const;

		const std::string& GetContent() const
		{ return m_content; }

	private:
		void FillLineOffsets(const std::string_view& data);

		// Content owner (if loaded from file)
		std::string m_content;

		std::vector<std::ptrdiff_t> m_lineOffsets;
};

/// Log a human-readable error message
void LogError(const XmlDocument& doc,
              const XmlDocument::LoadResult& res,
              const std::string_view& filename);



#endif
