#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPATTRIBUTEPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPATTRIBUTEPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CMapAttributePage : public CPageCtrl
{
// Construction
public:
	CMapAttributePage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent = NULL);

	void Initialize();
	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CMapAttributePage)
	enum { IDD = IDD_MAP_ATTRIBUTE };
	CEdit m_BrushSizeEdit;
	CSliderCtrl m_ctrAttrScale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapAttributePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBitmap m_BitmapBrushShapeCircle;
	CBitmap m_BitmapBrushShapeSquare;

	char szTextLabel[16];
	int m_iBrushSize;

	void UpdateBrushMode();
	void UpdateBrushShape();
	void UpdateBrushSize();
	BYTE GetAttributeFlag();

	// Generated message map functions
	//{{AFX_MSG(CMapAttributePage)
	afx_msg void OnCheckCellselect();
	afx_msg void OnCheckCelleedit();
	afx_msg void OnCheckAttreraser();
	afx_msg void OnMapAttrBrushShapeCircle();
	afx_msg void OnMapAttrBrushShapeSquare();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMapAttrCheckbox();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
