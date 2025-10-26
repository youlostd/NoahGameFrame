#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpIndexBuffer.h"
#include "StateManager.h"

LPDIRECT3DINDEXBUFFER9 CGraphicIndexBuffer::GetD3DIndexBuffer() const
{
    assert(m_lpd3dIdxBuf != NULL);
    return m_lpd3dIdxBuf;
}

void CGraphicIndexBuffer::Bind() const
{
    assert(ms_lpd3dDevice != nullptr);
    assert(m_lpd3dIdxBuf != nullptr);
    STATEMANAGER.SetIndices(m_lpd3dIdxBuf);
}

bool CGraphicIndexBuffer::Lock(void **pretIndices, size_t sizeToLock) const
{
    assert(m_lpd3dIdxBuf != NULL);

    const auto hr = m_lpd3dIdxBuf->Lock(0, sizeToLock, pretIndices, 0);
    if (FAILED(hr))
    {
        spdlog::trace("IndexBuffer::Lock() failed with {0}", hr);
        return false;
    }

    return true;
}

void CGraphicIndexBuffer::Unlock() const
{
    assert(m_lpd3dIdxBuf != NULL);

    const auto hr = m_lpd3dIdxBuf->Unlock();
    if (FAILED(hr))
    {
        spdlog::trace("IndexBuffer::Unlock() failed for "
                      "size {0} fmt {1} with {2}",
                      m_dwBufferSize, m_d3dFmt, hr);
    }
}

bool CGraphicIndexBuffer::Copy(int bufSize, const void *srcIndices)
{
    assert(m_lpd3dIdxBuf != NULL);

    BYTE *dstIndices;
    if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, (void**)&dstIndices, 0)))
        return false;

    memcpy(dstIndices, srcIndices, bufSize);

    m_lpd3dIdxBuf->Unlock();
    return true;
}

bool CGraphicIndexBuffer::Create(int faceCount, TFace *faces)
{
    int idxCount = faceCount * 3;
    m_iidxCount = idxCount;
    if (!Create(idxCount, D3DFMT_INDEX16))
        return false;

    WORD *dstIndices;
    if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, (void**)&dstIndices, 0)))
        return false;

    for (int i = 0; i < faceCount; ++i, dstIndices += 3)
    {
        TFace *curFace = faces + i;
        dstIndices[0] = curFace->indices[0];
        dstIndices[1] = curFace->indices[1];
        dstIndices[2] = curFace->indices[2];
    }

    m_lpd3dIdxBuf->Unlock();
    return true;
}

bool CGraphicIndexBuffer::CreateDeviceObjects()
{
    const auto hr = ms_lpd3dDevice->CreateIndexBuffer(m_dwBufferSize,
                                                      D3DUSAGE_WRITEONLY,
                                                      m_d3dFmt,
                                                      D3DPOOL_MANAGED,
                                                      &m_lpd3dIdxBuf,
                                                      NULL);
    if (FAILED(hr))
    {
        spdlog::trace("CreateIndexBuffer() failed for "
                      "size {0} fmt {1} with {2}",
                      m_dwBufferSize, m_d3dFmt, hr);
        return false;
    }

    return true;
}

void CGraphicIndexBuffer::DestroyDeviceObjects()
{
    M2_SAFE_RELEASE_CHECK(m_lpd3dIdxBuf);
}

bool CGraphicIndexBuffer::Create(int idxCount, D3DFORMAT d3dFmt)
{
    Destroy();

    m_iidxCount = idxCount;
    m_dwBufferSize = sizeof(WORD) * idxCount;
    m_d3dFmt = d3dFmt;

    return CreateDeviceObjects();
}

void CGraphicIndexBuffer::Destroy()
{
    DestroyDeviceObjects();
}

void CGraphicIndexBuffer::Initialize()
{
    m_lpd3dIdxBuf = NULL;
}

CGraphicIndexBuffer::CGraphicIndexBuffer()
{
    Initialize();
}

CGraphicIndexBuffer::~CGraphicIndexBuffer()
{
    Destroy();
}
