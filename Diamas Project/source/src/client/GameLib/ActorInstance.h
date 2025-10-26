#ifndef METIN2_CLIENT_GAMELIB_ACTORINSTANCE_H
#define METIN2_CLIENT_GAMELIB_ACTORINSTANCE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "FlyTarget.h"
#include "RaceData.h"
#include "RaceMotionData.h"
#include "PhysicsObject.h"
#include "Interface.h"
//#include "../eterGrnLib/ThingInstance.h"
#include <memory>
#include <game/Constants.hpp>
#include "../eterGrnLib/ThingInstance.h"

class CItemData;
class CWeaponTrace;
class IFlyEventHandler;
class CSpeedTreeWrapper;

class IMobProto : public CSingleton<IMobProto>
{
public:
    IMobProto()
    {
    }

    virtual ~IMobProto()
    {
    }

    virtual bool FindRaceType(UINT eRace, UINT *puType);
};

enum ActorFlags
{
    // Actors cannot be pushed (during attack, ...)
    kActorFlagCannotPush = (1 << 0),
};

class CActorInstance : public CGraphicThingInstance, public IFlyTargetableObject
{
public:
    class IEventHandler
    {
    public:
        static IEventHandler *GetEmptyPtr();

    public:
        struct SState
        {
            TPixelPosition kPPosSelf;
            FLOAT fAdvRotSelf;
        };

    public:
        IEventHandler()
        {
        }

        virtual ~IEventHandler()
        {
        }

        virtual void OnSyncing(const SState &c_rkState) = 0;
        virtual void OnWaiting(const SState &c_rkState) = 0;
        virtual void OnMoving(const SState &c_rkState) = 0;
        virtual void OnMove(const SState &c_rkState) = 0;
        virtual void OnStop(const SState &c_rkState) = 0;
        virtual void OnWarp(const SState &c_rkState) = 0;

        virtual void OnAttack(const SState &c_rkState, uint16_t wMotionIndex) = 0;
        virtual void OnUseSkill(const SState &c_rkState, UINT uMotSkill, UINT uLoopCount, bool isMovingSkill, uint32_t color) = 0;

        virtual void OnHit(uint32_t motionKey, UINT uSkill, CActorInstance &rkActorVictim,  const Vector3& pushDest, bool isSendPacket) = 0;

        virtual void OnChangeShape() = 0;
    };

    // 2004.07.05.myevan.�ý�ź�� �ʿ� ���̴� �����ذ�

public:
    static bool IsDirLine();

public:

    enum EType
    {
        TYPE_ENEMY,
        TYPE_NPC,
        TYPE_STONE,
        TYPE_WARP,
        TYPE_DOOR,
        TYPE_BUILDING,
        TYPE_PC,
        TYPE_POLY,
        TYPE_HORSE,
        TYPE_GOTO,
        TYPE_SHOP,
        TYPE_PET,
        TYPE_MOUNT,
        TYPE_GROWTH_PET,
        TYPE_BUFFBOT,
        TYPE_ATTACK_PET,

        TYPE_OBJECT,
        // Only For Client
    };

    enum ERenderMode
    {
        RENDER_MODE_NORMAL,
        RENDER_MODE_BLEND,
        RENDER_MODE_ADD,
        RENDER_MODE_MODULATE,
        RENDER_MODE_DIFFUSE_ONLY,
    };

    /////////////////////////////////////////////////////////////////////////////////////
    // Motion Queueing System
    enum EMotionPushType
    {
        MOTION_TYPE_NONE,
        MOTION_TYPE_ONCE,
        MOTION_TYPE_LOOP,
    };

    typedef struct SReservingMotionNode
    {
        EMotionPushType iMotionType;

        float fStartTime;
        float fBlendTime;
        float fDuration;
        float fSpeedRatio;

        uint32_t dwMotionKey;
    } TReservingMotionNode;

    struct SCurrentMotionNode
    {
        EMotionPushType iMotionType;
        uint32_t dwMotionKey;

        uint32_t dwcurFrame;
        uint32_t dwFrameCount;

        float fStartTime;
        float fEndTime;
        float fSpeedRatio;

        int iLoopCount;
        UINT uSkill;
        uint32_t addColor;
    };

    typedef std::deque<TReservingMotionNode> TMotionDeque;
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // Motion Event
    typedef struct SMotionEventInstance
    {
        int iType;
        int iMotionEventIndex;
        float fStartingTime;

        const CRaceMotionData::TMotionEventData *c_pMotionData;
    } TMotionEventInstance;

    typedef std::list<TMotionEventInstance> TMotionEventInstanceList;
    typedef TMotionEventInstanceList::iterator TMotionEventInstanceListIterator;
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // For Collision Detection
    typedef struct SCollisionPointInstance
    {
        const NRaceData::TCollisionData *c_pCollisionData;
        bool isAttached;
        uint32_t dwModelIndex;
        uint32_t dwBoneIndex;
        CDynamicSphereInstanceVector SphereInstanceVector;
    } TCollisionPointInstance;

    typedef std::list<TCollisionPointInstance> TCollisionPointInstanceList;
    typedef TCollisionPointInstanceList::iterator TCollisionPointInstanceListIterator;

    typedef std::map<CActorInstance *, float> THittedInstanceMap;
    typedef std::map<const NRaceData::THitData *, THittedInstanceMap> THitDataMap;

    struct SSplashArea
    {
        bool isEnableHitProcess;
        UINT uSkill;
        uint32_t MotionKey;
        float fDisappearingTime;
        const CRaceMotionData::TMotionAttackingEventData *c_pAttackingEvent;
        CDynamicSphereInstanceVector SphereInstanceVector;

        THittedInstanceMap HittedInstanceMap;
    };

    typedef struct SHittingData
    {
        uint8_t byAttackingType;
        uint32_t dwMotionKey;
        uint8_t byEventIndex;
    } THittingData;

    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // For Attaching
    enum EAttachEffect
    {
        EFFECT_LIFE_NORMAL,
        EFFECT_LIFE_INFINITE,
        EFFECT_LIFE_WITH_MOTION,
    };

    struct TAttachingEffect
    {
        uint32_t dwEffectId;
        uint32_t dwEffectIndex;
        int iBoneIndex;
        uint32_t dwModelIndex;
        Matrix matTranslation;
        bool isAttaching;

        int iLifeType;
        uint32_t dwEndTime;
    };

    /////////////////////////////////////////////////////////////////////////////////////

public:
    static void ShowDirectionLine(bool isVisible);
    static void DestroySystem();

public:
    CActorInstance();
    virtual ~CActorInstance();

    // 20041201.myevan.�ν��Ͻ����̽��� �Լ�
    void Transform();
    void Deform();

    void Destroy();

    void Move();
    void Stop(float fBlendingTime = 0.15f);

    void SetMainInstance();

    void SetParalysis(bool isParalysis);
    void SetFaint(bool isFaint);
    void SetSleep(bool isSleep);
    void SetResistFallen(bool isResistFallen);

    void SetAttackSpeed(float fAtkSpd);
    void SetMoveSpeed(float fMovSpd);

    void SetMaterialAlpha(uint32_t dwAlpha);
    void SetMaterialColor(uint32_t dwColor);

    void SetEventHandler(IEventHandler *pkEventHandler);

    bool SetRace(uint32_t eRace);
    void SetHair(uint32_t eHair);
    void SetVirtualID(uint32_t dwVID);

    void SetShape(uint32_t eShape, float fSpecular = 0.0f);
    void ChangeMaterial(const char *c_szFileName);

public:
    void SetComboType(uint16_t wComboType);

    uint32_t GetRace();
    uint32_t GetVirtualID();

    UINT GetActorType() const;
    void SetActorType(UINT eType);
    void SetActorAiFlags(uint32_t flags);

    bool CanAct();
    bool CanMove();
    bool CanAttack();
    bool CanUseSkill();

    bool IsPC();
    bool IsNPC();
    bool IsBuffBot();
    bool IsEnemy();
    bool IsStone();
    bool IsWarp();
    bool IsGoto();
    bool IsObject();
    bool IsShop();
    bool IsPet();
    bool IsAttackPet();
    bool IsMount();
    bool IsGrowthPet();
    uint32_t GetRank();
    void SetRank(uint8_t rank);
    bool IsDoor();
    bool IsPoly();

    bool IsBuilding();

    bool IsHandMode();
    bool IsBowMode();
    bool IsTwoHandMode();

    void AttachWeapon(CItemData *itemData);
    void AttachWeapon(uint32_t dwPartIndex, CItemData *pItemData);
    void AttachAcce(CItemData *itemData);

    // Attach Effect
    bool GetAttachingBoneName(uint32_t dwPartIndex, const char **c_szBoneName);
    void UpdateAttachingInstances();
    void RenderAttachingEffect();
    void SetAlwaysRenderAttachingEffect();
    bool IsShowMinorAttachingEffect();
    void DettachEffect(uint32_t dwEID);
    uint32_t AttachEffectByName(uint32_t dwParentPartIndex,
                                const char *c_pszBoneName,
                                const char *c_pszEffectFileName,
                                uint32_t addColor = 0, float scale = 1.0f,
                                uint32_t effectKind = EFFECT_KIND_NONE);
    uint32_t AttachEffectByID(uint32_t dwParentPartIndex,
                              const char *c_pszBoneName,
                              uint32_t dwEffectID,
                              Vector3 const *c_pv3Position = NULL,
                              uint32_t addColor = 0,
                              float scale = 1.0f,
                              uint32_t effectKind = EFFECT_KIND_NONE);
    void RefreshCollisionAttachments();
    uint32_t AttachSmokeEffect(uint32_t eSmoke);

    /////////////////////////////////////////////////////////////////////////////////////
    // Motion Queueing System
    void SetMotionMode(int iMotionMode); // FIXME : ����� �ð��� ������ �����ϰԲ� �Ѵ�.
    int GetMotionMode();
    void SetLoopMotion(uint32_t dwMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
    bool InterceptOnceMotion(uint32_t dwMotion, float fBlendTime = 0.1f, UINT uSkill = 0, float fSpeedRatio = 1.0f,
                             uint32_t addColor = 0);
    bool InterceptLoopMotion(uint32_t dwMotion, float fBlendTime = 0.1f);
    bool PushOnceMotion(uint32_t dwMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
    // FIXME : ����� �ð��� ������ �����ϰԲ� �Ѵ�.
    bool PushLoopMotion(uint32_t dwMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
    // FIXME : ����� �ð��� ������ �����ϰԲ� �Ѵ�.
    void SetMotionLoopCount(int iCount);

    bool IsPushing();

    bool isLock();
    bool IsUsingSkill();
    bool IsMining();
    bool CanCheckAttacking();
    bool CanCancelSkill();
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // Collison Detection
    bool CreateCollisionInstancePiece(uint32_t dwAttachingModelIndex, const NRaceData::TAttachingData *c_pAttachingData,
                                      TCollisionPointInstance *pPointInstance);

    void UpdatePointInstance();
    void UpdatePointInstance(TCollisionPointInstance *pPointInstance);
    bool CheckCollisionDetection(const CDynamicSphereInstanceVector *c_pAttackingSphereVector,
                                 Vector3 *pv3Position);

    // Collision Detection Checking
    virtual bool TestCollisionWithDynamicSphere(const CDynamicSphereInstance &dsi);

    void UpdateAdvancingPointInstance();

    bool IsClickableDistanceDestInstance(CActorInstance &rkInstDst, float fDistance);

    bool AvoidObject(const CGraphicObjectInstance &c_rkBGObj);
    bool IsBlockObject(const CGraphicObjectInstance &c_rkBGObj);
    void BlockMovement();
    /////////////////////////////////////////////////////////////////////////////////////

protected:
    bool __TestObjectCollision(const CGraphicObjectInstance *c_pObjectInstance);

public:
    bool TestActorCollision(CActorInstance &rVictim);
    bool TestPhysicsBlendingCollision(CActorInstance &rVictim);

    bool AttackingProcess(CActorInstance &rVictim);

    void PreAttack();
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // Battle
    // Input
    // ������ �ű� ���ɼ��� �ִ� �ڵ��
    // ��Ʈ�� ������ ���� ������ �÷��̾ �����ϰ� �ܼ��� Showing Type�̱� ������
    // ���� �˻簡 �ʿ� ����.
    void InputNormalAttackCommand(float fDirRot); // Process input - Only used by player's character
    bool InputComboAttackCommand(float fDirRot);  // Process input - Only used by player's character

    // Command
    bool isAttacking();
    bool isNormalAttacking();
    bool isComboAttacking();
    bool IsSplashAttacking();
    bool IsUsingMovingSkill();
    bool IsActEmotion();
    uint32_t GetComboIndex();
    float GetAttackingElapsedTime();
    void SetBlendingPosition(const TPixelPosition &c_rPosition, float fBlendingTime = 1.0f);
    void ResetBlendingPosition();
    void GetBlendingPosition(TPixelPosition *pPosition);

    bool NormalAttack(float fDirRot, float fBlendTime = 0.1f);
    bool ComboAttack(uint32_t wMotionIndex, float fDirRot, float fBlendTime = 0.1f);

    void Revive();

    bool IsSleep();
    bool IsParalysis();
    bool IsFaint();
    bool IsResistFallen();
    bool IsWaiting();
    bool IsMoving();
    bool IsDead();
    bool IsStun();
    bool IsAttacked();
    bool IsDamage();
    bool IsKnockDown();
    bool IsEmotionMotion();
    void SetWalkMode();
    void SetRunMode();
    void Stun();
    void Die();
    void DieEnd();

    void SetBattleHitEffect(uint32_t dwID);
    void SetBattleAttachEffect(uint32_t dwID);

    uint32_t GetNormalAttackIndex();
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // Position
    const Vector3 &GetMovementVectorRef();
    const Vector3 &GetPositionVectorRef();

    void SetCurPixelPosition(const TPixelPosition &c_rkPPosCur);
    void NEW_SetAtkPixelPosition(const TPixelPosition &c_rkPPosAtk);
    void NEW_SetSrcPixelPosition(const TPixelPosition &c_rkPPosSrc);
    void NEW_SetDstPixelPosition(const TPixelPosition &c_rkPPosDst);
    void NEW_SetDstPixelPositionZ(float z);

    const TPixelPosition &NEW_GetAtkPixelPositionRef();
    const TPixelPosition &NEW_GetCurPixelPositionRef();
    const TPixelPosition &NEW_GetSrcPixelPositionRef();
    const TPixelPosition &NEW_GetDstPixelPositionRef();

    const TPixelPosition &NEW_GetLastPixelPositionRef();

    void GetPixelPosition(TPixelPosition *pPixelPosition);
    void SetPixelPosition(const TPixelPosition &c_rPixelPos);

    // Rotation Command
    void LookAt(float fDirRot);
    void LookAt(float fx, float fy);
    void LookAt(CActorInstance *pInstance);
    void LookWith(CActorInstance *pInstance);
    void LookAtFromXY(float x, float y, CActorInstance *pDestInstance);

    void SetReachScale(float fScale);
    void SetOwner(uint32_t dwOwnerVID);

    float GetRotation();
    float GetTargetRotation();

    float GetAdvancingRotation();

    float GetRotatingTime();
    void SetRotation(float fRot);
    void SetXYRotation(float fRotX, float fRotY);
    void BlendRotation(float fRot, float fBlendTime);
    void SetAdvancingRotation(float fRot);
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    void MotionEventProcess();
    void MotionEventProcess(uint32_t dwcurTime, int iIndex, const CRaceMotionData::TMotionEventData *c_pData);
    void SoundEventProcess(bool bCheckFrequency);
    /////////////////////////////////////////////////////////////////////////////////////

    ////
    // Rendering Functions - Temporary Place
    bool IsMovement();

    void RestoreRenderMode();

    void BeginDiffuseRender();
    void EndDiffuseRender();
    void BeginOpacityRender();
    void EndOpacityRender();

    void BeginBlendRender();
    void EndBlendRender();
    void SetBlendRenderMode();
    void SetAlphaValue(float fAlpha);
    float GetAlphaValue();
    void BlendAlphaValue(float fDstAlpha, float fDuration);
    void SetDiffuseRenderOnly();
    void SetNormalRenderMode();
    void SetSpecularInfo(bool bEnable, int iPart, float fAlpha);
    void SetSpecularInfoForce(bool bEnable, int iPart, float fAlpha);

    void SetAddColor(const DirectX::SimpleMath::Color &c_rColor);

    void SetRenderMode(int iRenderMode);

    void RenderTrace();
    void RenderCollisionData();
    void RenderToShadowMap(bool showWeapon = true);

protected:
    void __AdjustCollisionMovement(const CGraphicObjectInstance *c_pGraphicObjectInstance);

public:
    void AdjustDynamicCollisionMovement(const CActorInstance *c_pActorInstance);

    // Weapon Trace
    void SetWeaponTraceTexture(const char *szTextureName);
    void UseTextureWeaponTrace();
    void UseAlphaWeaponTrace();

    // ETC
    void UpdateAttribute();
    bool IntersectDefendingSphere();
    float GetHeight();
    void ShowAllAttachingEffect();
    void HideAllAttachingEffect();
    void HideEffect(uint32_t index);
    void ShowEffect(uint32_t index);
    void ShowMinorAttachingEffect();
    void HideMinorAttachingEffect();
    void ClearAttachingEffect();

    // Fishing
    bool CanFishing();
    bool IsFishing();
    void SetFishingPosition(Vector3 &rv3Position);

    // Flying Methods
    // As a Flying Target
public:
    virtual Vector3 OnGetFlyTargetPosition();

    void OnShootDamage();

    // As a Shooter
    // NOTE : target and target position are exclusive
public:
    void ClearFlyTarget();
    bool IsFlyTargetObject();
    void AddFlyTarget(const CFlyTarget &cr_FlyTarget);
    void SetFlyTarget(const CFlyTarget &cr_FlyTarget);
    void LookAtFlyTarget();

    float GetFlyTargetDistance();

    void ClearFlyEventHandler();
    void SetFlyEventHandler(IFlyEventHandler *pHandler);

    // 2004. 07. 07. [levites] - ��ų ����� Ÿ���� �ٲ�� ���� �ذ��� ���� �ڵ�
    bool CanChangeTarget();

protected:
    IFlyEventHandler *m_pFlyEventHandler;

public:
    void MountHorse(CActorInstance *pkHorse);
    void HORSE_MotionProcess(bool isPC);
    void MotionProcess(bool isPC);
    void RotationProcess();
    void PhysicsProcess();
    void ComboProcess();
    void TransformMovement();
    void AccumulationMovement();
    void ShakeProcess();
    void TraceProcess();
    void __MotionEventProcess(bool isPC);
    void __AccumulationMovement(float fRot);
    bool __SplashAttackProcess(CActorInstance &rVictim);
    bool __NormalAttackProcess(CActorInstance &rVictim);
    bool __CanInputNormalAttackCommand();
#ifdef ENABLE_FLY_MOUNT
    void UcanAdam(float z);
#endif
    private:
    void __Shake(uint32_t dwDuration);

protected:
    CFlyTarget m_kFlyTarget;
    CFlyTarget m_kBackupFlyTarget;
    std::deque<CFlyTarget> m_kQue_kFlyTarget;

protected:
    bool __IsInSplashTime();

    void OnUpdate();
    void OnRender();

    bool isValidAttacking();

    void ReservingMotionProcess();
    void CurrentMotionProcess();
    uint32_t GetRandomMotionKey(uint32_t dwMotionKey);

    float GetLastMotionTime(float fBlendTime); // NOTE : �ڵ����� BlendTime��ŭ�� �մ�� �ð��� ����
    float GetMotionDuration(uint32_t dwMotionKey);

    bool InterceptMotion(EMotionPushType iMotionType, uint16_t wMotion,
                         float fBlendTime = 0.1f, UINT uSkill = 0,
                         float fSpeedRatio = 1.0f, uint32_t addColor = 0);
    void PushMotion(EMotionPushType iMotionType, uint32_t dwMotionKey, float fBlendTime, float fSpeedRatio = 1.0f);
    void ProcessMotionEventEffectEvent(const CRaceMotionData::TMotionEventData *c_pData);
    void ProcessMotionEventEffectToTargetEvent(const CRaceMotionData::TMotionEventData *c_pData);
    void ProcessMotionEventSpecialAttacking(int iMotionEventIndex, const CRaceMotionData::TMotionEventData *c_pData);
    void ProcessMotionEventSound(const CRaceMotionData::TMotionEventData *c_pData);
    void ProcessMotionEventFly(const CRaceMotionData::TMotionEventData *c_pData);
    void ProcessMotionEventWarp(const CRaceMotionData::TMotionEventData *c_pData);
    void ProcessMotionEventRelativeMoveOn(const CRaceMotionData::TMotionEventData *c_pData);
    void ProcessMotionEventRelativeMoveOff(const CRaceMotionData::TMotionEventData *c_pData);

    void AddMovement(float fx, float fy, float fz);

    bool __IsLeftHandWeapon(uint32_t type) const;
    bool __IsRightHandWeapon(uint32_t type) const;
    bool __IsWeaponTrace(uint32_t weaponType) const;

protected:
    void __InitializeMovement();

protected:
    void __Initialize();

    void __ClearAttachingEffect();

#ifdef ENHANCED_FLY_FIX
    float __GetOwnerTime() const;
#endif
    uint32_t __GetOwnerVID() const;
    bool __CanPushDestActor(CActorInstance &rkActorDst);

protected:
    void __RunNextCombo();
    void __ClearCombo();
    void __OnEndCombo();

    void __ProcessDataAttackSuccess(uint32_t motionKey, const NRaceData::TAttackData &c_rAttackData,
                                    CActorInstance &rVictim, const Vector3 &c_rv3Position, UINT uiSkill = 0,
                                    bool isSendPacket = TRUE);
    void __ProcessMotionEventAttackSuccess(uint32_t dwMotionKey, uint8_t byEventIndex, CActorInstance &rVictim);
    void __ProcessMotionAttackSuccess(uint32_t dwMotionKey, CActorInstance &rVictim);

    void __HitStone(CActorInstance &rVictim);
    void __HitGood(CActorInstance &rVictim);
    void __HitGreate(CActorInstance &rVictim);

    void __PushDirect(CActorInstance &rVictim);
    void __PushCircle(CActorInstance &rVictim);
    bool __isInvisible();
    void __SetFallingDirection(float fx, float fy);

protected:
    struct SSetMotionData
    {
        uint32_t dwMotKey;
        float fSpeedRatio;
        float fBlendTime;
        int iLoopCount;
        UINT uSkill;

        SSetMotionData()
        {
            iLoopCount = 0;
            dwMotKey = 0;
            fSpeedRatio = 1.0f;
            fBlendTime = 0.0f;
            uSkill = 0;
        }
    };

protected:
    float __GetAttackSpeed();
    uint32_t __SetMotion(const SSetMotionData &c_rkSetMotData, uint32_t dwRandMotKey = 0); // ��� ������ ����
    void __ClearMotion();

    bool __BindMotionData(uint32_t dwMotionKey); // ��� �����͸� ���ε�
    void __ClearHittedActorInstanceMap();        // ������ ���� �ν��Ͻ� ���� �����

    UINT __GetMotionType(); // ��� Ÿ�� ���

    bool __IsNeedFlyTargetMotion(); // FlyTarget �� �ʿ��� ����ΰ�?
    bool __HasMotionFlyEvent();     // ���𰡸� ��°�?
    bool __IsWaitMotion();          // ��� ��� �ΰ�?
    bool __IsMoveMotion();          // �̵� ��� �ΰ�?
    bool __IsAttackMotion();        // ���� ��� �ΰ�?
    bool __IsComboAttackMotion();   // �޺� ���� ��� �ΰ�?
    bool __IsDamageMotion();        // ������ ����ΰ�?
    bool __IsKnockDownMotion();     // �˴ٿ� ����ΰ�?
    bool __IsEmotionMotion();
    bool __IsEventMotion();
    bool __IsDieMotion();     // ��� ��� �ΰ�?
    bool __IsStandUpMotion(); // �Ͼ�� ����ΰ�?
    bool __IsMountingHorse() const;
    CActorInstance *GetHorseActor();

    bool __CanAttack();          // ���� �Ҽ� �ִ°�?
    bool __CanNextComboAttack(); // ���� �޺� ������ �����Ѱ�?

    bool __IsComboAttacking();  // �޺� �������ΰ�?
    void __CancelComboAttack(); // �޺� ���� ���

    uint16_t __GetCurrentMotionIndex();
    uint32_t __GetCurrentMotionKey();

    int __GetLoopCount();
    uint16_t __GetCurrentComboType();

    void __ShowEvent();
    void __HideEvent();
    bool __IsHiding();
    bool __IsMovingSkill(uint16_t wSkillNumber);

    float __GetReachScale();

    void __CreateAttributeInstance(CAttributeData::Ptr pData);

    bool __IsFlyTargetPC();
    bool __IsSameFlyTarget(CActorInstance *pInstance);
    Vector3 __GetFlyTargetPosition();

protected:
    void __DestroyWeaponTrace(); // ���� �ܻ��� �����Ѵ�
    void __ShowWeaponTrace();    // ���� �ܻ��� ���δ�
    void __HideWeaponTrace();    // ���� �ܻ��� �����

protected:
    // collision data
    void OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector);
    void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance);
    bool OnGetObjectHeight(float fX, float fY, float *pfHeight);

protected:
    /////////////////////////////////////////////////////////////////////////////////////
    // Motion Queueing System
    TMotionDeque m_MotionDeque;
    SCurrentMotionNode m_kCurMotNode;
    uint16_t m_wcurMotionMode;
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // For Collision Detection
    TCollisionPointInstanceList m_BodyPointInstanceList;
    TCollisionPointInstanceList m_DefendingPointInstanceList;
    SSplashArea m_kSplashArea; // TODO : ������ ���� ������ �ؾ��Ѵ� - [levites]
    CAttributeInstance *m_pAttributeInstance;
    /////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // For Battle System
    std::vector<CWeaponTrace *> m_WeaponTraceVector;
    CPhysicsObject m_PhysicsObject;

    uint32_t m_dwcurComboIndex;

    uint32_t m_eActorType;

    uint32_t m_eRace;
    uint32_t m_eShape;
    uint32_t m_eHair;
    uint32_t m_eRank;
    uint32_t m_eAiFlag;

    bool m_isPreInput;
    bool m_isNextPreInput;
    uint32_t m_dwcurComboBackMotionIndex;

    uint16_t m_wcurComboType;

    float m_fAtkDirRot;

    CRaceData *m_pkCurRaceData;
    CRaceMotionData *m_pkCurRaceMotionData;

    // Defender
    float m_fInvisibleTime;
    bool m_isHiding;

    // TODO : State�� ���� ��ų �� �ִ��� ������ ����
    bool m_isResistFallen = 0;
    bool m_isSleep;
    bool m_isFaint;
    bool m_isParalysis;
    bool m_isStun;
    bool m_isRealDead;
    bool m_isWalking;
    bool m_isMain;

    // Effect
    uint32_t m_dwBattleHitEffectID;
    uint32_t m_dwBattleAttachEffectID;
    /////////////////////////////////////////////////////////////////////////////////////

    // Fishing
    Vector3 m_v3FishingPosition;
    int m_iFishingEffectID;

    // Position
    float m_x;
    float m_y;
    float m_z;
    Vector3 m_v3Pos;
    Vector3 m_v3Movement;
    float m_moveFactor;
    bool m_bNeedUpdateCollision;

    uint32_t m_dwShakeTime;

    float m_fReachScale;
    float m_fMovSpd;
    float m_fAtkSpd;
#ifdef ENABLE_FLY_MOUNT
    float ucanAdam_z;
#endif
    // Rotation
    float m_fcurRotation;
    float m_rotBegin;
    float m_rotEnd;
    float m_rotEndTime;
    float m_rotBeginTime;
    float m_rotBlendTime;
    float m_fAdvancingRotation;
    float m_rotX;
    float m_rotY;

    DWORD m_fOwnerBaseTime;

    // Rendering
    int m_iRenderMode;
    DirectX::SimpleMath::Color m_AddColor;
    float m_fAlphaValue;

    // Attached Effect
    std::list<TAttachingEffect> m_AttachingEffectList;
    bool m_bEffectInitialized;

    // material color
    uint32_t m_dwMtrlColor;
    uint32_t m_dwMtrlAlpha;

    TPixelPosition m_kPPosCur;
    TPixelPosition m_kPPosSrc;
    TPixelPosition m_kPPosDst;
    TPixelPosition m_kPPosAtk;

    TPixelPosition m_kPPosLast;

    THitDataMap m_HitDataMap;

    CActorInstance *m_pkHorse;
    CSpeedTreeWrapper *m_pkTree;
public:
    bool m_isUsingQuiver;
    uint32_t m_quiverEffectID;

protected:
    uint32_t m_dwSelfVID;
    uint32_t m_dwOwnerVID;

protected:
    void __InitializeStateData();
    void __InitializeMotionData();
    void __InitializeRotationData();
    void __InitializePositionData();

public: // InstanceBase ������ �ӽ÷� public
    IEventHandler *__GetEventHandlerPtr();
    IEventHandler &__GetEventHandlerRef();

    void __OnSyncing();
    void __OnWaiting();
    void __OnMoving();
    void __OnMove();
    void __OnStop();
    void __OnWarp();
    void __OnAttack(uint16_t wMotionIndex);
    void __OnUseSkill(UINT uMotSkill, UINT uLoopCount, bool isMoving, uint32_t color = 0);

protected:
    void __OnHit(uint32_t motionKey, UINT uSkill, CActorInstance &rkInstVictm, const Vector3& pushDest, bool isSendPacket);

public:
    void EnableSkipCollision();
    void DisableSkipCollision();
    bool CanSkipCollision() const;

protected:
    void __InitializeCollisionData();

    uint32_t m_canSkipCollision;

protected:
    struct SBlendAlpha
    {
        float m_fBaseTime;
        float m_fBaseAlpha;
        float m_fDuration;
        float m_fDstAlpha;

        uint32_t m_iOldRenderMode;
        bool m_isBlending;
    } m_kBlendAlpha;

    void __BlendAlpha_Initialize();
    void __BlendAlpha_Apply(float fDstAlpha, float fDuration);
    void __BlendAlpha_Update();
    void __BlendAlpha_UpdateFadeIn();
    void __BlendAlpha_UpdateFadeOut();
    void __BlendAlpha_UpdateComplete();
    float __BlendAlpha_GetElapsedTime();

    void __Push(int x, int y);

public:
    void TEMP_Push(int x, int y);
    bool __IsSyncing();

    void __CreateTree(const char *c_szFileName);
    void __DestroyTree();
    void __SetTreePosition(float fx, float fy, float fz);

    bool HasFlag(uint32_t flag) const;
    void SetFlag(uint32_t flag, bool value = true);

protected:
    IEventHandler *m_pkEventHandler;
    uint32_t m_flags = 0;

protected:
    static bool ms_isDirLine;
};
#endif /* METIN2_CLIENT_GAMELIB_ACTORINSTANCE_H */
