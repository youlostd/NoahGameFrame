﻿#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "../eterLib/GrpMath.h"
#include <game/SkillConstants.hpp>

void CInstanceBase::SetAttackSpeed(UINT uAtkSpd)
{
    if (uAtkSpd > 1100)
        uAtkSpd = 0;

    m_GraphicThingInstance.SetAttackSpeed(uAtkSpd / 100.0f);
    m_kHorse.SetAttackSpeed(uAtkSpd);
}

void CInstanceBase::SetMoveSpeed(UINT uMovSpd)
{
    if (uMovSpd > 1100)
        uMovSpd = 0;

    m_GraphicThingInstance.SetMoveSpeed(uMovSpd / 100.0f);
    m_kHorse.SetMoveSpeed(uMovSpd);
}

void CInstanceBase::SetRotationSpeed(float fRotSpd)
{
    m_fMaxRotSpd = fRotSpd;
}

void CInstanceBase::SetGuildLeader(uint8_t isGuildLeader)
{
    m_isGuildLeader = isGuildLeader;
}

bool CInstanceBase::IsGuildLeader() const
{
    return m_isGuildLeader == 1;
}

void CInstanceBase::SetGuildGeneral(uint8_t isGuildLeader)
{
    m_isGuildGeneral = isGuildLeader;
}

bool CInstanceBase::IsGuildGeneral() const
{
    return m_isGuildGeneral == 1;
}

void CInstanceBase::NEW_Stop()
{
    if (__IsSyncing())
        return;

    if (isLock())
        return;

    if (IsUsingSkill())
        return;

    if (!IsWaiting())
        EndWalking();

    m_GraphicThingInstance.__OnStop();
}

void CInstanceBase::NEW_SyncPixelPosition(long &nPPosX, long &nPPosY)
{
    m_GraphicThingInstance.TEMP_Push(nPPosX, nPPosY);
}

void CInstanceBase::Warp(float x, float y)
{
    m_kQue_kCmdNew.clear();
    SCRIPT_SetPixelPosition(x, y);
}

bool CInstanceBase::NEW_CanMoveToDestPixelPosition(const TPixelPosition &c_rkPPosDst)
{
    TPixelPosition kPPosCur;
    NEW_GetPixelPosition(&kPPosCur);

    if (kPPosCur.x == c_rkPPosDst.x && kPPosCur.y == c_rkPPosDst.y)
        return false;

    return true;
}

float CInstanceBase_GetDegreeFromPosition(float x, float y)
{
    Vector3 vtDir(floor(x), floor(y), 0.0f);
    vtDir.Normalize();

    Vector3 vtStan(0, -1, 0);
    float ret = DirectX::XMConvertToDegrees(acosf(vtDir.Dot(vtStan)));

    if (vtDir.x < 0.0f)
        ret = 360.0f - ret;

    return ret;
}

float CInstanceBase::NEW_GetAdvancingRotationFromDirPixelPosition(const TPixelPosition &c_rkPPosDir)
{
    float fDirRot = CInstanceBase_GetDegreeFromPosition(c_rkPPosDir.x, -c_rkPPosDir.y);
    float fClampDirRot = ClampDegree(fDirRot);

    return fClampDirRot;
}

float CInstanceBase::NEW_GetAdvancingRotationFromDestPixelPosition(const TPixelPosition &c_rkPPosDst)
{
    TPixelPosition kPPosCur;
    NEW_GetPixelPosition(&kPPosCur);
    return NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, c_rkPPosDst);
}

float CInstanceBase::NEW_GetAdvancingRotationFromPixelPosition(const TPixelPosition &c_rkPPosSrc,
                                                               const TPixelPosition &c_rkPPosDst)
{
    TPixelPosition kPPosDelta;
    kPPosDelta = c_rkPPosDst - c_rkPPosSrc;
    return NEW_GetAdvancingRotationFromDirPixelPosition(kPPosDelta);
}

void CInstanceBase::NEW_SetAdvancingRotationFromDirPixelPosition(const TPixelPosition &c_rkPPosDir)
{
    float fClampDirRot = NEW_GetAdvancingRotationFromDirPixelPosition(c_rkPPosDir);
    m_GraphicThingInstance.SetAdvancingRotation(fClampDirRot);

    float fCurRot = m_GraphicThingInstance.GetRotation();
    float fAdvRot = m_GraphicThingInstance.GetAdvancingRotation();

    m_iRotatingDirection = GetRotatingDirection(fCurRot, fAdvRot);
}

void CInstanceBase::NEW_SetAdvancingRotationFromPixelPosition(const TPixelPosition &c_rkPPosSrc,
                                                              const TPixelPosition &c_rkPPosDst)
{
    TPixelPosition kPPosDelta;
    kPPosDelta = c_rkPPosDst - c_rkPPosSrc;

    NEW_SetAdvancingRotationFromDirPixelPosition(kPPosDelta);
}

bool CInstanceBase::NEW_SetAdvancingRotationFromDestPixelPosition(const TPixelPosition &c_rkPPosDst)
{
    if (!NEW_CanMoveToDestPixelPosition(c_rkPPosDst))
    {
        SPDLOG_ERROR("Failed to move next position ({0}, {1}, {2})", c_rkPPosDst.x, c_rkPPosDst.y, c_rkPPosDst.z);
        return false;
    }

    TPixelPosition kPPosSrc;
    NEW_GetPixelPosition(&kPPosSrc);
    NEW_SetAdvancingRotationFromPixelPosition(kPPosSrc, c_rkPPosDst);
    return true;
}

void CInstanceBase::SetAdvancingRotation(float fRotation)
{
    float frotDifference = GetDegreeDifference(GetRotation(), fRotation);

    if (frotDifference > 45.0f)
        m_fRotSpd = m_fMaxRotSpd;
    else
        m_fRotSpd = m_fMaxRotSpd * 5 / 12;

    m_GraphicThingInstance.SetAdvancingRotation(ClampDegree(fRotation));
    m_iRotatingDirection = GetRotatingDirection(m_GraphicThingInstance.GetRotation(),
                                                m_GraphicThingInstance.GetAdvancingRotation());
}

void CInstanceBase::StartWalking()
{
    m_GraphicThingInstance.Move();

    if (HasAffect(SKILL_GYEONGGONG) && !m_affectEffects[SKILL_GYEONGGONG])
        m_affectEffects[SKILL_GYEONGGONG] = AttachAffectEffect(SKILL_GYEONGGONG);
    else if (HasAffect(SKILL_KWAESOK) && !m_affectEffects[SKILL_KWAESOK])
        m_affectEffects[SKILL_KWAESOK] = AttachAffectEffect(SKILL_KWAESOK);
}

void CInstanceBase::EndWalking(float fBlendingTime)
{
    assert(!IsWaiting() && "CInstanceBase::EndWalking");

    m_isGoing = false;

    // 걷고 있을때는 무조건 멈추게 해야 한다
    if (IsWalking() || !IsAttacked())
    {
        m_GraphicThingInstance.Stop(fBlendingTime);

        if (HasAffect(SKILL_GYEONGGONG) && m_affectEffects[SKILL_GYEONGGONG])
        {
            DetachEffect(m_affectEffects[SKILL_GYEONGGONG]);
            m_affectEffects[SKILL_GYEONGGONG] = 0;
        }
        else if (HasAffect(SKILL_KWAESOK) && m_affectEffects[SKILL_KWAESOK])
        {
            DetachEffect(m_affectEffects[SKILL_KWAESOK]);
            m_affectEffects[SKILL_KWAESOK] = 0;
        }
    }
}

void CInstanceBase::EndWalkingWithoutBlending()
{
    EndWalking(0.0f);
}

bool CInstanceBase::IsWaiting()
{
    return m_GraphicThingInstance.IsWaiting();
}

bool CInstanceBase::IsWalking()
{
    return m_GraphicThingInstance.IsMoving();
}

bool CInstanceBase::IsPushing()
{
    return m_GraphicThingInstance.IsPushing();
}

bool CInstanceBase::IsAttacked()
{
    return m_GraphicThingInstance.IsAttacked();
}

bool CInstanceBase::IsKnockDown()
{
    if (!m_GraphicThingInstance.IsKnockDown())
        return FALSE;

    return TRUE;
}

bool CInstanceBase::IsAttacking()
{
    return m_GraphicThingInstance.isAttacking();
}

bool CInstanceBase::IsActingEmotion()
{
    return m_GraphicThingInstance.IsActEmotion();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool CInstanceBase::IsGoing()
{
    return m_isGoing;
}

void CInstanceBase::NEW_MoveToDestInstanceDirection(CInstanceBase &rkInstDst)
{
    TPixelPosition kPPosDst;
    rkInstDst.NEW_GetPixelPosition(&kPPosDst);

    NEW_MoveToDestPixelPositionDirection(kPPosDst);
}

bool CInstanceBase::NEW_MoveToDestPixelPositionDirection(const TPixelPosition &c_rkPPosDst)
{
    TPixelPosition kPPosCur;
    NEW_GetPixelPosition(&kPPosCur);
    float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, c_rkPPosDst);

    return NEW_Goto(c_rkPPosDst, fDstRot);
}

bool CInstanceBase::NEW_Goto(const TPixelPosition &c_rkPPosDst, float fDstRot)
{
    if (__IsSyncing())
    {
        return false;
    }

    if (m_GraphicThingInstance.IsUsingMovingSkill())
    {
        SetAdvancingRotation(fDstRot);
        return true;
    }

    if (isLock())
    {
        return false;
    }

    if (!NEW_CanMoveToDestPixelPosition(c_rkPPosDst))
    {
        if (!IsWaiting())
            EndWalking();

        return true;
    }

    NEW_SetSrcPixelPosition(NEW_GetCurPixelPositionRef());
    NEW_SetDstPixelPosition(c_rkPPosDst);
    NEW_SetDstPixelPositionZ(NEW_GetSrcPixelPositionRef().z);
    m_fDstRot = fDstRot;
    m_isGoing = TRUE;

    if (!IsWalking())
    {
        StartWalking();
    }

    NEW_SetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

    return true;
}

void CInstanceBase::NEW_MoveToDirection(float fDirRot)
{
    if (__IsSyncing())
        return;

    if (m_GraphicThingInstance.IsUsingMovingSkill())
    {
        SetAdvancingRotation(fDirRot);
        return;
    }

    if (isLock())
        return;

    m_isGoing = FALSE;

    SetAdvancingRotation(fDirRot);

    if (!IsWalking())
    {
        StartWalking();
    }

    TPixelPosition kPPosCur;
    NEW_GetPixelPosition(&kPPosCur);

    Vector3 kD3DVt3Cur(kPPosCur.x, -kPPosCur.y, kPPosCur.z);
    Vector3 kD3DVt3Dst;

    Vector3 kD3DVt3AdvDir(0.0f, -1.0f, 0.0f);

    Matrix kD3DMatAdv = Matrix::CreateRotationZ(DirectX::XMConvertToRadians(fDirRot));
    kD3DVt3AdvDir = Vector3::Transform(kD3DVt3AdvDir, kD3DMatAdv);
    kD3DVt3AdvDir = XMVectorScale(kD3DVt3AdvDir, 300.0f);
    kD3DVt3Dst = XMVectorAdd(kD3DVt3AdvDir, kD3DVt3Cur);

    TPixelPosition kPPosDst;
    kPPosDst.x = +kD3DVt3Dst.x;
    kPPosDst.y = -kD3DVt3Dst.y;
    kPPosDst.z = +kD3DVt3Dst.z;

    NEW_SetSrcPixelPosition(kPPosCur);
    NEW_SetDstPixelPosition(kPPosDst);
}

void CInstanceBase::EndGoing()
{
    if (!IsWaiting())
        EndWalking();

    m_isGoing = false;
}

void CInstanceBase::SetRunMode()
{
    m_GraphicThingInstance.SetRunMode();
}

void CInstanceBase::SetWalkMode()
{
    m_GraphicThingInstance.SetWalkMode();
}
