#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTY_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG__HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MapObjectPropertyPageTree.h"
#include "MapObjectPropertyPageBuilding.h"
#include "MapObjectPropertyPageEffect.h"
#include "MapObjectPropertyPageAmbience.h"
#include "MapObjectPropertyPageDungeonBlock.h"

#include <vstl/string.hpp>

METIN2_BEGIN_NS

class CMapObjectProperty : public CDialog
{
public:
	enum
	{
		OBJECTIVE_CREATE,
		OBJECTIVE_EDIT,
	};

// Construction
public:
	CMapObjectProperty(CWnd* pParent = NULL);   // standard constructor

public:
	void SetPath(const char * c_szPathName);
	void SetData(CProperty * pProperty);

	int GetPropertyType();
	const char * GetPropertyName();
	uint32_t GetPropertyCRC32();

	void OnOk();
	void OnCancel();

// Dialog Data
	//{{AFX_DATA(CMapObjectProperty)
	enum { IDD = IDD_MAP_OBJECT_PROPERTY };
	CComboBox	m_ctrlPropertyTypeList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapObjectProperty)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void SetupProperty();

	// Generated message map functions
	//{{AFX_MSG(CMapObjectProperty)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnChangePropertyType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CProperty * m_pProperty;
	uint32_t m_dwPropertyCRC32;

	std::string m_strPath;
	std::string m_strPropertyName;

	int m_iPropertyType;
	CMapObjectPropertyPageBase * m_pActivePage;
	CMapObjectPropertyPageBase * m_pPropertyPage[PROPERTY_TYPE_MAX_NUM];
	CMapObjectPropertyPageTree m_pageTree;
	CMapObjectPropertyPageBuilding m_pageBuilding;
	CMapObjectPropertyPageEffect m_pageEffect;
	CMapObjectPropertyPageAmbience m_pageAmbience;
	CMapObjectPropertyPageDungeonBlock m_pageDungeonBlock;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
