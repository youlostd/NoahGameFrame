#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_PAGECTRL_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_PAGECTRL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CPageCtrl : public CDialog
{
// Construction
public:
	virtual void UpdateUI() = 0;

	virtual void OnCancel();
	virtual void OnOK();
	void CalcLayout();

	void SetTemporaryHeight(int nTemporaryHeight);

	BOOL EnableWindow(BOOL bEnable = TRUE);

	CPageCtrl(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPageCtrl)
	enum { IDD = 0 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPageCtrl)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPoint m_pt;
	int m_nScrollPos;
	int m_nTemporaryHeight;

	// Generated message map functions
	//{{AFX_MSG(CPageCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
