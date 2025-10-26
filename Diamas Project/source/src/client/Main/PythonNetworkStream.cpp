#include "StdAfx.h"

#include "PythonNetworkStream.h"
#include <game/GamePacket.hpp>
#include <utility>
#include "NetworkActorManager.h"

#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "MarkManager.h"
#include "InstanceBase.h"

#include "../EterLib/Engine.hpp"
#include "../EterBase/Timer.h"

#include <StepTimer.h>

#ifdef __OFFLINE_SHOP__
#include "PythonOfflineShop.h"
#endif
// MARK_BUG_FIX
static uint32_t gs_nextDownloadMarkTime = 0;
// END_OF_MARK_BUG_FIX
#define DEF_STR(x) #x

std::shared_ptr<CPythonNetworkStream> gPythonNetworkStream;


void CPythonNetworkStream::ExitApplication()
{
    if (__IsNotPing())
    {
        AbsoluteExitApplication();
    }
    else
    {
        SendChatPacket("/quit");
    }
}

void CPythonNetworkStream::ExitGame()
{
    if (__IsNotPing())
    {
        LogOutGame();
    }
    else
    {
        SendChatPacket("/phase_select");
    }
}

void CPythonNetworkStream::LogOutGame()
{
    if (__IsNotPing())
    {
        AbsoluteExitGame();
    }
    else
    {
        SendChatPacket("/logout");
    }
}

void CPythonNetworkStream::AbsoluteExitGame()
{
    if (IsClosed())
        return;
    SPDLOG_ERROR("Network Error: AbsoluteExitGame");

    OnRemoteDisconnect();
    Disconnect();
}

void CPythonNetworkStream::AbsoluteExitApplication()
{
    SPDLOG_ERROR(" CPythonNetworkStream::AbsoluteExitApplication");
    PostQuitMessage(0);
}

bool CPythonNetworkStream::__IsNotPing()
{
    // TODO: Implement this properly so exiting from the game is possible without using the task manager
    // in some cases the game does not detect that the connection is lost and we are stuck in the game phase
    return m_dwLastGamePingTime + 60000000 < ELTimer_GetMSec();
}

uint32_t CPythonNetworkStream::GetGuildID()
{
    return m_dwGuildID;
}

UINT CPythonNetworkStream::UploadMark(const char *c_szImageFileName)
{
    // MARK_BUG_FIX
    // 길드를 만든 직후는 길드 아이디가 0이다.
    if (0 == m_dwGuildID)
        return ERROR_MARK_UPLOAD_NEED_RECONNECT;

    gs_nextDownloadMarkTime = 0;
    // END_OF_MARK_BUG_FIX

    UINT uError = ERROR_UNKNOWN;
    if (!gGuildMarkUploader->Connect(m_markAddr, m_markPort, m_dwGuildID, c_szImageFileName, &uError))
    {
        switch (uError)
        {
        case CGuildMarkUploader::ERROR_CONNECT:
            return ERROR_CONNECT_MARK_SERVER;
        case CGuildMarkUploader::ERROR_LOAD:
            return ERROR_LOAD_MARK;
        case CGuildMarkUploader::ERROR_WIDTH:
            return ERROR_MARK_WIDTH;
        case CGuildMarkUploader::ERROR_HEIGHT:
            return ERROR_MARK_HEIGHT;
        default:
            return ERROR_UNKNOWN;
        }
    }

    // MARK_BUG_FIX	
    __DownloadMark();
    // END_OF_MARK_BUG_FIX

    if (CGuildMarkManager::INVALID_MARK_ID == CGuildMarkManager::Instance().GetMarkID(m_dwGuildID))
        return ERROR_MARK_CHECK_NEED_RECONNECT;

    return ERROR_NONE;
}

UINT CPythonNetworkStream::UploadSymbol(const char *c_szImageFileName)
{
    UINT uError = ERROR_UNKNOWN;
    if (!gGuildMarkUploader->ConnectToSendSymbol(m_markAddr, m_markPort, m_dwGuildID, c_szImageFileName, &uError))
    {
        switch (uError)
        {
        case CGuildMarkUploader::ERROR_CONNECT:
            return ERROR_CONNECT_MARK_SERVER;
        case CGuildMarkUploader::ERROR_LOAD:
            return ERROR_LOAD_MARK;
        case CGuildMarkUploader::ERROR_WIDTH:
            return ERROR_MARK_WIDTH;
        case CGuildMarkUploader::ERROR_HEIGHT:
            return ERROR_MARK_HEIGHT;
        default:
            return ERROR_UNKNOWN;
        }
    }

    return ERROR_NONE;
}

void CPythonNetworkStream::__DownloadMark()
{
    // 3분 안에는 다시 접속하지 않는다.
    uint32_t curTime = ELTimer_GetMSec();

    if (curTime < gs_nextDownloadMarkTime)
        return;

    gs_nextDownloadMarkTime = curTime + 60000 * 3; // 3분

    gGuildMarkDownloader->Connect(m_markAddr, m_markPort);
}

void CPythonNetworkStream::__DownloadSymbol(const std::vector<uint32_t> &c_rkVec_dwGuildID)
{
   gGuildMarkDownloader->ConnectToRecvSymbol(m_markAddr, m_markPort, c_rkVec_dwGuildID);
}

py::object CPythonNetworkStream::GetPhaseWindow(UINT ePhaseWnd)
{
    if (ePhaseWnd >= PHASE_WINDOW_NUM)
        return py::none();

    return m_apoPhaseWnd[ePhaseWnd];
}

void CPythonNetworkStream::SetPhaseWindow(UINT ePhaseWnd, py::object poPhaseWnd)
{
    if (ePhaseWnd >= PHASE_WINDOW_NUM)
        return;

    if (ePhaseWnd == PHASE_WINDOW_LOGIN)
        isInputEnabled = 1;

    m_apoPhaseWnd[ePhaseWnd] = std::move(poPhaseWnd);
}

void CPythonNetworkStream::ClearPhaseWindow(UINT ePhaseWnd, py::object poPhaseWnd)
{
    if (ePhaseWnd >= PHASE_WINDOW_NUM)
        return;

    if (!m_apoPhaseWnd[ePhaseWnd].is(poPhaseWnd))
        return;

    m_apoPhaseWnd[ePhaseWnd] = py::none();
}

void CPythonNetworkStream::SetServerCommandParserWindow(py::object poWnd)
{
    m_poSerCommandParserWnd = poWnd;
}

bool CPythonNetworkStream::IsSelectedEmpire()
{
    for (auto &player : m_players)
        if (player.id != 0)
            return true;

    return false;
}

void CPythonNetworkStream::SetLogin(const std::string &login)
{
    m_login = login;
}

void CPythonNetworkStream::SetSessionId(SessionId sessionId)
{
    m_sessionId = sessionId;
}

bool CPythonNetworkStream::ConnectLoginServer(const char *c_szAddr, UINT uPort)
{
    return Connect(c_szAddr, fmt::to_string(uPort));
}

void CPythonNetworkStream::SetMarkServer(const char *c_szAddr, UINT uPort)
{
    m_markAddr = c_szAddr;
    m_markPort = fmt::to_string(uPort);
}

void CPythonNetworkStream::ConnectGameServer(UINT iChrSlot)
{
    if (iChrSlot >= PLAYER_PER_ACCOUNT)
        return;

    m_selectedCharacterSlot = iChrSlot;
    m_autoSelectCharacter = true;

    const auto &info = m_players[iChrSlot];

    SPDLOG_TRACE("Selecting char {0} with addr {1}:{2} current {3}:{4}",
                  iChrSlot, info.addr, info.port,
                  m_host, m_service);

    Connect(info.addr, fmt::to_string(info.port));
}


void CPythonNetworkStream::RecvUnhandled(int header)
{
    SPDLOG_ERROR(
        "Phase {0} does not handle this packet (header: {1})",
        m_strPhase, header);

    Disconnect();
    ClosePhase();
}

bool CPythonNetworkStream::RecvMultiStatus(const BlankPacket& p) {
    m_multiAllowed = p.dummydata;
    for(const auto& win : m_apoPhaseWnd)
        PyCallClassMemberFunc(win, "BINARY_RefreshMultiAllow", p.dummydata);

    return true;
}

bool CPythonNetworkStream::RecvPhasePacket(const TPacketGCPhase& p)
{
    CgPhaseAckPacket ack;
    Send(HEADER_CG_PHASE_ACK, ack);
    
    switch (p.phase)
    {
    case PHASE_CLOSE: // 끊기는 상태 (또는 끊기 전 상태)
        ClosePhase();
        m_isChatEnable = FALSE;
        break;

    case PHASE_SAME_LOGIN: // Phase you get sent to if someone logged into your account
        SameLoginPhase();
        m_isChatEnable = FALSE;
        break;

    case PHASE_HANDSHAKE: // 악수..;;
        SetHandShakePhase();
        m_isChatEnable = FALSE;
        break;

    case PHASE_LOGIN: // 로그인 중
        SetLoginPhase();
        m_isChatEnable = FALSE;
        break;

    case PHASE_SELECT: // 캐릭터 선택 화면
        SetSelectPhase();
        m_isChatEnable = FALSE;

        //BuildProcessCRC();

        // MARK_BUG_FIX
        __DownloadMark();
        // END_OF_MARK_BUG_FIX
        break;

    case PHASE_LOADING: // 선택 후 로딩 화면
        SetLoadingPhase();
        m_isChatEnable = FALSE;
        break;

    case PHASE_GAME: // 게임 화면
        SetGamePhase();
        DX::StepTimer::instance().ResetElapsedTime();
        SendTimeSyncReqPacket();
        m_isChatEnable = TRUE;
        break;

    case PHASE_DEAD: // 죽었을 때.. (게임 안에 있는 것일 수도..)
        break;
    default:
        break;
    }

    return true;
}

bool CPythonNetworkStream::RecvPingPacket(const TPacketGCPing& p)
{
    SPDLOG_DEBUG("recv ping packet");

    m_dwLastGamePingTime = ELTimer_GetMSec();

    Send(HEADER_CG_PONG, p);
    return true;
}

// Set
void CPythonNetworkStream::SetOffLinePhase()
{
    if ("OffLine" != m_strPhase)
        m_phaseLeaveFunc.Run();

    m_strPhase = "OffLine";

    SPDLOG_DEBUG("");
    SPDLOG_DEBUG("## Network - OffLine Phase ##");
    SPDLOG_DEBUG("");

    m_dwChangingPhaseTime = ELTimer_GetMSec();
    m_phaseProcessFunc.Set(this, &CPythonNetworkStream::OffLinePhase);
    m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveOfflinePhase);

    SetGameOffline();
    m_firstHandShakeCompleted = false;
    m_selectedCharacterSlot = PLAYER_PER_ACCOUNT;
    m_autoSelectCharacter = false;

    __BettingGuildWar_Initialize();
}

void CPythonNetworkStream::ClosePhase()
{
    PyCallClassMemberFunc(m_poHandler, "SetLoginPhase");
}

void CPythonNetworkStream::SameLoginPhase()
{
    PyCallClassMemberFunc(m_poHandler, "SameLogin_SetLoginPhase");
}

// Game Online
void CPythonNetworkStream::SetGameOnline()
{
    m_isGameOnline = TRUE;
}

void CPythonNetworkStream::SetGameOffline()
{
    m_isGameOnline = FALSE;
}

bool CPythonNetworkStream::IsGameOnline()
{
    return m_isGameOnline;
}

// Handler
void CPythonNetworkStream::SetHandler(py::object poHandler)
{
    m_poHandler = poHandler;
}

// ETC
uint32_t CPythonNetworkStream::GetMainActorVID()
{
    return m_dwMainActorVID;
}

uint32_t CPythonNetworkStream::GetMainActorRace()
{
    return m_dwMainActorRace;
}

uint32_t CPythonNetworkStream::GetMainActorEmpire()
{
    return m_dwMainActorEmpire;
}

uint32_t CPythonNetworkStream::GetMainActorSkillGroup()
{
    return m_dwMainActorSkillGroup;
}

void CPythonNetworkStream::__ClearSelectCharacterData()
{
    memset(&m_players, 0, sizeof(m_players));

    for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
    {
        m_adwGuildID[i] = 0;
        m_astrGuildName[i] = "";
        m_playerNames[i] = "";
    }
}

void CPythonNetworkStream::__BettingGuildWar_Initialize()
{
    m_kBettingGuildWar.m_dwBettingMoney = 0;
    m_kBettingGuildWar.m_dwObserverCount = 0;
}

void CPythonNetworkStream::__BettingGuildWar_SetObserverCount(UINT uObserverCount)
{
    m_kBettingGuildWar.m_dwObserverCount = uObserverCount;
}

void CPythonNetworkStream::__BettingGuildWar_SetBettingMoney(UINT uBettingMoney)
{
    m_kBettingGuildWar.m_dwBettingMoney = uBettingMoney;
}

void CPythonNetworkStream::detected(uint8_t state, std::string desc)
{
    m_timeState = state;
    m_timeDesc.assign(desc);
    //m_timeDesc(desc);
}

void CPythonNetworkStream::OnConnectSuccess() {
    SPDLOG_INFO("Successfully connected {0}", m_socket);
    StartReadSome();
}

void CPythonNetworkStream::OnConnectFailure(const boost::system::error_code& ec) {
  	SPDLOG_ERROR("Failed to connect to {0}:{1} with {2}",
	          m_host, m_service, ec.message());

	//Reconnect();
}

bool CPythonNetworkStream::HandlePacket(const PacketHeader &              header,
                                        const boost::asio::const_buffer & data) {

  #define HANDLE_PACKET(id, fn, type) \
	case id: return this->fn(ReadPacket<type>(data))

    switch (header.id)
    {
    HANDLE_PACKET(HEADER_GC_PHASE, RecvPhasePacket,
                      TPacketGCPhase);

    HANDLE_PACKET(HEADER_GC_PING, RecvPingPacket,
                      TPacketGCPing);

    HANDLE_PACKET(HEADER_GC_MULTI_STATUS, RecvMultiStatus, BlankPacket);
    HANDLE_PACKET(HEADER_GC_TIME, RecvTimePacket, TPacketGCTime);

    default:
        break;
    }

    m_rokNetActorMgr->Update();

    if (m_phaseProcessFunc.IsEmpty())
        return true;

    //TPacketHeader header;
    //while(CheckPacket(&header))
    {
        return m_phaseProcessFunc.Run(header, data);
    }


#undef HANDLE_PACKET
}

uint32_t CPythonNetworkStream::EXPORT_GetBettingGuildWarValue(const char *c_szValueName)
{
    if (stricmp(c_szValueName, "OBSERVER_COUNT") == 0)
        return m_kBettingGuildWar.m_dwObserverCount;

    if (stricmp(c_szValueName, "BETTING_MONEY") == 0)
        return m_kBettingGuildWar.m_dwBettingMoney;

    return 0;
}

void CPythonNetworkStream::__ServerTimeSync_Initialize()
{
    m_kServerTimeSync.m_dwChangeClientTime = 0;
    m_kServerTimeSync.m_dwChangeServerTime = 0;
}

void CPythonNetworkStream::SetWaitFlag()
{
    m_isWaitLoginKey = TRUE;
}

void CPythonNetworkStream::SendEmoticon(UINT eEmoticon)
{
    if (eEmoticon < m_EmoticonStringVector.size())
        SendChatPacket(m_EmoticonStringVector[eEmoticon].c_str());
    else
        assert(false && "SendEmoticon Error");
}

CPythonNetworkStream::CPythonNetworkStream(boost::asio::io_service &ioService)
    : AsyncClientSocketAdapter(ioService), m_rokNetActorMgr(new CNetworkActorManager)
{
    m_phaseProcessFunc.Clear();

    m_dwGuildID = 0;
    m_langCode = 0;

    m_dwMainActorVID = 0;
    m_dwMainActorRace = 0;
    m_dwMainActorEmpire = 0;
    m_dwMainActorSkillGroup = 0;

    m_dwLastGamePingTime = 0;

    m_sessionId = 0;
    m_isWaitLoginKey = false;
    m_firstHandShakeCompleted = false;
    m_isChatEnable = true;
    m_bComboSkillFlag = false;
    m_strPhase = "OffLine";

    __InitializeGamePhase();

    __BettingGuildWar_Initialize();

    std::fill(m_apoPhaseWnd, m_apoPhaseWnd + PHASE_WINDOW_NUM, py::none());
    m_poSerCommandParserWnd = py::none();

    SetOffLinePhase();
}

CPythonNetworkStream::~CPythonNetworkStream()
{
    m_poSerCommandParserWnd.release();
}

std::string CPythonNetworkStream::GetLoginId() const
{
    return m_login;
}
