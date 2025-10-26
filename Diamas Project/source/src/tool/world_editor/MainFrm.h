#ifndef METIN2_TOOL_WORLDEDITOR_MAINFRM_HPP
#define METIN2_TOOL_WORLDEDITOR_MAINFRM_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Resource.h"
#include "./DockingBar/MapCtrlBar.h"
#include "./DockingBar/ObjectCtrlBar.h"
#include "./DockingBar/EffectCtrlBar.h"
#include "./DockingBar/FlyCtrlBar.h"

#include "./ToolBar/ToolBar256.h"
#include "./ToolBar/ToolBarFile.h"

METIN2_BEGIN_NS

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	void Initialize();

	void UpdateMapControlBar();
	void UpdateObjectControlBar();
	void UpdateAnimationPage();
	void UpdateEffectControlBar();
	void UpdateFlyControlBar();

	void RunLoadMapEvent();

	void PlayEffect();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void UpdateStatusBar(float fx, float fy, float fz);
	void UpdateStatusBarHoldIcon();	// hold 기능을 위해 추가

	CStatusBar			m_wndStatusBar;
	CToolBarFile		m_wndFileToolBar;

	CMapCtrlBar			m_wndMapCtrlBar;
	CObjectCtrlBar		m_wndObjectCtrlBar;
	CEffectCtrlBar		m_wndEffectCtrlBar;
	CFlyCtrlBar			m_wndFlyCtrlBar;

// Generated message map functions
protected:
	BOOL CreateStatusBar();
	BOOL CreateToolBar();
	BOOL CreateDockingBar();
	BOOL CN_UPDATE_HANDLER(UINT nID, CCmdUI *pCmdUI);
	BOOL CN_COMMAND_HANDLER(UINT nID, CCmdUI *pCmdUI);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUndo();
	afx_msg void OnRedo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void LoadBitmapIcon();

	// Status Bar icon(Hold?)
	HICON m_hHoldIcon[2];
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
