#include "../eterlib/Camera.h"
#include "PythonApplication.h"
#include "StdAfx.h"

#include <imgui_impl_win32.cpp>
#include <imgui_impl_win32.h>
#include <winuser.h>

static int gs_nMouseCaptureRef = 0;

void CPythonApplication::SafeSetCapture()
{
    SetCapture(m_hWnd);
    gs_nMouseCaptureRef++;
}

void CPythonApplication::SafeReleaseCapture()
{
    gs_nMouseCaptureRef--;
    if (gs_nMouseCaptureRef == 0)
        ReleaseCapture();
}

void CPythonApplication::__SetFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwBPP)
{
    DEVMODE DevMode;
    DevMode.dmSize = sizeof(DevMode);
    DevMode.dmBitsPerPel = dwBPP;
    DevMode.dmPelsWidth = dwWidth;
    DevMode.dmPelsHeight = dwHeight;
    DevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    LONG Error = ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
    if (Error == DISP_CHANGE_RESTART)
    {
        ChangeDisplaySettings(0, 0);
    }
}

void CPythonApplication::__MinimizeFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight)
{
    ChangeDisplaySettings(0, 0);
    SetWindowPos(hWnd, 0, 0, 0, dwWidth, dwHeight, SWP_SHOWWINDOW);
    ShowWindow(hWnd, SW_MINIMIZE);
}

LRESULT CPythonApplication::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard || io.WantCaptureMouse)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uiMsg, wParam, lParam))
            return true;
    }

    if (WebManager::instance().GetFocusedWebView() &&
        (uiMsg == WM_KEYDOWN || uiMsg == WM_KEYUP || uiMsg == WM_CHAR || uiMsg == WM_SYSCHAR ||
         uiMsg == WM_SYSKEYDOWN || uiMsg == WM_SYSKEYUP))
    {
        WebManager::instance().ProcessInputMessage(uiMsg, wParam, lParam);
        return 0;
    }

    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;
    static bool s_fullscreen = false;

    switch (uiMsg)
    {
    case WM_PAINT:
        if (s_in_sizemove)
        {
            Process();
        }
        else
        {
            PAINTSTRUCT ps;
            (void)BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_MOVE:
        OnWindowMoved();
        break;

    case WM_ACTIVATEAPP: {
        m_isActivateWnd = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);

        if (m_isActivateWnd)
        {
            m_SoundManager.RestoreVolume();

            if (m_isWindowFullScreenEnable)
                __SetFullScreenWindow(hWnd, m_dwWidth, m_dwHeight, Engine::GetSettings().GetBPP());
            // prevent keys from getting stuck due to missed keyup events
            KeyboardInput &keyboard = GetKeyboardInput();
            for (KeyCode i = 0; i < std::numeric_limits<KeyCode>::max(); i++)
            {
                if (keyboard.IsKeyPressed(i))
                    OnKeyUp(i);
            }
        }
        else
        {
            m_SoundManager.SaveVolume();
            if (m_isWindowFullScreenEnable)
                __MinimizeFullScreenWindow(hWnd, m_dwWidth, m_dwHeight);
        }

        if (m_isActivateWnd)
        {
            OnActivated();
        }
        else
        {
            OnDeactivated();
        }

        break;
    }

    case WM_CHAR: {

        OnChar(wParam);
    }

        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        OnKeyDown(LOWORD(wParam));
        return 0;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        OnKeyUp(LOWORD(wParam));
        return 0;

    case WM_LBUTTONDOWN:
        SafeSetCapture();
        OnMouseLeftButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
        return 0;

    case WM_LBUTTONUP:
        if (hWnd == GetCapture())
        {
            SafeReleaseCapture();
            OnMouseLeftButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
        }
        return 0;

    case WM_LBUTTONDBLCLK:
        SafeSetCapture();
        OnMouseLeftButtonDoubleClick(short(LOWORD(lParam)), short(HIWORD(lParam)));
        return 0;

    case WM_MBUTTONDOWN:
        SafeSetCapture();
        OnMouseMiddleButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
        break;

    case WM_MBUTTONUP:
        if (GetCapture() == hWnd)
        {
            SafeReleaseCapture();
            OnMouseMiddleButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
        }
        break;

    case WM_RBUTTONDOWN:
        SafeSetCapture();
        OnMouseRightButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
        return 0;

    case WM_RBUTTONUP:
        if (hWnd == GetCapture())
        {
            SafeReleaseCapture();
            OnMouseRightButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
        }
        return 0;

        //...
        // case 0x20a:
    case WM_MOUSEWHEEL:
        OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        break;

    case WM_SIZE:
        switch (wParam)
        {
        case SIZE_RESTORED:
        case SIZE_MAXIMIZED: {
            RECT rcWnd;
            GetClientRect(&rcWnd);

            UINT uWidth = rcWnd.right - rcWnd.left;
            UINT uHeight = rcWnd.bottom - rcWnd.left;
            m_grpDevice.ResizeBackBuffer(uWidth, uHeight);
        }
        break;
        }

        if (wParam == SIZE_MINIMIZED)
        {
            if (!m_isMinimizedWnd)
            {
                m_isMinimizedWnd = true;
                if (!s_in_suspend)
                    OnSuspending();
                s_in_suspend = true;
            }
        }
        else if (m_isMinimizedWnd)
        {
            m_isMinimizedWnd = false;
            if (s_in_suspend)
                OnResuming();
            s_in_suspend = false;
        }

        OnSizeChange(short(LOWORD(lParam)), short(HIWORD(lParam)));
        break;

    case WM_EXITSIZEMOVE: {
        RECT rcWnd;
        GetClientRect(&rcWnd);

        UINT uWidth = rcWnd.right - rcWnd.left;
        UINT uHeight = rcWnd.bottom - rcWnd.left;
        m_grpDevice.ResizeBackBuffer(uWidth, uHeight);
        OnSizeChange(short(LOWORD(lParam)), short(HIWORD(lParam)));
    }
    break;

    case WM_SETCURSOR:
        if (IsActive())
        {
            if (m_bCursorVisible && CURSOR_MODE_HARDWARE == m_iCursorMode)
            {
                SetCursor((HCURSOR)m_hCurrentCursor);
                return 0;
            }
            else
            {
                SetCursor(NULL);
                return 0;
            }
        }
        break;

    case WM_CLOSE:
#ifdef _DEBUG
        PostQuitMessage(0);
#else
        RunPressExitKey();
#endif
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_MENUCHAR:
        // A menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);

    default:
        // Tracenf("%x msg %x", timeGetTime(), uiMsg);
        break;
    }

    return CMSApplication::WindowProcedure(hWnd, uiMsg, wParam, lParam);
}
