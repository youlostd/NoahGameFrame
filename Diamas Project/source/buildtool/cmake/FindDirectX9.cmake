# Find DirectX9

message("Looking for DirectX9...")

set(DIRECTX_INCLUDE_DIRS
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.*.0/um"
        "C:/Program Files (x86)/Windows Kits/10/Include/10.0.*.0/shared"
        )

SET(DX9_LIBRARY_PATHS
        "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64/"
        "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64/"
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.14393.0/um/x64/"
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.10586.0/um/x64/"
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.15063.0/um/x64/"
		"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.16299.0/um/x64"
		"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17134.0/um/x64"
		"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x64"
		"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64"
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.28.29333/lib/x64"
        )

find_library(DIRECT3D9_LIBRARY NAMES d3d9 PATHS ${DX9_LIBRARY_PATHS} NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH)

message("Include: ${DIRECTX_INCLUDE_DIRS}")
message("DIRECT3D9_LIBRARY: ${DIRECT3D9_LIBRARY}")

mark_as_advanced(DIRECTX_INCLUDE_DIRS DIRECT3D9_LIBRARY)