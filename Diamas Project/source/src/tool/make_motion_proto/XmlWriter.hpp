#ifndef METIN2_TOOL_MAKEMOTIONPROTO_PROTOWRITER_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_PROTOWRITER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Race.hpp"

#include <storm/String.hpp>

#include <list>

METIN2_BEGIN_NS

bool WriteMotionProto(const storm::StringRef& filename,
                      const std::list<Race>& races);

METIN2_END_NS

#endif
