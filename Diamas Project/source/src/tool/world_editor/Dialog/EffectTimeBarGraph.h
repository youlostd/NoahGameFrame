#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTTIMEBARGRAPH_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTTIMEBARGRAPH_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GraphCtrl.h"

METIN2_BEGIN_NS

class CEffectTimeBarGraph : public CStatic
{
// Construction
public:
	CEffectTimeBarGraph();
	void Initialize();

// Attributes
public:

// Operations
public:
	void SetGraphCtrl(CGraphCtrl * m_pGraphCtrl);
	void Resizing(int iWidth, int iHeight);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectTimeBarGraph)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEffectTimeBarGraph();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEffectTimeBarGraph)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	CGraphCtrl * m_pGraphCtrl;

	// Mouse
	bool m_isLButtonDown;
	bool m_isMButtonDown;
	CPoint m_LastPoint;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
