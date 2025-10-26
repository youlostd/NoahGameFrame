#ifndef METIN2_CLIENT_MAIN_INSTANCEBASE_H
#define METIN2_CLIENT_MAIN_INSTANCEBASE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../gamelib/RaceData.h"
#include "../gamelib/ActorInstance.h"

#include <game/AffectsHolder.hpp>
#include <game/Constants.hpp>
#include <game/EffectConstants.hpp>
#include <game/MotionConstants.hpp>

#include "NetworkActorManager.h"
#include "../EterLib/Engine.hpp"

struct AffectData;
struct CharacterEffectInfo;

namespace
{
bool IsDuelMap(int32_t mapIndex)
{
    if (mapIndex >= 10000)
        mapIndex /= 10000;

    return false;
}

inline uint32_t EvaluatePartData(const Part &part)
{
    uint32_t data = part.vnum;

    if (part.vnum == part.appearance || part.appearance == 0)
        data = part.vnum;
    else
        data = part.appearance;

    if (Engine::GetSettings().IsShowCostume() || CItemData::IsWeddingDress(part.costume))
    {
        if (part.costume != 0 && (part.costume == part.costume_appearance || part.costume_appearance == 0))
            data = part.costume;
        else if (part.costume_appearance)
            data = part.costume_appearance;
    }

    return data;
}
}

void RegisterTitleName(int iIndex, const char *c_szTitleName, const char *c_szTitleNameFemale);
void RegisterNamePrefix(int iIndex, const char *prefix);
bool RegisterNameColor(UINT uIndex, UINT r, UINT g, UINT b);
bool RegisterTitleColor(UINT uIndex, UINT r, UINT g, UINT b);

struct SNetworkMoveActorData;

class CInstanceBase
{
public:
    struct SCreateData
    {
        uint8_t m_bType{};
        uint32_t m_dwStateFlags{};
        uint32_t m_dwEmpireID{};
        uint32_t m_dwGuildID{};
        uint32_t m_dwLevel{};
#if defined(WJ_SHOW_MOB_INFO)
        uint32_t m_dwAIFlag{};
#endif
        uint32_t m_dwVID{};
        uint32_t m_dwRace{};
        uint32_t m_dwMovSpd{};
        uint32_t m_dwAtkSpd{};
        LONG m_lPosX{};
        LONG m_lPosY{};
        uint32_t rotX, rotY{};
        float m_fRot{};
        Part m_parts[PART_MAX_NUM]{};

        uint32_t m_dwMountVnum = 0;
        uint32_t m_ownerVid = 0;
        uint16_t m_scale = 100;
        PlayerAlignment m_sAlignment{};
        uint8_t m_byPKMode{};

        std::string m_stName;

        uint8_t m_isGuildLeader = 0;
        uint8_t m_isGuildGeneral = 0;
        uint8_t m_langCode = 0;
#ifdef ENABLE_PLAYTIME_ICON
        DWORD m_dwPlayTime{};
#endif
        TPlayerTitle m_pt{};

        bool m_isMain{};
    };

public:
    typedef uint32_t TType;

    enum EMobAIFlags
    {
        AIFLAG_AGGRESSIVE = (1 << 0),
        AIFLAG_NOMOVE = (1 << 1),
        AIFLAG_COWARD = (1 << 2),
        AIFLAG_NOATTACKSHINSU = (1 << 3),
        AIFLAG_NOATTACKJINNO = (1 << 4),
        AIFLAG_NOATTACKCHUNJO = (1 << 5),
        AIFLAG_ATTACKMOB = (1 << 6),
        AIFLAG_BERSERK = (1 << 7),
        AIFLAG_STONESKIN = (1 << 8),
        AIFLAG_GODSPEED = (1 << 9),
        AIFLAG_DEATHBLOW = (1 << 10),
        AIFLAG_REVIVE = (1 << 11),
        AIFLAG_HEALER = (1 << 12),
        AIFLAG_COUNT = (1 << 13),
        AIFLAG_NORECOVERY = (1 << 14),
        AIFLAG_REFLECT = (1 << 15),
        AIFLAG_FALL = (1 << 16),
        AIFLAG_VIT = (1 << 17),
        AIFLAG_RATTSPEED = (1 << 18),
        AIFLAG_RCASTSPEED = (1 << 19),
        AIFLAG_RHP_REGEN = (1 << 20),
        AIFLAG_TIMEVIT = (1 << 21),
        AIFLAG_NOPUSH = (1 << 22),
    };

    enum EDirection
    {
        DIR_NORTH,
        DIR_NORTHEAST,
        DIR_EAST,
        DIR_SOUTHEAST,
        DIR_SOUTH,
        DIR_SOUTHWEST,
        DIR_WEST,
        DIR_NORTHWEST,
        DIR_MAX_NUM,
    };

    enum
    {
        FUNC_WAIT,
        FUNC_MOVE,
        FUNC_ATTACK,
        FUNC_COMBO,
        FUNC_MOB_SKILL,
        FUNC_EMOTION,
        FUNC_SKILL = 0x80,
    };

    enum
    {
        NEW_AFFECT_MOV_SPEED = 200,
        NEW_AFFECT_ATT_SPEED,
        NEW_AFFECT_ATT_GRADE,
        NEW_AFFECT_INVISIBILITY,
        NEW_AFFECT_STR,
        NEW_AFFECT_DEX,
        // 205
        NEW_AFFECT_CON,
        NEW_AFFECT_INT,
        NEW_AFFECT_FISH_MIND_PILL,

        NEW_AFFECT_POISON,
        NEW_AFFECT_STUN,
        // 210
        NEW_AFFECT_SLOW,
        NEW_AFFECT_DUNGEON_READY,
        NEW_AFFECT_DUNGEON_UNIQUE,

        NEW_AFFECT_BUILDING,
        NEW_AFFECT_REVIVE_INVISIBLE,
        // 215
        NEW_AFFECT_FIRE,
        NEW_AFFECT_CAST_SPEED,
        NEW_AFFECT_HP_RECOVER_CONTINUE,
        NEW_AFFECT_SP_RECOVER_CONTINUE,

        NEW_AFFECT_POLYMORPH,
        // 220
        NEW_AFFECT_MOUNT,

        NEW_AFFECT_WAR_FLAG,
        // 222

        NEW_AFFECT_BLOCK_CHAT,
        // 223
        NEW_AFFECT_CHINA_FIREWORK,

        NEW_AFFECT_BOW_DISTANCE,
        // 225

        NEW_AFFECT_EXP_BONUS = 500,
        // ������ ����
        NEW_AFFECT_ITEM_BONUS = 501,
        // ������ �尩
        NEW_AFFECT_SAFEBOX = 502,
        // PREMIUM_SAFEBOX,
        NEW_AFFECT_AUTOLOOT = 503,
        // PREMIUM_AUTOLOOT,
        NEW_AFFECT_FISH_MIND = 504,
        // PREMIUM_FISH_MIND,
        NEW_AFFECT_MARRIAGE_FAST = 505,
        // ������ ���� (�ݽ�),
        NEW_AFFECT_GOLD_BONUS = 506,
        NEW_AFFECT_SHOP_DOUBLE_UP = 507,
        NEW_AFFECT_VOTE_BONUS = 508,

        NEW_AFFECT_MALL = 510,
        // �� ������ ����Ʈ
        NEW_AFFECT_NO_DEATH_PENALTY = 511,
        // ����� ��ȣ (����ġ �г�Ƽ�� �ѹ� �����ش�)
        NEW_AFFECT_SKILL_BOOK_BONUS = 512,
        // ������ ���� (å ���� ���� Ȯ���� 50% ����)
        NEW_AFFECT_SKILL_BOOK_NO_DELAY = 513,
        // �־� ���� (å ���� ������ ����)

        NEW_AFFECT_EXP_BONUS_EURO_FREE = 516,
        // ������ ���� (���� ���� 14 ���� ���� �⺻ ȿ��)
        NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 = 517,

        NEW_AFFECT_AUTO_HP_RECOVERY = 534,
        // �ڵ����� HP
        NEW_AFFECT_AUTO_SP_RECOVERY = 535,
        // �ڵ����� SP

        NEW_AFFECT_DRAGON_SOUL_QUALIFIED = 540,
        NEW_AFFECT_DRAGON_SOUL_DECK1 = 541,
        NEW_AFFECT_DRAGON_SOUL_DECK2 = 542,
        NEW_AFFECT_EXP_CURSE = 543,

        NEW_AFFECT_RAMADAN_ABILITY = 300,
        NEW_AFFECT_RAMADAN_RING = 301,
        // �󸶴� �̺�Ʈ�� Ư�������� �ʽ´��� ���� ���� ����

        NEW_AFFECT_NOG_POCKET_ABILITY = 302,

        NEW_AFFECT_QUEST_START_IDX = 1000,
        NEW_AFFECT_DECAY = 228
    };

    enum
    {
        STONE_SMOKE1 = 0,
        // 99%
        STONE_SMOKE2 = 1,
        // 85%
        STONE_SMOKE3 = 2,
        // 80%
        STONE_SMOKE4 = 3,
        // 60%
        STONE_SMOKE5 = 4,
        // 45%
        STONE_SMOKE6 = 5,
        // 40%
        STONE_SMOKE7 = 6,
        // 20%
        STONE_SMOKE8 = 7,
        // 10%
        STONE_SMOKE_NUM = 4,
    };

    enum EBuildingAffect
    {
        BUILDING_CONSTRUCTION_SMALL = 0,
        BUILDING_CONSTRUCTION_LARGE = 1,
        BUILDING_UPGRADE = 2,
    };

    enum
    {
        WEAPON_DUALHAND,
        WEAPON_ONEHAND,
        WEAPON_TWOHAND,
        WEAPON_NUM,
    };

    enum
    {
        NAMECOLOR_MOB,
        NAMECOLOR_STONE,
        NAMECOLOR_NPC,
        NAMECOLOR_PC,
        NAMECOLOR_PC_END = NAMECOLOR_PC + EMPIRE_MAX_NUM,
        NAMECOLOR_NORMAL_MOB,
        NAMECOLOR_NORMAL_NPC,
        NAMECOLOR_NORMAL_PC,
        NAMECOLOR_NORMAL_PC_END = NAMECOLOR_NORMAL_PC + EMPIRE_MAX_NUM,
        NAMECOLOR_EMPIRE_MOB,
        NAMECOLOR_EMPIRE_NPC,
        NAMECOLOR_EMPIRE_PC,
        NAMECOLOR_EMPIRE_PC_END = NAMECOLOR_EMPIRE_PC + EMPIRE_MAX_NUM,
        NAMECOLOR_DEAD,
        NAMECOLOR_FUNC,
        NAMECOLOR_PK,
        NAMECOLOR_PVP,
        NAMECOLOR_PARTY,
        NAMECOLOR_WARP,
        NAMECOLOR_WAYPOINT,
#ifdef __OFFLINE_SHOP__
        NAMECOLOR_OFFLINE_SHOP,
#endif
        NAMECOLOR_BUFFBOT,
        NAMECOLOR_TARGET_VICTIM,
        NAMECOLOR_EXTRA = NAMECOLOR_FUNC + 10,
        NAMECOLOR_NUM = NAMECOLOR_EXTRA + 10,
    };

    enum
    {
        ALIGNMENT_TYPE_WHITE,
        ALIGNMENT_TYPE_NORMAL,
        ALIGNMENT_TYPE_DARK,
    };

    enum DamageFlag
    {
        DAMAGE_NORMAL = (1 << 0),
        DAMAGE_POISON = (1 << 1),
        DAMAGE_DODGE = (1 << 2),
        DAMAGE_BLOCK = (1 << 3),
        DAMAGE_PENETRATE= (1 << 4),
        DAMAGE_CRITICAL = (1 << 5),
        DAMAGE_BLEEDING = (1 << 6),
        // ��-_-��
    };

    enum
    {
        DUEL_NONE,
        DUEL_CANNOTATTACK,
        DUEL_START,
    };

public:


    static void SetDustGap(float fDustGap);
    static void SetHorseDustGap(float fDustGap);

    static void SetEmpireNameMode(bool isEnable);
    static const DirectX::SimpleMath::Color &GetIndexedNameColor(UINT eNameColor);

public:
    void SetMainInstance();

    void OnSelected();
    void OnUnselected();
    void OnTargeted();
    void OnUntargeted();
    void SetAlwaysRenderAttachingEffect();
    void SetTitlePreview(const std::string &title, uint32_t color);
    void SetTitle(const std::string &title, uint32_t color);
    bool HasEffect(uint32_t dwEffect);

protected:
    static bool __IsExistMainInstance();
    bool __IsMainInstance();
    bool __MainCanSeeHiddenThing() const;
    float __GetBowRange();

    static float ms_fDustGap;
    static float ms_fHorseDustGap;

public:
    CInstanceBase();
    virtual ~CInstanceBase();

    bool LessRenderOrder(CInstanceBase *pkInst);

    void MountHorse(UINT eRace);
    void DismountHorse();

    float CalculateDistanceSq3d(const TPixelPosition &c_rkPPosDst);

    // Instance Data
    bool IsFlyTargetObject();
    void ClearFlyTargetInstance();
    void SetFlyTargetInstance(CInstanceBase &rkInstDst);
    void AddFlyTargetInstance(CInstanceBase &rkInstDst);
    void AddFlyTargetPosition(const TPixelPosition &c_rkPPosDst);

    float GetFlyTargetDistance();

    void SetAlpha(float fAlpha);

    void DeleteBlendOut();

    void AttachTextTail();
    void DetachTextTail();
    void UpdateTextTailLevel(uint32_t level);

    void RefreshTextTail();
    void RefreshTextTailTitle();

    bool Create(const SCreateData &c_rkCreateData);

    bool CreateDeviceObjects();
    void DestroyDeviceObjects();

    void UpdateHideStatus(bool isType, bool isHide);
    void UpdateOptions();
    void Update();
    bool UpdateDeleting();

    void Transform();
    void Deform();
    void Render();
    void RenderTrace();
    void RenderAttachingEffect();
    void RenderToShadowMap(bool showWeapon = true);
    void RenderCollision();
    void RegisterBoundingSphere();

    // Temporary
    void GetBoundBox(Vector3 *vtMin, Vector3 *vtMax);

    void SetNameString(const char *c_szName, int len);
    bool SetRace(uint32_t dwRaceIndex);
    void SetVirtualID(uint32_t wVirtualNumber);
    void SetVirtualNumber(uint32_t dwVirtualNumber);
    void SetInstanceType(int iInstanceType);
    void SetAlignment(PlayerAlignment sAlignment);

    void SetLevel(uint32_t level);
#ifdef ENABLE_PLAYTIME_ICON
    void SetPlayTime(DWORD dwPlayTime);
    DWORD GetPlayTime() const;
    const char* GetMyTimeIcon() const;
#endif
    void SetPKMode(uint8_t byPKMode);
    void SetKiller(bool bFlag);
    void SetPartyMemberFlag(bool bFlag);
    void SetStateFlags(uint32_t dwStateFlags);
    void SetImmuneFlags(uint32_t immuneFlags);

    bool SetAcce(const Part &eAcce);
    void SetArmor(const Part &dwArmor);
    void SetShape(uint32_t eShape, float fSpecular = 0.0f);
    void RefreshShape();
    void SetHair(const Part &eHair);
    bool SetWeapon(const Part &part);
    bool SetArrow(const Part &eArrow);
    void SetPrefixNum(const Part &part);

    bool SetWeaponEffect(const Part &eArrow);
    bool SetArmorEffect(const Part &eArrow);
    bool SetWingEffect(const Part &eArrow);

    bool ChangeArmor(const Part &dwArmor);
    void ChangeWeapon(const Part &eWeapon);
    void ChangeScale(uint16_t scale);
    void RefreshWeapon();
    void RefreshAcce();
    void ChangeArmorEffect(const Part &part);
    void ChangeWingEffect(const Part &part);
    void ChangeWeaponEffect(const Part &part);
    void ChangeArrow(const Part &eArrow);
    void ChangeAcce(const Part &eAcce);
    void ChangeHair(const Part &eHair);
    void ChangeGuild(uint32_t dwGuildID);
    uint32_t GetWeaponType() const;
    void __HideArmorShiningEffect() const;
    bool IsShowingArmorShiningEffect() const;
    void __ShowArmorShiningEffect() const;
    void __HideWeaponShiningEffect() const;
    bool IsShowingWeaponShiningEffect() const;
    void __ShowWeaponShiningEffect() const;
    void __HideWeaponRefineEffect();
    bool IsShowingWeaponRefineEffect() const;
    void __ShowWeaponRefineEffect();

    void SetOwnerVid(uint32_t vid)
    {
        m_ownerVid = vid;
    }

    uint32_t GetOwnerVid() const
    {
        return m_ownerVid;
    }

    void SetComboType(UINT uComboType);
    void SetAttackSpeed(UINT uAtkSpd);
    void SetMoveSpeed(UINT uMovSpd);
    void SetRotationSpeed(float fRotSpd);

    // Guild Leader/General Information
    void SetGuildLeader(uint8_t isGuildLeader);
    bool IsGuildLeader() const;

    void SetGuildGeneral(uint8_t val);
    bool IsGuildGeneral() const;

    const char *GetNameString() const;
    const std::string &GetName() const;
    int GetInstanceType() const;
    const Part &GetPart(EParts part) const;
    uint32_t GetShape() const;
    uint32_t GetRace() const;
    uint32_t GetVirtualID();
    uint32_t GetVirtualNumber() const;
    uint32_t GetEmpireID() const;
    uint32_t GetGuildID() const;
#ifdef WJ_SHOW_MOB_INFO
    uint32_t GetAIFlag() const;
#endif
    int GetAlignment() const;
    UINT GetAlignmentGrade() const;
    int GetAlignmentType() const;
    uint8_t GetPKMode() const;
    bool IsKiller() const;
    bool IsPartyMember() const;

    int GetLevel() const
    {
        return m_dwLevel;
    }

    void ActDualEmotion(CInstanceBase &rkDstInst, uint16_t dwMotionNumber1, uint16_t dwMotionNumber2);
    void ActEmotion(uint32_t dwMotionNumber);
    void LevelUp();
    void SkillUp();
    void UseSpinTop();
    void Revive();
    void Stun();
    void Die();
    void Hide();
    void Show();

    bool CanAct();
    bool CanMove();
    bool CanAttack();
    bool CanUseSkill();
    bool CanFishing();
    bool IsConflictAlignmentInstance(CInstanceBase &rkInstVictim) const;
    bool IsAttackableInstance(CInstanceBase &rkInstVictim);
    bool IsTargetableInstance(CInstanceBase &rkInstVictim) const;
    bool IsPVPInstance(CInstanceBase &rkInstVictim);
    bool CanChangeTarget();
    bool CanPickInstance(bool enableTextTail);
    bool CanViewTargetHP(CInstanceBase &rkInstVictim) const;

    // Movement
    bool IsGoing();
    bool NEW_Goto(const TPixelPosition &c_rkPPosDst, float fDstRot);
    void EndGoing();

    void SetRunMode();
    void SetWalkMode();
    // Affects
    void ClearAffects();

    void AddAffect(const AffectData &affect);
    void RemoveAffect(uint32_t type, uint32_t pointType);

    bool HasAffect(uint32_t type, uint32_t pointType = POINT_NONE);

    bool IsInvisibility();
    bool IsParalysis();
    void SetParalysis(bool val);
    bool IsGameMaster() const;

    const AffectsHolder &GetAffects() const
    {
        return m_affects;
    }

    bool IsSameEmpire(CInstanceBase &rkInstDst) const;
    bool IsBowMode();
    bool IsHandMode();
    bool IsFishingMode();
    bool IsMining();
    bool IsFishing();

    bool IsWearingDress() const;
    bool IsHoldingPickAxe() const;
    bool IsMountingHorse() const;
    bool IsNewMount() const;
    bool IsForceVisible();
    bool IsInSafe();
    bool IsEnemy();
    bool IsStone();
    bool IsResource() const;
    bool IsNPC();
    bool IsPet();
    bool IsAttackPet();
    bool IsMount();
    bool IsBuffBot();
    bool IsPC();
    bool IsPoly();
    bool IsWarp();
    bool IsGoto();
    bool IsObject();
    bool IsShop();
    bool IsDoor();
    bool IsBuilding();
    bool IsWoodenDoor();
    bool IsStoneDoor();
    bool IsFlag() const;
    bool IsGuildWall() const;

    bool IsDead();
    bool IsStun();
    bool IsSleep();
    bool __IsSyncing();
    bool IsWaiting();
    bool IsWalking();
    bool IsPushing();
    bool IsAttacking();
    bool IsActingEmotion();
    bool IsAttacked();
    bool IsKnockDown();
    bool IsUsingSkill();
    bool IsUsingMovingSkill();
    bool CanCancelSkill();
    bool CanAttackHorseLevel();

#ifdef __MOVIE_MODE__
		bool					IsMovieMode(); // ��ڿ� ������ �Ⱥ��̴°�
#endif
    bool NEW_CanMoveToDestPixelPosition(const TPixelPosition &c_rkPPosDst);

    void NEW_SetAdvancingRotationFromPixelPosition(const TPixelPosition &c_rkPPosSrc,
                                                   const TPixelPosition &c_rkPPosDst);
    void NEW_SetAdvancingRotationFromDirPixelPosition(const TPixelPosition &c_rkPPosDir);
    bool NEW_SetAdvancingRotationFromDestPixelPosition(const TPixelPosition &c_rkPPosDst);
    void SetAdvancingRotation(float fRotation);

    void EndWalking(float fBlendingTime = 0.15f);
    void EndWalkingWithoutBlending();

    // Battle
    void SetEventHandler(CActorInstance::IEventHandler *pkEventHandler);

    void PushTCPState(uint32_t dwCmdTime, const TPixelPosition &c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg,
                      uint32_t color);
    void PushTCPStateExpanded(uint32_t dwCmdTime, const TPixelPosition &c_rkPPosDst, float fDstRot, UINT eFunc,
                              UINT uArg,
                              UINT uTargetVID);
    void PushTCPState(const SNetworkMoveActorData& moveData, const TPixelPosition& c_rkPPosDst);

    void NEW_Stop();

    bool NEW_UseSkill(UINT uSkill, UINT uMot, UINT uMotLoopCount, bool isMovingSkill, uint32_t color);
    void NEW_Attack();
    void NEW_Attack(float fDirRot);
    void NEW_AttackToDestPixelPositionDirection(const TPixelPosition &c_rkPPosDst);
    bool NEW_AttackToDestInstanceDirection(CInstanceBase &rkInstDst, IFlyEventHandler *pkFlyHandler);
    bool NEW_AttackToDestInstanceDirection(CInstanceBase &rkInstDst);

    bool NEW_MoveToDestPixelPositionDirection(const TPixelPosition &c_rkPPosDst);
    void NEW_MoveToDestInstanceDirection(CInstanceBase &rkInstDst);
    void NEW_MoveToDirection(float fDirRot);

    float NEW_GetDistanceFromDirPixelPosition(const TPixelPosition &c_rkPPosDir);
    float NEW_GetDistanceFromDestPixelPosition(const TPixelPosition &c_rkPPosDst);
    float NEW_GetDistanceFromDestInstance(CInstanceBase &rkInstDst);

    float NEW_GetRotation();
    float NEW_GetRotationFromDestPixelPosition(const TPixelPosition &c_rkPPosDst);
    float NEW_GetRotationFromDirPixelPosition(const TPixelPosition &c_rkPPosDir);
    float NEW_GetRotationFromDestInstance(CInstanceBase &rkInstDst);

    float NEW_GetAdvancingRotationFromDirPixelPosition(const TPixelPosition &c_rkPPosDir);
    float NEW_GetAdvancingRotationFromDestPixelPosition(const TPixelPosition &c_rkPPosDst);
    float NEW_GetAdvancingRotationFromPixelPosition(const TPixelPosition &c_rkPPosSrc,
                                                    const TPixelPosition &c_rkPPosDst);

    bool NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition &c_rkPPosDst);
    bool NEW_IsClickableDistanceDestInstance(CInstanceBase &rkInstDst);

    bool NEW_GetFrontInstance(CInstanceBase **ppoutTargetInstance, float fDistance);
    void NEW_GetRandomPositionInFanRange(CInstanceBase &rkInstTarget, TPixelPosition *pkPPosDst);
    bool NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase &rkInstTarget,
                                         std::vector<CInstanceBase *> *pkVct_pkInst);
    bool NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase *> *pkVct_pkInst);

    void SetSyncOwner(uint32_t dwOwnerVID);
    void Warp(float x, float y);
    void NEW_SyncPixelPosition(long &nPPosX, long &nPPosY);
    void NEW_SyncCurrentPixelPosition();

    void NEW_SetPixelPosition(const TPixelPosition &c_rkPPosDst);

    bool NEW_IsLastPixelPosition();
    const TPixelPosition &NEW_GetLastPixelPositionRef();

    // Battle
    bool isNormalAttacking();
    bool isComboAttacking();
    uint32_t GetNormalAttackIndex();
    uint32_t GetComboIndex();
    float GetAttackingElapsedTime();
    void InputNormalAttack(float fAtkDirRot);
    void InputComboAttack(float fAtkDirRot);

    void RunNormalAttack(float fAtkDirRot);
    void RunComboAttack(float fAtkDirRot, uint32_t wMotionIndex);

    CInstanceBase *FindNearestVictim();
    bool CheckAdvancing();

    bool AvoidObject(const CGraphicObjectInstance &c_rkBGObj);
    bool IsBlockObject(const CGraphicObjectInstance &c_rkBGObj);
    void BlockMovement();

public:
    bool CheckAttacking(CInstanceBase &rkInstVictim);
    void ProcessHitting(uint32_t dwMotionKey, CInstanceBase *pVictimInstance);
    void ProcessHitting(uint32_t dwMotionKey, uint8_t byEventIndex, CInstanceBase *pVictimInstance);
    void GetBlendingPosition(TPixelPosition *pPixelPosition);
    void SetBlendingPosition(const TPixelPosition &c_rPixelPosition);

    // Fishing
    void StartFishing(float frot);
    void StopFishing();
    void ReactFishing();
    void CatchSuccess();
    void CatchFail();
    bool GetFishingRot(int *pirot);

    // Render Mode
    void RestoreRenderMode();
    void SetAddRenderMode();
    void SetModulateRenderMode();
    void SetRenderMode(int iRenderMode);
    void SetAddColor(const DirectX::SimpleMath::Color &c_rColor);

    // Position
    void SCRIPT_SetPixelPosition(float fx, float fy);
    void NEW_GetPixelPosition(TPixelPosition *pPixelPosition);

    // Rotation
    void NEW_LookAtFlyTarget();
    void NEW_LookAtDestInstance(CInstanceBase &rkInstDst);
    void NEW_LookAtDestPixelPosition(const TPixelPosition &c_rkPPosDst);

    float GetRotation();
    float GetAdvancingRotation();
    void SetRotation(float fRotation);
    void SetRank(unsigned char rank);
    void SetScale(float fScale);
    void BlendRotation(float fRotation, float fBlendTime = 0.1f);

    void SetDirection(int dir);
    void BlendDirection(int dir, float blendTime);
    float GetDegreeFromDirection(int dir);

    // Motion
    //	Motion Deque
    bool isLock();

    void SetMotionMode(int iMotionMode);
    int GetMotionMode(uint32_t dwMotionIndex);

    // Motion
    //	Pushing Motion
    void ResetLocalTime();
    void SetLoopMotion(uint16_t wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
    void PushOnceMotion(uint16_t wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
    void PushLoopMotion(uint16_t wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
    void SetEndStopMotion();

    // Intersect
    bool IntersectDefendingSphere();
    bool IntersectBoundingBox();

    bool CanSkipCollision() const
    {
        return m_GraphicThingInstance.CanSkipCollision();
    }

    void EnableSkipCollision()
    {
        m_GraphicThingInstance.EnableSkipCollision();
    }

    void DisableSkipCollision()
    {
        m_GraphicThingInstance.DisableSkipCollision();
    }

    // Part
    void Refresh(uint32_t dwMotIndex, bool isLoop);

    //void					AttachEffectByID(uint32_t dwParentPartIndex, const char * c_pszBoneName, uint32_t dwEffectID, int dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // ������ ms�����Դϴ�.
    //void					AttachEffectByName(uint32_t dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName, int dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // ������ ms�����Դϴ�.

    float GetDistance(CInstanceBase *pkTargetInst);
    float GetDistance(const TPixelPosition &c_rPixelPosition);

    // ETC
    CActorInstance &GetGraphicThingInstanceRef();
    CActorInstance *GetGraphicThingInstancePtr();

    static bool __Background_IsWaterPixelPosition(const TPixelPosition &c_rkPPos);
    static bool __Background_GetWaterHeight(const TPixelPosition &c_rkPPos, float *pfHeight);

    void SetEmoticon(UINT eEmoticon);
    void SetFishEmoticon();
    bool IsPossibleEmoticon();

    uint32_t CreateEffect(uint32_t effect, uint32_t color = 0, float scale = 1.0f,
                          uint32_t effectKind = EFFECT_KIND_NONE);
    uint32_t CreateEffectWithPositionAndRotation(uint32_t effect, Vector3 &pos, Vector3 &, uint32_t color = 0,
                                                 float scale = 1.0f, uint32_t effectKind = EFFECT_KIND_NONE);
    uint32_t AttachEffect(uint32_t effect, uint32_t color = 0, float scale = 1.0f,
                          uint32_t effectKind = EFFECT_KIND_NONE);

    uint32_t AttachAffectEffect(uint32_t affectType, uint32_t color = 0);

    uint32_t AttachEffect(const CharacterEffectInfo &info,
                          uint32_t color = 0, float scale = 1.0f, uint32_t effectKind = EFFECT_KIND_ATTACHING);
    void DetachEffect(uint32_t index);
    void HideEffect(uint32_t index);
    void ShowEffect(uint32_t index);
    void AttacAcceEffect();

protected:
    void __InitializeRotationSpeed();

    void __Create_SetName(const SCreateData &c_rkCreateData);
    void __Create_SetWarpName(const SCreateData &c_rkCreateData);

    static CInstanceBase *__GetMainInstancePtr();
    static CInstanceBase *__FindInstancePtr(uint32_t dwVID);

    static bool __FindRaceType(uint32_t dwRace, uint8_t *pbType);
    uint32_t __GetRaceType() const;

    bool __IsShapeAnimalWear() const;
    bool __IsChangableWeapon(int iWeaponID) const;

    static void __ClearMainInstance();

    bool __CanProcessNetworkStatePacket();

    bool __IsInDustRange();

    // Emotion
    void __ProcessFunctionEmotion(uint32_t dwMotionNumber, uint32_t dwTargetVID, const TPixelPosition &c_rkPosDst);
    void __EnableChangingTCPState();
    void __DisableChangingTCPState();
    bool __IsEnableTCPProcess(UINT eCurFunc);

    // 2004.07.17.levites.isShow�� ViewFrustumCheck�� ����
    bool __CanRender();
    bool __IsInViewFrustum() const;
    void __RefreshRefineEffect(CItemData *pItem);

#ifdef ENABLE_SHINING_SYSTEM

    void __GetShiningEffect(CItemData *pItem);

    void __AttachArmorCostumeShiningEffect(int persistent, const char *effectFileName, const char *boneName = "Bip01",
                                           float scale = 1.0f);
    void __AttachWeaponCostumeEffectShiningEffect(int persistent, const char *effectFileName,
                                                  const char *boneName = "Bip01", float scale = 1.0f);
    void __AttachArmorCostumeEffectShiningEffect(int persistent, const char *effectFileName,
                                                 const char *boneName = "Bip01", float scale = 1.0f);
    void __AttachWingEffect(int persistent, const char *effectFileName, const char *boneName = "Bip01",
                            float scale = 1.0f);
    void __ClearWeaponShiningEffect(bool detaching = true);
    void __ClearWeaponCostumeShiningEffect();
    void __ClearWeaponCostumeEffectShiningEffect();

    void __ClearArmorShiningEffect();
    void __ClearArmorCostumeShiningEffect();
    void __ClearArmorCostumeEffectShiningEffect();
    void __ClearWingShining();

    void __AttachWeaponShiningEffect(int persistent, const char *effectFileName,
                                     const char *boneName = "Bip01");

    void __AttachArmorShiningEffect(int persistent, const char *effectFileName,
                                    const char *boneName = "Bip01", float scale = 1.0f);
    void __AttachWeaponCostumeShiningEffect(int persistent, const char *effectFileName, const char *boneName);

#endif
    void __ClearWeaponRefineEffect();
    void __ClearArmorRefineEffect();
    void __ClearAcceRefineEffect();

    // HORSE
    void __AttachHorseSaddle();
    void __DetachHorseSaddle();

    struct SHORSE
    {
        bool m_isMounting{};
        CActorInstance *m_pkActor{};

        SHORSE();
        ~SHORSE();

        void Destroy();
        void Create(const TPixelPosition &c_rkPPos, UINT eRace, UINT eHitEffect);

        void SetAttackSpeed(UINT uAtkSpd) const;
        void SetMoveSpeed(UINT uMovSpd) const;
        void Deform() const;
        void Render() const;
        CActorInstance &GetActorRef() const;
        CActorInstance *GetActorPtr() const;

        bool IsMounting() const;
        bool CanAttack() const;
        static bool CanUseSkill();

        static UINT GetLevel();
        bool IsNewMount() const;

        void __Initialize();
    };

    SHORSE m_kHorse;
public:
    const SHORSE& GetHorseInfo() const { return m_kHorse; }

protected:
    // Blend Mode
    void __SetBlendRenderingMode();
    void __SetAlphaValue(float fAlpha);
    float __GetAlphaValue();

    void __ComboProcess();
    void MovementProcess();
    void TodoProcess();
    void StateProcess();
    void AttackProcess();

    void StartWalking();
    float GetLocalTime();

    void RefreshState(uint32_t dwMotIndex, bool isLoop);

protected:
    void OnSyncing();
    void OnWaiting();
    void OnMoving();

    void NEW_SetCurPixelPosition(const TPixelPosition &c_rkPPosDst);
    void NEW_SetSrcPixelPosition(const TPixelPosition &c_rkPPosDst);
    void NEW_SetDstPixelPosition(const TPixelPosition &c_rkPPosDst);
    void NEW_SetDstPixelPositionZ(FLOAT z);

    const TPixelPosition &NEW_GetCurPixelPositionRef();
    const TPixelPosition &NEW_GetSrcPixelPositionRef();

public:
    const TPixelPosition &NEW_GetDstPixelPositionRef();

    uint8_t GetLanguageCode()
    {
        return m_langCode;
    }

protected:
    bool m_isTextTail = false;

    // Instance Data
    std::string m_stName;
    std::string m_title;
    uint32_t m_titleColor = 0;

    std::string m_titlePreview;
    uint32_t m_titleColorPreview = 0;

    Part m_awPart[PART_MAX_NUM] = {};
    bool m_hasWeapon = false;

    uint32_t m_dwLevel = 0;
#if defined(WJ_SHOW_MOB_INFO)
    uint32_t m_dwAIFlag = 0;
#endif
    uint32_t m_dwEmpireID = 0;
    uint32_t m_dwGuildID = 0;
    uint8_t m_langCode;
    uint32_t m_ownerVid = 0;
    bool m_appendedOwnerName = false;
private:
    // Ordered by affect type
    AffectsHolder m_affects;

    // type -> effect index
    std::unordered_map<uint32_t, uint32_t> m_affectEffects;

    void SetAffectActive(uint32_t type, bool activate,
                         uint32_t color = 0);
    void __Warrior_SetGeomgyeongAffect(bool isVisible, uint32_t color);
    void __Assassin_SetEunhyeongAffect(bool isVisible);

protected:
    void __AttachSelectEffect();
    void __DetachSelectEffect();

    void __AttachTargetEffect();
    void __DetachTargetEffect();

    void __AttachEmpireEffect(uint32_t eEmpire);
protected:
    std::unordered_map<uint32_t, uint32_t> m_attachedEffects;

    void __EffectContainer_Initialize();
    void __EffectContainer_Destroy();

    uint32_t __EffectContainer_AttachEffect(uint32_t id);
    void __EffectContainer_DeactiveteEffect(uint32_t id);
    void __EffectContainer_ActiveEffect(uint32_t id);
    void __EffectContainer_DetachEffect(uint32_t id);

protected:
    uint32_t m_stoneSmokeIndex = 0;

    void __StoneSmoke_Inialize();
    void __StoneSmoke_Destroy();
    void __StoneSmoke_Create(uint32_t id);

protected:
    uint8_t m_eType;
    uint8_t m_eRaceType;
    uint8_t m_isGuildLeader = 0;
    uint8_t m_isGuildGeneral = 0;

    uint32_t m_eShape;
    uint32_t m_dwRace;
#ifdef ENABLE_PLAYTIME_ICON
    DWORD m_dwPlayTime;
#endif
    uint32_t m_stateFlags;
    uint32_t m_immuneFlags;
    uint32_t m_dwVirtualNumber{};
    uint32_t m_prefixNum = 0;

    PlayerAlignment m_sAlignment{};
    uint8_t m_byPKMode;
    bool m_isKiller;
    bool m_isPartyMember;

    // Movement
    int m_iRotatingDirection;

    uint32_t m_dwAdvActorVID;
    uint32_t m_dwLastDmgActorVID;

    LONG m_nAverageNetworkGap{};
    uint32_t m_dwNextUpdateHeightTime;

    bool m_isGoing;

    TPixelPosition m_kPPosDust;

    uint32_t m_dwLastComboIndex;

    uint32_t m_swordRefineEffectRight;
    uint32_t m_swordRefineEffectLeft;
    uint32_t m_armorRefineEffect;
    uint32_t m_acceRefineEffect = 0;

    struct SMoveAfterFunc
    {
        UINT eFunc;
        UINT uArg;
        uint32_t color = 0;
        uint8_t loopCount = 0;
        uint8_t isMovingSkill = 0;

        // For Emotion Function
        UINT uArgExpanded;
        TPixelPosition kPosDst;
    };

    SMoveAfterFunc m_kMovAfterFunc;

    float m_fDstRot;
    float m_fAtkPosTime;
    float m_fRotSpd;
    float m_fMaxRotSpd;
    bool m_bEnableTCPState = TRUE;

#ifdef ENABLE_SHINING_SYSTEM

    //2-Dimensions for Left & Right sided effects

    uint32_t m_weaponShiningEffects[2]{};
    uint32_t m_armorShiningEffects{};
    uint32_t m_weaponCostumeShiningEffects[2]{};
    uint32_t m_armorCostumeShiningEffects{};
    uint32_t m_weaponCostumeEffectShiningEffects[2]{};
    uint32_t m_armorCostumeEffectShiningEffects{};
    uint32_t m_wingCostumeShiningEffect{};

#endif

    uint32_t m_gmEffect = 0;
    // Graphic Instance
    CActorInstance m_GraphicThingInstance;

protected:
    struct SCommand
    {
        uint32_t m_dwChkTime;
        uint32_t m_dwCmdTime;
        float m_fDstRot;
        UINT m_eFunc;
        UINT m_uArg;
        UINT m_uTargetVID;
        uint32_t color;
        uint8_t isMovingSkill;
        uint8_t loopCount;

        TPixelPosition m_kPPosDst;
    };

    typedef std::list<SCommand> CommandQueue;

    uint32_t m_dwBaseChkTime{};
    uint32_t m_dwBaseCmdTime{};

    uint32_t m_dwSkipTime{};

    CommandQueue m_kQue_kCmdNew;

    bool m_bDamageEffectType = false;

    struct SEffectDamage
    {
        uint32_t damage;
        uint8_t flag;
        bool bSelf;
        bool bTarget;
    };

    typedef std::list<SEffectDamage> CommandDamageQueue;
    CommandDamageQueue m_DamageQueue;

    void ProcessDamage();

public:
    void AddDamageEffect(uint32_t damage, uint8_t flag, bool bSelf, bool bTarget);

public:
    static void ClearPVPKeySystem();

    static void InsertPVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);
    static void InsertPVPReadyKey(uint32_t dwSrcVID, uint32_t dwDstVID);
    static void RemovePVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);

    static void InsertGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID);
    static void RemoveGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID);

    static void InsertDUELKey(uint32_t dwSrcVID, uint32_t dwDstVID);

    UINT GetNameColorIndex();

    const DirectX::SimpleMath::Color &GetNameColor();
    const DirectX::SimpleMath::Color &GetTitleColor();

protected:
    static uint32_t __GetPVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);
    static bool __FindPVPKey(uint32_t dwSrcVID, uint32_t dwDstVID);
    static bool __FindPVPReadyKey(uint32_t dwSrcVID, uint32_t dwDstVID);
    static bool __FindGVGKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);
    static bool __FindDUELKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);

protected:
    CActorInstance::IEventHandler *GetEventHandlerPtr();
    CActorInstance::IEventHandler &GetEventHandlerRef();

protected:
    static float __GetBackgroundHeight(float x, float y);
    static uint32_t __GetShadowMapColor(float x, float y);

public:
    static void ResetPerformanceCounter();

protected:
    static uint32_t ms_dwUpdateCounter;
    static uint32_t ms_dwRenderCounter;
    static uint32_t ms_dwDeformCounter;

public:
    uint32_t GetDuelMode() const;
    void SetDuelMode(uint32_t type);
protected:
    uint32_t m_dwDuelMode = DUEL_NONE;
    uint32_t m_dwEmoticonTime = 0;
protected:
    bool m_IsAlwaysRender;
public:
    bool IsAlwaysRender() const;
    void SetAlwaysRender(bool val);
};

#endif /* METIN2_CLIENT_MAIN_INSTANCEBASE_H */
