set(SOURCE_FILES
	Terrain.cpp
	TextureSet.cpp
)

add_library(metin2client_terrain STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_terrain PRIVATE spdlog::spdlog effolkronium_random metin2client_lib)
target_precompile_headers(metin2client_terrain PRIVATE "StdAfx.h" "StdAfx.cpp")
