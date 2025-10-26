#ifndef METIN2_GAME_PROTOXML_HPP
#define METIN2_GAME_PROTOXML_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <xml/Types.hpp>
#include <xml/Document.hpp>
#include <xml/ErrorHandling.hpp>

#include <storm/io/FileUtil.hpp>
#include <storm/io/File.hpp>
#include <storm/String.hpp>
#include <storm/Log.hpp>

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

#include <vector>
#include <map>
#include <unordered_map>

METIN2_BEGIN_NS

template <typename T>
class is_convert_proto
{
	typedef char (& yes)[1];
	typedef char (& no)[2];

	template <typename C> static yes check(typename C::ConvertFrom*);
	template <typename C> static no check(...);

	public:
		enum {value = sizeof(check<T>(nullptr)) == sizeof(yes), ivalue = sizeof(check<T>(nullptr)) == sizeof(false)};
};

template <typename T>
typename std::enable_if<is_convert_proto<T>::value, bool>::type MakeProto(T& entry, const XmlNode* node)
{
	typename T::ConvertFrom preConv{};
	if (!ParseProto(preConv, node))
		return false;

	return ConvertProto(preConv, entry);
}

template <typename T>
typename std::enable_if<!is_convert_proto<T>::value, bool>::type MakeProto(T& entry, const XmlNode* node)
{
	return ParseProto(entry, node);
}

template <typename T>
bool LoadProtoCsv(const std::string& filename,
                  typename std::vector<T>& items)
{
	XmlDocument doc;

	const auto res = doc.LoadFile(filename);
	if (!res) {
		LogError(doc, res, filename);
		return false;
	}

	auto root = doc.first_node();
	if (!root) {
		spdlog::error("XML '{0}' has no root node",
		          filename);
		return false;
	}

	for (auto node = root->first_node(); node; node = node->next_sibling()) {
		T entry = {};
		try {
			if (!MakeProto(entry, node)) {
				spdlog::error("Failed to parse proto entry {0} in {1}",
				          items.size(), filename);
				return false;
			}
		} catch (XmlError& e) {
			LogError(doc, e, filename);
			return false;
		}

		items.emplace_back(std::move(entry));
	}

	return true;
}

template <typename T>
bool WriteProtoXml(const storm::StringRef& filename,
                   const std::vector<T>& items,
                   const char* name = "proto")
{
	storm::File output;
	bsys::error_code ec;

	output.Open(filename, ec,
	            storm::AccessMode::kWrite,
	            storm::CreationDisposition::kCreateAlways,
	            storm::ShareMode::kNone,
	            storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open XML file {0} with {1}",
		          filename, ec);
		return false;
	}

	rapidxml::xml_document<char> doc;

	auto node = doc.allocate_node(rapidxml::node_element, name);
	doc.append_node(node);

	for (const auto& entry : items) {
		XmlNode* proto;
		try {
			proto = FormatProto(entry, &doc);
		} catch (std::exception& e) {
			spdlog::error("{0}: Failed to format entry with {1}", name, e.what());
			return false;
		}

		if (!proto) {
			// TODO(tim): Improve error handling here
			spdlog::error("{0}: Failed to format entry", name);
			return false;
		}

		node->append_node(proto);
	}

	std::string buf;
	rapidxml::print(std::back_inserter(buf), doc, 0);

	output.Write(buf.data(), buf.length(), ec);

	if (!ec)
		return true;

	spdlog::error("{0}: Failed to write proto data with {1}", name, ec);
	return false;
}

METIN2_END_NS

#endif
