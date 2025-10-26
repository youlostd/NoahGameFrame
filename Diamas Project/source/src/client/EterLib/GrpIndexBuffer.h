#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpBase.h"

class CGraphicIndexBuffer : public CGraphicBase
{
public:
    CGraphicIndexBuffer();
    ~CGraphicIndexBuffer();

    bool Create(int idxCount, D3DFORMAT d3dFmt);
    bool Create(int faceCount, TFace *faces);
    void Destroy();

    bool CreateDeviceObjects();
    void DestroyDeviceObjects();

    bool Copy(int bufSize, const void *srcIndices);

    bool Lock(void **pretIndices, size_t sizeToLock = 0) const;
    void Unlock() const;

    void Bind() const;

    LPDIRECT3DINDEXBUFFER9 GetD3DIndexBuffer() const;

    int GetIndexCount() const
    {
        return m_iidxCount;
    }

protected:
    void Initialize();

    LPDIRECT3DINDEXBUFFER9 m_lpd3dIdxBuf;
    DWORD m_dwBufferSize;
    D3DFORMAT m_d3dFmt;
    int m_iidxCount;
};
