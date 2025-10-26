set(SOURCE_FILES
	BoundaryShapeManager.cpp
	BoundaryShapeManager.h
	SpeedGrassRT.cpp
	SpeedGrassRT.h
	SpeedGrassWrapper.cpp
	SpeedGrassWrapper.h
	SpeedTreeForest.cpp
	SpeedTreeForest.h
	SpeedTreeForestDirectX9.cpp
	SpeedTreeForestDirectX9.h
	SpeedTreeWrapper.cpp
	SpeedTreeWrapper.h
	Constants.h
	VertexShaders.h
)

add_library(metin2client_speedtree STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_speedtree PUBLIC m2.shared metin2_base metin2client_lib speedtree_static)
target_precompile_headers(metin2client_speedtree PRIVATE "StdAfx.h" "StdAfx.cpp")
