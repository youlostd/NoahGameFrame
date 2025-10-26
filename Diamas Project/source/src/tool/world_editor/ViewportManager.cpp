#include "StdAfx.h"
#include "ViewportManager.h"
#include "MainFrm.h"
#include "WorldEditorDoc.h"

#include <eterlib/Camera.h>

METIN2_BEGIN_NS

void CViewportManager::ControlKeyDown()
{
	m_ControlKey = true;
}

void CViewportManager::ControlKeyUp()
{
	m_ControlKey = false;
}

void CViewportManager::LeftButtonDown(CPoint Mouse)
{
	m_ClickedLeftButton = true;
	m_ClickedLeftButtonPoint = Mouse;
}

void CViewportManager::LeftButtonUp(CPoint Mouse)
{
	m_ClickedLeftButton = false;
	m_ClickedLeftButtonPoint = CPoint(-1, -1);
}

void CViewportManager::MiddleButtonDown(CPoint Mouse)
{
	m_ClickedMiddleButton = true;
	m_ClickedMiddleButtonPoint = Mouse;
}

void CViewportManager::MiddleButtonUp(CPoint Mouse)
{
	m_ClickedMiddleButton = false;
	m_ClickedMiddleButtonPoint = CPoint(-1, -1);
}

void CViewportManager::RightButtonDown(CPoint Mouse, float localX, float localY)
{
	m_ClickedRightButton = true;
	m_ClickedRightButtonPoint = Mouse;
	m_lastRightLocalX = localX;
	m_lastRightLocalY = localY;
}

void CViewportManager::RightButtonUp(CPoint Mouse)
{
	m_ClickedRightButton = false;
	m_ClickedRightButtonPoint = CPoint(-1, -1);
}

void CViewportManager::MouseWheel(float Wheel)
{
	float fcurDistance = CCameraManager::Instance().GetCurrentCamera()->GetDistance();
	if (fcurDistance > 40000.0f)
		Wheel *= 10.0f;
	else if (fcurDistance > 10000.0f)
	{
		Wheel *= 5.0f;
	}
	else if (fcurDistance > 7000.0f)
	{
		Wheel *= 3.0f;
	}
	else if (fcurDistance > 3000.0f)
	{
		Wheel *= 2.0f;
	}

	float fDistance = CCameraManager::Instance().GetCurrentCamera()->GetDistance();
	fDistance = std::min(std::max(fDistance - Wheel * 2.0f, 200.0f), 80000.0f);
	CCameraManager::Instance().GetCurrentCamera()->SetDistance(fDistance);
}

void CViewportManager::MouseMove(CPoint Mouse)
{
	CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	CMainFrame* mainFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CWorldEditorDoc* doc = (CWorldEditorDoc*)mainFrame->GetActiveView()->GetDocument();

	if (m_ControlKey && m_ClickedRightButton) {
		float fRoll, fPitch;
		fRoll =  - (m_ClickedRightButtonPoint.x - Mouse.x) * 0.3f;
		fPitch = std::max(-pCurrentCamera->GetPitch() - 89.9f,
		                  std::min(89.9f - pCurrentCamera->GetPitch(),
		                           -(m_ClickedRightButtonPoint.y - Mouse.y) * 0.3f));

		pCurrentCamera->RotateEyeAroundTarget(fPitch, fRoll);

		m_ClickedRightButtonPoint = Mouse;
	}
}

float CViewportManager::GetFrustumFarDistance()
{
	return m_fFrustumFar;
}

void CViewportManager::SetScreenStatus(int iWidth, int iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
}

void CViewportManager::SetCursorPosition(int ix, int iy)
{
	m_ixCursor = ix;
	m_iyCursor = iy;
}

bool CViewportManager::CheckControlKey()
{
	return m_ControlKey;
}

bool CViewportManager::isLeftButton()
{
	return m_ClickedLeftButton;
}

bool CViewportManager::isMiddleButton()
{
	return m_ClickedMiddleButton;
}

bool CViewportManager::isRightButton()
{
	return m_ClickedRightButton;
}

bool CViewportManager::IsMouseTranslationMode()
{
	return !m_ControlKey && m_ClickedRightButton;
}

void CViewportManager::UpdateMouseTranslation(float x, float y, float& xMove, float& yMove)
{
	xMove = m_lastRightLocalX - x;
	yMove = m_lastRightLocalY - y;

	m_lastRightLocalX = x;
	m_lastRightLocalY = y;
}

void CViewportManager::Initialize()
{
	m_fFrustumNear = 100.0f;
	m_fFrustumFar = 120000.0f;

	m_ClickedLeftButtonPoint = CPoint(-1, -1);
	m_ClickedMiddleButtonPoint = CPoint(-1, -1);
	m_ClickedRightButtonPoint = CPoint(-1, -1);

	m_ControlKey = false;
	m_ClickedLeftButton = false;
	m_ClickedMiddleButton = false;
	m_ClickedRightButton = false;
}

CViewportManager::CViewportManager()
{
	Initialize();
}

CViewportManager::~CViewportManager()
{
}


//////////////////////////////////////////////////////////////////////////
// CCAmeraManager 함수들...
//////////////////////////////////////////////////////////////////////////

void CCamera::ProcessTerrainCollision()
{
}

void CCamera::ProcessBuildingCollision()
{
}

void CCamera::Update()
{
}

METIN2_END_NS
