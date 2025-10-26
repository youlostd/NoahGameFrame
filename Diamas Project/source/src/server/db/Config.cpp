#include "Config.h"
#include <toml.hpp>

void LoadDbElement(const toml::value data, DbConfig& db, std::string dbName)
{
	if(data.count(dbName) != 0) {
		const auto tab = toml::get<toml::table>(data.at(dbName));
		db.user = toml::get<std::string>(tab.at("user"));
		db.addr = toml::get<std::string>(tab.at("host"));
		db.port = toml::get_or(tab.at("port"), 3306);
		db.password = toml::get<std::string>(tab.at("password"));
		db.name = toml::get<std::string>(tab.at("name"));
	}
}

bool LoadDbConfig(Config& c)
{
	try {
		const auto data = toml::parse("config.db.toml");
		LoadDbElement(data, c.playerDb, "player");
		LoadDbElement(data, c.commonDb, "common");
		LoadDbElement(data, c.logDb, "log");
		LoadDbElement(data, c.accountDb, "account");

		return true;
	} catch(...) {
		return false;
	}

}

bool LoadFile(Config& c, const std::string& filename)
{
	if(!LoadDbConfig(c))
		return false;


	try {
		const auto data = toml::parse(filename);


		if(data.count("test-server"))
			c.testServer = toml::get<toml::boolean>(data.at("test-server"));

		if(data.count("verbose"))
			c.verbose = toml::get<toml::boolean>(data.at("verbose"));
		
		if(data.count("china-event-server"))
			c.chinaEventServer = toml::get<toml::boolean>(data.at("china-event-server"));
	
		if(data.count("bind-addr"))
			c.bindAddr = toml::get<std::string>(data.at("bind-addr"));

		if(data.count("bind-port"))
			c.bindPort = toml::get<std::string>(data.at("bind-port"));

		if(data.count("player-cache-flush-time"))
			c.playerCacheFlushTime = toml::get<toml::integer>(data.at("player-cache-flush-time"));

		if(data.count("item-cache-flush-time"))
			c.itemCacheFlushTime = toml::get<toml::integer>(data.at("item-cache-flush-time"));

		if(data.count("price-cache-flush-time"))
			c.itemPriceListCacheFlushTime = toml::get<toml::integer>(data.at("price-cache-flush-time"));

		if(data.count("player-delete-level-min"))
			c.playerDeleteLevelMin = toml::get<toml::integer>(data.at("player-delete-level-min"));

		if(data.count("player-delete-level-max"))
			c.playerDeleteLevelMax = toml::get<toml::integer>(data.at("player-delete-level-max"));

		if(data.count("guild-power-poly"))
			c.guildPowerPoly = toml::get<std::string>(data.at("guild-power-poly"));

		if(data.count("guild-handicap-poly"))
			c.guildHandicapPoly = toml::get<std::string>(data.at("guild-handicap-poly"));

		if(data.count("current-battlepass-id"))
			c.battlepassId = toml::get<toml::integer>(data.at("current-battlepass-id"));

	} catch(const toml::syntax_error& err) {
		spdlog::critical(err.what());
		return false;
	}catch(const toml::type_error& err) {
		spdlog::critical(err.what());
		return false;
	}

	return true;
}
