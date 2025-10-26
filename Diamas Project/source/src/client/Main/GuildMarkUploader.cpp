#include "StdAfx.h"
#include "GuildMarkUploader.h"

#include <game/GamePacket.hpp>

#include <base/Crc32.hpp>

#include <storm/io/FileLoader.hpp>

std::shared_ptr<CGuildMarkUploader> gGuildMarkUploader;

CGuildMarkUploader::CGuildMarkUploader(boost::asio::io_service &ioService)
    : m_pbySymbolBuf(nullptr), AsyncClientSocketAdapter(ioService)
{
    __Inialize();
}

CGuildMarkUploader::~CGuildMarkUploader()
{
    __OfflineState_Set();
}

void CGuildMarkUploader::Disconnect()
{
    __OfflineState_Set();

    SocketBase::Disconnect();

}

bool CGuildMarkUploader::IsCompleteUploading() const
{
    return STATE_OFFLINE == m_eState;
}

bool CGuildMarkUploader::HandlePacket(const PacketHeader &header, const boost::asio::const_buffer &data)
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
    }



    return true;
}

#include <base/stb_image.h>
void ConvertBetweenBGRAandRGBA(unsigned char* input, int pixel_width,
    int pixel_height)
{
    auto data = input;
      for (uint32_t y = 0; y < pixel_height; y++) {
        uint32_t* row = reinterpret_cast<uint32_t*>(data);
        for (uint32_t x = 0; x < pixel_width; x++) {
          uint8_t* bgra = reinterpret_cast<uint8_t*>(row);
          uint8_t b = *bgra;
          *bgra = *(bgra + 2);
          *(bgra + 2) = b;
          row++;
        }
        data += pixel_width * 4;
      }
}
bool CGuildMarkUploader::__Load(const char *c_szFileName, uint32_t *peError)
{
    int height;
    int width;
    int bpp;
    struct free_deleter {
      void operator()(void *p) const { free(p); }
    };
    using c_ptr = std::unique_ptr<uint8_t[], free_deleter>;

    c_ptr tex_mem =
        c_ptr{stbi_load(c_szFileName, &width, &height, &bpp, STBI_rgb_alpha)};
    auto *tex_ptr = tex_mem.get();

    if (!tex_ptr) {
      *peError = ERROR_LOAD;
      return false;
    }

    if (width != SGuildMark::WIDTH) {
      *peError = ERROR_WIDTH;
      return false;
    }

    if (height != SGuildMark::HEIGHT) {
      *peError = ERROR_HEIGHT;
      return false;
    }
    ConvertBetweenBGRAandRGBA(tex_ptr, 16, 12);

    std::memcpy(m_kMark.m_apxBuf, tex_ptr, sizeof(m_kMark.m_apxBuf));
    return true;
}

bool CGuildMarkUploader::__LoadSymbol(const char *c_szFileName, uint32_t *peError)
{
    //	For Check Image
    ILuint uImg;
    ilGenImages(1, &uImg);
    ilBindImage(uImg);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
    if (!ilLoad(IL_TYPE_UNKNOWN, (const ILstring)c_szFileName))
    {
        *peError = ERROR_LOAD;
        return false;
    }
    if (ilGetInteger(IL_IMAGE_WIDTH) != 64)
    {
        *peError = ERROR_WIDTH;
        return false;
    }
    if (ilGetInteger(IL_IMAGE_HEIGHT) != 128)
    {
        *peError = ERROR_HEIGHT;
        return false;
    }
    ilDeleteImages(1, &uImg);
    ilShutDown();

    /////

    storm::FileLoader loader(storm::GetDefaultAllocator());

    bsys::error_code ec;
    loader.Load(c_szFileName, ec);

    if (ec)
    {
        SPDLOG_ERROR("Failed to load {0} with {1}", c_szFileName, ec.message());
        *peError = ERROR_LOAD;
        return false;
    }

    m_dwSymbolBufSize = loader.GetSize();
    m_pbySymbolBuf = new BYTE[m_dwSymbolBufSize];
    m_dwSymbolCRC32 = ComputeCrc32(0, loader.GetData(), loader.GetSize());

    std::memcpy(m_pbySymbolBuf, loader.GetData(), loader.GetSize());
    return true;
}

bool CGuildMarkUploader::Connect(std::string addr, std::string port, uint32_t dwGuildID, const char *c_szFileName,
                                 uint32_t *peError)
{
    __OfflineState_Set();


    if (!AsyncClientSocketAdapter::Connect(addr, port))
    {
        *peError = ERROR_CONNECT;
        return false;
    }

    m_dwSendType = SEND_TYPE_MARK;
    m_dwGuildID = dwGuildID;

    if (!__Load(c_szFileName, peError))
        return false;

    return true;
}

bool CGuildMarkUploader::ConnectToSendSymbol(std::string addr, std::string port, uint32_t dwGuildID,
                                             const char *c_szFileName, uint32_t *peError)
{
    __OfflineState_Set();

    if (!AsyncClientSocketAdapter::Connect(addr, port))
    {
        *peError = ERROR_CONNECT;
        return false;
    }

    m_dwSendType = SEND_TYPE_SYMBOL;
    m_dwGuildID = dwGuildID;

    if (!__LoadSymbol(c_szFileName, peError))
        return false;

    return true;
}

void CGuildMarkUploader::OnConnectFailure(const boost::system::error_code &ec)
{
    __OfflineState_Set();
}

void CGuildMarkUploader::OnConnectSuccess()
{
    __LoginState_Set();

    StartReadSome();

}

void CGuildMarkUploader::OnRemoteDisconnect()
{
    __OfflineState_Set();
}

void CGuildMarkUploader::OnDisconnect()
{
    __OfflineState_Set();
}

void CGuildMarkUploader::__Inialize()
{
    m_eState = STATE_OFFLINE;

    m_dwGuildID = 0;

    if (m_pbySymbolBuf)
    {
        delete m_pbySymbolBuf;
    }

    m_dwSymbolBufSize = 0;
    m_pbySymbolBuf = nullptr;
}

bool CGuildMarkUploader::__StateProcess()
{
    switch (m_eState)
    {
    case STATE_LOGIN:
        return __LoginState_Process();
    default:
        break;
    }

    return true;
}

void CGuildMarkUploader::__OfflineState_Set()
{
    __Inialize();
}

void CGuildMarkUploader::__CompleteState_Set()
{
    m_eState = STATE_COMPLETE;

    __OfflineState_Set();
}

void CGuildMarkUploader::__LoginState_Set()
{
    m_eState = STATE_LOGIN;
}

bool CGuildMarkUploader::__LoginState_Process()
{


    return true;
}

bool CGuildMarkUploader::__SendMarkPacket()
{
    TPacketCGMarkUpload kPacketMarkUpload;
    kPacketMarkUpload.gid = m_dwGuildID;

    assert(sizeof(kPacketMarkUpload.image) == sizeof(m_kMark.m_apxBuf));
    memcpy(kPacketMarkUpload.image, m_kMark.m_apxBuf, sizeof(kPacketMarkUpload.image));

    Send(HEADER_CG_MARK_UPLOAD, kPacketMarkUpload);

    return true;
}

bool CGuildMarkUploader::__SendSymbolPacket()
{
    if (!m_pbySymbolBuf)
        return false;

    TPacketCGGuildSymbolUpload kPacketSymbolUpload;
    kPacketSymbolUpload.guild_id = m_dwGuildID;
    std::memcpy(kPacketSymbolUpload.data.data(), m_pbySymbolBuf, m_dwSymbolBufSize);
    Send(HEADER_CG_GUILD_SYMBOL_UPLOAD, kPacketSymbolUpload);

    __CompleteState_Set();

    return true;
}

bool CGuildMarkUploader::__LoginState_RecvPhase(const TPacketGCPhase& p)
{
    CgPhaseAckPacket ack;
    Send(HEADER_CG_PHASE_ACK, ack);

    if (p.phase == PHASE_LOGIN)
    {
        if (SEND_TYPE_MARK == m_dwSendType)
        {
            if (!__SendMarkPacket())
                return true;
        }
        else if (SEND_TYPE_SYMBOL == m_dwSendType)
        {
            if (!__SendSymbolPacket())
                return true;
        }
    }

    return true;
}

bool CGuildMarkUploader::__LoginState_RecvHandshake(const TPacketGCHandshake& p)
{
    TPacketCGMarkLogin kPacketMarkLogin;
    Send(HEADER_CG_MARK_LOGIN, kPacketMarkLogin);
    return true;
}

bool CGuildMarkUploader::__LoginState_RecvPing(const TPacketGCPing& p)
{
    TPacketCGPong kPacketPong;

    Send(HEADER_CG_PONG, kPacketPong);
    return true;
}
