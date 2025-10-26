#ifndef METIN2_CONFIG_HPP
#define METIN2_CONFIG_HPP

#include <storm/Config.hpp>

#include <fmt/format.h>
#include <boost/system/error_code.hpp>

template <>
struct fmt::formatter<boost::system::error_code> {
  // Presentation format: 'f' - fixed, 'e' - exponential.
  char presentation = 's';

  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    // [ctx.begin(), ctx.end()) is a character range that contains a part of
    // the format string starting from the format specifications to be parsed,
    // e.g. in
    //
    //   fmt::format("{:f} - point of interest", point{1, 2});
    //
    // the range will contain "f} - point of interest". The formatter should
    // parse specifiers until '}' or the end of the range. In this example
    // the formatter should parse the 'f' specifier and return an iterator
    // pointing to '}'.

    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 's')) presentation = *it++;

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }


  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const boost::system::error_code& p, FormatContext& ctx) {
      // ctx.out() is an output iterator to write to.
    return format_to(
        ctx.out(),
			"({}:{})",
        p.category().name(), p.message());
  }
};

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#define METIN2_NS 
#define METIN2_BEGIN_NS 
#define METIN2_END_NS 

#include <Namespaces.hpp>
#include <Services.hpp>

METIN2_BEGIN_NS

using boost::int8_t;
using boost::int_least8_t;
using boost::int_fast8_t;
using boost::uint8_t;
using boost::uint_least8_t;
using boost::uint_fast8_t;

using boost::int16_t;
using boost::int_least16_t;
using boost::int_fast16_t;
using boost::uint16_t;
using boost::uint_least16_t;
using boost::uint_fast16_t;

using boost::int32_t;
using boost::int_least32_t;
using boost::int_fast32_t;
using boost::uint32_t;
using boost::uint_least32_t;
using boost::uint_fast32_t;

#ifndef BOOST_NO_INT64_T

using boost::int64_t;
using boost::int_least64_t;
using boost::int_fast64_t;
using boost::uint64_t;
using boost::uint_least64_t;
using boost::uint_fast64_t;

#endif

using boost::intmax_t;
using boost::uintmax_t;

METIN2_END_NS

#if VSTD_PLATFORM_WINDOWS
	#include <Win32Defines.hpp>
#endif

#endif
