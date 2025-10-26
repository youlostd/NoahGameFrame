#ifndef METIN2_CLIENT_ETERGRNLIB_GRANNYLIBRARY_HPP
#define METIN2_CLIENT_ETERGRNLIB_GRANNYLIBRARY_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <Direct3D.hpp>


class MatrixStack
{
private:
    int m_stackSize;
    Matrix *m_pstack;

    int m_currentPos;
    ULONG m_ulRefCount;

public:
    MatrixStack();
    ~MatrixStack();

    HRESULT Initialize(int stackSize);

    // IUnknown
    ULONG  AddRef();
    ULONG  Release();
    HRESULT Pop();
    HRESULT Push();
    HRESULT LoadIdentity();
    HRESULT LoadMatrix(const Matrix &pMat);
    HRESULT MultMatrix(const Matrix &pMat);
    HRESULT MultMatrixLocal(const Matrix &pMat);
    Matrix *GetTop();
    HRESULT RotateAxis(const Vector3 &pV, float angle);
    HRESULT RotateAxisLocal(const Vector3 &pV, float angle);
    HRESULT RotateYawPitchRoll(float yaw, float pitch, float roll);
    HRESULT RotateYawPitchRollLocal(float yaw, float pitch, float roll);
    HRESULT Scale(float x, float y, float z);
    HRESULT ScaleLocal(float x, float y, float z);
    HRESULT Translate(float x, float y, float z);
    HRESULT TranslateLocal(float x, float y, float z);

    // ID3DXMatrixStack


};
HRESULT WINAPI
CreateMatrixStack( DWORD flags, MatrixStack **ppStack );

#endif
