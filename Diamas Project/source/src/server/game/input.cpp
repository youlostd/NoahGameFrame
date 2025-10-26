#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "config.h"

#include "log.h"
#include "questmanager.h"
#include "fishing.h"
#include "priv_manager.h"
#include "char_manager.h"
#include "DbCacheSocket.hpp"
#include "MasterUtil.hpp"
#include <game/MasterPackets.hpp>

CInputProcessor::CInputProcessor()
{

}

void CInputProcessor::Pong(DESC *d) { d->SetPong(true); }

int CInputProcessor::Text(DESC *d, const std::string& c_pData)
{

    std::string res = HandleSocketCmd(d, c_pData);

    //SPDLOG_INFO("WebAPI from {2}: {0} -> {1}", cmd, res, d->GetHostName());

    res += "\n";
    d->Send(HEADER_GC_TEXT, res);
    
    return true;
}

void CInputProcessor::Handshake(DESC *d, const TPacketCGHandshake& p)
{

    if (d->GetHandshake() != p.dwHandshake)
    {
        SPDLOG_ERROR("Invalid Handshake on {0}", d->GetSocket());
        d->DelayedDisconnect(0, "INVALID_HANDSHAKE");
    }
    else { d->HandshakeProcess(p.dwTime, p.lDelta, true); }
}

void LoginFailure(DESC *d, const char *c_pszStatus)
{
    if (!d)
        return;

    TPacketGCLoginFailure p;
    p.szStatus = c_pszStatus;
    d->Send(HEADER_GC_LOGIN_FAILURE, p);
}

bool CInputPhase::Analyze(DESC *d, const PacketHeader &header,
    const boost::asio::const_buffer &data)
{
    switch (header.id)
    {
    case HEADER_CG_PHASE_ACK:
        RecvPhaseAck(d);
        break;

    case HEADER_CG_PONG:
        Pong(d);
        break;

    case HEADER_CG_TIME_SYNC:
        Handshake(d, ReadPacket<TPacketCGHandshake>(data));
        break;

    default: { return true; }
    }

    return true;
}

void CInputPhase::RecvPhaseAck(DESC *d) { d->SetInputProcessor(d->GetPhase()); }

CInputHandshake::CInputHandshake()
{
}

CInputHandshake::~CInputHandshake()
{
}

// BLOCK_CHAT
ACMD(do_block_chat);
// END_OF_BLOCK_CHAT

std::string HandleSocketCmd(DESC *d, const std::string &cmd)
{
    if (cmd == "IS_SERVER_UP")
    {
        if (g_bNoMoreClient)
            return "NO";
        else
            return "YES";
    }
    else if (cmd == "QUIT")
    {
        // QUIT is useful for netcat scripts, no need to use a timeout and it'll quit as soon as stuff is done
        d->DelayedDisconnect(0, "QUIT");
    }
    else if (cmd == gConfig.adminpagePassword)
    {
        if (std::find(gConfig.adminpageAddrs.begin(), gConfig.adminpageAddrs.end(), d->GetHostName().c_str()) !=
            gConfig.adminpageAddrs.end())
        {
            d->SetAdminMode();
            return "ELEVATED";
        }
        else { return "REJECTED"; }
    }

    if (!d->IsAdminMode())
        return "REFUSED";

    if (cmd == "USER_COUNT")
    {
        char szTmp[64];

        int iTotal;
        int *paiEmpireUserCount;
        int iLocal;
        DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);
        snprintf(szTmp, sizeof(szTmp), "%d %d %d %d %d", iTotal, paiEmpireUserCount[1], paiEmpireUserCount[2],
                 paiEmpireUserCount[3], iLocal);

        return szTmp;
    }
    else if (!cmd.compare(0, 7, "NOTICE "))
    {
        std::string msg = cmd.substr(7, cmd.length());
        LogManager::instance().CharLog(0, 0, 0, 1, "NOTICE", msg.c_str(), d->GetHostName().c_str());
        BroadcastNotice(msg.c_str());
        return "OK";
    }
    else if (cmd == "SHUTDOWN")
    {
        LogManager::instance().CharLog(0, 0, 0, 2, "SHUTDOWN", "", d->GetHostName().c_str());

        GmShutdownBroadcastPacket p;
        p.countdown = 10;
        p.reason = "WEBAPI"; //d->GetHostName();

        SPDLOG_ERROR("Accept shutdown command from {0}", p.reason);

        DESC_MANAGER::instance().GetMasterSocket()->Send(kGmShutdownBroadcast,
                                                         p);

        Shutdown(p.countdown);
        return "OK";
    }
    else if (cmd == "SHUTDOWN_ONLY")
    {
        LogManager::instance().CharLog(0, 0, 0, 2, "SHUTDOWN", "", d->GetHostName().c_str());
        Shutdown(10);
        return "OK";
    }
    else if (!cmd.compare(0, 3, "DC "))
    {
        std::string msg = cmd.substr(3, LOGIN_MAX_LEN);

        LogManager::instance().CharLog(0, 0, 0, 3, "DC", msg.c_str(), d->GetHostName().c_str());

        CHARACTER *ch = g_pCharManager->FindPC(msg.c_str());
        if (ch) { DESC_MANAGER::instance().DestroyDesc(ch->GetDesc()); }
        else
        {
            const auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(msg);
            if (op)
                RelayDisconnect(op->pid);
        }

        // ...

        return ch ? "OK" : "OK;relayed";
    }
    else if (!cmd.compare(0, 6, "RELOAD"))
    {
        if (cmd.size() == 8)
        {
            char c = cmd[7];

            switch (LOWER(c))
            {
            case 'q':
                quest::CQuestManager::instance().Reload();
                break;

            case 'f':
                fishing::Initialize();
                break;

            case 'a':
                db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, nullptr, 0);
                SPDLOG_INFO("Reloading admin infomation.");
                break;

            default:
                return "SYNTAX_ERROR";
            }
            return "OK";
        }
    }
    else if (!cmd.compare(0, 6, "EVENT "))
    {
        std::istringstream is(cmd.c_str());
        std::string strEvent, strFlagName;
        long lValue;
        is >> strEvent >> strFlagName >> lValue;

        if (!is.fail())
        {
            SPDLOG_INFO("EXTERNAL EVENT FLAG name {0} value {1}",
                        strFlagName, lValue);

            quest::CQuestManager::instance().RequestSetEventFlag(strFlagName.c_str(), lValue);
            return "OK";
        }
        else { return "SYNTAX_ERROR"; }
    }
    else if (!cmd.compare(0, 11, "BLOCK_CHAT "))
    {
        std::istringstream is(cmd.c_str());
        std::string strBlockChat, strCharName;
        long lDuration;
        is >> strBlockChat >> strCharName >> lDuration;

        if (!is.fail())
        {
            SPDLOG_INFO("EXTERNAL BLOCK_CHAT name {0} duration {1}",
                        strCharName, lDuration);

            std::string copy(cmd.c_str());
            do_block_chat(nullptr, &*copy.begin() + 11, 0, 0);

            return "OK";
        }
        else { return "SYNTAX_ERROR"; }
    }
    else if (!cmd.compare(0, 12, "PRIV_EMPIRE "))
    {
        int empire, type, value, duration;
        std::istringstream is(cmd.c_str());
        std::string strPrivEmpire;
        is >> strPrivEmpire >> empire >> type >> value >> duration;

        // 최대치 10배
        value = std::clamp(value, 0, 1000);

        if (!is.fail() && empire >= 0 && empire < 4 &&
            type > 0 && type < 5 &&
            value > 0 && duration > 0)
        {
            // 시간 단위로 변경
            duration = duration * (60 * 60);

            SPDLOG_INFO("PRIV EMPIRE empire {0} type {1} value {2} duration {3}",
                        empire, type, value, duration);

            CPrivManager::instance().RequestGiveEmpirePriv(empire, type, value, duration);
            return "OK";
        }
        else { return "SYNTAX_ERROR"; }
    }

    return "ERROR";
}

bool CInputHandshake::Analyze(DESC *d, const PacketHeader &header,
    const boost::asio::const_buffer &data)
{
    if (header.id == 10)
        return 0;

    if (header.id == HEADER_CG_TEXT) { return Text(d, ReadPacket<std::string>(data)); }
    else if (header.id == HEADER_CG_MARK_LOGIN)
    {
        if (!gConfig.guildMarkServer)
        {
            SPDLOG_ERROR("Guild Mark login requested but i'm not a mark server!");
            d->DelayedDisconnect(0, "NOT_MARK_SERVER");
            return true;
        }

        SPDLOG_INFO("MARK_SERVER: Login");
        d->SetPhase(PHASE_LOGIN);
        return true;
    }
    else if (header.id == HEADER_CG_PONG)
        Pong(d);
    else if (header.id == HEADER_CG_TIME_SYNC)
        Handshake(d, ReadPacket<TPacketCGHandshake>(data));
    else
        SPDLOG_ERROR("Handshake phase does not handle packet {0} (fd {1})",
                 header.id, d->GetSocket());

    return true;
}
