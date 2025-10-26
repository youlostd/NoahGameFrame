#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_COLORANIMATIONDLG_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_COLORANIMATIONDLG_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "EffectParticleTimeEventGraph.h"

METIN2_BEGIN_NS

class CColorAnimationDlg : public CDialog
{
public:
	CColorAnimationDlg(CWnd* pParent = NULL);

	static void CallBack();

	void Update();
	virtual BOOL Create(CWnd* pParent);

	void SetRGBAAccessor(
		CTimeEventDoubleAccessor* pRedAccessor,
		CTimeEventDoubleAccessor* pGreenAccessor,
		CTimeEventDoubleAccessor* pBlueAccessor,
		CTimeEventDoubleAccessor* pAlphaAccessor
	);

	//{{AFX_DATA(CColorAnimationDlg)
	enum { IDD = IDD_COLOR_ANIMATION_DIALOG };
	CSliderCtrl	m_ctrlAlpha;
	CColorBox	m_ctrlColor;
	CStatic	m_ctrlRGBBar;
	CStatic	m_ctrlAlphaBar;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorAnimationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

protected:
	CTimeEventDoubleAccessor * m_pRedAccessor;
	CTimeEventDoubleAccessor * m_pGreenAccessor;
	CTimeEventDoubleAccessor * m_pBlueAccessor;
	CTimeEventDoubleAccessor * m_pAlphaAccessor;

	int m_iLeftLimit;
	int m_iRightLimit;

	int m_iSliderIndex;

	uint32_t m_dwSelectedColor;
	uint32_t m_dwSelectedAlpha;

	enum
	{
		SELECT_NONE,
		SELECT_COLOR,
		SELECT_ALPHA,
	};

	int m_iCurrentSelection;

	void AlignRGB();

	static CColorAnimationDlg * ms_pThis;

	// Generated message map functions
	//{{AFX_MSG(CColorAnimationDlg)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
