#include "StdAfx.h"

#include "AuthPackets.hpp"
#include "AuthSocket.hpp"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include <game/AuthPackets.hpp>

bool HandleSetPinRequestPacket(AuthSocket *auth, const AcRequestPacket &p)
{
    spdlog::trace("AUTH: Received an SetPIN Request");

    if (auth->GetHandler())
        PyCallClassMemberFunc(auth->GetHandler(), "OnPinSetRequest");
    return true;
}

bool HandleHGuardRequest(AuthSocket *auth, const AcRequestPacket &p)
{
    auth->SendHGuardInfoPacket();
    return true;
}

bool HandleHGuardResult(AuthSocket *auth, const AcHGuardResultPacket &p)
{
    switch (p.code)
    {
    case AcHGuardResultPacket::kRequireCode:
        PyCallClassMemberFunc(auth->GetHandler(), "OnHGuardPrompt");
        break;

    case AcHGuardResultPacket::kIncorrectCode:
        PyCallClassMemberFunc(auth->GetHandler(), "OnHGuardFailure");
        break;
    default:
        break;
    }

    return true;
}

bool HandlePinRequestPacket(AuthSocket *auth, const AcRequestPacket &p)
{
    spdlog::trace("AUTH: Received an PIN Request");

    if (auth->GetHandler())
        PyCallClassMemberFunc(auth->GetHandler(), "OnPinRequest");
    return true;
}

bool HandleAuthFailurePacket(AuthSocket *auth, const AcAuthFailurePacket &p)
{
    std::string status = p.status;

#ifdef _DEBUG
    SPDLOG_ERROR("AUTH: Authentication failed with {0}", status);
#endif

    auto pystring = py::bytes(status);
    if (auth->GetHandler())
        PyCallClassMemberFunc(auth->GetHandler(), "OnLoginFailure", pystring);

    auth->Disconnect();
    return true;
}

bool HandleAuthSuccessPacket(AuthSocket *auth, const AcAuthSuccessPacket &p)
{
    spdlog::trace("AUTH: Succeeded with session {0}", p.sessionId);

    gPythonNetworkStream->SetLogin(auth->GetLogin());
    gPythonNetworkStream->SetSessionId(p.sessionId);

    gPythonNetworkStream->Connect(auth->GetCharAddr(), fmt::to_string(auth->GetCharPort()));

    auth->Disconnect();
    return true;
}
