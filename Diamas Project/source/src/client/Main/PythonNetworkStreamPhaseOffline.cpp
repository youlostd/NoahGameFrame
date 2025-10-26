#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include <game/GamePacket.hpp>

bool CPythonNetworkStream::OffLinePhase(const PacketHeader &header, const boost::asio::const_buffer &data)
{
    return true;
}
