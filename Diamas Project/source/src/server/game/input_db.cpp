#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "constants.h"
#include "desc_manager.h"
#include "item.h"
#include "item_manager.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "shop_manager.h"
#include "skill.h"
#include "utils.h"
#include <game/GamePacket.hpp>

#include "GBanWord.h"
#include "GRefineManager.h"
#include "building.h"
#include "guild.h"
#include "guild_manager.h"
#include "party.h"
#include "priv_manager.h"
#include "start_position.h"
#include "wedding.h"

#include "log.h"

#include "ChatUtil.hpp"
#include "DbCacheSocket.hpp"
#include "ItemUtils.h"
#include "gm.h"
#include "horsename_manager.h"
#include "map_location.h"
#include "protocol.h"
#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
#include "EventsManager.h"
#endif

#include "dungeon_info.h"
#include "itemname_manager.h"
#include "messenger_manager.h"
#include "mob_manager.h"

#include <complex.h>
#ifdef __OFFLINE_SHOP__
#include "OfflineShop.h"
#include "OfflineShopManager.h"
#endif
#ifndef __NEW_GAMEMASTER_CONFIG__
extern void gm_insert(const char* name, uint8_t level);
extern uint8_t gm_get_level(const char* name, const char* host,
                            const char* account);
extern void gm_host_insert(const char* host);
#endif

namespace
{
bool AddItem(CHARACTER* ch, const TItemPos& pos, CItem* item)
{
    if (ch->GetItem(pos))
        return false;

    switch (pos.window_type) {
        case EQUIPMENT: {
            ch->SetWear(pos.cell, item);
            return true;
        }

        case SWITCHBOT: {
            ch->SetSwitchbotItem(pos.cell, item);
            return true;
        }

        default:
            item->AddToCharacter(ch, pos);
            break;
    }

    return true;
}

const char* LoadItems(CHARACTER* ch, const char* data)
{
    rmt_ScopedCPUSample(PlayerLoadItems, 0);

    uint32_t count;
	std::memcpy(&count, data, sizeof(count));
	data += sizeof(count);

	SPDLOG_INFO("ITEM_LOAD: COUNT {0} {1}", ch->GetName(), count);

    std::vector<CItem*> v;
    std::vector<TPlayerItem> items;
    items.reserve(count);

    TPlayerItem* p = (TPlayerItem*)data;
    for (uint32_t i = 0; i < count; ++i, ++p) {
        items.push_back(*p);
    }

    for (auto& itemData : items) {
        auto* item = ITEM_MANAGER::instance().CreateItem(
            itemData.data.vnum, itemData.data.count, itemData.id);
        if (!item) {
            SPDLOG_ERROR("cannot create item by vnum {} (name {} id {})",
                         itemData.data.vnum, ch->GetName(), itemData.id);
            continue;
        }

        item->SetSkipSave(true);
        item->SetSkipPacket(true);
        item->SetTransmutationVnum(itemData.data.transVnum);
        item->SetSealDate(itemData.data.nSealDate);
        item->SetSockets(itemData.data.sockets);
        item->SetAttributes(itemData.data.attrs);
        item->SetLastOwnerPid(itemData.owner);
        item->SetGMOwner(itemData.is_gm_owner);
        item->SetBlocked(itemData.is_blocked);
        item->SetPrivateShopPrice(itemData.price);

        const TItemPos pos(itemData.window, itemData.pos);

        if (AddItem(ch, pos, item)) {
            events::Item::OnLoad(ch, item);
            item->OnLoad();
        } else {
            SPDLOG_TRACE("ITEM_RESTORE: {0} {1}", ch->GetName(),
                         item->GetName());
            v.push_back(item);
        }

        item->SetSkipSave(false);
        item->SetSkipPacket(false);
    }

    for (int i = WEAR_MAX_NUM - 1; i >= 0; --i) {
        const auto& wear = ch->GetWear(i);
        if (!wear)
            continue;

        if (!CanEquipItem(ch, wear, nullptr, true, false)) {
            wear->RemoveFromCharacter();
            SPDLOG_TRACE("ITEM_RESTORE: {0} {1}", ch->GetName(),
                         item->GetName());
            v.push_back(wear);
        }
    }

    for (auto& item : v) {
        int pos = ch->GetEmptyInventory(item);

        if (pos < 0) {
            PIXEL_POSITION coord;
            coord.x = ch->GetX();
            coord.y = ch->GetY();

            item->AddToGround(ch->GetMapIndex(), coord);
            item->StartDestroyEvent(gConfig.itemGroundTimeLong);
            item->SetOwnership(ch, gConfig.itemOwnershipTimeLong);
        } else {
            item->SetSkipSave(true);
            item->AddToCharacter(ch, TItemPos(INVENTORY, pos));
            item->SetSkipSave(false);
        }
    }

    return data + sizeof(TPlayerItem) * count;
}

const char* LoadSwitchbotData(CHARACTER* ch, const char* data)
{
    uint32_t count;
    std::memcpy(&count, data, sizeof(count));
    data += sizeof(count);

    ch->LoadSwitchbotData(count, (const SwitchBotSlotData*)data);
    return data + sizeof(SwitchBotSlotData) * count;
}

const char* LoadAffects(CHARACTER* ch, const char* data)
{
    rmt_ScopedCPUSample(PlayerLoadAffects, 0);

    uint32_t count;
    std::memcpy(&count, data, sizeof(count));
    data += sizeof(count);

    ch->LoadAffect(count, (const AffectData*)data);
    return data + sizeof(AffectData) * count;
}

const char* LoadQuests(CHARACTER* ch, const char* data)
{
    rmt_ScopedCPUSample(PlayerLoadQuests, 0);

    uint32_t count;
    std::memcpy(&count, data, sizeof(count));
    data += sizeof(count);

    const auto* table = reinterpret_cast<const TQuestTable*>(data);

    if (count != 0) {
        if (ch->GetPlayerID() != table[0].dwPID) {
            SPDLOG_ERROR("PID differs {} {}", ch->GetPlayerID(),
                         table[0].dwPID);
            return data + sizeof(TQuestTable) * count;
        }
    }

    SPDLOG_TRACE("QUEST_LOAD: count {}", count);

    auto* pkPc = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
    if (!pkPc) {
        SPDLOG_ERROR("null quest::PC with id {}", table[0].dwPID);
        return data + sizeof(TQuestTable) * count;
    }

    if (pkPc->IsLoaded())
        return data + sizeof(TQuestTable) * count;

    for (unsigned int i = 0; i < count; ++i) {
        SPDLOG_TRACE("            {} {}", table[i].szName, table[i].lValue);
        pkPc->SetFlag(fmt::format("{}.{}", table[i].szName, table[i].szState),
                      table[i].lValue, true);
    }

    pkPc->SetLoaded();
    pkPc->Build();
    return data + sizeof(TQuestTable) * count;
}

const char* LoadMessengerBlock(CHARACTER* ch, const char* data)
{
    uint32_t count;
    std::memcpy(&count, data, sizeof(count));
    data += sizeof(count);

    ch->LoadMessengerBlock(reinterpret_cast<const ElementLoadBlockedPC*>(data),
                           count);
    return data + sizeof(ElementLoadBlockedPC) * count;
}

const char* LoadBattlePassMissions(CHARACTER* ch, const char* data)
{
    uint32_t count;
    std::memcpy(&count, data, sizeof(count));
    data += sizeof(count);

    ch->LoadBattlePass(count, (TPlayerBattlePassMission*)(data));
    return data + sizeof(TPlayerBattlePassMission) * count;
}

const char* LoadHuntingMissions(CHARACTER* ch, const char* data)
{
    uint32_t count;
    std::memcpy(&count, data, sizeof(count));
    data += sizeof(count);

    ch->LoadHuntingMissions(count, (TPlayerHuntingMission*)(data));
    return data + sizeof(TPlayerHuntingMission) * count;
}
const char* LoadDungeonInfos(CHARACTER* ch, const char* data)
{
    uint32_t count;
    std::memcpy(&count, data, sizeof(count));
    data += sizeof(count);

    std::vector<TPlayerDungeonInfo> dungeonInfo;
    TPlayerDungeonInfo* p = (TPlayerDungeonInfo*)data;

    for (uint32_t i = 0; i < count; ++i, ++p) {
        dungeonInfo.push_back(*p);
    }

    ch->SetDungeonInfoVec(dungeonInfo);
    ch->SetDungeonInfoLoaded();

    return data + sizeof(TPlayerDungeonInfo) * count;
}

} // namespace

bool GetServerLocation(TAccountTable& tab)
{
    bool found = false;
    std::string addr;
    for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i) {
        if (0 == tab.players[i].id)
            continue;

        found = true;
        if (!CMapLocation::instance().Get(tab.players[i].mapIndex, addr,
                                          tab.players[i].port)) {
            found = false;
            SPDLOG_WARN("Map not registered {0} mapindex {1} {2} x {3} empire "
                        "{4}",
                        tab.players[i].name, tab.players[i].mapIndex,
                        tab.players[i].x, tab.players[i].y,
                        tab.players[i].empire);

            tab.players[i].mapIndex = EMPIRE_START_MAP(tab.players[i].empire);
            tab.players[i].x = EMPIRE_START_X(tab.players[i].empire);
            tab.players[i].y = EMPIRE_START_Y(tab.players[i].empire);

            if (!CMapLocation::instance().Get(tab.players[i].mapIndex, addr,
                                              tab.players[i].port)) {
                SPDLOG_ERROR("Cannot place character in world map does not "
                             "exist mapindex {0} {1} x {2} (name {3})",
                             tab.players[i].mapIndex, tab.players[i].x,
                             tab.players[i].y, tab.players[i].name);
                continue;
            } else {
                storm::CopyStringSafe(tab.players[i].addr, addr);
            }
            found = true;
        } else {
            storm::CopyStringSafe(tab.players[i].addr, addr);
        }
    }

    return found;
}

void PlayerListResult(uint32_t handle, const DgPlayerListResultPacket& p)
{
    rmt_ScopedCPUSample(LePlayerListResult, 0);

    DESC* d = DESC_MANAGER::instance().FindByHandle(handle);
    if (!d) {
        SPDLOG_DEBUG("Warning: PlayerListResult - failed to find desc {0}",
                     handle);
        return;
    }

    if (p.failed) {
        SPDLOG_ERROR("PlayerList loading failed for {0}", handle);
        return;
    }

    TAccountTable tab = p.tab;
    const bool found = GetServerLocation(tab);
    d->SetAccountTable(tab);

    for (uint32_t i = 0; i != PLAYER_PER_ACCOUNT; ++i)
        SPDLOG_TRACE("Loaded {0}: {1} empire {2} addr {3}:{4}",
                     tab.players[i].id, tab.players[i].name,
                     tab.players[i].empire, tab.players[i].addr,
                     tab.players[i].port);

    d->SendLoginSuccessPacket();
    d->SetPhase(PHASE_SELECT);
}

void PlayerCreateFailure(DESC* d, uint8_t type)
{
    if (!d)
        return;

    GcCharacterCreateFailurePacket p = {};
    p.type = type;
    d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, p);
}

void PlayerCreateSuccess(DESC* d, const char* data)
{
    if (!d)
        return;

    auto* pPacketDB = (TPacketDGCreateSuccess*)data;

    if (pPacketDB->slot >= PLAYER_PER_ACCOUNT) {
        GcCharacterCreateFailurePacket p = {};
        p.type = GcCharacterCreateFailurePacket::kFullAccount;
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, p);
        return;
    }

    std::string addr;
    if (!CMapLocation::instance().Get(pPacketDB->player.mapIndex, addr,
                                      pPacketDB->player.port)) {
        SPDLOG_ERROR("InputDB::PlayerCreateSuccess: cannot find server for "
                     "mapindex {0} {1} x {2} (name {3})",
                     pPacketDB->player.mapIndex, pPacketDB->player.x,
                     pPacketDB->player.y, pPacketDB->player.name);
    }

    storm::CopyStringSafe(pPacketDB->player.addr, addr);

    auto& r_Tab = d->GetAccountTable();
    r_Tab.players[pPacketDB->slot] = pPacketDB->player;

    TPacketGCPlayerCreateSuccess pack;
    pack.slot = pPacketDB->slot;
    pack.player = pPacketDB->player;
    d->Send(HEADER_GC_CHARACTER_CREATE_SUCCESS, pack);

    TPlayerItem t{};
    t.window = INVENTORY;
    t.data.count = 1;
    t.owner = r_Tab.players[pPacketDB->slot].id;
    t.id = ITEM_MANAGER::instance().GetNewID();
    t.pos = 0;
    t.data.vnum = 50187;
    db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_SAVE, 0, sizeof(TPlayerItem));
    db_clientdesc->Packet(&t, sizeof(TPlayerItem));

    LogManager::instance().CharLog(pack.player.id, 0, 0, 0, "CREATE PLAYER", "",
                                   d->GetHostName());
}

void PlayerDeleteSuccess(DESC* d, const char* data)
{
    if (!d)
        return;

    uint32_t pid = *(uint32_t*)data;

    GcCharacterDeleteSuccessPacket p;
    for (uint32_t i = 0; i != PLAYER_PER_ACCOUNT; ++i) {
        if (d->GetAccountTable().players[i].id == pid)
            p.slot = i;
    }

    d->Send(HEADER_GC_CHARACTER_DELETE_SUCCESS, p);
    d->GetAccountTable().players[p.slot].id = 0;
}

void PlayerDeleteFail(DESC* d)
{
    if (!d)
        return;

    BlankPacket pack;
    d->Send(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID, pack);
}

void ChangeName(DESC* d, const char* data)
{
    if (!d)
        return;

    auto* p = (DgChangeNamePacket*)data;

 /* if (p->failed) {
        GcCharacterCreateFailurePacket pFail = {
            GcCharacterPhaseFailurePacket::kUsedName};
        d->Send(HEADER_GC_CHARACTER_CREATE_FAILURE, pFail);
        return;
    }
   */
    auto& r = d->GetAccountTable();
    if (!r.id)
        return;

    for (size_t i = 0; i != PLAYER_PER_ACCOUNT; ++i) {
        if (r.players[i].id == p->pid) {
            MessengerManager::instance().RemoveAllList(r.players[i].name);

            strlcpy(r.players[i].name, p->name, sizeof(r.players[i].name));
            r.players[i].bChangeName = 0;



            GcChangeNamePacket p2;
            p2.index = i;
            p2.name = p->name;
            d->Send(HEADER_GC_CHANGE_NAME, p2);
            break;
        }
    }

       
}

void ChangeEmpire(DESC* d, const char* data)
{
    if (!d)
        return;

    auto* p = (DgChangeEmpirePacket*)data;

    auto& r = d->GetAccountTable();
    if (!r.id)
        return;

    for (size_t i = 0; i != PLAYER_PER_ACCOUNT; ++i) {
        if (r.players[i].id == p->pid) {
            r.players[i].empire = p->empire;
            r.players[i].bChangeEmpire = 0;

            GcChangeEmpirePacket p2;
            p2.slot = i;
            p2.empire = p->empire;
            d->Send(HEADER_GC_CHANGE_EMPIRE, p2);
            break;
        }
    }
}

void PlayerLoad(DESC* d, const char* data)
{
    if (!d)
        return;

    rmt_ScopedCPUSample(PlayerLoad, 0);

    TPlayerTable* pTab = (TPlayerTable*)data;
    data += sizeof(TPlayerTable);

    int32_t lMapIndex = pTab->lMapIndex;
    PIXEL_POSITION pos;

    int32_t lPublicMapIndex =
        lMapIndex >= 10000 ? lMapIndex / 10000 : lMapIndex;
    if (lMapIndex == 0 || !gConfig.IsHostingMap(lPublicMapIndex)) {
        lMapIndex = EMPIRE_START_MAP(pTab->empire);
        pos.x = EMPIRE_START_X(pTab->empire);
        pos.y = EMPIRE_START_Y(pTab->empire);
    }

    pTab->lMapIndex = lMapIndex;

    if (!SECTREE_MANAGER::instance().GetValidLocation(
            pTab->lMapIndex, pTab->x, pTab->y, lMapIndex, pos, pTab->empire)) {
        SPDLOG_ERROR("InputDB::PlayerLoad : cannot find valid location {} {} x "
                     "{} (name: {})",
                     pTab->lMapIndex, pTab->x, pTab->y, pTab->name);
        d->SetDisconnectReason("Invalid location");
        d->Disconnect();

        return;
    }

    if (!SECTREE_MANAGER::instance().GetMovablePosition(pTab->lMapIndex, pos.x,
                                                        pos.y, pos)) {
        PIXEL_POSITION pos2;
        SECTREE_MANAGER::instance().GetRecallPositionByEmpire(
            pTab->lMapIndex, pTab->empire, pos2);
        SPDLOG_WARN("Character spawned on non blocked Position setting to "
                    "empire recall Position (name {} {}x{} map {} changed to "
                    "{}x{})",
                    pTab->name, pos.x, pos.y, pTab->lMapIndex, pos2.x, pos2.y);
        pos = pos2;
    }

    pTab->x = pos.x;
    pTab->y = pos.y;
    pTab->lMapIndex = lMapIndex;
    pTab->last_play = get_global_time();

    if (d->GetCharacter() || d->IsPhase(PHASE_GAME)) {
        CHARACTER* p = d->GetCharacter();
        SPDLOG_ERROR("InputDB: PlayerLoad: Player is already in game phase "
                     "(Character {} {:p})",
                     p->GetName(), fmt::ptr(p));
        return;
    }

    if (nullptr != g_pCharManager->FindPC(pTab->name)) {
        SPDLOG_ERROR("InputDB: PlayerLoad : {} already exist in game",
                     pTab->name);
        return;
    }

    auto* ch = g_pCharManager->CreateCharacter(pTab->name, pTab->id);

    ch->BindDesc(d);
    ch->SetPlayerProto(pTab);
    ch->SetClientLocale(d->GetClientLocale());

    d->BindCharacter(ch);

    {
        LogManager::instance().CharLog(
            ch, 0, "LOGIN",
            fmt::format("{} {} {} {} {}", ch->GetDesc()->GetHostName().c_str(),
                        ch->GetGold(), gConfig.channel, ch->GetMapIndex(),
                        ch->GetAlignment())
                .c_str());
    }

    d->SetPhase(PHASE_LOADING);
    ch->MainCharacterPacket();

    ch->SetSkipSendPoint(true);
    ch->UpdateChatFilter();

    for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
        ch->SetQuickslot(i, pTab->quickslot[i]);

    data = LoadItems(ch, data);
    data = LoadQuests(ch, data);
    data = LoadAffects(ch, data);
    data = LoadSwitchbotData(ch, data);
    data = LoadMessengerBlock(ch, data);
#ifdef ENABLE_BATTLE_PASS
    data = LoadBattlePassMissions(ch, data);
#endif
    data = LoadHuntingMissions(ch, data);
    data = LoadDungeonInfos(ch, data);

    quest::CQuestManager::instance().BroadcastEventFlagOnLogin(ch);

    ch->CheckMaximumPoints();
    ch->SetSkipSendPoint(false);

    SPDLOG_TRACE("InputDB: player_load {} {}x{}x{} LEVEL {} MOV_SPEED {} JOB "
                 "{} ATG {} DFG {} GMLv {}",
                 pTab->name, ch->GetX(), ch->GetY(), ch->GetZ(), ch->GetLevel(),
                 ch->GetPoint(POINT_MOV_SPEED), ch->GetJob(),
                 ch->GetPoint(POINT_ATT_GRADE), ch->GetPoint(POINT_DEF_GRADE),
                 ch->GetGMLevel());
}

void Boot(const char* data)
{
    // 패킷 사이즈 체크
    uint32_t dwPacketSize = decode_4bytes(data);
    data += 4;

    // 패킷 버전 체크
    uint8_t bVersion = decode_byte(data);
    data += 1;

    SPDLOG_TRACE("BOOT: PACKET: {}", dwPacketSize);
    SPDLOG_TRACE("BOOT: VERSION: {}", bVersion);
    if (bVersion != 6) {
        SPDLOG_ERROR("boot version error");
        thecore_shutdown();
    }

    SPDLOG_TRACE("sizeof(TBanwordTable) = {}", sizeof(TBanwordTable));
    SPDLOG_TRACE("sizeof(TLand) = {}", sizeof(TLand));
    SPDLOG_TRACE("sizeof(TObject) = {}", sizeof(TObject));
    // ADMIN_MANAGER
    SPDLOG_TRACE("sizeof(TAdminManager) = {}", sizeof(TAdminInfo));
    // END_ADMIN_MANAGER
#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
    data += 2;
    int eventsize = decode_2bytes(data);
    data += 2;

    for (int n = 0; n < eventsize; ++n) {
        TEventInfo& rEventInfo = *(TEventInfo*)data;

        CEventsManager::instance().InsertEvent(rEventInfo);
        data += sizeof(rEventInfo);
    }
    /*
     * Start the Event timer to check Events.
     */
    CEventsManager::instance().StartEventsTimer();
#endif
    uint16_t size;

    /*
     * BANWORDS
     */

    if (decode_2bytes(data) != sizeof(TBanwordTable)) {
        SPDLOG_ERROR("ban word table size error");
        thecore_shutdown();
        return;
    }
    data += 2;

    size = decode_2bytes(data);
    data += 2;

    if (size) {
        CBanwordManager::instance().Initialize((TBanwordTable*)data, size);
        data += size * sizeof(TBanwordTable);
    }
    SPDLOG_TRACE("[BANWORD] Size {} Count {}", sizeof(TBanwordTable), size);

    {
        using namespace building;

        /*
         * LANDS
         */

        if (decode_2bytes(data) != sizeof(TLand)) {
            SPDLOG_ERROR("land table size error");
            thecore_shutdown();
            return;
        }
        data += 2;

        size = decode_2bytes(data);
        data += 2;

        if (size) {
            TLand* kLand = (TLand*)data;
            data += size * sizeof(TLand);

            for (uint16_t i = 0; i < size; ++i, ++kLand)
                CManager::instance().LoadLand(kLand);
        }

        SPDLOG_TRACE("[LAND] Size {} Count {}", sizeof(TLand), size);

        /*
         * OBJECT
         */
        if (decode_2bytes(data) != sizeof(TObject)) {
            SPDLOG_ERROR("object table size error");
            thecore_shutdown();
            return;
        }
        data += 2;

        size = decode_2bytes(data);
        data += 2;

        TObject* kObj = (TObject*)data;
        data += size * sizeof(TObject);

        for (uint16_t i = 0; i < size; ++i, ++kObj) {
            if (!gConfig.IsHostingMap(kObj->mapIndex))
                continue;

            CManager::instance().LoadObject(kObj, true);
        }
    }

    {
        /*
         * Dungeon ranking
         */
        if (decode_2bytes(data) != sizeof(TDungeonRankSet)) {
            SPDLOG_CRITICAL("dungeon rank size error");
            thecore_shutdown();
            return;
        }
        data += 2;

        size = decode_2bytes(data);
        data += 2;

        TDungeonRankSet* kRank = (TDungeonRankSet*)data;
        data += size * sizeof(TDungeonRankSet);

        CDungeonInfo::instance().SetInfoLoading(true);
        CDungeonInfo::instance().ClearInfoMap();

        for (int i = 0; i < size; ++i, ++kRank) {
            CDungeonInfo::instance().AppendRankSet(kRank);
        }

        CDungeonInfo::instance().SetInfoLoading(false);
    }

    set_global_time(*(time_t*)data);
    data += sizeof(time_t);

    uint16_t sizeOfTable;
    memcpy(&sizeOfTable, data, sizeof(sizeOfTable));
    data += sizeof(sizeOfTable);

    if (sizeOfTable != sizeof(ItemIdRange)) {
        SPDLOG_ERROR("ITEM ID RANGE size error");
        thecore_shutdown();
        return;
    }

    memcpy(&size, data, sizeof(size));
    data += sizeof(size);

    const auto* const range = (const ItemIdRange*)data;
    data += size * sizeof(ItemIdRange);

    const auto* const rangespare = (const ItemIdRange*)data;
    data += size * sizeof(ItemIdRange);

    // ADMIN_MANAGER
    data += 2;
    int32_t adminsize = decode_2bytes(data);
    data += 2;

    for (int32_t n = 0; n < adminsize; ++n) {
        tAdminInfo& rAdminInfo = *(tAdminInfo*)data;

#ifdef __NEW_GAMEMASTER_CONFIG__
        GM::insert(rAdminInfo);
#else
        gm_new_insert(rAdminInfo);
#endif

        data += sizeof(rAdminInfo);
    }

    GM::init((uint32_t*)data);
    data += sizeof(uint32_t) * GM_MAX_NUM;
    // END_ADMIN_MANAGER

#ifdef __OFFLINE_SHOP__
    if (sizeof(TPacketDGOfflineShopCreate) != decode_2bytes(data)) {
        SPDLOG_ERROR("The offline shop table for transfering informations from "
                     "database core to game core is not equivalent!");
        return;
    }
    data += sizeof(uint16_t);

    if (sizeof(TOfflineShopItemData) != decode_2bytes(data)) {
        SPDLOG_ERROR("The offline shop item table for transfering informations "
                     "from database core to game core is not "
                     "equivalent!");
        return;
    }
    data += sizeof(uint16_t);

    uint32_t dwOfflineShopCount = (*((uint32_t*)data));
    data += sizeof(uint32_t);

#ifdef _DEBUG
    SPDLOG_DEBUG("CInputDB::Boot :: Loading {} offline shops",
                 dwOfflineShopCount);
#endif

    COfflineShopManager::TCreationDataVector vecOfflineShops;

    while (dwOfflineShopCount > 0) {
        dwOfflineShopCount--;

        TPacketDGOfflineShopCreate kPacket =
            (*((TPacketDGOfflineShopCreate*)data));
        data += sizeof(TPacketDGOfflineShopCreate);

        uint32_t dwOfflineShopItemCount = (*((uint32_t*)data));
        data += sizeof(uint32_t);

        std::vector<TOfflineShopItemData> vecItems;
        while (dwOfflineShopItemCount > 0) {
            TOfflineShopItemData kItem = (*((TOfflineShopItemData*)data));
            data += sizeof(TOfflineShopItemData);

            vecItems.push_back(kItem);

            dwOfflineShopItemCount--;
        }

        vecOfflineShops.emplace_back(kPacket.kShopData, vecItems);
    }
#endif

    uint16_t endCheck = decode_2bytes(data);
    if (endCheck != 0xffff) {
        SPDLOG_ERROR("boot packet end check error [{}]!=0xffff", endCheck);
        thecore_shutdown();
        return;
    } else
        SPDLOG_TRACE("boot packet end check ok [{}]==0xffff", endCheck);
    data += 2;

    if (!ITEM_MANAGER::instance().SetMaxItemID(*range)) {
        SPDLOG_ERROR("not enough item id contact your administrator!");
        thecore_shutdown();
        return;
    }

    if (!ITEM_MANAGER::instance().SetMaxSpareItemID(*rangespare)) {
        SPDLOG_ERROR("not enough item id for spare contact your "
                     "administrator!");
        thecore_shutdown();
        return;
    }

    // LOCALE_SERVICE

    building::CManager::instance().FinalizeBoot();

#ifdef __OFFLINE_SHOP__
    COfflineShopManager::instance().CreateOfflineShops(vecOfflineShops);
    COfflineShopManager::instance().OpenAllShops();
#endif

    SPDLOG_INFO("Boot Complete");
}

void SafeboxLoad(DESC* d, const char* c_pData)
{
    if (!d)
        return;

    TSafeboxTable* p = (TSafeboxTable*)c_pData;

    if (d->GetAccountTable().id != p->dwID) {
        SPDLOG_ERROR("SafeboxLoad: safebox has different id {} != {}",
                     d->GetAccountTable().id, p->dwID);
        return;
    }

    if (!d->GetCharacter())
        return;

    CHARACTER* ch = d->GetCharacter();

    // PREVENT_TRADE_WINDOW
    if (ch->GetShopOwner() || ch->GetExchange() || ch->IsCubeOpen()) {
        SendI18nChatPacket(d->GetCharacter(), CHAT_TYPE_INFO,
                           "다른거래창이 열린상태에서는 창고를 열수가 "
                           "없습니다.");
        d->GetCharacter()->CancelSafeboxLoad();
        return;
    }
    // END_PREVENT_TRADE_WINDOW

    d->GetCharacter()->LoadSafebox(
        15, p->dwGold, p->wItemCount,
        (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
}

void SafeboxChangeSize(DESC* d, const char* c_pData)
{
    if (!d)
        return;

    uint8_t bSize = *(uint8_t*)c_pData;

    if (!d->GetCharacter())
        return;

    d->GetCharacter()->ChangeSafeboxSize(bSize);
}

//
// @version	05/06/20 Bang2ni - ReqSafeboxLoad 의 취소
//
void SafeboxWrongPassword(DESC* d)
{
    if (!d)
        return;

    if (!d->GetCharacter())
        return;

    TPacketCGSafeboxWrongPassword p;
    d->Send(HEADER_GC_SAFEBOX_WRONG_PASSWORD, p);

    d->GetCharacter()->CancelSafeboxLoad();
}

void SafeboxChangePasswordAnswer(DESC* d, const char* c_pData)
{
    if (!d)
        return;

    if (!d->GetCharacter())
        return;

    TSafeboxChangePasswordPacketAnswer* p =
        (TSafeboxChangePasswordPacketAnswer*)c_pData;
    if (p->flag) {
        SendI18nChatPacket(d->GetCharacter(), CHAT_TYPE_INFO,
                           "<창고> 창고 비밀번호가 변경되었습니다.");
    } else {
        SendI18nChatPacket(d->GetCharacter(), CHAT_TYPE_INFO,
                           "<창고> 기존 비밀번호가 틀렸습니다.");
    }
}

void MallLoad(DESC* d, const char* c_pData)
{
    if (!d)
        return;

    TSafeboxTable* p = (TSafeboxTable*)c_pData;

    if (d->GetAccountTable().id != p->dwID) {
        SPDLOG_ERROR("safebox has different id {} != {}",
                     d->GetAccountTable().id, p->dwID);
        return;
    }

    if (!d->GetCharacter())
        return;

    d->GetCharacter()->LoadMall(
        p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
}

void GuildLoad(const char* c_pData)
{
    CGuildManager::instance().LoadGuild(*(uint32_t*)c_pData);
}

void GuildSkillUpdate(const char* c_pData)
{
    TPacketGuildSkillUpdate* p = (TPacketGuildSkillUpdate*)c_pData;

    CGuild* g = CGuildManager::instance().TouchGuild(p->guild_id);

    if (g) {
        g->UpdateSkill(p->skill_point, p->skill_levels);
        g->GuildPointChange(POINT_SP, p->amount, p->save ? true : false);
    }
}

void GuildWar(const char* c_pData)
{
    TPacketGuildWar* p = (TPacketGuildWar*)c_pData;

    SPDLOG_TRACE("InputDB::GuildWar {} {} state {}", p->dwGuildFrom,
                 p->dwGuildTo, p->bWar);

    switch (p->bWar) {
        case GUILD_WAR_SEND_DECLARE:
        case GUILD_WAR_RECV_DECLARE:
            CGuildManager::instance().DeclareWar(p->dwGuildFrom, p->dwGuildTo,
                                                 p->bType);
            break;

        case GUILD_WAR_REFUSE:
            CGuildManager::instance().RefuseWar(p->dwGuildFrom, p->dwGuildTo);
            break;

        case GUILD_WAR_WAIT_START:
            CGuildManager::instance().WaitStartWar(p->dwGuildFrom,
                                                   p->dwGuildTo);
            break;

        case GUILD_WAR_CANCEL:
            CGuildManager::instance().CancelWar(p->dwGuildFrom, p->dwGuildTo);
            break;

        case GUILD_WAR_ON_WAR:
            CGuildManager::instance().StartWar(p->dwGuildFrom, p->dwGuildTo);
            break;

        case GUILD_WAR_OVER:
            CGuildManager::instance().WarOver(p->dwGuildFrom, p->dwGuildTo,
                                              p->bType);
            break;

        case GUILD_WAR_END:
            CGuildManager::instance().EndWar(p->dwGuildFrom, p->dwGuildTo);
            break;

        case GUILD_WAR_RESERVE:
            CGuildManager::instance().ReserveWar(p->dwGuildFrom, p->dwGuildTo,
                                                 p->bType);
            break;

        default:
            SPDLOG_ERROR("Unknown guild war state");
            break;
    }
}

void GuildWarScore(const char* c_pData)
{
    TPacketGuildWarScore* p = (TPacketGuildWarScore*)c_pData;
    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuildGainPoint);
    g->SetWarScoreAgainstTo(p->dwGuildOpponent, p->lScore);
}

void GuildSkillRecharge()
{
    CGuildManager::instance().SkillRecharge();
}

void GuildExpUpdate(const char* c_pData)
{
    TPacketGuildSkillUpdate* p = (TPacketGuildSkillUpdate*)c_pData;
    SPDLOG_TRACE("GuildExpUpdate {}", p->amount);

    CGuild* g = CGuildManager::instance().TouchGuild(p->guild_id);

    if (g)
        g->GuildPointChange(POINT_EXP, p->amount);
}

void GuildAddMember(const char* c_pData)
{
    TPacketDGGuildMember* p = (TPacketDGGuildMember*)c_pData;
    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

    if (g)
        g->AddMember(p);
}

void GuildRemoveMember(const char* c_pData)
{
    TPacketGuild* p = (TPacketGuild*)c_pData;
    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

    if (g)
        g->RemoveMember(p->dwInfo);
}

void GuildChangeGrade(const char* c_pData)
{
    TPacketGuild* p = (TPacketGuild*)c_pData;
    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

    if (g)
        g->P2PChangeGrade((uint8_t)p->dwInfo);
}

void GuildChangeMemberData(const char* c_pData)
{
    SPDLOG_TRACE("Recv GuildChangeMemberData");
    TPacketGuildChangeMemberData* p = (TPacketGuildChangeMemberData*)c_pData;
    CGuild* g = CGuildManager::instance().TouchGuild(p->guild_id);

    if (g)
        g->ChangeMemberData(p->pid, p->offer, p->level, p->grade);
}

void GuildDisband(const char* c_pData)
{
    TPacketGuild* p = (TPacketGuild*)c_pData;
    CGuildManager::instance().DisbandGuild(p->dwGuild);
}

void GuildLadder(const char* c_pData)
{
    TPacketGuildLadder* p = (TPacketGuildLadder*)c_pData;
    SPDLOG_TRACE("Recv GuildLadder {0} {1} / w {2} d {3} l {4}", p->dwGuild,
                 p->lLadderPoint, p->lWin, p->lDraw, p->lLoss);
    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

    g->SetLadderPoint(p->lLadderPoint);
    g->SetWarData(p->lWin, p->lDraw, p->lLoss);
}

#ifdef ENABLE_GUILD_STORAGE
void GuildStorageMoveItem(const char* c_pData)
{
    TPacketGuildStorageMoveItem* p = (TPacketGuildStorageMoveItem*)c_pData;
    CGuild* guild =
        CGuildManager::instance().FindGuild(p->guildID); // Search for guild
    if (guild) {
        guild->GetStorage()->moveItemFromDB(p->dwSrcPos, p->dwDestPos, p->item,
                                            p->iTick, p->playerId);
    } else {
        SPDLOG_ERROR("[GuildStorage] Cannot move item, guild {} (Guild not "
                     "exists).",
                     p->guildID);
    }
}

void GuildStorageRemoveItemSync(const char* c_pData)
{
    TPacketGuildStorageRemoveSyncItem* p =
        (TPacketGuildStorageRemoveSyncItem*)c_pData;
    CGuild* guild =
        CGuildManager::instance().FindGuild(p->guildID); // Search for guild
    if (guild) {
        guild->GetStorage()->RemoveSyncItemFromDB(p->pos);
    } else {
        SPDLOG_ERROR("[GuildStorage] Cannot erase que to guild {} (Guild not "
                     "exists).",
                     p->guildID);
    }
}

void GuildStorageRemoveItem(const char* c_pData)
{
    TPacketGuildStorageRemoveItemReturn* p =
        (TPacketGuildStorageRemoveItemReturn*)c_pData;
    CGuild* guild =
        CGuildManager::instance().FindGuild(p->guildID); // Search for guild
    if (guild) {
        guild->GetStorage()->RemoveItemFromDB(p->queItem, p->queID, p->wPos);
    } else {
        SPDLOG_ERROR("[GuildStorage] Cannot erase que to guild {} (Guild not "
                     "exists).",
                     p->guildID);
    }
}

void GuildStorageEraseQue(const char* c_pData)
{
    TPacketGuildStorageEraseQue* p = (TPacketGuildStorageEraseQue*)c_pData;
    CGuild* guild =
        CGuildManager::instance().FindGuild(p->guildID); // Search for guild
    if (guild) {
        guild->GetStorage()->eraseQue(p->queID);
    } else {
        SPDLOG_ERROR("[GuildStorage] Cannot erase que to guild {} (Guild not "
                     "exists).",
                     p->guildID);
    }
}

void GuildStorageFirstResponseAddItem(const char* c_pData)
{
    TPacketGuildStorageAddItem* p = (TPacketGuildStorageAddItem*)c_pData;

    CGuild* guild =
        CGuildManager::instance().FindGuild(p->guildID); // Search for guild
    if (guild) {
        guild->GetStorage()->FirstResponeAddItemFromDB(p->queItem, p->queID,
                                                       p->wPos);
    } else {
        SPDLOG_ERROR("[GuildStorage] Cannot create first response to guild {} "
                     "(Guild not exists).",
                     p->guildID);
    }
}

void GuildStorageItem(const char* c_pData)
{
    TPacketGuildStorageItem* p = (TPacketGuildStorageItem*)c_pData;

    CGuild* guild =
        CGuildManager::instance().FindGuild(p->guildID); // Search for guild
    if (guild) {
        guild->GetStorage()->addItemFromDB(p->wPos, p->item, p->loadCount,
                                           p->iTick);
        SPDLOG_TRACE("[GuildStorage] Loaded item for guild {} with ID {}.",
                     p->guildID, p->item.id);
    } else {
        SPDLOG_ERROR("[GuildStorage] Cannot add item to guild {} (Guild not "
                     "exists).",
                     p->guildID);
    }
}
#endif

void DungeonRankingLoad(const char* c_pData)
{
    // Same way boot load them
    if (decode_2bytes(c_pData) != sizeof(TDungeonRankSet)) {
        SPDLOG_ERROR("DungeonRankingLoad size error!");
        return;
    }

    c_pData += sizeof(uint16_t);

    uint16_t rankCount = decode_2bytes(c_pData);
    c_pData += sizeof(uint16_t);

    SPDLOG_INFO("DUNGEON_RANKING_LOAD: COUNT {}", rankCount);

    TDungeonRankSet* kRank = (TDungeonRankSet*)c_pData;
    c_pData += rankCount * sizeof(TDungeonRankSet);

    CDungeonInfo::instance().SetInfoLoading(true);
    CDungeonInfo::instance().ClearInfoMap();

    for (int i = 0; i < rankCount; ++i, ++kRank) {
        CDungeonInfo::instance().AppendRankSet(kRank);
    }

    CDungeonInfo::instance().SetInfoLoading(false);
}

void PartyCreate(const char* c_pData)
{
    TPacketPartyCreate* p = (TPacketPartyCreate*)c_pData;
    CPartyManager::instance().P2PCreateParty(p->dwLeaderPID);
}

void PartyDelete(const char* c_pData)
{
    TPacketPartyDelete* p = (TPacketPartyDelete*)c_pData;
    CPartyManager::instance().P2PDeleteParty(p->dwLeaderPID);
}

void PartyAdd(const char* c_pData)
{
    TPacketPartyAdd* p = (TPacketPartyAdd*)c_pData;
    CPartyManager::instance().P2PJoinParty(p->dwLeaderPID, p->dwPID, p->bState);
}

void PartyRemove(const char* c_pData)
{
    TPacketPartyRemove* p = (TPacketPartyRemove*)c_pData;
    CPartyManager::instance().P2PQuitParty(p->dwPID);
}

void PartyStateChange(const char* c_pData)
{
    TPacketPartyStateChange* p = (TPacketPartyStateChange*)c_pData;
    auto* pParty = CPartyManager::instance().P2PCreateParty(p->dwLeaderPID);

    if (!pParty)
        return;

    pParty->SetRole(p->dwPID, p->bRole, p->bFlag);
}

void PartySetMemberLevel(const char* c_pData)
{
    TPacketPartySetMemberLevel* p = (TPacketPartySetMemberLevel*)c_pData;
    auto* pParty = CPartyManager::instance().P2PCreateParty(p->dwLeaderPID);

    if (!pParty)
        return;

    pParty->P2PSetMemberLevel(p->dwPID, p->bLevel);
}

void Time(const char* c_pData)
{
    set_global_time(*(time_t*)c_pData);
}

void ReloadProto(const char* c_pData)
{
    CSkillManager::instance().Initialize();
    ITEM_MANAGER::instance().ReloadItemProto();
    CMobManager::instance().ReloadMobProto();
    uint16_t wSize;

    /*
     * Banwords
     */

    wSize = decode_2bytes(c_pData);
    c_pData += sizeof(uint16_t);
    CBanwordManager::instance().Initialize((TBanwordTable*)c_pData, wSize);
    c_pData += sizeof(TBanwordTable) * wSize;

    g_pCharManager->for_each_pc([](CHARACTER* ch) {
        ch->ComputePoints();
        ch->ComputeMountPoints();
    });
}

void GuildSkillUsableChange(const char* c_pData)
{
    TPacketGuildSkillUsableChange* p = (TPacketGuildSkillUsableChange*)c_pData;

    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);

    g->SkillUsableChange(p->dwSkillVnum, p->bUsable ? true : false);
}

void ChangeEmpirePriv(const char* c_pData)
{
    TPacketDGChangeEmpirePriv* p = (TPacketDGChangeEmpirePriv*)c_pData;

    // ADD_EMPIRE_PRIV_TIME
    CPrivManager::instance().GiveEmpirePriv(p->empire, p->type, p->value,
                                            p->bLog, p->end_time_sec);
    // END_OF_ADD_EMPIRE_PRIV_TIME
}

/**
 * @version 05/06/08	Bang2ni - 지속시간 추가
 */
void ChangeGuildPriv(const char* c_pData)
{
    TPacketDGChangeGuildPriv* p = (TPacketDGChangeGuildPriv*)c_pData;

    // ADD_GUILD_PRIV_TIME
    CPrivManager::instance().GiveGuildPriv(p->guild_id, p->type, p->value,
                                           p->bLog, p->end_time_sec);
    // END_OF_ADD_GUILD_PRIV_TIME
}

void ChangeCharacterPriv(const char* c_pData)
{
    TPacketDGChangeCharacterPriv* p = (TPacketDGChangeCharacterPriv*)c_pData;
    CPrivManager::instance().GiveCharacterPriv(p->pid, p->type, p->value,
                                               p->bLog);
}

void MoneyLog(const char* c_pData)
{
    TPacketMoneyLog* p = (TPacketMoneyLog*)c_pData;

    if (p->type == 4) // QUEST_MONEY_LOG_SKIP
        return;

    if (gConfig.authServer)
        return;

    LogManager::instance().MoneyLog(p->type, p->vnum, p->gold);
}

#ifdef ENABLE_GEM_SYSTEM
void GemLog(const char* c_pData)
{
    TPacketGemLog* p = (TPacketGemLog*)c_pData;

    if (p->type == 4) // QUEST_MONEY_LOG_SKIP
        return;

    if (gConfig.authServer)
        return;

    LogManager::instance().GemLog(p->type, p->gem);
}
#endif

void GuildMoneyChange(const char* c_pData)
{
    TPacketDGGuildMoneyChange* p = (TPacketDGGuildMoneyChange*)c_pData;

    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);
    if (g) {
        g->RecvMoneyChange(p->iTotalGold);
    }
}

void GuildWithdrawMoney(const char* c_pData)
{
    TPacketDGGuildMoneyWithdraw* p = (TPacketDGGuildMoneyWithdraw*)c_pData;

    CGuild* g = CGuildManager::instance().TouchGuild(p->dwGuild);
    if (g) {
        g->RecvWithdrawMoneyGive(p->iChangeGold);
    }
}

void SetEventFlag(const char* c_pData)
{
    TPacketSetEventFlag* p = (TPacketSetEventFlag*)c_pData;
    quest::CQuestManager::instance().SetEventFlag(p->szFlagName, p->lValue);
}

void SetHwidFlag(const char* c_pData)
{
    TPacketSetHwidFlag* p = (TPacketSetHwidFlag*)c_pData;
    quest::CQuestManager::instance().SetHwidFlag(p->szHwid, p->szFlagName,
                                                 p->lValue);
}

void CreateObject(const char* c_pData)
{
    using namespace building;
    CManager::instance().LoadObject((TObject*)c_pData);
}

void UpdateObject(const char* c_pData)
{
    using namespace building;
    CManager::instance().UpdateObject((TObject*)c_pData);
}

void DeleteObject(const char* c_pData)
{
    using namespace building;
    CManager::instance().DeleteObject(*(uint32_t*)c_pData);
}

void UpdateLand(const char* c_pData)
{
    using namespace building;
    CManager::instance().UpdateLand((TLand*)c_pData);
}

void Notice(const char* c_pData)
{
    char szBuf[256 + 1];
    strlcpy(szBuf, c_pData, sizeof(szBuf));

    SPDLOG_TRACE("InputDB:: Notice: {}", szBuf);

    // SendNotice(LC_TEXT(szBuf));
    SendNotice(szBuf);
}

void GuildWarReserveAdd(TGuildWarReserve* p)
{
    CGuildManager::instance().ReserveWarAdd(p);
}

void GuildWarReserveDelete(uint32_t dwID)
{
    CGuildManager::instance().ReserveWarDelete(dwID);
}

void MarriageAdd(TPacketMarriageAdd* p)
{
    SPDLOG_TRACE("MarriageAdd {} {} {} {} {}", p->dwPID1, p->dwPID2,
                 (uint32_t)p->tMarryTime, p->szName1, p->szName2);
    marriage::CManager::instance().Add(p->dwPID1, p->dwPID2, p->tMarryTime,
                                       p->szName1, p->szName2);
}

void MarriageUpdate(TPacketMarriageUpdate* p)
{
    SPDLOG_TRACE("MarriageUpdate {} {} {} {}", p->dwPID1, p->dwPID2,
                 p->iLovePoint, p->byMarried);
    marriage::CManager::instance().Update(p->dwPID1, p->dwPID2, p->iLovePoint,
                                          p->byMarried);
}

void MarriageRemove(TPacketMarriageRemove* p)
{
    SPDLOG_TRACE("MarriageRemove {} {}", p->dwPID1, p->dwPID2);
    marriage::CManager::instance().Remove(p->dwPID1, p->dwPID2);
}

void WeddingRequest(TPacketWeddingRequest* p)
{
    marriage::WeddingManager::instance().Request(p->dwPID1, p->dwPID2);
}

void WeddingReady(TPacketWeddingReady* p)
{
    SPDLOG_TRACE("WeddingReady {} {} {}", p->dwPID1, p->dwPID2, p->dwMapIndex);
    marriage::CManager::instance().WeddingReady(p->dwPID1, p->dwPID2,
                                                p->dwMapIndex);
}

void WeddingStart(TPacketWeddingStart* p)
{
    SPDLOG_TRACE("WeddingStart {} {}", p->dwPID1, p->dwPID2);
    marriage::CManager::instance().WeddingStart(p->dwPID1, p->dwPID2);
}

void WeddingEnd(TPacketWeddingEnd* p)
{
    SPDLOG_TRACE("WeddingEnd {} {}", p->dwPID1, p->dwPID2);
    marriage::CManager::instance().WeddingEnd(p->dwPID1, p->dwPID2);
}

// MYSHOP_PRICE_LIST
void MyshopPricelistRes(DESC* d, const MyShopPriceListHeader* p)
{
    CHARACTER* ch;

    if (!d || !(ch = d->GetCharacter()))
        return;

    SPDLOG_DEBUG("RecvMyshopPricelistRes name[{}]", ch->GetName());
    ch->UseSilkBotaryReal(p);
}

// END_OF_MYSHOP_PRICE_LIST

// RELOAD_ADMIN
void ReloadAdmin(const char* c_pData)
{
    GM::clear();

    int size = decode_2bytes(c_pData);
    c_pData += 2;

    for (int n = 0; n < size; ++n) {
        tAdminInfo& rAdminInfo = *(tAdminInfo*)c_pData;

        GM::insert(rAdminInfo);

        c_pData += sizeof(tAdminInfo);

        CHARACTER* pChar = g_pCharManager->FindPC(rAdminInfo.m_szName);
        if (pChar) {
            pChar->SetGMLevel();
        }
    }

    GM::init((uint32_t*)c_pData);
    c_pData += sizeof(uint32_t) * GM_MAX_NUM;

    MessengerManager::instance().ReloadGameMasters();
}

// END_RELOAD_ADMIN

#ifdef __OFFLINE_SHOP__
void ReceiveOfflineShopLoadPacket(const char* data)
{
    TPacketDGOfflineShopCreate* pkPacket = ((TPacketDGOfflineShopCreate*)data);
    data += sizeof(TPacketDGOfflineShopCreate);

    uint32_t dwItemCount = decode_4bytes(data);
    data += sizeof(uint32_t);

    if (dwItemCount < 1 && pkPacket->kShopData.llGold < 1) {
        TPacketOfflineShopDestroy kPacket;
        kPacket.dwOwnerPID = pkPacket->kShopData.dwOwnerPID;

        db_clientdesc->DBPacket(HEADER_GD_OFFLINE_SHOP_DESTROY, 0, &kPacket,
                                sizeof(kPacket));
        return;
    }

    CHARACTER* pkOwner =
        g_pCharManager->FindByPID(pkPacket->kShopData.dwOwnerPID);
    if (!pkOwner) {
        SPDLOG_ERROR("Received packet even without the owner is on this core!");
        data += dwItemCount * sizeof(TOfflineShopItemData);
        return;
    }

    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().CreateOfflineShop(&pkPacket->kShopData);
    if (!pkOfflineShop) {
        SPDLOG_ERROR("Failed to load the offline shop for user %d!",
                     pkPacket->kShopData.dwOwnerPID);
        data += dwItemCount * sizeof(TOfflineShopItemData);
        return;
    }

    while (dwItemCount > 0) {
        TOfflineShopItemData* pkItem = ((TOfflineShopItemData*)data);
        data += sizeof(TOfflineShopItemData);

        pkOfflineShop->AddItem(pkItem);

        dwItemCount--;
    }

    pkOwner->SetMyOfflineShop(pkOfflineShop);
}

void ReceiveOfflineShopUpdateNamePacket(TPacketOfflineShopUpdateName* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        SPDLOG_ERROR("ReceiveOfflineShopUpdateNamePacket :: There's no shop "
                     "for user %d!",
                     pkPacket->dwOwnerPID);
        return;
    }

    pkOfflineShop->SetName(pkPacket->szName, true);
}

void ReceiveOfflineShopAddItemPacket(TPacketOfflineShopAddItem* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        SPDLOG_ERROR("ReceiveOfflineShopAddItemPacket :: There's no shop for "
                     "user %d!",
                     pkPacket->dwOwnerPID);
        return;
    }

    pkOfflineShop->AddItem(&(pkPacket->kItem));
}

void ReceiveOfflineShopMoveItemPacket(TPacketOfflineShopMoveItem* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        SPDLOG_ERROR("ReceiveOfflineShopMoveItemPacket :: There's no shop for "
                     "user %d!",
                     pkPacket->dwOwnerPID);
        return;
    }

    pkOfflineShop->MoveItem(pkPacket->dwOldPosition, pkPacket->dwNewPosition,
                            true);
}

void ReceiveOfflineShopRemoveItemPacket(TPacketOfflineShopRemoveItem* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        SPDLOG_ERROR("ReceiveOfflineShopRemoveItemPacket :: There's no shop "
                     "for user %d!",
                     pkPacket->dwOwnerPID);
        return;
    }

    pkOfflineShop->RemoveItem(pkPacket->dwPosition, true);
}

void ReceiveOfflineShopUpdateGoldPacket(TPacketOfflineShopUpdateGold* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        SPDLOG_ERROR("ReceiveOfflineShopUpdateGoldPacket :: There's no shop "
                     "for user {}!",
                     pkPacket->dwOwnerPID);
        return;
    }

    pkOfflineShop->SetGold(pkPacket->llGold, true);
}

void ReceiveOfflineShopDestroyPacket(TPacketOfflineShopDestroy* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        SPDLOG_ERROR("ReceiveOfflineShopDestroyPacket :: There's no shop for "
                     "user %d!",
                     pkPacket->dwOwnerPID);
        return;
    }

    COfflineShopManager::instance().DestroyOfflineShop(pkOfflineShop, true);
}

void ReceiveOfflineShopClosePacket(TPacketOfflineShopClose* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        return;
    }

    pkOfflineShop->CloseShop(true);
}

void ReceiveOfflineShopOpenPacket(TPacketOfflineShopOpen* pkPacket)
{
    COfflineShop* pkOfflineShop =
        COfflineShopManager::instance().FindOfflineShop(pkPacket->dwOwnerPID);
    if (!pkOfflineShop) {
        return;
    }

    pkOfflineShop->OpenShop();
}
#endif

#ifdef ENABLE_BATTLE_PASS

void BattlePassLoadRanking(DESC* d, const char* c_pData)
{
    // sys_err("BattlePassLoadRanking");
    if (!d || !d->GetCharacter())
        return;

    CHARACTER* ch = d->GetCharacter();
    if (!ch)
        return;

    uint32_t dwPID = decode_4bytes(c_pData);
    c_pData += sizeof(uint32_t);

    uint8_t bIsGlobal = decode_byte(c_pData);
    c_pData += sizeof(uint8_t);

    uint32_t dwCount = decode_4bytes(c_pData);
    c_pData += sizeof(uint32_t);

    // sys_err("BattlePassLoadRanking count %d playerid %d", dwCount, dwPID);

    if (ch->GetPlayerID() != dwPID)
        return;

    if (dwCount) {
        std::vector<TBattlePassRanking> sendVector;
        sendVector.resize(dwCount);

        TBattlePassRanking* p = (TBattlePassRanking*)c_pData;

        for (int i = 0; i < dwCount; ++i, ++p) {
            TBattlePassRanking newRanking;
            newRanking.bPos = p->bPos;
            strlcpy(newRanking.playerName, p->playerName,
                    sizeof(newRanking.playerName));
            newRanking.dwFinishTime = p->dwFinishTime;

            sendVector.push_back(newRanking);
        }

        if (!sendVector.empty()) {
            TPacketGCBattlePassRanking packet;
            packet.bIsGlobal = bIsGlobal;
            packet.elems = sendVector;

            ch->GetDesc()->Send(HEADER_GC_BATTLE_PASS_RANKING, packet);
        }
    } else {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                           "There is no one currently registered in the "
                           "ranking.");
    }
}
#endif

void GuildChangeMaster(TPacketChangeGuildMaster* p)
{
    CGuildManager::instance().ChangeMaster(p->dwGuildID);
}

#ifdef __DUNGEON_FOR_GUILD__
void GuildDungeon(const char* c_pData)
{
    TPacketDGGuildDungeon* sPacket = (TPacketDGGuildDungeon*)c_pData;
    CGuild* pkGuild = CGuildManager::instance().TouchGuild(sPacket->dwGuildID);
    if (pkGuild)
        pkGuild->RecvDungeon(sPacket->bChannel, sPacket->lMapIndex);
}

void GuildDungeonCD(const char* c_pData)
{
    TPacketDGGuildDungeonCD* sPacket = (TPacketDGGuildDungeonCD*)c_pData;
    CGuild* pkGuild = CGuildManager::instance().TouchGuild(sPacket->dwGuildID);
    if (pkGuild)
        pkGuild->RecvDungeonCD(sPacket->dwTime);
}
#endif

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
void AutoEvents(const char* c_pData)
{
    TPacketAutoEvents* p = (TPacketAutoEvents*)c_pData;
    CEventsManager::instance().P2P_SetEvent(p->dwID, p->bValue, p->bEnd);
}
#endif
void UpdateCash(DESC* d)
{
    if (!d)
        return;

    auto* ch = d->GetCharacter();
    if (!ch)
        return;

    TPacketGCPointChange pack;
    pack.dwVID = ch->GetVID();
    pack.type = POINT_CASH;
    pack.value = ch->GetCash("cash");
    pack.amount = 0;
    d->Send(HEADER_GC_CHARACTER_POINT_CHANGE, pack);
}

void ReceiveOfflineShopItemInfo(DESC* d, const char* data)
{
    if (!d || !d->GetCharacter())
        return;

    uint8_t found = *(uint8_t*)data;
    data += sizeof(uint8_t);

    if (!found) {
        return;
    }

    TPacketOfflineShopSendItemInfo itemInfo =
        *(TPacketOfflineShopSendItemInfo*)data;
    data += sizeof(TPacketOfflineShopSendItemInfo);

    auto ch = d->GetCharacter();
    ch->ShopSearch_HandleBuyItemFromDb(itemInfo);
}

void ReceiveOfflineShopBuyDone(DESC* d, const char* data)
{
    if (!d || !d->GetCharacter())
        return;

    uint8_t found = *(uint8_t*)data;
    data += sizeof(uint8_t);

    if (!found) {
        SendI18nChatPacket(d->GetCharacter(), CHAT_TYPE_INFO, "The item you want to buy was not found.");
        return;
    }

    TPacketOfflineShopSendItemInfo itemInfo =
        *(TPacketOfflineShopSendItemInfo*)data;
    data += sizeof(TPacketOfflineShopSendItemInfo);

    auto ch = d->GetCharacter();
    ch->ShopSearch_HandleBuyCompleteFromDb(itemInfo);
}

void ReceiveOfflineShopSearchResultPacket(DESC* d, const char* data)
{
    if (!d || !d->GetCharacter())
        return;

    uint32_t count = decode_4bytes(data);
    data += sizeof(uint32_t);

    if (!count) {
        return;
    }

    std::vector<ShopSearchItemData> items;
    items.reserve(count);

    const ShopSearchItemData* p = reinterpret_cast<const ShopSearchItemData *>(data);
    for (uint32_t i = 0; i < count; ++i, ++p)
        items.emplace_back(*p);

    data += sizeof(ShopSearchItemData) * count;

    d->GetCharacter()->ShopSearch_SetResults(items);
}
////////////////////////////////////////////////////////////////////
// Analyze
// @version	05/06/10 Bang2ni - 아이템 가격정보 리스트
// 패킷(HEADER_DG_MYSHOP_PRICELIST_RES) 처리루틴 추가.
////////////////////////////////////////////////////////////////////
bool HandleDbPacket(const DbPacketHeader& header, const char* data)
{
    switch (header.id) {
        case HEADER_DG_BOOT:
            Boot(data);
            break;

        case HEADER_DG_PLAYER_LIST_RESULT:
            PlayerListResult(header.handle,
                             *(const DgPlayerListResultPacket*)data);
            break;

        case HEADER_DG_PLAYER_LOAD_SUCCESS:
            PlayerLoad(DESC_MANAGER::instance().FindByHandle(header.handle),
                       data);
            break;

        case HEADER_DG_PLAYER_CREATE_SUCCESS:
            PlayerCreateSuccess(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;

        case HEADER_DG_PLAYER_CREATE_FAILED:
            PlayerCreateFailure(
                DESC_MANAGER::instance().FindByHandle(header.handle),
                *(uint32_t*)data);
            break;

        case HEADER_DG_PLAYER_DELETE_SUCCESS:
            PlayerDeleteSuccess(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;

        case HEADER_DG_PLAYER_LOAD_FAILED:
            DESC_MANAGER::instance()
                .FindByHandle(header.handle)
                ->SetDisconnectReason("Player load failed");

            DESC_MANAGER::instance().FindByHandle(header.handle)->Disconnect();
            SPDLOG_TRACE("PLAYER_LOAD_FAILED");
            break;

        case HEADER_DG_PLAYER_DELETE_FAILED:
            // SPDLOG_TRACE( "PLAYER_DELETE_FAILED");
            PlayerDeleteFail(
                DESC_MANAGER::instance().FindByHandle(header.handle));
            break;

        case HEADER_DG_SAFEBOX_LOAD:
            SafeboxLoad(DESC_MANAGER::instance().FindByHandle(header.handle),
                        data);
            break;

        case HEADER_DG_SAFEBOX_CHANGE_SIZE:
            SafeboxChangeSize(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;

        case HEADER_DG_SAFEBOX_WRONG_PASSWORD:
            SafeboxWrongPassword(
                DESC_MANAGER::instance().FindByHandle(header.handle));
            break;

        case HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER:
            SafeboxChangePasswordAnswer(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;

        case HEADER_DG_MALL_LOAD:
            MallLoad(DESC_MANAGER::instance().FindByHandle(header.handle),
                     data);
            break;

        case HEADER_DG_GUILD_SKILL_UPDATE:
            GuildSkillUpdate(data);
            break;

        case HEADER_DG_GUILD_LOAD:
            GuildLoad(data);
            break;

        case HEADER_DG_GUILD_SKILL_RECHARGE:
            GuildSkillRecharge();
            break;

        case HEADER_DG_GUILD_EXP_UPDATE:
            GuildExpUpdate(data);
            break;

#ifdef ENABLE_GUILD_STORAGE
        case HEADER_DG_GUILDSTORAGE_MOVE:
            GuildStorageMoveItem(data);
            break;

        case HEADER_DG_GUILDSTORAGE_ITEM:
            GuildStorageItem(data);
            break;

        case HEADER_DG_GUILDSTORAGE_ADD_ITEM_RESPONE_1:
            GuildStorageFirstResponseAddItem(data);
            break;

        case HEADER_DG_GUILDSTORAGE_DESTROY_QUE:
            GuildStorageEraseQue(data);
            break;

        case HEADER_DG_GUILDSTORAGE_REMOVE_SYNC_ITEM:
            GuildStorageRemoveItemSync(data);
            break;

        case HEADER_DG_GUILDSTORAGE_REMOVE_ITEM:
            GuildStorageRemoveItem(data);
            break;
#endif
        case HEADER_DG_PARTY_CREATE:
            PartyCreate(data);
            break;

        case HEADER_DG_DUNGEON_RANKING_LOAD:
            DungeonRankingLoad(data);
            break;

        case HEADER_DG_PARTY_DELETE:
            PartyDelete(data);
            break;

        case HEADER_DG_PARTY_ADD:
            PartyAdd(data);
            break;

        case HEADER_DG_PARTY_REMOVE:
            PartyRemove(data);
            break;

        case HEADER_DG_PARTY_STATE_CHANGE:
            PartyStateChange(data);
            break;

        case HEADER_DG_PARTY_SET_MEMBER_LEVEL:
            PartySetMemberLevel(data);
            break;

        case HEADER_DG_TIME:
            Time(data);
            break;

        case HEADER_DG_GUILD_ADD_MEMBER:
            GuildAddMember(data);
            break;

        case HEADER_DG_GUILD_REMOVE_MEMBER:
            GuildRemoveMember(data);
            break;

        case HEADER_DG_GUILD_CHANGE_GRADE:
            GuildChangeGrade(data);
            break;

        case HEADER_DG_GUILD_CHANGE_MEMBER_DATA:
            GuildChangeMemberData(data);
            break;

        case HEADER_DG_GUILD_DISBAND:
            GuildDisband(data);
            break;

        case HEADER_DG_RELOAD_PROTO:
            ReloadProto(data);
            break;

        case HEADER_DG_GUILD_WAR:
            GuildWar(data);
            break;

        case HEADER_DG_GUILD_WAR_SCORE:
            GuildWarScore(data);
            break;

        case HEADER_DG_GUILD_LADDER:
            GuildLadder(data);
            break;

        case HEADER_DG_GUILD_SKILL_USABLE_CHANGE:
            GuildSkillUsableChange(data);
            break;

        case HEADER_DG_CHANGE_NAME:
            ChangeName(DESC_MANAGER::instance().FindByHandle(header.handle),
                       data);
            break;

        case HEADER_DG_UPDATE_CASH:
            UpdateCash(DESC_MANAGER::instance().FindByHandle(header.handle));
            break;

        case HEADER_DG_CHANGE_EMPIRE:
            ChangeEmpire(DESC_MANAGER::instance().FindByHandle(header.handle),
                         data);
            break;

        case HEADER_DG_CHANGE_EMPIRE_PRIV:
            ChangeEmpirePriv(data);
            break;

        case HEADER_DG_CHANGE_GUILD_PRIV:
            ChangeGuildPriv(data);
            break;

        case HEADER_DG_CHANGE_CHARACTER_PRIV:
            ChangeCharacterPriv(data);
            break;

        case HEADER_DG_MONEY_LOG:
            MoneyLog(data);
            break;

        case HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE:
            GuildWithdrawMoney(data);
            break;

        case HEADER_DG_GUILD_MONEY_CHANGE:
            GuildMoneyChange(data);
            break;

        case HEADER_DG_SET_EVENT_FLAG:
            SetEventFlag(data);
            break;

        case HEADER_DG_SET_HWID_FLAG:
            SetHwidFlag(data);
            break;

        case HEADER_DG_CREATE_OBJECT:
            CreateObject(data);
            break;

        case HEADER_DG_UPDATE_OBJECT:
            UpdateObject(data);
            break;

        case HEADER_DG_DELETE_OBJECT:
            DeleteObject(data);
            break;

        case HEADER_DG_UPDATE_LAND:
            UpdateLand(data);
            break;

        case HEADER_DG_NOTICE:
            Notice(data);
            break;

        case HEADER_DG_GUILD_WAR_RESERVE_ADD:
            GuildWarReserveAdd((TGuildWarReserve*)data);
            break;

        case HEADER_DG_GUILD_WAR_RESERVE_DEL:
            GuildWarReserveDelete(*(uint32_t*)data);
            break;

        case HEADER_DG_MARRIAGE_ADD:
            MarriageAdd((TPacketMarriageAdd*)data);
            break;

        case HEADER_DG_MARRIAGE_UPDATE:
            MarriageUpdate((TPacketMarriageUpdate*)data);
            break;

        case HEADER_DG_MARRIAGE_REMOVE:
            MarriageRemove((TPacketMarriageRemove*)data);
            break;

        case HEADER_DG_WEDDING_REQUEST:
            WeddingRequest((TPacketWeddingRequest*)data);
            break;

        case HEADER_DG_WEDDING_READY:
            WeddingReady((TPacketWeddingReady*)data);
            break;

        case HEADER_DG_WEDDING_START:
            WeddingStart((TPacketWeddingStart*)data);
            break;

        case HEADER_DG_WEDDING_END:
            WeddingEnd((TPacketWeddingEnd*)data);
            break;

            // MYSHOP_PRICE_LIST
        case HEADER_DG_MYSHOP_PRICELIST_RES:
            MyshopPricelistRes(
                DESC_MANAGER::instance().FindByHandle(header.handle),
                (MyShopPriceListHeader*)data);
            break;
            // END_OF_MYSHOP_PRICE_LIST
            //
            // RELOAD_ADMIN
        case HEADER_DG_RELOAD_ADMIN:
            ReloadAdmin(data);
            break;
            // END_RELOAD_ADMIN

        case HEADER_DG_ACK_CHANGE_GUILD_MASTER:
            GuildChangeMaster((TPacketChangeGuildMaster*)data);
            break;
        case HEADER_DG_ACK_SPARE_ITEM_ID_RANGE:
            ITEM_MANAGER::instance().SetMaxSpareItemID(*((ItemIdRange*)data));
            break;

        case HEADER_DG_UPDATE_HORSE_NAME:
        case HEADER_DG_ACK_HORSE_NAME:
            CHorseNameManager::instance().UpdateHorseName(
                ((TPacketUpdateHorseName*)data)->dwPlayerID,
                ((TPacketUpdateHorseName*)data)->szHorseName);
            break;

        case HEADER_DG_UPDATE_ITEM_NAME:
        case HEADER_DG_ACK_ITEM_NAME:
            CItemNameManager::instance().UpdateItemName(
                ((TPacketUpdateItemName*)data)->dwItemID,
                ((TPacketUpdateItemName*)data)->szName);
            break;
#ifdef __OFFLINE_SHOP__
        case HEADER_DG_OFFLINE_SHOP_LOAD:
            ReceiveOfflineShopLoadPacket(data);
            break;

        case HEADER_DG_OFFLINE_SHOP_NAME:
            ReceiveOfflineShopUpdateNamePacket(
                (TPacketOfflineShopUpdateName*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_ITEM_ADD:
            ReceiveOfflineShopAddItemPacket((TPacketOfflineShopAddItem*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_ITEM_MOVE:
            ReceiveOfflineShopMoveItemPacket((TPacketOfflineShopMoveItem*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_ITEM_REMOVE:
            ReceiveOfflineShopRemoveItemPacket(
                (TPacketOfflineShopRemoveItem*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_ITEM_INFO:
            ReceiveOfflineShopItemInfo(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;

        case HEADER_DG_OFFLINE_SHOP_BUY_DONE:
            ReceiveOfflineShopBuyDone(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;

        case HEADER_DG_OFFLINE_SHOP_GOLD:
            ReceiveOfflineShopUpdateGoldPacket(
                (TPacketOfflineShopUpdateGold*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_DESTROY:
            ReceiveOfflineShopDestroyPacket((TPacketOfflineShopDestroy*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_CLOSE:
            ReceiveOfflineShopClosePacket((TPacketOfflineShopClose*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_OPEN:
            ReceiveOfflineShopOpenPacket((TPacketOfflineShopOpen*)data);
            break;

        case HEADER_DG_OFFLINE_SHOP_SEARCH_RESULT:
            ReceiveOfflineShopSearchResultPacket(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;
#endif

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
        case HEADER_DG_AUTO_EVENT:
            AutoEvents(data);
            break;
#endif

#ifdef ENABLE_BATTLE_PASS

        case HEADER_DG_BATTLE_PASS_LOAD_RANKING:
            BattlePassLoadRanking(
                DESC_MANAGER::instance().FindByHandle(header.handle), data);
            break;
#endif

#ifdef __DUNGEON_FOR_GUILD__
        case HEADER_DG_GUILD_DUNGEON:
            GuildDungeon(data);
            break;
        case HEADER_DG_GUILD_DUNGEON_CD:
            GuildDungeonCD(data);
            break;
#endif

#ifdef ENABLE_GEM_SYSTEM
        case HEADER_DG_GEM_LOG:
            GemLog(data);
            break;
#endif

        default:
            return false;
    }

    return true;
}
