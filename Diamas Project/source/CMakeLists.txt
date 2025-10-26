cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
cmake_policy(SET CMP0091 OLD)
set(CRASHPAD_BUILD_TESTS ON)


set(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/buildtool/cmake"
)
set(METIN2_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32 AND NOT ANDROID)
set(HUNTER_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Hunter dependency build variants" FORCE)
else()
set(HUNTER_CONFIGURATION_TYPES "Release" CACHE STRING "Hunter dependency build variants" FORCE)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Setting build type to Debug as none was specified.")
	set(CMAKE_BUILD_TYPE Debug
    		CACHE STRING "Choose the type of build, options are: None Debug Release 		RelWithDebInfo MinSizeRel tsan asan lsan msan ubsan"
   		 FORCE)
else()
	set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
	    CACHE STRING "Choose the type of build, options are: None Debug Release 			RelWithDebInfo MinSizeRel tsan asan lsan msan ubsan"
	    FORCE)
endif()

#set(HUNTER_ROOT ".hunter") ## Set local path instead of global one
include(HunterGate)

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.24.0.tar.gz"
    SHA1 "a3d7f4372b1dcd52faa6ff4a3bd5358e1d0e5efd"
	LOCAL # load `buildtool/cmake/Hunter/config.cmake`

)

project(Diamas_Project LANGUAGES CXX C)
include(GNUInstallDirs)

## Hunter Packages

hunter_add_package(Boost COMPONENTS program_options filesystem)
hunter_add_package(Microsoft.GSL)
hunter_add_package(Eigen)
hunter_add_package(CreateLaunchers)

if(WIN32)
	hunter_add_package(freetype)
	find_package(freetype CONFIG REQUIRED) 

	hunter_add_package(harfbuzz)
	find_package(harfbuzz CONFIG REQUIRED)
endif()

hunter_add_package(Catch)
hunter_add_package(Microsoft.GSL)
hunter_add_package(fmt)
hunter_add_package(spdlog)
hunter_add_package(pegtl)
hunter_add_package(utf8)
hunter_add_package(xxhash)
hunter_add_package(cpp_redis)
hunter_add_package(flatbuffers)
hunter_add_package(MySQL-client)

find_package(MySQL-client REQUIRED)
find_package(Flatbuffers CONFIG REQUIRED)
find_package(cpp_redis CONFIG REQUIRED)
find_package(xxhash CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(pegtl CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(CreateLaunchers CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED program_options filesystem)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)

include(CreateLaunchers)


SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(METIN2_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(METIN2_PGO ${CMAKE_CURRENT_SOURCE_DIR}/data/pgo)
set(METIN2_BIN_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(METIN2_SHARE ${METIN2_ROOT}/src/share)
set(METIN2_3RDPARTY ${METIN2_ROOT}/thirdparty CACHE STRING "Metin2 3rd party path")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(METIN2_USE_PREFIX_HEADERS "Enable the use of precompiled headers" ON)
option(METIN2_USE_ASAN "Pass -fsanitize=address during Clang builds" OFF)
option(METIN2_USE_CYTHON "Build the client with cython" ON)
option(METIN2_BUILD_DOC "Build documentation" OFF)
option(METIN2_BUILD_NATIVE "Build with -march=native" OFF)



include(Compiler)
include(Config)

include(CommonTargets)
include(PythonTargets)

if(METIN2_IS_WIN)
include(cef_cmake)
endif()

include(${METIN2_3RDPARTY}/Config.cmake)

if(METIN2_BUILD_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

if (METIN2_IS_WIN)
	find_package(DirectX9 REQUIRED)

else ()
	find_package(Backtrace REQUIRED)
endif()

if(METIN2_BUILD_DOC)
	add_subdirectory(doc)
endif()

if(METIN2_USE_CYTHON)
	add_compile_definitions(ENABLE_CYTHON)
endif()

add_subdirectory(src)


if (METIN2_IS_WIN)
	add_custom_target(
		metin2 DEPENDS
		metin2client_run
		metin2server_db
		metin2server_game
		metin2server_auth
		metin2server_master
		metin2tool_qc
		metin2tool_makemotionproto
		metin2tool_dumpproto
	)

else ()
	add_custom_target(
		metin2 DEPENDS
		metin2server_db
		metin2server_game
		metin2server_auth
		metin2server_master
		metin2tool_qc
	)
	

	
endif ()

#install(DIRECTORY ${MYSQL_ROOT}/lib/ DESTINATION server)
