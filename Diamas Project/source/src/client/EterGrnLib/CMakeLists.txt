set(SOURCE_FILES
	Material.h
	Material.cpp
	Mesh.h
	Mesh.cpp
	Model.cpp
	Model.h
	ModelInstance.cpp
	ModelInstance.h
	ModelInstanceCollisionDetection.cpp
	ModelInstanceModel.cpp
	ModelInstanceMotion.cpp
	ModelInstanceRender.cpp
	ModelInstanceUpdate.cpp
	ModelShader.cpp
	ModelShader.h
	Motion.cpp
	Motion.h
	Thing.cpp
	Thing.h
	ThingInstance.cpp
	ThingInstance.h
	Util.h
	Util.cpp
	GrannyState.cpp
	GrannyState.hpp
)



add_library(metin2client_granny STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_granny PUBLIC metin2client_lib ${DIRECT3D_LIBRARY} granny2)
target_precompile_headers(metin2client_granny PRIVATE "StdAfx.h" "StdAfx.cpp")
