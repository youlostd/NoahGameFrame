#include "CharUtil.hpp"
#include "char.h"
#include "desc.h"
#include "item.h"
#include "ItemUtils.h"

#include "desc_manager.h"
#include "char_manager.h"

#if defined(WJ_COMBAT_ZONE)
#include "combat_zone.h"
#endif

#include <game/MotionConstants.hpp>
#include <game/GamePacket.hpp>
#include <storm/Tokenizer.hpp>


#include "GArena.h"
#include "war_map.h"
#include <fmt/args.h>
#include <absl/strings/string_view.h>
#include <absl/strings/str_split.h>

const Locale *GetLocale(CHARACTER *ch)
{
    auto locale = GetLocaleService().GetLocale(ch->GetClientLocale());
    if (!locale)
        locale = &GetLocaleService().GetDefaultLocale();

    return locale;
}

bool Unmount(CHARACTER *ch)
{
    if (!ch->IsPC())
        return false;

    auto *mountItem = FindToggleItem(ch, true, TOGGLE_MOUNT);
    if (mountItem && mountItem->GetSocket(ITEM_SOCKET_TOGGLE_RIDING))
    {
        mountItem->SetSocket(ITEM_SOCKET_TOGGLE_RIDING, 0);
        ch->PointChange(POINT_ST, 0);
        ch->PointChange(POINT_DX, 0);
        ch->PointChange(POINT_HT, 0);
        ch->PointChange(POINT_IQ, 0);

        return true;
    }

    if (ch->IsHorseRiding())
    {
        ch->StopRiding();
        return true;
    }

    return false;
}


bool Mount(CHARACTER *ch)
{
    if (ch->GetHorse())
    {
        ch->StartRiding();
        return true;
    }

    if (ch->GetMotionMode() == MOTION_MODE_WEDDING_DRESS)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "CANNOT_RIDE_WHILE_WEARING_DRESS");
        return false;
    }

    if (!IsMountableZone(ch->GetMapIndex(), false))
        return false;


    if(get_dword_time() - ch->GetLastPlayerHitReceivedTime() < gConfig.portalLimitTime * 1000)
        return false;

    if(get_dword_time() - ch->GetLastPlayerHitTime() < gConfig.portalLimitTime * 1000)
        return false;

    if (ch->IsPolymorphed())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_RIDE_WHILE_POLYMORPHED");
        return false;
    }

#if defined(WJ_COMBAT_ZONE)
	if (CCombatZoneManager::Instance().IsCombatZoneMap(ch->GetMapIndex())) {
		SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_RIDE_IN_COMBAT_ZONE");
		return false;
	}
#endif

    auto *item = FindToggleItem(ch, true, TOGGLE_MOUNT);
    if (item)
    {
        if (!CanUseItem(ch, item))
            return false;

        item->SetSocket(ITEM_SOCKET_TOGGLE_RIDING, 1);

        return true;
    }

    auto *newMountItem = FindToggleItem(ch, false, TOGGLE_MOUNT);

    if (newMountItem)
    {
        if (!CanUseItem(ch, newMountItem))
            return false;

        if (!events::Item::OnUse(ch, newMountItem))
            return false;

        ActivateToggleItem(ch, newMountItem);
        newMountItem->SetSocket(ITEM_SOCKET_TOGGLE_RIDING, 1);
        return true;
    }

    return false;
}

void SendSystemWhisperPacket(CHARACTER *ch, const std::string &msg)
{
    DESC *d = ch->GetDesc();
    if (!d)
        return;

    TPacketGCWhisper pack;
    pack.bType = WHISPER_TYPE_SYSTEM;
    pack.szNameFrom = "<[ Information ]>";
    pack.message = msg;
    d->Send(HEADER_GC_WHISPER, pack);
}

void SendItemPickupInfo(CHARACTER *ch, ItemVnum vnum, CountType count)
{
    GcItemPickupInfoPacket p;
    p.vnum = vnum;
    p.count = count;
    ch->GetDesc()->Send(HEADER_GC_PICKUP_INFO, p);
}

void SendChatPacket(CHARACTER *ch, uint8_t type, const std::string &msg)
{
    DESC *d = ch->GetDesc();
    if (!d)
        return;

    TPacketGCChat pack;
    pack.type = type;
    pack.id = 0;
    pack.bEmpire = ch->GetEmpire();
    pack.message = msg;
    d->Send(HEADER_GC_CHAT, pack);
}

void DisconnectByHwid(const std::string &hwid)
{
    const auto &descs = DESC_MANAGER::instance().GetClientSet();
    std::for_each(descs.begin(), descs.end(), [&](DescPtr d)
    {
        if (d->GetHWIDHash() == hwid)
        {
            if (!d->GetCharacter())
            {
                d->DelayedDisconnect(0, "ADMIN_DISCONNECT");
                return;
            }
            else { d->DelayedDisconnect(5, "ADMIN_DISCONNECT"); }
        }
    });
}

bool IsTier4Map(int32_t mapIndex)
{
    if (mapIndex >= 10000)
        mapIndex /= 10000;
    /*
        switch(mapIndex) {
        case 507:

        case 508:
        case 509:
        case 510:
        case 511:
        case 512:
        case 513:
        case 514:
            return true;

        default: break;
        }
    */
    return false;
}

bool IsNonTier4Map(int32_t mapIndex)
{
    if (mapIndex >= 10000)
        mapIndex /= 10000;
    /*
        switch(mapIndex) {
        case 507:
        case 508:
        case 509:
        case 510:
        case 511:
        case 512:
        case 513:
        case 514:
            return false;

        default: break;
        }
    */
    return true;
}

bool IsDuelMap(int32_t mapIndex)
{
    if (mapIndex >= 10000)
        mapIndex /= 10000;

    /*
        switch(mapIndex) {
        case 75:
        case 74:
        case 72:
        case 505:
        case 112:
        case 515:
        case 63:
        case 531:
        case 532:
            return true;

        default: break;
        }
    */
    return false;
}

bool IsPvmMap(int32_t mapIndex)
{
    if (mapIndex >= 10000)
        mapIndex /= 10000;
    /*
        switch(mapIndex) {
        case 507:
        case 508:
        case 509:
        case 510:
        case 511:
        case 512:
        case 513:
        case 514:
        case 216:
        case 301:
        case 303:
        case 351:
        case 352:
        case 353:
        case 402:
        case 403:
        case 404:
        case 501:
        case 502:
        case 503:
        case 504:
        case 551:
        case 574:
        case 571:
        case 572:
        case 66:
        case 65:
        case 67:
        case 71:
        case 5:
        case 25:
        case 45:
        case 107:
        case 108:
        case 109:
        case 215:
            return true;

        default: break;
        }
    */
    return false;
}

void SendSpecialI18nOXPacket(CHARACTER *ch, uint8_t type, const std::string &format)
{
    const auto* locale = GetLocale(ch);

    std::vector<std::string> result;
    storm::Tokenize(format, "#", result);
    if (result.size() > 1)
    {
        const auto        formatMessage     = result[0];
        const auto* const translatedMessage = locale->oxTable.Translate(formatMessage);

        try
        {
            const auto& argumentString = result[1];

            std::vector<std::string> arguments = absl::StrSplit(argumentString, ';');

            fmt::dynamic_format_arg_store<fmt::printf_context> store;

            for (auto &argument : arguments)
                store.push_back(argument);

            SendChatPacket(ch, type, fmt::vsprintf(translatedMessage, store));
        }
        catch (const std::exception &ex)
        {
            SPDLOG_CRITICAL("Error while formatting: {}", formatMessage);
            SPDLOG_CRITICAL("{}", ex.what());
            SendChatPacket(ch, type, format);
        }
    }
    else
    {
        const auto* translatedMessage = locale->oxTable.Translate(format);
        SendChatPacket(ch, type, translatedMessage);
    }
}

void SendSpecialI18nChatPacket(CHARACTER *ch, uint8_t type, const std::string &format)
{
    const auto *locale = GetLocale(ch);

    std::vector<std::string> result;
    storm::Tokenize(format, "#", result);
    if (result.size() > 1)
    {
        const auto formatMessage = result[0];
        const auto *translatedMessage = locale->stringTable.Translate(formatMessage);

        try
        {
            const auto& argumentString = result[1];

            std::vector<std::string> arguments = absl::StrSplit(argumentString, ';');

            fmt::dynamic_format_arg_store<fmt::printf_context> store;

            for (auto &argument : arguments)
                store.push_back(argument);

            SendChatPacket(ch, type, fmt::vsprintf(translatedMessage, store));
        }
        catch (const std::exception &ex)
        {
            SPDLOG_CRITICAL("Error while formatting: {}", formatMessage);
            SPDLOG_CRITICAL("{}", ex.what());
            SendChatPacket(ch, type, format);
        }
    }
    else
    {
        const auto *translatedMessage = locale->stringTable.Translate(format);
        SendChatPacket(ch, type, translatedMessage);
    }
}

void EffectPacketByPointType(CHARACTER *ch, uint32_t pointType)
{
    switch (pointType)
    {
    case POINT_HP:
        ch->EffectPacket(SE_HPUP_RED);
        break;
    case POINT_SP:
        ch->EffectPacket(SE_SPUP_BLUE);
        break;
    case POINT_MOV_SPEED:
        ch->EffectPacket(SE_DXUP_PURPLE);
        break;
    case POINT_ATT_SPEED:
        ch->EffectPacket(SE_SPEEDUP_GREEN);
        break;
    default:
        break;
    }
}

void MapLevelChecks(CHARACTER *ch)
{
    if (ch->IsGM())
        return;

    auto mapIndex = ch->GetMapIndex();
    if (mapIndex >= 10000)
        mapIndex /= 10000;

    if (const auto it = gConfig.mapLevelLimit.find(mapIndex); it != gConfig.mapLevelLimit.end())
    {
        if (ch->GetLevel() > it->second.max) { ch->GoHome(); }

        if (ch->GetLevel() < it->second.min) { ch->GoHome(); }
    }
}

void CleanUpShopOwners(CHARACTER *shopOwner)
{
    g_pCharManager->for_each_pc([shopOwner](CHARACTER *ch)
    {
        if (ch->GetShopOwner() == shopOwner) { ch->SetShopOwner(nullptr); }
    });
}

bool CAN_ENTER_ZONE(const CHARACTER *ch, int map_index)
{
    switch (map_index)
    {
    case 301:
    case 302:
    case 303:
    case 304:
        if (ch->GetLevel() < 90)
            return false;
    }
    return true;
}

bool IsPetUsableZone(int map_index)
{
    // (Private map instances, i.e dungeons, have high indexes)
    if (map_index >= 10000)
        map_index /= 10000;

    if(map_index == OXEVENT_MAP_INDEX)
        return false;

    if (CWarMapManager::instance().IsWarMap(map_index))
        return false;

    if (CArenaManager::instance().IsArenaMap(map_index))
        return false;

    if (gConfig.IsBlockedPetMap(map_index))
        return false;

    return true;
}

bool IsLevelPetUsableZone(int map_index)
{
    // (Private map instances, i.e dungeons, have high indexes)
    if (map_index >= 10000)
        map_index /= 10000;

    if(map_index == OXEVENT_MAP_INDEX)
        return false;

    if (CWarMapManager::instance().IsWarMap(map_index))
        return false;

    if (CArenaManager::instance().IsArenaMap(map_index))
        return false;

    if (gConfig.IsBlockedLevelPetMap(map_index))
        return false;

    return true;
}

bool IsMountableZone(int map_index, bool isHorse)
{
#if defined(WJ_COMBAT_ZONE)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(map_index))
        return false;
#endif

    // CHAOS DUNGEON
    if (map_index >= 2500000 && map_index <= 2509999)
        return false;

    // (Private map instances, i.e dungeons, have high indexes)
    if (map_index >= 10000)
        map_index /= 10000;

    if(map_index == OXEVENT_MAP_INDEX)
        return false;

    if (CWarMapManager::instance().IsWarMap(map_index))
        return false;

    if (CArenaManager::instance().IsArenaMap(map_index))
        return false;

    if (gConfig.IsBlockedMountMap(map_index))
        return false;

    return true;
}
