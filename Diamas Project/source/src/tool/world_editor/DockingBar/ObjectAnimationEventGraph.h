#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTANIMATIONEVENTGRAPH_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_OBJECTANIMATIONEVENTGRAPH_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../Dialog/ObjectAnimationEvent.h"
#include "../DataCtrl/ObjectAnimationAccessor.h"

METIN2_BEGIN_NS

class CObjectAnimationEventGraph : public CStatic
{
// Construction
public:
	CObjectAnimationEventGraph();

// Attributes
public:

// Operations
public:
	void Create();
	void Render();
	void SetMotionData(CRaceMotionDataAccessor * pMotionData);
	void SetCurrentHitDataIndex(int iIndex);
	void ToggleFrameStep();

	void Refresh();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectAnimationEventGraph)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CObjectAnimationEventGraph();

	// Generated message map functions
protected:
	//{{AFX_MSG(CObjectAnimationEventGraph)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	enum ENodeType
	{
		NODE_TYPE_LEFT,
		NODE_TYPE_RIGHT,
	};

	typedef struct SEventNode
	{
		uint32_t dwIndex;
		float fPosition;
		const CRaceMotionData::TMotionEventData * c_pEventData;
	} TEventNode;

	typedef std::list<TEventNode*> TEventNodeList;
	typedef TEventNodeList::iterator TEventNodeListiterator;

	/////

	enum EComboNode
	{
		INPUT_START,
		NEXT_COMBO,
		INPUT_END,

		COMBO_NODE_MAX_NUM,
	};

	enum EAttackingNode
	{
		START,
		END,

		ATTACKING_NODE_MAX_NUM,
	};

	enum ELoopNode
	{
		LOOP_NODE_START,
		LOOP_NODE_END,

		LOOP_NODE_MAX_NUM,
	};

	typedef struct SNodeInstance
	{
		BOOL isOverriding;
		BOOL isPicking;
		float fPosition;

		CGraphicTextInstance TextInstance;
	} TNodeInstance;

protected:
	void ClearEventNode();
	void RefreshGraph();

	void MakeEventNode(float fPosition);
	void DeleteEventNode(TEventNode * pEventNode);
	void GetEventNodeData(CObjectAnimationEvent & rDlgEvent, uint32_t dwIndex);

	void RenderNodeOutline(ENodeType eType, float fPosition);
	void RenderNodeOver(ENodeType eType, float fPosition);
	void RenderNodePicked(ENodeType eType, float fPosition);
	BOOL isOverNode(ENodeType eType, float fPosition, const CPoint & c_rPoint);

	void RenderComboData();

	void RenderAttackingRange();
	void RenderAttackingNode(float fCenter, float fPosition);
	BOOL isOverridingAttacking(float fCenter, float fPosition, CPoint & c_rPoint);

	float PositionToTime(float fPosition);
	float TimeToPosition(float fTime);

	void __UpdateAttackingNode();

protected:
	// Motion Event
	TEventNodeList m_EventNodeList;
	TEventNode * m_pOverNode;
	TEventNode * m_pSelectedNode;
	TEventNode * m_pPickedNode;
	CDynamicPool<TEventNode> m_EventNodePool;

	// Combo Input Data
	TNodeInstance m_ComboNode[COMBO_NODE_MAX_NUM];

	// Attacking Range
	TNodeInstance m_AttackingNode[ATTACKING_NODE_MAX_NUM];

	// Loop Start Time
	TNodeInstance m_LoopNode[LOOP_NODE_MAX_NUM];

	CRaceMotionDataAccessor * m_pMotionData;
	CScreen m_Screen;
	CRect m_WindowRect;
	float m_fDuration;
	uint32_t m_dwcurHitDataIndex;

	uint32_t m_dwcurFrame;
	BOOL m_isFrameStep;
	CGraphicTextInstance m_FrameIndexInstance;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
