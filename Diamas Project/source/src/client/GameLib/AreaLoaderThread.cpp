#include "AreaLoaderThread.h"
#include "AreaTerrain.h"
#include "MapOutdoor.h"
#include <mutex>
#include <fmt/printf.h>

TEMP_CAreaLoaderThread::TEMP_CAreaLoaderThread() : m_bShutdowned(false)
{
}

TEMP_CAreaLoaderThread::~TEMP_CAreaLoaderThread()
{
    Destroy();
}

bool TEMP_CAreaLoaderThread::Create(void *arg)
{
    assert(!m_asyncThread.joinable() && "Thread must not be joinable, "
                                        "otherwise terminate() is called");
    auto f = [this]() {
        ProcessArea();
    };
    std::thread thr(f);
    std::swap(m_asyncThread, thr);

    return true;
}

void TEMP_CAreaLoaderThread::Destroy()
{
    m_bShutdowned = true;
    m_semaphore.Clear();

    if (m_asyncThread.joinable())
        m_asyncThread.join();
}

void TEMP_CAreaLoaderThread::Shutdown()
{
    m_bShutdowned = true;
    m_semaphore.Release();

    if (m_asyncThread.joinable())
        m_asyncThread.join();
}

std::queue<CArea *> TEMP_CAreaLoaderThread::PopPendingRequests()
{
    std::lock_guard<std::mutex> lock(m_AreaRequestMutex);

    std::queue<CArea *> queue;
    swap(queue, m_pAreaRequestDeque);

    return queue;
}

void TEMP_CAreaLoaderThread::PushResult(CArea *p)
{
    std::lock_guard<std::mutex> lock(m_resultMutex);
    m_queue_result.push(p);
}

void TEMP_CAreaLoaderThread::Request(CArea *pArea)
{
    {
        std::lock_guard<std::mutex> lock(m_AreaRequestMutex);
        m_pAreaRequestDeque.push(pArea);
    }

    m_semaphore.Release();
}

bool TEMP_CAreaLoaderThread::Fetch(CArea **ppArea)
{

    std::lock_guard<std::mutex> lock(m_resultMutex);

    if (m_queue_result.empty())
        return false;

    *ppArea = m_queue_result.front();
    m_queue_result.pop();
    return true;
}

void TEMP_CAreaLoaderThread::ProcessArea()
{
    while (!m_bShutdowned)
    {
        m_semaphore.Wait();
        auto queue = PopPendingRequests();

        while (!queue.empty())
        {
            auto *p = queue.front();
             
            WORD wAreaCoordX, wAreaCoordY;
            p->GetCoordinate(&wAreaCoordX, &wAreaCoordY);
            DWORD dwID = (DWORD)(wAreaCoordX)*1000L + (DWORD)(wAreaCoordY);

            const std::string &c_rStrMapName = p->GetOwner()->GetName();

            p->Load(fmt::sprintf( "%s\\%06u\\", c_rStrMapName.c_str(), dwID).c_str());

            queue.pop();

            PushResult(p);
        }

    }


    auto queue = PopPendingRequests();
    while (!queue.empty())
    {
        auto p = queue.front();

        DWORD dwStartTime = ELTimer_GetMSec();
        WORD wAreaCoordX, wAreaCoordY;
        p->GetCoordinate(&wAreaCoordX, &wAreaCoordY);
        DWORD dwID = (DWORD)(wAreaCoordX)*1000L + (DWORD)(wAreaCoordY);

        const std::string &c_rStrMapName = p->GetOwner()->GetName();

        char szAreaPathName[64 + 1];
        _snprintf(szAreaPathName, sizeof(szAreaPathName), "%s\\%06u\\", c_rStrMapName.c_str(), dwID);

        p->Load(szAreaPathName);


        queue.pop();

        PushResult(p);
    }
}
