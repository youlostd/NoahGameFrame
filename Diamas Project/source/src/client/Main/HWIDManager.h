#ifndef _HWIDMANAGER_H____
#define _HWIDMANAGER_H____

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <string>

namespace HWIDMANAGER
{
	void InitHardwareId();
    const std::string& getMachineGUID();
    const std::string& getMacAddr();
    const std::string& getCPUid();

};

#endif // HWIDMANAGER
