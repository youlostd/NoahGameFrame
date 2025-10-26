# METIN2 Configuration file
# Defines constants used by the whole build-process.
# Depends on Compiler.cmake

# Target output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${METIN2_BIN_ROOT}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${METIN2_BIN_ROOT}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${METIN2_BIN_ROOT}/lib)

set(Random_BuildTests OFF)
set(BUILD_TESTING OFF)
set(CATCH_BUILD_TESTING OFF)
set(CATCH_INSTALL_DOCS OFF)
set(UTF8_TESTS OFF)
set(UTF8_SAMPLES OFF)
set(RAPIDXML_BUILD_TESTS OFF)
set(GSL_TEST OFF)
set(CEF_USE_SANDBOX OFF)

