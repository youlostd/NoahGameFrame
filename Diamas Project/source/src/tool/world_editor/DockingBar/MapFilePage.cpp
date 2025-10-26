#include "stdafx.h"
#include "..\WorldEditor.h"
#include "MapFilePage.h"
#include "../Dialog/NewMapDlg.h"
#include "../Dialog/DlgGoto.h"
#include "../Dialog/DlginitBaseTexture.h"
#include "../MainFrm.h"
#include "../WorldEditorDoc.h"
#include "../WorldEditorView.h"
#include "../DataCtrl/MapAccessorOutdoor.h"
#include "../Dialog/MapPortalDialog.h"

#include <EterBase/Utils.h>

#include <SpdLog.hpp>

METIN2_BEGIN_NS

/*  ZeroMemory 추가
* 1. 로드시 무조건 g_szProgramWindowPath(D:\ymir work\bin) 로 열지 않고
*    최근에 열었던 폴더 경로로 열기위해 m_szInitialFolder 추가
*/
CMapFilePage::CMapFilePage(CWnd* pParent /*=NULL*/)
	: CPageCtrl(CMapFilePage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapFilePage)
		// NOTE: the ClassWizard will add member initialization here
	strEnvironmentDataPath = "D:\\Ymir Work\\environment";
	//}}AFX_DATA_INIT
}

CMapFilePage::~CMapFilePage()
{
	if (m_pPortalDialog)
	{
		delete m_pPortalDialog;
	}

	m_pPortalDialog = NULL;
}

void CMapFilePage::DoDataExchange(CDataExchange* pDX)
{
	CPageCtrl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapFilePage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapFilePage, CPageCtrl)
	//{{AFX_MSG_MAP(CMapFilePage)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_OPTION_WIRE_FRAME, OnCheckOptionWireFrame)
	ON_BN_CLICKED(IDC_CHECK_GRID, OnCheckGrid)
	ON_BN_CLICKED(IDC_CHECK_GRID2, OnCheckGrid2)
	ON_BN_CLICKED(IDC_CHECK_CHARACTER, OnCheckCharacterRendering)
	ON_BN_CLICKED(IDC_MAP_FILE_NEW, OnNewMap)
	ON_BN_CLICKED(IDC_MAP_FILE_LOAD, OnLoadMap)
	ON_BN_CLICKED(IDC_MAP_FILE_SAVE, OnSaveMap)
	ON_BN_CLICKED(IDC_MAP_FILE_SAVEAS, OnSaveAsMap)
	ON_BN_CLICKED(IDC_SAVE_SERVER_ATTR, OnSaveServerAttr)
	ON_BN_CLICKED(IDC_MAP_FILE_SAVE_ATTRMAP, OnSaveAttrMap)
	ON_BN_CLICKED(IDC_BUTTON_GOTO, OnGoto)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_OPTION_OBJECT_COLLISION, OnCheckOptionObjectCollisionRendering)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_OPTION_OBJECT, OnCheckOptionObjectRendering)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_OPTION_OBJECT_SHADOW, OnMapTerrainOptionObjectShadow)
	ON_BN_CLICKED(IDC_BUTTON_INIT_BASETEXTUREMAP, OnButtonInitBasetexturemap)
	ON_BN_CLICKED(IDC_CHECK_PATCH_GRID, OnCheckPatchGrid)
	ON_BN_CLICKED(IDC_CHECK_WATER, OnCheckWater)
	ON_BN_CLICKED(IDC_CHECK_COMPASS, OnCheckCompass)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_ATLAS, OnButtonSaveAtlas)
	ON_BN_CLICKED(IDC_BUTTON_SELECTE_ENVIRONMENT_SET, OnButtonSelecteEnvironmentSet)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_OPTION_TERRAIN, OnCheckTerrainOption)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_TERRAIN_HEIGHT, OnButtonChangeTerrainHeight)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_OPTION_FAKE_PORTAL_ENABLE, OnMapTerrainOptionFakePortalEnable)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_OPTION_FAKE_PORTAL_ID_LIST, OnMapTerrainOptionFakePortalIdList)
	ON_BN_CLICKED(IDC_MAP_TERRAIN_GUILD_AREA, OnMapTerrainGuildArea)
	ON_BN_CLICKED(IDC_BUTTON_ALL_SHADOWMAP_MINIMAP_REFRESH, &CMapFilePage::OnBnClickedButtonAllShadowmapMinimapRefresh)
	ON_BN_CLICKED(IDC_EXPORT_HEIGHTMAPS, &CMapFilePage::OnExportHeightmaps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapFilePage normal functions

BOOL CMapFilePage::Create(CWnd * pParent)
{
	if (!CPageCtrl::Create(CMapFilePage::IDD, pParent))
		return FALSE;

	m_pPortalDialog = new CMapPortalDialog;
	m_pPortalDialog->Create(CMapPortalDialog::IDD, this);
	m_pPortalDialog->ShowWindow(SW_HIDE);

	m_initialFolder = AfxGetApp()->GetProfileString("MRU",
	                                                "MapFolder",
	                                                g_szProgramWindowPath);
	return TRUE;
}

void CMapFilePage::Initialize()
{
	CheckDlgButton(IDC_MAP_TERRAIN_OPTION_WIRE_FRAME, FALSE);
	CheckDlgButton(IDC_CHECK_GRID, FALSE);
	CheckDlgButton(IDC_CHECK_GRID2, FALSE);
	CheckDlgButton(IDC_CHECK_CHARACTER, FALSE);
	CheckDlgButton(IDC_MAP_TERRAIN_OPTION_OBJECT, TRUE);
	CheckDlgButton(IDC_MAP_TERRAIN_OPTION_OBJECT_COLLISION, FALSE);
	CheckDlgButton(IDC_MAP_TERRAIN_OPTION_OBJECT_SHADOW, FALSE);
	CheckDlgButton(IDC_MAP_TERRAIN_OPTION_TERRAIN, TRUE);
	OnCheckOptionWireFrame();
	OnCheckGrid();
	OnCheckGrid2();
	OnCheckCharacterRendering();
	OnCheckOptionObjectRendering();
	OnCheckOptionObjectCollisionRendering();
}

void CMapFilePage::UpdateUI()
{
	const auto mgr = ((CWorldEditorApp *)AfxGetApp())->GetMapManagerAccessor();
	if (!mgr)
		return;

	const auto map = mgr->GetMapOutdoorPtr();
	if (!map)
		return;

	m_lViewRadius = map->GetViewRadius();
	m_fHeightScale = map->GetHeightScale();

	char buf[128];
	SetDlgItemText(IDC_MAP_SCRIPT_NAME_PRINT, map->GetName().c_str());

	sprintf(buf, "%.2fm", (float) (m_lViewRadius) * (float)(CTerrainImpl::CELLSCALE) / 100.0f);
	SetDlgItemText(IDC_STATIC_VIEW_RADIUS, buf);

	sprintf(buf, "%.2fm", (float)(CTerrainImpl::CELLSCALE) / 100.0f);
	SetDlgItemText(IDC_STATIC_WORLD_SCALE, buf);

	sprintf(buf, "%.2fm", m_fHeightScale * 65535.0f / 100.0f);
	SetDlgItemText(IDC_STATIC_MAX_HEIGHT, buf);

	std::string path;
	GetOnlyFileName(map->GetEnvironmentPath(map->GetSelectedEnvironment()),
	                path);

	SetDlgItemText(IDC_STATIC_ENVIRONMENT_SET, path.c_str());
}

void CMapFilePage::RunLoadMapEvent()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CMapManagerAccessor * pMapManagerAccessor = pDocument->GetMapManagerAccessor();

	m_pPortalDialog->SetMapManagerHandler(pMapManagerAccessor);
}

/////////////////////////////////////////////////////////////////////////////
// CMapFilePage message handlers

void CMapFilePage::OnNewMap()
{
	CNewMapDlg dlg(AfxGetMainWnd());
	if(IDOK != dlg.DoModal())
		return;

	CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
}

/*
* OnLoadMap 변경
* 변경사항
* 1. 로드시 무조건 g_szProgramWindowPath(D:\ymir work\bin) 로 열지 않고
*    최근에 열었던 폴더 경로로 연다.
* 2. pSceneMap->DetachObject(), pMapManagerAccessor->CancelSelect() 추가
*	 로드시 마우스에 Attach 되어있는 오브젝트 정보를 지운다.
*    ReferenceCount 이상문제때문에 추가.
*
* 변경이유
* 디자이너분들은 리소스 수정을 많이 하신다.
* gr2 파일 수정하고 나면 맵툴을 껐다 켜야하는 불편함이 있다.
* 이 과정이 잦게 일어난다.
* 맵 Load 시 리소스를 Reload 하게끔 수정
*/

void CMapFilePage::OnLoadMap()
{
	char szSavingFolder[256 + 32];
	if (!XBrowseForFolder(GetSafeHwnd(), m_initialFolder.c_str(), szSavingFolder, 256 + 32))
		return;

	AfxGetApp()->WriteProfileString("MRU", "MapFolder", szSavingFolder);
	m_initialFolder = szSavingFolder;

	char* pszDir = strrchr(szSavingFolder, '\\');
	if (!pszDir)
		return;

	pszDir++;

	CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	CSceneMap* pSceneMap = pApplication->GetSceneMap();

	if(!pSceneMap)
		return;

	if(!pMapManagerAccessor)
		return;
	
	pSceneMap->DetachObject();
	pMapManagerAccessor->CancelSelect();

	if (!pMapManagerAccessor->LoadMap(pszDir))
		return;

	CMainFrame * pMainFrame = (CMainFrame *) AfxGetMainWnd();
	CWorldEditorView * pView = (CWorldEditorView *) pMainFrame->GetActiveView();
	pView->UpdateTargetPosition(0.0f, 0.0f);
	pMainFrame->RunLoadMapEvent();

	UpdateUI();
}

void CMapFilePage::OnSaveMap()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	pMapManagerAccessor->SaveMap();
}

void CMapFilePage::OnSaveAsMap()
{
	CString cstrMap;

	if (!GetDlgItemText(IDC_SAVEAS_FILENAME, cstrMap))
		return;

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	pMapManagerAccessor->SaveMap(cstrMap);
}

void CMapFilePage::OnSaveServerAttr()
{
	uint32_t dwFlag = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "All Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Save", "", dwFlag, c_szFilter, this);
	if (FileOpener.DoModal()) {
		const auto mgr = ((CWorldEditorApp *)AfxGetApp())->GetMapManagerAccessor();
		if (!mgr)
			return;

		const auto map = mgr->GetMapOutdoorPtr();
		if (!map)
			return;

		map->SaveServerAttr(FileOpener.GetPathName().GetBuffer());
	}
}

void CMapFilePage::OnSaveAttrMap()
{
	int iResult = MessageBox("Are you sure you want to reset all attributes?", NULL, MB_YESNO);
	if (6 != iResult)
		return;

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	pMapManagerAccessor->ResetToDefaultAttr();
}

void CMapFilePage::OnGoto()
{
	CDlgGoto Goto(AfxGetMainWnd());
	if(IDOK != Goto.DoModal())
		return;
}

// Option

void CMapFilePage::OnCheckOptionWireFrame()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	pMapManagerAccessor->SetWireframe(IsDlgButtonChecked(IDC_MAP_TERRAIN_OPTION_WIRE_FRAME) == 1 ? true : false);
}

void CMapFilePage::OnCheckGrid()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetMeterGrid(IsDlgButtonChecked(IDC_CHECK_GRID) != 0);
}

void CMapFilePage::OnCheckGrid2()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetMapBoundGrid(IsDlgButtonChecked(IDC_CHECK_GRID2) != 0);
}

void CMapFilePage::OnCheckCharacterRendering()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetCharacterRendering(IsDlgButtonChecked(IDC_CHECK_CHARACTER) != 0);
}

void CMapFilePage::OnCheckOptionObjectRendering()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetObjectRendering(IsDlgButtonChecked(IDC_MAP_TERRAIN_OPTION_OBJECT) != 0);
}

void CMapFilePage::OnCheckOptionObjectCollisionRendering()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetObjectCollisionRendering(IsDlgButtonChecked(IDC_MAP_TERRAIN_OPTION_OBJECT_COLLISION) != 0);
}

void CMapFilePage::OnCheckTerrainOption()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetTerrainRendering(IsDlgButtonChecked(IDC_MAP_TERRAIN_OPTION_TERRAIN) != 0);
}

// Option

void CMapFilePage::OnMapTerrainOptionObjectShadow()
{
	// TODO: Add your control notification handler code here
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetObjectShadowRendering(IsDlgButtonChecked(IDC_MAP_TERRAIN_OPTION_OBJECT_SHADOW) != 0);
}

void CMapFilePage::OnButtonInitBasetexturemap()
{
	// TODO: Add your control notification handler code here
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor->IsMapReady())
		return;

	CDlgInitBaseTexture InitBaseTexture(AfxGetMainWnd());
	if (IDOK == InitBaseTexture.DoModal()) {
		std::string name = pMapManagerAccessor->GetMapOutdoorRef().GetName();
		pMapManagerAccessor->LoadMap(name);
	}
}

void CMapFilePage::OnCheckPatchGrid()
{
	// TODO: Add your control notification handler code here
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetPatchGrid(IsDlgButtonChecked(IDC_CHECK_PATCH_GRID) != 0);
}

void CMapFilePage::OnCheckWater()
{
	// TODO: Add your control notification handler code here
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetWaterRendering(IsDlgButtonChecked(IDC_CHECK_WATER) != 0);
}

void CMapFilePage::OnCheckCompass()
{
	// TODO: Add your control notification handler code here
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetCompass(IsDlgButtonChecked(IDC_CHECK_COMPASS) != 0);

}

void CMapFilePage::OnButtonSaveAtlas()
{
	// TODO: Add your control notification handler code here
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor->IsMapReady())
		return;

	pMapManagerAccessor->SaveAtlas();
}

/* (수)
*  환경변수지정 버튼을 통하여 환경변수 셋팅시 Data를 로드하게 코드를 추가함.
*  기존에는 UI 쪽에 이름만 변경되었었음.
*  ReloadEnvironmentScript() 추가.
*/
void CMapFilePage::OnButtonSelecteEnvironmentSet()
{
	// TODO: Add your control notification handler code here
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor->IsMapReady())
		return;

	uint32_t dwFlag = OFN_NOCHANGEDIR;
	const char * c_szFilter = "Environment Files (*.msenv)|*.msenv|*.*|All Files (*.*)|";
	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);
	if (TRUE == FileOpener.DoModal())
	{
		//std::string strEnvironmentName;
		//GetOnlyFileName(FileOpener.GetPathName(), strEnvironmentName);
		//pMapManagerAccessor->GetMapOutdoorRef().SetEnvironmentDataName(strEnvironmentName);
		//pMapManagerAccessor->ReloadEnvironmentScript(("d:/ymir work/environment/" + strEnvironmentName).c_str());
		//UpdateUI();
	}
}

void CMapFilePage::OnButtonChangeTerrainHeight()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor->IsMapReady())
	{
		spdlog::error("맵 작성중이 아닙니다");
		return;
	}

	CMapOutdoorAccessor * pMapOutdoor = pMapManagerAccessor->GetMapOutdoorPtr();
	pMapOutdoor->ArrangeTerrainHeight();
}

void CMapFilePage::OnMapTerrainOptionFakePortalEnable()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor->IsMapReady())
	{
		spdlog::error("맵 작성중이 아닙니다");
		CheckDlgButton(IDC_MAP_TERRAIN_OPTION_FAKE_PORTAL_ENABLE, FALSE);
		return;
	}

	pMapManagerAccessor->EnablePortal(IsDlgButtonChecked(IDC_MAP_TERRAIN_OPTION_FAKE_PORTAL_ENABLE) != 0);
	pMapManagerAccessor->RefreshPortal();
}

void CMapFilePage::OnMapTerrainOptionFakePortalIdList()
{
}

void CMapFilePage::OnMapTerrainGuildArea()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetGuildAreaRendering(IsDlgButtonChecked(IDC_MAP_TERRAIN_GUILD_AREA) != 0);
}

/* (수) shadowmap,minimap 전체 갱신 시켜주는 기능 추가
*  현재 insert 키를 통하여 해당영역만 갱신시키게끔 변경하고
*  전체 갱신을 할수 있는 기능을 위해 추가
*/
void CMapFilePage::OnBnClickedButtonAllShadowmapMinimapRefresh()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor->IsMapReady())
	{
		spdlog::error("맵 작성중이 아닙니다");
		return;
	}

	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->SetTerrainShadowMapAndMiniMapUpdateNeeded(true);
	pScene->SetTerrainShadowMapAndMiniMapUpdateAll(true);
}

void CMapFilePage::OnExportHeightmaps()
{
	auto mapManager = ((CWorldEditorApp*)AfxGetApp())->GetMapManagerAccessor();
	if (!mapManager || !mapManager->IsMapReady())
		return;

	mapManager->GetMapOutdoorPtr()->ExportHeightmaps();
}

METIN2_END_NS
