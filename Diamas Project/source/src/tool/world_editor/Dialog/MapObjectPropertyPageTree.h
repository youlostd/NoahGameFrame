#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGETREE_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGETREE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MapObjectPropertyPageBase.h"

METIN2_BEGIN_NS

class CMapObjectProperty;

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageTree dialog

class CMapObjectPropertyPageTree : public CMapObjectPropertyPageBase
{
// Construction
public:
	CMapObjectPropertyPageTree(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CMapObjectProperty * pParent, const CRect & c_rRect);

// Dialog Data
	//{{AFX_DATA(CMapObjectPropertyPageTree)
	enum { IDD = IDD_MAP_OBJECT_PROPERTY_PAGE_TREE };
	CSliderCtrl	m_ctrlTreeSizeVariance;
	CSliderCtrl	m_ctrlTreeSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapObjectPropertyPageTree)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapObjectPropertyPageTree)
	afx_msg void OnLoadTreeFile();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAcceptTree();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnUpdateUI(CProperty * pProperty);
	void OnUpdatePropertyData(const char * c_szPropertyName);
	bool OnSave(const char * c_szPathName, CProperty * pProperty);
	void OnRender(HWND hWnd);

	void UpdateScrollBarState();

	void OnClearTree();

protected:
	CScreen m_Screen;

	TPropertyTree		m_propertyTree;

	CMapObjectProperty *	m_pParent;

	CSpeedTreeWrapper *		m_pTree;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
