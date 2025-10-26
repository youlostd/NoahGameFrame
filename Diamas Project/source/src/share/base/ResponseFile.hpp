#ifndef METIN2_BASE_RESPONSEFILE_HPP
#define METIN2_BASE_RESPONSEFILE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <vector>
#include <string>
#include <string_view>
#include <utility>



// Additional command line parser which interprets '@something' as a
// option "config-file" with the value "something"
std::pair<std::string, std::string> AtOptionParser(const std::string& s);

bool ParseResponseFile(const std::string_view& filename,
                       std::vector<std::string>& args);



#endif
