set(SOURCE_FILES
	AuthPackets.cpp
	AuthPackets.hpp
	AuthSocket.cpp
	AuthSocket.hpp
	Config.cpp
	Config.hpp
	GamePackets.cpp
	GamePackets.hpp
	GameSocket.cpp
	GameSocket.hpp
	GameUtil.cpp
	GameUtil.hpp
	LoginSessions.cpp
	LoginSessions.hpp
	Main.cpp
	OnlinePlayers.cpp
	OnlinePlayers.hpp
	Server.cpp
	Server.hpp
	stdafx.h
)

metin2_add_executable(metin2server_master master ${SOURCE_FILES})

target_link_libraries(metin2server_master PRIVATE
	metin2_base
	metin2_thecore
	metin2_mysql
	metin2_game
	metin2_net
	toml11
)

if (WIN32)
	target_link_libraries(metin2server_master PRIVATE ws2_32)
endif ()