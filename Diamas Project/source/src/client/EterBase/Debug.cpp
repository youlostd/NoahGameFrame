#include "StdAfx.h"

#include <ctime>
#include <stdio.h>
#include "Debug.h"
#include <base/Singleton.hpp>
#include "Timer.h"

HWND g_PopupHwnd = NULL;

void LogBoxf(const char *c_szFormat, ...)
{
    va_list args;
    va_start(args, c_szFormat);

    char szBuf[2048];
    _vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);

    LogBox(szBuf);
}

void LogBox(const char *c_szMsg, const char *c_szCaption, HWND hWnd)
{
    if (!hWnd)
        hWnd = g_PopupHwnd;

    MessageBox(hWnd, c_szMsg, c_szCaption ? c_szCaption : "LOG", MB_OK);
}
