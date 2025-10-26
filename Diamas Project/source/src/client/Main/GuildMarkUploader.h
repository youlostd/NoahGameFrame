#ifndef METIN2_CLIENT_RUN_GUILDMARKUPLOADER_HPP
#define METIN2_CLIENT_RUN_GUILDMARKUPLOADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MarkImage.h"

#include <base/Singleton.hpp>

#include <il/il.h>

#include <net/Socket.hpp>
#include <net/AsyncClientSocketAdapter.hpp>
#include "game/GamePacket.hpp"

class CGuildMarkUploader : public AsyncClientSocketAdapter<Socket>
{
public:
    enum
    {
        ERROR_NONE,
        ERROR_CONNECT,
        ERROR_LOAD,
        ERROR_WIDTH,
        ERROR_HEIGHT,
    };

    enum
    {
        SEND_TYPE_MARK,
        SEND_TYPE_SYMBOL,
    };

public:
    CGuildMarkUploader(boost::asio::io_service &ioService);
    ~CGuildMarkUploader();

    void Disconnect();
    bool Connect(std::string addr, std::string port, uint32_t dwGuildID, const char *c_szFileName, uint32_t *peError);
    bool ConnectToSendSymbol(std::string addr, std::string port,
                             uint32_t dwGuildID, const char *c_szFileName, uint32_t *peError);
    bool IsCompleteUploading() const;
    bool HandlePacket(const PacketHeader&              header,
                      const boost::asio::const_buffer& data) override;

private:
    enum
    {
        STATE_OFFLINE,
        STATE_LOGIN,
        STATE_COMPLETE,
    };

private:
    void OnConnectFailure(const boost::system::error_code &ec) override;
    void OnConnectSuccess();
    void OnRemoteDisconnect();
    void OnDisconnect();

    bool __Load(const char *c_szFileName, uint32_t *peError);
    bool __LoadSymbol(const char *c_szFileName, uint32_t *peError);

    void __Inialize();
    bool __StateProcess();

    void __OfflineState_Set();
    void __CompleteState_Set();

    void __LoginState_Set();
    bool __LoginState_Process();
    bool __LoginState_RecvPhase(const TPacketGCPhase &p);
    bool __LoginState_RecvHandshake(const TPacketGCHandshake &p);
    bool __LoginState_RecvPing(const TPacketGCPing &p);


    bool __SendMarkPacket();
    bool __SendSymbolPacket();

private:
    uint32_t m_eState;

    uint32_t m_dwSendType;
    uint32_t m_dwGuildID;

    SGuildMark m_kMark;

    uint32_t m_dwSymbolBufSize;
    uint32_t m_dwSymbolCRC32;
    BYTE *m_pbySymbolBuf;
};
extern std::shared_ptr<CGuildMarkUploader> gGuildMarkUploader;

#endif
