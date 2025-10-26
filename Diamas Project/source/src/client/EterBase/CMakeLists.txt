set(SOURCE_FILES
	CRC32.cpp
	DDSTextureLoader.cpp
	Debug.cpp
	FileBase.cpp
	FileDir.cpp
	FileLoader.cpp
	lzo.cpp
	MappedFile.cpp
	Random.cpp
	sha256.cpp
	Stl.cpp
	tea.cpp
	Timer.cpp
	UnicodeUtil.cpp
	Utils.cpp
	WICTextureLoader9.hpp
	WICTextureLoader9.cpp
	imconfig.h
	imgui.cpp
	imgui.h
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_impl_win32.h
	imgui_impl_win32.cpp
	imgui_impl_dx9.cpp
	imgui_impl_dx9.h
	imgui_internal.h
	imgui_widgets.cpp
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
	Poly/Base.cpp
	Poly/Poly.cpp
	Poly/Symbol.cpp
	Poly/SymTable.cpp
)

add_library(metin2client_base STATIC ${SOURCE_FILES})
target_precompile_headers(metin2client_base PRIVATE "StdAfx.h" "StdAfx.cpp")

target_link_libraries(metin2client_base PRIVATE lzo-pro PUBLIC Boost::boost spdlog::spdlog effolkronium_random storm vstl m2.shared)
target_include_directories(metin2client_base
    PUBLIC 
        $<INSTALL_INTERFACE:..>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
