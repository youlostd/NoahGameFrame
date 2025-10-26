#include "GrpShaderFX.h"
#include "StdAfx.h"

#include "Engine.hpp"
#include <pak/Vfs.hpp>

#include <Utils.h>

#include "pak/pak/PakFile.hpp"

#include <Stl.h>

class GrpShaderFileIncluder : public ID3DXInclude
{
  public:
    void setParentFilename(std::string strFilename)
    {
        m_strParentFilename = strFilename;
    }

    GrpShaderFileIncluder() : m_view(storm::GetDefaultAllocator())
    {
    }

  protected:
    STDMETHOD(Open)
    (THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {

        std::string strFilename = CFileNameHelper::GetPath(m_strParentFilename) + pFileName;
        if (m_fp = GetVfs().Open(strFilename, kVfsOpenFullyBuffered); m_fp)
        {

            const auto size = m_fp->GetSize();

            m_fp->GetView(0, m_view, size);

            *ppData = m_view.GetData();
            *pBytes = size;
            return S_OK;
        }
        SPDLOG_ERROR("Failed to include {} in shader {}", strFilename, m_strParentFilename);
        return S_FALSE;
    }
    STDMETHOD(Close)(THIS_ LPCVOID pData)
    {
        return S_OK;
    }

  private:
    std::string m_strParentFilename;
    std::unique_ptr<VfsFile> m_fp;
    storm::View m_view;
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

GrpShaderFX::GrpShaderFX(LPDIRECT3DDEVICE9 pDevice)
    : m_pEffect(NULL), m_pd3dDevice(pDevice), m_pCompiledEffectBuffer(NULL), m_pErrorMsgs(NULL), m_dwShaderFlags(0),
      m_pShaderMacros(NULL), m_bOpenCompiledShaderFile(false), m_pCompiledEffectFileBuffer(NULL),
      m_nCompiledEffectFileBufferSize(0)
{
}

///////////////////////////////////////////////////////////////////////
//  GrpShaderFX::~GrpShaderFX

GrpShaderFX::~GrpShaderFX()
{

    SAFE_DELETE_ARRAY(m_pCompiledEffectFileBuffer);
    SAFE_DELETE_ARRAY(m_pShaderMacros);
    M2_SAFE_RELEASE_CHECK(m_pCompiledEffectBuffer);
    M2_SAFE_RELEASE_CHECK(m_pErrorMsgs);
    M2_SAFE_RELEASE_CHECK(m_pEffect);
}

//////////////////////////////////////////////////////////////////////////
// Resource Loading

// SetFromString 또는 SetFromFile 둘중에 하나로부터 생성
bool GrpShaderFX::SetFromString(const std::string &strShader, const std::vector<std::string> &vShaderDefines,
                                GrpShaderFXPool *pShaderPool, const char *szCachingFile)
{
    if (!m_strFilename.empty())
    {
        _ASSERT(false);
        return false;
    }

    m_strShaderString = strShader;
    m_vShaderDefines = vShaderDefines;
    m_pShaderPool = pShaderPool;

    if (szCachingFile)
    {
        m_strCachingFileName = std::string("shadercache/") + szCachingFile;
    }

    return true;
}

bool GrpShaderFX::SetFromFile(const std::string &strFilename, const std::vector<std::string> &vShaderDefines,
                              GrpShaderFXPool *pShaderPool)
{
    if (!m_strShaderString.empty())
    {
        _ASSERT(false);
        return false;
    }

    m_strFilename = strFilename;
    m_vShaderDefines = vShaderDefines;
    m_pShaderPool = pShaderPool;

    return true;
}

bool GrpShaderFX::Setup()
{
    SetupShaderFlags();
    SetupShaderMacros();

    if (IsSettingFromFile())
    {
        SPDLOG_TRACE("Setting up shader from file");
        if (LoadShaderFile() == false)
        {
            return false;
        }
    }
    else
    {
        if (Engine::GetSettings().IsUsingShaderCache())
        {
            // 조합된 쉐이더인데 만약 캐슁된 컴파일된 쉐이더 파일이 있으면 읽는다.
            LoadCachingFile();
        }
    }

    if (CompileEffect() == false)
    {
        return false;
    }
    Load();

    return true;
}

bool GrpShaderFX::Load()
{
    LPD3DXEFFECTPOOL pEffectPool = (m_pShaderPool != NULL ? m_pShaderPool->GetEffectPool() : NULL);

    GrpShaderFileIncluder shaderFileIncluder;
    shaderFileIncluder.setParentFilename(m_strFilename);

#ifdef DEBUG_VS

    // 파일에서 생성하는 경우에만 디버깅이 가능
    if (m_strFullFilename.size())
    {
        // PIX에서 *.fx 파일 디버깅을 하기 위해 D3DXCreateEffectFromFile 함수로 파일을 읽어 들인다.
        if (FAILED(::D3DXCreateEffectFromFile(m_pd3dDevice, m_strFullFilename.c_str(), m_pShaderMacros,
                                              &shaderFileIncluder, m_dwShaderFlags, pEffectPool, &m_pEffect,
                                              &m_pErrorMsgs)))
        {
            if (m_pErrorMsgs)
            {
                const char *pCompileErrors = (const char *)m_pErrorMsgs->GetBufferPointer();
                SPDLOG_ERROR(pCompileErrors);
                SAFE_RELEASE(m_pErrorMsgs);
            }
            return false;
            _ASSERT(false);
        }
    }
    else
    {
        if (FAILED(D3DXCreateEffect(m_pd3dDevice, m_strShaderString.c_str(), m_strShaderString.length() + 1,
                                    m_pShaderMacros, &shaderFileIncluder, m_dwShaderFlags, pEffectPool, &m_pEffect,
                                    &m_pErrorMsgs)))
        {
            if (m_pErrorMsgs)
            {
                const char *pCompileErrors = (const char *)m_pErrorMsgs->GetBufferPointer();
                SPDLOG_ERROR(pCompileErrors);
                SAFE_RELEASE(m_pErrorMsgs);
            }
            _ASSERT(false);
            SAFE_DELETE_ARRAY(m_pShaderMacros);
            return false;
        }
    }
#else
    LPCVOID pSrcData;
    UINT nSrcDataLen;

    if (m_bOpenCompiledShaderFile)
    {
        pSrcData = m_pCompiledEffectFileBuffer;
        nSrcDataLen = m_nCompiledEffectFileBufferSize;
    }
    else
    {
        pSrcData = m_pCompiledEffectBuffer->GetBufferPointer();
        nSrcDataLen = m_pCompiledEffectBuffer->GetBufferSize();
    }

    if (FAILED(D3DXCreateEffect(m_pd3dDevice, pSrcData, nSrcDataLen, m_pShaderMacros, NULL, m_dwShaderFlags,
                                pEffectPool, &m_pEffect, &m_pErrorMsgs)))
    {
        if (m_pErrorMsgs)
        {
            const char *pCompileErrors = (const char *)m_pErrorMsgs->GetBufferPointer();
            SPDLOG_ERROR(pCompileErrors);
            SAFE_RELEASE(m_pErrorMsgs);
        }
        _ASSERT(false);
        SAFE_DELETE_ARRAY(m_pShaderMacros);
        SAFE_RELEASE(m_pCompiledEffectBuffer);
        SAFE_DELETE_ARRAY(m_pCompiledEffectFileBuffer);
        return false;
    }

#endif

    SAFE_DELETE_ARRAY(m_pShaderMacros);
    SAFE_RELEASE(m_pCompiledEffectBuffer);
    SAFE_DELETE_ARRAY(m_pCompiledEffectFileBuffer);
    return true;
}

GrpTechnique GrpShaderFX::GetTechniqueByIndex(unsigned int index) const
{
    GrpTechnique cTechnique;
    _ASSERT(m_pEffect != NULL);

    if (m_pEffect)
    {
        cTechnique.m_pEffect = m_pEffect;
        cTechnique.m_hTechnique = m_pEffect->GetTechnique(index);
        _ASSERT(cTechnique.m_hTechnique);
    }
    return cTechnique;
}

GrpTechnique GrpShaderFX::GetTechnique(const char *pName) const
{
    GrpTechnique cTechnique;
    _ASSERT(m_pEffect != NULL);

    if (m_pEffect)
    {
        cTechnique.m_pEffect = m_pEffect;
        cTechnique.m_hTechnique = m_pEffect->GetTechniqueByName(pName);
        _ASSERT(cTechnique.m_hTechnique);
    }
    return cTechnique;
}

///////////////////////////////////////////////////////////////////////
//  GrpShaderFX::GetConstant

GrpShaderConstant GrpShaderFX::GetConstant(const char *pName) const
{
    GrpShaderConstant cParameter;

    if (m_pEffect)
    {
        cParameter.m_pEffect = m_pEffect;
        cParameter.m_hParameter = m_pEffect->GetParameterByName(NULL, pName);
        cParameter.m_strName = pName;
        if (!cParameter.m_hParameter)
            SPDLOG_ERROR("Failed to find FX constant [{}]", cParameter.m_strName.c_str());

#ifdef _DEBUG
        D3DXPARAMETER_DESC descParam;
        HRESULT hResult = m_pEffect->GetParameterDesc(cParameter.m_hParameter, &descParam);
        if (SUCCEEDED(hResult))
        {
            _ASSERT(strcmp(descParam.Name, pName) == 0);
        }
        else
        {
            _ASSERT(false);
        }
#endif
    }

    return cParameter;
}

///////////////////////////////////////////////////////////////////////
//  GrpShaderFX::CommitChanges

bool GrpShaderFX::CommitChanges(void) const
{
    bool bSuccess = false;
    bSuccess = SUCCEEDED(m_pEffect->CommitChanges());
    return bSuccess;
}

void GrpShaderFX::OnLostDevice()
{
    if (m_pEffect)
        m_pEffect->OnLostDevice();
}

void GrpShaderFX::OnResetDevice()
{
    if (m_pEffect)
        m_pEffect->OnResetDevice();
}

const char *GrpShaderFX::GetDebugName()
{
    return m_strFilename.c_str();
}

void GrpShaderFX::SetupShaderFlags()
{
    //////////////////////////////////////////////////////////////////////////
    // 플래그등 공통 설정
#ifdef DEBUG_VS
    m_dwShaderFlags |= D3DXSHADER_DEBUG; // | D3DXSHADER_SKIPOPTIMIZATION;
#endif

#ifdef SHADER_USE_LEGACY_D3DX9_31
    m_dwShaderFlags |= D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
#endif
    m_dwShaderFlags |= D3DXSHADER_OPTIMIZATION_LEVEL3; // | D3DXSHADER_SKIPOPTIMIZATION;

    // m_dwShaderFlags |= D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY;
}

void GrpShaderFX::SetupShaderMacros()
{
    // setup shader #defines
    _ASSERT((m_vShaderDefines.size() % 2) == 0); // must be an even number of strings

    // 쉐이더 매크로 설정 --------------------------
    m_pShaderMacros = new D3DXMACRO[m_vShaderDefines.size() / 2 + 1];
    int i = 0;
    for (; i < int(m_vShaderDefines.size()); i += 2)
    {
        m_pShaderMacros[i / 2].Name = m_vShaderDefines[i].c_str();
        m_pShaderMacros[i / 2].Definition = m_vShaderDefines[i + 1].c_str();
    }
    // terminate the shader array
    m_pShaderMacros[i / 2].Name = NULL;
    m_pShaderMacros[i / 2].Definition = NULL;
}

bool GrpShaderFX::LoadShaderFile()
{
    //////////////////////////////////////////////////////////////////////////
    // 파일에서 읽는 것이면 읽어서 m_strShaderString 을 채움
    std::unique_ptr<VfsFile> file;
    if (OpenShaderFile(file) == false)
        return false;

    SPDLOG_TRACE("Opened shader file");

    MakeShaderStringFromFile(file);
    return true;
}

bool GrpShaderFX::OpenShaderFile(std::unique_ptr<VfsFile> &file)
{
    std::string FileNameWithRelativePath;

    /* if (Engine::GetSettings().IsUsingShaderCache())
     {
         std::string strCompiledFileFullPathName;
         std::string strPureFileName = MGetPureFileName(m_strFilename);
         strCompiledFileFullPathName = strPureFileName + COMPILED_SHADER_FILENAME_EXT;

         if (file.Open(strCompiledFileFullPathName.c_str()))
         {
             m_bOpenCompiledShaderFile = true;
             m_strFullFilename = FileNameWithRelativePath;
             return true;
         }
     }
     */
    file = GetVfs().Open(m_strFilename, kVfsOpenFullyBuffered);
    if (file)
    {
        // m_strFullFilename = FileNameWithRelativePath;
        return true;
    }

    SPDLOG_ERROR("{} shader file open error", m_strFilename.c_str());
    return false;
}

void GrpShaderFX::MakeShaderStringFromFile(std::unique_ptr<VfsFile> &file)
{
    unsigned int size = file->GetSize();

    if (m_bOpenCompiledShaderFile)
    {
        m_pCompiledEffectFileBuffer = new char[size + 1];
        m_nCompiledEffectFileBufferSize = size;
        file->Read(0, m_pCompiledEffectFileBuffer, size);
    }
    else
    {
        m_strShaderString.resize(size + 1);
        file->Read(0, &m_strShaderString[0], size);
        m_strShaderString[size] = '\0';
    }
}

bool GrpShaderFX::CompileEffect()
{
    // 이미 컴파일된 쉐이더로 읽었으면 컴파일하지 않는다.
    if (m_bOpenCompiledShaderFile == true)
        return true;

    //////////////////////////////////////////////////////////////////////////
    // compile effect
    GrpShaderFileIncluder shaderFileIncluder;
    shaderFileIncluder.setParentFilename(m_strFilename);

    LPD3DXEFFECTCOMPILER pEffectCompiler;
    if (S_OK != D3DXCreateEffectCompiler(m_strShaderString.c_str(), m_strShaderString.length() + 1, m_pShaderMacros,
                                         &shaderFileIncluder, m_dwShaderFlags, &pEffectCompiler, &m_pErrorMsgs))
    {
        if (m_pErrorMsgs)
        {
            const char *pCompileErrors = (const char *)m_pErrorMsgs->GetBufferPointer();
            SPDLOG_ERROR(pCompileErrors);
            SAFE_RELEASE(m_pErrorMsgs);
        }

        FILE *fp = fopen("create_error.fx", "wt");
        if (fp)
        {
            fprintf(fp, "%s\n", m_strShaderString.c_str());
            fclose(fp);
        }

        SPDLOG_ERROR("Failed to compile model shader");

        return false;
    }

    if (S_OK != pEffectCompiler->CompileEffect(m_dwShaderFlags, &m_pCompiledEffectBuffer, &m_pErrorMsgs))
    {
        if (m_pErrorMsgs)
        {
            const char *pCompileErrors = (const char *)m_pErrorMsgs->GetBufferPointer();
            SPDLOG_ERROR(pCompileErrors);
            SAFE_RELEASE(m_pErrorMsgs);
        }

        SPDLOG_ERROR("Failed to compile model shader");

        FILE *fp = fopen("compile_error.fx", "wt");
        if (fp)
        {
            fprintf(fp, "%s\n", m_strShaderString.c_str());
            fclose(fp);
        }

        SAFE_RELEASE(pEffectCompiler);
        return false;
    }

    SAFE_RELEASE(pEffectCompiler);

    // 컴파일한 것을 캐쉬 파일로 저장한다.
    SaveCacheFile();
    SPDLOG_TRACE("Shader compiled successfully");
    return true;
}

bool GrpShaderFX::IsSettingFromFile()
{
    return (!m_strFilename.empty());
}

void GrpShaderFX::LoadCachingFile()
{
    m_bOpenCompiledShaderFile = false;

    std::string shaderName = CFileNameHelper::NoExtension(m_strFilename) + ".fxo";

    std::unique_ptr<VfsFile> fp;
    auto opened = OpenShaderFile(fp);
    if(!opened)
        return;
    // std::string strPureName = MGetPureFileName(m_strCachingFileName);
    // std::string strFullFileName =
    //    REngine::GetConfig().m_strCompiledShaderCachePath + strPureName + COMPILED_SHADER_FILENAME_EXT;
    /*
     *
    // 파일이 없으면 리턴
    MFile file;
    file.SetAllowAccessUncompFile(true);
    if (file.Open(strFullFileName.c_str(), true) == false)
    {
        return;
    }
  */
    // 캐시 파일을 읽는다.
    m_bOpenCompiledShaderFile = true;
    MakeShaderStringFromFile(fp);
       
}

void GrpShaderFX::SaveCacheFile()
{
    /* if (REngine::GetConfig().m_bUsingCompiledShaderCaching == false)
         return;
     if (m_strCachingFileName.empty())
         return;

     string strPureName = MGetPureFileName(m_strCachingFileName);
     string strFullFileName =
         REngine::GetConfig().m_strCompiledShaderCachePath + strPureName + COMPILED_SHADER_FILENAME_EXT;

     FILE *file;
     file = fopen(strFullFileName.c_str(), "wb");
     if (file)
     {
         fwrite(m_pCompiledEffectBuffer->GetBufferPointer(), 1, m_pCompiledEffectBuffer->GetBufferSize(), file);
         fclose(file);
     }
     else
     {
         errno_t err;
         _get_errno(&err);
         mlog("셰이더 캐시 파일 저장 실패. 에러코드 - %d\n", err);
         _ASSERT(false);
     }

 */
}

//////////////////////////////////////////////////////////////////////////
//

GrpShaderFXPool::GrpShaderFXPool()
{
    HRESULT hr = D3DXCreateEffectPool(&m_pEffectPool);
    _ASSERT(SUCCEEDED(hr));
}

GrpShaderFXPool::~GrpShaderFXPool()
{
    SAFE_RELEASE(m_pEffectPool);
}

LPD3DXEFFECTPOOL GrpShaderFXPool::GetEffectPool()
{
    return m_pEffectPool;
}
