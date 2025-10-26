#include "ErrorHandling.hpp"
#include "Document.hpp"

#include <SpdLog.hpp>

#include <boost/exception/get_error_info.hpp>
#include <SpdLog.hpp>



void LogError(const XmlDocument& doc,
              const XmlError& error,
              const std::string_view& filename)
{
	uint32_t line = 0;
	std::string_view msg = "<no message>";

	auto node = boost::get_error_info<XmlErrorNodeInfo>(error);
	if (node)
		line = doc.GetLineFromOffset((*node)->offset() - doc.GetContent().data());

	auto realMsg = boost::get_error_info<XmlErrorMsgInfo>(error);
	if (realMsg)
		msg = *realMsg;

	spdlog::error("{0}:{1}: {2}", filename, line, msg);
}


