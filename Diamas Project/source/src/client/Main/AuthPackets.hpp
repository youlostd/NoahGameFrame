#ifndef METIN2_CLIENT_RUN_AUTHPACKETS_HPP
#define METIN2_CLIENT_RUN_AUTHPACKETS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

struct AcHGuardResultPacket;
struct AcRequestPacket;
struct AcAuthFailurePacket;
struct AcAuthSuccessPacket;

class AuthSocket;

bool HandlePinRequestPacket(AuthSocket *auth, const AcRequestPacket &p);
bool HandleSetPinRequestPacket(AuthSocket *auth, const AcRequestPacket &p);
bool HandleHGuardRequest(AuthSocket *auth, const AcRequestPacket &p);
bool HandleHGuardResult(AuthSocket *auth, const AcHGuardResultPacket &p);

bool HandleAuthFailurePacket(AuthSocket *auth, const AcAuthFailurePacket &p);
bool HandleAuthSuccessPacket(AuthSocket *auth, const AcAuthSuccessPacket &p);

#endif
