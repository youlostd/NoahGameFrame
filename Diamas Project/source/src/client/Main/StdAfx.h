#ifndef METIN2_CLIENT_MAIN_STDAFX_HPP
#define METIN2_CLIENT_MAIN_STDAFX_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#if defined(_DEBUG) || defined(DEBUG)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#include "../EterLib/StdAfx.h"
#include "../ScriptLib/StdAfx.h"
#include "../EterPythonLib/StdAfx.h"
#include "../GameLib/StdAfx.h"
#include "../MilesLib/Stdafx.h"
#include "../EffectLib/StdAfx.h"
#include "../SpeedTreeLib/StdAfx.h"

#include "Locale.h"

#include "GameType.h"

#define APP_NAME	"Metin 2"

#endif
