#ifndef __INC_ETERLIB_DEBUG_H__
#define __INC_ETERLIB_DEBUG_H__

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#include <windows.h>


extern void LogBox(const char* c_szMsg, const char * c_szCaption = nullptr, HWND hWnd = nullptr);
extern void LogBoxf(const char* c_szMsg, ...);



extern HWND g_PopupHwnd;

#define CHECK_RETURN(flag, string)			\
	if (flag)								\
	{										\
		LogBox(string);						\
		return;								\
	}										\



#endif
