#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTCTRLBAR_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTCTRLBAR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SizeCBar.h"
#include "ObjectTabPage.h"
#include "ObjectModelPage.h"
#include "ObjectAnimationPage.h"

METIN2_BEGIN_NS

class CObjectCtrlBar : public CSizingControlBar
{
// Construction
public:
	CObjectCtrlBar();
	BOOL Create(CWnd * pParent);
	void Initialize();

// Attributes
public:
	CObjectTabPage m_pageTab;
	CObjectModelPage m_pageModel;
	CObjectAnimationPage m_pageAnimation;

// Operations
public:
	void UpdatePage();
	void UpdateAnimationPage();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectCtrlBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CObjectCtrlBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CObjectCtrlBar)
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
