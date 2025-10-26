set(SOURCE_FILES
	frustum.cpp
	frustum.h
	pool.h
	sphere.cpp
	sphere.h
	spherepack.cpp
	spherepack.h
	vector.h
)

add_library(metin2client_spherelib STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_spherelib PRIVATE metin2client_lib)
target_precompile_headers(metin2client_spherelib PRIVATE "StdAfx.h" "StdAfx.cpp")
