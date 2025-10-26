#ifndef METIN2_CLIENT_MAIN_PYTHONPLAYEREVENTHANDLER_H
#define METIN2_CLIENT_MAIN_PYTHONPLAYEREVENTHANDLER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../gamelib/ActorInstance.h"
#include "../gamelib/FlyHandler.h"

#include "PythonNetworkStream.h"
#include "InstanceBase.h"

class NormalBowAttackHandler : public IFlyEventHandler
{
public:
    NormalBowAttackHandler();

    void Set(CInstanceBase *target);

    virtual void Clear();

    virtual void OnSetFlyTarget();
    virtual void OnShoot(uint32_t motionKey, uint32_t skill);

    virtual void OnNoTarget()
    {
        /*Tracenf("Shoot : target이 없습니다.");*/
    }

    virtual void OnExplodingOutOfRange()
    {
        /*Tracenf("Shoot : 사정거리가 끝났습니다.");*/
    }

    virtual void OnExplodingAtBackground()
    {
        /*Tracenf("Shoot : 배경에 맞았습니다.");*/
    }

    virtual void OnExplodingAtAnotherTarget(uint32_t dwSkillIndex, uint32_t dwVID);
    virtual void OnExplodingAtTarget(uint32_t dwSkillIndex);

protected:
    uint32_t m_targetVid;
};

class CPythonPlayerEventHandler : public CActorInstance::IEventHandler
{
public:
    static CPythonPlayerEventHandler &GetSingleton();

public:
    virtual ~CPythonPlayerEventHandler();

    virtual void OnSyncing(const SState &c_rkState);
    virtual void OnWaiting(const SState &c_rkState);
    virtual void OnMoving(const SState &c_rkState);
    virtual void OnMove(const SState &c_rkState);
    virtual void OnStop(const SState &c_rkState);
    virtual void OnWarp(const SState &c_rkState);
    virtual void OnAttack(const SState &c_rkState, uint16_t wMotionIndex);
    virtual void OnUseSkill(const SState &c_rkState, UINT uMotSkill, UINT uLoopCount, bool isMovingSkill, uint32_t color=0);
    virtual void OnUpdate();
    virtual void OnChangeShape();
    virtual void OnHit(uint32_t motionKey, UINT uSkill, CActorInstance &rkActorVictim, const Vector3& pushDest, bool isSendPacket);

    void FlushVictimList();

    NormalBowAttackHandler &GetNormalBowAttackHandler()
    {
        return m_normalBowAttackHandler;
    }

protected:
    CPythonPlayerEventHandler();

protected:
    struct SVictim
    {
        uint32_t m_dwVID;
        int32_t m_lPixelX;
        int32_t m_lPixelY;
    };

protected:
    std::vector<SVictim> m_kVctkVictim;

    uint32_t m_dwPrevComboIndex;
    uint32_t m_dwNextWaitingNotifyTime;
    uint32_t m_dwNextMovingNotifyTime;
    TPixelPosition m_kPPosPrevWaiting;

private:
    NormalBowAttackHandler m_normalBowAttackHandler;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONPLAYEREVENTHANDLER_H */
