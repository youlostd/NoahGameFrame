include_directories(
	${LUA_ROOT}/include
	${LUA_ROOT}/src
)


set(SOURCE_FILES
	crc32.cpp
	main.cpp
)

metin2_add_executable(metin2tool_qc qc ${SOURCE_FILES})
target_link_libraries(metin2tool_qc PRIVATE lua)
