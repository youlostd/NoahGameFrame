#ifndef METIN2_SERVER_AUTH_CLIENTUTIL_HPP
#define METIN2_SERVER_AUTH_CLIENTUTIL_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <game/Types.hpp>
#include <string>


struct CaHGuardInfoPacket;
class ClientSocket;

std::string GetHGuardHash(const CaHGuardInfoPacket& p, uint32_t accID);
std::string GetHWIDHash(const CaHGuardInfoPacket& p);

void SendPinRequestPacket(ClientSocket* client);
void SendSetPinRequestPacket(ClientSocket* client);
void SendHGuardRequestPacket(ClientSocket* client);
void SendHGuardResultPacket(ClientSocket* client, uint8_t code);


void SendAuthFailurePacket(ClientSocket* client,
                           const std::string& status);

void SendAuthSuccessPacket(ClientSocket* client,
                           SessionId sessionId);



#endif
