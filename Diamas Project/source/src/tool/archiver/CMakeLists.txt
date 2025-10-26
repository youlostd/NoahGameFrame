set(SOURCE_FILES
	${METIN2_SHARE}/StdioErrorHandlers.cpp

	Main.cpp
	Main.hpp
	XmlUtil.hpp
	resource.rc
)

metin2_add_executable(metin2tool_archiver archiver ${SOURCE_FILES})
target_link_libraries(metin2tool_archiver PRIVATE 
	metin2_base
	metin2_xml
	metin2_game
	metin2_pak
)
