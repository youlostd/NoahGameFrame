set(SOURCE_FILES
	${METIN2_SHARE}/StdioErrorHandlers.cpp

	GrannyScene.cpp
	GrannyRuntime.cpp
	GrannyUtil.cpp
	Race.cpp
	Motion.cpp
	MotionList.cpp
	MotionManager.cpp
	XmlWriter.cpp

	Main.cpp
	resource.rc
)

add_definitions(-DBUILDING_GRANNY_STATIC)

metin2_add_executable(metin2tool_makemotionproto make_motion_proto ${SOURCE_FILES})
target_link_libraries(metin2tool_makemotionproto PRIVATE metin2_base metin2_game PUBLIC granny2)
