#ifndef METIN2_XML_TYPES_HPP
#define METIN2_XML_TYPES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

/// @file TypesFwd.hpp
/// Forward-declarations of various utility functions for xml file parsing.
///
/// @{

namespace rapidxml
{

template <class Ch>
class memory_pool;

template <class Ch>
class xml_base;

template <typename Ch>
class xml_node;

template <typename Ch>
class xml_attribute;

template <typename Ch>
class xml_document;

}



typedef rapidxml::memory_pool<char> XmlMemoryPool;
typedef rapidxml::xml_base<char> XmlBase;
typedef rapidxml::xml_node<char> XmlNode;
typedef rapidxml::xml_attribute<char> XmlAttribute;



#endif
