# Order is important!
add_subdirectory(share)

if (WIN32)
	add_subdirectory(client)
endif ()

add_subdirectory(server)
add_subdirectory(tool)
