


set(SOURCE_FILES
	MainDialog.cpp
	MainDialog.h
	Metin2Config.cpp
	Metin2Config.rc
	resource.h
	ui.rc
)

metin2_add_executable(metin2tool_config config ${SOURCE_FILES})

