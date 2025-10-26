#include "stdafx.h"
#include "../WorldEditor.h"
#include "MapTerrainPage.h"
#include "texturepropertydlg.h"
#include "../MainFrm.h"
#include "../WorldEditorDoc.h"
#include "../WorldEditorView.h"
#include "../DataCtrl/MapAccessorTerrain.h"

#include <EterBase/Utils.h>

#include <SpdLog.hpp>

METIN2_BEGIN_NS

CMapTerrainPage::CMapTerrainPage(CWnd* pParent /*=NULL*/)
: CPageCtrl(CMapTerrainPage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapTerrainPage)
	m_iBrushSize	= 1;
	m_iStrengthSize = 1;
	m_iWaterheight = 0;
	//}}AFX_DATA_INIT
}


void CMapTerrainPage::DoDataExchange(CDataExchange* pDX)
{
	CPageCtrl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapTerrainPage)
	DDX_Control(pDX, IDC_LIST_TERRAINTEXTURE, m_ctrTerrainTexList);
	DDX_Control(pDX, IDC_MAP_TERRAIN_SCALE_WATERHEIGHT, m_SliderWaterHeight);
	DDX_Control(pDX, IDC_MAP_TERRAIN_WATERHEIGHT_PRINT, m_EditWaterHeight);
	DDX_Control(pDX, IDC_PREVIEW, m_ctrTerrainTexturePreview);
	DDX_Control(pDX, IDC_MAP_TERRAIN_STRENGTH_PRINT, m_StrengthSizeEdit);
	DDX_Control(pDX, IDC_MAP_TERRAIN_SCALE_PRINT, m_BrushSizeEdit);
	DDX_Control(pDX, IDC_MAP_TERRAIN_SCALE2, m_ctrStrengthScale);
	DDX_Control(pDX, IDC_MAP_TERRAIN_SCALE, m_ctrBrushScale);
	DDX_Slider(pDX, IDC_MAP_TERRAIN_SCALE, m_iBrushSize);
	DDX_Slider(pDX, IDC_MAP_TERRAIN_SCALE2, m_iStrengthSize);
	DDX_Slider(pDX, IDC_MAP_TERRAIN_SCALE_WATERHEIGHT, m_iWaterheight);
	//}}AFX_DATA_MAP

	// NOTE
	//	DDX_Text(pDX, IDC_EDIT_MOVETO_X, m_usMoveToX);
	//	DDV_MinMaxUInt(pDX, m_usMoveToX, 0, 999);
	//	DDX_Text(pDX, IDC_EDIT_MOVETO_Y, m_usMoveToY);
	//	DDV_MinMaxUInt(pDX, m_usMoveToY, 0, 999);

	m_ctrTerrainTexturePreview.Create();
	DDX_Control(pDX, IDC_MAP_TERRAIN_WATERHEIGHT_PRINT_CUR_CELL, m_CurCellWaterHeight);
}


BEGIN_MESSAGE_MAP(CMapTerrainPage, CPageCtrl)
//{{AFX_MSG_MAP(CMapTerrainPage)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE, OnSelectBrushShapeCircle)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_SHAPE_SQUARE, OnSelectBrushShapeSquare)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_UP, OnSelectBrushTypeUp)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_DOWN, OnSelectBrushTypeDown)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_PLATEAU, OnSelectBrushTypePlateau)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_NOISE, OnSelectBrushTypeNoise)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_SMOOTH, OnSelectBrushTypeSmooth)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_CHECK_HEIGHTBRUSH, OnSelectHeightBrush)
ON_BN_CLICKED(IDC_CHECK_TEXTUREBRUSH, OnSelectTextureBrush)
ON_NOTIFY(NM_CLICK, IDC_LIST_TERRAINTEXTURE, OnClickListTerraintexture)
ON_NOTIFY(NM_DBLCLK, IDC_LIST_TERRAINTEXTURE, OnDblclkListTerraintexture)
ON_BN_CLICKED(IDC_BUTTON_ADDTEXTURE, OnButtonAddtexture)
ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_TERRAINTEXTURE, OnKeydownListTerraintexture)
ON_BN_CLICKED(IDC_CHECK_ERASER, OnCheckEraser)
ON_BN_CLICKED(IDC_CHECK_WATERBRUSH, OnCheckWaterbrush)
ON_BN_CLICKED(IDC_CHECK_ERASERWATER, OnCheckEraserwater)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TERRAINTEXTURE, OnItemchangedListTerraintexture)
ON_BN_CLICKED(IDC_MAP_TERRAIN_BRUSH_SMOOTH, OnSelectBrushTypeSmooth)
	ON_BN_CLICKED(IDC_CHECK_DRAW_ON_BLANK_TILE_ONLY, OnCheckDrawOnBlankTileOnly)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_DELETETEXTURE, &CMapTerrainPage::OnBnClickedButtonDeletetexture)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapEditPage normal functions

BOOL CMapTerrainPage::Create(CWnd * pParent)
{
	if (!CPageCtrl::Create(CMapTerrainPage::IDD, pParent))
		return FALSE;

	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_TERRAIN_BRUSH_UP), IDB_MAP_TERRAIN_BRUSH_UP, m_BitmapUpBrush);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_TERRAIN_BRUSH_DOWN), IDB_MAP_TERRAIN_BRUSH_DOWN, m_BitmapDownBrush);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_TERRAIN_BRUSH_PLATEAU), IDB_MAP_TERRAIN_BRUSH_PLATEAU, m_BitmapPlateauBrush);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_TERRAIN_BRUSH_NOISE), IDB_MAP_TERRAIN_BRUSH_NOISE, m_BitmapNoiseBrush);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_TERRAIN_BRUSH_SMOOTH), IDB_MAP_TERRAIN_BRUSH_SMOOTH, m_BitmapSmoothBrush);

	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE), IDB_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE, m_BitmapBrushShapeCircle);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_MAP_TERRAIN_BRUSH_SHAPE_SQUARE), IDB_MAP_TERRAIN_BRUSH_SHAPE_SQUARE, m_BitmapBrushShapeSquare);

	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_UP, IDC_MAP_TERRAIN_BRUSH_SMOOTH, IDC_MAP_TERRAIN_BRUSH_UP);
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE, IDC_MAP_TERRAIN_BRUSH_SHAPE_SQUARE, IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE);

	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH,	0);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH,	0);
	CheckDlgButton(IDC_CHECK_WATERBRUSH,	0);
	CheckDlgButton(IDC_CHECK_ERASER,		0);
	CheckDlgButton(IDC_CHECK_ERASERWATER,	0);

	m_ctrBrushScale.SetRange(0, 30);
	m_ctrStrengthScale.SetRange(1, 128);
 	m_SliderWaterHeight.SetRange(0, 32768);

	UpdateBrushSize();
	UpdateBrushStrength();
 	UpdateBrushWaterHeight();

//	m_ctrTerrainTexList.InsertColumn(0, "번호", LVCFMT_RIGHT, 40);
	m_ctrTerrainTexList.InsertColumn(0, "Texture path", LVCFMT_LEFT, 150);

	//현재 Cell 의 Water Height 값 출력을 위해 추가
	sprintf(m_szTextLabel, "None");
	m_CurCellWaterHeight.SetWindowText(m_szTextLabel);

	return TRUE;
}

void CMapTerrainPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	if (pScrollBar->GetSafeHwnd() == m_ctrBrushScale.GetSafeHwnd())
	{
		pMapManagerAccessor->SetBrushSize(m_ctrBrushScale.GetPos());
		UpdateBrushSize();
	}
	else if (pScrollBar->GetSafeHwnd() == m_ctrStrengthScale.GetSafeHwnd())
	{
		pMapManagerAccessor->SetBrushStrength(m_ctrStrengthScale.GetPos());
		UpdateBrushStrength();
	}
	else if (pScrollBar->GetSafeHwnd() == m_SliderWaterHeight.GetSafeHwnd())
	{
		//printf("nPos %d %d\n", (WORD) nPos, m_SliderWaterHeight.GetPos());
		pMapManagerAccessor->SetBrushWaterHeight(m_SliderWaterHeight.GetPos() * 2);
		UpdateBrushWaterHeight();
		CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 0);
		CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 0);
		CheckDlgButton(IDC_CHECK_WATERBRUSH, 1);
		UpdateBrushApply();
	}
}

void CMapTerrainPage::UpdateBrushSize()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (pMapManagerAccessor)
		m_iBrushSize = pMapManagerAccessor->GetBrushSize();
	else
		m_iBrushSize = 1;

	sprintf(m_szTextLabel, "%d", m_iBrushSize);
	m_BrushSizeEdit.SetWindowText(m_szTextLabel);
	m_ctrBrushScale.SetPos(m_iBrushSize);
}

void CMapTerrainPage::UpdateBrushStrength()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (pMapManagerAccessor)
		m_iStrengthSize = pMapManagerAccessor->GetBrushStrength();
	else
		m_iStrengthSize = 30;

	sprintf(m_szTextLabel, "%d", m_iStrengthSize);
	m_StrengthSizeEdit.SetWindowText(m_szTextLabel);
	m_ctrStrengthScale.SetPos(m_iStrengthSize);
}

/*  sprintf 문 수정
*  water height 값 표기를 위한 수치값을 소수점 둘째자리까지 표기하기 위해 수정.
*/
void CMapTerrainPage::UpdateBrushWaterHeight()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (pMapManagerAccessor)
		m_iWaterheight = pMapManagerAccessor->GetBrushWaterHeight() / 2;
	else
		m_iWaterheight = 16384;

	float fHeight = (float) (m_iWaterheight - 16384) * 0.01f;

	sprintf(m_szTextLabel, "%.2f", fHeight);
	m_EditWaterHeight.SetWindowText(m_szTextLabel);
	m_SliderWaterHeight.SetPos(m_iWaterheight);
}

void CMapTerrainPage::Initialize()
{
	UpdateUI();
}

void CMapTerrainPage::UpdateUI()
{
	UpdateData();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	pMapManagerAccessor->SetMaxBrushSize(m_ctrBrushScale.GetRangeMax());
	pMapManagerAccessor->SetMaxBrushStrength(m_ctrStrengthScale.GetRangeMax());

	UpdateBrushShape();
	UpdateBrushType();
	UpdateBrushApply();

	UpdateBrushSize();
	UpdateBrushStrength();

	UpdateTextureList();

	UpdateBrushWaterHeight();
}

void CMapTerrainPage::UpdateTextureList()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *) AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

 	LoadList();
	m_ctrTerrainTexturePreview.UpdatePreview(0);
}

void CMapTerrainPage::LoadList()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	CTextureSet * pTextureSet = CTerrain::GetTextureSet();

 	m_ctrTerrainTexList.DeleteAllItems();

	for (unsigned long i = 1; i < pTextureSet->GetTextureCount(); ++i)
	{
		char szNum[128];
		std::string strName;
		GetOnlyFileName(pTextureSet->GetTexture(i).stFilename.c_str(), strName);
		sprintf(szNum, "%3d  %s", i, strName.c_str());
		m_ctrTerrainTexList.InsertItem(LVIF_TEXT, i, szNum, 0, 0, 0, 0);
	}
}

void CMapTerrainPage::UpdateBrushShape()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	if (TRUE == IsDlgButtonChecked(IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE))
	{
		pMapManagerAccessor->SetBrushShape(CTerrainAccessor::BRUSH_SHAPE_CIRCLE);
		SetDlgItemText(IDC_MAP_TERRAIN_BRUSH_SHAPE_PRINT, "Circle");
	}
	else if (TRUE == IsDlgButtonChecked(IDC_MAP_TERRAIN_BRUSH_SHAPE_SQUARE))
	{
		pMapManagerAccessor->SetBrushShape(CTerrainAccessor::BRUSH_SHAPE_SQUARE);
		SetDlgItemText(IDC_MAP_TERRAIN_BRUSH_SHAPE_PRINT, "Square");
	}
}

void CMapTerrainPage::UpdateBrushType()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (pMapManagerAccessor)
	{
		if (TRUE == IsDlgButtonChecked(IDC_MAP_TERRAIN_BRUSH_UP))
		{
			pMapManagerAccessor->SetBrushType(CTerrainAccessor::BRUSH_TYPE_UP);
			SetDlgItemText(IDC_MAP_TERRAIN_BRUSH_TYPE_PRINT, "Up");
		}
		else if (TRUE == IsDlgButtonChecked(IDC_MAP_TERRAIN_BRUSH_DOWN))
		{
			pMapManagerAccessor->SetBrushType(CTerrainAccessor::BRUSH_TYPE_DOWN);
			SetDlgItemText(IDC_MAP_TERRAIN_BRUSH_TYPE_PRINT, "Down");
		}
		else if (TRUE == IsDlgButtonChecked(IDC_MAP_TERRAIN_BRUSH_PLATEAU))
		{
			pMapManagerAccessor->SetBrushType(CTerrainAccessor::BRUSH_TYPE_PLATEAU);
			SetDlgItemText(IDC_MAP_TERRAIN_BRUSH_TYPE_PRINT, "Plateau");
		}
		else if (TRUE == IsDlgButtonChecked(IDC_MAP_TERRAIN_BRUSH_NOISE))
		{
			pMapManagerAccessor->SetBrushType(CTerrainAccessor::BRUSH_TYPE_NOISE);
			SetDlgItemText(IDC_MAP_TERRAIN_BRUSH_TYPE_PRINT, "Noise");
		}
		else if (TRUE == IsDlgButtonChecked(IDC_MAP_TERRAIN_BRUSH_SMOOTH))
		{
			pMapManagerAccessor->SetBrushType(CTerrainAccessor::BRUSH_TYPE_SMOOTH);
			SetDlgItemText(IDC_MAP_TERRAIN_BRUSH_TYPE_PRINT, "Smooth");
		}
	}
}

void CMapTerrainPage::UpdateBrushApply()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	if (TRUE == IsDlgButtonChecked(IDC_CHECK_HEIGHTBRUSH))
	{
		pMapManagerAccessor->SetHeightEditing(true);
		pMapManagerAccessor->SetTextureEditing(false);
		pMapManagerAccessor->SetWaterEditing(false);
 		pMapManagerAccessor->SetAttrEditing(false);
	}
	else if (TRUE == IsDlgButtonChecked(IDC_CHECK_TEXTUREBRUSH))
	{
		pMapManagerAccessor->SetHeightEditing(false);
		pMapManagerAccessor->SetTextureEditing(true);
		pMapManagerAccessor->SetWaterEditing(false);
		pMapManagerAccessor->SetAttrEditing(false);
	}
	else if (TRUE == IsDlgButtonChecked(IDC_CHECK_WATERBRUSH))
	{
		pMapManagerAccessor->SetHeightEditing(false);
		pMapManagerAccessor->SetTextureEditing(false);
		pMapManagerAccessor->SetWaterEditing(true);
		pMapManagerAccessor->SetAttrEditing(false);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMapTerrainPage message handlers

void CMapTerrainPage::OnSelectBrushShapeCircle()
{
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE, IDC_MAP_TERRAIN_BRUSH_SHAPE_SQUARE, IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE);
	UpdateBrushShape();
}

void CMapTerrainPage::OnSelectBrushShapeSquare()
{
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_SHAPE_CIRCLE, IDC_MAP_TERRAIN_BRUSH_SHAPE_SQUARE, IDC_MAP_TERRAIN_BRUSH_SHAPE_SQUARE);
	UpdateBrushShape();
}

void CMapTerrainPage::OnSelectBrushTypeUp()
{
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_UP, IDC_MAP_TERRAIN_BRUSH_SMOOTH, IDC_MAP_TERRAIN_BRUSH_UP);
	UpdateBrushType();
	OnSelectHeightBrush();
}

void CMapTerrainPage::OnSelectBrushTypeDown()
{
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_UP, IDC_MAP_TERRAIN_BRUSH_SMOOTH, IDC_MAP_TERRAIN_BRUSH_DOWN);
	UpdateBrushType();
	OnSelectHeightBrush();
}

void CMapTerrainPage::OnSelectBrushTypePlateau()
{
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_UP, IDC_MAP_TERRAIN_BRUSH_SMOOTH, IDC_MAP_TERRAIN_BRUSH_PLATEAU);
	UpdateBrushType();
	OnSelectHeightBrush();
}

void CMapTerrainPage::OnSelectBrushTypeNoise()
{
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_UP, IDC_MAP_TERRAIN_BRUSH_SMOOTH, IDC_MAP_TERRAIN_BRUSH_NOISE);
	UpdateBrushType();
	OnSelectHeightBrush();
}

void CMapTerrainPage::OnSelectBrushTypeSmooth()
{
	CheckRadioButton(IDC_MAP_TERRAIN_BRUSH_UP, IDC_MAP_TERRAIN_BRUSH_SMOOTH, IDC_MAP_TERRAIN_BRUSH_SMOOTH);
	UpdateBrushType();
	OnSelectHeightBrush();
}

void CMapTerrainPage::OnSelectHeightBrush()
{
	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 1);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 0);
	CheckDlgButton(IDC_CHECK_WATERBRUSH, 0);
	CheckDlgButton(IDC_CHECK_ERASER, 0);
// 	CheckDlgButton(IDC_CHECK_SHADOWBRUSH, 0);

	m_ctrTerrainTexturePreview.UpdatePreview(0);
	UpdateBrushApply();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	pMapManagerAccessor->EditingEnd();
}

void CMapTerrainPage::OnSelectTextureBrush()
{
	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 0);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 1);
	CheckDlgButton(IDC_CHECK_WATERBRUSH, 0);
// 	CheckDlgButton(IDC_CHECK_SHADOWBRUSH, 0);
	UpdateBrushApply();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;
	pMapManagerAccessor->EditingEnd();
}

void CMapTerrainPage::OnClickListTerraintexture(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 0);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 1);
	CheckDlgButton(IDC_CHECK_WATERBRUSH, 0);
// 	CheckDlgButton(IDC_CHECK_SHADOWBRUSH, 0);

// 	CheckDlgButton(IDC_CHECK_ERASER, 0);
	UpdateBrushApply();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	pMapManagerAccessor->EditingEnd();

	NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

	if (pNMListView->iItem >= 0)
		m_ctrTerrainTexturePreview.UpdatePreview(pNMListView->iItem + 1);
	else
		m_ctrTerrainTexturePreview.UpdatePreview(0);

	*pResult = 0;
}

void CMapTerrainPage::OnDblclkListTerraintexture(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_CHECK_ERASER, 0);
	OnCheckEraser();

	NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;

	CTexturePropertyDlg dlg(AfxGetMainWnd());
	dlg.SetTextureNum(pNMListView->iItem + 1);
	dlg.DoModal();

	*pResult = 0;
}


void CMapTerrainPage::OnItemchangedListTerraintexture(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	//NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;
	std::vector<BYTE> TextureNumberVector;

	POSITION p = m_ctrTerrainTexList.GetFirstSelectedItemPosition();

	while (p)
	{
		int nSelected = m_ctrTerrainTexList.GetNextSelectedItem(p);

		if (nSelected < 0)
			break;

		TextureNumberVector.push_back(nSelected + 1);
	}

	pMapManagerAccessor->SetTextureBrushVector(TextureNumberVector);
}

/* (수)
*  이미지 추가 실패면 실패지 왜 프로그램을 꺼트렸을까??
*  1. assert(false); 주석처리 return; 으로 교체
*  2. 텍스처 추가 성공시 TextureSet 저장
*/
void CMapTerrainPage::OnButtonAddtexture()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_CHECK_ERASER, 0);
	OnCheckEraser();

	uint32_t dwFlag = OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	const char * c_szFilter = "All Texture Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (TRUE == FileOpener.DoModal())
	{
		CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
		CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

		std::string path = FileOpener.GetPathName().GetBuffer(0);

		if (!pMapManagerAccessor->AddTerrainTexture(path.c_str()))
			return;

		UpdateTextureList();

		if (!pMapManagerAccessor->SaveTerrainTextureSet())
			spdlog::error("Failed to save textureset");
	}
}

/* (목)
*  텍스쳐 삭제 성공시 TextureSet 저장하도록 추가
*/
void CMapTerrainPage::OnKeydownListTerraintexture(NMHDR* pNMHDR, LRESULT* pResult)
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	CTerrainAccessor * pTerrainAccessor;

	if (!pMapManagerAccessor->GetEditTerrain(&pTerrainAccessor))
		return;

	LV_KEYDOWN * pLVKeyDow = (LV_KEYDOWN *) pNMHDR;

	if (VK_DELETE == pLVKeyDow->wVKey)
	{
		RemoveTerrainTexture();
		/*POSITION p = m_ctrTerrainTexList.GetFirstSelectedItemPosition();

		while (p)
		{
			int nSelected = m_ctrTerrainTexList.GetNextSelectedItem(p);

			if (nSelected < 0)
				break;

			char szWarningText[256+1];

			snprintf(szWarningText, 256, "%s 텍스춰를 목록에서 삭제하시겠습니까?",
					pTerrainAccessor->GetTexture(nSelected + 1).stFilename.c_str());

			if (IDOK == MessageBox(szWarningText, "삭제", MB_OKCANCEL))
			{
				if (!pMapManagerAccessor->RemoveTerrainTexture(nSelected + 1))
					LogBoxf("%s 를 목록에서 삭제하는 데 실패했습니다.",
							pTerrainAccessor->GetTexture(nSelected + 1).stFilename.c_str());

				UpdateTextureList();

				if(!pMapManagerAccessor->SaveTerrainTextureSet())
					LogBox("TextureSet 파일을 저장하는데 실패했습니다.");
			}
		}	*/
	}
	else if( VK_UP == pLVKeyDow->wVKey || VK_DOWN == pLVKeyDow->wVKey )
	{
		std::vector<BYTE> TextureNumberVector;
		POSITION p = m_ctrTerrainTexList.GetFirstSelectedItemPosition();

		while (p)
		{
			int nSelected = m_ctrTerrainTexList.GetNextSelectedItem(p);

			(VK_UP == pLVKeyDow->wVKey)? nSelected -= 1 : nSelected += 1;

			if(nSelected < 0)
				nSelected = 0;

			if(nSelected >= m_ctrTerrainTexList.GetItemCount())
				nSelected = m_ctrTerrainTexList.GetItemCount() - 1;

			/*if (nSelected < 0)
				break;*/

			TextureNumberVector.push_back(nSelected + 1);
		}

		pMapManagerAccessor->SetTextureBrushVector(TextureNumberVector);

		if (!TextureNumberVector.empty())
			m_ctrTerrainTexturePreview.UpdatePreview(TextureNumberVector[0]);
	}
	*pResult = 0;
}

void CMapTerrainPage::OnCheckEraser()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH,	0);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH,	1);
	CheckDlgButton(IDC_CHECK_WATERBRUSH,	0);
	CheckDlgButton(IDC_CHECK_DRAW_ON_BLANK_TILE_ONLY, 0);
// 	CheckDlgButton(IDC_CHECK_SHADOWBRUSH,	0);
	UpdateBrushApply();

	pMapManagerAccessor->EditingEnd();

	pMapManagerAccessor->SetEraseTexture(TRUE == IsDlgButtonChecked(IDC_CHECK_ERASER));
	pMapManagerAccessor->SetDrawOnlyOnBlankTile(TRUE == IsDlgButtonChecked(IDC_CHECK_DRAW_ON_BLANK_TILE_ONLY));

//	if (IsDlgButtonChecked(IDC_CHECK_ERASER)) // 체크드
//	{
//		std::vector<BYTE> emptyVector;
//		pMapManagerAccessor->SetTextureBrushVector(emptyVector); // 선택된 벡터 비우기
//
//		m_ctrTerrainTexturePreview.UpdatePreview(0);
//	}
}

void CMapTerrainPage::OnCheckTerrain0()
{
}

void CMapTerrainPage::OnCheckTerrain1()
{
}

void CMapTerrainPage::OnCheckTerrain2()
{
}

void CMapTerrainPage::OnCheckTerrain3()
{
}

void CMapTerrainPage::OnCheckTerrain4()
{
}

void CMapTerrainPage::OnCheckTerrain5()
{
}

void CMapTerrainPage::OnCheckTerrain6()
{
}

void CMapTerrainPage::OnCheckTerrain7()
{
}

void CMapTerrainPage::OnCheckTerrain8()
{
}

/* (금)
*  Water brush 활성시 물을 출력 시키도록 수정.
*/
void CMapTerrainPage::OnCheckWaterbrush()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 0);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 0);
	CheckDlgButton(IDC_CHECK_WATERBRUSH, 1);

	//변경된 내용 추가
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CWorldEditorDoc * pDocument = (CWorldEditorDoc *)pFrame->GetActiveDocument();
	CSceneMap* pScene = pDocument->GetSceneMap();
	pScene->WaterBrushCheck();

	// 기존에 있던 내용 주석
 	/*CheckDlgButton(IDC_CHECK_SHADOWBRUSH, 0);
	UpdateBrushApply();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;
	pMapManagerAccessor->EditingEnd();*/
}

void CMapTerrainPage::OnCheckEraserwater()
{
	// TODO: Add your control notification handler code here
	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 0);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 0);
	CheckDlgButton(IDC_CHECK_WATERBRUSH, 1);
	CheckDlgButton(IDC_CHECK_DRAW_ON_BLANK_TILE_ONLY, 0);
// 	CheckDlgButton(IDC_CHECK_SHADOWBRUSH, 0);
	UpdateBrushApply();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;
	pMapManagerAccessor->SetEraseWater(TRUE == IsDlgButtonChecked(IDC_CHECK_ERASERWATER));

}

//void CMapTerrainPage::OnCheckShadowbrush()
//{
//	// TODO: Add your control notification handler code here
//	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 0);
//	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 0);
//	CheckDlgButton(IDC_CHECK_WATERBRUSH, 0);
//	CheckDlgButton(IDC_CHECK_SHADOWBRUSH, 1);
//	UpdateBrushApply();
//}

//void CMapTerrainPage::OnCheckShadowBrushEraser()
//{
//	// TODO: Add your control notification handler code here
//	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH, 0);
//	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH, 0);
//	CheckDlgButton(IDC_CHECK_WATERBRUSH, 0);
//	CheckDlgButton(IDC_CHECK_SHADOWBRUSH, 1);
//	UpdateBrushApply();
//
//	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
//	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
//
//	if (!pMapManagerAccessor)
//		return;
//
//	pMapManagerAccessor->SetEraseShadow(TRUE == IsDlgButtonChecked(IDC_CHECK_SHADOWERASER));
//}

void CMapTerrainPage::OnCheckDrawOnBlankTileOnly()
{
	// TODO: Add your control notification handler code here
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	CheckDlgButton(IDC_CHECK_HEIGHTBRUSH,	0);
	CheckDlgButton(IDC_CHECK_TEXTUREBRUSH,	1);
	CheckDlgButton(IDC_CHECK_WATERBRUSH,	0);
	CheckDlgButton(IDC_CHECK_ERASER, 0);
	// 	CheckDlgButton(IDC_CHECK_SHADOWBRUSH,	0);
	UpdateBrushApply();

	pMapManagerAccessor->EditingEnd();

	pMapManagerAccessor->SetEraseTexture(TRUE == IsDlgButtonChecked(IDC_CHECK_ERASER));
	pMapManagerAccessor->SetDrawOnlyOnBlankTile(TRUE == IsDlgButtonChecked(IDC_CHECK_DRAW_ON_BLANK_TILE_ONLY));
}

/* (목)
*  Texture 목록에서 삭제하는 기능 추가
*/
void CMapTerrainPage::OnBnClickedButtonDeletetexture()
{
	// TODO: Add your control notification handler code here
	RemoveTerrainTexture();
}

/* (목)
*  Texture 목록에서 삭제하는 기능 추가
*/
void CMapTerrainPage::RemoveTerrainTexture()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	if (!pMapManagerAccessor)
		return;

	CTerrainAccessor * pTerrainAccessor;

	if (!pMapManagerAccessor->GetEditTerrain(&pTerrainAccessor))
		return;

	POSITION p = m_ctrTerrainTexList.GetFirstSelectedItemPosition();

	while (p)
	{
		int nSelected = m_ctrTerrainTexList.GetNextSelectedItem(p);

		if (nSelected < 0)
			break;

		char szWarningText[256+1];

		snprintf(szWarningText, 256, "Are you sure you want to remove the texture (%s) from the list?",
			pTerrainAccessor->GetTexture(nSelected + 1).stFilename.c_str());

		if (IDOK == MessageBox(szWarningText, "Delete", MB_OKCANCEL))
		{
			if (!pMapManagerAccessor->RemoveTerrainTexture(nSelected + 1))
				spdlog::error("Failed to remove texture {0}",
				pTerrainAccessor->GetTexture(nSelected + 1).stFilename);

			UpdateTextureList();

			if(!pMapManagerAccessor->SaveTerrainTextureSet())
				spdlog::error("Failed to save textureset");
		}
	}
}

METIN2_END_NS
