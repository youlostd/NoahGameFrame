
SET(SOURCE_FILES 
    ComUtil.cpp
    ComUtil.hpp
    DiskSerial.cpp
    DiskSerial.hpp
    NetworkAdapters.cpp
    NetworkAdapters.hpp
)

metin2_add_static_library(metin2_winsup ${SOURCE_FILES})
target_link_libraries(metin2_winsup PUBLIC m2.shared oleaut32.lib wbemuuid.lib iphlpapi.lib)
target_include_directories(metin2_winsup PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:.>")