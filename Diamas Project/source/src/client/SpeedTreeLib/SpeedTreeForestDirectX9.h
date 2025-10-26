#ifndef METIN2_CLIENT_SPEEDTREELIB_SPEEDTREEFORESTDIRECTX9_HPP
#define METIN2_CLIENT_SPEEDTREELIB_SPEEDTREEFORESTDIRECTX9_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SpeedTreeForest.h"
#include "SpeedTreeMaterial.h"

#include "../EterLib/GrpBase.h"

METIN2_BEGIN_NS

class CSpeedTreeForestDirectX9 : public CSpeedTreeForest, public CGraphicBase, public CSingleton<CSpeedTreeForestDirectX9>
{
	public:
		CSpeedTreeForestDirectX9();
		~CSpeedTreeForestDirectX9( );

		bool Initialize();
		void UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const;
		void Render(unsigned long ulRenderBitVector = Forest_RenderAll);
		void UpdateCompoundMatrix(const Vector3 & c_rEyeVec, const Matrix & c_rmatView, const Matrix & c_rmatProj);

	private:
		bool                        InitVertexShaders( );

	private:
		LPDIRECT3DVERTEXSHADER9     m_dwBranchVertexShader;             // branch vertex shader
		LPDIRECT3DVERTEXSHADER9     m_dwLeafVertexShader;               // leaf vertex shader
};

METIN2_END_NS

#endif
