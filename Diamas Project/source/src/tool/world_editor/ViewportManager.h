#ifndef METIN2_TOOL_WORLDEDITOR_VIEWPORTMANAGER_HPP
#define METIN2_TOOL_WORLDEDITOR_VIEWPORTMANAGER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CViewportManager
{
	public:
		CViewportManager();
		~CViewportManager();

		void Initialize();

		void MouseMove(CPoint Mouse);

		void ControlKeyDown();
		void ControlKeyUp();

		void LeftButtonDown(CPoint Mouse);
		void LeftButtonUp(CPoint Mouse);

		void MiddleButtonDown(CPoint Mouse);
		void MiddleButtonUp(CPoint Mouse);

		void RightButtonDown(CPoint Mouse, float localX, float localY);
		void RightButtonUp(CPoint Mouse);

		void MouseWheel(float Wheel);

		// Data Accessor
		void SetCursorPosition(int ix, int iy);
		void SetScreenStatus(int iWidth, int iHeight);
		float GetFrustumFarDistance();
		float GetFrustumNearDistance() {return m_fFrustumNear;}
		float GetDistance();
		float GetPitch();
		float GetRotation();

		bool CheckControlKey();

		bool isLeftButton();
		bool isMiddleButton();
		bool isRightButton();

		bool IsMouseTranslationMode();
		void UpdateMouseTranslation(float x, float y, float& xMove, float& yMove);

	protected:
		bool				m_ControlKey;

		bool				m_ClickedLeftButton;
		bool				m_ClickedMiddleButton;
		bool				m_ClickedRightButton;
		CPoint				m_ClickedMiddleButtonPoint;
		CPoint				m_ClickedLeftButtonPoint;
		CPoint				m_ClickedRightButtonPoint;
		float m_lastRightLocalX, m_lastRightLocalY;

		float				m_fFrustumNear;
		float				m_fFrustumFar;

		int					m_iWidth;
		int					m_iHeight;

		int					m_ixCursor;
		int					m_iyCursor;
};

METIN2_END_NS

#endif
