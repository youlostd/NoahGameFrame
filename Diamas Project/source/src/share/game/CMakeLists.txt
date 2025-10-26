set(SOURCE_FILES
	AffectConstants.hpp
	AffectsHolder.cpp
	AffectsHolder.hpp
	AffectsHolderUtil.cpp
	AffectsHolderUtil.hpp
	attribute.cc
	attribute.h
	AuthPackets.hpp
	BuildingConstants.cpp
	BuildingConstants.hpp
	BuildingTypes.cpp
	BuildingTypes.hpp
	Constants.cpp
	Constants.hpp
	CubeConstants.cpp
	CubeConstants.hpp
	CubeTypes.cpp
	CubeTypes.hpp
	CsvReader.cpp
	CsvReader.hpp
	DbPackets.hpp
	DragonSoulTable.cpp
	DragonSoulTable.hpp
	EffectConstants.hpp
	Format.hpp
	GamePacket.hpp
	game_type.h
	grid.cc
	grid.h
	HuntingManager.cpp
	HuntingManager.hpp
	ItemConstants.cpp
	ItemConstants.hpp
	ItemTypes.cpp
	ItemTypes.hpp
	item_length.h
	length.h
	MasterPackets.hpp
	MobConstants.cpp
	MobConstants.hpp
	MobTypes.cpp
	MobTypes.hpp
	MotionConstants.cpp
	MotionConstants.hpp
	MotionTypes.cpp
	MotionTypes.hpp
	PageGrid.hpp
	PrefixHeader.hpp
	ProtoClientDb.hpp
	ProtoCsv.hpp
	ProtoReader.cpp
	ProtoReader.hpp
	ProtoXml.hpp
	SkillConstants.cpp
	SkillConstants.hpp
	SkillTypes.cpp
	SkillTypes.hpp
	targa.cc
	targa.h
	Types.cpp
	Types.hpp
	WarpManager.hpp
	WarpManager.cpp
)

metin2_add_static_library(metin2_game ${SOURCE_FILES})

if(NOT WIN32)
	target_link_libraries(metin2_game PUBLIC metin2_xml
	Eigen3::Eigen pthread)
else()
	target_link_libraries(metin2_game PUBLIC metin2_xml
	Eigen3::Eigen)
endif()