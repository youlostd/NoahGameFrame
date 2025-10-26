#ifndef METIN2_SERVER_GAME_DESC_H
#define METIN2_SERVER_GAME_DESC_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "input.h"
#include <net/SocketBase.hpp>

#include <boost/asio/ip/tcp.hpp>
#include "event.h"
#include <net/Socket.hpp>

#include <xxhash/xxhash.h>

class CInputProcessor;
class DESC;
class CHARACTER;

class DESC : public Socket
{
    friend EVENTFUNC(disconnect_event);

public:
    EVENTINFO(desc_event_info)
    {
        DESC *desc;
        EPhase phase;

        desc_event_info()
            : desc(nullptr), phase(PHASE_CLOSE)
        {
        }
    };

    DESC(asio::ip::tcp::socket socket, uint32_t handle, uint32_t handshake);

    void Setup();
    void SetDisconnectPhase(EPhase phase);

    void Disconnect() override;
    void SetDisconnectReason(const std::string &reason);
    void SetInputProcessor(int phase);

    void SetPhase(int phase);

    bool IsPhase(int phase) const { return m_iPhase == phase; }

    int GetPhase() const { return m_iPhase; }

    CInputProcessor *GetInputProcessor() const { return m_pInputProcessor; }

    void SetAccountTable(const TAccountTable &tab) { m_accountTable = tab; }

    TAccountTable &GetAccountTable() { return m_accountTable; }

    void BindCharacter(CHARACTER *ch);

    CHARACTER *GetCharacter() { return m_character; }

    uint32_t GetHandle() const { return m_dwHandle; }

    std::string GetHostName() const;

    // 핸드쉐이크 (시간 동기화)
    void SendHandshake(uint32_t dwCurTime, int32_t lNewDelta);
    bool HandshakeProcess(uint32_t dwTime, int32_t lDelta, bool bInfiniteRetry = false);
    bool IsHandshaking();

    uint32_t GetHandshake() const { return m_dwHandshake; }

    uint32_t GetClientTime();

    bool DelayedDisconnect(int iSec, std::optional<std::string> reason = std::nullopt, EPhase disconnectPhase = PHASE_CLOSE);
    void DisconnectOfSameLogin();

    void SetAdminMode();
    bool IsAdminMode(); // Handshake 에서 어드민 명령을 쓸수있나?

    void SetPong(bool b);
    bool IsPong();

    void SendLoginSuccessPacket();

    void Logout();
    bool HandlePacket(const PacketHeader &header, const boost::asio::const_buffer &data) override;

    uint32_t GetAid() const { return m_aid; }

    void SetAid(uint32_t aid) { m_aid = aid; }

    const std::string &GetLogin() const { return m_login; }

    void SetLogin(const std::string &login) { m_login = login; }

    const std::string &GetSocialId() const { return m_socialId; }

    void SetSocialId(const std::string &socialId) { m_socialId = socialId; }

    const uint32_t *GetPremiumTimes() const { return m_premiumTimes; }

    void SetPremiumTimes(const uint32_t *premiumTimes) { std::memcpy(m_premiumTimes, premiumTimes, sizeof(m_premiumTimes)); }

    void SetHWIDHash(std::string hash) { m_hwid = hash; }

    const std::string &GetHWIDHash() const { return m_hwid; }

    void SetDropsAllowed(bool v);

    bool GetDropsAllowed() const { return m_dropsAllowed; }
    uint64_t GetHwid() const { return XXH64(m_hwid.data(), m_hwid.size(), 0); }

    void SetHGuardHash(std::string hash) { m_hguardId = hash; }

    const std::string &GetHGuardHash() const { return m_hguardId; };

    const std::string &GetClientLocale() const { return m_clientLocale; }

    void SetClientLocale(const std::string &locale) { m_clientLocale = locale; }

protected:

    int m_iPhase;
    uint32_t m_dwHandle;

    CInputProcessor *m_pInputProcessor;
    CInputClose m_inputClose;
    CInputPhase m_inputPhase;
    CInputHandshake m_inputHandshake;
    CInputLogin m_inputLogin;
    CInputMain m_inputMain;
    CInputDead m_inputDead;

    uint32_t m_dwHandshake;
    uint32_t m_dwHandshakeSentTime;
    int m_iHandshakeRetry;
    uint32_t m_dwClientTime;
    bool m_bHandshaking;

    LPEVENT m_pkPingEvent;
    LPEVENT m_pkDisconnectEvent;

    bool m_bAdminMode; // Handshake 에서 어드민 명령을 쓸수있나?
    bool m_bPong;
    bool m_dropsAllowed = false;

    uint32_t m_aid;
    std::string m_login;
    std::string m_socialId;
    uint32_t m_premiumTimes[PREMIUM_MAX_NUM]{};
    TAccountTable m_accountTable{};

    std::string m_hwid;
    std::string m_hguardId;
    std::string m_clientLocale;
    std::string m_socketAddress;
    std::optional<std::string> m_disconnectReason;
    EPhase m_disconnectPhase = PHASE_CLOSE;
    CHARACTER *m_character;
};

using DescPtr = std::shared_ptr<DESC>;

#endif /* METIN2_SERVER_GAME_DESC_H */
