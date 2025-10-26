#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENT_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENT_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ObjectAnimationEventScreenWaving.h"
#include "ObjectAnimationEventScreenFlashing.h"
#include "ObjectAnimationEventEffect.h"
#include "ObjectAnimationEventFly.h"
#include "ObjectAnimationEventAttack.h"
#include "ObjectAnimationEventSound.h"
#include "ObjectAnimationEventCharacterShow.h"
#include "ObjectAnimationEventCharacterHide.h"
#include "ObjectAnimationEventWarp.h"
#include "ObjectAnimationEventEffectToTarget.h"

METIN2_BEGIN_NS

/////////////////////////////////////////////////////////////////////////////
// CObjectAnimationEvent dialog

class CObjectAnimationEvent : public CDialog
{
// Construction
public:
	CObjectAnimationEvent(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CObjectAnimationEvent)
	enum { IDD = IDD_OBJECT_ANIMATION_EVENT };
	CComboBox	m_ctrlEventType;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectAnimationEvent)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:

	// Setting Data
	void SetData(const CRaceMotionDataAccessor::TMotionEventData * c_pData);

	// Getting Data
	int GetEventType();
	void GetMotionEventData(CRaceMotionDataAccessor::TMotionEventData * pData);

protected:

	void OnOK();
	void OnCancel();

	void InitializePage();

	void __BuildEventPageVector();
	void __BuildEventTypeList();

	void __SelectEvent(uint32_t dwEventType);

	// Generated message map functions
	//{{AFX_MSG(CObjectAnimationEvent)
	afx_msg void OnAcceptNewEvent();
	afx_msg void OnCancelNewEvent();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEventType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CObjectAnimationEventScreenWaving m_EventPageScreenWaving;
	CObjectAnimationEventScreenFlashing m_EventPageScreenFlashing;
	CObjectAnimationEventEffect m_EventPageEffect;
	CObjectAnimationEventFly m_EventPageFly;
	CObjectAnimationEventAttack m_EventPageAttack;
	CObjectAnimationEventSound m_EventPageSound;
	CObjectAnimationEventCharacterShow m_EventPageCharacterShow;
	CObjectAnimationEventCharacterHide m_EventPageCharacterHide;
	CObjectAnimationEventWarp m_EventPageWarp;
	CObjectAnimationEventEffectToTarget m_EventPageEffectToTarget;
	std::vector<CObjectAnimationEventBase *> m_EventPageVector;

	uint32_t m_dwEventType;
	const CRaceMotionDataAccessor::TMotionEventData * mc_pPreserveData;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
