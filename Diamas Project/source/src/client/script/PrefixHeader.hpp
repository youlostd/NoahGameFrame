#ifndef METIN2_CLIENT_SCRIPT_PREFIXHEADER_HPP
#define METIN2_CLIENT_SCRIPT_PREFIXHEADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Python.h"

#include "longintrepr.h"

#include <stddef.h>
#include <stdint.h>

#if defined(WIN32) || defined(MS_WINDOWS)
  #define _USE_MATH_DEFINES
#endif

#include <math.h>

#if defined(WIN32) || defined(MS_WINDOWS)
  #undef _USE_MATH_DEFINES
#endif

#endif
