#ifndef METIN2_CLIENT_RUN_GUILDMARKDOWNLOADER_HPP
#define METIN2_CLIENT_RUN_GUILDMARKDOWNLOADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MarkManager.h"

#include <base/Singleton.hpp>
#include <net/AsyncClientSocketAdapter.hpp>
#include <net/Socket.hpp>

#include "game/GamePacket.hpp"

class CGuildMarkDownloader : public AsyncClientSocketAdapter<Socket>
{
public:
    CGuildMarkDownloader(boost::asio::io_service &ioService);
    virtual ~CGuildMarkDownloader();

    bool Connect(const std::string& ip, const std::string& port);
    void Disconnect();
    bool ConnectToRecvSymbol(const std::string&            ip, const std::string& port,
                             const std::vector<uint32_t> & c_rkVec_dwGuildID);
    virtual bool HandlePacket(const PacketHeader& header,
		                          const boost::asio::const_buffer& data);
    void Process();

private:
    enum
    {
        STATE_OFFLINE,
        STATE_LOGIN,
        STATE_COMPLETE,
    };

    enum
    {
        TODO_RECV_NONE,
        TODO_RECV_MARK,
        TODO_RECV_SYMBOL,
    };

private:
    void OnConnectFailure(const boost::system::error_code &ec);
    void OnConnectSuccess();
    void OnRemoteDisconnect();
    void OnDisconnect();

    void __Initialize();
    bool __StateProcess();

    uint32_t __GetPacketSize(uint32_t header);

    void __OfflineState_Set();
    void __CompleteState_Set();

    void __LoginState_Set();
    bool __LoginState_Process();
    bool __LoginState_RecvPhase(const TPacketGCPhase &p);
    bool __LoginState_RecvHandshake(const TPacketGCHandshake &p);
    bool __LoginState_RecvPing(const TPacketGCPing &p);
    bool __LoginState_RecvMarkIndex(const TPacketGCMarkIDXList& p);
    bool __LoginState_RecvMarkBlock(const TPacketGCMarkBlock &kPacket);
    bool __LoginState_RecvSymbolData(const TPacketGCGuildSymbolData &kPacketSymbolData);

    bool __SendMarkIDXList();
    bool __SendMarkCRCList();
    bool __SendSymbolCRCList();

private:
    uint32_t m_dwTodo;

    std::vector<uint32_t> m_kVec_dwGuildID;

    uint32_t m_eState;

    uint8_t m_currentRequestingImageIndex;

    CGuildMarkManager *m_pkMarkMgr;

    uint32_t m_dwBlockIndex;
    uint32_t m_dwBlockDataSize;
    uint32_t m_dwBlockDataPos;
};
extern std::shared_ptr<CGuildMarkDownloader> gGuildMarkDownloader;
#endif
