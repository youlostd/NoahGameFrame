#ifndef __INC_LIBTHECORE_MAIN_H__
#define __INC_LIBTHECORE_MAIN_H__

#include "heart.hpp"
#include "utils.hpp"
#include <chrono>
typedef std::chrono::duration<float, std::ratio<1,25>> hz25;

uint64_t SecsToTicks(int secs);
uint64_t MSecsToTicks(int secs);

#define THECORE_SECS_TO_PASSES(secs) ((secs) * thecore_heart->passes_per_sec)
#define THECORE_MSECS_TO_PASSES(msecs) ((msecs) * thecore_heart->passes_per_sec / 1000)

extern LPHEART thecore_heart;

extern bool thecore_init(int fps);
extern int thecore_idle(HeartIdleFunc idle_func = thecore_sleep);
extern void thecore_shutdown();
extern void thecore_destroy();
extern int thecore_pulse();
extern float thecore_time();
extern float thecore_pulse_per_second();
extern int thecore_is_shutdowned();


#endif
