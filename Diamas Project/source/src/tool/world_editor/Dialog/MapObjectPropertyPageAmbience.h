#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGEAMBIENCE_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGEAMBIENCE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MapObjectPropertyPageBase.h"

METIN2_BEGIN_NS

class CMapObjectProperty;

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageAmbience dialog

class CMapObjectPropertyPageAmbience : public CMapObjectPropertyPageBase
{
// Construction
public:
	CMapObjectPropertyPageAmbience(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CMapObjectProperty * pParent, const CRect & c_rRect);

// Dialog Data
	//{{AFX_DATA(CMapObjectPropertyPageAmbience)
	enum { IDD = IDD_MAP_OBJECT_PROPERTY_PAGE_AMBIENCE };
	CComboBox	m_ctrlPlayType;
	CListBox	m_ctrlSoundFileList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapObjectPropertyPageAmbience)
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

	const char * __GetPlayTypeName();

	// Generated message map functions
	//{{AFX_MSG(CMapObjectPropertyPageAmbience)
	afx_msg void OnNewSoundFile();
	afx_msg void OnDeleteSoundFile();
	afx_msg void OnChangePlayType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CScreen m_Screen;

	TPropertyAmbience m_propertyAmbience;

	CMapObjectProperty * m_pParent;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
