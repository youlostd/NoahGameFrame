#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTTEXTUREPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTTEXTUREPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "EffectUI.h"

METIN2_BEGIN_NS

class CEffectTexturePage : public CPageCtrl
{
// Construction
public:
	CEffectTexturePage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent);

	void UpdateUI();

	void SetData(uint32_t dwIndex);

// Dialog Data
	//{{AFX_DATA(CEffectTexturePage)
	enum { IDD = IDD_EFFECT_TEXTURE };
	CBlendTypeComboBox	m_ctrlBlendDestType;
	CBlendTypeComboBox	m_ctrlBlendSrcType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectTexturePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEffectTexturePage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
