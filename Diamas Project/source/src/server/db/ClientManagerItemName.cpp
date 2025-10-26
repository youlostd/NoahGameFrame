// vim:ts=4 sw=4
#include "ClientManager.h"

void CClientManager::UpdateItemName(TPacketUpdateItemName* data, CPeer* peer)
{
	std::unique_ptr<SQLMsg> pmsg_insert(CDBManager::instance().DirectQuery(fmt::format("REPLACE INTO item_name VALUES({}, '{}')", data->dwItemID, data->szName)));

	ForwardPacket(HEADER_DG_UPDATE_ITEM_NAME, data, sizeof(TPacketUpdateItemName), 0, peer);
}

void CClientManager::AckItemName(uint32_t dwItemID, CPeer* peer)
{
	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(fmt::format("SELECT name FROM item_name WHERE id = {}", dwItemID)));

	TPacketUpdateItemName packet;
	packet.dwItemID = dwItemID;

	if (pmsg->Get()->uiNumRows == 0)
	{
		memset(packet.szName, 0, sizeof (packet.szName));
	}
	else
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		strlcpy(packet.szName, row[0], sizeof(packet.szName));
	}

	peer->EncodeHeader(HEADER_DG_ACK_ITEM_NAME, 0, sizeof(TPacketUpdateItemName));
	peer->Encode(&packet, sizeof(TPacketUpdateItemName));
}

