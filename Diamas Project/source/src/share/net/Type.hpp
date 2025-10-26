#ifndef METIN2_NET_TYPES_HPP
#define METIN2_NET_TYPES_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Serialization.hpp>
#include <storm/ErrorReporting.hpp>


typedef uint8_t PacketId;

#pragma pack(push, 1)

struct PacketHeader
{
	PacketId id;

	// Size in bytes - excluding header
	uint32_t size;
};

template <class T>
T ReadPacket(boost::asio::const_buffer data)
{
	const auto p = Read<T>(data);
	assert(boost::asio::buffer_size(p.second) == 0 &&
	                 "Packet not read completely");

	return p.first;
}

template <class C, class T>
void BroadcastPacket(const C& sockets,
                     PacketId id, const T& packet,
                     const typename C::value_type::element_type* except = nullptr)
{
	for (const auto& socket : sockets) {
		if (socket.get() == except)
			continue;

		socket->Send(id, packet);
	}
}

#pragma pack(pop)



#endif
