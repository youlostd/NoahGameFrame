#ifndef METIN2_XML_ERRORHANDLING_HPP
#define METIN2_XML_ERRORHANDLING_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Types.hpp"

#include <fmt/format.h>
#include <storm/String.hpp>

#include <boost/exception/exception.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/info.hpp>
#include <boost/throw_exception.hpp>

#include <stdexcept>



class XmlDocument;

struct XmlError : virtual boost::exception, virtual std::exception
{ };

using XmlErrorMsgInfo = boost::error_info<struct XmlErrorMsgInfoTag, std::string>;
using XmlErrorNodeInfo = boost::error_info<struct XmlErrorNodeInfoTag, const XmlNode*>;
using XmlErrorAttrInfo = boost::error_info<struct XmlErrorAttrInfoTag, const XmlAttribute*>;

template <class... A>
XmlErrorMsgInfo FormatXmlError(const std::string_view& format, A&&... args)
{
	std::string s = fmt::format(format, std::forward<A>(args)...);
	return s;
}

template <class... A>
void ThrowXmlErrorIfNot(bool value, const std::string_view& format, A&&... args)
{
	if (value)
		return;

	throw XmlError() << FormatXmlError(format, std::forward<A>(args)...);
}

void LogError(const XmlDocument& doc,
              const XmlError& error,
              const std::string_view& filename);



#endif
