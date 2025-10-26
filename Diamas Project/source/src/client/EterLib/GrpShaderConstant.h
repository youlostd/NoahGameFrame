#pragma once
#include <d3dx9effect.h>

#include "GrpTexture.h"

class CGraphicImageTexture;

enum VALUE_TYPE {
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_VECTOR2,
	TYPE_VECTOR3,
	TYPE_VECTOR4,
	TYPE_MATRIX,
	TYPE_BOOL,
	TYPE_UNKNOWN,
};

class D3DXDUALQATERNION	
{
public:
	Quaternion qNoneDualPart;
	Quaternion qDualPart;

	D3DXDUALQATERNION()	{}
	D3DXDUALQATERNION( Quaternion q1, Vector3 vTran)
	{
		// 논 튜얼 파트는 그냥 적용
		qNoneDualPart = q1;

		// 듀얼 파트 계산
		qDualPart.x = -0.5f * ( vTran.x*q1.y + vTran.y*q1.z + vTran.z*q1.w);
		qDualPart.y =  0.5f * ( vTran.x*q1.x + vTran.y*q1.w + vTran.z*q1.z);
		qDualPart.z =  0.5f * ( vTran.x*q1.w + vTran.y*q1.x + vTran.z*q1.y);
		qDualPart.w =  0.5f * ( vTran.x*q1.z + vTran.y*q1.y + vTran.z*q1.x);
	}

};

class GrpShaderConstant
{
public:
		GrpShaderConstant();
		~GrpShaderConstant();

		bool				SetInt(int x);
		bool				SetIntArray(const int* pArray, int nSize, int nOffset =0);
		bool				SetFloat(float x);
		bool				SetVector2(float x, float y);
		bool				SetVector2(const Vector2& v);
		bool				SetVector3(float x, float y, float z);
		bool				SetVector3(const Vector3& v);
		bool				SetVector4(float x, float y, float z, float w);
		bool                SetVector4(const Vector4& v);
		bool                SetVector4Array(const Vector4* pArray, int nSize, int nOffset = 0);
		bool                SetMatrix(const Matrix& rMatrix, bool bRepetitionCheck = true);
		bool                SetMatrixTranspose(const Matrix& rMatrix);
		bool                SetMatrixIndex(const Matrix& rMatrix, int nIndex);
		bool                SetMatrixTransposeIndex(const Matrix& rMatrix, int nIndex);
		bool                SetMatrixArray(const Matrix* pMatrix,int nCount);
        bool SetMatrixTransposeArray(const Matrix *pMatrix, int nCount);
        bool                SetDQuaternionArray(const D3DXDUALQATERNION* pDQ,int nCount);	
		bool                SetQuaternionArray(const Quaternion* pDQ,int nCount);		
		bool                SetVector3Array(const Vector3* pDQ,int nCount);
        bool SetTexture(CGraphicTexture *hTex);
        bool SetTexture(LPDIRECT3DTEXTURE9 hTex);

        bool				IsValid(void);

        std::string				GetName(void) { return m_strName; }


		LPD3DXEFFECT		m_pEffect;
		D3DXHANDLE			m_hParameter;

        std::string				m_strName;

		void*				m_pValue;
		VALUE_TYPE			m_eValueType;
		bool				m_bChkValue;
		/// µÎ¹ø ÀÌ»ó °ªÀÌ ¼³Á¤ µÇ¾ú´ÂÁöÀÇ Ã¼Å©¸¦ À§ÇÑ Ä«¿îÅÍ. ±×·¯¹Ç·Î ¹«ÇÑÈ÷ ¿Ã¶ó°¥ÇÊ¿ä´Â ¾ø´Ù. °í·Î »çÀÌÁîµµ Å¬ ÇÊ¿ä ¾øÀ½.
		BYTE				m_uiCounter;

		void				SetValue( VALUE_TYPE eType, const void* pValue);
		bool				IsSame( VALUE_TYPE eType, const void* pValue);
		bool				CreateValue( VALUE_TYPE eType);
		void				Destroy();
};


