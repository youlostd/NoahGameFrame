
#add_subdirectory(qc)
if(WIN32)
	add_subdirectory(conv_proto)
	add_subdirectory(clientconfig)
	add_subdirectory(dump_proto)
	add_subdirectory(format_proto)
	add_subdirectory(make_motion_proto)
	add_subdirectory(archiver)
	add_subdirectory(world_editor)
	
	set_target_properties(metin2tool_archiver PROPERTIES FOLDER Tools)
	set_target_properties(metin2tool_conv_item_proto PROPERTIES FOLDER Tools)
	set_target_properties(metin2tool_conv_mob_proto PROPERTIES FOLDER Tools)
	set_target_properties(metin2tool_dumpproto PROPERTIES FOLDER Tools)
	set_target_properties(metin2tool_formatproto PROPERTIES FOLDER Tools)
	set_target_properties(metin2tool_makemotionproto PROPERTIES FOLDER Tools)
	set_target_properties(metin2tool_worldeditor PROPERTIES FOLDER Tools)
	set_target_properties(metin2tool_config PROPERTIES FOLDER Tools)


endif()