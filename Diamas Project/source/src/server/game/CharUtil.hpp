#ifndef METIN2_SERVER_GAME_CHARUTIL_HPP
#define METIN2_SERVER_GAME_CHARUTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Constants.hpp>
#include <game/Constants.hpp>
#include <game/ItemConstants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "entity.h"
#include "locale.hpp"
#include "net/Type.hpp"
class CHARACTER;
struct Locale;
#include <fmt/printf.h>
#include <game/Types.hpp>

bool Unmount(CHARACTER *ch);
bool Mount(CHARACTER *ch);
const Locale *GetLocale(CHARACTER *ch);
const std::string &GetLocaleCode(CHARACTER *ch);
void SendChatPacket(CHARACTER *ch, uint8_t type, const std::string &msg);
void SendSystemWhisperPacket(CHARACTER *ch, const std::string &msg);
void SendItemPickupInfo(CHARACTER *ch, ItemVnum vnum, CountType count);

template <typename ...Args>
void SendI18nSystemWhisperPacket(CHARACTER *ch, const char *format, Args ... args)
{
    const auto locale = GetLocale(ch);
    std::string resultString;
    try { resultString = fmt::sprintf(locale->stringTable.Translate(format), std::forward<Args>(args)...); }
    catch (const fmt::format_error &err) { resultString = locale->stringTable.Translate(format); }
    SendSystemWhisperPacket(ch, resultString);
}

template <typename ...Args>
void SendI18nChatPacket(CHARACTER *ch, uint8_t type, const char *format, Args ... args)
{
    const auto locale = GetLocale(ch);
    std::string resultString;
    try { resultString = fmt::sprintf(locale->stringTable.Translate(format), std::forward<Args>(args)...); }
    catch (const fmt::format_error &err) { resultString = locale->stringTable.Translate(format); }
    SendChatPacket(ch, type, resultString);
}

void SendSpecialI18nChatPacket(CHARACTER *ch, uint8_t type, const std::string &format);
void SendSpecialI18nOXPacket(CHARACTER *ch, uint8_t type, const std::string &format);

void EffectPacketByPointType(CHARACTER *ch, uint32_t pointType);

void DisconnectByHwid(const std::string &hwid);

bool IsDuelMap(int32_t mapIndex);
bool IsPvmMap(int32_t mapIndex);
void MapLevelChecks(CHARACTER *ch);
void CleanUpShopOwners(CHARACTER *shopOwner);

bool CAN_ENTER_ZONE(const CHARACTER *ch, int map_index);

bool IsMountableZone(int map_index, bool isHorse);
bool IsLevelPetUsableZone(int map_index);
bool IsPetUsableZone(int map_index);


#endif
