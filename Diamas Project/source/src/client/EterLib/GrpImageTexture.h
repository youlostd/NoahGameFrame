#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpTexture.h"

class CGraphicImageTexture : public CGraphicTexture
{
public:
    CGraphicImageTexture();
    ~CGraphicImageTexture() override;

    void Destroy() override;

    bool Create(UINT width, UINT height, D3DFORMAT d3dFmt);
    bool CreateDeviceObjects();

    void CreateFromTexturePointer(const CGraphicTexture *c_pSrcTexture);
    bool CreateWithStbImage(UINT bufSize, const void *c_pvBuf);
    bool CreateFromDiskFile(const char *c_szFileName, D3DFORMAT d3dFmt);
    bool CreateFromMemoryFile(UINT bufSize, const void *c_pvBuf);

    void SetFileName(const char *c_szFileName);

    bool Lock(int *pRetPitch, void **ppRetPixels, int level = 0);
    void Unlock(int level = 0);

protected:
    void Initialize() override;

    D3DFORMAT m_d3dFmt;

    std::string m_stFileName;
};
