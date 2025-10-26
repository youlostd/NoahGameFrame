#include "Config.hpp"
#include <toml.hpp>
#include "spdlog/spdlog.h"


bool LoadFile(Config& c, const std::string& filename)
{

	try {
		const auto data = toml::parse(filename);

		if(data.count("bind-addr"))
			c.bindAddr = toml::get<std::string>(data.at("bind-addr"));

		if(data.count("bind-port"))
			c.bindPort = toml::get<std::string>(data.at("bind-port"));

		if(data.count("master-addr"))
			c.masterAddr = toml::get<std::string>(data.at("master-addr"));

		if(data.count("master-port"))
			c.masterPort = toml::get<std::string>(data.at("master-port"));

		if(data.count("db-addr"))
			c.dbCacheAddr = toml::get<std::string>(data.at("db-addr"));

		if(data.count("db-port"))
			c.dbCachePort = toml::get<std::string>(data.at("db-port"));

		if(data.count("version"))
			c.version = toml::get<toml::integer>(data.at("version"));

		if(data.count("log-level"))
			c.logLevel = toml::get<toml::integer>(data.at("log-level"));


	} catch(const toml::syntax_error& err) {
		spdlog::critical(err.what());
		return false;
	}catch(const toml::type_error& err) {
		spdlog::critical(err.what());
		return false;
	}

	return true;

}


