#include <game/DbPackets.hpp>

#include "DbCacheSocket.hpp"
#include "guild_manager.h"
#include "GBufferManager.h"
#include "party.h"
#include "desc_manager.h"

#include <net/Util.hpp>

#include <storm/StringUtil.hpp>

#include "input.h"

std::shared_ptr<DbCacheSocket> db_clientdesc;

DbCacheSocket::DbCacheSocket(asio::io_service &ioService)
    : ClientSocketAdapter(ioService,
                          32 * 1024, 32 * 1024)
{
    // ctor
}

void DbCacheSocket::Disconnect()
{
    SPDLOG_INFO("DbCacheSocket Connection {0} closed.", m_socket);

    CPartyManager::instance().DeleteAllParty();
    CPartyManager::instance().DisablePCParty();
    CGuildManager::instance().StopAllGuildWar();

    ClientSocketAdapter::Disconnect();
}

void DbCacheSocket::DBPacketHeader(uint8_t id, uint32_t handle, uint32_t size)
{
    DbPacketHeader hdr;
    hdr.id = id;
    hdr.handle = handle;
    hdr.size = size;
    Send(&hdr, sizeof(hdr));
}

void DbCacheSocket::DBPacket(uint8_t id, uint32_t handle,
                             const void *data, uint32_t size)
{
    DBPacketHeader(id, handle, size);
    Send(data, size);
}

void DbCacheSocket::Packet(const void *data, uint32_t size) { Send(data, size); }

/*virtual*/
void DbCacheSocket::OnConnectSuccess()
{
    SPDLOG_INFO("Successfully connected {0} to {1}:{2}", m_socket, m_host, m_service);

    static bool bSentBoot = false;
    if (!bSentBoot)
    {
        TPacketGDBoot p;
        p.dwItemIDRange[0] = 0;
        p.dwItemIDRange[1] = 0;
        strlcpy(p.szIP, gConfig.gameIp.c_str(), sizeof(p.szIP));
        DBPacket(HEADER_GD_BOOT, 0, &p, sizeof(p));
        bSentBoot = true;
    }

    TEMP_BUFFER buf;

    TPacketGDSetup p{};

    strlcpy(p.gameIp, gConfig.gameIp.c_str(), sizeof(p.gameIp));
    storm::ParseNumber(gConfig.gamePort.c_str(), p.gamePort);

    p.bChannel = gConfig.channel;
    p.bAuthServer = false;
    buf.write(&p, sizeof(p));

    // 파티를 처리할 수 있게 됨.
    CPartyManager::instance().EnablePCParty();
    //CPartyManager::instance().SendPartyToDB();

    DBPacket(HEADER_GD_SETUP, 0, buf.read_peek(), buf.size());

    StartReadSome();
}

/*virtual*/
void DbCacheSocket::OnConnectFailure(const boost::system::error_code &ec)
{
    if (ec == asio::error::operation_aborted)
        return;

    SPDLOG_ERROR("Failed to connect to {0}:{1} with {2}",
                 m_host, m_service, ec);

    Reconnect();
}

/*virtual*/
uint32_t DbCacheSocket::ProcessData(const asio::const_buffer &data)
{
    STORM_ASSERT(m_socket.is_open(), "Disconnected");

    using asio::buffer_size;
    using asio::buffer_cast;

    asio::const_buffer cur(data);
    while (buffer_size(cur) >= sizeof(DbPacketHeader))
    {
        auto *p = buffer_cast<const DbPacketHeader *>(cur);

        if (buffer_size(cur) - sizeof(DbPacketHeader) < p->size)
            break;

        cur = cur + sizeof(DbPacketHeader);

        HandleDbPacket(*p, buffer_cast<const char *>(cur));

        if (!m_socket.is_open())
            break;

        cur = cur + p->size;
    }

    return buffer_size(data) - buffer_size(cur);
}
