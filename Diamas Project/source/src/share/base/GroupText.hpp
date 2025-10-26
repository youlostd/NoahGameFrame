#ifndef METIN2_BASE_GROUPTEXT_HPP
#define METIN2_BASE_GROUPTEXT_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/memory/Allocator.hpp>
#include <storm/io/TextFileLoader.hpp>
#include <storm/StringUtil.hpp>
#include <storm/Tokenizer.hpp>
#include <storm/String.hpp>

#include <boost/spirit/home/x3.hpp>
#include <storm/io/FileUtil.hpp>

namespace x3 = boost::spirit::x3;


VSTD_SCOPED_ENUM_BEGIN(GroupTextScope)
{
	/// Scope is a group.
	kGroup,

	/// Scope is a list.
	kList
};
VSTD_SCOPED_ENUM_END(GroupTextScope)

template <class Handler>
class GroupTextParser
{
	public:
		~GroupTextParser();

		bool Parse(storm::StringRef input);

	protected:
		GroupTextParser();

		uint32_t m_depth;
		uint32_t m_currentLine;
		bool m_expectParens;
		bool m_errorOccurred;
		GroupTextScope m_currentScopeType;

	private:
		Handler& GetHandler();
		void ParseLine(storm::StringRef s);
};

template <class Handler>
GroupTextParser<Handler>::~GroupTextParser()
{
	// TODO(tim): Re-add scope check.
}

template <class Handler>
bool GroupTextParser<Handler>::Parse(storm::StringRef input)
{
    if(input.empty())
        return true;

    std::vector<std::string> lines;

    auto write = [&lines](auto const &ctx) {
        lines.emplace_back(std::string_view(&(x3::_attr(ctx))[0], x3::_attr(ctx).size()));
    };
    const auto skip = x3::char_("\r\n");
    if (!x3::parse(input.begin(), input.end(), x3::raw[+~skip][write] % +skip))
        SPDLOG_WARN("x3 Failed to parse the input string");

    for (const auto &line : lines)
    {
        ++m_currentLine;
        ParseLine(line);
    }

    return !m_errorOccurred;
}

template <class Handler>
GroupTextParser<Handler>::GroupTextParser()
	: m_depth(0)
	, m_currentLine(0)
	, m_expectParens(false)
	, m_errorOccurred(false)
	, m_currentScopeType(GroupTextScope::kGroup)
{
	// ctor
}

template <class Handler>
Handler& GroupTextParser<Handler>::GetHandler()
{ return static_cast<Handler&>(*this); }

template <class Handler>
void GroupTextParser<Handler>::ParseLine(storm::StringRef s)
{
	storm::TrimAndAssign(s);

	// Filter empty lines and comments early on.
	if (s.empty() || s.front() == '#')
		return;

	if (s.front() == '{') {
		if (m_expectParens) {
			++m_depth;
			m_expectParens = false;
		} else {
			GetHandler().OnError("unexpected '{'");
		}

		return;
	} else if (s.front() == '}') {
		if (m_depth != 0) {
			--m_depth;

			if (m_currentScopeType == GroupTextScope::kList)
				m_currentScopeType = GroupTextScope::kGroup;

			GetHandler().OnScopeClose();
		} else {
			GetHandler().OnError("unexpected '}'");
		}

		return;
	}

	// Did we expect a '{' and didn't get one?
	if (m_expectParens) {
		GetHandler().OnError("expected '{'");
		return;
	}

	//
	// List handling is fairly easy.
	// We just let our child-class do the parsing...
	//

	if (m_currentScopeType == GroupTextScope::kList) {
		GetHandler().OnListItems(s);
		return;
	}

	storm::ArgumentExtractor ex(s);
	if (!ex.Next()) {
		GetHandler().OnError("Failed to extract first token");
		return;
	}

	const auto key = ex.GetCurrent();
	if (key == "Group") {
		if (!ex.Next()) {
			GetHandler().OnError("Failed to extract group name");
			return;
		}

		GetHandler().OnScopeOpen(GroupTextScope::kGroup,
		                         ex.GetCurrent());

		m_expectParens = true;
	} else if (key == "List") {
		if (!ex.Next()) {
			GetHandler().OnError("Failed to extract list name");
			return;
		}

		GetHandler().OnScopeOpen(GroupTextScope::kList,
		                         ex.GetCurrent());

		m_expectParens = true;
		m_currentScopeType = GroupTextScope::kList;
	} else {
		GetHandler().OnKeyValuePair(key, ex);
	}
}

template <class Parser>
bool LoadGroupTextFile(const storm::Allocator& allocator,
                       const storm::StringRef& filename,
                       Parser& parser)
{
        std::string s;
  	bsys::error_code ec;
        storm::ReadFileToString(filename, s, ec);

	if (!ec) {
		return parser.Parse(s);
	} else {
		spdlog::error("Failed to load '{0}' with '{1}'", filename, ec);
		return false;
	}
}



#endif
