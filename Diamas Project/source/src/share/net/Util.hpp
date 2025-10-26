#ifndef METIN2_NET_UTIL_HPP
#define METIN2_NET_UTIL_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <fmt/format.h>



namespace fmt {
template <>
struct formatter<boost::asio::basic_stream_socket<boost::asio::ip::tcp>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const boost::asio::basic_stream_socket<boost::asio::ip::tcp> &p, FormatContext &ctx) {
	  boost::system::error_code ec;
		return format_to(ctx.out(), "{} <> {}", p.local_endpoint(ec), p.remote_endpoint(ec));
  }
};

template <>
struct formatter<boost::asio::ip::basic_endpoint<boost::asio::ip::tcp>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> &p, FormatContext &ctx) {
	 boost::system::error_code ec;
	const auto addr = p.address();
	const auto addrStr = addr.to_string(ec);
	if (ec)
		return format_to(ctx.out(), "{}", ec.message());

	if (addr.is_v4()) {
		return format_to(ctx.out(), "{}:{}", addrStr, p.port());
	} else {
		return format_to(ctx.out(), "[{}]:{}", addrStr, p.port());
	}

  }
};
}






#endif
