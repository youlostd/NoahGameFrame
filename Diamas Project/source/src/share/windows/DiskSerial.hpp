#ifndef METIN2_WINDOWS_DISKSERIAL_HPP
#define METIN2_WINDOWS_DISKSERIAL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vstl/string.hpp>

METIN2_BEGIN_NS

bool GetDriveSerialNumber(const char* drivePath, std::string& serialNumber);
bool GetDriveSerialNumberWmi(const char* drivePath, std::string& serialNumber);

bool GetDriveSerial(uint32_t driveIndex, std::string& serialNumber);

METIN2_END_NS

#endif
