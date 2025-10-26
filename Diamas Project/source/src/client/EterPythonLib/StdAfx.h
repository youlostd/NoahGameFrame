#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <boost/functional/hash.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/locale/utf.hpp>

#include <list>
#include <algorithm>
#include <unordered_set>
#include <memory>
#include <list>

#include <base/Clipboard.hpp>
#include <base/Crc32.hpp>

#include "../EterLib/ResourceManager.h"
#include "../EterLib/Engine.hpp"
#include "../EterLib/FontManager.hpp"
#include "../EterLib/StateManager.h"
#include "../EterLib/KeyboardInput.hpp"
#include "../EterLib/TextTag.h"
#include "../EterLib/GrpBase.h"

#include "../ScriptLib/StdAfx.h"
#include "../eterBase/Timer.h"
#include "../eterBase/CRC32.h"
#include "../eterBase/Stl.h"

#include "PythonWindowManager.h"
#include "PythonGraphic.h"

#undef BYTE
#include <../shared/dxgitype.h> // We need the windows sdk version of dxgitype
#include <wincodec.h>

void ImageCapsuleDestroyer(PyObject *capsule);