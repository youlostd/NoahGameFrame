#ifndef METIN2_SERVER_GAME_EVENT_QUEUE_H
#define METIN2_SERVER_GAME_EVENT_QUEUE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef M2_USE_POOL
#include "pool.h"
#endif

#include "stable_priority_queue.h"

struct TQueueElement
{
    LPEVENT pvData;
    int iStartTime;
    int iKey;
    bool bCancel;
};

class CEventQueue
{
public:
    struct FuncQueueComp
    {
        bool operator ()(TQueueElement *left, TQueueElement *right) const { return (left->iKey > right->iKey); }
    };

public:
    CEventQueue();
    ~CEventQueue();

    TQueueElement *Enqueue(LPEVENT data, int duration, int pulse);
    TQueueElement *Dequeue();
    void Delete(TQueueElement *pElem);
    void Requeue(TQueueElement *, int key);
    int GetTopKey();
    int Size();

protected:
    void Destroy();

private:
    stable_priority_queue<TQueueElement *, std::vector<TQueueElement *>, FuncQueueComp> m_pq_queue;

#ifdef M2_USE_POOL
		ObjectPool<TQueueElement> pool_;
#endif
};

#endif /* METIN2_SERVER_GAME_EVENT_QUEUE_H */
