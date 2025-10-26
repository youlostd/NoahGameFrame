#ifndef METIN2_SERVER_GAME_MASTERPACKETS_HPP
#define METIN2_SERVER_GAME_MASTERPACKETS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/MasterPackets.hpp>



class MasterSocket;

bool HandleLoginSuccessPacket(MasterSocket* master,
                              const MaLoginSuccessPacket& p);
bool HandleLoginFailurePacket(MasterSocket* master,
                              const MaLoginFailurePacket& p);



#endif
