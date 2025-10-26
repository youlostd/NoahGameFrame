set(SOURCES
	WebManager.cpp
	WebManager.h
	WebManager_impl.cpp
	WebManager_impl.h
	WebView.cpp
	WebView.h
)

add_library(metin2client_webview STATIC ${SOURCES}) 
target_link_libraries(metin2client_webview PUBLIC cefdll_wrapper
 spdlog::spdlog
 m2.shared
 metin2_base
 metin2client_scriptlib
 python27_static
 effolkronium_random
 metin2client_script
 bx)

target_precompile_headers(metin2client_webview PRIVATE "StdAfx.h" "StdAfx.cpp")
