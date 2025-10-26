#ifndef __INC_LIBTHECORE_STDAFX_H__
#define __INC_LIBTHECORE_STDAFX_H__

#if defined(__GNUC__)
#define INLINE __inline__
#elif defined(_MSC_VER)
#define INLINE inline
#endif

#ifdef _MSC_VER
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef NOMINMAX
#define NOMINMAX //remove min/max macros since for windows they are already provided by std
#endif

#include <windows.h>
#include <winsock2.h>
#include <tchar.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <conio.h>
#include <process.h>
#include <climits>
#include <math.h>
#include <locale.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <cstdint>
#include "xdirent.hpp"
#include "xgetopt.hpp"

#define S_ISDIR(m)	(m & _S_IFDIR)


#define __USE_SELECT__

#define PATH_MAX _MAX_PATH

// C runtime library adjustments
#define strlcpy(dst, src, size) strncpy_s(dst, size, src, _TRUNCATE)
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
#define atoll(str) _atoi64(str)
// #define localtime_r(timet, result) localtime_s(result, timet)
#define strtok_r(s, delim, ptrptr) strtok_s(s, delim, ptrptr)


// dummy declaration of non-supported signals
#define SIGUSR1     30  /* user defined signal 1 */
#define SIGUSR2     31  /* user defined signal 2 */

inline void usleep(unsigned long usec) {
	::Sleep(usec / 1000);
}
inline unsigned sleep(unsigned sec) {
	::Sleep(sec * 1000);
	return 0;
}
inline double rint(double x)
{
	return ::floor(x+.5);
}


#else

#ifndef __FreeBSD__
#define __USE_SELECT__
#ifdef __CYGWIN__
#define _POSIX_SOURCE 1
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include <sys/signal.h>
#include <sys/wait.h>

#include <pthread.h>
#include <semaphore.h>

#ifdef __FreeBSD__
#include <sys/event.h>
#endif

#endif

#include <cstring>
#include <cstdio>

#include "typedef.h"
#include "heart.hpp"
#include "fdwatch.hpp"
#include "socket.hpp"
#include "buffer.hpp"
#include "signal.hpp"
#include "log.hpp"
#include "main.hpp"
#include "utils.hpp"

#endif // __INC_LIBTHECORE_STDAFX_H__
