#include "Util.hpp"

#include <storm/StringUtil.hpp>
#include <storm/String.hpp>



bool IsStringTrue(const std::string_view& s)
{
	if (s.empty())
		return false;

	return s[0] == '1' || storm::EqualsIgnoreCase(s, "true");
}

const XmlNode* TryGetNode(const XmlNode* node, const std::string_view& name)
{
	return node->first_node(name.data(), name.size());
}

const XmlNode* GetNode(const XmlNode* node, const std::string_view& name)
{
	const auto res = TryGetNode(node, name);
	if (res)
		return res;

	throw XmlError()
		<< FormatXmlError("Failed to find child {0} in node {1}",
		                  name, GetName(node))
		<< XmlErrorNodeInfo(node);
}

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name,
                     std::string_view& value)
{
	const auto attr = node->first_attribute(name.data(), name.size());
	if (attr) {
		value = GetValue(attr);
		return true;
	}

	return false;
}

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name,
                     vstd::string& value)
{
	const auto attr = node->first_attribute(name.data(), name.size());
	if (attr) {
		value.assign(attr->value(), attr->value_size());
		return true;
	}

	return false;
}

void GetAttribute(const XmlNode* node,
                  const std::string_view& name,
                  std::string_view& value)
{
	if (TryGetAttribute(node, name, value))
		return;

	throw XmlError()
		<< FormatXmlError("Failed to find attribute {0} in node {1}",
		                  name, GetName(node))
		<< XmlErrorNodeInfo(node);
}
void GetAttribute(const XmlNode* node,
                  const std::string_view& name,
                  std::string& value)
{
	if (TryGetAttribute(node, name, value))
		return;

	throw XmlError()
		<< FormatXmlError("Failed to find attribute {0} in node {1}",
		                  name, GetName(node))
		<< XmlErrorNodeInfo(node);
}

void GetAttribute(const XmlNode* node,
                  const std::string_view& name,
                  vstd::string& value)
{
	if (TryGetAttribute(node, name, value))
		return;

	throw XmlError()
		<< FormatXmlError("Failed to find attribute {0} in node {1}",
		                  name, GetName(node))
		<< XmlErrorNodeInfo(node);
}

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name, bool& value)
{
	const auto attr = node->first_attribute(name.data(), name.size());
	if (attr) {
		value = IsStringTrue(std::string_view(attr->value(),
		                                       attr->value_size()));
		return true;
	}

	return false;
}

bool TryGetAttribute(const XmlNode* node,
                     const std::string_view& name, std::string& value)
{
	const auto attr = node->first_attribute(name.data(), name.size());
	if (attr) {
		value = attr->value();
		return true;
	}

	return false;
}

void GetAttribute(const XmlNode* node,
                  const std::string_view& name, bool& value)
{
	const auto attr = node->first_attribute(name.data(), name.size());
	if (attr) {
		value = IsStringTrue(std::string_view(attr->value(),
		                                       attr->value_size()));
		return;
	}

	throw XmlError()
		<< FormatXmlError("Failed to find boolean attribute {0} in node {1}",
		                  name, GetName(node))
		<< XmlErrorNodeInfo(node);
}

std::string_view GetAttributeOr(const XmlNode* node,
                                 const std::string_view& name,
                                 const std::string_view& placeholder)
{
	std::string_view s;
	if (TryGetAttribute(node, name, s))
		return s;

	return placeholder;
}

void AddAttribute(XmlMemoryPool* pool, XmlNode* node,
                  const std::string_view& name,
                  const std::string_view& value)
{
	auto n = pool->allocate_string(name.data(), name.size());
	auto v = pool->allocate_string(value.data(), value.size());
	auto a = pool->allocate_attribute(n, v, name.size(), value.size());

	node->append_attribute(a);
}

void AddAttribute(XmlMemoryPool* pool, XmlNode* node,
                  const std::string_view& name, bool value)
{
	AddAttribute(pool, node, name,
	             std::string_view(value ? "true" : "false"));
}

void AddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                       const std::string_view& name,
                       const std::string_view& value,
                       const std::string_view& placeholder)
{
	if (value == placeholder)
		return;

	AddAttribute(pool, node, name, value);
}

bool TryAddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                          const std::string_view& name,
                          bool value, bool placeholder)
{
	if (value == placeholder)
		return true;

	AddAttribute(pool, node, name, value ? "true" : "false");
	return true;
}

void AddAttributeIfNot(XmlMemoryPool* pool, XmlNode* node,
                       const std::string_view& name,
                       bool value, bool placeholder)
{
	if (value == placeholder)
		return;

	AddAttribute(pool, node, name, value ? "true" : "false");
}


