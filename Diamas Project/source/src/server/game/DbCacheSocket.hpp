#ifndef METIN2_SERVER_GAME_DBCACHESOCKET_HPP
#define METIN2_SERVER_GAME_DBCACHESOCKET_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/SocketBase.hpp>
#include <net/ClientSocketAdapter.hpp>
#include <net/Util.hpp>

#include <storm/String.hpp>

#include <boost/asio/ip/tcp.hpp>

namespace asio = boost::asio;

class DbCacheSocket
    : public ClientSocketAdapter<SocketBase<asio::ip::tcp::socket>>
{
public:
    DbCacheSocket(asio::io_service &ioService);

    virtual void Disconnect();

    void DBPacketHeader(uint8_t id, uint32_t handle, uint32_t size);
    void DBPacket(uint8_t id, uint32_t handle, const void *data, uint32_t size);
    void Packet(const void *data, uint32_t size);

protected:
    virtual void OnConnectSuccess();
    virtual void OnConnectFailure(const boost::system::error_code &ec);

    virtual uint32_t ProcessData(const asio::const_buffer &data);
};

// TODO(tim): Get rid of this
extern std::shared_ptr<DbCacheSocket> db_clientdesc;

#endif
