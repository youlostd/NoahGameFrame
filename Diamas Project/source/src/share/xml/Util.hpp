#ifndef METIN2_XML_UTIL_HPP
#define METIN2_XML_UTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Types.hpp"
#include "ErrorHandling.hpp"

#include <rapidxml.hpp>
#include <rapidxml_iterators.hpp>

#include <type_traits>
#include <vstl/string.hpp>
#include <storm/StringUtil.hpp>


bool IsStringTrue(const std::string_view& s);

//
// XML parsing
//

inline std::string_view GetName(const XmlBase* node)
{
	return std::string_view(node->name(), node->name_size());
}

inline std::string_view GetValue(const XmlBase* node)
{
	return std::string_view(node->value(), node->value_size());
}

const XmlNode* TryGetNode(const XmlNode* node, const std::string_view& name);
const XmlNode* GetNode(const XmlNode* node, const std::string_view& name);

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name,
                     std::string_view& value);

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name,
                     vstd::string& value);

void GetAttribute(const XmlNode* node,
                  const std::string_view& name,
                  std::string_view& value);

void GetAttribute(const XmlNode* node,
                  const std::string_view& name,
                  std::string& value);

std::string_view GetAttributeOr(const XmlNode* node,
                                 const std::string_view& name,
                                 const std::string_view& placeholder);

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name, bool& value);

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name, std::string& value);

void GetAttribute(const XmlNode* node,
                  const std::string_view& name, bool& value);




template <typename T>
bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name, T& value)
{
	std::string_view s;
	if (!TryGetAttribute(node, name, s))
		return false;

	if (storm::ParseNumber(s, value))
		return true;

	return false;
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
void GetAttribute(const XmlNode* node,
                  const std::string_view& name, T& value)
{
	std::string_view attr;
	GetAttribute(node, name, attr);

	if (!storm::ParseNumber(attr, value)) {
		throw XmlError()
			<< FormatXmlError("Failed to parse number attribute {0} "
			                  "(value {1}) in {2}",
			                  name, attr, GetName(node))
			<< XmlErrorNodeInfo(node);
	}
}

/// @param parser bool parser(const vstd::string_view& data)
template <class Function>
void ParseAttribute(const XmlNode* node, const std::string_view& name,
                    Function&& parser)
{
	std::string attr;
	GetAttribute(node, name, attr);

	if (!parser(attr)) {
		throw XmlError()
			<< FormatXmlError("Failed to parse attribute {0} "
			                  "(value {1}) in {2}",
			                  name, attr, GetName(node))
			<< XmlErrorNodeInfo(node);
	}
}

template <class Function, class T>
void ParseStringTableEntry(const XmlNode* node,
                           const std::string_view& attr,
                           Function&& parser,
                           T& value)
{
	const auto f = [&parser, &value] (const std::string& s) {
		uint32_t val;
		if (!parser(s, val))
			return false;

		value = val;
		return true;
	};

	ParseAttribute(node, attr, f);
}

inline void AssertNodeParse(const XmlNode* node, bool value,
                            const std::string_view& message)
{
	if (value)
		return;

	throw XmlError()
		<< FormatXmlError("{0}: {1}", GetName(node), message)
		<< XmlErrorNodeInfo(node);
}

#define METIN2_XML_PARSE_ASSERT(node, expr) \
	AssertNodeParse(node, (expr), #expr)

//
// XML editing
//

void AddAttribute(XmlMemoryPool* pool, XmlNode* node,
                  const std::string_view& name,
                  const std::string_view& value);

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
bool TryAddAttribute(XmlMemoryPool* pool, XmlNode* node,
                     const std::string_view& name, T value)
{
	vstd::string str;
	if (!storm::FormatNumber(str, value))
		return false;

	AddAttribute(pool, node, name, str.c_str());
	return true;
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
void AddAttribute(XmlMemoryPool* pool, XmlNode* node,
                  const std::string_view& name, T value)
{
	AddAttribute(pool, node, name, fmt::format("{}", value));
}

void AddAttribute(XmlMemoryPool* pool, XmlNode* node,
                  const std::string_view& name, bool value);

void AddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                       const std::string_view& name,
                       const std::string_view& value,
                       const std::string_view& placeholder);

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
bool TryAddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                          const std::string_view& name, T value, T placeholder)
{
	if (value == placeholder)
		return true;

	try {
		AddAttribute(pool, node, name, fmt::format("{}", value));
	} catch(const std::exception& ex) {
		return false;
	}
	return true;
}

bool TryAddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                          const std::string_view& name,
                          bool value, bool placeholder);

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
void AddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                       const std::string_view& name, T value, T placeholder)
{
	if (value == placeholder)
		return;

	try {
		AddAttribute(pool, node, name, fmt::format("{}", value));
	} catch(const std::exception& ex) {
				throw XmlError()
			<< FormatXmlError("Failed to format {0} as number for attr {1} "
			                  "in node {2}",
			                  value, name, GetName(node))
			<< XmlErrorNodeInfo(node);
	}

}

void AddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                       const std::string_view& name,
                       bool value, bool placeholder);



#endif
