#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_TEXTUREPROPERTYDLG_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_TEXTUREPROPERTYDLG_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CTexturePropertyDlg : public CDialog
{
// Construction
public:
	CTexturePropertyDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTexturePropertyDlg)
	enum { IDD = IDD_DIALOG_TEXTURE_PROPERTY };
	CSliderCtrl	m_SliderVScale;
	CSliderCtrl	m_SliderVOffset;
	CSliderCtrl	m_SliderUScale;
	CSliderCtrl	m_SliderUOffset;
	CSliderCtrl	m_SliderEnd;
	CSliderCtrl	m_SliderBegin;
	int		m_iBegin;
	int		m_iEnd;
	int		m_iUOffset;
	int		m_iUScale;
	int		m_iVOffset;
	int		m_iVScale;
	float	m_fEditBegin;
	float	m_fEditEnd;
	bool m_splat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexturePropertyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void	SetTextureNum(long lNum)	{ m_lTexNum = lNum; }

protected:
	long			m_lTexNum;

	float			m_fUScale;
	float			m_fVScale;
	float			m_fUOffset;
	float			m_fVOffset;
	bool			m_bSplst;
	unsigned short	m_usBegin;
	unsigned short	m_usEnd;

	float			m_fHeightScale;

	void SetUScale(int iUScale);
	void SetVScale(int iVScale);
	void SetUOffset(int iUOffset);
	void SetVOffset(int iVOffset);
	void SetBegin(int iBegin);
	void SetEnd(int iEnd);
	void ResetTextures();

	// Generated message map functions
	//{{AFX_MSG(CTexturePropertyDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheck1();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUpdateEditBegin();
	afx_msg void OnUpdateEditEnd();
	afx_msg void OnUpdateEditUOffset();
	afx_msg void OnUpdateEditUScale();
	afx_msg void OnUpdateEditVOffset();
	afx_msg void OnUpdateEditVScale();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
