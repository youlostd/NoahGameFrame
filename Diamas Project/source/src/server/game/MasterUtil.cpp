#include "MasterUtil.hpp"
#include "MasterSocket.hpp"
#include "desc_manager.h"


#include <game/MasterPackets.hpp>

void SendLoginPacket(uint32_t handle,
                     std::string_view login,
                     SessionId sessionId)
{
    GmLoginPacket p;
    p.handle = handle;
    p.login = login;
    p.sessionId = sessionId;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmLogin,
                                                     p);
}

void SendLogoutPacket(uint32_t aid)
{
    GmLogoutPacket p;
    p.aid = aid;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmLogout,
                                                     p);
}

void RelayDisconnect(uint32_t pid)
{
    GmCharacterDisconnectPacket p;
    p.pid = pid;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmCharacterDisconnect,
                                                     p);
}

void RelayChatPacket(uint32_t pid, uint8_t type,
                     std::string_view message)
{
    GmChatPacket p;
    p.pid = pid;
    p.type = type;
    p.message = message;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmChat, p);
}

void RelayWhisperPacket(uint32_t sourcePid, uint32_t targetPid, uint8_t type, uint8_t sourceLc, 
                     const std::string& from, const std::string& message)
{
    GmWhisperPacket gwp;
    gwp.sourcePid = sourcePid;
    gwp.targetPid = targetPid;
    gwp.localeCode = sourceLc;
    gwp.type = type;
    gwp.message = message;
    gwp.from = from;

    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmWhisper, gwp);
}
