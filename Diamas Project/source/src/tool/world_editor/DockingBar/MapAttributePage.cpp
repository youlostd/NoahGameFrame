#include "stdafx.h"

#include <storm/Util.hpp>

#include "..\WorldEditor.h"
#include "mapattributepage.h"
#include "../DataCtrl/MapAccessorTerrain.h"
#include "MainFrm.h"
#include "WorldEditorDoc.h"

METIN2_BEGIN_NS

CMapAttributePage::CMapAttributePage(CWnd* pParent /*=NULL*/)
	: CPageCtrl(CMapAttributePage::IDD, pParent)
{
}

void CMapAttributePage::DoDataExchange(CDataExchange* pDX)
{
	CPageCtrl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapAttributePage)
	DDX_Control(pDX, IDC_MAP_ATTR_SCALE_PRINT, m_BrushSizeEdit);
	DDX_Control(pDX, IDC_MAP_ATTR_SCALE, m_ctrAttrScale);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMapAttributePage, CPageCtrl)
	//{{AFX_MSG_MAP(CMapAttributePage)
	ON_BN_CLICKED(IDC_CHECK_CELLEEDIT, OnCheckCelleedit)
	ON_BN_CLICKED(IDC_CHECK_ATTRERASER, OnCheckAttreraser)
	ON_BN_CLICKED(IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE, OnMapAttrBrushShapeCircle)
	ON_BN_CLICKED(IDC_MAP_ATTR_BRUSH_SHAPE_SQUARE, OnMapAttrBrushShapeSquare)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_MAP_ATTR0_CHECKBOX, OnMapAttrCheckbox)
	ON_BN_CLICKED(IDC_MAP_ATTR1_CHECKBOX, OnMapAttrCheckbox)
	ON_BN_CLICKED(IDC_MAP_ATTR2_CHECKBOX, OnMapAttrCheckbox)
	ON_BN_CLICKED(IDC_MAP_ATTR3_CHECKBOX, OnMapAttrCheckbox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMapAttributePage::Create(CWnd * pParent)
{
	if (!CPageCtrl::Create(CMapAttributePage::IDD, pParent))
		return FALSE;

	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE), IDB_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE, m_BitmapBrushShapeCircle);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_ATTR_BRUSH_SHAPE_SQUARE), IDB_MAP_TERRAIN_BRUSH_SHAPE_SQUARE, m_BitmapBrushShapeSquare);

	CheckRadioButton(IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE, IDC_MAP_ATTR_BRUSH_SHAPE_SQUARE, IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE);
	CheckRadioButton(IDC_RADIO0, IDC_RADIO7, IDC_RADIO0);

	m_ctrAttrScale.SetRange(0, 120);
	UpdateBrushSize();
	UpdateBrushShape();
	return TRUE;
}

void CMapAttributePage::Initialize()
{
	CheckDlgButton(IDC_CHECK_CELLEEDIT, 0);
}

void CMapAttributePage::UpdateUI()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	if (!pMapManagerAccessor)
		return;

	UpdateBrushSize();
	UpdateBrushMode();
}

/*
*  1. doc->SetEditingMapAttribute() 부분 주석처리
*  2. 추후 Edit 버튼을 없앨 계획이기 때문에 IDC_CHECK_CELLEEDIT 를 이용하지 않고
*     속성 check 값을 이용하여 속성 brush 를 활성화 시키도록 변경함.
*  (수)
*  1. 추후 Monster info 부분은 없앨 예정.
*  2. UpdateBrushShape() 호출 추가.
*     호출이 이루어 지지 않아. 터레인탭을 갔다오면 터레인탭의 브러쉬 모양으로 적용되는 문제가 있었음.
*  3. SetSelectedAttrFlag() 호출 추가.
*     터레인 탭에서 물을 지우고 속성탭으로 와서 속성을 살펴보면
*     갱신이 안되서 속성이 안지워진 것 같이 보인다. 갱신을 위하여 추가.
*/
void CMapAttributePage::UpdateBrushMode()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	if (!pMapManagerAccessor)
		return;

	CMainFrame* mainFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	CWorldEditorDoc* doc = (CWorldEditorDoc*)mainFrame->GetActiveView()->GetDocument();

	UpdateBrushShape();

	const bool flag = GetAttributeFlag();
	pMapManagerAccessor->SetHeightEditing(false);
	pMapManagerAccessor->SetTextureEditing(false);
	pMapManagerAccessor->SetWaterEditing(false);
	pMapManagerAccessor->SetAttrEditing(0 != flag);
	pMapManagerAccessor->SetSelectedAttrFlag(flag);
	pMapManagerAccessor->SetEraseAttr(IsDlgButtonChecked(IDC_CHECK_ATTRERASER));
}

void CMapAttributePage::OnCheckCelleedit()
{
	UpdateBrushMode();
}

void CMapAttributePage::OnCheckAttreraser()
{
	//CheckDlgButton(IDC_CHECK_CELLEEDIT, 1);
	UpdateBrushMode();
}

void CMapAttributePage::OnMapAttrBrushShapeCircle()
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE, IDC_MAP_ATTR_BRUSH_SHAPE_SQUARE, IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE);
	UpdateBrushShape();
}

void CMapAttributePage::OnMapAttrBrushShapeSquare()
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE, IDC_MAP_ATTR_BRUSH_SHAPE_SQUARE, IDC_MAP_ATTR_BRUSH_SHAPE_SQUARE);
	UpdateBrushShape();
}

void CMapAttributePage::UpdateBrushShape()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	if (TRUE == IsDlgButtonChecked(IDC_MAP_ATTR_BRUSH_SHAPE_CIRCLE))
	{
		pMapManagerAccessor->SetBrushShape(CTerrainAccessor::BRUSH_SHAPE_CIRCLE);
		SetDlgItemText(IDC_MAP_ATTR_BRUSH_SHAPE_PRINT, "Circle");
	}
	else if (TRUE == IsDlgButtonChecked(IDC_MAP_ATTR_BRUSH_SHAPE_SQUARE))
	{
		pMapManagerAccessor->SetBrushShape(CTerrainAccessor::BRUSH_SHAPE_SQUARE);
		SetDlgItemText(IDC_MAP_ATTR_BRUSH_SHAPE_PRINT, "Square");
	}
}

void CMapAttributePage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	if (pScrollBar->GetSafeHwnd() == m_ctrAttrScale.GetSafeHwnd())
	{
		pMapManagerAccessor->SetBrushSize(m_ctrAttrScale.GetPos());
		UpdateBrushSize();
	}
}

void CMapAttributePage::UpdateBrushSize()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	if (!pMapManagerAccessor)
		return;

	m_iBrushSize = pMapManagerAccessor->GetBrushSize();

	char szTextLabel[16];
	memset(szTextLabel, 0, sizeof(char)*16);
	sprintf(szTextLabel, "%d", m_iBrushSize);
	m_BrushSizeEdit.SetWindowText(szTextLabel);
	m_ctrAttrScale.SetPos(m_iBrushSize);
}

BYTE CMapAttributePage::GetAttributeFlag()
{
	int bits[] = {
		IsDlgButtonChecked(IDC_MAP_ATTR0_CHECKBOX),
		IsDlgButtonChecked(IDC_MAP_ATTR1_CHECKBOX),
		IsDlgButtonChecked(IDC_MAP_ATTR2_CHECKBOX),
		IsDlgButtonChecked(IDC_MAP_ATTR3_CHECKBOX),
	};

	BYTE flag = 0;
	for (int i = 0; i < STORM_ARRAYSIZE(bits); ++i) {
		if (bits[i])
			flag |= (1 << i);
	}

	return flag;
}

void CMapAttributePage::OnMapAttrCheckbox()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	if (!pMapManagerAccessor)
		return;

	const auto flag = GetAttributeFlag();
	pMapManagerAccessor->SetAttrEditing(0 != flag);
	pMapManagerAccessor->SetSelectedAttrFlag(flag);
}

BOOL CMapAttributePage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return CPageCtrl::OnCommand(wParam, lParam);
}

METIN2_END_NS
