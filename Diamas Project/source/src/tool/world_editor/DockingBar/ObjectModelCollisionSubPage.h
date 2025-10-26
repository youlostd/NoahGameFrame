#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELCOLLISIONSUBPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELCOLLISIONSUBPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CObjectModelPage;

/////////////////////////////////////////////////////////////////////////////
// CObjectModelCollisionSubPage dialog

class CObjectModelCollisionSubPage : public CDialog
{
// Construction
public:
	CObjectModelCollisionSubPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CObjectModelPage * pParent, const CRect & c_rRect);

	void SetAttachingIndex(uint32_t dwIndex);
	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CObjectModelCollisionSubPage)
	enum { IDD = IDD_OBJECT_MODEL_COLLISION_PAGE };
	CComboBox	m_ctrlSphereList;
	CSliderCtrl	m_ctrlSphereSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectModelCollisionSubPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectModelCollisionSubPage)
	afx_msg void OnCheckTypeBody();
	afx_msg void OnCheckTypeDefending();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnInsertCollisionData();
	afx_msg void OnDeleteOneCollisionData();
	afx_msg void OnCollisionSizeType1();
	afx_msg void OnCollisionSizeType2();
	afx_msg void OnCollisionSizeType3();
	afx_msg void OnCollisionSizeType4();
	afx_msg void OnCollisionSizeType5();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL GetCollisionDataPointer(uint32_t dwAttachingIndex, NRaceData::TCollisionData ** ppCollisionData);
	BOOL GetSphereDataPointer(uint32_t dwAttachingIndex, uint32_t dwSphereIndex, TSphereData ** ppSphereData);

	void SetSphereSize(uint32_t dwAttachingIndex, uint32_t dwSphereIndex, int iSize);
	void SetSphereSizeType(uint32_t dwAttachingIndex, uint32_t dwSphereIndex, uint32_t dwSizeType);

	void UpdateSphereData();

protected:
	CObjectModelPage * m_pParent;

	uint32_t m_dwAttachingIndex;
	uint32_t m_dwSphereIndex;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
