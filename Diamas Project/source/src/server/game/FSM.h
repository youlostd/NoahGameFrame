#ifndef METIN2_SERVER_GAME_FSM_H
#define METIN2_SERVER_GAME_FSM_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "state.h"

// FSM Class
class CFSM
{
protected:
    CState *m_pCurrentState;             // Current State
    CState *m_pNewState;                 // New State
    CStateTemplate<CFSM> m_stateInitial; // Initial State

public:
    // Constructor
    CFSM();

    // Destructor
    virtual ~CFSM()
    {
    }

    // Global Functions
    virtual void Update();

    // State Functions
    bool IsState(CState &State) const;
    bool GotoState(CState &NewState);

    virtual void BeginStateInitial()
    {
    }

    virtual void StateInitial()
    {
    }

    virtual void EndStateInitial()
    {
    }
};

#endif /* METIN2_SERVER_GAME_FSM_H */
