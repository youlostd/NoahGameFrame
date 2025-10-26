#ifndef METIN2_CLIENT_GAMELIB_FLYHANDLER_HPP
#define METIN2_CLIENT_GAMELIB_FLYHANDLER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class IFlyEventHandler
{
  public:
    IFlyEventHandler()
    {
    }
    virtual ~IFlyEventHandler()
    {
    }

    virtual void Clear()
    {
    }
    // Call by ActorInstance
    virtual void OnSetFlyTarget()
    {
    }
    virtual void OnShoot(uint32_t motionKey, uint32_t skill)
    {
    }

    virtual void OnNoTarget()
    {
    }
    virtual void OnNoArrow()
    {
    }

    // Call by FlyingInstance
    virtual void OnExplodingOutOfRange()
    {
    }
    virtual void OnExplodingAtBackground()
    {
    }
    virtual void OnExplodingAtAnotherTarget(uint32_t dwSkillIndex, uint32_t dwVID)
    {
    }
    virtual void OnExplodingAtTarget(uint32_t dwSkillIndex)
    {
    }
};



#endif
