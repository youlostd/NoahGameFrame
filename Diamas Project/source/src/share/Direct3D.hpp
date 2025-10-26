#ifndef METIN2_DIRECT3D_HPP
#define METIN2_DIRECT3D_HPP

#include <Config.hpp>

#ifdef DEBUG
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb173355%28v=vs.85%29.aspx
#define D3D_DEBUG_INFO
#endif

#include <d3d9.h>
#include <d3d9types.h>
#include <base/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

#endif
