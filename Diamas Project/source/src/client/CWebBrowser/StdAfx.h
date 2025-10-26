
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_ERROR

#include <SpdLog.hpp>
#include <Direct3D.hpp>