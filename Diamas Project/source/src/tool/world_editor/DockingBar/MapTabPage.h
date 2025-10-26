#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTABPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTABPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CMapTabPage : public CPageCtrl
{
public:
	enum
	{
		PAGE_TYPE_FILE,
		PAGE_TYPE_TERRAIN,
		PAGE_TYPE_OBJECT,
		PAGE_TYPE_ENVIRONMENT,
		PAGE_TYPE_ATTRIBUTE
	};

// Construction
public:
	CMapTabPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent = NULL);

	void Initialize();

	int GetCurrentTab();

	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CMapTabPage)
	enum { IDD = IDD_MAP_TAB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapTabPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapTabPage)
	afx_msg void OnSelectTab();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CBitmap m_BitmapFile;
	CBitmap m_BitmapTerrain;
	CBitmap m_BitmapObject;
	CBitmap m_BitmapEnvironment;
	CBitmap m_BitmapAttribute;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
