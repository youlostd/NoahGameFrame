// vim: ts=8 sw=4

#ifndef METIN2_SERVER_DB_PEER_H
#define METIN2_SERVER_DB_PEER_H


#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <boost/asio/ip/tcp.hpp>
#include <net/SocketBase.hpp>
#include <game/Constants.hpp>
#include <game/ItemConstants.hpp>
#include <game/ItemTypes.hpp>

class CPeer : public SocketBase<asio::ip::tcp::socket>
{
	public:
		CPeer(asio::ip::tcp::socket socket, uint32_t handle);

		void Setup();

		virtual void Disconnect();

		uint32_t GetHandle() const { return m_handle; }


		const std::string& GetGameIp() const { return m_gameIp; }
		void SetGameIp(const std::string& ip) { m_gameIp =  ip; }

		uint16_t GetGamePort() const { return m_gamePort; }
		void SetGamePort(uint16_t port) { m_gamePort = port; }

		uint8_t GetChannel() const { return m_channel; }
		void SetChannel(uint8_t channel) { m_channel = channel; }

		bool IsAuth() const { return m_channel == 0; }

		bool SetItemIDRange(const ItemIdRange& itemRange);
		bool SetSpareItemIDRange(const ItemIdRange& itemRange);

		// TODO Move these somewhere else
		bool CheckItemIDRangeCollision(const ItemIdRange& itemRange);
		void SendSpareItemIDRange();

		void Encode(const void* data, uint32_t size);
		void EncodeHeader(uint8_t id, uint32_t handle, uint32_t size);
		void EncodeReturn(uint8_t id, uint32_t handle);
		void EncodeBYTE(uint8_t data);
		void EncodeWORD(uint16_t data);
		void EncodeDWORD(uint32_t data);

	protected:
		virtual uint32_t ProcessData(const asio::const_buffer& data);

	private:
		uint32_t m_handle;

		uint8_t m_channel;

		std::string m_gameIp;
		uint16_t m_gamePort;

		ItemIdRange m_itemRange;
		ItemIdRange m_itemSpareRange;
};


#endif /* METIN2_SERVER_DB_PEER_H */
