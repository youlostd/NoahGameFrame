#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTDIRECTORY_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTDIRECTORY_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CMapObjectDirectory : public CDialog
{
// Construction
public:
	CMapObjectDirectory(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapObjectDirectory)
	enum { IDD = IDD_MAP_OBJECT_CREATE_DIRECTORY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Operations
public:
	const char * GetDirectoryName();

// Overrides
public:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapObjectDirectory)
	protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapObjectDirectory)
	afx_msg void OnDirectoryAccept();
	afx_msg void OnDirectoryCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	char m_szFileName[64+1];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
