# Handy macro to parse macro arguments
macro(metin2_parse_arguments prefix arg_names option_names)
	set(DEFAULT_ARGS)
	foreach(arg_name ${arg_names})
		set(${prefix}_${arg_name})
	endforeach(arg_name)

	foreach(option ${option_names})
		set(${prefix}_${option} FALSE)
	endforeach(option)

	set(current_arg_name DEFAULT_ARGS)
	set(current_arg_list)

	foreach(arg ${ARGN})
		set(larg_names ${arg_names})
		list(FIND larg_names "${arg}" is_arg_name)
		IF (is_arg_name GREATER -1)
			set(${prefix}_${current_arg_name} ${current_arg_list})
			set(current_arg_name ${arg})
			set(current_arg_list)
		else (is_arg_name GREATER -1)
			set(loption_names ${option_names})
			list(FIND loption_names "${arg}" is_option)
			IF (is_option GREATER -1)
				set(${prefix}_${arg} TRUE)
			else (is_option GREATER -1)
				set(current_arg_list ${current_arg_list} ${arg})
			endif ()
		endif ()
	endforeach(arg)

	set(${prefix}_${current_arg_name} ${current_arg_list})
endmacro()

function (metin2_ensure_path_exists TARGET_FILE)
	get_filename_component(TARGET_ABS_PATH ${TARGET_FILE} PATH)

	if (TARGET_ABS_PATH)
		if (NOT IS_ABSOLUTE ${TARGET_ABS_PATH})
			set(TARGET_ABS_PATH ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_ABS_PATH})
		endif ()

		if (NOT EXISTS ${TARGET_ABS_PATH})
			file(MAKE_DIRECTORY ${TARGET_ABS_PATH})
		endif ()
	endif ()
endfunction ()

macro(metin2_add_msvc_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
	if (MSVC)
		get_filename_component(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
		set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
		set(Sources ${${SourcesVar}})

		set_source_files_properties(${PrecompiledSource}
									PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
											OBJECT_OUTPUTS "${PrecompiledBinary}")

		set_source_files_properties(${Sources}
									PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
											OBJECT_DEPENDS "${PrecompiledBinary}")

		list(APPEND ${SourcesVar} ${PrecompiledSource})
	endif ()
endmacro()

