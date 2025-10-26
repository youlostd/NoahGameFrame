#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTCHARACTERSHOW_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_OBJECTANIMATIONEVENTCHARACTERSHOW_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ObjectAnimationEventBase.h"

METIN2_BEGIN_NS

/////////////////////////////////////////////////////////////////////////////
// CObjectAnimationEventCharacterShow dialog

class CObjectAnimationEventCharacterShow : public CObjectAnimationEventBase
{
// Construction
public:
	CObjectAnimationEventCharacterShow(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent, const CRect & c_rRect);

// Dialog Data
	//{{AFX_DATA(CObjectAnimationEventCharacterShow)
	enum { IDD = IDD_OBJECT_ANIMATION_EVENT_CHARACTER_SHOW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectAnimationEventCharacterShow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void Close();
	void GetData(CRaceMotionDataAccessor::TMotionEventData * pData);
	void SetData(const CRaceMotionDataAccessor::TMotionEventData * c_pData);

protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectAnimationEventCharacterShow)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
