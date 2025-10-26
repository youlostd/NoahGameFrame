#include "char.h"

#include <game/AffectConstants.hpp>
#include <game/AffectsHolder.hpp>
#include <game/GamePacket.hpp>

#include "DbCacheSocket.hpp"
#include "guild.h"
#include "horsename_manager.h"

#include "ItemUtils.h"
#include "PacketUtils.hpp"
#include "war_map.h"

void SendAffectRemovePacket(CHARACTER* ch, const AffectData& affect)
{
    GcAffectDelPacket p{};
    p.vid = ch->GetVID();
    p.type = affect.type;
    p.pointType = affect.pointType;
    PacketView(ch->GetViewMap(), static_cast<CEntity*>(ch),
               HEADER_GC_AFFECT_REMOVE, p);

    if (ch->IsPC() && !IsSkipSaveAffect(affect.type)) {
        TPacketGDRemoveAffect p2;
        p2.dwPID = ch->GetPlayerID();
        p2.dwType = affect.type;
        p2.bApplyOn = affect.pointType;
        db_clientdesc->DBPacket(HEADER_GD_REMOVE_AFFECT, 0, &p2, sizeof(p2));
    }
}

void SendAffectAddPacket(CHARACTER* ch, const AffectData& affect)
{
    GcAffectAddPacket p;
    p.vid = ch->GetVID();
    p.data = affect;

    if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()) ||
        ch->GetMapIndex() == 112 ||
        gConfig.IsBlockedSkillColorMap(ch->GetMapIndex()))
        p.data.color = 0;

    PacketView(ch->GetViewMap(), static_cast<CEntity*>(ch),
               HEADER_GC_AFFECT_ADD, p);
}

////////////////////////////////////////////////////////////////////
// Affect
AffectData* CHARACTER::FindAffect(uint32_t type, uint8_t apply)
{
    return SearchAffect(m_affects, type, apply);
}

const AffectData* CHARACTER::FindAffect(uint32_t type, uint8_t apply) const
{
    return SearchAffect(m_affects, type, apply);
}

EVENTFUNC(affect_event)
{
    char_event_info* info = static_cast<char_event_info*>(event->info);

    if (info == nullptr) {
        SPDLOG_ERROR("affect_event> <Factor> Null pointer");
        return 0;
    }

    CHARACTER* ch = info->ch;

    if (ch == nullptr) {
        // <Factor>
        return 0;
    }

    if (!ch->UpdateAffect())
        return 0;

    return THECORE_SECS_TO_PASSES(1); // 1ÃÊ
}

bool CHARACTER::UpdateAffect()
{
    const auto* hasNoRecoveryAffect = FindAffect(AFFECT_NO_RECOVERY);

    if (!hasNoRecoveryAffect) {
        if (GetPoint(POINT_HP_RECOVERY) > 0) {
            if (GetMaxHP() <= GetHP()) {
                PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
            } else {
                if (IsPC()) {
                    int percentage = 9;

                    if (GetLevel() > 120) {
                        percentage = 9;
                    }

                    const PointValue iVal =
                        std::min<PointValue>(GetPoint(POINT_HP_RECOVERY),
                                             GetMaxHP() * percentage / 100);

                    PointChange(POINT_HP, iVal);
                    PointChange(POINT_HP_RECOVERY, -iVal);
                } else {
                    const PointValue iVal = std::min<PointValue>(
                        GetPoint(POINT_HP_RECOVERY), GetMaxHP() * 9 / 100);

                    PointChange(POINT_HP, iVal);
                    PointChange(POINT_HP_RECOVERY, -iVal);
                }
            }
        }
    }

    if (GetPoint(POINT_SP_RECOVERY) > 0) {
        int iVal = 0;

        if (GetMaxSP() <= GetSP()) {
            PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));
        } else {
            PointChange(POINT_SP,
                        std::min<PointValue>(GetPoint(POINT_SP_RECOVERY),
                                             GetMaxSP() * 7 / 100));
            PointChange(POINT_SP_RECOVERY, -iVal);
        }
    }

    if (!hasNoRecoveryAffect) {
        if (GetPoint(POINT_HP_RECOVER_CONTINUE) > 0) {
            PointChange(POINT_HP, GetPoint(POINT_HP_RECOVER_CONTINUE));
        }
    }

    if (GetPoint(POINT_SP_RECOVER_CONTINUE) > 0) {
        PointChange(POINT_SP, GetPoint(POINT_SP_RECOVER_CONTINUE));
    }

    auto* hpItem = FindToggleItem(this, true, TOGGLE_AUTO_RECOVERY_HP);
    if (hpItem)
        ProcessAutoRecoveryItem(this, hpItem);

    auto* spItem = FindToggleItem(this, true, TOGGLE_AUTO_RECOVERY_SP);
    if (spItem)
        ProcessAutoRecoveryItem(this, spItem);

    if (GetMaxStamina() > GetStamina()) {
        int iSec = (get_dword_time() - GetStopTime()) / 3000;
        if (iSec)
            PointChange(POINT_STAMINA, GetMaxStamina() / 10);
    }

#ifdef __FAKE_PC__
    if (ProcessAffect() && !FakePC_Check())
#else
    if (ProcessAffect())
#endif
    {
        if (GetPoint(POINT_HP_RECOVERY) == 0 &&
            GetPoint(POINT_SP_RECOVERY) == 0 &&
            GetStamina() == GetMaxStamina() && !hpItem && !spItem) {
            m_pkAffectEvent = nullptr;
            return false;
        }
    }

    return true;
}

void CHARACTER::StartAffectEvent()
{
    if (m_pkAffectEvent)
        return;

    char_event_info* info = AllocEventInfo<char_event_info>();
    info->ch = this;
    m_pkAffectEvent =
        event_create(affect_event, info, THECORE_SECS_TO_PASSES(1));
    SPDLOG_DEBUG("StartAffectEvent {} {} {}", GetName(), (void*)this,
                 (void*)(m_pkAffectEvent).get());
}

void CHARACTER::ClearAffect()
{
    const auto wMovSpd = GetPoint(POINT_MOV_SPEED);
    const auto wAttSpd = GetPoint(POINT_ATT_SPEED);

    auto it = m_affects.begin();
    while (it != m_affects.end()) {
        const auto& affect = *it;

        if (IsRetainOnDeathAffect(affect.type) ||
            IsSkipSaveAffect(affect.type)) {
            ++it;
            continue;
        }

        SendAffectRemovePacket(this, affect);

        ComputeAffect(affect, false);
#ifdef __FAKE_PC__
        if (FakePC_Check()) {
            auto it = m_map_pkFakePCAffects.find(affect);
            if (it != m_map_pkFakePCAffects.end()) {
                m_map_pkFakePCAffects.erase(it->second);
                m_map_pkFakePCAffects.erase(it);
            }
        }
#endif
        it = m_affects.erase(it);
    }

    if (wMovSpd != GetLimitPoint(POINT_MOV_SPEED) ||
        wAttSpd != GetLimitPoint(POINT_ATT_SPEED))
        UpdatePacket();

    CheckMaximumPoints();
#ifdef __FAKE_PC__
    FakePC_Owner_ExecFunc(&CHARACTER::CheckMaximumPoints);
#endif

    if (m_affects.empty())
        event_cancel(&m_pkAffectEvent);
}

int CHARACTER::ProcessAffect()
{
    //@Adalet Silah takmadan hava kılıcı ve büyülü silah açılma düzeltmesi.
    if (!GetWear(WEAR_WEAPON)) {
        if (FindAffect(SKILL_GEOMKYUNG)) {
            ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Hava kılıcı skilini açmak veya "
                                               "açık kalması için ilk önce "
                                               "silahınızın takılı olması "
                                               "gerekiyor. "));
            RemoveAffect(SKILL_GEOMKYUNG);
        }

        if (FindAffect(SKILL_GWIGEOM)) {
            ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Büyülü silah skilini açmak "
                                               "veya açık kalması için ilk "
                                               "önce silahınızın takılı olması "
                                               "gerekiyor."));
            RemoveAffect(SKILL_GWIGEOM);
        }
    }


    bool bDiff = false;

    for (int i = 0; i < PREMIUM_MAX_NUM; ++i) {
        int aff_idx = i + AFFECT_PREMIUM_START;

        auto* affPtr = FindAffect(aff_idx);
        if (!affPtr)
            continue;

        int remain = GetPremiumRemainSeconds(i);
        if (remain < 0) {
            if (aff_idx == AFFECT_VOTE_BONUS)
			{
				RemoveAffect(AFFECT_VOTE_BONUS2);
				RemoveAffect(AFFECT_VOTE_BONUS3);
				RemoveAffect(AFFECT_VOTE_BONUS4);
			}

            RemoveAffect(aff_idx);
            // TODO: Affect callbacks?
            if (aff_idx == AFFECT_BLOCK_CHAT) {
                SendI18nChatPacket(this, CHAT_TYPE_INFO,
                                   "Chat ban has been lifted.");
            }

            bDiff = true;
        } else {
            affPtr->duration = remain + 1;
        }
    }

#ifdef ENABLE_BATTLE_PASS
    auto affPtr = FindAffect(AFFECT_BATTLE_PASS);
    if (affPtr) {
        int remain = GetBattlePassEndTime();

        if (remain < 0) {
            RemoveAffect(AFFECT_BATTLE_PASS);
            m_dwBattlePassEndTime = 0;
            bDiff = true;
        } else
            affPtr->duration = remain + 1;
    }
#endif

    CHorseNameManager::instance().Validate(this);

    int32_t lMovSpd = GetLimitPoint(POINT_MOV_SPEED);
    int32_t lAttSpd = GetLimitPoint(POINT_ATT_SPEED);

    for (auto it = m_affects.begin(); it != m_affects.end();) {
        auto& affect = *it;

        bool bEnd = false;

        if (affect.type >= GUILD_SKILL_START &&
            affect.type <= GUILD_SKILL_END) {
            if (!GetGuild() || !GetGuild()->UnderAnyWar())
                bEnd = true;
        }

        if (affect.spCost > 0) {
            if (GetSP() < affect.spCost)
                bEnd = true;
            else
                PointChange(POINT_SP, -affect.spCost);
        }

        if (affect.subType == AFFECT_TYPE_NORMAL) {
            if (affect.duration != INFINITE_AFFECT_DURATION) {
                bEnd = --affect.duration <= 0;
            }
        } else if (affect.subType == AFFECT_TYPE_REAL_TIME) {

            bEnd = affect.duration - get_global_time() <= 0;
        }

        if (bEnd) {
            SPDLOG_TRACE("{0}: Remove expired affect {1}:{2}", GetName(),
                         affect.type, affect.pointType);

            ComputeAffect(affect, false);
            bDiff = true;

            SendAffectRemovePacket(this, affect);
            it = m_affects.erase(it);
            continue;
        }

        ++it;
    }

    if (bDiff) {
        if (lMovSpd != GetLimitPoint(POINT_MOV_SPEED) ||
            lAttSpd != GetLimitPoint(POINT_ATT_SPEED))
            UpdatePacket();

        CheckMaximumPoints();
    }

    if (m_affects.empty())
        return true;

    return false;
}

void CHARACTER::SaveAffect()
{
    TPacketGDAddAffect p;
    for (const auto& affect : m_affects) {
        if (IsSkipSaveAffect(affect.type))
            continue;

        SPDLOG_TRACE("AFFECT_SAVE: {0} {1} {2} {3}", affect.type,
                     affect.pointType, affect.pointValue, affect.duration);

        p.dwPID = GetPlayerID();
        p.elem = affect;
        db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
    }
}

void CHARACTER::LoadAffect(uint32_t count, const AffectData* affects)
{
    ClearAffect();

    SPDLOG_DEBUG("LOAD_AFFECT: {0} count {1}", GetName(), count);

    int32_t lMovSpd = GetLimitPoint(POINT_MOV_SPEED);
    int32_t lAttSpd = GetLimitPoint(POINT_ATT_SPEED);

    for (uint32_t i = 0; i < count; ++i, ++affects) {
        if (affects->type == SKILL_MUYEONG)
            continue;

        // We might add affects to this list and don't delete existing entries
        // in the db.
        if (IsSkipSaveAffect(affects->type))
            continue;

        if (affects->pointType >= POINT_MAX_NUM) {
            SPDLOG_ERROR("invalid affect data {} ApplyOn {} ApplyValue {}",
                         GetName(), affects->pointType, affects->pointValue);
            continue;
        }

        SPDLOG_DEBUG("Load Affect : Affect {0} {1} {2}", GetName(),
                     affects->type, affects->pointType);
        InsertAffect(m_affects, *affects);

        SendAffectAddPacket(this, *affects);
        ComputeAffect(*affects, true);
    }

    /*if (GetArena())
        RemoveGoodAffect();
        */

    if (lMovSpd != GetLimitPoint(POINT_MOV_SPEED) ||
        lAttSpd != GetLimitPoint(POINT_ATT_SPEED))
        UpdatePacket();
    //Adalet correct begin
    if (!IsDead()) {
        PointChange(POINT_HP, GetMaxHP() - GetHP());
        PointChange(POINT_SP, GetMaxSP() - GetSP());
    }
    //Adalet correct end
    DragonSoul_Initialize();
}

bool CHARACTER::AddAffect(uint32_t type, uint8_t pointType,
                          PointValue pointValue, int32_t duration,
                          int32_t spCost, bool isOverride, bool isCube,
                          uint32_t color)
{
    if (duration == 0) {
        SPDLOG_ERROR("Character::AddAffect duration == 0 type {0}", duration,
                     type);
        duration = 1;
    }

    AffectData* affectPtr;

    if (isCube)
        affectPtr = FindAffect(type, pointType);
    else
        affectPtr = FindAffect(type);

    if (type == AFFECT_STUN) {
        if (IsPosition(POS_FIGHTING))
            SetPosition(POS_STANDING);

        SyncPacket();
    }

    if (affectPtr && isOverride) {
        if (affectPtr->pointValue > pointValue)
            return false;

        ComputeAffect(*affectPtr, false);
        SendAffectRemovePacket(this, *affectPtr);
        EraseAffect(m_affects, *affectPtr);
    }

    SPDLOG_TRACE("AddAffect {0} type {1} apply {2} {3} duration {4}", GetName(),
                 type, pointType, pointValue, duration);

    AffectData data = {};
    data.type = type;
    data.subType = IsRealTimeAffect(type) ? AFFECT_TYPE_REAL_TIME : AFFECT_TYPE_NORMAL;
    data.pointType = pointType;
    data.pointValue = pointValue;
    data.duration = !IsRealTimeAffect(type)  ? duration : get_global_time() + duration;
    data.spCost = spCost;
    data.color = color;
    InsertAffect(m_affects, data);

    uint16_t wMovSpd = GetLimitPoint(POINT_MOV_SPEED);
    uint16_t wAttSpd = GetLimitPoint(POINT_ATT_SPEED);

    ComputeAffect(data, true);

    if (wMovSpd != GetLimitPoint(POINT_MOV_SPEED) ||
        wAttSpd != GetLimitPoint(POINT_ATT_SPEED))
        UpdatePacket();

    StartAffectEvent();

    SendAffectAddPacket(this, data);

    if (IsPC() && !IsSkipSaveAffect(data.type)) {
        TPacketGDAddAffect p;
        p.dwPID = GetPlayerID();
        p.elem = data;
        db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
    }

    return true;
}

void CHARACTER::RefreshAffect()
{
    for (const auto& affect : m_affects)
        ComputeAffect(affect, true);
}

void CHARACTER::ComputeAffect(const AffectData& affect, bool add)
{
    if (add && affect.type >= GUILD_SKILL_START &&
        affect.type <= GUILD_SKILL_END) {
        if (!GetGuild())
            return;

        if (!GetGuild()->UnderAnyWar())
            return;
    }

    if (add)
        PointChange(affect.pointType, affect.pointValue);
    else
        PointChange(affect.pointType, -affect.pointValue);

    if (affect.type == SKILL_MUYEONG) {
        if (add)
            StartMuyeongEvent();
        else
            StopMuyeongEvent();
    }
}

const std::vector<WORD> PvpSkills = {
    AFFECT_MOUNT, AFFECT_MOUNT_BONUS, AFFECT_REVIVE_INVISIBLE, AFFECT_MOV_SPEED,
    AFFECT_ATT_SPEED, AFFECT_STR, AFFECT_DEX, AFFECT_INT, AFFECT_CON,
    AFFECT_CHINA_FIREWORK,
    // Body Warrior
    SKILL_JEONGWI,   // 3 (Berserk)
    SKILL_GEOMKYUNG, // 4 (Aura of the Sword)
    SKILL_TANHWAN,
    // Mental Warrior
    SKILL_CHUNKEON, // 19 (Strong Body)
    // Blade-Fight Ninja
    SKILL_EUNHYUNG, // 34 (Stealth)
    SKILL_CHARYUN,  // 33 cevirme
    // Archery Ninja
    SKILL_GYEONGGONG, // 49 (Feather Walk)
    // Weaponry Sura
    SKILL_GWIGEOM, // 63 (Enchanted Blade)
    SKILL_TERROR,  // 64 (Fear)
    SKILL_JUMAGAP, // 65 (Enchanted Armour)
    // Black Magic Sura
    SKILL_MANASHIELD, // 79 (Dark Protection)
    // Dragon Force Shaman
    SKILL_HOSIN,   // 94 (Blessing)
    SKILL_REFLECT, // 95 (Reflection)
    SKILL_GICHEON, // 96 (Dragon's Strength)
    // Healing Force Shaman
    SKILL_KWAESOK,    // 110 (Swiftness)
    SKILL_JEUNGRYEOK, // 111 (Attack Up)
};

bool CHARACTER::RemoveAffect(const AffectData& affect)
{
    SPDLOG_TRACE("{0}: Remove affect {1}:{2}", GetName(), affect.type,
                 affect.pointType);

    uint16_t wMovSpd = GetLimitPoint(POINT_MOV_SPEED);
    uint16_t wAttSpd = GetLimitPoint(POINT_ATT_SPEED);

    AffectData affect2 = affect;
    EraseAffect(m_affects, affect2); // affect2 olarak düzeltildi

    ComputeAffect(affect2, false);

    bool IsPvPSkill = false;
    for (const auto& it : PvpSkills) { // PvpSkills üzerinde döngüyü const
                                       // referans olarak tanımladık
        const AffectData* pkAff = FindAffect(
            static_cast<WORD>(it)); // PvpSkills türü WORD olduğundan
                                    // static_cast ile dönüşüm gerçekleştirildi
        if (pkAff) {
            IsPvPSkill = true;
            break;
        }
    }

    if (IsPvPSkill) {
        ComputePoints();
    } else {
        UpdatePacket(); // Null işaretçi kontrolü yerine doğrudan else bloğunda
                        // UpdatePacket çağrıldı.
    }

    if (wMovSpd != GetLimitPoint(POINT_MOV_SPEED) ||
        wAttSpd != GetLimitPoint(POINT_ATT_SPEED)) {
        UpdatePacket();
    }

    SendAffectRemovePacket(this, affect2);
    return true;
}

bool CHARACTER::RemoveAffect(uint32_t type)
{
    uint32_t count = 0;
    for (auto p = FindAffect(type); p; p = FindAffect(type)) {
        RemoveAffect(*p);
        ++count;
    }

    return count != 0;
}

void CHARACTER::RemoveGoodAffect()
{
    RemoveAffect(AFFECT_MOV_SPEED);
    RemoveAffect(AFFECT_ATT_SPEED);
    RemoveAffect(AFFECT_ATT_GRADE);
    RemoveAffect(AFFECT_STR);
    RemoveAffect(AFFECT_DEX); // 205
    RemoveAffect(AFFECT_CON);
    RemoveAffect(AFFECT_INT);
    RemoveAffect(AFFECT_CAST_SPEED);
    RemoveAffect(AFFECT_HP_RECOVER_CONTINUE);
    RemoveAffect(AFFECT_SP_RECOVER_CONTINUE);
    RemoveAffect(AFFECT_RAMADAN_ABILITY);
    RemoveAffect(AFFECT_RAMADAN_RING);
    RemoveAffect(AFFECT_NOG_ABILITY);
    RemoveAffect(AFFECT_HOLLY_STONE_POWER);
    RemoveAffect(AFFECT_BLEND);
    RemoveAffect(AFFECT_CHINA_FIREWORK);

    RemoveAffect(SKILL_JEONGWI);
    RemoveAffect(SKILL_GEOMKYUNG);
    RemoveAffect(SKILL_CHUNKEON);
    RemoveAffect(SKILL_EUNHYUNG);
    RemoveAffect(SKILL_GYEONGGONG);
    RemoveAffect(SKILL_GWIGEOM);
    RemoveAffect(SKILL_TERROR);
    RemoveAffect(SKILL_JUMAGAP);
    RemoveAffect(SKILL_MANASHIELD);
    RemoveAffect(SKILL_HOSIN);
    RemoveAffect(SKILL_REFLECT);
    RemoveAffect(SKILL_KWAESOK);
    RemoveAffect(SKILL_JEUNGRYEOK);
    RemoveAffect(SKILL_GICHEON);
    RemoveAffect(SKILL_JEOKRANG);
    RemoveAffect(SKILL_CHEONGRANG);
}

bool CHARACTER::IsGoodAffect(uint8_t bAffectType) const
{
    switch (bAffectType) {
        case AFFECT_MOV_SPEED:
        case AFFECT_ATT_SPEED:
        case AFFECT_ATT_GRADE:
        case AFFECT_STR:
        case AFFECT_DEX: // 205
        case AFFECT_CON:
        case AFFECT_INT:
        case AFFECT_CAST_SPEED:
        case AFFECT_HP_RECOVER_CONTINUE:
        case AFFECT_SP_RECOVER_CONTINUE:
        case AFFECT_RAMADAN_ABILITY:
        case AFFECT_RAMADAN_RING:
        case AFFECT_NOG_ABILITY:
        case AFFECT_HOLLY_STONE_POWER:
        case AFFECT_BLEND:

        case SKILL_JEONGWI:
        case SKILL_GEOMKYUNG:
        case SKILL_CHUNKEON:
        case SKILL_EUNHYUNG:
        case SKILL_GYEONGGONG:
        case SKILL_GWIGEOM:
        case SKILL_TERROR:
        case SKILL_JUMAGAP:
        case SKILL_MANASHIELD:
        case SKILL_HOSIN:
        case SKILL_REFLECT:
        case SKILL_KWAESOK:
        case SKILL_JEUNGRYEOK:
        case SKILL_GICHEON:
        case SKILL_JEOKRANG:
        case SKILL_CHEONGRANG:
            return true;
    }

    return false;
}

void CHARACTER::RemoveBadAffect()
{
    RemovePoison();
    RemoveFire();

    RemoveAffect(AFFECT_ATT_SPEED_SLOW);
    RemoveAffect(AFFECT_ATT_GRADE_DOWN);
    RemoveAffect(AFFECT_DEF_GRADE_DOWN);
    RemoveAffect(AFFFECT_CRITICAL_PCT_DOWN);
    RemoveAffect(AFFECT_MOUNT_FALL);
    RemoveAffect(AFFECT_NO_RECOVERY);
    RemoveAffect(AFFECT_REDUCE_CAST_SPEED);

    RemoveAffect(AFFECT_STUN);
    RemoveAffect(AFFECT_SLOW);
    RemoveAffect(SKILL_TUSOK);

}

void CHARACTER::RemoveBadAffectAfterDuel()
{
    RemovePoison();
    RemoveFire();
    RemoveAffect(AFFECT_SLOW);
    RemoveAffect(SKILL_PABEOB);
}