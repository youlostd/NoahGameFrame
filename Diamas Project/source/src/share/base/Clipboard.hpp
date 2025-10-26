#ifndef METIN2_BASE_CLIPBOARD_HPP
#define METIN2_BASE_CLIPBOARD_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/io/File.hpp>
#include <storm/io/FileLoader.hpp>
#include <storm/Util.hpp>

#include <boost/system/error_code.hpp>

#include <utility>
#include <memory>



typedef unsigned int ClipboardType;

extern ClipboardType CF_METIN2;

storm::String GetClipboardText();
std::pair<std::unique_ptr<uint8_t>, size_t> GetClipboardContent(ClipboardType type);

bool ClearClipboard();

bool SetClipboardContent(ClipboardType type, const void* data, size_t len);
bool SetClipboardText(const storm::StringRef& str);

bool ClipboardInit();



#endif
