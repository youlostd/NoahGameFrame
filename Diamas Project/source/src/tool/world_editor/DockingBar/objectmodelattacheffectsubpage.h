#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELATTACHEFFECTSUBPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTMODELATTACHEFFECTSUBPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CObjectModelAttachEffectSubPage : public CDialog
{
// Construction
public:
	CObjectModelAttachEffectSubPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent, const CRect & c_rRect);
	void SetAttachingIndex(uint32_t dwIndex);
	void UpdateUI();

// Dialog Data
	//{{AFX_DATA(CObjectModelAttachEffectSubPage)
	enum { IDD = IDD_OBJECT_MODEL_ATTACH_EFFECT_PAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectModelAttachEffectSubPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectModelAttachEffectSubPage)
	afx_msg void OnEffectPageLoadEffect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


protected:
	uint32_t m_dwAttachingIndex;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
