set(SOURCE_FILES
	LzoConfig.cpp
	LzoCompressor.cpp
)

metin2_add_static_library(metin2_lzo ${SOURCE_FILES})
target_link_libraries(metin2_lzo PUBLIC lzo-pro)
