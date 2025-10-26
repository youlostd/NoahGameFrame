#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTEFFECT_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTEFFECT_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ObjectAnimationEventBase.h"

#include <vstl/string.hpp>

METIN2_BEGIN_NS

/////////////////////////////////////////////////////////////////////////////
// CObjectAnimationEventEffect dialog

class CObjectAnimationEventEffect : public CObjectAnimationEventBase
{
// Construction
public:
	CObjectAnimationEventEffect(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent, const CRect & c_rRect);

// Dialog Data
	//{{AFX_DATA(CObjectAnimationEventEffect)
	enum { IDD = IDD_OBJECT_ANIMATION_EVENT_EFFECT };
	CComboBox	m_ctrlAttachingBone;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectAnimationEventEffect)
	protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL canClose();
	void Close();

	void GetData(CRaceMotionDataAccessor::TMotionEventData * pData);
	void SetData(const CRaceMotionDataAccessor::TMotionEventData * c_pData);

protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectAnimationEventEffect)
	afx_msg void OnLoadEffect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CString m_strEffectFileName;

	BOOL m_isAttachingEnable;
	BOOL m_isFollowingEnable;
	BOOL m_isIndependentEnable;
	D3DXVECTOR3 m_v3EffectPosition;
	std::string m_strAttachingBoneName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
