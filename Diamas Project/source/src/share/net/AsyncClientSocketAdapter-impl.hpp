#ifndef METIN2_NET_AsyncClientSocketAdapter_IMPL_HPP
#define METIN2_NET_AsyncClientSocketAdapter_IMPL_HPP

#include <boost/asio/connect.hpp>

#include <utility>



template <class T>
template <class... Args>
AsyncClientSocketAdapter<T>::AsyncClientSocketAdapter(boost::asio::io_service& ioService,
                                            Args&&... args)
	: T(typename T::SocketType(ioService),
	    std::forward<Args>(args)...)
	, m_resolver(ioService)
	, m_reconnect(false)
{
	// ctor
}

template <class T>
bool AsyncClientSocketAdapter<T>::Connect(const std::string& host,
                                     const std::string& service)
{
	m_host = host;
	m_service = service;

	if (!this->IsClosed())
		Disconnect();

	if (!m_reconnect)
		Reconnect();

	return true;
}

template <class T>
/*virtual*/ void AsyncClientSocketAdapter<T>::Disconnect()
{
	T::Disconnect();

	if (m_reconnect)
		Reconnect();
}

template <class T>
void AsyncClientSocketAdapter<T>::Reconnect()
{
	auto self(this->shared_from_this());

	m_resolver.async_resolve(boost::asio::ip::tcp::v4(), m_host, m_service, boost::asio::ip::resolver_base::numeric_host | boost::asio::ip::resolver_base::numeric_service,
	                         [this, self] (const boost::system::error_code& ec,
	                                       typename Resolver::iterator begin) {
		HandleResolve(ec, begin);
	});
}

template <class T>
void AsyncClientSocketAdapter<T>::HandleResolve(const boost::system::error_code& ec,
                                           typename Resolver::iterator begin)
{
	if (ec) {
		OnConnectFailure(ec);
		return;
	}

	auto self(this->shared_from_this());
	boost::asio::async_connect(this->m_socket, begin,
	                    [this, self] (const boost::system::error_code& ec,
	                                  typename Resolver::iterator it) {
		HandleConnect(ec, it);
	});
}

template <class T>
void AsyncClientSocketAdapter<T>::HandleConnect(const boost::system::error_code& ec,
                                           typename Resolver::iterator it)
{
	if (ec) {
		OnConnectFailure(ec);
		return;
	}

	//
	// Reset the socket buffers to a clean state.
	// This is necessary because we don't know what happened
	// to this socket before reconnecting.
	//

	// TODO(tim): remove this
	this->ResetBuffers();

	OnConnectSuccess();
}



#endif
