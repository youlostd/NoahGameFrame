#ifndef METIN2_SERVER_AUTH_DBCACHEPACKETS_HPP
#define METIN2_SERVER_AUTH_DBCACHEPACKETS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Constants.hpp>

#include <boost/asio/buffer.hpp>
namespace asio = boost::asio;


struct DbPacketHeader;
class DbCacheSocket;

void HandleDbPacket(DbCacheSocket* dbCache,
                    const DbPacketHeader& header,
                    const asio::const_buffer& data);



#endif
