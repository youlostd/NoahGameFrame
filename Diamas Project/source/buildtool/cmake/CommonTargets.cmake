
macro(metin2_add_executable TARGET_NAME OUTPUT_NAME)
	add_executable(${TARGET_NAME} ${ARGN})
	target_link_libraries(${TARGET_NAME} PRIVATE vstl storm Boost::boost Boost::program_options spdlog::spdlog effolkronium_random m2.shared)

	target_include_directories(${TARGET_NAME} PUBLIC
		${METIN2_SHARE}
	)

	target_precompile_headers(${TARGET_NAME} PRIVATE 
	    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/PrefixHeader.hpp>"
		)

	set_target_properties(${TARGET_NAME} PROPERTIES
		OUTPUT_NAME ${OUTPUT_NAME}
	)
endmacro()

macro(metin2_add_static_library TARGET_NAME)
	add_library(${TARGET_NAME} STATIC ${ARGN})
	find_package(Backtrace)
	if(Backtrace_FOUND)
		include_directories(${Backtrace_INCLUDE_DIRS})
		target_link_libraries(${TARGET_NAME} PRIVATE vstl storm Boost::boost Boost::program_options spdlog::spdlog effolkronium_random m2.shared ${Backtrace_LIBRARIES})
	else ()
		target_link_libraries(${TARGET_NAME} PRIVATE vstl storm Boost::boost Boost::program_options spdlog::spdlog effolkronium_random m2.shared)
	endif ()

	target_include_directories(${TARGET_NAME} PUBLIC
		${METIN2_SHARE}
	)

	target_precompile_headers(${TARGET_NAME} PRIVATE 
	    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/PrefixHeader.hpp>"
		)


endmacro()

macro(metin2_add_shared_library TARGET_NAME OUTPUT_NAME)
	add_library(${TARGET_NAME} SHARED ${ARGN})
	target_link_libraries(${TARGET_NAME} PRIVATE vstl storm Boost::boost Boost::program_options spdlog::spdlog effolkronium_random m2.shared ${BOOST_LIBRARIES})

	target_include_directories(${TARGET_NAME} PUBLIC
		${METIN2_SHARE}
	)

	target_precompile_headers(${TARGET_NAME} PRIVATE 
	    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/PrefixHeader.hpp>"
		)
	set_target_properties(${TARGET_NAME} PROPERTIES
		OUTPUT_NAME ${OUTPUT_NAME}
	)
endmacro()
