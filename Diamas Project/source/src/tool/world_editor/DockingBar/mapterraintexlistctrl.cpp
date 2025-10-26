#include "stdafx.h"
#include "..\WorldEditor.h"
#include "mapterraintexlistctrl.h"
#include "../MainFrm.h"
#include "../WorldEditorDoc.h"
#include "../DataCtrl/MapAccessorTerrain.h"

#include <EterBase/Utils.h>

METIN2_BEGIN_NS

CMapTerrainTexListCtrl::CMapTerrainTexListCtrl()
{
}

CMapTerrainTexListCtrl::~CMapTerrainTexListCtrl()
{
}


BEGIN_MESSAGE_MAP(CMapTerrainTexListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CMapTerrainTexListCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapTerrainTexListCtrl message handlers

void CMapTerrainTexListCtrl::LoadList()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();

	CTextureSet * pTextureSet = CTerrain::GetTextureSet();

	SetRedraw(FALSE);

	LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvColumn.iSubItem = 0;
	lvColumn.pszText = "Index";
	lvColumn.cx = 40;
	lvColumn.fmt = LVCFMT_RIGHT;
	DeleteColumn(0);
	InsertColumn(0, &lvColumn);

	lvColumn.iSubItem = 1;
	lvColumn.pszText = "Name";
	lvColumn.cx = 110;
	lvColumn.fmt = LVCFMT_LEFT;
	DeleteColumn(1);
	InsertColumn(1, &lvColumn);

	DeleteAllItems();

	LVITEM item;

	for (unsigned long i = 1; i < pTextureSet->GetTextureCount(); ++i)
	{
		item.mask = LVIF_TEXT;

		item.iItem = i;
		item.iSubItem = 0;
		char szNum[32];
		sprintf(szNum, "%d", i);
		item.pszText = (LPSTR) szNum;
// 		item.cchTextMax = strlen(item.pszText);
		InsertItem(&item);

		item.iSubItem = 1;
		std::string strName;
		GetOnlyFileName(pTextureSet->GetTexture(i).stFilename.c_str(), strName);
		item.pszText = (LPSTR)strName.c_str();
// 		item.cchTextMax = strlen(item.pszText);
		SetItem(&item);
	}

/*

	for (i = 1; i < pTextureSet->GetTextureCount(); ++i)
	{
		std::string strName;
		GetOnlyFileName(pTextureSet->GetTexture(i).stFilename.c_str(), strName);
		LPTSTR lpszItem = (char *) (strName.c_str());
		item.iItem = i;
		item.pszText = (char *)lpszItem;
		item.cchTextMax = strlen(item.pszText);
		SetItem(&item);
	}
*/

	SetRedraw(TRUE);
}

METIN2_END_NS
