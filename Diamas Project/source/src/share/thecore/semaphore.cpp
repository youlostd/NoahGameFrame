#include "semaphore.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <cerrno>
#include <cstdio>
#endif
#include <cassert>
#include <SpdLog.hpp>
#include <storm/ErrorReporting.hpp>


#if defined(_WIN32)

CSemaphore::CSemaphore()
    : m_sem(NULL)
{
	Initialize();
}

CSemaphore::~CSemaphore()
{
	Destroy();
}

bool CSemaphore::Initialize()
{
	Clear();

	m_sem = ::CreateSemaphore(NULL, 0, 32, NULL);
	if (m_sem == NULL)
		return false;

	return true;
}

void CSemaphore::Destroy()
{
	Clear();
}

void CSemaphore::Clear()
{
	if (m_sem) {
		::CloseHandle(m_sem);
		m_sem = nullptr;
	}
}

bool CSemaphore::Wait()
{
	STORM_ASSERT(m_sem, "Not initialized");

	uint32_t dwWaitResult = ::WaitForSingleObject(m_sem, INFINITE);
	switch (dwWaitResult) {
		case WAIT_OBJECT_0:
			return true;

		default:
			break;
	}

	return false;
}

bool CSemaphore::Release(int count)
{
	STORM_ASSERT(m_sem, "Not initialized");

	::ReleaseSemaphore(m_sem, count, NULL);
	return true;
}

#else

CSemaphore::CSemaphore()
{
	Initialize();
}

CSemaphore::~CSemaphore()
{
	Destroy();
}

bool CSemaphore::Initialize()
{
	if (sem_init(&m_sem, 0, 0) == -1) {
		spdlog::error( "sem_init() failed with {0}",
		          (errno));
		return false;
	}

	return true;
}

void CSemaphore::Destroy()
{
	Clear();
}

void CSemaphore::Clear()
{
	sem_destroy(&m_sem);
}

bool CSemaphore::Wait()
{
	while (true) {
		int re = sem_wait(&m_sem);
		if (re == 0)
			break;

		if (errno != EINTR) {
			spdlog::error( "sem_wait() failed with {0}",
			          (errno));
			return false;
		}
	}

	return true;
}

bool CSemaphore::Release(int count)
{
	for (int i = 0; i < count; ++i) {
		if (sem_post(&m_sem) != 0) {
			spdlog::error( "sem_post({0:p}) failed with {1}", (void*)&m_sem,
			          (errno));
			return false;
		}
	}

	return true;
}

#endif