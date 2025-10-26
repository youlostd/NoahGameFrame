#include "char.h"

#include <game/GamePacket.hpp>
#include <game/AffectConstants.hpp>
#include <game/MotionTypes.hpp>
#include <game/Constants.hpp>
#include <game/ItemConstants.hpp>
#include <game/MasterPackets.hpp>
#include <base/Crc32.hpp>

#include <absl/strings/str_cat.h>
#include <storm/StringFlags.hpp>
#include <utility>
#include <locale>

#include "config.h"
#include "utils.h"
#include "char_manager.h"
#include "DbCacheSocket.hpp"
#include "desc_manager.h"
#include "GBufferManager.h"
#include "item_manager.h"
#include "motion.h"
#include "vector.h"
#include "cmd.h"
#include "fishing.h"
#include "exchange.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "pvp.h"
#include "party.h"
#include "start_position.h"
#include "questmanager.h"
#include "log.h"

#include "guild_manager.h"
#include "dungeon.h"
#include "messenger_manager.h"

#include "war_map.h"
#include "xmas_event.h"
#include "GBanWord.h"
#include "target.h"
#include "wedding.h"
#include "mob_manager.h"
#include "mining.h"
#include "GArena.h"
#include "gm.h"
#include "map_location.h"
#include "BlueDragon_Binder.h"

#include "main.h"

#ifdef __PET_SYSTEM__
#include "PetSystem.h"
#endif
#include "DragonSoul.h"
#include "ItemUtils.h"

#include "OXEvent.h"
#include "itemname_manager.h"
#include "desc.h"
#include <game/HuntingManager.hpp>
#include "guild.h"
#include "item.h"
#include "PacketUtils.hpp"
#include "skill.h"

#ifdef ENABLE_MELEY_LAIR_DUNGEON
#include "MeleyLair.h"
#endif

#if defined(WJ_COMBAT_ZONE)
#include "combat_zone.h"
#endif

#ifdef ENABLE_HYDRA_DUNGEON
#include "HydraDungeon.h"
#endif

#ifdef __OFFLINE_SHOP__
#include "OfflineShop.h"
#include "OfflineShopManager.h"
#include "OfflineShopConfig.h"
#endif

#ifdef ENABLE_BATTLE_PASS
#include "battle_pass.h"
#endif



void ELPlainCoord_GetRotatedPixelPosition(long centerX, long centerY, float distance, float rotDegree, long *pdstX,
                                          long *pdstY)
{
    float rotRadian = float(3.141592 * rotDegree / 180.0f);
    *pdstX = (long)(centerX + distance * float(sin((double)rotRadian)));
    *pdstY = (long)(centerY + distance * float(cos((double)rotRadian)));
}

// <Factor> DynamicCharacterPtr member function definitions

CHARACTER *DynamicCharacterPtr::Get() const
{
    CHARACTER *p = nullptr;
    if (is_pc) { p = g_pCharManager->FindByPID(id); }
    else { p = g_pCharManager->Find(id); }
    return p;
}

DynamicCharacterPtr &DynamicCharacterPtr::operator=(CHARACTER *character)
{
    if (character == nullptr)
    {
        Reset();
        return *this;
    }
    if (character->IsPC())
    {
        is_pc = true;
        id = character->GetPlayerID();
    }
    else
    {
        is_pc = false;
        id = character->GetVID();
    }
    return *this;
}

uint32_t CalculateMotionMovementTime(CHARACTER *ch, const Motion *motion, float distance)
{
    const auto dur = static_cast<uint32_t>(distance / motion->moveSpeed * 1000.0f);
    return CalculateDuration(ch->GetLimitPoint(POINT_MOV_SPEED), dur);
}

CHARACTER::CHARACTER()
    : CEntity(ENTITY_CHARACTER), m_toggleMount(nullptr, [](CHARACTER *ch) -> void
    {
        if(thecore_is_shutdowned())
            return;

        if (ch)
        {
            ch->SetRider(nullptr);
            M2_DESTROY_CHARACTER(ch);
        }
    })
{
    m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateIdle, &CHARACTER::EndStateEmpty);
    m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
    m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateBattle, &CHARACTER::EndStateEmpty);

    Initialize();
}

CHARACTER::~CHARACTER() { CHARACTER::Destroy(); }

void CHARACTER::Initialize()
{
    const auto now = get_dword_time();

    m_bNoOpenedShop = true;

    m_bOpeningSafebox = false;

    m_dwPlayerID = 0;
    m_dwKillerPID = 0;

    m_pkRegen = nullptr;
    regen_id_ = 0;
    m_posRegen.x = m_posRegen.y = m_posRegen.z = 0;

    m_fRegenAngle = 0.0f;

    _canRegenHP = true;

    m_pkChrShopOwner = nullptr;
    m_pkExchange = nullptr;
    m_pkParty = nullptr;
    m_pkPartyRequestEvent = nullptr;

    m_pGuild = nullptr;

    m_pkChrTarget = nullptr;

    m_pkMuyeongEvent = nullptr;
#if defined(WJ_COMBAT_ZONE)
    m_pkCombatZoneLeaveEvent = NULL;
    m_pkCombatZoneWarpEvent = NULL;
#endif
    m_pkWarpNPCEvent = nullptr;
    m_pkDeadEvent = nullptr;
    m_pkStunEvent = nullptr;
    m_pkSaveEvent = nullptr;
    m_pkRecoveryEvent = nullptr;
    m_pkTimedEvent = nullptr;
    m_pkFishingEvent = nullptr;
    m_pkWarpEvent = nullptr;

    // MINING
    m_despawnEvent = nullptr;
    // END_OF_MINING

    m_pkPoisonEvent = nullptr;
    m_pkFireEvent = nullptr;
    m_pkBleedEvent = nullptr;

    m_pkAffectEvent = nullptr;

    m_pkDestroyWhenIdleEvent = nullptr;
    m_pkCheckSpeedHackEvent = nullptr;
    m_speed_hack_count = 0;

    m_motionKey = 0;
    m_motion = nullptr;
    m_motionStartTime = 0;
    m_motionDuration = 0;

    m_posStart.x = m_posStart.y = 0;
    m_posDest.x = m_posDest.y = 0;

    memset(&m_points, 0, sizeof(m_points));
    memset(&m_pointsInstant, 0, sizeof(m_pointsInstant));

    for (unsigned short &pAcceSlot : m_pointsInstant.pAcceSlots)
        pAcceSlot = INVENTORY_MAX_NUM;

    m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_LEFT] = INVENTORY_MAX_NUM;
    m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_RIGHT] = INVENTORY_MAX_NUM;
    m_pointsInstant.changeLookSlots[CHANGELOOK_SLOT_SPECIAL_ITEM] = INVENTORY_MAX_NUM;

    memset(&m_quickslot, 0, sizeof(m_quickslot));

    m_bCharType = CHAR_TYPE_MONSTER;

    SetPosition(POS_STANDING);

    m_dwPlayStartTime = now;

    GotoState(m_stateIdle);
    m_dwStateDuration = 1;

    m_lastMoveTime = 0;
    m_clientLastMoveTime = 0;
    m_dwLastAttackTime = 0;
    m_walkStartTime = 0;

    m_bAddChrState = 0;

    m_pkChrStone = nullptr;

    m_pkSafebox = nullptr;
    m_iSafeboxSize = SAFEBOX_PAGE_SIZE * 15;
    m_iSafeboxLoadTime = 0;

    m_iMallLoadTime = 0;

    m_posWarp.x = m_posWarp.y = m_posWarp.z = 0;
    m_lWarpMapIndex = 0;

    m_posExit.x = m_posExit.y = m_posExit.z = 0;
    m_lExitMapIndex = 0;

    m_pSkillLevels = nullptr;

    m_dwFlyTargetID = 0;

    m_dwNextStatePulse = 0;

    m_dwLastDeadTime = now - 180000;

    m_bSkipSave = false;

    m_bHasPoisoned = false;

    m_pkDungeon = nullptr;
    m_iEventAttr = 0;

    m_bNowWalking = m_bWalking = false;
    ResetChangeAttackPositionTime();

    m_bDetailLog = false;
    m_bMonsterLog = false;

    m_bDisableCooltime = false;

    m_iAlignment = 0;
    m_iRealAlignment = 0;
#if defined(WJ_COMBAT_ZONE)
    m_iCombatZonePoints = 0;
    m_iCombatZoneDeaths = 0;
#endif
    m_iKillerModePulse = 0;
    m_bPKMode = PK_MODE_PEACE;

    m_dwQuestNPCVID = 0;
    m_dwQuestByVnum = 0;

    m_dwUnderGuildWarInfoMessageTime = now - 60000;

    m_bUnderRefine = false;

    // REFINE_NPC
    m_dwRefineNPCVID = 0;
    // END_OF_REFINE_NPC

    m_dwPolymorphRace = 0;

    m_bStaminaConsume = false;

    ResetChainLightningIndex();

    m_dwMountVnum = 0;
    m_chHorse = nullptr;
    m_chRider = nullptr;

    m_pWarMap = nullptr;

    m_bChatCounter = 0;

    m_dwLastVictimSetTime = now - 3000;
    m_iMaxAggro = -100;

    m_bSendHorseLevel = 0;
    m_bSendHorseHealthGrade = 0;
    m_bSendHorseStaminaGrade = 0;

    m_dwLoginPlayTime = 0;

    m_pkChrMarried = nullptr;

    m_posSafeboxOpen.x = -1000;
    m_posSafeboxOpen.y = -1000;

    // EQUIP_LAST_SKILL_DELAY
    m_dwLastSkillTime = now - 20000;
    // END_OF_EQUIP_LAST_SKILL_DELAY

    // MOB_SKILL_COOLTIME
    memset(m_adwMobSkillCooltime, 0, sizeof(m_adwMobSkillCooltime));
    // END_OF_MOB_SKILL_COOLTIME

    // ARENA
    m_pArena = nullptr;
    m_nPotionLimit = quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count");
    // END_ARENA

    // PREVENT_TRADE_WINDOW
    m_isOpenSafebox = 0;
    // END_PREVENT_TRADE_WINDOW

    // PREVENT_REFINE_HACK
    m_iRefineTime = 0;
    // END_PREVENT_REFINE_HACK

    // RESTRICT_USE_SEED_OR_MOONBOTTLE
    m_iSeedTime = 0;
    // END_RESTRICT_USE_SEED_OR_MOONBOTTLE
    // PREVENT_PORTAL_AFTER_EXCHANGE
    m_iExchangeTime = 0;
    // END_PREVENT_PORTAL_AFTER_EXCHANGE
    //

    m_iMyShopTime = 0;

    m_deposit_pulse = 0;
    m_braveryCapePulse = 0;
    m_braveryCapePulsePremium = 0;
    m_equipPulse = 0;

    m_known_guild.clear();

    m_dwLogOffInterval = 0;
    m_dwLastPlay = 0;

    m_lastComboTime = 0;
    m_comboInputTime = 0;
    m_comboDuration = 0;
    m_iComboHackCount = 0;
    m_comboLevel = 0;
    m_comboIndex = 0;

    m_dwMountTime = 0;

#ifdef __PET_SYSTEM__
    m_petSystem = nullptr;
    m_petActor = nullptr;
#endif

#ifdef __FAKE_PC__
    m_pkFakePCAfkEvent = nullptr;
    m_pkFakePCOwner = nullptr;
    m_pkFakePCSpawnItem = nullptr;
    m_fFakePCDamageFactor = 1.0f;
    m_bIsNoAttackFakePC = false;
#endif

    m_fAttMul = 1.0f;
    m_fDamMul = 1.0f;

    // Anti-hack
    m_pointsInstant.iDragonSoulActiveDeck = -1;

    m_iSyncNoHackCount = 0;
    m_iSyncPlayerHackCount = 0;
    m_iSyncMonsterHackCount = 0;

    // Regen penalties
    m_RegenDelay = 0;
    m_RegenPctReduction = 0;

    // Private shops
    m_pkMyShop = nullptr;
    m_stShopSign.clear();
    m_shopVnum = 30008;
    m_shopTitleType = 0;

    // Acce
    m_AcceWindowType = ACCE_SLOT_TYPE_MAX;

#ifdef __OFFLINE_SHOP__
    this->m_pkMyOfflineShop = nullptr;
    this->m_pkViewingOfflineShop = nullptr;

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
    this->m_buildingOfflineShop = false;
#endif

    this->m_iOfflineShopLastOpeningTime = 0;

#endif

    m_bDungeonInfoLoaded = false;
	m_dungeonInfo.clear();

    m_shopSearchPerPage = 25;
    m_shopSearchCurrentPage = 1;
    m_shopSearchPageCount = 0;
}

void CHARACTER::Create(std::string stName, uint32_t vid, bool isPC)
{
    static int s_crc = 172814;

    const auto crcString = fmt::format("{}{}{}", stName, (void *)this, ++s_crc);
    m_vid = VID(vid, ComputeCrc32(0, crcString.c_str(), crcString.length()));

    if (isPC) { m_stName = stName; }
}

void CHARACTER::Destroy()
{
#ifdef __FAKE_PC__
    if (FakePC_Check() && !FakePC_IsSupporter())
    {
        if (!IsDead() && GetDungeon())
            Dead(nullptr, false);
    }
#endif

    CloseMyShop();

    // destroy fake pc
#ifdef __FAKE_PC__
    FakePC_Destroy();
#endif

    if (m_pkRegen)
    {
        if (m_pkDungeon)
        {
            // Dungeon regen may not be valid at this point
            if (m_pkDungeon->IsValidRegen(m_pkRegen, regen_id_)) { --m_pkRegen->count; }
        }
        else
        {
            // Is this really safe? NO IT ISNT
            if (is_valid_regen(m_pkRegen))
                --m_pkRegen->count;
        }
        m_pkRegen = nullptr;
    }

    SetDungeon(nullptr);

#ifdef __PET_SYSTEM__

    if (m_petActor)
    {
        m_petActor->OnDestroy();
        m_petActor = nullptr;
    }
#endif

    HorseSummon(false);

    if (GetRider())
        GetRider()->ClearHorseInfo();

    if (GetDesc())
    {
        GetDesc()->BindCharacter(nullptr);
        //		BindDesc(NULL);
    }

    if (m_pkExchange)
        m_pkExchange->Cancel();

    SetVictim(nullptr);

    if (GetShop())
    {
        GetShop()->RemoveGuest(this);
        SetShop(nullptr);
    }

    if (IsNPC()) { CleanUpShopOwners(this); }

#ifdef ENABLE_HYDRA_DUNGEON
    CHydraDungeonManager::instance().OnDestroy(this);
#endif

#if defined(WJ_COMBAT_ZONE)
    if (IsPC())
    {
        CCombatZoneManager::instance().OnLogout(this);
    }
#endif

    ClearStone();
    ClearTarget();
    ClearSync();

    if (nullptr == m_pkMobData)
    {
        DragonSoul_CleanUp();
        ClearItem();
    }

    // <Factor> m_pkParty becomes NULL after CParty destructor call!
    auto *party = m_pkParty;
    if (party)
    {
        if (party->GetLeaderPID() == (uint32_t)GetVID() && !IsPC()) { delete party; }
        else
        {
            party->Unlink(this);

            if (!IsPC())
                party->Quit(GetVID());
        }

        SetParty(nullptr); // 안해도 되지만 안전하게.
    }


    StopMuyeongEvent();
    event_cancel(&m_pkWarpNPCEvent);
    event_cancel(&m_pkRecoveryEvent);
    event_cancel(&m_pkDeadEvent);
    event_cancel(&m_pkSaveEvent);
#if defined(WJ_COMBAT_ZONE)
    event_cancel(&m_pkCombatZoneLeaveEvent);
    event_cancel(&m_pkCombatZoneWarpEvent);
#endif
    event_cancel(&m_pkTimedEvent);
    event_cancel(&m_pkStunEvent);
    event_cancel(&m_pkFishingEvent);
    event_cancel(&m_pkPoisonEvent);
    event_cancel(&m_pkFireEvent);
    event_cancel(&m_pkCheckSpeedHackEvent);

    event_cancel(&m_pkPartyRequestEvent);
    // DELAYED_WARP
    event_cancel(&m_pkWarpEvent);
    // END_DELAYED_WARP

    event_cancel(&m_miningEvent);
    event_cancel(&m_despawnEvent);

    for (auto &it : m_mapMobSkillEvent)
    {
        LPEVENT pkEvent = it.second;
        event_cancel(&pkEvent);
    }
    m_mapMobSkillEvent.clear();

    event_cancel(&m_pkAffectEvent);
    event_cancel(&m_pkDestroyWhenIdleEvent);

    CEntity::Destroy();

    if (GetSectree())
        GetSectree()->RemoveEntity(this);

    if (m_bMonsterLog)
        g_pCharManager->UnregisterForMonsterLog(this);
}

const std::string &CHARACTER::GetName() const
{
    return m_stName.empty() ? (m_pkMobData ? m_pkMobData->szLocaleName : m_stName) : m_stName;
}

const std::string &CHARACTER::GetProtoName() const { return m_pkMobData ? m_pkMobData->szName : m_stName; }

bool CHARACTER::OpenMyShop(const char *sign, const TItemPos &bundleItem, const TShopItemTable *table, uint8_t itemCount)
{
    if (GetPart(PART_MAIN).vnum > 2 || GetPart(PART_MAIN).appearance != 0)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "갑옷을 벗어야 개인 상점을 열 수 있습니다.");
        return false;
    }

    if (GetMyShop()) // 이미 샵이 열려 있으면 닫는다.
    {
        CloseMyShop();
        return false;
    }

    // 진행중인 퀘스트가 있으면 상점을 열 수 없다.
    quest::PC *pPC = quest::CQuestManager::instance().GetPCForce(GetPlayerID());

    // GetPCForce는 NULL일 수 없으므로 따로 확인하지 않음
    if (pPC->IsRunning())
        return false;

    if (itemCount == 0 || itemCount > SHOP_HOST_ITEM_MAX_NUM)
        return false;

    Gold nTotalMoney = GetGold();
    for (int n = 0; n < itemCount; ++n)
        nTotalMoney += table[n].price;

    if (GetAllowedGold() <= nTotalMoney)
    {
        SPDLOG_ERROR("ShopOpen Gold overflow for id {0} name {1}. {2} <= {3}", GetPlayerID(), GetName(),
                     GetAllowedGold(), nTotalMoney);

        SendI18nChatPacket(this, CHAT_TYPE_INFO, "20억 냥을 초과하여 상점을 열수가 없습니다");
        return false;
    }

    char szSign[SHOP_SIGN_MAX_LEN + 1];
    strlcpy(szSign, sign, sizeof(szSign));

    m_stShopSign = szSign;

    if (m_stShopSign.length() == 0)
        return false;

    if (CBanwordManager::instance().CheckString(m_stShopSign.c_str(), m_stShopSign.length()))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "비속어나 은어가 포함된 상점 이름으로 상점을 열 수 없습니다.");
        return false;
    }

    // 아이템 종류별 가격, first: vnum, second: 단일 수량 가격
    std::unordered_map<uint32_t, Gold> priceList;

    std::unordered_set<TItemPos> cont;
    for (uint8_t i = 0; i < itemCount; ++i)
    {
        const auto &entry = table[i];

        if (cont.find(entry.pos) != cont.end())
        {
            SPDLOG_WARN("MYSHOP: duplicate shop item detected for {0}", GetName());
            return false;
        }

        if (entry.display_pos < 0 || entry.display_pos >= SHOP_HOST_ITEM_MAX_NUM)
        {
            SPDLOG_WARN("MYSHOP: display_pos {0} invalid for name {1}", entry.display_pos, GetName());
            return false;
        }

        if (entry.pos == bundleItem)
        {
            SPDLOG_WARN("MYSHOP: {0} tried to sell bundle-item", GetName());
            return false;
        }

        CItem *pkItem = GetItem(entry.pos);
        if (!pkItem)
            return false;

        const auto item_table = pkItem->GetProto();
        STORM_ASSERT(nullptr != item_table, "Invalid item");

        if (IS_SET(item_table->dwAntiFlags, ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_MYSHOP))
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "유료화 아이템은 개인상점에서 판매할 수 없습니다.");
            return false;
        }

        if (pkItem->IsEquipped())
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "장비중인 아이템은 개인상점에서 판매할 수 없습니다.");
            return false;
        }

        if (pkItem->isLocked())
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "사용중인 아이템은 개인상점에서 판매할 수 없습니다.");
            return false;
        }

        priceList[pkItem->GetVnum()] = entry.price / pkItem->GetCount();
        cont.insert(entry.pos);
    }

    CItem *bundle = GetItem(bundleItem);
    if (!bundle)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "SHOP_BUNDLE_REMOVED");
        return false;
    }

    if (bundle->GetSubType() == USE_SILK_BOTARY)
    {
        //
        // 아이템 가격정보를 저장하기 위해 아이템 가격정보 패킷을 만들어 DB 캐시에 보낸다.
        //

        MyShopPriceListHeader header;
        header.pid = GetPlayerID();
        header.count = priceList.size();

        TEMP_BUFFER buf;
        buf.write(&header, sizeof(header));

        for (const auto &pricePair : priceList)
        {
            MyShopPriceInfo info;
            info.vnum = pricePair.first;
            info.price = pricePair.second;
            buf.write(&info, sizeof(info));
        }

        db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_UPDATE, 0, buf.read_peek(), buf.size());
    }
    else
    {
        // silk botary expires by time
        bundle->SetCount(bundle->GetCount() - 1);
    }

    if (m_pkExchange)
        m_pkExchange->Cancel();

    TPacketGCShopSign p;
    p.dwVID = GetVID();
    p.szSign = sign;
    PacketAround(m_map_view, this,HEADER_GC_SHOP_SIGN, p);

    m_pkMyShop = CShopManager::instance().CreatePCShop(this, table, itemCount);

    if (IsPolymorphed())
        RemoveAffect(AFFECT_POLYMORPH);

    Unmount(this);

    SetPolymorph(30008, true);
    return true;
}

void CHARACTER::CloseMyShop()
{
    if (GetMyShop())
    {
        m_stShopSign.clear();
        CShopManager::instance().DestroyPCShop(this);
        m_pkMyShop = nullptr;

        TPacketGCShopSign p;
        p.dwVID = GetVID();
        PacketAround(m_map_view, this,HEADER_GC_SHOP_SIGN, p);

        SetPolymorph(0, true);
    }
}

void EncodeMovePacket(TPacketGCMove &pack, uint32_t dwVID, uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y,
                      uint32_t dwDuration, uint32_t dwTime, uint8_t bRot)
{
    pack.bFunc = bFunc;
    pack.bArg = bArg;
    pack.dwVID = dwVID;
    pack.dwTime = dwTime ? dwTime : get_dword_time();
    pack.bRot = bRot;
    pack.lX = x;
    pack.lY = y;
    pack.dwDuration = dwDuration;
}

// Entity에 내가 나타났다고 패킷을 보낸다.
void CHARACTER::EncodeInsertPacket(CEntity *entity)
{
    DESC *d;
    if (!((d = entity->GetDesc())))
        return;

    // 길드이름 버그 수정 코드
    CHARACTER *ch = (CHARACTER *)entity;
    ch->SendGuildName(GetGuild());
    // 길드이름 버그 수정 코드

    const auto now = get_dword_time();
    const auto pos = GetInterpolatedPosition(now);

    auto f = GetRequirementFunction();
    if (f && !f(ch)) { return; }

    // Some NPCs appearance might be overwritten later on.
    uint16_t raceNum = GetRaceNum();

    if (IsBuffBot())
        raceNum = MAIN_RACE_SHAMAN_W;

    TPacketGCCharacterAdd pack = {};
    pack.dwVID = m_vid;
    pack.scale = GetScale();

    if (IsMonster() || IsStone())
    {
        pack.dwLevel = GetLevel();
        pack.dwAIFlag = IsMonster() ? GetAIFlag() : 0;
    }

    pack.bType = GetCharType();
    pack.angle = GetRotation();
    pack.x = pos.x;
    pack.y = pos.y;
    pack.z = 0;
    pack.wRaceNum = raceNum;
    pack.bMovingSpeed = GetLimitPoint(POINT_MOV_SPEED);
    pack.bAttackSpeed = GetLimitPoint(POINT_ATT_SPEED);

    pack.bStateFlag = m_bAddChrState;
#ifdef __FAKE_PC__
    if (FakePC_Check())
    {
        pack.bType = CHAR_TYPE_PC;
        pack.wRaceNum = FakePC_GetOwner()->GetRaceNum();
    }
#endif
    d->Send(HEADER_GC_CHARACTER_ADD, pack);
    bool isPC = 	IsPC();
    if (isPC || m_bCharType == CHAR_TYPE_NPC || m_bCharType == CHAR_TYPE_PET || m_bCharType == CHAR_TYPE_MONSTER ||
        m_bCharType == CHAR_TYPE_ATTACK_PET || m_bCharType == CHAR_TYPE_MOUNT || IsShop() || IsBuffBot())
    {
        TPacketGCCharacterAdditionalInfo addPacket = {};
        addPacket.dwVID = m_vid;

        // Player name
        addPacket.name = GetName();

        // Visible Parts
        std::memcpy(addPacket.adwPart, m_pointsInstant.parts, sizeof(m_pointsInstant.parts));

        // For the special buff bot npc's we replace the existing
        // equipment with the buffbot equipment.
        if (IsBuffBot())
        {
            addPacket.adwPart[PART_MAIN].vnum = 44039;
            addPacket.adwPart[PART_WEAPON].vnum = 7379;
            addPacket.adwPart[PART_HEAD].vnum = 0;
            addPacket.adwPart[PART_HAIR].costume = 251;
            addPacket.adwPart[PART_HAIR].vnum = 251;
            addPacket.adwPart[PART_ACCE].vnum = 0;
            addPacket.name = "Buffbot";
        }

        // Title System
        addPacket.pt = GetPlayerTitle();

        // PvP Team
        addPacket.pvpTeam = m_sPVPTeam;

        // Owner name e.g. Player's Pet
        if (IsPet() || GetRider() != nullptr)
        {
            if (IsPet())
                addPacket.ownerVid = GetPet()->GetOwner().GetVID();
            else
                addPacket.ownerVid = GetRider()->GetVID();
        }

        const auto *guild = GetGuild();
        if (guild)
        {
            // Guild crowns
            if (guild->GetMasterPID() == GetPlayerID())
                addPacket.isGuildLeader = 1;

            if (const auto *member = guild->GetMember(GetPlayerID()); member)
            {
                addPacket.isGuildGeneral = member->is_general;
            }
        }

#ifdef __OFFLINE_SHOP__
        if (this->GetMyOfflineShop() && this->GetRaceNum() == g_dwOfflineShopKeeperVNum &&
            ch->GetPlayerID() == this->GetMyOfflineShop()->GetOwnerPID())
        {
            addPacket.name = "Your shop";
        }
        else { addPacket.name = GetName(); }
#else
        strlcpy(addPacket.name, GetName(), sizeof(addPacket.name));
#endif
#ifdef ENABLE_PLAYTIME_ICON
        addPacket.dwPlayTime = 0;
#endif
        // Language code
        if (IsPC())
        {
            const auto lcCode = GetLanguageIDByName(GetLocale(this)->name);
            if (lcCode) { addPacket.lang = lcCode; }
            else { addPacket.lang = 8; }

#ifdef ENABLE_PLAYTIME_ICON
            addPacket.dwPlayTime = GetRealPoint(POINT_PLAYTIME);
#endif
        }

        addPacket.bPKMode = m_bPKMode;
        addPacket.dwMountVnum = GetMountVnum();
        addPacket.bEmpire = m_bEmpire;

        if (IsPC() || IsPet() || IsMonster() || IsStone())
            addPacket.dwLevel = GetLevel();

        if (GetGuild())
            addPacket.dwGuildID = GetGuild()->GetID();

        addPacket.sAlignment = m_iAlignment / 10;
#ifdef ENABLE_PLAYTIME_ICON
        addPacket.dwPlayTime = GetRealPoint(POINT_PLAYTIME);
#endif
        addPacket.scale = GetScale();

        d->Send(HEADER_GC_CHAR_ADDITIONAL_INFO, addPacket);
    }

    for (const auto &affect : m_affects)
    {
        GcAffectAddPacket p{};
        p.vid = GetVID();
        p.data = affect;
        if (CWarMapManager::instance().IsWarMap(GetMapIndex()) || GetMapIndex() == 112 || gConfig.IsBlockedSkillColorMap(GetMapIndex()))
            p.data.color = 0;

        d->Send(HEADER_GC_AFFECT_ADD, p);
    }

    if (now < m_motionStartTime + m_motionDuration)
    {
        TPacketGCMove pack{};
        EncodeMovePacket(pack, GetVID(), FUNC_MOVE, 0, m_posDest.x, m_posDest.y,
                         m_motionDuration - (now - m_motionStartTime), 0, (uint8_t)(GetRotation() / 5));
        d->Send(HEADER_GC_CHARACTER_MOVE, pack);

        TPacketGCWalkMode p;
        p.vid = GetVID();
        p.mode = m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;
        d->Send(HEADER_GC_WALK_MODE, p);
    }

#ifdef __OFFLINE_SHOP__
    if (this->GetRaceNum() == g_dwOfflineShopKeeperVNum && this->GetMyOfflineShop())
    {
        TPacketGCOfflineShopAdditionalInfo kPacket;
        kPacket.dwVID = this->GetVID();
        kPacket.dwShopOwnerPID = this->GetMyOfflineShop()->GetOwnerPID();
        kPacket.szShopName =  this->GetMyOfflineShop()->GetName();

        d->Send(HEADER_GC_OFFLINE_SHOP_ADDITIONAL_INFO, kPacket);
    }
#endif

    if (entity->IsType(ENTITY_CHARACTER))
    {
        SPDLOG_TRACE("EntityInsert {0} (RaceNum {1}) ({2} {3}) TO {4}", GetName(), GetRaceNum(), GetX() / SECTREE_SIZE,
                     GetY() / SECTREE_SIZE, ((CHARACTER *)entity)->GetName());
    }
}

void CHARACTER::EncodeRemovePacket(CEntity *entity)
{
    if (entity->GetEntityType() != ENTITY_CHARACTER)
        return;

    DESC *d = entity->GetDesc();
    if (!d)
        return;

    TPacketGCCharacterDelete pack;
    pack.id = m_vid;
    d->Send(HEADER_GC_CHARACTER_DEL, pack);

    if (entity->IsType(ENTITY_CHARACTER))
        SPDLOG_TRACE("EntityRemove {0}({1}) FROM {2}", GetName(), (uint32_t)m_vid, ((CHARACTER *)entity)->GetName());
}

void CHARACTER::UpdatePacket()
{
    if (!GetSectree())
        return;

    if (IsPC() && (!GetDesc() || !GetDesc()->GetCharacter()))
        return;

    // UpdatePacket() is called several times during ComputePoints(),
    // e.g. when re-applying the equipped items. Since that happens for every
    // item equipped, the first update packets will contain invalid parts etc.
    if (!m_pointsInstant.computed)
        return;

    TPacketGCCharacterUpdate pack;
    pack.dwVID = m_vid;

    std::memcpy(pack.adwPart, m_pointsInstant.parts, sizeof(m_pointsInstant.parts));

    if (IsBuffBot())
    {
        pack.adwPart[PART_MAIN].vnum = 44039;
        pack.adwPart[PART_WEAPON].vnum = 7379;
        pack.adwPart[PART_HAIR].costume = 251;
        pack.adwPart[PART_HAIR].vnum = 251;
    }

    pack.bMovingSpeed = GetLimitPoint(POINT_MOV_SPEED);
    pack.bAttackSpeed = GetLimitPoint(POINT_ATT_SPEED);
    pack.bStateFlag = m_bAddChrState;

    pack.dwGuildID = GetGuild() ? GetGuild()->GetID() : 0;
    pack.sAlignment = m_iAlignment / 10;
    pack.dwLevel = IsPC() || IsMonster() || IsStone() || IsPet() ? GetLevel() : 0;
    pack.bPKMode = m_bPKMode;
	pack.comboLevel = m_comboLevel;

    pack.pt = GetPlayerTitle();
#if defined(WJ_COMBAT_ZONE)
    pack.combat_zone_points = GetCombatZonePoints();
#endif

    pack.scale = GetScale();
    pack.pvpTeam = GetPVPTeam();

#ifdef ENABLE_PLAYTIME_ICON
    pack.dwPlayTime = GetRealPoint(POINT_PLAYTIME);
#endif
    PacketAround(m_map_view, this,HEADER_GC_CHARACTER_UPDATE, pack);
}

void CHARACTER::SendBlockModeInfo()
{
    auto locale = GetLocale(this);
    bool hasBlockMode = false;

    std::string message = LC_TEXT_LC("Block: ", locale);

    if (IsBlockMode(BLOCK_EXCHANGE))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Exchange", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_GUILD_INVITE))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Guild", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_MESSENGER_INVITE))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Friend", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_PARTY_INVITE))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Group", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_PARTY_REQUEST))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Request", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_VIEW_EQUIP))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Equip", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_WHISPER))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Whisper", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_CHAT))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Chat", locale), " ");
        hasBlockMode = true;
    }

    if (IsBlockMode(BLOCK_DUEL_REQUEST))
    {
        absl::StrAppend(&message, LC_TEXT_LC("Duel", locale), " ");
        hasBlockMode = true;
    }

    if (hasBlockMode) { ChatPacket(CHAT_TYPE_INFO, message.c_str()); }
}

CHARACTER *CHARACTER::FindCharacterInView(const char *c_pszName, bool bFindPCOnly)
{
    ENTITY_MAP::iterator it = m_map_view.begin();

    for (; it != m_map_view.end(); ++it)
    {
        if (!it->first->IsType(ENTITY_CHARACTER))
            continue;

        CHARACTER *tch = (CHARACTER *)it->first;

        if (bFindPCOnly && tch->IsNPC())
            continue;

        if (!strcasecmp(tch->GetName().c_str(), c_pszName))
            return (tch);
    }

    return nullptr;
}

std::vector<CHARACTER *> CHARACTER::FindCharactersInRange(int32_t range)
{
    std::vector<CHARACTER *> chars;

    for (const auto &elem : m_map_view)
    {
        if (!elem.first->IsType(ENTITY_CHARACTER))
            continue;

        const auto tch = static_cast<CHARACTER *>(elem.first);

        if (!tch->IsPC())
            continue;

        if (DISTANCE_APPROX(GetX() - tch->GetX(), GetY() - tch->GetY()) > range)
            continue;

        chars.push_back(tch);
    }

    return chars;
}

void CHARACTER::SetPosition(int pos)
{
    if (pos == POS_STANDING)
    {
        REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);
        REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

        event_cancel(&m_pkDeadEvent);
        event_cancel(&m_pkStunEvent);
    }
    else if (pos == POS_DEAD)
        SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);

    if (!IsStone())
    {
        switch (pos)
        {
        case POS_FIGHTING:
            if (!IsState(m_stateBattle))
                MonsterLog("[BATTLE] Fighting state");

            GotoState(m_stateBattle);
            break;

        default:
            if (!IsState(m_stateIdle))
                MonsterLog("[IDLE] Resting state");

            GotoState(m_stateIdle);
            break;
        }
    }

    m_pointsInstant.position = pos;
    UpdatePacket();
}

void CHARACTER::Save()
{
    if (!m_bSkipSave)
        g_pCharManager->DelayedSave(this);
}

void CHARACTER::CreatePlayerProto(TPlayerTable &tab)
{
    tab  = {};

    tab.id = m_dwPlayerID;
    storm::CopyStringSafe(tab.name, GetName());
    storm::CopyStringSafe(tab.ip, GetDesc()->GetHostName());
    tab.empire = m_bEmpire;
    tab.job = m_points.job;
    tab.voice = GetPoint(POINT_VOICE);
    tab.level = GetLevel();
    tab.level_step = GetPoint(POINT_LEVEL_STEP);
    tab.exp = GetExp();
    tab.gold = GetGold();
    tab.part_base = m_pointsInstant.bBasePart;
    tab.skill_group = m_points.skill_group;

#ifdef ENABLE_GEM_SYSTEM
    tab.gem = GetGem();
#endif

    uint32_t dwPlayedTime = (get_dword_time() - m_dwPlayStartTime);

    /*
     *
    if (dwPlayedTime > 60000)
    {
        if (GetSectree() && !GetSectree()->IsAttr(GetX(), GetY(), ATTR_BANPK))
        {
            if (GetRealAlignment() < 0)
            {
                if (IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_TIME))
                    UpdateAlignment(0);
                else
                    UpdateAlignment(0);
            }
            else
                UpdateAlignment(0);
        }


    }	 */

    if (dwPlayedTime > 60000) {
        m_points.playtime += dwPlayedTime / 60000;
        ResetPlayTime(dwPlayedTime % 60000);
    }

    uint8_t bBattlePassId = GetBattlePassId();

    uint32_t dwMinLevel, targetPlaytime;
    if (CBattlePass::instance().BattlePassMissionGetInfo(
            bBattlePassId, REACH_PLAYTIME, &dwMinLevel, &targetPlaytime)) {
        UpdateMissionProgress(REACH_PLAYTIME, bBattlePassId, m_points.playtime,
                              targetPlaytime, true);
    }

    tab.playtime = m_points.playtime;
    tab.lAlignment = m_iRealAlignment;
    tab.last_play = m_dwLastPlay;

    if (m_posWarp.x != 0 || m_posWarp.y != 0)
    {
        tab.x = m_posWarp.x;
        tab.y = m_posWarp.y;
        tab.z = 0;
        tab.lMapIndex = m_lWarpMapIndex;
    }
    else
    {
        tab.x = GetX();
        tab.y = GetY();
        tab.z = GetZ();
        tab.lMapIndex = GetMapIndex();
    }

    if (m_lExitMapIndex == 0)
    {
        tab.lExitMapIndex = tab.lMapIndex;
        tab.lExitX = tab.x;
        tab.lExitY = tab.y;
    }
    else
    {
        tab.lExitMapIndex = m_lExitMapIndex;
        tab.lExitX = m_posExit.x;
        tab.lExitY = m_posExit.y;
    }

    SPDLOG_INFO("SAVE: {0} {1}x{2}", GetName(), tab.x, tab.y);

    tab.st = m_points.st;
    tab.ht = m_points.ht;
    tab.dx = m_points.dx;
    tab.iq = m_points.iq;;

    tab.stat_point = GetPoint(POINT_STAT);
    tab.skill_point = GetPoint(POINT_SKILL);
    tab.sub_skill_point = GetPoint(POINT_SUB_SKILL);
    tab.horse_skill_point = GetPoint(POINT_HORSE_SKILL);
    tab.tree_skill_point = GetPoint(POINT_SKILLTREE_POINTS);

#ifdef ENABLE_GEM_SYSTEM
    tab.gem = GetGem();
#endif

    tab.hp = GetHP();
    tab.sp = GetSP();

    tab.stamina = GetStamina();

    tab.sRandomHP = m_points.iRandomHP;
    tab.sRandomSP = m_points.iRandomSP;

    for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
        tab.quickslot[i] = m_quickslot[i];

    std::memcpy(tab.parts, m_pointsInstant.parts, sizeof(tab.parts));

    // REMOVE_REAL_SKILL_LEVLES
    std::memcpy(tab.skills, m_pSkillLevels.get(), sizeof(TPlayerSkill) * SKILL_MAX_NUM);
    // END_OF_REMOVE_REAL_SKILL_LEVLES
#ifdef ENABLE_BATTLE_PASS
    tab.dwBattlePassEndTime = m_dwBattlePassEndTime;
#endif

#ifdef __FAKE_PC__
    strlcpy(tab.fakepc_name, m_stFakePCName.c_str(), sizeof(tab.fakepc_name));
#endif

    CopyStringSafe(tab.title.title, m_playerTitle.title);
    tab.title.color = m_playerTitle.color;

    tab.horse = GetHorseData();
    tab.blockMode = m_points.blockMode;
    tab.chatFilter = m_points.chatFilter;

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    tab.iOfflineShopFarmedOpeningTime = this->GetOfflineShopFarmedOpeningTime();
    tab.iOfflineShopSpecialOpeningTime = this->GetOfflineShopSpecialOpeningTime();
#endif
}

void CHARACTER::SaveReal()
{
    if (m_bSkipSave)
        return;

    if (IsShop())
    {
        SPDLOG_ERROR("SaveReal should not be happening on a shop char (PID {0})", GetPlayerID());
        return;
    }

    if (!GetDesc())
    {
        SPDLOG_ERROR("Character::Save : no descriptor when saving (name: {0})", GetName());
        return;
    }

    // Save player
    TPlayerTable table;
    CreatePlayerProto(table);

    db_clientdesc->DBPacket(HEADER_GD_PLAYER_SAVE, GetDesc()->GetHandle(), &table, sizeof(TPlayerTable));

    // Save quest
    quest::PC *pkQuestPC = quest::CQuestManager::instance().GetPCForce(GetPlayerID());

    if (!pkQuestPC)
        SPDLOG_ERROR("CHARACTER::Save : null quest::PC pointer! (name {0})", GetName());
    else { pkQuestPC->Save(); }

    // Save marriage (if existing)
    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(GetPlayerID());
    if (pMarriage)
        pMarriage->Save();
}

void CHARACTER::FlushDelayedSaveItem()
{
    // 저장 안된 소지품을 전부 저장시킨다.
    for (int i = 0; i < m_windows.inventory.GetSize(); ++i)
        if (auto item = m_windows.inventory.GetUnique(i); item)
            ITEM_MANAGER::instance().FlushDelayedSave(item);

    for (auto item : m_pointsInstant.wear)
        if (item)
            ITEM_MANAGER::instance().FlushDelayedSave(item);

    for (auto item : m_pointsInstant.switchbot)
        if (item)
            ITEM_MANAGER::instance().FlushDelayedSave(item);

    for (int i = 0; i < m_windows.belt.GetSize(); ++i)
        if (auto item = m_windows.belt.GetUnique(i); item)
            ITEM_MANAGER::instance().FlushDelayedSave(item);
}

void CHARACTER::Disconnect(const char *c_pszReason)
{
    assert(GetDesc() != NULL);

    SPDLOG_INFO("DISCONNECT: {0} ({1})", GetName(), c_pszReason ? c_pszReason : "unset");

#ifdef __FAKE_PC__
    FakePC_Owner_DespawnAll();
#endif

    if (GetShop())
    {
        GetShop()->RemoveGuest(this);
        SetShop(nullptr);
    }

#ifdef __OFFLINE_SHOP__
    if (this->GetViewingOfflineShop())
    {
        this->GetViewingOfflineShop()->RemoveViewer(this);
        this->GetViewingOfflineShop()->RemoveViewerFromSet(this);
        this->SetViewingOfflineShop(nullptr);
    }
#endif

    if (GetArena() != nullptr) { GetArena()->OnDisconnect(GetPlayerID()); }

    if (GetParty() != nullptr) { GetParty()->UpdateOfflineState(GetPlayerID(), GetMapIndex(), gConfig.channel); }

    if (GetWeddingMap()) { SetWeddingMap(nullptr); }

    marriage::CManager::instance().Logout(this);

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if (MeleyLair::CMgr::instance().IsMeleyMap(GetMapIndex()))
        MeleyLair::CMgr::instance().Leave(this, false);
#endif

    GmCharacterLeavePacket p{};
    p.pid = m_dwPlayerID;
    DESC_MANAGER::instance().GetMasterSocket()->Send(kGmCharacterLeave, p);

    GdPlayerDisconnectPacket p2;
    p2.pid = GetPlayerID();
    p2.aid = GetDesc()->GetAid();
    db_clientdesc->DBPacket(HEADER_GD_PLAYER_DISCONNECT, GetDesc()->GetHandle(), &p2, sizeof(p2));

    LogManager::instance().CharLog(this, 0, "LOGOUT", "");

    if (m_pWarMap)
        SetWarMap(nullptr);

    if (GetGuild())
        GetGuild()->LogoutMember(this);

    quest::CQuestManager::instance().LogoutPC(this);

    if (GetParty())
        GetParty()->Unlink(this);

    // 죽었을 때 접속끊으면 경험치 줄게 하기
    if (IsStun() || IsDead())
    {
        DeathPenalty(0);
        PointChange(POINT_HP, 50 - GetHP());
    }

    if (!g_pCharManager->FlushDelayedSave(this))
        SaveReal();

    FlushDelayedSaveItem();

    SaveAffect();

#ifdef ENABLE_BATTLE_PASS
    auto it = m_listBattlePass.begin();
    while (it != m_listBattlePass.end())
    {
        TPlayerBattlePassMission *pkMission = *it++;

        if (!pkMission->bIsUpdated)
            continue;

        db_clientdesc->DBPacket(HEADER_GD_SAVE_BATTLE_PASS, 0, pkMission, sizeof(TPlayerBattlePassMission));
    }
    m_bIsLoadedBattlePass = false;
#endif

    for (const auto &m : m_huntingMissions)
    {
        if (!m.bIsUpdated)
            continue;

        db_clientdesc->DBPacket(HEADER_GD_SAVE_HUNTING_MISSION, 0, &m, sizeof(TPlayerHuntingMission));
    }

    SaveSwitchbotData();

    m_bSkipSave = true; // 이 이후에는 더이상 저장하면 안된다.

    quest::CQuestManager::instance().DisconnectPC(this);

    CloseSafebox();

    CloseMall();

    CPVPManager::instance().Disconnect(this);

    CTargetManager::instance().Logout(GetPlayerID());

    MessengerManager::instance().Logout(GetName());
    if (GetMapIndex() == OXEVENT_MAP_INDEX)
        COXEventManager::instance().RemoveFromAttenderList(GetPlayerID());

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    COfflineShopManager::instance().LogoutOwner(this);
#endif

    if (GetDesc())
    {
        TPacketGCPointChange pack;
        pack.dwVID = m_vid;
        pack.type = POINT_PLAYTIME;
        pack.value = m_points.playtime + (get_dword_time() - m_dwPlayStartTime) / 60000;
        pack.amount = 0;
        GetDesc()->Send(HEADER_GC_CHARACTER_POINT_CHANGE, pack);

        TGcUpdateLastPlay updateLastPlay;
        updateLastPlay.lastPlay = get_global_time();
        GetDesc()->Send(HEADER_GC_UPDATE_LAST_PLAY, updateLastPlay);

        GetDesc()->BindCharacter(nullptr);
        //		BindDesc(NULL);
    }

    M2_DESTROY_CHARACTER(this);
}

bool CHARACTER::Show(long lMapIndex, long x, long y, long z, bool bShowSpawnMotion, bool bAggressive)
{
    SECTREE *sectree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);
    if (!sectree)
    {
        SPDLOG_INFO("cannot find sectree by %dx%d mapindex %d", x, y, lMapIndex);
        return false;
    }

    SetMapIndex(lMapIndex);

    bool bChangeTree = false;

    if (!GetSectree() || GetSectree() != sectree)
        bChangeTree = true;

    if (bChangeTree)
    {
        if (GetSectree())
            GetSectree()->RemoveEntity(this);

        ViewCleanup();
    }

    if (!IsNPC())
    {
        SPDLOG_INFO("SHOW: {0} {1}x{2}x{3}", GetName(), x, y, z);
        if (GetStamina() < GetMaxStamina())
            StartAffectEvent();
    }
    else if (m_pkMobData)
    {
        m_pkMobInst->m_posLastAttacked.x = x;
        m_pkMobInst->m_posLastAttacked.y = y;
        m_pkMobInst->m_posLastAttacked.z = z;
    }

    /* if (bShowSpawnMotion)
        SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);*/

    if (bAggressive)
        SetAggressive();

    SetXYZ(x, y, z);

    m_posDest.x = x;
    m_posDest.y = y;
    m_posDest.z = z;

    m_posStart.x = x;
    m_posStart.y = y;
    m_posStart.z = z;

    if (bChangeTree)
    {
        EncodeInsertPacket(this);
        sectree->InsertEntity(this);

        UpdateSectree();
    }
    else
    {
        ViewReencode();
        SPDLOG_INFO("      in same sectree");
    }

    REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

    return true;
}

// BGM_INFO
struct BGMInfo
{
    std::string name;
    float vol;
};

typedef std::map<unsigned, BGMInfo> BGMInfoMap;

static BGMInfoMap gs_bgmInfoMap;
static bool gs_bgmVolEnable = false;

void CHARACTER_SetBGMVolumeEnable()
{
    gs_bgmVolEnable = true;
    SPDLOG_INFO("bgm_info.set_bgm_volume_enable");
}

void CHARACTER_AddBGMInfo(unsigned mapIndex, const char *name, float vol)
{
    BGMInfo newInfo;
    newInfo.name = name;
    newInfo.vol = vol;

    gs_bgmInfoMap[mapIndex] = newInfo;

    SPDLOG_INFO("bgm_info.add_info({0}, '{1}', {2})", mapIndex, name, vol);
}

const BGMInfo &CHARACTER_GetBGMInfo(unsigned mapIndex)
{
    BGMInfoMap::iterator f = gs_bgmInfoMap.find(mapIndex);
    if (gs_bgmInfoMap.end() == f)
    {
        static BGMInfo s_empty = {"", 0.0f};
        return s_empty;
    }
    return f->second;
}

bool CHARACTER_IsBGMVolumeEnable() { return gs_bgmVolEnable; }

// END_OF_BGM_INFO

void CHARACTER::MainCharacterPacket()
{
    GcMainCharacterPacket pack = {};
    pack.dwVID = m_vid;
    pack.wRaceNum = GetRaceNum();
    pack.mapIndex = GetMapIndex();
    if (pack.mapIndex >= 10000)
        pack.mapIndex /= 10000;
    pack.lx = GetX();
    pack.ly = GetY();
    pack.lz = GetZ();
    pack.blockMode = m_points.blockMode;
    pack.empire = GetEmpire();
    pack.skill_group = GetSkillGroup();
    pack.szName = GetName();
	pack.comboLevel = m_comboLevel;

    GetDesc()->Send(HEADER_GC_MAIN_CHARACTER, pack);
}

void CHARACTER::PointsPacket()
{
    const auto d = GetDesc();
    if (!d)
        return;

    TPacketGCPoints pack;
    for (int i = POINT_ST; i < POINT_MAX_NUM; ++i)
        pack.points[i] = GetPoint(i);

    pack.points[POINT_LEVEL] = GetLevel();
    pack.points[POINT_EXP] = GetExp();
    pack.points[POINT_NEXT_EXP] = GetNextExp();
    pack.points[POINT_HP] = GetHP();
    pack.points[POINT_MAX_HP] = GetMaxHP();
    pack.points[POINT_SP] = GetSP();
    pack.points[POINT_MAX_SP] = GetMaxSP();
    pack.points[POINT_STAMINA] = GetStamina();
    pack.points[POINT_MAX_STAMINA] = GetMaxStamina();
    pack.points[POINT_CASH] = GetCash("cash");

    pack.points[POINT_ATT_SPEED] = GetPoint(POINT_ATT_SPEED);
    pack.points[POINT_MOV_SPEED] = GetPoint(POINT_MOV_SPEED);
    pack.points[POINT_MALL_ATTBONUS] = GetPoint(POINT_MALL_ATTBONUS);
    pack.points[POINT_MALL_DEFBONUS] = GetPoint(POINT_MALL_DEFBONUS);

    GetDesc()->Send(HEADER_GC_CHARACTER_POINTS, pack);

    TPacketGCGold packGold;
    packGold.gold = GetGold();
    GetDesc()->Send(HEADER_GC_CHARACTER_GOLD, packGold);

#ifdef ENABLE_GEM_SYSTEM
    TPacketGCGem packGem;
    packGem.header = HEADER_GC_CHARACTER_GEM;
    packGem.gem = GetGem();
    GetDesc()->Packet(&packGem, sizeof(TPacketGCGem));
#endif
}

bool CHARACTER::ChangeSex()
{
    int src_race = GetRaceNum();

    switch (src_race)
    {
    case MAIN_RACE_WARRIOR_M:
        m_points.job = MAIN_RACE_WARRIOR_W;
        break;

    case MAIN_RACE_WARRIOR_W:
        m_points.job = MAIN_RACE_WARRIOR_M;
        break;

    case MAIN_RACE_ASSASSIN_M:
        m_points.job = MAIN_RACE_ASSASSIN_W;
        break;

    case MAIN_RACE_ASSASSIN_W:
        m_points.job = MAIN_RACE_ASSASSIN_M;
        break;

    case MAIN_RACE_SURA_M:
        m_points.job = MAIN_RACE_SURA_W;
        break;

    case MAIN_RACE_SURA_W:
        m_points.job = MAIN_RACE_SURA_M;
        break;

    case MAIN_RACE_SHAMAN_M:
        m_points.job = MAIN_RACE_SHAMAN_W;
        break;

    case MAIN_RACE_SHAMAN_W:
        m_points.job = MAIN_RACE_SHAMAN_M;
        break;

    case MAIN_RACE_WOLFMAN_M:
        m_points.job = MAIN_RACE_WOLFMAN_M;
        break;

    default:
        SPDLOG_ERROR("CHANGE_SEX: {0} unknown race {1}", GetName(), src_race);
        return false;
    }

    SPDLOG_INFO("CHANGE_SEX: {0} ({1} -> {2})", GetName(), src_race, m_points.job);

    UpdatePacket();
    SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
    ViewReencode();
    REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

    return true;
}

uint32_t CHARACTER::GetRaceNum() const
{
    if (m_dwPolymorphRace)
        return m_dwPolymorphRace;

    if (m_pkMobData) { return m_pkMobData->dwVnum; }

    return m_points.job;
}

uint16_t CHARACTER::GetRealRaceNum() const
{
    if (m_pkMobData)
        return m_pkMobData->dwVnum;

    return m_points.job;
}

void CHARACTER::SetRace(uint8_t race)
{
    if (race >= MAIN_RACE_MAX_NUM)
    {
        SPDLOG_ERROR("CHARACTER::SetRace(name={0}, race={1}).OUT_OF_RACE_RANGE", GetName(), race);
        return;
    }

    m_points.job = race;
}

uint8_t CHARACTER::GetJob() const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->GetJob();
#endif

    unsigned race = m_points.job;
    auto job = GetJobByRace(race);
    if (job != JOB_MAX_NUM)
        return job;

    SPDLOG_ERROR("CHARACTER::GetJob(name={0}, race={1}).OUT_OF_RACE_RANGE", GetName(), race);
    return JOB_WARRIOR;
}

void CHARACTER::SetLevel(uint8_t level)
{
    m_points.level = level;
#if defined(WJ_COMBAT_ZONE)
    if (IsPC() && CCombatZoneManager::Instance().IsCombatZoneMap(GetMapIndex()))
        return;
#endif
    if (IsPC())
    {
        if (level < gConfig.pkProtectLevel)
            SetPKMode(PK_MODE_PROTECT);
        else if (!gConfig.testServer && GetGMLevel() != GM_PLAYER)
            SetPKMode(PK_MODE_PROTECT);
        else if (m_bPKMode == PK_MODE_PROTECT)
            SetPKMode(PK_MODE_PEACE);
    }
}

void CHARACTER::SetEmpire(uint8_t bEmpire) { m_bEmpire = bEmpire; }

void CHARACTER::SetPlayerProto(const TPlayerTable *t)
{
    if (!GetDesc() || GetDesc()->GetHostName().empty())
        SPDLOG_ERROR("cannot get desc or hostname");
    else
        SetGMLevel();

    m_bCharType = CHAR_TYPE_PC;

    SetAccountID(GetDesc()->GetAid());
    SetPlayerID(t->id);

    m_bEmpire = t->empire;

    m_iAlignment = t->lAlignment;
    m_iRealAlignment = t->lAlignment;

    m_points.voice = t->voice;

    m_points.skill_group = t->skill_group;
    m_points.blockMode = t->blockMode;
    m_points.chatFilter = t->chatFilter;

    m_pointsInstant.bBasePart = t->part_base;
    SetPart(PART_HAIR, t->parts[PART_HAIR]);
    SetPart(PART_ACCE, t->parts[PART_ACCE]);

    m_points.iRandomHP = t->sRandomHP;
    m_points.iRandomSP = t->sRandomSP;

    // REMOVE_REAL_SKILL_LEVLES

    m_pSkillLevels = std::make_unique<TPlayerSkill[]>(SKILL_MAX_NUM);
    std::copy(std::begin(t->skills), std::end(t->skills), m_pSkillLevels.get());

    // std::memcpy(m_pSkillLevels.get(), t->skills, sizeof(TPlayerSkill) * SKILL_MAX_NUM);

#ifdef ENABLE_BATTLE_PASS
    m_dwBattlePassEndTime = t->dwBattlePassEndTime;
#endif

    m_playerTitle = t->title;
    // END_OF_REMOVE_REAL_SKILL_LEVLES

    if (t->lMapIndex >= 10000)
    {
        m_posWarp.x = t->lExitX;
        m_posWarp.y = t->lExitY;
        m_lWarpMapIndex = t->lExitMapIndex;
    }

    m_points.playtime = t->playtime;
    m_dwLoginPlayTime = t->playtime;
    SetRealPoint(POINT_ST, t->st);
    SetRealPoint(POINT_HT, t->ht);
    SetRealPoint(POINT_DX, t->dx);
    SetRealPoint(POINT_IQ, t->iq);

    SetPoint(POINT_ST, t->st);
    SetPoint(POINT_HT, t->ht);
    SetPoint(POINT_DX, t->dx);
    SetPoint(POINT_IQ, t->iq);

    SetPoint(POINT_STAT, t->stat_point);
    SetPoint(POINT_SKILL, t->skill_point);
    SetPoint(POINT_SUB_SKILL, t->sub_skill_point);
    SetPoint(POINT_HORSE_SKILL, t->horse_skill_point);
    SetPoint(POINT_SKILLTREE_POINTS, t->tree_skill_point);

    SetPoint(POINT_LEVEL_STEP, t->level_step);
    m_points.levelStep = t->level_step;

    SetRace((uint8_t)t->job);

    SetLevel(t->level);
    SetExp(t->exp);
    SetGold(t->gold);

#ifdef ENABLE_GEM_SYSTEM
    SetGem(t->gem);
#endif

    SetMapIndex(t->lMapIndex);
    SetXYZ(t->x, t->y, t->z);

    ComputePoints();

    SetHP(t->hp);
    SetSP(t->sp);
    SetStamina(t->stamina);

    // GM Effect, dont show it on test servers
    if (!gConfig.testServer && GetGMLevel() > GM_LOW_WIZARD)
    {
        switch (GetGMLevel())
        {
        case GM_WIZARD:
            SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_GM);
            break;
        case GM_HIGH_WIZARD:
            SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SGM);
            break;
        case GM_GOD:
            SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SA);
            break;
        case GM_IMPLEMENTOR:
            SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SA);
            break;
        default:
            SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_GM);
            break;
        }

        m_bPKMode = PK_MODE_PROTECT;
    }

    if (GetLevel() < gConfig.pkProtectLevel)
        m_bPKMode = PK_MODE_PROTECT;

    SetHorseData(t->horse);

    if (GetHorseLevel() > 0)
        UpdateHorseDataByLogoff(t->logoff_interval);

    m_dwLogOffInterval = t->logoff_interval;
    m_dwLastAttackTime = t->last_play;
    m_dwLastPlay = t->last_play;
#ifdef __FAKE_PC__
    m_stFakePCName = t->fakepc_name;
#endif


    if (GetGMLevel() != GM_PLAYER)
    {
        LogManager::instance().CharLog(this, GetGMLevel(), "GM_LOGIN", "");
        SPDLOG_INFO("GM_LOGIN(gmlevel={}, name={}({}), pos=({}, {})", GetGMLevel(), GetName(), GetPlayerID(), GetX(),
                    GetY());
    }

#ifdef __PET_SYSTEM__
    m_petSystem = std::make_unique<CPetSystem>(*this);
#endif

#ifdef __OFFLINE_SHOP__
    this->m_pkMyOfflineShop = COfflineShopManager::instance().FindOfflineShop(this->GetPlayerID(), true);

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    this->SetOfflineShopFarmedOpeningTime(t->iOfflineShopFarmedOpeningTime);
    this->SetOfflineShopSpecialOpeningTime(t->iOfflineShopSpecialOpeningTime);
#endif
#endif
}

void CHARACTER::SetTitle(const std::string &title, uint32_t color)
{
    storm::CopyStringSafe(m_playerTitle.title, title);
    m_playerTitle.color = color;

    UpdatePacket();
}

void CHARACTER::DelayedItemLoad()
{
    for (auto &item : m_pointsInstant.wear)
        if (item)
            item->UpdatePacket();

    for (auto &item : m_pointsInstant.switchbot)
        if (item)
            item->UpdatePacket();

    for (auto &item : m_pointsInstant.pDSItems)
        if (item)
            item->UpdatePacket();

    for (int i = 0; i < m_windows.inventory.GetSize(); ++i)
        if (auto item = m_windows.inventory.GetUnique(i); item)
            item->UpdatePacket();
}

EVENTFUNC(despawn_event)
{
    char_event_info *info = static_cast<char_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_ERROR("despawn_event Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;
    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }

    ch->m_despawnEvent = nullptr;
    M2_DESTROY_CHARACTER(ch);
    return 0;
}

void CHARACTER::SetStateHorse()
{
    m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
    m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
    m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
}

void CHARACTER::SetStateEmpty()
{
    m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateIdle, &CHARACTER::EndStateEmpty);
    m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateIdle, &CHARACTER::EndStateEmpty);
    m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateIdle, &CHARACTER::EndStateEmpty);
}

int32_t GetItemArmorValue(const TItemTable *p)
{
    if (p->bType != ITEM_ARMOR)
        return 0;

    switch (p->bSubType)
    {
    case ARMOR_BODY:
    case ARMOR_HEAD:
    case ARMOR_FOOTS:
    case ARMOR_SHIELD:
        return p->alValues[ITEM_VALUE_ARMOR_DEF_GRADE] + 2 * p->alValues[ITEM_VALUE_ARMOR_DEF_BONUS];
    }

    return 0;
}

void CHARACTER::ComputeBattlePoints()
{
    if (IsPolymorphed())
    {
        uint32_t dwMobVnum = GetPolymorphVnum();
        const auto *pMob = CMobManager::instance().Get(dwMobVnum);
        int iAtt = 0;
        int iDef = 0;

        if (pMob)
        {
            iAtt = GetLevel() * 2;
            // lev + con
            iDef = GetLevel() + GetPolymorphPoint(POINT_HT) + pMob->wDef;
        }

        SetPoint(POINT_ATT_GRADE, iAtt);
        SetPoint(POINT_DEF_GRADE, iDef);
        SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
        SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));
    }
#ifdef __FAKE_PC__
    else if (IsPC() || FakePC_Check())
#else
    else if (IsPC())
#endif
    {
        SetPoint(POINT_ATT_GRADE, 0);
        SetPoint(POINT_DEF_GRADE, 0);
        SetPoint(POINT_CLIENT_DEF_GRADE, 0);
        SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
        SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));

        //
        // 기본 ATK = 2lev + 2str, 직업에 마다 2str은 바뀔 수 있음
        //
        int iAtk = GetLevel() * 2;
        PointValue iStatAtk = 0;

        switch (GetJob())
        {
        case JOB_WARRIOR:
            iStatAtk = (2 * GetPoint(POINT_HT) + (4 * GetPoint(POINT_ST))) / 3;;
            break;

        case JOB_SURA:
            iStatAtk = (2 * GetPoint(POINT_DX) + (4 * GetPoint(POINT_IQ))) / 3;;
            break;

        case JOB_ASSASSIN:
            iStatAtk = (2 * GetPoint(POINT_ST) + (4 * GetPoint(POINT_DX))) / 3;
            break;

        case JOB_SHAMAN:
            iStatAtk = (2 * GetPoint(POINT_DX) + 4 * GetPoint(POINT_IQ)) / 3;
            break;

        case JOB_WOLFMAN:
            iStatAtk = (4 * GetPoint(POINT_DX) + 2 * GetPoint(POINT_HT)) / 2;
            break;
        default:
            SPDLOG_ERROR("Invalid job {0} using default", GetJob());
            iStatAtk = (2 * GetPoint(POINT_ST));
            break;
        }

        iAtk += iStatAtk;
        //
        // ATK Setting
        //
        PointChange(POINT_ATT_GRADE, iAtk);

        // DEF = LEV + CON + ARMOR
        int iShowDef = GetLevel() + GetPoint(POINT_HT);           // For Ymir(천마)
        int iDef = GetLevel() + (int)(GetPoint(POINT_HT) / 1.25); // For Other
        int iArmor = 0;

        for (int i = 0; i < WEAR_MAX_NUM; ++i)
        {
            const auto item = GetWear(i);
            if (!item || item->GetItemType() != ITEM_ARMOR)
                continue;

            iArmor += GetItemArmorValue(item->GetProto());
        }

        const auto sash = GetWear(WEAR_COSTUME_ACCE);
        if (sash && sash->GetSocket(1) != 0)
        {
            const auto vnum = sash->GetSocket(1);
            const auto f = sash->GetSocket(0);

            const auto p = ITEM_MANAGER::instance().GetTable(vnum);
            if (p)
                iArmor += std::max<int32_t>(0, GetItemArmorValue(p) * f / 100);
        }

        iArmor += GetPoint(POINT_DEF_GRADE_BONUS);
#ifndef SHELIA_BUILD
        iArmor += GetPoint(POINT_PARTY_DEFENDER_BONUS);
#endif
        PointChange(POINT_DEF_GRADE, iDef + iArmor);
        PointChange(POINT_CLIENT_DEF_GRADE, (iShowDef + iArmor) - GetPoint(POINT_DEF_GRADE));
        PointChange(POINT_MAGIC_ATT_GRADE,
                    GetLevel() * 2 + GetPoint(POINT_IQ) * 2 + GetPoint(POINT_MAGIC_ATT_GRADE_BONUS));
        PointChange(POINT_MAGIC_DEF_GRADE, GetLevel() + (GetPoint(POINT_IQ) * 3 + GetPoint(POINT_HT)) / 3 + iArmor / 2 +
                                           GetPoint(POINT_MAGIC_DEF_GRADE_BONUS));
    }
    else
    {
        // 2lev + str * 2
        int iAtt = GetLevel() * 2 + GetPoint(POINT_ST) * 2;
        // lev + con
        int iDef = GetLevel() + GetPoint(POINT_HT) + GetMobTable().wDef;

        SetPoint(POINT_ATT_GRADE, iAtt);
        SetPoint(POINT_DEF_GRADE, iDef);
        SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
        SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));
    }
}

void CHARACTER::ComputePoints()
{
    long lStat = GetPoint(POINT_STAT);
    long lStatResetCount = GetPoint(POINT_STAT_RESET_COUNT);
    long lSkillActive = GetPoint(POINT_SKILL);
    long lSkillSub = GetPoint(POINT_SUB_SKILL);
    long lSkillHorse = GetPoint(POINT_HORSE_SKILL);
    long lSkillTree = GetPoint(POINT_SKILLTREE_POINTS);
    long lLevelStep = GetPoint(POINT_LEVEL_STEP);

    long lAttackerBonus = GetPoint(POINT_PARTY_ATTACKER_BONUS);
    long lTankerBonus = GetPoint(POINT_PARTY_TANKER_BONUS);
#ifdef SHELIA_BUILD
    long lBufferBonus = GetPoint(POINT_PARTY_BLOCKER_BONUS);
#else
    long lBufferBonus = GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif
    long lSkillMasterBonus = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
#ifdef SHELIA_BUILD
    long lHalfHumanBonus = GetPoint(POINT_PARTY_DESTROYER_BONUS);
#else
    long lHasteBonus = GetPoint(POINT_PARTY_HASTE_BONUS);
#endif

    long lDefenderBonus = GetPoint(POINT_PARTY_DEFENDER_BONUS);

    long lHPRecovery = GetPoint(POINT_HP_RECOVERY);
    long lSPRecovery = GetPoint(POINT_SP_RECOVERY);

    m_pointsInstant.computed = false;

    memset(m_pointsInstant.points, 0, sizeof(m_pointsInstant.points));
    BuffOnAttr_ClearAll();
    m_SkillDamageBonus.clear();

    SetPoint(POINT_STAT, lStat);
    SetPoint(POINT_SKILL, lSkillActive);
    SetPoint(POINT_SUB_SKILL, lSkillSub);
    SetPoint(POINT_HORSE_SKILL, lSkillHorse);
    SetPoint(POINT_SKILLTREE_POINTS, lSkillTree);

    SetPoint(POINT_LEVEL_STEP, lLevelStep);
    SetPoint(POINT_STAT_RESET_COUNT, lStatResetCount);

    SetPoint(POINT_ST, m_points.st);
    SetPoint(POINT_HT, m_points.ht);
    SetPoint(POINT_DX, m_points.dx);
    SetPoint(POINT_IQ, m_points.iq);

    SetPart(PART_MAIN, GetOriginalPart(PART_MAIN));
    SetPart(PART_WEAPON, GetOriginalPart(PART_WEAPON));
    SetPart(PART_HEAD, GetOriginalPart(PART_HEAD));
    SetPart(PART_HAIR, GetOriginalPart(PART_HAIR));
    SetPart(PART_ACCE, GetOriginalPart(PART_ACCE));

    SetPoint(POINT_PARTY_ATTACKER_BONUS, lAttackerBonus);
    SetPoint(POINT_PARTY_TANKER_BONUS, lTankerBonus);
#ifdef SHELIA_BUILD
    SetPoint(POINT_PARTY_BLOCKER_BONUS, lBufferBonus);
    SetPoint(POINT_PARTY_DESTROYER_BONUS, lHalfHumanBonus);
#else
    SetPoint(POINT_PARTY_BUFFER_BONUS, lBufferBonus);
    SetPoint(POINT_PARTY_HASTE_BONUS, lHasteBonus);
#endif
    SetPoint(POINT_PARTY_SKILL_MASTER_BONUS, lSkillMasterBonus);
    SetPoint(POINT_PARTY_DEFENDER_BONUS, lDefenderBonus);

    SetPoint(POINT_HP_RECOVERY, lHPRecovery);
    SetPoint(POINT_SP_RECOVERY, lSPRecovery);

    // PC_BANG_ITEM_ADD
    SetPoint(POINT_PC_BANG_EXP_BONUS, 0);
    SetPoint(POINT_PC_BANG_DROP_BONUS, 0);
    // END_PC_BANG_ITEM_ADD

    int64_t iMaxHP, iMaxSP;
    int iMaxStamina;

    int baseStrongAgainstMob = 0;

#ifdef __FAKE_PC__
    if (IsPC() || FakePC_Check())
#else
    if (IsPC())
#endif
    {
        // 최대 생명력/정신력
        iMaxHP = JobInitialPoints[GetJob()].max_hp + m_points.iRandomHP +
                 GetPoint(POINT_HT) * JobInitialPoints[GetJob()].hp_per_ht;
        iMaxSP = JobInitialPoints[GetJob()].max_sp + m_points.iRandomSP +
                 GetPoint(POINT_IQ) * JobInitialPoints[GetJob()].sp_per_iq;
        iMaxStamina =
            JobInitialPoints[GetJob()].max_stamina + GetPoint(POINT_HT) * JobInitialPoints[GetJob()].stamina_per_con;

        {
            CSkillProto *pkSk = CSkillManager::instance().Get(SKILL_ADD_HP);

            if (nullptr != pkSk)
            {
                pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_ADD_HP) / 100.0f);

                iMaxHP += static_cast<int>(pkSk->kPointPoly.Eval());
            }
        }

        {
            CSkillProto *pkSk = CSkillManager::instance().Get(SKILL_ADD_MONSTER);

            if (nullptr != pkSk)
            {
                pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_ADD_MONSTER) / 100.0f);

                baseStrongAgainstMob = static_cast<int>(pkSk->kPointPoly.Eval());
            }
        }

        // 기본 값들
        SetPoint(POINT_MOV_SPEED, 200);
        SetPoint(POINT_ATT_SPEED, 130);
#ifndef SHELIA_BUILD
        PointChange(POINT_ATT_SPEED, GetPoint(POINT_PARTY_HASTE_BONUS));
#endif
        SetPoint(POINT_CASTING_SPEED, 100);
        SetPoint(POINT_ATTBONUS_MONSTER, baseStrongAgainstMob);

        // TODO: We probably should add something like SKILL_TYPE_PASSIVE to do this automatically
        for (int i = 180; i <= 203; ++i) { ComputePassiveSkill(i); }
    }
    else
    {
        iMaxHP = m_pkMobData->dwMaxHP;
        iMaxSP = 0;
        iMaxStamina = 0;

        SetPoint(POINT_ATT_SPEED, m_pkMobData->sAttackSpeed);
        SetPoint(POINT_MOV_SPEED, m_pkMobData->sMovingSpeed);
        SetPoint(POINT_CASTING_SPEED, m_pkMobData->sAttackSpeed);
        SetPoint(POINT_ATTBONUS_MONSTER, baseStrongAgainstMob);
    }

    ComputeBattlePoints();

    if (iMaxHP != GetMaxHP())
        m_points.maxHp = iMaxHP;

    PointChange(POINT_MAX_HP, 0);

    if (iMaxSP != GetMaxSP())
        m_points.maxSp = iMaxSP;

    PointChange(POINT_MAX_SP, 0);

    SetMaxStamina(iMaxStamina);

    // Equipped items
    for (uint32_t i = 0; i < DRAGON_SOUL_EQUIP_SLOT_START; ++i)
    {
         auto* item = GetWear(i);
        if (!item)
            continue;

        item->ModifyPoints(true);
    }

    for (uint32_t i = 0; i < INVENTORY_PAGE_SIZE * NORMAL_INVENTORY_MAX_PAGE; ++i)
    {
        auto* item = GetInventoryItem(i);
        if (!item || item->GetItemType() != ITEM_TOGGLE)
            continue;

        if (!item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE))
            continue;

        if (item->GetSubType() != TOGGLE_MOUNT)
            item->ModifyPoints(true);
    }

    // 용혼석 시스템
    // ComputePoints에서는 케릭터의 모든 속성값을 초기화하고,
    // 아이템, 버프 등에 관련된 모든 속성값을 재계산하기 때문에,
    // 용혼석 시스템도 ActiveDeck에 있는 모든 용혼석의 속성값을 다시 적용시켜야 한다.
    if (DragonSoul_IsDeckActivated())
    {
        for (int i = DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * DragonSoul_GetActiveDeck();
             i < DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * (DragonSoul_GetActiveDeck() + 1); i++)
        {
            auto pItem = GetWear(i);
            if (pItem)
            {
                if (DSManager::instance().IsTimeLeftDragonSoul(pItem))
                    pItem->ModifyPoints(true);
            }
        }
    }
    DragonSoul_HandleSetBonus();

    RefreshAffect();

    CheckMaximumPoints();

    m_pointsInstant.computed = true;
    UpdatePacket();
    // PointsPacket();

#ifdef __FAKE_PC__
    if (IsPC())
        FakePC_Owner_ExecFunc(&CHARACTER::ComputePoints);
#endif
}

// m_dwPlayStartTime의 단위는 milisecond다. 데이터베이스에는 분단위로 기록하기
// 때문에 플레이시간을 계산할 때 / 60000 으로 나눠서 하는데, 그 나머지 값이 남았
// 을 때 여기에 dwTimeRemain으로 넣어서 제대로 계산되도록 해주어야 한다.
void CHARACTER::ResetPlayTime(uint32_t dwTimeRemain) { m_dwPlayStartTime = get_dword_time() - dwTimeRemain; }

const int aiRecoveryPercents[10] = {1, 2, 2, 2, 2, 3, 3, 3, 3, 5};

EVENTFUNC(recovery_event)
{
    char_event_info *info = static_cast<char_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_ERROR("recovery_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;

    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }

    if (!ch->IsPC())
    {
        if (!ch->CanRegenHP())
            return 0;

        if (IS_SET(ch->GetMobTable().dwAIFlag, AIFLAG_NORECOVERY))
        {
            ch->m_pkRecoveryEvent = nullptr;
            return 0;
        }

        //
        // 몬스터 회복
        //
        if (ch->FindAffect(AFFECT_POISON))
            return THECORE_SECS_TO_PASSES(std::max<int>(1, ch->GetMobTable().bRegenCycle));

        uint32_t vnum = ch->GetMobTable().dwVnum;
        if (vnum == 2493) // Aqua Dragon
        {
            int regenPct = BlueDragon_GetRangeFactor("hp_regen", ch->GetHPPct());
            regenPct += ch->GetMobTable().bRegenPercent;

            for (int i = 1; i <= 4; ++i)
            {
                if (REGEN_PECT_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
                {
                    uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
                    size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
                    size_t cnt = SECTREE_MANAGER::instance().GetMonsterCountInMap(ch->GetMapIndex(), dwDragonStoneID);

                    regenPct += (val * cnt);

                    break;
                }
            }

            ch->MonsterLog("AQUA_HP_REGEN +%d", std::max<int64_t>(1, (ch->GetMaxHP() * regenPct) / 100));
            ch->PointChange(POINT_HP, std::max<int64_t>(1, (ch->GetMaxHP() * regenPct) / 100));
        }

        else if (!ch->IsDoor())
        {
            ch->MonsterLog(
                "HP_REGEN +%d",
                std::max<int>(1, (ch->GetMaxHP() * std::min<PointValue>(100, ch->GetMobTable().bRegenPercent +
                                                                             ch->GetPoint(POINT_HP_REGEN))) /
                                 100));
            ch->PointChange(POINT_HP, std::max<int64_t>(1, (ch->GetMaxHP() *
                                                        std::min<PointValue>(100, ch->GetMobTable().bRegenPercent +
                                                                                 ch->GetPoint(POINT_HP_REGEN))) /
                                                       100));
        }

        if (ch->GetHP() >= ch->GetMaxHP())
        {
            ch->m_pkRecoveryEvent = nullptr;
            return 0;
        }

        if (2493 == ch->GetMobTable().dwVnum)
        {
            for (int i = 1; i <= 4; ++i)
            {
                if (REGEN_TIME_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
                {
                    uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
                    size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
                    size_t cnt = SECTREE_MANAGER::instance().GetMonsterCountInMap(ch->GetMapIndex(), dwDragonStoneID);

                    return THECORE_SECS_TO_PASSES(std::max<int>(1, (ch->GetMobTable().bRegenCycle - (val * cnt))));
                }
            }
        }

        return THECORE_SECS_TO_PASSES(std::max<int>(1, ch->GetMobTable().bRegenCycle + ch->GetPenaltyRegenTime()));
    }
    else
    {
        //
        // PC 회복
        //
        ch->CheckTarget();
        // ch->UpdateSectree(); // 여기서 이걸 왜하지?
        ch->UpdateKillerMode();

        if (ch->FindAffect(AFFECT_POISON))
            return THECORE_SECS_TO_PASSES(3);

        if (ch->FindAffect(AFFECT_NO_RECOVERY))
            return THECORE_SECS_TO_PASSES(3);

        int iSec = (get_dword_time() - ch->GetLastMoveTime()) / 3000;

        // SP 회복 루틴.
        // 왜 이걸로 해서 함수로 빼놨는가 ?!
        ch->DistributeSP(ch);

        if (ch->GetMaxHP() <= ch->GetHP())
            return THECORE_SECS_TO_PASSES(3);

        int iPercent = 0;
        int64_t iAmount = 0;

        {
            iPercent = aiRecoveryPercents[std::min(9, iSec)];
            iAmount = 15 + (ch->GetMaxHP() * iPercent) / 100;
        }

        iAmount += iAmount * (ch->GetPoint(POINT_HP_REGEN) * 0.66) / 100;

        SPDLOG_DEBUG("RECOVERY_EVENT: {0} {1} HP_REGEN {2} HP +{3}", ch->GetName(), iPercent,
                      ch->GetPoint(POINT_HP_REGEN), iAmount);

        ch->PointChange(POINT_HP, iAmount, false);
        return THECORE_SECS_TO_PASSES(3);
    }
}

void CHARACTER::StartRecoveryEvent()
{
    if (m_pkRecoveryEvent)
        return;

    if (IsDead() || IsStun())
        return;

    if (IsNPC() && GetHP() >= GetMaxHP()) // 몬스터는 체력이 다 차있으면 시작 안한다.
        return;

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if (!IsPC())
    {
        if (IS_SET(GetAIFlag(), AIFLAG_NORECOVERY))
            return;
    }
#endif

    char_event_info *info = AllocEventInfo<char_event_info>();

    info->ch = this;

    int iSec = IsPC() ? 3 : (std::max<int>(1, GetMobTable().bRegenCycle + GetPenaltyRegenTime()));
    m_pkRecoveryEvent = event_create(recovery_event, info, THECORE_SECS_TO_PASSES(iSec));
}

void CHARACTER::SetRotation(float fRot) { m_pointsInstant.fRot = fRot; }

// x, y 방향으로 보고 선다.
void CHARACTER::SetRotationToXY(long x, long y) { SetRotation(GetDegreeFromPositionXY(GetX(), GetY(), x, y)); }

bool CHARACTER::CannotMoveByAffect() const { return FindAffect(AFFECT_STUN); }

bool CHARACTER::CanMove() const
{
    if (CannotMoveByAffect())
        return false;

    if (IsShop())
        return false;

    // It can not move if it is 0.2 seconds ago.
    /*if(!IsDuelMap(GetMapIndex())) {
        if (get_float_time() - m_fSyncTime < 200.0f)
            return false;
    }*/

    return true;
}

bool CHARACTER::Sync(int32_t x, int32_t y)
{
    if (!GetSectree())
        return false;

    if (IsPC() && IsDead()) { return false; }

    SECTREE *new_tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), x, y);
    if (!new_tree)
        return false;

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if (!IsPC())
    {
        if (IS_SET(GetAIFlag(), AIFLAG_NOMOVE))
            return false;
    }
#endif

    SetRotationToXY(x, y);
    SetXYZ(x, y, 0);

    if (GetDungeon())
    {
        int iLastEventAttr = m_iEventAttr;
        m_iEventAttr = new_tree->GetEventAttribute(x, y);

        if (m_iEventAttr != iLastEventAttr)
        {
            if (GetParty())
            {
                quest::CQuestManager::instance().AttrOut(GetParty()->GetLeaderPID(), this, iLastEventAttr);
                quest::CQuestManager::instance().AttrIn(GetParty()->GetLeaderPID(), this, m_iEventAttr);
            }
            else
            {
                quest::CQuestManager::instance().AttrOut(GetPlayerID(), this, iLastEventAttr);
                quest::CQuestManager::instance().AttrIn(GetPlayerID(), this, m_iEventAttr);
            }
        }
    }

    if (GetSectree() != new_tree)
    {
        // Despite the name this takes care of unlinking us from our
        // current sectree, updating our m_pSectree member, etc.
        // *sigh*
        new_tree->InsertEntity(this);
    }

    UpdateSectree();
    return true;
}

void CHARACTER::Stop()
{
    if (!IsState(m_stateIdle))
        MonsterLog("[IDLE] I do not have an object.");

    GotoState(m_stateIdle);

    m_dwStateDuration = 1;

    m_posDest.x = m_posStart.x = GetX();
    m_posDest.y = m_posStart.y = GetY();

    m_motionKey = 0;
    m_motion = nullptr;
    m_motionStartTime = 0;
    m_motionDuration = 0;
}

bool CHARACTER::Goto(int32_t x, int32_t y)
{
    // Already moving...
    if (m_posDest.x == x && m_posDest.y == y && IsState(m_stateMove))
        return false;

    return Goto(get_dword_time(), x, y);
}

bool CHARACTER::ForceGoto(int32_t x, int32_t y)
{
    const auto m = GetMoveMotion();
    if (!m)
        return false;

    if (m_posDest.x == x && m_posDest.y == y)
    {
        if (!IsState(m_stateMove))
        {
            m_dwStateDuration = 4;
            GotoState(m_stateMove);
        }
        return false;
    }

    m_posStart = GetXYZ();
    m_posDest.x = x;
    m_posDest.y = y;

    m_motionKey = m->key;
    m_motion = m;
    m_motionStartTime = get_dword_time();
    m_motionDuration = CalculateMoveDuration();

    if (!IsState(m_stateMove))
    {
        MonsterLog("[MOVE] %s", GetVictim() ? "Target Tracking" : "Just go");

        if (GetVictim())
        {
            // MonsterChat(MONSTER_CHAT_CHASE);
            MonsterChat(MONSTER_CHAT_ATTACK);
        }

        m_dwStateDuration = 4;
        GotoState(m_stateMove);
    }

    if (IsPC()) { Update(); }
    else { SendMovePacket(FUNC_WAIT, 0, 0, 0, 0); }

    return true;
}

bool CHARACTER::Goto(uint32_t startTime, int32_t x, int32_t y)
{
    if (GetX() == x && GetY() == y)
        return false;

    const auto m = GetMoveMotion();
    if (!m)
        return false;

    if (m_posDest.x == x && m_posDest.y == y)
    {
        if (!IsState(m_stateMove))
        {
            m_dwStateDuration = 4;
            GotoState(m_stateMove);
        }
        return false;
    }

    m_posStart = GetXYZ();
    m_posDest.x = x;
    m_posDest.y = y;

    m_motionKey = m->key;
    m_motion = m;
    m_motionStartTime = startTime;
    m_motionDuration = CalculateMoveDuration();

    if (!IsState(m_stateMove))
    {
        MonsterLog("[MOVE] %s", GetVictim() ? "Target Tracking" : "Just go");

        if (GetVictim())
        {
            // MonsterChat(MONSTER_CHAT_CHASE);
            MonsterChat(MONSTER_CHAT_ATTACK);
        }

        m_dwStateDuration = 4;
        GotoState(m_stateMove);
    }

    if (IsPC()) { Update(); }
    else { SendMovePacket(FUNC_WAIT, 0, 0, 0, 0); }

    return true;
}

PIXEL_POSITION CHARACTER::GetInterpolatedPosition(uint32_t time) const
{
    if (m_posDest.x == GetX() && m_posDest.y == GetY())
        return GetXYZ();

    if (m_motionDuration == 0)
        return GetXYZ();

    // No data for that much in the past...
    if (time < m_motionStartTime)
        return GetXYZ();

    // We need these as signed ints (due to our position deltas below)
    const int32_t elapsed = std::min(time - m_motionStartTime, m_motionDuration);

    const int32_t dur = static_cast<int32_t>(m_motionDuration);

    int32_t dX = (m_posDest.x - m_posStart.x) * elapsed / dur;
    int32_t dY = (m_posDest.y - m_posStart.y) * elapsed / dur;

    return {m_posStart.x + dX, m_posStart.y + dY, 0};
}

uint32_t CHARACTER::GetMotionMode() const
{
    if (IsPolymorphed())
        return MOTION_MODE_GENERAL;

    switch (GetPart(PART_MAIN).vnum)
    {
    case 11903:
    case 11904:
    case 11913:
    case 11914:
    case 11901:
    case 11902:
    case 11911:
    case 11912:
        return MOTION_MODE_WEDDING_DRESS;
    default:
        break;
    }

    const auto weapon = GetWear(WEAR_WEAPON);

    if (weapon && weapon->GetItemType() == ITEM_WEAPON)
    {
#define MAKE_MOTION_MODE(name) IsRiding() ? MOTION_MODE_HORSE_##name : MOTION_MODE_##name

        switch (weapon->GetSubType())
        {
        case WEAPON_SWORD:
            return MAKE_MOTION_MODE(ONEHAND_SWORD);
        case WEAPON_TWO_HANDED:
            return MAKE_MOTION_MODE(TWOHAND_SWORD);
        case WEAPON_DAGGER:
            return MAKE_MOTION_MODE(DUALHAND_SWORD);
        case WEAPON_BOW:
            return MAKE_MOTION_MODE(BOW);
        case WEAPON_BELL:
            return MAKE_MOTION_MODE(BELL);
        case WEAPON_FAN:
            return MAKE_MOTION_MODE(FAN);
        case WEAPON_CLAW:
            return MAKE_MOTION_MODE(CLAW);
        }

#undef MAKE_MOTION_MODE
    }

    if (weapon && weapon->GetItemType() == ITEM_ROD && !IsRiding())
        return MOTION_MODE_FISHING;

    return IsRiding() ? MOTION_MODE_HORSE : MOTION_MODE_GENERAL;
}

const Motion *CHARACTER::GetMoveMotion(bool forceRun) const
{
    uint32_t mode = GetMotionMode();
    uint32_t index = (IsWalking() && !forceRun) ? MOTION_WALK : MOTION_RUN;
    uint32_t key = MakeMotionKey(mode, index);

    auto &motionManager = GetMotionManager();
    const Motion *motion = nullptr;

    if (!GetMountVnum())
    {
        motion = motionManager.Get(GetRaceNum(), key);

        if (!motion && index == MOTION_WALK && !IsPC())
        {
            index = MOTION_RUN;
            key = MakeMotionKey(mode, index);
            motion = motionManager.Get(GetRaceNum(), key);
        }
    }
    else
    {
        key = MakeMotionKey(MOTION_MODE_GENERAL, index);
        motion = motionManager.Get(GetMountVnum(), key);

        if (!motion)
        {
            key = MakeMotionKey(MOTION_MODE_HORSE, index);
            motion = motionManager.Get(GetRaceNum(), key);
        }
    }

    if (IsBuffBot()) {
        return false;
    }

    if (motion)
        return motion;

    SPDLOG_ERROR("cannot find motion (name {0} race {1} mode {2} index {3})", GetName(), GetRaceNum(), mode, index);
    return nullptr;
}

float CHARACTER::GetMoveSpeed() const
{
    static const int kIntScale = 10000;
    const int scale = CalculateDuration(GetLimitPoint(POINT_MOV_SPEED), kIntScale);

    const auto motion = GetMoveMotion();
    if (motion)
        return motion->moveSpeed * kIntScale / scale;

    return 300.0f * kIntScale / scale;
}

uint32_t CHARACTER::CalculateMoveDuration()
{
    float fDist = DISTANCE_SQRT(m_posStart.x - m_posDest.x, m_posStart.y - m_posDest.y);

    if (!m_motion)
        return CalculateDuration(GetLimitPoint(POINT_MOV_SPEED), fDist / 300);

    return CalculateMotionMovementTime(this, m_motion, fDist);
}

bool CHARACTER::Move(int32_t x, int32_t y)
{
    if (GetX() == x && GetY() == y)
        return true;

    OnMove();
    return Sync(x, y);
}

void CHARACTER::SendMovePacket(uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y, uint32_t dwDuration,
                               uint32_t dwTime, int iRot)
{
    TPacketGCMove pack;

    if (bFunc == FUNC_WAIT)
    {
        x = m_posDest.x;
        y = m_posDest.y;
        dwDuration = m_motionDuration;
        dwTime = m_motionStartTime;
    }

    EncodeMovePacket(pack, GetVID(), bFunc, bArg, x, y, dwDuration, dwTime, iRot == -1 ? (int)GetRotation() / 5 : iRot);
    PacketView(m_map_view, static_cast<CEntity*>(this), HEADER_GC_CHARACTER_MOVE, pack, this);
}

void CHARACTER::SendBuffBotSkillPacket(uint32_t skill_vnum)
{
    TPacketGCNPCUseSkill pack;
    pack.dwVnum = skill_vnum;
    pack.dwVid = GetVID();
    pack.dwLevel = 41;

    PacketView(m_map_view, static_cast<CEntity*>(this), HEADER_GC_NPC_SKILL, pack);
}

PointValue CHARACTER::GetRealPoint(uint8_t type) const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->GetRealPoint(type);
#endif

    switch (type)
    {
    case POINT_LEVEL_STEP:
        return m_points.levelStep;
    case POINT_ST:
        return m_points.st;
    case POINT_HT:
        return m_points.ht;
    case POINT_DX:
        return m_points.dx;
    case POINT_IQ:
        return m_points.iq;
    case POINT_PLAYTIME:
        return m_points.playtime;

    default:
        SPDLOG_ERROR("Invalid real point type {0}", type);
        return 0;
    }
}

void CHARACTER::SetRealPoint(uint8_t type, PointValue val)
{
    switch (type)
    {
    case POINT_LEVEL_STEP:
        m_points.levelStep = val;
        break;
    case POINT_ST:
        m_points.st = val;
        break;
    case POINT_HT:
        m_points.ht = val;
        break;
    case POINT_DX:
        m_points.dx = val;
        break;
    case POINT_IQ:
        m_points.iq = val;
        break;
    case POINT_PLAYTIME:
        m_points.playtime = val;
        break;

    default:
        SPDLOG_ERROR("Invalid real point type {0}", type);
        return;
    }
}

PointValue CHARACTER::GetPolymorphPoint(uint8_t type) const
{
    if (IsPolymorphed() && !IsPolyMaintainStat())
    {
        uint32_t dwMobVnum = GetPolymorphVnum();
        const auto *pMob = CMobManager::instance().Get(dwMobVnum);
        int iPower = GetPolymorphPower();

        if (pMob)
        {
            switch (type)
            {
            case POINT_ST:
                if (GetJob() == JOB_SHAMAN || (GetJob() == JOB_SURA && GetSkillGroup() == 2))
                    return pMob->bStr * iPower / 200 + GetPoint(POINT_IQ);
                return pMob->bStr * iPower / 200 + GetPoint(POINT_ST);

            case POINT_HT:
                return pMob->bCon * iPower / 200 + GetPoint(POINT_HT);

            case POINT_IQ:
                return pMob->bInt * iPower / 200 + GetPoint(POINT_IQ);

            case POINT_DX:
                return pMob->bDex * iPower / 200 + GetPoint(POINT_DX);
            }
        }
    }

    return GetPoint(type);
}

PointValue CHARACTER::GetPoint(uint8_t type) const
{
    if (type >= POINT_MAX_NUM)
    {
        SPDLOG_ERROR("Point type overflow (type {0})", type);
        return 0;
    }

#ifdef __FAKE_PC__
    if (FakePC_Check())
    {
        const static uint8_t abTypesFromOwner[] = {
            POINT_PARTY_ATTACKER_BONUS, POINT_PARTY_TANKER_BONUS, POINT_PARTY_BUFFER_BONUS,
            POINT_PARTY_SKILL_MASTER_BONUS, POINT_PARTY_HASTE_BONUS, POINT_PARTY_DEFENDER_BONUS,
        };

        const uint8_t *pbFirstVal = &(abTypesFromOwner[0]);
        const uint8_t *pbLastVal = pbFirstVal + (sizeof(abTypesFromOwner) / sizeof(abTypesFromOwner[0]));

        const uint8_t *pbFind = std::find(pbFirstVal, pbLastVal, type);
        if (pbFind != pbLastVal)
            return FakePC_GetOwner()->GetPoint(type);
    }
#endif

    PointValue val = m_pointsInstant.points[type];
    PointValue max_val = std::numeric_limits<PointValue>::max();

    switch (type)
    {
    case POINT_STEAL_HP:
    case POINT_STEAL_SP:
        max_val = 60;
        break;
    }

    if (val > max_val)
        SPDLOG_ERROR("{0} has point {1} with value {2} the maximum is: {3}", GetName(), type, val, max_val);

    return (val);
}

PointValue CHARACTER::GetLimitPoint(uint8_t type) const
{
    if (type >= POINT_MAX_NUM)
    {
        SPDLOG_ERROR("Point type overflow (type {0})", type);
        return 0;
    }

    auto val = m_pointsInstant.points[type];
    auto max_val = std::numeric_limits<PointValue>::max();
    auto limit = max_val;
    auto min_limit = std::numeric_limits<PointValue>::min();
    auto mountVnum = GetMountVnum();

    switch (type)
    {
    case POINT_ATT_SPEED:
        min_limit = 1;

        if (IsPC())
            limit = gConfig.pcMaxAttackSpeed;
        else
            limit = 250;
        break;

    case POINT_MOV_SPEED: {
        min_limit = 1;

        const auto &mobMan = CMobManager::instance();

        if (IsPC())
        {
            if (IsGM())
            {
                if (mobMan.MountHasSpeedLimit(mountVnum)) { limit = mobMan.GetMountSpeedLimit(mountVnum); }
                else { limit = gConfig.gmMaxMoveSpeed; }
            }
            else
            {
                if (!mountVnum) { limit = gConfig.pcMaxMoveSpeed; }
                else
                {
                    if (!mobMan.MountHasSpeedLimit(mountVnum))
                    {
                        if (mountVnum == 20246 || mountVnum == 20247) { limit = gConfig.pcMaxMoveSpeed + 50; }
                        else { limit = gConfig.pcMaxMoveSpeed + 70; }
                    }
                    else { limit = mobMan.GetMountSpeedLimit(mountVnum); }
                }
            }
        }
        else { limit = gConfig.mobMaxMoveSpeed; }
    }
    break;
    default:
        break;
    }

    if (val > max_val)
        SPDLOG_ERROR("{0} has point {1} with value {2} the maximum is: {3}", GetName(), type, val, max_val);

    if (val > limit)
        val = limit;

    if (val < min_limit)
        val = min_limit;

    if (mountVnum)
        val = limit;

    return (val);
}

void CHARACTER::SetPoint(uint8_t type, PointValue val)
{
    if (type >= POINT_MAX_NUM)
    {
        SPDLOG_ERROR("Point type overflow (type {0})", type);
        return;
    }

    m_pointsInstant.points[type] = val;

    // 아직 이동이 다 안끝났다면 이동 시간 계산을 다시 해야 한다.
    const auto now = get_dword_time();
    const auto index = MakeMotionId(m_motionKey).index;
    if (type == POINT_MOV_SPEED && (index == MOTION_WALK || index == MOTION_RUN) &&
        now < m_motionStartTime + m_motionDuration)
    {
        m_posStart = GetInterpolatedPosition(now);
        m_motionStartTime = now;
        m_motionDuration = CalculateMoveDuration();
    }
}

Gold CHARACTER::GetAllowedGold() const { return gConfig.maxGold; }

void CHARACTER::CheckMaximumPoints()
{
    if (GetMaxHP() < GetHP())
        PointChange(POINT_HP, GetMaxHP() - GetHP());

    if (GetMaxSP() < GetSP())
        PointChange(POINT_SP, GetMaxSP() - GetSP());
}

void CHARACTER::ChangeGold(Gold amount)
{
    const Gold nTotalMoney = GetGold() + amount;

    if (GetAllowedGold() <= nTotalMoney)
    {
        SPDLOG_ERROR("Gold overflow player {3}({2}) has {0} and would gain {1}", GetGold(), amount, GetPlayerID(),
                     GetName());
        LogManager::instance().CharLog(this, GetGold() + amount, "OVERFLOW_GOLD", "");
        return;
    }

    SetGold(GetGold() + amount);

    if (GetDesc())
    {
        TPacketGCGoldChange pack;
        pack.dwVID = m_vid;

        if (amount <= 0)
            pack.amount = 0;
        else
            pack.amount = amount;

        pack.value = GetGold();
        GetDesc()->Send(HEADER_GC_CHARACTER_GOLD_CHANGE, pack);
    }
}

#ifdef ENABLE_GEM_SYSTEM
void CHARACTER::ChangeGem(int amount)
{
    const int nTotalGem = GetGem() + amount;

    if (GEM_MAX <= nTotalGem)
    {
        SPDLOG_ERROR("[OVERFLOW_GEM] OriGem {} AddedGem {} id {} Name {} ", GetGem(), amount, GetPlayerID(), GetName());
        LogManager::instance().CharLog(this, GetGem() + amount, "OVERFLOW_GEM", "");
        return;
    }

    SetGem(GetGem() + amount);

    if (GetDesc())
    {
        TPacketGCGemChange pack;

        pack.header = HEADER_GC_CHARACTER_GEM_CHANGE;
        pack.dwVID = m_vid;

        if (amount <= 0)
            pack.amount = 0;
        else
            pack.amount = amount;

        pack.value = GetGem();
        GetDesc()->Packet(&pack, sizeof(TPacketGCGemChange));
    }
}
#endif

Level CHARACTER::GetMaxLevel() const
{
    /* if (!GetQuestFlag("level_manager.allow_over_55") && GetLevel() <= 55)
        return 55;

    if(!GetQuestFlag("level_manager.allow_over_75") && GetLevel() <= 75)
        return 75;

    if(!GetQuestFlag("level_manager.allow_over_100")  && GetLevel() <= 100)
        return 100;

   if(!GetQuestFlag("level_manager.allow_over_125")  && GetLevel() <= 125)
        return 125;
        */
    return gConfig.maxLevel;
}

void CHARACTER::PointChange(uint8_t type, PointValue amount, bool bAmount, bool bBroadcast)
{
    PointValue val = 0;

    // SPDLOG_INFO( "PointChange %d %d | %d -> %d cHP %d mHP %d", type, amount, GetPoint(type), GetPoint(type)+amount,
    // GetHP(), GetMaxHP());

    switch (type)
    {
    case POINT_NONE:
#ifdef ENABLE_BATTLE_PASS
    case POINT_BATTLE_PASS_ID:
#endif
        return;

    case POINT_CASH:
        if(amount > 1)
            ChangeCash(amount, "cash", true);
        else if(amount < 0)
            ChangeCash(amount, "cash", false);

        return;
        break;
    case POINT_LEVEL:
        if ((GetLevel() + amount) > GetMaxLevel())
            return;

        SetLevel(GetLevel() + amount);
        val = GetLevel();

        SPDLOG_INFO("LEVELUP: {0} {1} NEXT EXP {2}", GetName(), GetLevel(), GetNextExp());

        PointChange(POINT_NEXT_EXP, GetNextExp(), false);

        if (amount)
        {
            quest::CQuestManager::instance().LevelUp(GetPlayerID());

            if (GetLevel() >= 5 && GetSkillGroup() == 0) { ChatPacket(CHAT_TYPE_COMMAND, "OpenSkillGroupSelect"); }

            LogManager::instance().LevelLog(this, val,
                                            m_points.playtime + (get_dword_time() - m_dwPlayStartTime) / 60000);

            if (GetGuild())
                GetGuild()->LevelChange(GetPlayerID(), GetLevel());

            if (GetParty())
                GetParty()->RequestSetMemberLevel(GetPlayerID(), GetLevel());

            MapLevelChecks(this);

            if (GetJob() == JOB_WOLFMAN)
            {
                if (val >= 5) { if (GetSkillGroup() != 1) { SetSkillGroup(1); } }
                else { SetSkillGroup(0); }
            }
        }

        // PointsPacket();

        break;

    case POINT_NEXT_EXP:
        val = GetNextExp();
        bAmount = false; // 무조건 bAmount는 false 여야 한다.
        break;

    case POINT_EXP: {
        if (GetMaxLevel() <= GetLevel())
            return;

        PointValue exp = GetExp();
        PointValue next_exp = GetNextExp();
        PointValue expBalance = 0;

        if (amount < 0)
        {
            SetExp(exp - std::min<PointValue>(exp, std::abs(amount)));
            exp = GetExp();
        }
        else if (exp + amount >= next_exp)
        {
            // The amount of EXP have left after the level-up
            expBalance = (exp + amount) - next_exp;

            amount = next_exp - exp;
            SetExp(0);
            exp = next_exp;
        }
        else
        {
            SetExp(exp + amount);
            exp = GetExp();
        }

        PointValue q = next_exp / 4;
        int iLevStep = m_points.levelStep;

        if (iLevStep >= 4)
        {
            SPDLOG_ERROR("{0} LEVEL_STEP bigger than 4! ({1})", GetName(), iLevStep);
            iLevStep = 4;
        }

        if (exp >= next_exp && iLevStep < 4)
        {
            for (int i = 0; i < 4 - iLevStep; ++i)
                PointChange(POINT_LEVEL_STEP, 1, false, true);
        }
        else if (exp >= q * 3 && iLevStep < 3)
        {
            for (int i = 0; i < 3 - iLevStep; ++i)
                PointChange(POINT_LEVEL_STEP, 1, false, true);
        }
        else if (exp >= q * 2 && iLevStep < 2)
        {
            for (int i = 0; i < 2 - iLevStep; ++i)
                PointChange(POINT_LEVEL_STEP, 1, false, true);
        }
        else if (exp >= q && iLevStep < 1) { PointChange(POINT_LEVEL_STEP, 1); }

        if (expBalance)
            PointChange(POINT_EXP, expBalance);

        val = GetExp();
    }
    break;

    case POINT_LEVEL_STEP:
        if (amount > 0)
        {
            val = GetPoint(POINT_LEVEL_STEP) + amount;

            switch (static_cast<int>(val))
            {
            case 1:
            case 2:
            case 3:
                if (GetLevel() <= gConfig.maxLevelStats)
                    PointChange(POINT_STAT, 1);
                break;

            case 4: {
                int64_t iHP = JobInitialPoints[GetJob()].hp_per_lv_end;
                int64_t iSP = JobInitialPoints[GetJob()].sp_per_lv_end;

                m_points.iRandomHP += iHP;
                m_points.iRandomSP += iSP;

                if (GetSkillGroup())
                {
                    if (GetLevel() >= 5)
                        PointChange(POINT_SKILL, 1);

                    if (GetLevel() >= 9)
                        PointChange(POINT_SUB_SKILL, 1);
                }

                PointChange(POINT_MAX_HP, iHP);
                PointChange(POINT_MAX_SP, iSP);
                PointChange(POINT_LEVEL, 1, false, true);

                val = 0;
            }
            break;
            }

#ifndef RUBINU_NO_POTIONS
            if (GetLevel() <= 10)
                AutoGiveItem(27001, 2);
            else if (GetLevel() <= 30)
                AutoGiveItem(27002, 2);
            else
                AutoGiveItem(27003, 2);
#endif
            PointChange(POINT_HP, GetMaxHP() - GetHP());
            PointChange(POINT_SP, GetMaxSP() - GetSP());
            PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());

            SetPoint(POINT_LEVEL_STEP, val);
            m_points.levelStep = val;

            Save();
        }
        else
            val = GetPoint(POINT_LEVEL_STEP);

        break;

    case POINT_HP: {
        if (IsDead() || IsStun())
            return;

        int64_t prev_hp = GetHP();

        amount = std::min<PointValue>(GetMaxHP() - GetHP(), amount);
        SetHP(GetHP() + static_cast<int64_t>(amount));
        val = GetHP();

        BroadcastTargetPacket(prev_hp);

        if (GetParty() && IsPC() && val != prev_hp)
            GetParty()->SendPartyInfoOneToAll(this);
    }
    break;

    case POINT_SP: {
        if (IsDead() || IsStun())
            return;

        amount = std::min<PointValue>(GetMaxSP() - GetSP(), amount);
        SetSP(GetSP() + amount);
        val = GetSP();
    }
    break;

    case POINT_STAMINA: {
        if (IsDead() || IsStun())
            return;

        int prev_val = GetStamina();
        amount = std::min<PointValue>(GetMaxStamina() - GetStamina(), amount);
        SetStamina(GetStamina() + amount);
        val = GetStamina();

        if (val == 0)
        {
            // Stamina가 없으니 걷자!
            SetNowWalking(true);
        }
        else if (prev_val == 0)
        {
            // 없던 스테미나가 생겼으니 이전 모드 복귀
            ResetWalking();
        }

        if (amount < 0 && val != 0) // 감소는 보내지않는다.
            return;
    }
    break;

    case POINT_MAX_HP: {
        SetPoint(type, GetPoint(type) + amount);

        // SetMaxHP(GetMaxHP() + amount);
        // 최대 생명력 = (기본 최대 생명력 + 추가) * 최대생명력%
        auto hp = m_points.maxHp;
        PointValue add_hp = std::min<PointValue>(3500, hp * GetPoint(POINT_MAX_HP_PCT) / 100);
        add_hp += GetPoint(POINT_MAX_HP);
        add_hp += GetPoint(POINT_PARTY_TANKER_BONUS);
#ifdef __FAKE_PC__
        if (FakePC_Check())
        {
            hp += hp;
            add_hp += add_hp;
        }
#endif
        SetMaxHP(hp + add_hp);

        val = GetMaxHP();
    }
    break;

    case POINT_MAX_SP: {
        SetPoint(type, GetPoint(type) + amount);

        // SetMaxSP(GetMaxSP() + amount);
        // 최대 정신력 = (기본 최대 정신력 + 추가) * 최대정신력%
        auto sp = m_points.maxSp;
        PointValue add_sp = std::min<PointValue>(800, sp * GetPoint(POINT_MAX_SP_PCT) / 100);
        add_sp += GetPoint(POINT_MAX_SP);
#ifndef SHELIA_BUILD
        add_sp += GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
#endif
        SetMaxSP(sp + add_sp);
        val = GetMaxSP();
    }
    break;

    case POINT_MAX_HP_PCT:
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);

        PointChange(POINT_MAX_HP, 0);
        break;

    case POINT_MAX_SP_PCT:
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);

        PointChange(POINT_MAX_SP, 0);
        break;

    case POINT_MAX_STAMINA:
        SetMaxStamina(GetMaxStamina() + amount);
        val = GetMaxStamina();
        break;


    case POINT_DEF_GRADE:
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);

        PointChange(POINT_CLIENT_DEF_GRADE, amount);
        break;

    case POINT_IMMUNE_STUN: // 76
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);
        if (val) { SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN); }
        else { REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN); }
        break;

    case POINT_IMMUNE_SLOW: // 77
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);
        if (val) { SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW); }
        else { REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW); }
        break;

    case POINT_IMMUNE_FALL: // 78
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);
        if (val) { SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL); }
        else { REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL); }
        break;


    case POINT_DEF_GRADE_BONUS:
        SetPoint(type, GetPoint(type) + amount);
        PointChange(POINT_DEF_GRADE, amount);
        val = GetPoint(type);
        break;

    case POINT_MAGIC_ATT_GRADE_BONUS:
        SetPoint(type, GetPoint(type) + amount);
        PointChange(POINT_MAGIC_ATT_GRADE, amount);
        val = GetPoint(type);
        break;

    case POINT_MAGIC_DEF_GRADE_BONUS:
        SetPoint(type, GetPoint(type) + amount);
        PointChange(POINT_MAGIC_DEF_GRADE, amount);
        val = GetPoint(type);
        break;

    case POINT_POLYMORPH:
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);
        SetPolymorph(val);
        break;

        // case POINT_ENERGY:
    case POINT_COSTUME_ATTR_BONUS: {
        int old_val = GetPoint(type);
        SetPoint(type, old_val + amount);
        val = GetPoint(type);
        BuffOnAttr_ValueChange(type, old_val, val);
    }
    break;

    default:
        SetPoint(type, GetPoint(type) + amount);
        val = GetPoint(type);
        break;
    }

    switch (type)
    {
    case POINT_LEVEL:
    case POINT_ST:
    case POINT_DX:
    case POINT_IQ:
    case POINT_HT:
        ComputeBattlePoints();
        break;
    case POINT_MAX_HP:
    case POINT_MAX_SP:
    case POINT_MAX_STAMINA:
        if (!m_pointsInstant.computed)
            return;
        break;
    }

    if (type == POINT_HP && amount == 0)
        return;

    if (auto* d = GetDesc(); d)
    {
        if (d->IsPhase(PHASE_LOADING) || d->IsPhase(PHASE_SELECT))
            return;

        if (m_skipSendPoints)
            return;

        TPacketGCPointChange pack = {};
        pack.dwVID = m_vid;
        pack.type = type;
        pack.value = val;

        if (bAmount)
            pack.amount = amount;
        else
            pack.amount = 0;

        if (!bBroadcast)
            GetDesc()->Send(HEADER_GC_CHARACTER_POINT_CHANGE, pack);
        else
            PacketAround(m_map_view, this, HEADER_GC_CHARACTER_POINT_CHANGE, pack);
    }
}

void CHARACTER::ApplyPoint(uint8_t bApplyType, ApplyValue iVal)
{
    switch (bApplyType)
    {
    case APPLY_NONE: // 0
        break;;

    case APPLY_CON:
        PointChange(POINT_HT, iVal);
        PointChange(POINT_MAX_HP, (iVal * JobInitialPoints[GetJob()].hp_per_ht));
        PointChange(POINT_MAX_STAMINA, (iVal * JobInitialPoints[GetJob()].stamina_per_con));
        break;

    case APPLY_INT:
        PointChange(POINT_IQ, iVal);
        PointChange(POINT_MAX_SP, (iVal * JobInitialPoints[GetJob()].sp_per_iq));
        break;

    case APPLY_MAX_HP:
    case APPLY_MAX_HP_PCT: {
        auto i = GetMaxHP();
        if (i == 0)
            break;

        PointChange(GetApplyPoint(bApplyType), iVal);
    }
    break;

    case APPLY_MAX_SP:
    case APPLY_MAX_SP_PCT: {
        auto i = GetMaxSP();
        if (i == 0)
            break;

        PointChange(GetApplyPoint(bApplyType), iVal);
    }
    break;

    case APPLY_SKILL:
        // SKILL_DAMAGE_BONUS
    {
        // 최상위 비트 기준으로 8비트 vnum, 9비트 add, 15비트 change
        // 00000000 00000000 00000000 00000000
        // ^^^^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^^
        // vnum     ^ add       change
        uint8_t bSkillVnum = (uint8_t)(((uint32_t)iVal) >> 24);
        int iAdd = int(iVal) & 0x00800000;
        int iChange = int(iVal) & 0x007fffff;

        SPDLOG_DEBUG("APPLY_SKILL skill {0} add? {1} change {2}", bSkillVnum, iAdd ? 1 : 0, iChange);

        if (0 == iAdd)
            iChange = -iChange;

        auto iter = m_SkillDamageBonus.find(bSkillVnum);
        if (iter == m_SkillDamageBonus.end())
            m_SkillDamageBonus.emplace(bSkillVnum, iChange);
        else
            iter->second += iChange;
    }
        // END_OF_SKILL_DAMAGE_BONUS
    break;

    default:
        PointChange(GetApplyPoint(bApplyType), iVal);
        break;
    }
}

EVENTFUNC(save_event)
{
    char_event_info *info = static_cast<char_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_ERROR("save_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;
    if (ch == nullptr) { return 0; }

    SPDLOG_DEBUG("SAVE_EVENT: {0}", ch->GetName());
    ch->Save();
    ch->FlushDelayedSaveItem();
    return THECORE_SECS_TO_PASSES(gConfig.saveTimeout);
}

void CHARACTER::StartSaveEvent()
{
    if (m_pkSaveEvent)
        return;

    char_event_info *info = AllocEventInfo<char_event_info>();

    info->ch = this;
    m_pkSaveEvent = event_create(save_event, info, THECORE_SECS_TO_PASSES(gConfig.saveTimeout));
}

// MINING
void CHARACTER::mining_take() { m_miningEvent = nullptr; }

void CHARACTER::mining_cancel()
{
    if (m_miningEvent)
    {
        // SPDLOG_INFO( "MINING CANCEL");
        event_cancel(&m_miningEvent);
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "채광을 중단하였습니다.");
    }
}

void CHARACTER::mining(CHARACTER *chLoad)
{
    if (m_miningEvent)
    {
        mining_cancel();
        return;
    }

    if (!chLoad)
        return;

    // Map index and position checks are necessary,
    // since dungeons share map coordinates.
    if (GetMapIndex() != chLoad->GetMapIndex())
        return;

    if (DISTANCE_APPROX(GetX() - chLoad->GetX(), GetY() - chLoad->GetY()) > 1000)
        return;

    if (mining::GetRawOreFromLoad(chLoad->GetRaceNum()) == 0)
        return;

    const auto pick = GetWear(WEAR_WEAPON);
    if (!pick || pick->GetItemType() != ITEM_PICK)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "곡괭이를 장착하세요.");
        return;
    }

    if (pick->GetSubType() != 0)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "You can't use this type of pickaxe on this vein.");
        return;
    }

    int count = Random::get(5, 15); // 동작 횟수, 한 동작당 2초

    TPacketGCDigMotion p;
    p.vid = GetVID();
    p.target_vid = chLoad->GetVID();
    p.count = count;

    PacketAround(m_map_view, this,HEADER_GC_DIG_MOTION, p);

    m_miningEvent = mining::CreateMiningEvent(this, chLoad, count);
}

// END_OF_MINING

// CrystalMining MartPwnS 17.06.2014

bool CHARACTER::IS_VALID_FISHING_POSITION(int *returnPosx, int *returnPosy)
{
    int charX = GetX();
    int charY = GetY();

    SECTREE *curWaterPostitionTree;

    long fX, fY;
    for (auto rot = 0; rot <= 180; rot += 10) // mimics behaviour of client.
    {
        ELPlainCoord_GetRotatedPixelPosition(charX, charY, 600.0f, GetRotation() + rot, &fX, &fY);
        curWaterPostitionTree = SECTREE_MANAGER::instance().Get(GetMapIndex(), fX, fY);
        if (curWaterPostitionTree && curWaterPostitionTree->IsAttr(fX, fY, ATTR_WATER))
        {
            *returnPosx = fX;
            *returnPosy = fY;
            return true;
        }
        // No idea if thats needed client uses it.
        ELPlainCoord_GetRotatedPixelPosition(charX, charY, 600.0f, GetRotation() - rot, &fX, &fY);
        curWaterPostitionTree = SECTREE_MANAGER::instance().Get(GetMapIndex(), fX, fY);
        if (curWaterPostitionTree && curWaterPostitionTree->IsAttr(fX, fY, ATTR_WATER))
        {
            *returnPosx = fX;
            *returnPosy = fY;
            return true;
        }
    }

    return false;
}

void CHARACTER::fishing()
{
    if (m_pkFishingEvent)
    {
        return;
    }

    CItem *rod = GetWear(WEAR_WEAPON);

    // 낚시대 장착
    if (!rod || rod->GetItemType() != ITEM_ROD)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "낚시대를 장착 하세요.");
        return;
    }

    if (0 == rod->GetSocket(2))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "미끼를 끼고 던져 주세요.");
        return;
    }

    // 못감 속성에서 낚시를 시도한다?
    {
        SECTREE_MAP *pkSectreeMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

        int x = GetX();
        int y = GetY();

        SECTREE *playerTree = pkSectreeMap->Find(x, y);
        if (IS_SET(playerTree->GetAttribute(x, y), ATTR_BLOCK))
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "낚시를 할 수 있는 곳이 아닙니다");
            return;
        }

        int newPosx, newPosy;
        if (!IS_VALID_FISHING_POSITION(&newPosx, &newPosy))
        {
            ChatPacket(CHAT_TYPE_INFO, "I cannot go fishing here.");
            LogManager::instance().HackLog("FISH_BOT_LOCATION", this);
            return;
        }

        SetRotationToXY(newPosx, newPosy);
    }

    m_pkFishingEvent = fishing::CreateFishingEvent(this);
}

void CHARACTER::fishing_take(uint32_t hitCount, float time)
{
    SPDLOG_ERROR("Take {} {}", hitCount, time);

    if (hitCount < 3 || time < 0.0f)
    {
        event_cancel(&m_pkFishingEvent);

        TPacketGCFishing p = {};
        p.subheader = FISHING_SUBHEADER_GC_STOP;
        p.info = GetVID();
        PacketAround(GetViewMap(), this, HEADER_GC_FISHING, p);

        return;
    }

    CItem *rod = GetWear(WEAR_WEAPON);
    if (rod && rod->GetItemType() == ITEM_ROD)
    {
        using fishing::fishing_event_info;
        if (m_pkFishingEvent)
        {
            auto *info = static_cast<fishing_event_info *>(m_pkFishingEvent->info);

            if (info)
                fishing::Take(info, this);
        }
    }
    else
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "낚시대가 아닌 물건으로 낚시를 할 수 없습니다!");
    }

    event_cancel(&m_pkFishingEvent);
}

bool CHARACTER::StartStateMachine(int iNextPulse)
{
    if (g_pCharManager->AddToStateList(this))
    {
        m_dwNextStatePulse = thecore_heart->pulse + iNextPulse;
        return true;
    }

    return false;
}

void CHARACTER::StopStateMachine()
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return;
#endif

    g_pCharManager->RemoveFromStateList(this);
}

void CHARACTER::UpdateStateMachine(uint32_t dwPulse)
{
    if (dwPulse < m_dwNextStatePulse)
        return;

    if (IsDead())
        return;

    Update();
    m_dwNextStatePulse = dwPulse + m_dwStateDuration;
}

void CHARACTER::SetNextStatePulse(int iNextPulse)
{
    g_pCharManager->AddToStateList(this);
    m_dwNextStatePulse = iNextPulse;

    if (iNextPulse < 10)
        MonsterLog("Let's go to the next State");
}

// 캐릭터 인스턴스 업데이트 함수.
void CHARACTER::UpdateCharacter(uint32_t dwPulse) { CFSM::Update(); }

void CHARACTER::SetShop(Shop *pkShop)
{
    if ((m_pkShop = pkShop))
        SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
    else
    {
        REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
        SetShopOwner(nullptr);
    }
}

void CHARACTER::SetExchange(CExchange *pkExchange) { m_pkExchange = pkExchange; }

void CHARACTER::SetPart(uint8_t index, const Part &part)
{
    assert(index < PART_MAX_NUM);
    m_pointsInstant.parts[index] = part;
}

Part CHARACTER::GetPart(uint8_t index) const
{
    assert(index < PART_MAX_NUM);
    return m_pointsInstant.parts[index];
}

Part CHARACTER::GetOriginalPart(uint8_t bPartPos) const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->GetOriginalPart(bPartPos);
#endif

    switch (bPartPos)
    {
    case PART_MAIN: {
        if (!IsPC())
            return GetPart(PART_MAIN);

        return {m_pointsInstant.bBasePart, 0, 0, 0};
    }

    default:
        return {};
    }
}

uint8_t CHARACTER::GetCharType() const { return m_bCharType; }

bool CHARACTER::SetSyncOwner(CHARACTER *ch, bool bRemoveFromList)
{
    // TRENT_MONSTER
    if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
        return false;
    // END_OF_TRENT_MONSTER

    if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOPUSH))
        return false;

    if (ch == this) { return false; }

    if (!ch)
    {
        if (bRemoveFromList && m_pkChrSyncOwner) { m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this); }

        // 리스트에서 제거하지 않더라도 포인터는 NULL로 셋팅되어야 한다.
        m_pkChrSyncOwner = nullptr;
    }
    else
    {
        const auto mapIndex = ch->GetMapIndex();

        if (m_pkChrSyncOwner != ch)
        {
            if (m_pkChrSyncOwner) { m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this); }

            m_pkChrSyncOwner = ch;
            m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.push_back(this);

            // SyncOwner가 바뀌면 LastSyncTime을 초기화한다.
            static const timeval zero_tv = {0, 0};
            SetLastSyncTime(zero_tv);
        }

#ifdef ENHANCED_FLY_FIX
        m_fSyncTime = get_float_time();
#endif
    }

    // TODO: Sync Owner가 같더라도 계속 패킷을 보내고 있으므로,
    //       동기화 된 시간이 3초 이상 지났을 때 풀어주는 패킷을
    //       보내는 방식으로 하면 패킷을 줄일 수 있다.
    TPacketGCOwnership pack;
    pack.dwOwnerVID = ch ? ch->GetVID() : 0;
    pack.dwVictimVID = GetVID();

    PacketAround(m_map_view, this,HEADER_GC_OWNERSHIP, pack);
    return true;
}

struct FuncClearSync
{
    void operator()(CHARACTER *ch)
    {
        assert(ch != NULL);
        ch->SetSyncOwner(nullptr, false); // false 플래그로 해야 for_each 가 제대로 돈다.
    }
};

void CHARACTER::ClearSync()
{
    SetSyncOwner(nullptr);

    // 아래 for_each에서 나를 m_pkChrSyncOwner로 가진 자들의 포인터를 NULL로 한다.
    std::for_each(m_kLst_pkChrSyncOwned.begin(), m_kLst_pkChrSyncOwned.end(), FuncClearSync());
    m_kLst_pkChrSyncOwned.clear();
}

float get_float_time_kiruma()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tv.tv_sec -= 1057699978;
    return ((float)tv.tv_sec + ((float)tv.tv_usec / 1000000.0f));
}

bool CHARACTER::IsSyncOwner(CHARACTER *ch) const
{
    if (m_pkChrSyncOwner == ch)
        return true;

    // 마지막으로 동기화 된 시간이 3초 이상 지났다면 소유권이 아무에게도
    // 없다. 따라서 아무나 SyncOwner이므로 true 리턴

#ifdef ENHANCED_FLY_FIX
    if (get_float_time() - m_fSyncTime >= 3.0f)
        return true;
#endif

    return false;
}

void CHARACTER::SetDungeon(CDungeon *pkDungeon)
{
    if (m_pkDungeon == pkDungeon)
        return;

    if (m_pkDungeon)
        m_pkDungeon->OnLeave(this);

    m_pkDungeon = pkDungeon;

    if (m_pkDungeon)
        m_pkDungeon->OnEnter(this);
}

void CHARACTER::SetWarMap(CWarMap *pWarMap)
{
    ChatPacket(CHAT_TYPE_COMMAND, "warboard toggle|0");

    if (m_pWarMap)
        m_pWarMap->DecMember(this);

    m_pWarMap = pWarMap;

    if (m_pWarMap)
        m_pWarMap->IncMember(this);
}

void CHARACTER::SetWeddingMap(std::shared_ptr<marriage::WeddingMap> pMap)
{
    if (m_pWeddingMap)
        m_pWeddingMap->DecMember(this);

    m_pWeddingMap = std::move(pMap);

    if (m_pWeddingMap)
        m_pWeddingMap->IncMember(this);
}

void CHARACTER::SetRegen(regen *pkRegen)
{
    m_pkRegen = pkRegen;
    if (pkRegen != nullptr) { regen_id_ = pkRegen->id; }
    m_fRegenAngle = GetRotation();
    m_posRegen = GetXYZ();
}

bool CHARACTER::OnIdle() { return false; }

void CHARACTER::SyncPacket(CEntity *except)
{
    TPacketCGSyncPositionElement elem;

    elem.dwVID = GetVID();
    elem.lX = GetX();
    elem.lY = GetY();

    TPacketGCSyncPosition pack;
    pack.elems.emplace_back(elem);

    PacketAround(m_map_view, this,HEADER_GC_SYNC_POSITION, pack);
}

void CHARACTER::OnMove(bool bIsAttack)
{
#ifdef __FAKE_PC__
    if (IsPC())
        FakePC_Owner_ResetAfkEvent();
#endif

    m_lastMoveTime = get_dword_time();

    if (bIsAttack)
    {
        m_dwLastAttackTime = m_lastMoveTime;

        if (FindAffect(AFFECT_REVIVE_INVISIBLE))
            RemoveAffect(AFFECT_REVIVE_INVISIBLE);

        if (FindAffect(SKILL_EUNHYUNG))
        {
            RemoveAffect(SKILL_EUNHYUNG);
            SetAffectedEunhyung();
        }
        else { ClearAffectedEunhyung(); }
    }

    mining_cancel();

    if (IsPC() && GetParty())
        GetParty()->SendPartyPositionOneToAll(this);
}

void CHARACTER::OnClick(CHARACTER *pkCharClicking)
{
    if (!pkCharClicking)
    {
        SPDLOG_ERROR("{}: OnClick by NULL", GetName());
        return;
    }

    uint32_t vid = GetVID();
    SPDLOG_INFO("{0}: OnClick [vnum {1} ServerUniqueID {2}, pid {3}] by {4}", GetName(), GetRaceNum(), vid,
                GetPlayerID(), pkCharClicking->GetName());

#ifdef ENABLE_MELEY_LAIR_DUNGEON
    if ((IsNPC()) && (GetRaceNum() == (uint16_t)(MeleyLair::GATE_VNUM)) &&
        (MeleyLair::CMgr::instance().IsMeleyMap(pkCharClicking->GetMapIndex())))
    {
        SPDLOG_DEBUG("Start Meley {}[vnum {} ServerUniqueID {}, pid {}] by {}", GetName(), GetRaceNum(), vid,
                      GetPlayerID(), pkCharClicking->GetName());
        MeleyLair::CMgr::instance().Start(pkCharClicking);
        return;
    }
#endif

    {
        if (pkCharClicking->GetMyShop() && pkCharClicking != this)
        {
            SPDLOG_ERROR("OnClick Fail ({0}->{1}) - pc has shop", pkCharClicking->GetName(), GetName());
            return;
        }
    }
    {
        if (pkCharClicking->GetExchange())
        {
            SPDLOG_ERROR("OnClick Fail ({0}->{1}) - pc is exchanging", pkCharClicking->GetName(), GetName());
            return;
        }
    }

    if (IsPC())
    {
        if (!CTargetManager::instance().GetTargetInfo(pkCharClicking->GetPlayerID(), TARGET_TYPE_VID, GetVID()))
        {
            if (GetMyShop())
            {
                if (pkCharClicking->IsDead() == true)
                    return;
                if (pkCharClicking == this)
                {
                    if ((GetExchange() || IsOpenSafebox() || GetShopOwner()) || IsCubeOpen())
                    {
                        SendI18nChatPacket(pkCharClicking, CHAT_TYPE_INFO, "You can't use a private shop now.");
                        return;
                    }
                }
                else
                {
                    if ((pkCharClicking->GetExchange() || pkCharClicking->IsOpenSafebox() ||
                         pkCharClicking->GetMyShop() || pkCharClicking->GetShopOwner()) ||
                        pkCharClicking->IsCubeOpen())
                    {
                        SendI18nChatPacket(pkCharClicking, CHAT_TYPE_INFO, "You can't use a private shop now.");
                        return;
                    }
                    if ((GetExchange() || IsOpenSafebox() || IsCubeOpen()))
                    {
                        SendI18nChatPacket(pkCharClicking, CHAT_TYPE_INFO, "This player trades already with another player.");
                        return;
                    }
                }

                if (pkCharClicking->GetShop())
                {
                    pkCharClicking->GetShop()->RemoveGuest(pkCharClicking);
                    pkCharClicking->SetShop(nullptr);
                }

                GetMyShop()->AddGuest(pkCharClicking, GetVID(), false);
                pkCharClicking->SetShopOwner(this);
                return;
            }

            if (gConfig.testServer)
                SPDLOG_ERROR("{0}.OnClickFailure({1}) - target is PC", pkCharClicking->GetName(), GetName());

            return;
        }
    }

#ifdef __OFFLINE_SHOP__
    else if (this->GetRaceNum() == g_dwOfflineShopKeeperVNum)
    {
        if (this->GetMyOfflineShop())
        {
            this->GetMyOfflineShop()->AddViewer(pkCharClicking);
            return;
        }
    }
#endif

    if (IsBuffBot())
    {
        BuffBot_BuffClicker(pkCharClicking);
        return;
    }

    pkCharClicking->SetQuestNPCID(GetVID());

    if (quest::CQuestManager::instance().Click(pkCharClicking->GetPlayerID(), this))
        return;

    // NPC 전용 기능 수행 : 상점 열기 등
    if (!IsPC())
    {
        if (!m_triggerOnClick.pFunc)
        {
            // NPC 트리거 시스템 로그 보기
            // SPDLOG_ERROR("%s.OnClickFailure(%s) : triggerOnClick.pFunc is EMPTY(pid=%d)",
            //			pkChrCauser->GetName(),
            //			GetName(),
            //			pkChrCauser->GetPlayerID());
            return;
        }

        m_triggerOnClick.pFunc(this, pkCharClicking);
    }
}

void CHARACTER::BuffBot_BuffClicker(CHARACTER *clickerCharacter)
{
    SetPoint(POINT_ST, 1);
    SetPoint(POINT_HT, 1);
    SetPoint(POINT_DX, 1);
    SetPoint(POINT_IQ, gConfig.buffBotIq);

    if (clickerCharacter->FindAffect(SKILL_HOSIN) == nullptr)
    {
        FlyTarget(clickerCharacter->GetVID(), clickerCharacter->GetX(), clickerCharacter->GetY(), HEADER_CG_FLY_TARGETING);

        ComputeSkill(SKILL_HOSIN, clickerCharacter, gConfig.buffBotSkillLevel);
        SendBuffBotSkillPacket(SKILL_HOSIN);
    }
    else if (clickerCharacter->FindAffect(SKILL_REFLECT) == nullptr)
    {
        FlyTarget(clickerCharacter->GetVID(), clickerCharacter->GetX(), clickerCharacter->GetY(), HEADER_CG_FLY_TARGETING);
        ComputeSkill(SKILL_REFLECT, clickerCharacter, gConfig.buffBotSkillLevel);
        SendBuffBotSkillPacket(SKILL_REFLECT);
    }
    else if (clickerCharacter->FindAffect(SKILL_GICHEON) == nullptr)
    {
        FlyTarget(clickerCharacter->GetVID(), clickerCharacter->GetX(), clickerCharacter->GetY(), HEADER_CG_FLY_TARGETING);
        ComputeSkill(SKILL_GICHEON, clickerCharacter, gConfig.buffBotSkillLevel);
        SendBuffBotSkillPacket(SKILL_GICHEON);
    }
    else if (clickerCharacter->FindAffect(SKILL_KWAESOK) == nullptr)
    {
        FlyTarget(clickerCharacter->GetVID(), clickerCharacter->GetX(), clickerCharacter->GetY(), HEADER_CG_FLY_TARGETING);
        ComputeSkill(SKILL_KWAESOK, clickerCharacter, gConfig.buffBotSkillLevel);
        SendBuffBotSkillPacket(SKILL_KWAESOK);
    }
    else if (clickerCharacter->FindAffect(SKILL_JEUNGRYEOK) == nullptr)
    {
        FlyTarget(clickerCharacter->GetVID(), clickerCharacter->GetX(), clickerCharacter->GetY(), HEADER_CG_FLY_TARGETING);

        ComputeSkill(SKILL_JEUNGRYEOK, clickerCharacter, gConfig.buffBotSkillLevel);
        SendBuffBotSkillPacket(SKILL_JEUNGRYEOK);
    }

    clickerCharacter->SetBuffbotTime();
}

uint8_t CHARACTER::GetGMLevel(bool bIgnoreTestServer) const
{
    if (bIgnoreTestServer && IsPC())
        return GM::get_level(GetName().c_str(), GetDesc()->GetLogin().c_str(), true);

    if (gConfig.testServer)
        return GM_IMPLEMENTOR;
    return m_pointsInstant.gm_level;
}

void CHARACTER::SetGMLevel()
{
    if (GetDesc())
    {
#ifdef __NEW_GAMEMASTER_CONFIG__
        m_pointsInstant.gm_level = GM::get_level(GetName().c_str(), GetDesc()->GetLogin().c_str());
#else
        m_pointsInstant.gm_level = gm_get_level(GetName(), GetDesc()->GetHostName(), GetDesc()->GetLogin().c_str());
#endif
    }
    else { m_pointsInstant.gm_level = GM_PLAYER; }
}

bool CHARACTER::IsGM() const
{
    if (gConfig.testServer)
        return true;

    if (GetGMLevel() != GM_PLAYER)
        return true;

    return false;
}

void CHARACTER::SetStone(CHARACTER *pkChrStone)
{
    m_pkChrStone = pkChrStone;

    if (m_pkChrStone)
    {
        if (pkChrStone->m_set_pkChrSpawnedBy.find(this) == pkChrStone->m_set_pkChrSpawnedBy.end())
            pkChrStone->m_set_pkChrSpawnedBy.insert(this);
    }
}

struct FuncDeadSpawnedByStone
{
    void operator()(CHARACTER *ch)
    {
        ch->Dead(nullptr, true);
        ch->SetStone(nullptr);
    }
};

void CHARACTER::ClearStone()
{
    if (!m_set_pkChrSpawnedBy.empty())
    {
        FuncDeadSpawnedByStone f;
        std::for_each(m_set_pkChrSpawnedBy.begin(), m_set_pkChrSpawnedBy.end(), f);
        m_set_pkChrSpawnedBy.clear();
    }

    if (!m_pkChrStone)
        return;

    m_pkChrStone->m_set_pkChrSpawnedBy.erase(this);
    m_pkChrStone = nullptr;
}

void CHARACTER::ClearTarget()
{
    if (m_pkChrTarget)
    {
        m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);
        m_pkChrTarget = nullptr;
    }

    TPacketGCTarget p;
    p.dwVID = 0;
    p.lHP = 0;
    p.lMaxHP = 0;
    p.prevHp = 0;

    for (auto *pkChr : m_set_pkChrTargetedBy)
    {
        pkChr->m_pkChrTarget = nullptr;

        if (!pkChr->GetDesc()) { continue; }
        pkChr->GetDesc()->Send(HEADER_GC_TARGET, p);
    }

    m_set_pkChrTargetedBy.clear();
}

void CHARACTER::SetTarget(CHARACTER *pkChrTarget)
{
    if (m_pkChrTarget == pkChrTarget)
        return;

    if (m_pkChrTarget)
        m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);

    m_pkChrTarget = pkChrTarget;

    TPacketGCTarget p;

    if (m_pkChrTarget)
    {
        m_pkChrTarget->m_set_pkChrTargetedBy.insert(this);

        p.dwVID = m_pkChrTarget->GetVID();
        p.lHP = 1;
        p.lMaxHP = 1;
        p.prevHp = 1;

        if (m_pkChrTarget->GetMaxHP() > 0)
        {
            if (m_pkChrTarget->GetRaceNum() == 20101 || m_pkChrTarget->GetRaceNum() == 20102 ||
                m_pkChrTarget->GetRaceNum() == 20103 || m_pkChrTarget->GetRaceNum() == 20104 ||
                m_pkChrTarget->GetRaceNum() == 20105 || m_pkChrTarget->GetRaceNum() == 20106 ||
                m_pkChrTarget->GetRaceNum() == 20107 || m_pkChrTarget->GetRaceNum() == 20108 ||
                m_pkChrTarget->GetRaceNum() == 20109)
            {
                CHARACTER *owner = m_pkChrTarget->GetVictim();

                if (owner)
                {
                    int iHorseHealth = owner->GetHorseHealth();
                    int iHorseMaxHealth = owner->GetHorseMaxHealth();

                    if (iHorseMaxHealth)
                    {
                        p.lHP = iHorseHealth;
                        p.lMaxHP = iHorseMaxHealth;
                    }
                }
            }
            else
            {
                p.prevHp = m_pkChrTarget->GetHP();
                p.lHP = m_pkChrTarget->GetHP();
                p.lMaxHP = m_pkChrTarget->GetMaxHP();
            }
        }
    }
    else
    {
        p.dwVID = 0;
        p.lHP = 0;
        p.lMaxHP = 0;
        p.prevHp = 0;
    }

    GetDesc()->Send(HEADER_GC_TARGET, p);
}

void CHARACTER::BroadcastTargetPacket(uint32_t prevHp)
{
    if (m_set_pkChrTargetedBy.empty())
        return;

    TPacketGCTarget p = {};
    p.dwVID = GetVID();
    p.prevHp = prevHp;
    p.lHP = GetHP();
    p.lMaxHP = GetMaxHP();

    for (const auto &ch : m_set_pkChrTargetedBy)
    {
        if (!ch->GetDesc())
        {
            SPDLOG_ERROR("{} has no DESC", ch->GetName());
            continue;
        }

        ch->GetDesc()->Send(HEADER_GC_TARGET, p);
    }
}

void CHARACTER::CheckTarget()
{
    if (!m_pkChrTarget)
        return;

    if (DISTANCE_APPROX(GetX() - m_pkChrTarget->GetX(), GetY() - m_pkChrTarget->GetY()) >= 4800)
        SetTarget(nullptr);
}

void CHARACTER::SetWarpLocation(long lMapIndex, long x, long y)
{
    m_posWarp.x = x * 100;
    m_posWarp.y = y * 100;
    m_lWarpMapIndex = lMapIndex;
}

void CHARACTER::SaveExitLocation()
{
    m_posExit = GetXYZ();
    m_lExitMapIndex = GetMapIndex();
}

void CHARACTER::ExitToSavedLocation()
{
    if (m_lWarpMapIndex != 0)
        WarpSet(m_lWarpMapIndex, m_posWarp.x, m_posWarp.y);

    m_posExit.x = m_posExit.y = m_posExit.z = 0;
    m_lExitMapIndex = 0;
}

EVENTINFO(WarpEventInfo)
{
    DynamicCharacterPtr ch;

    char addr[46];
    uint16_t port;

    WarpEventInfo()
        : port(0)
    {
    }
};

EVENTFUNC(warp_event)
{
    auto info = static_cast<WarpEventInfo *>(event->info);
    if (!info)
    {
        SPDLOG_ERROR("warp_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;
    if (!ch)
        return 0;

    ch->m_pkWarpEvent = nullptr;

    // Grab a handle to the DESC, since Disconnect() deletes |ch|
    auto d = ch->GetDesc();

    TPacketGCWarp p;
    p.lAddr = info->addr;
    p.wPort = info->port;
    d->Send(HEADER_GC_WARP, p);
    ch->GetDesc()->SetDisconnectReason("Warp or Channel change");
    ch->Disconnect("WARP");
    d->Logout();
    d->SetInputProcessor(PHASE_CLOSE);
    return 0;
}

// fixme
// 지금까진 privateMapIndex 가 현재 맵 인덱스와 같은지 체크 하는 것을 외부에서 하고,
// 다르면 warpset을 불렀는데
// 이를 warpset 안으로 넣자.
bool CHARACTER::WarpSet(long mapIndex, long x, long y, uint8_t channel)
{
    if (!IsPC() || m_pkWarpEvent)
        return false;

    std::string addr;
    uint16_t port;

    uint32_t realIndex = mapIndex;
    if (realIndex > 10000)
        realIndex /= 10000;

    if (channel == 0)
    {
        if (!CMapLocation::instance().Get(realIndex, addr, port))
        {
            SPDLOG_ERROR("{0}: failed to find game for {1}:{2}x{3}", GetName(), mapIndex, x, y);
            return false;
        }
    }
    else
    {
        if (!CMapLocation::instance().Get(channel, realIndex, addr, port))
        {
            SPDLOG_ERROR("{0}: failed to find game for {1}:{2}x{3} ch {4}", GetName(), mapIndex, x, y, channel);
            return false;
        }
    }

    Save();

    if (GetSectree())
    {
        GetSectree()->RemoveEntity(this);
        ViewCleanup();

        EncodeRemovePacket(this);
    }

    m_lWarpMapIndex = mapIndex;
    m_posWarp.x = x;
    m_posWarp.y = y;

    SPDLOG_INFO("WarpSet {0} {1} {2} current map {3} target map {4}", GetName(), x, y, GetMapIndex(), mapIndex);

    auto info = AllocEventInfo<WarpEventInfo>();
    info->ch = this;
    storm::CopyStringSafe(info->addr, addr);
    info->port = port;
    m_pkWarpEvent = event_create(warp_event, info, 1);

    LogManager::instance().CharLog(this, 0, "WARP",
                                   fmt::sprintf("%s MapIdx %d DestMapIdx%d DestX%d DestY%d Empire%d", GetName(),
                                                GetMapIndex(), mapIndex, x, y, GetEmpire())
                                   .c_str());
    return true;
}

bool CHARACTER::Return()
{
    if (!IsNPC())
        return false;

    int x, y;
    SetVictim(nullptr);

    x = m_pkMobInst->m_posLastAttacked.x;
    y = m_pkMobInst->m_posLastAttacked.y;

    SetRotationToXY(x, y);

    if (!Goto(x, y))
        return false;

    SPDLOG_DEBUG("{0} {1} Give up and go back! {2} {3}", GetName(), (void *)this, x, y);

    if (GetParty())
        GetParty()->SendMessage(this, PM_RETURN, x, y);

    return true;
}

bool CHARACTER::Follow(CHARACTER *pkChr, float fMinDistance)
{
    if (IsPC())
    {
        SPDLOG_ERROR("CHARACTER::Follow : PC cannot use this method", GetName());
        return false;
    }

    // TRENT_MONSTER
    if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
    {
        if (pkChr->IsPC()) // 쫓아가는 상대가 PC일 때
        {
#ifdef ENABLE_MELEY_LAIR_DUNGEON
            if (GetRaceNum() == MeleyLair::BOSS_VNUM)
                return false;
#endif
            // If i'm in a party. I must obey party leader's AI.
            if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
            {
                if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >=
                    15000) // 마지막으로 공격받은지 15초가 지났고
                {
                    // 마지막 맞은 곳으로 부터 50미터 이상 차이나면 포기하고 돌아간다.
                    if (m_pkMobData->wAttackRange < DISTANCE_APPROX(pkChr->GetX() - GetX(), pkChr->GetY() - GetY()))
                        if (Return())
                            return true;
                }
            }
        }
        return false;
    }
    // END_OF_TRENT_MONSTER

    long x = pkChr->GetX();
    long y = pkChr->GetY();

    if (pkChr->IsPC()) // 쫓아가는 상대가 PC일 때
    {
        // If i'm in a party. I must obey party leader's AI.
        if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
        {
            if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >= 15000) // 마지막으로 공격받은지 15초가 지났고
            {
                // 마지막 맞은 곳으로 부터 50미터 이상 차이나면 포기하고 돌아간다.
                if (5000 < DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(),
                                           m_pkMobInst->m_posLastAttacked.y - GetY()))
                    if (Return())
                        return true;
            }
        }
    }

    if (IsGuardNPC())
    {
        if (5000 <
            DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(), m_pkMobInst->m_posLastAttacked.y - GetY()))
            if (Return())
                return true;
    }

    if (pkChr->IsState(pkChr->m_stateMove) && GetMobBattleType() != BATTLE_TYPE_RANGE &&
        GetMobBattleType() != BATTLE_TYPE_MAGIC && false == IsPet())
    {
        // 대상이 이동중이면 예측 이동을 한다
        // 나와 상대방의 속도차와 거리로부터 만날 시간을 예상한 후
        // 상대방이 그 시간까지 직선으로 이동한다고 가정하여 거기로 이동한다.
        float rot = pkChr->GetRotation();
        float rot_delta = GetDegreeDelta(rot, GetDegreeFromPositionXY(GetX(), GetY(), pkChr->GetX(), pkChr->GetY()));

        float yourSpeed = pkChr->GetMoveSpeed();
        float mySpeed = GetMoveSpeed();

        float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());
        float fFollowSpeed = mySpeed - yourSpeed * cos(rot_delta * boost::math::constants::pi<float>() / 180);

        if (fFollowSpeed >= 0.1f)
        {
            float fMeetTime = fDist / fFollowSpeed;
            float fYourMoveEstimateX, fYourMoveEstimateY;

            if (fMeetTime * yourSpeed <= 100000.0f)
            {
                GetDeltaByDegree(pkChr->GetRotation(), fMeetTime * yourSpeed, &fYourMoveEstimateX, &fYourMoveEstimateY);

                x += (long)fYourMoveEstimateX;
                y += (long)fYourMoveEstimateY;

                float fDistNew = sqrt(((double)x - GetX()) * (x - GetX()) + ((double)y - GetY()) * (y - GetY()));
                if (fDist < fDistNew)
                {
                    x = (long)(GetX() + (x - GetX()) * fDist / fDistNew);
                    y = (long)(GetY() + (y - GetY()) * fDist / fDistNew);
                }
            }
        }
    }

    // 가려는 위치를 바라봐야 한다.
    SetRotationToXY(x, y);

    float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());

    if (fDist <= fMinDistance)
        return false;

    float fx, fy;

    if (IsChangeAttackPosition(pkChr) && GetMobRank() < MOB_RANK_BOSS)
    {
        // 상대방 주변 랜덤한 곳으로 이동
        SetChangeAttackPositionTime();

        int retry = 16;
        int dx, dy;
        int rot = static_cast<int>(GetDegreeFromPositionXY(x, y, GetX(), GetY()));

        while (--retry)
        {
            if (fDist < 500.0f)
                GetDeltaByDegree((float)((rot + Random::get(-90, 90) + Random::get(-90, 90)) % 360), fMinDistance, &fx,
                                 &fy);
            else
                GetDeltaByDegree((float)Random::get(0, 359), fMinDistance, &fx, &fy);

            dx = x + (int)fx;
            dy = y + (int)fy;

            SECTREE *tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), dx, dy);

            if (nullptr == tree)
                break;

            if (0 == (tree->GetAttribute(dx, dy) & (ATTR_BLOCK | ATTR_OBJECT)))
                break;
        }

        // SPDLOG_INFO( "근처 어딘가로 이동 %s retry %d", GetName(), retry);
        if (!Goto(dx, dy))
            return false;
    }
    else
    {
        // 직선 따라가기
        float fDistToGo = fDist - fMinDistance;
        GetDeltaByDegree(GetRotation(), fDistToGo, &fx, &fy);

        // SPDLOG_INFO( "직선으로 이동 %s", GetName());
        if (!Goto(GetX() + (int)fx, GetY() + (int)fy))
            return false;
    }

    // MonsterLog("쫓아가기; %s", pkChr->GetName());
    return true;
}

float CHARACTER::GetDistanceFromSafeboxOpen() const
{
    return (float)DISTANCE_APPROX(GetX() - m_posSafeboxOpen.x, GetY() - m_posSafeboxOpen.y);
}

void CHARACTER::SetSafeboxOpenPosition() { m_posSafeboxOpen = GetXYZ(); }

CSafebox *CHARACTER::GetSafebox() const { return m_pkSafebox.get(); }

void CHARACTER::ReqSafeboxLoad(const char *pszPassword)
{
#ifdef __NEW_GAMEMASTER_CONFIG__
    if (!GM::check_allow(GetGMLevel(), GM_ALLOW_USE_SAFEBOX))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "You cannot do this with this gamemaster rank.");
        return;
    }
#endif

    if (!*pszPassword || strlen(pszPassword) > SAFEBOX_PASSWORD_MAX_LEN)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<창고> 잘못된 암호를 입력하셨습니다.");
        return;
    }
    else if (m_pkSafebox)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<창고> 창고가 이미 열려있습니다.");
        return;
    }

    int iPulse = thecore_pulse();

    if (iPulse - GetSafeboxLoadTime() < THECORE_SECS_TO_PASSES(10))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<창고> 창고를 닫은지 10초 안에는 열 수 없습니다.");
        return;
    }
    else if (GetDistanceFromSafeboxOpen() > 1000)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "<창고> 거리가 멀어서 창고를 열 수 없습니다.");
        return;
    }
    else if (m_bOpeningSafebox)
    {
        SPDLOG_INFO("{}: Overlapped safebox load request", GetName());
        return;
    }

    SetSafeboxLoadTime();
    m_bOpeningSafebox = true;

    TSafeboxLoadPacket p;
    p.dwID = GetDesc()->GetAccountTable().id;
    strlcpy(p.szLogin, GetDesc()->GetLogin().c_str(), sizeof(p.szLogin));
    strlcpy(p.szPassword, pszPassword, sizeof(p.szPassword));

    db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_LOAD, GetDesc()->GetHandle(), &p, sizeof(p));
}

void CHARACTER::LoadSafebox(int iSize, uint32_t dwGold, int iItemCount, TPlayerItem *pItems)
{
    bool bLoaded = false;
    SetOpenSafebox(true);

    if (m_pkSafebox)
        bLoaded = true;

    if (!m_pkSafebox)
        m_pkSafebox = std::make_unique<CSafebox>(this, iSize, dwGold);
    else
        m_pkSafebox->ChangeSize(iSize);

    m_iSafeboxSize = iSize;

    TPacketCGSafeboxSize p;
    p.bSize = iSize * SAFEBOX_PAGE_SIZE;
    GetDesc()->Send(HEADER_GC_SAFEBOX_SIZE, p);

    if (!bLoaded)
    {
        for (int i = 0; i < iItemCount; ++i, ++pItems)
        {
            if (!m_pkSafebox->IsValidPosition(pItems->pos))
                continue;

            CItem *item = ITEM_MANAGER::instance().CreateItem(pItems->data.vnum, pItems->data.count, pItems->id);

            if (!item)
            {
                SPDLOG_ERROR("LoadSafebox: Cannot create item vnum {} id {} (name: {})", pItems->data.vnum, pItems->id,
                             GetName());
                continue;
            }

            item->SetSkipSave(true);
            item->SetTransmutationVnum(pItems->data.transVnum);
            item->SetSealDate(pItems->data.nSealDate);
            item->SetSockets(pItems->data.sockets);
            item->SetAttributes(pItems->data.attrs);
            item->SetLastOwnerPid(pItems->owner);
            item->SetGMOwner(pItems->is_gm_owner);
            item->SetBlocked(pItems->is_blocked);
            item->SetPrivateShopPrice(pItems->price);
            events::Item::OnLoad(this, item);

            if (!m_pkSafebox->Add(pItems->pos, item)) { M2_DESTROY_ITEM(item); }
            else
                item->SetSkipSave(false);
        }
    }
}

void CHARACTER::ChangeSafeboxSize(uint8_t bSize)
{
    // if (!m_pkSafebox)
    // return;

    TPacketCGSafeboxSize p;
    p.bSize = bSize * SAFEBOX_PAGE_SIZE;
    GetDesc()->Send(HEADER_GC_SAFEBOX_SIZE, p);

    if (m_pkSafebox)
        m_pkSafebox->ChangeSize(bSize);

    m_iSafeboxSize = bSize;
}

void CHARACTER::CloseSafebox()
{
    if (!m_pkSafebox)
        return;

    // PREVENT_TRADE_WINDOW
    SetOpenSafebox(false);
    // END_PREVENT_TRADE_WINDOW

    m_pkSafebox->Save();
    m_pkSafebox.reset();

    ChatPacket(CHAT_TYPE_COMMAND, "CloseSafebox");

    SetSafeboxLoadTime();
    m_bOpeningSafebox = false;

    Save();
}

CSafebox *CHARACTER::GetMall() const { return m_pkMall.get(); }

void CHARACTER::LoadMall(int iItemCount, TPlayerItem *pItems)
{
    static const uint32_t cPageCount = 3;

    bool bLoaded;
    if (!m_pkMall)
    {
        m_pkMall = std::make_unique<CSafebox>(this, cPageCount, 0);
        bLoaded = false;
    }
    else
    {
        m_pkMall->ChangeSize(cPageCount);
        bLoaded = true;
    }

    m_pkMall->SetWindowMode(MALL);

    TPacketCGSafeboxSize p;
    p.bSize = cPageCount * SAFEBOX_PAGE_SIZE;
    GetDesc()->Send(HEADER_GC_MALL_OPEN, p);

    if (!bLoaded)
    {
        for (auto i = 0; i < iItemCount; ++i, ++pItems)
        {
            if (!m_pkMall->IsValidPosition(pItems->pos))
                continue;

            auto item = ITEM_MANAGER::instance().CreateItem(pItems->data.vnum, pItems->data.count, pItems->id);

            if (!item)
            {
                SPDLOG_ERROR("LoadMall: Cannot create item vnum {} id {} (name: {})", pItems->data.vnum, pItems->id,
                             GetName());
                continue;
            }

            item->SetSkipSave(true);
            item->SetTransmutationVnum(pItems->data.transVnum);
            item->SetSockets(pItems->data.sockets);
            item->SetAttributes(pItems->data.attrs);
            item->SetSealDate(pItems->data.nSealDate);
            item->SetLastOwnerPid(pItems->owner);
            item->SetGMOwner(pItems->is_gm_owner);
            item->SetBlocked(pItems->is_blocked);
            item->SetPrivateShopPrice(pItems->price);

            // item->OnLoad();

            if (!m_pkMall->Add(pItems->pos, item))
                M2_DESTROY_ITEM(item);
            else
                item->SetSkipSave(false);
        }
    }
}

void CHARACTER::CloseMall()
{
    if (!m_pkMall)
        return;

    m_pkMall->Save();

    m_pkMall.reset();

    ChatPacket(CHAT_TYPE_COMMAND, "CloseMall");
}

int CHARACTER::GetLeadershipSkillLevel() const { return GetSkillLevel(SKILL_LEADERSHIP); }

int CHARACTER::GetSafeboxSize() const { return m_iSafeboxSize; }

void CHARACTER::SetNowWalking(bool bWalkFlag)
{
    // if (m_bNowWalking != bWalkFlag || IsNPC())
    if (m_bNowWalking != bWalkFlag)
    {
        if (bWalkFlag)
        {
            m_bNowWalking = true;
            m_walkStartTime = get_dword_time();
        }
        else { m_bNowWalking = false; }

        // if (m_bNowWalking)
        {
            TPacketGCWalkMode p;
            p.vid = GetVID();
            p.mode = m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;
            PacketView(m_map_view, static_cast<CEntity*>(this), HEADER_GC_WALK_MODE, p);
        }

        if (IsNPC())
        {
            if (m_bNowWalking)
                MonsterLog("Walking");
            else
                MonsterLog("Running");
        }

        // SPDLOG_INFO( "{0} is now {1}", GetName(), m_bNowWalking?"walking.":"running.");
    }
}

void CHARACTER::StartStaminaConsume()
{
    if (m_bStaminaConsume)
        return;

    PointChange(POINT_STAMINA, 0);
    m_bStaminaConsume = true;

    ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * THECORE_SECS_TO_PASSES(1),
               GetStamina());
}

void CHARACTER::StopStaminaConsume()
{
    if (!m_bStaminaConsume)
        return;
    PointChange(POINT_STAMINA, 0);
    m_bStaminaConsume = false;
    ChatPacket(CHAT_TYPE_COMMAND, "StopStaminaConsume %d", GetStamina());
}

bool CHARACTER::IsStaminaConsume() const { return m_bStaminaConsume; }

void CHARACTER::ResetStopTime() { m_dwStopTime = get_dword_time(); }

uint32_t CHARACTER::GetStopTime() const { return m_dwStopTime; }

void CHARACTER::ResetPoint(int iLv)
{
    uint8_t bJob = GetJob();

    PointChange(POINT_LEVEL, iLv - GetLevel(), false, true);

    if (iLv != GetLevel())
        SetPoint(POINT_EXP, 0);

    m_points.st = JobInitialPoints[bJob].st;
    m_points.ht = JobInitialPoints[bJob].ht;
    m_points.dx = JobInitialPoints[bJob].dx;
    m_points.iq = JobInitialPoints[bJob].iq;

    SetPoint(POINT_ST, m_points.st);
    SetPoint(POINT_HT, m_points.ht);
    SetPoint(POINT_DX, m_points.dx);
    SetPoint(POINT_IQ, m_points.iq);

    SetRandomHP((iLv - 1) * JobInitialPoints[GetJob()].hp_per_lv_end);
    SetRandomSP((iLv - 1) * JobInitialPoints[GetJob()].sp_per_lv_end);

    // PointChange(POINT_STAT, ((MINMAX(1, iLv, 90) - 1) * 3) + GetPoint(POINT_LEVEL_STEP) - GetPoint(POINT_STAT));

    PointChange(POINT_STAT,
                std::min<PointValue>((iLv - 1) * 3 + GetPoint(POINT_LEVEL_STEP), gConfig.maxLevelStats * 3) -
                GetPoint(POINT_STAT));
    ComputePoints();
    ComputeMountPoints();
    PointChange(POINT_HP, GetMaxHP() - GetHP());
    PointChange(POINT_SP, GetMaxSP() - GetSP());

    PointsPacket();

    LogManager::instance().CharLog(this, 0, "RESET_POINT", "");
}

bool CHARACTER::IsChangeAttackPosition(CHARACTER *target) const
{
    if (!IsNPC())
        return true;

    uint32_t dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_NEAR;

    if (DISTANCE_APPROX(GetX() - target->GetX(), GetY() - target->GetY()) >
        AI_CHANGE_ATTACK_POISITION_DISTANCE + GetMobAttackRange())
        dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_FAR;

    return get_dword_time() - m_dwLastChangeAttackPositionTime > dwChangeTime;
}

void CHARACTER::GiveRandomSkillBook()
{
    CItem *item = AutoGiveItem(50300);

    if (nullptr != item)
    {
        uint8_t bJob = 0;

        if (!Random::get<bool>())
            bJob = GetJob() + 1;

        uint32_t dwSkillVnum = 0;

        do
        {
#ifdef ENABLE_WOLFMAN
            dwSkillVnum = Random::get(1, 175);
            if (dwSkillVnum > 111)
                dwSkillVnum = Random::get(170, 175);
            else
#endif
            dwSkillVnum = Random::get(1, 111);

            const CSkillProto *pkSk = CSkillManager::instance().Get(dwSkillVnum);

            if (nullptr == pkSk)
                continue;

            if (bJob && bJob != pkSk->dwType)
                continue;

            break;
        }
        while (true);

        item->SetSocket(0, dwSkillVnum);
    }
}

void CHARACTER::ReviveInvisible(int iDur) { AddAffect(AFFECT_REVIVE_INVISIBLE, POINT_NONE, 0, iDur, 0, true); }

void CHARACTER::ToggleMonsterLog()
{
    m_bMonsterLog = !m_bMonsterLog;

    if (m_bMonsterLog) { g_pCharManager->RegisterForMonsterLog(this); }
    else { g_pCharManager->UnregisterForMonsterLog(this); }
}

void CHARACTER::SetGuild(CGuild *pGuild)
{
    if (m_pGuild != pGuild)
    {
        m_pGuild = pGuild;
        UpdatePacket();
    }
}

void CHARACTER::BeginStateEmpty() { MonsterLog("!"); }

void CHARACTER::EffectPacket(int enumEffectType)
{
    TPacketGCSpecialEffect p;
    p.effectType = enumEffectType;
    p.type = 0;
    p.vid = GetVID();

    PacketAround(m_map_view, this,HEADER_GC_SEPCIAL_EFFECT, p);
}

void CHARACTER::SpecificEffectPacket(const std::string &filename)
{
    TPacketGCSpecificEffect p;
    p.vid = GetVID();
    p.effect_file = filename;
    PacketAround(m_map_view, this,HEADER_GC_SPECIFIC_EFFECT, p);
}

void CHARACTER::SpecificTargetEffectPacket(CHARACTER *target, const std::string &filename)
{
    if (!IsPC())
        return;

    TPacketGCSpecificEffect p;
    p.vid = target->GetVID();
    p.effect_file = filename;
    PacketAround(m_map_view, this,HEADER_GC_SPECIFIC_EFFECT, p);
}

void CHARACTER::MonsterChat(uint8_t bMonsterChatType)
{
    if (IsPC())
        return;

    std::string sbuf;

    if (IsMonster())
    {
        if (!Random::get<bool>(0.01))
            return;

        sbuf = fmt::sprintf("(gameforge.monster_chat[%i] and gameforge.monster_chat[%i][%d] or '')", GetRaceNum(),
                            GetRaceNum(), bMonsterChatType * 3 + Random::get(1, 3));
    }
    else
    {
        if (bMonsterChatType != MONSTER_CHAT_WAIT)
            return;

        if (IsGuardNPC())
        {
            if (!Random::get<bool>(0.6))
                return;
        }
        else
        {
            if (!Random::get<bool>(0.3))
                return;
        }

        sbuf = fmt::sprintf("(gameforge.monster_chat[%i] and gameforge.monster_chat[%i][number(1, "
                            "table.getn(gameforge.monster_chat[%i]))] or '')",
                            GetRaceNum(), GetRaceNum(), GetRaceNum());
    }

    for (const auto &elem : m_map_view)
    {
        if (!elem.first->IsType(ENTITY_CHARACTER))
            continue;

        const auto tch = static_cast<CHARACTER *>(elem.first);

        if (!tch->IsPC())
            continue;

        const auto text = quest::ScriptToString(GetLocale(tch)->name, sbuf);
        if (text.empty())
            return;

        TPacketGCChat pack_chat;
        pack_chat.type = CHAT_TYPE_TALKING;
        pack_chat.id = GetVID();
        pack_chat.bEmpire = 0;
        pack_chat.message = text;
        tch->GetDesc()->Send(HEADER_GC_CHAT, pack_chat);
    }
}

void CHARACTER::MonsterChat(const std::string &text)
{
    if (IsPC())
        return;

    if (!gConfig.testServer)
        return;

    TPacketGCChat pack_chat = {};
    pack_chat.type = CHAT_TYPE_TALKING;
    pack_chat.id = GetVID();
    pack_chat.bEmpire = 0;
    pack_chat.message = text;
    PacketAround(m_map_view, this,HEADER_CG_CHAT, pack_chat);
}

void CHARACTER::SetQuestNPCID(uint32_t vid) { m_dwQuestNPCVID = vid; }

CHARACTER *CHARACTER::GetQuestNPC() const { return g_pCharManager->Find(m_dwQuestNPCVID); }

void CHARACTER::SetBlockMode(uint32_t flag) { m_points.blockMode = flag; }

bool CHARACTER::IsBlockMode(uint32_t flag, uint32_t pid) const
{
    if (m_points.blockMode & flag)
        return true;

    if (pid)
    {
        const auto it = m_messengerBlock.find(pid);
        if (it != m_messengerBlock.end() && it->second.mode & flag)
            return true;
    }

    return false;
}

bool CHARACTER::IsGuardNPC() const
{
    return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004);
}

int CHARACTER::GetPolymorphPower() const
{
    if (gConfig.testServer)
    {
        int value = quest::CQuestManager::instance().GetEventFlag("poly");
        if (value)
            return value;
    }
    return aiPolymorphPowerByLevel[std::clamp(GetSkillLevel(SKILL_POLYMORPH), 0, 40)];
}

void CHARACTER::SetPolymorph(uint32_t dwRaceNum, bool bMaintainStat)
{
    if (m_dwPolymorphRace == dwRaceNum)
        return;

    m_bPolyMaintainStat = bMaintainStat;
    m_dwPolymorphRace = dwRaceNum;

    SPDLOG_INFO("POLYMORPH: {0} race {1} ", GetName(), dwRaceNum);

    if (dwRaceNum != 0)
        StopRiding();

    SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

    ViewReencode();

    REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

    if (!bMaintainStat)
    {
        PointChange(POINT_ST, 0);
        PointChange(POINT_DX, 0);
        PointChange(POINT_IQ, 0);
        PointChange(POINT_HT, 0);
    }

    // 폴리모프 상태에서 죽는 경우, 폴리모프가 풀리게 되는데
    // 폴리 모프 전후로 valid combo interval이 다르기 때문에
    // Combo 핵 또는 Hacker로 인식하는 경우가 있다.
    // 따라서 폴리모프를 풀거나 폴리모프 하게 되면,
    // valid combo interval을 reset한다.
    m_comboInputTime = 0;
    m_comboDuration = 0;
    SetValidComboInterval(0);
    SetComboSequence(0);

    ComputeBattlePoints();
}

int CHARACTER::GetQuestFlag(const std::string &flag) const
{
    if (!IsPC())
    {
        SPDLOG_ERROR("Trying to get qf %s from non player character", flag.c_str());
        return 0;
    }
    uint32_t pid = GetPlayerID();

    quest::CQuestManager &q = quest::CQuestManager::instance();
    quest::PC *pPC = q.GetPC(pid);

    if (!pPC)
    {
        SPDLOG_ERROR("Nullpointer when trying to access questflag %s for player with pid %lu", flag.c_str(), pid);
        return 0;
    }

    return pPC->GetFlag(flag);
}

void CHARACTER::SetQuestFlag(const std::string &flag, int value)
{
    uint32_t pid = GetPlayerID();

    quest::CQuestManager &q = quest::CQuestManager::instance();
    quest::PC *pPC = q.GetPC(pid);

    if (!pPC)
    {
        SPDLOG_ERROR("Nullpointer when trying to set questflag %s for player with pid %lu", flag.c_str(), pid);
        return;
    }

    pPC->SetFlag(flag, value);
}

void CHARACTER::DetermineDropMetinStone()
{
    const int METIN_STONE_NUM = 14;
    static uint32_t c_adwMetin[METIN_STONE_NUM] = {
        28030, 28031, 28032, 28033, 28034, 28035, 28036, 28037, 28038, 28039, 28040, 28041, 28042, 28043,
#ifdef ENABLE_WOLFMAN_CHARACTER
        28012,
#endif
    };
    uint32_t stone_num = GetRaceNum();
    int idx = std::lower_bound(aStoneDrop, aStoneDrop + STONE_INFO_MAX_NUM, stone_num) - aStoneDrop;
    if (idx >= STONE_INFO_MAX_NUM || aStoneDrop[idx].dwMobVnum != stone_num) { m_dwDropMetinStone = 0; }
    else
    {
        const SStoneDropInfo &info = aStoneDrop[idx];
        m_bDropMetinStonePct = info.iDropPct;
        {
            m_dwDropMetinStone = c_adwMetin[Random::get(0, METIN_STONE_NUM - 1)];
            int iGradePct = Random::get(1, 100);
            for (int iStoneLevel = 0; iStoneLevel < STONE_LEVEL_MAX_NUM; iStoneLevel++)
            {
                int iLevelGradePortion = info.iLevelPct[iStoneLevel];
                if (iGradePct <= iLevelGradePortion) { break; }
                else
                {
                    iGradePct -= iLevelGradePortion;
                    m_dwDropMetinStone += 100; // 돌 +a -> +(a+1)이 될때마다 100씩 증가
                }
            }
        }
    }
}

void CHARACTER::SendEquipment(CHARACTER *ch)
{
    TPacketViewEquip p;
    p.vid = GetVID();
    for (int i = 0; i < WEAR_MAX_NUM; i++)
    {
        CItem *item = GetWear(i);
        if (item)
        {
            p.items[i].vnum = item->GetVnum();
            p.items[i].count = item->GetCount();

            std::memcpy(p.items[i].sockets, item->GetSockets(), sizeof(p.items[i].sockets));
            std::memcpy(p.items[i].attrs, item->GetAttributes(), sizeof(p.items[i].attrs));
        }
        else { p.items[i].vnum = 0; }
    }
    ch->GetDesc()->Send(HEADER_GC_VIEW_EQUIP, p);
}

bool CHARACTER::CanSummon(int iLeaderShip) { return (iLeaderShip >= 30 && m_dwLastDeadTime + 300 > get_dword_time()); }

void CHARACTER::SetMountVnum(uint32_t vnum)
{
    if (m_dwMountVnum == vnum)
        return;

    m_dwMountVnum = 0;
}

void CHARACTER::MountVnum(uint32_t vnum)
{
    if (m_dwMountVnum == vnum)
        return;

    GcMountPacket p;
    p.vid = GetVID();
    p.vnum = vnum;
    p.x = GetX();
    p.y = GetY();
    PacketView(m_map_view, static_cast<CEntity*>(this), HEADER_GC_MOUNT, p);

    const bool mustRecompute = (m_dwMountVnum == 0 && vnum != 0) || (m_dwMountVnum != 0 && vnum == 0);

    m_dwMountVnum = vnum;
    m_dwMountTime = get_dword_time();

    m_comboDuration = 0;
    m_comboInputTime = 0;
    SetValidComboInterval(0);
    SetComboSequence(0);

#ifdef __FAKE_PC__
    if (auto pkSup = FakePC_Owner_GetSupporter())
        pkSup->ViewReencode();
#endif

    if (mustRecompute)
    {
        PointChange(POINT_MOV_SPEED, 0);
        PointChange(POINT_ATT_SPEED, 0);
        PointChange(POINT_ST, 0);
        PointChange(POINT_DX, 0);
        PointChange(POINT_HT, 0);
        PointChange(POINT_IQ, 0);
        UpdatePacket();
    }
    //	ComputePoints();
}

namespace
{
struct FuncWarp
{
    FuncWarp(CHARACTER *pkWarp)
        : m_bInvalid(false), m_bUseWarp(true), m_lX(pkWarp->GetX()), m_lY(pkWarp->GetY()), m_lTargetX(0), m_lTargetY(0),
          m_targetMapIndex(0), m_bEmpire(pkWarp->GetEmpire())
    {
        char szTmp[64];

        if (pkWarp->IsGoto())
        {
            m_bUseWarp = false;
            if (3 != sscanf(pkWarp->GetName().c_str(), " %s %d %d ", szTmp, &m_lTargetX, &m_lTargetY))
            {
                SPDLOG_ERROR("Warp (goto) NPC name wrong : vnum(%d) name(%s)", pkWarp->GetRaceNum(), pkWarp->GetName());

                m_bInvalid = true;
                return;
            }
        }
        else
        {
            if (4 !=
                sscanf(pkWarp->GetName().c_str(), " %s %d %d %d ", szTmp, &m_targetMapIndex, &m_lTargetX, &m_lTargetY))
            {
                SPDLOG_ERROR("Warp NPC name wrong : vnum({0}) name({1})", pkWarp->GetRaceNum(), pkWarp->GetName());

                m_bInvalid = true;
                return;
            }
        }

        m_lTargetX *= 100;
        m_lTargetY *= 100;
    }

    bool Valid() { return !m_bInvalid; }

    void operator()(CEntity *ent)
    {
        if (!Valid())
            return;

        if (!ent->IsType(ENTITY_CHARACTER))
            return;

        CHARACTER *pkChr = (CHARACTER *)ent;

        if (!pkChr->IsPC())
            return;

        int iDist = DISTANCE_APPROX(pkChr->GetX() - m_lX, pkChr->GetY() - m_lY);
        if (iDist > 300)
            return;

        if (m_bEmpire && pkChr->GetEmpire() && m_bEmpire != pkChr->GetEmpire())
            return;

        if (!pkChr->CanHandleItem(false, true))
            return;

        if (pkChr->IsHack())
            return;

        if (m_bUseWarp)
            pkChr->WarpSet(m_targetMapIndex, m_lTargetX, m_lTargetY);
        else { pkChr->Show(pkChr->GetMapIndex(), m_lTargetX, m_lTargetY); }
    }

    bool m_bInvalid;
    bool m_bUseWarp;

    int32_t m_lX;
    int32_t m_lY;
    int32_t m_lTargetX;
    int32_t m_lTargetY;
    int32_t m_targetMapIndex;

    uint8_t m_bEmpire;
};
} // namespace
EVENTFUNC(warp_npc_event)
{
    char_event_info *info = static_cast<char_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_ERROR("warp_npc_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;
    if (ch == nullptr) // <Factor>
        return 0;

    if (!ch->GetSectree())
        return 0;

    FuncWarp f(ch);
    if (!f.Valid())
        return 0;

    ch->ForEachSeen(f);
    return THECORE_SECS_TO_PASSES(1) / 2;
}

void CHARACTER::StartWarpNPCEvent()
{
    if (m_pkWarpNPCEvent)
        return;

    if (!IsWarp() && !IsGoto())
        return;

    char_event_info *info = AllocEventInfo<char_event_info>();

    info->ch = this;

    m_pkWarpNPCEvent = event_create(warp_npc_event, info, THECORE_SECS_TO_PASSES(1) / 2);
}

CHARACTER *CHARACTER::GetMarryPartner() const { return m_pkChrMarried; }

void CHARACTER::SetMarryPartner(CHARACTER *ch) { m_pkChrMarried = ch; }

int CHARACTER::GetMarriageBonus(uint32_t dwItemVnum, bool bSum)
{
    if (IsNPC())
        return 0;

    marriage::TMarriage *pMarriage = marriage::CManager::instance().Get(GetPlayerID());

    if (!pMarriage)
        return 0;

    return pMarriage->GetBonus(dwItemVnum, bSum, this);
}

void CHARACTER::ConfirmWithMsg(const char *szMsg, int iTimeout, uint32_t dwRequestPID)
{
    if (!IsPC())
        return;

    TPacketGCQuestConfirm p;
    p.requestPID = dwRequestPID;
    p.timeout = iTimeout;
    p.msg = szMsg;
    GetDesc()->Send(HEADER_GC_QUEST_CONFIRM, p);
}

int CHARACTER::GetPremiumRemainSeconds(uint8_t bType) const
{
    if (bType >= PREMIUM_MAX_NUM)
        return 0;

    if(!GetDesc())
        return 0;

    if(!GetDesc()->GetPremiumTimes())
        return 0;

    return GetDesc()->GetPremiumTimes()[bType] - get_global_time();
}

bool CHARACTER::WarpToPID(uint32_t dwPID)
{
    if (MeleyLair::CMgr::instance().IsMeleyMap(GetMapIndex()))
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "CANNOT_WARP_NOW");
        return false;
    }

    CHARACTER *victim;
    if ((victim = (g_pCharManager->FindByPID(dwPID))))
    {
        int mapIdx = victim->GetMapIndex();
        if (IsSummonableZone(mapIdx) || IsGM())
        {
            if (CAN_ENTER_ZONE(this, mapIdx)) { WarpSet(mapIdx, victim->GetX(), victim->GetY()); }
            else
            {
                SendI18nChatPacket(this, CHAT_TYPE_INFO, "상대방이 있는 곳으로 워프할 수 없습니다.");
                return false;
            }
        }
        else
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "상대방이 있는 곳으로 워프할 수 없습니다.");
            return false;
        }
    }
    else
    {
        // 다른 서버에 로그인된 사람이 있음 -> 메시지 보내 좌표를 받아오자
        // 1. A.pid, B.pid 를 뿌림
        // 2. B.pid를 가진 서버가 뿌린서버에게 A.pid, 좌표 를 보냄
        // 3. 워프
        const auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(dwPID);
        if (!op)
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "상대방이 온라인 상태가 아닙니다.");
            return false;
        }

        if (!IsSummonableZone(op->mapIndex) && !IsGM())
        {
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "상대방이 있는 곳으로 워프할 수 없습니다.");
            return false;
        }
        else
        {
            if (!CAN_ENTER_ZONE(this, op->mapIndex))
            {
                SendI18nChatPacket(this, CHAT_TYPE_INFO, "상대방이 있는 곳으로 워프할 수 없습니다.");
                return false;
            }

            GmCharacterTransferPacket p;
            p.sourcePid = m_dwPlayerID;
            p.targetPid = dwPID;
            DESC_MANAGER::instance().GetMasterSocket()->Send(kGmCharacterTransfer, p);

            if (gConfig.testServer)
                ChatPacket(CHAT_TYPE_PARTY, "sent find position packet for teleport");
        }
    }
    return true;
}

// ADD_REFINE_BUILDING
CGuild *CHARACTER::GetRefineGuild() const
{
    CHARACTER *chRefineNPC = GetRefineNPC();
    return (chRefineNPC ? chRefineNPC->GetGuild() : nullptr);
}

CHARACTER *CHARACTER::GetRefineNPC() const
{
    if (!m_dwRefineNPCVID)
        return nullptr;

    return g_pCharManager->Find(m_dwRefineNPCVID);
}

bool CHARACTER::IsRefineThroughGuild() const { return GetRefineGuild() != nullptr; }

Gold CHARACTER::ComputeRefineFee(Gold iCost, int iMultiply) const
{
    CGuild *pGuild = GetRefineGuild();
    if (pGuild)
    {
        if (pGuild == GetGuild())
            return iCost * iMultiply * 9 / 10;

        // 다른 제국 사람이 시도하는 경우 추가로 3배 더
        CHARACTER *chRefineNPC = GetRefineNPC();
        if (chRefineNPC && chRefineNPC->GetEmpire() != GetEmpire())
            return iCost * iMultiply * 3;

        return iCost * iMultiply;
    }
    else
        return iCost;
}

void CHARACTER::PayRefineFee(Gold iTotalMoney)
{
    Gold iFee = iTotalMoney / 10;
    CGuild *pGuild = GetRefineGuild();

    Gold iRemain = iTotalMoney;

    if (pGuild)
    {
        // 자기 길드이면 iTotalMoney에 이미 10%가 제외되어있다
        if (pGuild != GetGuild())
        {
            pGuild->RequestDepositMoney(this, iFee);
            iRemain -= iFee;
        }
    }

    ChangeGold(-iRemain);
}

// END_OF_ADD_REFINE_BUILDING

// Hack 방지를 위한 체크.
bool CHARACTER::IsHack(bool bSendMsg, bool bCheckShopOwner, int limittime)
{
    const int iPulse = thecore_pulse();

    if (gConfig.testServer)
        bSendMsg = true;

    //창고 연후 체크
    if (iPulse - GetSafeboxLoadTime() < THECORE_SECS_TO_PASSES(limittime))
    {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "창고를 연후 %d초 이내에는 다른곳으로 이동할수 없습니다.",
                               limittime);

        if (gConfig.testServer)
            ChatPacket(CHAT_TYPE_INFO, "[TestOnly]Pulse %d LoadTime %d PASS %d", iPulse, GetSafeboxLoadTime(),
                       THECORE_SECS_TO_PASSES(limittime));
        return true;
    }
    /*
    //거래관련 창 체크
    if (bCheckShopOwner)
    {
        if (GetExchange() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
        {
            if (bSendMsg)
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "거래창,창고 등을 연 상태에서는 다른곳으로 이동,종료 할수 없습니다");

            return true;
        }
    }
    else
    {
        if (GetExchange() || IsOpenSafebox() || IsCubeOpen())
        {
            if (bSendMsg)
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "거래창,창고 등을 연 상태에서는 다른곳으로 이동,종료 할수 없습니다");

            return true;
        }
    }

    // PREVENT_PORTAL_AFTER_EXCHANGE
    //교환 후 시간체크
     if (iPulse - GetExchangeTime() < THECORE_SECS_TO_PASSES(limittime))
    {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "거래 후 %d초 이내에는 다른지역으로 이동 할 수 없습니다.",
                               limittime);
        return true;
    }
    // END_PREVENT_PORTAL_AFTER_EXCHANGE

    // PREVENT_ITEM_COPY
    if (iPulse - GetMyShopTime() < THECORE_SECS_TO_PASSES(limittime))
    {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "거래 후 %d초 이내에는 다른지역으로 이동 할 수 없습니다.",
                               limittime);
        return true;
    }

    if (iPulse - GetRefineTime() < THECORE_SECS_TO_PASSES(limittime))
    {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "아이템 개량후 %d초 이내에는 귀환부,귀환기억부를 사용할 수 없습니다.", limittime);
        return true;
    }

    if (iPulse - GetBuffbotTime() < THECORE_SECS_TO_PASSES(limittime))
    {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "You cannot do this shortly after using a buff bot.", limittime);
        return true;
    }

#ifdef __OFFLINE_SHOP__
    if ((iPulse - this->GetOfflineShopLastOpeningTime()) < THECORE_SECS_TO_PASSES(limittime))
    {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO, "You cannot go elsewhere for %d seconds after trading.",
                               limittime);

        return true;
    }
#endif

*/
    // END_PREVENT_ITEM_COPY
    if (get_dword_time() - GetLastPlayerHitReceivedTime() <
        gConfig.portalLimitTime * 200) {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "Duello(katledilme) sonrasında %d saniye sonra "
                               "kalkabilirsiniz.",
                               limittime);
        return true;
    }

    if (get_dword_time() - GetLastPlayerHitTime() < gConfig.portalLimitTime * 200) {
        if (bSendMsg)
            SendI18nChatPacket(this, CHAT_TYPE_INFO,
                               "Duello(katledilme) sonrasında %d saniye sonra "
                               "kalkabilirsiniz.",
                               limittime);
        return true;
    }
    


    return false;
}

void CHARACTER::Say(const std::string &s)
{
    if (!GetDesc())
        return;

    packet_script pack;
    pack.skin = 1;
    pack.script = s;
    GetDesc()->Send(HEADER_GC_SCRIPT, pack);
}

bool CHARACTER::IsSiegeNPC() const
{
    return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004);
}

void CHARACTER::UpdateEmotionPulse() { m_emotionPulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(3)); }

void CHARACTER::UpdateBraveryCapePulse() { m_braveryCapePulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(1) / 2); }

void CHARACTER::UpdateEquipPulse() { m_equipPulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(3)); }

void CHARACTER::UpdateAutoPotionPulseTp() { m_autoPotionPulseTp = thecore_pulse() + (THECORE_SECS_TO_PASSES(6)); }

void CHARACTER::UpdateAutoPotionPulseMp() { m_autoPotionPulseMp = thecore_pulse() + (THECORE_SECS_TO_PASSES(6)); }

void CHARACTER::UpdateEquipSkillUsePulse()
{
    m_equipSkillUsePulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(1) * 0.75f);
}

void CHARACTER::UpdateBraveryCapePulsePremium()
{
    m_braveryCapePulsePremium = thecore_pulse() + (THECORE_SECS_TO_PASSES(1) / 2);
}

void CHARACTER::UpdateFireworksPulse() { n_fireworksPulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(2)); }

void CHARACTER::UpdateUseItemPulse() { m_useItemPulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(1) / 5); }

void CHARACTER::UpdateComboChangePulse() { m_comboChangePulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(5)); }

void CHARACTER::UpdateShopPulse() { m_shopPulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(1) / 5); }

void CHARACTER::UpdateFriendshipRequestPulse()
{
    m_friendRequestPulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(10));
}

void CHARACTER::UpdateOfflineMessagePulse() { m_offlineMessagePulse = thecore_pulse() + THECORE_SECS_TO_PASSES(1); }

void CHARACTER::UpdateMountPulse() { m_mountPulse = thecore_pulse() + THECORE_SECS_TO_PASSES(1); }

bool CHARACTER::IsNextEmotionPulse() const { return (m_emotionPulse == 0 || (m_emotionPulse < thecore_pulse())); }

bool CHARACTER::IsNextMountPulse() const { return (m_mountPulse == 0 || (m_mountPulse < thecore_pulse())); }

bool CHARACTER::IsNextComboChangePulse() const
{
    return (m_comboChangePulse == 0 || (m_comboChangePulse < thecore_pulse()));
}

bool CHARACTER::IsNextOfflineMessagePulse() const
{
    return (m_offlineMessagePulse == 0 || (m_offlineMessagePulse < thecore_pulse()));
}

void CHARACTER::UpdateCampfirePulse() { m_campFirePulse = thecore_pulse() + (THECORE_SECS_TO_PASSES(2)); }

bool CHARACTER::CanUseBraveryCape() const
{
    return (m_braveryCapePulse == 0 || (m_braveryCapePulse < thecore_pulse()));
}

bool CHARACTER::CanUseCampfire() const { return (m_campFirePulse == 0 || (m_campFirePulse < thecore_pulse())); }

bool CHARACTER::CanUseFireworks() const { return (n_fireworksPulse == 0 || (n_fireworksPulse < thecore_pulse())); }

bool CHARACTER::IsNextUseItemPule() const { return (m_useItemPulse == 0 || (m_useItemPulse < thecore_pulse())); }

bool CHARACTER::IsNextAutoPotionPulseTp() const
{
    return (m_autoPotionPulseTp == 0 || (m_autoPotionPulseTp < thecore_pulse()));
}

bool CHARACTER::IsNextAutoPotionPulseMp() const
{
    return (m_autoPotionPulseMp == 0 || (m_autoPotionPulseMp < thecore_pulse()));
}

bool CHARACTER::IsNextShopPulse() const { return (m_shopPulse == 0 || (m_shopPulse < thecore_pulse())); }

bool CHARACTER::IsNextFriendshipRequestPulse() const
{
    return (m_friendRequestPulse == 0 || (m_friendRequestPulse < thecore_pulse()));
}

bool CHARACTER::IsNextEquipPulse() const { return (m_equipPulse == 0 || (m_equipPulse < thecore_pulse())); }

bool CHARACTER::IsNextEquipSkillUsePulse() const
{
    return (m_equipSkillUsePulse == 0 || (m_equipSkillUsePulse < thecore_pulse()));
}

bool CHARACTER::CanUseBraveryCapePremium() const
{
    return (m_braveryCapePulsePremium == 0 || (m_braveryCapePulsePremium < thecore_pulse()));
}

//------------------------------------------------
void CHARACTER::UpdateDepositPulse()
{
    m_deposit_pulse = thecore_pulse() + THECORE_SECS_TO_PASSES(60 * 5); // 5분
}

bool CHARACTER::CanDeposit() const { return (m_deposit_pulse == 0 || (m_deposit_pulse < thecore_pulse())); }

CHARACTER *CHARACTER::CreateToggleMount(CItem *mountItem)
{
    m_toggleMount.reset();

    auto mountVnum = mountItem->GetValue(ITEM_VALUE_MOUNT_VNUM);

    if (mountItem->GetTransmutationVnum())
    {
        const auto* tbl = ITEM_MANAGER::instance().GetTable(mountItem->GetTransmutationVnum());
        if (tbl) { mountVnum = tbl->alValues[ITEM_VALUE_MOUNT_VNUM]; }
    }

    CHARACTER *pkCharacter =
        g_pCharManager->SpawnMob(mountVnum, GetMapIndex(), GetX() + Random::get(-50, 50), GetY() + Random::get(-50, 50),
                                 0, false, -1, false, false, 100, true);

    if (!pkCharacter)
    {
        SPDLOG_ERROR("Failed to create mount character.");
        return nullptr;
    }

    uint32_t itemId = mountItem->GetID();
    std::string sName = CItemNameManager::instance().GetItemName(itemId);
    if (sName.empty())
    {
        db_clientdesc->DBPacket(HEADER_GD_REQ_ITEM_NAME, 0, &itemId, sizeof(uint32_t));
        sName = GetName();
        sName += LC_TEXT("'s Mount");
    }
    else { pkCharacter->SetHasCustomeName(true); }

    pkCharacter->SetName(sName);

    pkCharacter->SetEmpire(GetEmpire());
    pkCharacter->SetRider(this);
    pkCharacter->SetStateHorse();
    pkCharacter->SetAsToggleMount();
    pkCharacter->MarkAsMount();
    auto vid = GetVID();
    pkCharacter->SetRequirementFunction([vid](CHARACTER *player) -> bool
    {
        if (!player || !player->IsPC())
            return false;

        const auto* owner = g_pCharManager->Find(vid);
        if (!owner)
            return false;

        if (owner->IsObserverMode()) { return player->IsObserverMode(); }

        return true;
    });

    pkCharacter->Show(GetMapIndex(), GetX() + Random::get(-50, 50), GetY() + Random::get(-50, 50));

    m_toggleMount.reset(pkCharacter);

    return m_toggleMount.get();
}

void CHARACTER::SendHuntingMissions()
{
    GcHuntingMissionPacket p;
    p.load = m_huntingMissions;
    GetDesc()->Send(HEADER_GC_HUNTING_MISSIONS, p);
}

void CHARACTER::LoadHuntingMissions(uint32_t dwCount, TPlayerHuntingMission *data)
{
    for (uint32_t i = 0; i < dwCount; i++)
    {
        m_huntingMissions.push_back(*data);
        ++data;
    }
    SendHuntingMissions();
}

uint32_t CHARACTER::GetHuntingMissionProgress(uint32_t dwMissionID)
{
    for (const auto &pkMission : m_huntingMissions)
    {
        if (pkMission.missionId == dwMissionID)
            return pkMission.killCount;
    }

    return 0;
}

bool CHARACTER::IsHuntingMissionCompleted(uint32_t dwMissionID)
{
    for (const auto &pkMission : m_huntingMissions)
    {
        if (pkMission.missionId == dwMissionID)
            return pkMission.bCompleted;
    }

    return false;
}

bool CHARACTER::IsHuntingMissionForCurrentDay(uint32_t dwMissionID)
{
    std::time_t ti = std::time(nullptr);
    auto* t = std::localtime(&ti);

    for (const auto &pkMission : m_huntingMissions)
    {
        if (pkMission.missionId == dwMissionID)
            return pkMission.day == t->tm_mday;
    }

    return false;
}

bool CHARACTER::UpdateHuntingMission(uint32_t dwMissionID, uint32_t dwUpdateValue, uint32_t dwTotalValue,
                                     bool isOverride)
{
    const auto *mission = HuntingManager::instance().GetHuntingMissionById(dwMissionID);
    if (!mission)
        return false;

    bool foundMission = false;
    uint32_t dwSaveProgress = 0;
    std::time_t ti = std::time(nullptr);
    auto* t = std::localtime(&ti);

    auto it = m_huntingMissions.begin();
    while (it != m_huntingMissions.end())
    {
        TPlayerHuntingMission &pkMission = *it++;

        if (pkMission.missionId == dwMissionID)
        {
            pkMission.bIsUpdated = 1;

            if(mission->isDaily)
                pkMission.day = t->tm_mday;

            if (isOverride)
                pkMission.killCount = dwUpdateValue;
            else
                pkMission.killCount += dwUpdateValue;

            if (pkMission.killCount >= dwTotalValue)
            {
                pkMission.killCount = dwTotalValue;
                pkMission.bCompleted = 1;
                db_clientdesc->DBPacket(HEADER_GD_SAVE_HUNTING_MISSION, 0, &pkMission, sizeof(TPlayerHuntingMission));

                GiveHuntingMissionReward(mission);

                // CBattlePass::instance().CheckBattlePassCompleted(this);
            }

            dwSaveProgress = pkMission.killCount;
            foundMission = true;
            break;
        }
    }

    if (!foundMission)
    {
        TPlayerHuntingMission newMission;
        newMission.playerId = GetPlayerID();
        newMission.missionId = dwMissionID;
        if(mission->isDaily)
            newMission.day = t->tm_mday;

        if (dwUpdateValue >= dwTotalValue)
        {
            newMission.killCount = dwTotalValue;
            newMission.bCompleted = 1;

            GiveHuntingMissionReward(mission);

            dwSaveProgress = dwTotalValue;
        }
        else
        {
            newMission.killCount = dwUpdateValue;
            newMission.bCompleted = 0;

            dwSaveProgress = dwUpdateValue;
        }

        newMission.bIsUpdated = 1;

        m_huntingMissions.push_back(newMission);
    }

    if (!GetDesc())
        return false;

    auto res = std::find_if(m_huntingMissions.begin(), m_huntingMissions.end(), [&](const TPlayerHuntingMission& m)
    {
        return m.missionId == dwMissionID;
    });

    if(res == m_huntingMissions.end())
        return false;

    GcHuntingMissionPacket p;
    p.update = *(res);
    GetDesc()->Send(HEADER_GC_HUNTING_MISSIONS, p);

    return true;
}

void CHARACTER::GiveHuntingMissionReward(const HuntingMission *mission)
{
    for (const auto &reward : mission->rewards)
    {
        if (reward.vnum == 3) { PointChange(POINT_SKILLTREE_POINTS, reward.count); }
        else { AutoGiveItem(reward.vnum, reward.count); }
    }

}

int CHARACTER::GetHPPct(int perc) const { return static_cast<int64_t>((GetHP() * perc) / GetMaxHP()); }

#define IS_SPEED_HACK_PLAYER(ch) ((ch)->m_speed_hack_count > gConfig.speedHackLimitCount)

EVENTFUNC(check_speedhack_event)
{
    char_event_info *info = dynamic_cast<char_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_INFO("check_speedhack_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;

    if (nullptr == ch || ch->IsNPC())
        return 0;

    if (IS_SPEED_HACK_PLAYER(ch))
    {
        // write hack log
        LogManager::instance().SpeedHackLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), ch->m_speed_hack_count);

        // close connection
        DESC* desc = ch->GetDesc();

        if (desc)
        {
            desc->DelayedDisconnect(Random::get(3, 10), "Speedhack");
            return 0;
        }
    }

    ch->m_speed_hack_count = 0;

    ch->ResetComboHackCount();
    return THECORE_SECS_TO_PASSES(60);
}

void CHARACTER::StartCheckSpeedHackEvent()
{
    if (m_pkCheckSpeedHackEvent)
        return;

    char_event_info *info = AllocEventInfo<char_event_info>();

    info->ch = this;

    m_pkCheckSpeedHackEvent = event_create(check_speedhack_event, info, THECORE_SECS_TO_PASSES(60)); // 1분
}

void CHARACTER::GoHome()
{
    WarpSet(EMPIRE_START_MAP(GetEmpire()), EMPIRE_START_X(GetEmpire()), EMPIRE_START_Y(GetEmpire()));
}

void CHARACTER::SendGuildName(CGuild *pGuild)
{
    if (nullptr == pGuild)
        return;

    DESC *desc = GetDesc();

    if (nullptr == desc)
        return;
    if (m_known_guild.find(pGuild->GetID()) != m_known_guild.end())
        return;

    m_known_guild.insert(pGuild->GetID());

    TPacketGCGuild pack = {};

    pack.subheader = GUILD_SUBHEADER_GC_GUILD_NAME;
            const GuildIdAndName n{pGuild->GetID(), pGuild->GetName()};

    pack.name = n;

    desc->Send(HEADER_GC_GUILD, pack);
}

void CHARACTER::SendGuildName(uint32_t dwGuildID) { SendGuildName(CGuildManager::instance().FindGuild(dwGuildID)); }

EVENTFUNC(destroy_when_idle_event)
{
    char_event_info *info = static_cast<char_event_info *>(event->info);
    if (info == nullptr)
    {
        SPDLOG_ERROR("destroy_when_idle_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER *ch = info->ch;
    if (ch == nullptr)
    {
        // <Factor>
        return 0;
    }

    if (ch->GetVictim()) { return THECORE_SECS_TO_PASSES(300); }

    if (ch->IsToggleMount()) { return 0; }

    SPDLOG_TRACE("DESTROY_WHEN_IDLE: %s", ch->GetName());

    ch->m_pkDestroyWhenIdleEvent = nullptr;
    M2_DESTROY_CHARACTER(ch);
    return 0;
}

void CHARACTER::StartDestroyWhenIdleEvent()
{
    if (m_pkDestroyWhenIdleEvent)
        return;

    char_event_info *info = AllocEventInfo<char_event_info>();

    info->ch = this;

    m_pkDestroyWhenIdleEvent = event_create(destroy_when_idle_event, info, THECORE_SECS_TO_PASSES(300));
}

void CHARACTER::SetValidComboInterval(int interval) { m_iValidComboInterval = interval; }

int CHARACTER::GetValidComboInterval() const { return m_iValidComboInterval; }

void CHARACTER::IncreaseComboHackCount(int k)
{
    m_iComboHackCount += k;

    if (m_iComboHackCount >= 10 && GetDesc())
    {
        if (GetDesc()->DelayedDisconnect(Random::get(2, 7), "COMBO_HACK_DISCONNECT"))
        {
            SPDLOG_INFO("COMBO_HACK_DISCONNECT: %s count: %d", GetName(), m_iComboHackCount);
            LogManager::instance().HackLog("Combo", this);
        }
    }
}

void CHARACTER::ResetComboHackCount() { m_iComboHackCount = 0; }

void CHARACTER::UpdateSyncHackCount(const std::string &who, bool increase)
{
    int variance = increase ? 1 : -1;
    if (who == "monster")
        m_iSyncMonsterHackCount += variance;
    else if (who == "player")
        m_iSyncPlayerHackCount += variance;
}

int CHARACTER::GetSyncHackCount(const std::string &who)
{
    if (who == "monster") { return m_iSyncMonsterHackCount; }

    if (who == "player") { return m_iSyncPlayerHackCount; }

    return -1;
}

void CHARACTER::SkipComboAttackByTime(int interval) { m_dwSkipComboAttackByTime = get_dword_time() + interval; }

uint32_t CHARACTER::GetSkipComboAttackByTime() const { return m_dwSkipComboAttackByTime; }

void CHARACTER::SetComboSequence(uint8_t seq) { m_bComboSequence = seq; }

uint8_t CHARACTER::GetComboSequence() const { return m_bComboSequence; }

// 말이나 다른것을 타고 있나?
bool CHARACTER::IsRiding() const
{
#ifdef __FAKE_PC__
    if (FakePC_Check())
        return FakePC_GetOwner()->IsRiding();
#endif

    return IsHorseRiding() || GetMountVnum();
}

bool CHARACTER::CanWarp() const
{
    const int iPulse = thecore_pulse();
    const int limit_time = THECORE_SECS_TO_PASSES(gConfig.portalLimitTime);

    if ((iPulse - GetSafeboxLoadTime()) < limit_time)
        return false;

    if ((iPulse - GetExchangeTime()) < limit_time)
        return false;

    if ((iPulse - GetMyShopTime()) < limit_time)
        return false;

    if ((iPulse - GetRefineTime()) < limit_time)
        return false;

    if ((iPulse - GetBuffbotTime()) < limit_time)
        return false;

    if(get_dword_time() - GetLastPlayerHitReceivedTime() < gConfig.portalLimitTime * 1000)
        return false;

    if(get_dword_time() - GetLastPlayerHitTime() < gConfig.portalLimitTime * 1000)
        return false;
    

    if (GetExchange() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
        return false;

    return true;
}

PointValue CHARACTER::GetNextExp() const
{
    if (PLAYER_EXP_TABLE_MAX < GetLevel())
        return 2500000000u;

    return exp_table[GetLevel()];
}

bool CHARACTER::CanFall()
{
    if (IsImmune(IMMUNE_FALL)) // Immune flag
        return false;

    return true;
}

bool CHARACTER::IsInSafezone() const
{
    SECTREE *sectree = GetSectree();
    return (sectree && sectree->IsAttr(GetX(), GetY(), ATTR_BANPK));
}

#ifdef __OFFLINE_MESSAGE__
void CHARACTER::CheckOfflineMessage()
{
    std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(
        "SELECT sender,message,id FROM offline_messages WHERE pid={} AND received = 0", GetPlayerID()));

    if (pMsg->Get()->rows.size() >= 1)
    {
        for(const auto& row : pMsg->Get()->rows) {
            TPacketGCWhisper pack{};
            pack.bType = GM::get_level(row[0].c_str()) ? WHISPER_TYPE_GM : WHISPER_TYPE_NORMAL;
            pack.szNameFrom =  row[0];
            pack.message =  fmt::format("[Offline-PM]{}", row[1]);

            GetDesc()->Send(HEADER_GC_WHISPER, pack);

            DBManager::instance().DirectQuery("UPDATE offline_messages SET received = 1 where id='{}';", row[2]);
        }
    }
}
#endif

#if defined(WJ_COMBAT_ZONE)
void CHARACTER::UpdateCombatZoneRankings(const char *memberName, uint32_t memberEmpire, uint32_t memberPoints)
{
    DBManager::instance().DirectQuery(
        "INSERT INTO %s.combat_zone_ranking_weekly (memberName, memberEmpire, memberPoints) VALUES('%s', '%d', '%d') "
        "ON DUPLICATE KEY UPDATE memberPoints = memberPoints + '%d'",
        g_PlayerDatabase.c_str(), memberName, memberEmpire, memberPoints, memberPoints);
    DBManager::instance().DirectQuery(
        "INSERT INTO %s.combat_zone_ranking_general (memberName, memberEmpire, memberPoints) VALUES('%s', '%d', '%d') "
        "ON DUPLICATE KEY UPDATE memberPoints = memberPoints + '%d'",
        g_PlayerDatabase.c_str(), memberName, memberEmpire, memberPoints, memberPoints);
}

uint8_t CHARACTER::GetCombatZoneRank()
{
    if (GetDesc() != NULL)
    {
        std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(
            "SELECT combat_zone_rank FROM %s.player WHERE id = %u", g_PlayerDatabase.c_str(), GetPlayerID()));
        MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
        return atoi(row[0]);
    }

    return 0;
}

uint32_t CHARACTER::GetRealCombatZonePoints()
{
    return CCombatZoneManager::Instance().GetValueByType(this, COMBAT_ZONE_GET_POINTS);
}

void CHARACTER::SetRealCombatZonePoints(uint32_t iValue)
{
    DBManager::instance().Query("UPDATE %s.player SET combat_zone_points = %d WHERE id = %u", g_PlayerDatabase.c_str(),
                                iValue, GetPlayerID());
}
#endif

int32_t CHARACTER::GetCash(std::string cashType) const
{
    if (GetDesc() != nullptr)
    {
        std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(
            "SELECT {} FROM {}.account WHERE id = {}", cashType, gConfig.accountDb.name, GetDesc()->GetAid()));
        if (!pMsg->Get()->uiNumRows)
            return 0;

        return atoi(mysql_fetch_row(pMsg->Get()->pSQLResult)[0]);
    }

    return 0;
}

void CHARACTER::ChangeCash(int32_t amount, std::string strChargeType, bool add) const
{
    if (!GetDesc())
        return;

    TRequestChargeCash packet;
    packet.dwAID = GetDesc()->GetAccountTable().id;
    packet.iAmount = amount;
    packet.eChargeType = ERequestCharge_Cash;
    packet.bAdd = add;

    if (0 < strChargeType.length())
        std::transform(strChargeType.begin(), strChargeType.end(), strChargeType.begin(), ::tolower);

    if ("mileage" == strChargeType)
        packet.eChargeType = ERequestCharge_Mileage;

    db_clientdesc->DBPacket(HEADER_GD_REQUEST_CHARGE_CASH, GetDesc()->GetHandle(), &packet, sizeof(TRequestChargeCash));
}

void CHARACTER::LoadMessengerBlock(const ElementLoadBlockedPC *pcs, uint16_t count)
{
    for (uint16_t i = 0; i < count; i++)
    {
        m_messengerBlock.emplace(pcs[i].pid, pcs[i].data);
        SPDLOG_INFO("Recieved block {0}", pcs[i].data.name);
    }

    if (GetDesc()->IsPhase(PHASE_GAME))
        SendMessengerBlock();
}

void CHARACTER::SendMessengerBlock()
{
    TPacketGCMessenger pack;
    pack.subheader = MESSENGER_SUBHEADER_GC_BLOCK_LIST;

    std::vector<BlockedPC> vbpc;
    for (const auto &it : m_messengerBlock)
        vbpc.push_back(it.second);
    
    pack.listBlocked = vbpc;
    GetDesc()->Send(HEADER_GC_MESSENGER, pack);

}

void CHARACTER::SetMessengerBlock(uint32_t pid, const BlockedPC &pc)
{
    auto it = m_messengerBlock.find(pid);
    if (it != m_messengerBlock.end())
        it->second = pc;
    else
        m_messengerBlock.emplace(pid, pc);
}

void CHARACTER::SetPVPTeam(short sTeam)
{
    if (m_sPVPTeam == sTeam)
        return;

    m_sPVPTeam = sTeam;

    UpdatePacket();
}

bool CHARACTER::IsPrivateMap(long lMapIndex) const
{
    if (lMapIndex)
        return GetMapIndex() >= lMapIndex * 10000 && GetMapIndex() <= lMapIndex * 10000 + 9999;

    return GetMapIndex() >= 10000;
}

#ifdef ENABLE_BATTLE_PASS
void CHARACTER::LoadBattlePass(uint32_t dwCount, TPlayerBattlePassMission *data)
{
    // m_listBattlePass.clear();
    m_bIsLoadedBattlePass = false;

    for (int i = 0; i < dwCount; ++i, ++data)
    {
        TPlayerBattlePassMission *newMission = new TPlayerBattlePassMission;
        newMission->dwPlayerId = data->dwPlayerId;
        newMission->dwMissionId = data->dwMissionId;
        newMission->dwBattlePassId = data->dwBattlePassId;
        newMission->dwExtraInfo = data->dwExtraInfo;
        newMission->bCompleted = data->bCompleted;
        newMission->bIsUpdated = data->bIsUpdated;

        m_listBattlePass.push_back(newMission);
    }

    m_bIsLoadedBattlePass = true;
}

uint64_t CHARACTER::GetMissionProgress(uint32_t dwMissionID, uint32_t dwBattlePassID)
{
    auto it = m_listBattlePass.begin();
    while (it != m_listBattlePass.end())
    {
        TPlayerBattlePassMission *pkMission = *it++;
        if (pkMission->dwMissionId == dwMissionID && pkMission->dwBattlePassId == dwBattlePassID)
            return pkMission->dwExtraInfo;
    }

    return 0;
}

bool CHARACTER::IsCompletedMission(uint8_t bMissionType)
{
    auto it = m_listBattlePass.begin();
    while (it != m_listBattlePass.end())
    {
        TPlayerBattlePassMission *pkMission = *it++;
        if (pkMission->dwMissionId == bMissionType && pkMission->dwBattlePassId == GetBattlePassId())
            return (pkMission->bCompleted ? true : false);
    }

    return false;
}

void CHARACTER::UpdateMissionProgress(uint32_t dwMissionID, uint32_t dwBattlePassID, uint64_t dwUpdateValue,
                                      uint64_t dwTotalValue, bool isOverride)
{
    if (!m_bIsLoadedBattlePass)
        return;

    bool foundMission = false;
    uint64_t dwSaveProgress = 0;

    auto it = m_listBattlePass.begin();
    while (it != m_listBattlePass.end())
    {
        TPlayerBattlePassMission *pkMission = *it++;

        if (pkMission->dwMissionId == dwMissionID && pkMission->dwBattlePassId == dwBattlePassID)
        {
            pkMission->bIsUpdated = 1;

            if (isOverride)
                pkMission->dwExtraInfo = dwUpdateValue;
            else
                pkMission->dwExtraInfo += dwUpdateValue;

            if (pkMission->dwExtraInfo >= dwTotalValue)
            {
                pkMission->dwExtraInfo = dwTotalValue;
                
                if (pkMission->bCompleted != 1)
                    CBattlePass::instance().BattlePassRewardMission(this, dwMissionID, dwBattlePassID);//@Adalet and Light Work Correct BP Reward Control

                pkMission->bCompleted = 1;
                // CBattlePass::instance().CheckBattlePassCompleted(this);
            }

            dwSaveProgress = pkMission->dwExtraInfo;
            foundMission = true;
            break;
        }
    }

    if (!foundMission)
    {
        TPlayerBattlePassMission *newMission = new TPlayerBattlePassMission;
        newMission->dwPlayerId = GetPlayerID();
        newMission->dwMissionId = dwMissionID;
        newMission->dwBattlePassId = dwBattlePassID;

        if (dwUpdateValue >= dwTotalValue)
        {
            newMission->dwExtraInfo = dwTotalValue;
            newMission->bCompleted = 1;

            CBattlePass::instance().BattlePassRewardMission(this, dwMissionID, dwBattlePassID);

            dwSaveProgress = dwTotalValue;
        }
        else
        {
            newMission->dwExtraInfo = dwUpdateValue;
            newMission->bCompleted = 0;

            dwSaveProgress = dwUpdateValue;
        }

        newMission->bIsUpdated = 1;

        m_listBattlePass.push_back(newMission);
    }

    if (!GetDesc())
        return;

    GcBattlePassUpdatePacket packet;
    packet.bMissionType = dwMissionID;
    packet.dwNewProgress = dwSaveProgress;
    GetDesc()->Send(HEADER_GC_BATTLE_PASS_UPDATE, packet);
}

uint8_t CHARACTER::GetBattlePassId()
{
    const auto* pAffect = FindAffect(AFFECT_BATTLE_PASS, POINT_NONE);

    if (!pAffect)
        return 0;

    return pAffect->pointValue;
}

uint8_t CHARACTER::GetBattlePassState()
{
    const auto* pAffect = FindAffect(AFFECT_BATTLE_PASS, POINT_NONE);

    if (!pAffect)
        return 0;

    return pAffect->color;
}
bool CHARACTER::IsChatFilter(uint8_t bLanguageID) const
{
    if (GetDesc())
    {
        if (GetLanguageIDByName(GetDesc()->GetCharacter()->GetClientLocale()) == bLanguageID ||
            IS_SET(m_points.chatFilter, 1 << LANGUAGE_MAX_NUM))
            return false;

        return IS_SET(m_points.chatFilter, 1 << bLanguageID);
    }

    return false;
}

void CHARACTER::SetChatFilter(uint8_t bLanguageID, bool bValue)
{
    uint16_t currentFilter = 0;

    if (!GetDesc() || bLanguageID > LANGUAGE_MAX_NUM)
        return;

    currentFilter = m_points.chatFilter;

    if (IS_SET(currentFilter, 1 << bLanguageID) && bValue)
        return;

    if (!IS_SET(currentFilter, 1 << bLanguageID) && bValue) { SET_BIT(currentFilter, 1 << bLanguageID); }

    if (IS_SET(currentFilter, 1 << bLanguageID) && !bValue) { REMOVE_BIT(currentFilter, 1 << bLanguageID); }

    m_points.chatFilter = currentFilter;

    Save();
}

void CHARACTER::UpdateChatFilter()
{
    if (GetDesc())
    {
      TPacketGCUpdateChatFilter pack;
        uint16_t currentFilter = m_points.chatFilter;
        for (int i = 0; i <= LANGUAGE_MAX_NUM; i++)
            pack.bFilterList.emplace_back(IS_SET(currentFilter, 1 << i) ? 1 : 0);

        GetDesc()->Send(HEADER_GC_UPDATE_CHAT_FILTER, pack);
    }
}

#endif

void CHARACTER::SendDungeonInfo()
{
	if(!GetDesc())
		return;
	
	TPacketGCDungeonInfo p;
    p.infos = m_dungeonInfo;
	GetDesc()->Send(HEADER_GC_DUNGEON_INFO, p);
	//m_dungeonInfo.clear();
}

void CHARACTER::SetDungeonCooldown(uint8_t bDungeonID, uint16_t wCooldown)
{
	// It's ok
	bool bFound = false;
	uint32_t dwCooldownEnd = get_global_time() + (wCooldown * 60);
	for (int i = 0; i < m_dungeonInfo.size(); i++)
	{
		if(m_dungeonInfo[i].bDungeonID == bDungeonID)
		{
			m_dungeonInfo[i].dwCooldownEnd = dwCooldownEnd;
			
			db_clientdesc->DBPacketHeader(HEADER_GD_DUNGEON_INFO_SAVE, 0, sizeof(TPlayerDungeonInfo));
			db_clientdesc->Packet(&m_dungeonInfo[i], sizeof(TPlayerDungeonInfo));
			
			bFound = true;
			break;
		}
	}
	
	if(!bFound)
	{
		TPlayerDungeonInfo info;
		info.dwPlayerID = GetPlayerID();
		info.bDungeonID = bDungeonID;
		info.dwCooldownEnd = dwCooldownEnd;
		info.wCompleteCount = 0;
		info.wFastestTime = 0;
		info.dwHightestDamage = 0;
		
		m_dungeonInfo.push_back(info);
		
		db_clientdesc->DBPacketHeader(HEADER_GD_DUNGEON_INFO_SAVE, 0, sizeof(TPlayerDungeonInfo));
		db_clientdesc->Packet(&info, sizeof(TPlayerDungeonInfo));
	}
	
	TPacketGCDungeonUpdate packet;
	packet.bDungeonID = bDungeonID;
	packet.dwCooldownEnd = dwCooldownEnd;
	GetDesc()->Send(HEADER_GC_DUNGEON_UPDATE, packet);
}

void CHARACTER::ResetDungeonCooldown(uint8_t bDungeonID)
{
	for (int i = 0; i < m_dungeonInfo.size(); i++)
	{
		if(m_dungeonInfo[i].bDungeonID == bDungeonID)
		{
			m_dungeonInfo[i].dwCooldownEnd = 0;
			
			db_clientdesc->DBPacketHeader(HEADER_GD_DUNGEON_INFO_SAVE, 0, sizeof(TPlayerDungeonInfo));
			db_clientdesc->Packet(&m_dungeonInfo[i], sizeof(TPlayerDungeonInfo));

			TPacketGCDungeonUpdate packet;
			packet.bDungeonID = bDungeonID;
			packet.dwCooldownEnd = 0;
			GetDesc()->Send(HEADER_GC_DUNGEON_UPDATE, packet);

			break;
		}
	}
}

void CHARACTER::SetDungeonRanking(uint8_t bDungeonID, uint8_t bCompleteCount, uint32_t wFastestTime, DamageValue dwHightestDamage)
{
	// It's ok
	bool bFound = false;
	for (int i = 0; i < m_dungeonInfo.size(); i++)
	{
		if(m_dungeonInfo[i].bDungeonID == bDungeonID)
		{
			m_dungeonInfo[i].wCompleteCount = bCompleteCount;
			
			if(wFastestTime > m_dungeonInfo[i].wFastestTime)
			{
				m_dungeonInfo[i].wFastestTime = wFastestTime;
			}

			if(dwHightestDamage > m_dungeonInfo[i].dwHightestDamage)
			{
				m_dungeonInfo[i].dwHightestDamage = dwHightestDamage;
			}

			db_clientdesc->DBPacketHeader(HEADER_GD_DUNGEON_INFO_SAVE, 0, sizeof(TPlayerDungeonInfo));
			db_clientdesc->Packet(&m_dungeonInfo[i], sizeof(TPlayerDungeonInfo));

			bFound = true;
			break;
		}
	}
	
	if(!bFound)
	{
		TPlayerDungeonInfo info;
		info.dwPlayerID = GetPlayerID();
		info.bDungeonID = bDungeonID;
		info.dwCooldownEnd = 0;
		info.wCompleteCount = bCompleteCount;
		info.wFastestTime = wFastestTime;
		info.dwHightestDamage = dwHightestDamage;
		
		m_dungeonInfo.push_back(info);
		
		db_clientdesc->DBPacketHeader(HEADER_GD_DUNGEON_INFO_SAVE, 0, sizeof(TPlayerDungeonInfo));
		db_clientdesc->Packet(&info, sizeof(TPlayerDungeonInfo));
	}
}

uint32_t CHARACTER::GetDungeonCooldownEnd(uint8_t bDungeonID)
{
	for (const auto info : m_dungeonInfo)
	{
		if(info.bDungeonID == bDungeonID)
		{
			return info.dwCooldownEnd;
		}
	}
	
	return 0;
}

