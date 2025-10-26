set(SOURCE_FILES
	CColorPicker.cpp
	BandiVideoLibrary.cpp
	EditControl.cpp
	PythonGraphic.cpp
	PythonGraphicImageModule.cpp
	PythonGraphicModule.cpp
	PythonGraphicTextModule.cpp
	PythonGraphicThingModule.cpp
	PythonGridSlotWindow.cpp
	PythonSlotWindow.cpp
	PythonWindow.cpp
	PythonWindowManager.cpp
	PythonWindowManagerModule.cpp
	UIRenderer.cpp
	UIRenderer.hpp
)

add_library(metin2client_ui STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_ui PRIVATE metin2client_lib metin2client_webview metin2client_scriptlib PUBLIC python27_static metin2client_granny metin2client_script tweeny)
target_precompile_headers(metin2client_ui PRIVATE "StdAfx.h" "StdAfx.cpp")
