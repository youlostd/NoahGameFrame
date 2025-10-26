set(SOURCE_FILES
	PythonDebugModule.cpp
	PythonMarshal.cpp
	PythonLauncher.cpp
	PythonUtils.cpp
	PyStr.cpp
)

add_library(metin2client_scriptlib STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_scriptlib PRIVATE metin2client_base PUBLIC python27_static pybind11::pybind11)
target_precompile_headers(metin2client_scriptlib PRIVATE "StdAfx.h" "StdAfx.cpp")
