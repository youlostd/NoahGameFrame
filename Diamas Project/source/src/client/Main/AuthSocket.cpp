#include "StdAfx.h"
#include "AuthSocket.hpp"
#include <game/GamePacket.hpp>
#include "PythonNetworkStream.h"

#include "HWIDManager.h"
#include "../EterBase/sha256.h"
#include <game/Constants.hpp>
#include "../EterBase/Timer.h"
#include "AuthPackets.hpp"
#include "game/AuthPackets.hpp"

#undef APP_VERSION
constexpr uint32_t APP_VERSION = 1621060937u;

std::shared_ptr<AuthSocket> gAuthSocket;

AuthSocket::AuthSocket(boost::asio::io_service &ioService)
    : AsyncClientSocketAdapter(ioService)
{
    // ctor
}

/*virtual*/
void AuthSocket::Disconnect()
{
    SPDLOG_INFO("AUTH: Connection {0} closed.", m_socket);

    AsyncClientSocketAdapter::Disconnect();
}

void AuthSocket::SetLoginInfo(const std::string &login,
                              const std::string &password)
{
    m_login = login;
    m_password = password;
}

void AuthSocket::SetCharAddr(const std::string &charAddr)
{
    m_charAddr = charAddr;
}

void AuthSocket::SetCharPort(const std::string &charPort)
{
    m_charPort = charPort;
}

void AuthSocket::SendHGuardCodePacket(const std::string &code)
{
    std::string authCode = code;

    CaHGuardCodePacket p;
    p.code = authCode;
    Send(kCaHGuardCode, p);
}

void AuthSocket::SendHGuardInfoPacket()
{
    CaHGuardInfoPacket p;
    p.macAddr = HWIDMANAGER::getMacAddr();
    p.guid = HWIDMANAGER::getMachineGUID();
    p.cpuId = HWIDMANAGER::getCPUid();

    Send(kCaHGuardInfo, p);
}

void AuthSocket::SendPinResponsePacket(uint32_t code)
{
    const CaPinResponsePacket p = {code};
    Send(kCaPinResponse, p);
}

std::string AuthSocket::GenerateHwid()
{
    
    std::string chain;
    chain.append(HWIDMANAGER::getCPUid())
         .append(HWIDMANAGER::getMachineGUID())
         .append(HWIDMANAGER::getMacAddr())
         .append("K,u$#<W|k(T0=;d%Tt!k");

    return  sha256(chain);
}

void AuthSocket::SendAuthPacket()
{
    CaAuthPacket p;
    p.login = m_login;
    p.password = m_password;
    p.version = APP_VERSION;
    p.hwid = GenerateHwid();
    Send(kCaAuth, p);
}

void AuthSocket::SendRegisterPacket(const std::string& login,
                                    const std::string& password,
                                    const std::string& deleteCode,
                                    const std::string& mailAdress)
{
    CaRegisterPacket p;
    p.login = login;
    p.password = password;
    p.deleteCode = deleteCode;
    p.mailAdress = mailAdress;
    p.hwid = GenerateHwid();
    Send(kCaRegisterRequest, p);
}
/*virtual*/
void AuthSocket::OnConnectSuccess()
{
    SPDLOG_INFO("AUTH: Successfully connected {0}", m_socket);

    if (m_handler)
        PyCallClassMemberFunc(m_handler, "OnConnectSuccess");

    StartReadSome();
}

/*virtual*/
void AuthSocket::OnConnectFailure(const boost::system::error_code &ec)
{
    SPDLOG_ERROR("AUTH: Failed to connect to {0}:{1} with {2}",
                  m_host, m_service, ec.message());

    if (m_handler)
        PyCallClassMemberFunc(m_handler, "OnConnectFailure");
}

/*virtual*/
bool AuthSocket::HandlePacket(const PacketHeader &header,
                              const boost::asio::const_buffer &data)
{
#define HANDLE_PACKET(id, fn, type) \
	case id: return fn(this, ReadPacket<type>(data))

    switch (header.id)
    {
    HANDLE_PACKET(kAcAuthFailure, HandleAuthFailurePacket,
                      AcAuthFailurePacket);
    HANDLE_PACKET(kAcPinRequest, HandlePinRequestPacket,
                      AcRequestPacket);
    HANDLE_PACKET(kAcSetPinRequest, HandleSetPinRequestPacket,
                      AcRequestPacket);

    HANDLE_PACKET(kAcAuthSuccess, HandleAuthSuccessPacket,
                      AcAuthSuccessPacket);
    HANDLE_PACKET(kAcHGuardRequest, HandleHGuardRequest,
                      AcRequestPacket);
    HANDLE_PACKET(kAcHGuardResult, HandleHGuardResult,
                      AcHGuardResultPacket);

    default:
        SPDLOG_ERROR("Unhandled packet: {0}", header.id);
        return false;
    }

#undef HANDLE_PACKET
}

void init_auth(py::module &m)
{
    py::module auth = m.def_submodule("auth", "Provides access to the auth socket");

    py::class_<AuthSocket, std::shared_ptr<AuthSocket>>(auth, "authSocket")
        .def(py::init([]()
        {
            return gAuthSocket;
        }), pybind11::return_value_policy::reference_internal)
        .def("SendHGuardCodePacket", &AuthSocket::SendHGuardCodePacket)
        .def("SendPinResponsePacket", &AuthSocket::SendPinResponsePacket)
        .def("Disconnect", &AuthSocket::Disconnect)
        .def("GetLogin", &AuthSocket::GetLogin)
        .def("GetCharAddr", &AuthSocket::GetCharAddr)
        .def("GetCharPort", &AuthSocket::GetCharPort)
        .def("GetHandler", &AuthSocket::GetHandler)
        .def("SetHandler", &AuthSocket::SetHandler)
        .def("IsClosed", &AuthSocket::IsClosed)
        .def("SetCharAddr", &AuthSocket::SetCharAddr)
        .def("SetCharPort", &AuthSocket::SetCharPort)
        .def("SetLoginInfo", &AuthSocket::SetLoginInfo)
        .def("SendAuthPacket", &AuthSocket::SendAuthPacket)
        .def("SendRegisterPacket", &AuthSocket::SendRegisterPacket)
        .def("Connect", &AuthSocket::Connect);
}
