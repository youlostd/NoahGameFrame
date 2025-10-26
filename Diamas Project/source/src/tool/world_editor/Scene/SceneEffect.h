#ifndef METIN2_TOOL_WORLDEDITOR_SCENE_SCENEEFFECT_HPP
#define METIN2_TOOL_WORLDEDITOR_SCENE_SCENEEFFECT_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "PickingArrow.h"
#include "../Dialog/EffectTranslationDialog.h"

#include <GameLib/EnvironmentInstance.hpp>

METIN2_BEGIN_NS

class CSceneEffect : public CSceneBase
{
	public:
		enum
		{
			POINT_VALUE_VISIBLE,
			POINT_VALUE_POSITION,
		};

		enum
		{
			POINT_TYPE_POSITION,
			POINT_TYPE_CONTROL_POINT,
		};

	public:
		typedef std::vector<float> TStartTimeVector;
		typedef std::list<CEffectInstanceAccessor*> TEffectInstanceAccessorList;

	public:
		CSceneEffect();
		virtual ~CSceneEffect();

		void Initialize();

		void Play();
		void PlayLoop();
		void StopLoop();
		void Stop();

		uint32_t GetStartTimeCount();
		bool GetStartTime(uint32_t dwIndex, float * pTime);

		void SelectEffectElement(uint32_t dwIndex);
		void SetEffectAccessor(CEffectAccessor * pData);

		void UpdatePreviewInstance();

		uint32_t GetSelectedPositionIndex();
		void SetSelectedPositionIndex(uint32_t dwNewIndex);

		void UsingSceneObject(BOOL bUsing);
		void DeselectGrippedPosition();

		void SetCursorVisibility(BOOL bShow);
		void SetCharacterVisibility(BOOL bShow);
		void SetBoundingSphereVisibility(BOOL bShow);

		void RefreshTranslationDialog();
		void OnChangeEffectPosition();

	protected:
		void OnUpdate();
		void OnRender(BOOL bClear);
		void OnRenderUI(float fx, float fy);

		void RenderEffectPosition();
		void RenderPositionGraph();
		void RenderBoundingSphere();
		void PickingPositionGraph();

		void OnKeyDown(int iChar);
		void OnKeyUp(int iChar);
		void OnMouseMove(long x, long y);
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp();
		void OnRButtonDown();
		void OnRButtonUp();
		BOOL OnMouseWheel(short zDelta);
		void OnMovePosition(float fx, float fy);

	protected:
		CEffectAccessor * m_pEffectAccessor;

		EnvironmentInstance m_env;

		uint32_t m_dwSelectedIndex;

		int m_iGrippedPointType;
		uint32_t m_dwGrippedPositionIndex;
		int m_iGrippedDirection;

		D3DXVECTOR3 m_vecGrippedPosition;
		D3DXVECTOR3 m_vecGrippedValue;

		float m_fCurrentTime;
		TStartTimeVector m_StartTimeVector;
		TEffectInstanceAccessorList m_EffectInstanceAccessorList;

		bool m_isLoop;
		uint32_t m_dwLoopStartTimeIndex;

		CEffectTranslationDialog * m_pEffectTranslationDialog;

		///////////////////////////////////////////////////////////////////////////

		D3DXCOLOR m_ClearColor;
		BOOL m_bUsingSceneObject;
		BOOL m_bShowCursor;
		BOOL m_bShowCharacter;
		BOOL m_bShowBoundingSphere;

		///////////////////////////////////////////////////////////////////////////

		int m_iStartTime;
		int m_iMaxTime;
		int m_iMinTime;
		CGraphicTextInstance m_TextInstanceFrame;
		CGraphicTextInstance m_TextInstanceElapsedTime;

	protected:
		CDynamicPool<CEffectInstanceAccessor> m_EffectInstanceAccessorPool;
};

METIN2_END_NS

#endif
