set(SOURCE_FILES
	${METIN2_SHARE}/StdioErrorHandlers.cpp

	Main.cpp
	Main.hpp
	Actions.cpp
	resource.rc
)

metin2_add_executable(metin2tool_formatproto format_proto ${SOURCE_FILES})
target_link_libraries(metin2tool_formatproto PRIVATE metin2_base metin2_xml metin2_game)
