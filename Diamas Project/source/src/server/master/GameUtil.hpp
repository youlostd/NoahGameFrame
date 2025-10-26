#ifndef METIN2_SERVER_MASTER_GAMEUTIL_HPP
#define METIN2_SERVER_MASTER_GAMEUTIL_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



class GameSocket;

void SendLoginSuccessPacket(GameSocket* game, uint32_t handle, uint32_t aid,
                            const std::string& socialId,
                            const uint32_t* premiumTimes,
                            const std::string& hwid);

void SendLoginFailurePacket(GameSocket* game, uint32_t handle,
                            const std::string& status);

void SendLoginDisconnectPacket(GameSocket* game, uint32_t aid);


#endif
