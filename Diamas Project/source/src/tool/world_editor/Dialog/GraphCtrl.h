#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_GRAPHCTRL_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_GRAPHCTRL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#define WORLD_EDITOR

#include <EterLib/GrpTextInstance.h>

#include <memory>

METIN2_BEGIN_NS

class CGraphCtrl : public CScreen
{
	public:
		class IValueAccessor
		{
			public:
				IValueAccessor(){}
				virtual ~IValueAccessor(){}

				virtual uint32_t GetValueCount() = 0;

				virtual void Insert(float fTime, uint32_t dwValue) = 0;
				virtual void Delete(uint32_t dwIndex) = 0;

				virtual bool GetTime(uint32_t dwIndex, float * pTime) = 0;
				virtual bool GetValue(uint32_t dwIndex, uint32_t * pValue) = 0;

				virtual void SetTime(uint32_t dwIndex, float fTime) = 0;
				virtual void SetValue(uint32_t dwIndex, uint32_t dwValue) = 0;
		};

		typedef struct SPoint
		{
			SPoint(int ix_ = 0, int iy_ = 0) : ix(ix_), iy(iy_) {}

			int ix;
			int iy;
		} TPoint;

	public:
		enum
		{
			POINT_NONE = 0xffffffff,

			HORIZON_ZOOMING_MAX = 10,
			HORIZON_ZOOMING_MIN = 1,
		};
		enum EGraphType
		{
			GRAPH_TYPE_BOOLEAN,
			GRAPH_TYPE_BRIDGE,
			GRAPH_TYPE_LINEAR,
			GRAPH_TYPE_BLOCK,
		};
		enum EValueType
		{
			VALUE_TYPE_CENTER,
			VALUE_TYPE_ONLY_UP,
			VALUE_TYPE_ONLY_DOWN,
			VALUE_TYPE_UP_AND_DOWN,
		};

		typedef std::vector<std::unique_ptr<CGraphicTextInstance> > TTextInstanceVector;
		typedef std::vector<TPoint> TPointVector;

	public:
		CGraphCtrl();
		virtual ~CGraphCtrl();

		void Initialize();
		void SetGraphType(int iType);
		void SetValueType(int iType);
		void SetAccessorPointer(IValueAccessor * pAccessor);
		void SetSize(int iWidth, int iHeight);

		void Move(float fx, float fy);

		void Update();
		void GraphBegin();
		void GraphEnd(RECT * pRect, HWND hWnd);
		void Render();
		void RenderTimeLine(float fTime);
		void RenderEndLine(float fTime);

		void ZoomInHorizon();
		void ZoomOutHorizon();

		void OnMouseMove(int ix, int iy);
		void OnLButtonDown(int ix, int iy);
		void OnLButtonUp(int ix, int iy);
		void OnRButtonDown(int ix, int iy);
		void OnRButtonUp(int ix, int iy);
		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		uint32_t GetSelectedIndex();
		void SetSelectedIndex(uint32_t dwSelectedIndex);

	protected:
		void BuildHorizontalLine(int iCount);
		void UpdateHorizontalLine(int iStartPosition, int iZoom);

		void RenderGrid();
		void RenderGraph();
		void ConnectPoint(TPoint & rLeft, TPoint & rRight);

		void TimeToScreen(float fTime, int * px);
		void ScreenToTime(uint32_t ix, float * pTime);

		bool isCreatingMode();

	protected:
		int m_iGraphType;
		int m_iValueType;

		IValueAccessor * m_pAccessor;

		// Mouse Control
		uint32_t m_dwSelectedIndex;
		uint32_t m_dwGrippedIndex;

		TPointVector m_PointVector;

		// For Rendering
		int m_iWidth;
		int m_iHeight;

		float m_fxPosition;
		float m_fyPosition;
		int m_iHorizontalZoom;

		int m_ixTemporary;
		int m_iyTemporary;
		int m_ixGridStep;
		int m_iyGridStep;
		int m_ixGridCount;
		int m_iyGridCount;

		// Text
		TTextInstanceVector m_HorizontalTextLine;
		TTextInstanceVector m_VerticalTextLine;
};

METIN2_END_NS

#endif
