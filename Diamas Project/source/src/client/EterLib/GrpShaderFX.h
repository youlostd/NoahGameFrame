#pragma once

#include "GrpShaderConstant.h"
#include "GrpTechnique.h"
#include "Resource.h"
#include "pak/VfsFile.hpp"

class GrpShaderFXPool;

class GrpShaderFX
{
  public:
    GrpShaderFX(LPDIRECT3DDEVICE9 pDevice);
    virtual ~GrpShaderFX();

    bool SetFromString(const std::string &strShader, const std::vector<std::string> &vShaderDefines,
                       GrpShaderFXPool *pShaderPool, const char *szCachingFile = NULL);
    bool SetFromFile(const std::string &strFilename, const std::vector<std::string> &vShaderDefines,
                     GrpShaderFXPool *pShaderPool);
    bool Setup();
    bool Load();
    GrpTechnique GetTechniqueByIndex(unsigned index) const;
    GrpTechnique GetTechnique(const char *pName) const;
    GrpShaderConstant GetConstant(const char *pName) const;

    bool CommitChanges(void) const;

    LPDIRECT3DDEVICE9 m_pd3dDevice;
    LPD3DXEFFECT m_pEffect;

    void OnLostDevice();
    void OnResetDevice();

    virtual const char *GetDebugName();

  protected:
    void SaveCacheFile();
    bool LoadShaderFile();
    void LoadCachingFile();
    void SetupShaderMacros();
    void SetupShaderFlags();

    bool OpenShaderFile(std::unique_ptr<VfsFile> &file);

    std::string m_strFilename;
    std::string m_strFullFilename;
    std::string m_strShaderString;
    std::vector<std::string> m_vShaderDefines;
    GrpShaderFXPool *m_pShaderPool;

    LPD3DXBUFFER m_pCompiledEffectBuffer;
    LPD3DXBUFFER m_pErrorMsgs;
    D3DXMACRO *m_pShaderMacros;
    DWORD m_dwShaderFlags;

    // Compiled
    bool m_bOpenCompiledShaderFile;
    char *m_pCompiledEffectFileBuffer;
    unsigned int m_nCompiledEffectFileBufferSize;
    std::string m_strCachingFileName;

    void MakeShaderStringFromFile(std::unique_ptr<VfsFile> &file);

    bool CompileEffect();
    bool IsSettingFromFile();
};

class GrpShaderFXPool
{
  protected:
    LPD3DXEFFECTPOOL m_pEffectPool;

  public:
    GrpShaderFXPool();
    ~GrpShaderFXPool();

    LPD3DXEFFECTPOOL GetEffectPool();
};
