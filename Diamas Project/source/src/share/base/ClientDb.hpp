#ifndef METIN2_BASE_CLIENTDB_HPP
#define METIN2_BASE_CLIENTDB_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Serialization.hpp"

#include <storm/io/File.hpp>
#include <storm/io/FileUtil.hpp>
#include <storm/io/StreamUtil.hpp>
#include <storm/Util.hpp>

#include <boost/system/error_code.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <utility>
#include <vector>



BOOST_STATIC_CONSTEXPR uint32_t kClientDbFourCc =
	STORM_MAKEFOURCC('E', 'C', 'D', 'B');

template <typename T>
struct ClientDb
{
	ClientDb(std::vector<T>& data)
		: data(data)
	{ }

	std::integral_constant<uint32_t, kClientDbFourCc> fourcc;
	std::integral_constant<uint32_t, T::kVersion> version;
	std::vector<T>& data;
};

template <typename T>
struct ConstClientDb
{
	ConstClientDb(const std::vector<T>& data)
		: data(data)
	{ }

	std::integral_constant<uint32_t, kClientDbFourCc> fourcc;
	std::integral_constant<uint32_t, T::kVersion> version;
	const std::vector<T>& data;
};

template <class T>
bool LoadClientDb(asio::const_buffer buffer, std::vector<T>& data)
{
	ClientDb<T> db(data);
	try {
		Reader r(std::move(buffer));
		r(db);
	} catch (BadMessage& e) {
		spdlog::error("Failed to read/deserialize clientdb: {0}", e.what());
		return false;
	}
	return true;
}

template <class T>
bool LoadClientDbFromFile(const storm::StringRef& filename,
                          std::vector<T>& data)
{
	storm::String content;
	bsys::error_code ec;
	storm::ReadFileToString(filename, content, ec);

	if (ec) {
		spdlog::error("Failed to open {0} for reading with {1}",
		          filename, ec);
		return false;
	}

	if (!LoadClientDb(asio::buffer(content.data(), content.size()), data)) {
		spdlog::error("LoadClientDb failed for file {0}",
		          filename);
		return false;
	}

	return true;
}

template <class T>
bool SaveClientDb(const storm::StringRef& filename, const std::vector<T>& data)
{
	storm::File fp;

	bsys::error_code ec;
	fp.Open(filename, ec,
	        storm::AccessMode::kWrite,
	        storm::CreationDisposition::kCreateAlways,
	        storm::ShareMode::kNone,
	        storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          filename, ec);
		return false;
	}

	ConstClientDb<T> db(data);

	std::size_t bufferSize = 12 + data.size() * sizeof(T);
	std::vector<char> buffer(bufferSize);

	while (true) {
		buffer.resize(bufferSize);

		try {
			bufferSize -= asio::buffer_size(Write(asio::buffer(buffer), db));
			storm::WriteExact(fp, buffer.data(), bufferSize, ec);

			if (ec) {
				spdlog::error("Failed to write {0} with {1}",
				          filename, ec);
				return false;
			}

			break;
		} catch (MessageBufferTooSmall& e) {
			bufferSize *= 2;
			continue;
		} catch (BadMessage& e) {
			spdlog::error("Failed to save/serialize clientdb: {0}", e.what());
			return false;
		}
	}

	return true;
}



BOOST_FUSION_ADAPT_TPL_STRUCT(
	(T),
	(ClientDb)(T),
	fourcc,
	version,
	data
);

BOOST_FUSION_ADAPT_TPL_STRUCT(
	(T),
	(ConstClientDb)(T),
	fourcc,
	version,
	data
);

#endif
