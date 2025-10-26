#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include <game/GamePacket.hpp>
#include "PythonApplication.h"
#include "NetworkActorManager.h"
#include "GameData.hpp"

#include <pak/Vfs.hpp>

#include "pak/Util.hpp"

#ifdef ENABLE_ADMIN_MANAGER
#include "PythonAdmin.h"
#endif

void CPythonNetworkStream::__FilterInsult(char *szLine, UINT uLineLen)
{
    m_kInsultChecker.FilterInsult(szLine, uLineLen);
}

bool CPythonNetworkStream::IsInsultIn(const char *c_szMsg)
{
    return m_kInsultChecker.IsInsultIn(c_szMsg, strlen(c_szMsg));
}

bool CPythonNetworkStream::LoadInsultList(const char *c_szInsultListFileName)
{
    auto str = LoadFileToString(GetVfs(), c_szInsultListFileName);
    if (!str)
        return false;

    CMemoryTextFileLoader kMemTextFileLoader;
    kMemTextFileLoader.Bind(str.value());

    m_kInsultChecker.Clear();
    for (uint32_t dwLineIndex = 0; dwLineIndex < kMemTextFileLoader.GetLineCount(); ++dwLineIndex)
    {
        const std::string &c_rstLine = kMemTextFileLoader.GetLineString(dwLineIndex);
        m_kInsultChecker.AppendInsult(c_rstLine);
    }
    return true;
}


// Loading ---------------------------------------------------------------------------
bool CPythonNetworkStream::LoadingPhase(const PacketHeader &header, const boost::asio::const_buffer &data)
{
#define HANDLE_PACKET(id, fn, type)                                                                                    \
    case id:                                                                                                           \
        return this->fn(ReadPacket<type>(data))

    switch (header.id)
    {

    HANDLE_PACKET(HEADER_GC_MAIN_CHARACTER, RecvMainCharacter, GcMainCharacterPacket);

    default:
        return GamePhase(header, data);
    }

}

void CPythonNetworkStream::SetLoadingPhase()
{
    if ("Loading" != m_strPhase)
        m_phaseLeaveFunc.Run();

    SPDLOG_DEBUG("");
    SPDLOG_DEBUG("## Network - Loading Phase ##");
    SPDLOG_DEBUG("");

    m_strPhase = "Loading";

    m_dwChangingPhaseTime = ELTimer_GetMSec();
    m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoadingPhase);
    m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoadingPhase);

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.Clear();
#ifdef ENABLE_ADMIN_MANAGER
	CPythonAdmin& rkAdmin = CPythonAdmin::Instance();
	rkAdmin.Clear();
#endif
    CPythonGuild::Instance().Destroy();

    CFlyingManager::Instance().DeleteAllInstances();
    CEffectManager::Instance().DeleteAllInstances();
}


bool CPythonNetworkStream::RecvMainCharacter(const GcMainCharacterPacket& p)
{
    m_dwMainActorVID = p.dwVID;
    m_dwMainActorRace = p.wRaceNum;
    m_dwMainActorEmpire = p.empire;
    m_dwMainActorSkillGroup = p.skill_group;

    m_rokNetActorMgr->SetMainActorVID(m_dwMainActorVID);

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.SetName(p.szName.c_str());
    rkPlayer.SetBlockMode(p.blockMode);
    rkPlayer.SetMainCharacterIndex(GetMainActorVID());
    CPythonPlayer::instance().ClearSkillDict();

    RegisterSkill(p.wRaceNum, p.skill_group, p.empire);

    rkPlayer.SetComboSkillFlag(p.comboLevel > 0);
    m_bComboSkillFlag = p.comboLevel > 0 ? true : false;

    Warp(p.mapIndex, p.lx, p.ly);



    return true;
}

bool CPythonNetworkStream::__RecvPlayerPoints(const TPacketGCPoints& p)
{

    for (uint32_t i = 0; i < POINT_MAX_NUM; ++i)
    {
        CPythonPlayer::Instance().SetStatus(i, p.points[i]);
        if (i == POINT_LEVEL)
            m_players[m_selectedCharacterSlot].byLevel = p.points[i];
        else if (i == POINT_ST)
            m_players[m_selectedCharacterSlot].byST = p.points[i];
        else if (i == POINT_HT)
            m_players[m_selectedCharacterSlot].byHT = p.points[i];
        else if (i == POINT_DX)
            m_players[m_selectedCharacterSlot].byDX = p.points[i];
        else if (i == POINT_IQ)
            m_players[m_selectedCharacterSlot].byIQ = p.points[i];
    }
     

    CPythonPlayer::instance().UpdateBattleStatus();
    return true;
}

bool CPythonNetworkStream::__RecvPlayerGold(const TPacketGCGold& p)
{
    CPythonPlayer::Instance().SetGold(p.gold);

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus");
    return true;
}

#ifdef ENABLE_GEM_SYSTEM
bool CPythonNetworkStream::__RecvPlayerGem()
{
	TPacketGCGem GemPacket;

	if (!Recv(sizeof(TPacketGCGem), &GemPacket))
		return false;

	CPythonPlayer::Instance().SetGem(GemPacket.gem);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus");
	return true;
}
#endif

void CPythonNetworkStream::StartGame()
{
    PyCallClassMemberFunc(m_poHandler, "SetGamePhase");

    m_isChatEnable = true;
}

bool CPythonNetworkStream::SendEnterGame()
{
    BlankPacket p = {};
    Send(HEADER_CG_ENTERGAME, p);
    return true;
}
