#ifndef METIN2_NET_AsyncClientSocketAdapter_HPP
#define METIN2_NET_AsyncClientSocketAdapter_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/SocketBase.hpp>

#include <storm/memory/Allocator.hpp>

#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>


template <class T>
class AsyncClientSocketAdapter : public T
{
	public:
		template <class... Args>
		AsyncClientSocketAdapter(boost::asio::io_service& ioService,
		                    Args&&... args);

		bool Connect(const std::string& host,
		             const std::string& service);

		virtual void Disconnect();

		BOOST_FORCEINLINE bool DoReconnect() const
		{ return m_reconnect; }

		BOOST_FORCEINLINE void SetDoReconnect(bool doReconnect)
		{ m_reconnect = doReconnect; }

		//
		// Event callbacks
		//

		virtual void OnConnectSuccess() = 0;
		virtual void OnConnectFailure(const boost::system::error_code& ec) = 0;

	protected:
		typedef typename T::SocketType::protocol_type::resolver Resolver;
		typedef typename T::SocketType::protocol_type::endpoint Endpoint;
		void Reconnect();

		void HandleResolve(const boost::system::error_code& ec,
		                   typename Resolver::iterator begin);

		void HandleConnect(const boost::system::error_code& ec,
		                   typename Resolver::iterator it);

		// boost forces us to use std::string
		std::string m_host;
		std::string m_service;

		Resolver m_resolver;

		bool m_reconnect;
};


#include "AsyncClientSocketAdapter-impl.hpp"

#endif
