#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTBOUNDINGSPHERE_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTBOUNDINGSPHERE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CEffectBoundingSphere : public CDialog
{
// Construction
public:
	CEffectBoundingSphere(CWnd* pParent = NULL);   // standard constructor

// Functions
	void RefreshInfo();
	void SetBoundingSphereData();

// Dialog Data
	//{{AFX_DATA(CEffectBoundingSphere)
	enum { IDD = IDD_EFFECT_BOUNDING_SPHERE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectBoundingSphere)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void OnOK();
	void OnCancel();

	// Generated message map functions
	//{{AFX_MSG(CEffectBoundingSphere)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEffectBoundingSphereRadius();
	afx_msg void OnChangeEffectBoundingSpherePosX();
	afx_msg void OnChangeEffectBoundingSpherePosY();
	afx_msg void OnChangeEffectBoundingSpherePosZ();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	BOOL m_bCallBackEnable;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
