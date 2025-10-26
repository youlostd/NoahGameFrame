#ifndef METIN2_NET_SOCKET_HPP
#define METIN2_NET_SOCKET_HPP

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SocketBase.hpp"
#include "Type.hpp"

#include <base/Serialization.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <net/Util.hpp>


class Socket : public SocketBase<boost::asio::ip::tcp::socket>
{
	public:
		Socket(boost::asio::ip::tcp::socket socket);

		virtual ~Socket();

		template <class T>
		void Send(PacketId id, const T& data)
		{
			static_assert(!std::is_pointer<T>(), "T should not be a pointer");
			using Base = SocketBase<boost::asio::ip::tcp::socket>;

			PacketHeader hdr{};
			hdr.id = id;
			hdr.size = GetObjectSize(data);

			if (!VerifySend(sizeof(hdr) + hdr.size))
				return;

			auto bufferPtr = Base::Send(sizeof(hdr) + hdr.size);
			if(!bufferPtr)
				return;

			boost::asio::mutable_buffer buffer(bufferPtr,
			                            sizeof(hdr) + hdr.size);

			boost::asio::buffer_copy(buffer, boost::asio::buffer(&hdr, sizeof(hdr)));
			buffer = buffer + sizeof(hdr);

			Write(buffer, data);
		}

	protected:
		virtual uint32_t ProcessData(const boost::asio::const_buffer& data);

		virtual bool HandlePacket(const PacketHeader& header,
		                          const boost::asio::const_buffer& data) = 0;
};



#endif
