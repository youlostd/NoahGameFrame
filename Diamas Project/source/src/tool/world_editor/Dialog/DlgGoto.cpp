#include "stdafx.h"
#include "..\WorldEditor.h"
#include "DlgGoto.h"
#include "../MainFrm.h"
#include "../WorldEditorDoc.h"
#include "../WorldEditorView.h"
#include "../DataCtrl/MapAccessorOutdoor.h"

#include <eterlib/Camera.h>

METIN2_BEGIN_NS

CDlgGoto::CDlgGoto(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgGoto::IDD, pParent)
	, m_GotoRadio(0)
{
	//{{AFX_DATA_INIT(CDlgGoto)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgGoto::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgGoto)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Radio(pDX, IDC_RADIO1, m_GotoRadio);
}


BEGIN_MESSAGE_MAP(CDlgGoto, CDialog)
	//{{AFX_MSG_MAP(CDlgGoto)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgGoto message handlers


BOOL CDlgGoto::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDialogIntegerText(GetSafeHwnd(), IDC_EDIT_GOTO_POSITION_X, 0);
	SetDialogIntegerText(GetSafeHwnd(), IDC_EDIT_GOTO_POSITION_Y, 0);

	UpdateData(false);

	GetDlgItem(IDC_EDIT_GOTO_POSITION_X)->SetFocus();
	return FALSE;
}

/*
*  좌표이동과 구역이동이 가능하도록 변경함에따른 데이터 타입 변경.
*/
void CDlgGoto::OnOK()
{
	//uint32_t dwxGoto = GetDialogIntegerText(GetSafeHwnd(), IDC_EDIT_GOTO_POSITION_X);
	//uint32_t dwyGoto = GetDialogIntegerText(GetSafeHwnd(), IDC_EDIT_GOTO_POSITION_Y);
	float fXGoto = GetDialogFloatText(GetSafeHwnd(), IDC_EDIT_GOTO_POSITION_X);
	float fYGoto = GetDialogFloatText(GetSafeHwnd(), IDC_EDIT_GOTO_POSITION_Y);

	Goto(fXGoto, fYGoto);
	EndDialog(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgGoto normal functions

/*
*  좌표이동과 구역이동이 가능하도록 변경.
*/
//void CDlgGoto::Goto(uint32_t dwxGoto, uint32_t dwyGoto)
void CDlgGoto::Goto(float fXGoto, float fYGoto)
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	CWorldEditorView * pView = (CWorldEditorView *)pFrame->GetActiveView();
	Vector3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();

	short sTerrainCountX, sTerrainCountY;

	CMapOutdoorAccessor * pOutdoor = (CMapOutdoorAccessor *) &pMapManagerAccessor->GetMapOutdoorRef();
	pOutdoor->GetTerrainCount(&sTerrainCountX, &sTerrainCountY);

	UpdateData(true);
	pMapManagerAccessor->SaveTerrains();
	pMapManagerAccessor->SaveAreas();
	pMapManagerAccessor->SetTerrainModified();


	switch(m_GotoRadio)
	{
	case 0:
		fXGoto *= 100.0f;
		fYGoto *= 100.0f;

		if( fXGoto > (sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE) )
			fXGoto = CTerrainImpl::TERRAIN_XSIZE;
		if( fYGoto > (sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE) )
			fYGoto = CTerrainImpl::TERRAIN_YSIZE;

		pMapManagerAccessor->UpdateMap( fXGoto, - fYGoto, 0.0f );
		pView->UpdateTargetPosition( fXGoto - v3Target.x, -(fYGoto + v3Target.y) );
		break;

	case 1:
		if (fXGoto >= (UINT) sTerrainCountX - 1)
			fXGoto = sTerrainCountX - 1;
		if (fYGoto >= (UINT) sTerrainCountY - 1)
			fYGoto = sTerrainCountY - 1;

		pMapManagerAccessor->UpdateMap( (fXGoto * CTerrainImpl::TERRAIN_XSIZE), - (fYGoto * CTerrainImpl::TERRAIN_YSIZE), 0.0f );
		pView->UpdateTargetPosition( (fXGoto * CTerrainImpl::TERRAIN_XSIZE) - v3Target.x, - (fYGoto * CTerrainImpl::TERRAIN_YSIZE) - v3Target.y);
		break;
	}
}

METIN2_END_NS
