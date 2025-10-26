#include "stdafx.h"
#include "..\WorldEditor.h"
#include "NewMapDlg.h"

METIN2_BEGIN_NS

CNewMapDlg::CNewMapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialog)
	//}}AFX_DATA_INIT
}

void CNewMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTexturePropertyDlg)
	DDX_Text(pDX, IDC_EDIT_NEWMAP_NAME, m_StringName);
	DDX_Text(pDX, IDC_TEXTURESET_EDIT, m_textureSet);
	DDX_Text(pDX, IDC_EDIT_NEWMAP_SIZEX, m_uiSizeX);
	DDV_MinMaxUInt(pDX, m_uiSizeX, 0, 1000);
	DDX_Text(pDX, IDC_EDIT_NEWMAP_SIZEY, m_uiSizeY);
	DDV_MinMaxUInt(pDX, m_uiSizeY, 0, 1000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMapDlg, CDialog)
	//{{AFX_MSG_MAP(CNewMapDlg)
//	ON_EN_CHANGE(IDC_EDIT_NEWMAP_NAME, OnUpdateName)
//	ON_EN_CHANGE(IDC_EDIT_NEWMAP_SIZEX, OnUpdateSizeX)
//	ON_EN_CHANGE(IDC_EDIT_NEWMAP_SIZEY, OnUpdateSizeY)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewMapDlg message handlers

BOOL CNewMapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_StringName	= "";
	m_uiSizeX		= 0;
	m_uiSizeY		= 0;

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CNewMapDlg::OnOK()
{
	// TODO: Add extra validation here
	UpdateData();
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapData = pApplication->GetMapManagerAccessor();

	if ("" == m_StringName || 0 == m_uiSizeX || 0 == m_uiSizeY)
		return;

	pMapData->NewMap(m_StringName.GetBuffer(0), m_uiSizeX, m_uiSizeY,
	                 m_textureSet.GetBuffer(0));
	CDialog::OnOK();
}

void CNewMapDlg::OnUpdateName()
{
	UpdateData();
}

void CNewMapDlg::OnUpdateSizeX()
{
	UpdateData();
}

void CNewMapDlg::OnUpdateSizeY()
{
	UpdateData();
}

METIN2_END_NS
