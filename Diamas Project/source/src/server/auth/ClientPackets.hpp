#ifndef METIN2_SERVER_AUTH_CLIENTPACKETS_HPP
#define METIN2_SERVER_AUTH_CLIENTPACKETS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/AuthPackets.hpp>



class ClientSocket;

bool HandleAuthPacket(ClientSocket* client, const CaAuthPacket& p);
bool HandleRegisterPacket(ClientSocket* client, const CaRegisterPacket& p);
bool HandleHGuardInfoPacket(ClientSocket* client, const CaHGuardInfoPacket& p);
bool HandleHGuardCodePacket(ClientSocket* client, const CaHGuardCodePacket& p);

bool HandlePinResponsePacket(ClientSocket* client,
                             const CaPinResponsePacket& p);



#endif
