#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTWARP_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTWARP_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ObjectAnimationEventBase.h"

METIN2_BEGIN_NS

class CObjectAnimationEventWarp : public CObjectAnimationEventBase
{
// Construction
public:
	CObjectAnimationEventWarp(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent, const CRect & c_rRect);

// Dialog Data
	//{{AFX_DATA(CObjectAnimationEventWarp)
	enum { IDD = IDD_OBJECT_ANIMATION_EVENT_WARP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectAnimationEventWarp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void Close();

	void GetData(CRaceMotionData::TMotionEventData * pMotionEventData);
	void SetData(const CRaceMotionData::TMotionEventData * c_pMotionEventData);

protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectAnimationEventWarp)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
