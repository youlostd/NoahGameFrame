#include "PythonPlayerEventHandler.h"
#include "../EterBase/Timer.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"
#include "PythonPlayer.h"
#include "StdAfx.h"

CPythonPlayerEventHandler& CPythonPlayerEventHandler::GetSingleton()
{
    static CPythonPlayerEventHandler s_kPlayerEventHandler;
    return s_kPlayerEventHandler;
}

CPythonPlayerEventHandler::~CPythonPlayerEventHandler() {}

void CPythonPlayerEventHandler::OnSyncing(const SState& c_rkState)
{
    const TPixelPosition& c_rkPPosCurSyncing = c_rkState.kPPosSelf;
    m_kPPosPrevWaiting = c_rkPPosCurSyncing;
}

void CPythonPlayerEventHandler::OnWaiting(const SState& c_rkState)
{
    uint32_t dwCurTime = ELTimer_GetMSec();
    if (m_dwNextWaitingNotifyTime > dwCurTime)
        return;

    m_dwNextWaitingNotifyTime = dwCurTime + 100;

    const TPixelPosition& c_rkPPosCurWaiting = c_rkState.kPPosSelf;
    float dx = m_kPPosPrevWaiting.x - c_rkPPosCurWaiting.x;
    float dy = m_kPPosPrevWaiting.y - c_rkPPosCurWaiting.y;
    float len = sqrt(dx * dx + dy * dy);

    if (len < 1.0f)
        return;

    m_kPPosPrevWaiting = c_rkPPosCurWaiting;

    gPythonNetworkStream->SendCharacterStatePacket(c_rkState.kPPosSelf,
                                                   c_rkState.fAdvRotSelf,
                                                   CInstanceBase::FUNC_WAIT, 0);

    // Trace("waiting\n");
}

void CPythonPlayerEventHandler::OnMoving(const SState& c_rkState)
{
    uint32_t dwCurTime = ELTimer_GetMSec();
    if (m_dwNextMovingNotifyTime > dwCurTime)
        return;

#ifdef ENHANCED_SYNC_UPDATE
    m_dwNextMovingNotifyTime = dwCurTime + 300;
#else
    m_dwNextMovingNotifyTime = dwCurTime + 300;
#endif


    gPythonNetworkStream->SendCharacterStatePacket(c_rkState.kPPosSelf,
                                                   c_rkState.fAdvRotSelf,
                                                   CInstanceBase::FUNC_MOVE, 0);

    //	Trace("moving\n");
}

void CPythonPlayerEventHandler::OnMove(const SState& c_rkState)
{
    uint32_t dwCurTime = ELTimer_GetMSec();
    m_dwNextWaitingNotifyTime = dwCurTime + 100;
#ifdef ENHANCED_SYNC_UPDATE
    m_dwNextMovingNotifyTime = dwCurTime + 300;
#else
    m_dwNextMovingNotifyTime = dwCurTime + 300;
#endif

    gPythonNetworkStream->SendCharacterStatePacket(c_rkState.kPPosSelf,
                                                   c_rkState.fAdvRotSelf,
                                                   CInstanceBase::FUNC_MOVE, 0);

    //	Trace("move\n");
}

void CPythonPlayerEventHandler::OnStop(const SState& c_rkState)
{
    gPythonNetworkStream->SendCharacterStatePacket(c_rkState.kPPosSelf,
                                                   c_rkState.fAdvRotSelf,
                                                   CInstanceBase::FUNC_WAIT, 0);

    //	Trace("stop\n");
}

void CPythonPlayerEventHandler::OnWarp(const SState& c_rkState)
{
    gPythonNetworkStream->SendCharacterStatePacket(c_rkState.kPPosSelf,
                                                   c_rkState.fAdvRotSelf,
                                                   CInstanceBase::FUNC_WAIT, 0);
}

void CPythonPlayerEventHandler::OnAttack(const SState& c_rkState,
                                         uint16_t wMotionIndex)
{
    //	Tracef("CPythonPlayerEventHandler::OnAttack [%d]\n", wMotionIndex);
    assert(wMotionIndex < 255);

    gPythonNetworkStream->SendCharacterStatePacket(
        c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, CInstanceBase::FUNC_COMBO,
        wMotionIndex);

#ifdef __ATTACK_SPEED_CHECK__
    static uint32_t s_dwLastTime = timeGetTime();

    uint32_t dwCurTime = timeGetTime();
    Tracef("%d\n", dwCurTime - s_dwLastTime);
    s_dwLastTime = dwCurTime;
#endif
}

void CPythonPlayerEventHandler::OnUseSkill(const SState& c_rkState,
                                           UINT uMotSkill, UINT uLoopCount,
                                           bool isMovingSkill, uint32_t color)
{
    gPythonNetworkStream->SendCharacterSkillStatePacket(
        c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, uMotSkill, uLoopCount,
        isMovingSkill, color, 0);
}

void CPythonPlayerEventHandler::OnUpdate() {}

void CPythonPlayerEventHandler::OnChangeShape()
{
    CPythonPlayer::Instance().NEW_Stop();
}

void CPythonPlayerEventHandler::OnHit(uint32_t motionKey, UINT uSkill,
                                      CActorInstance& rkActorVictim,
                                      const Vector3& pushDest,
                                      bool isSendPacket)
{
    uint32_t dwVIDVictim = rkActorVictim.GetVirtualID();

    CPythonPlayer::Instance().SetTarget(dwVIDVictim, FALSE);

    if (!isSendPacket)
        return;

    auto& chrMgr = CPythonCharacterManager::Instance();
    auto* main = chrMgr.GetMainInstancePtr();

    const auto& victimPos = rkActorVictim.GetPositionVectorRef();

    TPixelPosition mainPos;
    main->NEW_GetPixelPosition(&mainPos);

    gPythonNetworkStream->SendAttackPacket(uSkill, dwVIDVictim, motionKey,
                                           mainPos.x, mainPos.y, pushDest,
                                           victimPos);

    if (!rkActorVictim.IsPushing())
        return;

    chrMgr.AdjustCollisionWithOtherObjects(&rkActorVictim);

    const TPixelPosition& kPPosLast =
        rkActorVictim.NEW_GetLastPixelPositionRef();

    SVictim kVictim{};
    kVictim.m_dwVID = dwVIDVictim;
    kVictim.m_lPixelX = kPPosLast.x;
    kVictim.m_lPixelY = kPPosLast.y;

    rkActorVictim.TEMP_Push(kVictim.m_lPixelX, kVictim.m_lPixelY);

    m_kVctkVictim.push_back(kVictim);
}

void CPythonPlayerEventHandler::FlushVictimList()
{
    if (m_kVctkVictim.empty())
        return;

    // #0000682: [M2EU] 대진각 스킬 사용시 튕김
    unsigned int SYNC_POSITION_COUNT_LIMIT = 4;
    unsigned int uiVictimCount = m_kVctkVictim.size();

    TPacketCGSyncPosition kPacketSyncPos;
    std::vector<TPacketCGSyncPositionElement> v;
    for (unsigned int i = 0; i < uiVictimCount; ++i) {
        const SVictim& rkVictim = m_kVctkVictim[i];
        TPacketCGSyncPositionElement k;
        k.dwVID = rkVictim.m_dwVID;
        k.lX = rkVictim.m_lPixelX;
        k.lY = rkVictim.m_lPixelY;

        v.emplace_back(k);
    }
    kPacketSyncPos.elems = v;
    gPythonNetworkStream->Send(HEADER_CG_SYNC_POSITION, kPacketSyncPos);

    m_kVctkVictim.clear();
}

CPythonPlayerEventHandler::CPythonPlayerEventHandler()
{
    m_dwPrevComboIndex = 0;
    m_dwNextMovingNotifyTime = 0;
    m_dwNextWaitingNotifyTime = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

NormalBowAttackHandler::NormalBowAttackHandler()
    : m_targetVid(0)
{
    // ctor
}

void NormalBowAttackHandler::Set(CInstanceBase* target)
{
    m_targetVid = target->GetVirtualID();
}

void NormalBowAttackHandler::Clear()
{
    m_targetVid = 0;
}

void NormalBowAttackHandler::OnSetFlyTarget()
{
    if (m_targetVid == 0)
        return;

    auto& chr = CPythonCharacterManager::Instance();

    auto target = chr.GetInstancePtr(m_targetVid);
    if (!target)
        return;

    gPythonNetworkStream->SendFlyTargetingPacket(
        m_targetVid,
        target->GetGraphicThingInstancePtr()->OnGetFlyTargetPosition());
}

void NormalBowAttackHandler::OnShoot(uint32_t motionKey, uint32_t skill)
{
    gPythonNetworkStream->SendShootPacket(motionKey, skill);
}

void NormalBowAttackHandler::OnExplodingAtAnotherTarget(uint32_t dwSkillIndex,
                                                        uint32_t dwVID)
{
}

void NormalBowAttackHandler::OnExplodingAtTarget(uint32_t dwSkillIndex)
{
    //	Tracef("Shoot : 원하는 target에 맞았습니다 : %d, %d\n", dwSkillIndex,
    //m_pInstTarget->GetVirtualID()); 	gPythonNe
}
