#include "stdafx.h"
#include "WorldEditor.h"
#include "MainFrm.h"
#include "WorldEditorDoc.h"
#include "WorldEditorView.h"
#include "Dialog/DlgGoto.h"
#include "DataCtrl/ShadowRenderHelper.h"

#include <GameLib/GameUtil.h>
#include <EterLib/Camera.h>
#include <EterBase/Utils.h>


#include <storm/math/AngleUtil.hpp>
#include <storm/math/Eigen.hpp>

#include <Eigen/Geometry>

METIN2_BEGIN_NS

IMPLEMENT_DYNCREATE(CWorldEditorView, CView)

BEGIN_MESSAGE_MAP(CWorldEditorView, CView)
	//{{AFX_MSG_MAP(CWorldEditorView)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_CREATE()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

#define EDITOR_LOCAL_CAMERA		10
/////////////////////////////////////////////////////////////////////////////
// CWorldEditorView construction/destruction

CWorldEditorView::CWorldEditorView()
	: m_bRenderUI(false)
	, m_bInitialized(false)
	, m_isLocked(false)
	, m_isRight(false)
	, m_isLeft(false)
	, m_isDown(false)
	, m_isUp(false)
{
	CCameraManager::Instance().AddCamera(EDITOR_LOCAL_CAMERA);
}

CWorldEditorView::~CWorldEditorView()
{
	// dtor
}

int CWorldEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pDlgPerspective.reset(new CDlgPerspective);
	m_pDlgPerspective->Create(IDD_DIALOG_PERSPECTIVE, this);
	m_pDlgPerspective->SetWindowPos(NULL, 0, 0, 0, 0, TRUE);
	m_pDlgPerspective->SetFOV(40.0f);
	m_pDlgPerspective->SetNearDistance(100.0f);
	m_pDlgPerspective->SetFarDistance(30000.0f);
	return 0;
}

BOOL CWorldEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CWorldEditorView::Initialize()
{
	CSceneBase::CreateUI();

	m_Screen.SetPositionCamera(0.0f, 0.0f, 0.0f, 5000.0f, 30.0f, 0.0f);
	Vector3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
	v3Target.z += 100.0f;
	CCameraManager::Instance().GetCurrentCamera()->SetTarget(v3Target);

	UpdateFrustum();

	m_bInitialized = true;

	CWorldEditorDoc * pDoc = GetDocument();
	CSceneMap * pSceneMap = pDoc->GetSceneMap();
	pSceneMap->CreateEnvironment();
}

void CWorldEditorView::Lock()
{
	m_isLocked = true;
}

void CWorldEditorView::Unlock()
{
	m_isLocked = false;
}

bool CWorldEditorView::IsLocked()
{
	return m_isLocked;
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorView drawing

void CWorldEditorView::OnDraw(CDC* pDC)
{
	CWorldEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorView printing

BOOL CWorldEditorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CWorldEditorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CWorldEditorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorView diagnostics

#ifdef _DEBUG
void CWorldEditorView::AssertValid()const
{
	CView::AssertValid();
}

void CWorldEditorView::Dump(CDumpContext& dc)const
{
	CView::Dump(dc);
}

CWorldEditorDoc* CWorldEditorView::GetDocument()// non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWorldEditorDoc)));
	return (CWorldEditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorView normal functions

void CWorldEditorView::TogglePerspectiveDialog()
{
	m_pDlgPerspective->ShowWindow(!m_pDlgPerspective->IsWindowVisible());
}
BOOL CWorldEditorView::isShowingPerspectiveDialog()
{
	return m_pDlgPerspective->IsWindowVisible();
}

void CWorldEditorView::GetMouseLocalPosition(POINT point, float * px, float * py)
{
	CCamera * pOriginalCamera = CCameraManager::Instance().GetCurrentCamera();
 	CCameraManager::Instance().SetCurrentCamera(EDITOR_LOCAL_CAMERA);

	Matrix matIdentity;
	DirectX::SimpleMath::MatrixIdentity(&matIdentity);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matIdentity);
	STATEMANAGER.SaveTransform(D3DTS_VIEW, &matIdentity);

	CRect Rect;
	GetWindowRect(&Rect);
	m_Screen.SetPerspective(m_pDlgPerspective->GetFOV(), float(Rect.Width()) / float(Rect.Height()), 100.0f, m_ViewportManager.GetFrustumFarDistance());
	m_Screen.SetAroundCamera(pOriginalCamera->GetDistance(), pOriginalCamera->GetPitch(), pOriginalCamera->GetRoll(), 100.0f);

	m_Screen.SetCursorPosition(point.x, point.y, Rect.Width(), Rect.Height());
	m_Screen.GetCursorXYPosition(px, py);

	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

 	CCameraManager::Instance().ResetToPreviousCamera();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Camera Start
void CWorldEditorView::UpdateCursorPosition(int ix, int iy)
{
	CRect Rect;
	GetWindowRect(&Rect);
	m_Screen.SetCursorPosition(ix, iy, Rect.Width(), Rect.Height());
}

void CWorldEditorView::UpdateFrustum()
{
	m_pDlgPerspective->SetNearDistance(m_ViewportManager.GetFrustumNearDistance());
	m_pDlgPerspective->SetFarDistance(m_ViewportManager.GetFrustumFarDistance());

	CRect Rect;
	GetWindowRect(&Rect);

	float fFOV = m_pDlgPerspective->GetFOV();
	float fAspect = float(Rect.Width()) / float(Rect.Height());
	float fNearDistance = m_ViewportManager.GetFrustumNearDistance();
	float fFarDistance = m_ViewportManager.GetFrustumFarDistance();

	m_Screen.SetPerspective(fFOV, fAspect, fNearDistance, fFarDistance);
	m_Screen.UpdateViewMatrix();
}

void CWorldEditorView::UpdateTargetPosition(float fx, float fy)
{
	CWorldEditorDoc* pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->MovePosition(fx, fy);
	UpdateFrustum();
}
// Camera End
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// ETC
void CWorldEditorView::ToggleInfoBoxVisibleState()
{
	m_bRenderUI = m_bRenderUI ? false : true;
}
BOOL CWorldEditorView::IsInfoBoxVisible()
{
	return m_bRenderUI;
}
// ETC
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// Input Process Start

void CWorldEditorView::RunKeyDown(int iChar)
{
	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->KeyDown(iChar);
}

void CWorldEditorView::RunKeyUp(int iChar)
{
	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->KeyUp(iChar);
}

void CWorldEditorView::RunSysKeyDown(int iChar)
{
	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->SysKeyDown(iChar);
}

void CWorldEditorView::RunSysKeyUp(int iChar)
{
	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->SysKeyUp(iChar);
}

void CWorldEditorView::RunMouseMove(LONG x, LONG y)
{
	UpdateFrustum();

	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->MouseMove(x, y);
}

void CWorldEditorView::RunLButtonDown(UINT nFlags, CPoint point)
{
	UpdateFrustum();

	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->LButtonDown(nFlags, point);
}

void CWorldEditorView::RunLButtonUp()
{
	UpdateFrustum();

	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->LButtonUp();
}

void CWorldEditorView::RunRButtonDown()
{
	UpdateFrustum();

	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->RButtonDown();
}

void CWorldEditorView::RunRButtonUp()
{
	UpdateFrustum();

	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->RButtonUp();
}

bool CWorldEditorView::RunMouseWheel(short zDelta)
{
	UpdateFrustum();

	CWorldEditorDoc * pDoc = GetDocument();
	CSceneBase * pScene = pDoc->GetActiveScene();
	return pScene->MouseWheel(zDelta);
}

void CWorldEditorView::UpdateKeyMovement()
{
	CRect rect;
	GetWindowRect(&rect);

	// TODO(tim): Optimize
	const auto dir = GetKeyMovementDirection(m_isLeft, m_isRight, m_isUp, m_isDown);
	auto dirRot = GetKeyMovementRotation(dir.first, dir.second,
	                                     rect.Width(), rect.Height());

	const auto cam = CCameraManager::Instance().GetCurrentCamera();
	if (cam) {
		const auto curCamRot = CameraRotationToCharacterRotation(cam->GetRoll());
		dirRot = std::fmod(360.0f + curCamRot + dirRot, 360.0f);
	}

	Eigen::AngleAxisf yawAxis(storm::RadiansFromDegrees(dirRot),
	                          Eigen::Vector3f::UnitZ());

	Eigen::Vector3f pos = 300.0f * (yawAxis * Eigen::Vector3f(0.0f, -1.0f, 0.0f));

	UpdateTargetPosition(pos.x(), pos.y());
}

// Input Process End
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorView message handlers

void CWorldEditorView::OnSize(UINT nType, int cx, int cy)
{
	if (!m_bInitialized)
	{
		CView::OnSize(nType, cx, cy);
		return;
	}

	if (SIZE_MAXIMIZED == nType || SIZE_RESTORED == nType)
	{
		m_Screen.SetViewport(0, 0, cx, cy, 0.0f, 1.0f);
		//////////////////////////////////////////////////////////////////////////
		CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
		CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
		CGraphicDevice & rDevice = pApplication->GetGraphicDevice();

		pMapManagerAccessor->DestroyShadowTexture();
		rDevice.ResizeBackBuffer(cx, cy);
		pMapManagerAccessor->RecreateShadowTexture();
		//////////////////////////////////////////////////////////////////////////
	}

	UpdateFrustum();
	CView::OnSize(nType, cx, cy);
}

// Keyboard
void CWorldEditorView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		case VK_G: // G
			{
				CDlgGoto DlgGoto;
				DlgGoto.DoModal();
			}
			break;

		case VK_CONTROL:
			m_ViewportManager.ControlKeyDown();
			break;

		case VK_F11: // F12로 하면 이상한 에러가 나더군요. 그래서 F11로 수정 - 동현
			{
				m_bRenderUI = !m_bRenderUI;
			}
			break;

		case VK_F4:
			{
				CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
				CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
				pMapManagerAccessor->ToggleWireframe();
			}
			break;

		case 0xDB: // '['
			{
				CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
				CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

				if (pMapManagerAccessor->GetBrushSize() != 0)
					pMapManagerAccessor->SetBrushSize(pMapManagerAccessor->GetBrushSize() - 1);

				CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
				pFrame->UpdateMapControlBar();
			}
			break;

		case 0xDD: // ']'
			{
				CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
				CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
				pMapManagerAccessor->SetBrushSize(pMapManagerAccessor->GetBrushSize() + 1);

				CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
				pFrame->UpdateMapControlBar();
			}
			break;

		case VK_UP:
			m_isUp = true;
			UpdateKeyMovement();
			break;

		case VK_DOWN:
			m_isDown = true;
			UpdateKeyMovement();
			break;

		case VK_LEFT:
			m_isLeft = true;
			UpdateKeyMovement();
			break;

		case VK_RIGHT:
			m_isRight = true;
			UpdateKeyMovement();
			break;
	}

	RunKeyDown(nChar);
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	Invalidate(false);
}

void CWorldEditorView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		case VK_CONTROL:
			m_ViewportManager.ControlKeyUp();
			break;

		case VK_UP:
			m_isUp = false;
			break;

		case VK_DOWN:
			m_isDown = false;
			break;

		case VK_LEFT:
			m_isLeft = false;
			break;

		case VK_RIGHT:
			m_isRight = false;
			break;
	}

	RunKeyUp(nChar);
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
	Invalidate(false);
}

void CWorldEditorView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	RunSysKeyDown(nChar);
	//if (18 == nChar)	// ALT 키
	//{
		//m_ViewportManager.ModeButtonDown();
	//}

	//CView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CWorldEditorView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	RunSysKeyUp(nChar);
	//if (18 == nChar)	// ALT 키
	//{
		//m_ViewportManager.ModeButtonUp();
	//}

	//CView::OnSysKeyUp(nChar, nRepCnt, nFlags);
}

// Movement
void CWorldEditorView::OnMouseMove(UINT nFlags, CPoint point)
{
	m_ViewportManager.MouseMove(point);

	RunMouseMove(point.x, point.y);
	UpdateCursorPosition(point.x, point.y);

	if (m_ViewportManager.IsMouseTranslationMode()) {
		float fx, fy;
		GetMouseLocalPosition(point, &fx, &fy);

		float xMove, yMove;
		m_ViewportManager.UpdateMouseTranslation(fx, fy, xMove, yMove);

		if (GetAsyncKeyState(VK_LSHIFT)&0x8000) {
			xMove *= 5.0f;
			yMove *= 5.0f;
		}

		xMove = fMAX(xMove, -5000.0f);
		yMove = fMAX(yMove, -5000.0f);
		xMove = fMIN(xMove, +5000.0f);
		yMove = fMIN(yMove, +5000.0f);

		UpdateTargetPosition(xMove, yMove);
	}

	CView::OnMouseMove(nFlags, point);
	Invalidate(false);
}

// Left Button
void CWorldEditorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	RunLButtonDown(nFlags, point);
	m_ViewportManager.LeftButtonDown(point);
	CView::OnLButtonDown(nFlags, point);
	SetCapture();
	Invalidate(false);
}
void CWorldEditorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	RunLButtonUp();
	m_ViewportManager.LeftButtonUp(point);
	CView::OnLButtonUp(nFlags, point);
	ReleaseCapture();
	Invalidate(false);
}

// Middle Button
void CWorldEditorView::OnMButtonDown(UINT nFlags, CPoint point)
{
	m_ViewportManager.MiddleButtonDown(point);

	CView::OnMButtonDown(nFlags, point);
	SetCapture();
	Invalidate(false);
}
void CWorldEditorView::OnMButtonUp(UINT nFlags, CPoint point)
{
	m_ViewportManager.MiddleButtonUp(point);
	CView::OnMButtonUp(nFlags, point);
	ReleaseCapture();
	Invalidate(false);
}
BOOL CWorldEditorView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!RunMouseWheel(zDelta))
		m_ViewportManager.MouseWheel(float(zDelta));

	Invalidate(false);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

// Right Button
void CWorldEditorView::OnRButtonDown(UINT nFlags, CPoint point)
{
	RunRButtonDown();

	float fx, fy;
	GetMouseLocalPosition(point, &fx, &fy);
	m_ViewportManager.RightButtonDown(point, fx, fy);

	CView::OnRButtonDown(nFlags, point);
	SetCapture();
	Invalidate(false);
}

void CWorldEditorView::OnRButtonUp(UINT nFlags, CPoint point)
{
	RunRButtonUp();
	m_ViewportManager.RightButtonUp(point);
	CView::OnRButtonUp(nFlags, point);
	ReleaseCapture();
	Invalidate(false);
}

BOOL CWorldEditorView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
//	if (GetFocus() != pWnd)
//		pWnd->SetFocus();
	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CWorldEditorView::Process()
{
	if (IsLocked())
		return;

	CWorldEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CResourceManager::Instance().Update();

	UpdateFrustum();

	CSceneBase * pScene = pDoc->GetActiveScene();
	pScene->Update();

	m_Screen.Begin();

 	pScene->Render(true);

	if (m_bRenderUI)
	{
		///////////////////////////////////////////////////////////////////////////////////////////////
		Matrix matIdentity;
		DirectX::SimpleMath::MatrixIdentity(&matIdentity);
		STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matIdentity);
		STATEMANAGER.SaveTransform(D3DTS_VIEW, &matIdentity);
		STATEMANAGER.SetTransform(D3DTS_WORLD, &matIdentity);
		STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		CRect Rect;
		GetClientRect(&Rect);
		m_Screen.SetOrtho2D(Rect.Width(), Rect.Height(), 400.0f);
		m_Screen.SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);

		pScene->RenderUI();

		STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
		STATEMANAGER.RestoreTransform(D3DTS_VIEW);
		STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	m_Screen.End();
	m_Screen.Show(NULL, GetSafeHwnd());

	m_Screen.ResetFaceCount();
}

METIN2_END_NS
