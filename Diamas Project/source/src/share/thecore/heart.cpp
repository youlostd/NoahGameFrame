﻿/*
 *    Filename: heart.c
 * Description: fps ¿¡ ÇÑ¹ø¾¿ È£ÃâµÇ´Â "½ÉÀå" ÀÌ´Ù.
 *
 *      Author: ºñ¿± aka. Cronan
 */
#define __LIBTHECORE__
#include "heart.hpp"
#include "utils.hpp"
#ifdef __FreeBSD__
     #include <sys/time.h>
#endif


LPHEART heart_new(int opt_usec)
{
	LPHEART ht = new HEART;

	gettimeofday(&ht->before_sleep, (struct timezone*)0);

	ht->opt_time.tv_sec = 0;
	ht->opt_time.tv_usec = opt_usec;

	gettimeofday(&ht->last_time, (struct timezone*)0);

	ht->passes_per_sec = 1000000 / opt_usec;
	ht->pulse = 0;

	return ht;
}

void heart_delete(LPHEART ht)
{
	delete ht;
}

int heart_idle(LPHEART ht, HeartIdleFunc idle_func)
{
	struct timeval now, process_time, timeout, temp_time;
	int missed_pulse;

	gettimeofday(&ht->before_sleep, (struct timezone*)0);
	process_time = *timediff(&ht->before_sleep, &ht->last_time);

	/*
	 * If we were asleep for more than one pass, count missed pulses and sleep
	 * until we're resynchronized with the next upcoming pulse.
	 */
	if (process_time.tv_sec == 0 &&
	    process_time.tv_usec < ht->opt_time.tv_usec) {
		missed_pulse = 0;
	} else {
		missed_pulse = process_time.tv_sec * ht->passes_per_sec;
		missed_pulse += process_time.tv_usec / ht->opt_time.tv_usec;
	}

	// 바빠서 pulse도 놓쳤는데 잘 시간이 어딨어...
	// 펄스 fps 어차피 틀어져있는데, 정확히 맞추는 건 중요하지 않아.
	if (missed_pulse > 0) {
		gettimeofday(&ht->last_time, (struct timezone*)0);
	} else {
		/* Calculate the time we should wake up */
		temp_time = *timediff(&ht->opt_time, &process_time);
		ht->last_time = *timeadd(&ht->before_sleep, &temp_time);

		/* Now keep sleeping until that time has come */
		gettimeofday(&now, (struct timezone*)0);
		timeout = *timediff(&ht->last_time, &now);

		idle_func(&timeout);
	}

	++missed_pulse;

	if (missed_pulse <= 0) {
		spdlog::error( "missed_pulse is not positive! ({0})", missed_pulse);
		missed_pulse = 1;
	}

	if (missed_pulse > (30 * ht->passes_per_sec)) {
		spdlog::error( "losing {0} seconds. (lag occured)",
		          missed_pulse / ht->passes_per_sec);
		missed_pulse = 30 * ht->passes_per_sec;
	}

	return missed_pulse;
}