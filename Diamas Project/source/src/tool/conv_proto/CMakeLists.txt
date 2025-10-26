
set(CONV_ITEM_SOURCE_FILES
	${METIN2_SHARE}/StdioErrorHandlers.cpp

	ConvItemProto.cpp
	FormatItem.cpp
	ItemList.cpp
	resource.rc
)

set(CONV_MOB_SOURCE_FILES
	${METIN2_SHARE}/StdioErrorHandlers.cpp

	ConvMobProto.cpp
	FormatMob.cpp
	NpcList.cpp
	resource.rc
)

add_executable(metin2tool_conv_item_proto ${CONV_ITEM_SOURCE_FILES})
target_link_libraries(metin2tool_conv_item_proto PRIVATE vstl storm Boost::boost Boost::program_options spdlog::spdlog effolkronium_random m2.shared metin2_base
	metin2_xml
	metin2_game)

target_include_directories(metin2tool_conv_item_proto PUBLIC
	${METIN2_SHARE}
)
set_target_properties(metin2tool_conv_item_proto PROPERTIES
		OUTPUT_NAME conv_item_proto
)
	
add_executable(metin2tool_conv_mob_proto ${CONV_MOB_SOURCE_FILES})
target_link_libraries(metin2tool_conv_mob_proto PRIVATE vstl storm Boost::boost Boost::program_options spdlog::spdlog effolkronium_random m2.shared metin2_base
	metin2_xml
	metin2_game)

target_include_directories(metin2tool_conv_mob_proto PUBLIC
	${METIN2_SHARE}
)
set_target_properties(metin2tool_conv_mob_proto PROPERTIES
		OUTPUT_NAME conv_mob_proto
)