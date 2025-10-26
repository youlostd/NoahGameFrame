/*
 *    Filename: utils.c
 * Description: ¢®¨¡¢®EA¡§u A?¡§¢®¡Ë?¡Ë¡þ¡Ëc¡§¢®¡§u
 *
 *      Author: ¡§¡þn¡Ë?¢®¨ú aka. Cronan
 */
#define __LIBTHECORE__
#include "stdafx.h"
#include <cctype>
#include <chrono>
#include <cstring>
#include <random>
#include <SpdLog.hpp>

static struct timeval null_time = {0, 0};

#define ishprint(x) ((((x)&0xE0) > 0x90) || isprint(x))

struct timeval *timediff(const struct timeval *a, const struct timeval *b)
{
    static struct timeval rslt;

    if (a->tv_sec < b->tv_sec)
        return &null_time;
    else if (a->tv_sec == b->tv_sec)
    {
        if (a->tv_usec < b->tv_usec)
            return &null_time;
        else
        {
            rslt.tv_sec = 0;
            rslt.tv_usec = a->tv_usec - b->tv_usec;
            return &rslt;
        }
    }
    else
    { /* a->tv_sec > b->tv_sec */
        rslt.tv_sec = a->tv_sec - b->tv_sec;

        if (a->tv_usec < b->tv_usec)
        {
            rslt.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
            rslt.tv_sec--;
        }
        else
            rslt.tv_usec = a->tv_usec - b->tv_usec;

        return &rslt;
    }
}

struct timeval *timeadd(struct timeval *a, struct timeval *b)
{
    static struct timeval rslt;

    rslt.tv_sec = a->tv_sec + b->tv_sec;
    rslt.tv_usec = a->tv_usec + b->tv_usec;

    while (rslt.tv_usec >= 1000000)
    {
        rslt.tv_usec -= 1000000;
        rslt.tv_sec++;
    }

    return &rslt;
}

char *time_str(time_t ct)
{
    static char *time_s;

    time_s = asctime(localtime(&ct));

    time_s[strlen(time_s) - 6] = '\0';
    time_s += 4;

    return (time_s);
}

void trim_and_lower(const char *src, char *dest, size_t dest_size)
{
    const char *tmp = src;
    size_t len = 0;

    if (!dest || dest_size == 0)
        return;

    if (!src)
    {
        *dest = '\0';
        return;
    }

    // Skip before blank
    while (*tmp)
    {
        if (!isspace(static_cast<uint8_t>(*tmp)))
            break;

        tmp++;
    }

    // Secure \0
    --dest_size;

    while (*tmp && len < dest_size)
    {
        *(dest++) = LOWER(*tmp); // LOWER should not be a macro ++.
        ++tmp;
        ++len;
    }

    *dest = '\0';

    if (len > 0)
    {
        // Clear the space behind
        --dest;

        while (*dest && isspace(static_cast<uint8_t>(*dest)) && len--)
            *(dest--) = '\0';
    }
}

void lower_string(const char *src, char *dest, size_t dest_size)
{
    const char *tmp = src;
    size_t len = 0;

    if (!dest || dest_size == 0)
        return;

    if (!src)
    {
        *dest = '\0';
        return;
    }

    // \0 E¡Ëc¡§¡þ¡Ë¡þ
    --dest_size;

    while (*tmp && len < dest_size)
    {
        *(dest++) = LOWER(*tmp); // LOWER¡Ë¡ÍA ¡Ë¡þAA¡§I¢®¢´I¡ËOo ++ ¡§u¨Ï¡À¡Ë¡þe ¡§uE¡ÍiE
        ++tmp;
        ++len;
    }

    *dest = '\0';
}

char *str_dup(const char *source)
{
    char *new_line;

    CREATE(new_line, char, strlen(source) + 1);
    return (strcpy(new_line, source));
}

int filesize(FILE *fp)
{
    int pos;
    int size;

    pos = ftell(fp);

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, pos, SEEK_SET);

    return (size);
}

#ifdef _WIN32
int MAX(int a, int b)
{
    return a > b ? a : b;
}
#endif

#ifndef _WIN32
void thecore_sleep(struct timeval *timeout)
{
    if (select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, timeout) < 0)
    {
        if (errno != EINTR)
        {
            spdlog::error("select sleep {0}", (errno));
            return;
        }
    }
}

void thecore_msleep(uint32_t dwMillisecond)
{
    static struct timeval tv_sleep;
    tv_sleep.tv_sec = dwMillisecond / 1000;
    tv_sleep.tv_usec = dwMillisecond * 1000;
    thecore_sleep(&tv_sleep);
}

void core_dump_unix(const char *who, long line)
{
    spdlog::error("*** Dumping Core %s:%ld ***", who, line);

    fflush(stdout);
    fflush(stderr);

    if (fork() == 0)
        abort();
}

/*
uint64_t rdtsc()
{
    uint64_t x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}
*/

#else

void thecore_sleep(struct timeval *timeout)
{
    Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
}

void thecore_msleep(uint32_t dwMillisecond)
{
    Sleep(dwMillisecond);
}

void gettimeofday(struct timeval *t, struct timezone *dummy)
{
    uint64_t millisec = GetTickCount64();

    t->tv_sec = (millisec / 1000);
    t->tv_usec = (millisec % 1000) * 1000;
}

void core_dump_unix(const char *who, long line)
{
    spdlog::error("*** Attempting a linux core dump %s:%ld ***", who, line);
}

#endif

using FpSeconds = std::chrono::duration<float, std::chrono::milliseconds::period>;


static const std::chrono::steady_clock::time_point &get_boot_time()
{
    static std::chrono::steady_clock::time_point bootTime = std::chrono::steady_clock::now();

    return bootTime;
}

float get_float_time()
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<FpSeconds>(now - get_boot_time()).count();
}

uint64_t get_dword_time()
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - get_boot_time()).count();
}

void stl_lowers(std::string &rstRet)
{
    for (size_t i = 0; i < rstRet.length(); ++i)
        rstRet[i] = tolower(rstRet[i]);
}