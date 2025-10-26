#include "stdafx.h"
#include "..\WorldEditor.h"
#include "texturepropertydlg.h"
#include "../DataCtrl/MapAccessorOutdoor.h"

#include <EterBase/Utils.h>

#include <SpdLog.hpp>

METIN2_BEGIN_NS

#define Prolog() \
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp(); \
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor(); \
\
	CTextureSet * pTextureSet = CTerrain::GetTextureSet(); \
	TTerrainTexture & c_rTerrainTexture = pTextureSet->GetTexture(m_lTexNum); \
\
	UpdateData()

#define PostUpdate(iEditControl) \
	if (iEditControl != 0) \
	{ \
		CEdit * pEdit = (CEdit *) GetDlgItem(iEditControl); \
		int len = pEdit->GetWindowTextLength(); \
		pEdit->SetSel(len, len); \
	} \
\
	UpdateData(FALSE)

/////////////////////////////////////////////////////////////////////////////
// CTexturePropertyDlg dialog
CTexturePropertyDlg::CTexturePropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTexturePropertyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTexturePropertyDlg)
	m_iBegin = 0;
	m_iEnd = 0;
	m_iUOffset = 0;
	m_iUScale = 0;
	m_iVOffset = 0;
	m_iVScale = 0;
	m_fEditBegin = 0.0f;
	m_fEditEnd = 0.0f;
	m_splat = false;
	//}}AFX_DATA_INIT
}


void CTexturePropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTexturePropertyDlg)
	DDX_Control(pDX, IDC_SLIDER_VSCALE, m_SliderVScale);
	DDX_Control(pDX, IDC_SLIDER_VOFFSET, m_SliderVOffset);
	DDX_Control(pDX, IDC_SLIDER_USCALE, m_SliderUScale);
	DDX_Control(pDX, IDC_SLIDER_UOFFSET, m_SliderUOffset);
	DDX_Control(pDX, IDC_SLIDER_END, m_SliderEnd);
	DDX_Control(pDX, IDC_SLIDER_BEGIN, m_SliderBegin);
	DDX_Slider(pDX, IDC_SLIDER_BEGIN, m_iBegin);
	DDX_Slider(pDX, IDC_SLIDER_END, m_iEnd);
	DDX_Slider(pDX, IDC_SLIDER_UOFFSET, m_iUOffset);
	DDX_Slider(pDX, IDC_SLIDER_USCALE, m_iUScale);
	DDX_Slider(pDX, IDC_SLIDER_VOFFSET, m_iVOffset);
	DDX_Slider(pDX, IDC_SLIDER_VSCALE, m_iVScale);
	DDX_Text(pDX, IDC_EDIT_UOFFSET, m_fUOffset);
	DDV_MinMaxFloat(pDX, m_fUOffset, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_USCALE, m_fUScale);
	DDV_MinMaxFloat(pDX, m_fUScale, 0.f, 256.f);
	DDX_Text(pDX, IDC_EDIT_VOFFSET, m_fVOffset);
	DDV_MinMaxFloat(pDX, m_fVOffset, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_VSCALE, m_fVScale);
	DDV_MinMaxFloat(pDX, m_fVScale, 0.f, 256.f);
	DDX_Text(pDX, IDC_EDIT_BEGIN, m_fEditBegin);
	DDV_MinMaxFloat(pDX, m_fEditBegin, 0.f, 1.e+020f);
	DDX_Text(pDX, IDC_EDIT_END, m_fEditEnd);
	DDV_MinMaxFloat(pDX, m_fEditEnd, 0.f, 1.e+020f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTexturePropertyDlg, CDialog)
	//{{AFX_MSG_MAP(CTexturePropertyDlg)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT_BEGIN, OnUpdateEditBegin)
	ON_EN_CHANGE(IDC_EDIT_END, OnUpdateEditEnd)
	ON_EN_CHANGE(IDC_EDIT_UOFFSET, OnUpdateEditUOffset)
	ON_EN_CHANGE(IDC_EDIT_USCALE, OnUpdateEditUScale)
	ON_EN_CHANGE(IDC_EDIT_VOFFSET, OnUpdateEditVOffset)
	ON_EN_CHANGE(IDC_EDIT_VSCALE, OnUpdateEditVScale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTexturePropertyDlg message handlers

/* (금)
*  수치를 변경되면 변경된 수치로 파일에 저장한다.
*/
void CTexturePropertyDlg::OnOK()
{
	// TODO: Add extra validation here
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	CTextureSet * pTextureSet = CTerrain::GetTextureSet();

	auto& tex = pTextureSet->GetTexture(m_lTexNum);
	tex.UScale = m_fUScale;
	tex.VScale = m_fVScale;
	tex.UOffset = m_fUOffset;
	tex.VOffset = m_fVOffset;
	tex.Begin = m_usBegin;
	tex.End =  m_usEnd;
	tex.bSplat = m_splat;

	ResetTextures();

	if (!pMapManagerAccessor->SaveTerrainTextureSet())
		spdlog::error("Failed to save textureset");

	CDialog::OnOK();
}

BOOL CTexturePropertyDlg::OnInitDialog()
{
	if (m_lTexNum == 0)	// 텍스춰가 선택되지 않음
	{
		CDialog::OnCancel();
		return FALSE;
	}

	CDialog::OnInitDialog();

	m_SliderUScale.SetRange(0, 65535);
	m_SliderVScale.SetRange(0, 65535);
	m_SliderUOffset.SetRange(0, 65535);
	m_SliderVOffset.SetRange(0, 65535);
	m_SliderBegin.SetRange(0, 65535);
	m_SliderEnd.SetRange(0, 65535);

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return FALSE;

	CTextureSet * pTextureSet = CTerrain::GetTextureSet();

	const TTerrainTexture & c_rTerrainTexture = pTextureSet->GetTexture(m_lTexNum);

	SetDlgItemText(IDC_STATIC, c_rTerrainTexture.stFilename.c_str());

	m_fUScale	= c_rTerrainTexture.UScale;
	m_fVScale	= c_rTerrainTexture.VScale;
	m_fUOffset	= c_rTerrainTexture.UOffset;
	m_fVOffset	= c_rTerrainTexture.VOffset;
	m_splat	= c_rTerrainTexture.bSplat;
	m_usBegin	= c_rTerrainTexture.Begin;
	m_usEnd		= c_rTerrainTexture.End;

	m_fHeightScale = pMapManagerAccessor->GetMapOutdoorRef().GetHeightScale();

	m_SliderUScale.SetPos((int) m_fUScale * 255);
	m_SliderVScale.SetPos((int) m_fVScale * 255);
	m_SliderUOffset.SetPos((int) m_fUOffset * 65535);
	m_SliderVOffset.SetPos((int) m_fVOffset * 65535);
	m_SliderBegin.SetPos(m_usBegin);
	m_SliderEnd.SetPos(m_usEnd);
	UpdateData();

	m_fEditBegin = (float) (m_usBegin) * m_fHeightScale / 100.0f;
	m_fEditEnd = (float) (m_usEnd) * m_fHeightScale / 100.0f;
	UpdateData(FALSE);

	if (m_splat)
	{
		CheckDlgButton(IDC_CHECK1, 1);
		m_SliderBegin.EnableWindow(true);
		m_SliderEnd.EnableWindow(true);
		GetDlgItem(IDC_EDIT_BEGIN)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_END)->EnableWindow(true);
	}
	else
	{
		CheckDlgButton(IDC_CHECK1, 0);
		m_SliderBegin.EnableWindow(false);
		m_SliderEnd.EnableWindow(false);
		GetDlgItem(IDC_EDIT_BEGIN)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_END)->EnableWindow(false);
	}

	AfxGetMainWnd()->PostMessage(WM_SYSKEYDOWN, 18, 0);
	AfxGetMainWnd()->PostMessage(WM_SYSKEYUP, 18, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTexturePropertyDlg::OnCheck1()
{
	if (IsDlgButtonChecked(IDC_CHECK1))
	{
		m_SliderBegin.EnableWindow(true);
		m_SliderEnd.EnableWindow(true);
		GetDlgItem(IDC_EDIT_BEGIN)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_END)->EnableWindow(true);
		m_splat = true;
	}
	else
	{
		m_SliderBegin.EnableWindow(false);
		m_SliderEnd.EnableWindow(false);
		GetDlgItem(IDC_EDIT_BEGIN)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_END)->EnableWindow(false);
		m_splat = false;
	}
}

void CTexturePropertyDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->GetSafeHwnd() == m_SliderUScale.GetSafeHwnd())
	{
		SetUScale(m_SliderUScale.GetPos());
	}
	else if (pScrollBar->GetSafeHwnd() == m_SliderVScale.GetSafeHwnd())
	{
		SetVScale(m_SliderVScale.GetPos());
	}
	else if (pScrollBar->GetSafeHwnd() == m_SliderUOffset.GetSafeHwnd())
	{
		SetUOffset(m_SliderUOffset.GetPos());
	}
	else if (pScrollBar->GetSafeHwnd() == m_SliderVOffset.GetSafeHwnd())
	{
		SetVOffset(m_SliderVOffset.GetPos());
	}
	else if (pScrollBar->GetSafeHwnd() == m_SliderBegin.GetSafeHwnd())
	{
		SetBegin(m_SliderBegin.GetPos());
	}
	else if (pScrollBar->GetSafeHwnd() == m_SliderEnd.GetSafeHwnd())
	{
		SetEnd(m_SliderEnd.GetPos());
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTexturePropertyDlg::ResetTextures()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	pMapManagerAccessor->ResetTerrainTexture();
}

//
// SetXX
//
void CTexturePropertyDlg::SetUScale(int iUScale)
{
	m_fUScale = (float)iUScale / 256.0f;
	UpdateData(FALSE);
}

void CTexturePropertyDlg::SetVScale(int iVScale)
{
	m_fVScale = (float)iVScale / 256.0f;
	UpdateData(FALSE);
}

void CTexturePropertyDlg::SetUOffset(int iUOffset)
{
	m_fUOffset = (float)iUOffset / 65535.0f;
	UpdateData(FALSE);
}

void CTexturePropertyDlg::SetVOffset(int iVOffset)
{
	m_fVOffset = (float)iVOffset / 65535.0f;
	UpdateData(FALSE);
}

void CTexturePropertyDlg::SetBegin(int iBegin)
{
	m_usBegin = iBegin;
	m_fEditBegin = (float)(m_usBegin) * m_fHeightScale / 100.0f;
	UpdateData(FALSE);
}

void CTexturePropertyDlg::SetEnd(int iEnd)
{
	m_usEnd = iEnd;
	m_fEditEnd = (float) (m_usEnd) * m_fHeightScale / 100.0f;
	UpdateData(FALSE);
}

//
// OnUpdateXX
//
void CTexturePropertyDlg::OnUpdateEditBegin()
{
	m_fEditBegin = fMINMAX(0.0f, m_fEditBegin, 65535.0f * m_fHeightScale / 100.0f);

	UpdateData(FALSE);
	m_usBegin = (unsigned short) (m_fEditBegin / m_fHeightScale * 100);
	m_SliderBegin.SetPos(m_usBegin);

	PostUpdate(IDC_EDIT_BEGIN);
}

void CTexturePropertyDlg::OnUpdateEditEnd()
{
	m_fEditEnd = fMINMAX(0.0f, m_fEditEnd, 65535.0f * m_fHeightScale / 100.0f);

	UpdateData(FALSE);
	m_usEnd = (unsigned short) (m_fEditEnd / m_fHeightScale * 100);
	m_SliderEnd.SetPos(m_usEnd);

	PostUpdate(IDC_EDIT_END);
}

void CTexturePropertyDlg::OnUpdateEditUOffset()
{
	m_fUOffset = fMINMAX(0.0f, m_fUOffset, 1.0f);

	UpdateData(FALSE);
	m_SliderUOffset.SetPos((int) (m_fUOffset * 65535));

	PostUpdate(IDC_EDIT_UOFFSET);
}

void CTexturePropertyDlg::OnUpdateEditUScale()
{
	m_fUScale = fMINMAX(0.0f, m_fUScale, 256.0f);

	UpdateData(FALSE);
	m_SliderUScale.SetPos((int)(m_fUScale * 256));

	PostUpdate(IDC_EDIT_USCALE);
}

void CTexturePropertyDlg::OnUpdateEditVOffset()
{
	m_fVOffset = fMINMAX(0.0f, m_fVOffset, 1.0f);

	UpdateData(FALSE);
	m_SliderVOffset.SetPos((int) (m_fVOffset * 65535));

	PostUpdate(IDC_EDIT_VOFFSET);
}

void CTexturePropertyDlg::OnUpdateEditVScale()
{
	m_fVScale = fMINMAX(0.0f, m_fVScale, 256.0f);

	UpdateData(FALSE);
	m_SliderVScale.SetPos((int) (m_fVScale * 256));

	PostUpdate(IDC_EDIT_VSCALE);
}

METIN2_END_NS
