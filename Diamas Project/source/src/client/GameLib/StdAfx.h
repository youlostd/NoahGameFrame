#ifndef METIN2_CLIENT_GAMELIB_STDAFX_H
#define METIN2_CLIENT_GAMELIB_STDAFX_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterBase/Utils.h"
#include <base/Crc32.hpp>
#include <effolkronium/random.hpp>

using Random = effolkronium::random_static;

#include "../eterLib/StdAfx.h"
#include "../milesLib/StdAfx.h"
#include "../effectLib/StdAfx.h"

#include "GameType.h"
#include "GameUtil.h"
#include "MapType.h"
#include "MapUtil.h"
#include "Interface.h"

#endif /* METIN2_CLIENT_GAMELIB_STDAFX_H */
