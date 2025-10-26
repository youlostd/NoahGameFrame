#include "../eterlib/Camera.h"
#include "PythonApplication.h"
#include "StdAfx.h"
#include "base/Remotery.h"

void CPythonApplication::OnCameraUpdate()
{
    if (m_pyBackground.IsMapReady())
    {
        CCamera *pkCameraMgr = CCameraManager::Instance().GetCurrentCamera();
        if (pkCameraMgr)
            pkCameraMgr->Update();
    }
}

void CPythonApplication::OnUIUpdate()
{
    m_kWndMgr.Update();
}

void CPythonApplication::OnUIRender()
{
    rmt_ScopedCPUSample(OnUIRender, 0);

    m_kWndMgr.Render();
}

void CPythonApplication::OnSizeChange(int width, int height)
{
}

void CPythonApplication::OnResuming()
{
        m_timer2.ResetElapsedTime();
}

void CPythonApplication::OnSuspending()
{
}

void CPythonApplication::OnWindowMoved()
{
}

void CPythonApplication::OnActivated()
{
}

void CPythonApplication::OnDeactivated()
{
}

void CPythonApplication::OnMouseMiddleButtonDown(int x, int y)
{
    CCameraManager &rkCmrMgr = CCameraManager::Instance();
    CCamera *pkCmrCur = rkCmrMgr.GetCurrentCamera();
    if (pkCmrCur)
        pkCmrCur->BeginDrag(x, y);

    if (!m_pyBackground.IsMapReady())
        return;

    SetCursorNum(CAMERA_ROTATE);
    if (CURSOR_MODE_HARDWARE == GetCursorMode())
        SetCursorVisible(FALSE, true);
}

void CPythonApplication::OnMouseMiddleButtonUp(int x, int y)
{
    CCameraManager &rkCmrMgr = CCameraManager::Instance();
    CCamera *pkCmrCur = rkCmrMgr.GetCurrentCamera();
    if (pkCmrCur)
        pkCmrCur->EndDrag();

    if (!m_pyBackground.IsMapReady())
        return;

    SetCursorNum(NORMAL);
    if (CURSOR_MODE_HARDWARE == GetCursorMode())
        SetCursorVisible(TRUE);
}

void CPythonApplication::OnMouseWheel(int nLen)
{
    if (!(UI::CWindowManager::Instance().RunMouseWheelEvent(nLen)))
    {
        CCameraManager &rkCmrMgr = CCameraManager::Instance();
        CCamera *pkCmrCur = rkCmrMgr.GetCurrentCamera();
        if (pkCmrCur)
            pkCmrCur->Wheel(nLen);
    }
}

void CPythonApplication::OnMouseMove(int x, int y)
{
    CCameraManager &rkCmrMgr = CCameraManager::Instance();
    CCamera *pkCmrCur = rkCmrMgr.GetCurrentCamera();

    POINT Point;
    if (pkCmrCur)
    {
        if (m_isActivateWnd && CPythonBackground::Instance().IsMapReady() && pkCmrCur->Drag(x, y, &Point))
        {
            x = Point.x;
            y = Point.y;
            ClientToScreen(m_hWnd, &Point);

            // 2004.07.26.myevan.Ahn Chul-soo collides with HackShield
            SetCursorPos(Point.x, Point.y);
        }
    }

    RECT rcWnd;
    GetClientRect(&rcWnd);

    m_kWndMgr.SetResolution(rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top);
    m_kWndMgr.RunMouseMove(x, y);
}

void CPythonApplication::OnMouseLeftButtonDown(int x, int y)
{
    m_kWndMgr.RunMouseMove(x, y);
    m_kWndMgr.RunMouseLeftButtonDown();
}

void CPythonApplication::OnMouseLeftButtonUp(int x, int y)
{
    m_kWndMgr.RunMouseMove(x, y);
    m_kWndMgr.RunMouseLeftButtonUp();
}

void CPythonApplication::OnMouseLeftButtonDoubleClick(int x, int y)
{
    m_kWndMgr.RunMouseMove(x, y);
    m_kWndMgr.RunMouseLeftButtonDoubleClick();
}

void CPythonApplication::OnMouseRightButtonDown(int x, int y)
{
    m_kWndMgr.RunMouseMove(x, y);
    m_kWndMgr.RunMouseRightButtonDown();
}

void CPythonApplication::OnMouseRightButtonUp(int x, int y)
{
    m_kWndMgr.RunMouseMove(x, y);
    m_kWndMgr.RunMouseRightButtonUp();
}

void CPythonApplication::OnChar(uint32_t ch)
{
    m_kWndMgr.RunChar(ch);
}

void CPythonApplication::OnKeyDown(KeyCode code)
{
    m_keyboard.OnKeyDown(code);

    if (code == kVirtualKeyTab)
    {
        m_kWndMgr.RunTab();

        const auto rkPlayer = CPythonPlayer::InstancePtr();
        if (rkPlayer)
        {
            const auto pkInstTarget =
                CPythonCharacterManager::Instance().GetTabNextTargetPointer(rkPlayer->NEW_GetMainActorPtr());
            if (pkInstTarget)
                rkPlayer->SetTarget(pkInstTarget->GetVirtualID(), true);
        }
    }

    m_kWndMgr.RunKeyDown(code);
}

void CPythonApplication::OnKeyUp(KeyCode code)
{
    m_keyboard.OnKeyUp(code);
    m_kWndMgr.RunKeyUp(code);
}

void CPythonApplication::RunPressExitKey()
{
    m_kWndMgr.RunPressExitKey();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPythonApplication::OnMouseUpdate()
{
    auto [lx, ly] = m_kWndMgr.GetMousePosition();
    PyCallClassMemberFunc(m_poMouseHandler, "Update", Py_BuildValue("(ii)", lx, ly));
}

void CPythonApplication::OnMouseRender()
{
    rmt_ScopedCPUSample(OnMouseRender, 0);

    PyCallClassMemberFunc(m_poMouseHandler, "Render", Py_BuildValue("()"));
}
