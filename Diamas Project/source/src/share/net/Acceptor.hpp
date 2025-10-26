#ifndef METIN2_NET_ACCEPTOR_HPP
#define METIN2_NET_ACCEPTOR_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/memory/Allocator.hpp>
#include <storm/memory/StdAllocator.hpp>
#include <storm/String.hpp>

#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>

#include <functional>



class Acceptor : private boost::noncopyable
{
	public:
		Acceptor(boost::asio::io_service& ioService);

		/// Bind the acceptor to a specific port/address.
		///
		/// This function binds the acceptor to the given
		/// port on the given address.
		///
		/// @param address The target network address.
		///
		/// @param port The target port.
		///
		/// @param handler The accept handler function, which
		/// needs to be compatible with the following signature:
		///
		/// 	void HandleAccept(asio::ip::tcp::socket&& socket);
		///
		/// @param ec The error_code object.
		///
		/// @param backlog Number of un-accept()ed connections the
		/// underlying system should buffer.
		template <typename Handler>
		void Bind(std::string_view address,
		          std::string_view port,
		          boost::system::error_code& ec,
		          const Handler& handler,
		          int backlog = boost::asio::socket_base::max_connections);

		/// Closes the acceptor.
		///
		/// This function closes the acceptor and
		void Close();

	private:
		typedef boost::asio::ip::tcp Protocol;

		typedef std::function<void(Protocol::socket&&)> HandlerType;

		void StartAccept();

		boost::asio::io_service& m_ioService;

		Protocol::acceptor m_acceptor;
		Protocol::socket m_socket;

		HandlerType m_handler;
};

template <typename Handler>
void Acceptor::Bind(std::string_view address,
                    std::string_view port,
                    boost::system::error_code& ec,
                    const Handler& handler,
                    int backlog)
{
	Protocol::resolver resolver(m_ioService);

	// TODO(tim): This doesn't look good!
	std::string host(address.data(), address.length());
	std::string service(port.data(), port.length());
	Protocol::resolver::query query(host, service);

	Protocol::endpoint endpoint = *resolver.resolve(query, ec);
	if (ec) return;

	m_acceptor.open(endpoint.protocol(), ec);

	if (ec) return;

	Protocol::acceptor::reuse_address opt(true);
	m_acceptor.set_option(opt, ec);

	if (ec) return;

	
	Protocol::acceptor::linger lopt(false, 0);
	m_acceptor.set_option(lopt, ec);

	if (ec) return;

	m_acceptor.non_blocking(true, ec);

	if (ec) return;

	m_acceptor.bind(endpoint, ec);
	if (ec) return;

	m_acceptor.listen(backlog, ec);
	if (ec) return;

	m_handler = handler;
	StartAccept();
}



#endif
