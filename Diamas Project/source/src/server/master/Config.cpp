#include "Config.hpp"
#include <toml.hpp>
#include "spdlog/spdlog.h"

bool LoadFile(Config& c, const std::string& filename)
{
		try {
		const auto data = toml::parse(filename);

		if(data.count("bind-addr"))
			c.gameBindAddr = toml::get<std::string>(data.at("bind-addr"));

		if(data.count("bind-port"))
			c.gameBindPort = toml::get<std::string>(data.at("bind-port"));

		if(data.count("auth-addr"))
			c.authBindAddr = toml::get<std::string>(data.at("auth-addr"));

		if(data.count("auth-port"))
			c.authBindPort = toml::get<std::string>(data.at("auth-port"));
			
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


