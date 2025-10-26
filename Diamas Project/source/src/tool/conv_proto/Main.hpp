#ifndef METIN2_TOOL_CONVPROTO_MAIN_HPP
#define METIN2_TOOL_CONVPROTO_MAIN_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <optionparser.h>

METIN2_BEGIN_NS

class ActionInterface
{
	public:
		virtual ~ActionInterface() {}
		virtual bool ParseArguments(option::Parser& parse) = 0;
		virtual bool Run(bool verbose) = 0;
};

struct ActionRegistrator
{
	typedef ActionInterface* (*CreateFunction)();

	ActionRegistrator(const storm::StringRef& name, CreateFunction fn);

	storm::StringRef name;
	CreateFunction fn;
	ActionRegistrator* next;
};

METIN2_END_NS

#endif // METIN2_TOOL_CONVPROTO_MAIN_HPP
