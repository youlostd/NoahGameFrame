#include "GrpShaderConstant.h"
#include "StdAfx.h"

#include "GrpImageTexture.h"
#include "GrpTechnique.h"

#include <Utils.h>

#define CHECK_VALUE 1

GrpShaderConstant::GrpShaderConstant()
    : m_pEffect(NULL), m_hParameter(NULL), m_pValue(NULL), m_eValueType(TYPE_UNKNOWN), m_bChkValue(true), m_uiCounter(0)
{
}

GrpShaderConstant::~GrpShaderConstant()
{
    Destroy();
}

void GrpShaderConstant::Destroy()
{
    SAFE_DELETE(m_pValue);
}

void GrpShaderConstant::SetValue(VALUE_TYPE eType, const void *pValue)
{
#ifndef CHECK_VALUE
    return;
#endif
    if (!m_bChkValue)
        return;

    /// 카운터가 1 미만. 즉, 처음 값이 셋팅 되었을 경우는 이전 값을 저장해놓지 않는다.
    /// 로컬 블럭 내에서 GetConstant().Set()시 마다 매번 new가 일어 나는 것을 방지하기 위함.
    if (m_uiCounter < 1)
    {
        m_uiCounter = 1;
        return;
    }

    if (!m_pValue)
        CreateValue(eType);

    if (m_eValueType != eType)
        return;

    switch (eType)
    {
    case TYPE_INT:
        *((int *)m_pValue) = *((int *)pValue);
        break;
    case TYPE_FLOAT:
        *((float *)m_pValue) = *((float *)pValue);
        break;
    case TYPE_VECTOR2:
        *((Vector2 *)m_pValue) = *((Vector2 *)pValue);
        break;
    case TYPE_VECTOR3:
        *((Vector3 *)m_pValue) = *((Vector3 *)pValue);
        break;
    case TYPE_VECTOR4:
        *((Vector4 *)m_pValue) = *((Vector4 *)pValue);
        break;
    case TYPE_MATRIX:
        *((Matrix *)m_pValue) = *((Matrix *)pValue);
        break;
    case TYPE_BOOL:
        *((BOOL *)m_pValue) = *((BOOL *)pValue);
        break;
    default:
        return;
    }
}

bool GrpShaderConstant::IsSame(VALUE_TYPE eType, const void *pValue)
{
#ifndef CHECK_VALUE
    return false;
#endif

    if (!m_bChkValue || m_eValueType != eType || !m_pValue)
        return false;

    switch (eType)
    {
    case TYPE_INT:
        return (memcmp(m_pValue, pValue, sizeof(int)) == 0);
    case TYPE_FLOAT:
        return (memcmp(m_pValue, pValue, sizeof(float)) == 0);
    case TYPE_VECTOR2:
        return (memcmp(m_pValue, pValue, sizeof(Vector2)) == 0);
    case TYPE_VECTOR3:
        return (memcmp(m_pValue, pValue, sizeof(Vector3)) == 0);
    case TYPE_VECTOR4:
        return (memcmp(m_pValue, pValue, sizeof(Vector4)) == 0);
    case TYPE_MATRIX:
        return (memcmp(m_pValue, pValue, sizeof(Matrix)) == 0);
    default:
        return false;
    }
}

bool GrpShaderConstant::CreateValue(VALUE_TYPE eType)
{
#ifndef CHECK_VALUE
    return false;
#endif

    if (!m_bChkValue)
        return false;

    switch (eType)
    {
    case TYPE_INT:
        m_pValue = new int;
        break;
    case TYPE_FLOAT:
        m_pValue = new float;
        break;
    case TYPE_VECTOR2:
        m_pValue = new Vector2;
        break;
    case TYPE_VECTOR3:
        m_pValue = new Vector3;
        break;
    case TYPE_VECTOR4:
        m_pValue = new Vector4;
        break;
    case TYPE_MATRIX:
        m_pValue = new Matrix;
        break;
    default:
        return false;
    }
    m_eValueType = eType;

    return true;
}

bool GrpShaderConstant::SetInt(int x)
{
    if(!IsValid())
        return false;

    bool returnVal = true;
    if (!IsSame(TYPE_INT, &x))
    {
        returnVal = SUCCEEDED(m_pEffect->SetInt(m_hParameter, x));
        if (returnVal)
            SetValue(TYPE_INT, &x);
    }

    return returnVal;
}

bool GrpShaderConstant::SetIntArray(const int *pArray, int nSize, int nOffset)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    if (nOffset == 0)
    {
        return SUCCEEDED(m_pEffect->SetIntArray(m_hParameter, pArray, nSize));
    }
    else
    {
        for (int i = 0; i < nSize; ++i)
        {
            D3DXHANDLE hParameter = m_pEffect->GetParameterElement(m_hParameter, nOffset + i);
            if (!SUCCEEDED(m_pEffect->SetInt(hParameter, pArray[i])))
                return false;
        }

        return true;
    }
}

bool GrpShaderConstant::SetFloat(float x)
{
    if(!IsValid())
        return false;

    bool returnVal = true;
    if (!IsSame(TYPE_FLOAT, &x))
    {
        returnVal = SUCCEEDED(m_pEffect->SetFloat(m_hParameter, x));
        if (returnVal)
            SetValue(TYPE_FLOAT, &x);
    }

    return returnVal;
}

bool GrpShaderConstant::SetVector2(float x, float y)
{
    if(!IsValid())
        return false;

    Vector4 vec;
    vec.x = x;
    vec.y = y;
    vec.z = 0;
    vec.w = 0;

    bool returnVal = true;
    if (!IsSame(TYPE_VECTOR4, &vec))
    {
        returnVal = SUCCEEDED(m_pEffect->SetVector(m_hParameter, (D3DXVECTOR4*)&vec));
        if (returnVal)
            SetValue(TYPE_VECTOR4, &vec);
    }

    return returnVal;
}

bool GrpShaderConstant::SetVector2(const Vector2 &v)
{
    if(!IsValid())
        return false;

    return SetVector2(v.x, v.y);
}

bool GrpShaderConstant::SetVector3(float x, float y, float z)
{
    if(!IsValid())
        return false;

    Vector4 vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    vec.w = 0;

    bool returnVal = true;
    if (!IsSame(TYPE_VECTOR4, &vec))
    {
        returnVal = SUCCEEDED(m_pEffect->SetVector(m_hParameter, (D3DXVECTOR4*)&vec));
        if (returnVal)
            SetValue(TYPE_VECTOR4, &vec);
    }

    return returnVal;
}

bool GrpShaderConstant::SetVector3(const Vector3 &v)
{
    if(!IsValid())
        return false;

    return SetVector3(v.x, v.y, v.z);
}

bool GrpShaderConstant::SetVector4(float x, float y, float z, float w)
{
    if(!IsValid())
        return false;

    Vector4 vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    vec.w = w;

    bool returnVal = true;
    if (!IsSame(TYPE_VECTOR4, &vec))
    {
        returnVal = SUCCEEDED(m_pEffect->SetVector(m_hParameter, (D3DXVECTOR4*)&vec));
        if (returnVal)
            SetValue(TYPE_VECTOR4, &vec);
    }

    return returnVal;
}

bool GrpShaderConstant::SetVector4(const Vector4 &v)
{
    if(!IsValid())
        return false;

    return SetVector4(v.x, v.y, v.z, v.w);
}

bool GrpShaderConstant::SetVector4Array(const Vector4 *pArray, int nSize, int nOffset)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    if (nOffset == 0)
    {
        return SUCCEEDED(m_pEffect->SetVectorArray(m_hParameter, (const D3DXVECTOR4 *)pArray, nSize));
    }
    else
    {
        for (int i = 0; i < nSize; ++i)
        {
            D3DXHANDLE hParameter = m_pEffect->GetParameterElement(m_hParameter, nOffset + i);
            if (!SUCCEEDED(m_pEffect->SetVector(hParameter, (const D3DXVECTOR4 *)pArray)))
                return false;
        }

        return true;
    }
}

bool GrpShaderConstant::SetMatrix(const Matrix &rMatrix, bool bRepetitionCheck)
{
    if(!IsValid())
        return false;

    bool returnVal = true;
    if (!bRepetitionCheck || !IsSame(TYPE_MATRIX, &rMatrix))
    {
        returnVal = SUCCEEDED(m_pEffect->SetMatrix(m_hParameter, (const D3DXMATRIX *)&rMatrix));
        if (returnVal)
            SetValue(TYPE_MATRIX, &rMatrix);
    }

    return returnVal;
}

bool GrpShaderConstant::SetMatrixTranspose(const Matrix &rMatrix)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    return SUCCEEDED(m_pEffect->SetMatrixTranspose(m_hParameter, (const D3DXMATRIX *)&rMatrix));
}

bool GrpShaderConstant::SetMatrixIndex(const Matrix &rMatrix, int nIndex)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    D3DXHANDLE hElement = m_pEffect->GetParameterElement(m_hParameter, nIndex);
    _ASSERT(hElement);
    return SUCCEEDED(m_pEffect->SetMatrix(hElement, (const D3DXMATRIX *)&rMatrix));
}

bool GrpShaderConstant::SetMatrixTransposeIndex(const Matrix &rMatrix, int nIndex)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    D3DXHANDLE hElement = m_pEffect->GetParameterElement(m_hParameter, nIndex);
    _ASSERT(hElement);
    return SUCCEEDED(m_pEffect->SetMatrixTranspose(hElement, (const D3DXMATRIX *)&rMatrix));
}

bool GrpShaderConstant::SetMatrixArray(const Matrix *pMatrix, int nCount)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    return SUCCEEDED(m_pEffect->SetMatrixArray(m_hParameter, (const D3DXMATRIX *)pMatrix, nCount));
}

bool GrpShaderConstant::SetMatrixTransposeArray(const Matrix *pMatrix, int nCount)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    return SUCCEEDED(m_pEffect->SetMatrixTransposeArray(m_hParameter, (const D3DXMATRIX *)pMatrix, nCount));
}

bool GrpShaderConstant::SetDQuaternionArray(const D3DXDUALQATERNION *pDQ, int nCount)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    return SUCCEEDED(m_pEffect->SetValue(m_hParameter, pDQ, D3DX_DEFAULT));
}

bool GrpShaderConstant::SetQuaternionArray(const Quaternion *pQ, int nCount)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    return SUCCEEDED(m_pEffect->SetValue(m_hParameter, pQ, D3DX_DEFAULT));
}

bool GrpShaderConstant::SetVector3Array(const Vector3 *pV, int nCount)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    return SUCCEEDED(m_pEffect->SetValue(m_hParameter, pV, D3DX_DEFAULT));
}

bool GrpShaderConstant::SetTexture(CGraphicTexture *hTex)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.
    

    LPDIRECT3DBASETEXTURE9 tex;
    m_pEffect->GetTexture(m_hParameter, &tex);

    if (tex == hTex->GetD3DTexture())
        return true;

    if (!hTex)
        return SUCCEEDED(m_pEffect->SetTexture(m_hParameter, nullptr));

    return SUCCEEDED(m_pEffect->SetTexture(m_hParameter, hTex->GetD3DTexture()));
}

bool GrpShaderConstant::SetTexture(LPDIRECT3DTEXTURE9 hTex)
{
    if(!IsValid())
        return false;

    m_bChkValue = false; // 이전 값과의 중복 체크를 더이상 하지 않는다.

    return SUCCEEDED(m_pEffect->SetTexture(m_hParameter, hTex));
}

bool GrpShaderConstant::IsValid()
{
    return m_hParameter != NULL && m_pEffect != NULL;
}

//// by pok, 새로운 텍스쳐에 대응하는 쉐이더
// bool GrpShaderConstant::SetTexture( RTexture_x* _pTexture)
//{
//	return SUCCEEDED(m_pEffect->SetTexture(m_hParameter, _pTexture ? static_cast< RDeviceD3D::RTextureD3D_* >( _pTexture
//)->texture : NULL ) );
//}
