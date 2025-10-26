#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTTABPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTTABPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CObjectTabPage : public CPageCtrl
{
public:
	enum
	{
		PAGE_TYPE_MODEL,
		PAGE_TYPE_ANIMATION,
	};

// Construction
public:
	CObjectTabPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent);

	void UpdateUI();

	int GetCurrentTab();

// Dialog Data
	//{{AFX_DATA(CObjectTabPage)
	enum { IDD = IDD_OBJECT_TAB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectTabPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CObjectTabPage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void OnSelectTab();

protected:
	CBitmap m_BitmapModel;
	CBitmap m_BitmapAnimation;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
