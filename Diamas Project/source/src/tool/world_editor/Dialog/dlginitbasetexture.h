#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_DLGINITBASETEXTURE_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_DLGINITBASETEXTURE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CDlgInitBaseTexture : public CDialog
{
// Construction
public:
	CDlgInitBaseTexture(CWnd* pParent = NULL);   // standard constructor

	//{{AFX_DATA(CDlgInitBaseTexture)
	enum { IDD = IDD_DIALOG_INITBASETEXTURE };
	CListCtrl	m_ListTexture;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgInitBaseTexture)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	void LoadList();
	void InitTexture();

	std::vector<BYTE> m_TextureNumberVector;

	// Generated message map functions
	//{{AFX_MSG(CDlgInitBaseTexture)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedListTexture(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
