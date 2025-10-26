#ifndef METIN2_CLIENT_GAMELIB_FLYTRACE_HPP
#define METIN2_CLIENT_GAMELIB_FLYTRACE_HPP

#include <Config.hpp>

#include "../EterLib/GrpScreen.h"
#include "FlyingData.h"

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



class CFlyTrace : public CScreen
{
	public:
		CFlyTrace();
		virtual ~CFlyTrace();

		void Destroy();

		void Create(const CFlyingData::TFlyingAttachData & rFlyingAttachData);

		void UpdateNewPosition(const Vector3 & v3Position);

		void Update();
		void Render();

	protected:
		void __Initialize();

	protected:
		typedef std::pair<float, Vector3> TTimePosition;
		typedef std::deque<TTimePosition> TTimePositionDeque;

		//CGraphicImageInstance m_ImageInstance;
		//LPDIRECT3DTEXTURE8 m_lpTexture;

		bool m_bRectShape;
		DWORD m_dwColor;
		float m_fSize;
		float m_fTailLength;

		TTimePositionDeque m_TimePositionDeque;

	public:
		static void DestroySystem();

		static CFlyTrace* New();
		static void Delete(CFlyTrace* pkInst);

		static CDynamicPool<CFlyTrace>		ms_kPool;
};


#endif