
#include "MatrixStack.h"

MatrixStack::MatrixStack()
{
    m_pstack = NULL;
    m_stackSize = 0;

    m_currentPos = 0;
    m_ulRefCount = 1;
}

MatrixStack::~MatrixStack()
{
    if(m_pstack)
        delete [] m_pstack;

    return;
}

HRESULT
MatrixStack::Initialize(int stackSize)
{
    assert(m_pstack == NULL);

    if(!(m_pstack = new Matrix[stackSize]))
        return E_OUTOFMEMORY;

    m_stackSize = stackSize;
    m_currentPos = 0;

    LoadIdentity();
    return S_OK;
}

ULONG  STDMETHODCALLTYPE
MatrixStack::AddRef()
{
    return ++m_ulRefCount;
}

ULONG  STDMETHODCALLTYPE
MatrixStack::Release()
{
    ULONG ulRefCount = --m_ulRefCount;
    if( ulRefCount == 0 )
    {
        delete this;
    }
    return ulRefCount; // return the copy on the stack, which is guaranteed to still exist.
}


HRESULT
MatrixStack::Pop()
{
    if (m_currentPos == 0)
        return S_OK;
    else
        m_currentPos--;

    return S_OK;
}

HRESULT
MatrixStack::Push()
{
    m_currentPos++;

    if (m_currentPos >= m_stackSize)
    {
        Matrix *pstack;

        if(!(pstack = new Matrix[m_stackSize * 2]))
            return E_OUTOFMEMORY;

        if(m_pstack)
        {
            memcpy(pstack, m_pstack, m_stackSize * sizeof(Matrix));
            delete [] m_pstack;
        }

        m_pstack = pstack;
        m_stackSize *= 2;
    }

    m_pstack[m_currentPos] = m_pstack[m_currentPos-1];
    return S_OK;
}

HRESULT
MatrixStack::LoadIdentity()
{
    m_pstack[m_currentPos] = Matrix::Identity;
    return S_OK;
}

HRESULT
MatrixStack::LoadMatrix( const Matrix &pMat )
{
#if DBG
    if(!pMat)
    {
        DPF(0, "pMat pointer is invalid");
        return D3DERR_INVALIDCALL;
    }
#endif

    m_pstack[m_currentPos] = pMat;
    return S_OK;
}

HRESULT
MatrixStack::MultMatrix( const Matrix &pMat )
{
#if DBG
    if(!pMat)
    {
        DPF(0, "pMat pointer is invalid");
        return D3DERR_INVALIDCALL;
    }
#endif

    m_pstack[m_currentPos] = m_pstack[m_currentPos] * pMat;
    return S_OK;
}

HRESULT
MatrixStack::MultMatrixLocal( const Matrix &pMat )
{
#if DBG
    if(!pMat)
    {
        DPF(0, "pMat pointer is invalid");
        return D3DERR_INVALIDCALL;
    }
#endif

    m_pstack[m_currentPos] = pMat * m_pstack[m_currentPos];
    return S_OK;
}

Matrix *
MatrixStack::GetTop()
{
    return &m_pstack[m_currentPos];
}

HRESULT
MatrixStack::RotateAxis( const Vector3 &pV, float angle )
{
#if DBG
    if(!pV)
    {
        DPF(0, "pV pointer is invalid");
        return D3DERR_INVALIDCALL;
    }
#endif

    Matrix tmp = XMMatrixRotationAxis(pV, angle);
    m_pstack[m_currentPos] = m_pstack[m_currentPos] * tmp;
    return S_OK;
}

HRESULT
MatrixStack::RotateAxisLocal( const Vector3 &pV, float angle )
{
#if DBG
    if(!pV)
    {
        DPF(0, "pV pointer is invalid");
        return D3DERR_INVALIDCALL;
    }
#endif

    Matrix tmp = XMMatrixRotationAxis( pV, angle );
    m_pstack[m_currentPos] = tmp * m_pstack[m_currentPos];
    return S_OK;
}

HRESULT
MatrixStack::RotateYawPitchRoll( float yaw, float pitch, float roll )
{
    Matrix tmp = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
    m_pstack[m_currentPos] = m_pstack[m_currentPos] * tmp;
    return S_OK;
}

HRESULT
MatrixStack::RotateYawPitchRollLocal( float yaw, float pitch, float roll )
{
    Matrix tmp = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);

    m_pstack[m_currentPos] = tmp * m_pstack[m_currentPos];
    return S_OK;
}

HRESULT
MatrixStack::Scale( float x, float y, float z )
{
    Matrix tmp = Matrix::CreateScale(x, y, z );
    m_pstack[m_currentPos] = m_pstack[m_currentPos] * tmp;
    return S_OK;
}

HRESULT
MatrixStack::ScaleLocal( float x, float y, float z )
{
    Matrix tmp = Matrix::CreateScale(x, y, z );

    m_pstack[m_currentPos] = tmp * m_pstack[m_currentPos];
    return S_OK;
}

HRESULT
MatrixStack::Translate( float x, float y, float z )
{
    Matrix tmp = Matrix::CreateTranslation(x, y, z );
    m_pstack[m_currentPos] = m_pstack[m_currentPos] * tmp;
    return S_OK;
}

HRESULT
MatrixStack::TranslateLocal( float x, float y, float z )
{
    Matrix tmp = Matrix::CreateTranslation(x, y, z );

    m_pstack[m_currentPos] = tmp * m_pstack[m_currentPos];
    return S_OK;
}




HRESULT WINAPI
CreateMatrixStack( DWORD flags, MatrixStack **ppStack )
{
    HRESULT hr;

    // These flags will be used in the future to specify that
    // the user wants the internals to be double precision

    // Validate Parameters
    if( ppStack == NULL )
    {
        SPDLOG_DEBUG("ppStack pointer is invalid");
        return D3DERR_INVALIDCALL;
    }

    MatrixStack *pStack = new MatrixStack;

    if( pStack == NULL )
        return E_OUTOFMEMORY;

    if( FAILED(hr = pStack->Initialize(16)) )
    {
        pStack->Release();
        return hr;
    }

    *ppStack = pStack;
    return S_OK;
}