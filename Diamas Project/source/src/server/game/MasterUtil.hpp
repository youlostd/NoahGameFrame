#ifndef METIN2_SERVER_GAME_MASTERUTIL_HPP
#define METIN2_SERVER_GAME_MASTERUTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <cstdint>
#include <string_view>
#include <game/Types.hpp>

void SendLoginPacket(uint32_t handle,
                     std::string_view login,
                     SessionId sessionId);

void SendLogoutPacket(uint32_t aid);

void RelayDisconnect(uint32_t pid);

void RelayChatPacket(uint32_t pid, uint8_t type,
                     std::string_view message);

void RelayWhisperPacket(uint32_t sourcePid, uint32_t targetPid, uint8_t type, uint8_t sourceLc, 
                     const std::string& from, const std::string& message);

#endif
