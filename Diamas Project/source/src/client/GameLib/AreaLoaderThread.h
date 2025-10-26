#ifndef AREAD_LOADER_THREAD_HPP
#define AREAD_LOADER_THREAD_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include <mutex>

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>

class CSemaphore
{
  public:
    CSemaphore()
    {
        m_sem = 0;
        Initialize();
    }
    ~CSemaphore()
    {
        Destroy();
    }

    bool Initialize()
    {
        Clear();

        m_sem = ::CreateSemaphore(NULL, 0, 32, NULL);
        if (m_sem == NULL)
            return false;

        return true;
    }
    void Clear()
    {
        if (m_sem)
        {
            ::CloseHandle(m_sem);
            m_sem = nullptr;
        }
    }
    void Destroy()
    {
        Clear();
    }

    bool Wait()
    {
        STORM_ASSERT(m_sem, "Not initialized");

        uint32_t dwWaitResult = ::WaitForSingleObject(m_sem, INFINITE);
        switch (dwWaitResult)
        {
        case WAIT_OBJECT_0:
            return true;

        default:
            break;
        }

        return false;
    }
    bool Release(int count = 1)
    {
        STORM_ASSERT(m_sem, "Not initialized");

        ::ReleaseSemaphore(m_sem, count, NULL);
        return true;
    }

  private:
#if defined(_WIN32)
    void *m_sem;
#else
    sem_t m_sem;
#endif
};

class CTerrain;
class CArea;

class TEMP_CAreaLoaderThread
{
  public:
    TEMP_CAreaLoaderThread();
    virtual ~TEMP_CAreaLoaderThread();

    bool Create(void *arg);
    void Shutdown();
    std::queue<CArea *> PopPendingRequests();
    void PushResult(CArea *p);
    void Request(CArea *pArea);
    bool Fetch(CArea **ppArea);

  private:
    std::thread m_asyncThread;

  protected:
    void Destroy();
    void ProcessArea();

  private:
    std::queue<CArea *> m_pAreaRequestDeque;
    std::mutex m_AreaRequestMutex;

    std::queue<CArea *> m_queue_result;
    std::mutex m_resultMutex;

    CSemaphore m_semaphore;

    int m_iRestSemCount;
    std::atomic<bool> m_bShutdowned;
};

#endif