#ifndef METIN2_TOOL_FORMATPROTO_MAIN_HPP
#define METIN2_TOOL_FORMATPROTO_MAIN_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <string>
#include <storm/String.hpp>

#include <optionparser.h>

METIN2_BEGIN_NS

typedef bool (*FormatProtoFunction)(const std::string& input,
                                    const std::string& output,
                                    bool verbose);

struct ActionRegistrator
{
	ActionRegistrator(const std::string& name, FormatProtoFunction fn);

	std::string name;
	FormatProtoFunction fn;
	ActionRegistrator* next;
};

METIN2_END_NS

#endif
