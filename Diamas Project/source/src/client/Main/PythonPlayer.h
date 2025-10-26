#ifndef METIN2_CLIENT_MAIN_PYTHONPLAYER_H
#define METIN2_CLIENT_MAIN_PYTHONPLAYER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/GamePacket.hpp>
#include "PythonSkill.h"
#include <game/PageGrid.hpp>
#include <game/SkillConstants.hpp>
#include <game/Types.hpp>

class CInstanceBase;

enum
{
    REFINE_CANT,
    REFINE_OK,
    REFINE_ALREADY_MAX_SOCKET_COUNT,
    REFINE_NEED_MORE_GOOD_SCROLL,
    REFINE_CANT_MAKE_SOCKET_ITEM,
    REFINE_NOT_NEXT_GRADE_ITEM,
    REFINE_CANT_REFINE_METIN_TO_EQUIPMENT,
    REFINE_CANT_REFINE_ROD,
    REFINE_CANT_DD_ITEM,
    REFINE_CANT_NO_DD_ITEM,
};

enum
{
    ATTACH_METIN_CANT,
    ATTACH_METIN_OK,
    ATTACH_METIN_NOT_MATCHABLE_ITEM,
    ATTACH_METIN_NO_MATCHABLE_SOCKET,
    ATTACH_METIN_NOT_EXIST_GOLD_SOCKET,
    ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT,
    ATTACH_METIN_SEALED,
};

enum
{
    DETACH_METIN_CANT,
    DETACH_METIN_OK,
};

enum
{
    REFINE_SCROLL_TYPE_MAKE_SOCKET = 1,
    REFINE_SCROLL_TYPE_UP_GRADE = 2,
    REFINE_SCROLL_TYPE_YAGHAN_STONE = 7,
    REFINE_SCROLL_TYPE_JOHEUN_STONE = 8,
    REFINE_SCROLL_TYPE_KEUN_STONE = 9,
};

/*
 *	���� ĳ���� (�ڽ��� �����ϴ� ĳ����) �� ���� �������� �����Ѵ�.
 *
 * 2003-01-12 Levites	������ CPythonCharacter�� ������ �־����� �Ը� �ʹ� Ŀ�� ��������
 *						��ġ�� �ָ��ؼ� ���� �и�
 * 2003-07-19 Levites	���� ĳ������ �̵� ó�� CharacterInstance���� ����� ����
 *						������ ����Ÿ ������ ���ҿ��� �Ϻ��� ���� �÷��̾� ���� Ŭ������
 *						Ż�ٲ� ��.
 */

using ClientItemVector = std::vector<ClientItemData>;

class CPythonPlayer : public CSingleton<CPythonPlayer>
{
public:
    enum
    {
        CATEGORY_NONE = 0,
        CATEGORY_ACTIVE = 1,
        CATEGORY_PASSIVE = 2,
        CATEGORY_MAX_NUM = 3,

        STATUS_INDEX_ST = 1,
        STATUS_INDEX_DX = 2,
        STATUS_INDEX_IQ = 3,
        STATUS_INDEX_HT = 4,
    };

    enum
    {
        MBT_LEFT,
        MBT_RIGHT,
        MBT_MIDDLE,
        MBT_NUM,
    };

    enum
    {
        MBF_SMART,
        MBF_MOVE,
        MBF_CAMERA,
        MBF_ATTACK,
        MBF_SKILL,
        MBF_AUTO,
    };

    enum
    {
        MBS_CLICK,
        MBS_PRESS,
    };

    enum EMode
    {
        MODE_NONE,
        MODE_CLICK_POSITION,
        MODE_CLICK_ITEM,
        MODE_CLICK_ACTOR,
        MODE_USE_SKILL,
    };

    enum EEffect
    {
        EFFECT_PICK,
        EFFECT_NUM,
    };

    enum EMetinSocketType
    {
        METIN_SOCKET_TYPE_NONE,
        METIN_SOCKET_TYPE_SILVER,
        METIN_SOCKET_TYPE_GOLD,
    };

    typedef struct SSkillInstance
    {
        uint32_t dwIndex;
        int iType;
        int iGrade;
        int iLevel;
        uint32_t color;

        float fcurEfficientPercentage;
        float fnextEfficientPercentage;
        bool isCoolTime;

        double fCoolTime;     // NOTE : ��Ÿ�� ���� ��ų ������
        double fLastUsedTime; //        ��â�� ����� �� ����ϴ� ����
        bool bActive;
    } TSkillInstance;

    enum EKeyBoard_UD
    {
        KEYBOARD_UD_NONE,
        KEYBOARD_UD_UP,
        KEYBOARD_UD_DOWN,
    };

    enum EKeyBoard_LR
    {
        KEYBOARD_LR_NONE,
        KEYBOARD_LR_LEFT,
        KEYBOARD_LR_RIGHT,
    };

    enum
    {
        DIR_UP,
        DIR_DOWN,
        DIR_LEFT,
        DIR_RIGHT,
    };

    typedef struct SPlayerStatus
    {
        void Clear()
        {
            std::memset(aQuickSlot, 0, sizeof(aQuickSlot));
            std::memset(m_alPoint, 0, sizeof(m_alPoint));
            std::memset(aDSItem, 0, sizeof(aDSItem));

            m_ullGold = 0;
            lQuickPageIndex = 0;
        }

        ClientItemData aDSItem[DRAGON_SOUL_INVENTORY_MAX_NUM];
        TQuickslot aQuickSlot[QUICKSLOT_MAX_NUM];
        PointValue m_alPoint[POINT_MAX_NUM];
        long lQuickPageIndex;
        uint32_t blockMode;

        Gold m_ullGold;

        void SetPoint(UINT ePoint, PointValue lPoint);
        PointValue GetPoint(UINT ePoint) const;

        void SetGold(Gold gold);
        Gold GetGold();

#ifdef ENABLE_GEM_SYSTEM
			int  m_gem;
			void SetGem(int gem);
			int GetGem();
#endif
    } TPlayerStatus;

    struct Windows
    {
        Windows();

        void Clear();

        const PagedGrid<ClientItemData> *Get(uint8_t type) const;
        PagedGrid<ClientItemData> *Get(uint8_t type);

        PagedGrid<ClientItemData> inventory;
        PagedGrid<ClientItemData> equipment;
        PagedGrid<ClientItemData> switchbot;
        PagedGrid<ClientItemData> belt;
    };
    using HuntingMissionMap = std::unordered_map<uint32_t, TPlayerHuntingMission>;

    typedef struct SPartyMemberInfo
    {
        SPartyMemberInfo(uint32_t _dwPID, const char *c_szName)
            : dwPID(_dwPID), strName(c_szName), dwVID(0)
        {
        }

        uint32_t dwVID;
        uint32_t dwPID;
        std::string strName;
        uint8_t byState;
        uint8_t byHPPercentage;
        short sAffects[7];
        uint8_t byIsLeader;
        uint8_t byRace;
    } TPartyMemberInfo;

    enum EPartyRole
    {
        PARTY_ROLE_NORMAL,
        PARTY_ROLE_LEADER,
        PARTY_ROLE_ATTACKER,
        PARTY_ROLE_TANKER,
        PARTY_ROLE_BUFFER,
        PARTY_ROLE_SKILL_MASTER,
        PARTY_ROLE_BERSERKER,
        PARTY_ROLE_DEFENDER,
        PARTY_ROLE_MAX_NUM,
    };

    enum
    {
        SKILL_NORMAL,
        SKILL_MASTER,
        SKILL_GRAND_MASTER,
        SKILL_PERFECT_MASTER,
        SKILL_LEGENDARY_MASTER,
    };

    // �ڵ����� ���� ���� Ưȭ ����ü.. �̷����� Ưȭ ó�� �۾��� �� �Ϸ��� �ִ��� ��������� �����ϰ� �ᱹ Ưȭó��.
    struct SAutoPotionInfo
    {
        SAutoPotionInfo()
            : bActivated(false), totalAmount(0), currentAmount(0)
        {
        }

        bool bActivated;         // Ȱ��ȭ �Ǿ��°�?			
        long currentAmount;      // ���� ���� ��
        long totalAmount;        // ��ü ��
        long inventorySlotIndex; // ������� �������� �κ��丮�� ���� �ε���
    };

    enum EAutoPotionType
    {
        AUTO_POTION_TYPE_HP = 0,
        AUTO_POTION_TYPE_SP = 1,
        AUTO_POTION_TYPE_NUM
    };

    typedef struct SPetSkill
    {
        uint32_t skillId;
        uint32_t skillLevel;
        uint32_t skillCoolTime;
    } TPetSkill;

    typedef struct SPetData
    {
        uint32_t id;
        float hpPct;
        float defPct;
        float mpPct;
        // ???
        int32_t level;
        // ???
        uint32_t exp;
        uint32_t needExp;
        uint32_t itemExp;
        uint32_t needItemExp;
        uint32_t evolLevel;
        uint32_t birthday;
        uint32_t skillCount;
    } TPetData;

public:
    CPythonPlayer();
    ~CPythonPlayer();

    void PickCloseMoney();
    void PickCloseItem();

    void SetGameWindow(py::object ppyObject);

    void SetObserverMode(bool isEnable);
    bool IsObserverMode();
    bool IsGameMaster();
    bool IsFishing();

    //Free camera - [Think]
    void SetFreeCameraMode(bool isEnable);
    bool IsFreeCameraMode();
    //End free camera

    void SetQuickCameraMode(bool isEnable);

    void SetAttackKeyState(bool isPress);

    void NEW_GetMainActorPosition(TPixelPosition *pkPPosActor) const;

    bool RegisterEffect(uint32_t dwEID, const char *c_szEftFileName, bool isCache);

    bool NEW_SetMouseState(int eMBType, int eMBState);
    bool NEW_SetMouseFunc(int eMBType, int eMBFunc);
    int NEW_GetMouseFunc(int eMBT);
    void NEW_SetMouseMiddleButtonState(int eMBState);

    void NEW_SetAutoCameraRotationSpeed(float fRotSpd);
    void NEW_ResetCameraRotation();

    void NEW_SetSingleDirKeyState(int eDirKey, bool isPress);
    void NEW_SetSingleDIKKeyState(int eDIKKey, bool isPress);
    void SetMultiDirKeyState(bool isLeft, bool isRight, bool isUp, bool isDown);

    void NEW_Attack();
    void NEW_Fishing();
    bool NEW_CancelFishing();

    void NEW_LookAtFocusActor();
    bool NEW_IsAttackableDistanceFocusActor();

    bool NEW_MoveToDestPixelPositionDirection(const TPixelPosition &c_rkPPosDst);
    bool NEW_MoveToMousePickedDirection();
    bool NEW_MoveToMouseScreenDirection();
    bool NEW_MoveToDirection(float fDirRot);
    void NEW_Stop();

    // Reserved
    bool NEW_IsEmptyReservedDelayTime(float fElapsedtime); // ���̹� ���� ���� �ʿ� - [levites]

    // Dungeon
    void SetDungeonDestinationPosition(int ix, int iy);
    void AlarmHaveToGo();

    CInstanceBase *NEW_FindActorPtr(uint32_t dwVID) const;
    CInstanceBase *NEW_GetMainActorPtr() const;

    // flying target set
    void Clear();
    void LeaveGamePhase();
    void ClearSkillDict(); // �������ų� ClearGame ������ ���Ե� �Լ�
    void NEW_ClearSkillData(bool bAll = false);

    void Update();

    // Play Time
    uint32_t GetPlayTime();
    void SetPlayTime(uint32_t dwPlayTime);

    // System
    void SetMainCharacterIndex(int iIndex);

    uint32_t GetMainCharacterIndex();
    bool IsMainCharacterIndex(uint32_t dwIndex);
    uint32_t GetGuildID();
    void NotifyDeletingCharacterInstance(uint32_t dwVID);
    void NotifyCharacterDead(uint32_t dwVID);
    void NotifyCharacterUpdate(uint32_t dwVID);
    void NotifyDeadMainCharacter();
    void NotifyChangePKMode() const;

    // Player Status
    const char *GetName() const;
    void SetName(const char *name);

    void SetRace(uint32_t dwRace);
    uint32_t GetRace();

    void SetStatus(uint32_t dwType, PointValue lValue);
    PointValue GetStatus(uint32_t dwType) const;
    void SetTitlePreview(std::string title, uint32_t color);
    void UpdateBattleStatus();

    uint32_t GetBlockMode() const
    {
        return m_playerStatus.blockMode;
    }

    void SetBlockMode(uint32_t blockMode)
    {
        m_playerStatus.blockMode = blockMode;
    }

    void ChangeBlockMode(uint32_t flagToXor);

    void SetGold(Gold value);
    Gold GetGold();

#ifdef ENABLE_GEM_SYSTEM
	int GetGem();
	void SetGem(int value);
	void SetGemShopWindowOpen(bool isEnable) { m_isGemShopWindowOpen = isEnable; }
	bool IsGemShopWindowOpen() const { return m_isGemShopWindowOpen; }
#endif

    void ShowPopUp(std::string message);
    // Item
    bool SealItem(TItemPos Cell);
    bool UnSealItem(TItemPos Cell);

    void MoveItemData(TItemPos SrcCell, TItemPos DstCell);
    void SetItemData(TItemPos Cell, const ClientItemData &data);
    uint32_t GetItemTypeBySlot(TItemPos Cell);
    uint32_t GetItemSubTypeBySlot(TItemPos Cell);
    const ClientItemData *GetItemData(TItemPos Cell) const;
    [[nodiscard]] py::bytes GetItemLink(TItemPos Cell) const;
    void SetItemCount(TItemPos Cell, CountType byCount);
    void SetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex, uint32_t dwMetinNumber);
    void SetItemAttribute(TItemPos Cell, uint32_t dwAttrIndex, uint8_t byType, short sValue);
    uint32_t GetItemIndex(TItemPos Cell) const;
    uint32_t GetItemValue(TItemPos Cell, uint32_t index);
    uint32_t GetItemChangeLookVnum(TItemPos Cell) const;
    uint32_t GetItemFlags(TItemPos Cell);
    uint32_t GetItemAntiFlags(TItemPos Cell) const;
    CountType GetItemCount(TItemPos Cell) const;
    int32_t GetEmptyInventory(uint8_t size, uint8_t type, int32_t except = -1);
    CountType GetItemCountByVnum(uint32_t dwVnum);
    SocketValue GetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex);
    void GetItemAttribute(TItemPos Cell, uint32_t dwAttrSlotIndex, ApplyType *pbyType, ApplyValue *psValue) const;
    TPlayerItemAttribute GetItemAttributeNew(TItemPos Cell, uint32_t index) const;
    [[nodiscard]] std::optional<std::vector<TPlayerItemAttribute>> GetItemAttributes(const TItemPos &Cell) const;

    int32_t CanRefine(uint32_t vnum, TItemPos targetCell);
    int32_t CanAttachMetin(uint32_t vnum, TItemPos targetCell) const;
    int32_t CanDetachMetin(uint32_t vnum, TItemPos targetCell) const;
    int32_t CanSealItem(uint32_t vnum, TItemPos targetCell) const;
    int32_t CanUnlockTreasure(uint32_t vnum, TItemPos targetCell) const;

    void SendClickItemPacket(uint32_t dwIID);

    void RequestAddLocalQuickSlot(uint32_t dwLocalSlotIndex, uint32_t dwWndType, uint32_t dwWndItemPos);
    void RequestAddToEmptyLocalQuickSlot(uint32_t dwWndType, uint32_t dwWndItemPos);
    void RequestMoveGlobalQuickSlotToLocalQuickSlot(uint32_t dwGlobalSrcSlotIndex, uint32_t dwLocalDstSlotIndex);
    void RequestDeleteGlobalQuickSlot(uint32_t dwGlobalSlotIndex);
    void RequestUseLocalQuickSlot(uint32_t dwLocalSlotIndex);
    uint32_t LocalQuickSlotIndexToGlobalQuickSlotIndex(uint32_t dwLocalSlotIndex);

    void GetGlobalQuickSlotData(uint32_t dwGlobalSlotIndex, uint32_t *pdwWndType, uint32_t *pdwWndItemPos);
    void GetLocalQuickSlotData(uint32_t dwSlotPos, uint32_t *pdwWndType, uint32_t *pdwWndItemPos);

    TQuickslot GetLocalQuickSlot(int SlotIndex);
    TQuickslot GetGlobalQuickSlot(int SlotIndex);

    void RemoveQuickSlotByValue(int iType, int iPosition);

    bool IsItem(TItemPos SlotIndex);

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
    bool IsBeltInventorySlot(TItemPos Cell);
#endif
    bool IsInventorySlot(TItemPos SlotIndex);
    bool IsEquipmentSlot(TItemPos SlotIndex);

    bool IsEquipItemInSlot(TItemPos iSlotIndex);

    // ACCE NEW
    void                          SetAcceItemData(uint32_t dwSlotIndex, const ClientItemData & rItemInstance);
    void                          DelAcceItemData(uint32_t dwSlotIndex);
    int                           GetCurrentAcceSize();
    bool                          GetAcceSlotItemID(uint32_t dwSlotIndex, uint32_t * pdwItemID);
    bool                          GetAcceItemDataPtr(uint32_t dwSlotIndex, ClientItemData ** ppInstance);
    std::optional<ClientItemData> GetAcceItemData(uint32_t dwSlotIndex);
    bool                          IsEmtpyAcceWindow();
    int                           GetCurrentAcceItemCount();
    void                          SetAcceRefineWindowOpen(int type);
    void                          SetActivedAcceSlot(int acceSlot, int itemPos);
    int                           FindActivedSlot(int acceSlot);
    int                           FindUsingSlot(int acceSlot);
    int                           FineMoveAcceItemSlot();
    // Quickslot
    int GetQuickPage();
    void SetQuickPage(int nPageIndex);
    void AddQuickSlot(int QuickslotIndex, char IconType, uint16_t IconPosition);
    void DeleteQuickSlot(int QuickslotIndex);
    void MoveQuickSlot(int Source, int Target);

    // Skill
    void SetSkill(uint32_t dwSlotIndex, uint32_t dwSkillIndex);
    bool GetSkillSlotIndex(uint32_t dwSkillIndex, uint32_t *pdwSlotIndex);
    int GetSkillIndex(uint32_t dwSlotIndex);
    std::optional<uint32_t> GetSkillSlotIndexNew(uint32_t dwSkillIndex);
    int GetSkillGrade(uint32_t dwSlotIndex);
    int GetSkillDisplayLevel(uint32_t dwSlotIndex);
    int GetSkillLevel(uint32_t dwSlotIndex);
    uint32_t GetSkillColor(uint32_t slotIndex);
    float GetSkillCurrentEfficientPercentage(uint32_t dwSlotIndex);
    float GetSkillNextEfficientPercentage(uint32_t dwSlotIndex);
    void SetSkillLevel(uint32_t dwSlotIndex, uint32_t dwSkillLevel);
    void SetSkillInfo(uint32_t dwSkillIndex, uint32_t dwSkillGrade, uint32_t dwSkillLevel, uint32_t color);
    bool IsToggleSkill(uint32_t dwSlotIndex);
    void ClickSkillSlot(uint32_t dwSlotIndex);
    void ChangeCurrentSkillNumberOnly(uint32_t dwSlotIndex);

    bool FindSkillSlotIndexBySkillIndex(uint32_t dwSkillIndex, uint32_t *pdwSkillSlotIndex);

    void SetSkillCoolTime(uint32_t dwSkillIndex);
    void EndSkillCoolTime(uint32_t dwSkillIndex);

    double GetSkillCoolTime(uint32_t dwSlotIndex);
    double GetSkillElapsedCoolTime(uint32_t dwSlotIndex);
    bool IsSkillActive(uint32_t dwSlotIndex);
    bool IsSkillCoolTime(uint32_t dwSlotIndex);
    void UseGuildSkill(uint32_t dwSkillSlotIndex);
    void DeactivateSkillSlot(uint32_t dwSlotIndex);
    void ActivateSkillSlot(uint32_t dwSlotIndex);

    // Target
    void SetTarget(uint32_t dwVID, bool bForceChange = TRUE);
    void OpenCharacterMenu(uint32_t dwVictimActorID);
    uint32_t GetTargetVID();

    // Party
    void ExitParty();
    void AppendPartyMember(uint32_t dwPID, const char *c_szName);
    void LinkPartyMember(uint32_t dwPID, uint32_t dwVID);
    void UnlinkPartyMember(uint32_t dwPID);
    void UpdatePartyMemberInfo(uint32_t dwPID, uint8_t byState, uint8_t byHPPercentage, uint8_t byIsLeader, uint8_t byRace);
    void UpdatePartyMemberAffect(uint32_t dwPID, uint8_t byAffectSlotIndex, short sAffectNumber);
    void RemovePartyMember(uint32_t dwPID);
    bool IsPartyMemberByVID(uint32_t dwVID);
    bool IsPartyMemberByName(const char *c_szName);
    bool GetPartyMemberPtr(uint32_t dwPID, TPartyMemberInfo **ppPartyMemberInfo);
    bool PartyMemberPIDToVID(uint32_t dwPID, uint32_t *pdwVID);
    bool PartyMemberVIDToPID(uint32_t dwVID, uint32_t *pdwPID);
    bool IsSamePartyMember(uint32_t dwVID1, uint32_t dwVID2);

    // Fight
    void RememberChallengeInstance(uint32_t dwVID);
    void RememberRevengeInstance(uint32_t dwVID);
    void RememberCantFightInstance(uint32_t dwVID);
    void ForgetInstance(uint32_t dwVID);
    bool IsChallengeInstance(uint32_t dwVID);
    bool IsRevengeInstance(uint32_t dwVID);
    bool IsCantFightInstance(uint32_t dwVID);

    // Private Shop
    void OpenPrivateShop();
    void ClosePrivateShop();
    bool IsOpenPrivateShop();

    // PK Mode
    uint32_t GetPKMode();

    // Combo
    void SetComboSkillFlag(bool bFlag);

    // Emotion
    void ActEmotion(uint32_t dwEmotionID);
    void StartEmotionProcess();
    void EndEmotionProcess();

    // Function Only For Console System
    bool __ToggleCoolTime();
    bool __ToggleLevelLimit();

    __inline const SAutoPotionInfo &GetAutoPotionInfo(int type) const
    {
        return m_kAutoPotionInfo[type];
    }

    __inline SAutoPotionInfo &GetAutoPotionInfo(int type)
    {
        return m_kAutoPotionInfo[type];
    }

    __inline void SetAutoPotionInfo(int type, const SAutoPotionInfo &info)
    {
        m_kAutoPotionInfo[type] = info;
    }

protected:
    TQuickslot &__RefLocalQuickSlot(int SlotIndex);
    TQuickslot &__RefGlobalQuickSlot(int SlotIndex);

    uint32_t __GetLevelAtk();
    uint32_t __GetStatAtk();
    uint32_t __GetWeaponAtk(uint32_t dwWeaponPower);
    uint32_t __GetTotalAtk(uint32_t dwWeaponPower, uint32_t dwRefineBonus);
    uint32_t __GetRaceStat();
    uint32_t __GetHitRate();
    uint32_t __GetEvadeRate();


    void __OnPressSmart(CInstanceBase &rkInstMain, bool isAuto);
    void __OnClickSmart(CInstanceBase &rkInstMain, bool isAuto);

    void __OnPressItem(CInstanceBase &rkInstMain, uint32_t dwPickedItemID);
    void __OnPressActor(CInstanceBase &rkInstMain, uint32_t dwPickedActorID, bool isAuto);
    void __OnPressGround(CInstanceBase &rkInstMain, const TPixelPosition &c_rkPPosPickedGround);
    void __OnPressScreen(CInstanceBase &rkInstMain);

    void __OnClickActor(CInstanceBase &rkInstMain, uint32_t dwPickedActorID, bool isAuto);
    void __OnClickItem(CInstanceBase &rkInstMain, uint32_t dwPickedItemID);
    void __OnClickGround(CInstanceBase &rkInstMain, const TPixelPosition &c_rkPPosPickedGround);

    bool __IsMovableGroundDistance(CInstanceBase &rkInstMain, const TPixelPosition &c_rkPPosPickedGround);

    bool __GetPickedActorPtr(CInstanceBase **pkInstPicked);

    bool __GetPickedActorID(uint32_t *pdwActorID);
    bool __GetPickedItemID(uint32_t *pdwItemID);
    bool __GetPickedGroundPos(TPixelPosition *pkPPosPicked);

    void __ClearReservedAction();
    void __ReserveClickItem(uint32_t dwItemID);
    void __ReserveClickActor(uint32_t dwActorID);
    void __ReserveClickGround(const TPixelPosition &c_rkPPosPickedGround);
    void __ReserveUseSkill(uint32_t dwActorID, uint32_t dwSkillSlotIndex, uint32_t dwRange);

    void __ReserveProcess_ClickActor();

    void __ShowPickedEffect(const TPixelPosition &c_rkPPosPickedGround);
    void __SendClickActorPacket(CInstanceBase &rkInstVictim);

    void __ClearAutoAttackTargetActorID();
    void __SetAutoAttackTargetActorID(uint32_t dwActorID);

    void NEW_ShowEffect(int dwEID, TPixelPosition kPPosDst);

    void NEW_SetMouseSmartState(int eMBS, bool isAuto);
    void NEW_SetMouseMoveState(int eMBS);
    void NEW_SetMouseCameraState(int eMBS);

    bool CheckCategory(int iCategory);
    bool CheckAbilitySlot(int iSlotIndex);

    void RefreshKeyWalkingDirection();
    void NEW_RefreshMouseWalkingDirection();

    // Instances
    void RefreshInstances();

    bool __CanShot(CInstanceBase &rkInstMain, CInstanceBase &rkInstTarget);
    bool __CanUseSkill();

    bool __CanMove();

    bool __CanAttack();
    bool __CanChangeTarget();

    bool __CheckSkillUsable(uint32_t dwSlotIndex);
    void __UseCurrentSkill();
    void __UseChargeSkill(uint32_t dwSkillSlotIndex);
    bool __UseSkill(uint32_t dwSlotIndex);
    bool __CheckSpecialSkill(uint32_t dwSkillIndex);

    bool __CheckRestSkillCoolTime(uint32_t dwSkillSlotIndex);
    bool __CheckShortLife(TSkillInstance &rkSkillInst, CPythonSkill::TSkillData &rkSkillData);
    bool __CheckShortMana(TSkillInstance &rkSkillInst, CPythonSkill::TSkillData &rkSkillData);
    bool __CheckShortArrow(TSkillInstance &rkSkillInst, CPythonSkill::TSkillData &rkSkillData);
    bool __CheckDashAffect(CInstanceBase &rkInstMain);

    void __SendUseSkill(uint32_t dwSkillSlotIndex, uint32_t dwTargetVID);
    void __RunCoolTime(uint32_t dwSkillSlotIndex);

    uint8_t __GetSkillType(uint32_t dwSkillSlotIndex);

    bool __IsReservedUseSkill(uint32_t dwSkillSlotIndex);
    bool __IsMeleeSkill(CPythonSkill::TSkillData &rkSkillData);
    bool __IsChargeSkill(CPythonSkill::TSkillData &rkSkillData);
    uint32_t __GetSkillTargetRange(CPythonSkill::TSkillData &rkSkillData);
    bool __SearchNearTarget();
    bool __IsUsingChargeSkill();

    bool __ProcessEnemySkillTargetRange(CInstanceBase &rkInstMain, CInstanceBase &rkInstTarget,
                                        CPythonSkill::TSkillData &rkSkillData, uint32_t dwSkillSlotIndex);

    // Item
    bool __HasEnoughArrow() const;
    bool __HasItem(uint32_t dwItemID);
    uint32_t __GetPickableDistance();

    // Target
    CInstanceBase *__GetTargetActorPtr();
    void __ClearTarget();
    uint32_t __GetTargetVID() const;
    void __SetTargetVID(uint32_t dwVID);
    bool __IsSameTargetVID(uint32_t dwVID);
    bool __IsTarget();
    bool __ChangeTargetToPickedInstance();

    CInstanceBase *__GetSkillTargetInstancePtr(CPythonSkill::TSkillData &rkSkillData);
    CInstanceBase *__GetAliveTargetInstancePtr();
    CInstanceBase *__GetDeadTargetInstancePtr();

    bool __IsRightButtonSkillMode();

    // Update
    void __Update_AutoAttack();
    void __Update_NotifyGuildAreaEvent();

    // Emotion
    bool __IsProcessingEmotion();
protected:
    py::object m_ppyGameWindow;

    // Client Player Data
    std::unordered_map<uint32_t, uint32_t> m_skillSlotDict;
    std::string m_stName;
    uint32_t m_dwMainCharacterIndex;
    uint32_t m_dwRace;
    uint32_t m_dwWeaponMinPower;
    uint32_t m_dwWeaponMaxPower;
    uint32_t m_dwWeaponMinMagicPower;
    uint32_t m_dwWeaponMaxMagicPower;
    uint32_t m_dwWeaponAddPower;

    // Todo
    uint32_t m_dwSendingTargetVID;
    float m_fTargetUpdateTime;

    // Attack
    uint32_t m_dwAutoAttackTargetVID;

    // NEW_Move
    EMode m_eReservedMode;
    float m_fReservedDelayTime;

    float m_fMovDirRot;

    bool m_isUp;
    bool m_isDown;
    bool m_isLeft;
    bool m_isRight;
    bool m_isAtkKey;
    bool m_isDirKey;
    bool m_isCmrRot;
    bool m_isSmtMov;
    bool m_isDirMov;

    float m_fCmrRotSpd;

    TPlayerStatus m_playerStatus;
    Windows m_windows;
    std::map<uint32_t, TSkillInstance> m_Skill;

    UINT m_iComboOld;
    uint32_t m_dwVIDReserved;
    uint32_t m_dwIIDReserved;

    uint32_t m_dwcurSkillSlotIndex;
    uint32_t m_dwSkillSlotIndexReserved;
    uint32_t m_dwSkillRangeReserved;

    TPixelPosition m_kPPosInstPrev;
    TPixelPosition m_kPPosReserved;

    // Emotion
    bool m_bisProcessingEmotion;

    // Dungeon
    bool m_isDestPosition;
    int m_ixDestPos;
    int m_iyDestPos;
    int m_iLastAlarmTime;

    // Party
    std::unordered_map<uint32_t, TPartyMemberInfo> m_PartyMemberMap;

    // PVP
    std::unordered_set<uint32_t> m_ChallengeInstanceSet;
    std::unordered_set<uint32_t> m_RevengeInstanceSet;
    std::unordered_set<uint32_t> m_CantFightInstanceSet;

    // Private Shop
    bool m_isOpenPrivateShop;

    // Observer mode
    bool m_isObserverMode;

#ifdef ENABLE_GEM_SYSTEM
		bool					m_isGemShopWindowOpen;
#endif

    // Free camera mode - [Think]
    bool m_isFreeCameraMode;

    // Guild
    uint32_t m_inGuildAreaID;

    // Mobile
    bool m_bMobileFlag;

    // System
    bool m_sysIsCoolTime;
    bool m_sysIsLevelLimit;

protected:
    // Game Cursor Data
    TPixelPosition m_MovingCursorPosition;
    float m_fMovingCursorSettingTime;
    uint32_t m_adwEffect[EFFECT_NUM];

    uint32_t m_dwVIDPicked;
    uint32_t m_dwIIDPicked;
    int m_aeMBFButton[MBT_NUM];

    uint32_t m_dwTargetVID;
    uint64_t m_dwTargetEndTime;
    uint32_t m_dwPlayTime;

    SAutoPotionInfo m_kAutoPotionInfo[AUTO_POTION_TYPE_NUM];

protected:
    float MOVABLE_GROUND_DISTANCE;
public:
    void SetPVPTeam(int iTeam)
    {
        m_iPVPTeam = iTeam;
    }

    int GetPVPTeam() const
    {
        return m_iPVPTeam;
    }

private:
    int m_iPVPTeam = -1;

private:
    std::map<uint32_t, uint32_t> m_kMap_dwAffectIndexToSkillIndex;
    ClientItemVector m_ItemAcceInstanceVector;
public:
    uint8_t m_acceRefineWindowIsOpen;
    uint32_t m_acceRefineWindowType;
    signed int m_acceRefineActivedSlot[3];

private:
    uint32_t m_activePetItemVnum;
    uint32_t m_activePetId;
public:
    void ClearActivePet()
    {
        m_activePetItemVnum = 0;
        m_activePetId = 0;
    }

    uint32_t GetActivePetId()
    {
        return m_activePetId;
    }

    void SetActivePetId(uint32_t vnum)
    {
        m_activePetId = vnum;
    }

    uint32_t GetActivePetItemVnum()
    {
        return m_activePetItemVnum;
    }

    void SetActivePetItemVnum(uint32_t vnum)
    {
        m_activePetItemVnum = vnum;
    }

private:
    bool m_isChangeLookWindowOpen = false;
    ClientItemVector m_ItemChangeLookInstanceVector;
public:
    TItemPos m_changeLookActivedSlot[CHANGELOOK_SLOT_MAX];
public:
    bool GetChangeLookWindowOpen()
    {
        return m_isChangeLookWindowOpen;
    }

    void SetChangeLookWindowOpen(bool isChangeLookWindowOpen)
    {
        m_isChangeLookWindowOpen = isChangeLookWindowOpen;
    }

private:
#ifdef ENABLE_MOVE_COSTUME_ATTR
private:
    int32_t m_itemCombinationActivedItemSlot[COMB_WND_SLOT_MAX];
public:
    void SetItemCombinationWindowActivedItemSlot(int32_t index, int32_t value);
    int32_t GetConbWindowSlotByAttachedInvenSlot(int32_t value) const;
    int32_t GetInvenSlotAttachedToConbWindowSlot(int32_t index) const;
    void SetChangeLookItemInvenSlot(int32_t index, const TItemPos &pos);
    const TItemPos &GetChangeLookItemInvenSlot(int32_t index) const;
    void SetChangeLookItemInvenSlot(int32_t index, int32_t value);
    bool CanAttachItemToCombSlotIndex(int32_t index, CItemData *pItemData);
void ClearHuntingMissions();
#endif

    void SetChangeLookItemData(uint32_t dwSlotIndex, const ClientItemData &rItemInstance);
    void DelChangeLookItemData(uint32_t dwSlotIndex);
    std::optional<ClientItemData> GetChangeLookData(uint32_t dwSlotIndex);
    std::optional<ClientItemData *> GetChangeLookItemDataPtr(uint32_t dwSlotIndex);
    bool GetChangeWindowChangeLookVnum(uint32_t dwSlotIndex, uint32_t *pdwItemID);
    std::optional<uint32_t> GetChangeLookItemID(uint32_t dwSlotIndex);
    bool GetChangeLookItemID(uint32_t dwSlotIndex, uint32_t *pdwItemID);
    bool GetChangeLookItemDataPtr(uint32_t dwSlotIndex, ClientItemData **ppInstance);

private:
    bool m_pvpDisabled = false;
public:
    bool IsPvPDisabled() const
    {
        return m_pvpDisabled;
    }

    void SetPvPDisabled(bool enable)
    {
        m_pvpDisabled = enable;
    }

private:
    std::array<std::optional<SwitchBotSlotData>, 6> m_switchbotSlotData;
public:
    void SetSwitchbotSlotData(SwitchBotSlotData d);
    void SetSwitchbotSlotAttribute(uint32_t slot, uint32_t alternative, uint32_t attrIndex, TItemApply apply);
    void SetSwitchbotSlotStatus(uint32_t slot, uint8_t status);
    std::optional<SwitchBotSlotData> GetSwitchbotSlotData(uint32_t slot);
    std::optional<CInstanceBase *> __GetPickedActorPtrNew();

    void SetHuntingMission(uint32_t id, const TPlayerHuntingMission &mission);

    std::optional<TPlayerHuntingMission> GetHuntingMission(uint32_t id);

    const HuntingMissionMap &GetHuntingMissions();

private:
  HuntingMissionMap m_huntingMissions;


};

extern const int c_iFastestSendingCount;
extern const int c_iSlowestSendingCount;
extern const float c_fFastestSendingDelay;
extern const float c_fSlowestSendingDelay;
extern const float c_fRotatingStepTime;

extern const float c_fComboDistance;
extern const float c_fPickupDistance;
extern const float c_fClickDistance;
#endif /* METIN2_CLIENT_MAIN_PYTHONPLAYER_H */
