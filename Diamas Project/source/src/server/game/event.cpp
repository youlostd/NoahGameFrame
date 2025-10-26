/*
 *    Filename: event.c
 * Description: ÀÌº¥Æ® °ü·Ã (timed event)
 *
 *      Author: ±èÇÑÁÖ (aka. ºñ¿±, Cronan), ¼Û¿µÁø (aka. myevan, ºøÀÚ·ç)
 */

#include "event_queue.h"

static CEventQueue event_queue;

/* 이벤트를 생성하고 리턴한다 */
LPEVENT event_create_ex(TEVENTFUNC func, event_info_data *info, int32_t when)
{
    LPEVENT new_event = nullptr;

    /* Be sure to call after the next pulse or more. */
    if (when < 1)
        when = 1;

    new_event = new event;

    assert(NULL != new_event);

    new_event->func = func;
    new_event->info = info;
    new_event->q_el = event_queue.Enqueue(new_event, when, thecore_heart->pulse);
    new_event->is_processing = false;
    new_event->is_force_to_end = false;

    return new_event;
}

/* 시스템으로 부터 이벤트를 제거한다 */
void event_cancel(LPEVENT *ppevent)
{
    LPEVENT ev;

    if (!ppevent)
        return;

    if (!((ev = *ppevent)))
        return;

    if (ev->is_processing)
    {
        ev->is_force_to_end = true;

        if (ev->q_el)
            ev->q_el->bCancel = true;

        *ppevent = nullptr;
        return;
    }

    // 이미 취소 되었는가?
    if (!ev->q_el)
    {
        *ppevent = nullptr;
        return;
    }

    if (ev->q_el->bCancel)
    {
        *ppevent = nullptr;
        return;
    }

    ev->q_el->bCancel = true;

    *ppevent = nullptr;
}

void event_reset_time(LPEVENT event, int32_t when)
{
    if (!event->is_processing)
    {
        if (event->q_el)
            event->q_el->bCancel = true;

        event->q_el = event_queue.Enqueue(event, when, thecore_heart->pulse);
    }
}

/* 이벤트를 실행할 시간에 도달한 이벤트들을 실행한다 */
int event_process(int pulse)
{
    rmt_ScopedCPUSample(EventProcess, 0);

    int32_t new_time;
    int num_events = 0;

    // event_q 즉 이벤트 큐의 헤드의 시간보다 현재의 pulse 가 적으면 루프문이
    // 돌지 않게 된다.
    while (pulse >= event_queue.GetTopKey())
    {
        TQueueElement *pElem = event_queue.Dequeue();

        if (pElem->bCancel)
        {
            event_queue.Delete(pElem);
            continue;
        }

        new_time = pElem->iKey;

        LPEVENT the_event = pElem->pvData;
        int32_t processing_time = event_processing_time(the_event);

        event_queue.Delete(pElem);
        the_event->q_el = nullptr;

        /*
        * 리턴 값은 새로운 시간이며 리턴 값이 0 보다 클 경우 이벤트를 다시 추가한다.
        * 리턴 값을 0 이상으로 할 경우 event 에 할당된 메모리 정보를 삭제하지 않도록
        * 주의한다.
        */
        the_event->is_processing = true;

        assert(the_event->info);

        new_time = (the_event->func)(the_event, processing_time);
        if (new_time <= 0 || the_event->is_force_to_end) { the_event->q_el = nullptr; }
        else
        {
            the_event->q_el = event_queue.Enqueue(the_event, new_time, pulse);
            the_event->is_processing = false;
        }

        ++num_events;
    }

    return num_events;
}

/* Event returns execution time in pulse units  */
int32_t event_processing_time(LPEVENT event)
{
    int32_t start_time;

    if (!event->q_el)
        return 0;

    start_time = event->q_el->iStartTime;
    return (thecore_heart->pulse - start_time);
}

/* Returns the remaining time of the event in pulse units.  */
int32_t event_time(LPEVENT event)
{
    int32_t when;

    if (!event->q_el)
        return 0;

    when = event->q_el->iKey;
    return (when - thecore_heart->pulse);
}

/* 모든 이벤트를 제거한다 */
void event_destroy()
{
    TQueueElement *pElem;

    while ((pElem = event_queue.Dequeue()))
    {
        LPEVENT the_event = (LPEVENT)pElem->pvData;

        if (!pElem->bCancel)
        {
            // no op here
        }

        event_queue.Delete(pElem);
    }
}

int event_count() { return event_queue.Size(); }

void intrusive_ptr_add_ref(EVENT *p) { ++(p->ref_count); }

void intrusive_ptr_release(EVENT *p) { if (--(p->ref_count) == 0) { delete p; } }
