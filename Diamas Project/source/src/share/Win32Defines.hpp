#ifndef METIN2_WIN32DEFINES_HPP
#define METIN2_WIN32DEFINES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS


#ifndef S_ISDIR
#define S_ISDIR(m)	(m & _S_IFDIR)
#endif


#define PATH_MAX _MAX_PATH

// dummy declaration of non-supported signals
#define SIGUSR1     30
#define SIGUSR2     31



METIN2_END_NS

#endif
