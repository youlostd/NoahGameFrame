// MapObjectPage.cpp : implementation file
//

#include "stdafx.h"
#include "MapObjectPage.h"
#include "..\WorldEditor.h"
#include "../MainFrm.h"
#include "../WorldEditorDoc.h"
#include "../Dialog/MapObjectDirectory.h"
#include "../Dialog/MapObjectProperty.h"
#include "../Dialog/InputNumber.h"
#include "../DataCtrl/MapAccessorOutdoor.h"

#include <GameLib/Property.h>

#include <EterBase/Utils.h>

METIN2_BEGIN_NS

CMapObjectPage::CMapObjectPage(CWnd* pParent /*=NULL*/)
	: CPageCtrl(CMapObjectPage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapObjectPage)
	//}}AFX_DATA_INIT
}

CMapObjectPage::~CMapObjectPage()
{
}

void CMapObjectPage::DoDataExchange(CDataExchange* pDX)
{
	CPageCtrl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapObjectPage)
	DDX_Control(pDX, IDD_MAP_PORTAL_NUMBER_LIST, m_ctrlPortalNumber);
	DDX_Control(pDX, IDC_STATIC_SELECTED_OBJECT_INFO, m_StaticSelectedObjectInfo);
	DDX_Control(pDX, IDC_MAP_OBJECT_SCALE, m_ctrlObjectScale);
	DDX_Control(pDX, IDC_MAP_OBJECT_YAW, m_ctrlCursorYaw);
	DDX_Control(pDX, IDC_MAP_OBJECT_ROLL, m_ctrlCursorRoll);
	DDX_Control(pDX, IDC_MAP_OBJECT_PITCH, m_ctrlCursorPitch);
	DDX_Control(pDX, IDC_MAP_OBJECT_HEIGHT, m_ctrlObjectHeight);
	DDX_Control(pDX, IDC_MAP_OBJECT_GRID_DISTANCE, m_ctrlGridDistance);
	DDX_Control(pDX, IDC_MAP_OBJECT_TREE, m_ctrlPropertyTree);
	//}}AFX_DATA_MAP

	m_ctrlCursorRoll.SetRangeMin(0);
	m_ctrlCursorRoll.SetRangeMax(CArea::ROTATION_STEP_COUNT);
	m_ctrlCursorYaw.SetRangeMin(0);
	m_ctrlCursorYaw.SetRangeMax(CArea::YAW_STEP_COUNT);
	m_ctrlCursorPitch.SetRangeMin(0);
	m_ctrlCursorPitch.SetRangeMax(CArea::PITCH_STEP_COUNT);
	m_ctrlGridDistance.SetRangeMin(1);
	m_ctrlGridDistance.SetRangeMax(50);
	for (int i = 10; i < 50; i+=10)
	{
		m_ctrlGridDistance.SetTic(i);
	}
	m_ctrlObjectHeight.SetRangeMin(0);
	m_ctrlObjectHeight.SetRangeMax(18000);
	m_ctrlObjectHeight.SetTic(9000);
	m_ctrlObjectScale.SetRangeMin(0);
	m_ctrlObjectScale.SetRangeMax(10000);
}


BEGIN_MESSAGE_MAP(CMapObjectPage, CPageCtrl)
	//{{AFX_MSG_MAP(CMapObjectPage)
	ON_BN_CLICKED(IDD_MAP_CREATE_PROPERTY, OnCreateProperty)
	ON_BN_CLICKED(IDD_MAP_CREATE_DIRECTORY, OnCreateDirectory)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDD_MAP_OBJECT_RANDOM_ROTATION, OnCheckRandomRotation)
	ON_BN_CLICKED(IDD_MAP_OBJECT_GRID_MODE, OnCheckGridModeEnable)
	ON_NOTIFY(NM_DBLCLK, IDC_MAP_OBJECT_TREE, OnDblclkMapObjectTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_MAP_OBJECT_TREE, OnSelchangedMapObjectTree)
	ON_BN_CLICKED(IDD_MAP_DELETE, OnDelete)
	ON_BN_CLICKED(IDD_MAP_UNSELECT, OnUnselect)
	ON_BN_CLICKED(IDD_MAP_OBJECT_INSERT_PORTAL_NUMBER, OnInsertPortalNumber)
	ON_BN_CLICKED(IDD_MAP_OBJECT_DELETE_PORTAL_NUMBER, OnDeletePortalNumber)
	//}}AFX_MSG_MAP
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyTreeControler normal functions

void CPropertyTreeControler::RegisterPath(HTREEITEM hTreeItem, const std::string & c_rstrPathName)
{
	m_PathMap.insert(TPathMap::value_type(hTreeItem, c_rstrPathName));
}
void CPropertyTreeControler::DeletePath(HTREEITEM hTreeItem)
{
	TPathIterator itor = m_PathMap.find(hTreeItem);

	if (m_PathMap.end() == itor)
		return;

	m_PathMap.erase(itor);
}
bool CPropertyTreeControler::GetPath(HTREEITEM hTreeItem, const char ** c_ppszPathName)
{
	TPathIterator itor = m_PathMap.find(hTreeItem);

	if (m_PathMap.end() == itor)
		return false;

	*c_ppszPathName = (itor->second).c_str();

	return true;
}
CPropertyTreeControler::CPropertyTreeControler()
{
	m_PathMap.insert(TPathMap::value_type(TVI_ROOT, "Property\\"));
}
CPropertyTreeControler::~CPropertyTreeControler()
{
}

const char * GetNumberText(int iNumber)
{
	static char s_szText[16+1];
	_snprintf(s_szText, 16, "%d", iNumber);

	return s_szText;
}

/////////////////////////////////////////////////////////////////////////////
// CMapEditPage normal functions

BOOL CMapObjectPage::Create(CWnd * pParent)
{
	if (!CPageCtrl::Create(CMapObjectPage::IDD, pParent))
		return FALSE;

	return TRUE;
}

void CMapObjectPage::Initialize()
{
	// Make Image List
	CreateHighColorImageList(IDB_MAP_OBJECT_TREE_ITEM_IMAGE_LIST, &m_TreeImageList);
	m_ctrlPropertyTree.SetImageList(&m_TreeImageList, TVSIL_NORMAL);

	LoadProperty();

	// Setting Brush
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pSceneMap = pDocument->GetSceneMap();

	int iScale = 100;
	int iDensity = 1;
	int iRandom = 0;
	int iCursorRotation = 0;
	int iGridDistance = 10;
	float fObjectHeight = 0.0f;
	uint32_t dwObjectScale = 1000;
	pSceneMap->SetObjectBrushScale(iScale);
	pSceneMap->SetObjectBrushDensity(iDensity);
	pSceneMap->SetObjectBrushRandom(iRandom);
	pSceneMap->SetObjectHeight(0.0f);
	pSceneMap->SetRandomRotation(IsDlgButtonChecked(IDD_MAP_OBJECT_RANDOM_ROTATION) == 1 ? true : false);
	pSceneMap->SetGridMode(IsDlgButtonChecked(IDD_MAP_OBJECT_GRID_MODE) == 1 ? true : false);
	pSceneMap->SetGridDistance(float(m_ctrlGridDistance.GetPos() * 10));
	pSceneMap->SetCursorScale(dwObjectScale);
	m_ctrlCursorRoll.SetPos(CArea::ROTATION_STEP_COUNT/2);
	m_ctrlCursorYaw.SetPos(CArea::YAW_STEP_COUNT/2);
	m_ctrlCursorPitch.SetPos(CArea::PITCH_STEP_COUNT/2);
	m_ctrlGridDistance.SetPos(iGridDistance);
	m_ctrlObjectHeight.SetPos(int(fObjectHeight) + 9000);
	m_ctrlObjectScale.SetPos(dwObjectScale);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_GRID_DISTANCE_PRINT, float(m_ctrlGridDistance.GetPos()/10.0f));
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_HEIGHT_PRINT, float(m_ctrlObjectHeight.GetPos() - 9000)/100.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_SCALE_PRINT, float(m_ctrlObjectScale.GetPos())/100.0f);
}

void CMapObjectPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pSceneMap = pDocument->GetSceneMap();

	float fYaw		= SliderValueToCursorRotation(m_ctrlCursorYaw.GetPos(),CArea::YAW_STEP_COUNT, CArea::YAW_STEP_AMOUNT);
	float fPitch	= SliderValueToCursorRotation(m_ctrlCursorPitch.GetPos(),CArea::PITCH_STEP_COUNT, CArea::PITCH_STEP_AMOUNT);
	float fRoll		= SliderValueToCursorRotation(m_ctrlCursorRoll.GetPos(),CArea::ROTATION_STEP_COUNT, CArea::ROTATION_STEP_AMOUNT);
	pSceneMap->AddSelectedObjectRotation(fYaw - pSceneMap->GetCursorYaw()
										, fPitch - pSceneMap->GetCursorPitch()
										, fRoll - pSceneMap->GetCursorRoll() );


	float fHeight	= float(m_ctrlObjectHeight.GetPos() - 9000);
	pSceneMap->MoveSelectedObjectHeight(fHeight - pSceneMap->GetCursorObjectHeight());

	pSceneMap->SetCursorYawPitchRoll(fYaw, fPitch, fRoll);
	pSceneMap->SetGridDistance(float(m_ctrlGridDistance.GetPos() * 10));
	pSceneMap->SetObjectHeight(float(m_ctrlObjectHeight.GetPos() - 9000));
	pSceneMap->SetCursorScale(m_ctrlObjectScale.GetPos());
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_GRID_DISTANCE_PRINT, float(m_ctrlGridDistance.GetPos()/10.0f));
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_HEIGHT_PRINT, float(m_ctrlObjectHeight.GetPos() - 9000)/100.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_SCALE_PRINT, float(m_ctrlObjectScale.GetPos())/100.0f);

	CPageCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMapObjectPage::LoadProperty()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CMapManagerAccessor * pMapManagerAccessor = pDocument->GetMapManagerAccessor();

	BOOL bSuccessed = m_ctrlPropertyTree.DeleteAllItems();
	pMapManagerAccessor->LoadProperty(&m_ctrlPropertyTree);
}

void CMapObjectPage::UpdateUI()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pSceneMap = pDocument->GetSceneMap();
	CMapManagerAccessor * pMapManagerAccessor = pDocument->GetMapManagerAccessor();
	CMapOutdoorAccessor * pOutdoor = (CMapOutdoorAccessor *) &pMapManagerAccessor->GetMapOutdoorRef();

	m_ctrlCursorYaw.SetPos(CursorRotationToSliderValue(pSceneMap->GetCursorYaw(),CArea::YAW_STEP_COUNT,CArea::YAW_STEP_AMOUNT));
	m_ctrlCursorPitch.SetPos(CursorRotationToSliderValue(pSceneMap->GetCursorPitch(),CArea::PITCH_STEP_COUNT,CArea::PITCH_STEP_AMOUNT));
	m_ctrlCursorRoll.SetPos(CursorRotationToSliderValue(pSceneMap->GetCursorRoll(),CArea::ROTATION_STEP_COUNT,CArea::ROTATION_STEP_AMOUNT));
	m_ctrlObjectHeight.SetPos(int(pSceneMap->GetCursorObjectHeight()) + 9000);
	m_ctrlObjectScale.SetPos(pSceneMap->GetCursorScale());
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_GRID_DISTANCE_PRINT, float(m_ctrlGridDistance.GetPos()/10.0f));
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_HEIGHT_PRINT, float(m_ctrlObjectHeight.GetPos() - 9000)/100.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_SCALE_PRINT, float(m_ctrlObjectScale.GetPos())/100.0f);

	__RefreshSelectedObjectInfo();
	__RefreshSelectedObjectProperty();

	// Cursor
	const HTREEITEM c_hTreeItem = m_ctrlPropertyTree.GetSelectedItem();
	if (c_hTreeItem)
	{
		uint32_t dwCRC = m_ctrlPropertyTree.GetItemData(c_hTreeItem);
		pSceneMap->ChangeCursor(dwCRC);
	}
	else
	{
		pSceneMap->ClearCursor();
	}
}

void CMapObjectPage::__RefreshSelectedObjectInfo()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pSceneMap = pDocument->GetSceneMap();
	CMapManagerAccessor * pMapManagerAccessor = pDocument->GetMapManagerAccessor();
	CMapOutdoorAccessor * pOutdoor = (CMapOutdoorAccessor *) &pMapManagerAccessor->GetMapOutdoorRef();

	// Selected Object Name
	const char * c_szSelectedObjectName = pOutdoor->GetSelectedObjectName();
	m_StaticSelectedObjectInfo.SetWindowText(c_szSelectedObjectName);

	// Selected Object Portal
	m_ctrlPortalNumber.ResetContent();
	const CMapOutdoorAccessor::TPortalNumberVector & c_rkVec_iPortalNumber = pOutdoor->GetSelectedObjectPortalVectorRef();
	for (uint32_t i = 0; i != c_rkVec_iPortalNumber.size(); ++i)
	{
		int iNum = c_rkVec_iPortalNumber[i];
		m_ctrlPortalNumber.InsertString(i, _getf("Portal Number : %d", iNum));
	}
}

void CMapObjectPage::__RefreshSelectedObjectProperty()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pSceneMap = pDocument->GetSceneMap();
	CMapManagerAccessor * pMapManagerAccessor = pDocument->GetMapManagerAccessor();
	CMapOutdoorAccessor * pOutdoor = (CMapOutdoorAccessor *) &pMapManagerAccessor->GetMapOutdoorRef();

	CAreaAccessor * pAreaAccessor;
	if (!pOutdoor->GetAreaAccessor(4, &pAreaAccessor))		// -_- 공포의 숫자 4는... 이런 문법 관련해서 다 그렇게 하드코딩 되어있다 -_-;;
		return;

	const CArea::TObjectData* pData = pAreaAccessor->GetLastSelectedObjectData();

	if (pData)
	{
		// Rotation Values
		m_ctrlCursorYaw.SetPos(CursorRotationToSliderValue(pData->m_fYaw, CArea::YAW_STEP_COUNT, CArea::YAW_STEP_AMOUNT));
		m_ctrlCursorPitch.SetPos(CursorRotationToSliderValue(pData->m_fPitch, CArea::PITCH_STEP_COUNT, CArea::PITCH_STEP_AMOUNT));
		m_ctrlCursorRoll.SetPos(CursorRotationToSliderValue(pData->m_fRoll, CArea::ROTATION_STEP_COUNT, CArea::ROTATION_STEP_AMOUNT));

		// , CCursorRenderer 에 값을 적용시켜주지 않으면 물체 Rotation 이 이상하게 작동한다.
		pSceneMap->SetCursorYaw(pData->m_fYaw);
		pSceneMap->SetCursorPitch(pData->m_fPitch);
		pSceneMap->SetCursorRoll(pData->m_fRoll);
		pSceneMap->SetObjectHeight(pData->m_fHeightBias);

		// Height Bias - Slider Ctrl, Text Value
		m_ctrlObjectHeight.SetPos(int(pData->m_fHeightBias + 9000));
		SetDialogFloatText(GetSafeHwnd(), IDC_MAP_OBJECT_HEIGHT_PRINT, float(m_ctrlObjectHeight.GetPos() - 9000)/100.0f);
	}
}


bool CMapObjectPage::GetCurrentParentItem(HTREEITEM * pTreeItem, const char ** c_ppszPathName)
{
	HTREEITEM hTreeItem = m_ctrlPropertyTree.GetSelectedItem();

	if (!hTreeItem)
		return false;

	// when client select child node which is not directory,
	// change parent root to directory
	while (!m_ctrlPropertyTree.GetPath(hTreeItem, c_ppszPathName))
	{
		hTreeItem = m_ctrlPropertyTree.GetParentItem(hTreeItem);
		if (!hTreeItem)
			hTreeItem = TVI_ROOT;
	}

	*pTreeItem = hTreeItem;

	return true;
}

int CMapObjectPage::SliderValueToCursorRotation(int iStep, int iCount, int iAmount)
{
	if (iStep > iCount/2)
	{
		return (iStep - iCount/2) * iAmount;
	}
	else if (iStep < iCount/2)
	{
		return (iCount/2 + iStep) * iAmount;
	}

	return 0;
}

int CMapObjectPage::CursorRotationToSliderValue(int iRotation, int iCount, int iAmount)
{
	int iStep = iRotation / iAmount;
	iStep = (iStep + iCount/2) % iCount;

	return iStep;
}

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPage message handlers
BOOL CMapObjectPage::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch(LOWORD(wParam))
	{
		case IDC_MAP_OBJECT_TREE:
			switch (LOWORD(lParam))
			{
				case 63528:
				case 63348:
				case 63564:
					{
					}
					break;
			}

			break;
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

/* (금)
*  New Directory 버튼 기능 수정
*  Root 에도 폴더만들수 있게 변경.
*/
void CMapObjectPage::OnCreateDirectory()
{
	const char * c_szPathName;
	HTREEITEM hParentItem;

	if (!GetCurrentParentItem(&hParentItem, &c_szPathName))
	{
		c_szPathName	= "property";
		hParentItem		= TVI_ROOT;
	}

	CMapObjectDirectory CreateDirectory;

	if (CreateDirectory.DoModal())
	{
		std::string strPathName;
		strPathName = c_szPathName;
		strPathName += "/";
		strPathName += CreateDirectory.GetDirectoryName();

		::CreateDirectory(strPathName.c_str(), NULL);

		char szDirectoryName[64+1];
		_snprintf(szDirectoryName, 64, "[ %s ]", CreateDirectory.GetDirectoryName());
		HTREEITEM hTreeItem = m_ctrlPropertyTree.InsertItem(szDirectoryName, 0, 1, hParentItem, TVI_SORT);
		m_ctrlPropertyTree.RegisterPath(hTreeItem, strPathName.c_str());
		m_ctrlPropertyTree.SelectItem(hTreeItem);
	}


	/*const char * c_szPathName;
	HTREEITEM hParentItem;

	if (!GetCurrentParentItem(&hParentItem, &c_szPathName))
		return;

	if (TVI_ROOT != hParentItem)
		m_ctrlPropertyTree.SelectItem(hParentItem);

	CMapObjectDirectory CreateDirectory;

	if (CreateDirectory.DoModal())
	{
		std::string strPathName;
		strPathName = c_szPathName;
		strPathName += "/";
		strPathName += CreateDirectory.GetDirectoryName();

		::CreateDirectory(strPathName.c_str(), NULL);

		char szDirectoryName[64+1];
		_snprintf(szDirectoryName, 64, "[ %s ]", CreateDirectory.GetDirectoryName());
		HTREEITEM hTreeItem = m_ctrlPropertyTree.InsertItem(szDirectoryName, 0, 1, hParentItem, TVI_SORT);
		m_ctrlPropertyTree.RegisterPath(hTreeItem, strPathName.c_str());
	}*/
}

void CMapObjectPage::OnCreateProperty()
{
	HTREEITEM hTreeItem;
	const char * c_szPathName;
	if (!GetCurrentParentItem(&hTreeItem, &c_szPathName))
		return;
	SPDLOG_INFO("Save path {0}", c_szPathName);

	if (TVI_ROOT != hTreeItem)
		m_ctrlPropertyTree.SelectItem(hTreeItem);

	CMapObjectProperty CreateProperty;
	CreateProperty.SetPath(c_szPathName);

	if (CreateProperty.DoModal())
	{
		int iType = CreateProperty.GetPropertyType();
		HTREEITEM hNewItem = m_ctrlPropertyTree.InsertItem(CreateProperty.GetPropertyName(), iType+1, iType+1, hTreeItem, TVI_SORT);
		m_ctrlPropertyTree.SetItemData(hNewItem, CreateProperty.GetPropertyCRC32());
	}
}

/* (금)
*  폴더 삭제시 부모아이템을 선택하게 추가.
*/
void CMapObjectPage::OnDelete()
{
	HTREEITEM hTreeItem		= m_ctrlPropertyTree.GetSelectedItem();
	HTREEITEM hParentItem	= m_ctrlPropertyTree.GetParentItem(hTreeItem);

	if (!hTreeItem)
		return;

	const char * pszPath;

	if (m_ctrlPropertyTree.GetPath(hTreeItem, &pszPath)) // 선택된 것이 디렉토리 라면..
	{
		if (!RemoveDirectory(pszPath))
		{
			spdlog::error("하위에 파일 혹은 폴더가 존재 합니다");
			return;
		}

		m_ctrlPropertyTree.DeletePath(hTreeItem);
		if (m_ctrlPropertyTree.DeleteItem(hTreeItem))
			m_ctrlPropertyTree.SelectItem(hParentItem);
	}
	else
	{
		HTREEITEM hParentItem;
		const char * c_szPathName;

		if (!GetCurrentParentItem(&hParentItem, &c_szPathName))
			return;

		if (IDYES != MessageBox("정말 지우시겠습니까?", NULL, MB_YESNO))
			return;

		std::string stFileName;
		stFileName = c_szPathName;
		stFileName += "/";
		stFileName += m_ctrlPropertyTree.GetItemText(hTreeItem);

		if (CPropertyManager::Instance().Erase(stFileName.c_str()))	// 내부적으로 DeleteFile 호출 함
		{
			m_ctrlPropertyTree.DeletePath(hTreeItem);
			m_ctrlPropertyTree.DeleteItem(hTreeItem);
		}
	}
}

void CMapObjectPage::OnUnselect()
{
	m_ctrlPropertyTree.SelectItem(NULL);

	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pScene = pDocument->GetSceneMap();
	pScene->ChangeCursor(0);
}

void CMapObjectPage::OnCheckRandomRotation()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pSceneMap = pDocument->GetSceneMap();

	pSceneMap->SetRandomRotation(IsDlgButtonChecked(IDD_MAP_OBJECT_RANDOM_ROTATION) == 1 ? true : false);
}

void CMapObjectPage::OnCheckGridModeEnable()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pSceneMap = pDocument->GetSceneMap();

	pSceneMap->SetGridMode(IsDlgButtonChecked(IDD_MAP_OBJECT_GRID_MODE) == 1 ? true : false);
}

void CMapObjectPage::OnDblclkMapObjectTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	const HTREEITEM c_hTreeItem = m_ctrlPropertyTree.GetSelectedItem();
	uint32_t dwCRC = m_ctrlPropertyTree.GetItemData(c_hTreeItem);

	CProperty * pProperty;
	if (!CPropertyManager::Instance().Get(dwCRC, &pProperty))
		return;

	const HTREEITEM c_hParentItem = m_ctrlPropertyTree.GetParentItem(c_hTreeItem);
	const char * c_szPath;

	if (!m_ctrlPropertyTree.GetPath(c_hParentItem, &c_szPath))
		return;

	CMapObjectProperty ObjectProperty;

	ObjectProperty.SetPath(c_szPath);
	ObjectProperty.SetData(pProperty);

	if (ObjectProperty.DoModal())
	{
		CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
		CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
		CSceneMap * pScene = pDocument->GetSceneMap();

		pScene->RefreshCursor();
		pScene->RefreshArea();
	}
}

void CMapObjectPage::OnSelchangedMapObjectTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	const HTREEITEM c_hTreeItem = pNMTreeView->itemNew.hItem;

	if (!c_hTreeItem)
		return;

	uint32_t dwCRC = m_ctrlPropertyTree.GetItemData(c_hTreeItem);

	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap * pScene = pDocument->GetSceneMap();

	pScene->ChangeCursor(dwCRC);

	*pResult = 0;
}

void CMapObjectPage::OnInsertPortalNumber()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CMapManagerAccessor * pMapManagerAccessor = pDocument->GetMapManagerAccessor();
	CMapOutdoorAccessor * pOutdoor = (CMapOutdoorAccessor *) &pMapManagerAccessor->GetMapOutdoorRef();

	if (!pMapManagerAccessor->IsSelected())
	{
		spdlog::error("선택 중인 오브젝트가 없습니다");
		return;
	}

	CInputNumber InputNumber;
	if (InputNumber.DoModal())
	{
		int iNum = InputNumber.GetNumber();
		pMapManagerAccessor->SetSelectedObjectPortalNumber(iNum);

		__RefreshSelectedObjectInfo();
	}
}

void CMapObjectPage::OnDeletePortalNumber()
{
	uint32_t dwIndex = m_ctrlPortalNumber.GetCurSel();

	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CMapManagerAccessor * pMapManagerAccessor = pDocument->GetMapManagerAccessor();
	CMapOutdoorAccessor * pOutdoor = (CMapOutdoorAccessor *) &pMapManagerAccessor->GetMapOutdoorRef();
	const CMapOutdoorAccessor::TPortalNumberVector & c_rkVec_iPortalNumber = pOutdoor->GetSelectedObjectPortalVectorRef();

	if (dwIndex >= c_rkVec_iPortalNumber.size())
		return;

	pMapManagerAccessor->DelSelectedObjectPortalNumber(c_rkVec_iPortalNumber[dwIndex]);
}

/*
*  에디트 모드(오브젝트) 일때 단축키 제공.
*  CSceneMap 에서만 추가 하니 포커스를 CMapObjectPage 으로 뺐기고 나면
*  단축키가 작동하지 않아 CMapObjectPage 에
*  PreTranslateMessage 와 OnKeyDown 를 재정의 하였다.
*  현재 같은 기능에 대해 CSceneMap 과 CMapObjectPage 두 곳에서 정의되어있다.
*/
void CMapObjectPage::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	switch(nChar)
	{
	case VK_1:
		m_ctrlCursorYaw.SetFocus();
		break;
	case  VK_2:
		m_ctrlCursorPitch.SetFocus();
		break;
	case  VK_3:
		m_ctrlCursorRoll.SetFocus();
		break;
	case VK_4:
		m_ctrlObjectHeight.SetFocus();
		break;
	/*
	*  오브젝트 설치시 Snap 활성,비활성을 많이 하신다고 한다.
	*  포커스가 MapObjectPage 일때도 Snap 을 활성 비활성화 할수 있게 단축키 추가
	*  같은 부분이 SceneMap 의 OnKeyDown에도 있다.
	*/
	case VK_S:
		CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
		CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
		CSceneMap * pSceneMap = pDocument->GetSceneMap();
		pSceneMap->SnapEnable();
		break;
	}

	CPageCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CMapObjectPage::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	switch(pMsg->message)
	{
	case WM_KEYDOWN:
		SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
		break;
	}

	return CPageCtrl::PreTranslateMessage(pMsg);
}

METIN2_END_NS
