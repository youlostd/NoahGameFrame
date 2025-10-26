#include "StdAfx.h"
#include "Timer.h"

#include <windows.h>

#include <mmsystem.h>

#include "StepTimer.h"

static uint64_t gs_dwBaseTime = 0;
static uint64_t gs_dwServerTime = 0;
static uint64_t gs_dwClientTime = 0;
static uint64_t gs_dwFrameTime = 0;

bool ELTimer_Init()
{
    gs_dwBaseTime = timeGetTime();
    return 1;
}

uint64_t ELTimer_GetMSec()
{
    return timeGetTime() - gs_dwBaseTime;
}

VOID ELTimer_SetServerMSec(uint64_t dwServerTime)
{
    gs_dwServerTime = dwServerTime;
    gs_dwClientTime = DX::StepTimer::instance().GetTotalMillieSeconds();
}

uint64_t ELTimer_GetServerMSec()
{
    return DX::StepTimer::instance().GetTotalMillieSeconds() - gs_dwClientTime + gs_dwServerTime;
}

uint64_t ELTimer_GetFrameMSec()
{
    return gs_dwFrameTime;
}

uint64_t ELTimer_GetServerFrameMSec()
{
    return ELTimer_GetFrameMSec() - gs_dwClientTime + gs_dwServerTime;
}

VOID ELTimer_SetFrameMSec()
{
    gs_dwFrameTime = DX::StepTimer::instance().GetTotalMillieSeconds();
}

CTimer::CTimer()
{
    ELTimer_Init();

    m_dwCurrentTime = 0;
    m_bUseRealTime = true;
    m_index = 0;

    m_dwElapsedTime = 0;

    m_fCurrentTime = 0.0;
}

void CTimer::SetBaseTime()
{
    m_dwCurrentTime = 0;
}

void CTimer::Advance()
{
    if (!m_bUseRealTime)
    {
        ++m_index;

        if (m_index == 1)
            m_index = -1;

        m_dwCurrentTime += 16 + (m_index & 1);
        m_fCurrentTime = m_dwCurrentTime / 1000.0f;
    }
    else
    {
        const auto currentTime = ELTimer_GetMSec();

        if (m_dwCurrentTime == 0)
            m_dwCurrentTime = currentTime;

        m_dwElapsedTime = currentTime - m_dwCurrentTime;
        m_dwCurrentTime = currentTime;
    }
}

void CTimer::Adjust(int iTimeGap)
{
    m_dwCurrentTime += iTimeGap;
}

double CTimer::GetCurrentSecond() const
{
    if (m_bUseRealTime)
        return ELTimer_GetMSec() / 1000.0f;

    return m_fCurrentTime;
}

uint64_t CTimer::GetCurrentMillisecond() const
{
    if (m_bUseRealTime)
        return ELTimer_GetMSec();

    return m_dwCurrentTime;
}

double CTimer::GetElapsedSecond() const
{
    return GetElapsedMilliecond() / 1000.0;
}

uint64_t CTimer::GetElapsedMilliecond() const
{
    if (!m_bUseRealTime)
        return 16 + (m_index & 1);

    return m_dwElapsedTime;
}

void CTimer::UseCustomTime()
{
    m_bUseRealTime = false;
}
