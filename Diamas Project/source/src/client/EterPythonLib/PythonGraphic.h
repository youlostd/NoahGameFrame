#pragma once

#include <Config.hpp>

#include "../EterLib/GrpTextInstance.h"
#include "../EterLib/GrpMarkInstance.h"
#include "../EterLib/GrpImageInstance.h"
#include "../EterLib/GrpExpandedImageInstance.h"
#include "../EterLib/CullingManager.h"

#include "../EterGrnLib/ThingInstance.h"

#include <base/Singleton.hpp>
#include <stack>
#include "CColorPicker.h"

class CPythonGraphic : public CScreen, public CSingleton<CPythonGraphic>
{
	public:
		CPythonGraphic();
		virtual ~CPythonGraphic();

		void Destroy();

		void PushState();
		void PopState();

		LPDIRECT3D9 GetD3D();

		float GetOrthoDepth();
		void SetInterfaceRenderState();
		void SetGameRenderState();

		void SetCursorPosition (int x, int y);

		void SetOmniLight();

		void SetViewport (DWORD fx, DWORD fy, float fWidth, float fHeight);
		void RestoreViewport();

		long GenerateColor (float r, float g, float b, float a);
		void RenderDownButton (float sx, float sy, float ex, float ey);
		void RenderUpButton (float sx, float sy, float ex, float ey);

		void RenderImage (CGraphicImageInstance* pImageInstance, float x, float y);
		void RenderAlphaImage (CGraphicImageInstance* pImageInstance, float x, float y, float aLeft, float aRight);
		void RenderCoolTimeBoxColor (float fxCenter, float fyCenter, float fRadius, float fTime, float fColorR, float fColorG, float fColorB, float fColorA);
	void RenderCoolTimeImageBox(CGraphicImageInstance* image, float fTime) const;
	void RenderCoolTimeBoxInverse (float fxCenter, float fyCenter, float fRadius, float fTime);

		bool SaveScreenShot (const char* szFileName);
	bool SaveScreenShotFull(const char* c_pszFileName);

	DWORD GetAvailableMemory();
		void SetGamma (float fGammaFactor = 1.0f);


	protected:
		typedef struct SState
		{
			Matrix matView;
			Matrix matProj;
		} TState;

		DWORD		m_lightColor;
		DWORD		m_darkColor;

	protected:
		std::stack<TState>						m_stateStack;

		Matrix								m_SaveWorldMatrix;

		CCullingManager							m_CullingManager;

		D3DVIEWPORT9							m_backupViewport;

		float									m_fOrthoDepth;

		CColorPicker							m_colorPicker;

};

