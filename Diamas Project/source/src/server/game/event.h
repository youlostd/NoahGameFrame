/*
 *    Filename: event.h
 * Description: �̺�Ʈ ���� (timed event)
 *
 *      Author: ������ (aka. ��, Cronan), �ۿ��� (aka. myevan, ���ڷ�)
 */

#ifndef METIN2_SERVER_GAME_EVENT_H
#define METIN2_SERVER_GAME_EVENT_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <boost/intrusive_ptr.hpp>

/**
* Base class for all event info data
*/
struct event_info_data
{
    event_info_data()
    {
    }

    virtual ~event_info_data()
    {
    }
};

typedef struct event EVENT;
typedef boost::intrusive_ptr<EVENT> LPEVENT;
typedef int32_t (*TEVENTFUNC)(LPEVENT event, int32_t processing_time);

#define EVENTFUNC(name)	int32_t (name) (LPEVENT event, int32_t processing_time)
#define EVENTINFO(name) struct name : public event_info_data

struct TQueueElement;

struct event
{
    event() = default;

    ~event()
    {
        if (info != nullptr)
            delete info;
    }

    TEVENTFUNC func{};
    event_info_data *info{};
    TQueueElement *q_el{};
    char is_force_to_end{};
    char is_processing{};
    size_t ref_count{};
};

extern void intrusive_ptr_add_ref(EVENT *p);
extern void intrusive_ptr_release(EVENT *p);

template <class T> // T should be a subclass of event_info_data
T *AllocEventInfo() { return new T; }

extern void event_destroy();
extern int event_process(int pulse);
extern int event_count();

#define event_create(func, info, when) event_create_ex(func, info, when)
extern LPEVENT event_create_ex(TEVENTFUNC func, event_info_data *info, int32_t when);
extern void event_cancel(LPEVENT *event);                  // 이벤트 취소
extern int32_t event_processing_time(LPEVENT event);       // 수행 시간 리턴
extern int32_t event_time(LPEVENT event);                  // 남은 시간 리턴
extern void event_reset_time(LPEVENT event, int32_t when); // 실행 시간 재 설정
extern void event_set_verbose(int level);

extern event_info_data *FindEventInfo(uint32_t dwID);
extern event_info_data *event_info(LPEVENT event);

#endif /* METIN2_SERVER_GAME_EVENT_H */
