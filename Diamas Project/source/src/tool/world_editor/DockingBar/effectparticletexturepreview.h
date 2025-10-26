#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTPARTICLETEXTUREPREVIEW_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTPARTICLETEXTUREPREVIEW_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CEffectParticleTexturePreview : public CWnd, private CScreen
{
// Construction
public:
	CEffectParticleTexturePreview();

// Attributes
public:
protected:
	 CGraphicImage::Ptr m_pImage;
	 CGraphicImageInstance m_ImageInstance;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectParticleTexturePreview)
	//}}AFX_VIRTUAL

// Implementation
public:
	void UpdatePreview(CGraphicImage::Ptr pImage);

	virtual ~CEffectParticleTexturePreview();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEffectParticleTexturePreview)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
