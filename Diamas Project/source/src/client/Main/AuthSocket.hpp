#ifndef METIN2_CLIENT_RUN_AUTHSOCKET_HPP
#define METIN2_CLIENT_RUN_AUTHSOCKET_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../ScriptLib/PythonUtils.h"

#include <net/Socket.hpp>
#include <net/AsyncClientSocketAdapter.hpp>

#include <memory>
#include <string>

class AuthSocket : public AsyncClientSocketAdapter<Socket>
{
public:
    AuthSocket(boost::asio::io_service &ioService);

    virtual void Disconnect() override final;

    py::object GetHandler() const
    {
        return m_handler;
    }

    void SetHandler(py::object handler)
    {
        m_handler = handler;
    }

    const std::string &GetLogin() const
    {
        return m_login;
    }

    void SetLoginInfo(const std::string &login,
                      const std::string &password);

    const std::string &GetCharAddr() const
    {
        return m_charAddr;
    }

    void SetCharAddr(const std::string &charAddr);

    const std::string &GetCharPort() const
    {
        return m_charPort;
    }

    void SetCharPort(const std::string &charPort);
    void SendHGuardCodePacket(const std::string &code);

    void SendHGuardInfoPacket();
    void SendPinResponsePacket(uint32_t code);
std::string GenerateHwid();
void SendAuthPacket();
void SendRegisterPacket(const std::string& login, const std::string& password,
                        const std::string& deleteCode,
                        const std::string& mailAdress);

protected:
    virtual void OnConnectSuccess() override final;
    virtual void OnConnectFailure(const boost::system::error_code &ec) override final;

    virtual bool HandlePacket(const PacketHeader &header,
                              const boost::asio::const_buffer &data) override final;

    py::object m_handler;

    std::string m_login;
    std::string m_password;

    std::string m_charAddr;
    std::string m_charPort;
};

extern std::shared_ptr<AuthSocket> gAuthSocket;

#endif
