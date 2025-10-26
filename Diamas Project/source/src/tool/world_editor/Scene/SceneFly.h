#ifndef METIN2_TOOL_WORLDEDITOR_SCENE_SCENEFLY_HPP
#define METIN2_TOOL_WORLDEDITOR_SCENE_SCENEFLY_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "PickingArrow.h"

METIN2_BEGIN_NS

class CFlyingData;
class CFlyingInstance;

class CSceneFly : public CSceneBase
{
	public:
		CSceneFly();
		virtual ~CSceneFly();

		void Initialize();

		void Play();
		void Stop();

		CFlyingData * GetFlyingDataPointer() { return m_pFlyingData;}

	protected:
		void RenderFlyPosition();

		void	OnUpdate() ;
		void	OnRender(BOOL bClear) ;
		void	OnRenderUI(float fx, float fy) ;

		void	OnKeyDown(int iChar) ;
		void	OnKeyUp(int iChar) ;
		void	OnMouseMove(LONG x, LONG y) ;
		void	OnLButtonDown(UINT nFlags, CPoint point) ;
		void	OnLButtonUp() ;
		void	OnRButtonDown() ;
		void	OnRButtonUp() ;
		BOOL	OnMouseWheel(short zDelta) ;
		void	OnMovePosition(float fx, float fy) ;

		const D3DXVECTOR3 start;

		uint32_t m_dwPlayStartTime;
		CFlyingData * m_pFlyingData;
		CFlyingInstance * m_pFlyingInstance;

		D3DXVECTOR3 m_v3Target;

		int m_iGrippedDirection;

		D3DXVECTOR3 m_vecGrippedPosition;
		D3DXVECTOR3 m_vecGrippedValue;
};

METIN2_END_NS

#endif
