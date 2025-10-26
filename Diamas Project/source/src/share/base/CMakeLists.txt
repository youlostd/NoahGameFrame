set(source_files
	ClientDb.hpp
	Console.cpp
	Console.hpp
	Crc32.cpp
	Crc32.hpp
	DumpVersion.cpp
	DumpVersion.hpp
	Exception.hpp
	GroupText.hpp
	GroupTextTree.cpp
	GroupTextTree.hpp
	GroupTextTreeUtil.cpp
	GroupTextTreeUtil.hpp
	PrefixHeader.hpp
	Random.cpp
	Random.hpp
	RefCounted.hpp
	ResponseFile.cpp
	ResponseFile.hpp
	Serialization.cpp
	Serialization.hpp
	SimpleApp.cpp
	SimpleApp.hpp
	Singleton.hpp
	Remotery.h
	Remotery.c
)

if (WIN32)
	list(APPEND source_files Clipboard.cpp)
	list(APPEND source_files Clipboard.hpp)
	list(APPEND source_files SimpleMath.cpp)
	list(APPEND source_files SimpleMath.h)
	list(APPEND source_files SimpleMath.inl)
endif ()

set(link_libs
	spdlog::spdlog
	Microsoft.GSL::GSL
	Eigen3::Eigen
	)

if (WIN32)
	list(APPEND link_libs d3d9.lib)
	list(APPEND link_libs d3dx9::d3dx9)
endif ()


metin2_add_static_library(metin2_base ${source_files})
target_link_libraries(metin2_base PRIVATE ${link_libs} )
