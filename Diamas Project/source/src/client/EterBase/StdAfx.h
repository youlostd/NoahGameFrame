
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <cassert>
#include <cstdio>
#include <ctime>
#include <algorithm>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <list>
#include <map>

#if _MSC_VER >= 1400
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strupt _strupr
#define strcmpi _strcmpi
#define fileno _fileno
//#define access _access_s
//#define _access _access_s
#endif

#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;


#include <SpdLog.hpp>
#include <Direct3D.hpp>

#include "Filename.h"
#include "ServiceDefs.h"
#include <base/Singleton.hpp>