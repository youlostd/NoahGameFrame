#include "itemname_manager.h"
#include "DbCacheSocket.hpp"
#include "char_manager.h"
#include "char.h"
#include <game/AffectConstants.hpp>
#include "utils.h"

CItemNameManager::CItemNameManager() { m_mapItemNames.clear(); }

const char *CItemNameManager::GetItemName(uint32_t dwItemId)
{
    const auto iter = m_mapItemNames.find(dwItemId);

    if (iter != m_mapItemNames.end()) { return iter->second.c_str(); }

    return "";
}

void CItemNameManager::UpdateItemName(uint32_t dwItemId, const char *szName, bool broadcast)
{
    if (szName == nullptr)
    {
        SPDLOG_ERROR("ITEMNAME: NULL NAME (%u)", dwItemId);
        szName = "";
    }

    SPDLOG_INFO("ITEMNAME: update %u %s", dwItemId, szName);

    m_mapItemNames[dwItemId] = szName;

    if (broadcast == true) { BroadCastItemName(dwItemId, szName); }
}

void CItemNameManager::BroadCastItemName(uint32_t dwItemId, const char *szName)
{
    TPacketUpdateItemName packet;
    packet.dwItemID = dwItemId;
    strlcpy(packet.szName, szName, sizeof(packet.szName));

    db_clientdesc->DBPacket(HEADER_GD_UPDATE_ITEM_NAME, 0, &packet, sizeof(TPacketUpdateItemName));
}

void CItemNameManager::Validate(CItem *item)
{
}
