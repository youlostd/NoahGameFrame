# This file contains the compiler detection code and appends
# a set of default options to the CMAKE_<LANG>_FLAGS variables,
# that will be used by every target.
#
# It also defines a set of list variables that contain compiler-specific
# options that targets can use (some of them might end up being default
# target flags)
#
# List of configs:
#   - Disable RTTI
#     * METIN2_CONFIG_NORTTI_CFLAGS
#
#   - Disable exceptions
#     * METIN2_CONFIG_NOEXCEPTIONS_CFLAGS
#
#   - METIN2 Compiler warnings
#     * METIN2_CONFIG_WARNINGS_CFLAGS
#
#   - Code optimization
#     * METIN2_CONFIG_RELOPT_CFLAGS
#     * METIN2_CONFIG_RELOPT_LDFLAGS
#     * METIN2_CONFIG_DBGOPT_CFLAGS
#     * METIN2_CONFIG_DBGOPT_LDFLAGS
#
#   - Symbols
#     * METIN2_CONFIG_SYMBOLS_CFLAGS
#     * METIN2_CONFIG_SYMBOLS_LDFLAGS
#
#   - rpath (Unix only)
#     * METIN2_CONFIG_RPATH_LDFLAGS
#
#   - Runtime library (Windows only)
#     * METIN2_CONFIG_DBGCRT_CFLAGS
#     * METIN2_CONFIG_RELCRT_CFLAGS

set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	set(METIN2_IS_WIN TRUE)
	set(METIN2_IS_FREEBSD FALSE)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
	set(METIN2_IS_WIN FALSE)
	set(METIN2_IS_FREEBSD TRUE)
endif ()

if (${CMAKE_SIZEOF_VOID_P} MATCHES 4)
	set(METIN2_IS_X86 TRUE)
	set(METIN2_IS_AMD64 FALSE)
elseif (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
	set(METIN2_IS_X86 FALSE)
	set(METIN2_IS_AMD64 TRUE)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(METIN2_IS_MSVC TRUE)
	set(METIN2_IS_GCC FALSE)
	set(METIN2_IS_CLANG FALSE)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(METIN2_IS_MSVC FALSE)
	set(METIN2_IS_GCC TRUE)
	set(METIN2_IS_CLANG FALSE)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(METIN2_IS_MSVC FALSE)
	set(METIN2_IS_GCC FALSE)
	set(METIN2_IS_CLANG TRUE)
else ()
	message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER_ID}")
endif ()


# ThreadSanitizer
set(CMAKE_C_FLAGS_TSAN
    "-fsanitize=thread -g -O1"
    CACHE STRING "Flags used by the C compiler during ThreadSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_TSAN
    "-fsanitize=thread -g -O1"
    CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds."
    FORCE)

# AddressSanitize
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds."
    FORCE)

# MemorySanitizer
set(CMAKE_C_FLAGS_MSAN
    "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2"
    CACHE STRING "Flags used by the C compiler during MemorySanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_MSAN
    "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2"
    CACHE STRING "Flags used by the C++ compiler during MemorySanitizer builds."
    FORCE)

# UndefinedBehaviour
set(CMAKE_C_FLAGS_UBSAN
    "-fsanitize=undefined"
    CACHE STRING "Flags used by the C compiler during UndefinedBehaviourSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_UBSAN
    "-fsanitize=undefined"
    CACHE STRING "Flags used by the C++ compiler during UndefinedBehaviourSanitizer builds."
    FORCE)


option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options (-fdiagnostics-color=always)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_compile_options (-fcolor-diagnostics)
	endif ()
endif ()