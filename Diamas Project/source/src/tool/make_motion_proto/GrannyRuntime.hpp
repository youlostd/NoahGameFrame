#ifndef METIN2_TOOL_MAKEMOTIONPROTO_GRANNYRUNTIME_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_GRANNYRUNTIME_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

void InitializeGrannyRuntime();
void DestroyGrannyRuntime();

struct GrannyRuntimeInitializer
{
	GrannyRuntimeInitializer();
	~GrannyRuntimeInitializer();
};

// TODO(tim): Move to *-impl.hpp
BOOST_FORCEINLINE GrannyRuntimeInitializer::GrannyRuntimeInitializer()
{
	InitializeGrannyRuntime();
}

BOOST_FORCEINLINE GrannyRuntimeInitializer::~GrannyRuntimeInitializer()
{
	DestroyGrannyRuntime();
}

METIN2_END_NS

#endif // METIN2_TOOL_MAKEMOTIONPROTO_GRANNYRUNTIME_HPP
