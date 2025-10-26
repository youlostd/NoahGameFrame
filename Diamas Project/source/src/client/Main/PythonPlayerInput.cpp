#include "StdAfx.h"
#include "PythonPlayer.h"
#include "PythonPlayerEventHandler.h"
#include "PythonApplication.h"
#include "../eterlib/Camera.h"
#include "../eterbase/Timer.h"

const int c_iFastestSendingCount = 3;
const int c_iSlowestSendingCount = 3;
const float c_fFastestSendingDelay = 1.0f / float(c_iFastestSendingCount);
const float c_fSlowestSendingDelay = 1.0f / float(c_iSlowestSendingCount);
const float c_fRotatingStepTime = 0.5f;

const float c_fComboDistance = 250.0f;
const float c_fClickDistance = 300.0f;

uint32_t CPythonPlayer::__GetPickableDistance()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (pkInstMain)
        if (pkInstMain->IsMountingHorse())
            return 500;

    return 300;
}

void CPythonPlayer::PickCloseMoney()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    TPixelPosition kPPosMain;
    pkInstMain->NEW_GetPixelPosition(&kPPosMain);

    uint32_t dwItemID;
    CPythonItem &rkItem = CPythonItem::Instance();
    if (!rkItem.GetCloseMoney(kPPosMain, &dwItemID, __GetPickableDistance()))
        return;

    SendClickItemPacket(dwItemID);
}

void CPythonPlayer::PickCloseItem()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    TPixelPosition kPPosMain;
    pkInstMain->NEW_GetPixelPosition(&kPPosMain);
    uint32_t dwItemID;
    CPythonItem &rkItem = CPythonItem::Instance();

    auto closeItems = rkItem.GetCloseItem(pkInstMain->GetNameString(), kPPosMain, &dwItemID, __GetPickableDistance());
    for (const auto &item : closeItems)
        SendClickItemPacket(item);
}

bool CPythonPlayer::__IsTarget()
{
    return 0 != __GetTargetVID();
}

bool CPythonPlayer::__IsSameTargetVID(uint32_t dwVID)
{
    return dwVID == __GetTargetVID();
}

uint32_t CPythonPlayer::__GetTargetVID() const
{
    return m_dwTargetVID;
}

uint32_t CPythonPlayer::GetTargetVID()
{
    return __GetTargetVID();
}

CInstanceBase *CPythonPlayer::__GetTargetActorPtr()
{
    return NEW_FindActorPtr(__GetTargetVID());
}

void CPythonPlayer::__SetTargetVID(uint32_t dwVID)
{
    m_dwTargetVID = dwVID;
}

void CPythonPlayer::__ClearTarget()
{
    if (!__IsTarget())
        return;

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    if (pkInstMain->CanChangeTarget())
        pkInstMain->ClearFlyTargetInstance();

    CInstanceBase *pTargetedInstance = __GetTargetActorPtr();
    if (pTargetedInstance)
        pTargetedInstance->OnUntargeted();

    __SetTargetVID(0);

    gPythonNetworkStream->SendTargetPacket(0);
}

void CPythonPlayer::SetTarget(uint32_t dwVID, bool bForceChange)
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    // 2004. 07. 07. [levites] - ��ų ����� Ÿ���� �ٲ�� ���� �ذ��� ���� �ڵ�
    if (!pkInstMain->CanChangeTarget())
    {
        return;
    }

    const uint64_t dwCurrentTime = DX::StepTimer::instance().GetTotalMillieSeconds();

    if (__IsSameTargetVID(dwVID))
    {
        if (dwVID == pkInstMain->GetVirtualID())
        {
            __SetTargetVID(0);

            pkInstMain->OnUntargeted();
            pkInstMain->ClearFlyTargetInstance();
            gPythonNetworkStream->SendTargetPacket(0);
            return;
        }
        m_dwTargetEndTime = dwCurrentTime + 1000;
        return;
    }

    if (bForceChange)
    {
        m_dwTargetEndTime = dwCurrentTime + 2000;
    }
    else
    {
        if (m_dwTargetEndTime > dwCurrentTime)
        {
            return;
        }

        m_dwTargetEndTime = dwCurrentTime + 1000;
    }

    if (__IsTarget())
    {
        CInstanceBase *pTargetedInstance = __GetTargetActorPtr();
        if (pTargetedInstance)
            pTargetedInstance->OnUntargeted();
    }

    CInstanceBase *pkInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
    if (pkInstTarget)
    {
        if (pkInstMain->IsTargetableInstance(*pkInstTarget))
        {
            __SetTargetVID(dwVID);

            pkInstTarget->OnTargeted();
            pkInstMain->SetFlyTargetInstance(*pkInstTarget);

            auto &handler = CPythonPlayerEventHandler::GetSingleton().GetNormalBowAttackHandler();
            handler.Set(pkInstTarget);
            pkInstMain->GetGraphicThingInstanceRef().SetFlyEventHandler(&handler);

            gPythonNetworkStream->SendTargetPacket(dwVID);
            return;
        }
    }

    __SetTargetVID(0);

    pkInstMain->ClearFlyTargetInstance();
    gPythonNetworkStream->SendTargetPacket(0);
}

bool CPythonPlayer::__ChangeTargetToPickedInstance()
{
    uint32_t dwVID;
    if (!CPythonCharacterManager::Instance().OLD_GetPickedInstanceVID(&dwVID))
        return false;

    SetTarget(dwVID);
    return true;
}

CInstanceBase *CPythonPlayer::__GetSkillTargetInstancePtr(CPythonSkill::TSkillData &rkSkillData)
{
    if (rkSkillData.IsNeedCorpse())
        return __GetDeadTargetInstancePtr();

    return __GetAliveTargetInstancePtr();
}

CInstanceBase *CPythonPlayer::__GetDeadTargetInstancePtr()
{
    if (!__IsTarget())
        return NULL;

    CInstanceBase *pkInstTarget = __GetTargetActorPtr();

    if (!pkInstTarget)
        return NULL;

    if (!pkInstTarget->IsDead())
        return NULL;

    return pkInstTarget;
}

CInstanceBase *CPythonPlayer::__GetAliveTargetInstancePtr()
{
    if (!__IsTarget())
        return NULL;

    CInstanceBase *pkInstTarget = __GetTargetActorPtr();

    if (!pkInstTarget)
        return NULL;

    if (pkInstTarget->IsDead())
        return NULL;

    if (!pkInstTarget->CanPickInstance(0))
        return NULL;

    return pkInstTarget;
}

void CPythonPlayer::OpenCharacterMenu(uint32_t dwVictimActorID)
{
    CInstanceBase *pkInstMain = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    CInstanceBase *pkInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(dwVictimActorID);
    if (!pkInstTarget)
        return;

    if (!pkInstTarget->IsPC()) {
#ifdef __SPECIAL_OFFLINE_SHOP_WHISPER__
        if (pkInstTarget->GetRace() == OFFLINE_SHOP_NPC_RACE) {
            PyCallClassMemberFunc(
                m_ppyGameWindow, "SetOfflineShopTargetBoard",
                pkInstTarget->GetNameString());
            return;
        }
#endif

        if (!pkInstTarget->IsBuilding()) {
            return;
        }
    }

    //	if (pkInstMain == pkInstTarget)
    //		return;

    uint32_t vid = pkInstTarget->GetVirtualID();
  

    PyCallClassMemberFunc(m_ppyGameWindow, "SetPCTargetBoard", vid, py::bytes(pkInstTarget->GetNameString()));
}

void CPythonPlayer::__OnClickItem(CInstanceBase &rkInstMain, uint32_t dwItemID)
{
}

void CPythonPlayer::__OnClickActor(CInstanceBase &rkInstMain, uint32_t dwPickedActorID, bool isAuto)
{
    // ���� ��ų�� �Ἥ �������̶��..
    if (MODE_USE_SKILL == m_eReservedMode)
    {
        // ���� ĳ���͸� Ŭ�� ������ ����
        if (__GetTargetVID() == dwPickedActorID)
            return;

        // 2005.03.25.levites
        // źȯ���� ���� �޷������� ������ �� �ִ� �ٸ� Ÿ���� Ŭ���ϸ�
        if (__CheckDashAffect(rkInstMain))
        {
            m_dwVIDReserved = dwPickedActorID;
            return;
        }
    }

    __ClearReservedAction();

    CInstanceBase *pkInstVictim = NEW_FindActorPtr(dwPickedActorID);
    if (!pkInstVictim)
        return;

    CInstanceBase &rkInstVictim = *pkInstVictim;

    // 2005.01.28.myevan
    // �ʱ޸� ���¿����� ������ �ȵǳ� NPC Ŭ���̵Ǿ����
    if (rkInstMain.IsAttackableInstance(*pkInstVictim))
        if (!__CanAttack())
            return;

    if (!rkInstMain.NEW_IsClickableDistanceDestInstance(rkInstVictim))
    {
        __ReserveClickActor(dwPickedActorID);
        return;
    }

    if (rkInstVictim.IsNPC() || rkInstVictim.IsBuffBot())
    {
        __SendClickActorPacket(rkInstVictim);
    }

    rkInstMain.NEW_Stop();
    return;
}

void CPythonPlayer::__OnPressActor(CInstanceBase &rkInstMain, uint32_t dwPickedActorID, bool isAuto)
{
    // ���� ��ų�� �Ἥ �������̶��..
    if (MODE_USE_SKILL == m_eReservedMode)
    {
        // ���� ĳ���͸� Ŭ�� ������ ����
        if (__GetTargetVID() == dwPickedActorID)
            return;

        // 2005.03.25.levites
        // źȯ���� ���� �޷������� ������ �� �ִ� �ٸ� Ÿ���� Ŭ���ϸ�
        if (__CheckDashAffect(rkInstMain))
        {
            m_dwVIDReserved = dwPickedActorID;
            return;
        }
    }

    __ChangeTargetToPickedInstance();
    __ClearReservedAction();

    if (!__CanAttack())
        return;

    CInstanceBase *pkInstVictim = NEW_FindActorPtr(dwPickedActorID);
    if (!pkInstVictim)
        return;

    CInstanceBase &rkInstVictim = *pkInstVictim;

    if (isAuto)
    {
        // 2004.10.21.myevan.���ݰ����� ��븸 �ڵ� ����
        if (rkInstMain.IsAttackableInstance(rkInstVictim))
            __SetAutoAttackTargetActorID(rkInstVictim.GetVirtualID());
    }

    if (rkInstMain.IsBowMode())
    {
        if (rkInstMain.IsAttackableInstance(rkInstVictim))
            if (!__CanShot(rkInstMain, rkInstVictim))
                return;
    }

    if (!rkInstMain.NEW_IsClickableDistanceDestInstance(rkInstVictim))
    {
        __ReserveClickActor(dwPickedActorID);
        return;
    }

    if (!rkInstMain.IsAttackableInstance(rkInstVictim))
    {
        return;
    }

    auto &evt = CPythonPlayerEventHandler::GetSingleton();
    auto &fly = evt.GetNormalBowAttackHandler();
    fly.Set(pkInstVictim);

    rkInstMain.NEW_AttackToDestInstanceDirection(rkInstVictim);
}

void CPythonPlayer::__OnPressItem(CInstanceBase &rkInstMain, uint32_t dwPickedItemID)
{
    static uint32_t s_dwLastPickItemID = 0;

    if (s_dwLastPickItemID == dwPickedItemID)
    {
        SPDLOG_WARN("CPythonPlayer::__OnPressItem - ALREADY PICKED ITEM");
        return;
    }

    __ClearReservedAction();
    __ClearAutoAttackTargetActorID();

    CPythonItem &rkItem = CPythonItem::Instance();

    TPixelPosition kPPosPickedItem;
    if (!rkItem.GetGroundItemPosition(dwPickedItemID, &kPPosPickedItem))
        return;

    if (!rkInstMain.NEW_IsClickableDistanceDestPixelPosition(kPPosPickedItem))
    {
        __ReserveClickItem(dwPickedItemID);
        return;
    }

    rkInstMain.NEW_Stop();
    SendClickItemPacket(dwPickedItemID);
}

void CPythonPlayer::__OnClickGround(CInstanceBase &rkInstMain, const TPixelPosition &c_rkPPosPickedGround)
{
    if (!__IsMovableGroundDistance(rkInstMain, c_rkPPosPickedGround))
        return;

    if (rkInstMain.NEW_MoveToDestPixelPositionDirection(c_rkPPosPickedGround))
        __ShowPickedEffect(c_rkPPosPickedGround);
}

bool CPythonPlayer::__IsMovableGroundDistance(CInstanceBase &rkInstMain, const TPixelPosition &c_rkPPosPickedGround)
{
    static const float kMinimumDistance = 200.0f;

    float fDistance = rkInstMain.NEW_GetDistanceFromDestPixelPosition(c_rkPPosPickedGround);
    if (fDistance < kMinimumDistance)
        return false;

    //SPDLOG_DEBUG("IsMovableGroundDistance(%f)\n", fDistance);
    return true;
}

void CPythonPlayer::__OnPressGround(CInstanceBase &rkInstMain, const TPixelPosition &c_rkPPosPickedGround)
{
    __ClearReservedAction();
    __ClearAutoAttackTargetActorID();

    if (NEW_CancelFishing())
        return;

    if (!__IsMovableGroundDistance(rkInstMain, c_rkPPosPickedGround))
        return;

    if (!rkInstMain.NEW_MoveToDestPixelPositionDirection(c_rkPPosPickedGround))
    {
        __ReserveClickGround(c_rkPPosPickedGround);
        return;
    }
}

void CPythonPlayer::__OnPressScreen(CInstanceBase &rkInstMain)
{
    __ClearReservedAction();

    NEW_MoveToMouseScreenDirection();
}

bool CPythonPlayer::NEW_MoveToDirection(float fDirRot)
{
    // PrivateShop
    if (IsOpenPrivateShop())
        return true;

    __ClearReservedAction();

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    if (pkInstMain->isLock())
        if (!pkInstMain->IsUsingMovingSkill())
            return true;

    CCamera *pkCmrCur = CCameraManager::Instance().GetCurrentCamera();
    if (pkCmrCur)
    {
        float fCmrCurRot = CameraRotationToCharacterRotation(pkCmrCur->GetRoll());

        // ���� 
        if (m_isCmrRot)
        {
            float fSigDirRot = fDirRot;
            if (fSigDirRot > 180.0f)
                fSigDirRot = fSigDirRot - 360.0f;

            float fRotRat = fSigDirRot;
            if (fRotRat > 90.0f)
                fRotRat = (180.0f - fRotRat);
            else if (fRotRat < -90.0f)
                fRotRat = (-180.0f - fRotRat);

            const auto fElapsedTime = CPythonApplication::AppInst().GetGlobalElapsedTime();

            float fRotDeg = -m_fCmrRotSpd * fElapsedTime * fRotRat / 90.0f;
            pkCmrCur->Roll(fRotDeg);
        }

        fDirRot = fmod(360.0f + fCmrCurRot + fDirRot, 360.0f);
    }

    //Don't move if free camera - [Think]
    if (IsFreeCameraMode())
    {
        CPythonApplication::AppInst().MoveFreeCamera(fDirRot, false);
        return true;
    }

    pkInstMain->NEW_MoveToDirection(fDirRot);

    return true;
}

void CPythonPlayer::NEW_Stop()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    pkInstMain->NEW_Stop();
    m_isLeft = false;
    m_isRight = false;
    m_isUp = false;
    m_isDown = false;
    m_isDirKey = false;
}

bool CPythonPlayer::NEW_CancelFishing()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    if (pkInstMain->IsFishing())
    {
        const auto now = ELTimer_GetMSec();

        static uint32_t s_dwLastCancelTime = 0;
        if (now < s_dwLastCancelTime + 500)
            return false;

        gPythonNetworkStream->SendFishingPacket(0);
        s_dwLastCancelTime = now;
        return true;
    }

    return false;
}

void CPythonPlayer::NEW_Fishing()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    if (pkInstMain->IsFishing())
    {
        gPythonNetworkStream->SendFishingPacket(0);
    }
    else
    {
        if (pkInstMain->CanFishing())
        {
            int irot;
            if (pkInstMain->GetFishingRot(&irot))
                gPythonNetworkStream->SendFishingPacket(irot);
            else
            {
                m_ppyGameWindow.attr("OnFishingWrongPlace")();
            }
        }
    }
}

void CPythonPlayer::NEW_Attack()
{
    // PrivateShop
    if (IsOpenPrivateShop())
        return;

    if (!__CanAttack())
        return;

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return;

    if (pkInstMain->IsBowMode())
    {
        //CPythonPlayerEventHandler& rkPlayerEventHandler=CPythonPlayerEventHandler::GetSingleton();

        CInstanceBase *pkInstTarget = __GetAliveTargetInstancePtr();
        if (!pkInstTarget)
        {
            __ChangeTargetToPickedInstance();
            pkInstTarget = __GetAliveTargetInstancePtr();
        }

        if (pkInstTarget)
        {
            if (!__CanShot(*pkInstMain, *pkInstTarget))
                return;

            if (!pkInstMain->NEW_IsClickableDistanceDestInstance(*pkInstTarget))
            {
                __ReserveClickActor(pkInstTarget->GetVirtualID());
                return;
            }

            if (pkInstMain->IsAttackableInstance(*pkInstTarget))
            {
                pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);
                pkInstMain->NEW_AttackToDestInstanceDirection(*pkInstTarget);
            }
        }
        else
        {
            m_ppyGameWindow.attr("OnCannotUseSkill")(GetMainCharacterIndex(), "NEED_TARGET");
            return;
        }
    }
    else if (m_isDirKey)
    {
        const auto width = UI::CWindowManager::Instance().GetScreenWidth();
        const auto height = UI::CWindowManager::Instance().GetScreenHeight();

        const auto dir = GetKeyMovementDirection(m_isLeft, m_isRight, m_isUp, m_isDown);
        auto dirRot = GetKeyMovementRotation(dir.first, dir.second,
                                             width, height);

        CCamera *pkCmrCur = CCameraManager::Instance().GetCurrentCamera();
        if (pkCmrCur)
        {
            float fCmrCurRot = CameraRotationToCharacterRotation(pkCmrCur->GetRoll());
            dirRot = fmod(360.0f + fCmrCurRot + dirRot, 360.0f);
        }

        pkInstMain->NEW_Attack(dirRot);
    }
    else
    {
        //!@# ���� ź ���¿��� �Ǽ� ������ �������� �ʴ´� - [levites]
        if (pkInstMain->IsMountingHorse())
        {
            if (pkInstMain->IsHandMode())
                return;
        }

        pkInstMain->NEW_Attack();
    }
}

bool CPythonPlayer::NEW_IsEmptyReservedDelayTime(float fElapsedTime)
{
    m_fReservedDelayTime -= fElapsedTime;

    if (m_fReservedDelayTime <= 0.0f)
    {
        m_fReservedDelayTime = 0.0f;
        return true;
    }

    return false;
}

void CPythonPlayer::NEW_SetAutoCameraRotationSpeed(float fRotSpd)
{
    m_fCmrRotSpd = fRotSpd;
}

void CPythonPlayer::NEW_ResetCameraRotation()
{
    CCamera *pkCmrCur = CCameraManager::Instance().GetCurrentCamera();
    CPythonApplication &rkApp = CPythonApplication::AppInst();

    pkCmrCur->EndDrag();

    rkApp.SetCursorNum(CPythonApplication::NORMAL);
    if (CPythonApplication::CURSOR_MODE_HARDWARE == rkApp.GetCursorMode())
        rkApp.SetCursorVisible(TRUE);
}

bool CPythonPlayer::__CanShot(CInstanceBase &rkInstMain, CInstanceBase &rkInstTarget)
{
    if (!__HasEnoughArrow())
    {
        PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", GetMainCharacterIndex(), "EMPTY_ARROW");
        return false;
    }

    if (rkInstMain.IsInSafe())
    {
        PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", GetMainCharacterIndex(), "IN_SAFE");
        return false;
    }

    if (rkInstTarget.IsInSafe())
    {
        PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", GetMainCharacterIndex(), "DEST_IN_SAFE");
        return false;
    }

    return true;
}

bool CPythonPlayer::__CanChangeTarget()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    return pkInstMain->CanChangeTarget();
}

bool CPythonPlayer::__CanMove()
{
    if (__IsProcessingEmotion())
    {
        return false;
    }

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    if (!pkInstMain->CanMove())
    {
        if (!pkInstMain->IsUsingMovingSkill())
            return false;
    }

    return true;
}

bool CPythonPlayer::__CanAttack()
{
    if (__IsProcessingEmotion())
    {
        return false;
    }

    if (IsOpenPrivateShop())
        return false;

    if (IsObserverMode())
        return false;

    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    if (!pkInstMain)
        return false;

    // Fix me
    // ������Ʈ 25���� �̻� 35���� �̸��� ��� �߱� ����Ʈ�� Ÿ�� ���ݸ��ϵ��� �ϵ� �ڵ�... 
    // ���߿� �ð� ���� can attack üũ�� �������� ������...
    // ��_�� unique ���Կ� ���� Ż���� �� �����̶� ������� ������ �� �־�� �Ѵ� ��_��
    // ��_�� ������Ʈ�� �� üũ�� �ϰ� ��... ��_��_��_��_��

    /*if (pkInstMain->IsMountingHorse() && pkInstMain->IsNewMount() && (GetSkillGrade(109) < 1 && GetSkillLevel(109) < 11))
    {
        return false;
    }*/

    return pkInstMain->CanAttack();
}

void CPythonPlayer::__ClearReservedAction()
{
    m_eReservedMode = MODE_NONE;
}

void CPythonPlayer::__ReserveClickItem(uint32_t dwItemID)
{
    m_eReservedMode = MODE_CLICK_ITEM;
    m_dwIIDReserved = dwItemID;
}

void CPythonPlayer::__ReserveClickActor(uint32_t dwActorID)
{
    m_eReservedMode = MODE_CLICK_ACTOR;
    m_dwVIDReserved = dwActorID;
}

void CPythonPlayer::__ReserveClickGround(const TPixelPosition &c_rkPPosPickedGround)
{
    m_eReservedMode = MODE_CLICK_POSITION;
    m_kPPosReserved = c_rkPPosPickedGround;
    m_fReservedDelayTime = 0.1f;
}

bool CPythonPlayer::__IsReservedUseSkill(uint32_t dwSkillSlotIndex)
{
    if (MODE_USE_SKILL != m_eReservedMode)
        return false;

    if (m_dwSkillSlotIndexReserved != dwSkillSlotIndex)
        return false;

    return true;
}

void CPythonPlayer::__ReserveUseSkill(uint32_t dwActorID, uint32_t dwSkillSlotIndex, uint32_t dwRange)
{
    m_eReservedMode = MODE_USE_SKILL;
    m_dwVIDReserved = dwActorID;
    m_dwSkillSlotIndexReserved = dwSkillSlotIndex;
    m_dwSkillRangeReserved = dwRange;

    // NOTE : �ƽ��ƽ��ϰ� �Ÿ��� ���̴� ������ �־ �ణ �����ϰ�..
    if (m_dwSkillRangeReserved > 100)
        m_dwSkillRangeReserved -= 10;
}

void CPythonPlayer::__ClearAutoAttackTargetActorID()
{
    __SetAutoAttackTargetActorID(0);
}

void CPythonPlayer::__SetAutoAttackTargetActorID(uint32_t dwVID)
{
    m_dwAutoAttackTargetVID = dwVID;
}

void CPythonPlayer::__ReserveProcess_ClickActor()
{
    CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
    CInstanceBase *pkInstReserved = NEW_FindActorPtr(m_dwVIDReserved);
    if (!pkInstMain || !pkInstReserved)
    {
        __ClearReservedAction();
        return;
    }

    if (!pkInstMain->NEW_IsClickableDistanceDestInstance(*pkInstReserved))
    {
        pkInstMain->NEW_MoveToDestInstanceDirection(*pkInstReserved);
        return;
    }

    if (!pkInstMain->IsAttackableInstance(*pkInstReserved))
    {
        pkInstMain->NEW_Stop();
        __SendClickActorPacket(*pkInstReserved);
        __ClearReservedAction();
        return;
    }

    if (pkInstReserved->IsDead())
    {
        __ClearReservedAction();
        return;
    }

    if (pkInstMain->IsInSafe())
    {
        PyCallClassMemberFunc(m_ppyGameWindow.ptr(), "OnCannotAttack",
                              Py_BuildValue("(is)", GetMainCharacterIndex(), "IN_SAFE"));
        pkInstMain->NEW_Stop();
        __ClearReservedAction();
        return;
    }

    if (pkInstReserved->IsInSafe())
    {
        PyCallClassMemberFunc(m_ppyGameWindow.ptr(), "OnCannotAttack",
                              Py_BuildValue("(is)", GetMainCharacterIndex(), "DEST_IN_SAFE"));
        pkInstMain->NEW_Stop();
        __ClearReservedAction();
        return;
    }

    // źȯ�� ���� �޷����� ���߿��� �������� �ʴ´�.
    if (__CheckDashAffect(*pkInstMain))
    {
        return;
    }

    /////

    if (pkInstMain->IsBowMode())
    {
        if (!__HasEnoughArrow())
        {
            PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", GetMainCharacterIndex(), "EMPTY_ARROW");
            pkInstMain->NEW_Stop();
            __ClearReservedAction();
            return;
        }
    }

    if (pkInstReserved->GetVirtualID() != GetTargetVID())
    {
        SetTarget(pkInstReserved->GetVirtualID());
    }

    /* note :  �ڵ����� ���� ������ ���ʿ� �߰�.
     * CanAttack�� ȣ���� �� �ʿ��ѵ����� ȣ��ǰ� ���ľ��� �ʿ伺�� ����..
     * __CanAttack�� CanAttack�� ����� �ٸ��͵� ���� �ʿ�.. ~ ity ~
     */
    if (!__CanAttack())
    {
        return;
    }

    pkInstMain->NEW_AttackToDestInstanceDirection(*pkInstReserved);
    __ClearReservedAction();
}
