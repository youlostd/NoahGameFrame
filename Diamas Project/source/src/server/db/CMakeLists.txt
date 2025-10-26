set(SOURCE_FILES Cache.cpp
	Cache.h
	ClientManager.cpp
	ClientManager.h
	ClientManagerBoot.cpp
	ClientManagerEventFlag.cpp
	ClientManagerGuild.cpp
	ClientManagerHorseName.cpp
	ClientManagerHwidFlag.cpp
	ClientManagerItemName.cpp
	ClientManagerLogin.cpp
	ClientManagerOfflineShop.cpp
	ClientManagerParty.cpp
	ClientManagerPlayer.cpp
	Config.cpp
	Config.h
	CsvReader.cpp
	CsvReader.h
	DBManager.cpp
	DBManager.h
	GuildManager.cpp
	GuildManager.h
	GuildStorage.cpp
	GuildStorage.h
	GuildStorageManager.cpp
	GuildStorageManager.h
	ItemAwardManager.cpp
	ItemAwardManager.h
	ItemIDRangeManager.cpp
	ItemIDRangeManager.h
	Main.cpp
	Main.h
	Marriage.cpp
	Marriage.h
	MoneyLog.cpp
	MoneyLog.h
	Peer.cpp
	Peer.h
	Persistence.cpp
	Persistence.hpp
	PlayerLoader.cpp
	PlayerLoader.hpp
	PrivManager.cpp
	PrivManager.h
	QID.h
	stdafx.h
)

metin2_add_executable(metin2server_db db ${SOURCE_FILES})

target_link_libraries(metin2server_db PRIVATE
	metin2_base
	metin2_thecore
	metin2_mysql
	metin2_game
	metin2_net
	toml11
)

if (WIN32)
	target_link_libraries(metin2server_db PRIVATE ws2_32)
endif ()
