#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTTRANSLATIONDIALOG_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTTRANSLATIONDIALOG_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../DataCtrl/EffectAccessor.h"

METIN2_BEGIN_NS

class CEffectTranslationDialog : public CDialog
{
// Event Reciever
public:
	class IEventReciever
	{
		public:
			virtual void OnEvent() = 0;
	};

// Construction
public:
	CEffectTranslationDialog(IEventReciever * pEventReciever = NULL);   // standard constructor

// Operation
public:
	void GetPosition(float * pfx, float * pfy, float * pfz);
	void SetPosition(float fx, float fy, float fz);

// Dialog Data
	//{{AFX_DATA(CEffectTranslationDialog)
	enum { IDD = IDD_EFFECT_TRANSLATION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectTranslationDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEffectTranslationDialog)
	afx_msg void OnChangePosition();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	IEventReciever * m_pEventReciever;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
