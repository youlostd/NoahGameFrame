#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_FLYCTRLBAR_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_FLYCTRLBAR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SizeCBar.h"
#include "FlyTabPage.h"

METIN2_BEGIN_NS

class CFlyCtrlBar : public CSizingControlBar
{
// Construction
public:
	CFlyCtrlBar();

	BOOL Create(CWnd * pParent);

	void Initialize();

	void UpdatePage();
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlyCtrlBar)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFlyCtrlBar();

	// Generated message map functions
protected:
	CFlyTabPage m_pageTab;
	//{{AFX_MSG(CFlyCtrlBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

METIN2_END_NS

#endif
