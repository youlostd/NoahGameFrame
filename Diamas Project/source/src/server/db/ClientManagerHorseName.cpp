// vim:ts=4 sw=4
#include "ClientManager.h"

void CClientManager::UpdateHorseName(TPacketUpdateHorseName* data, CPeer* peer)
{
	std::unique_ptr<SQLMsg> pmsg_insert(CDBManager::instance().DirectQuery(fmt::format("REPLACE INTO horse_name VALUES({}, '{}')", data->dwPlayerID, data->szHorseName)));

	ForwardPacket(HEADER_DG_UPDATE_HORSE_NAME, data, sizeof(TPacketUpdateHorseName), 0, peer);
}

void CClientManager::AckHorseName(uint32_t dwPID, CPeer* peer)
{
	std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(fmt::format("SELECT name FROM horse_name WHERE id = {}", dwPID)));

	TPacketUpdateHorseName packet;
	packet.dwPlayerID = dwPID;

	if (pmsg->Get()->uiNumRows == 0)
	{
		memset(packet.szHorseName, 0, sizeof (packet.szHorseName));
	}
	else
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		strlcpy(packet.szHorseName, row[0], sizeof(packet.szHorseName));
	}

	peer->EncodeHeader(HEADER_DG_ACK_HORSE_NAME, 0, sizeof(TPacketUpdateHorseName));
	peer->Encode(&packet, sizeof(TPacketUpdateHorseName));
}

