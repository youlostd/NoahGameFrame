#ifndef METIN2_SERVER_GAME_STATE_H
#define METIN2_SERVER_GAME_STATE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

//==================================================================================================
// CState

// CState Class

class CState
{
public:
    // Destructor
    virtual ~CState()
    {
    }

    // State Functions
    virtual void ExecuteBeginState() = 0;
    virtual void ExecuteState() = 0;
    virtual void ExecuteEndState() = 0;
};

//==================================================================================================
// CStateTemplate

// CStateTemplate Class
template <class T>
class CStateTemplate : public CState
{
public:
    typedef void (T::*PFNSTATE)(void);

protected:
    T *m_pInstance;           // Instance Pointer
    PFNSTATE m_pfnBeginState; // State Function Pointer
    PFNSTATE m_pfnState;      // State Function Pointer
    PFNSTATE m_pfnEndState;   // State Function Pointer

public:
    // Constructor
    CStateTemplate()
        : m_pInstance(nullptr), m_pfnBeginState(nullptr), m_pfnState(nullptr), m_pfnEndState(nullptr)
    {
    }

    // Initialize Functions
    void Set(T *pInstance, PFNSTATE pfnBeginState, PFNSTATE pfnState, PFNSTATE pfnEndState)
    {
        // Set Instance
        assert(pInstance && "NULL not valid");
        m_pInstance = pInstance;

        // Set Function Pointers
        assert(pfnBeginState && "NULL not valid");
        m_pfnBeginState = pfnBeginState;

        assert(pfnState && "NULL not valid");
        m_pfnState = pfnState;

        assert(pfnEndState && "NULL not valid");
        m_pfnEndState = pfnEndState;
    }

    // State Functions
    virtual void ExecuteBeginState()
    {
        // Begin State
        assert(m_pInstance && m_pfnBeginState && "NULL not valid");
        (m_pInstance->*m_pfnBeginState)();
    }

    virtual void ExecuteState()
    {
        // State
        assert(m_pInstance && m_pfnState && "NULL not valid");
        (m_pInstance->*m_pfnState)();
    }

    virtual void ExecuteEndState()
    {
        // End State
        assert(m_pInstance && m_pfnEndState && "NULL not valid");
        (m_pInstance->*m_pfnEndState)();
    }
};

#endif /* METIN2_SERVER_GAME_STATE_H */
