#include "constants.h"

#include "config.h"
#include "utils.h"
#include "input.h"
#include "DbCacheSocket.hpp"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "GBufferManager.h"
#include "pvp.h"
#include "start_position.h"
#include "messenger_manager.h"
#include "guild_manager.h"
#include "party.h"
#include "dungeon.h"
#include "war_map.h"
#include "questmanager.h"
#include "building.h"
#include "wedding.h"
#include <game/AffectConstants.hpp>
#include "GArena.h"
#include "OXEvent.h"
#include "priv_manager.h"
#include "log.h"
#include "horsename_manager.h"
#include "MarkManager.h"
#include "threeway_war.h"
#include "main.h"
#if defined(WJ_COMBAT_ZONE)
#include "combat_zone.h"
#endif
#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

#ifdef ENABLE_HYDRA_DUNGEON
#include "HydraDungeon.h"
#endif

#include "gm.h"

#include "CharUtil.hpp"
#include "desc.h"
#include "guild.h"
#include "item.h"
#include "MasterUtil.hpp"
#include <storm/Util.hpp>
#include "OfflineShopManager.h"

static void _send_bonus_info(CHARACTER *ch)
{
    int item_drop_bonus = 0;
    int gold_drop_bonus = 0;
    int gold10_drop_bonus = 0;
    int exp_bonus = 0;

    item_drop_bonus = CPrivManager::instance().GetPriv(ch, PRIV_ITEM_DROP);
    gold_drop_bonus = CPrivManager::instance().GetPriv(ch, PRIV_GOLD_DROP);
    gold10_drop_bonus = CPrivManager::instance().GetPriv(ch, PRIV_GOLD10_DROP);
    exp_bonus = CPrivManager::instance().GetPriv(ch, PRIV_EXP_PCT);
    auto double_drop_count_bonus = CPrivManager::instance().GetPriv(ch, PRIV_DOUBLE_DROP_COUNT);

    if (item_drop_bonus)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_NOTICE,
                           ("아이템 드롭률  %d%% 추가 이벤트 중입니다."), item_drop_bonus);
    }
    if (gold_drop_bonus)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_NOTICE,
                           ("골드 드롭률 %d%% 추가 이벤트 중입니다."), gold_drop_bonus);
    }
    if (gold10_drop_bonus)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_NOTICE,
                           ("대박골드 드롭률 %d%% 추가 이벤트 중입니다."), gold10_drop_bonus);
    }
    if (exp_bonus)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_NOTICE,
                           ("경험치 %d%% 추가 획득 이벤트 중입니다."), exp_bonus);

    }

    auto globalExp = CPrivManager::instance().GetPrivByEmpire(0, PRIV_EXP_PCT);
	if(globalExp) {
		auto end_time = CPrivManager::instance().GetPrivEndByEmpire(0, PRIV_EXP_PCT);
		if(globalExp < 50)
    		ch->ChatPacket(CHAT_TYPE_COMMAND, "exp_bonus_event_start 0 %d %d", globalExp, end_time);
		else
    		ch->ChatPacket(CHAT_TYPE_COMMAND, "exp_bonus_event_start 1 %d %d", globalExp, end_time);
	}
	
    if (double_drop_count_bonus) { SendI18nChatPacket(ch, CHAT_TYPE_NOTICE, "The Double-Loot Event is active!"); }

    if(g_pCharManager->GetMobGoldAmountRate(ch) / 100 == 2) {
        SendI18nChatPacket(ch, CHAT_TYPE_NOTICE, "The Double-Yang Event is active.");
    }
}

static bool FN_is_battle_zone(CHARACTER *ch)
{
    switch (ch->GetMapIndex())
    {
    case 1:                 // 신수 1차 마을
    case 2:                 // 신수 2차 마을
    case 21:                // 천조 1차 마을
    case 23:                // 천조 2차 마을
    case 41:                // 진노 1차 마을
    case 43:                // 진노 2차 마을
    case OXEVENT_MAP_INDEX: // OX 맵
        return false;
    }

    return true;
}

void CInputLogin::LoginByKey(DESC *d, const CgKeyLoginPacket& p)
{

    char login[LOGIN_MAX_LEN + 1];
    trim_and_lower(p.login.c_str(), login, sizeof(login));

    if (g_bNoMoreClient)
    {
        TPacketGCLoginFailure failurePacket;
        failurePacket.szStatus = "SHUTDOWN";
        d->Send(HEADER_GC_LOGIN_FAILURE, failurePacket);
        return;
    }

    if (gConfig.userLimit > 0)
    {
        int iTotal;
        int *paiEmpireUserCount;
        int iLocal;

        DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

        if (gConfig.userLimit <= iTotal)
        {
            TPacketGCLoginFailure failurePacket;
            failurePacket.szStatus = "FULL";
            d->Send(HEADER_GC_LOGIN_FAILURE, failurePacket);
            return;
        }
    }

    SPDLOG_TRACE("LOGIN_BY_KEY: {0} session {1}", login, p.sessionId);

    d->SetLogin(login);
    SendLoginPacket(d->GetHandle(), login, p.sessionId);
}

void CInputLogin::ChangeName(DESC *d, const CgChangeNamePacket& p)
{


    const auto &tab = d->GetAccountTable();

    if (p.index >= PLAYER_PER_ACCOUNT)
    {
        GcCharacterCreateFailurePacket pFail = {GcCharacterPhaseFailurePacket::kStrangeIndex};
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }
   
    if (!tab.players[p.index].bChangeName)
        return;
    
    if (!check_name(p.name.c_str()))
    {
        GcCharacterCreateFailurePacket pFail = {GcCharacterPhaseFailurePacket::kInvalidName};
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }
    
    GdChangeNamePacket p2{};
    p2.pid = tab.players[p.index].id;
    storm::CopyStringSafe(p2.name, p.name);
    db_clientdesc->DBPacket(HEADER_GD_CHANGE_NAME, d->GetHandle(), &p2, sizeof(p2));

    db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
    db_clientdesc->Packet(&p2.pid, sizeof(uint32_t));

    LogManager::instance().ChangeNameLog(p2.pid, p.name.c_str(),
                                         tab.players[p.index].name,
                                         d->GetHostName().c_str());


}

void CInputLogin::ChangeEmpire(DESC *d, const CgChangeEmpirePacket& p)
{
    if (p.slot >= PLAYER_PER_ACCOUNT || p.empire >= EMPIRE_MAX_NUM)
        return;

    const auto &tab = d->GetAccountTable();
    if (!tab.players[p.slot].bChangeEmpire)
        return;

    GdChangeEmpirePacket p2;
    p2.pid = tab.players[p.slot].id;
    p2.empire = p.empire;
    db_clientdesc->DBPacket(HEADER_GD_CHANGE_EMPIRE, d->GetHandle(), &p2, sizeof(p2));
}

void CInputLogin::CharacterSelect(DESC *d, const TPacketCGPlayerSelect& p)
{
    auto &c_r = d->GetAccountTable();

    SPDLOG_TRACE("player_select: login: {0} index: {1}",
                d->GetLogin(), p.index);

    if (!c_r.id)
    {
        SPDLOG_ERROR("no account table");
        return;
    }

    if (p.index >= PLAYER_PER_ACCOUNT)
    {
        SPDLOG_ERROR("index overflow {0}, login: {1}",
                     p.index, d->GetLogin());
        return;
    }

    if (c_r.players[p.index].bChangeName)
    {
        SPDLOG_ERROR("name must be changed idx {0}, login {1}, name {2}",
                     p.index, d->GetLogin(), c_r.players[p.index].name);
        return;
    }

    if (c_r.players[p.index].bChangeEmpire)
    {
        SPDLOG_ERROR(
            "empire must be changed idx {0}, login {1}, empire {2}",
            p.index, d->GetLogin(),
            c_r.players[p.index].empire);
        return;
    }

    TPlayerLoadPacket p2;
    p2.account_id = c_r.id;
    p2.player_id = c_r.players[p.index].id;
    p2.account_index = p.index;
    c_r.players[p.index].last_play = get_global_time();

    db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD, d->GetHandle(), &p2, sizeof(TPlayerLoadPacket));

    char localeName[LOCALE_NAME_MAX_LEN + 1];
    CopyStringSafe(localeName, p.localeName);

    d->SetClientLocale(localeName);
}

// 신규 캐릭터 지원
bool NewPlayerTable2(TPlayerTable *table, const std::string&name, uint8_t race, uint8_t shape, uint8_t bEmpire)
{
    if (race >= MAIN_RACE_MAX_NUM)
    {
        return false;
    }

    const auto job = GetJobByRace(race);
    if (job == JOB_MAX_NUM)
    {
        return false;
    }


    std::memset(table, 0, sizeof(TPlayerTable));

    storm::CopyStringSafe(table->name, name);

    table->level = JobInitialPoints[job].start_lv;
    table->job = race; // 직업대신 종족을 넣는다
    table->voice = 0;
    table->part_base = shape;
    table->empire = bEmpire;

    table->st = JobInitialPoints[job].st;
    table->dx = JobInitialPoints[job].dx;
    table->ht = JobInitialPoints[job].ht;
    table->iq = JobInitialPoints[job].iq;

    table->hp = JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
    table->sp = JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
    table->stamina = JobInitialPoints[job].max_stamina;

    table->lMapIndex = CREATE_START_MAP(bEmpire);
    table->x = CREATE_START_X(bEmpire);
    table->y = CREATE_START_Y(bEmpire);

    table->x += Random::get(-300, 300);
    table->y += Random::get(-300, 300);
    table->z = 0;
    table->dir = 0;
    table->playtime = 0;
    table->gold = 0;

#ifdef ENABLE_GEM_SYSTEM
	table->gem = 0;
#endif

    table->skill_group = 0;
    table->stat_point = (table->level - 1) * 3;
    table->skill_point = table->level - 1;

    for (int i = 1; i < table->level; ++i)
    {
        table->sRandomHP += Random::get(JobInitialPoints[job].hp_per_lv_begin,
                                        JobInitialPoints[job].hp_per_lv_end);
        table->sRandomSP += Random::get(JobInitialPoints[job].sp_per_lv_begin,
                                        JobInitialPoints[job].sp_per_lv_end);
    }

    table->hp += table->sRandomHP;
    table->sp += table->sRandomSP;

    table->last_play = get_global_time();
 //   table->iOfflineShopFarmedOpeningTime = 60 * 60 * 6;
    table->iOfflineShopFarmedOpeningTime = 60 * 60 * 24 * 365;

    return true;
}

bool CheckCreateLevel(const TAccountTable &tab, uint8_t race)
{
    const auto job = GetJobByRace(race);
    if (job == JOB_MAX_NUM)
    {
        SPDLOG_ERROR("Invalid race {0}", race);
        return false;
    }

    if (JobInitialPoints[job].create_lv == 0)
        return true;

    if (gConfig.testServer)
        return true;

    for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
    {
        if (tab.players[i].byLevel >= JobInitialPoints[job].create_lv)
            return true;
    }

    return false;
}

void CInputLogin::CharacterCreate(DESC *d, const TPacketCGPlayerCreate& p)
{

    SPDLOG_TRACE("PlayerCreate: name {0} pos {1} job {2} shape {3}",
                p.name,
                p.index,
                p.job,
                p.shape);

    GcCharacterCreateFailurePacket pFail = {};

    if (gConfig.disableCharCreation)
    {
        pFail.type = GcCharacterCreateFailurePacket::kBlocked;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

#ifdef __NEW_GAMEMASTER_CONFIG__
    if (!GM::check_account_allow(d->GetLogin().c_str(), GM_ALLOW_CREATE_PLAYER))
    {
        SPDLOG_ERROR("gm may not create a character");
        pFail.type = GcCharacterCreateFailurePacket::kGmBlocked;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }
#endif

    unsigned job = GetJobByRace(p.job);

    if (job == JOB_MAX_NUM)
    {
        pFail.type = GcCharacterCreateFailurePacket::kInvalidRace;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    if (job == JOB_WOLFMAN)
    {
        pFail.type = GcCharacterCreateFailurePacket::kRaceBlocked;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    // 사용할 수 없는 이름이거나, 잘못된 평상복이면 생설 실패 GM_ALLOW_SPECIAL_NAMES
    if (!check_name(p.name.c_str()))
    {
        pFail.type = GcCharacterCreateFailurePacket::kInvalidName;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }
    if (p.shape > 1)
    {
        pFail.type = GcCharacterCreateFailurePacket::kInvalidShape;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    if (p.name.length() > PLAYER_NAME_MAX_LEN)
    {
        pFail.type = GcCharacterCreateFailurePacket::kNameLength;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    const auto &tab = d->GetAccountTable();
    if (!CheckCreateLevel(tab, p.job))
    {
        SPDLOG_TRACE("Error: Failed to create character: {0}: level",
                     p.name);
        pFail.type = GcCharacterCreateFailurePacket::kLevel;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    if (p.empire <= 0 || p.empire >= EMPIRE_MAX_NUM)
    {
        SPDLOG_ERROR("Failed to create character: {0}: invalid empire",
                     p.name);
        pFail.type = GcCharacterCreateFailurePacket::kInvalidEmpire;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    if (d->GetLogin() == p.name)
    {
        SPDLOG_TRACE("Error: Failed to create character: {0}: name == login",
                     p.name);
        pFail.type = GcCharacterCreateFailurePacket::kLoginName;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    TPlayerCreatePacket p2 = {};
    if (!NewPlayerTable2(&p2.player_table, p.name, p.job, p.shape, p.empire))
    {
        SPDLOG_ERROR("player_prototype error: job {0} face {1} ", p.job);
        pFail.type = GcCharacterCreateFailurePacket::kInvalidRace;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }

    p2.account_id = d->GetAid();
    p2.slot = p.index;

    SPDLOG_TRACE("PlayerCreate: name {0} index {1}", p.name, p.index);
    db_clientdesc->DBPacket(HEADER_GD_PLAYER_CREATE, d->GetHandle(), &p2, sizeof(p2));
}

void CInputLogin::CharacterDelete(DESC *d, const TPacketCGPlayerDelete& p)
{
    const auto &c_rAccountTable = d->GetAccountTable();

    if (d->GetAid() == 0)
    {
        SPDLOG_ERROR("PlayerDelete: no login data");
        return;
    }

    SPDLOG_TRACE("PlayerDelete: login: {0} index: {1}, social_id {2}",
                d->GetLogin(), p.index, p.private_code);

    if (p.index >= PLAYER_PER_ACCOUNT)
    {
        SPDLOG_ERROR("PlayerDelete: index overflow {0}, login: {1}",
                     p.index, d->GetLogin());
        return;
    }

    if (!c_rAccountTable.players[p.index].id || d->GetSocialId() != p.private_code)
    {
        SPDLOG_ERROR("PlayerDelete: Wrong Social ID index {0}, login: {1}",
                     p.index, d->GetLogin());
        BlankPacket pack; 
        d->Send(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID, pack);
        return;
    }

    GdPlayerDeletePacket p2;
    p2.pid = c_rAccountTable.players[p.index].id;
    storm::CopyStringSafe(p2.name, c_rAccountTable.players[p.index].name);
    db_clientdesc->DBPacket(HEADER_GD_PLAYER_DELETE, d->GetHandle(), &p2, sizeof(p2));
}

#pragma pack(1)
typedef struct SPacketGTLogin
{
    uint8_t header;
    uint16_t empty;
    uint32_t id;
} TPacketGTLogin;
#pragma pack()

void ItemEquipCheck(CHARACTER *ch)
{
    const auto weapon = ch->GetWear(WEAR_WEAPON);
    const auto body = ch->GetWear(WEAR_BODY);

    const auto weaponCostume = ch->GetWear(WEAR_COSTUME_WEAPON);

    const auto weaponCostumeEffect = ch->GetWear(WEAR_COSTUME_WEAPON_EFFECT);
    const auto bodyCostumeEffect = ch->GetWear(WEAR_COSTUME_BODY_EFFECT);

    if (weaponCostume)
    {
        if (!weapon || weaponCostume->GetValue(ITEM_VALUE_COSTUME_WEAPON_TYPE) != weapon->GetSubType())
        {
            ch->SetWear(WEAR_COSTUME_WEAPON, nullptr);
            ch->AutoGiveItem(weaponCostume);
            ch->SetWear(WEAR_COSTUME_WEAPON_EFFECT, nullptr);
            ch->AutoGiveItem(weaponCostumeEffect);
        }
    }

    if (bodyCostumeEffect && bodyCostumeEffect->IsEquipped())
    {
        if (!body)
        {
            ch->SetWear(WEAR_COSTUME_BODY_EFFECT, nullptr);
            ch->AutoGiveItem(bodyCostumeEffect);
        }
    }

    if (weaponCostumeEffect && weaponCostumeEffect->IsEquipped())
    {
        if (!weapon)
        {
            ch->SetWear(WEAR_COSTUME_WEAPON_EFFECT, nullptr);
            ch->AutoGiveItem(weaponCostumeEffect);
        }
    }
}

void CInputLogin::Entergame(DESC *d, const TPacketCGEnterGame& p)
{
    CHARACTER *ch;
    if (!((ch = d->GetCharacter())))
    {
        d->DelayedDisconnect(0, "No character in EnterGame");
        return;
    }

    d->SetPhase(PHASE_GAME);
    auto& dngMgr = CDungeonManager::instance();

    CGuildManager::instance().LoginMember(ch);

    auto pos = ch->GetXYZ();
    ch->Show(ch->GetMapIndex(), pos.x, pos.y, pos.z);
    ch->ReviveInvisible(5);

    SECTREE_MANAGER::instance().SendNPCPosition(ch);

    LogManager::instance().DetailLoginLog(true, ch);
    SPDLOG_TRACE("ENTERGAME: {0} {1}x{2}x{3} {4} map_index {5}",
                ch->GetName(), ch->GetX(), ch->GetY(), ch->GetZ(),
                d->GetHostName(), ch->GetMapIndex());


    if (ch->GetHorseLevel() > 0)
        ch->EnterHorse();

    // 플레이시간 레코딩 시작
    ch->ResetPlayTime();

    // 자동 저장 이벤트 추가
    ch->StartSaveEvent();
    ch->StartRecoveryEvent();
    ch->StartAffectEvent();
    ch->StartCheckSpeedHackEvent();

    CPVPManager::instance().Connect(ch);
    CPVPManager::instance().SendList(d);
#if defined(WJ_COMBAT_ZONE)
	CCombatZoneManager::instance().OnLogin(ch);
#endif
    MessengerManager::instance().Login(ch->GetName());

    CPartyManager::instance().SetParty(ch);
    CGuildManager::instance().SendGuildWar(ch);

    building::CManager::instance().SendLandList(d, ch->GetMapIndex());

    marriage::CManager::instance().Login(ch);

    TPacketGCTime pt;
    pt.time = get_global_time();
    d->Send(HEADER_GC_TIME, pt);

    TPacketGCChannel p2;
    p2.channel = gConfig.channel;
    d->Send(HEADER_GC_CHANNEL, p2);

    _send_bonus_info(ch);

    // Enable Input
    TPacketGCInputState packetInputState;
    packetInputState.isEnable = 1;
    ch->GetDesc()->Send(HEADER_GC_INPUT_STATE, packetInputState);


#ifdef ENABLE_HYDRA_DUNGEON
    CHydraDungeonManager::instance().OnLogin(ch);
#endif

    if (ch->GetMapIndex() == 175) {
        if (!ch->FindAffect(AFFECT_NON_OBSERVER)) {
            ch->SetObserverMode(true);
        } else {
            ch->RemoveAffect(AFFECT_NON_OBSERVER);
        }
    }
#ifdef ENABLE_NEW_GUILD_WAR
    if (ch->GetMapIndex() >= 10000) {
        if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex())) {
            CWarMap* pMap = CWarMapManager::instance().Find(ch->GetMapIndex());
            if (pMap) {
                BYTE bIdx;
                if (ch->GetGuild() &&
                    pMap->GetTeamIndex(ch->GetGuild()->GetID(), bIdx)) {
                    // sys_err("[TEST]Current: %d Max: %d",
                    // pMap->GetCurrentPlayer(bIdx), pMap->GetMaxPlayer(bIdx));

                    if (pMap->GetCurrentPlayer(bIdx) + 1 >
                        quest::CQuestManager::instance().GetEventFlagBR(
                            "maxplayer", ch->GetGuild()->GetName())) {
                        ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "oyuncu limiti "
                                                             "dolu!");
                        ch->GoHome();
                        return;
                    }
                }
            }
            ch->SetWarMap(pMap);
        } else if (marriage::WeddingManager::instance().IsWeddingMap(
                       ch->GetMapIndex())) {
            if (const auto marriageMap =
                    marriage::WeddingManager::instance().Find(
                        ch->GetMapIndex());
                marriageMap)
                ch->SetWeddingMap(marriageMap.value());
        }
#ifdef ENABLE_MELEY_LAIR_DUNGEON
        else if (MeleyLair::CMgr::instance().IsMeleyMap(ch->GetMapIndex())) {
            MeleyLair::CMgr::instance().DungeonSet(ch);
        }
#endif
        else {
            ch->SetDungeon(dngMgr.FindByMapIndex<CDungeon>(ch->GetMapIndex()));
        }
    }
#else
    if (ch->GetMapIndex() >= 10000) {
        if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex())) {
            ch->SetWarMap(CWarMapManager::instance().Find(ch->GetMapIndex()));
        } else if (marriage::WeddingManager::instance().IsWeddingMap(
                       ch->GetMapIndex())) {
            if (const auto marriageMap =
                    marriage::WeddingManager::instance().Find(
                        ch->GetMapIndex());
                marriageMap)
                ch->SetWeddingMap(marriageMap.value());
        }
#ifdef ENABLE_MELEY_LAIR_DUNGEON
        else if (MeleyLair::CMgr::instance().IsMeleyMap(ch->GetMapIndex())) {
            MeleyLair::CMgr::instance().DungeonSet(ch);
        }
#endif
        else {
            ch->SetDungeon(dngMgr.FindByMapIndex<CDungeon>(ch->GetMapIndex()));
        }
    }
#endif
    else if (CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
    {
        int memberFlag = CArenaManager::instance().IsMember(ch->GetMapIndex(),
                                                            ch->GetPlayerID());
        if (memberFlag == MEMBER_NO || memberFlag == MEMBER_OBSERVER) {
            ch->SetObserverMode(true);
            ch->SetArenaObserverMode(true);
            if (CArenaManager::instance().RegisterObserverPtr(
                    ch, ch->GetMapIndex(), ch->GetX() / 100,
                    ch->GetY() / 100)) {
                SPDLOG_TRACE("ARENA : Observer add failed");
            }

            if (ch->IsHorseRiding() == true) {
                ch->StopRiding();
                ch->HorseSummon(false);
            }
        } else if (memberFlag == MEMBER_DUELIST) {
            TPacketGCDuelStart duelStart{};

            ch->GetDesc()->Send(HEADER_GC_DUEL_START, duelStart);

            if (ch->IsHorseRiding() == true) {
                ch->StopRiding();
                ch->HorseSummon(false);
            }

            auto pParty = ch->GetParty();
            if (pParty != nullptr) {
                if (pParty->GetMemberCount() == 2) {
                    CPartyManager::instance().DeleteParty(pParty);
                } else {
                    pParty->Quit(ch->GetPlayerID());
                }
            }
        }
    } else if (ch->GetMapIndex() == 113) {
        if (COXEventManager::instance().Enter(ch) == false) {
            if (ch->GetGMLevel() == GM_PLAYER)
                ch->WarpSet(EMPIRE_START_MAP(ch->GetEmpire()),
                            EMPIRE_START_X(ch->GetEmpire()),
                            EMPIRE_START_Y(ch->GetEmpire()));
        }
    } else {
        if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()) ||
            marriage::WeddingManager::instance().IsWeddingMap(
                ch->GetMapIndex())) {
            ch->WarpSet(EMPIRE_START_MAP(ch->GetEmpire()),
                        EMPIRE_START_X(ch->GetEmpire()),
                        EMPIRE_START_Y(ch->GetEmpire()));
        }
    }

    if (ch->IsGM() && ch->FindAffect(AFFECT_INVISIBILITY) &&
        !gConfig.testServer)
        ch->SetObserverMode(true);

    if (ch->IsGM())
        ch->SetArmada();

    MapLevelChecks(ch);

    if (ch->GetHorseLevel() > 0) {
        uint32_t pid = ch->GetPlayerID();

        if (pid > 0) {
            const char* horseName =
                CHorseNameManager::instance().GetHorseName(pid);
            if (!horseName || !*horseName)
                db_clientdesc->DBPacket(HEADER_GD_REQ_HORSE_NAME, 0, &pid,
                                        sizeof(uint32_t));
        }
    }

    if (ch->GetLevel() > 5 && ch->GetSkillGroup() == 0) {
        ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenSkillGroupSelect");
    }

    if (ch->IsGM() == true)
        ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");

#ifdef __OFFLINE_MESSAGE__
    ch->CheckOfflineMessage();
#endif

    if (ch->GetMapIndex() == 113) { ch->RemoveGoodAffect(); }

    if (IsPvmMap(ch->GetMapIndex()))
    {
        ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
        ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
    }

    ch->DelayedItemLoad();
    ch->PointsPacket();
    ch->SkillLevelPacket();

    SPDLOG_DEBUG("Quest Login Entergame: pid {0} name {1}",
                 ch->GetPlayerID(), ch->GetName());
    quest::CQuestManager::instance().Login(ch->GetPlayerID());

    ch->SendMessengerBlock();
    ch->SendBlockModeInfo();
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    ch->SendLeftOpeningTimePacket();
    COfflineShopManager::instance().LoginOwner(ch->GetPlayerID());
#endif

    ch->SendHuntingMissions();

    if (ch->GetGuild()) {
        ch->ChatPacket(CHAT_TYPE_COMMAND, "guild_war %d", ch->GetGuild()->UnderAnyWar() != 0 && !ch->GetWarMap());
        ch->ChatPacket(CHAT_TYPE_COMMAND, "warboard toggle|%d", ch->GetWarMap() != nullptr);
    }

    ItemEquipCheck(ch);

    for (int i = 0; i < PREMIUM_MAX_NUM; ++i)
    {
        int remain = ch->GetPremiumRemainSeconds(i);

        if (remain <= 0)
            continue;

        if (i + AFFECT_PREMIUM_START == AFFECT_VOTE_BONUS)
        { 
            if(!gConfig.IsVoteBuffDisableMap(ch->GetMapIndex())) {
                ch->AddAffect(AFFECT_VOTE_BONUS2, POINT_ATTBONUS_HUMAN, 15, remain, 0, true, true);
                ch->AddAffect(AFFECT_VOTE_BONUS2, POINT_ATTBONUS_MONSTER, 30, remain, 0, true, true);
            }

        }
        else
        {
            ch->AddAffect(AFFECT_PREMIUM_START + i,
                          POINT_NONE, 0,
                          remain, 0, true);
        }
        SPDLOG_TRACE("PREMIUM: {} type {} {}min", ch->GetName(), i, remain);
    }

    if(gConfig.IsVoteBuffDisableMap(ch->GetMapIndex())) 
    {
        ch->RemoveAffect(AFFECT_VOTE_BONUS);
        ch->RemoveAffect(AFFECT_VOTE_BONUS2);
        ch->RemoveAffect(AFFECT_VOTE_BONUS3);
        ch->RemoveAffect(AFFECT_VOTE_BONUS4);
    }

    bool showDungeonRejoin = false;
    if (quest::CQuestManager::instance().GetEventFlag("disable_dungeon_"
                                                      "reconnect") == 0 &&
        dngMgr.HasPlayerInfo(ch->GetPlayerID())) {
        const auto dIndex = dngMgr.GetPlayerInfo(ch->GetPlayerID());
        if (dIndex > 0 && dIndex != ch->GetMapIndex()) {

            if (dIndex != ch->GetMapIndex()) {
                showDungeonRejoin = true;
            }
        }
    }

    SendChatPacket(ch, CHAT_TYPE_COMMAND, fmt::format("ShowDungeonRejoin {}", static_cast<int>(showDungeonRejoin)));

    GmCharacterEnterPacket pEnter;
    pEnter.pid = ch->GetPlayerID();
    pEnter.aid = ch->GetDesc()->GetAid();
    pEnter.name = ch->GetName();
    pEnter.empire = ch->GetEmpire();
    pEnter.mapIndex = ch->GetMapIndex();
    pEnter.channel = gConfig.channel;
    pEnter.hwid = ch->GetDesc()->GetHWIDHash();
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmCharacterEnter, pEnter);
}

int CInputLogin::GuildSymbolUpload(DESC *d, const TPacketCGGuildSymbolUpload& p)
{
    int iSymbolSize = p.data.size();

    if (iSymbolSize <= 0 || iSymbolSize > 64 * 1024)
    {
        // 64k 보다 큰 길드 심볼은 올릴수없다
        // 접속을 끊고 무시
        d->DelayedDisconnect(0);
        return true;
    }

    // 땅을 소유하지 않은 길드인 경우.
    if (!gConfig.testServer && !building::CManager::instance().FindLandByGuild(p.guild_id))
    {
        d->DelayedDisconnect(0);
        return true;
    }


    CGuildMarkManager::instance().UploadSymbol(p.guild_id, iSymbolSize, p.data.data());
    CGuildMarkManager::instance().SaveSymbol(GUILD_SYMBOL_FILENAME);
    return true;
}

void CInputLogin::GuildSymbolCRC(DESC *d, const TPacketCGSymbolCRC& p)
{

    const CGuildMarkManager::TGuildSymbol *pkGS = CGuildMarkManager::instance().GetGuildSymbol(p.guild_id);

    if (!pkGS)
    {
        if (p.lastRequest)
        {
            // No symbol found but this was the last request so politely answer
            TPacketGCGuildSymbolData GCPacket;
            GCPacket.size = 0;
            GCPacket.guild_id = p.guild_id;
            d->Send(HEADER_GC_SYMBOL_DATA, GCPacket);
        }

        return;
    }

    SPDLOG_TRACE("  Server %u %u", pkGS->crc, pkGS->raw.size());

    if (pkGS->raw.size() != p.size || pkGS->crc != p.crc)
    {
        TPacketGCGuildSymbolData GCPacket;

        GCPacket.size = sizeof(GCPacket) + pkGS->raw.size();
        GCPacket.guild_id = p.guild_id;
        GCPacket.data = pkGS->raw;

        d->Send(HEADER_GC_SYMBOL_DATA, GCPacket);

        SPDLOG_TRACE("SendGuildSymbolHead %02X%02X%02X%02X Size %d",
                    pkGS->raw[0], pkGS->raw[1], pkGS->raw[2], pkGS->raw[3], pkGS->raw.size());
    }
}

void CInputLogin::GuildMarkUpload(DESC *d, TPacketCGMarkUpload p)
{
    CGuildManager &rkGuildMgr = CGuildManager::instance();
    CGuild *pkGuild;

    if (!(pkGuild = rkGuildMgr.FindGuild(p.gid)))
    {
        SPDLOG_ERROR("MARK_SERVER: GuildMarkUpload: no guild. gid {0}", p.gid);
        return;
    }

    if (pkGuild->GetLevel() < gConfig.markMinLevel)
    {
        SPDLOG_TRACE("MARK_SERVER: GuildMarkUpload: level < {0} ({1})",
                    gConfig.markMinLevel, pkGuild->GetLevel());
        return;
    }

    CGuildMarkManager &rkMarkMgr = CGuildMarkManager::instance();

    SPDLOG_TRACE("MARK_SERVER: GuildMarkUpload: gid {0}", p.gid);

    bool isEmpty = true;

    for (uint32_t iPixel = 0; iPixel < SGuildMark::SIZE; ++iPixel)
        if (*(((uint32_t *)p.image) + iPixel) != 0x00000000)
            isEmpty = false;

    if (isEmpty)
        rkMarkMgr.DeleteMark(p.gid);
    else
        rkMarkMgr.SaveMark(p.gid, p.image);
}

void CInputLogin::GuildMarkIDXList(DESC *d, const TPacketCGMarkIDXList &p)
{
    CGuildMarkManager &rkMarkMgr = CGuildMarkManager::instance();

    TPacketGCMarkIDXList p2;
    p2.indices = rkMarkMgr.CopyMarkIdx();
    d->Send(HEADER_GC_MARK_IDXLIST, p2);

    SPDLOG_TRACE("MARK_SERVER: GuildMarkIDXList {} bytes sent.", p.indices.size() * 8);
}

void CInputLogin::GuildMarkCRCList(DESC *d, const TPacketCGMarkCRCList& pCG)
{

    std::unordered_map<uint8_t, const SGuildMarkBlock *> mapDiffBlocks;
    CGuildMarkManager::instance().GetDiffBlocks(pCG.imgIdx, pCG.crclist, mapDiffBlocks);

	std::unordered_map<uint8_t, std::vector<uint8_t>> blocks;

    for (auto it = mapDiffBlocks.begin(); it != mapDiffBlocks.end(); ++it)
    {
        uint8_t posBlock = it->first;
        const SGuildMarkBlock &rkBlock = *it->second;
        blocks.emplace(posBlock, rkBlock.m_abCompBuf);
    }

    TPacketGCMarkBlock pGC;
    pGC.imgIdx = pCG.imgIdx;
    pGC.blocks = blocks;

    SPDLOG_TRACE("MARK_SERVER: Sending blocks. (imgIdx {0} diff {1})", pCG->imgIdx, mapDiffBlocks.size());


    d->Send(HEADER_GC_MARK_BLOCK, pGC);
}

bool CInputProcessor::Analyze(DESC* d, const PacketHeader& header, const boost::asio::const_buffer& data) {
    return true;
}

bool CInputLogin::Analyze(DESC *                            d, const PacketHeader & header,
                          const boost::asio::const_buffer & data)
{
    int iExtraLen = 0;

    switch (header.id)
    {
    case HEADER_CG_MARK_LOGIN:
        break;

    case HEADER_CG_PONG:
        Pong(d);
        break;

    case HEADER_CG_TEXT:
        iExtraLen = Text(d, ReadPacket<std::string>(data));
        break;

    case HEADER_CG_TIME_SYNC:
        Handshake(d, ReadPacket<TPacketCGHandshake>(data));
        break;

    case HEADER_CG_REQ_TIMESYNC:
        d->SendHandshake(get_dword_time(), 0);
        break;

    case HEADER_CG_KEY_LOGIN:
        LoginByKey(d, ReadPacket<CgKeyLoginPacket>(data));
        break;

    case HEADER_CG_CHARACTER_SELECT:
        CharacterSelect(d, ReadPacket<TPacketCGPlayerSelect>(data));
        break;

    case HEADER_CG_CHARACTER_CREATE:
        CharacterCreate(d, ReadPacket<TPacketCGPlayerCreate>(data));
        break;

    case HEADER_CG_CHARACTER_DELETE:
        CharacterDelete(d, ReadPacket<TPacketCGPlayerDelete>(data));
        break;

    case HEADER_CG_ENTERGAME:
        Entergame(d, ReadPacket<TPacketCGEnterGame>(data));
        break;

        ///////////////////////////////////////
        // Guild Mark
        /////////////////////////////////////
    case HEADER_CG_MARK_CRCLIST:
        GuildMarkCRCList(d, ReadPacket<TPacketCGMarkCRCList>(data));
        break;

    case HEADER_CG_MARK_IDXLIST:
        GuildMarkIDXList(d, ReadPacket<TPacketCGMarkIDXList>(data));
        break;

    case HEADER_CG_MARK_UPLOAD:
        GuildMarkUpload(d, ReadPacket<TPacketCGMarkUpload>(data));
        break;

        //////////////////////////////////////
        // Guild Symbol
        /////////////////////////////////////
    case HEADER_CG_GUILD_SYMBOL_UPLOAD:
        return GuildSymbolUpload(d, ReadPacket<TPacketCGGuildSymbolUpload>(data));

    case HEADER_CG_SYMBOL_CRC:
        GuildSymbolCRC(d, ReadPacket<TPacketCGSymbolCRC>(data));
        break;
        /////////////////////////////////////

    case HEADER_CG_HACK:
        break;

    case HEADER_CG_OFFLINE_SHOP: {
        return true;
    }

    break;
    case HEADER_CG_CHANGE_NAME:
        ChangeName(d, ReadPacket<CgChangeNamePacket>(data));
        break;
    case HEADER_CG_CHANGE_EMPIRE:
        ChangeEmpire(d, ReadPacket<CgChangeEmpirePacket>(data));
        break;

    default:
        return true;
    }

    return true;
}
