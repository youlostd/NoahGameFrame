#ifndef METIN2_SERVER_MASTER_AUTHPACKETS_HPP
#define METIN2_SERVER_MASTER_AUTHPACKETS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/MasterPackets.hpp>


class AuthSocket;

bool HandleLoginPacket(AuthSocket* auth, const AmLoginPacket& p);


#endif
