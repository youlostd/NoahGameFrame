#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_INPUTNUMBER_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_INPUTNUMBER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CInputNumber : public CDialog
{
// Construction
public:
	CInputNumber(CWnd* pParent = NULL);   // standard constructor

// Operation
public:
	int GetNumber();

// Dialog Data
	//{{AFX_DATA(CInputNumber)
	enum { IDD = IDD_INPUT_NUMBER };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInputNumber)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInputNumber)
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Variables
protected:
	int m_iNumber;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
