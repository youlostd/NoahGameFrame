#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_DLGPERSPECTIVE_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_DLGPERSPECTIVE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CDlgPerspective : public CDialog
{
// Construction
public:
	CDlgPerspective(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgPerspective)
	enum { IDD = IDD_DIALOG_PERSPECTIVE };
	CSliderCtrl	m_ctrlFarDistance;
	CSliderCtrl	m_ctrlNearDistance;
	CSliderCtrl	m_ctrlFOV;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPerspective)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetFOV(float fFOV);
	void SetNearDistance(float fDistance);
	void SetFarDistance(float fDistance);

	float GetFOV();

protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgPerspective)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	float m_fFOV;
	float m_fNearDistance;
	float m_fFarDistance;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
