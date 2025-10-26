/*
 *    Filename: signal.c
 * Description: 시그널 관련 함수.
 *
 *      Author: 비엽 aka. Cronan
 */
#define __LIBTHECORE__
#include <thecore/signal.hpp>
#include <thecore/main.hpp>
#include <thecore/utils.hpp>
#include <SpdLog.hpp>

#if !defined(_WIN32)

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define RETSIGTYPE void

RETSIGTYPE reap(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	signal(SIGCHLD, reap);
}

RETSIGTYPE hupsig(int sig)
{
	thecore_shutdown();
	spdlog::error( "SIGHUP, SIGINT, SIGTERM signal has been received. "
	                 "shutting down.");
}

RETSIGTYPE usrsig(int sig)
{
	core_dump();
}

void signal_setup(void)
{
	// just to be on the safe side
	signal(SIGHUP, hupsig);
	signal(SIGCHLD, reap);
	signal(SIGINT, hupsig);
	signal(SIGTERM, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	signal(SIGUSR1, usrsig);
}

#else

void signal_setup()
{
	// nop
}

#endif
