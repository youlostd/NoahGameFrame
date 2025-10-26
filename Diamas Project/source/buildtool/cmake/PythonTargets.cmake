include(Util)

macro(metin2_add_python_library)
	set(oneValueArgs TARGET)
	set(multiValueArgs SOURCES INCLUDE_DIRECTORIES)

	cmake_parse_arguments(args "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	set(c_sources "")
	foreach (python_file ${args_SOURCES})
		get_filename_component(python_file ${python_file} ABSOLUTE)
 
		# Current source dir. ends up in current binary dir.
		string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/cytmp" c_source_file ${python_file})

		# Handle paths outside the current subdirectory
		string(REPLACE ${CMAKE_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/cytmp" c_source_file ${c_source_file})

		# Change extension
		string(REPLACE ".pyx" ".cpp" c_source_file ${c_source_file})
		string(REPLACE ".py" ".cpp" c_source_file ${c_source_file})

		metin2_ensure_path_exists(${c_source_file})

		add_custom_command(
			OUTPUT ${c_source_file}
			COMMAND python ${METIN2_ROOT}/buildtool/cythonize.py ${python_file} ${c_source_file} ${args_INCLUDE_DIRECTORIES}
			DEPENDS ${python_file}
			VERBATIM
		)

		list(APPEND c_sources ${c_source_file})
	endforeach ()

	metin2_add_static_library(${args_TARGET} ${c_sources})
	target_link_libraries(${args_TARGET} PRIVATE python27_static)
endmacro()
