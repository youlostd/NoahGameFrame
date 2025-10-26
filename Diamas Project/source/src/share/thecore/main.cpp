/*
 *    Filename: main.c
 * Description: 라이브러리 초기화/삭제 등
 *
 *      Author: 비엽 aka. Cronan
 */
#define __LIBTHECORE__
#include "stdafx.h"
#include "main.hpp"

LPHEART thecore_heart = NULL;

namespace
{

bool isShutdown = false;
}

uint64_t SecsToTicks(int secs) {
    std::chrono::seconds sec(secs);
    return     std::chrono::duration_cast<std::chrono::microseconds>(sec).count() / 25;
}
uint64_t MSecsToTicks(int secs) {
    std::chrono::milliseconds msec(secs);
    return     std::chrono::duration_cast<std::chrono::microseconds>(msec).count() / 25;
}

bool thecore_init(int fps)
{
	thecore_heart = heart_new(1000000 / fps);
	return true;
}

void thecore_shutdown()
{
	isShutdown = true;
}

int thecore_idle(HeartIdleFunc idle_func)
{
	if (!isShutdown)
		return heart_idle(thecore_heart, idle_func);

	return 0;
}

void thecore_destroy()
{
	// noop
}

int thecore_pulse()
{
	return thecore_heart->pulse;
}

float thecore_pulse_per_second()
{
	return (float)thecore_heart->passes_per_sec;
}

float thecore_time()
{
	return (float)thecore_heart->pulse / (float)thecore_heart->passes_per_sec;
}

int thecore_is_shutdowned()
{
	return isShutdown;
}
