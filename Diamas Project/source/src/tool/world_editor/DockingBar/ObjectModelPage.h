#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR__HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR__HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ObjectModelTypeCharacterSubPage.h"
#include "ObjectModelTypeEquipmentSubPage.h"
#include "ObjectModelCollisionSubPage.h"
#include "ObjectModelAttachObjectSubPage.h"
#include "ObjectModelAttachEffectSubPage.h"

METIN2_BEGIN_NS

class CObjectModelPage : public CPageCtrl
{
// Construction
public:
	CObjectModelPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent);

	void Initialize();
	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CObjectModelPage)
	enum { IDD = IDD_OBJECT_MODEL };
	CComboBox	m_ctrlModelType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectModelPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL CreateModelTypePages();
	BOOL CreateAttachingPages();

	void UpdateAttachedDataComboBox();
	void UpdateBoneComboBox();

	void SelectModelType(uint32_t dwModeIndex);
	void SelectAttachingDataPage(uint32_t dwPageIndex);

	// Generated message map functions
	//{{AFX_MSG(CObjectModelPage)
	afx_msg void OnLoadModel();
	afx_msg void OnSaveModelScript();
	afx_msg void OnLoadModelScript();
	afx_msg void OnMakeCollisionData();
	afx_msg void OnCheckShowingMainCharacterFlag();
	afx_msg void OnClearAllAttachingData();
	afx_msg void OnOnClearOneAttachingData();
	afx_msg void OnCheckAttachingBoneEnable();
	afx_msg void OnCheckShowingCollisionDataFlag();
	afx_msg void OnMakeObject();
	afx_msg void OnObjectModelMakeEffect();
	afx_msg void OnObjectModelSaveAttr();
	afx_msg void OnModelNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CObjectModelTypeCharacterSubPage m_TypeCharacterSubPage;
	CObjectModelTypeEquipmentSubPage m_TypeEquipmentSubPage;
	CObjectModelCollisionSubPage m_AttachCollisionSubPage;
	CObjectModelAttachObjectSubPage m_AttachObjectSubPage;
	CObjectModelAttachEffectSubPage m_AttachEffectSubPage;

	uint32_t m_dwcurAttachingDataIndex;

	CComboBox m_ctrlAttachingData;
	CComboBox m_ctrlAttachingBone;

	std::string m_strLoadedPath;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
