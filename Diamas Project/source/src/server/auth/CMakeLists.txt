set(SOURCE_FILES
	ClientPackets.cpp
    ClientPackets.hpp
    ClientSocket.cpp
    ClientSocket.hpp
    ClientUtil.cpp
    ClientUtil.hpp
    Config.cpp
    Config.hpp
    DbCachePackets.cpp
    DbCachePackets.hpp
    DbCacheSocket.cpp
    DbCacheSocket.hpp
    Main.cpp
    MasterPackets.cpp
    MasterPackets.hpp
    MasterSocket.cpp
    MasterSocket.hpp
    MasterUtil.cpp
    MasterUtil.hpp
    MySqlPassword.cpp
    MySqlPassword.hpp
    Server.cpp
    Server.hpp
)

metin2_add_executable(metin2server_auth auth ${SOURCE_FILES})

target_link_libraries(metin2server_auth PRIVATE
	metin2_base
	metin2_thecore
	metin2_mysql
	metin2_game
	metin2_net
	toml11
)

if (WIN32)
	target_link_libraries(metin2server_auth PRIVATE ws2_32)
endif ()
