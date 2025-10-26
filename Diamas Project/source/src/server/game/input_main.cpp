
#include "DbCacheSocket.hpp"
#include "DragonSoul.h"
#include "GBanWord.h"
#include "GBattle.h"
#include "GBufferManager.h"
#include "OXEvent.h"
#include "PetSystem.h"
#include "building.h"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "config.h"
#include "constants.h"
#include "desc_manager.h"
#include "exchange.h"
#include "gm.h"
#include "guild.h"
#include "guild_manager.h"
#include "item.h"
#include "item_manager.h"
#include "locale_service.h"
#include "log.h"
#include "messenger_manager.h"
#include "motion.h"
#include <game/AffectConstants.hpp>

#include "party.h"
#include "questmanager.h"
#include "regen.h"
#include "safebox.h"
#include "shop.h"
#include "shop_manager.h"
#include <game/GamePacket.hpp>

#include "utils.h"
#include "xmas_event.h"
#include <boost/algorithm/string.hpp>
#include <game/grid.h>

#include "ItemUtils.h"
#include "TextTagUtil.hpp"
#include "war_map.h"
#include <boost/algorithm/clamp.hpp>
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>
#include <storm/StringFlags.hpp>

#include "ChatUtil.hpp"
#include "MasterUtil.hpp"
#include "OfflineShop.h"
#include "OfflineShopManager.h"
#include "PacketUtils.hpp"
#include "desc.h"
#include "dungeon_info.h"
#include "input.h"
#include "mob_manager.h"

#if defined(WJ_COMBAT_ZONE)
#include "combat_zone.h"
#endif

#ifdef ENABLE_BATTLE_PASS
#include "battle_pass.h"
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
#include "entity.h"
#include "locale.hpp"
#include <unordered_map>

struct SearchSendItem {
    uint32_t vnum; // 아이템 번호
    Gold price;    // 가격
    uint8_t count;
    CItem* item;
};

bool CompareItemVnumAcPriceAC(const SearchSendItem& i, const SearchSendItem& j)
{
    return (i.vnum < j.vnum);
}

bool CompareItemVnumAcPriceAC(const TOfflineShopItemData& i,
                              const TOfflineShopItemData& j)
{
    return (i.info.vnum < j.info.vnum);
}
#endif

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

namespace
{
bool IsMotionModeValid(CHARACTER* ch, uint32_t mode)
{
    if (mode == ch->GetMotionMode())
        return true;

    uint32_t generalMode = ch->GetMotionMode();
    switch (generalMode) {
        case MOTION_MODE_HORSE_ONEHAND_SWORD:
        case MOTION_MODE_HORSE_TWOHAND_SWORD:
        case MOTION_MODE_HORSE_DUALHAND_SWORD:
        case MOTION_MODE_HORSE_FAN:
        case MOTION_MODE_HORSE_BELL:
        case MOTION_MODE_HORSE_BOW:
        case MOTION_MODE_HORSE_CLAW:
            generalMode = MOTION_MODE_HORSE;
            break;

        default:
            generalMode = MOTION_MODE_GENERAL;
            break;
    }

    if (mode != generalMode) {
        SPDLOG_DEBUG("{0}: Server mode {1}/{3} != Client mode {2}",
                     ch->GetName(), ch->GetMotionMode(), mode, generalMode);
        return false;
    }

    return true;
}

bool TrySync(CHARACTER* ch, CHARACTER* victim, int32_t x, int32_t y,
             bool isSkill = false)
{
    static const int kMaxSyncDistance = 2500 + 1000;

    const auto syncDist =
        DISTANCE_APPROX(victim->GetX() - x, victim->GetY() - y);

    if (syncDist > kMaxSyncDistance) {
        LogManager::instance().HackLog("SYNC_POSITION_DIST", ch);

        SPDLOG_ERROR("SyncPosition too far (attacker {0} {1} {2}, victim {3} "
                     "{4} {5} dst {6} {7})",
                     ch->GetName(), ch->GetX(), ch->GetY(), victim->GetName(),
                     victim->GetX(), victim->GetY(), x, y);
        return false;
    }

    const uint32_t delta = ch->IsRiding() ? 500 : (isSkill ? 800 : 300);

    const auto ownerDist = DISTANCE_APPROX(victim->GetX() - ch->GetX(),
                                           victim->GetY() - ch->GetY());
    if (ownerDist > delta) {
        SPDLOG_DEBUG("Distance {0} > {1}", ownerDist, delta);
        return false;
    }

    if (!victim->Sync(x, y)) {
        LogManager::instance().HackLog("SYNC_POSITION_DEST", ch);

        SPDLOG_ERROR("SyncPosition destination invalid ({0} {1} attacker {2} "
                     "victim {3})",
                     x, y, ch->GetName(), victim->GetName());
        return false;
    }

    victim->SyncPacket();
    return true;
}

bool CheckSpeedHack(CHARACTER* ch, uint32_t serverTime, uint32_t clientTime)
{
    static const int32_t kMinSyncAge = 7 * 1000;

    const int32_t syncAge = serverTime - ch->GetDesc()->GetClientTime();
    const int32_t deltaT = static_cast<int32_t>(serverTime - clientTime);

    SPDLOG_DEBUG("Checking {0}: deltaT {1} sync-age {2}", ch->GetName(), deltaT,
                 syncAge);

    if (!ch->GetDesc()->IsHandshaking() && syncAge > kMinSyncAge) {
        if (deltaT >= 30000) {
            // Client time-tracking was slowed down - timeGetTime hooks,
            // etc.
           // SPDLOG_CRITICAL("SPEEDHACK: slow timer name {0} delta {1} age {2}",
            //                ch->GetName(), deltaT, syncAge);

            //LogManager::instance().HackLog("SPEEDHACK_SLOW", ch);
            // ch->GetDesc()->DelayedDisconnect(3, "SPEEDHACK_SLOW");
            ch->GetDesc()->SendHandshake(get_dword_time(), 0);
            return true;
        } else if (deltaT < 0 && -deltaT > syncAge / 100) {
            // Allow 1ms speedup per 50ms age
            if(gConfig.testServer)
                SPDLOG_INFO("SPEEDHACK DETECTED for {0} delta {1} age {2}",
                        ch->GetName(), deltaT, syncAge);

            LogManager::instance().HackLog("SPEEDHACK_FAST", ch);
            ch->GetDesc()->DelayedDisconnect(3, "SPEEDHACK_FAST");
            return false;
        }
    }

    return true;
}

bool ValidateMovement(CHARACTER* ch, uint32_t clientTime, int32_t x, int32_t y)
{
    static const uint32_t kGlobalMaxDistance = 50 * 100; // 50m
    static const uint32_t kAllowedDelta = 400;           // 400ms

    const uint32_t distance = DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y);
    if (distance >= kGlobalMaxDistance) {
        SPDLOG_DEBUG("{0}: moved too far: {1}cm", ch->GetName(), distance);

        return false;
    }

    auto moveMotion = ch->GetMoveMotion();
    if (!moveMotion) {
        SPDLOG_ERROR("{0}: null movement motion for mode {1}",
                     ch->GetMotionMode());
        return false;
    }

    auto expectedDur = CalculateMotionMovementTime(ch, moveMotion, distance);
    const auto actualDur = clientTime - ch->GetClientLastMoveTime();

    SPDLOG_TRACE("{0}: move: actual {1} expected {2}", ch->GetName(), actualDur,
                 expectedDur);

    if (actualDur + kAllowedDelta < expectedDur) {
        SPDLOG_WARN("{0}: move excess: last {1} {2} req {3} {4}", ch->GetName(),
                    ch->GetX(), ch->GetY(), x, y);

        // Check again by using the run motion even if the player might have the
        // walk motion
        moveMotion = ch->GetMoveMotion(true);
        if (!moveMotion) {
            SPDLOG_ERROR("{0}: null running motion for mode {1}",
                         ch->GetMotionMode());
            return false;
        }

        expectedDur = CalculateMotionMovementTime(ch, moveMotion, distance);
        if (actualDur + kAllowedDelta < expectedDur) {
            SPDLOG_WARN("{0}: 2nd move excess: last {1} {2} req {3} {4}",
                        ch->GetName(), ch->GetX(), ch->GetY(), x, y);
            return false;
        }
    }

    ch->SetClientLastMoveTime(clientTime);
    return true;
}

bool IsValidPosition(CHARACTER* ch, int32_t x, int32_t y)
{
    // Number of adjacent cells that will get checked as well.
    static const int32_t kThreshold = 3 * CELL_SIZE;

    const auto map = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());
    if (!map) {
        SPDLOG_ERROR("{0}: No map bound", ch->GetName());
        return false;
    }

    for (int32_t dy = y - kThreshold; dy != y + kThreshold; dy += CELL_SIZE) {
        for (int32_t dx = x - kThreshold; dx != x + kThreshold;
             dx += CELL_SIZE) {
            auto tree = map->Find(x, y);
            if (!tree)
                continue;

            if (!tree->IsAttr(dx, dy, ATTR_BLOCK))
                return true;
        }
    }

    return false;
}

bool CheckAttackMotion(CHARACTER* ch, uint32_t clientTime, uint32_t motionKey)
{
    // This attack motion arrived before our last attack motion finished
    // Sounds like a hack
    if (clientTime < ch->GetLastComboTime() + ch->GetComboInputTime()) {
        SPDLOG_INFO("attack-speed hack: now {0} last {1} valid {2}", clientTime,
                    ch->GetLastComboTime(), ch->GetComboInputTime());

        LogManager::instance().HackLog("ATTACK_SPEED_HACK", ch);
        ch->GetDesc()->SendHandshake(get_dword_time(), 0);
        return false;
    }

    const auto key = MakeMotionKey(ch->GetMotionMode(), motionKey);

    ch->SetLastComboTime(clientTime);
    ch->SetComboInputTime(GetAttackMotionInputTime(ch, key));
    ch->SetComboDuration(GetAttackMotionDuration(ch, key));
    return true;
}

bool CheckAttackHitCount(CHARACTER* ch, CHARACTER* victim)
{
    uint32_t maxHitCount = 1;

    const auto item = ch->GetWear(WEAR_WEAPON);
    if (item && (item->GetSubType() == WEAPON_DAGGER ||
                 item->GetSubType() == WEAPON_CLAW))
        maxHitCount = 3;

    if (ch->GetHitLog().Hit(victim->GetVID()) >= maxHitCount)
        return false;

    return true;
}
} // namespace

extern void SendShout(const char* szText, CHARACTER* ch);
extern void SendEmpireShout(const char* szText);

static Gold __deposit_limit()
{
    return 5'000'000'000; // 1천만
}

static const char* GetEmpireName(int empire)
{
    return c_apszEmpireNamesAlt[empire];
}

static const char* GetEmpireColor(int empire)
{
    return c_apszEmpireColor[empire];
}

void SendBlockChatInfo(CHARACTER* ch, int sec)
{
    if (sec <= 0) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "채팅 금지 상태입니다.");
        return;
    }

    long hour = sec / 3600;
    sec -= hour * 3600;

    long min = (sec / 60);
    sec -= min * 60;

    char buf[128 + 1];

    if (hour > 0 && min > 0)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "%d 시간 %d 분 %d 초 동안 채팅금지 상태입니다", hour,
                           min, sec);
    else if (hour > 0 && min == 0)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "%d 시간 %d 초 동안 채팅금지 상태입니다", hour, sec);
    else if (hour == 0 && min > 0)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "%d 분 %d 초 동안 채팅금지 상태입니다", min, sec);
    else
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%d 초 동안 채팅금지 상태입니다",
                           sec);
}

enum GlassError {
    E_GLASS_OK,
    E_GLASS_NEED_MORE,
    E_GLASS_EXCHANGING,
    E_GLASS_HAS_WEB_LINK,
    E_GLASS_COLORED_WITHOUT_LINKS,
};

void HyperLinkRequest(std::string chatLine)
{
    std::vector<uint32_t> requestItems;

    TextTag::TextTag tag;
    for (auto first = chatLine.begin(), last = chatLine.end(); first != last;) {
        auto curString = std::string(first, last);
        if (GetTextTag(curString, tag)) {
            first += tag.length;
            std::vector<std::string> tokens;
            storm::Tokenize(tag.content, ":", tokens);

            if (tokens.size() > 0) {
                if (tokens[0] == "item") {
                    requestItems.emplace_back(std::stoi(tokens[2], 0, 16));
                }
            }
            continue;
        } else {
            ++first;
        }
    }

    for (const auto& val : requestItems) {
        if (auto d = ITEM_MANAGER::instance().GetHyperlinkItemData(val); d) {
            CgHyperlinkItemPacket p;
            p.d = d.value();
            BroadcastPacket(DESC_MANAGER::instance().GetClientSet(),
                            HEADER_GC_HYPERLINK_ITEM, p);
        } else if (auto item = ITEM_MANAGER::instance().Find(val); item) {
            ClientItemData d;
            d.id = val;
            d.vnum = item->GetVnum();
            d.count = item->GetCount();
            d.highlighted = false;
            d.nSealDate = item->GetSealDate();
            d.transVnum = item->GetTransmutationVnum();
            std::memcpy(d.attrs, item->GetAttributes(), sizeof(d.attrs));
            std::memcpy(d.sockets, item->GetSockets(), sizeof(d.sockets));

            ITEM_MANAGER::instance().RegisterHyperlink(val, d);
            CgHyperlinkItemPacket p;
            p.d = d;
            BroadcastPacket(DESC_MANAGER::instance().GetClientSet(),
                            HEADER_GC_HYPERLINK_ITEM, p);
        }
    }

    GmHyperlinkRequestPacket req{};
    req.ids = requestItems;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmHyperlinkItemRequest,
                                                     req);
}

void CInputMain::SendOfflineMessage(CHARACTER* ch, const TPacketCGWhisper& p)
{
    if (ch->GetLevel() >= gConfig.minOfflineMessageLevel &&
        GM::get_level(p.szNameTo.c_str()) == GM_PLAYER) {
        std::string checkName = p.szNameTo;
        if (checkName.find("[SYSTEM") == std::string::npos &&
            ch->IsNextOfflineMessagePulse()) {
            char msg[CHAT_MAX_LEN + 1];
            char cNameSender[CHARACTER_NAME_MAX_LEN + 1];
            char cNameTo[CHARACTER_NAME_MAX_LEN + 1];

            DBManager::instance().EscapeString(
                msg, sizeof(msg), p.message.c_str(), p.message.length());
            DBManager::instance().EscapeString(cNameSender, sizeof(cNameSender),
                                               ch->GetName().c_str(),
                                               ch->GetName().length());
            DBManager::instance().EscapeString(cNameTo, sizeof(cNameTo),
                                               p.szNameTo.data(),
                                               p.szNameTo.length());

            std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(
                "SELECT id FROM {}.player WHERE name='{}' AND deleted = 0 "
                "LIMIT 1;",
                gConfig.playerDb.name, cNameTo));

            if (pMsg->Get()->uiNumRows == 1) {
                MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

                uint32_t pid;
                str_to_number(pid, row[0]);

                DBManager::Instance().Query("INSERT INTO {}.offline_messages "
                                            "(pid, sender, message) "
                                            "VALUES ({}, '{}', '{}');",
                                            gConfig.playerDb.name, pid,
                                            cNameSender, msg);
                LogManager::instance().WhisperLog(ch->GetPlayerID(), pid, msg);

                auto message = std::string(
                    LC_TEXT_LC("Offline message was sent", GetLocale(ch)));
                TPacketGCWhisper pack{};
                pack.bType = WHISPER_TYPE_SYSTEM;
                pack.pid = ch->GetPlayerID();
                pack.szNameFrom = "[SYSTEM]";
                pack.message = message;
                ch->GetDesc()->Send(HEADER_GC_WHISPER, pack);

                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "OFFLINE_MESSAGE_SUCCESS");
                ch->UpdateOfflineMessagePulse();
            }
        }
    } else {
        TPacketGCWhisper pack{};
        pack.bType = WHISPER_TYPE_NOT_EXIST;
        pack.szNameFrom = p.szNameTo;
        ch->GetDesc()->Send(HEADER_GC_WHISPER, pack);
    }
}

bool CInputMain::Whisper(CHARACTER* ch, const TPacketCGWhisper& p)
{
    if (!ch->GetDesc())
        return true;

    if (ch->FindAffect(AFFECT_BLOCK_CHAT)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "채팅 금지 상태입니다.");
        return true;
    }

#if defined(WJ_COMBAT_ZONE) && defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(ch->GetMapIndex()) &&
        (ch->GetGMLevel() < GM_IMPLEMENTOR)) {
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("cz_cannot_use_whisper"));
        return (true);
    }
#endif

    auto* target = g_pCharManager->FindPC(p.szNameTo);
    if (target == ch)
        return true;

    HyperLinkRequest(p.message);

    uint32_t targetPid = 0;

    const OnlinePlayer* op = nullptr;
    if (!target) {
        op = DESC_MANAGER::instance().GetOnlinePlayers().Get(p.szNameTo);
        if (!op) {
#ifdef __OFFLINE_MESSAGE__
            SendOfflineMessage(ch, p);
#endif
            return true;
        }

        targetPid = op->pid;
    } else {
        targetPid = target->GetPlayerID();
    }

    if (ch->IsBlockMode(BLOCK_WHISPER, targetPid) &&
        (!target || !target->IsGM())) {
        if (ch->GetDesc()) {
            TPacketGCWhisper pack{};
            pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
            pack.szNameFrom = p.szNameTo;
            ch->GetDesc()->Send(HEADER_GC_WHISPER, pack);
        }

        return true;
    }

    if (target && target->IsBlockMode(BLOCK_WHISPER, ch->GetPlayerID()) &&
        !ch->IsGM()) {
        if (ch->GetDesc()) {
            TPacketGCWhisper pack{};
            pack.bType = WHISPER_TYPE_TARGET_BLOCKED;
            pack.szNameFrom = p.szNameTo;
            ch->GetDesc()->Send(HEADER_GC_WHISPER, pack);
        }
        return true;
    }

    uint8_t bType = WHISPER_TYPE_NORMAL;
    if (ch->IsGM())
        bType |= WHISPER_TYPE_GM;

    std::string msg = p.message;

    //CBanwordManager::instance().ConvertString(msg);

    if (p.message.length() <= 0)
        return true;

    auto sourcePid = ch->GetPlayerID();
    auto sourceName = ch->GetName();
    auto sourceLc = GetLanguageIDByName(GetLocale(ch)->name);

    if (target) {
        SendWhisperPacket(target, bType, msg, sourceLc, sourcePid, sourceName);
    } else {
        RelayWhisperPacket(sourcePid, targetPid, bType, sourceLc, sourceName,
                           msg);
    }

    LogManager::Instance().WhisperLog(ch->GetPlayerID(), targetPid,
                                      p.message.c_str());
    return true;
}

bool CInputMain::Chat(CHARACTER* ch, const TPacketCGChat& p)
{
    const auto chatType = p.type;

    std::string chatLine = p.message;

    if (chatLine.length() > 1 && chatLine[0] == '/') {
        interpret_command(ch, chatLine.data() + 1, chatLine.length() - 1);
        return true;
    }

    ch->IncreaseChatCounter();
    if (ch->GetChatCounter() >= 15) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "Do not spam the chat!");
        ch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, 30, 0, false);
        return true;
    }

    HyperLinkRequest(chatLine);

    // 채팅 금지 Affect 처리
    const auto pAffect = ch->FindAffect(AFFECT_BLOCK_CHAT);
    if (pAffect) {
        SendBlockChatInfo(ch, pAffect->duration);
        return true;
    }

#if defined(WJ_COMBAT_ZONE) && defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
    if (CCombatZoneManager::Instance().IsCombatZoneMap(ch->GetMapIndex()) &&
        (ch->GetGMLevel() < GM_IMPLEMENTOR)) {
        ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("cz_cannot_use_chat"));
        return true;
    }
#endif
    // We have some special formating with TextTags we need to to here

    TPacketGCChat chatPacket = {};
    chatPacket.type = chatType;
    chatPacket.id = ch->GetVID();

    switch (chatType) {
        case CHAT_TYPE_SHOUT:
        case CHAT_TYPE_EMPIRE: {
            const auto name = fmt::format(
                "{} {}",
                TextTag::hyperlink("pmn", ch->GetName(), ch->GetName()),
                TextTag::color("ff98ff33",
                               fmt::format("Lv {}", ch->GetLevel())));

            std::string empireString;

            switch (ch->GetEmpire()) {
                case EMPIRE_A:
                    empireString = "shinsoo";
                    break;
                case EMPIRE_B:
                    empireString = "chunjo";
                    break;
                case EMPIRE_C:
                    empireString = "jinno";
                    break;
            }

            chatPacket.message =
                fmt::format("{}{}{}: {}", TextTag::emote(GetLocale(ch)->name),
                            TextTag::emote(empireString), name, chatLine);
        } break;
        default: {
            const auto name =
                TextTag::hyperlink("pmn", ch->GetName(), ch->GetName());
            chatPacket.message = fmt::format("{0}: {1}", name, chatLine);
        } break;
    }

    LogManager::instance().ChatLog(ch, chatType, chatLine.c_str());

    auto shoutLeveLimit = gConfig.shoutLevelLimit;
    if (chatType == CHAT_TYPE_SHOUT || chatType == CHAT_TYPE_EMPIRE) {
        if (chatType == CHAT_TYPE_EMPIRE)
            shoutLeveLimit = 1;

        if (ch->GetLevel() < shoutLeveLimit) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You need to be atleast level %d to use the "
                               "shout chat.",
                               gConfig.shoutLevelLimit);
            return (true);
        }

        if (thecore_heart->pulse - (int)ch->GetLastShoutPulse() <
                THECORE_SECS_TO_PASSES(1) * 8 &&
            !ch->IsGM())
            return (true);

        ch->SetLastShoutPulse(thecore_heart->pulse);

        BroadcastShout(chatPacket.message.c_str(), ch->GetEmpire(), p.lang);

        return (true);
    }

    switch (chatType) {
        case CHAT_TYPE_TALKING: {
            const auto& cRefSet = DESC_MANAGER::instance().GetClientSet();
            std::for_each(
                cRefSet.begin(), cRefSet.end(), [ch, chatPacket, p](DescPtr d) {
                    if (!d->GetCharacter())
                        return;

                    if (d->GetCharacter()->IsChatFilter(p.lang))
                        return;

                    if (d->GetCharacter()->GetMapIndex() != ch->GetMapIndex())
                        return;

                    if (auto* descCh = d->GetCharacter(); descCh) {
                        if (descCh->IsBlockMode(BLOCK_CHAT, ch->GetPlayerID()))
                            return;
                    }

                    d->Send(HEADER_GC_CHAT, chatPacket);
                });
        } break;

        case CHAT_TYPE_PARTY: {
            if (!ch->GetParty())
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "파티 중이 아닙니다.");
            else {
                ch->GetParty()->ForEachOnlineMember([ch, &chatPacket](
                                                        CHARACTER* c) {
                    if (!c->GetDesc())
                        return;

                    if (auto descCh = c->GetDesc()->GetCharacter(); descCh) {
                        if (descCh->IsBlockMode(BLOCK_CHAT, ch->GetPlayerID()))
                            return;
                    }

                    c->GetDesc()->Send(HEADER_GC_CHAT, chatPacket);
                });
            }
        } break;

        case CHAT_TYPE_GUILD: {
            if (!ch->GetGuild())
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "길드에 가입하지 않았습니다.");
            else
                ch->GetGuild()->Chat(chatPacket.message.c_str());
        } break;

        default:
            SPDLOG_ERROR("Unknown chat type {}", p.type);
            break;
    }

    return (true);
}

void CInputMain::SetTitle(CHARACTER* ch, const CgSetTitlePacket& p)
{
    if (!ch->IsGM()) {
        if (!ch->CountSpecifyTypeItem(ITEM_USE, USE_SET_TITLE)) {
            return;
        }
    }

    if (!check_name(p.szTitle.c_str())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "There are invalid characters in your title.");
        return;
    }

    if (!ch->IsGM()) {
        if (!ch->CountSpecifyTypeItem(ITEM_USE, USE_SET_TITLE)) {
            return;
        }
        auto item = ch->FindSpecifyTypeItem(ITEM_USE, USE_SET_TITLE);
        if (!item)
            return;
        ITEM_MANAGER::instance().RemoveItem(item, "SET_TITLE");
    }

    ch->SetTitle(p.szTitle, p.color);
}

void CInputMain::ItemUse(CHARACTER* ch, const TPacketCGItemUse& p)
{
    ch->UseItem(p.Cell);
}

void CInputMain::ItemUseMultiple(CHARACTER* ch, const TPacketCGItemUse& p)
{
    CItem* pkItem = ch->GetItem(p.Cell);
    if (!pkItem)
        return;

    for (int i = 0; i < 50; ++i) {
        if (!ch->UseItem(p.Cell, NPOS, true)) {
            return;
        }
    }
}

void CInputMain::ItemToItem(CHARACTER* ch, const TPacketCGItemUseToItem& p)
{
    if (ch)
        ch->UseItem(p.Cell, p.TargetCell);
}

void CInputMain::ItemDrop(CHARACTER* ch, const TPacketCGItemDrop& p)
{
    if (!ch)
        return;

    // 엘크가 0보다 크면 엘크를 버리는 것 이다.
    if (p.gold > 0)
        ch->DropGold(p.gold);
    else
        ch->DropItem(p.Cell);
}

void CInputMain::ItemDrop2(CHARACTER* ch, const TPacketCGItemDrop2& p)
{
    // 엘크가 0보다 크면 엘크를 버리는 것 이다.

    if (!ch)
        return;
    if (p.gold > 0)
        ch->DropGold(p.gold);
    else
        ch->DropItem(p.Cell, p.count);
}

#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
void CInputMain::ItemDestroy(CHARACTER* ch, const TPacketCGItemDestroy& p)
{
    if (!ch)
        return;

    ch->RemoveItem(p.Cell);
}
#endif

void CInputMain::ItemRemoveMetin(CHARACTER* ch, const CgRemoveMetinPacket& p)
{
    if (!ch)
        return;

    if (const auto item = ch->GetItem(p.targetItem); item) {
        const auto socketItem = item->GetSocket(p.slot);

        if (!ITEM_MANAGER::instance().GetTable(socketItem))
            return; // There is no item in the given socket;

        ch->AutoGiveItem(socketItem);
        item->SetSocket(p.slot, 1);
        item->UpdatePacket();
    }
}

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM

struct FCollectShops {
    std::vector<CHARACTER*> shops;
    CHARACTER* me;

    FCollectShops(CHARACTER* ch)
        : me(ch){};

    void operator()(CEntity* ent)
    {
        if (ent->IsType(ENTITY_CHARACTER)) {
            CHARACTER* ch = static_cast<CHARACTER*>(ent);
            if (ch && ch->IsShop() && ch->GetMyShop() && ch != me) {
                shops.push_back(ch);
            }
        }
    }
};

void CInputMain::ShopSearch(CHARACTER* ch, const TPacketCGShopSearch& p)
{
    if (!ch)
        return;

    if (ch->GetExchange() || ch->GetMyShop() || ch->IsOpenSafebox() ||
        ch->GetShopOwner() || ch->IsCubeOpen() || ch->IsAcceWindowOpen()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "CANT_DO_THIS_BECAUSE_OTHER_WINDOW_OPEN");
        return;
    }
#ifdef __ENABLE_GROWTH_PET_SYSTEM__
    if (ch->IsOpenPetHatchingWindow()) {
        ch->SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "CANT_DO_THIS_BECAUSE_PET_HATCHING_WINDOW_OPEN");
        return;
    }
#endif

    if (0 == quest::CQuestManager::instance().GetEventFlag("enable_shop_"
                                                           "search")) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "PRIVATE_SHOP_SEARCH_SYSTEM_DISABLED");
        return;
    }

    uint32_t ItemVnum = 0;



#ifndef DISABLE_PRIVATE_SHOP_SEARCH_NEED_ITEM
    if (!ch->FindSpecifyItem(60004) && !ch->FindSpecifyItem(60005)) {
        ch->SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "PRIVATE_SHOP_SEARCH_NEED_ITEM");
        return;
    }
#endif

    if(p.itemSearch) {
        ch->ShopSearch_ItemSearch(p.itemSearch.value().vnum, p.itemSearch.value().extra);
    } else if(p.openCategory) {
        ch->ShopSearch_OpenCategory(p.openCategory.value().itemType, p.openCategory.value().itemSubType);
    } else if(p.openFilter) {
        ch->ShopSearch_OpenFilter(p.openFilter.value());
    } else if(p.setPage) {
        ch->ShopSearch_SetPage(p.setPage.value().page);
    } else if(p.setFilters) {
        //ch->ShopSearch_SetFilters(p.setFilters.value().vnum, p.itemSearch.value().extra);
    } else if(p.buyItem) {
        ch->ShopSearch_BuyItem(p.buyItem.value());
    }

}
//
//void CInputMain::ShopSearchBuy(CHARACTER* ch, const TPacketCGShopSearchBuy& p)
//{
//    if (!ch)
//        return;
//
//    if (0 == quest::CQuestManager::instance().GetEventFlag("enable_shop_"
//                                                           "search")) {
//        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
//                           "PRIVATE_SHOP_SEARCH_SYSTEM_DISABLED");
//        return;
//    }
//
//#ifndef DISABLE_PRIVATE_SHOP_SEARCH_NEED_ITEM
//    if (!ch->FindSpecifyItem(60005)) {
//        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "PRIVATE_SHOP_SEARCH_NEED_ITEM");
//        return;
//    }
//#endif
//
//    auto tch = g_pCharManager->FindPC(p.seller);
//    if (tch) {
//        auto offlineShop =
//            COfflineShopManager::instance().FindOfflineShop(tch->GetPlayerID());
//        if (offlineShop) {
//            auto oldViewingShop = ch->GetViewingOfflineShop();
//            ch->SetViewingOfflineShop(offlineShop);
//            offlineShop->BuyItem(ch, p.shopItemPos);
//            ch->ChatPacket(CHAT_TYPE_COMMAND, "ShopSearchBuy");
//            ch->SetViewingOfflineShop(oldViewingShop);
//        } else {
//            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
//                               "The offline shop you wanted to buy from is "
//                               "currently not available.");
//        }
//    } else {
//        const auto op =
//            DESC_MANAGER::instance().GetOnlinePlayers().Get(p.seller);
//        if (op) {
//            auto offlineShop =
//                COfflineShopManager::instance().FindOfflineShop(op->pid);
//            if (offlineShop) {
//                auto oldViewingShop = ch->GetViewingOfflineShop();
//                ch->SetViewingOfflineShop(offlineShop);
//                offlineShop->BuyItem(ch, p.shopItemPos);
//                ch->ChatPacket(CHAT_TYPE_COMMAND, "ShopSearchBuy");
//                ch->SetViewingOfflineShop(oldViewingShop);
//            } else {
//                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
//                                   "The offline shop you wanted to buy from is "
//                                   "currently not available.");
//            }
//        } else {
//            auto offlineShop = COfflineShopManager::instance().FindOfflineShop(
//                p.seller.c_str());
//            if (offlineShop) {
//                auto oldViewingShop = ch->GetViewingOfflineShop();
//                ch->SetViewingOfflineShop(offlineShop);
//                offlineShop->BuyItem(ch, p.shopItemPos);
//                ch->ChatPacket(CHAT_TYPE_COMMAND, "ShopSearchBuy");
//                ch->SetViewingOfflineShop(oldViewingShop);
//            } else {
//                SPDLOG_ERROR("Could not offline shop to buy from player {}",
//                             p.seller);
//                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
//                                   "The offline shop from player %s you wanted "
//                                   "to buy from is currently not available.",
//                                   p.seller);
//            }
//        }
//    }
//}

#endif

void CInputMain::ItemMove(CHARACTER* ch, const TPacketCGItemMove& p)
{
    if (ch)
        ch->MoveItem(p.Cell, p.CellTo, p.count);
}

void CInputMain::ItemSplit(CHARACTER* ch, const TPacketCGItemSplit& p)
{

    if (ch)
        ch->SplitItem(p.Cell, p.count);
}

void CInputMain::ItemPickup(CHARACTER* ch, const TPacketCGItemPickup& p)
{
    if (!ch)
        return;

    // if (!CheckSpeedHack(ch, get_dword_time(), p.time))
    //    return;

    if (ch) {
        ch->PickupItem(p.vid, p.time);
    }
}

void CInputMain::QuickslotAdd(CHARACTER* ch, const TPacketCGQuickslotAdd& p)
{
    ch->SetQuickslot(p.pos, p.slot);
}

void CInputMain::QuickslotDelete(CHARACTER* ch, const TPacketCGQuickslotDel& p)
{
    ch->DelQuickslot(p.pos);
}

void CInputMain::QuickslotSwap(CHARACTER* ch, const TPacketCGQuickslotSwap& p)
{
    ch->SwapQuickslot(p.pos, p.change_pos);
}

bool CInputMain::Messenger(CHARACTER* ch, const TPacketCGMessenger& p)
{

    switch (p.subheader) {
        case MESSENGER_SUBHEADER_CG_ADD_BY_VID: {
            if (!p.vid)
                return true;

            CHARACTER* ch_companion = g_pCharManager->Find(p.vid.value());

            if (!ch_companion)
                return true;

            if (!ch_companion->IsPC())
                return true;

            if (ch->IsObserverMode() && !ch->IsGM())
                return true;

            if (ch_companion->IsBlockMode(BLOCK_MESSENGER_INVITE, ch)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "상대방이 메신져 추가 거부 상태입니다.");
                return true;
            }

            DESC* d = ch_companion->GetDesc();

            if (!d)
                return true;

            if (ch->GetGMLevel() == GM_PLAYER &&
                ch_companion->GetGMLevel() != GM_PLAYER) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<메신져> 운영자는 메신져에 추가할 수 "
                                   "없습니다.");
                return true;
            }

            if (ch->GetDesc() == d) // 자신은 추가할 수 없다.
                return true;

            if (!ch->IsNextFriendshipRequestPulse())
                return true;

            MessengerManager::instance().RequestToAdd(ch, ch_companion);
            // MessengerManager::instance().AddToList(ch->GetName(),
            // ch_companion->GetName());
        }
            return true;

        case MESSENGER_SUBHEADER_CG_ADD_BY_NAME: {
            if (!p.name)
                return true;

            if (ch->GetGMLevel() == GM_PLAYER &&
                GM::get_level(p.name.value().c_str(), nullptr, 0) !=
                    GM_PLAYER) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<메신져> 운영자는 메신져에 추가할 수 "
                                   "없습니다.");
                return true;
            }

            CHARACTER* tch = g_pCharManager->FindPC(p.name.value());

            if (!tch) {
                if (const auto op =
                        DESC_MANAGER::instance().GetOnlinePlayers().Get(
                            p.name.value());
                    op) {
                    GmMessengerRequestAddPacket p2;
                    p2.account = ch->GetName();
                    p2.companion = p.name.value();
                    DESC_MANAGER::instance().GetMasterSocket()->Send(
                        kGmMessengerRequestAdd, p2);
                } else {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "%s 님은 접속되 있지 않습니다.",
                                       p.name.value().c_str());
                }
            } else {
                if (tch == ch) // 자신은 추가할 수 없다.
                    return true;

                if (tch->IsBlockMode(BLOCK_MESSENGER_INVITE, ch)) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "상대방이 메신져 추가 거부 상태입니다.");
                } else {
                    if (!ch->IsNextFriendshipRequestPulse())
                        return true;

                    // 메신저가 캐릭터단위가 되면서 변경
                    MessengerManager::instance().RequestToAdd(ch, tch);
                    // MessengerManager::instance().AddToList(ch->GetName(),
                    // tch->GetName());
                }
            }
        }
            return true;

        case MESSENGER_SUBHEADER_CG_REMOVE: {
            if (!p.name)
                return true;

            MessengerManager::instance().RemoveFromList(ch->GetName(),
                                                        p.name.value());
            MessengerManager::instance().RemoveFromList(p.name.value(),
                                                        ch->GetName());
        }
            return true;
        case MESSENGER_SUBHEADER_CG_SET_BLOCK: {
            if (!p.setBlock)
                return true;

            const auto& p2 = p.setBlock.value();

            if (GM::get_level(p2.name.c_str()) != GM_PLAYER) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "You cannot block a game master.");
                return true;
            }

            PacketGDMessengerSetBlock pd;
            pd.pid = ch->GetPlayerID();
            pd.mode = p2.mode;

            auto f = [pd](SQLMsg* msg) mutable {
                pd.other_pid = 0;
                MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);
                if (row)
                    str_to_number(pd.other_pid, row[0]);

                CHARACTER* ch = g_pCharManager->FindByPID(pd.pid);
                if (pd.other_pid) {
                    db_clientdesc->DBPacketHeader(
                        HEADER_GD_MESSENGER_SET_BLOCK, 0,
                        sizeof(PacketGDMessengerSetBlock));
                    db_clientdesc->Packet(&pd,
                                          sizeof(PacketGDMessengerSetBlock));
                    if (ch && ch->GetDesc()) {
                        TPacketGCMessenger pack;
                        pack.subheader = MESSENGER_SUBHEADER_GC_BLOCK_LIST;
                        std::vector<BlockedPC> v;

                        BlockedPC pc;
                        strlcpy(pc.name, row[1], sizeof(pc.name));
                        pc.mode = pd.mode;
                        v.emplace_back(pc);
                        pack.listBlocked = v;

                        ch->GetDesc()->Send(HEADER_GC_MESSENGER, pack);
                        ch->SetMessengerBlock(pd.other_pid, pc);
                    }
                } else if (ch) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "그런 사람이 없습니다.");
                }
            };

            char escapedName[CHARACTER_NAME_MAX_LEN * 4];
            DBManager::instance().EscapeString(escapedName, sizeof(escapedName),
                                               p2.name.data(),
                                               p2.name.length());
            DBManager::instance().FuncQuery(f,
                                            "SELECT id, name FROM player WHERE "
                                            "name='%s' AND deleted = 0",
                                            escapedName);

            return true;
        }

        default:
            SPDLOG_ERROR("CInputMain::Messenger : Unknown subheader {} : {}",
                         p.subheader, ch->GetName());
            break;
    }

    return true;
}

void CInputMain::BlockMode(CHARACTER* ch, const CgBlockModePacket& p)
{
    ch->SetBlockMode(p.blockMode);
}

void CInputMain::Shop(CHARACTER* ch, const TPacketCGShop& p)
{
    switch (p.subheader) {
        case SHOP_SUBHEADER_CG_END:
            SPDLOG_TRACE("INPUT: {0} SHOP: END", ch->GetName());
            if (ch->IsNextShopPulse()) {
                CShopManager::instance().StopShopping(ch);
                ch->UpdateShopPulse();
            }
        break;

        case SHOP_SUBHEADER_CG_BUY: {
            if (!p.buyAction)
                return;

            if (ch->IsNextShopPulse()) {
                CShopManager::instance().Buy(ch, p.buyAction.value().pos, p.buyAction.value().amount);
                ch->UpdateShopPulse();
            }
        }
        break;

        case SHOP_SUBHEADER_CG_SELL: {
            if (!p.sellAction)
                return;

            TItemPos pos = p.sellAction.value().pos;

            SPDLOG_TRACE("INPUT: {} SHOP: SELL", ch->GetName());

            if (ch->IsNextShopPulse()) {
                CShopManager::instance().Sell(ch, pos);
                ch->UpdateShopPulse();
            }

        }
        break;

        case SHOP_SUBHEADER_CG_SELL2: {
            if (!p.sellAction)
                return;

            TItemPos pos = p.sellAction.value().pos;
            const auto count = p.sellAction.value().amount;

            SPDLOG_TRACE("INPUT: {} SHOP: SELL2", ch->GetName());

            if (ch->IsNextShopPulse()) {
                CShopManager::instance().Sell(ch, pos, count);
                ch->UpdateShopPulse();
            }

        }
        break;

        default:
            SPDLOG_ERROR("CInputMain::Shop : Unknown subheader {} : {}",
                         p.subheader, ch->GetName());
            break;
    }

}

#ifdef ENABLE_BATTLE_PASS
void CInputMain::BattlePass(CHARACTER* ch, const TPacketCGBattlePassAction& p)
{
    switch (p.bAction) {
        case 1:
            CBattlePass::instance().BattlePassRequestOpen(ch);
            break;

        case 2:
            CBattlePass::instance().BattlePassRequestReward(ch);
            break;

        case 3: {
            uint32_t dwPlayerId = ch->GetPlayerID();
            uint8_t bIsGlobal = 0;

            db_clientdesc->DBPacketHeader(HEADER_GD_BATTLE_PASS_RANKING,
                                          ch->GetDesc()->GetHandle(),
                                          sizeof(uint32_t) + sizeof(uint8_t));
            db_clientdesc->Packet(&dwPlayerId, sizeof(uint32_t));
            db_clientdesc->Packet(&bIsGlobal, sizeof(uint8_t));
        } break;

        default:
            break;
    }
}
#endif

bool CInputMain::OnClick(CHARACTER* ch, const TPacketCGOnClick& p)
{
    CHARACTER* victim;

    if ((victim = g_pCharManager->Find(p.vid)))
        victim->OnClick(ch);
    else if (gConfig.testServer) {
        SPDLOG_ERROR("CInputMain::OnClick {}.Click.NOT_EXIST_VID[{}]",
                     ch->GetName(), p.vid);
    }
    return true;
}

bool CInputMain::Exchange(CHARACTER* ch, const TPacketCGExchange& p)
{
    CHARACTER* to_ch = nullptr;

    if (!ch->CanHandleItem())
        return true;

    int iPulse = thecore_pulse();

    if ((to_ch = g_pCharManager->Find(p.arg1))) {
        if (iPulse - to_ch->GetSafeboxLoadTime() <
            THECORE_SECS_TO_PASSES(gConfig.portalLimitTime)) {
            SendI18nChatPacket(to_ch, CHAT_TYPE_INFO,
                               "거래 후 %d초 이내에 창고를 열수 없습니다.",
                               gConfig.portalLimitTime);
            return true;
        }

        if (true == to_ch->IsDead()) {
            return true;
        }
    }

    SPDLOG_INFO("CInputMain()::Exchange()  SubHeader {0} ", p.sub_header);

    if (iPulse - ch->GetSafeboxLoadTime() <
        THECORE_SECS_TO_PASSES(gConfig.portalLimitTime)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "거래 후 %d초 이내에 창고를 열수 없습니다.",
                           gConfig.portalLimitTime);
        return true;
    }

    switch (p.sub_header) {
        case EXCHANGE_SUBHEADER_CG_START: // arg1 == vid of target character
            if (!ch->GetExchange()) {
                if ((to_ch = g_pCharManager->Find(p.arg1))) {
                    // MONARCH_LIMIT
                    /*
                    if (to_ch->IsMonarch() || ch->IsMonarch())
                    {
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "군주와는 거래를
                    할수가 없습니다", gConfig.portalLimitTime); return;
                    }
                    //END_MONARCH_LIMIT
                    */
                    if (iPulse - ch->GetSafeboxLoadTime() <
                        THECORE_SECS_TO_PASSES(gConfig.portalLimitTime)) {
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "창고를 연후 %d초 이내에는 거래를 "
                                           "할수 없습니다.",
                                           gConfig.portalLimitTime);

                        if (gConfig.testServer)
                            ch->ChatPacket(CHAT_TYPE_INFO,
                                           "[TestOnly][Safebox]Pulse %d "
                                           "LoadTime %d PASS %d",
                                           iPulse, ch->GetSafeboxLoadTime(),
                                           THECORE_SECS_TO_PASSES(
                                               gConfig.portalLimitTime));
                        return true;
                    }

                    if (iPulse - to_ch->GetSafeboxLoadTime() <
                        THECORE_SECS_TO_PASSES(gConfig.portalLimitTime)) {
                        SendI18nChatPacket(to_ch, CHAT_TYPE_INFO,
                                           "창고를 연후 %d초 이내에는 거래를 "
                                           "할수 없습니다.",
                                           gConfig.portalLimitTime);

                        if (gConfig.testServer)
                            to_ch->ChatPacket(CHAT_TYPE_INFO,
                                              "[TestOnly][Safebox]Pulse %d "
                                              "LoadTime %d PASS %d",
                                              iPulse,
                                              to_ch->GetSafeboxLoadTime(),
                                              THECORE_SECS_TO_PASSES(
                                                  gConfig.portalLimitTime));
                        return true;
                    }

                    if (ch->GetGold() >= gConfig.maxGold) {
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "액수가 20억 냥을 초과하여 거래를 "
                                           "할수가 없습니다..");

                        SPDLOG_ERROR("[OVERFLOG_GOLD] START ({0} >= {3}) id "
                                     "{1} name {2} ",
                                     ch->GetGold(), ch->GetPlayerID(),
                                     ch->GetName(), gConfig.maxGold);
                        return true;
                    }

                    if (to_ch->IsPC()) {
                        if (quest::CQuestManager::instance().GiveItemToPC(
                                ch->GetPlayerID(), to_ch)) {
                            SPDLOG_INFO("Exchange canceled by quest {0} {1}",
                                        ch->GetName(), to_ch->GetName());
                            return true;
                        }
                    }

                    if (ch->GetMyShop() || ch->IsOpenSafebox() ||
                        ch->GetShopOwner() || ch->IsCubeOpen()) {
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                           "다른 거래중일경우 개인상점을 "
                                           "열수가 없습니다.");
                        return true;
                    }

                    ch->ExchangeStart(to_ch);
                }
            }
            break;

        case EXCHANGE_SUBHEADER_CG_ITEM_ADD: // arg1 == position of item, arg2
                                             // == position in exchange window
            if (ch->GetExchange()) {
                if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
                    ch->GetExchange()->AddItem(p.Pos, p.arg2);
            }
            break;

        case EXCHANGE_SUBHEADER_CG_ITEM_DEL: // arg1 == position of item
            if (ch->GetExchange()) {
                if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
                    ch->GetExchange()->RemoveItem(p.arg1);
            }
            break;

        case EXCHANGE_SUBHEADER_CG_ELK_SET: // arg1 == amount of gold
            if (ch->GetExchange()) {
                const uint64_t nTotalGold =
                    static_cast<uint64_t>(ch->GetExchange()
                                              ->GetCompany()
                                              ->GetOwner()
                                              ->GetGold()) +
                    static_cast<uint64_t>(p.arg1);
                if (gConfig.maxGold <= nTotalGold) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "상대방의 총금액이 20억 냥을 초과하여 "
                                       "거래를 할수가 없습니다..");

                    SPDLOG_ERROR(
                        "[OVERFLOW_GOLD] ELK_ADD ({0} >= {3}) id {1} name {2} ",
                        ch->GetExchange()->GetCompany()->GetOwner()->GetGold(),
                        ch->GetExchange()
                            ->GetCompany()
                            ->GetOwner()
                            ->GetPlayerID(),
                        ch->GetExchange()->GetCompany()->GetOwner()->GetName(),
                        gConfig.maxGold);

                    return true;
                }

                if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
                    ch->GetExchange()->AddGold(p.arg1);
            }
            break;

        case EXCHANGE_SUBHEADER_CG_ACCEPT: // arg1 == not used
            if (ch->GetExchange()) {
                SPDLOG_INFO("CInputMain()::Exchange() ==> ACCEPT ");
                ch->GetExchange()->Accept(true);
            }

            break;

        case EXCHANGE_SUBHEADER_CG_CANCEL: // arg1 == not used
            if (ch->GetExchange())
                ch->GetExchange()->Cancel();
            break;
    }

    return true;
}

static const int ComboSequenceBySkillLevel[3][8] = {
    // 0   1   2   3   4   5   6   7
    {14, 15, 16, 17, 0, 0, 0, 0},
    {14, 15, 16, 18, 20, 0, 0, 0},
    {14, 15, 16, 18, 19, 17, 0, 0},
};

bool CheckAttackSpeedHack(CHARACTER* ch, uint32_t clientTime,
                          uint8_t motionIndex)
{
    // This attack motion arrived before our last attack motion finished
    // Sounds like a hack
    if (clientTime < ch->GetLastComboTime() + ch->GetValidComboInterval()) {
        SPDLOG_TRACE("attack-speed hack: now {0} last {1} valid {2}",
                     clientTime, ch->GetLastComboTime(),
                     ch->GetValidComboInterval());
        // Temporarily disabled;
        // LogManager::instance().HackLog("ATTACK_SPEED_HACK", ch);
        // ch->GetDesc()->DelayedDisconnect(3);
        return false;
    }

    const auto key = MakeMotionKey(ch->GetMotionMode(), motionIndex);
    const auto duration = GetAttackMotionDuration(ch, key);

    ch->SetLastComboTime(clientTime);
    ch->SetValidComboInterval(duration);
    return true;
}

void CInputMain::Move(CHARACTER* ch, const TPacketCGMove& p)
{

    if (gConfig.IsHackCheckedMap(ch->GetMapIndex())) {

        if (!CheckSpeedHack(ch, get_dword_time(), p.dwTime))
            return;

        if (!IsValidPosition(ch, p.lX, p.lY)) {
            ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
            return;
        }

    }

    if (p.bFunc == FUNC_MOVE) {
        if (ch->GetLimitPoint(POINT_MOV_SPEED) == 0)
            return;
       
        ch->SetRotation(p.bRot * 5); // 중복 코드
        ch->ResetStopTime();         // ""

        ch->Goto(p.lX, p.lY);
    } else {
        if (p.bFunc == FUNC_ATTACK || p.bFunc == FUNC_COMBO) {
            ch->GetHitLog().Clear();
            ch->SetLastComboTime(p.dwTime);
            ch->OnMove(true);

        } else if (p.bFunc & FUNC_SKILL) {

            ch->OnMove();
        }

        ch->Move(p.lX, p.lY);
        ch->SetRotation(p.bRot * 5); // 중복 코드
        ch->ResetStopTime();         // ""

        ch->Stop();
        ch->StopStaminaConsume();
    }

    TPacketGCMove pack;
    pack.bFunc = p.bFunc;
    pack.bArg = p.bArg;
    pack.bRot = p.bRot;
    pack.dwVID = ch->GetVID();
    pack.lX = p.lX;
    pack.lY = p.lY;
    pack.dwTime = p.dwTime;
    pack.dwDuration = ch->CalculateMoveDuration();
    pack.color = p.color;
    pack.isMovingSkill = p.isMovingSkill;
    pack.loopCount = p.loopCount;

    if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()) ||
        ch->GetMapIndex() == 112 ||
        gConfig.IsBlockedSkillColorMap(ch->GetMapIndex()))
        pack.color = 0;

    PacketAround(ch->GetViewMap(), ch, HEADER_GC_CHARACTER_MOVE, pack, ch);
}

void CInputMain::Attack(CHARACTER* ch, const TPacketCGAttack& p)
{
    if (!ch)
        return;

    struct type_identifier {
        uint8_t header;
        uint8_t type;
    };

    if (nullptr == ch->GetDesc())
        return;

    if (!CheckSpeedHack(ch, get_dword_time(), p.attackTime)) {
        if (gConfig.testServer)
            SendChatPacket(ch, CHAT_TYPE_INFO,
                           "Attack canceled: Speedhack detected");
        return;
    }

    if (p.bType > 0 && !ch->CanUseSkill(p.bType)) {
        SPDLOG_ERROR("{0}: cannot attack with skill {1}", ch->GetName(),
                     p.bType);
        if (gConfig.testServer)
            SendChatPacket(ch, CHAT_TYPE_INFO,
                           "Attack canceled: Cannot use skill");
        return;
    }

    auto* victim = g_pCharManager->Find(p.dwVictimVID);
    if (!victim || ch == victim)
        return;

    switch (victim->GetCharType()) {
        case CHAR_TYPE_NPC:
        case CHAR_TYPE_WARP:
        case CHAR_TYPE_GOTO:
            return;
    }

    if (p.bType > 0 && !ch->CheckSkillHitCount(p.bType, victim->GetVID())) {
        if (gConfig.testServer)
            SendChatPacket(ch, CHAT_TYPE_INFO,
                           "Attack canceled: Hit count exceeded");
        return;
    }

    if (p.bType == 0 && gConfig.IsHackCheckedMap(ch->GetMapIndex())) {
        auto id = MakeMotionId(p.motionKey);
        if (id.mode != ch->GetMotionMode()) {
            SPDLOG_DEBUG("Attack packet has wrong motion-mode: {0} != {1}",
                         id.mode, ch->GetMotionMode());
            if (gConfig.testServer)
                SendChatPacket(ch, CHAT_TYPE_INFO,
                               "Attack canceled: Wrong attack motion");
            return;
        }

        //if (p.time > ch->GetLastComboTime() + ch->GetComboDuration()) {
        //    SPDLOG_DEBUG("{0}: normal attack too late: time {1} last {2} dur "
        //                 "{3}",
        //                 ch->GetName(), p.time, ch->GetLastComboTime(),
        //                 ch->GetComboDuration());
        //    if (gConfig.testServer)
        //        SendChatPacket(ch, CHAT_TYPE_INFO,
        //                       "Attack canceled: Normal attack too late");
        //    return;
        //}

        if (!CheckAttackHitCount(ch, victim)) {
            SPDLOG_DEBUG("{0}: normal attack hit count exceeded",
                         ch->GetName());
            if (gConfig.testServer)
                SendChatPacket(ch, CHAT_TYPE_INFO,
                               "Attack canceled: normal attack hit count "
                               "exceeded");
            return;
        }
    }

    // Check attack position
    if (gConfig.IsHackCheckedMap(ch->GetMapIndex()) &&
        (!ValidateMovement(ch, p.attackTime, p.x, p.y) ||
         !IsValidPosition(ch, p.x, p.y)))
        return;

    /*	if (!ValidateMovement(ch, p.time, p.x, p.y) ||
            !IsValidPosition(ch, p.x, p.y))
            return;
            */

    ch->Attack(victim, p.motionKey /*motKey*/, p.attackTime /*time*/, p.bType);
}

void CInputMain::Shoot(CHARACTER* ch, const TPacketCGShoot& p)
{
    if (!ch)
        return;

    if (p.type > 0 && !ch->CanUseSkill(p.type))
        return;

    ch->Shoot(p.type, p.motionKey);
}

void CInputMain::FlyTarget(CHARACTER* ch, const TPacketCGFlyTargeting& p,
                           PacketId header)
{
    ch->FlyTarget(p.dwTargetVID, p.x, p.y, header);
}

void CInputMain::UseSkill(CHARACTER* ch, const TPacketCGUseSkill& p)
{
    ch->UseSkill(p.dwVnum, g_pCharManager->Find(p.dwVID));
}

void CInputMain::ChooseSkillGroup(CHARACTER* ch,
                                  const TPacketCGChooseSkillGroup& p)
{

    if (ch->GetSkillGroup() != 0) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "You cannot change your teaching this way after "
                           "choosing one.");
        return;
    }

    if (ch->GetLevel() < 5) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "Your level is too low to select a teaching.");
        return;
    }

    if (p.skillGroup != 1 && p.skillGroup != 2)
        return;

    ch->ClearSkill();
    ch->SetSkillGroup(p.skillGroup);

    const uint32_t* skill_List = GetUsableSkillList(ch->GetJob(), p.skillGroup);
    static const uint8_t skill_level{
        59}; // 59 for legendary master 40 for perfect

    if (skill_List) {
        for (size_t i = 0; i < 6; ++i) {
            ch->SetSkillLevel(skill_List[i], skill_level);
        }
    }

    ch->SkillLevelPacket();
}

void CInputMain::ChangeSkillColor(CHARACTER* ch,
                                  const TPacketCGChangeSkillColorPacket& p)
{
    if (ch->CountSpecifyItem(50306) < 1) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           LC_TEXT("You cannot do it without a "
                                   "Skill Color "
                                   "Ticket in your inventory."));
        return;
    }
    if (ch->GetSkillLevel(p.vnum) != SKILL_MAX_LEVEL) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "The skill needs to be on the maximum level to "
                           "change its color");
        return;
    }

    ch->SetSkillColor(p.vnum, p.color);
    ch->SkillLevelPacket();
    ch->RemoveSpecifyItem(50306, 1);
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, LC_TEXT("You have successfully changed "
                                                   "the color of "
                                           "your skill."));
}

void CInputMain::ScriptButton(CHARACTER* ch, const TPacketCGScriptButton& p)
{
    SPDLOG_INFO("QUEST ScriptButton pid {} idx {}", ch->GetPlayerID(), p.idx);

    quest::PC* pc =
        quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
    if (pc && pc->IsConfirmWait()) {
        quest::CQuestManager::instance().Confirm(ch->GetPlayerID(),
                                                 quest::CONFIRM_TIMEOUT);
    } else if (p.idx & 0x80000000) {
        //퀘스트 창에서 클릭시(__SelectQuest) 여기로
        quest::CQuestManager::Instance().QuestInfo(ch->GetPlayerID(),
                                                   p.idx & 0x7fffffff);
    } else {
        quest::CQuestManager::Instance().QuestButton(ch->GetPlayerID(), p.idx);
    }
}

void CInputMain::ScriptAnswer(CHARACTER* ch, const TPacketCGScriptAnswer& p)
{
    SPDLOG_INFO("QUEST ScriptAnswer pid {} answer {}", ch->GetPlayerID(),
                p.answer);

    if (p.answer > 250) // 다음 버튼에 대한 응답으로 온 패킷인 경우
    {
        quest::CQuestManager::Instance().Resume(ch->GetPlayerID(), p.qIndex);
    } else // 선택 버튼을 골라서 온 패킷인 경우
    {
        quest::CQuestManager::Instance().Select(ch->GetPlayerID(), p.answer,
                                                p.qIndex);
    }
}

// SCRIPT_SELECT_ITEM
void CInputMain::ScriptSelectItem(CHARACTER* ch,
                                  const TPacketCGScriptSelectItem& p)
{
    SPDLOG_INFO("QUEST ScriptSelectItem pid %d answer %d", ch->GetPlayerID(),
                p.selection);
    quest::CQuestManager::Instance().SelectItem(ch->GetPlayerID(), p.selection);
}

// END_OF_SCRIPT_SELECT_ITEM

void CInputMain::QuestInputString(CHARACTER* ch,
                                  const TPacketCGQuestInputString& p)
{
    SPDLOG_INFO("QUEST InputString pid {} msg {}", ch->GetPlayerID(), p.msg);

    quest::CQuestManager::Instance().Input(ch->GetPlayerID(), p.msg.c_str(),
                                           p.qIndex);
}

void CInputMain::QuestReceive(CHARACTER* ch, const TPacketCGQuestRcv& p)
{
    quest::CQuestManager::Instance().Receive(ch->GetPlayerID(), p.msg.c_str(),
                                             p.questID);
}

void CInputMain::QuestConfirm(CHARACTER* ch, const TPacketCGQuestConfirm& p)
{
    CHARACTER* ch_wait = g_pCharManager->FindByPID(p.requestPID);

    SPDLOG_INFO("QuestConfirm from {} pid {} name {} answer {}", ch->GetName(),
                p.requestPID, (ch_wait) ? ch_wait->GetName() : "", p.answer);
    if (ch_wait) {
        quest::CQuestManager::Instance().Confirm(
            ch_wait->GetPlayerID(), (quest::EQuestConfirmType)p.answer,
            ch->GetPlayerID());
    }
}

void CInputMain::Target(CHARACTER* ch, const TPacketCGTarget& p)
{
    building::CObject* pkObj =
        building::CManager::instance().FindObjectByVID(p.dwVID);

    if (pkObj) {
        TPacketGCTarget pckTarget;
        pckTarget.dwVID = p.dwVID;
        ch->GetDesc()->Send(HEADER_GC_TARGET, pckTarget);
    } else
        ch->SetTarget(g_pCharManager->Find(p.dwVID));
}

void CInputMain::LoadTargetInfo(CHARACTER* pkCharacter,
                                const TPacketCGTargetLoad& p)
{
    if (!pkCharacter || !pkCharacter->GetDesc()) {
        return;
    }

    CHARACTER* pkMonster = g_pCharManager->Find(p.dwVID);
    if (!pkMonster) {
        return;
    }

    TPacketGCTargetInfo kPacket;
    kPacket.dwVNum = pkMonster->GetRaceNum();

    const auto& pvecDropInfo =
        ITEM_MANAGER::instance().FindDropInfo(pkMonster->GetRaceNum());
    if (pvecDropInfo) {
        kPacket.drops = pvecDropInfo.value();
    }

    pkCharacter->GetDesc()->Send(HEADER_GC_TARGET_INFO, kPacket);
}

void CInputMain::SafeboxCheckin(CHARACTER* ch, const TPacketCGSafeboxCheckin& p)
{
    if (quest::CQuestManager::instance()
            .GetPCForce(ch->GetPlayerID())
            ->IsRunning())
        return;

    if (!ch->CanHandleItem())
        return;

    CSafebox* pkSafebox = ch->GetSafebox();
    CItem* pkItem = ch->GetItem(p.ItemPos);

    if (!pkSafebox || !pkItem)
        return;

    // TODO(tim): Just use CanRemove?
    if (pkItem->IsEquipped() && !CanUnequipItem(ch, pkItem)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<창고> 창고로 옮길 수 없는 아이템 입니다.");
        return;
    }

    if (!CanModifyItem(pkItem)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<창고> 이 아이템은 넣을 수 없습니다.");
        return;
    }

    uint16_t pos = p.bSafePos;

    if (!pkSafebox->IsEmpty(pos, pkItem->GetSize())) {
        const auto newPos = pkSafebox->FindEmpty(pkItem->GetSize());
        if (-1 == newPos) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<창고> 옮길 수 없는 위치입니다.");
            return;
        }

        pos = newPos;
    }

    if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_SAFEBOX)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<창고> 이 아이템은 넣을 수 없습니다.");
        return;
    }

    pkItem->RemoveFromCharacter();
    ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, p.ItemPos.cell, 255);
    pkSafebox->Add(pos, pkItem);

    char szHint[128];
    std::snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(),
                  pkItem->GetCount());
    LogManager::instance().ItemLog(ch, pkItem, "SAFEBOX PUT", szHint);
}

void CInputMain::SafeboxCheckout(CHARACTER* ch,
                                 const TPacketCGSafeboxCheckout& p, bool bMall)
{
    if (!ch->CanHandleItem())
        return;

    CSafebox* pkSafebox;

    if (bMall)
        pkSafebox = ch->GetMall();
    else
        pkSafebox = ch->GetSafebox();

    if (!pkSafebox)
        return;

    auto pkItem = pkSafebox->Get(p.bSafePos);
    if (!pkItem)
        return;

    if (p.ItemPos.IsBeltInventoryPosition()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "이 아이템은 벨트 인벤토리로 옮길 수 없습니다.");
        return;
    }

    bool useAutoGive = false;
    auto pos = p.ItemPos;

    if (pos.IsValidItemPosition()) {
        if (pkItem->IsDragonSoul()) {
            if (!ch->IsEmptyItemGridDS(pos, pkItem->GetSize()))
                return;
        } else {
            if (!ch->IsEmptyItemGrid(pos, pkItem->GetSize()))
                return;
        }
    } else {
        if (pkItem->IsDragonSoul()) {
            const auto cell = ch->GetEmptyDragonSoulInventory(pkItem);
            if (cell == -1) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "There isn't enough space "
                                   "in the inventory.");
                return;
            }

            pos.window_type = DRAGON_SOUL_INVENTORY;
            pos.cell = cell;
            useAutoGive = false;
        } else {
            const auto cell = ch->GetEmptyInventory(pkItem);
            if (cell == -1) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "There isn't enough space "
                                   "in the inventory.");
                return;
            }

            pos.window_type = INVENTORY;
            pos.cell = cell;
            useAutoGive = false;
        }
    }

    // 아이템 몰에서 인벤으로 옮기는 부분에서 용혼석 특수 처리
    // (몰에서 만드는 아이템은 item_proto에 정의된대로 속성이 붙기 때문에,
    //  용혼석의 경우, 이 처리를 하지 않으면 속성이 하나도 붙지 않게 된다.)
    if (pkItem->IsDragonSoul()) {
        if (bMall) {
            DSManager::instance().DragonSoulItemInitialize(pkItem);
        }

        if (DRAGON_SOUL_INVENTORY != p.ItemPos.window_type) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<창고> 옮길 수 없는 위치입니다.");
            return;
        }

        TItemPos DestPos = pos;
        if (!DSManager::instance().IsValidCellForThisItem(pkItem, DestPos)) {
            int iCell = ch->GetEmptyDragonSoulInventory(pkItem);
            if (iCell < 0) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<창고> 옮길 수 없는 위치입니다.");
                return;
            }
            DestPos = TItemPos(DRAGON_SOUL_INVENTORY, iCell);
        }

        pkSafebox->Remove(p.bSafePos);
        pkItem->AddToCharacter(ch, DestPos);

        events::Item::OnLoad(ch, pkItem);

        ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
    } else {
        if (DRAGON_SOUL_INVENTORY == p.ItemPos.window_type) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<창고> 옮길 수 없는 위치입니다.");
            return;
        }

        pkSafebox->Remove(p.bSafePos);
        pkItem->AddToCharacter(ch, pos);

        events::Item::OnLoad(ch, pkItem);

        ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
    }

    char szHint[128];
    std::snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(),
                  pkItem->GetCount());
    if (bMall)
        LogManager::instance().ItemLog(ch, pkItem, "MALL GET", szHint);
    else
        LogManager::instance().ItemLog(ch, pkItem, "SAFEBOX GET", szHint);
}

void CInputMain::SafeboxItemMove(CHARACTER* ch, const TPacketCGItemMove& p)
{
    if (!ch->CanHandleItem())
        return;

    if (!ch->GetSafebox())
        return;

    ch->GetSafebox()->MoveItem(p.Cell.cell, p.CellTo.cell, p.count);
}

// PARTY_JOIN_BUG_FIX
void CInputMain::PartyInvite(CHARACTER* ch, const TPacketCGPartyInvite& p)
{
    if (ch->GetArena()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "대련장에서 사용하실 수 없습니다.");
        return;
    }

    CHARACTER* pInvitee = g_pCharManager->Find(p.vid);

    if (!pInvitee || !ch->GetDesc() || !pInvitee->GetDesc()) {
        SPDLOG_ERROR("PARTY Cannot find invited character");
        return;
    }

#ifdef ENABLE_MESSENGER_BLOCK
    if (MessengerManager::instance().IsBlocked_Target(ch->GetName(),
                                                      pInvitee->GetName())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You blocked %s",
                           pInvitee->GetName());
        return;
    }
    if (MessengerManager::instance().IsBlocked_Me(ch->GetName(),
                                                  pInvitee->GetName())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%s blocked you",
                           pInvitee->GetName());
        return;
    }
#endif

    ch->PartyInvite(pInvitee);
}

void CInputMain::PartyInviteAnswer(CHARACTER* ch,
                                   const TPacketCGPartyInviteAnswer& p)
{
    if (ch->GetArena()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "대련장에서 사용하실 수 없습니다.");
        return;
    }

    CHARACTER* pInviter = g_pCharManager->Find(p.leader_vid);

    // pInviter 가 ch 에게 파티 요청을 했었다.

    if (!pInviter)
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 파티요청을 한 캐릭터를 찾을수 없습니다.");
    else if (!p.accept)
        pInviter->PartyInviteDeny(ch->GetPlayerID());
    else
        pInviter->PartyInviteAccept(ch);
}

// END_OF_PARTY_JOIN_BUG_FIX

void CInputMain::PartySetState(CHARACTER* ch, const TPacketCGPartySetState& p)
{
    if (!CPartyManager::instance().IsEnablePCParty()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 서버 문제로 파티 관련 처리를 할 수 "
                           "없습니다.");
        return;
    }

    if (!ch->GetParty())
        return;

    if (ch->GetParty()->GetLeaderPID() != ch->GetPlayerID()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 리더만 변경할 수 있습니다.");
        return;
    }

    if (!ch->GetParty()->IsMember(p.pid)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 상태를 변경하려는 사람이 파티원이 "
                           "아닙니다.");
        return;
    }

    uint32_t pid = p.pid;
    SPDLOG_INFO("PARTY SetRole pid %d to role %d state %s", pid, p.byRole,
                p.flag ? "on" : "off");

    switch (p.byRole) {
        case PARTY_ROLE_NORMAL:
            break;

        case PARTY_ROLE_ATTACKER:
        case PARTY_ROLE_TANKER:
        case PARTY_ROLE_BUFFER:
        case PARTY_ROLE_SKILL_MASTER:
        case PARTY_ROLE_HASTE:
        case PARTY_ROLE_DEFENDER:
            if (ch->GetParty()->SetRole(pid, p.byRole, p.flag != 0)) {
                TPacketPartyStateChange pack;
                pack.dwLeaderPID = ch->GetPlayerID();
                pack.dwPID = p.pid;
                pack.bRole = p.byRole;
                pack.bFlag = p.flag;
                db_clientdesc->DBPacket(HEADER_GD_PARTY_STATE_CHANGE, 0, &pack,
                                        sizeof(pack));
            }
            /* else
               SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<파티> 어태커 설정에
               실패하였습니다."); */
            break;

        default:
            SPDLOG_ERROR("wrong byRole in PartySetState Packet name %s state "
                         "%d",
                         ch->GetName(), p.byRole);
            break;
    }
}

void CInputMain::PartyRemove(CHARACTER* ch, const TPacketCGPartyRemove& p)
{
    if (ch->GetArena()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "대련장에서 사용하실 수 없습니다.");
        return;
    }

    if (!CPartyManager::instance().IsEnablePCParty()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 서버 문제로 파티 관련 처리를 할 수 "
                           "없습니다.");
        return;
    }

    if (ch->GetDungeon()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 던전 안에서는 파티에서 추방할 수 없습니다.");
        return;
    }

    if (!ch->GetParty())
        return;

    auto pParty = ch->GetParty();

    if (pParty->GetLeaderPID() == ch->GetPlayerID()) {
        if (ch->GetDungeon()) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<파티> 던젼내에서는 파티원을 추방할 수 "
                               "없습니다.");
        } else {
            // 적룡성에서 파티장이 던젼 밖에서 파티 해산 못하게 막자
            if (pParty->IsPartyInDungeon(351)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<파티>던전 안에 파티원이 있어 파티를 해산 "
                                   "할 수 없습니다.");
                return;
            }

            // leader can remove any member
            if (p.pid == ch->GetPlayerID() || pParty->GetMemberCount() == 2) {
                // party disband
                CPartyManager::instance().DeleteParty(pParty);
            } else {
                CHARACTER* B = g_pCharManager->FindByPID(p.pid);
                if (B) {
                    // pParty->SendPartyRemoveOneToAll(B);
                    SendI18nChatPacket(B, CHAT_TYPE_INFO,
                                       "<파티> 파티에서 추방당하셨습니다.");
                    // pParty->Unlink(B);
                    // CPartyManager::instance().SetPartyMember(B->GetPlayerID(),
                    // NULL);
                }
                pParty->Quit(p.pid);
            }
        }
    } else {
        // otherwise, only remove itself
        if (p.pid == ch->GetPlayerID()) {
            if (ch->GetDungeon()) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<파티> 던젼내에서는 파티를 나갈 수 "
                                   "없습니다.");
            } else {
                if (pParty->GetMemberCount() == 2) {
                    // party disband
                    CPartyManager::instance().DeleteParty(pParty);
                } else {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "<파티> 파티에서 나가셨습니다.");
                    // pParty->SendPartyRemoveOneToAll(ch);
                    pParty->Quit(ch->GetPlayerID());
                    // pParty->SendPartyRemoveAllToOne(ch);
                    // CPartyManager::instance().SetPartyMember(ch->GetPlayerID(),
                    // NULL);
                }
            }
        } else {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<파티> 다른 파티원을 탈퇴시킬 수 없습니다.");
        }
    }
}

void CInputMain::AnswerMakeGuild(CHARACTER* ch,
                                 const TPacketCGAnswerMakeGuild& p)
{
    if (get_global_time() - ch->GetQuestFlag("guild_manage.new_disband_time") <
        CGuildManager::instance().GetDisbandDelay()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 해산한 후 %d일 이내에는 길드를 만들 수 "
                           "없습니다.",
                           quest::CQuestManager::instance().GetEventFlag("guild"
                                                                         "_disb"
                                                                         "and_"
                                                                         "dela"
                                                                         "y"));
        return;
    }

    if (get_global_time() - ch->GetQuestFlag("guild_manage.new_withdraw_time") <
        CGuildManager::instance().GetWithdrawDelay()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 탈퇴한 후 %d일 이내에는 길드를 만들 수 "
                           "없습니다.",
                           quest::CQuestManager::instance().GetEventFlag("guild"
                                                                         "_with"
                                                                         "draw_"
                                                                         "dela"
                                                                         "y"));
        return;
    }

    if (ch->GetGuild())
        return;

    CGuildManager& gm = CGuildManager::instance();

    TGuildCreateParameter cp;
    memset(&cp, 0, sizeof(cp));

    cp.master = ch;
    storm::CopyStringSafe(cp.name, p.guild_name);

    if (cp.name[0] == 0 || !check_name(cp.name)) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "적합하지 않은 길드 이름 입니다.");
        return;
    }

    uint32_t dwGuildID = gm.CreateGuild(cp);

    if (dwGuildID) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> [%s] 길드가 생성되었습니다.", cp.name);

        char Log[128];

        std::snprintf(Log, sizeof(Log), "GUILD_NAME %s MASTER %s", cp.name,
                      ch->GetName().c_str());
        LogManager::instance().CharLog(ch, 0, "MAKE_GUILD", Log);
        ch-> WarpSet(ch->GetMapIndex(), ch->GetX(), ch->GetY());

        // ch->SendGuildName(dwGuildID);
    } else
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<길드> 길드 생성에 실패하였습니다.");
}

void CInputMain::PartyUseSkill(CHARACTER* ch, const TPacketCGPartyUseSkill& p)
{
    if (!ch->GetParty())
        return;

    if (ch->GetPlayerID() != ch->GetParty()->GetLeaderPID()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "<파티> 파티 기술은 파티장만 사용할 수 있습니다.");
        return;
    }

    switch (p.bySkillIndex) {
        case PARTY_SKILL_WARP: {
            ch->GetParty()->SummonToLeader(p.vid);

        } break;
    }
}

void CInputMain::PartyParameter(CHARACTER* ch, const TPacketCGPartyParameter& p)
{
    if (ch->GetParty())
        ch->GetParty()->SetParameter(p.bDistributeMode);
}

int CInputMain::Guild(CHARACTER* ch, const TPacketCGGuild& p)
{
    CGuild* pGuild = ch->GetGuild();

    if (nullptr == pGuild) {
        if (p.subheader != GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "<길드> 길드에 속해있지 않습니다.");
            return true;
        }
    }

    switch (p.subheader) {
        case GUILD_SUBHEADER_CG_DEPOSIT_MONEY: {
            return true;
        }

        case GUILD_SUBHEADER_CG_WITHDRAW_MONEY: {
            return true;
        }

        case GUILD_SUBHEADER_CG_ADD_MEMBER: {
            if (!p.vid)
                return true;
            const uint32_t vid = p.vid.value();

            CHARACTER* newmember = g_pCharManager->Find(vid);

            if (!newmember) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 그러한 사람을 찾을 수 없습니다.");
                return true;
            }

            if (!ch->IsPC() || !newmember->IsPC())
                return true;

#ifdef ENABLE_MESSENGER_BLOCK
            if (MessengerManager::instance().IsBlocked_Target(
                    ch->GetName(), newmember->GetName())) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You blocked %s",
                                   newmember->GetName());
                return true;
            }
            if (MessengerManager::instance().IsBlocked_Me(
                    ch->GetName(), newmember->GetName())) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%s blocked you",
                                   newmember->GetName());
                return true;
            }
#endif

            pGuild->Invite(ch, newmember);
        }
            return true;

        case GUILD_SUBHEADER_CG_REMOVE_MEMBER: {
            if (pGuild->UnderAnyWar() != 0) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 길드전 중에는 길드원을 탈퇴시킬 수 "
                                   "없습니다.");
                return true;
            }
            if (!p.pid)
                return true;

            const uint32_t pid = p.pid.value();
            const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

            if (nullptr == m)
                return true;

            CHARACTER* member = g_pCharManager->FindByPID(pid);

            if (member) {
                if (member->GetGuild() != pGuild) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "<길드> 상대방이 같은 길드가 아닙니다.");
                    return true;
                }

                if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER)) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "<길드> 길드원을 강제 탈퇴 시킬 권한이 "
                                       "없습니다.");
                    return true;
                }

                member->SetQuestFlag("guild_manage.new_withdraw_time",
                                     get_global_time());
                pGuild->RequestRemoveMember(member->GetPlayerID());
            } else {
                if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER)) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "<길드> 길드원을 강제 탈퇴 시킬 권한이 "
                                       "없습니다.");
                    return true;
                }

                if (pGuild->RequestRemoveMember(pid))
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "<길드> 길드원을 강제 탈퇴 시켰습니다.");
                else
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "<길드> 그러한 사람을 찾을 수 "
                                       "없습니다.");
            }
        }
            return true;

        case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME: {
            if (!p.changeGradeName)
                return true;

            const auto& changeGrade = p.changeGradeName.value();

            const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

            if (nullptr == m)
                return true;

            if (m->grade != GUILD_LEADER_GRADE) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 직위 이름을 변경할 권한이 "
                                   "없습니다.");
            } else if (changeGrade.index == GUILD_LEADER_GRADE) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 길드장의 직위 이름은 변경할 수 "
                                   "없습니다.");
            } else if (!check_name(changeGrade.name.c_str())) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 적합하지 않은 직위 이름 입니다.");
            } else {
                pGuild->ChangeGradeName(changeGrade.index,
                                        changeGrade.name.c_str());
            }
        }
            return true;

        case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY: {
            if (!p.changeGradeAuth)
                return true;

            const auto& changeAuth = p.changeGradeAuth.value();
            const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

            if (nullptr == m)
                return true;
#ifdef ENABLE_GUILD_STORAGE
            if (m->grade != GUILD_LEADER_GRADE &&
                !pGuild->HasGradeAuth(m->grade, GUILD_AUTH_MANAGE))
#else
            if (m->grade != GUILD_LEADER_GRADE)
#endif
            {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 직위 권한을 변경할 권한이 없습니다.",
                                   INT_MAX);
            } else if (changeAuth.index == GUILD_LEADER_GRADE) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 길드장의 권한은 변경할 수 "
                                   "없습니다.");
            } else {
                pGuild->ChangeGradeAuth(changeAuth.index, changeAuth.auth);
            }
        }
            return true;

        case GUILD_SUBHEADER_CG_OFFER: {
            if (!p.expOffer)
                return true;

            const auto& expOffer = p.expOffer.value();
            PointValue offer = expOffer;

            if (pGuild->GetLevel() >= GUILD_MAX_LEVEL) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 길드가 이미 최고 레벨입니다.");
                return true;
            }

            const auto* activeAntiExp =
                FindToggleItem(ch, true, TOGGLE_ANTI_EXP);
            if (activeAntiExp) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<Guild> You cannot offer exp while using "
                                   "an anti exp ring.",
                                   offer);
                return true;
            }

            if (offer < 0)
                offer = ch->GetExp();

            if (offer > ch->GetExp())
                offer = ch->GetExp();

            if (pGuild->OfferExp(ch, offer)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> %u의 경험치를 투자하였습니다.",
                                   offer);
            } else {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 경험치 투자에 실패하였습니다.");
            }

            return true;
        }

        case GUILD_SUBHEADER_CG_CHARGE_GSP: {
            if (!p.money)
                return true;

            const auto& offer = p.money.value();
            const Gold gold = offer * 100;

            if (offer < 0 || gold < offer || gold < 0 || ch->GetGold() < gold) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 돈이 부족합니다.");
                return true;
            }

            if (!pGuild->ChargeSP(ch, offer)) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 용신력 회복에 실패하였습니다.");
            }
            return true;
        }

        case GUILD_SUBHEADER_CG_POST_COMMENT: {
            if (!p.comment)
                return true;

            const auto& comment = p.comment.value();

            const size_t length = comment.length();

            if (comment.length() > GUILD_COMMENT_MAX_LEN) {
                // 잘못된 길이.. 끊어주자.
                SPDLOG_ERROR("POST_COMMENT: %s comment too long (length: %u)",
                             ch->GetName(), length);

                ch->GetDesc()->DelayedDisconnect(0, "POST_COMMENT_TOO_LONG");
                return true;
            }

            const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

            if (nullptr == m) {
                SPDLOG_ERROR("POST_COMMENT: %s no valid member found for that "
                             "id! "
                             "(id: %u)",
                             ch->GetPlayerID());
                return true;
            }

            if (length && !pGuild->HasGradeAuth(m->grade, GUILD_AUTH_NOTICE) &&
                comment[0] == '!') {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 공지글을 작성할 권한이 없습니다.");
            } else {
                pGuild->AddComment(ch, comment);
            }

            return true;
        }

        case GUILD_SUBHEADER_CG_DELETE_COMMENT: {
            if (!p.commentIndex)
                return true;

            const auto& comment_id = p.commentIndex.value();

            pGuild->DeleteComment(ch, comment_id);
        }
            return true;

        case GUILD_SUBHEADER_CG_REFRESH_COMMENT:
            pGuild->RefreshComment(ch);
            return true;

        case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE: {
            if (!p.changeMemberGrade)
                return true;

            const auto& cmg = p.changeMemberGrade.value();

            const uint32_t pid = cmg.pid;
            const uint8_t grade = cmg.grade;
            const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

            if (nullptr == m)
                return true;

            if (m->grade != GUILD_LEADER_GRADE)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 직위를 변경할 권한이 없습니다.");
            else if (ch->GetPlayerID() == pid)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 길드장의 직위는 변경할 수 "
                                   "없습니다.");
            else if (grade == 1)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 길드장으로 직위를 변경할 수 "
                                   "없습니다.");
            else
                pGuild->ChangeMemberGrade(pid, grade);
        }
            return true;

        case GUILD_SUBHEADER_CG_USE_SKILL: {
            if (!p.useSkill)
                return true;

            const auto& useSkill = p.useSkill.value();

            pGuild->UseSkill(useSkill.skillIndex, ch, useSkill.targetVid);
        }
            return true;

        case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL: {
            if (!p.changeGeneral)
                return true;

            const auto& cg = p.changeGeneral.value();

            const uint32_t pid = cg.pid;
            const uint8_t is_general = cg.flag;
            const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

            if (nullptr == m)
                return true;

            if (m->grade != GUILD_LEADER_GRADE) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "<길드> 장군을 지정할 권한이 없습니다.");
            } else {
                if (!pGuild->ChangeMemberGeneral(pid, is_general)) {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "<길드> 더이상 장수를 지정할 수 "
                                       "없습니다.");
                }
            }
        }
            return true;

        case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER: {
            if (!p.inviteAnswer)
                return true;

            const auto& ia = p.inviteAnswer.value();

            const uint32_t guild_id = ia.gid;
            const uint8_t accept = ia.flag;

            CGuild* g = CGuildManager::instance().FindGuild(guild_id);

            if (g) {
                if (accept)
                    g->InviteAccept(ch);
                else
                    g->InviteDeny(ch->GetPlayerID());
            }
        }
            return true;
    }

    return true;
}

void CInputMain::FishingGame(CHARACTER* ch, const TPacketCGFishingGame& p)
{
    ch->fishing_take(p.hitCount, p.time);
}

void CInputMain::Fishing(CHARACTER* ch, const TPacketCGFishing& p)
{
    ch->SetRotation(p.dir * 5.0f);
    ch->fishing();
    return;
}

void CInputMain::ItemGive(CHARACTER* ch, const TPacketCGGiveItem& p)
{
    CHARACTER* to_ch = g_pCharManager->Find(p.dwTargetVID);

    if (to_ch)
        ch->GiveItem(to_ch, p.ItemPos);
    else
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "아이템을 건네줄 수 없습니다.");
}

void CInputMain::Hack(CHARACTER* ch, const TPacketCGHack& p) {}

int CInputMain::MyShop(CHARACTER* ch, const CgMyShopOpen& p)
{

    if (ch->GetGold() >= ch->GetAllowedGold()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "소유 돈이 20억냥을 넘어 거래를 핼수가 없습니다.");
        SPDLOG_DEBUG("MyShop ==> OverFlow Gold id {0} name {1}. {2} >= {3}",
                     ch->GetPlayerID(), ch->GetName(), ch->GetGold(),
                     ch->GetAllowedGold());
        return (true);
    }

    if (ch->IsStun() || ch->IsDead())
        return (true);

    if (ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() ||
        ch->IsCubeOpen()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "다른 거래중일경우 개인상점을 열수가 없습니다.");
        return (true);
    }

    ch->OpenMyShop(p.sign.c_str(), p.bundleItem, p.table.data(),
                   p.table.size());

    return true;
}

void CInputMain::Refine(CHARACTER* ch, const TPacketCGRefine& p)
{
    if (ch->GetExchange() || ch->IsOpenSafebox() || ch->GetShopOwner() ||
        ch->IsCubeOpen()) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           ("창고,거래창등이 열린 상태에서는 개량을 할수가 "
                            "없습니다"));
        ch->ClearRefineMode();
        return;
    }

    if (p.type == 255) {
        // DoRefine Cancel
        ch->ClearRefineMode();
        return;
    }

    if (p.pos >= INVENTORY_MAX_NUM) {
        ch->ClearRefineMode();
        return;
    }

    CItem* item = ch->GetInventoryItem(p.pos);

    if (!item) {
        ch->ClearRefineMode();
        return;
    }

    ch->SetRefineTime();

    if (p.type == REFINE_TYPE_NORMAL) {
        SPDLOG_INFO("refine_type_normal");
        ch->DoRefine(item, p.type);
    } else if (p.type == REFINE_TYPE_SCROLL || p.type == REFINE_TYPE_HYUNIRON ||
               p.type == REFINE_TYPE_MUSIN || p.type == REFINE_TYPE_BDRAGON) {
        SPDLOG_INFO("refine_type_scroll, ...");
        ch->DoRefineWithScroll(item, p.type);
    } else if (p.type == REFINE_TYPE_MONEY_ONLY) {
        auto invenItem = ch->GetInventoryItem(p.pos);
        if (invenItem) {
            if (500 <= invenItem->GetRefineSet()) {
                LogManager::instance().HackLog("DT_REFINE_SET_HACK", ch);
            } else {
                CHARACTER* refNPC = ch->GetRefineNPC();
                if (!refNPC) {
                    LogManager::instance().HackLog("DT_DIRECT_REFINE_HACK", ch);
                } else if (!refNPC->CanReceiveItem(ch, invenItem, true)) {
                    LogManager::instance().HackLog("DT_TYPE_REFINE_HACK", ch);
                    return;
                }

                if (ch->GetQuestFlag("deviltower_zone.can_refine")) {
                    ch->DoRefine(invenItem, REFINE_TYPE_MONEY_ONLY);
                    ch->SetQuestFlag("deviltower_zone.can_refine", 0);
                } else {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                       "You can only upgrade items once.");
                }
            }
        }
    }

    ch->ClearRefineMode();
}

void CInputMain::OpenPlayerShop(CHARACTER* ch)
{
#ifdef __OFFLINE_SHOP__
    ch->OfflineShopStartCreate();
#else
#ifdef ENABLE_PRIVATESHOP_SYSTEM_ALWAYS_SILK
    if (!ch->CountSpecifyItem(71221))
        ch->UseSilkBotary();
    else
        ch->UseKashmirBotary();
#else
    ch->OpenPrivateShop();
#endif

#endif
}

void CInputMain::MyShopWarp(CHARACTER* ch)
{
    if (MeleyLair::CMgr::instance().IsMeleyMap(ch->GetMapIndex())) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return;
    }
    const auto now = get_global_time();
    if (auto* questPc =
            quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
        questPc) {
        uint32_t useCycle =
            quest::CQuestManager::instance().GetEventFlag("shop_warp_cycle");
        if (!useCycle)
            useCycle = 1;
        uint32_t laseUseTime = questPc->GetFlag("ShopWarp.LastUseTime");

        if (laseUseTime + useCycle > now) {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                               "You can not use this so fast.");
            return;
        }
    }

    if (auto* offShop = ch->GetMyOfflineShop(); offShop) {
        if (ch->CanWarp() && !ch->IsHack()) {
            if (const auto questPc =
                    quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
                questPc) {
                questPc->SetFlag("ShopWarp.LastUseTime", now);
            }
            ch->WarpSet(offShop->GetMapIndex(), offShop->GetX(),
                        offShop->GetY(), offShop->GetChannel());
        } else {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You cannot warp right now");
        }
    }
}

void CInputMain::WikiRequest(CHARACTER* ch, const CgRecvWikiPacket& p)
{
    if (!p.is_mob) {
        const TWikiItemInfo* tbl =
            ITEM_MANAGER::instance().GetItemWikiInfo(p.vnum);
        if (tbl) {
            const std::vector<TWikiItemOriginInfo>& originVec =
                ITEM_MANAGER::Instance().GetItemOrigin(p.vnum);

            GcWikiItemInfo sub;
            sub.bIsCommon = tbl->bIsCommon;
            sub.isSet = tbl->isSet;
            sub.dwOrigin = tbl->dwOrigin;

            GcWikiPacket p2;
            p2.itemInfo = sub;
            p2.chestInfos = tbl->pChestInfo;
            p2.refineInfo = tbl->pRefineData;
            p2.originInfos = originVec;
            p2.ret_id = p.ret_id;
            p2.vnum = p.vnum;
            ch->GetDesc()->Send(HEADER_GC_WIKI, p2);
        }
    } else {
        const auto& pvecDropInfo =
            ITEM_MANAGER::instance().FindDropInfo(p.vnum);

        GcWikiMobPacket p2;
        p2.ret_id = p.ret_id;
        p2.vnum = p.vnum;

        if (pvecDropInfo) {
            p2.vnums = pvecDropInfo.value();
        }
        ch->GetDesc()->Send(HEADER_GC_WIKI_MOB, p2);
    }
}

void CInputMain::OpenPlayerShopSearch(CHARACTER* ch)
{
#ifdef DISABLE_PRIVATE_SHOP_SEARCH_NEED_ITEM
    ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenPShopSearchDialogCash");
#else
    if (ch->CountSpecifyItem(60005))
        ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenPShopSearchDialogCash");
    else if (ch->CountSpecifyItem(60004))
        ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenPShopSearchDialog");
    else
        ch->SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "YOU_NEED_A_SHOP_SEARCH_GLASS");
#endif
}

int CInputMain::MyShopDeco(CHARACTER* ch, const TPacketCGMyShopDeco& p)
{
    switch (p.subheader) {
        case MYSHOP_DECO_SUBHEADER_CG_STATE: {
            ch->SetMyShopDecoWindowOpen(p.state.value());
        }
            return true;

        case MYSHOP_DECO_SUBHEADER_CG_SET: {
            const auto& subpkt = p.set.value();
            ch->SetShopTitleType(subpkt.titleType);
            ch->SetShopVnum(subpkt.polyVnum);
            ch->OpenPrivateShop();
        }
            return true;
    }

    return true;
}

int CInputMain::Switchbot(CHARACTER* ch, const CgSwitchbotPacket& p)
{

    switch (p.subheader) {
        case SWITCHBOT_SUBHEADER_CG_SET_ATTRIBUTE: {
            const CgSwitchbotAttributeUpdatePacket& subPacket =
                p.updateAttr.value();
            ch->UpdateSwitchbotSlotAttribute(
                subPacket.slotIndex, subPacket.altIndex, subPacket.attrIndex,
                subPacket.attribute);
        }
            return true;

        case SWITCHBOT_SUBHEADER_CG_START_SLOT: {
            const CgSwitchbotStatusPacket& subPacket = p.status.value();
            ch->ActivateSwitchbotSlot(subPacket.slotIndex);
        }
            return true;

        case SWITCHBOT_SUBHEADER_CG_STOP_SLOT: {
            const CgSwitchbotStatusPacket& subPacket = p.status.value();
            ch->DeactivateSwitchbotSlot(subPacket.slotIndex);
        }
            return true;
    }

    return true;
}

int CInputMain::LevelPet(CHARACTER* ch, const CgLevelPetPacket& p)
{

    switch (p.subheader) {
        case LEVELPET_SUBHEADER_CG_OPEN: {
            TItemPos InventoryPos = p.itemPos.value();

            ch->LevelPetOpen(InventoryPos);
            return true;
        }

        case LEVELPET_SUBHEADER_CG_CLOSE: {
            ch->LevelPetClose();
            return true;
        }

        case LEVELPET_SUBHEADER_CG_PLUS_ATTR: {
            uint8_t index = p.index.value();

            ch->LevelPetPlusAttr(index);
            return true;
        }
    }

    return true;
}

int CInputMain::Changelook(CHARACTER* ch, const CgChangeLookPacket& p)
{

    switch (p.subheader) {
        case CHANGELOOK_SUBHEADER_CG_REFINE_CHECKIN: {
            if (!p.checkin)
                return true;
            ch->ChangeLookCheckin(p.checkin.value().targetPos,
                                  p.checkin.value().invenPos);
            return true;
        }

        case CHANGELOOK_SUBHEADER_CG_REFINE_CHECKOUT: {
            if (!p.checkout)
                return true;
            ch->ChangeLookCheckout(p.checkout.value());
        }
            return true;

        case CHANGELOOK_SUBHEADER_CG_REFINE_ACCEPT: {
            ch->ChangeLookAccept();
        }
            return true;

        case CHANGELOOK_SUBHEADER_CG_REFINE_CANCEL:
            ch->ChangeLookCancel();
            return true;
    }

    return true;
}

int CInputMain::AcceRefine(CHARACTER* ch, const TPacketCGAcce& p)
{

    switch (p.subheader) {
        case ACCE_SUBHEADER_CG_REFINE_CHECKIN: {
            const auto& subpkt = p.checkin.value();
            ch->AcceRefineCheckin(subpkt.bAccePos, subpkt.ItemPos,
                                  subpkt.windowType);
        }
            return true;

        case ACCE_SUBHEADER_CG_REFINE_CHECKOUT: {
            const auto& subpkt = p.checkout.value();

            ch->AcceRefineCheckout(subpkt.bAccePos);
        }
            return true;

        case ACCE_SUBHEADER_CG_REFINE_ACCEPT: {
            const auto& subpkt = p.accept.value();

            ch->AcceRefineAccept(subpkt.windowType);
        }
            return true;

        case ACCE_SUBHEADER_CG_REFINE_CANCEL:
            ch->AcceRefineClear();
            return true;
    }

    return true;
}

#ifdef ENABLE_MOVE_COSTUME_ATTR
void CInputMain::ItemCombination(CHARACTER* ch,
                                 const TPacketCGItemCombiation& p)
{
    if (!p.mediumPos.IsValidItemPosition())
        return;

    if (!p.basePos.IsValidItemPosition())
        return;

    if (!p.materialPos.IsValidItemPosition())
        return;

    auto mediumItem = ch->GetItem(p.mediumPos);
    auto baseItem = ch->GetItem(p.basePos);
    auto materialItem = ch->GetItem(p.materialPos);

    if (!mediumItem || !baseItem || !materialItem)
        return;

    if (mediumItem->GetItemType() != ITEM_MEDIUM)
        return;

    if (mediumItem->GetSubType() != MEDIUM_MOVE_COSTUME_ATTR)
        return;

    if (baseItem->GetItemType() != ITEM_COSTUME ||
        materialItem->GetItemType() != ITEM_COSTUME)
        return;

    if (baseItem->GetSubType() == COSTUME_ACCE ||
        baseItem->GetSubType() == COSTUME_MOUNT)
        return;

    if (materialItem->GetSubType() == COSTUME_ACCE ||
        materialItem->GetSubType() == COSTUME_MOUNT)
        return;

    if (baseItem->GetSubType() != materialItem->GetSubType())
        return;

    materialItem->CopyAttributeTo(baseItem);

    ITEM_MANAGER::instance().SaveSingleItem(baseItem);
    baseItem->UpdatePacket();
    ITEM_MANAGER::instance().RemoveItem(materialItem, "COSTUME_COMBINE");
    mediumItem->SetCount(mediumItem->GetCount() - 1);
}
#endif

bool CInputMain::Analyze(DESC* d, const PacketHeader& header,
                         const boost::asio::const_buffer& data)
{
    if (!d) {
        SPDLOG_ERROR("no desc");
        return true;
    }

    CHARACTER* ch = d->GetCharacter();
    if (!ch) {
        SPDLOG_ERROR("no character on desc");
        d->DelayedDisconnect(0, "NO_CHARACTER");
        return true;
    }

    SPDLOG_DEBUG("CInputMain::Analyze() ==> Header [{0}] ", header.id);

    switch (header.id) {
        case HEADER_CG_PONG:
            Pong(d);
            return true;
        case HEADER_CG_REQ_TIMESYNC:
            d->SendHandshake(get_dword_time(), 0);
            return true;
        case HEADER_CG_TIME_SYNC:
            Handshake(d, ReadPacket<TPacketCGHandshake>(data));
            return true;

        case HEADER_CG_CHAT:
            return Chat(ch, ReadPacket<TPacketCGChat>(data));

        case HEADER_CG_WHISPER:
            return Whisper(ch, ReadPacket<TPacketCGWhisper>(data));

        case HEADER_CG_CHARACTER_MOVE:
            Move(ch, ReadPacket<TPacketCGMove>(data));
            return true;

        case HEADER_CG_SET_TITLE:
            SetTitle(ch, ReadPacket<CgSetTitlePacket>(data));
            return true;

        case HEADER_CG_ITEM_USE:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemUse(ch, ReadPacket<TPacketGCItemUse>(data));
            return true;

        case HEADER_CG_ITEM_USE_MULTIPLE:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemUseMultiple(ch, ReadPacket<TPacketGCItemUse>(data));
            return true;

        case HEADER_CG_ITEM_DROP:
            if (ch->IsGM() || !ch->IsObserverMode()) {
                ItemDrop(ch, ReadPacket<TPacketCGItemDrop>(data));
            }
            return true;

        case HEADER_CG_ITEM_DROP2:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemDrop2(ch, ReadPacket<TPacketCGItemDrop2>(data));
            return true;

        case HEADER_GC_REMOVE_METIN:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemRemoveMetin(ch, ReadPacket<CgRemoveMetinPacket>(data));
            return true;

        case HEADER_CG_ITEM_DESTROY:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemDestroy(ch, ReadPacket<TPacketCGItemDestroy>(data));
            return true;

        case HEADER_CG_ITEM_MOVE:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemMove(ch, ReadPacket<TPacketCGItemMove>(data));
            return true;

        case HEADER_CG_ITEM_SPLIT:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemSplit(ch, ReadPacket<TPacketCGItemSplit>(data));
            return true;

        case HEADER_CG_ITEM_PICKUP:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemPickup(ch, ReadPacket<TPacketCGItemPickup>(data));
            return true;

        case HEADER_CG_DUNGEON_INFO:
			ch->SendDungeonInfo();
			break;
			
		case HEADER_CG_DUNGEON_WARP:
			CDungeonInfo::instance().DungeonWarpSet(ch, ReadPacket<TPacketCGDungeonWarp>(data));
			break;
			
		case HEADER_CG_DUNGEON_RANKING:
			CDungeonInfo::instance().GetRankingSet(ch, ReadPacket<TPacketCGDungeonRanking>(data));
			break;

        case HEADER_CG_ITEM_USE_TO_ITEM:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemToItem(ch, ReadPacket<TPacketCGItemUseToItem>(data));
            return true;

        case HEADER_CG_ITEM_GIVE:
            if (ch->IsGM() || !ch->IsObserverMode())
                ItemGive(ch, ReadPacket<TPacketCGGiveItem>(data));
            return true;

        case HEADER_CG_EXCHANGE:
            if (ch->IsGM() || !ch->IsObserverMode())
                Exchange(ch, ReadPacket<TPacketCGExchange>(data));
            return true;

        case HEADER_CG_ATTACK:
            if (ch->IsGM() || !ch->IsObserverMode())
                Attack(ch, ReadPacket<TPacketCGAttack>(data));
            return true;

        case HEADER_CG_SHOOT:
            if (ch->IsGM() || !ch->IsObserverMode())
                Shoot(ch, ReadPacket<TPacketCGShoot>(data));
            return true;

        case HEADER_CG_USE_SKILL:
            if (ch->IsGM() || !ch->IsObserverMode())
                UseSkill(ch, ReadPacket<TPacketCGUseSkill>(data));
            return true;

        case HEADER_CG_CHANGE_SKILL_COLOR:
            if (ch->IsGM() || !ch->IsObserverMode())
                ChangeSkillColor(
                    ch, ReadPacket<TPacketCGChangeSkillColorPacket>(data));
            return true;

        case HEADER_CG_CHOOSE_SKILL_GROUP:
            if (ch->IsGM() || !ch->IsObserverMode())
                ChooseSkillGroup(ch,
                                 ReadPacket<TPacketCGChooseSkillGroup>(data));
            return true;

        case HEADER_CG_BLOCK_MODE:
            BlockMode(ch, ReadPacket<CgBlockModePacket>(data));
            return true;

        case HEADER_CG_QUICKSLOT_ADD:
            QuickslotAdd(ch, ReadPacket<TPacketCGQuickslotAdd>(data));
            return true;

        case HEADER_CG_QUICKSLOT_DEL:
            QuickslotDelete(ch, ReadPacket<TPacketCGQuickslotDel>(data));
            return true;

        case HEADER_CG_QUICKSLOT_SWAP:
            QuickslotSwap(ch, ReadPacket<TPacketCGQuickslotSwap>(data));
            return true;

        case HEADER_CG_CHAT_FILTER:
            ChatFilter(ch, ReadPacket<TPacketCGChatFilter>(data));
            return true;

        case HEADER_CG_SWITCHBOT:
            return Switchbot(ch, ReadPacket<CgSwitchbotPacket>(data));

        case HEADER_CG_ACCE:
            return AcceRefine(ch, ReadPacket<TPacketCGAcce>(data));

        case HEADER_CG_CHANGELOOK:
            return Changelook(ch, ReadPacket<CgChangeLookPacket>(data));
        case HEADER_CG_LEVEL_PET:
            return LevelPet(ch, ReadPacket<CgLevelPetPacket>(data));
        case HEADER_CG_MYSHOP_DECO:
            return MyShopDeco(ch, ReadPacket<TPacketCGMyShopDeco>(data));

#ifdef ENABLE_MOVE_COSTUME_ATTR
        case HEADER_CG_ITEM_COMBINATION:
            ItemCombination(ch, ReadPacket<TPacketCGItemCombiation>(data));
            return true;
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
        case HEADER_CG_SHOP_SEARCH:
            if (ch->IsGM() || !ch->IsObserverMode())
                ShopSearch(ch, ReadPacket<TPacketCGShopSearch>(data));
            return true;
#endif

        case HEADER_CG_SHOP:
            Shop(ch, ReadPacket<TPacketCGShop>(data));
            return true;

        case HEADER_CG_MESSENGER:
            return Messenger(ch, ReadPacket<TPacketCGMessenger>(data));

#ifdef ENABLE_BATTLE_PASS
        case HEADER_CG_BATTLE_PASS:
            BattlePass(ch, ReadPacket<TPacketCGBattlePassAction>(data));
            return true;
#endif

        case HEADER_CG_ON_CLICK:
            OnClick(ch, ReadPacket<TPacketCGOnClick>(data));
            return true;

        case HEADER_CG_SYNC_POSITION:
            return SyncPosition(ch, ReadPacket<TPacketCGSyncPosition>(data));

        case HEADER_CG_ADD_FLY_TARGETING:
        case HEADER_CG_FLY_TARGETING:
            FlyTarget(ch, ReadPacket<TPacketCGFlyTargeting>(data), header.id);
            return true;

        case HEADER_CG_SCRIPT_BUTTON:
            ScriptButton(ch, ReadPacket<TPacketCGScriptButton>(data));
            return true;

        case HEADER_CG_SCRIPT_SELECT_ITEM:
            ScriptSelectItem(ch, ReadPacket<TPacketCGScriptSelectItem>(data));
            return true;

        case HEADER_CG_SCRIPT_ANSWER:
            ScriptAnswer(ch, ReadPacket<TPacketCGScriptAnswer>(data));
            return true;

        case HEADER_CG_QUEST_INPUT_STRING:
            QuestInputString(ch, ReadPacket<TPacketCGQuestInputString>(data));
            return true;

        case HEADER_CG_QUEST_RECEIVE:
            QuestReceive(ch, ReadPacket<TPacketCGQuestRcv>(data));
            return true;

        case HEADER_CG_QUEST_CONFIRM:
            QuestConfirm(ch, ReadPacket<TPacketCGQuestConfirm>(data));
            return true;

        case HEADER_CG_TARGET:
            Target(ch, ReadPacket<TPacketCGTarget>(data));
            return true;

        case HEADER_CG_TARGET_LOAD:
            LoadTargetInfo(ch, ReadPacket<TPacketCGTargetLoad>(data));
            return true;

        case HEADER_CG_SAFEBOX_CHECKIN:
            SafeboxCheckin(ch, ReadPacket<TPacketCGSafeboxCheckin>(data));
            return true;

        case HEADER_CG_SAFEBOX_CHECKOUT:
            SafeboxCheckout(ch, ReadPacket<TPacketCGSafeboxCheckout>(data),
                            false);
            return true;

        case HEADER_CG_SAFEBOX_ITEM_MOVE:
            SafeboxItemMove(ch, ReadPacket<TPacketCGItemMove>(data));
            return true;

        case HEADER_CG_MALL_CHECKOUT:
            SafeboxCheckout(ch, ReadPacket<TPacketCGSafeboxCheckout>(data),
                            true);
            return true;

        case HEADER_CG_PARTY_INVITE:
            PartyInvite(ch, ReadPacket<TPacketCGPartyInvite>(data));
            return true;

        case HEADER_CG_PARTY_REMOVE:
            PartyRemove(ch, ReadPacket<TPacketCGPartyRemove>(data));
            return true;

        case HEADER_CG_PARTY_INVITE_ANSWER:
            PartyInviteAnswer(ch, ReadPacket<TPacketCGPartyInviteAnswer>(data));
            return true;

        case HEADER_CG_PARTY_SET_STATE:
            PartySetState(ch, ReadPacket<TPacketCGPartySetState>(data));
            return true;

        case HEADER_CG_PARTY_USE_SKILL:
            PartyUseSkill(ch, ReadPacket<TPacketCGPartyUseSkill>(data));
            return true;

        case HEADER_CG_PARTY_PARAMETER:
            PartyParameter(ch, ReadPacket<TPacketCGPartyParameter>(data));
            return true;

        case HEADER_CG_ANSWER_MAKE_GUILD:
            AnswerMakeGuild(ch, ReadPacket<TPacketCGAnswerMakeGuild>(data));
            return true;

        case HEADER_CG_GUILD:
            return Guild(ch, ReadPacket<TPacketCGGuild>(data));

        case HEADER_CG_FISHING:
            Fishing(ch, ReadPacket<TPacketCGFishing>(data));
            return true;

        case HEADER_CG_FISHING_GAME:
            FishingGame(ch, ReadPacket<TPacketCGFishingGame>(data));
            return true;

        case HEADER_CG_HACK:
            Hack(ch, ReadPacket<TPacketCGHack>(data));
            return true;

        case HEADER_CG_MYSHOP:
            return MyShop(ch, ReadPacket<CgMyShopOpen>(data));

        case HEADER_CG_REFINE:
            Refine(ch, ReadPacket<TPacketCGRefine>(data));
            return true;

#if defined(WJ_COMBAT_ZONE)
        case HEADER_CG_COMBAT_ZONE_REQUEST_ACTION:
            CCombatZoneManager::instance().RequestAction(ch, data);
            return true;
#endif

        case HEADER_CG_DRAGON_SOUL_REFINE: {
            auto p = ReadPacket<TPacketCGDragonSoulRefine>(data);
            switch (p.bSubType) {
                case DS_SUB_HEADER_CLOSE:
                    ch->DragonSoul_RefineWindow_Close();
                    break;
                case DS_SUB_HEADER_DO_REFINE_GRADE: {
                    DSManager::instance().DoRefineGrade(ch, p.ItemGrid);
                } break;
                case DS_SUB_HEADER_DO_REFINE_STEP: {
                    DSManager::instance().DoRefineStep(ch, p.ItemGrid);
                } break;
                case DS_SUB_HEADER_DO_REFINE_STRENGTH: {
                    DSManager::instance().DoRefineStrength(ch, p.ItemGrid);
                } break;
            }
            return true;
        }

#ifdef __OFFLINE_SHOP__
        case HEADER_CG_OFFLINE_SHOP:
            this->ReceiveOfflineShopPacket(
                ch, ReadPacket<TPacketCGOfflineShop>(data));
            return true;

        case HEADER_CG_OFFLINE_SHOP_CREATE: {
            return this->ReceiveOfflineShopCreatePacket(
                ch, ReadPacket<TPacketCGOfflineShopCreate>(data));
        }

        case HEADER_CG_OFFLINE_SHOP_NAME:
            this->ReceiveOfflineShopNamePacket(
                ch, ReadPacket<TPacketCGOfflineShopName>(data));
            return true;

#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
        case HEADER_CG_OFFLINE_SHOP_POSITION:
            this->ReceiveOfflineShopPositionPacket(
                ch, ((TPacketCGOfflineShopPosition*)data));
            return true;
#endif

        case HEADER_CG_OFFLINE_SHOP_ITEM_ADD:
            this->ReceiveOfflineShopAddItemPacket(
                ch, ReadPacket<TPacketCGOfflineShopAddItem>(data));
            return true;

        case HEADER_CG_OFFLINE_SHOP_ITEM_MOVE:
            this->ReceiveOfflineShopMoveItemPacket(
                ch, ReadPacket<TPacketCGOfflineShopMoveItem>(data));
            return true;

        case HEADER_CG_OFFLINE_SHOP_ITEM_REMOVE:
            this->ReceiveOfflineShopRemoveItemPacket(
                ch, ReadPacket<TPacketCGOfflineShopRemoveItem>(data));
            return true;

        case HEADER_CG_OFFLINE_SHOP_ITEM_BUY:
            this->ReceiveOfflineShopBuyItemPacket(
                ch, ReadPacket<TPacketCGOfflineShopBuyItem>(data));
            return true;

        case HEADER_CG_OFFLINE_SHOP_WITHDRAW_GOLD:
            this->ReceiveOfflineShopWithdrawGoldPacket(
                ch, ReadPacket<TPacketCGOfflineShopWithdrawGold>(data));
            return true;

        case HEADER_CG_OFFLINE_SHOP_CLICK:
            this->ReceiveOfflineShopClickShopPacket(
                ch, ReadPacket<TPacketCGOfflineShopClickShop>(data));
            return true;

        case HEADER_CG_OFFLINE_SHOP_REOPEN:
            this->ReceiveOfflineShopReopenPacket(
                ch, ReadPacket<TPacketCGOfflineShopReopen>(data));
            return true;
#endif

        case HEADER_CG_MYSHOP_WARP:
            MyShopWarp(ch);
            return true;

        case HEADER_CG_MYSHOP_OPEN:
            OpenPlayerShop(ch);
            return true;

        case HEADER_CG_MYSHOP_OPEN_SEARCH:
            OpenPlayerShopSearch(ch);
            return true;

        case HEADER_CG_WIKI_REQUEST: {
            WikiRequest(ch, ReadPacket<CgRecvWikiPacket>(data));
            return true;
        }

        default:
            SPDLOG_ERROR("Phase game does not handle this packet: {}",
                         header.id);
            return true;
    }

    return true;
}

bool CInputDead::Analyze(DESC* d, const PacketHeader& header,
                         const boost::asio::const_buffer& data)
{
    CHARACTER* ch = d->GetCharacter();
    if (!ch) {
        SPDLOG_ERROR("no character on desc");
        d->DelayedDisconnect(0, "NO_CHARACTER");
        return true;
    }

    switch (header.id) {
        case HEADER_CG_PONG:
            Pong(d);
            break;

        case HEADER_CG_TIME_SYNC:
            Handshake(d, ReadPacket<TPacketCGHandshake>(data));
            break;

        case HEADER_CG_TARGET:
            Target(ch, ReadPacket<TPacketCGTarget>(data));
            break;

        case HEADER_CG_TARGET_LOAD:
            LoadTargetInfo(ch, ReadPacket<TPacketCGTargetLoad>(data));
            break;

        default: {
            return true;
        }
    }

    return true;
}

bool CInputMain::SyncPosition(CHARACTER* ch, const TPacketCGSyncPosition& p)
{
    int32_t iCount = p.elems.size();

    if (iCount <= 0)
        return true;

    static const int32_t nCountLimit = 16;

    if (iCount > nCountLimit) {
        SPDLOG_WARN("Too many SyncPosition Count({}) from Name({})", iCount,
                    ch->GetName());
        iCount = nCountLimit;
    }

    timeval tvCurTime;
    gettimeofday(&tvCurTime, nullptr);

    decltype(p.elems) v;

    for (const auto& e : p.elems) {
        auto* victim = CHARACTER_MANAGER::instance().Find(e.dwVID);

        if (!victim)
            continue;

        switch (victim->GetCharType()) {
            case CHAR_TYPE_NPC:
            case CHAR_TYPE_WARP:
            case CHAR_TYPE_GOTO:
                continue;
        }

        if (!victim->SetSyncOwner(ch))
            continue;

        const float fDistWithSyncOwner =
            DISTANCE_SQRT((victim->GetX() - ch->GetX()) / 100,
                          (victim->GetY() - ch->GetY()) / 100);
        static const float fLimitDistWithSyncOwner = 25.f + 10.f;
        if (fDistWithSyncOwner > fLimitDistWithSyncOwner) {
            if (ch->GetSyncHackCount() < gConfig.syncHackLimitCount) {
				ch->UpdateSyncHackCount(victim->IsPC() ? "player" : "monster", true);
				ch->SetNoHackCount(0);
                continue;
            } else {
                LogManager::instance().HackLog("SYNC_POSITION_COUNT", ch);

                SPDLOG_ERROR("Too far SyncPosition "
                             "DistanceWithSyncOwner({:f})({}) from Name({}) "
                             "CH({},{}) VICTIM({},{}) "
                             "SYNC({},{})",
                             fDistWithSyncOwner, victim->GetName(),
                             ch->GetName(), ch->GetX(), ch->GetY(),
                             victim->GetX(), victim->GetY(), e.lX, e.lY);

                ch->GetDesc()->DelayedDisconnect(0, "SYNC_POSITION_HACK_COUNT");

                return true;
            }
        }

        const float fDist = DISTANCE_SQRT((victim->GetX() - e.lX) / 100,
                                          (victim->GetY() - e.lY) / 100);
        static const int32_t g_lValidSyncInterval =
            gConfig.syncHackLimitTime * 1000;
        const timeval& tvLastSyncTime = victim->GetLastSyncTime();
        timeval* tvDiff = timediff(&tvCurTime, &tvLastSyncTime);

        if (tvDiff->tv_sec == 0 && tvDiff->tv_usec < g_lValidSyncInterval) {
            if (ch->GetSyncHackCount("player") < gConfig.syncHackLimitCount &&
                ch->GetSyncHackCount("monster") < gConfig.syncHackLimitCount * 2
                ) {

                std::string s_targetType = victim->IsPC() ? "player" : "monster";

				ch->UpdateSyncHackCount(s_targetType, true);
				ch->SetNoHackCount(0);
				continue;
            } else {
                LogManager::instance().HackLog("SYNC_POSITION_TIME", ch);

                SPDLOG_ERROR("Too often SyncPosition Interval({}ms)({}) from "
                             "Name({}) VICTIM({},{}) SYNC({},{})",
                             tvDiff->tv_sec * 1000 + tvDiff->tv_usec / 1000,
                             victim->GetName(), ch->GetName(), victim->GetX(),
                             victim->GetY(), e.lX, e.lY);
                ch->GetDesc()->DelayedDisconnect(0, "SYNC_POSITION_HACK_TIME");

                return true;
            }
        } else if (fDist > 25.0f) {
            LogManager::instance().HackLog("SYNC_POSITION_DIST", ch);

            SPDLOG_ERROR("Too far SyncPosition Distance({})({}) from Name({}) "
                         "CH({},{}) VICTIM({},{}) SYNC({},{})",
                         fDist, victim->GetName(), ch->GetName(), ch->GetX(),
                         ch->GetY(), victim->GetX(), victim->GetY(), e.lX,
                         e.lY);

            ch->GetDesc()->DelayedDisconnect(0, "SYNC_POSITION_HACK_DIST");

            return true;
        } else {
            if (ch->GetSyncHackCount("monster") > 0) 
			{
				int i_curCount = ch->GetNoHackCount();
				ch->SetNoHackCount(++i_curCount);
				
				if (i_curCount > 10) {
					ch->SetNoHackCount(0);
					ch->UpdateSyncHackCount("monster", false);
				}
			}

            victim->SetLastSyncTime(tvCurTime);
            victim->Sync(e.lX, e.lY);
            v.emplace_back(e);
        }
    }

    TPacketGCSyncPosition p2;
    p2.elems = v;
    PacketAround(ch->GetViewMap(), ch, HEADER_GC_SYNC_POSITION, p2, ch);

    return true;
}

void CInputMain::ChatFilter(CHARACTER* ch, const TPacketCGChatFilter& p)
{
    if (!ch)
        return;

    for (int i = 0; i < LANGUAGE_MAX_NUM + 1; i++) {
        if (p.bFilterList[i] == 1)
            ch->SetChatFilter(i, true);
        else
            ch->SetChatFilter(i, false);
    }

    ch->UpdateChatFilter();
}
