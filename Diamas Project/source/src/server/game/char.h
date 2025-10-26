#ifndef METIN2_SERVER_GAME_CHAR_H
#define METIN2_SERVER_GAME_CHAR_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "CharUtil.hpp"
#include "FSM.h"
#include "PetSystem.h"
#include "buff_on_attributes.h"
#include "entity.h"
#include "horse_rider.h"
#include "sectree_manager.h"
#include "utils.h"
#include "vid.h"

#include "GBufferManager.h"
#include "config.h"

#include <game/AffectsHolder.hpp>
#include <game/DbPackets.hpp>
#include <game/GamePacket.hpp>
#include <game/MobConstants.hpp>
#include <game/MobTypes.hpp>
#include <thecore/main.hpp>
#include <thecore/utils.hpp>

#include "event.h"

#include "ItemGrid.hpp"
#include "MeleyLair.h"

class COfflineShop;
class CArena;
class CBuffOnAttributes;
class CPetSystem;
class CPVP;
struct Motion;

using CHARACTER_SET = std::unordered_set<CHARACTER*>;

enum EInstantFlags {
    INSTANT_FLAG_DEATH_PENALTY = (1 << 0),
    INSTANT_FLAG_SHOP = (1 << 1),
    INSTANT_FLAG_EXCHANGE = (1 << 2),
    INSTANT_FLAG_STUN = (1 << 3),
    INSTANT_FLAG_NO_REWARD = (1 << 4),
    INSTANT_FLAG_REFUSE_EXP = (1 << 5),
    INSTANT_FLAG_REFUSE_DMG_BODY_WARRIOR = (1 << 6),
    INSTANT_FLAG_REFUSE_DMG_MENTAL_WARRIOR = (1 << 7),
    INSTANT_FLAG_REFUSE_DMG_BOW_NINJA = (1 << 8),
    INSTANT_FLAG_REFUSE_DMG_DAGGER_NINJA = (1 << 9),
    INSTANT_FLAG_REFUSE_DMG_BM_SURA = (1 << 10),
    INSTANT_FLAG_REFUSE_DMG_WEAPON_SURA = (1 << 11),
    INSTANT_FLAG_REFUSE_DMG_DRAGON_SHAMAN = (1 << 12),
    INSTANT_FLAG_REFUSE_DMG_HEALING_SHAMAN = (1 << 13),
};

#define AI_FLAG_NPC (1 << 0)
#define AI_FLAG_AGGRESSIVE (1 << 1)
#define AI_FLAG_HELPER (1 << 2)
#define AI_FLAG_STAYZONE (1 << 3)

#define SET_OVER_TIME(ch, time) (ch)->SetOverTime(time)

extern int g_nPortalLimitTime;

enum {
    POISON_LENGTH = 30,
    BLEEDING_LENGTH = 30,
    STAMINA_PER_STEP = 1,
    AI_CHANGE_ATTACK_POISITION_TIME_NEAR = 10000,
    AI_CHANGE_ATTACK_POISITION_TIME_FAR = 1000,
    AI_CHANGE_ATTACK_POISITION_DISTANCE = 100,
    SUMMON_MONSTER_COUNT = 5,
    SUMMON_HEALER_COUNT = 4,
};

// <Factor> Dynamically evaluated CHARACTER* equivalent.
// Referring to SCharDeadEventInfo.
struct DynamicCharacterPtr {
    DynamicCharacterPtr()
        : is_pc(false)
        , id(0)
    {
    }

    DynamicCharacterPtr(const DynamicCharacterPtr& o)
        : is_pc(o.is_pc)
        , id(o.id)
    {
    }

    // Returns the CHARACTER* found in CHARACTER_MANAGER.
    CHARACTER* Get() const;

    // Clears the current settings.
    void Reset()
    {
        is_pc = false;
        id = 0;
    }

    // Basic assignment operator.
    DynamicCharacterPtr& operator=(const DynamicCharacterPtr& rhs)
    {
        is_pc = rhs.is_pc;
        id = rhs.id;
        return *this;
    }

    // Supports assignment with CHARACTER* type.
    DynamicCharacterPtr& operator=(CHARACTER* character);

    // Supports type casting to CHARACTER*.
    operator CHARACTER*() const { return Get(); }

    bool is_pc;
    uint32_t id;
};

uint32_t CalculateMotionMovementTime(CHARACTER* ch, const Motion* motion,
                                     float distance);

bool __CHARACTER_GotoNearTarget(CHARACTER* self, CHARACTER* victim);

/* ÀúÀåÇÏ´Â µ¥ÀÌÅÍ */
typedef struct character_point {
    uint8_t job;
    uint8_t voice;

    uint8_t level;
    uint8_t levelStep;
    uint32_t exp;
    Gold gold;

    int64_t hp;
    int64_t sp;

    int iRandomHP;
    int iRandomSP;

    int stamina;

    uint8_t skill_group;
    uint32_t blockMode;
    uint32_t chatFilter;

    uint16_t st, ht, dx, iq;

    uint32_t playtime;

    int64_t maxHp, maxSp;

#ifdef ENABLE_GEM_SYSTEM
    int gem;
#endif
} CHARACTER_POINT;

/* ÀúÀåµÇÁö ¾Ê´Â Ä³¸¯ÅÍ µ¥ÀÌÅÍ */
typedef struct character_point_instant {
    PointValue points[POINT_MAX_NUM];

    float fRot;

    int64_t iMaxHP;
    int iMaxSP;

    long position;

    long instant_flag;
    uint32_t dwAIFlag;
    uint32_t dwImmuneFlag;
    uint32_t dwLastShoutPulse;
    uint32_t lastInventorySortPulse;

    Part parts[PART_MAX_NUM];

    CItem* wear[WEAR_MAX_NUM];
    CItem* switchbot[SWITCHBOT_SLOT_COUNT];

    // ¿ëÈ¥¼® ÀÎº¥Åä¸®.
    CItem* pDSItems[DRAGON_SOUL_INVENTORY_MAX_NUM];
    uint16_t wDSItemGrid[DRAGON_SOUL_INVENTORY_MAX_NUM];

    CItem* pOpenLevelPet;

    bool cubeOpen;

    uint16_t pAcceSlots[3];
    uint16_t changeLookSlots[3];

    CHARACTER* pAcceNpc;

    CHARACTER* battle_victim;

    uint8_t gm_level;

    uint8_t bBasePart; // Æò»óº¹ ¹øÈ£

    int iMaxStamina;

    uint8_t bBlockMode;

    int iDragonSoulActiveDeck;
    bool m_pDragonSoulRefineWindowOpener = false;
    bool computed;
} CHARACTER_POINT_INSTANT;

#define TRIGGERPARAM CHARACTER *ch, CHARACTER *causer

typedef struct trigger {
    uint8_t type;
    int (*func)(TRIGGERPARAM);
    long value;
} TRIGGER;

class CTrigger
{
    public:
    CTrigger()
        : bType(0)
        , pFunc(nullptr)
    {
    }

    uint8_t bType;
    int (*pFunc)(TRIGGERPARAM);
};

EVENTINFO(char_event_info)
{
    DynamicCharacterPtr ch;
};

class CharacterHitLog
{
    public:
    void Clear();

    // @return The number of times the character was hit already
    // (before this call occurred.)
    uint32_t Hit(uint32_t vid);

    private:
    // vid, hit count
    std::unordered_map<uint32_t, uint32_t> m_count;
};

struct TSkillUseInfo {
    int iHitCount;
    int iMaxHitCount;
    int iSplashCount;
    int64_t dwNextSkillUsableTime;
    int iRange;
    bool bUsed;
    uint32_t dwVID;
    bool isGrandMaster;

    CharacterHitLog hitLog;

    TSkillUseInfo()
        : iHitCount(0)
        , iMaxHitCount(0)
        , iSplashCount(0)
        , dwNextSkillUsableTime(0)
        , iRange(0)
        , bUsed(false)
        , dwVID(0)
        , isGrandMaster(false)
    {
    }

    bool HitOnce(uint32_t dwVnum = 0);
    bool IsCooltimeOver() const;

    bool UseSkill(bool isGrandMaster, uint32_t vid, uint32_t dwCooltime,
                  int splashcount = 1, int hitcount = -1, int range = -1);

    uint32_t GetMainTargetVID() const { return dwVID; }

    void SetMainTargetVID(uint32_t vid) { dwVID = vid; }

    void ResetHitCount()
    {
        if (iSplashCount) {
            iHitCount = iMaxHitCount;
            iSplashCount--;
        }
    }
};

class CExchange;
class CSkillProto;
class CParty;
class CDungeon;
class CWarMap;
class CAffect;
class CGuild;
class CSafebox;
class Shop;
class PcShop;

class CMobInstance;
typedef struct SMobSkillInfo TMobSkillInfo;

namespace marriage
{
class WeddingMap;
}

enum DamageFlag {
    DAMAGE_NORMAL = (1 << 0),
    DAMAGE_POISON = (1 << 1),
    DAMAGE_DODGE = (1 << 2),
    DAMAGE_BLOCK = (1 << 3),
    DAMAGE_PENETRATE = (1 << 4),
    DAMAGE_CRITICAL = (1 << 5),
    DAMAGE_BLEEDING = (1 << 6),
};

enum DamageEvents {
    CRITICAL = (1 << 0),
    PENETRATE = (1 << 1),
    BLOCK = (1 << 2),
    DODGE = (1 << 3),
    BERSEK = (1 << 4),
    FEAR = (1 << 5),
    BLESSING = (1 << 6),
    REFLECT = (1 << 7),
    STEAL_HP = (1 << 8),
    STEAL_SP = (1 << 9),
    HP_RECOVER = (1 << 10),
    SP_BURN = (1 << 11),
    APOTHIC_ABSORB = (1 << 12),
};

class CHARACTER : public CEntity, public CFSM, public CHorseRider
{
    friend EVENTFUNC(warp_event);
    friend struct FuncSplashDamage;
    friend struct FuncSplashAffect;
    friend struct CFuncShoot;

    struct Windows {
        Windows();

        ItemGrid* Get(uint8_t type);
        const ItemGrid* Get(uint8_t type) const;

        ItemGrid inventory;
        ItemGrid belt;
    };

    protected:
    //////////////////////////////////////////////////////////////////////////////////
    // Entity °ü·Ã
    void EncodeInsertPacket(CEntity* entity) override;
    void EncodeRemovePacket(CEntity* entity) override;
    //////////////////////////////////////////////////////////////////////////////////

    public:
    CHARACTER* FindCharacterInView(const char* name, bool bFindPCOnly);
    std::vector<CHARACTER*> FindCharactersInRange(int32_t range);
    void UpdatePacket();
    void SendBlockModeInfo();


    //////////////////////////////////////////////////////////////////////////////////
    // FSM (Finite State Machine) °ü·Ã
    protected:
    CStateTemplate<CHARACTER> m_stateMove;
    CStateTemplate<CHARACTER> m_stateBattle;
    CStateTemplate<CHARACTER> m_stateIdle;

    public:
    virtual void BeginStateEmpty();

    virtual void EndStateEmpty() {}

    virtual void StateMove();
    virtual void StateBattle();
    virtual void StateIdle();
    virtual void StateFlag();
    virtual void StateFlagBase();
    void StateHorse();

    uint32_t GetStateDuration() const { return m_dwStateDuration; }

    void SetStateDuration(uint32_t val) { m_dwStateDuration = val; }

    void SetStateHorse();
    void SetStateEmpty();

    private:
    uint32_t m_dwStateDuration;

    protected:
    // STATE_IDLE_REFACTORING
    void __StateIdle_Monster();
    void __StateIdle_Stone();
    void __StateIdle_NPC();
    // END_OF_STATE_IDLE_REFACTORING

    public:
    uint32_t GetAIFlag() const { return m_pointsInstant.dwAIFlag; }

    void SetAggressive();
    bool IsAggressive() const;

    void SetCoward();
    bool IsCoward() const;
    void CowardEscape();

    void SetNoAttackShinsu();
    bool IsNoAttackShinsu() const;

    void SetNoAttackChunjo();
    bool IsNoAttackChunjo() const;

    void SetNoAttackJinno();
    bool IsNoAttackJinno() const;

    void SetAttackMob();
    bool IsAttackMob() const;

    void SetPetPawn();
    bool IsPetPawn() const;

    void SetNoMoveFlag() { SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE); }

    bool IsNoMoveFlag() const
    {
        return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE);
    }

    //////////////////////////////////////////////////////////////////////////////////

    public:
    CHARACTER();
    virtual ~CHARACTER();

    void Create(std::string c_pszName, uint32_t vid, bool isPC);
    void Destroy() override;

    void Disconnect(const char* c_pszReason);

    protected:
    void Initialize() override;

    //////////////////////////////////////////////////////////////////////////////////
    // Basic Points
    public:
    void SetAccountID(uint32_t aid) { m_dwAccountID = aid; }

    void SetPlayerID(uint32_t pid) { m_dwPlayerID = pid; }

    uint32_t GetPlayerID() const { return m_dwPlayerID; }

    void DelayedItemLoad();
    void SetPlayerProto(const TPlayerTable* table);
    void CreatePlayerProto(TPlayerTable& tab); // ÀúÀå ½Ã »ç¿ë

    void SetProto(const TMobTable* c_pkMob);

    uint32_t GetRaceNum() const;
    uint16_t GetRealRaceNum() const;

    // Cache
    void Save();
    void SaveReal();
    void FlushDelayedSaveItem();

    void SetSkipSendPoint(bool val) { m_skipSendPoints = val; }

    public:
    const std::string& GetName() const;

    const std::string& GetProtoName() const;

    const VID& GetVID() const { return m_vid; }

    void SetName(const std::string& name) { m_stName = name; }

    bool HasCustomName() const { return m_bHasCustomName; }

    void SetHasCustomeName(bool val) { m_bHasCustomName = val; }

    void SetRace(uint8_t race);
    bool ChangeSex();

    uint8_t GetJob() const;
    uint8_t GetCharType() const;

    bool IsPC() const { return GetDesc() ? true : false; }

    bool IsNPC() const { return m_bCharType != CHAR_TYPE_PC; }

    bool IsMonster() const { return m_bCharType == CHAR_TYPE_MONSTER; }

    bool IsStone() const { return m_bCharType == CHAR_TYPE_STONE; }

    bool IsDoor() const { return m_bCharType == CHAR_TYPE_DOOR; }

    bool IsBuilding() const { return m_bCharType == CHAR_TYPE_BUILDING; }

    bool IsWarp() const { return m_bCharType == CHAR_TYPE_WARP; }

    bool IsGoto() const { return m_bCharType == CHAR_TYPE_GOTO; }

    bool IsShop() const { return m_bCharType == CHAR_TYPE_SHOP; }

    bool IsBuffBot() const { return m_bCharType == CHAR_TYPE_BUFFBOT; }

    // Time pulse checks
    uint32_t GetLastShoutPulse() const
    {
        return m_pointsInstant.dwLastShoutPulse;
    }

    void SetLastShoutPulse(uint32_t pulse)
    {
        m_pointsInstant.dwLastShoutPulse = pulse;
    }

    uint32_t GetLastInventorySortPulse() const
    {
        return m_pointsInstant.lastInventorySortPulse;
    }

    void SetLastInventorySortPulse(uint32_t pulse)
    {
        m_pointsInstant.lastInventorySortPulse = pulse;
    }

    bool IsLevelPetStatusWindowOpen() const
    {
        return m_pointsInstant.pOpenLevelPet != nullptr;
    }

    uint8_t GetGMLevel(bool bIgnoreTestServer = false) const;
    bool IsGM() const;
    void SetGMLevel();

    void SetLevel(uint8_t level);
#ifdef __FAKE_PC__
    int GetLevel() const
    {
        return FakePC_Check() ? FakePC_GetOwner()->m_points.level
                              : m_points.level;
    }
#else
    int GetLevel() const { return m_points.level; }
#endif

    PointValue GetExp() const { return m_points.exp; }

    PointValue GetNextExp() const;

    void SetExp(PointValue exp) { m_points.exp = exp; }

    CHARACTER* DistributeExp();

    bool CanFall();

    void DistributeHP(CHARACTER* pkKiller);
    void DistributeSP(CHARACTER* pkKiller, int iMethod = 0);

    void SetPosition(int pos);

    bool IsPosition(int pos) const
    {
        return m_pointsInstant.position == pos ? true : false;
    }

    int GetPosition() const { return m_pointsInstant.position; }

    void SetPart(uint8_t index, const Part& part);
    Part GetPart(uint8_t index) const;
    Part GetOriginalPart(uint8_t index) const;

    void SetHP(int64_t hp) { m_points.hp = hp; }

    int64_t GetHP() const { return m_points.hp; }

    void SetSP(int sp) { m_points.sp = sp; }

    int GetSP() const { return m_points.sp; }

    void SetStamina(int stamina) { m_points.stamina = stamina; }

    int GetStamina() const { return m_points.stamina; }

    void SetMaxHP(int64_t iVal) { m_pointsInstant.iMaxHP = iVal; }

    int64_t GetMaxHP() const { return m_pointsInstant.iMaxHP; }

    void SetMaxSP(int iVal) { m_pointsInstant.iMaxSP = iVal; }

    int GetMaxSP() const { return m_pointsInstant.iMaxSP; }

    void SetMaxStamina(int iVal) { m_pointsInstant.iMaxStamina = iVal; }

    int GetMaxStamina() const { return m_pointsInstant.iMaxStamina; }

    void SetRandomHP(int v) { m_points.iRandomHP = v; }

    int GetRandomHP() const { return m_points.iRandomHP; }

    void SetRandomSP(int v) { m_points.iRandomSP = v; }

    int GetRandomSP() const { return m_points.iRandomSP; }

    int GetHPPct(int perc = 100) const;

    void SetRealPoint(uint8_t idx, PointValue val);
    PointValue GetRealPoint(uint8_t idx) const;

    void SetPoint(uint8_t idx, PointValue val);
    PointValue GetPoint(uint8_t idx) const;
    PointValue GetLimitPoint(uint8_t idx) const;
    PointValue GetPolymorphPoint(uint8_t idx) const;

    const TMobTable& GetMobTable() const;
    uint8_t GetMobRank() const;
    uint8_t GetMobBattleType() const;
    uint16_t GetMobScale() const;
    uint32_t GetMobVnum() const;
    uint32_t GetMobDamageMin() const;
    uint32_t GetMobDamageMax() const;
    uint16_t GetMobAttackRange() const;
    uint32_t GetMobDropItemVnum() const;
    float GetMobDamageMultiply() const;

    // NEWAI
    bool IsBerserker() const;
    bool IsBerserk() const;
    void SetBerserk(bool mode);

    bool IsStoneSkinner() const;

    bool IsGodSpeeder() const;
    bool IsGodSpeed() const;
    void SetGodSpeed(bool mode);

    bool IsDeathBlower() const;
    bool IsDeathBlow() const;

    bool IsReviver() const;
    bool HasReviverInParty() const;
    bool IsRevive() const;
    void SetRevive(bool mode);
    // NEWAI END
    bool SetMobPosLastAttacked(PIXEL_POSITION& pos);

    bool IsRaceFlag(uint32_t dwBit) const;
    bool IsSummonMonster() const;

    bool CanSummonMonster();
    uint32_t GetSummonVnum() const;

    uint32_t GetPolymorphItemVnum() const;
    uint32_t GetMonsterDrainSPPoint() const;

    void MainCharacterPacket(); // ³»°¡ ¸ÞÀÎÄ³¸¯ÅÍ¶ó°í º¸³»ÁØ´Ù.

    void ComputePoints();
    void ComputeBattlePoints();
    void PointChange(uint8_t type, PointValue amount, bool bAmount = false,
                     bool bBroadcast = false);
    void PointsPacket();
    void ApplyPoint(uint8_t bApplyType, ApplyValue iVal);
    void CheckMaximumPoints(); // HP, SP µîÀÇ ÇöÀç °ªÀÌ ÃÖ´ë°ª º¸´Ù ³ôÀºÁö
                               // °Ë»çÇÏ°í ³ô´Ù¸é ³·Ãá´Ù.

    bool Show(long lMapIndex, long x, long y, long z = LONG_MAX,
              bool bShowSpawnMotion = false, bool bAggressive = false);

    void SetRotation(float fRot);
    void SetRotationToXY(long x, long y);

    float GetRotation() const { return m_pointsInstant.fRot; }

    template <typename... Args>
    void ChatPacket(uint8_t type, const char* format, Args... args);

    void MonsterChat(uint8_t bMonsterChatType);
    void MonsterChat(const std::string& text);

    void ResetPoint(int iLv);

    void SetBlockMode(uint32_t bFlag);
    bool IsBlockMode(uint32_t bFlag, uint32_t pid = 0) const;

    bool IsBlockMode(uint32_t bFlag, CHARACTER* ch) const
    {
        return IsBlockMode(bFlag, ch ? ch->GetPlayerID() : 0);
    }

    bool IsPolymorphed() const { return m_dwPolymorphRace > 0; }

    bool IsPolyMaintainStat() const { return m_bPolyMaintainStat; }

    void SetPolymorph(uint32_t dwRaceNum, bool bMaintainStat = false);

    uint32_t GetPolymorphVnum() const { return m_dwPolymorphRace; }

    int GetPolymorphPower() const;

    // FISHING
    bool IS_VALID_FISHING_POSITION(int* returnPosx, int* returnPosy);
    void fishing();
    void fishing_take(uint32_t hitCount, float time);
    // END_OF_FISHING

    // MINING
    void mining(CHARACTER* chLoad);
    void mining_cancel();
    void mining_take();
    // END_OF_MINING

    void ResetPlayTime(uint32_t dwTimeRemain = 0);
    void CreateFly(uint8_t bType, CHARACTER* pkVictim);

    void ResetChatCounter() { m_bChatCounter = 0; }

    void IncreaseChatCounter() { ++m_bChatCounter; }

    uint8_t GetChatCounter() const { return m_bChatCounter; }

    void ResetWhisperCounter() { m_bWhisperCounter = 0; }

    void IncreaseWhisperCounter() { ++m_bWhisperCounter; }

    uint8_t GetWhisperCounter() const { return m_bWhisperCounter; }

    bool HasInstantFlag(uint32_t flag) const
    {
        return IS_SET(m_pointsInstant.instant_flag, flag);
    }

    void SetInstantFlag(uint32_t flag)
    {
        SET_BIT(m_pointsInstant.instant_flag, flag);
    }

    void ClearInstantFlag(uint32_t flag)
    {
        REMOVE_BIT(m_pointsInstant.instant_flag, flag);
    }

    const std::string& GetClientLocale() const { return m_clientLocale; }

    void SetClientLocale(const std::string& locale)
    {
        m_clientLocale.assign(locale);
    }

    void MarkAsShop() { m_bCharType = CHAR_TYPE_SHOP; }

    void MarkAsPet() { m_bCharType = CHAR_TYPE_PET; }

    void MarkAsAttackPet() { m_bCharType = CHAR_TYPE_ATTACK_PET; }

    void MarkAsMount() { m_bCharType = CHAR_TYPE_MOUNT; }

    uint32_t GetPlayStartTime() const { return m_dwLoginPlayTime; }

    void SetCharType(ECharType charType) { m_bCharType = charType; }

    protected:
    uint32_t m_dwPolymorphRace;
    uint32_t m_dwPlayerID = 0;
    uint32_t m_dwAccountID = 0;

    VID m_vid;
    std::string m_stName;
    ECharType m_bCharType;

    bool m_bHasCustomName = false;
    bool m_skipSendPoints = false;
    bool m_bSkipSave = false;
    bool m_bPolyMaintainStat;

    uint32_t m_newSummonInterval;
    uint32_t m_lastSummonTime;

    CHARACTER_POINT m_points = {};
    CHARACTER_POINT_INSTANT m_pointsInstant = {};
    Windows m_windows;

    uint32_t m_dwPlayStartTime;
    uint32_t m_dwLoginPlayTime = 0;

    uint8_t m_bAddChrState;
    uint8_t m_bChatCounter;
    uint8_t m_bWhisperCounter = 0;

    std::string m_clientLocale;

    int32_t m_RegenDelay;
    int32_t m_RegenPctReduction;

    // End of Basic Points

    //////////////////////////////////////////////////////////////////////////////////
    // Move & Synchronize Positions
    //////////////////////////////////////////////////////////////////////////////////
    public:
    bool IsStateMove() const { return IsState((CState&)m_stateMove); }

    bool IsStateIdle() const { return IsState((CState&)m_stateIdle); }

    bool IsWalking() const { return m_bNowWalking; }

    void SetWalking(bool bWalkFlag) { m_bWalking = bWalkFlag; }

    void SetNowWalking(bool bWalkFlag);

    void ResetWalking() { SetNowWalking(m_bWalking); }

    void Stop();
    bool Goto(int32_t x, int32_t y);
    bool ForceGoto(int32_t x, int32_t y);
    bool Goto(uint32_t startTime, int32_t x, int32_t y);

    PIXEL_POSITION GetInterpolatedPosition(uint32_t time) const;

    bool CanMove() const; // 이동할 수 있는가?

    bool Sync(int32_t x, int32_t y); // 실제 이 메소드로 이동 한다 (각 종 조건에
                                     // 의한 이동 불가가 없음)
    bool Move(int32_t x,
              int32_t y); // 조건을 검사하고 Sync 메소드를 통해 이동 한다.
    void SendMovePacket(uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y,
                        uint32_t dwDuration, uint32_t dwTime = 0,
                        int iRot = -1);
    void SendBuffBotSkillPacket(uint32_t skill_vnum);
    void SyncPacket(CEntity* except = nullptr);
    void OnMove(bool bIsAttack = false); // 움직일때 불린다. Move() 메소드
                                         // 이외에서도 불릴 수 있다.

    uint32_t GetMotionMode() const;
    const Motion* GetMoveMotion(bool forceRun = false) const;
    float GetMoveSpeed() const;
    uint32_t CalculateMoveDuration();

    void SetClientLastMoveTime(uint32_t time) { m_clientLastMoveTime = time; }

    uint32_t GetClientLastMoveTime() const { return m_clientLastMoveTime; }

    void SetLastMoveTime(uint32_t time) { m_lastMoveTime = time; }

    uint32_t GetLastMoveTime() const { return m_lastMoveTime; }

    uint32_t GetLastAttackTime() const { return m_dwLastAttackTime; }

    void SetLastAttacked(uint32_t time);

#ifdef __OFFLINE_SHOP__
    private:
    COfflineShop* m_pkViewingOfflineShop;
    COfflineShop* m_pkMyOfflineShop;

    public:
    void SetViewingOfflineShop(COfflineShop* pkOfflineShop);
    void SetMyOfflineShop(COfflineShop* pkOfflineShop);

    COfflineShop* GetViewingOfflineShop() const
    {
        return this->m_pkViewingOfflineShop;
    }

    COfflineShop* GetMyOfflineShop() const { return this->m_pkMyOfflineShop; }

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
    private:
    int m_iOfflineShopFarmedOpeningTime;
    int m_iOfflineShopSpecialOpeningTime;

    public:
    void SetOfflineShopFarmedOpeningTime(int iTime);
    void SetOfflineShopSpecialOpeningTime(int iTime);
    int GetOfflineShopFarmedOpeningTime() const;
    int GetOfflineShopSpecialOpeningTime() const;
    void SendLeftOpeningTimePacket();
#endif

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
    private:
    bool m_buildingOfflineShop;

    public:
    void OfflineShopStartCreate();
    void SetBuildingOfflineShop(bool val);
    bool IsBuildingOfflineShop() const;
    void SendOfflineShopStartCreatePacket();
#endif

    private:
    int m_iOfflineShopLastOpeningTime;

    public:
    int GetOfflineShopLastOpeningTime() const
    {
        return m_iOfflineShopLastOpeningTime;
    }

    void SetOfflineShopLastOpeningTime()
    {
        m_iOfflineShopLastOpeningTime = thecore_pulse();
    }
#endif

    private:
    // SyncPosition을 악용하여 타유저를 이상한 곳으로 보내는 핵 방어하기
    // 위하여, SyncPosition이 일어날 때를 기록.
    timeval m_tvLastSyncTime;

    public:
    void SetLastSyncTime(const timeval& tv)
    {
        memcpy(&m_tvLastSyncTime, &tv, sizeof(timeval));
    }

    const timeval& GetLastSyncTime() const { return m_tvLastSyncTime; }

    bool SetSyncOwner(CHARACTER* ch, bool bRemoveFromList = true);
    bool IsSyncOwner(CHARACTER* ch) const;

    uint32_t GetWalkStartTime() const { return m_walkStartTime; }

    void MarkHitReceived(uint32_t time) { m_dwLastHitReceivedTime = time; }

    // Both PC/NPC run this, executed when damaged.
    uint32_t GetLastHitReceivedTime() const { return m_dwLastHitReceivedTime; }

    void MarkPlayerHitReceived(uint32_t time)
    {
        m_dwLastPlayerHitReceivedTime = time;
    }
    void MarkPlayerHit(uint32_t time)
    {
        m_dwLastPlayerHitTime = time;
    }
    uint32_t GetLastPlayerHitTime() const
    {
        return m_dwLastPlayerHitTime;
    }
    // Both PC/NPC run this, executed when damaged.
    uint32_t GetLastPlayerHitReceivedTime() const
    {
        return m_dwLastPlayerHitReceivedTime;
    }

    bool WarpSet(long lRealMapIndex, long x, long y, uint8_t channel = 0);
    void SetWarpLocation(long lMapIndex, long x, long y);

    const PIXEL_POSITION& GetWarpPosition() const { return m_posWarp; }

    bool WarpToPID(uint32_t dwPID);

    uint32_t GetWarpMap() const { return m_lWarpMapIndex; }

    void SaveExitLocation();
    void ExitToSavedLocation();

    void StartStaminaConsume();
    void StopStaminaConsume();
    bool IsStaminaConsume() const;

    void ResetStopTime();
    uint32_t GetStopTime() const;

    int32_t GetPenaltyRegenTime() const { return m_RegenDelay; }

    void SetPenaltyRegenTime(int32_t value) { m_RegenDelay = value; }

    int32_t GetPenaltyRegenPct() const { return m_RegenPctReduction; }

    void SetPenaltyRegenPct(int32_t value)
    {
        m_RegenPctReduction = std::clamp(value, 0, 100);
    }

    protected:
    void ClearSync();

#ifdef ENHANCED_FLY_FIX
    float m_fSyncTime = 0.0f;
#endif
    CHARACTER* m_pkChrSyncOwner = nullptr;
    std::list<CHARACTER*> m_kLst_pkChrSyncOwned; // 내가 SyncOwner인 자들

    uint32_t m_motionKey;
    const Motion* m_motion;
    uint32_t m_motionStartTime;
    uint32_t m_motionDuration;

    // Motions with movement
    PIXEL_POSITION m_posDest;
    PIXEL_POSITION m_posStart;

    PIXEL_POSITION m_posWarp;
    int32_t m_lWarpMapIndex;

    PIXEL_POSITION m_posExit;
    int32_t m_lExitMapIndex;

    uint32_t m_clientLastMoveTime = 0;
    uint32_t m_lastMoveTime = 0;
    uint32_t m_dwLastAttackTime = 0;
    uint32_t m_dwLastHitReceivedTime = 0;
    uint32_t m_dwLastPlayerHitReceivedTime = 0;
    uint32_t m_dwLastPlayerHitTime = 0;
    uint32_t m_dwStopTime = 0;
    uint32_t m_lastSyncTime = 0;
    uint32_t m_walkStartTime = 0;

    bool m_bWalking;
    bool m_bNowWalking;
    bool m_bStaminaConsume;

    // End
#ifdef ENABLE_HYDRA_DUNGEON
    protected:
    bool m_bLockTarget = false;

    public:
    void LockOnTarget(bool bSet) { m_bLockTarget = bSet; }
#endif
    // Quickslot °ü·Ã
    public:
    void SyncQuickslot(uint8_t bType, uint16_t bOldPos, uint16_t bNewPos);
    int GetQuickslotPosition(uint8_t bType, uint16_t bInventoryPos);
    bool GetQuickslot(uint8_t pos, TQuickslot** ppSlot);
    bool SetQuickslot(uint8_t pos, const TQuickslot& rSlot);
    bool DelQuickslot(uint8_t pos);
    bool SwapQuickslot(uint8_t a, uint8_t b);
    void ChainQuickslotItem(uint16_t itemCell, uint8_t bType, uint16_t bOldPos);

    protected:
    TQuickslot m_quickslot[QUICKSLOT_MAX_NUM]{};
    ////////////////////////////////////////////////////////////////////////////////////////
    // Affect
    public:
    void StartAffectEvent();
    void ClearAffect();
    void ComputeAffect(const AffectData& affect, bool add);
    bool AddAffect(uint32_t type, uint8_t pointType, PointValue pointValue,
                   int32_t duration, int32_t spCost, bool override,
                   bool isCube = false, uint32_t color = 0);

    void RefreshAffect();
    bool RemoveAffect(uint32_t type);

    bool UpdateAffect(); // called from EVENT
    int ProcessAffect();

    void LoadAffect(uint32_t count, const AffectData* affects);
    void SaveAffect();

    bool IsGoodAffect(uint8_t bAffectType) const;

    void RemoveGoodAffect();
    void RemoveBadAffect();
    void RemoveBadAffectAfterDuel();
    AffectData* FindAffect(uint32_t dwType, uint8_t point = POINT_NONE);
    const AffectData* FindAffect(uint32_t dwType,
                                 uint8_t point = POINT_NONE) const;

    const AffectsHolder& GetAffectContainer() const { return m_affects; }

    bool RemoveAffect(const AffectData& affect);

    protected:
    AffectsHolder m_affects;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Switchbot
    //
    public:
    void UpdateSwitchbotSlotAttribute(uint32_t slotIndex, uint32_t tabIndex,
                                      uint32_t attrIndex, TItemApply attr);
    void SaveSwitchbotData();
    void LoadSwitchbotData(uint32_t count, const SwitchBotSlotData* data);
    void ActivateSwitchbotSlot(uint32_t slotIndex);
    std::optional<SwitchBotSlotData> GetSwitchbotSlotData(uint32_t slotIndex);
    void DeactivateSwitchbotSlot(uint32_t slotIndex);
    void ClearSwitchbotData();
    void OnSwitchbotDone(uint16_t cell);

    private:
    std::unordered_map<uint32_t, SwitchBotSlotData> m_switchbotData;

    public:
    // PARTY_JOIN_BUG_FIX
    void SetParty(CParty* pkParty);

    CParty* GetParty() const { return m_pkParty; }

    bool RequestToParty(CHARACTER* leader);
    void DenyToParty(CHARACTER* member);
    void AcceptToParty(CHARACTER* member);

    void PartyInvite(CHARACTER* pchInvitee);
    void PartyInviteAccept(CHARACTER* pchInvitee);
    void PartyInviteDeny(uint32_t dwPID);

    bool BuildUpdatePartyPacket(TPacketGCPartyUpdate& out);
    int GetLeadershipSkillLevel() const;

    bool CanSummon(int iLeaderShip);
    void SetMountVnum(uint32_t vnum);

    void SetPartyRequestEvent(LPEVENT pkEvent)
    {
        m_pkPartyRequestEvent = pkEvent;
    }

    protected:
    void PartyJoin(CHARACTER* pkLeader);

    enum PartyJoinErrCode {
        PERR_NONE = 0,
        PERR_SERVER,
        PERR_DUNGEON,
        PERR_OBSERVER,
        PERR_LVBOUNDARY,
        PERR_LOWLEVEL,
        PERR_HILEVEL,
        PERR_ALREADYJOIN,
        PERR_PARTYISFULL,
        PERR_SEPARATOR,
        PERR_DIFFEMPIRE,
        PERR_MAX
    };

    static PartyJoinErrCode IsPartyJoinableCondition(const CHARACTER* pchLeader,
                                                     const CHARACTER* pchGuest);
    static PartyJoinErrCode
    IsPartyJoinableMutableCondition(const CHARACTER* pchLeader,
                                    const CHARACTER* pchGuest);

    CParty* m_pkParty;
    uint32_t m_dwLastDeadTime;
    LPEVENT m_pkPartyRequestEvent;
    using EventMap = std::unordered_map<uint32_t, LPEVENT>;
    EventMap m_PartyInviteEventMap;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Dungeon
    public:
    void SetDungeon(CDungeon* pkDungeon);

    CDungeon* GetDungeon() const { return m_pkDungeon; }

    protected:
    CDungeon* m_pkDungeon;
    int m_iEventAttr;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Guild
    public:
    void SetGuild(CGuild* pGuild);

    CGuild* GetGuild() const { return m_pGuild; }

    void SetWarMap(CWarMap* pWarMap);

    CWarMap* GetWarMap() const { return m_pWarMap; }

    protected:
    CGuild* m_pGuild;
    CWarMap* m_pWarMap;
    uint32_t m_dwUnderGuildWarInfoMessageTime;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Item related
    public:
    bool CanHandleItem(
        bool bSkipRefineCheck = false,
        bool bSkipObserver = false); // ¾ÆÀÌÅÛ °ü·Ã ÇàÀ§¸¦ ÇÒ ¼ö ÀÖ´Â°¡?
    void ClearItem();

    void SetItem(const TItemPos& Cell, CItem* item);
    CItem* GetItem(const TItemPos& Cell) const;
    CItem* GetItem_NEW(const TItemPos& Cell) const;
    // REAL item in the cell (Even if it's "empty" but actually used up). Lame
    // name but well...
    bool IsValidItemPosition(const TItemPos& Pos) const;

    CItem* GetInventoryItem(const uint16_t& wCell) const;
    bool IsEmptyItemGrid(TItemPos Cell, uint8_t size,
                         const CItem* except = nullptr) const;
    bool IsEmptyItemGridDS(const TItemPos& Cell, uint8_t size,
                           int iExceptionCell = -1) const;
    bool IsEmptyItemGridSpecialDS(const TItemPos& Cell, uint8_t bSize,
                                  int iExceptionCell,
                                  std::vector<uint16_t>& vec) const;

    int GetEmptyInventory(uint8_t size) const;
    int GetEmptyInventory(uint8_t size, uint8_t type) const;

    void SetWear(uint16_t bCell, CItem* item);
    CItem* GetWear(uint16_t bCell) const;

    CItem* GetSwitchbotItem(uint16_t bCell) const;
    void SetSwitchbotItem(uint16_t bCell, CItem* item);

    // MYSHOP_PRICE_LIST
    void UseSilkBotary();
    void UseSilkBotaryReal(const MyShopPriceListHeader* p);
    void UseKashmirBotary();
    void OpenPrivateShop();
    // END_OF_MYSHOP_PRICE_LIST

    bool HandleUseItemMetin(CItem* item, CItem* item2);
    bool HandleItemUsePotionNoDelay(CItem* item);
    bool HandleItemUsePotion(CItem* item);
    bool HandleItemUseAbilityUp(CItem* item);
    bool HandleItemUseLevelPetFood(CItem* item, CItem* item2);
    bool HandleItemUseChangePetAttr(CItem* item, CItem* item2);
    bool HandleItemUseEnhanceTime(CItem* item, CItem* item2);
    bool HandleItemUseTalisman(CItem* item);
    bool HandleUseItemUse(CItem* item, CItem* item2);
    bool HandleUseItemSkillbook(CItem* item);
    bool HandleItemUseCampfire(CItem* item);
    bool UseItemEx(CItem* item, CItem* item2);
    bool UseItem(const TItemPos& Cell, const TItemPos& = NPOS,
                 bool skipTimeChecks = false);

    // ADD_REFINE_BUILDING
    bool IsRefineThroughGuild() const;
    CGuild* GetRefineGuild() const;
    Gold ComputeRefineFee(Gold iCost, int iMultiply = 5) const;
    void PayRefineFee(Gold iTotalMoney);
    void SetRefineNPC(CHARACTER* ch);
    CHARACTER* GetRefineNPC() const;
    // END_OF_ADD_REFINE_BUILDING

    bool DropItem(const TItemPos& Cell, CountType bCount = 0);

#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
    bool RemoveItem(const TItemPos& Cell);
#endif

    bool GiveRecallItem(CItem* item);
    void ProcessRecallItem(CItem* item);

    void EffectPacket(int enumEffectType);
    void SpecificEffectPacket(const std::string& filename);
    void SpecificTargetEffectPacket(CHARACTER* target,
                                    const std::string& filename);

    bool DoRefine(CItem* item, int32_t refineType);
    bool DoRefineWithScroll(CItem* item, int32_t refineType);
    bool RefineInformation(uint16_t bCell, uint8_t bType,
                           int iAdditionalCell = -1);
    bool RefineItem(CItem* pkItem, CItem* pkTarget);

    void SetRefineMode(int iAdditionalCell = -1);
    void ClearRefineMode();

    bool GiveItem(CHARACTER* victim, const TItemPos& Cell);
    bool CanReceiveItem(CHARACTER* from, CItem* item,
                        bool ignoreDist = false) const;
    void ReceiveItem(CHARACTER* from, CItem* item);
    bool GiveItemFromSpecialItemGroup(uint32_t dwGroupNum,
                                      std::vector<uint32_t>& dwItemVnums,
                                      std::vector<uint32_t>& dwItemCounts,
                                      std::vector<CItem*>& item_gets,
                                      int& count);

    bool MoveItem(const TItemPos& pos, TItemPos change_pos, CountType num);
    bool PickupItem(uint32_t vid, uint32_t time);
    bool EquipItem(CItem* item, int iCandidateCell = -1);
    bool UnequipItem(CItem* item, bool unequipFromPlayer = false);

    bool SwapItems(const TItemPos& Cell, TItemPos& DestCell, CItem* item);
    bool SplitItem(const TItemPos& Cell, CountType count);
    bool SwapItem(TItemPos cell, TItemPos destCell);
    CItem* AutoGiveItem(uint32_t dwItemVnum, CountType bCount = 1,
                        int iRarePct = -1, bool bMsg = true);
    CItem* AutoGiveItem(CItem* item, bool longOwnerShip = false,
                        bool allowDrop = true);

    int GetEmptyInventory(const CItem* item) const;
    int GetEmptyCostumeInventoryCount(uint8_t size) const;
    int GetEmptyMaterialInventoryCount(uint8_t size) const;
    int GetEmptyNormalInventoryCount(uint8_t size) const;
    int GetEmptyInventoryCount(uint8_t size) const;
    int GetEmptyDragonSoulInventory(const TItemTable* pItemTable) const;
    int GetEmptyDragonSoulInventory(const CItem* pItem,
                                    int iExceptCell = -1) const;
    int GetEmptyDragonSoulInventoryWithExceptions(
        CItem* pItem, std::vector<uint16_t>& vec /*= -1*/) const;

    void
    CopyDragonSoulItemGrid(std::vector<uint16_t>& vDragonSoulItemGrid) const;

    int CountEmptyInventory() const;

    int CountSpecifyItem(uint32_t vnum, CItem* except = nullptr) const;
    void RemoveSpecifyItem(uint32_t vnum, CountType count = 1, CItem* except = nullptr);
    CItem* FindSpecifyItem(uint32_t vnum) const;
    CItem* FindQuiver() const;
    CItem* FindArrow() const;
    CItem* GetSwitcherByAttributeIndex(int setIndex, bool rare = false) const;

    CItem* FindItemByID(uint32_t id) const;

    int CountSpecifyTypeItem(uint8_t type, int8_t subtype = -1) const;
    CItem* FindSpecifyTypeItem(uint8_t type, int8_t subtype) const;
    void RemoveSpecifyTypeItem(uint8_t type, int8_t subtype = -1,
                               CountType count = 1);

    bool IsEquipUniqueItem(uint32_t dwItemVnum) const;

    // CHECK_UNIQUE_GROUP
    bool IsEquipUniqueGroup(uint32_t dwGroupVnum) const;
    // END_OF_CHECK_UNIQUE_GROUP

    void SendEquipment(CHARACTER* ch);

    // End of Item
    ItemGrid* GetGridWindow(uint8_t window) { return m_windows.Get(window); }

    // ACCE REFINE
    void AcceRefineCheckinCombine(uint8_t acceWindowCell,
                                  const TItemPos& invenPos,
                                  CItem* InventoryItem, CItem* OtherItem);
    void AcceRefineCheckinAbsorb(uint8_t acceWindowCell,
                                 const TItemPos& invenPos, CItem* InventoryItem,
                                 CItem* OtherItem);
    // ACCE REFINE
    void AcceRefineCheckin(uint8_t acceSlot, const TItemPos& currentCell,
                           uint8_t windowTypew);
    void AcceRefineCheckout(uint8_t acceSlot);
    void ChanceRefine(CItem* c_item, CItem* p_material_item, uint64_t cost);
    void AcceCombine();
    uint16_t GetAcceSlot(int32_t index);
    void SetAcceSlot(int32_t index, uint16_t slotIndex);
    CItem* GetAcceItemBySlot(int32_t index);
    bool IsEmptyAcceSlot(int32_t index) const;
    bool AcceAbsorb();
    void AcceRefineAccept(int32_t windowType);
    void AcceRefineClear();

    void ChangeLookCheckin(uint8_t pos, const TItemPos& itemPosition);
    void ChangeLookCheckout(uint8_t pos);
    void ChangeLookAccept();
    void ChangeLookCancel();

    protected:
    void SendMyShopPriceListCmd(uint32_t dwItemVnum, Gold dwItemPrice);
    bool m_bNoOpenedShop;
    bool m_bUnderRefine;
    uint32_t m_dwRefineNPCVID;
    int32_t m_iRefineAdditionalCell;

    public:
    ////////////////////////////////////////////////////////////////////////////////////////
    // Money related
    Gold GetGold() const { return m_points.gold; }

    void SetGold(Gold gold) { m_points.gold = gold; }

    bool DropGold(Gold gold);
    Gold GetAllowedGold() const;
    void GiveGold(Gold iAmount); // ÆÄÆ¼°¡ ÀÖÀ¸¸é ÆÄÆ¼ ºÐ¹è, ·Î±× µîÀÇ Ã³¸®
    void ChangeGold(Gold amount);
    Level GetMaxLevel() const;
    // End of Money

#ifdef ENABLE_GEM_SYSTEM
    INT GetGem() const { return m_points.gem; }
    void SetGem(INT gem) { m_points.gem = gem; }
    void ChangeGem(int amount);
#endif

    ////////////////////////////////////////////////////////////////////////////////////////
    // Shop related
    public:
    void SetShop(Shop* pkShop);

    Shop* GetShop() const { return m_pkShop; }

    void SetShopOwner(CHARACTER* ch) { m_pkChrShopOwner = ch; }

    CHARACTER* GetShopOwner() const { return m_pkChrShopOwner; }

    bool OpenMyShop(const char* sign, const TItemPos& bundleItem,
                    const TShopItemTable* table, uint8_t itemCount);

    PcShop* GetMyShop() const { return m_pkMyShop; }

    void CloseMyShop();

    //
    void SetShopVnum(uint32_t vnum) { m_shopVnum = vnum; }

    uint32_t GetShopVnum() const { return m_shopVnum; }

    void SetShopTitleType(uint32_t type) { m_shopTitleType = type; }

    uint32_t GetShopTitleType() const { return m_shopTitleType; }

    protected:
    Shop* m_pkShop = nullptr;
    PcShop* m_pkMyShop = nullptr;
    std::string m_stShopSign;
    CHARACTER* m_pkChrShopOwner = nullptr;
    bool m_myShopPricesRequsted = false;
    uint32_t m_shopTitleType = 0;
    GenericVnum m_shopVnum = 30008;
    ////////////////////////////////////////////////////////////////////////////////////////
    // Exchange related
    public:
    bool ExchangeStart(CHARACTER* victim);

    void SetExchange(CExchange* pkExchange);

    CExchange* GetExchange() const { return m_pkExchange; }

    protected:
    CExchange* m_pkExchange; // End of Exchange

    public:
    ////////////////////////////////////////////////////////////////////////////////////////
    // Battle
    struct TBattleInfo {
        DamageValue iTotalDamage;
#ifdef __FAKE_PC__
        DamageValue iTotalFakePCDamage;
#endif
        int iAggro;

#ifdef __FAKE_PC__
        TBattleInfo(DamageValue iTot, DamageValue iTotFakePC, int iAggr)
            : iTotalDamage(iTot)
            , iTotalFakePCDamage(iTotFakePC)
            , iAggro(iAggr)
#else
        TBattleInfo(DamageValue iTot, int iAggr)
            : iTotalDamage(iTot)
            , iAggro(iAggr)
#endif
        {
        }
    };

    using TDamageMap = std::map<VID, TBattleInfo>;

    typedef struct SAttackLog {
        uint32_t dwVID;
        uint32_t dwTime;
    } AttackLog;

    bool Damage(CHARACTER* pAttacker, DamageValue dam,
                EDamageType type = DAMAGE_TYPE_NORMAL, uint32_t skillVnum = 0);
    void DeathPenalty(uint8_t bExpLossPercent);
    void ReviveInvisible(int iDur);

    bool Attack(CHARACTER* victim, uint32_t motionKey, uint32_t time,
                uint8_t type = 0);

    bool IsAlive() const
    {
        return m_pointsInstant.position == POS_DEAD ? false : true;
    }

    bool CanFight() const;
    bool CanBeginFight() const;
    void BeginFight(CHARACTER* pkVictim);

    bool IsStun() const;
    bool IsDead() const;

    void Dead(CHARACTER* pkKiller = nullptr, bool bImmediateDead = false);
    void Stun();

    // reward

    void SetNoRewardFlag()
    {
        SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD);
    }

    void RemoveNoRewardFlag()
    {
        REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD);
    }

    bool HasNoRewardFlag() const
    {
        return IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD);
    }

    void Reward(bool bItemDrop);
    void RewardGold(CHARACTER* pkAttacker);

    //
    bool Shoot(uint8_t bType, uint32_t motionKey);
    void FlyTarget(uint32_t dwTargetVID, long x, long y, uint8_t bHeader);

    void ForgetMyAttacker();
    void AggregateMonster(uint32_t range);
    void AttractRanger();
    void PullMonster();

    int GetArrowAndBow(CItem** ppkBow, CItem** ppkArrow,
                       CountType iArrowCount = 1);

    void AttackedByPoison(CHARACTER* pkAttacker);
    void AttackedByBleeding(CHARACTER* pkAttacker);
    void RemovePoison();
    void RemoveBleeding();

    void AttackedByFire(CHARACTER* pkAttacker, int amount, int count);
    void RemoveFire();

    void UpdateAlignment(int iAmount);
    PlayerAlignment GetAlignment() const;

    int GetRealAlignment() const;
    void ShowAlignment(bool bShow);

    void SetKillerMode(bool bOn);
    bool IsKillerMode() const;
    void UpdateKillerMode();

    uint8_t GetPKMode() const;
    void SetPKMode(uint8_t bPKMode);

    void ItemDropPenalty(CHARACTER* pkKiller);

    void UpdateAggrPoint(CHARACTER* ch, EDamageType type, int dam);
    void UpdateAggrPointEx(CHARACTER* ch, EDamageType type, int dam,
                           TBattleInfo& info);
    void ChangeVictimByAggro(int iNewAggro, CHARACTER* pNewVictim);
    //
    // Time related anti cheat measures
    //
    public:
    uint32_t GetLastComboTime() const { return m_lastComboTime; }

    void SetLastComboTime(uint32_t time) { m_lastComboTime = time; }

    uint32_t GetComboInputTime() const { return m_comboInputTime; }

    void SetComboInputTime(uint32_t time) { m_comboInputTime = time; }

    uint32_t GetComboDuration() const { return m_comboDuration; }

    void SetComboDuration(uint32_t dur) { m_comboDuration = dur; }

    void IncreaseComboHackCount(int k = 1);
    void ResetComboHackCount();

    CharacterHitLog& GetHitLog() { return m_hitLog; }

    uint8_t GetComboLevel() const { return m_comboLevel; }

    uint8_t GetComboIndex() const { return m_comboIndex; }

    void SetComboIndex(uint8_t index) { m_comboIndex = index; }

    void SetComboSequence(uint8_t seq);
    uint8_t GetComboSequence() const;

    void SetSyncPosition(long x, long y);
    bool CheckSyncPosition(bool sync_check = false);
    void SetSyncCount(int count) { sync_count += count; }

    int GetValidComboInterval() const;
    void SetValidComboInterval(int interval);

    void SkipComboAttackByTime(int interval);
    uint32_t GetSkipComboAttackByTime() const;

    protected:
    uint32_t m_lastComboTime;
    uint32_t m_comboInputTime;
    uint32_t m_comboDuration;
    uint8_t m_comboLevel;
    uint8_t m_comboIndex;
    uint8_t m_bComboSequence;
    int32_t m_iComboHackCount;
    uint32_t m_dwLastComboTime;
    int32_t m_iValidComboInterval;
    uint32_t m_dwSkipComboAttackByTime;
    DWORD sync_hack;
    int sync_count;
    int sync_time;
    uint32_t m_dwFlyTargetID;
    std::vector<uint32_t> m_vec_dwFlyTargets;

    TDamageMap m_map_kDamage;
    uint32_t m_dwKillerPID;

    PlayerAlignment m_iAlignment; // Lawful/Chaotic value -200000 ~ 200000
    int m_iRealAlignment;
    int m_iKillerModePulse;
    uint8_t m_bPKMode;
    uint32_t m_dwLastVictimSetTime;
    int m_iMaxAggro;

    CharacterHitLog m_hitLog;

    public:
    void SetStone(CHARACTER* pkChrStone);
    void ClearStone();
    void DetermineDropMetinStone();

    uint32_t GetDropMetinStoneVnum() const { return m_dwDropMetinStone; }

    uint8_t GetDropMetinStonePct() const { return m_bDropMetinStonePct; }

    protected:
    CHARACTER* m_pkChrStone;
    CHARACTER_SET m_set_pkChrSpawnedBy;
    uint32_t m_dwDropMetinStone;
    uint8_t m_bDropMetinStonePct;
    // End of Stone

    public:
    void SkillLevelPacket();
    void SetSkillColor(uint32_t vnum, uint32_t color);
    void SkillLevelUp(uint32_t dwVnum, uint8_t bMethod = SKILL_UP_BY_POINT);
    bool SkillLevelDown(uint32_t dwVnum);

    bool CheckSkillHitCount(uint8_t SkillID, uint32_t TargetVID);
    bool CanUseSkill(uint32_t dwSkillVnum) const;
    bool CheckSkillRequirements(const CSkillProto* pkSk) const;
    bool IsLearnableSkill(uint32_t dwSkillVnum) const;

    bool UseSkill(uint32_t dwVnum, CHARACTER* pkVictim,
                  bool bUseGrandMaster = true);
    void ResetSkill();
    void ComputeMountPoints();

    int GetSkillLevel(uint32_t dwVnum) const;
    int GetSkillMasterType(uint32_t dwVnum) const;
    int GetSkillPower(uint32_t dwVnum, uint8_t bLevel = 0) const;
    uint32_t GetSkillColor(uint32_t vnum) const;
    int GetUsedSkillMasterType(uint32_t dwVnum);

    time_t GetSkillNextReadTime(uint32_t dwVnum) const;
    void SetSkillNextReadTime(uint32_t dwVnum, time_t time);
    void SkillLearnWaitMoreTimeMessage(uint32_t dwVnum);

    void ComputePassiveSkill(uint32_t dwVnum);
    int ComputeSkill(uint32_t dwVnum, CHARACTER* pkVictim,
                     uint8_t bSkillLevel = 0);
    int ComputeSkillAtPosition(uint32_t dwVnum, const PIXEL_POSITION& posTarget,
                               uint8_t bSkillLevel = 0);
    void ComputeSkillPoints();
    bool ComputePartySkill(uint32_t vnum, uint8_t skillLevel);

    void SetSkillGroup(uint8_t bSkillGroup);
    void SetSkillLevel(uint32_t dwVnum, uint8_t bLev);

    void ChangeSkillGroup();

#ifdef __FAKE_PC__
    uint8_t GetSkillGroup() const
    {
        return FakePC_Check() ? FakePC_GetOwner()->m_points.skill_group
                              : m_points.skill_group;
    }
#else
    uint8_t GetSkillGroup() const { return m_points.skill_group; }
#endif

    int ComputeCooltime(int time);

    void GiveRandomSkillBook();

    void DisableCooltime();
    bool LearnSkillByBook(uint32_t dwSkillVnum, uint8_t bProb = 0);
    bool LearnGrandMasterSkill(uint32_t dwSkillVnum);
    bool LearnLegendaryGrandMasterSkill(uint32_t dwSkillVnum);

    uint32_t GetLastSkillTime() const { return m_dwLastSkillTime; }

    private:
    bool m_bDisableCooltime;
    uint32_t m_dwLastSkillTime; ///< ¸¶Áö¸·À¸·Î skill À» ¾´ ½Ã°£(millisecond).
    uint32_t m_dwLastSkillVnum = 0;

    // End of Skill

    // MOB_SKILL
    public:
    bool HasMobSkill() const;
    size_t CountMobSkill() const;
    const TMobSkillLevel* GetMobSkill(unsigned int idx) const;
    bool CanUseMobSkill(unsigned int idx) const;
    bool UseMobSkill(unsigned int idx);
    void ResetMobSkillCooltime();

    void TouchMobSkillCooldown(uint32_t id, uint32_t time)
    {
        assert(id < MOB_SKILL_MAX_NUM && "ID out of range (MOB_SKILL_MAX_NUM)");
        m_adwMobSkillCooltime[id] = time;
    }

    uint32_t GetMobSkillCooldown(uint32_t id)
    {
        assert(id < MOB_SKILL_MAX_NUM && "ID out of range (MOB_SKILL_MAX_NUM)");
        return m_adwMobSkillCooltime[id];
    }

    protected:
    uint32_t m_adwMobSkillCooltime[MOB_SKILL_MAX_NUM];
    // END_OF_MOB_SKILL

    // for SKILL_MUYEONG
    public:
    void StartMuyeongEvent();
    void StopMuyeongEvent();

    private:
    LPEVENT m_pkMuyeongEvent;

    // for SKILL_CHAIN lighting
    public:
    int GetChainLightningIndex() const { return m_iChainLightingIndex; }

    void IncChainLightningIndex() { ++m_iChainLightingIndex; }

    void AddChainLightningExcept(CHARACTER* ch)
    {
        m_setExceptChainLighting.insert(ch);
    }

    void ResetChainLightningIndex()
    {
        m_iChainLightingIndex = 0;
        m_setExceptChainLighting.clear();
    }

    int GetChainLightningMaxCount() const;

    const CHARACTER_SET& GetChainLightingExcept() const
    {
        return m_setExceptChainLighting;
    }

    private:
    int m_iChainLightingIndex;
    CHARACTER_SET m_setExceptChainLighting;

    // for SKILL_EUNHYUNG
    public:
    void SetAffectedEunhyung();

    void ClearAffectedEunhyung() { m_dwAffectedEunhyungLevel = 0; }

    bool GetAffectedEunhyung() const { return m_dwAffectedEunhyungLevel != 0; }

    private:
    uint32_t m_dwAffectedEunhyungLevel;

    //
    // Skill levels
    //
    public:
    const TPlayerSkill* GetPlayerSkills() const { return m_pSkillLevels.get(); }

    protected:
    std::unique_ptr<TPlayerSkill[]> m_pSkillLevels;
    std::unordered_map<uint8_t, int> m_SkillDamageBonus;
    std::map<int, TSkillUseInfo> m_SkillUseInfo;

    public:
    const TPlayerTitle& GetPlayerTitle() const { return m_playerTitle; }

    protected:
    TPlayerTitle m_playerTitle = {};

    ////////////////////////////////////////////////////////////////////////////////////////
    // AI related
    public:
    void AssignTriggers(const TMobTable* table);
    CHARACTER* GetVictim() const; // °ø°ÝÇÒ ´ë»ó ¸®ÅÏ
    void SetVictim(CHARACTER* pkVictim);
    CHARACTER* GetNearestVictim(CHARACTER* pkChr);
    CHARACTER* GetProtege() const; // º¸È£ÇØ¾ß ÇÒ ´ë»ó ¸®ÅÏ

    bool Follow(CHARACTER* pkChr, float fMinimumDistance = 150.0f);
    bool Return();
    bool IsGuardNPC() const;
    bool IsChangeAttackPosition(CHARACTER* target) const;

    void ResetChangeAttackPositionTime()
    {
        m_dwLastChangeAttackPositionTime =
            get_dword_time() - AI_CHANGE_ATTACK_POISITION_TIME_NEAR;
    }

    void SetChangeAttackPositionTime()
    {
        m_dwLastChangeAttackPositionTime = get_dword_time();
    }

    bool OnIdle();

    void OnAttack(CHARACTER* pkChrAttacker);
    void OnClick(CHARACTER* pkChrCauser);
    void BuffBot_BuffClicker(CHARACTER* clickerCharacter);

    VID m_kVIDVictim;

#ifdef __HACKFIX_WALLHACK__
    private:
    PIXEL_POSITION m_ppLastPosition;
    uint32_t m_dwCountWallhackPoint;

    public:
    uint32_t GetWallhackCount() { return m_dwCountWallhackPoint; }

    void AddWallhackCount() { m_dwCountWallhackPoint += 1; }

    void ResetWallhackCount() { m_dwCountWallhackPoint = 0; }

    void SetLastPosition(int x, int y)
    {
        m_ppLastPosition.x = x;
        m_ppLastPosition.y = y;
    }

    PIXEL_POSITION GetLastPosition() { return m_ppLastPosition; }
#endif

    protected:
    uint32_t m_dwLastChangeAttackPositionTime;
    CTrigger m_triggerOnClick;
    // End of AI

    ////////////////////////////////////////////////////////////////////////////////////////
    // Target
    protected:
    CHARACTER* m_pkChrTarget; // ³» Å¸°Ù
    CHARACTER_SET m_set_pkChrTargetedBy; // ³ª¸¦ Å¸°ÙÀ¸·Î °¡Áö°í ÀÖ´Â »ç¶÷µé

    public:
    void SetTarget(CHARACTER* pkChrTarget);
    void BroadcastTargetPacket(uint32_t prevHp);
    void ClearTarget();
    void CheckTarget();

    CHARACTER* GetTarget() const { return m_pkChrTarget; }

    ////////////////////////////////////////////////////////////////////////////////////////
    // Safebox
    public:
    int GetSafeboxSize() const;

    CSafebox* GetSafebox() const;
    void LoadSafebox(int iSize, uint32_t dwGold, int iItemCount,
                     TPlayerItem* pItems);
    void ChangeSafeboxSize(uint8_t bSize);
    void CloseSafebox();
    void ReqSafeboxLoad(const char* pszPassword);

    void CancelSafeboxLoad() { m_bOpeningSafebox = false; }

    void SetMallLoadTime(int t) { m_iMallLoadTime = t; }

    int GetMallLoadTime() const { return m_iMallLoadTime; }

    CSafebox* GetMall() const;
    void LoadMall(int iItemCount, TPlayerItem* pItems);
    void CloseMall();

    void SetSafeboxOpenPosition();
    float GetDistanceFromSafeboxOpen() const;

    protected:
    std::unique_ptr<CSafebox> m_pkSafebox;
    int32_t m_iSafeboxSize;
    int32_t m_iSafeboxLoadTime;
    bool m_bOpeningSafebox;

    std::unique_ptr<CSafebox> m_pkMall;
    int m_iMallLoadTime;

    PIXEL_POSITION m_posSafeboxOpen;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Mounting
    public:
    void MountVnum(uint32_t vnum);

#ifdef __FAKE_PC__
    uint32_t GetMountVnum() const
    {
        return FakePC_Check() ? FakePC_GetOwner()->m_dwMountVnum
                              : m_dwMountVnum;
    }
#else
    uint32_t GetMountVnum() const { return m_dwMountVnum; }
#endif

    uint32_t GetLastMountTime() const { return m_dwMountTime; }

    bool CanUseHorseSkill() const;

    // Horse
    void SetHorseLevel(int iLevel) override;

    bool StartRiding() override;
    bool StopRiding() override;

    uint32_t GetMyHorseVnum() const override;

    void HorseDie() override;
    bool ReviveHorse() override;

    void SendHorseInfo() override;
    void ClearHorseInfo() override;

    void HorseSummon(bool bSummon, bool bFromFar = false, uint32_t dwVnum = 0,
                     const char* name = nullptr);

    CHARACTER* GetHorse() const { return m_chHorse; }

    CHARACTER* GetRider() const;
    void SetRider(CHARACTER* ch);

    bool IsRiding() const;

#ifdef __PET_SYSTEM__
    CPetSystem* GetPetSystem() { return m_petSystem.get(); }

    protected:
    std::unique_ptr<CPetSystem> m_petSystem;
#endif

    CHARACTER* m_chHorse;
    CHARACTER* m_chRider;

    uint32_t m_dwMountVnum;
    uint32_t m_dwMountTime;

    uint8_t m_bSendHorseLevel;
    uint8_t m_bSendHorseHealthGrade;
    uint8_t m_bSendHorseStaminaGrade;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Detailed Log
    public:
    void DetailLog() { m_bDetailLog = !m_bDetailLog; }

    void ToggleMonsterLog();
    template <typename... Args>
    void MonsterLog(const char* format, const Args&...);

    private:
    bool m_bDetailLog;
    bool m_bMonsterLog;

    public:
    ////////////////////////////////////////////////////////////////////////////////////////
    // Empire
    void SetEmpire(uint8_t bEmpire);

    uint8_t GetEmpire() const { return m_bEmpire; }

    protected:
    uint8_t m_bEmpire;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Regen
    public:
    void SetRegen(regen* pkRegen);

    protected:
    PIXEL_POSITION m_posRegen;
    float m_fRegenAngle;
    regen* m_pkRegen;
    size_t regen_id_; // to help dungeon regen identification
    // End of Regen

    ////////////////////////////////////////////////////////////////////////////////////////
    // Resists & Proofs
    public:
    bool CannotMoveByAffect() const;
    bool IsImmune(uint32_t dwImmuneFlag);

    void SetImmuneFlag(uint32_t dw) { m_pointsInstant.dwImmuneFlag = dw; }

    protected:
    void ApplyMobAttribute(const TMobTable* table);
    // End of Resists & Proofs

    ////////////////////////////////////////////////////////////////////////////////////////
    // QUEST
    //
    public:
    void SetQuestNPCID(uint32_t vid);

    uint32_t GetQuestNPCID() const { return m_dwQuestNPCVID; }

    CHARACTER* GetQuestNPC() const;

    void SetQuestBy(uint32_t dwQuestVnum) { m_dwQuestByVnum = dwQuestVnum; }

    uint32_t GetQuestBy() const { return m_dwQuestByVnum; }

    int GetQuestFlag(const std::string& flag) const;
    void SetQuestFlag(const std::string& flag, int value);

    void ConfirmWithMsg(const char* szMsg, int iTimeout, uint32_t dwRequestPID);

    private:
    uint32_t m_dwQuestNPCVID;
    uint32_t m_dwQuestByVnum;

    // Events
    public:
    bool StartStateMachine(int iPulse = 1);
    void StopStateMachine();
    void UpdateStateMachine(uint32_t dwPulse);
    void SetNextStatePulse(int iPulseNext);

    // Ä³¸¯ÅÍ ÀÎ½ºÅÏ½º ¾÷µ¥ÀÌÆ® ÇÔ¼ö. ±âÁ¸¿£ ÀÌ»óÇÑ »ó¼Ó±¸Á¶·Î CFSM::Update
    // ÇÔ¼ö¸¦ È£ÃâÇÏ°Å³ª UpdateStateMachine ÇÔ¼ö¸¦ »ç¿ëÇß´Âµ¥, º°°³ÀÇ ¾÷µ¥ÀÌÆ®
    // ÇÔ¼ö Ãß°¡ÇÔ.
    void UpdateCharacter(uint32_t dwPulse);

    protected:
    uint32_t m_dwNextStatePulse;

    // Marriage
    public:
    CHARACTER* GetMarryPartner() const;
    void SetMarryPartner(CHARACTER* ch);
    int GetMarriageBonus(uint32_t dwItemVnum, bool bSum = true);

    void SetWeddingMap(std::shared_ptr<marriage::WeddingMap> pMap);

    const std::shared_ptr<marriage::WeddingMap>& GetWeddingMap() const
    {
        return m_pWeddingMap;
    }

    private:
    std::shared_ptr<marriage::WeddingMap> m_pWeddingMap;
    CHARACTER* m_pkChrMarried;

    // Warp Character
    public:
    void StartWarpNPCEvent();
    void StartSaveEvent();
    void StartRecoveryEvent();
    void StartDestroyWhenIdleEvent();

    bool IsWarping() const { return !!m_pkWarpEvent; }

    public:
    LPEVENT m_pkDeadEvent = nullptr;
    LPEVENT m_pkStunEvent = nullptr;
    LPEVENT m_pkSaveEvent = nullptr;
    LPEVENT m_pkRecoveryEvent = nullptr;
    LPEVENT m_pkTimedEvent = nullptr;
    LPEVENT m_pkFishingEvent = nullptr;

    LPEVENT m_pkAffectEvent = nullptr;
    LPEVENT m_pkPoisonEvent = nullptr;
    LPEVENT m_pkBleedEvent = nullptr;

    LPEVENT m_pkFireEvent = nullptr;
    LPEVENT m_pkWarpNPCEvent = nullptr;
    LPEVENT m_despawnEvent = nullptr;
    LPEVENT m_miningEvent = nullptr;
    LPEVENT m_pkWarpEvent = nullptr;
    LPEVENT m_pkDestroyWhenIdleEvent = nullptr;
    LPEVENT m_pkPetSystemUpdateEvent = nullptr;
    LPEVENT m_pkCheckSpeedHackEvent = nullptr;

    private:
    const TMobTable* m_pkMobData = nullptr;
    std::unique_ptr<CMobInstance> m_pkMobInst;

    public:
    int GetPremiumRemainSeconds(uint8_t bType) const;

    bool CanRegenHP() const { return _canRegenHP; }

    void CanRegenHP(bool value) { _canRegenHP = value; }

    std::map<int, LPEVENT> m_mapMobSkillEvent;

    private:
    bool _canRegenHP = true;
    bool m_bHasPoisoned;
    bool m_bHasBleeded;

    public:
    void ClearSkill();
    void ClearSubSkill();
    bool ResetOneSkill(uint32_t dwVnum);

    void SendDamagePacket(CHARACTER* attacker, int damage, uint32_t damageFlag);

    private:
    void PirateTanaka_Damage();
    void AfterDamage(CHARACTER* pAttacker, int dam, EDamageType type);

#pragma region Arena

    CArena* m_pArena;
    bool m_ArenaObserver;
    int m_nPotionLimit;

    public:
    void SetArena(CArena* pArena) { m_pArena = pArena; }

    void SetArenaObserverMode(bool flag) { m_ArenaObserver = flag; }

    CArena* GetArena() const { return m_pArena; }

    bool GetArenaObserverMode() const { return m_ArenaObserver; }

    void SetPotionLimit(int count) { m_nPotionLimit = count; }

    int GetPotionLimit() const { return m_nPotionLimit; }

#pragma endregion

    public:
    bool IsOpenSafebox() const { return m_isOpenSafebox; }

    void SetOpenSafebox(bool b) { m_isOpenSafebox = b; }

    private:
    bool m_isOpenSafebox;

    public:
    bool IsHack(bool bSendMsg = true, bool bCheckShopOwner = true,
                int limittime = gConfig.portalLimitTime);
    void Say(const std::string& s);

    public:
    bool ItemProcess_Polymorph(CItem* item);

    bool IsCubeOpen() const { return m_pointsInstant.cubeOpen; }

    void SetCubeOpen(bool open) { m_pointsInstant.cubeOpen = open; }

    bool CanDoCube() const;
    bool IsSiegeNPC() const;

#pragma region Pulse_checks
    public:
    void UpdateEmotionPulse();
    void UpdateBraveryCapePulse();
    void UpdateBraveryCapePulsePremium();
    void UpdateEquipPulse();
    void UpdateAutoPotionPulseTp();
    void UpdateAutoPotionPulseMp();
    void UpdateEquipSkillUsePulse();
    void UpdateCampfirePulse();
    void UpdateFireworksPulse();
    void UpdateUseItemPulse();
    void UpdateComboChangePulse();
    void UpdateShopPulse();
    void UpdateFriendshipRequestPulse();
    void UpdateOfflineMessagePulse();
    void UpdateMountPulse();
    void UpdateDepositPulse();

    void SetBuffbotTime() { m_iBuffbotTime = thecore_pulse(); }

    void SetRefineTime() { m_iRefineTime = thecore_pulse(); }

    void SetUseSeedOrMoonBottleTime() { m_iSeedTime = thecore_pulse(); }

    void SetExchangeTime() { m_iExchangeTime = thecore_pulse(); }
    void SetMountTime() { m_iMountTime = thecore_pulse(); }

    void SetSafeboxTime() { m_iSafeboxTime = thecore_pulse(); }

    void SetMyShopTime() { m_iMyShopTime = thecore_pulse(); }

    void SetSafeboxLoadTime() { m_iSafeboxLoadTime = thecore_pulse(); }

    bool IsNextEmotionPulse() const;
    bool IsNextMountPulse() const;
    bool IsNextComboChangePulse() const;
    bool IsNextOfflineMessagePulse() const;
    bool CanUseBraveryCape() const;
    bool CanUseBraveryCapePremium() const;
    bool IsNextEquipPulse() const;
    bool IsNextEquipSkillUsePulse() const;
    bool CanUseCampfire() const;
    bool CanUseFireworks() const;
    bool IsNextUseItemPule() const;
    bool IsNextAutoPotionPulseTp() const;
    bool IsNextAutoPotionPulseMp() const;
    bool IsNextShopPulse() const;
    bool IsNextFriendshipRequestPulse() const;
    bool CanDeposit() const;

    int GetRefineTime() const { return m_iRefineTime; }

    int GetBuffbotTime() const { return m_iBuffbotTime; }

    int GetUseSeedOrMoonBottleTime() const { return m_iSeedTime; }

    int GetExchangeTime() const { return m_iExchangeTime; }
    int GetMountTime() const { return m_iMountTime; }

    int GetSafeboxTime() const { return m_iSafeboxTime; }

    int GetMyShopTime() const { return m_iMyShopTime; }

    int GetSafeboxLoadTime() const { return m_iSafeboxLoadTime; }

    private:
    int m_emotionPulse = 0;
    int m_braveryCapePulse = 0;
    int m_braveryCapePulsePremium = 0;
    int m_campFirePulse = 0;
    int n_fireworksPulse = 0;
    int m_equipPulse = 0;
    int m_equipSkillUsePulse = 0;
    int m_friendRequestPulse = 0;
    int m_autoPotionPulseTp = 0;
    int m_autoPotionPulseMp = 0;
    int m_mountPulse = 0;
    int m_useItemPulse = 0;
    int m_comboChangePulse = 0;
    int m_shopPulse = 0;
    int m_offlineMessagePulse = 0;
    int m_deposit_pulse;
    int m_iBuffbotTime = 0;
    int m_iRefineTime = 0;
    int m_iSeedTime = 0;
    int m_iExchangeTime = 0;
    int m_iSafeboxTime = 0;
    int m_iMyShopTime = 0;
	int m_iMountTime = 0;

#pragma endregion

    void __OpenPrivateShop();

    public:
    struct AttackedLog {
        uint32_t dwPID;
        uint32_t dwAttackedTime;

        AttackedLog()
            : dwPID(0)
            , dwAttackedTime(0)
        {
        }
    };

    AttackLog m_kAttackLog;
    AttackedLog m_AttackedLog;
    int m_speed_hack_count;

    public:
    void StartCheckSpeedHackEvent();
    void GoHome();

    private:
    std::unordered_set<uint32_t> m_known_guild;

    public:
    void SendGuildName(CGuild* pGuild);
    void SendGuildName(uint32_t dwGuildID);

    uint32_t GetLogOffInterval() const { return m_dwLogOffInterval; }

    uint32_t GetLastPlay() const { return m_dwLastPlay; }

    private:
    uint32_t m_dwLogOffInterval;
    uint32_t m_dwLastPlay;

    public:
    bool CanWarp() const;
    bool IsInSafezone() const;

    void BuffOnAttr_AddBuffsFromItem(CItem* pItem);
    void BuffOnAttr_RemoveBuffsFromItem(CItem* pItem);

    private:
    void BuffOnAttr_ValueChange(uint8_t bType, uint8_t bOldValue,
                                uint8_t bNewValue);
    void BuffOnAttr_ClearAll();

    using TMapBuffOnAttrs =
        std::unordered_map<uint8_t, std::unique_ptr<CBuffOnAttributes>>;
    TMapBuffOnAttrs m_map_buff_on_attrs;
    // ¹«Àû : ¿øÈ°ÇÑ Å×½ºÆ®¸¦ À§ÇÏ¿©.
    public:
    void SetArmada() { m_invincible = true; }

    void ResetArmada() { m_invincible = false; }

    private:
    bool m_invincible = false;

#ifdef __PET_SYSTEM__
    private:
    CPetActor* m_petActor = nullptr;

    public:
    void SetPet(CPetActor* pet) { m_petActor = pet; }

    bool IsPet() const { return m_petActor != nullptr; }

    CPetActor* GetPet() const { return m_petActor; }
#endif

    //ÃÖÁ¾ µ¥¹ÌÁö º¸Á¤.
    private:
    float m_fAttMul;
    float m_fDamMul;

    public:
    float GetAttMul() const { return m_fAttMul; }

    void SetAttMul(float newAttMul) { m_fAttMul = newAttMul; }

    float GetDamMul() const { return m_fDamMul; }

    void SetDamMul(float newDamMul) { m_fDamMul = newDamMul; }

    public:
#pragma region Dragonsoul
    void DragonSoul_Initialize();

    bool DragonSoul_IsQualified() const;
    void DragonSoul_GiveQualification();

    int DragonSoul_GetActiveDeck() const;
    bool DragonSoul_IsDeckActivated() const;
    bool DragonSoul_ActivateDeck(int deck_idx);

    void DragonSoul_DeactivateAll();
    void DragonSoul_CleanUp();
    // ¿ëÈ¥¼® °­È­Ã¢
#ifdef ENABLE_DS_SET
    void DragonSoul_HandleSetBonus();
#endif
    // 용혼석 강화창

    bool DragonSoul_RefineWindow_Open(bool v);
    bool DragonSoul_RefineWindow_Close();

    bool DragonSoul_RefineWindow_IsOpen() const
    {
        return m_pointsInstant.m_pDragonSoulRefineWindowOpener;
    }

    bool DragonSoul_RefineWindow_CanRefine();
#pragma endregion

#pragma region Synchack
    // sync hack
    void UpdateSyncHackCount(const std::string& who, bool increase);
    int GetSyncHackCount(const std::string& who);

    int GetSyncHackCount() const { return m_iSyncPlayerHackCount; }

    void SetSyncHackCount(int iCount) { m_iSyncPlayerHackCount = iCount; }

    int GetNoHackCount() const { return m_iSyncNoHackCount; }

    void SetNoHackCount(int iCount) { m_iSyncNoHackCount = iCount; }

    private:
    int m_iSyncPlayerHackCount = 0;
    int m_iSyncMonsterHackCount;
    int m_iSyncNoHackCount;
#pragma endregion
    public:
    // ACCE

    void SetAcceWindowType(EAcceSlotType type) { m_AcceWindowType = type; }

    int GetAcceWindowType() const { return m_AcceWindowType; }

    bool IsAcceWindowOpen()
    {
        return m_pointsInstant.pAcceSlots[ACCE_SLOT_LEFT] !=
                   INVENTORY_MAX_NUM ||
               m_pointsInstant.pAcceSlots[ACCE_SLOT_RIGHT] != INVENTORY_MAX_NUM;
    }

    private:
    EAcceSlotType m_AcceWindowType;

    template <typename T>
    using deleted_unique_ptr = std::unique_ptr<T, void (*)(T*)>;
    deleted_unique_ptr<CHARACTER> m_toggleMount;
    bool m_isToggleMount = false;

    public:
    bool HasToggleMount() const { return m_toggleMount != nullptr; }

    CHARACTER* CreateToggleMount(CItem* mountItem);

    void DestroyToggleMount() { m_toggleMount.reset(); }

    void SetAsToggleMount() { m_isToggleMount = true; }

    bool IsToggleMount() const { return m_isToggleMount; }

    CHARACTER* GetToggleMount() const { return m_toggleMount.get(); }

    public:
    void SendHuntingMissions();

    void SetMyShopDecoWindowOpen(bool state) { m_myShopDecoState = state; }

    bool IsMyShopDecoWindowOpen() const { return m_myShopDecoState; }

    bool IsRestrictedAccount() const { return m_isRestrictedAccount; }

    void SetRestrictedAccount(bool isRestricted)
    {
        m_isRestrictedAccount = isRestricted;
    }

    private:
    bool m_myShopDecoState = 0;
    bool m_isRestrictedAccount = 0;
#ifdef __OFFLINE_MESSAGE__
    public:
    void CheckOfflineMessage();
    int32_t GetCash(std::string cashType) const;
    void ChangeCash(int32_t amount, std::string strChargeType, bool add) const;
#endif

    public:
    void SetTitle(const std::string& title, uint32_t color);

    private:
    std::unordered_map<uint32_t, BlockedPC> m_messengerBlock;

    public:
    void LoadMessengerBlock(const ElementLoadBlockedPC* pcs, uint16_t count);
    void SendMessengerBlock();
    void SetMessengerBlock(uint32_t pid, const BlockedPC& pc);

    public:
    void SetPVPTeam(short sTeam);

    short GetPVPTeam() const { return m_sPVPTeam; }

    bool IsPrivateMap(long lMapIndex) const;

    private:
    int16_t m_sPVPTeam = -1;

#ifdef __FAKE_PC__
#pragma region FakePC
    public:
    // init / destroy
    void FakePC_Load(CHARACTER* pkOwner, CItem* pkSpawnItem = nullptr);
    void FakePC_Destroy();

    // set / get
    CHARACTER* FakePC_GetOwner() { return m_pkFakePCOwner; }

    CHARACTER* FakePC_GetOwner() const { return m_pkFakePCOwner; }

    CItem* FakePC_GetOwnerItem() { return m_pkFakePCSpawnItem; }

    bool FakePC_Check() const { return m_pkFakePCOwner != nullptr; }

    bool FakePC_IsSupporter() const
    {
        return FakePC_Check() && GetPVPTeam() != SHRT_MAX;
    }

    float FakePC_GetDamageFactor() { return m_fFakePCDamageFactor; }

    void FakePC_Owner_SetName(const char* szName) { m_stFakePCName = szName; }

    const char* FakePC_Owner_GetName() const { return m_stFakePCName.c_str(); }

    void FakePC_Owner_ResetAfkEvent();
    void FakePC_Owner_ClearAfkEvent();

    bool FakePC_CanAddAffect(const AffectData& pkAff);
    void FakePC_AddAffect(const AffectData& pkAff);
    void FakePC_RemoveAffect(const AffectData& pkAff);

    void FakePC_Owner_AddSpawned(CHARACTER* pkFakePC,
                                 CItem* pkSpawnItem = nullptr);
    bool FakePC_Owner_RemoveSpawned(CHARACTER* pkFakePC);
    bool FakePC_Owner_RemoveSpawned(CItem* pkSpawnItem);
    CHARACTER* FakePC_Owner_GetSpawnedByItem(CItem* pkItem);

    CHARACTER* FakePC_Owner_GetSupporter();
    CHARACTER* FakePC_Owner_GetSupporterByItem(CItem* pkItem);
    uint32_t FakePC_Owner_CountSummonedByItem();

    void FakePC_Owner_AddAffect(const AffectData& pkAff);
    void FakePC_Owner_RemoveAffect(const AffectData& pkAff);

    // compute
    void FakePC_Owner_ApplyPoint(ApplyType bType, ApplyValue lValue);
    void FakePC_Owner_ItemPoints(CItem* pkItem, bool bAdd);
    void FakePC_Owner_MountBuff(bool bAdd);
#ifdef __PET_SYSTEM__
    void FakePC_Owner_PetBuff(CPetActor* pkPetActor, bool bAdd);
#endif
#ifdef __ATTR_TREE__
    void FakePC_Owner_AttrTreeBuff(uint8_t bID, bool bAdd);
#endif
    void FakePC_ComputeDamageFactor();

    // spawn
    CHARACTER* FakePC_Owner_Spawn(int lX, int lY, CItem* pkItem = nullptr,
                                  bool bIsEnemy = false,
                                  bool bIsRedPotionEnabled = true);
    void FakePC_Owner_DespawnAll();
    bool FakePC_Owner_DespawnAllSupporter();
    bool FakePC_Owner_DespawnByItem(CItem* pkItem);

    // attack
    void FakePC_Owner_ForceFocus(CHARACTER* pkVictim);
    uint8_t FakePC_ComputeComboIndex();

    void FakePC_SetNoAttack() { m_bIsNoAttackFakePC = true; }

    bool FakePC_CanAttack() const { return !m_bIsNoAttackFakePC; }

    // skills
    bool FakePC_IsSkillNeeded(CSkillProto* pkSkill);
    bool FakePC_IsBuffSkill(uint32_t dwVnum);
    bool FakePC_UseSkill(CHARACTER* pkTarget = nullptr);
    void FakePC_SendSkillPacket(CSkillProto* pkSkill);

    // exec func for all fake pcs
    void FakePC_Owner_ExecFunc(void (CHARACTER::*func)());

    private:
    std::string m_stFakePCName;

    LPEVENT m_pkFakePCAfkEvent;

    CHARACTER* m_pkFakePCOwner;
    std::unordered_set<CHARACTER*> m_set_pkFakePCSpawns;

    CItem* m_pkFakePCSpawnItem;
    std::map<CItem*, CHARACTER*> m_map_pkFakePCSpawnItems;

    std::map<AffectData, AffectData> m_map_pkFakePCAffects;

    float m_fFakePCDamageFactor;

    bool m_bIsNoAttackFakePC;
#pragma endregion
#endif
#ifdef ENABLE_MELEY_LAIR_DUNGEON
    public:
    void SetMeley(MeleyLair::CMgrMap* pMeley) { m_pMeley = pMeley; }

    MeleyLair::CMgrMap* GetMeley() { return m_pMeley; }

    protected:
    MeleyLair::CMgrMap* m_pMeley = nullptr;
#endif

    public:
    void SetForceMonsterAttackRange(uint32_t dwRange)
    {
        m_dwForceMonsterAttackRange = dwRange;
    }

    uint32_t GetForceMonsterAttackRange() const
    {
        return m_dwForceMonsterAttackRange;
    }

    private:
    uint32_t m_dwForceMonsterAttackRange = 0;
#ifdef ENABLE_BATTLE_PASS
    public:
    void LoadBattlePass(uint32_t dwCount, TPlayerBattlePassMission* data);
    uint64_t GetMissionProgress(uint32_t dwMissionID, uint32_t dwBattlePassID);
    void UpdateMissionProgress(uint32_t dwMissionID, uint32_t dwBattlePassID,
                               uint64_t dwUpdateValue, uint64_t dwTotalValue,
                               bool isOverride = false);
    bool IsCompletedMission(uint8_t bMissionType);

    bool IsLoadedBattlePass() const { return m_bIsLoadedBattlePass; }

    uint8_t GetBattlePassId();
    uint8_t GetBattlePassState();
    bool IsChatFilter(uint8_t bLanguageID) const;
    void SetChatFilter(uint8_t bLanguageID, bool bValue);
    void UpdateChatFilter();

    private:
    bool m_bIsLoadedBattlePass;
    std::list<TPlayerBattlePassMission*> m_listBattlePass;
#endif
#ifdef ENABLE_BATTLE_PASS
    public:
    int GetBattlePassEndTime() const
    {
        return m_dwBattlePassEndTime - get_global_time();
    }

    protected:
    uint32_t m_dwBattlePassEndTime;
#endif
    public:
    void LoadHuntingMissions(uint32_t dwCount, TPlayerHuntingMission* data);
    uint32_t GetHuntingMissionProgress(uint32_t dwMissionID);
    bool IsHuntingMissionCompleted(uint32_t dwMissionID);
    bool IsHuntingMissionForCurrentDay(uint32_t dwMissionID);

    bool UpdateHuntingMission(uint32_t dwMissionID, uint32_t dwUpdateValue,
                              uint32_t dwTotalValue, bool isOverride = false);
    void GiveHuntingMissionReward(const HuntingMission* mission);

    private:
    std::vector<TPlayerHuntingMission> m_huntingMissions;

    // Level Pet
    public:
    void LevelPetOpen(const TItemPos& itemPos);
    void LevelPetClose();
    void LevelPetPlusAttr(uint8_t index);

    public:
    bool IsDungeonInfoLoaded() const { return m_bDungeonInfoLoaded; }
    void SetDungeonInfoLoaded() { m_bDungeonInfoLoaded = true; }

    void PushDungeonInfo(TPlayerDungeonInfo nh) { m_dungeonInfo.push_back(nh); }

    void SetDungeonInfoVec(std::vector<TPlayerDungeonInfo> vec)
    {
        m_dungeonInfo = vec;
    }
    std::vector<TPlayerDungeonInfo> GetDungeonInfoVec() const
    {
        return m_dungeonInfo;
    }

    void SetDungeonCooldown(uint8_t bDungeonID, uint16_t wCooldown);
    void ResetDungeonCooldown(uint8_t bDungeonID);
    uint32_t GetDungeonCooldownEnd(uint8_t bDungeonID);

    void SetDungeonRanking(uint8_t bDungeonID, uint8_t bCompleteCount,
                           uint32_t wFastestTime, DamageValue dwHightestDamage);

    void SendDungeonInfo();

    protected:
    std::vector<TPlayerDungeonInfo> m_dungeonInfo;
    bool m_bDungeonInfoLoaded;

    // Search
    public:
    void ShopSearch_ItemSearch(const int32_t vnum, const int32_t extra);
    void ShopSearch_OpenFilter(int32_t filterType);
    void ShopSearch_UpdateResults();
    void ShopSearch_OpenCategory(const int32_t itemType,
                                 const int32_t itemSubType);
    void ShopSearch_SetPage(const uint16_t page);
    void ShopSearch_BuyItem(const uint32_t itemId);
    void ShopSearch_HandleBuyItemFromDb(const TPacketOfflineShopSendItemInfo & itemInfo);
    void ShopSearch_HandleBuyCompleteFromDb(const TPacketOfflineShopSendItemInfo & itemInfo);
    void ShopSearch_SetResults(const std::vector<ShopSearchItemData> & items);

    private:
    int32_t m_shopSearchFilterType = -1;
    int32_t m_shopSearchItemType = -1;
    int32_t m_shopSearchItemSubType = -1;
    int64_t m_shopSearchVnumFilter = -1;
    int64_t m_shopSearchVnumExtraFilter = -1;
    uint32_t m_shopSearchPerPage;
    uint32_t m_shopSearchCurrentPage;
    uint32_t m_shopSearchPageCount;
    bool m_shopSearchSettingsChanged = false;
    std::vector<ShopSearchItemData> m_shopSearchData;
    std::vector<ShopSearchItemData> m_shopSearchCurrentPageData;

};

template <typename... Args>
void CHARACTER::ChatPacket(uint8_t type, const char* format, Args... args)
{
    std::string message;

    try {
        message = fmt::sprintf(format, std::forward<Args>(args)...);
    } catch (const fmt::format_error& err) {
        message = format;
    }

    SendChatPacket(this, type, message);
}

template <typename... Args>
inline void CHARACTER::MonsterLog(const char* format, const Args&... args)
{
    if (!gConfig.testServer)
        return;

    if (IsPC())
        return;

    auto chatString = fmt::format("({}) {}", (uint32_t)GetVID(),
                                  fmt::sprintf(format, args...));
    SPDLOG_TRACE(chatString);
}

EVENTFUNC(despawn_event);

#endif /* METIN2_SERVER_GAME_CHAR_H */
