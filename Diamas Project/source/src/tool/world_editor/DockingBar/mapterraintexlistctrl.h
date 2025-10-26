#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTERRAINTEXTLISTCTRL_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTERRAINTEXTLISTCTRL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CMapTerrainTexturePreview;

class CMapTerrainTexListCtrl : public CListCtrl
{
// Construction
public:
	CMapTerrainTexListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapTerrainTexListCtrl)
	//}}AFX_VIRTUAL

// Implementation
	void LoadList();
public:
	virtual ~CMapTerrainTexListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapTerrainTexListCtrl)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
