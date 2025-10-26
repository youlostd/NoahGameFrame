set(SOURCE_FILES
	EffectData.cpp
	EffectData.h
	EffectElementBase.cpp
	EffectElementBase.h
	EffectElementBaseInstance.cpp
	EffectElementBaseInstance.h
	EffectInstance.cpp
	EffectInstance.h
	EffectManager.cpp
	EffectManager.h
	EffectMesh.cpp
	EffectMesh.h
	EffectMeshInstance.cpp
	EffectMeshInstance.h
	EmitterProperty.cpp
	EmitterProperty.h
	FrameController.cpp
	FrameController.h
	ParticleInstance.cpp
	ParticleInstance.h
	ParticleProperty.cpp
	ParticleProperty.h
	ParticleSystemData.cpp
	ParticleSystemData.h
	ParticleSystemInstance.cpp
	ParticleSystemInstance.h
	SimpleLightData.cpp
	SimpleLightData.h
	SimpleLightInstance.cpp
	SimpleLightInstance.h
	Type.cpp
	Type.h
	Util.cpp
	Util.hpp
)

add_library(metin2client_effects STATIC ${SOURCE_FILES})
target_link_libraries(metin2client_effects PRIVATE metin2client_lib metin2client_miles)
target_precompile_headers(metin2client_effects PRIVATE "StdAfx.h" "StdAfx.cpp")
