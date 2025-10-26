#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELTYPECHARACTERSUBPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELTYPECHARACTERSUBPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CObjectModelTypeCharacterSubPage : public CDialog
{
// Construction
public:
	CObjectModelTypeCharacterSubPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent, const CRect & c_rRect);

	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CObjectModelTypeCharacterSubPage)
	enum { IDD = IDD_OBJECT_MODEL_TYPE_CHARACTER_PAGE };
	CListBox	m_ctrlEffectList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectModelTypeCharacterSubPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectModelTypeCharacterSubPage)
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnLoadHittingSound();
	afx_msg void OnLoadHittingEffect();
	afx_msg void OnHitTest();
	afx_msg void OnClearOneEffect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
