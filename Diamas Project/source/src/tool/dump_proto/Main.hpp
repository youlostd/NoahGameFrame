#ifndef METIN2_TOOL_DUMPPROTO_MAIN_HPP
#define METIN2_TOOL_DUMPPROTO_MAIN_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <string>

#include <optionparser.h>

METIN2_BEGIN_NS

typedef bool (*DumpProtoFunction)(const std::string& input,
                                  const std::string& output,
                                  bool verbose);

struct ActionRegistrator
{
	ActionRegistrator(const std::string& name, DumpProtoFunction fn);

	std::string name;
	DumpProtoFunction fn;
	ActionRegistrator* next;
};

METIN2_END_NS

#endif // METIN2_TOOL_DUMPPROTO_MAIN_HPP
