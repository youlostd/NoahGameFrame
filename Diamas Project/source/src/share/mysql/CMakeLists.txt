set(SOURCE_FILES
	AsyncSQL.cpp
)

metin2_add_static_library(metin2_mysql ${SOURCE_FILES})


if (NOT WIN32)
target_link_libraries(metin2_mysql PRIVATE metin2_thecore spdlog::spdlog effolkronium_random
								   PUBLIC MySQL::client pthread)
else()
	target_link_libraries(metin2_mysql PRIVATE metin2_thecore spdlog::spdlog effolkronium_random 
								   PUBLIC MySQL::client)
endif ()
