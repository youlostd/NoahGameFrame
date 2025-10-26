#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTLIGHTPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTLIGHTPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CEffectLightPage : public CPageCtrl
{
// Construction
public:
	CEffectLightPage(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent);
	void Initialize();
	void UpdateUI();

	void SetData(uint32_t dwIndex);

	static void CallBack();

// Dialog Data
	//{{AFX_DATA(CEffectLightPage)
	enum { IDD = IDD_EFFECT_LIGHT };
	CEffectTimeEventGraph	m_ctrlRangeGraph;
	CSliderCtrl	m_ctrlDiffuseAlpha;
	CSliderCtrl	m_ctrlAmbientAlpha;
	CColorBox	m_ctrlAmbient;
	CColorBox	m_ctrlDiffuse;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectLightPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEffectLightPage)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEffectLightLoop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	uint32_t m_dwIndex;
	uint32_t m_iAlphaSliderIndex;
	static CEffectLightPage* ms_pThis;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
