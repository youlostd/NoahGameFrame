#include "stdafx.h"
#include "MainFrm.h"
#include "WorldEditorDoc.h"
#include "WorldEditorView.h"
#include "WorldEditor.h"

#include <MilesLib/SoundVfs.hpp>

#include <EterLib/Camera.h>
#include <EterLib/Engine.hpp>

#include <EterBase/Utils.h>

#include <base/Console.hpp>


#include <SpeedTreeRT.h>

#include <direct.h>

// TODO(tim): remove once timeGetTime calls are gone
#include <mmsystem.h>
#include <SpdLog.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

METIN2_BEGIN_NS

float CCamera::CAMERA_MIN_DISTANCE = 200.0f;
float CCamera::CAMERA_MAX_DISTANCE = 2500.0f;

//#define USE_PACK

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp

BEGIN_MESSAGE_MAP(CWorldEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CWorldEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp construction

InitializationContext::InitializationContext()
	: m_allocator()
{
	storm::SetDefaultAllocator(&m_allocator);
}

CWorldEditorApp::CWorldEditorApp()
	: m_nextProcessTimestamp(0)
{
	timeBeginPeriod(1);

	OpenLogs();
	
	SetPak(&m_vfs);
	SetFontManager(&m_fontManager);

	m_config.LoadConfig();
}

CWorldEditorApp::~CWorldEditorApp()
{
	SetFontManager(nullptr);
	SetPak(nullptr);
	timeEndPeriod(1);
}

void CWorldEditorApp::OpenLogs()
{
	spdlog::init_thread_pool(8192, 1);
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
	auto rotating_syserr_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("syserr.txt", 1024 * 1024 * 10, 10);
	auto rotating_syslog_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("syslog.txt", 1024 * 1024 * 10, 10);
	std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_syserr_sink, rotating_syslog_sink };
	auto logger = std::make_shared<spdlog::async_logger>("wmap", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
			spdlog::register_logger(logger);


#define FULL_LOG
#if defined(FULL_LOG)
	logger->set_level(spdlog::level::trace);
	sinks[0]->set_level(spdlog::level::trace); // console. Allow everything.  Default value
	sinks[1]->set_level(spdlog::level::err); //  syserr
	sinks[2]->set_level(spdlog::level::trace); //  log
#else
		logger->set_level(spdlog::level::info);
		sinks[0]->set_level(spdlog::level::trace); // console. Allow everything.  Default value
		sinks[1]->set_level(spdlog::level::err); //  syserr
		sinks[2]->set_level(spdlog::level::info); //  syserr		
#endif
	logger->flush_on(spdlog::level::err); 
		spdlog::set_default_logger(logger);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWorldEditorApp object

CWorldEditorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp initialization

BOOL CWorldEditorApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif


	CreateConsoleWindow();

	SetRegistryKey(_T("WYG"));

	// Register document templates
	CSingleDocTemplate * pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(IDR_MAINFRAME,
	                                      RUNTIME_CLASS(CWorldEditorDoc),
	                                      RUNTIME_CLASS(CMainFrame),       // main SDI frame window
	                                      RUNTIME_CLASS(CWorldEditorView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	RegisterMilesVfs(); // Miles 파일 콜백을 셋팅해 줘야 한다.
	//CPropertyManager::Instance().Initialize();

	_getcwd(g_szProgramPath, PROGRAM_PATH_LENGTH);
	_getcwd(g_szProgramWindowPath, PROGRAM_PATH_LENGTH);
	StringPath(g_szProgramPath);

	m_GraphicDevice.Create(m_pMainWnd->GetSafeHwnd(), 1024, 768);

	// Initialize
	CMainFrame * pFrame = (CMainFrame *) m_pMainWnd;
	pFrame->Initialize();

	m_SoundManager.Create();

	m_LightManager.Initialize();
	//// Specialize Code End
	////////////////////////////////////////////////////////////////////
	/*  SetWindowText 추가
	*  현재 너무 많은 종류의 맵툴이 존재하여
	*  구분하기 위하여 추가하였음.
	*/
	m_pMainWnd->SetWindowTextA("World Editor - wmap");
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	/////
	CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();
	CRect Rect;
	pView->GetClientRect(&Rect);
	pApplication->GetGraphicDevice().ResizeBackBuffer(Rect.Width(), Rect.Height());

	return TRUE;
}

int CWorldEditorApp::ExitInstance()
{
	m_GraphicDevice.Destroy();
	m_EffectManager.Destroy();

	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CWorldEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp normal functions

CObjectData * CWorldEditorApp::GetObjectData()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();

	if (!pDocument)
		return NULL;

	return pDocument->GetObjectData();
}

CEffectAccessor * CWorldEditorApp::GetEffectAccessor()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();

	if (!pDocument)
		return NULL;

	return pDocument->GetEffectAccessor();
}

CMapManagerAccessor * CWorldEditorApp::GetMapManagerAccessor()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();

	// Temporary
	if (!pFrame->IsWindowVisible())
		return NULL;
	// Temporary

	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();

	if (!pDocument)
		return NULL;

	return pDocument->GetMapManagerAccessor();
}

CSceneObject * CWorldEditorApp::GetSceneObject()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();

	if (!pDocument)
		return NULL;

	return pDocument->GetSceneObject();
}

CSceneEffect * CWorldEditorApp::GetSceneEffect()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();

	if (!pDocument)
		return NULL;

	return pDocument->GetSceneEffect();
}

CSceneMap * CWorldEditorApp::GetSceneMap()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();

	if (!pDocument)
		return NULL;

	return pDocument->GetSceneMap();
}

CSceneFly * CWorldEditorApp::GetSceneFly()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();

	if (!pDocument)
		return NULL;

	return pDocument->GetSceneFly();
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp message handlers

BOOL CWorldEditorApp::OnIdle(LONG lCount)
{
	uint32_t currentTime = timeGetTime();
	if (currentTime >= m_nextProcessTimestamp) {
		CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
		CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();

		pView->Process();

		// [tim] 40 Hz should be enough
		m_nextProcessTimestamp = currentTime + (1000 / 40);
	}

	return CWinApp::OnIdle(lCount);
}

METIN2_END_NS
