// ToolBarFile.cpp : implementation file
//

#include "stdafx.h"
#include "../WorldEditor.h"
#include "ToolBarFile.h"

/////////////////////////////////////////////////////////////////////////////
// CToolBarFile

CToolBarFile::CToolBarFile()
{
}

CToolBarFile::~CToolBarFile()
{
}


BEGIN_MESSAGE_MAP(CToolBarFile, CToolBar256)
	//{{AFX_MSG_MAP(CToolBarFile)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolBarFile normal functions

void CToolBarFile::ShowButtonText(BOOL bShow)
{
	if (bShow)
	{
		SetSizes(CSize(43, 34), CSize(16, 15)); // 한글 3문자
		LoadBitmap(IDR_MAINFRAME);
		m_nBitmapID = IDR_MAINFRAME;

		SetButtonText(CommandToIndex(ID_EDIT_UNDO), "Undo");

		SetButtonText(CommandToIndex(ID_VIEW_OBJECT), "Objects");
		SetButtonText(CommandToIndex(ID_VIEW_ENVIRONMENT), "Environment");

		SetButtonText(CommandToIndex(ID_APP_ABOUT), "About");
	}
	else
	{
		SetSizes(CSize(43, 34), CSize(36, 28)); // 한글 3문자
		LoadBitmap(IDB_MAINFRAME_LARGE);
		m_nBitmapID = IDB_MAINFRAME_LARGE;
	}
}

BOOL CToolBarFile::CN_UPDATE_HANDLER(UINT nID, CCmdUI *pCmdUI)
{
	switch (nID)
	{
	case ID_EDIT_UNDO:
		pCmdUI->Enable(TRUE);
		return TRUE;
	}

	return FALSE;
}

BOOL CToolBarFile::CN_COMMAND_HANDLER(UINT nID, CCmdUI *pCmdUI)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CToolBarFile message handlers
