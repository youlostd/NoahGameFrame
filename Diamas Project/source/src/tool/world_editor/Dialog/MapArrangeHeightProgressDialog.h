#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_MAPARRANGEHEIGHTPROGRESSDIALOG_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_MAPARRANGEHEIGHTPROGRESSDIALOG_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../resource.h"

#include <vstl/string.hpp>

METIN2_BEGIN_NS

/////////////////////////////////////////////////////////////////////////////
// CMapArrangeHeightProgress dialog

class CMapArrangeHeightProgress : public CDialog
{
// Construction
public:
	CMapArrangeHeightProgress(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapArrangeHeightProgress)
	enum { IDD = IDD_MAP_ARRANGE_HEIGHT };
	CStatic	m_ctrlNotify;
	CProgressCtrl	m_ctrlProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapArrangeHeightProgress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Operations
public:
	void Init();
	void SetProgress(int iPos);
	void AddLine(const char * c_szFmt, ...);

// Variables
protected:
	bool m_bStopFlag;
	std::vector<std::string> m_kVec_strLine;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapArrangeHeightProgress)
	afx_msg void OnOK();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
