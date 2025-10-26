set(SOURCE_FILES
	${METIN2_SHARE}/StdioErrorHandlers.cpp

	Main.cpp
	Actions.cpp
	resource.rc
)

metin2_add_executable(metin2tool_dumpproto dump_proto ${SOURCE_FILES})
target_link_libraries(metin2tool_dumpproto PRIVATE 
	metin2_base
	metin2_xml
	metin2_game
)
