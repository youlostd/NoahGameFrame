#include "Peer.h"
#include "ItemIDRangeManager.h"
#include "ClientManager.h"


CPeer::CPeer(asio::ip::tcp::socket socket, uint32_t handle)
	: SocketBase(std::move(socket),
	             32 * 1024, 32 * 1024)
	, m_handle(handle)
	, m_channel(0)
	, m_gamePort(0)
	, m_itemRange(kNullRange)
	, m_itemSpareRange(kNullRange)
{
	// ctor
}

void CPeer::Setup()
{
	// This implicitly calls shared_from_this() which cannot be done in
	// the constructor.
	StartReadSome();
}

void CPeer::Disconnect()
{
	SPDLOG_INFO("Connection {0} closed.", m_socket);

	CClientManager::instance().RemovePeer(this);

	if (m_itemRange != kNullRange) {
		SPDLOG_INFO("ItemIDRange: returned. {0} ~ {1}",
		          m_itemRange.min, m_itemRange.max);

		CItemIDRangeManager::instance().ReturnRange(m_itemRange);
		m_itemRange = kNullRange;
	}

	if (m_itemSpareRange != kNullRange) {
		SPDLOG_INFO("ItemIDRange: returned. {0} ~ {1}",
		          m_itemSpareRange.min, m_itemSpareRange.max);

		CItemIDRangeManager::instance().ReturnRange(m_itemSpareRange);
		m_itemSpareRange = kNullRange;
	}

	SocketBase::Disconnect();
}

bool CPeer::SetItemIDRange(const ItemIdRange& itemRange)
{
	if (itemRange == kNullRange)
		return false;

	m_itemRange = itemRange;
	SPDLOG_INFO("ItemIDRange: SET {0} {1} ~ {2} start: {3}",
	          m_gameIp,
	          m_itemRange.min, m_itemRange.max,
	          m_itemRange.start);
	return true;
}

bool CPeer::SetSpareItemIDRange(const ItemIdRange& itemRange)
{
	if (itemRange == kNullRange)
		return false;

	m_itemSpareRange = itemRange;
	SPDLOG_INFO("ItemIDRange: SPARE SET {0} {1} ~ {2} start: {3}",
	          m_gameIp,
	          m_itemSpareRange.min, m_itemSpareRange.max,
	          m_itemSpareRange.start);
	return true;
}

bool CPeer::CheckItemIDRangeCollision(const ItemIdRange& itemRange)
{
	if (DoRangesCollide(m_itemRange, itemRange)) {
		spdlog::error(
		          "ItemIDRange: Collision!! this {0} ~ {1} check {2} ~ {3}",
		          m_itemRange.min, m_itemRange.max,
		          itemRange.min, itemRange.max);
		return true;
	}

	if (DoRangesCollide(m_itemSpareRange, itemRange)) {
		spdlog::error(
		          "ItemIDRange: Collision with spare range this {0} ~ {1} check {2} ~ {3}",
		          m_itemSpareRange.min, m_itemSpareRange.max,
		          itemRange.min, itemRange.max);
		return true;
	}

	return false;
}

void CPeer::SendSpareItemIDRange()
{
	if (!SetItemIDRange(m_itemSpareRange)) {
		EncodeHeader(HEADER_DG_ACK_SPARE_ITEM_ID_RANGE, 0, sizeof(m_itemSpareRange));
		Encode(&m_itemSpareRange, sizeof(m_itemSpareRange));
		return;
	}

	ItemIdRange range;
	if (!CItemIDRangeManager::instance().GetRange(range) ||
	    !SetSpareItemIDRange(range)) {
		SPDLOG_INFO("ItemIDRange: spare range set error");
		m_itemSpareRange = kNullRange;
	}

	EncodeHeader(HEADER_DG_ACK_SPARE_ITEM_ID_RANGE, 0, sizeof(m_itemSpareRange));
	Encode(&m_itemSpareRange, sizeof(m_itemSpareRange));
}

void CPeer::Encode(const void* data, uint32_t size)
{
	try {
		Send(data, size);
	} catch(const std::exception&e) {
		spdlog::critical("Exception occured on Encode {}", e.what());
	}
}

void CPeer::EncodeHeader(uint8_t id, uint32_t handle, uint32_t size)
{
	try {
		DbPacketHeader hdr = {id, handle, size};
		Send(&hdr, sizeof(hdr));
	} catch(const std::exception&e) {
		spdlog::critical("Exception occured on EncodeHeader {}", e.what());
	}

}

void CPeer::EncodeReturn(uint8_t id, uint32_t handle)
{
	DbPacketHeader hdr;
	hdr.id = id;
	hdr.handle = handle;
	hdr.size = 0;
	Send(&hdr, sizeof(hdr));
}

void CPeer::EncodeBYTE(uint8_t data)
{
	Send(&data, sizeof(data));
}

void CPeer::EncodeWORD(uint16_t data)
{
	Send(&data, sizeof(data));
}

void CPeer::EncodeDWORD(uint32_t data)
{
	Send(&data, sizeof(data));
}

/*virtual*/ uint32_t CPeer::ProcessData(const asio::const_buffer& data)
{
	STORM_ASSERT(m_socket.is_open(), "Disconnected");

	using asio::buffer_size;
	using asio::buffer_cast;

	asio::const_buffer cur(data);
	while (buffer_size(cur) >= sizeof(DbPacketHeader)) {
		auto p = buffer_cast<const DbPacketHeader*>(cur);

		if (buffer_size(cur) - sizeof(DbPacketHeader) < p->size)
			break;

		cur = cur + sizeof(DbPacketHeader);

		CClientManager::instance().ProcessPacket(this, p->id, p->handle,
		                                         buffer_cast<const char*>(cur),
		                                         p->size);

		// TODO(tim): Can ProcessPacket() DC the peer?
		if (!m_socket.is_open())
			break;

		cur = cur + p->size;
	}

	return buffer_size(data) - buffer_size(cur);
}
