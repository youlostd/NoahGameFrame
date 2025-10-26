#ifndef METIN2_NET_CLIENTSOCKETADAPTER_IMPL_HPP
#define METIN2_NET_CLIENTSOCKETADAPTER_IMPL_HPP

#include <boost/asio/connect.hpp>
#include <charconv>
#include <utility>

template <class T>
template <class... Args>
ClientSocketAdapter<T>::ClientSocketAdapter(boost::asio::io_service& ioService,
                                            Args&&... args)
    : T(typename T::SocketType(ioService), std::forward<Args>(args)...)
    , m_resolver(ioService)
    , m_reconnect(false)
{
    boost::system::error_code ec;
    this->m_socket.non_blocking(true, ec);
    // ctor
}

template <class T>
bool ClientSocketAdapter<T>::Connect(const std::string& host,
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
/*virtual*/ void ClientSocketAdapter<T>::Disconnect()
{
    T::Disconnect();

    if (m_reconnect)
        Reconnect();
}

template <class T> void ClientSocketAdapter<T>::Reconnect()
{
    if (this->m_socket.is_open()) {
        T::Disconnect();
    }
    boost::system::error_code ec;

    auto self(this->shared_from_this());
    unsigned short port;
    std::from_chars(m_service.c_str(), m_service.c_str() + m_service.length(),
                    port);

    boost::asio::ip::address ip_address = boost::asio::ip::make_address(m_host, ec);
    Endpoint ep(ip_address, port);

    if (ec) {
        OnConnectFailure(ec);
        return;
    }
    this->m_socket.non_blocking(true, ec);
    this->m_socket.open(ep.protocol(), ec);
    this->m_socket.connect(ep, ec);

    if (ec) {
        OnConnectFailure(ec);
        return;
    }

    this->m_socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true));
    this->m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

    // TODO(tim): remove this
    this->ResetBuffers();

    OnConnectSuccess();
}

template <class T>
void ClientSocketAdapter<T>::HandleResolve(const boost::system::error_code& ec,
                                           typename Resolver::iterator begin)
{
    if (ec) {
        OnConnectFailure(ec);
        return;
    }
}

template <class T>
void ClientSocketAdapter<T>::HandleConnect(const boost::system::error_code& ec,
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

    this->m_socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true));
    this->m_socket.set_option(boost::asio::ip::tcp::no_delay(true));

    // TODO(tim): remove this
    this->ResetBuffers();

    OnConnectSuccess();
}

#endif
