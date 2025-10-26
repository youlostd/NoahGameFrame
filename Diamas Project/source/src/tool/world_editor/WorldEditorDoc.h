#ifndef METIN2_TOOL_WORLDEDITOR_WORLDEDITORDOC_HPP
#define METIN2_TOOL_WORLDEDITOR_WORLDEDITORDOC_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "DataCtrl/UndoBuffer.h"

METIN2_BEGIN_NS

class CWorldEditorDoc : public CDocument
{
	public:
		typedef std::map<int, CSceneBase*> TSceneMap;
		typedef std::list<CSceneBase*> TSceneList;
		typedef TSceneList::iterator TSceneIterator;

	protected: // create from serialization only
		CWorldEditorDoc();
		DECLARE_DYNCREATE(CWorldEditorDoc)

	// Attributes
	public:
		CMapManagerAccessor m_MapManagerAccessor;
		CObjectData m_ObjectData;
		CEffectAccessor m_EffectAccessor;

		CSceneBase * m_pActiveScene;
		CSceneMap m_SceneMap;
		CSceneObject m_SceneObject;
		CSceneEffect m_SceneEffect;
		CSceneFly m_SceneFly;

		CUndoBuffer m_UndoBuffer;

		int m_iActiveMode;
		TSceneMap m_ScenePointerMap;
		TSceneList m_ScenePointerList;

	// Operations
	public:
		void SetActiveMode(int iMode);
		int GetActiveMode();
		CSceneBase * GetActiveScene();

		CMapManagerAccessor*  GetMapManagerAccessor();
		CObjectData * GetObjectData();
		CEffectAccessor * GetEffectAccessor();

		CSceneMap * GetSceneMap();
		CSceneObject * GetSceneObject();
		CSceneEffect * GetSceneEffect();
		CSceneFly *	GetSceneFly();

		CUndoBuffer * GetUndoBuffer();

		BOOL CreateSceneIterator(TSceneIterator & itor);
		BOOL NextSceneIterator(TSceneIterator & itor);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CWorldEditorDoc)
		public:
		virtual BOOL OnNewDocument();
		virtual void Serialize(CArchive& ar);
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CWorldEditorDoc();
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif

	// Generated message map functions
	protected:
		//{{AFX_MSG(CWorldEditorDoc)
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
