#include "utils.h"
#include "config.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"
#include "desc.h"
#include <game/GamePacket.hpp>
#include "target.h"

/////////////////////////////////////////////////////////////////////
// Packet
/////////////////////////////////////////////////////////////////////
void SendTargetCreatePacket(DESC *d, TargetInfo *info)
{
    if (!info->bSendToClient)
        return;

    TPacketGCTargetCreate pck;
    pck.lID = info->iID;
    pck.bType = info->iType;
    pck.dwVID = info->iArg1;
    pck.szName = info->szTargetDesc;
    d->Send(HEADER_GC_TARGET_CREATE, pck);
}

void SendTargetUpdatePacket(DESC *d, int iID, int x, int y)
{
    TPacketGCTargetUpdate pck;
    pck.lID = iID;
    pck.lX = x;
    pck.lY = y;
    d->Send(HEADER_GC_TARGET_UPDATE, pck);
}

void SendTargetDeletePacket(DESC *d, int iID)
{
    TPacketGCTargetDelete pck;
    pck.lID = iID;
    d->Send(HEADER_GC_TARGET_DELETE, pck);
}

/////////////////////////////////////////////////////////////////////
CTargetManager::CTargetManager()
    : m_iID(0)
{
}

CTargetManager::~CTargetManager()
{
}

EVENTFUNC(target_event)
{
    TargetInfo *info = static_cast<TargetInfo *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("target_event> <Factor> Null pointer");
        return 0;
    }

    // <Factor> Raplaced direct pointer reference with key searching.
    //CHARACTER* pkChr = info->pkChr;
    CHARACTER *pkChr = g_pCharManager->FindByPID(info->dwPID);
    if (pkChr == nullptr)
    {
        return 0; // <Factor> need to be confirmed
    }
    CHARACTER *tch = nullptr;
    int x = 0, y = 0;
    int iDist = 5000;

    if (info->iMapIndex != pkChr->GetMapIndex())
        return std::clamp(iDist / (1500 / THECORE_SECS_TO_PASSES(1)), THECORE_SECS_TO_PASSES(1) / 2,
                          THECORE_SECS_TO_PASSES(1) * 5);

    switch (info->iType)
    {
    case TARGET_TYPE_POS:
        x = info->iArg1;
        y = info->iArg2;
        iDist = DISTANCE_APPROX(pkChr->GetX() - x, pkChr->GetY() - y);
        break;

    case TARGET_TYPE_VID: {
        tch = g_pCharManager->Find(info->iArg1);

        if (tch && tch->GetMapIndex() == pkChr->GetMapIndex())
        {
            x = tch->GetX();
            y = tch->GetY();
            iDist = DISTANCE_APPROX(pkChr->GetX() - x, pkChr->GetY() - y);
        }
    }
    break;
    }

    bool bRet = true;

    if (iDist <= info->iDistance)
        bRet = quest::CQuestManager::instance().Target(pkChr->GetPlayerID(), info->dwQuestIndex, info->szTargetName,
                                                       "arrive");

    if (!tch && info->iType == TARGET_TYPE_VID)
    {
        quest::CQuestManager::instance().Target(pkChr->GetPlayerID(), info->dwQuestIndex, info->szTargetName, "die");
        CTargetManager::instance().DeleteTarget(pkChr->GetPlayerID(), info->dwQuestIndex, info->szTargetName);
    }

#if defined(WJ_COMBAT_ZONE)
	tch = g_pCharManager->Find(info->iArg1);
	if ((tch && tch->IsDead()) || (!tch && info->iType == TARGET_TYPE_COMBAT_ZONE))
	{
		CTargetManager::instance().DeleteTarget(pkChr->GetPlayerID(), info->dwQuestIndex, info->szTargetName);
	}
#endif

    if (event->is_force_to_end)
    {
        SPDLOG_TRACE("target_event: event canceled");
        return 0;
    }

    if (x != info->iOldX || y != info->iOldY)
    {
        if (info->bSendToClient)
            SendTargetUpdatePacket(pkChr->GetDesc(), info->iID, x, y);

        info->iOldX = x;
        info->iOldY = y;
    }

    if (!bRet)
        return THECORE_SECS_TO_PASSES(1);
    else
        return std::clamp(iDist / (1500 / THECORE_SECS_TO_PASSES(1)), THECORE_SECS_TO_PASSES(1) / 2,
                          THECORE_SECS_TO_PASSES(1) * 5);
}

void CTargetManager::CreateTarget(uint32_t dwPID,
                                  uint32_t dwQuestIndex,
                                  const char *c_pszTargetName,
                                  int iType,
                                  int iArg1,
                                  int iArg2,
                                  int iMapIndex,
                                  const char *c_pszTargetDesc,
                                  int iSendFlag,
                                  int iDistance)
{
    SPDLOG_TRACE("CreateTarget : target pid %u quest %u name %s arg %d %d %d",
                dwPID, dwQuestIndex, c_pszTargetName, iType, iArg1, iArg2);

    CHARACTER *pkChr = g_pCharManager->FindByPID(dwPID);

    if (!pkChr)
    {
        SPDLOG_ERROR("Cannot find character ptr by PID %u", dwPID);
        return;
    }

    if (pkChr->GetMapIndex() != iMapIndex)
        return;

    auto it = m_map_kListEvent.find(dwPID);

    if (it != m_map_kListEvent.end())
    {
        std::list<LPEVENT>::const_iterator it2 = it->second.begin();

        while (it2 != it->second.end())
        {
            LPEVENT pkEvent = *(it2++);
            TargetInfo *existInfo = static_cast<TargetInfo *>(pkEvent->info);

            if (nullptr == existInfo)
            {
                SPDLOG_ERROR("CreateTarget : event already exist, but have no info");
                return;
            }

            if (existInfo->dwQuestIndex == dwQuestIndex && !strcmp(existInfo->szTargetName, c_pszTargetName))
            {
                SPDLOG_TRACE("CreateTarget : same target will be replaced");

                if (existInfo->bSendToClient)
                    SendTargetDeletePacket(pkChr->GetDesc(), existInfo->iID);

                if (c_pszTargetDesc)
                {
                    strlcpy(existInfo->szTargetDesc, c_pszTargetDesc, sizeof(existInfo->szTargetDesc));
                }
                else { *existInfo->szTargetDesc = '\0'; }

                existInfo->iID = ++m_iID;
                existInfo->iType = iType;
                existInfo->iArg1 = iArg1;
                existInfo->iArg2 = iArg2;
                existInfo->iOldX = 0;
                existInfo->iOldY = 0;
                existInfo->bSendToClient = iSendFlag ? true : false;

                SendTargetCreatePacket(pkChr->GetDesc(), existInfo);
                return;
            }
        }
    }

    TargetInfo *newInfo = AllocEventInfo<TargetInfo>();

    if (c_pszTargetDesc) { strlcpy(newInfo->szTargetDesc, c_pszTargetDesc, sizeof(newInfo->szTargetDesc)); }
    else { *newInfo->szTargetDesc = '\0'; }

    newInfo->iID = ++m_iID;
    // <Factor> Removed pkChr
    //newInfo->pkChr = pkChr;
    newInfo->dwPID = dwPID;
    newInfo->dwQuestIndex = dwQuestIndex;
    strlcpy(newInfo->szTargetName, c_pszTargetName, sizeof(newInfo->szTargetName));
    newInfo->iType = iType;
    newInfo->iArg1 = iArg1;
    newInfo->iArg2 = iArg2;
    newInfo->iMapIndex = iMapIndex;
    newInfo->iOldX = 0;
    newInfo->iOldY = 0;
    newInfo->bSendToClient = iSendFlag ? true : false;
    newInfo->iDistance = iDistance;

    LPEVENT event = event_create(target_event, newInfo, THECORE_SECS_TO_PASSES(1));

    if (nullptr != event)
    {
        m_map_kListEvent[dwPID].push_back(event);

        SendTargetCreatePacket(pkChr->GetDesc(), newInfo);
    }
}

void CTargetManager::DeleteTarget(uint32_t dwPID, uint32_t dwQuestIndex, const char *c_pszTargetName)
{
    auto it = m_map_kListEvent.find(dwPID);

    if (it == m_map_kListEvent.end())
        return;

    std::list<LPEVENT>::iterator it2 = it->second.begin();

    while (it2 != it->second.end())
    {
        LPEVENT pkEvent = *it2;
        TargetInfo *info = static_cast<TargetInfo *>(pkEvent->info);

        if (info == nullptr)
        {
            SPDLOG_ERROR("CTargetManager::DeleteTarget> <Factor> Null pointer");
            ++it2;
            continue;
        }

        if (dwQuestIndex == info->dwQuestIndex)
        {
            if (!c_pszTargetName || !strcmp(info->szTargetName, c_pszTargetName))
            {
                if (info->bSendToClient)
                {
                    // <Factor> Removed pkChr
                    //SendTargetDeletePacket(info->pkChr->GetDesc(), info->iID);
                    CHARACTER *pkChr = g_pCharManager->FindByPID(info->dwPID);
                    if (pkChr != nullptr) { SendTargetDeletePacket(pkChr->GetDesc(), info->iID); }
                }

                event_cancel(&pkEvent);
                it2 = it->second.erase(it2);
                continue;
            }
        }

        ++it2;
    }
}

LPEVENT CTargetManager::GetTargetEvent(uint32_t dwPID, uint32_t dwQuestIndex, const char *c_pszTargetName)
{
    auto it = m_map_kListEvent.find(dwPID);

    if (it == m_map_kListEvent.end())
        return nullptr;

    std::list<LPEVENT>::iterator it2 = it->second.begin();

    while (it2 != it->second.end())
    {
        LPEVENT pkEvent = *(it2++);
        TargetInfo *info = static_cast<TargetInfo *>(pkEvent->info);

        if (info == nullptr)
        {
            SPDLOG_ERROR("CTargetManager::GetTargetEvent> <Factor> Null pointer");

            continue;
        }

        if (info->dwQuestIndex != dwQuestIndex)
            continue;

        if (strcmp(info->szTargetName, c_pszTargetName))
            continue;

        return pkEvent;
    }

    return nullptr;
}

TargetInfo *CTargetManager::GetTargetInfo(uint32_t dwPID, int iType, int iArg1)
{
    auto it = m_map_kListEvent.find(dwPID);

    if (it == m_map_kListEvent.end())
        return nullptr;

    std::list<LPEVENT>::iterator it2 = it->second.begin();

    while (it2 != it->second.end())
    {
        LPEVENT pkEvent = *(it2++);
        TargetInfo *info = static_cast<TargetInfo *>(pkEvent->info);

        if (info == nullptr)
        {
            SPDLOG_ERROR("CTargetManager::GetTargetInfo> <Factor> Null pointer");

            continue;
        }

        if (!IS_SET(info->iType, iType))
            continue;

        if (info->iArg1 != iArg1)
            continue;

        return info;
    }

    return nullptr;
}

void CTargetManager::Logout(uint32_t dwPID)
{
    auto it = m_map_kListEvent.find(dwPID);

    if (it == m_map_kListEvent.end())
        return;

    std::list<LPEVENT>::iterator it2 = it->second.begin();

    while (it2 != it->second.end())
        event_cancel(&(*(it2++)));

    m_map_kListEvent.erase(it);
}
