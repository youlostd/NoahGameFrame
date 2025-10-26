#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTPARTICLETIMEEVENTGRAPH_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_EFFECTPARTICLETIMEEVENTGRAPH_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GraphCtrl.h"

#include <EffectLib/Util.hpp>

METIN2_BEGIN_NS

class CFloatEditDialog;

class CEffectTimeEventGraph : public CStatic
{
public:
	class IValueAccessor
	{
		public:
			IValueAccessor(){}
			virtual ~IValueAccessor(){}

			virtual uint32_t GetValueCount() = 0;

			virtual void Insert(float fTime, float fValue) = 0;
			virtual void Delete(uint32_t dwIndex) = 0;

			virtual bool GetTime(uint32_t dwIndex, float * pfTime) = 0;
			virtual bool GetValue(uint32_t dwIndex, float* pfValue) = 0;

			virtual void SetTime(uint32_t dwIndex, float fTime) = 0;
			virtual void SetValue(uint32_t dwIndex, float fValue) = 0;
	};

	typedef struct SPoint
	{
		SPoint(int ix_ = 0, int iy_ = 0) : ix(ix_), iy(iy_) {}

		int ix;
		int iy;
	} TPoint;
	typedef std::vector<TPoint> TPointVector;

	enum
	{
		POINT_NONE = 0xffffffff,

		MAX_GRAPH_COUNT = 3,
	};

// Construction
public:
	CEffectTimeEventGraph();
	void Initialize(int iTimerID);

// Attributes
public:

// Operations
public:
	void Resizing(int iWidth, int iHeight);
	void SetAccessorPointer(IValueAccessor * pAccessor);

	void SetMaxTime(float fMaxTime);
	void SetMaxValue(float fMaxValue);
	void SetStartValue(float fStartValue);

protected:
	void RenderGrid();
	void RenderGraph();
	void ConnectPoint(TPoint & rLeft, TPoint & rRight);

	bool isCreatingMode();

	void TimeToScreen(float fTime, int * px);
	void ScreenToTime(int ix, float * pfTime);

	void ValueToScreen(float fValue, int * piy);
	void ScreenToValue(int iy, float * pfValue);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectTimeEventGraph)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEffectTimeEventGraph();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEffectTimeEventGraph)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	// Mouse
	bool m_isLButtonDown;
	bool m_isMButtonDown;
	CPoint m_LastPoint;

	float m_fMaxTime;
	float m_fMaxValue;
	float m_fStartValue;

	uint32_t m_dwSelectedTableIndex;
	uint32_t m_dwSelectedIndex;
	uint32_t m_dwGrippedIndex;

	int m_iWidth;
	int m_iHeight;
	int m_ixTemporary;
	int m_iyTemporary;
	int m_ixGridCount;
	int m_iyGridCount;
	float m_fxGridStep;
	float m_fyGridStep;

	IValueAccessor * m_pAccessor;
	TPointVector m_PointVector;

	CScreen m_Screen;

	static CFloatEditDialog * ms_pFloatEditDialog;
};

template <typename T = float>
class CTimeEventTableAccessor : public CEffectTimeEventGraph::IValueAccessor
{
public:
	CTimeEventTableAccessor(){}
	~CTimeEventTableAccessor(){}

	typedef std::vector<CTimeEvent<T> > TTimeEventTableType;

	TTimeEventTableType * m_pTimeEventTable;

	void SetTablePointer(TTimeEventTableType * pTimeEventTable)
	{
		m_pTimeEventTable = pTimeEventTable;
	}

	uint32_t GetValueCount()
	{
		return m_pTimeEventTable->size();
	}

	void GetTimeValue(float fTime, T * pfValue)
	{
		*pfValue = GetTimeEventBlendValue(fTime, *m_pTimeEventTable);
	}

	bool GetTime(uint32_t dwIndex, float * pfTime)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return false;

		*pfTime = m_pTimeEventTable->at(dwIndex).m_fTime;
		return true;
	}

	bool GetValue(uint32_t dwIndex, T * pfValue)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return false;

		*pfValue = m_pTimeEventTable->at(dwIndex).m_Value;
		return true;
	}

	void SetTime(uint32_t dwIndex, float fTime)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return;

		m_pTimeEventTable->at(dwIndex).m_fTime = fTime;
	}

	void SetValue(uint32_t dwIndex, T fValue)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return;

		m_pTimeEventTable->at(dwIndex).m_Value = fValue;
	}

	void Insert(float fTime, T fValue)
	{
		InsertItemTimeEvent(m_pTimeEventTable, fTime, fValue);
	}

	void InsertBlend(float fTime)
	{
		T fValue;
		GetTimeValue(fTime, &fValue);
		Insert(fTime, fValue);
	}

	void Delete(uint32_t dwIndex)
	{
		STORM_ASSERT(dwIndex < m_pTimeEventTable->size(), "Out of range");
		m_pTimeEventTable->erase(m_pTimeEventTable->begin() + dwIndex);
	}
};

template <>
class CTimeEventTableAccessor<double>: public CEffectTimeEventGraph::IValueAccessor
{
public:
	CTimeEventTableAccessor(){}
	~CTimeEventTableAccessor(){}

	typedef std::vector<CTimeEvent<double> > TTimeEventTableType;

	TTimeEventTableType * m_pTimeEventTable;

	void SetTablePointer(TTimeEventTableType * pTimeEventTable)
	{
		m_pTimeEventTable = pTimeEventTable;
	}

	uint32_t GetValueCount()
	{
		return m_pTimeEventTable->size();
	}

	void GetTimeValue(float fTime, double * pfValue)
	{
		*pfValue = GetTimeEventBlendValue(fTime, *m_pTimeEventTable);
	}

	bool GetTime(uint32_t dwIndex, float * pfTime)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return false;

		*pfTime = m_pTimeEventTable->at(dwIndex).m_fTime;
		return true;
	}

	bool GetValue(uint32_t dwIndex, float* pfValue)
	{
		return GetValue(dwIndex, (double*)pfValue);
	}

	bool GetValue(uint32_t dwIndex, double * pfValue)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return false;

		*pfValue = m_pTimeEventTable->at(dwIndex).m_Value;
		return true;
	}

	void SetTime(uint32_t dwIndex, float fTime)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return;

		m_pTimeEventTable->at(dwIndex).m_fTime = fTime;
	}

	void SetValue(uint32_t dwIndex, double fValue)
	{
		if (dwIndex >= m_pTimeEventTable->size())
			return;

		m_pTimeEventTable->at(dwIndex).m_Value = fValue;
	}

	void SetValue(uint32_t dwIndex, float fValue)
	{
		SetValue(dwIndex, double(fValue));
	}

	void Insert(float fTime, double fValue)
	{
		InsertItemTimeEvent(m_pTimeEventTable, fTime, fValue);
	}

	void Insert(float fTime, float fValue)
	{
		Insert(fTime, double(fValue));
	}

	void InsertBlend(float fTime)
	{
		double fValue;
		GetTimeValue(fTime, &fValue);
		Insert(fTime, fValue);
	}

	void Delete(uint32_t dwIndex)
	{
		STORM_ASSERT(dwIndex < m_pTimeEventTable->size(), "Out of range");
		m_pTimeEventTable->erase(m_pTimeEventTable->begin() + dwIndex);
	}
};


typedef CTimeEventTableAccessor<float> CTimeEventFloatAccessor;
typedef CTimeEventTableAccessor<double> CTimeEventDoubleAccessor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif
