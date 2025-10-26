#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <cstdint>
#include <base/Singleton.hpp>

class CTimer : public CSingleton<CTimer>
{
public:
	CTimer();
	~CTimer() = default;

	void	Advance();
	void	Adjust(int iTimeGap);
	void	SetBaseTime();

	double	GetCurrentSecond() const;
	uint64_t	GetCurrentMillisecond() const;

	double	GetElapsedSecond() const;
	uint64_t	GetElapsedMilliecond() const;

	void	UseCustomTime();

protected:
	bool	m_bUseRealTime;
	uint64_t	m_dwBaseTime{};
	uint64_t	m_dwCurrentTime;
	double	m_fCurrentTime;
	uint64_t	m_dwElapsedTime;
	uint64_t		m_index;
};

bool	ELTimer_Init();

uint64_t	ELTimer_GetMSec();

void	ELTimer_SetServerMSec(uint64_t dwServerTime);
uint64_t	ELTimer_GetServerMSec();

void	ELTimer_SetFrameMSec();
uint64_t	ELTimer_GetFrameMSec();

uint64_t	ELTimer_GetServerFrameMSec();