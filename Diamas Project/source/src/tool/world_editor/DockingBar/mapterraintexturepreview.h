#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTERRAINTEXTUREPREVIEW_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTERRAINTEXTUREPREVIEW_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "EterLib/GrpDIB.h"

METIN2_BEGIN_NS

class CMapTerrainTexturePreview : public CWnd, public CScreen
{
public:
	CMapTerrainTexturePreview();           // protected constructor used by dynamic creation

// Operations
public:
	BOOL Create();

protected:
	ILuint m_ilImage;
	CGraphicDib m_DIB;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapTerrainTexturePreview)
	//}}AFX_VIRTUAL

// Implementation
public:
	void UpdatePreview(int nTexNum);
	virtual ~CMapTerrainTexturePreview();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapTerrainTexturePreview)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
