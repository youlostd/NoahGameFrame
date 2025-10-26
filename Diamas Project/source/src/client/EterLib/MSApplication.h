#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MSWindow.h"

class CMSApplication : public CMSWindow
{
public:
    CMSApplication();
    virtual ~CMSApplication();

    void Initialize(HINSTANCE hInstance);


protected:

    LRESULT WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
};
