#ifndef METIN2_SERVER_GAME_MASTERSOCKET_HPP
#define METIN2_SERVER_GAME_MASTERSOCKET_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <net/Socket.hpp>
#include <net/ClientSocketAdapter.hpp>

#include <boost/asio/ip/tcp.hpp>

struct PacketHeader;

class MasterSocket : public ClientSocketAdapter<Socket>
{
public:
    MasterSocket(asio::io_service &ioService);

    void Disconnect() override;

protected:
    void OnConnectSuccess() override;
    void OnConnectFailure(const boost::system::error_code &ec) override;

    virtual bool HandlePacket(const PacketHeader &header,
                              const asio::const_buffer &data) override final;
};

#endif
