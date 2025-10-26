#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTTIMEBAR_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTTIMEBAR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "EffectTimeBarGraph.h"

METIN2_BEGIN_NS

class CEffectTimeBar : public CDialog
{
// Construction
public:
	CEffectTimeBar(CWnd* pParent = NULL);   // standard constructor
	void CreateButtons();
	void Initialize();
	void Play();

// Operation
public:
	void SetData(uint32_t dwSelectedIndex);
	void SelectGraph(uint32_t dwSelectedIndex, uint32_t dwGraphType);

// Dialog Data
	//{{AFX_DATA(CEffectTimeBar)
	enum { IDD = IDD_EFFECT_TIME_BAR };
	CEffectTimeBarGraph m_EffectTimeGraph;
	CComboBox m_ctrlGraphList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectTimeBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CreateBitmapButton(int iButtonID, int iBitmapID, CBitmap & rBitmap);

	// Generated message map functions
	//{{AFX_MSG(CEffectTimeBar)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPlay();
	afx_msg void OnZoomInHorizon();
	afx_msg void OnZoomOutHorizon();
	afx_msg void OnPlayLoop();
	afx_msg void OnMinimize();
	afx_msg void OnMaximize();
	afx_msg void OnStop();
	afx_msg void OnEffectTimeBarXyz();
	afx_msg void OnEffectTimeBarXyyzzx();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CGraphCtrl m_GraphCtrl;

	uint32_t m_dwSelectedIndex;

	CBitmap m_BitmapPlay;
	CBitmap m_BitmapPlayLoop;
	CBitmap m_BitmapStop;
	CBitmap m_BitmapZoomIn;
	CBitmap m_BitmapZoomOut;
	CBitmap m_BitmapMaximize;
	CBitmap m_BitmapMinimize;
	CBitmap m_BitmapXYZ;
	CBitmap m_BitmapXYYZZX;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
