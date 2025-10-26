#include "StdAfx.h"
#include "InstanceBase.h"
#include "../gameLib/ActorInstance.h"
#include "PythonPlayer.h"
#include <game/MotionConstants.hpp>
#include "../eterBase/Timer.h"

const int c_iFishingRotStep = 8;
const float c_fFishingDistance = 600.0f;

void CInstanceBase::SetMotionMode(int iMotionMode)
{
    m_GraphicThingInstance.SetMotionMode(iMotionMode);
}

int CInstanceBase::GetMotionMode(uint32_t dwMotionIndex)
{
    return m_GraphicThingInstance.GetMotionMode();
}

void CInstanceBase::SetLoopMotion(uint16_t wMotion, float fBlendTime/* =0.1f */, float fSpeedRatio)
{
    m_GraphicThingInstance.SetLoopMotion(wMotion, fBlendTime, fSpeedRatio);
}

void CInstanceBase::PushOnceMotion(uint16_t wMotion, float fBlendTime, float fSpeedRatio)
{
    m_GraphicThingInstance.PushOnceMotion(wMotion, fBlendTime, fSpeedRatio);
}

void CInstanceBase::PushLoopMotion(uint16_t wMotion, float fBlendTime, float fSpeedRatio)
{
    m_GraphicThingInstance.PushLoopMotion(wMotion, fBlendTime, fSpeedRatio);
}

void CInstanceBase::ResetLocalTime()
{
    m_GraphicThingInstance.ResetLocalTime();
}

void CInstanceBase::SetEndStopMotion()
{
    m_GraphicThingInstance.SetEndStopMotion();
}

bool CInstanceBase::isLock()
{
    return m_GraphicThingInstance.isLock();
}

void CInstanceBase::StartFishing(float frot)
{
    BlendRotation(frot);

    const TPixelPosition &c_rkPPosCur = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
    float fRot = m_GraphicThingInstance.GetTargetRotation();
    //float fPlainCoordRot=ELRightCoord_ConvertToPlainCoordDegree(fRightCoordRot);

    TPixelPosition kPPosFishing;
    ELPlainCoord_GetRotatedPixelPosition(c_rkPPosCur.x, c_rkPPosCur.y, c_fFishingDistance, fRot, &kPPosFishing.x,
                                         &kPPosFishing.y);
    if (!__Background_GetWaterHeight(kPPosFishing, &kPPosFishing.z))
        kPPosFishing.z = c_rkPPosCur.z;

    Vector3 v3Fishing;
    PixelPositionToVector3(kPPosFishing, &v3Fishing);
    m_GraphicThingInstance.SetFishingPosition(v3Fishing);

    PushOnceMotion(MOTION_FISHING_THROW);
    PushLoopMotion(MOTION_FISHING_WAIT);
}

void CInstanceBase::StopFishing()
{
    m_GraphicThingInstance.InterceptOnceMotion(MOTION_FISHING_STOP);
    PushLoopMotion(MOTION_WAIT);
}

void CInstanceBase::ReactFishing()
{
    PushOnceMotion(MOTION_FISHING_REACT);
    PushLoopMotion(MOTION_FISHING_WAIT);
}

void CInstanceBase::CatchSuccess()
{
    m_GraphicThingInstance.InterceptOnceMotion(MOTION_FISHING_CATCH);
    PushLoopMotion(MOTION_WAIT);
}

void CInstanceBase::CatchFail()
{
    m_GraphicThingInstance.InterceptOnceMotion(MOTION_FISHING_FAIL);
    PushLoopMotion(MOTION_WAIT);
}

bool CInstanceBase::GetFishingRot(int *pirot)
{
    const TPixelPosition &c_rkPPosCur = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
    float fCharacterRot = m_GraphicThingInstance.GetRotation();

    //float frot = fCharacterRot;

    for (float fRot = 0.0f; fRot <= 180.0f; fRot += 10.0f)
    {
        TPixelPosition kPPosFishingRight;
        ELPlainCoord_GetRotatedPixelPosition(c_rkPPosCur.x, c_rkPPosCur.y, c_fFishingDistance, fCharacterRot + fRot,
                                             &kPPosFishingRight.x, &kPPosFishingRight.y);
        if (__Background_IsWaterPixelPosition(kPPosFishingRight))
        {
            *pirot = fCharacterRot + fRot;
            return true;
        }

        TPixelPosition kPPosFishingLeft;
        ELPlainCoord_GetRotatedPixelPosition(c_rkPPosCur.x, c_rkPPosCur.y, c_fFishingDistance, fCharacterRot - fRot,
                                             &kPPosFishingLeft.x, &kPPosFishingLeft.y);
        if (__Background_IsWaterPixelPosition(kPPosFishingLeft))
        {
            *pirot = fCharacterRot - fRot;
            return true;
        }
    }

    return false;
}

void CInstanceBase::__EnableChangingTCPState()
{
    m_bEnableTCPState = TRUE;
}

void CInstanceBase::__DisableChangingTCPState()
{
    m_bEnableTCPState = FALSE;
}

void CInstanceBase::ActDualEmotion(CInstanceBase &rkDstInst, uint16_t wMotionNumber1, uint16_t wMotionNumber2)
{
    if (!IsWaiting())
    {
        m_GraphicThingInstance.SetLoopMotion(MOTION_WAIT, 0.05f);
    }
    if (!rkDstInst.IsWaiting())
    {
        rkDstInst.m_GraphicThingInstance.SetLoopMotion(MOTION_WAIT, 0.05f);
    }

    static const float c_fEmotionDistance = 100.0f;

    const TPixelPosition &c_rMainPosition = NEW_GetCurPixelPositionRef();
    const TPixelPosition &c_rTargetPosition = rkDstInst.NEW_GetCurPixelPositionRef();
    TPixelPosition kDirection = c_rMainPosition - c_rTargetPosition;
    float fDistance = sqrtf((kDirection.x * kDirection.x) + (kDirection.y * kDirection.y));
    TPixelPosition kDstPosition;
    kDstPosition.x = c_rTargetPosition.x + (kDirection.x / fDistance) * c_fEmotionDistance;
    kDstPosition.y = c_rTargetPosition.y + (kDirection.y / fDistance) * c_fEmotionDistance;

    uint32_t dwCurTime = ELTimer_GetServerMSec() + 500;
    PushTCPStateExpanded(dwCurTime, kDstPosition, 0.0f, FUNC_EMOTION, MAKELONG(wMotionNumber1, wMotionNumber2),
                         rkDstInst.GetVirtualID());

    __DisableChangingTCPState();
    rkDstInst.__DisableChangingTCPState();

    if (__IsMainInstance() || rkDstInst.__IsMainInstance())
    {
        auto &rPlayer = CPythonPlayer::Instance();
        rPlayer.StartEmotionProcess();
    }
}

void CInstanceBase::ActEmotion(uint32_t dwMotionNumber)
{
    if (!IsUsingSkill())
        PushOnceMotion(dwMotionNumber);
}
