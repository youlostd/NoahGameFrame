#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPCTRLBAR_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPCTRLBAR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SizeCBar.h"
#include "MapTabPage.h"
#include "MapFilePage.h"
#include "MapTerrainPage.h"
#include "MapObjectPage.h"
#include "MapEnvironmentPage.h"
#include "MapAttributePage.h"

METIN2_BEGIN_NS

class CMapCtrlBar : public CSizingControlBar
{
// Construction
public:
	CMapCtrlBar();
	BOOL Create(CWnd * pParent);
	void Initialize();
	void RunLoadMapEvent();

// Attributes
public:
	CMapTabPage m_pageTab;
	CMapFilePage m_pageFile;
	CMapTerrainPage m_pageTerrain;
	CMapObjectPage m_pageObject;
	CMapEnvironmentPage m_pageEnvironment;
	CMapAttributePage m_pageAttribute;

// Operations
public:
	// Map Editing Mode를 리턴한다.
	unsigned char UpdatePage();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapCtrlBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMapCtrlBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapCtrlBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
