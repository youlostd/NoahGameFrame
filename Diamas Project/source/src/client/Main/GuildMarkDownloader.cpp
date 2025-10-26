#include "StdAfx.h"
#include "GuildMarkDownloader.h"
#include "PythonCharacterManager.h"

#include "../EterLib/ResourceManager.h"
#include "../EterLib/GrpImage.h"

#include "../EterBase/Utils.h"

#include <game/GamePacket.hpp>

#include <base/Crc32.hpp>

#include <storm/io/FileLoader.hpp>

std::shared_ptr<CGuildMarkDownloader> gGuildMarkDownloader;
// MARK_BUG_FIX
struct SMarkIndex
{
    uint32_t guild_id;
    uint32_t mark_id;
};

// END_OFMARK_BUG_FIX

CGuildMarkDownloader::CGuildMarkDownloader(boost::asio::io_service &ioService) : AsyncClientSocketAdapter(ioService)
{
    __Initialize();
}

CGuildMarkDownloader::~CGuildMarkDownloader()
{
    __OfflineState_Set();
}

bool CGuildMarkDownloader::Connect(const std::string& ip, const std::string& port)
{
    __OfflineState_Set();

    m_dwTodo = TODO_RECV_MARK;
    return AsyncClientSocketAdapter::Connect(ip, port);
}

void CGuildMarkDownloader::Disconnect()
{
    SPDLOG_INFO("MARK: Connection {0} closed.", m_socket);

    SocketBase::Disconnect();

}
bool CGuildMarkDownloader::ConnectToRecvSymbol(const std::string& ip, const std::string& port,
                                               const std::vector<uint32_t> &c_rkVec_dwGuildID)
{
    __OfflineState_Set();

    m_dwTodo = TODO_RECV_SYMBOL;
    m_kVec_dwGuildID = c_rkVec_dwGuildID;
    return AsyncClientSocketAdapter::Connect(ip, port);
}



void CGuildMarkDownloader::OnConnectFailure(const boost::system::error_code &ec)
{
    __OfflineState_Set();
}

void CGuildMarkDownloader::OnConnectSuccess()
{
    __LoginState_Set();

    StartReadSome();
}

void CGuildMarkDownloader::OnRemoteDisconnect()
{
    __OfflineState_Set();
}

void CGuildMarkDownloader::OnDisconnect()
{
    //__OfflineState_Set();
}

void CGuildMarkDownloader::__Initialize()
{
    m_eState = STATE_OFFLINE;
    m_pkMarkMgr = NULL;
    m_currentRequestingImageIndex = 0;
    m_dwBlockIndex = 0;
    m_dwBlockDataPos = 0;
    m_dwBlockDataSize = 0;

    m_dwTodo = TODO_RECV_NONE;
    m_kVec_dwGuildID.clear();
}

bool CGuildMarkDownloader::__StateProcess()
{
    switch (m_eState)
    {
    case STATE_LOGIN:
        return __LoginState_Process();
    case STATE_COMPLETE:
        return false;
    default:
        break;
    }

    return true;
}

void CGuildMarkDownloader::__OfflineState_Set()
{
    __Initialize();
}

void CGuildMarkDownloader::__CompleteState_Set()
{
    m_eState = STATE_COMPLETE;
    CPythonCharacterManager::instance().RefreshAllGuildMark();
    Disconnect();
}

void CGuildMarkDownloader::__LoginState_Set()
{
    m_eState = STATE_LOGIN;
}

bool CGuildMarkDownloader::__LoginState_Process()
{

    return true;
}

// MARK_BUG_FIX
uint32_t CGuildMarkDownloader::__GetPacketSize(uint32_t header)
{
    switch (header)
    {
    case HEADER_GC_PHASE:
        return sizeof(TPacketGCPhase);
    case HEADER_GC_HANDSHAKE:
        return sizeof(TPacketGCHandshake);
    case HEADER_GC_PING:
        return sizeof(TPacketGCPing);
    case HEADER_GC_MARK_IDXLIST:
        return sizeof(TPacketGCMarkIDXList);
    case HEADER_GC_MARK_BLOCK:
        return sizeof(TPacketGCMarkBlock);
    case HEADER_GC_SYMBOL_DATA:
        return sizeof(TPacketGCGuildSymbolData);
    default:
        break;
    }
    return 0;
}

bool CGuildMarkDownloader::HandlePacket(const PacketHeader &header, const boost::asio::const_buffer &data)
{
      #define HANDLE_PACKET(id, fn, type) \
	case id: return this->fn(ReadPacket<type>(data))

    switch (header.id)
    {
    HANDLE_PACKET(HEADER_GC_PHASE, __LoginState_RecvPhase,
                      TPacketGCPhase);
    HANDLE_PACKET(HEADER_GC_HANDSHAKE, __LoginState_RecvHandshake,
                      TPacketGCHandshake);
    HANDLE_PACKET(HEADER_GC_PING, __LoginState_RecvPing,
                  TPacketGCPing);
    HANDLE_PACKET(HEADER_GC_SYMBOL_DATA, __LoginState_RecvSymbolData,
          TPacketGCGuildSymbolData);
    HANDLE_PACKET(HEADER_GC_MARK_IDXLIST, __LoginState_RecvMarkIndex,
          TPacketGCMarkIDXList);
    HANDLE_PACKET(HEADER_GC_MARK_BLOCK, __LoginState_RecvMarkBlock,
          TPacketGCMarkBlock);
    }



    return true;
}


// END_OF_MARK_BUG_FIX

bool CGuildMarkDownloader::__LoginState_RecvHandshake(const TPacketGCHandshake& p)
{
    TPacketCGMarkLogin kPacketMarkLogin;
    Send(HEADER_CG_MARK_LOGIN, kPacketMarkLogin);

    return true;
}

bool CGuildMarkDownloader::__LoginState_RecvPing(const TPacketGCPing& p)
{
    TPacketCGPong p2;
    Send(HEADER_CG_PONG, p2);
    return true;
}

bool CGuildMarkDownloader::__LoginState_RecvPhase(const TPacketGCPhase& p)
{

    CgPhaseAckPacket ack;
    Send(HEADER_CG_PHASE_ACK, ack);

    if (p.phase == PHASE_LOGIN)
    {
        switch (m_dwTodo)
        {
        case TODO_RECV_NONE: {
            assert(!"CGuildMarkDownloader::__LoginState_RecvPhase - Todo type is none");
            break;
        }
        case TODO_RECV_MARK: {
            // MARK_BUG_FIX
            if (!__SendMarkIDXList())
                return true;
            // END_OF_MARK_BUG_FIX
            break;
        }
        case TODO_RECV_SYMBOL: {
            if (!__SendSymbolCRCList())
                return true;
            break;
        }
        default:
            break;
        }
    }

    return true;
}

// MARK_BUG_FIX
bool CGuildMarkDownloader::__SendMarkIDXList()
{
    TPacketCGMarkIDXList kPacketMarkIDXList;
    Send(HEADER_CG_MARK_IDXLIST, kPacketMarkIDXList);
    return true;
}

bool CGuildMarkDownloader::__LoginState_RecvMarkIndex(const TPacketGCMarkIDXList& p)
{

    for(const auto& elem : p.indices)
    {
        CGuildMarkManager::Instance().AddMarkIDByGuildID(elem.guildId, elem.markIdx);
    }


    // 모든 마크 이미지 파일을 로드한다. (파일이 없으면 만들어짐)
    CGuildMarkManager::Instance().LoadMarkImages();

    m_currentRequestingImageIndex = 0;
    __SendMarkCRCList();
    return true;
}

bool CGuildMarkDownloader::__SendMarkCRCList()
{
    TPacketCGMarkCRCList kPacketMarkCRCList;

    if (!CGuildMarkManager::Instance().GetBlockCRCList(m_currentRequestingImageIndex, kPacketMarkCRCList.crclist))
        __CompleteState_Set();
    else
    {
        kPacketMarkCRCList.imgIdx = m_currentRequestingImageIndex;
        ++m_currentRequestingImageIndex;

        Send(HEADER_CG_MARK_CRCLIST, kPacketMarkCRCList);
    }
    return true;
}

bool CGuildMarkDownloader::__LoginState_RecvMarkBlock(const TPacketGCMarkBlock& p)
{
    for (const auto &block : p.blocks)
    {
        // 압축된 이미지를 실제로 저장한다. CRC등 여러가지 정보가 함께 빌드된다.
        CGuildMarkManager::Instance().SaveBlockFromCompressedData(
            p.imgIdx, block.first, (const uint8_t *)block.second.data(), block.second.size());
    }

    if (p.blocks.size() > 0)
    {
        // 마크 이미지 저장
        CGuildMarkManager::Instance().SaveMarkImage(p.imgIdx);

        // 리소스 리로딩 (재접속을 안해도 본인것은 잘 보이게 함)
        std::string imagePath;

        if (CGuildMarkManager::Instance().GetMarkImageFilename(p.imgIdx, imagePath))
        {
            auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(imagePath);
            if (r)
                r->Reload();
        }
    }

    // 더 요청할 것이 있으면 요청하고 아니면 이미지를 저장하고 종료
    if (m_currentRequestingImageIndex < CGuildMarkManager::Instance().GetMarkImageCount())
        __SendMarkCRCList();
    else
        __CompleteState_Set();

    return true;
}

// END_OF_MARK_BUG_FIX

bool CGuildMarkDownloader::__SendSymbolCRCList()
{
    for (unsigned long i : m_kVec_dwGuildID)
    {
        TPacketCGSymbolCRC kSymbolCRCPacket;
        kSymbolCRCPacket.guild_id = i;

        std::string strFileName = GetGuildSymbolFileName(i);
        storm::FileLoader loader(storm::GetDefaultAllocator());

        bsys::error_code ec;
        loader.Load(strFileName, ec);

        if (ec)
        {
            SPDLOG_ERROR("Failed to load {0} with {1}", strFileName, ec.message());
            return false;
        }

        kSymbolCRCPacket.crc = ComputeCrc32(0, loader.GetData(), loader.GetSize());
        kSymbolCRCPacket.size = loader.GetSize();

        Send(HEADER_CG_SYMBOL_CRC, kSymbolCRCPacket);
        
    }

    return true;
}

bool CGuildMarkDownloader::__LoginState_RecvSymbolData(const TPacketGCGuildSymbolData& kPacketSymbolData)
{
    uint32_t dwGuildID = kPacketSymbolData.guild_id;

    MyCreateDirectory(g_strGuildSymbolPathName.c_str());

    const std::string strFileName = GetGuildSymbolFileName(dwGuildID);

    boost::system::error_code ec;
    storm::File fp;
    fp.Open(strFileName.c_str(), ec, storm::AccessMode::kWrite);
    if (!fp.IsOpen())
    {
        return false;
    }
    fp.Write( kPacketSymbolData.data.data(), kPacketSymbolData.data.size(), ec);

    return true;
}

METIN2_END_NS
