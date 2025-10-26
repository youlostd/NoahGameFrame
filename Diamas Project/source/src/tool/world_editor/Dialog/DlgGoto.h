#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_DLGGOTO_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_DLGGOTO_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CDlgGoto : public CDialog
{
public:
	CDlgGoto(CWnd* pParent = NULL);

	//{{AFX_DATA(CDlgGoto)
	enum { IDD = IDD_DIALOG_GOTO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgGoto)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgGoto)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void Goto(float fXGoto, float fYGoto);

public:
	int m_GotoRadio;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
