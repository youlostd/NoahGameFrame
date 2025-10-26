#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPFILEPAGE_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPFILEPAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vstl/string.hpp>

METIN2_BEGIN_NS

class CMapPortalDialog;

class CMapFilePage : public CPageCtrl
{
// Construction
public:
	CMapFilePage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMapFilePage();
	BOOL Create(CWnd * pParent = NULL);
	void Initialize();
	void UpdateUI();
	void RunLoadMapEvent();

// Dialog Data
	//{{AFX_DATA(CMapFilePage)
	enum { IDD = IDD_MAP_FILE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapFilePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Operation
protected:
	CMapPortalDialog * m_pPortalDialog;

	long m_lViewRadius;
	float m_fHeightScale;
	std::string m_initialFolder;
	std::string strEnvironmentDataPath;

	// Generated message map functions
	//{{AFX_MSG(CMapFilePage)
	afx_msg void OnCheckOptionWireFrame();
	afx_msg void OnCheckGrid();
	afx_msg void OnCheckGrid2();
	afx_msg void OnCheckCharacterRendering();
	afx_msg void OnNewMap();
	afx_msg void OnLoadMap();
	afx_msg void OnSaveMap();
	afx_msg void OnSaveAsMap();
	afx_msg void OnSaveServerAttr();
	afx_msg void OnSaveAttrMap();
	afx_msg void OnGoto();
	afx_msg void OnCheckOptionObjectCollisionRendering();
	afx_msg void OnCheckOptionObjectRendering();
	afx_msg void OnMapTerrainOptionObjectShadow();
	afx_msg void OnButtonInitBasetexturemap();
	afx_msg void OnCheckPatchGrid();
	afx_msg void OnCheckWater();
	afx_msg void OnCheckCompass();
	afx_msg void OnButtonSaveAtlas();
	afx_msg void OnButtonSelecteEnvironmentSet();
	afx_msg void OnCheckTerrainOption();
	afx_msg void OnButtonChangeTerrainHeight();
	afx_msg void OnMapTerrainOptionFakePortalEnable();
	afx_msg void OnMapTerrainOptionFakePortalIdList();
	afx_msg void OnMapTerrainGuildArea();
	afx_msg void OnBnClickedButtonAllShadowmapMinimapRefresh();
	afx_msg void OnExportHeightmaps();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
