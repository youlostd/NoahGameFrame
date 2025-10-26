#ifndef METIN2_VFS_UTIL_HPP
#define METIN2_VFS_UTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <optional>

METIN2_BEGIN_NS

class Vfs;

std::optional<std::string> LoadFileToString(Vfs& vfs,
                      const std::string& filename);

METIN2_END_NS

#endif
