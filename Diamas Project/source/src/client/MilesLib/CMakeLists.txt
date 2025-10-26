set(SOURCE_FILES
	MusicInstance.cpp
	MusicInstance.hpp
	SampleFile.cpp
	SampleFile.hpp
	SampleFileCache.cpp
	SampleFileCache.hpp
	SoundManager.cpp
	SoundManager.h
	SoundSample.hpp
	SoundSample.cpp
	SoundStream.cpp
	SoundStream.hpp
	SoundVfs.cpp
	SoundVfs.hpp
	Type.cpp
	Type.h
)

add_library(metin2client_miles STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_miles PRIVATE spdlog::spdlog effolkronium_random metin2client_lib PUBLIC miles)
target_precompile_headers(metin2client_miles PRIVATE "StdAfx.h" "StdAfx.cpp")
