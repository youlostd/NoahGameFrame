#ifndef METIN2_SERVER_AUTH_MASTERUTIL_HPP
#define METIN2_SERVER_AUTH_MASTERUTIL_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#include <cstdint>
#include <string>


class MasterSocket;

void SendLoginPacket(MasterSocket* master, uint32_t handle, uint32_t aid,
                     const std::string& login,
                     const std::string& socialId,
                     const uint32_t* premiumTimes,
                     const std::string& hwid);




#endif
