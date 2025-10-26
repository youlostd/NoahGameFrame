#include "SITokenParser.h"

#include "DbCacheSocket.hpp"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "desc_manager.h"
#include "guild_manager.h"
#include "motion.h"
#include "party.h"
#include "pvp.h"
#include "start_position.h"
#include "utils.h"
#include <game/AffectConstants.hpp>
#include <game/GamePacket.hpp>

#include "GArena.h"
#include "GBufferManager.h"
#include "dungeon.h"
#include "gm.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "main.h"
#include "messenger_manager.h"
#include "mob_manager.h"
#include "questmanager.h"
#include "threeway_war.h"
#include "war_map.h"

#include "CharUtil.hpp"
#include "ChatUtil.hpp"
#include "ItemUtils.h"
#include "cube.h"
#include "map_location.h"
#include <thecore/md5.hpp>

#include "DbCacheSocket.hpp"
#include "MasterSocket.hpp"
#include "MeleyLair.h"
#include "guild.h"
#include <game/MasterPackets.hpp>

ACMD(do_user_horse_ride)
{
    if (ch->IsObserverMode() && !ch->IsGM())
        return;

    if (ch->IsDead() || ch->IsStun())
        return;

    if (ch->IsHorseRiding() == false) {
        // 말이 아닌 다른탈것을 타고있다.
        if (ch->GetMountVnum()) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "이미 탈것을 이용중입니다.");
            return;
        }

        if (ch->GetHorse() == nullptr) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "말을 먼저 소환해주세요.");
            return;
        }

        ch->StartRiding();
    } else {
        ch->StopRiding();
    }
}

ACMD(do_user_horse_back)
{
    if (!ch->HasToggleMount()) {
        if (ch->GetHorse() != nullptr) {
            ch->HorseSummon(false);
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "말을 돌려보냈습니다.");
            return;
        }

        if (ch->IsHorseRiding() == true) {
            ch->StopRiding();
            ch->HorseSummon(false);
        }
    } else {
        const auto mountItem = FindToggleItem(ch, true, TOGGLE_MOUNT);
        if (mountItem) {
            DeactivateToggleItem(ch, mountItem);
        }
    }
}

ACMD(do_user_horse_feed)
{
    if (!ch->GetHorse()) {
        if (ch->IsHorseRiding() == false)
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "말을 먼저 소환해주세요.");
        else
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "말을 탄 상태에서는 먹이를 줄 수 없습니다.");
        return;
    }

    uint32_t dwFood = ch->GetHorseGrade() + 50054 - 1;

    if (ch->CountSpecifyItem(dwFood) > 0) {
        ch->RemoveSpecifyItem(dwFood, 1);
        ch->FeedHorse();
        SendI18nChatPacket(
            ch, CHAT_TYPE_INFO, "말에게 %s%s 주었습니다.",
            ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName, "");
    } else {
        SendI18nChatPacket(
            ch, CHAT_TYPE_INFO, "%s 아이템이 필요합니다",
            ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName);
    }
}

#define MAX_REASON_LEN 128

struct SendDisconnectFunc {
    void operator()(const std::shared_ptr<DESC>& d)
    {
        if (d->GetCharacter()) {
            if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
                d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit "
                                                                 "Shutdown("
                                                                 "SendDisconnec"
                                                                 "tFunc)");
        }
    }
};

struct DisconnectFunc {
    void operator()(const std::shared_ptr<DESC>& d)
    {
        if (d->GetCharacter())
            d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

        // We're iterating over the set - we're not allowed to modify it.
        d->DelayedDisconnect(0, "Shutdown");
    }
};

EVENTINFO(shutdown_event_data)
{
    int seconds;

    shutdown_event_data()
        : seconds(0)
    {
    }
};

EVENTFUNC(shutdown_event)
{
    shutdown_event_data* info = static_cast<shutdown_event_data*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("shutdown_event> <Factor> Null pointer");
        return 0;
    }

    int& pSec = info->seconds;

    if (pSec < 0) {
        SPDLOG_INFO("shutdown_event sec %d", pSec);

        if (--pSec == -10) {
            const DESC_MANAGER::DESC_SET& c_set_desc =
                DESC_MANAGER::instance().GetClientSet();
            std::for_each(c_set_desc.begin(), c_set_desc.end(),
                          DisconnectFunc());
            return THECORE_SECS_TO_PASSES(1);
        }
        if (pSec < -10) {
            Shutdown(-10);
            return 0;
        }

        return THECORE_SECS_TO_PASSES(1);
    } else if (pSec == 0) {
        const DESC_MANAGER::DESC_SET& c_set_desc =
            DESC_MANAGER::instance().GetClientSet();
        std::for_each(c_set_desc.begin(), c_set_desc.end(),
                      SendDisconnectFunc());
        g_bNoMoreClient = true;
        --pSec;
        return THECORE_SECS_TO_PASSES(1);
    } else //> 0 secs
    {
        if (pSec % 60 == 0) {
            if ((pSec / 60 > 30 &&
                 pSec / 60 % 10 != 0) // Only every 10m for 30m+
                || (pSec / 60 > 10 &&
                    pSec / 60 % 5 != 0)) // Only every 5m for 10m+
            {
                pSec -= 60;
                return THECORE_SECS_TO_PASSES(60);
            }

            SendBigNotice(fmt::format("Sunucu bakım sebebiyle kapatılacaktır "
                                      "%d dakika.#{}",
                                      pSec / 60)
                              .c_str(),
                          -1); // all maps in this core
        } else if (pSec < 60) {
            if (pSec > 10 && pSec % 10 == 0) {
                SendI18nNotice("Sunucu bakım sebebiyle kapatılacaktır %d saniye.",
                               pSec);

                pSec -= 10;
                return THECORE_SECS_TO_PASSES(10);
            } else if (pSec < 11) {
                SendI18nNotice("Sunucu bakım sebebiyle kapatılacaktır %d saniye.",
                               pSec);
            }
        }

        --pSec;
        return THECORE_SECS_TO_PASSES(1);
    }
}

static LPEVENT running_shutdown;

void Shutdown(int iSec)
{
    if (g_bNoMoreClient) {
        thecore_shutdown();
        return;
    }

    if (iSec < 0 && running_shutdown) {
        event_cancel(&running_shutdown);
        SendBigNotice("Shutdown stopped", -1);
        return;
    }

    if (running_shutdown) {
        SPDLOG_ERROR("There's already a shutdown running! Shutdown ignored.");
        return;
    }

    CWarMapManager::instance().OnShutdown();

    shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
    info->seconds = iSec;

    running_shutdown = event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
    if (ch) {
        SPDLOG_ERROR("Accept shutdown command from %s.", ch->GetName());

        if (running_shutdown) {
            ch->ChatPacket(CHAT_TYPE_INFO, "There's already a running "
                                           "shutdown. Shutdown request "
                                           "ignored!");
            return;
        }
    }

    int secs = 0;
    str_to_number(secs, argument);

    if (!secs || secs < 10)
        secs = 10;

    GmShutdownBroadcastPacket p;
    if (ch)
        p.reason = ch->GetName();
    else
        p.reason = "UNKNOWN";
    p.countdown = secs;

    SPDLOG_ERROR("Accept shutdown command from {0}", p.reason);

    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmShutdownBroadcast, p);

    Shutdown(p.countdown);
}

ACMD(do_local_shutdown)
{
    if (ch) {
        SPDLOG_ERROR("Accept shutdown command from %s.", ch->GetName());

        if (running_shutdown) {
            ch->ChatPacket(CHAT_TYPE_INFO, "There's already a running "
                                           "shutdown. Shutdown request "
                                           "ignored!");
            return;
        }
    }

    int secs = 0;
    str_to_number(secs, argument);

    if (!secs || secs < 10)
        secs = 10;

    ch->ChatPacket(CHAT_TYPE_INFO, "Shutdown begun locally.");
    Shutdown(secs);
}

EVENTINFO(TimedEventInfo)
{
    DynamicCharacterPtr ch;
    int subcmd;
    uint32_t left_second;
    uint8_t channel;

    TimedEventInfo()
        : ch()
        , subcmd(0)
        , left_second(0)
        , channel(0)
    {
    }
};

EVENTFUNC(timed_event)
{
    TimedEventInfo* info = static_cast<TimedEventInfo*>(event->info);
    if (info == nullptr) {
        SPDLOG_ERROR("timed_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER* ch = info->ch;
    if (ch == nullptr) {
        // <Factor>
        return 0;
    }

    DESC* d = ch->GetDesc();

    if (info->left_second == 0) {
        ch->m_pkTimedEvent = nullptr;

        switch (info->subcmd) {
            case SCMD_LOGOUT:
            case SCMD_QUIT:
            case SCMD_PHASE_SELECT: {
                LogManager::instance().DetailLoginLog(false, ch);
                break;
            }
        }

        switch (info->subcmd) {
            case SCMD_LOGOUT:
                if (d) {
                    d->SetDisconnectReason("Logout");
                    d->Disconnect();
                }
                break;

            case SCMD_QUIT:
                ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
                break;

            case SCMD_CHANGE_CHANNEL:
                ch->WarpSet(ch->GetMapIndex(), ch->GetX(), ch->GetY(),
                            info->channel);
                break;

            case SCMD_PHASE_SELECT: {
                ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

                if (d)
                    d->SetPhase(PHASE_SELECT);
                break;
            }
        }

        return 0;
    } else {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%d초 남았습니다.",
                           info->left_second);
        --info->left_second;
    }

    return THECORE_SECS_TO_PASSES(1);
}

ACMD(do_channel_change)
{
    uint8_t channel = 0;

    if (ch->GetMapIndex() >= 10000) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CHANGE_CHANNEL_ERR_PRIV_MAP");
        return;
    }

    if (!ch->CanWarp()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return;
    }

    if (MeleyLair::CMgr::instance().IsMeleyMap(ch->GetMapIndex())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return;
    }

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));
    str_to_number(channel, arg1);

    if (gConfig.channel == channel) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CHANGE_CHANNEL_ALREADY");
        return;
    }

    std::string addr;
    uint16_t port;
    if (!CMapLocation::instance().Get(channel, ch->GetMapIndex(), addr, port)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CHANGE_CHANNEL_UNAVAILABLE");
        return;
    }

    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CHANGING_CHANNEL");

    auto info = AllocEventInfo<TimedEventInfo>();

    info->left_second = 3;
    ;
    info->ch = ch;
    info->subcmd = subcmd;
    info->channel = channel;

    ch->m_pkTimedEvent = event_create(timed_event, info, 1);
}

ACMD(do_cmd)
{
    if (ch->m_pkTimedEvent) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "취소 되었습니다.");
        event_cancel(&ch->m_pkTimedEvent);
        return;
    }

    uint8_t channel = 0;

    switch (subcmd) {
        case SCMD_LOGOUT:
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "로그인 화면으로 돌아 갑니다. 잠시만 "
                               "기다리세요.");
            break;

        case SCMD_QUIT:
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "게임을 종료 합니다. 잠시만 기다리세요.");
            break;

        case SCMD_PHASE_SELECT:
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "캐릭터를 전환 합니다. 잠시만 기다리세요.");
            break;

        case SCMD_CHANGE_CHANNEL: {
            if (ch->GetMapIndex() >= 10000) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "CHANGE_CHANNEL_ERR_PRIV_MAP");
                return;
            }

            char arg1[256];
            one_argument(argument, arg1, sizeof(arg1));
            str_to_number(channel, arg1);

            if (gConfig.channel == channel) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "CHANGE_CHANNEL_ALREADY");
                return;
            }

            std::string addr;
            uint16_t port;
            if (!CMapLocation::instance().Get(channel, ch->GetMapIndex(), addr,
                                              port)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "CHANGE_CHANNEL_UNAVAILABLE");
                return;
            }

            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CHANGING_CHANNEL");
            break;
        }
    }

    int nExitLimitTime = 10;

    if (ch->IsHack(false, true, nExitLimitTime) &&
        false == g_pThreeWayWar->IsSungZiMapIndex(ch->GetMapIndex()) &&
        (!ch->GetWarMap() ||
         ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG)) {
        return;
    }

    auto info = AllocEventInfo<TimedEventInfo>();

    if (ch->IsPosition(POS_FIGHTING))
        info->left_second = 3;
    else
        info->left_second = 1;

    info->ch = ch;
    info->subcmd = subcmd;
    info->channel = channel;

    ch->m_pkTimedEvent = event_create(timed_event, info, 1);
}

ACMD(do_mount) {}

ACMD(do_fishing)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    ch->SetRotation((float)atof(arg1));
    ch->fishing();
}

ACMD(do_console)
{
    ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
    if (!ch->IsDead()) {
        ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
        ch->StartRecoveryEvent();
        return;
    }

    if (nullptr == ch->m_pkDeadEvent)
        return;

    const int iTimeToDead =
        (event_time(ch->m_pkDeadEvent) / THECORE_SECS_TO_PASSES(1));

#if defined(WJ_COMBAT_ZONE)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(ch->GetMapIndex())) {
        CCombatZoneManager::Instance().OnRestart(ch, subcmd);
        return;
    }
#endif

    if (subcmd != SCMD_RESTART_TOWN &&
        (!ch->GetWarMap() ||
         ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG)) {
        if (ch->IsHack()) {
            //성지 맵일경우에는 체크 하지 않는다.
            if (false == g_pThreeWayWar->IsSungZiMapIndex(ch->GetMapIndex())) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "아직 재시작 할 수 없습니다. (%d초 남음)",
                                   iTimeToDead -
                                       (180 - gConfig.portalLimitTime));
                return;
            }
        }

        if (!gConfig.testServer &&
            !gConfig.IsInstantRestartMap(ch->GetMapIndex())) {
            if (iTimeToDead > 175) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "아직 재시작 할 수 없습니다. (%d초 남음)",
                                   iTimeToDead - 175);
                return;
            }
        }
    }

    // PREVENT_HACK
    // DESC : 창고, 교환 창 후 포탈을 사용하는 버그에 이용될수 있어서
    //		쿨타임을 추가
    if (subcmd == SCMD_RESTART_TOWN) {
        if (ch->IsHack()) {
            //길드맵, 성지맵에서는 체크 하지 않는다.
            if ((!ch->GetWarMap() ||
                 ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG) ||
                false == g_pThreeWayWar->IsSungZiMapIndex(ch->GetMapIndex())) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "아직 재시작 할 수 없습니다. (%d초 남음)",
                                   iTimeToDead -
                                       (180 - gConfig.portalLimitTime));
                return;
            }
        }

        if (iTimeToDead > 173) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "아직 마을에서 재시작 할 수 없습니다. (%d 초 "
                               "남음)",
                               iTimeToDead - 173);
            return;
        }
    }
    // END_PREVENT_HACK

    ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

    ch->GetDesc()->SetPhase(PHASE_GAME);
    ch->SetPosition(POS_STANDING);
    ch->StartRecoveryEvent();

    // FORKED_LOAD
    // DESC: 삼거리 전투시 부활을 할경우 맵의 입구가 아닌 삼거리 전투의
    // 시작지점으로 이동하게 된다.
    if (1 == quest::CQuestManager::instance().GetEventFlag("threeway_war")) {
        if (subcmd == SCMD_RESTART_TOWN || subcmd == SCMD_RESTART_HERE) {
            if (g_pThreeWayWar->IsThreeWayWarMapIndex(ch->GetMapIndex()) &&
                !g_pThreeWayWar->IsSungZiMapIndex(ch->GetMapIndex())) {
                ch->WarpSet(EMPIRE_START_MAP(ch->GetEmpire()),
                            EMPIRE_START_X(ch->GetEmpire()),
                            EMPIRE_START_Y(ch->GetEmpire()));

                ch->ReviveInvisible(5);
                ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
                ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
                return;
            }

            if (g_pThreeWayWar->IsSungZiMapIndex(ch->GetMapIndex())) {
                if (g_pThreeWayWar->GetReviveTokenForPlayer(
                        ch->GetPlayerID()) <= 0) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "We lost all the chance of resurrection "
                                       "in the Holy Land! Go to town!");
                    ch->WarpSet(EMPIRE_START_MAP(ch->GetEmpire()),
                                EMPIRE_START_X(ch->GetEmpire()),
                                EMPIRE_START_Y(ch->GetEmpire()));
                } else {
                    ch->Show(ch->GetMapIndex(),
                             GetSungziStartX(ch->GetEmpire()),
                             GetSungziStartY(ch->GetEmpire()));
                }

                ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
                ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
                ch->ReviveInvisible(5);
                return;
            }
        }
    }
    // END_FORKED_LOAD

    if (ch->GetWarMap() && !ch->IsObserverMode()) {
        CWarMap* pMap = ch->GetWarMap();
        uint32_t dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

        if (dwGuildOpponent) {
            switch (subcmd) {
                case SCMD_RESTART_TOWN:
                    SPDLOG_INFO("do_restart: restart town");
                    PIXEL_POSITION pos;

                    if (CWarMapManager::instance().GetStartPosition(
                            ch->GetMapIndex(),
                            ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1,
                            pos))
                        ch->Show(ch->GetMapIndex(), pos.x, pos.y);
                    else
                        ch->ExitToSavedLocation();

                    ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
                    ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
                    ch->RemoveBadAffect();
                    ch->ComputePoints();
                    ch->ComputeMountPoints();

                    ch->ReviveInvisible(5);

                    break;

                case SCMD_RESTART_HERE:
                    SPDLOG_INFO("do_restart: restart here");
                    ch->ViewReencode();
                    // ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
                    ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
                    ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
                    ch->RemoveBadAffect();
                    ch->ComputePoints();
                    ch->ComputeMountPoints();

                    ch->ReviveInvisible(5);
                    break;
            }

            return;
        }
    }

    switch (subcmd) {
        case SCMD_RESTART_TOWN: {
            SPDLOG_INFO("do_restart: restart town");
            PIXEL_POSITION pos;

            if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(
                    ch->GetMapIndex(), ch->GetEmpire(), pos))
                ch->WarpSet(ch->GetMapIndex(), pos.x, pos.y);
            else
                ch->GoHome();

            const auto mapIndex = ch->GetMapIndex();
            if (mapIndex == 1 || mapIndex == 3 || mapIndex == 21 ||
                mapIndex == 23 || mapIndex == 41 || mapIndex == 43) {
                ch->PointChange(POINT_HP, (ch->GetMaxHP() / 4) - ch->GetHP());
            } else {
                ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
                ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
            }
            ch->RemoveBadAffect();
            ch->DeathPenalty(1);
        }

        break;

        case SCMD_RESTART_HERE: {
            SPDLOG_INFO("do_restart: restart here");
            ch->ViewReencode();
            // ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
            // ch->PointChange(POINT_HP, (ch->GetMaxHP() / 100) * 10);
            const auto mapIndex = ch->GetMapIndex();
            if (mapIndex == 1 || mapIndex == 3 || mapIndex == 21 ||
                mapIndex == 23 || mapIndex == 41 || mapIndex == 43) {
                ch->PointChange(POINT_HP, (ch->GetMaxHP() / 4) - ch->GetHP());
            } else {
                ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
                ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
            }
            ch->DeathPenalty(0);
            ch->RemoveBadAffect();
            ch->ReviveInvisible(5);
            ch->ComputePoints();
            ch->ComputeMountPoints();

        }

        break;
    }
}

#define MAX_STAT 90

ACMD(do_stat_reset)
{
    ch->PointChange(POINT_STAT_RESET_COUNT,
                    12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_point_refresh)
{
    ch->PointsPacket();
}

ACMD(do_stat_minus)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    if (ch->IsPolymorphed()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "둔갑 중에는 능력을 올릴 수 없습니다.");
        return;
    }

    if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
        return;

    if (!strcmp(arg1, "st")) {
        if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
            return;

        ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
        ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
        ch->ComputePoints();
        ch->ComputeMountPoints();
        ch->PointChange(POINT_ST, 0);
    } else if (!strcmp(arg1, "dx")) {
        if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
            return;

        ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
        ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
        ch->ComputePoints();
        ch->ComputeMountPoints();
        ch->PointChange(POINT_DX, 0);
    } else if (!strcmp(arg1, "ht")) {
        if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
            return;

        ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
        ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
        ch->ComputePoints();
        ch->ComputeMountPoints();
        ch->PointChange(POINT_HT, 0);
        ch->PointChange(POINT_MAX_HP, 0);
    } else if (!strcmp(arg1, "iq")) {
        if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
            return;

        ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
        ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
        ch->ComputePoints();
        ch->ComputeMountPoints();
        ch->PointChange(POINT_IQ, 0);
        ch->PointChange(POINT_MAX_SP, 0);
    } else
        return;

    ch->PointChange(POINT_STAT, +1);
    ch->PointChange(POINT_STAT_RESET_COUNT, -1);
    ch->ComputePoints();
    ch->ComputeMountPoints();
}

ACMD(do_stat)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));
    bool checking = false;

    if (!*arg1)
        return;

    if (ch->IsPolymorphed()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "?媛 以?? ?λμ ?щ┫ ? "
                           "??듬??");
        return;
    }

    if (ch->GetPoint(POINT_STAT) <= 0)
        return;

    uint8_t idx = 0;

    if (!strcmp(arg1, "st"))
        idx = POINT_ST;
    else if (!strcmp(arg1, "dx"))
        idx = POINT_DX;
    else if (!strcmp(arg1, "ht"))
        idx = POINT_HT;
    else if (!strcmp(arg1, "iq"))
        idx = POINT_IQ;
    else
        return;

    if (ch->GetRealPoint(idx) >= gConfig.maxStat)
        return;

    ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
    ch->ComputePoints();
    ch->ComputeMountPoints();
    ch->PointChange(idx, 0);

    if (idx == POINT_IQ)
        ch->PointChange(POINT_MAX_HP, 0);
    else if (idx == POINT_HT)
        ch->PointChange(POINT_MAX_SP, 0);

    ch->PointChange(POINT_STAT, -1);
}

ACMD(do_pott)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    uint32_t val = 0;
    str_to_number(val, arg1);

    if (val == 1)
        ch->ChatPacket(CHAT_TYPE_COMMAND, "AutoPotionBig");
    else
        ch->ChatPacket(CHAT_TYPE_COMMAND, "AutoPotionNormal");
}

ACMD(do_pvp)
{
#if defined(WJ_COMBAT_ZONE)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(ch->GetMapIndex()))
        return;
#endif
    if (ch->GetArena() != nullptr ||
        CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "대련장에서 사용하실 수 없습니다.");
        return;
    }

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    uint32_t vid = 0;
    str_to_number(vid, arg1);
    CHARACTER* pkVictim = g_pCharManager->Find(vid);

    if (!pkVictim)
        return;
    if (!pkVictim->IsPC())
        return;

    if (pkVictim->IsBlockMode(BLOCK_DUEL_REQUEST, ch->GetPlayerID())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%s blocks duel requests",
                           pkVictim->GetName());
        return;
    }

    if (ch->IsBlockMode(BLOCK_DUEL_REQUEST, pkVictim->GetPlayerID())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You blocked duel requests for %s",
                           pkVictim->GetName());
        return;
    }

#ifdef ENABLE_MESSENGER_BLOCK
    if (MessengerManager::instance().IsBlocked_Target(ch->GetName(),
                                                      pkVictim->GetName())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You blocked %s",
                           pkVictim->GetName());
        return;
    }
    if (MessengerManager::instance().IsBlocked_Me(ch->GetName(),
                                                  pkVictim->GetName())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%s blocked you",
                           pkVictim->GetName());
        return;
    }
#endif

    if (pkVictim->GetArena() != nullptr) {
        SendI18nChatPacket(pkVictim, CHAT_TYPE_INFO, "상대방이 대련중입니다.");
        return;
    }

    CPVPManager::instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    if (!ch->GetGuild()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 길드에 속해있지 않습니다.");
        return;
    }

    CGuild* g = ch->GetGuild();
    const auto* gm = g->GetMember(ch->GetPlayerID());
    if (gm->grade == GUILD_LEADER_GRADE) {
        uint32_t vnum = 0;
        str_to_number(vnum, arg1);
        g->SkillLevelUp(vnum);
    } else {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 길드 스킬 레벨을 변경할 권한이 없습니다.");
    }
}

ACMD(do_skillup)
{
    /*const auto now = get_dword_time();
    if (now - ch->GetLastAttackTime() <= 1500 ||
        now - ch->GetLastSkillTime() <= 1500 ||
        now - ch->GetLastHitReceivedTime() <= 1500) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You can only adjust your points
    while you are still."); return;
    }*/

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    uint32_t vnum = 0;
    str_to_number(vnum, arg1);

    if (true == ch->CanUseSkill(vnum)) {
        ch->SkillLevelUp(vnum);
    } else {
        switch (vnum) {
            case SKILL_HORSE_WILDATTACK:
            case SKILL_HORSE_CHARGE:
            case SKILL_HORSE_ESCAPE:
            case SKILL_HORSE_WILDATTACK_RANGE:

            case SKILL_7_A_ANTI_TANHWAN:
            case SKILL_7_B_ANTI_AMSEOP:
            case SKILL_7_C_ANTI_SWAERYUNG:
            case SKILL_7_D_ANTI_YONGBI:

            case SKILL_8_A_ANTI_GIGONGCHAM:
            case SKILL_8_B_ANTI_YEONSA:
            case SKILL_8_C_ANTI_MAHWAN:
            case SKILL_8_D_ANTI_BYEURAK:

#ifdef ENABLE_678TH_SKILL
            case SKILL_7_A_ANTI_TANHWAN_2015:
            case SKILL_7_B_ANTI_AMSEOP_2015:
            case SKILL_7_C_ANTI_SWAERYUNG_2015:
            case SKILL_7_D_ANTI_YONGBI_2015:
            case SKILL_7_A_ANTI_GIGONGCHAM_2015:
            case SKILL_7_B_ANTI_YEONSA_2015:
            case SKILL_7_C_ANTI_MAHWAN_2015:
            case SKILL_7_D_ANTI_BYEURAK_2015:
            case SKILL_7_D_ANTI_SALPOONG_2015:
                // Power
            case SKILL_8_A_POWER_TANHWAN_2015:
            case SKILL_8_B_POWER_AMSEOP_2015:
            case SKILL_8_C_POWER_SWAERYUNG_2015:
            case SKILL_8_D_POWER_YONGBI_2015:
            case SKILL_8_A_POWER_GIGONGCHAM_2015:
            case SKILL_8_B_POWER_YEONSA_2015:
            case SKILL_8_C_POWER_MAHWAN_2015:
            case SKILL_8_D_POWER_BYEURAK_2015:
            case SKILL_8_D_POWER_SALPOONG_2015:
#endif
                ch->SkillLevelUp(vnum);
                break;
        }
    }
}

//
// @version	05/06/20 Bang2ni - 커맨드 처리 Delegate to CHARACTER class
//
ACMD(do_safebox_close)
{
    ch->CloseSafebox();
}

//
// @version	05/06/20 Bang2ni - 커맨드 처리 Delegate to CHARACTER class
//
ACMD(do_safebox_password)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));
    ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
    const char* szHelp = "Usage: /safebox_change_password <old_pw> <new_pw>";
    CHECK_COMMAND(Parser, 2, szHelp);

    auto old_pw = Parser.GetTokenString(0);
    auto new_pw = Parser.GetTokenString(1);

    if (old_pw.empty() || new_pw.empty()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<Safebox> Old or new password empty.");
        return;
    }

    if (old_pw.length() > 6 || new_pw.length() > 6) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<Safebox> Old or new password too long.");
        return;
    }

    TSafeboxChangePasswordPacket p;
    p.dwID = ch->GetDesc()->GetAid();
    storm::CopyStringSafe(p.szOldPassword, old_pw);
    storm::CopyStringSafe(p.szNewPassword, new_pw);

    db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD,
                            ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1 || strlen(arg1) > 6) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<창고> 잘못된 암호를 입력하셨습니다.");
        return;
    }

    int iPulse = thecore_pulse();

    if (ch->GetMall()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<창고> 창고가 이미 열려있습니다.");
        return;
    }

    if (iPulse - ch->GetMallLoadTime() <
        THECORE_SECS_TO_PASSES(1) * 10) // 10초에 한번만 요청 가능
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<창고> 창고를 닫은지 10초 안에는 열 수 없습니다.");
        return;
    }

    ch->SetMallLoadTime(iPulse);

    TSafeboxLoadPacket p;
    p.dwID = ch->GetDesc()->GetAccountTable().id;
    strlcpy(p.szLogin, ch->GetDesc()->GetLogin().c_str(), sizeof(p.szLogin));
    strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

    db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p,
                            sizeof(p));
}

ACMD(do_mall_close)
{
    if (ch->GetMall()) {
        ch->SetMallLoadTime(thecore_pulse());
        ch->CloseMall();
        ch->Save();
    }
}

ACMD(do_ungroup)
{
    if (!ch->GetParty())
        return;

    if (!CPartyManager::instance().IsEnablePCParty()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 서버 문제로 파티 관련 처리를 할 수 "
                           "없습니다.");
        return;
    }

    if (ch->GetDungeon()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 던전 안에서는 파티에서 나갈 수 없습니다.");
        return;
    }

    if (CParty* pParty = ch->GetParty(); pParty) {
        if (pParty->GetMemberCount() == 2) {
            // party disband
            CPartyManager::instance().DeleteParty(pParty);
        } else {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<파티> 파티에서 나가셨습니다.");
            // pParty->SendPartyRemoveOneToAll(ch);
            pParty->Quit(ch->GetPlayerID());
            // pParty->SendPartyRemoveAllToOne(ch);
        }
    }
}

ACMD(do_close_shop)
{
    return ch->ChatPacket(CHAT_TYPE_INFO, "This function is not implemented!");
}

ACMD(do_set_walk_mode)
{
    ch->SetNowWalking(true);
    ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
    ch->SetNowWalking(false);
    ch->SetWalking(false);
}

#if defined(ENABLE_AFFECT_POLYMORPH_REMOVE)
ACMD(do_remove_polymorph)
{
    if (!ch)
        return;

    if (!ch->IsPolymorphed())
        return;

    ch->SetPolymorph(0);
    ch->RemoveAffect(AFFECT_POLYMORPH);
}
#endif

ACMD(do_remove_affect_player)
{
    if (!ch)
        return;

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1) {
        return;
    }

    uint32_t affectType = 0;
    str_to_number(affectType, arg1);

    if (affectType == AFFECT_DS_SET)
        return;

    if (!ch->IsGoodAffect(affectType))
        return;

    ch->RemoveAffect(affectType);
}

ACMD(do_guildwar_request_enter)
{
    CGuild* g = ch->GetGuild();

    if (!g)
        return;

    auto e = g->UnderAnyWar();
    if (e == 0)
        return;

    g->GuildWarEntryAccept(e, ch);
}

ACMD(do_war)
{
    //내 길드 정보를 얻어오고
    CGuild* g = ch->GetGuild();

    if (!g)
        return;

    //전쟁중인지 체크한번!
    if (g->UnderAnyWar()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 이미 다른 전쟁에 참전 중 입니다.");
        return;
    }

    //파라메터를 두배로 나누고
    char arg1[256], arg2[256];

    uint8_t type = GUILD_WAR_TYPE_FIELD;
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1)
        return;

    if (*arg2) {
        str_to_number(type, arg2);

        if (type == GUILD_WAR_TYPE_FIELD) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<Guild> The field war is disabled on this "
                               "server.");
            return;
        }
    }

    //길드의 마스터 아이디를 얻어온뒤
    uint32_t gm_pid = g->GetMasterPID();

    //마스터인지 체크(길전은 길드장만이 가능)
    if (gm_pid != ch->GetPlayerID()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 길드전에 대한 권한이 없습니다.");
        return;
    }

    //상대 길드를 얻어오고
    CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

    if (!opp_g) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<길드> 그런 길드가 없습니다.");
        return;
    }

    //상대길드와의 상태 체크
    switch (g->GetGuildWarState(opp_g->GetID())) {
        case GUILD_WAR_NONE: {
            if (opp_g->UnderAnyWar()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 상대방 길드가 이미 전쟁 중 입니다.");
                return;
            }
        } break;

        case GUILD_WAR_SEND_DECLARE: {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "이미 선전포고 중인 길드입니다.");
            return;
        } break;

        case GUILD_WAR_RECV_DECLARE: {
            if (opp_g->UnderAnyWar()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 상대방 길드가 이미 전쟁 중 입니다.");
                g->RequestRefuseWar(opp_g->GetID());
                return;
            }
        } break;

        case GUILD_WAR_RESERVE: {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<길드> 이미 전쟁이 예약된 길드 입니다.");
            return;
        } break;

        case GUILD_WAR_END:
            return;

        default:
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<길드> 이미 전쟁 중인 길드입니다.");
            g->RequestRefuseWar(opp_g->GetID());
            return;
    }

    if (!g->CanStartWar(type)) {
        // 길드전을 할 수 있는 조건을 만족하지않는다.
        if (type == GUILD_WAR_TYPE_FIELD) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<Guild> The field war is disabled on this "
                               "server.");
        } else if (g->GetLadderPoint() == 0) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<길드> 레더 점수가 모자라서 길드전을 할 수 "
                               "없습니다.");
            SPDLOG_INFO("GuildWar.StartError.NEED_LADDER_POINT");
        } else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<길드> 길드전을 하기 위해선 최소한 %d명이 "
                               "있어야 합니다.",
                               GUILD_WAR_MIN_MEMBER_COUNT);
            SPDLOG_INFO("GuildWar.StartError.NEED_MINIMUM_MEMBER[{0}]",
                        GUILD_WAR_MIN_MEMBER_COUNT);
        } else {
            SPDLOG_INFO("GuildWar.StartError.UNKNOWN_ERROR");
        }
        return;
    }

    if (!opp_g->CanStartWar(type)) {
        if (type == GUILD_WAR_TYPE_FIELD) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<Guild> The field war is disabled on this "
                               "server.");
        } 
        else if (opp_g->GetLadderPoint() == 0)
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<길드> 상대방 길드의 레더 점수가 모자라서 "
                               "길드전을 할 수 없습니다.");
        else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<길드> 상대방 길드의 길드원 수가 부족하여 "
                               "길드전을 할 수 없습니다.");
        return;
    }

    auto& onlinePlayers = DESC_MANAGER::instance().GetOnlinePlayers();

    if (!g->GetMasterCharacter() && !onlinePlayers.Get(g->GetMasterPID())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 상대방 길드의 길드장이 접속중이 아닙니다.");
        g->RequestRefuseWar(opp_g->GetID());
        return;
    }

    if (!opp_g->GetMasterCharacter() &&
        !onlinePlayers.Get(opp_g->GetMasterPID())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 상대방 길드의 길드장이 접속중이 아닙니다.");
        g->RequestRefuseWar(opp_g->GetID());
        return;
    }

    g->RequestDeclareWar(opp_g->GetID(), type);
}

ACMD(do_nowar)
{
    CGuild* g = ch->GetGuild();
    if (!g)
        return;

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    uint32_t gm_pid = g->GetMasterPID();

    if (gm_pid != ch->GetPlayerID()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 길드전에 대한 권한이 없습니다.");
        return;
    }

    CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

    if (!opp_g) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<길드> 그런 길드가 없습니다.");
        return;
    }

    g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
    ch->DetailLog();
}

ACMD(do_monsterlog)
{
    ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    if (!isNumeric(arg1))
        return;

    uint8_t mode = 0;
    str_to_number(mode, arg1);

    if (mode == PK_MODE_PROTECT)
        return;

    if (ch->GetLevel() < gConfig.pkProtectLevel && mode != 0)
        return;

#if defined(WJ_COMBAT_ZONE)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(ch->GetMapIndex()))
        return;
#endif

    ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
    if (ch->GetArena()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "대련장에서 사용하실 수 없습니다.");
        return;
    }

    char arg1[256], arg2[256];
    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1 || !*arg2)
        return;

    char answer = LOWER(*arg1);

    CHARACTER* tch = g_pCharManager->FindPC(arg2);
    if (tch) {
        if (answer != 'y') {
            SendI18nChatPacket(tch, CHAT_TYPE_INFO,
                               "%s 님으로 부터 친구 등록을 거부 당했습니다.",
                               ch->GetName());
        }

        MessengerManager::instance().AuthToAdd(
            ch->GetName(), arg2, answer == 'y' ? false : true); // DENY
    } else {
        GmMessengerAuthAddPacket p;
        p.account = ch->GetName();
        p.companion = arg2;
        p.deny = answer != 'y';
        DESC_MANAGER::instance().GetMasterSocket()->Send(kGmMessengerAuthAdd,
                                                         p);
    }
}

ACMD(do_setblockmode)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (*arg1) {
        uint8_t flag = 0;
        str_to_number(flag, arg1);
        ch->SetBlockMode(flag);
    }
}

ACMD(do_unmount)
{
    if (!Unmount(ch))
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "인벤토리가 꽉 차서 내릴 수 없습니다.");
}

ACMD(do_observer_exit)
{
    if (ch->IsObserverMode() && ch->IsGM()) {
        ch->SetObserverMode(false);
        return;
    }

    if (ch->IsObserverMode()) {
        if (ch->GetWarMap())
            ch->SetWarMap(nullptr);

        if (ch->GetArena() != nullptr || ch->GetArenaObserverMode()) {
            ch->SetArenaObserverMode(false);

            if (ch->GetArena() != nullptr)
                ch->GetArena()->RemoveObserver(ch->GetPlayerID());

            ch->SetArena(nullptr);
            ch->WarpSet(ARENA_RETURN_POINT_MAP(ch->GetEmpire()),
                        ARENA_RETURN_POINT_X(ch->GetEmpire()),
                        ARENA_RETURN_POINT_Y(ch->GetEmpire()));
        } else {
            ch->GoHome();
        }
        ch->SetObserverMode(false);
    }
}

ACMD(do_view_equip)
{
    if (ch->GetLevel() <= 14) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "This function is available at level 15 and above");
        return;
    }

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (*arg1) {
        uint32_t vid = 0;
        str_to_number(vid, arg1);
        CHARACTER* tch = g_pCharManager->Find(vid);

        if (!tch)
            return;

        if (!tch->IsPC())
            return;

        if ((tch->IsBlockMode(BLOCK_VIEW_EQUIP, ch) && !ch->IsGM()) ||
            (!gConfig.testServer && tch->IsGM())) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "The player %s blocked viewing of his "
                               "equipment!",
                               tch->GetName());
            return;
        }

        tch->SendEquipment(ch);
    }
}

ACMD(do_party_request)
{
    if (ch->GetArena()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "대련장에서 사용하실 수 없습니다.");
        return;
    }

    if (ch->GetParty()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "이미 파티에 속해 있으므로 가입신청을 할 수 "
                           "없습니다.");
        return;
    }

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    uint32_t vid = 0;
    str_to_number(vid, arg1);
    CHARACTER* tch = g_pCharManager->Find(vid);

    if (tch)
        if (!ch->RequestToParty(tch))
            ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    uint32_t vid = 0;
    str_to_number(vid, arg1);
    CHARACTER* tch = g_pCharManager->Find(vid);

    if (tch)
        ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    uint32_t vid = 0;
    str_to_number(vid, arg1);
    CHARACTER* tch = g_pCharManager->Find(vid);

    if (tch)
        ch->DenyToParty(tch);
}

// LUA_ADD_GOTO_INFO

static const char* FN_point_string(CHARACTER* ch, int apply_number)
{
    auto locale = GetLocale(ch);

    switch (apply_number) {
        case POINT_MAX_HP:
            return LC_TEXT_LC("최대 생명력 +%d", locale);
        case POINT_MAX_SP:
            return LC_TEXT_LC("최대 정신력 +%d", locale);
        case POINT_HT:
            return LC_TEXT_LC("체력 +%d", locale);
        case POINT_IQ:
            return LC_TEXT_LC("지능 +%d", locale);
        case POINT_ST:
            return LC_TEXT_LC("근력 +%d", locale);
        case POINT_DX:
            return LC_TEXT_LC("민첩 +%d", locale);
        case POINT_ATT_SPEED:
            return LC_TEXT_LC("공격속도 +%d", locale);
        case POINT_MOV_SPEED:
            return LC_TEXT_LC("이동속도 %d", locale);
        case POINT_CASTING_SPEED:
            return LC_TEXT_LC("쿨타임 -%d", locale);
        case POINT_HP_REGEN:
            return LC_TEXT_LC("생명력 회복 +%d", locale);
        case POINT_SP_REGEN:
            return LC_TEXT_LC("정신력 회복 +%d", locale);
        case POINT_POISON_PCT:
            return LC_TEXT_LC("독공격 %d", locale);
        case POINT_STUN_PCT:
            return LC_TEXT_LC("스턴 +%d", locale);
        case POINT_SLOW_PCT:
            return LC_TEXT_LC("슬로우 +%d", locale);
        case POINT_CRITICAL_PCT:
            return LC_TEXT_LC("%d%% 확률로 치명타 공격", locale);
        case POINT_RESIST_CRITICAL:
            return LC_TEXT_LC("상대의 치명타 확률 %d%% 감소", locale);
        case POINT_PENETRATE_PCT:
            return LC_TEXT_LC("%d%% 확률로 관통 공격", locale);
        case POINT_RESIST_PENETRATE:
            return LC_TEXT_LC("상대의 관통 공격 확률 %d%% 감소", locale);
        case POINT_ATTBONUS_HUMAN:
            return LC_TEXT_LC("인간류 몬스터 타격치 +%d%%", locale);
        case POINT_ATTBONUS_ANIMAL:
            return LC_TEXT_LC("동물류 몬스터 타격치 +%d%%", locale);
        case POINT_ATTBONUS_ORC:
            return LC_TEXT_LC("웅귀족 타격치 +%d%%", locale);
        case POINT_ATTBONUS_MILGYO:
            return LC_TEXT_LC("밀교류 타격치 +%d%%", locale);
        case POINT_ATTBONUS_UNDEAD:
            return LC_TEXT_LC("시체류 타격치 +%d%%", locale);
        case POINT_ATTBONUS_DEVIL:
            return LC_TEXT_LC("악마류 타격치 +%d%%", locale);
        case POINT_ATTBONUS_METIN:
            return LC_TEXT_LC("Attack Boost against Metin + %d%%", locale);
        case POINT_ATTBONUS_TRENT:
            return LC_TEXT_LC("Attack Boost against Treelike + %d%%", locale);
        case POINT_ATTBONUS_BOSS:
            return LC_TEXT_LC("Attack Boost against Boss + %d%%", locale);
        case POINT_ATTBONUS_LEGEND:
            return LC_TEXT_LC("Attack Boost against Legends + %d%%", locale);
        case POINT_ATTBONUS_SHADOW:
            return LC_TEXT_LC("Attack Boost against Shadows + %d%%", locale);
        case POINT_STEAL_HP:
            return LC_TEXT_LC("타격치 %d%% 를 생명력으로 흡수", locale);
        case POINT_STEAL_SP:
            return LC_TEXT_LC("타력치 %d%% 를 정신력으로 흡수", locale);
        case POINT_MANA_BURN_PCT:
            return LC_TEXT_LC("%d%% 확률로 타격시 상대 전신력 소모", locale);
        case POINT_DAMAGE_SP_RECOVER:
            return LC_TEXT_LC("%d%% 확률로 피해시 정신력 회복", locale);
        case POINT_BLOCK:
            return LC_TEXT_LC("물리타격시 블럭 확률 %d%%", locale);
        case POINT_DODGE:
            return LC_TEXT_LC("활 공격 회피 확률 %d%%", locale);
        case POINT_RESIST_SWORD:
            return LC_TEXT_LC("한손검 방어 %d%%", locale);
        case POINT_RESIST_TWOHAND:
            return LC_TEXT_LC("양손검 방어 %d%%", locale);
        case POINT_RESIST_DAGGER:
            return LC_TEXT_LC("두손검 방어 %d%%", locale);
        case POINT_RESIST_BELL:
            return LC_TEXT_LC("방울 방어 %d%%", locale);
        case POINT_RESIST_FAN:
            return LC_TEXT_LC("부채 방어 %d%%", locale);
        case POINT_RESIST_BOW:
            return LC_TEXT_LC("활공격 저항 %d%%", locale);
        case POINT_RESIST_FIRE:
            return LC_TEXT_LC("화염 저항 %d%%", locale);
        case POINT_RESIST_ELEC:
            return LC_TEXT_LC("전기 저항 %d%%", locale);
        case POINT_RESIST_MAGIC:
            return LC_TEXT_LC("마법 저항 %d%%", locale);
        case POINT_RESIST_WIND:
            return LC_TEXT_LC("바람 저항 %d%%", locale);
        case POINT_RESIST_ICE:
            return LC_TEXT_LC("냉기 저항 %d%%", locale);
        case POINT_RESIST_EARTH:
            return LC_TEXT_LC("대지 저항 %d%%", locale);
        case POINT_RESIST_DARK:
            return LC_TEXT_LC("어둠 저항 %d%%", locale);
        case POINT_REFLECT_MELEE:
            return LC_TEXT_LC("직접 타격치 반사 확률 : %d%%", locale);
        case POINT_REFLECT_CURSE:
            return LC_TEXT_LC("저주 되돌리기 확률 %d%%", locale);
        case POINT_POISON_REDUCE:
            return LC_TEXT_LC("독 저항 %d%%", locale);
        case POINT_KILL_SP_RECOVER:
            return LC_TEXT_LC("%d%% 확률로 적퇴치시 정신력 회복", locale);
        case POINT_EXP_DOUBLE_BONUS:
            return LC_TEXT_LC("%d%% 확률로 적퇴치시 경험치 추가 상승", locale);
        case POINT_GOLD_DOUBLE_BONUS:
            return LC_TEXT_LC("%d%% 확률로 적퇴치시 돈 2배 드롭", locale);
        case POINT_ITEM_DROP_BONUS:
            return LC_TEXT_LC("%d%% 확률로 적퇴치시 아이템 2배 드롭", locale);
        case POINT_POTION_BONUS:
            return LC_TEXT_LC("물약 사용시 %d%% 성능 증가", locale);
        case POINT_KILL_HP_RECOVERY:
            return LC_TEXT_LC("%d%% 확률로 적퇴치시 생명력 회복", locale);
            //		case POINT_IMMUNE_STUN:	return LC_TEXT_LC("기절하지 않음
            //%d%%", locale); 		case POINT_IMMUNE_SLOW:	return
            //LC_TEXT_LC("느려지지 않음 %d%%", locale); 		case POINT_IMMUNE_FALL:
            //return LC_TEXT_LC("넘어지지 않음 %d%%", locale); 		case POINT_SKILL:
            //return LC_TEXT_LC("", locale); 		case POINT_BOW_DISTANCE:	return
            //LC_TEXT_LC("", locale);
        case POINT_ATT_GRADE_BONUS:
            return LC_TEXT_LC("공격력 +%d", locale);
        case POINT_DEF_GRADE_BONUS:
            return LC_TEXT_LC("방어력 +%d", locale);
        case POINT_MAGIC_ATT_GRADE:
            return LC_TEXT_LC("마법 공격력 +%d", locale);
        case POINT_MAGIC_DEF_GRADE:
            return LC_TEXT_LC("마법 방어력 +%d", locale);
            //		case POINT_CURSE_PCT:	return LC_TEXT_LC("", locale);
        case POINT_MAX_STAMINA:
            return LC_TEXT_LC("최대 지구력 +%d", locale);
        case POINT_ATTBONUS_WARRIOR:
            return LC_TEXT_LC("무사에게 강함 +%d%%", locale);
        case POINT_ATTBONUS_ASSASSIN:
            return LC_TEXT_LC("자객에게 강함 +%d%%", locale);
        case POINT_ATTBONUS_SURA:
            return LC_TEXT_LC("수라에게 강함 +%d%%", locale);
        case POINT_ATTBONUS_SHAMAN:
            return LC_TEXT_LC("무당에게 강함 +%d%%", locale);
        case POINT_ATTBONUS_MONSTER:
            return LC_TEXT_LC("몬스터에게 강함 +%d%%", locale);
        case POINT_MALL_ATTBONUS:
            return LC_TEXT_LC("공격력 +%d%%", locale);
        case POINT_MALL_DEFBONUS:
            return LC_TEXT_LC("방어력 +%d%%", locale);
        case POINT_MALL_EXPBONUS:
            return LC_TEXT_LC("경험치 %d%%", locale);
        case POINT_MALL_ITEMBONUS:
            return LC_TEXT_LC("아이템 드롭율 %.1f배", locale);
        case POINT_MALL_GOLDBONUS:
            return LC_TEXT_LC("돈 드롭율 %.1f배", locale);
        case POINT_MAX_HP_PCT:
            return LC_TEXT_LC("최대 생명력 +%d%%", locale);
        case POINT_MAX_SP_PCT:
            return LC_TEXT_LC("최대 정신력 +%d%%", locale);
        case POINT_SKILL_DAMAGE_BONUS:
            return LC_TEXT_LC("스킬 데미지 %d%%", locale);
        case POINT_NORMAL_HIT_DAMAGE_BONUS:
            return LC_TEXT_LC("평타 데미지 %d%%", locale);
        case POINT_SKILL_DEFEND_BONUS:
            return LC_TEXT_LC("스킬 데미지 저항 %d%%", locale);
        case POINT_NORMAL_HIT_DEFEND_BONUS:
            return LC_TEXT_LC("평타 데미지 저항 %d%%", locale);
            //		case POINT_PC_BANG_EXP_BONUS:	return LC_TEXT_LC("",
            //locale); 		case POINT_PC_BANG_DROP_BONUS:	return LC_TEXT_LC("",
            //locale); 		case POINT_EXTRACT_HP_PCT:	return LC_TEXT_LC("",
            //locale);
        case POINT_RESIST_WARRIOR:
            return LC_TEXT_LC("무사공격에 %d%% 저항", locale);
        case POINT_RESIST_ASSASSIN:
            return LC_TEXT_LC("자객공격에 %d%% 저항", locale);
        case POINT_RESIST_SURA:
            return LC_TEXT_LC("수라공격에 %d%% 저항", locale);
        case POINT_RESIST_SHAMAN:
            return LC_TEXT_LC("무당공격에 %d%% 저항", locale);
        case POINT_BLEEDING_PCT:
            return LC_TEXT_LC("무당공격에 %d%% 저항", locale);
        case POINT_BLEEDING_REDUCE:
            return LC_TEXT_LC("무당공격에 %d%% 저항", locale);
        case POINT_ATTBONUS_WOLFMAN:
            return LC_TEXT_LC("무당에게 강함 +%d%%", locale);
        case POINT_RESIST_WOLFMAN:
            return LC_TEXT_LC("무당공격에 %d%% 저항", locale);
        case POINT_RESIST_CLAW:
            return LC_TEXT_LC("한손검 방어 %d%%", locale);
        default:
            return nullptr;
    }
}

ACMD(do_costume)
{
    char buf[512];
    const size_t bufferSize = sizeof(buf);

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
    CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
    CItem* pMount = ch->GetWear(WEAR_COSTUME_MOUNT);
    CItem* pAcce = ch->GetWear(WEAR_COSTUME_ACCE);

    ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

    if (pHair) {
        const char* itemName = pHair->GetName();
        ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

        for (int i = 0; i < pHair->GetAttributeCount(); ++i) {
            const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
            if (0 < attr.bType) {
                std::string pointString =
                    fmt::sprintf(FN_point_string(ch, attr.bType), attr.sValue);
                ch->ChatPacket(CHAT_TYPE_INFO, "     %s", pointString.c_str());
            }
        }

        if (pHair->IsEquipped() && arg1[0] == 'h')
            ch->UnequipItem(pHair);
    }

    if (pBody) {
        const char* itemName = pBody->GetName();
        ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

        if (pBody->IsEquipped() && arg1[0] == 'b')
            ch->UnequipItem(pBody);
    }

    if (pMount) {
        const char* itemName = pMount->GetName();
        ch->ChatPacket(CHAT_TYPE_INFO, "  MOUNT : %s", itemName);

        if (pMount->IsEquipped() && arg1[0] == 'm')
            ch->UnequipItem(pMount);
    }

    if (pAcce) {
        const char* itemName = pAcce->GetName();
        ch->ChatPacket(CHAT_TYPE_INFO, "  ACCE : %s", itemName);

        if (pAcce->IsEquipped() && arg1[0] == 'a')
            ch->UnequipItem(pAcce);
    }
}

ACMD(do_inventory)
{
    int index = 0;
    int count = 1;

    char arg1[256];
    char arg2[256];

    CItem* item;

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (!*arg1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> "
                                       "<count>");
        return;
    }

    if (!*arg2) {
        index = 0;
        str_to_number(count, arg1);
    } else {
        str_to_number(index, arg1);
        index = std::min<int>(index, INVENTORY_MAX_NUM);
        str_to_number(count, arg2);
        count = std::min<int>(count, INVENTORY_MAX_NUM);
    }

    for (int i = 0; i < count; ++i) {
        if (index >= INVENTORY_MAX_NUM)
            break;

        item = ch->GetInventoryItem(index);

        ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s", index,
                       item ? item->GetName() : "<NONE>");
        ++index;
    }
}

ACMD(do_cube)
{
    if (!ch->CanDoCube())
        return;

    const char* line;

    char arg1[256], arg2[256], arg3[256];

    line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
    one_argument(line, arg3, sizeof(arg3));

    if (0 == arg1[0]) {
        // print usage
        ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
        ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
        ch->ChatPacket(CHAT_TYPE_INFO, "       cube make");
        return;
    }

    const std::string& strArg1 = std::string(arg1);

    switch (LOWER(arg1[0])) {
        case 'o': // open
            Cube_open(ch);
            break;

        case 'c': // close
            Cube_close(ch);
            break;

        case 'm': // make
            GenericVnum cubeVnum;
            storm::ParseNumber(arg2, cubeVnum);
            Cube_make(ch, cubeVnum);
            break;

        default:
            return;
    }
}

ACMD(do_switchbot)
{
    ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSwitchbot");
}

ACMD(do_in_game_mall)
{
    std::string sas =
        MD5(fmt::format("{0}{1}{2}", ch->GetDesc()->GetAid(),
                        ch->GetDesc()->GetLogin(), gConfig.ishopSas))
            .toStr();
    std::transform(sas.begin(), sas.end(), sas.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    std::string buf = fmt::format("mall http://{0}/?aid={1}&sid={2}&sas={3}",
                                  gConfig.ishopUrl, ch->GetDesc()->GetAid(),
                                  gConfig.serverId, sas);

    ch->ChatPacket(CHAT_TYPE_COMMAND, buf.c_str());
}

// 주사위
ACMD(do_dice)
{
    char arg1[256], arg2[256];
    int start = 1, end = 100;

    two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

    if (*arg1 && *arg2) {
        start = atoi(arg1);
        end = atoi(arg2);
    } else if (*arg1 && !*arg2) {
        start = 1;
        end = atoi(arg1);
    }

    end = std::max(start, end);
    start = std::min(start, end);

    int n = Random::get(start, end);

    if (ch->GetParty())
        ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO,
                                              LC_TEXT("%s님이 주사위를 굴려 "
                                                      "%d가 나왔습니다. "
                                                      "(%d-%d)"),
                                              ch->GetName(), n, start, end);
    else
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "당신이 주사위를 굴려 %d가 나왔습니다. (%d-%d)", n,
                           start, end);
}

ACMD(do_click_mall)
{
    ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

ACMD(do_ride)
{
    if (!ch->IsNextMountPulse()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "Bunu bu kadar hızlı yapamazsın, lütfen biraz sakinleş.");
        return;
    }

    if (ch->IsDead() || ch->IsStun())
        return;

    if (ch->IsRiding()) {
        Unmount(ch);
        return;
    }

    if (!Mount(ch))
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "말을 먼저 소환해주세요.");

    ch->UpdateMountPulse();
}

using ItemQuickSlotPair = std::pair<CItem*, uint16_t>;

ACMD(do_sort_inventory)
{
  

    if (!ch->CanHandleItem() || ch->GetExchange() || ch->IsShop() ||
        ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You have some windows open blocking sorting your "
                           "inventory.");
        return;
    }

    if (thecore_heart->pulse - (int)ch->GetLastInventorySortPulse() <
            THECORE_SECS_TO_PASSES(15) &&
        !ch->IsGM()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Inventory sorting only every 15 seconds");
        return;
    }

    ch->SetLastInventorySortPulse(thecore_heart->pulse);
    ch->LevelPetClose();

    for (uint32_t i = 0; i < INVENTORY_MAX_NUM;
         ++i) {
        auto* item = ch->GetInventoryItem(i);

        if (!item)
            continue;

        if (item->isLocked())
            continue;

        CountType count = item->GetCount();

        if (item->IsStackable()) {

            for (int j = 0; j < INVENTORY_MAX_NUM; ++j) {

                auto* item2 = ch->GetInventoryItem(j);
                if(!item2)
                    continue;

                if(!CanStack(item, item2))
                    continue;

                if (!CanModifyItem(item2))
                        continue;

                CountType count2 = std::min<CountType>(
                    GetItemMaxCount(item2) - item2->GetCount(), count);
                count -= count2;

                item2->SetCount(item2->GetCount() + count2);

                if (count == 0) {
                    M2_DESTROY_ITEM(item);
                    break;
                }
            }
        }
        
        // Only update the count if the original item isn't gone already.
        if (0 != count)
            item->SetCount(count);
    }

    LogManager::instance().CharLog(ch, 0, "STACK_INVENTORY", "");

    //std::vector<ItemQuickSlotPair> collectItems;
    //int totalSize = 0;
    //for (auto i = 0; i < INVENTORY_MAX_NUM; ++i) {
    //    auto item = ch->GetInventoryItem(i);
    //    if (item) {
    //        totalSize += item->GetSize();
    //        collectItems.emplace_back(
    //            item,
    //            ch->GetQuickslotPosition(QUICKSLOT_TYPE_ITEM, item->GetCell()));
    //    }
    //}
    //if (totalSize - 3 >= INVENTORY_MAX_NUM) {
    //    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
    //                       "Your inventory is full you cannot sort it");
    //    return;
    //}

    //for (auto& item : collectItems) {
    //    if (item.first)
    //        item.first->RemoveFromCharacter();
    //}

    //std::vector<CItem*> groundItems;

    //for (auto& sortedItem : collectItems) {
    //    auto* positionedItem = ch->AutoGiveItem(sortedItem.first, true);
    //    if (positionedItem) {
    //        TQuickslot t;
    //        t.type = QUICKSLOT_TYPE_ITEM;
    //        t.pos = positionedItem->GetCell();
    //        ch->SetQuickslot(sortedItem.second, t);
    //        if (positionedItem->GetPosition().window_type == GROUND) {
    //            groundItems.emplace_back(positionedItem);
    //            positionedItem->RemoveFromGround();
    //        }
    //    }
    //}

    //for (auto& groundItem : groundItems) {
    //    auto* positionedItem = ch->AutoGiveItem(groundItem, true);
    //    if (positionedItem && positionedItem->GetPosition().window_type == GROUND) {
    //        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
    //                           "No valid space found in inventory for %s",
    //                           TextTag::itemname(positionedItem->GetVnum()));
    //    }
    //}
}

ACMD(do_unstuck)
{
    // This command should only work when stuck
    if (SECTREE_MANAGER::instance().IsMovablePosition(ch->GetMapIndex(),
                                                      ch->GetX(), ch->GetY()))
        return;

    // Try to get a valid location to unstuck the player
    // ReSharper disable once CppPossiblyErroneousEmptyStatements

    PIXEL_POSITION SetPos;
    int TryCount = 0;
    bool FoundPos = false;

    // TODO: Test for a good TryCount
    while (TryCount < 15) {
        // Try to get a valid random position
        // TODO: the range needs some fine tuning
        if (SECTREE_MANAGER::instance().GetRandomLocation(
                ch->GetMapIndex(), SetPos, ch->GetX(), ch->GetY(), 500)) {
            // We found a good position
            FoundPos = true;
            break;
        }
        ++TryCount;
    };

    // Show the player at the new position
    if (FoundPos) {
        if (gConfig.testServer)
            ch->ChatPacket(CHAT_TYPE_INFO, "Found valid position in %d tries",
                           TryCount);

        ch->Show(ch->GetMapIndex(), SetPos.x, SetPos.y, SetPos.z);
        ch->Stop();

        return;
    }

    // We could not find a valid position notify the player
    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                       "Automatic unstuck did not find a valid position. Ask a "
                       "GM.");
}

ACMD(do_ds_refine_open)
{
    if (ch->DragonSoul_IsQualified())
        ch->DragonSoul_RefineWindow_Open(true);
}

ACMD(do_shop_items) {}

const std::string bioTimerCdrList[] = {
    "collect_quest_lv30",   "collect_quest_lv30_1", "collect_quest_lv30_2",
    "collect_quest_lv30_3", "collect_quest_lv30_4", "collect_quest_lv30_5",
    "collect_quest_lv30_6", "collect_quest_lv30_7",
};

const std::string dungeonTimerCdrList[] = {
    "deviltower_zone.__NEXT_TIME__", "devilcatacomb_zone.__NEXT_TIME__",
    "dragon_lair_new.__NEXT_TIME__", "snow_dungeon.__NEXT_TIME__",
    "flame_dungeon.__NEXT_TIME__",   "flame_dungeon.__NEXT_TIME__",

};

void RefreshTimerCDRs_G(CHARACTER* ch)
{
    quest::PC* curr =
        quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
    if (!curr)
        return;

    int dur = 0;
    std::string currBio;
    for (size_t i = 0; i < sizeof(bioTimerCdrList) / sizeof(std::string); ++i) {
        int currState =
            curr->GetFlag((bioTimerCdrList[i] + ".__status").c_str());
        if (currState == quest::CQuestManager::instance().GetQuestStateIndex(
                             bioTimerCdrList[i], "go_to_disciple")) {
            dur = curr->GetFlag((bioTimerCdrList[i] + ".duration").c_str()) -
                  get_global_time();
            currBio =
                &(bioTimerCdrList[i].c_str()[bioTimerCdrList[i].length() - 2]);
            break;
        }
    }
    ch->ChatPacket(CHAT_TYPE_COMMAND, "timer_cdr 0 %d", dur);
    ch->ChatPacket(CHAT_TYPE_COMMAND, "curr_biolog %s",
                   currBio.length() ? currBio.c_str() : "0");

    for (size_t i = 0; i < sizeof(dungeonTimerCdrList) / sizeof(std::string);
         ++i) {
        dur = curr->GetFlag(dungeonTimerCdrList[i].c_str()) - get_global_time();
        ch->ChatPacket(CHAT_TYPE_COMMAND, "timer_cdr %d %d", i + 1,
                       dur > 0 ? dur : 0);
    }

    // dur = 0;
    // if (ch->GetGuild())
    // dur = ch->GetGuild()->GetDungeonCooldown() - get_global_time();
    // ch->ChatPacket(CHAT_TYPE_COMMAND, "timer_cdr %d %d",
    // sizeof(dungeonTimerCdrList) / sizeof(std::string) + 1, dur);
}

ACMD(do_get_timer_cdrs)
{
    RefreshTimerCDRs_G(ch);
}

ACMD(do_timer_warp)
{
    if (quest::CQuestManager::instance().GetEventFlag("disable_timer_warp") ==
        1) {
        ch->ChatPacket(CHAT_TYPE_INFO, "This doesn't work currently!");
        return;
    }

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    int iReq;
    if (!*arg1 || !isNumeric(arg1) || !str_to_number(iReq, arg1)) {
        ch->ChatPacket(CHAT_TYPE_INFO, "usage: timer_warp <pos>");
        return;
    }

    int lMapIndex = 0;
    int x = 0;
    int y = 0;

    switch (iReq) {
            // Biologist
        case 0: {
            if (ch->GetEmpire() == 1) {
                lMapIndex = 1;
                x = 881;
                y = 610;
            } else if (ch->GetEmpire() == 2) {
                lMapIndex = 3;
                x = 885;
                y = 797;
            } else if (ch->GetEmpire() == 3) {
                lMapIndex = 5;
                x = 301;
                y = 282;
            }
            break;
        }

            // Ape
        case 1: {
            lMapIndex = 0;
            x = 283;
            y = 1448;
            break;
        }

            // Demontower
        case 2: {
            lMapIndex = 14;
            x = 529;
            y = 600;
            break;
        }

            // Spider Queen
        case 3: {
            lMapIndex = 19;
            x = 178;
            y = 469;
            break;
        }

            // Azrael
        case 4: {
            lMapIndex = 14;
            x = 540;
            y = 488;
            break;
        }

            // Dragon
        case 5: {
            lMapIndex = 21;
            x = 278;
            y = 174;
            break;
        }

            // Nemere
        case 6: {
            lMapIndex = 10;
            x = 736;
            y = 114;
            break;
        }

            // Razador
        case 7: {
            lMapIndex = 11;
            x = 251;
            y = 926;
            break;
        }
        default: {
            SPDLOG_ERROR("timer_warp wrong index {}", iReq);
            return;
        }
    }

    x *= 100;
    y *= 100;

    const TMapRegion* region =
        SECTREE_MANAGER::instance().GetMapRegion(lMapIndex);

    if (!region) {
        SPDLOG_ERROR("invalid map index {}", lMapIndex);
        return;
    }

    if (ch->GetGold() < 250000) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You don't have 250.000 Yang.");
        return;
    }

    bool bRet = ch->WarpSet(region->index, x, y);
    if (!bRet) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You warp to this location now.");
    } else {
        ch->ChangeGold(-250000);
    }
}

ACMD(do_dungeon_rejoin)
{
    const char* szHelp = "/dungeon_rejoin <rejoin>";
    CHECK_COMMAND(Parser, 1, szHelp);

    const bool reconnect = Parser.GetTokenBool(0);
    auto& dngMgr = CDungeonManager::instance();

    auto mapIndex = dngMgr.GetPlayerInfo(ch->GetPlayerID());

    if (!reconnect)
        return;

    if (!mapIndex) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<Dungeon> Could not rejoin the dungeon as it has already expired.");
    } else if (mapIndex != ch->GetMapIndex()) {

        auto* d = dngMgr.FindByMapIndex<CDungeon>(mapIndex);
        if (d) {
            d->Rejoin(ch);
        }
    }
}

#ifdef ENABLE_NEW_GUILD_WAR
ACMD(do_new_guild_war)
{
    std::vector<std::string> vecArgs;
    split_argument(argument, vecArgs);

    if (vecArgs.size() < 8) {
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_MOUNT_UNKNOWN_ARGUMENT"));
        return;
    }

    int skor = 0;
    BYTE minseviye = 0;
    BYTE maxplayer = 0;
    BYTE binekkullanimi = 0;

    BYTE savasci, ninja, sura, shaman = 0;

    str_to_number(skor, vecArgs[1].c_str());
    str_to_number(minseviye, vecArgs[2].c_str());
    str_to_number(maxplayer, vecArgs[3].c_str());
    str_to_number(binekkullanimi, vecArgs[4].c_str());
    str_to_number(savasci, vecArgs[5].c_str());
    str_to_number(ninja, vecArgs[6].c_str());
    str_to_number(sura, vecArgs[7].c_str());
    str_to_number(shaman, vecArgs[8].c_str());

    if (skor < 1 || minseviye < 1 || maxplayer < 1)
        return;

    if (skor > 500)
        skor = 500;

    if (!ch->GetGuild()) {
        return;
    }

    CGuild* g = ch->GetGuild();
    const auto* gm = g->GetMember(ch->GetPlayerID());

    if (g->UnderAnyWar()) {
        return;
    }

    if (gm->grade == GUILD_LEADER_GRADE) {

        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "binekkullanimi", g->GetName(), binekkullanimi);
        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "skor", g->GetName(), skor);
        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "maxplayer", g->GetName(), maxplayer);
        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "minseviye", g->GetName(), minseviye);

        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "savasci", g->GetName(), savasci);
        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "ninja", g->GetName(), ninja);
        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "sura", g->GetName(), sura);
        quest::CQuestManager::instance().RequestSetEventFlagBR(
            "shaman", g->GetName(), shaman);

    } else {
    }
}
#endif