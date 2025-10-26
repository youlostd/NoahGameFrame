#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELTYPEEQUIPMENTSUBPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELTYPEEQUIPMENTSUBPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CObjectModelTypeEquipmentSubPage : public CDialog
{
// Construction
public:
	CObjectModelTypeEquipmentSubPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent, const CRect & c_rRect);

	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CObjectModelTypeEquipmentSubPage)
	enum { IDD = IDD_OBJECT_MODEL_TYPE_EQUIPMENT_PAGE };
	CComboBox	m_ctrlEquipmentType;
	CListBox	m_ctrlEffectList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectModelTypeEquipmentSubPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectModelTypeEquipmentSubPage)
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnLoadSound();
	afx_msg void OnClearOneEffect();
	afx_msg void OnLoadEffect();
	afx_msg void OnLoadIconImage();
	afx_msg void OnLoadDropModel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
