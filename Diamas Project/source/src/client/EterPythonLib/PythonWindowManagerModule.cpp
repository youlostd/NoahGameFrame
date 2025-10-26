#include "StdAfx.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonGridSlotWindow.h"
#include "EditControl.hpp"

PyObject *wndMgrGetAspect(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("f", UI::CWindowManager::Instance().GetAspect());
}

PyObject *wndMgrOnceIgnoreMouseLeftButtonUpEvent(PyObject *poSelf, PyObject *poArgs)
{
    //UI::CWindowManager::Instance().OnceIgnoreMouseLeftButtonUpEvent();
    Py_RETURN_NONE;
}

PyObject *wndMgrSetMouseHandler(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poHandler;
    if (!PyTuple_GetObject(poArgs, 0, &poHandler))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().SetMouseHandler(poHandler);
    Py_RETURN_NONE;
}

void CapsuleDestroyer(PyObject *capsule)
{
    auto rawPtr = static_cast<UI::CWindow *>(PyCapsule_GetPointer(capsule, nullptr));
    UI::CWindowManager::instance().DestroyWindow(rawPtr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///// Register /////
// Window
template <class T>
PyObject *wndMgrRegister(PyObject *, PyObject *args)
{
    PyObject *po;
    std::string szLayer;
    if (!PyTuple_GetObject(args, 0, &po) ||
        !PyTuple_GetString(args, 1, szLayer))
        return Py_BadArgument();
    auto win = UI::CWindowManager::Instance().RegisterWindow<T>(po, szLayer);
    auto capsule = PyCapsule_New(win, nullptr, CapsuleDestroyer);
    if(capsule)
    {
        return capsule;
    }

    return Py_BuildException("Could not properly register window");
}

///// Register /////
/////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *wndMgrDestroy(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    if (!pWin)
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().DestroyWindow(pWin);
    Py_RETURN_NONE;
}

PyObject *wndMgrIsFocus(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    return Py_BuildValue("i", win == UI::CWindowManager::Instance().GetFocus());
}

PyObject *wndMgrSetFocus(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    UI::CWindowManager::Instance().SetFocus(win);
    Py_RETURN_NONE;
}

PyObject *wndMgrGetFocus(PyObject *poSelf, PyObject *poArgs)
{
    auto wnd = UI::CWindowManager::Instance().GetFocus();
    if (!wnd)
        Py_RETURN_NONE;

    auto h = wnd->GetHandler();
    if (!h)
        Py_RETURN_NONE;

    return Py_BuildValue("O", h);
}

PyObject *wndMgrKillFocus(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (PyTuple_GetWindow(poArgs, 0, &win))
    {
        if (win == UI::CWindowManager::Instance().GetFocus())
            UI::CWindowManager::Instance().KillFocus();
    }
    else
    {
        UI::CWindowManager::Instance().KillFocus();
    }

    Py_RETURN_NONE;
}

PyObject *wndMgrLock(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().LockWindow(pWin);
    Py_RETURN_NONE;
}

PyObject *wndMgrUnlock(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindowManager::Instance().UnlockWindow();
    Py_RETURN_NONE;
}

PyObject *wndMgrCaptureMouse(PyObject *, PyObject *args)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(args, 0, &win))
        return Py_BadArgument();

    UI::CWindowManager::Instance().CaptureMouse(win);
    Py_RETURN_NONE;
}

PyObject *wndMgrReleaseMouse(PyObject *, PyObject *args)
{
    UI::CWindow *win = nullptr;
    PyTuple_GetWindow(args, 0, &win);

    UI::CWindowManager::Instance().ReleaseMouse(win);
    Py_RETURN_NONE;
}

PyObject *wndMgrGetCapture(PyObject *, PyObject *args)
{
    auto* win = UI::CWindowManager::Instance().GetCapture();
    if (!win)
        Py_RETURN_NONE;

    auto* h = win->GetHandler();
    if (h)
        return h;

    Py_RETURN_NONE;
}

PyObject *wndMgrVideoFrameOpen(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    std::string szName;
    if (!PyTuple_GetString(poArgs, 1, szName))
    {
        return Py_BuildException();
    }

    ((UI::CVideoFrame *)(pWin))->Open(szName.c_str());
    Py_RETURN_NONE;
}

PyObject *wndMgrSetName(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    std::string szName;
    if (!PyTuple_GetString(poArgs, 1, szName))
    {
        return Py_BuildException();
    }

    pWin->SetName(szName);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetTop(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().SetTop(pWin);
    Py_RETURN_NONE;
}

PyObject *wndMgrShow(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    pWin->Show();
    Py_RETURN_NONE;
}

PyObject *wndMgrHide(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    pWin->Hide();
    Py_RETURN_NONE;
}

PyObject *wndMgrIsAttachedTo(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    UI::CWindow *pWinAttached;
    if (!PyTuple_GetWindow(poArgs, 1, &pWinAttached))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", 0);
}

PyObject *wndMgrSetAttachedTo(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    UI::CWindow *pWinAttachedTo;
    if (!PyTuple_GetWindow(poArgs, 1, &pWinAttachedTo))
    {
        return Py_BuildException();
    }


    return Py_BuildNone();
}

PyObject *wndMgrIsAttaching(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", 0);
}

PyObject *wndMgrIsRendering(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pWin->IsRendering());
}

PyObject *wndMgrIsShow(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pWin->IsShow());
}

PyObject *wndMgrIsRTL(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pWin->IsFlag(UI::CWindow::FLAG_RTL));
}

PyObject *wndMgrGetAlpha(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
        return Py_BuildException();

    return Py_BuildValue("f", pWindow->GetAlpha());
}

PyObject *wndMgrSetAlpha(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
        return Py_BuildException();
    float fAlpha;
    if (!PyTuple_GetFloat(poArgs, 1, &fAlpha))
        return Py_BuildException();

    pWindow->SetSingleAlpha(fAlpha);

    return Py_BuildNone();
}

PyObject *wndMgrSetAllAlpha(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
        return Py_BuildException();
    float fAlpha;
    if (!PyTuple_GetFloat(poArgs, 1, &fAlpha))
        return Py_BuildException();

    pWin->SetAllAlpha(fAlpha);

    return Py_BuildNone();
}

PyObject *wndMgrSetScreenSize(PyObject *poSelf, PyObject *poArgs)
{
    int width;
    if (!PyTuple_GetInteger(poArgs, 0, &width))
    {
        return Py_BuildException();
    }
    int height;
    if (!PyTuple_GetInteger(poArgs, 1, &height))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().SetScreenSize(width, height);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetParent(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    UI::CWindow *pParentWin;
    if (!PyTuple_GetWindow(poArgs, 1, &pParentWin))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().SetParent(pWin, pParentWin);
    Py_RETURN_NONE;
}

PyObject *wndMgrGetParent(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    auto wnd = win->GetParent();
    if (!wnd)
        Py_RETURN_NONE;

    auto handler = wnd->GetHandler();
    if (!handler)
        Py_RETURN_NONE;

    return Py_BuildValue("O", handler);
}

PyObject *wndMgrSetPickAlways(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    //UI::CWindowManager::Instance().SetPickAlways (pWin);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetWndSize(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int width;
    if (!PyTuple_GetInteger(poArgs, 1, &width))
    {
        return Py_BuildException();
    }
    int height;
    if (!PyTuple_GetInteger(poArgs, 2, &height))
    {
        return Py_BuildException();
    }

    pWin->SetSize(width, height);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetWndPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int x;
    if (!PyTuple_GetInteger(poArgs, 1, &x))
    {
        return Py_BuildException();
    }
    int y;
    if (!PyTuple_GetInteger(poArgs, 2, &y))
    {
        return Py_BuildException();
    }

    pWin->SetPosition(x, y);
    Py_RETURN_NONE;
}

PyObject *wndMgrGetName(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("s", pWin->GetName());
}

PyObject *wndMgrGetWndWidth(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pWin->GetWidth());
}

PyObject *wndMgrGetWndHeight(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pWin->GetHeight());
}

PyObject *wndMgrGetWndLocalPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    auto x = win->GetPositionX();
    auto y = win->GetPositionY();
    return Py_BuildValue("ii", x, y);
}

PyObject *wndMgrGetWndGlobalPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    RECT &rRect = pWindow->GetRect();
    return Py_BuildValue("ii", rRect.left, rRect.top);
}

PyObject *wndMgrGetWindowRect(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    RECT &rRect = pWin->GetRect();
    return Py_BuildValue("iiii", rRect.left, rRect.top, rRect.right - rRect.left, rRect.bottom - rRect.top);
}

PyObject *wndMgrGetWindowBaseRect(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    RECT &rRect = pWin->GetRect();
    return Py_BuildValue("iiii", rRect.left, rRect.top, rRect.right, rRect.bottom);
}

PyObject *wndMgrSetWindowHorizontalAlign(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iAlign;
    if (!PyTuple_GetInteger(poArgs, 1, &iAlign))
    {
        return Py_BuildException();
    }

    pWin->SetHorizontalAlign(iAlign);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetWindowVerticalAlign(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iAlign;
    if (!PyTuple_GetInteger(poArgs, 1, &iAlign))
    {
        return Py_BuildException();
    }

    pWin->SetVerticalAlign(iAlign);

    Py_RETURN_NONE;
}

PyObject *wndMgrIsIn(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pWin == UI::CWindowManager::Instance().GetPointWindow());
}

PyObject *wndMgrGetMouseLocalPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    auto [lx, ly] = UI::CWindowManager::Instance().GetMousePosition();

    win->MakeLocalPosition(lx, ly);
    return Py_BuildValue("ii", lx, ly);
}

PyObject *wndMgrGetScreenWidth(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", UI::CWindowManager::Instance().GetScreenWidth());
}

PyObject *wndMgrGetScreenHeight(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", UI::CWindowManager::Instance().GetScreenHeight());
}

PyObject *wndMgrGetMousePosition(PyObject *poSelf, PyObject *poArgs)
{
    auto [lx, ly] = UI::CWindowManager::Instance().GetMousePosition();

    return Py_BuildValue("ii", lx, ly);
}

PyObject *wndMgrIsDragging(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", UI::CWindowManager::Instance().IsDragging());
}

PyObject *wndMgrGetPickedWindow(PyObject *poSelf, PyObject *poArgs)
{
    auto wnd = UI::CWindowManager::Instance().GetPointWindow();
    if (!wnd)
        Py_RETURN_NONE;

    auto h = wnd->GetHandler();
    if (!h)
        Py_RETURN_NONE;

    // This increments the reference count by two do we really want this?
    return Py_BuildValue("O", h);
}

PyObject *wndMgrIsPickedWindow(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    UI::CWindow *pPickedWin = UI::CWindowManager::Instance().GetPointWindow();
    return Py_BuildValue("i", win == pPickedWin ? 1 : 0);
}

uint32_t GetFlag(const char *pszFlag)
{
    if (!stricmp(pszFlag, "moveable"))
        return UI::CWindow::FLAG_MOVABLE;
    else if (!stricmp(pszFlag, "dragable"))
        return UI::CWindow::FLAG_DRAGABLE;
    else if (!stricmp(pszFlag, "attach"))
        return UI::CWindow::FLAG_ATTACH;
    else if (!stricmp(pszFlag, "restrict_x"))
        return UI::CWindow::FLAG_RESTRICT_X;
    else if (!stricmp(pszFlag, "restrict_y"))
        return UI::CWindow::FLAG_RESTRICT_Y;
    else if (!stricmp(pszFlag, "float"))
        return UI::CWindow::FLAG_FLOAT;
    else if (!stricmp(pszFlag, "not_pick"))
        return UI::CWindow::FLAG_NOT_PICK;
    else if (!stricmp(pszFlag, "ignore_size"))
        return UI::CWindow::FLAG_IGNORE_SIZE;
    else if (!stricmp(pszFlag, "rtl"))
        return UI::CWindow::FLAG_RTL;
    else if (!stricmp(pszFlag, "focusable"))
        return UI::CWindow::FLAG_FOCUSABLE;
    else if (!stricmp(pszFlag, "animated_board"))
        return UI::CWindow::FLAG_ANIMATED_BOARD;
    else if (!stricmp(pszFlag, "component"))
        return UI::CWindow::FLAG_COMPONENT;
    else
        return 0;
}

PyObject *wndMgrAddFlag(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    std::string pszFlag;
    if (!PyTuple_GetString(poArgs, 1, pszFlag))
        return Py_BadArgument();

    if (!pszFlag.empty())
    {
        uint32_t flag = GetFlag(pszFlag.c_str());
        if (!flag)
            SPDLOG_WARN("Unknown window flag {0}", pszFlag);
        else
            win->AddFlag(flag);
    }

    Py_RETURN_NONE;
}

PyObject *wndMgrRemoveFlag(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    std::string pszFlag;
    if (!PyTuple_GetString(poArgs, 1, pszFlag))
        return Py_BadArgument();

    if (!pszFlag.empty())
    {
        uint32_t flag = GetFlag(pszFlag.c_str());
        if (!flag)
            SPDLOG_WARN("Unknown window flag {0}", pszFlag);
        else
            win->RemoveFlag(flag);
    }

    Py_RETURN_NONE;
}

PyObject *wndMgrEnableClipping(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    win->EnableClipping();

    Py_RETURN_NONE;
}

PyObject *wndMgrDisableClipping(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    win->DisableClipping();

    Py_RETURN_NONE;
}

PyObject *wndMgrUpdateRect(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    pWin->UpdateRect();
    Py_RETURN_NONE;
}

PyObject *wndMgrOnKeyDown(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *win;
    if (!PyTuple_GetWindow(poArgs, 0, &win))
        return Py_BadArgument();

    KeyCode c;
    if (!PyTuple_GetByte(poArgs, 1, &c))
        return Py_BadArgument();

    return PyBool_FromLong(win->OnKeyDown(c));
}

PyObject *wndMgrAppendSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
    {
        return Py_BuildException();
    }

    int ixPosition;
    if (!PyTuple_GetInteger(poArgs, 2, &ixPosition))
    {
        return Py_BuildException();
    }

    int iyPosition;
    if (!PyTuple_GetInteger(poArgs, 3, &iyPosition))
    {
        return Py_BuildException();
    }

    int ixCellSize;
    if (!PyTuple_GetInteger(poArgs, 4, &ixCellSize))
    {
        return Py_BuildException();
    }

    int iyCellSize;
    if (!PyTuple_GetInteger(poArgs, 5, &iyCellSize))
    {
        return Py_BuildException();
    }

    int iPlacementX;
    if (!PyTuple_GetInteger(poArgs, 6, &iPlacementX))
    {
        return Py_BuildException();
    }

    int iPlacementY;
    if (!PyTuple_GetInteger(poArgs, 7, &iPlacementY))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = static_cast<UI::CSlotWindow *>(pWin);
    pSlotWin->AppendSlot(iIndex, ixPosition, iyPosition, ixCellSize, iyCellSize, iPlacementX, iPlacementY);

    Py_RETURN_NONE;
}

PyObject *wndMgrArrangeSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iStartIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iStartIndex))
    {
        return Py_BuildException();
    }

    int ixCellCount;
    if (!PyTuple_GetInteger(poArgs, 2, &ixCellCount))
    {
        return Py_BuildException();
    }

    int iyCellCount;
    if (!PyTuple_GetInteger(poArgs, 3, &iyCellCount))
    {
        return Py_BuildException();
    }

    int ixCellSize;
    if (!PyTuple_GetInteger(poArgs, 4, &ixCellSize))
    {
        return Py_BuildException();
    }

    int iyCellSize;
    if (!PyTuple_GetInteger(poArgs, 5, &iyCellSize))
    {
        return Py_BuildException();
    }

    int ixBlank;
    if (!PyTuple_GetInteger(poArgs, 6, &ixBlank))
    {
        return Py_BuildException();
    }

    int iyBlank;
    if (!PyTuple_GetInteger(poArgs, 7, &iyBlank))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CGridSlotWindow::Type()))
    {
        SPDLOG_ERROR("wndMgr.ArrangeSlot : not a grid window");
        return Py_BuildException();
    }

    UI::CGridSlotWindow *pGridSlotWin = (UI::CGridSlotWindow *)pWin;
    pGridSlotWin->ArrangeGridSlot(iStartIndex, ixCellCount, iyCellCount, ixCellSize, iyCellSize, ixBlank, iyBlank);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetCoverButtonScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int index;
    if (!PyTuple_GetInteger(poArgs, 1, &index))
    {
        return Py_BuildException();
    }

    float fx;
    if (!PyTuple_GetFloat(poArgs, 2, &fx))
    {
        return Py_BuildException();
    }
    float fy;
    if (!PyTuple_GetFloat(poArgs, 3, &fy))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        SPDLOG_ERROR("wndMgr : not a slot window");
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetCoverButtonScale(index, fx, fy);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotBaseImageScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    float fx;
    if (!PyTuple_GetFloat(poArgs, 1, &fx))
    {
        return Py_BuildException();
    }
    float fy;
    if (!PyTuple_GetFloat(poArgs, 2, &fy))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        SPDLOG_ERROR("wndMgr.ArrangeSlot : not a slot window");
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotBaseImageScale(fx, fy);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotBaseImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

    float fr;
    if (!PyTuple_GetFloat(poArgs, 2, &fr))
    {
        return Py_BuildException();
    }
    float fg;
    if (!PyTuple_GetFloat(poArgs, 3, &fg))
    {
        return Py_BuildException();
    }
    float fb;
    if (!PyTuple_GetFloat(poArgs, 4, &fb))
    {
        return Py_BuildException();
    }
    float fa;
    if (!PyTuple_GetFloat(poArgs, 5, &fa))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        SPDLOG_ERROR("wndMgr.ArrangeSlot : not a slot window");
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotBaseImage(szFileName.c_str(), fr, fg, fb, fa);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    float fx;
    if (!PyTuple_GetFloat(poArgs, 1, &fx))
    {
        return Py_BuildException();
    }
    float fy;
    if (!PyTuple_GetFloat(poArgs, 2, &fy))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = static_cast<UI::CSlotWindow *>(pWin);
    pSlotWin->SetScale(fx, fy);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetCoverButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    std::string szUpImageName;
    if (!PyTuple_GetString(poArgs, 2, szUpImageName))
    {
        return Py_BuildException();
    }
    std::string szOverImageName;
    if (!PyTuple_GetString(poArgs, 3, szOverImageName))
    {
        return Py_BuildException();
    }
    std::string szDownImageName;
    if (!PyTuple_GetString(poArgs, 4, szDownImageName))
    {
        return Py_BuildException();
    }
    std::string szDisableImageName;
    if (!PyTuple_GetString(poArgs, 5, szDisableImageName))
    {
        return Py_BuildException();
    }

    int iLeftButtonEnable;
    if (!PyTuple_GetInteger(poArgs, 6, &iLeftButtonEnable))
    {
        return Py_BuildException();
    }
    int iRightButtonEnable;
    if (!PyTuple_GetInteger(poArgs, 7, &iRightButtonEnable))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->SetCoverButton(iSlotIndex, szUpImageName.c_str(), szOverImageName.c_str(), szDownImageName.c_str(),
                             szDisableImageName.c_str(), iLeftButtonEnable, iRightButtonEnable);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotCoverImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    std::string c_szFilename;
    if (!PyTuple_GetString(poArgs, 2, c_szFilename))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->SetSlotCoverImage(iSlotIndex, c_szFilename.c_str());

    Py_RETURN_NONE;
}

PyObject *wndMgrEnableSlotCoverImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    bool bEnable;
    if (!PyTuple_GetBoolean(poArgs, 2, &bEnable))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->EnableSlotCoverImage(iSlotIndex, bEnable);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetRenderTarget(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iRenderTarget;
    if (!PyTuple_GetInteger(poArgs, 1, &iRenderTarget))
    {
        return Py_BuildException();
    }

    UI::CRenderTarget *pRenderTarget = static_cast<UI::CRenderTarget *>(pWindow);
    pRenderTarget->SetRenderTarget(iRenderTarget);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetWikiRenderTarget(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
        return Py_BuildException();

    int renderWindow = -1;
    if (!PyTuple_GetInteger(poArgs, 1, &renderWindow))
        return Py_BuildException();

    ((UI::CRenderTarget *)pWin)->SetWikiRenderTarget(renderWindow);
    return Py_BuildNone();
}

PyObject *wndMgrWebViewLoadUrl(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    std::string url;
    if (!PyTuple_GetString(poArgs, 1, url))
    {
        return Py_BuildException();
    }

    UI::CWebView *pRenderTarget = static_cast<UI::CWebView *>(pWindow);
    pRenderTarget->LoadUrl(url.c_str());

    Py_RETURN_NONE;
}

PyObject *wndMgrWebViewOnCreateInstance(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    UI::CWebView *pRenderTarget = static_cast<UI::CWebView *>(pWindow);
    pRenderTarget->OnCreateInstance(UI::CWindowManager::Instance().GetAppWindow());

    Py_RETURN_NONE;
}

PyObject *wndMgrButtonSetAlwaysToolTip(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    bool bAlwaysTooltip;
    if (!PyTuple_GetBoolean(poArgs, 1, &bAlwaysTooltip))
    {
        return Py_BuildException();
    }

    UI::CButton *pButton = static_cast<UI::CButton *>(pWindow);
    pButton->SetAlwaysTooltip(bAlwaysTooltip);

    Py_RETURN_NONE;
}

PyObject *wndMgrEnableCoverButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->EnableCoverButton(iSlotIndex);

    Py_RETURN_NONE;
}

PyObject *wndMgrDisableCoverButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->DisableCoverButton(iSlotIndex);

    Py_RETURN_NONE;
}

PyObject *wndMgrIsDisableCoverButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    return Py_BuildValue("i", pSlotWin->IsDisableCoverButton(iSlotIndex));
}

PyObject *wndMgrSetAlwaysRenderCoverButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    bool bAlwaysRender = false;

    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetBoolean(poArgs, 2, &bAlwaysRender))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->SetAlwaysRenderCoverButton(iSlotIndex, bAlwaysRender);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    uint32_t index;
    if (!PyTuple_GetUnsignedInteger(poArgs, 1, &index))
        return Py_BadArgument();

    std::string upImageName;
    if (!PyTuple_GetString(poArgs, 2, upImageName))
        return Py_BadArgument();

    std::string overImageName;
    if (!PyTuple_GetString(poArgs, 3, overImageName))
        return Py_BadArgument();

    std::string downImageName;
    if (!PyTuple_GetString(poArgs, 4, downImageName))
        return Py_BadArgument();

    auto slotWin = static_cast<UI::CSlotWindow *>(window);
    slotWin->SetSlotButton(index, upImageName.c_str(), overImageName.c_str(), downImageName.c_str());
    Py_RETURN_NONE;
}

PyObject *wndMgrAppendSlotButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    std::string szUpImageName;
    if (!PyTuple_GetString(poArgs, 1, szUpImageName))
    {
        return Py_BuildException();
    }
    std::string szOverImageName;
    if (!PyTuple_GetString(poArgs, 2, szOverImageName))
    {
        return Py_BuildException();
    }
    std::string szDownImageName;
    if (!PyTuple_GetString(poArgs, 3, szDownImageName))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->AppendSlotButton(szUpImageName.c_str(), szOverImageName.c_str(), szDownImageName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndMgrAppendRequirementSignImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    std::string szImageName;
    if (!PyTuple_GetString(poArgs, 1, szImageName))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->AppendRequirementSignImage(szImageName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndMgrShowSlotButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->ShowSlotButton(iSlotNumber);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotButtonPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
    {
        return Py_BuildException();
    }

    int x;
    if (!PyTuple_GetInteger(poArgs, 2, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 3, &y))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->SetSlotButtonPosition(iSlotNumber, x, y);

    Py_RETURN_NONE;
}

PyObject *wndMgrHideAllSlotButton(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->HideAllSlotButton();

    Py_RETURN_NONE;
}

PyObject *wndMgrShowRequirementSign(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->ShowRequirementSign(iSlotNumber);

    Py_RETURN_NONE;
}

PyObject *wndMgrHideRequirementSign(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWindow;
    pSlotWin->HideRequirementSign(iSlotNumber);

    Py_RETURN_NONE;
}

PyObject *wndMgrRefreshSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->RefreshSlot();

    Py_RETURN_NONE;
}

PyObject *wndMgrSetUseMode(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetUseMode(iFlag);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetItemDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;

    DWORD index;
    float r, g, b, a;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetUnsignedLong(poArgs, 1, &index))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetFloat(poArgs, 2, &r))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetFloat(poArgs, 3, &g))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetFloat(poArgs, 4, &b))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetFloat(poArgs, 5, &a))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetItemDiffuseColor(index, r, g, b, a);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetUsableItem(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetUsableItem(iFlag);

    Py_RETURN_NONE;
}

PyObject *wndMgrClearSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->ClearSlot(iSlotIndex);

    Py_RETURN_NONE;
}

PyObject *wndMgrClearAllSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->ClearAllSlot();

    Py_RETURN_NONE;
}

PyObject *wndMgrHasSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;

    return Py_BuildValue("i", pSlotWin->HasSlot(iSlotIndex));
}

PyObject *wndMgrSetSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int iItemIndex;
    if (!PyTuple_GetInteger(poArgs, 2, &iItemIndex))
    {
        return Py_BuildException();
    }

    int iWidth;
    if (!PyTuple_GetInteger(poArgs, 3, &iWidth))
    {
        return Py_BuildException();
    }

    int iHeight;
    if (!PyTuple_GetInteger(poArgs, 4, &iHeight))
    {
        return Py_BuildException();
    }

    std::string imageFileName;
    if (!PyTuple_GetString(poArgs, 5, imageFileName))
        return Py_BadArgument();

    DirectX::SimpleMath::Color diffuseColor;
    PyObject *pTuple;
    if (!PyTuple_GetObject(poArgs, 6, &pTuple))
    {
        diffuseColor = DirectX::SimpleMath::Color(1.0, 1.0, 1.0, 1.0);
        //return Py_BuildException();
    }
    else
        // get diffuse color from pTuple
    {
        if (PyTuple_Size(pTuple) != 4)
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetFloat(pTuple, 0, &diffuseColor.x))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetFloat(pTuple, 1, &diffuseColor.y))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetFloat(pTuple, 2, &diffuseColor.z))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetFloat(pTuple, 3, &diffuseColor.w))
        {
            return Py_BuildException();
        }
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    auto image = CResourceManager::instance().LoadResource<CGraphicImage>(imageFileName);

    pSlotWin->SetSlot(iSlotIndex, iItemIndex, iWidth, iHeight, image, diffuseColor);

    Py_RETURN_NONE;
}


PyObject *wndMgrSetSlotCount(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int iCount;
    if (!PyTuple_GetInteger(poArgs, 2, &iCount))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotCount(iSlotIndex, iCount);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotBackground(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 2, szFileName))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotBackground(iSlotIndex, szFileName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotCountNew(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int iGrade;
    if (!PyTuple_GetInteger(poArgs, 2, &iGrade))
    {
        return Py_BuildException();
    }

    int iCount;
    if (!PyTuple_GetInteger(poArgs, 3, &iCount))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotCountNew(iSlotIndex, iGrade, iCount);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotSlotText(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    std::string text;
    if (!PyTuple_GetString(poArgs, 2, text))
    {
        return Py_BuildException();
    }


    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotSlotText(iSlotIndex, text);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotSlotNumber(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int iGrade;
    if (!PyTuple_GetInteger(poArgs, 2, &iGrade))
    {
        return Py_BuildException();
    }

    int iCount;
    if (!PyTuple_GetInteger(poArgs, 3, &iCount))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotSlotNumber(iSlotIndex, iGrade, iCount);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetRealSlotNumber(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int iRealSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 2, &iRealSlotNumber))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetRealSlotNumber(iSlotIndex, iRealSlotNumber);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotCoolTime(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    float fCoolTime;
    if (!PyTuple_GetFloat(poArgs, 2, &fCoolTime))
    {
        return Py_BuildException();
    }

    float fElapsedTime = 0.0f;
    PyTuple_GetFloat(poArgs, 3, &fElapsedTime);

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotCoolTime(iSlotIndex, fCoolTime, fElapsedTime);

    Py_RETURN_NONE;
}

PyObject *wndMgrStoreSlotCoolTime(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
        return Py_BuildException();

    int iKey;
    if (!PyTuple_GetInteger(poArgs, 1, &iKey))
        return Py_BuildException();

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 2, &iSlotIndex))
        return Py_BuildException();

    float fCoolTime;
    if (!PyTuple_GetFloat(poArgs, 3, &fCoolTime))
        return Py_BuildException();

    float fElapsedTime = 0.0f;
    PyTuple_GetFloat(poArgs, 4, &fElapsedTime);

    if (!pWin->IsType(UI::CSlotWindow::Type()))
        return Py_BuildException();

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->StoreSlotCoolTime(iKey, iSlotIndex, fCoolTime, fElapsedTime);

    Py_RETURN_NONE;
}

PyObject *wndMgrRestoreSlotCoolTime(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
        return Py_BuildException();

    int iKey;
    if (!PyTuple_GetInteger(poArgs, 1, &iKey))
        return Py_BuildException();

    if (!pWin->IsType(UI::CSlotWindow::Type()))
        return Py_BuildException();

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->RestoreSlotCoolTime(iKey);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotCoolTimeInverse(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    float fCoolTime;
    if (!PyTuple_GetFloat(poArgs, 2, &fCoolTime))
    {
        return Py_BuildException();
    }

    float fElapsedTime = 0.0f;
    PyTuple_GetFloat(poArgs, 3, &fElapsedTime);

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotCoolTimeInverse(iSlotIndex, fCoolTime, fElapsedTime);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotCoolTimeColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    float fCoolTime;
    if (!PyTuple_GetFloat(poArgs, 2, &fCoolTime))
    {
        return Py_BuildException();
    }

    float fElapsedTime = 0.0f;
    PyTuple_GetFloat(poArgs, 3, &fElapsedTime);

    float fColorR = 0.0f;
    PyTuple_GetFloat(poArgs, 4, &fColorR);
    float fColorG = 0.0f;
    PyTuple_GetFloat(poArgs, 5, &fColorG);
    float fColorB = 0.0f;
    PyTuple_GetFloat(poArgs, 6, &fColorB);
    float fColorA = 0.0f;
    PyTuple_GetFloat(poArgs, 7, &fColorA);

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotCoolTimeColor(iSlotIndex, fCoolTime, fElapsedTime, fColorR, fColorG, fColorB, fColorA);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotExpire(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    float fExpireTime;
    if (!PyTuple_GetFloat(poArgs, 2, &fExpireTime))
    {
        return Py_BuildException();
    }

    float fMaxTime = 0.0f;
    PyTuple_GetFloat(poArgs, 3, &fMaxTime);

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotExpireTime(iSlotIndex, fExpireTime, fMaxTime);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetToggleSlot(PyObject *poSelf, PyObject *poArgs)
{
    assert(!"wndMgrSetToggleSlot - 사용하지 않는 함수");
    Py_RETURN_NONE;
}
#ifdef ENABLE_CHANGE_LOOK_SYSTEM

PyObject *wndMgrDisableSlotCoverImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
        return Py_BuildException();

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
        return Py_BuildException();

    if (!pWin->IsType(UI::CSlotWindow::Type()))
        return Py_BuildException();

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->DisableSlotCoverImage(iSlotIndex);

    Py_RETURN_NONE;
}
#endif
PyObject *wndMgrActivateSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->ActivateSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrDeactivateSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->DeactivateSlot(iSlotIndex);
    Py_RETURN_NONE;
}
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject *wndMgrActivateChangeLookSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
        return Py_BuildException();

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
        return Py_BuildException();

    if (!pWin->IsType(UI::CSlotWindow::Type()))
        return Py_BuildException();

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->ActivateChangeLookSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrDeactivateChangeLookSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
        return Py_BuildException();

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
        return Py_BuildException();

    if (!pWin->IsType(UI::CSlotWindow::Type()))
        return Py_BuildException();

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->DeactivateChangeLookSlot(iSlotIndex);
    Py_RETURN_NONE;
}
#endif
PyObject *wndMgrEnableSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->EnableSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrDisableSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->DisableSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetCantMouseEventSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetCantMouseEventSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetCanMouseEventSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetCanMouseEventSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetUnusableSlotOnTopWnd(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = static_cast<UI::CSlotWindow *>(pWin);
    pSlotWin->SetUnusableSlotOnTopWnd(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetUsableSlotOnTopWnd(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetUsableSlotOnTopWnd(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotHighlightedGreeen(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = static_cast<UI::CSlotWindow *>(pWin);
    pSlotWin->SetSlotHighlightedGreeen(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrDisableSlotHighlightedGreen(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->DisableSlotHighlightedGreen(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrGetSlotGlobalPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    DWORD x = 0, y = 0;
    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->GetSlotGlobalPosition(iSlotIndex, &x, &y);

    return Py_BuildValue("ii", x, y);
}

PyObject *wndMgrGetSlotLocalPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    DWORD x = 0, y = 0;
    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->GetSlotLocalPosition(iSlotIndex, &x, &y);

    return Py_BuildValue("ii", x, y);
}

PyObject *wndMgrSetSlotTextPositon(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int x;
    if (!PyTuple_GetInteger(poArgs, 2, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 3, &y))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotTextPosition(iSlotIndex, x, y);

    Py_RETURN_NONE;
}

PyObject *wndMgrShowSlotBaseImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->ShowSlotBaseImage(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrHideSlotBaseImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->HideSlotBaseImage(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetWindowType(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iType;
    if (!PyTuple_GetInteger(poArgs, 1, &iType))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetWindowType(iType);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotType(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iType;
    if (!PyTuple_GetInteger(poArgs, 1, &iType))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotType(iType);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotStyle(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iStyle;
    if (!PyTuple_GetInteger(poArgs, 1, &iStyle))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetSlotStyle(iStyle);

    Py_RETURN_NONE;
}

PyObject *wndMgrSelectSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SelectSlot(iIndex);

    Py_RETURN_NONE;
}

PyObject *wndMgrClearSelected(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->ClearSelected();

    Py_RETURN_NONE;
}

PyObject *wndMgrGetSelectedSlotCount(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    return Py_BuildValue("i", pSlotWin->GetSelectedSlotCount());
}

PyObject *wndMgrGetSelectedSlotNumber(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    return Py_BuildValue("i", pSlotWin->GetSelectedSlotNumber(iSlotNumber));
}

PyObject *wndMgrIsSelectedSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    return Py_BuildValue("i", pSlotWin->isSelectedSlot(iSlotNumber));
}

PyObject *wndMgrGetSlotCount(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    return Py_BuildValue("i", pSlotWin->GetSlotCount());
}

PyObject *wndMgrLockSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->LockSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndMgrUnlockSlot(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->UnlockSlot(iSlotIndex);
    Py_RETURN_NONE;
}

PyObject *wndBarSetColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    DWORD dwColor;
    if (!PyTuple_GetUnsignedLong(poArgs, 1, &dwColor))
    {
        return Py_BuildException();
    }

    if (pWindow->IsType(UI::CBar3D::Type()))
    {
        DWORD dwLeftColor = dwColor;

        DWORD dwRightColor;
        if (!PyTuple_GetUnsignedLong(poArgs, 2, &dwRightColor))
        {
            return Py_BuildException();
        }
        DWORD dwCenterColor;
        if (!PyTuple_GetUnsignedLong(poArgs, 3, &dwCenterColor))
        {
            return Py_BuildException();
        }

        ((UI::CBar3D *)pWindow)->SetColor(dwLeftColor, dwRightColor, dwCenterColor);
    }
    else
    {
        ((UI::CWindow *)pWindow)->SetColor(dwColor);
    }
    Py_RETURN_NONE;
}

PyObject *wndMgrAttachIcon(PyObject *poSelf, PyObject *poArgs)
{
    int iType;
    if (!PyTuple_GetInteger(poArgs, 0, &iType))
    {
        return Py_BuildException();
    }
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
    {
        return Py_BuildException();
    }
    int iSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 2, &iSlotNumber))
    {
        return Py_BuildException();
    }
    int iWidth;
    if (!PyTuple_GetInteger(poArgs, 3, &iWidth))
    {
        return Py_BuildException();
    }
    int iHeight;
    if (!PyTuple_GetInteger(poArgs, 4, &iHeight))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().AttachIcon(iType, iIndex, iSlotNumber, iWidth, iHeight);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetAttachingRealSlotNumber(PyObject *poSelf, PyObject *poArgs)
{
    int iRealSlotNumber;
    if (!PyTuple_GetInteger(poArgs, 0, &iRealSlotNumber))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().SetAttachingRealSlotNumber(iRealSlotNumber);
    Py_RETURN_NONE;
}

PyObject *wndMgrDeattachIcon(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindowManager::Instance().DeattachIcon();
    Py_RETURN_NONE;
}

PyObject *wndMgrSetAttachingFlag(PyObject *poSelf, PyObject *poArgs)
{
    bool bFlag;
    if (!PyTuple_GetBoolean(poArgs, 0, &bFlag))
    {
        return Py_BuildException();
    }

    UI::CWindowManager::Instance().SetAttachingFlag(bFlag);
    Py_RETURN_NONE;
}

// Text

PyObject *wndTextGetLineCount(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CTextLine *)pWindow)->GetLineCount());
}

PyObject *wndTextSetHorizontalAlign(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    int iType;
    if (!PyTuple_GetInteger(poArgs, 1, &iType))
    {
        return Py_BuildException();
    }

    ((UI::CTextLine *)pWindow)->SetHorizontalAlign(iType);
    Py_RETURN_NONE;
}

PyObject *wndTextSetVerticalAlign(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    int iType;
    if (!PyTuple_GetInteger(poArgs, 1, &iType))
    {
        return Py_BuildException();
    }

    ((UI::CTextLine *)pWindow)->SetVerticalAlign(iType);
    Py_RETURN_NONE;
}

// Text
PyObject *wndTextSetSecret(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
        return Py_BadArgument();

    ((UI::CTextLine *)window)->SetSecret(iFlag);
    Py_RETURN_NONE;
}

PyObject *wndTextSetOutline(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
        return Py_BadArgument();

    ((UI::CTextLine *)window)->SetOutline(iFlag);
    Py_RETURN_NONE;
}

PyObject *wndTextSetMultiLine(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
        return Py_BadArgument();

    ((UI::CTextLine *)window)->SetMultiLine(iFlag);
    Py_RETURN_NONE;
}

PyObject *wndTextSetFontName(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();
    std::string szFontName;
    if (!PyTuple_GetString(poArgs, 1, szFontName))
        return Py_BadArgument();

    ((UI::CTextLine *)window)->SetFontName(szFontName);
    Py_RETURN_NONE;
}

PyObject *wndTextSetFontColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    if (2 == PyTuple_Size(poArgs))
    {
        DWORD dwColor;
        if (!PyTuple_GetUnsignedLong(poArgs, 1, &dwColor))
            return Py_BadArgument();
        ((UI::CTextLine *)window)->SetFontColor(dwColor);
    }
    else if (PyTuple_Size(poArgs) == 3)
    {
        DWORD dwColor;
        if (!PyTuple_GetUnsignedLong(poArgs, 1, &dwColor))
            return Py_BadArgument();

        DWORD dwColor2;
        if (!PyTuple_GetUnsignedLong(poArgs, 2, &dwColor2))
            return Py_BadArgument();
        ((UI::CTextLine *)window)->SetFontGradient(dwColor, dwColor2);
    }
    else if (PyTuple_Size(poArgs) >= 4)
    {
        float fr;
        if (!PyTuple_GetFloat(poArgs, 1, &fr))
            return Py_BadArgument();
        float fg;
        if (!PyTuple_GetFloat(poArgs, 2, &fg))
            return Py_BadArgument();
        float fb;
        if (!PyTuple_GetFloat(poArgs, 3, &fb))
            return Py_BadArgument();
        float fa = 1.0f;
        if (PyTuple_Size(poArgs) >= 5)
        {
            if (!PyTuple_GetFloat(poArgs, 4, &fa))
                return Py_BadArgument();
        }

        ((UI::CTextLine *)window)->SetFontColor(Color(fr, fg, fb, fa).BGRA());
    }
    else
    {
        return Py_BadArgument();
    }

    Py_RETURN_NONE;
}

PyObject *wndTextSetLimitWidth(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();
    float fWidth;
    if (!PyTuple_GetFloat(poArgs, 1, &fWidth))
        return Py_BadArgument();

    ((UI::CTextLine *)window)->SetLimitWidth(fWidth);
    Py_RETURN_NONE;
}

PyObject *wndTextSetText(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();
    std::string szText;
    if (!PyTuple_GetString(poArgs, 1, szText))
        return Py_BadArgument();

    ((UI::CTextLine *)window)->SetText(szText);
    Py_RETURN_NONE;
}

PyObject *wndTextGetText(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();


    return Py_BuildValue("s", ((UI::CTextLine *)window)->GetText().c_str());
}

PyObject *wndTextGetHyperlinkAtPos(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    int x;
    if (!PyTuple_GetInteger(poArgs, 1, &x))
        return Py_BadArgument();

    int y;
    if (!PyTuple_GetInteger(poArgs, 2, &y))
        return Py_BadArgument();

    auto textLine = (UI::CTextLine *)window;
    return Py_BuildValue("s", textLine->GetHyperlinkAtPos(x, y).c_str());
}

PyObject *wndTextSetTextClip(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    RECT r;
    if (!PyTuple_GetLong(poArgs, 1, &r.left))
        return Py_BadArgument();
    if (!PyTuple_GetLong(poArgs, 2, &r.top))
        return Py_BadArgument();
    if (!PyTuple_GetLong(poArgs, 3, &r.right))
        return Py_BadArgument();
    if (!PyTuple_GetLong(poArgs, 4, &r.bottom))
        return Py_BadArgument();

    ((UI::CWindow *)window)->SetClipRect(r);
    Py_RETURN_NONE;
}

PyObject *wndEditDisable(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    ((UI::EditControl *)window)->Disable();
    Py_RETURN_NONE;
}

PyObject *wndEditEnable(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    ((UI::EditControl *)window)->Enable();
    Py_RETURN_NONE;
}

PyObject *wndEditSetMax(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    int iMax;
    if (!PyTuple_GetInteger(poArgs, 1, &iMax))
        return Py_BadArgument();

    ((UI::EditControl *)window)->SetMax(iMax);
    Py_RETURN_NONE;
}

PyObject *wndEditSetMaxVisible(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    int iMax;
    if (!PyTuple_GetInteger(poArgs, 1, &iMax))
        return Py_BadArgument();

    ((UI::EditControl *)window)->SetMaxVisible(iMax);
    Py_RETURN_NONE;
}

PyObject *wndEditSetPlaceholderText(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    std::string str;
    if (!PyTuple_GetString(poArgs, 1, str))
        return Py_BadArgument();

    auto wnd = (UI::EditControl *)window;

    wnd->SetPlaceholderText(str);
    Py_RETURN_NONE;
}

PyObject *wndEditSetPlaceholderColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    DWORD color;
    if (!PyTuple_GetUnsignedLong(poArgs, 1, &color))
        return Py_BadArgument();

    auto wnd = (UI::EditControl *)window;

    wnd->SetPlaceholderColor(Color(color));
    Py_RETURN_NONE;
}

PyObject *wndEditInsert(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    std::string str;
    if (!PyTuple_GetString(poArgs, 1, str))
        return Py_BadArgument();

    auto wnd = (UI::EditControl *)window;

    uint32_t pos;
    if (!PyTuple_GetUnsignedInteger(poArgs, 2, &pos))
        pos = wnd->GetCursorPosition();

    wnd->Insert(pos, str);
    Py_RETURN_NONE;
}

PyObject *wndEditErase(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    uint32_t count;
    if (!PyTuple_GetUnsignedInteger(poArgs, 1, &count))
        return Py_BadArgument();

    auto wnd = (UI::EditControl *)window;

    uint32_t pos;
    if (!PyTuple_GetUnsignedInteger(poArgs, 2, &pos))
        pos = wnd->GetCursorPosition();

    wnd->Erase(pos, count);
    Py_RETURN_NONE;
}

PyObject *wndEditGetCursorPosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    auto wnd = (UI::EditControl *)window;
    return PyInt_FromSize_t(wnd->GetCursorPosition());
}

PyObject *wndEditMoveToEnd(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *window;
    if (!PyTuple_GetWindow(poArgs, 0, &window))
        return Py_BadArgument();

    auto wnd = (UI::EditControl *)window;
    wnd->MoveToEnd();
    Py_RETURN_NONE;
}

PyObject *wndNumberSetNumber(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szNumber;
    if (!PyTuple_GetString(poArgs, 1, szNumber))
    {
        return Py_BuildException();
    }

    ((UI::CNumberLine *)pWindow)->SetNumber(szNumber.c_str());

    Py_RETURN_NONE;
}

PyObject *wndNumberSetNumberHorizontalAlignCenter(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CNumberLine *)pWindow)->SetHorizontalAlign(UI::CWindow::HORIZONTAL_ALIGN_CENTER);

    Py_RETURN_NONE;
}

PyObject *wndNumberSetNumberHorizontalAlignRight(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CNumberLine *)pWindow)->SetHorizontalAlign(UI::CWindow::HORIZONTAL_ALIGN_RIGHT);

    Py_RETURN_NONE;
}

PyObject *wndNumberSetPath(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szPath;
    if (!PyTuple_GetString(poArgs, 1, szPath))
    {
        return Py_BuildException();
    }

    ((UI::CNumberLine *)pWindow)->SetPath(szPath.c_str());

    Py_RETURN_NONE;
}

PyObject *wndMarkBox_SetImageFilename(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

    ((UI::CMarkBox *)pWindow)->LoadImage(szFileName.c_str());
    Py_RETURN_NONE;
}

PyObject *wndMarkBox_SetImage(PyObject *poSelf, PyObject *poArgs)
{
    // 아무것도 하지 않음
    Py_RETURN_NONE;
}

PyObject *wndMarkBox_Load(PyObject *poSelf, PyObject *poArgs)
{
    // 아무것도 하지 않음
    Py_RETURN_NONE;
}

PyObject *wndMarkBox_SetIndex(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int nIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &nIndex))
    {
        return Py_BuildException();
    }

    ((UI::CMarkBox *)pWindow)->SetIndex(nIndex);
    Py_RETURN_NONE;
}

PyObject *wndMarkBox_SetScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    float fScale;
    if (!PyTuple_GetFloat(poArgs, 1, &fScale))
    {
        return Py_BuildException();
    }

    ((UI::CMarkBox *)pWindow)->SetScale(fScale);
    Py_RETURN_NONE;
}

PyObject *wndMarkBox_SetDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fr;
    if (!PyTuple_GetFloat(poArgs, 1, &fr))
    {
        return Py_BuildException();
    }
    float fg;
    if (!PyTuple_GetFloat(poArgs, 2, &fg))
    {
        return Py_BuildException();
    }
    float fb;
    if (!PyTuple_GetFloat(poArgs, 3, &fb))
    {
        return Py_BuildException();
    }
    float fa;
    if (!PyTuple_GetFloat(poArgs, 4, &fa))
    {
        return Py_BuildException();
    }

    ((UI::CMarkBox *)pWindow)->SetDiffuseColor(fr, fg, fb, fa);

    Py_RETURN_NONE;
}

PyObject *wndImageLoadImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

	if(szFileName.empty())
    Py_RETURN_NONE;
	
    if (!((UI::CImageBox *)pWindow)->LoadImage(szFileName.c_str()))
    {
        SPDLOG_WARN("Failed to load image (filename: {})", szFileName.c_str());
        Py_RETURN_NONE;
    }

    Py_RETURN_NONE;
}

PyObject *wndImageSetDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fr;
    if (!PyTuple_GetFloat(poArgs, 1, &fr))
    {
        return Py_BuildException();
    }
    float fg;
    if (!PyTuple_GetFloat(poArgs, 2, &fg))
    {
        return Py_BuildException();
    }
    float fb;
    if (!PyTuple_GetFloat(poArgs, 3, &fb))
    {
        return Py_BuildException();
    }
    float fa;
    if (!PyTuple_GetFloat(poArgs, 4, &fa))
    {
        return Py_BuildException();
    }

    ((UI::CImageBox *)pWindow)->SetDiffuseColor(fr, fg, fb, fa);

    Py_RETURN_NONE;
}

PyObject *wndImageGetWidth(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CImageBox *)pWindow)->GetWidth());
}

PyObject *wndButtonIsDisable(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CButton *)pWindow)->IsDisable());
}

PyObject *wndButtonIsEnabled(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CButton *)pWindow)->IsEnable());
}

PyObject *wndImageGetHeight(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CImageBox *)pWindow)->GetHeight());
}

PyObject *wndImageSetScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fx;
    if (!PyTuple_GetFloat(poArgs, 1, &fx))
    {
        return Py_BuildException();
    }
    float fy;
    if (!PyTuple_GetFloat(poArgs, 2, &fy))
    {
        return Py_BuildException();
    }

    ((UI::CExpandedImageBox *)pWindow)->SetScale(fx, fy);

    Py_RETURN_NONE;
}

PyObject *wndImageSetCoolTimeImageBox(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float coolTime;
    if (!PyTuple_GetFloat(poArgs, 1, &coolTime))
    {
        return Py_BuildException();
    }

    ((UI::CExpandedImageBox *)pWindow)->m_coolTime = coolTime;

    Py_RETURN_NONE;
}

PyObject *wndImageSetStartCoolTimeImageBox(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    float coolTime;
    if (!PyTuple_GetFloat(poArgs, 1, &coolTime))
    {
        return Py_BuildException();
    }

    ((UI::CExpandedImageBox *)pWindow)->m_startCoolTime = coolTime;
    Py_RETURN_NONE;
}

PyObject *wndImageSetInverse(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CExpandedImageBox *)pWindow)->LeftRightReverse();

    Py_RETURN_NONE;
}

PyObject *wndImageSetOrigin(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fx;
    if (!PyTuple_GetFloat(poArgs, 1, &fx))
    {
        return Py_BuildException();
    }
    float fy;
    if (!PyTuple_GetFloat(poArgs, 2, &fy))
    {
        return Py_BuildException();
    }

    ((UI::CExpandedImageBox *)pWindow)->SetOrigin(fx, fy);

    Py_RETURN_NONE;
}

PyObject *wndImageSetRotation(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fRotation;
    if (!PyTuple_GetFloat(poArgs, 1, &fRotation))
    {
        return Py_BuildException();
    }

    ((UI::CExpandedImageBox *)pWindow)->SetRotation(fRotation);

    Py_RETURN_NONE;
}

PyObject *wndImageSetRenderingRect(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fLeft;
    if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
    {
        return Py_BuildException();
    }
    float fTop;
    if (!PyTuple_GetFloat(poArgs, 2, &fTop))
    {
        return Py_BuildException();
    }
    float fRight;
    if (!PyTuple_GetFloat(poArgs, 3, &fRight))
    {
        return Py_BuildException();
    }
    float fBottom;
    if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
    {
        return Py_BuildException();
    }

    if (pWindow->IsType(UI::CExpandedImageBox::Type()))
    {
        ((UI::CExpandedImageBox *)pWindow)->SetRenderingRect(fLeft, fTop, fRight, fBottom);
    }
    else if (pWindow->IsType(UI::CAniImageBox::Type()))
    {
        ((UI::CAniImageBox *)pWindow)->SetRenderingRect(fLeft, fTop, fRight, fBottom);
    }

    Py_RETURN_NONE;
}

PyObject *wndImageSetRenderingRectWithScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fLeft;
    if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
    {
        return Py_BuildException();
    }
    float fTop;
    if (!PyTuple_GetFloat(poArgs, 2, &fTop))
    {
        return Py_BuildException();
    }
    float fRight;
    if (!PyTuple_GetFloat(poArgs, 3, &fRight))
    {
        return Py_BuildException();
    }
    float fBottom;
    if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
    {
        return Py_BuildException();
    }

    if (pWindow->IsType(UI::CExpandedImageBox::Type()))
    {
        ((UI::CExpandedImageBox *)pWindow)->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);
    }
    else if (pWindow->IsType(UI::CAniImageBox::Type()))
    {
        ((UI::CAniImageBox *)pWindow)->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);
    }

    Py_RETURN_NONE;
}

PyObject *wndImageSetRenderingMode(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    int iMode;
    if (!PyTuple_GetInteger(poArgs, 1, &iMode))
    {
        return Py_BuildException();
    }

    if (pWindow->IsType(UI::CExpandedImageBox::Type()))
    {
        ((UI::CExpandedImageBox *)pWindow)->SetRenderingMode(iMode);
    }

    Py_RETURN_NONE;
}

PyObject *wndImageSetDelay(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fDelay;
    if (!PyTuple_GetFloat(poArgs, 1, &fDelay))
    {
        return Py_BuildException();
    }

    ((UI::CAniImageBox *)pWindow)->SetDelay(fDelay);

    Py_RETURN_NONE;
}

PyObject *wndMgrSetAniImgDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    float r, g, b, a;
    if (!PyTuple_GetFloat(poArgs, 1, &r))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetFloat(poArgs, 2, &g))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetFloat(poArgs, 3, &b))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetFloat(poArgs, 4, &a))
    {
        return Py_BuildException();
    }

    ((UI::CAniImageBox *)pWindow)->SetDiffuseColor(r, g, b, a);
    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
    {
        return Py_BuildException();
    }

    int iColorType;
    if (!PyTuple_GetInteger(poArgs, 2, &iColorType))
    {
        return Py_BuildException();
    }

    ((UI::CSlotWindow *)pWindow)->SetDiffuseColor(iIndex, iColorType);
    Py_RETURN_NONE;
}

PyObject *wndImageSetAniImgScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    float fX;
    if (!PyTuple_GetFloat(poArgs, 1, &fX))
    {
        return Py_BuildException();
    }

    float fY;
    if (!PyTuple_GetFloat(poArgs, 2, &fY))
    {
        return Py_BuildException();
    }

    ((UI::CAniImageBox *)pWindow)->SetAniImgScale(fX, fY);
    Py_RETURN_NONE;
}

PyObject *wndImageAppendImage(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

    ((UI::CAniImageBox *)pWindow)->AppendImage(szFileName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndImageResetFrame(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CAniImageBox *)pWindow)->ResetFrame();

    Py_RETURN_NONE;
}

PyObject *wndAniImageStop(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
        return Py_BuildException();

    ((UI::CAniImageBox *)pWindow)->Stop();

    Py_RETURN_NONE;
}

PyObject *wndAniImageStart(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
        return Py_BuildException();

    ((UI::CAniImageBox *)pWindow)->Start();

    Py_RETURN_NONE;
}

PyObject *wndMoveImageMoveStart(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CMoveImageBox *)pWindow)->MoveStart();

    Py_RETURN_NONE;
}

PyObject *wndMoveImageMoveStop(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CMoveImageBox *)pWindow)->MoveStop();

    Py_RETURN_NONE;
}

PyObject *wndMoveImageSetMovePosition(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    float posX;
    if (!PyTuple_GetFloat(poArgs, 1, &posX))
    {
        return Py_BuildException();
    }

    float posY;
    if (!PyTuple_GetFloat(poArgs, 1, &posY))
    {
        return Py_BuildException();
    }

    ((UI::CMoveImageBox *)pWindow)->SetMovePosition(posX, posY);

    Py_RETURN_NONE;
}

PyObject *wndMoveImageSetMoveSpeed(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    float fSpeed;
    if (!PyTuple_GetFloat(poArgs, 1, &fSpeed))
    {
        return Py_BuildException();
    }

    ((UI::CMoveImageBox *)pWindow)->SetMoveSpeed(fSpeed);

    Py_RETURN_NONE;
}


PyObject *wndImageSetButtonDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float fr;
    if (!PyTuple_GetFloat(poArgs, 1, &fr))
    {
        return Py_BuildException();
    }
    float fg;
    if (!PyTuple_GetFloat(poArgs, 2, &fg))
    {
        return Py_BuildException();
    }
    float fb;
    if (!PyTuple_GetFloat(poArgs, 3, &fb))
    {
        return Py_BuildException();
    }
    float fa;
    if (!PyTuple_GetFloat(poArgs, 4, &fa))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->SetDiffuseColor(fr, fg, fb, fa);

    Py_RETURN_NONE;
}

PyObject *wndButtonSetButtonScale(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    float xScale;
    if (!PyTuple_GetFloat(poArgs, 1, &xScale))
    {
        return Py_BuildException();
    }
    float yScale;
    if (!PyTuple_GetFloat(poArgs, 2, &yScale))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->SetButtonScale(xScale, yScale);

    Py_RETURN_NONE;
}

PyObject *wndButtonGetButtonImageWidth(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CButton *)pWindow)->GetButtonImageWidth());
}

PyObject *wndButtonGetButtonImageHeight(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CButton *)pWindow)->GetButtonImageHeight());
}

PyObject *wndButtonSetUpVisual(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1,
                           szFileName))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->SetUpVisual(szFileName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndButtonSetOverVisual(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->SetOverVisual(szFileName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndButtonSetDownVisual(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->SetDownVisual(szFileName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndButtonSetDisableVisual(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->SetDisableVisual(szFileName.c_str());

    Py_RETURN_NONE;
}

PyObject *wndButtonGetUpVisualFileName(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("s", ((UI::CButton *)pWindow)->GetUpVisualFileName());
}

PyObject *wndButtonGetOverVisualFileName(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("s", ((UI::CButton *)pWindow)->GetOverVisualFileName());
}

PyObject *wndButtonGetDownVisualFileName(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("s", ((UI::CButton *)pWindow)->GetDownVisualFileName());
}

PyObject *wndButtonFlash(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->Flash();

    Py_RETURN_NONE;
}

PyObject *wndButtonEnableFlash(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->EnableFlash();

    Py_RETURN_NONE;
}

PyObject *wndButtonDisableFlash(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->DisableFlash();

    Py_RETURN_NONE;
}

PyObject *wndButtonEnable(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->Enable();

    Py_RETURN_NONE;
}

PyObject *wndButtonDisable(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->Disable();

    Py_RETURN_NONE;
}

PyObject *wndButtonDown(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->Down();

    Py_RETURN_NONE;
}

PyObject *wndButtonLeftRightReverse(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->LeftRightReverse();

    Py_RETURN_NONE;
}

PyObject *wndButtonOver(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->Over();

    Py_RETURN_NONE;
}

PyObject *wndButtonSetUp(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::CButton *)pWindow)->SetUp();

    Py_RETURN_NONE;
}

PyObject *wndButtonSetRestrictMovementArea(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }
    int ix;
    if (!PyTuple_GetInteger(poArgs, 1, &ix))
    {
        return Py_BuildException();
    }
    int iy;
    if (!PyTuple_GetInteger(poArgs, 2, &iy))
    {
        return Py_BuildException();
    }
    int iwidth;
    if (!PyTuple_GetInteger(poArgs, 3, &iwidth))
    {
        return Py_BuildException();
    }
    int iheight;
    if (!PyTuple_GetInteger(poArgs, 4, &iheight))
    {
        return Py_BuildException();
    }

    if (pWindow->IsType(UI::CDragButton::Type()))
        ((UI::CDragButton *)pWindow)->SetRestrictMovementArea(ix, iy, iwidth, iheight);
    else if (pWindow->IsType(UI::CDragBar::Type()))
        ((UI::CDragBar *)pWindow)->SetRestrictMovementArea(ix, iy, iwidth, iheight);

    Py_RETURN_NONE;
}

PyObject *wndButtonIsDown(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", ((UI::CButton *)pWindow)->IsPressed());
}

PyObject *wndMgrSetOutlineFlag(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
    {
        return Py_BuildException();
    }

    Py_RETURN_NONE;
}

PyObject *wndMgrColorPickerCreate(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWindow;
    if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
    {
        return Py_BuildException();
    }

    ((UI::ColorPicker *)pWindow)->Create();
    Py_RETURN_NONE;
}

PyObject *wndMgrShowOverInWindowName(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
    {
        return Py_BuildException();
    }

    Py_RETURN_NONE;
}

PyObject *wndMgrSetSlotDiffuse(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int iSlotColor;
    if (!PyTuple_GetInteger(poArgs, 2, &iSlotColor))
    {
        return Py_BuildException();
    }

    if (!pWin->IsType(UI::CSlotWindow::Type()))
    {
        return Py_BuildException();
    }

    UI::CSlotWindow *pSlotWin = (UI::CSlotWindow *)pWin;
    pSlotWin->SetDiffuseColor(iSlotIndex, iSlotColor);

    Py_RETURN_NONE;
}

PyObject *wndMgrGetChildCount(PyObject *poSelf, PyObject *poArgs)
{
    UI::CWindow *pWin;
    if (!PyTuple_GetWindow(poArgs, 0, &pWin))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pWin->GetChildrenCount());
}


PyObject * wndSetMoveSpeed(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fSpeed;
	if (!PyTuple_GetFloat(poArgs, 1, &fSpeed))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->SetMoveSpeed(fSpeed);
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->SetMoveSpeed(fSpeed);

	return Py_BuildNone();
}

PyObject * wndSetMovePosition(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fDstX(0.0f), fDstY(0.0f);
	if (!PyTuple_GetFloat(poArgs, 1, &fDstX))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 2, &fDstY))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->SetMovePosition(fDstX, fDstY);
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->SetMovePosition(fDstX, fDstY);

	return Py_BuildNone();
}

PyObject * wndMoveStart(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->MoveStart();
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->MoveStart();

	return Py_BuildNone();
}

PyObject * wndMoveStop(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->MoveStop();
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->MoveStop();

	return Py_BuildNone();
}

PyObject * wndGetMove(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		return Py_BuildValue("i", ((UI::CMoveImageBox*)pWindow)->GetMove());
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		return Py_BuildValue("i", ((UI::CMoveTextLine*)pWindow)->GetMove());
	else
		return Py_BuildValue("i", 0);
}

PyObject * wndSetMaxScale(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fMaxScale = 1.0f;
	if (!PyTuple_GetFloat(poArgs, 1, &fMaxScale))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveScaleImageBox*)pWindow)->SetMaxScale(fMaxScale);

	return Py_BuildNone();
}

PyObject * wndSetMaxScaleRate(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fMaxScaleRate = 1.0f;
	if (!PyTuple_GetFloat(poArgs, 1, &fMaxScaleRate))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveScaleImageBox*)pWindow)->SetMaxScaleRate(fMaxScaleRate);

	return Py_BuildNone();
}

PyObject * wndSetScalePivotCenter(PyObject * poSelf, PyObject * poArgs)
{
	UI::CWindow * pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	bool bScalePivotCenter = false;
	if (!PyTuple_GetBoolean(poArgs, 1, &bScalePivotCenter))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveScaleImageBox*)pWindow)->SetScalePivotCenter(bScalePivotCenter);

	return Py_BuildNone();
}

extern "C" void initwndMgr()
{
    static PyMethodDef s_methods[] =
    {
        // WindowManager
        {"SetMouseHandler", wndMgrSetMouseHandler, METH_VARARGS},
        {"SetScreenSize", wndMgrSetScreenSize, METH_VARARGS},
        {"GetScreenWidth", wndMgrGetScreenWidth, METH_VARARGS},
        {"GetScreenHeight", wndMgrGetScreenHeight, METH_VARARGS},
        {"AttachIcon", wndMgrAttachIcon, METH_VARARGS},
        {"DeattachIcon", wndMgrDeattachIcon, METH_VARARGS},
        {"SetAttachingFlag", wndMgrSetAttachingFlag, METH_VARARGS},
        {"SetAttachingRealSlotNumber", wndMgrSetAttachingRealSlotNumber, METH_VARARGS},
        {"GetAspect", wndMgrGetAspect, METH_VARARGS},

        // Window
#define METIN2_REGISTER_FN(name, cls) { #name, wndMgrRegister<cls>, METH_VARARGS },
        METIN2_REGISTER_FN(Register, UI::CWindow)
        METIN2_REGISTER_FN(RegisterSlotWindow, UI::CSlotWindow)
        METIN2_REGISTER_FN(RegisterGridSlotWindow, UI::CGridSlotWindow)
        METIN2_REGISTER_FN(RegisterTextLine, UI::CTextLine)
        METIN2_REGISTER_FN(RegisterEditLine, UI::EditControl)
        METIN2_REGISTER_FN(RegisterMarkBox, UI::CMarkBox)
        METIN2_REGISTER_FN(RegisterImageBox, UI::CImageBox)
        METIN2_REGISTER_FN(RegisterExpandedImageBox, UI::CExpandedImageBox)
        METIN2_REGISTER_FN(RegisterAniImageBox, UI::CAniImageBox)
        METIN2_REGISTER_FN(RegisterButton, UI::CButton)
        METIN2_REGISTER_FN(RegisterRadioButton, UI::CRadioButton)
        METIN2_REGISTER_FN(RegisterToggleButton, UI::CToggleButton)
        METIN2_REGISTER_FN(RegisterDragButton, UI::CDragButton)
        METIN2_REGISTER_FN(RegisterDragBar, UI::CDragBar)
        METIN2_REGISTER_FN(RegisterBox, UI::CBox)
        METIN2_REGISTER_FN(RegisterBar, UI::CBar)
        METIN2_REGISTER_FN(RegisterLine, UI::CLine)
        METIN2_REGISTER_FN(RegisterBar3D, UI::CBar3D)
        METIN2_REGISTER_FN(RegisterNumberLine, UI::CNumberLine)
        METIN2_REGISTER_FN(RegisterRenderTarget, UI::CRenderTarget)
        METIN2_REGISTER_FN(RegisterWebView, UI::CWebView)
        METIN2_REGISTER_FN(RegisterColorPicker, UI::ColorPicker)
        METIN2_REGISTER_FN(RegisterVideoFrame, UI::CVideoFrame)

        METIN2_REGISTER_FN(RegisterMoveTextLine, UI::CMoveTextLine)
        METIN2_REGISTER_FN(RegisterMoveImageBox, UI::CMoveImageBox)
        METIN2_REGISTER_FN(RegisterMoveScaleImageBox, UI::CMoveScaleImageBox)



#undef METIN2_REGISTER_FN



        {"Destroy", wndMgrDestroy, METH_VARARGS},
        {"AddFlag", wndMgrAddFlag, METH_VARARGS},
        {"RemoveFlag", wndMgrRemoveFlag, METH_VARARGS},
        {"IsRTL", wndMgrIsRTL, METH_VARARGS},
        {"SetAlpha", wndMgrSetAlpha, METH_VARARGS},
        {"GetAlpha", wndMgrGetAlpha, METH_VARARGS},
        {"SetAllAlpha", wndMgrSetAllAlpha, METH_VARARGS},

        // Base Window
        {"SetName", wndMgrSetName, METH_VARARGS},
        {"GetName", wndMgrGetName, METH_VARARGS},

        {"SetTop", wndMgrSetTop, METH_VARARGS},
        {"Show", wndMgrShow, METH_VARARGS},
        {"Hide", wndMgrHide, METH_VARARGS},
        {"IsShow", wndMgrIsShow, METH_VARARGS},
        {"IsRendering", wndMgrIsRendering, METH_VARARGS},

        {"IsAttachedTo", wndMgrIsAttachedTo, METH_VARARGS},
        {"IsAttaching", wndMgrIsAttaching, METH_VARARGS},
        {"SetAttachedTo", wndMgrSetAttachedTo, METH_VARARGS},

        {"SetParent", wndMgrSetParent, METH_VARARGS},
        {"GetParent", wndMgrGetParent, METH_VARARGS},

        {"SetPickAlways", wndMgrSetPickAlways, METH_VARARGS},

        {"IsFocus", wndMgrIsFocus, METH_VARARGS},
        {"SetFocus", wndMgrSetFocus, METH_VARARGS},
        {"KillFocus", wndMgrKillFocus, METH_VARARGS},
        {"GetFocus", wndMgrGetFocus, METH_VARARGS},

        {"Lock", wndMgrLock, METH_VARARGS},
        {"Unlock", wndMgrUnlock, METH_VARARGS},

        {"CaptureMouse", wndMgrCaptureMouse, METH_VARARGS},
        {"ReleaseMouse", wndMgrReleaseMouse, METH_VARARGS},
        {"GetCapture", wndMgrGetCapture, METH_VARARGS},

        {"SetWindowSize", wndMgrSetWndSize, METH_VARARGS},
        {"SetWindowPosition", wndMgrSetWndPosition, METH_VARARGS},
        {"GetWindowWidth", wndMgrGetWndWidth, METH_VARARGS},
        {"GetWindowHeight", wndMgrGetWndHeight, METH_VARARGS},
        {"GetWindowLocalPosition", wndMgrGetWndLocalPosition, METH_VARARGS},
        {"GetWindowGlobalPosition", wndMgrGetWndGlobalPosition, METH_VARARGS},
        {"GetWindowRect", wndMgrGetWindowRect, METH_VARARGS},
        {"GetWindowBaseRect", wndMgrGetWindowBaseRect, METH_VARARGS},

        {"SetWindowHorizontalAlign", wndMgrSetWindowHorizontalAlign, METH_VARARGS},
        {"SetWindowVerticalAlign", wndMgrSetWindowVerticalAlign, METH_VARARGS},

        {"GetChildCount", wndMgrGetChildCount, METH_VARARGS},

        {"IsPickedWindow", wndMgrIsPickedWindow, METH_VARARGS},
        {"GetPickedWindow", wndMgrGetPickedWindow, METH_VARARGS},

        {"IsIn", wndMgrIsIn, METH_VARARGS},
        {"GetMouseLocalPosition", wndMgrGetMouseLocalPosition, METH_VARARGS},
        {"GetMousePosition", wndMgrGetMousePosition, METH_VARARGS},
        {"IsDragging", wndMgrIsDragging, METH_VARARGS},

        {"EnableClipping", wndMgrEnableClipping, METH_VARARGS},
        {"DisableClipping", wndMgrDisableClipping, METH_VARARGS},

        {"UpdateRect", wndMgrUpdateRect, METH_VARARGS},
        {"OnKeyDown", wndMgrOnKeyDown, METH_VARARGS},

        // Slot Window
        {"AppendSlot", wndMgrAppendSlot, METH_VARARGS},
        {"ArrangeSlot", wndMgrArrangeSlot, METH_VARARGS},
        {"ClearSlot", wndMgrClearSlot, METH_VARARGS},
        {"ClearAllSlot", wndMgrClearAllSlot, METH_VARARGS},
        {"HasSlot", wndMgrHasSlot, METH_VARARGS},
        {"SetSlot", wndMgrSetSlot, METH_VARARGS},
        {"SetSlotCount", wndMgrSetSlotCount, METH_VARARGS},
        {"SetSlotCountNew", wndMgrSetSlotCountNew, METH_VARARGS},
        {"SetSlotSlotNumber", wndMgrSetSlotSlotNumber, METH_VARARGS},
        {"SetSlotSlotText", wndMgrSetSlotSlotText, METH_VARARGS},
        {"SetSlotRealNumber", wndMgrSetRealSlotNumber, METH_VARARGS},
        {"SetSlotCoolTime", wndMgrSetSlotCoolTime, METH_VARARGS},
        {"SetSlotCoolTimeColor", wndMgrSetSlotCoolTimeColor, METH_VARARGS},
        {"StoreSlotCoolTime", wndMgrStoreSlotCoolTime, METH_VARARGS},
        {"RestoreSlotCoolTime", wndMgrRestoreSlotCoolTime, METH_VARARGS},
        {"SetSlotCoolTimeInverse", wndMgrSetSlotCoolTimeInverse, METH_VARARGS},
        {"SetCantMouseEventSlot", wndMgrSetCantMouseEventSlot, METH_VARARGS},
        {"SetCanMouseEventSlot", wndMgrSetCanMouseEventSlot, METH_VARARGS},
        {"SetUnusableSlotOnTopWnd", wndMgrSetUnusableSlotOnTopWnd, METH_VARARGS},
        {"SetUsableSlotOnTopWnd", wndMgrSetUsableSlotOnTopWnd, METH_VARARGS},
        {"SetSlotHighlightedGreeen", wndMgrSetSlotHighlightedGreeen, METH_VARARGS},
        {"DisableSlotHighlightedGreen", wndMgrDisableSlotHighlightedGreen, METH_VARARGS},
        {"SetSlotDiffuseColor", wndMgrSetSlotDiffuseColor, METH_VARARGS},
        {"GetSlotGlobalPosition", wndMgrGetSlotGlobalPosition, METH_VARARGS},
        {"GetSlotLocalPosition", wndMgrGetSlotLocalPosition, METH_VARARGS},
        {"SetSlotTextPositon", wndMgrSetSlotTextPositon, METH_VARARGS},

        //
        {"SetSlotExpire", wndMgrSetSlotExpire, METH_VARARGS},
        {"SetSlotExpireTime", wndMgrSetSlotExpire, METH_VARARGS},
        {"SetToggleSlot", wndMgrSetToggleSlot, METH_VARARGS},

        {"ActivateSlot", wndMgrActivateSlot, METH_VARARGS},
        {"DeactivateSlot", wndMgrDeactivateSlot, METH_VARARGS},
        {"EnableSlot", wndMgrEnableSlot, METH_VARARGS},
        {"DisableSlot", wndMgrDisableSlot, METH_VARARGS},
        {"ShowSlotBaseImage", wndMgrShowSlotBaseImage, METH_VARARGS},
        {"HideSlotBaseImage", wndMgrHideSlotBaseImage, METH_VARARGS},

        {"SetWindowType", wndMgrSetWindowType, METH_VARARGS},

        {"SetSlotType", wndMgrSetSlotType, METH_VARARGS},
        {"SetSlotStyle", wndMgrSetSlotStyle, METH_VARARGS},
        {"SetSlotBaseImage", wndMgrSetSlotBaseImage, METH_VARARGS},

        {"SetSlotScale", wndMgrSetSlotScale, METH_VARARGS},
        {"SetBaseImageScale", wndMgrSetSlotBaseImageScale, METH_VARARGS},
        {"SetCoverButtonScale", wndMgrSetCoverButtonScale, METH_VARARGS},

        {"SetCoverButton", wndMgrSetCoverButton, METH_VARARGS},
        {"EnableCoverButton", wndMgrEnableCoverButton, METH_VARARGS},
        {"DisableCoverButton", wndMgrDisableCoverButton, METH_VARARGS},
        {"IsDisableCoverButton", wndMgrIsDisableCoverButton, METH_VARARGS},
        {"SetAlwaysRenderCoverButton", wndMgrSetAlwaysRenderCoverButton, METH_VARARGS},

        {"SetSlotButton", wndMgrSetSlotButton, METH_VARARGS},

        {"AppendSlotButton", wndMgrAppendSlotButton, METH_VARARGS},
        {"AppendRequirementSignImage", wndMgrAppendRequirementSignImage, METH_VARARGS},
        {"ShowSlotButton", wndMgrShowSlotButton, METH_VARARGS},
        {"SetSlotButtonPosition", wndMgrSetSlotButtonPosition, METH_VARARGS},

        {"HideAllSlotButton", wndMgrHideAllSlotButton, METH_VARARGS},
        {"ShowRequirementSign", wndMgrShowRequirementSign, METH_VARARGS},
        {"HideRequirementSign", wndMgrHideRequirementSign, METH_VARARGS},
        {"RefreshSlot", wndMgrRefreshSlot, METH_VARARGS},
        {"SetUseMode", wndMgrSetUseMode, METH_VARARGS},
        {"SetUsableItem", wndMgrSetUsableItem, METH_VARARGS},

        {"SelectSlot", wndMgrSelectSlot, METH_VARARGS},
        {"ClearSelected", wndMgrClearSelected, METH_VARARGS},
        {"GetSelectedSlotCount", wndMgrGetSelectedSlotCount, METH_VARARGS},
        {"GetSelectedSlotNumber", wndMgrGetSelectedSlotNumber, METH_VARARGS},
        {"IsSelectedSlot", wndMgrIsSelectedSlot, METH_VARARGS},
        {"GetSlotCount", wndMgrGetSlotCount, METH_VARARGS},
        {"LockSlot", wndMgrLockSlot, METH_VARARGS},
        {"UnlockSlot", wndMgrUnlockSlot, METH_VARARGS},

        // Bar
        {"SetColor", wndBarSetColor, METH_VARARGS},

        // TextLine
        {"SetSecret", wndTextSetSecret, METH_VARARGS},
        {"SetOutline", wndTextSetOutline, METH_VARARGS},
        {"SetMultiLine", wndTextSetMultiLine, METH_VARARGS},
        {"SetText", wndTextSetText, METH_VARARGS},
        {"SetFontName", wndTextSetFontName, METH_VARARGS},
        {"SetFontColor", wndTextSetFontColor, METH_VARARGS},
        {"SetLimitWidth", wndTextSetLimitWidth, METH_VARARGS},
        {"GetText", wndTextGetText, METH_VARARGS},
        {"GetHyperlinkAtPos", wndTextGetHyperlinkAtPos, METH_VARARGS},
        {"SetClipRect", wndTextSetTextClip, METH_VARARGS},
        {"GetTextLineCount", wndTextGetLineCount, METH_VARARGS},

        // EditLine
        {"SetMax", wndEditSetMax, METH_VARARGS},
        {"SetMaxVisible", wndEditSetMaxVisible, METH_VARARGS},
        {"Insert", wndEditInsert, METH_VARARGS},
        {"Erase", wndEditErase, METH_VARARGS},
        {"GetCursorPosition", wndEditGetCursorPosition, METH_VARARGS},
        {"MoveToEnd", wndEditMoveToEnd, METH_VARARGS},
        {"SetPlaceholderText", wndEditSetPlaceholderText, METH_VARARGS},
        {"SetPlaceholderColor", wndEditSetPlaceholderColor, METH_VARARGS},
        {"DisableEditLine", wndEditDisable, METH_VARARGS},
        {"EnableEditLine", wndEditEnable, METH_VARARGS},

        // NumberLine
        {"SetNumber", wndNumberSetNumber, METH_VARARGS},
        {"SetNumberHorizontalAlignCenter", wndNumberSetNumberHorizontalAlignCenter, METH_VARARGS},
        {"SetNumberHorizontalAlignRight", wndNumberSetNumberHorizontalAlignRight, METH_VARARGS},
        {"SetPath", wndNumberSetPath, METH_VARARGS},

        // ImageBox
        {"MarkBox_SetImage", wndMarkBox_SetImage, METH_VARARGS},
        {"MarkBox_SetImageFilename", wndMarkBox_SetImageFilename, METH_VARARGS},
        {"MarkBox_Load", wndMarkBox_Load, METH_VARARGS},
        {"MarkBox_SetIndex", wndMarkBox_SetIndex, METH_VARARGS},
        {"MarkBox_SetScale", wndMarkBox_SetScale, METH_VARARGS},
        {"MarkBox_SetDiffuseColor", wndMarkBox_SetDiffuseColor, METH_VARARGS},

        // ImageBox
        {"LoadImage", wndImageLoadImage, METH_VARARGS},
        {"SetDiffuseColor", wndImageSetDiffuseColor, METH_VARARGS},
        {"SetButtonDiffuseColor", wndImageSetButtonDiffuseColor, METH_VARARGS},
        {"IsDisable", wndButtonIsDisable, METH_VARARGS},
        {"IsEnabled", wndButtonIsEnabled, METH_VARARGS},
        //
        {"GetWidth", wndImageGetWidth, METH_VARARGS},
        {"GetHeight", wndImageGetHeight, METH_VARARGS},

        // ExpandedImageBox
        {"LeftRightReverseImageBox", wndImageSetInverse, METH_VARARGS},
        {"SetCoolTimeImageBox", wndImageSetCoolTimeImageBox, METH_VARARGS},
        {"SetStartCoolTimeImageBox", wndImageSetStartCoolTimeImageBox, METH_VARARGS},
        {"SetScale", wndImageSetScale, METH_VARARGS},
        {"SetOrigin", wndImageSetOrigin, METH_VARARGS},
        {"SetRotation", wndImageSetRotation, METH_VARARGS},
        {"SetRenderingRect", wndImageSetRenderingRect, METH_VARARGS},
        {"SetRenderingMode", wndImageSetRenderingMode, METH_VARARGS},

        // AniImageBox
        {"SetDelay", wndImageSetDelay, METH_VARARGS},
        {"AppendImage", wndImageAppendImage, METH_VARARGS},
        {"StartAnimation", wndAniImageStart, METH_VARARGS},
        {"StopAnimation", wndAniImageStop, METH_VARARGS},
        {"SetAniImgScale", wndImageSetAniImgScale, METH_VARARGS},
        {"SetAniImgDiffuseColor", wndMgrSetAniImgDiffuseColor, METH_VARARGS},
        {"SetRenderingRectWithScale", wndImageSetRenderingRectWithScale, METH_VARARGS},
        {"ResetFrame", wndImageResetFrame, METH_VARARGS},

        // MoveImageBox
		{ "SetMoveSpeed",				wndSetMoveSpeed,					METH_VARARGS },
		{ "SetMovePosition",			wndSetMovePosition,					METH_VARARGS },
		{ "MoveStart",					wndMoveStart,						METH_VARARGS },
		{ "MoveStop",					wndMoveStop,						METH_VARARGS },
		{ "GetMove",					wndGetMove,							METH_VARARGS },

		{ "SetMaxScale",				wndSetMaxScale,						METH_VARARGS },
		{ "SetMaxScaleRate",			wndSetMaxScaleRate,					METH_VARARGS },
		{ "SetScalePivotCenter",		wndSetScalePivotCenter,				METH_VARARGS },


        // Button
        {"SetUpVisual", wndButtonSetUpVisual, METH_VARARGS},
        {"SetOverVisual", wndButtonSetOverVisual, METH_VARARGS},
        {"SetDownVisual", wndButtonSetDownVisual, METH_VARARGS},
        {"SetDisableVisual", wndButtonSetDisableVisual, METH_VARARGS},
        {"GetUpVisualFileName", wndButtonGetUpVisualFileName, METH_VARARGS},
        {"GetOverVisualFileName", wndButtonGetOverVisualFileName, METH_VARARGS},
        {"GetDownVisualFileName", wndButtonGetDownVisualFileName, METH_VARARGS},
        {"Flash", wndButtonFlash, METH_VARARGS},
        {"EnableFlash", wndButtonEnableFlash, METH_VARARGS},
        {"DisableFlash", wndButtonDisableFlash, METH_VARARGS},
        {"Enable", wndButtonEnable, METH_VARARGS},
        {"Disable", wndButtonDisable, METH_VARARGS},
        {"Over", wndButtonOver, METH_VARARGS},
        {"LeftRightReverse", wndButtonLeftRightReverse, METH_VARARGS},
        {"SetButtonScale", wndButtonSetButtonScale, METH_VARARGS},
        {"GetButtonImageWidth", wndButtonGetButtonImageWidth, METH_VARARGS},
        {"GetButtonImageHeight", wndButtonGetButtonImageHeight, METH_VARARGS},

        {"Down", wndButtonDown, METH_VARARGS},
        {"SetUp", wndButtonSetUp, METH_VARARGS},
        {"IsDown", wndButtonIsDown, METH_VARARGS},

        // Background
        {"SetSlotBackground", wndMgrSetSlotBackground, METH_VARARGS},

        // DragButton
        {"SetRestrictMovementArea", wndButtonSetRestrictMovementArea, METH_VARARGS},
        {"SetSlotCoverImage", wndMgrSetSlotCoverImage, METH_VARARGS},
        {"EnableSlotCoverImage", wndMgrEnableSlotCoverImage, METH_VARARGS},
        {"SetRenderTarget", wndMgrSetRenderTarget, METH_VARARGS},
        {"SetWikiRenderTarget", wndMgrSetWikiRenderTarget, METH_VARARGS},

        {"SetAlwaysToolTip", wndMgrButtonSetAlwaysToolTip, METH_VARARGS},

        {"SetItemDiffuseColor", wndMgrSetItemDiffuseColor, METH_VARARGS},
        {"LoadUrl", wndMgrWebViewLoadUrl, METH_VARARGS},
        {"WebViewOnCreateInstance", wndMgrWebViewOnCreateInstance, METH_VARARGS},

        // For Debug
        {"SetOutlineFlag", wndMgrSetOutlineFlag, METH_VARARGS},
        {"ShowOverInWindowName", wndMgrShowOverInWindowName, METH_VARARGS},

        // ColorPicker
        {"ColorPickerCreate", wndMgrColorPickerCreate, METH_VARARGS},
        {"VideoOpen", wndMgrVideoFrameOpen, METH_VARARGS},

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
        {"DisableSlotCoverImage", wndMgrDisableSlotCoverImage, METH_VARARGS},
        {"ActivateChangeLookSlot", wndMgrActivateChangeLookSlot, METH_VARARGS},
        {"DeactivateChangeLookSlot", wndMgrDeactivateChangeLookSlot, METH_VARARGS},
#endif
        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("wndMgr", s_methods);

    // 하나의 딕셔너리에 너무 많은 Function이 포함 되는 것 같아 이런식으로 딕셔너리를 나누는 것을 고려 중 - [levites]
    //	PyObject * poMgrModule = Py_InitModule("wndMgr", s_methods);
    //	PyObject * poTextModule = Py_InitModule("wndText", s_methods);
    //	PyObject * poSlotModule = Py_InitModule("wndSlot", s_methods);

    PyModule_AddIntConstant(poModule, "SLOT_WND_DEFAULT", UI::SLOT_WND_DEFAULT);
    PyModule_AddIntConstant(poModule, "SLOT_WND_INVENTORY", UI::SLOT_WND_INVENTORY);

    PyModule_AddIntConstant(poModule, "SLOT_STYLE_NONE", UI::SLOT_STYLE_NONE);
    PyModule_AddIntConstant(poModule, "SLOT_STYLE_PICK_UP", UI::SLOT_STYLE_PICK_UP);
    PyModule_AddIntConstant(poModule, "SLOT_STYLE_SELECT", UI::SLOT_STYLE_SELECT);

    PyModule_AddIntConstant(poModule, "HORIZONTAL_ALIGN_LEFT", UI::CWindow::HORIZONTAL_ALIGN_LEFT);
    PyModule_AddIntConstant(poModule, "HORIZONTAL_ALIGN_CENTER", UI::CWindow::HORIZONTAL_ALIGN_CENTER);
    PyModule_AddIntConstant(poModule, "HORIZONTAL_ALIGN_RIGHT", UI::CWindow::HORIZONTAL_ALIGN_RIGHT);
    PyModule_AddIntConstant(poModule, "VERTICAL_ALIGN_TOP", UI::CWindow::VERTICAL_ALIGN_TOP);
    PyModule_AddIntConstant(poModule, "VERTICAL_ALIGN_CENTER", UI::CWindow::VERTICAL_ALIGN_CENTER);
    PyModule_AddIntConstant(poModule, "VERTICAL_ALIGN_BOTTOM", UI::CWindow::VERTICAL_ALIGN_BOTTOM);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_ORANGE", UI::SLOT_COLOR_TYPE_ORANGE);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_WHITE", UI::SLOT_COLOR_TYPE_WHITE);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_RED", UI::SLOT_COLOR_TYPE_RED);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_GREEN", UI::SLOT_COLOR_TYPE_GREEN);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_YELLOW", UI::SLOT_COLOR_TYPE_YELLOW);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_SKY", UI::SLOT_COLOR_TYPE_SKY);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_PINK", UI::SLOT_COLOR_TYPE_PINK);
    PyModule_AddIntConstant(poModule, "COLOR_TYPE_BLUE", UI::SLOT_COLOR_TYPE_BLUE);
    PyModule_AddIntConstant(poModule, "RENDERING_MODE_MODULATE",
                            CGraphicExpandedImageInstance::RENDERING_MODE_MODULATE);
    PyModule_AddIntConstant(poModule, "HILIGHTSLOT_ACCE", UI::HILIGHTSLOT_ACCE);
    PyModule_AddIntConstant(poModule, "HILIGHTSLOT_CHANGE_LOOK", UI::HILIGHTSLOT_CHANGE_LOOK);
}

void init_interface(py::module &m)
{
    py::module app = m.def_submodule("interface", "Provides access to the ui functions");

#define METIN2_REGISTER_FN(name, cls) .def( #name, &UI::CWindowManager::RegisterWindow<cls>)
    py::class_<UI::CWindow>(m, "Window")
        .def(py::init<>())
        .def("AddChild", &UI::CWindow::AddChild)
        .def("Clear", &UI::CWindow::Clear)
        .def("DestroyHandle", &UI::CWindow::DestroyHandle)
        .def("Update", &UI::CWindow::Update)
        .def("Render", &UI::CWindow::Render)
        .def("SetName", &UI::CWindow::SetName)
        .def("GetName", &UI::CWindow::GetName)
        .def("SetSize", &UI::CWindow::SetSize)
        .def("GetWidth", &UI::CWindow::GetWidth)
        .def("GetHeight", &UI::CWindow::GetHeight)
        .def("GetHandler", &UI::CWindow::GetHandler)
        .def("SetHorizontalAlign", &UI::CWindow::SetHorizontalAlign)
        .def("SetVerticalAlign", &UI::CWindow::SetVerticalAlign)
        .def("SetPosition", &UI::CWindow::SetPosition)
        .def("GetPositionX", &UI::CWindow::GetPositionY)
        .def("GetPositionY", &UI::CWindow::GetPositionY)
        .def("GetRect", &UI::CWindow::GetRect)
        .def("GetClipRect", &UI::CWindow::GetClipRect)
        .def("SetClipRect", &UI::CWindow::SetClipRect)
        .def("UpdateRect", &UI::CWindow::UpdateRect)
        .def("Show", &UI::CWindow::Show)
        .def("Hide", &UI::CWindow::Hide)
        .def("IsShow", &UI::CWindow::IsShow)
        .def("IsRendering", &UI::CWindow::IsRendering)
        .def("HasParent", &UI::CWindow::HasParent)
        .def("GetChildren", &UI::CWindow::GetChildren)
        .def("GetChildrenCount", &UI::CWindow::GetChildrenCount)
        .def("GetRoot", &UI::CWindow::GetRoot)
        .def("GetParent", &UI::CWindow::GetParent)
        .def("IsChild", &UI::CWindow::IsChild)
        .def("DeleteChild", &UI::CWindow::DeleteChild)
        .def("SetTop", &UI::CWindow::SetTop)
        .def("IsIn", py::overload_cast<int32_t, int32_t>(&UI::CWindow::IsIn))
        .def("IsIn", py::overload_cast<>(&UI::CWindow::IsIn))
        .def("PickWindow", &UI::CWindow::PickWindow)
        .def("EnableClipping", &UI::CWindow::EnableClipping)
        .def("DisableClipping", &UI::CWindow::DisableClipping)
        .def("AddFlag", &UI::CWindow::AddFlag)
        .def("RemoveFlag", &UI::CWindow::RemoveFlag)
        .def("IsFlag", &UI::CWindow::IsFlag);

    py::class_<UI::CBox, UI::CWindow>(m, "Box")
        .def(py::init<>())
        .def("SetColor", &UI::CBox::SetColor);

    py::class_<UI::CBar3D, UI::CWindow>(m, "Bar3D")
        .def(py::init<>())
        .def("SetColor", &UI::CBar3D::SetColor);

    py::class_<UI::CBar, UI::CWindow>(m, "Bar")
        .def(py::init<>())
        .def("SetColor", &UI::CBar::SetColor);

    py::class_<UI::CDragBar, UI::CBar, UI::CWindow>(m, "CDragBar")
        .def(py::init<>())
        .def("SetRestrictMovementArea", &UI::CDragBar::SetRestrictMovementArea)
        .def("IsPressed", &UI::CDragBar::IsPressed);

    py::class_<UI::CTextLine, UI::CWindow>(m, "TextLine")
        .def(py::init<>())
        .def("SetSecret", &UI::CTextLine::SetSecret)
        .def("SetOutline", &UI::CTextLine::SetOutline)
        .def("SetMultiLine", &UI::CTextLine::SetMultiLine)
        .def("SetFontName", &UI::CTextLine::SetFontName)
        .def("SetFontColor", &UI::CTextLine::SetFontColor)
        .def("SetFontGradient", &UI::CTextLine::SetFontGradient)
        .def("SetLimitWidth", &UI::CTextLine::SetLimitWidth)
        .def("SetText", &UI::CTextLine::SetText)
        .def("GetText", &UI::CTextLine::GetText)
        .def("GetHyperlinkAtPos", &UI::CTextLine::GetHyperlinkAtPos)
        .def("GetLineCount", &UI::CTextLine::GetLineCount);

    py::class_<UI::CNumberLine, UI::CWindow>(m, "NumberLine")
        .def(py::init<>())
        .def("SetPath", &UI::CNumberLine::SetPath)
        .def("SetHorizontalAlign", &UI::CNumberLine::SetHorizontalAlign)
        .def("SetNumber", &UI::CNumberLine::SetNumber);

    py::class_<UI::CVideoFrame, UI::CWindow>(m, "VideoFrame")
        .def(py::init<>())
        .def("Close", &UI::CVideoFrame::Close)
        .def("Open", &UI::CVideoFrame::Open);

    py::class_<UI::CWebView, UI::CWindow>(m, "WebView")
        .def(py::init<>())
        .def("LoadUrl", &UI::CWebView::LoadUrl)
        .def("OnCreateInstance", &UI::CWebView::OnCreateInstance);

    py::class_<UI::CImageBox, UI::CWindow>(m, "ImageBox")
        .def(py::init<>())
        .def("LoadImage", &UI::CImageBox::LoadImage)
        .def("SetDiffuseColor", &UI::CImageBox::SetDiffuseColor)
        .def("GetWidth", &UI::CImageBox::GetWidth)
        .def("GetHeight", &UI::CImageBox::GetHeight);

    py::class_<UI::CMoveImageBox, UI::CImageBox, UI::CWindow>(m, "MoveImageBox")
        .def(py::init<>())
        .def("MoveStop", &UI::CMoveImageBox::MoveStop)
        .def("MoveStart", &UI::CMoveImageBox::MoveStart)
        .def("SetMoveSpeed", &UI::CMoveImageBox::SetMoveSpeed)

        .def("SetMovePosition", &UI::CMoveImageBox::SetMovePosition);

    py::class_<UI::CExpandedImageBox, UI::CImageBox, UI::CWindow>(m, "ExpandedImageBox")
        .def(py::init<>())
        .def("SetScale", &UI::CExpandedImageBox::SetScale)
        .def("SetOrigin", &UI::CExpandedImageBox::SetOrigin)
        .def("SetRotation", &UI::CExpandedImageBox::SetRotation)
        .def("SetRenderingRect", &UI::CExpandedImageBox::SetRenderingRect)
        .def("SetRenderingRectWithScale", &UI::CExpandedImageBox::SetRenderingRectWithScale)
        .def("SetRenderingMode", &UI::CExpandedImageBox::SetRenderingMode)
        .def("LeftRightReverse", &UI::CExpandedImageBox::LeftRightReverse);

    py::class_<UI::CMarkBox, UI::CWindow>(m, "MarkBox")
        .def(py::init<>())
        .def("LoadImage", &UI::CMarkBox::LoadImage)
        .def("SetDiffuseColor", &UI::CMarkBox::SetDiffuseColor)
        .def("SetIndex", &UI::CMarkBox::SetIndex)
        .def("SetScale", &UI::CMarkBox::SetScale);

    py::class_<UI::CAniImageBox, UI::CWindow>(m, "CAniImageBox")
        .def(py::init<>())
        .def("SetDelay", &UI::CAniImageBox::SetDelay)
        .def("SetDiffuseColor", &UI::CAniImageBox::SetDiffuseColor)
        .def("AppendImage", &UI::CAniImageBox::AppendImage)
        .def("SetRenderingRect", &UI::CAniImageBox::SetRenderingRect)
        .def("SetRenderingRectWithScale", &UI::CAniImageBox::SetRenderingRectWithScale)
        .def("SetRenderingMode", &UI::CAniImageBox::SetRenderingMode)
        .def("Stop", &UI::CAniImageBox::Stop)
        .def("Start", &UI::CAniImageBox::Start)
        .def("SetAniImgScale", &UI::CAniImageBox::SetAniImgScale)
        .def("ResetFrame", &UI::CAniImageBox::ResetFrame);

    py::class_<UI::CRenderTarget, UI::CWindow>(m, "CRenderTarget")
        .def(py::init<>())
        .def("SetRenderTarget", &UI::CRenderTarget::SetRenderTarget)
        .def("SetWikiRenderTarget", &UI::CRenderTarget::SetWikiRenderTarget);
    py::class_<UI::ColorPicker, UI::CWindow>(m, "ColorPicker")
        .def(py::init<>())
        .def("Create", &UI::ColorPicker::Create);
    py::class_<UI::CButton, UI::CWindow>(m, "CButton")
        .def(py::init<>())
        .def("SetButtonScale", &UI::CButton::SetButtonScale)
        .def("GetButtonImageWidth", &UI::CButton::GetButtonImageWidth)
        .def("GetButtonImageHeight", &UI::CButton::GetButtonImageHeight)
        .def("SetUpVisual", &UI::CButton::SetUpVisual)
        .def("SetOverVisual", &UI::CButton::SetOverVisual)
        .def("SetDownVisual", &UI::CButton::SetDownVisual)
        .def("SetDisableVisual", &UI::CButton::SetDisableVisual)
        .def("GetUpVisualFileName", &UI::CButton::GetUpVisualFileName)
        .def("GetOverVisualFileName", &UI::CButton::GetOverVisualFileName)
        .def("GetDownVisualFileName", &UI::CButton::GetDownVisualFileName)
        .def("Flash", &UI::CButton::Flash)
        .def("EnableFlash", &UI::CButton::EnableFlash)
        .def("DisableFlash", &UI::CButton::DisableFlash)
        .def("Enable", &UI::CButton::Enable)
        .def("Disable", &UI::CButton::Disable)
        .def("SetUp", &UI::CButton::SetUp)
        .def("Up", &UI::CButton::Up)
        .def("Over", &UI::CButton::Over)
        .def("Down", &UI::CButton::Down)
        .def("LeftRightReverse", &UI::CButton::LeftRightReverse)
        .def("IsDisable", &UI::CButton::IsDisable)
        .def("IsPressed", &UI::CButton::IsPressed)
        .def("IsEnable", &UI::CButton::IsEnable)
        .def("SetDiffuseColor", &UI::CButton::SetDiffuseColor)
        .def("SetAlwaysTooltip", &UI::CButton::SetAlwaysTooltip);

    py::class_<UI::CRadioButton, UI::CButton, UI::CWindow>(m, "CRadioButton")
        .def(py::init<>());

    py::class_<UI::CToggleButton, UI::CButton, UI::CWindow>(m, "CToggleButton")
        .def(py::init<>());

    py::class_<UI::CDragButton, UI::CButton, UI::CWindow>(m, "CDragButton")
        .def(py::init<>())
        .def("SetRestrictMovementArea", &UI::CDragButton::SetRestrictMovementArea);

    // TODO: We want to completely get rid of the old module
    py::class_<UI::CWindowManager, std::unique_ptr<UI::CWindowManager, py::nodelete>>(app, "wndMgrInst")
        .def(py::init([]()
        {
            return std::unique_ptr<UI::CWindowManager, py::nodelete>(UI::CWindowManager::InstancePtr());
        }), pybind11::return_value_policy::reference_internal)
        .def("GetCapture", &UI::CWindowManager::GetCapture)
        .def("GetPointWindow", &UI::CWindowManager::GetPointWindow)
        .def("GetFocus", &UI::CWindowManager::GetFocus)
        .def("GetAppWindow", &UI::CWindowManager::GetAppWindow)
        .def("GetAspect", &UI::CWindowManager::GetAspect)
        .def("GetScreenHeight", &UI::CWindowManager::GetScreenHeight)
        .def("GetScreenWidth", &UI::CWindowManager::GetScreenWidth)
        .def("IsDragging", &UI::CWindowManager::IsDragging)
        .def("AttachIcon", &UI::CWindowManager::AttachIcon)
        .def("CaptureMouse", &UI::CWindowManager::CaptureMouse)
        .def("DeattachIcon", &UI::CWindowManager::DeattachIcon)
        .def("DestroyWindow", &UI::CWindowManager::DestroyWindow)
        .def("GetMousePosition", &UI::CWindowManager::GetMousePosition)
        .def("KillFocus", &UI::CWindowManager::KillFocus)
        .def("LockWindow", &UI::CWindowManager::LockWindow)
        .def("GetLockWindow", &UI::CWindowManager::GetLockWindow)
        //.def("ReleaseMouse", &UI::CWindowManager::ReleaseMouse)
        .def("SetAttachingFlag", &UI::CWindowManager::SetAttachingFlag)
        .def("SetAttachingRealSlotNumber", &UI::CWindowManager::SetAttachingRealSlotNumber)
        .def("SetMouseHandler", &UI::CWindowManager::SetMouseHandler)
        .def("SetParent", &UI::CWindowManager::SetParent)
        .def("SetTop", &UI::CWindowManager::SetTop)
        .def("UnlockWindow", &UI::CWindowManager::UnlockWindow)

        METIN2_REGISTER_FN(Register, UI::CWindow)
        METIN2_REGISTER_FN(RegisterSlotWindow, UI::CSlotWindow)
        METIN2_REGISTER_FN(RegisterGridSlotWindow, UI::CGridSlotWindow)
        METIN2_REGISTER_FN(RegisterTextLine, UI::CTextLine)
        METIN2_REGISTER_FN(RegisterEditLine, UI::EditControl)
        METIN2_REGISTER_FN(RegisterMarkBox, UI::CMarkBox)
        METIN2_REGISTER_FN(RegisterImageBox, UI::CImageBox)
        METIN2_REGISTER_FN(RegisterExpandedImageBox, UI::CExpandedImageBox)
        METIN2_REGISTER_FN(RegisterAniImageBox, UI::CAniImageBox)
        METIN2_REGISTER_FN(RegisterButton, UI::CButton)
        METIN2_REGISTER_FN(RegisterRadioButton, UI::CRadioButton)
        METIN2_REGISTER_FN(RegisterToggleButton, UI::CToggleButton)
        METIN2_REGISTER_FN(RegisterDragButton, UI::CDragButton)
        METIN2_REGISTER_FN(RegisterBox, UI::CBox)
        METIN2_REGISTER_FN(RegisterBar, UI::CBar)
        METIN2_REGISTER_FN(RegisterLine, UI::CLine)
        METIN2_REGISTER_FN(RegisterBar3D, UI::CBar3D)
        METIN2_REGISTER_FN(RegisterNumberLine, UI::CNumberLine)
        METIN2_REGISTER_FN(RegisterRenderTarget, UI::CRenderTarget)
        METIN2_REGISTER_FN(RegisterWebView, UI::CWebView)
        METIN2_REGISTER_FN(RegisterColorPicker, UI::ColorPicker);

#undef METIN2_REGISTER_FN
}
