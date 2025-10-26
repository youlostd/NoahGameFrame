#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGEDUNGEONBLOCK_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGEDUNGEONBLOCK_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MapObjectPropertyPageBase.h"

METIN2_BEGIN_NS

class CMapObjectProperty;

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageDungeonBlock dialog

class CMapObjectPropertyPageDungeonBlock : public CMapObjectPropertyPageBase
{
// Construction
public:
	CMapObjectPropertyPageDungeonBlock(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CMapObjectProperty * pParent, const CRect & c_rRect);

// Dialog Data
	//{{AFX_DATA(CMapObjectPropertyPageDungeonBlock)
	enum { IDD = IDD_MAP_OBJECT_PROPERTY_PAGE_DUNGEON_BLOCK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapObjectPropertyPageDungeonBlock)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:

	void OnUpdateUI(CProperty * pProperty);
	void OnUpdatePropertyData(const char * c_szPropertyName);
	void OnRender(HWND hWnd);
	bool OnSave(const char * c_szPathName, CProperty * pProperty);

	// Generated message map functions
	//{{AFX_MSG(CMapObjectPropertyPageDungeonBlock)
	afx_msg void OnLoadDungeonBlockFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CScreen m_Screen;

	TPropertyDungeonBlock m_propertyDungeonBlock;

	CMapObjectProperty * m_pParent;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
