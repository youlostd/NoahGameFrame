#ifndef METIN2_NAMESPACES_HPP
#define METIN2_NAMESPACES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

/// @file Namespaces.hpp
/// Defines various namespace aliases for commonly used third-party namespaces.

namespace boost { namespace system { } }
namespace boost { namespace asio { } }
namespace boost { namespace chrono { } }
namespace boost { namespace algorithm { } }
namespace boost { namespace locale {} }
namespace boost { namespace signals2 {} }
namespace boost { namespace program_options {} }

METIN2_BEGIN_NS

namespace bsys = boost::system;
namespace basio = boost::asio;
namespace bchrono = boost::chrono;
namespace balgo = boost::algorithm;
namespace blocale = boost::locale;

// Allows us to easily switch between different implementations
// We use spdlog and spdlog actually does this with chrono so
// basically dont do this for chrono here
namespace asio = boost::asio;
namespace signals = boost::signals2;
namespace po = boost::program_options;

METIN2_END_NS

#endif
