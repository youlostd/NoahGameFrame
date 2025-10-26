#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_NEWMAPDLG_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_NEWMAPDLG_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CNewMapDlg : public CDialog
{
public:
	CNewMapDlg(CWnd* pParent = NULL);

	// Dialog Data
	//{{AFX_DATA(CNewMapDlg)
	enum { IDD = IDD_DIALOG_NEWMAP };
	CString m_StringName;
	CString m_textureSet;
	UINT m_uiSizeX;
	UINT m_uiSizeY;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexturePropertyDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTexturePropertyDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnUpdateName();
	afx_msg void OnUpdateSizeX();
	afx_msg void OnUpdateSizeY();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
