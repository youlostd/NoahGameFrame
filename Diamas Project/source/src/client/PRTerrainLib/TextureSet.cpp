#include "StdAfx.h"
#include "TextureSet.h"

#include "../EterLib/ResourceManager.h"
#include "../EterLib/Util.h"

#include <storm/StringUtil.hpp>

CTextureSet::CTextureSet()
{
    Initialize();
}

CTextureSet::~CTextureSet()
{
    Clear();
}

void CTextureSet::Initialize()
{
}

void CTextureSet::Create()
{
    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>("d:/ymir work/special/error.tga");
    if (!r)
    {
        SPDLOG_ERROR("Failed to load placeholder texture");
        return;
    }

    m_ErrorTexture.ImageInstance.SetImagePointer(r);
    AddEmptyTexture(); // 지우개 텍스춰를 처음에 추가 해야 함
}

bool CTextureSet::Load(const char *c_szTextureSetFileName, float fTerrainTexCoordBase)
{
    Clear();

    CTokenVectorMap stTokenVectorMap;

    if (!LoadMultipleTextData(c_szTextureSetFileName, stTokenVectorMap))
    {
        SPDLOG_ERROR("TextureSet::Load : cannot load {0}", c_szTextureSetFileName);
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("textureset"))
    {
        SPDLOG_ERROR("TextureSet::Load : syntax error, TextureSet (filename: {0})", c_szTextureSetFileName);
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("texturecount"))
    {
        SPDLOG_ERROR("TextureSet::Load : syntax error, TextureCount (filename: {0})", c_szTextureSetFileName);
        return false;
    }

    Create();

    const auto &c_rstrCount = stTokenVectorMap["texturecount"][0];

    long lCount = atol(c_rstrCount.c_str());
    char szTextureName[32 + 1];

    m_Textures.resize(lCount + 1);

    for (long i = 0; i < lCount; ++i)
    {
        _snprintf(szTextureName, sizeof(szTextureName), "texture%03d", i + 1);

        if (stTokenVectorMap.end() == stTokenVectorMap.find(szTextureName))
            continue;

        const CTokenVector &rVector = stTokenVectorMap[szTextureName];
        const auto &c_rstrFileName = rVector[0];
        const auto &c_rstrUScale = rVector[1];
        const auto &c_rstrVScale = rVector[2];
        const auto &c_rstrUOffset = rVector[3];
        const auto &c_rstrVOffset = rVector[4];
        const auto &c_rstrbSplat = rVector[5];
        const auto &c_rstrBegin = rVector[6];
        const auto &c_rstrEnd = rVector[7];

        float fuScale, fvScale, fuOffset, fvOffset;
        uint8_t bSplat;
        unsigned short usBegin, usEnd;

        storm::ParseNumber(c_rstrUScale, fuScale);
        storm::ParseNumber(c_rstrVScale, fvScale);
        storm::ParseNumber(c_rstrUOffset, fuOffset);
        storm::ParseNumber(c_rstrVOffset, fvOffset);
        storm::ParseNumber(c_rstrbSplat, bSplat);
        
        usBegin = static_cast<unsigned short>(atoi(c_rstrBegin.c_str()));
        usEnd = static_cast<unsigned short>(atoi(c_rstrEnd.c_str()));

        if (!SetTexture(i + 1, c_rstrFileName.c_str(), fuScale, fvScale, fuOffset, fvOffset, bSplat, usBegin, usEnd,
                        fTerrainTexCoordBase))
            SPDLOG_ERROR("CTextureSet::Load : SetTexture failed : Filename: {0}", c_rstrFileName.c_str());
    }

    m_stFileName.assign(c_szTextureSetFileName);
    return true;
}

void CTextureSet::Clear()
{
    m_ErrorTexture.ImageInstance.Destroy();
    m_Textures.clear();
    Initialize();
}

void CTextureSet::AddEmptyTexture()
{
    TTerrainTexture eraser;
    m_Textures.push_back(eraser);
}

uint32_t CTextureSet::GetTextureCount()
{
    return m_Textures.size();
}

TTerrainTexture &CTextureSet::GetTexture(uint32_t ulIndex)
{
    if (GetTextureCount() <= ulIndex)
        return m_ErrorTexture;

    return m_Textures[ulIndex];
}

bool CTextureSet::SetTexture(uint32_t ulIndex,
                             const char *c_szFileName,
                             float fuScale,
                             float fvScale,
                             float fuOffset,
                             float fvOffset,
                             bool bSplat,
                             unsigned short usBegin,
                             unsigned short usEnd,
                             float fTerrainTexCoordBase)
{
    if (ulIndex >= m_Textures.size())
    {
        SPDLOG_ERROR("CTextureSet::SetTexture : Index Error : Index({0}) is Larger than TextureSet Size({1})", ulIndex,
                      m_Textures.size());
        return false;
    }

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
    if (!r)
    {
        SPDLOG_ERROR("Failed to load textureset");
        return false;
    }

    TTerrainTexture &tex = m_Textures[ulIndex];

    tex.stFilename = c_szFileName;
    tex.UScale = fuScale;
    tex.VScale = fvScale;
    tex.UOffset = fuOffset;
    tex.VOffset = fvOffset;
    tex.bSplat = bSplat;
    tex.Begin = usBegin;
    tex.End = usEnd;
    tex.ImageInstance.SetImagePointer(r);
    tex.pd3dTexture = tex.ImageInstance.GetTexturePointer()->GetD3DTexture();

    tex.m_matTransform = Matrix::CreateScale(fTerrainTexCoordBase * tex.UScale, -fTerrainTexCoordBase * tex.VScale, 0.0f);
    tex.m_matTransform._41 = tex.UOffset;
    tex.m_matTransform._42 = -tex.VOffset;
    return true;
}

void CTextureSet::Reload(float fTerrainTexCoordBase)
{
    for (uint32_t dwIndex = 1; dwIndex < GetTextureCount(); ++dwIndex)
    {
        TTerrainTexture &tex = m_Textures[dwIndex];

        auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(tex.stFilename.c_str());
        if (!r)
            continue;

        tex.ImageInstance.ReloadImagePointer(r);
        tex.pd3dTexture = tex.ImageInstance.GetTexturePointer()->GetD3DTexture();

        tex.m_matTransform = Matrix::CreateScale(fTerrainTexCoordBase * tex.UScale, -fTerrainTexCoordBase * tex.VScale,
                          0.0f);
        tex.m_matTransform._41 = tex.UOffset;
        tex.m_matTransform._42 = -tex.VOffset;
    }
}

bool CTextureSet::AddTexture(const char *c_szFileName,
                             float fuScale,
                             float fvScale,
                             float fuOffset,
                             float fvOffset,
                             bool bSplat,
                             unsigned short usBegin,
                             unsigned short usEnd,
                             float fTerrainTexCoordBase)
{
    if (GetTextureCount() >= 256)
    {
        SPDLOG_ERROR("Textures can no longer be added. Maximum number of textures: 255");
        return false;
    }

    for (uint32_t i = 1; i < GetTextureCount(); ++i)
    {
        if (0 == m_Textures[i].stFilename.compare(c_szFileName))
        {
            SPDLOG_ERROR("A texture with the same name already exists. overlap");
            return false;
        }
    }

    m_Textures.resize(m_Textures.size() + 1);
    if (!SetTexture(m_Textures.size() - 1,
                    c_szFileName,
                    fuScale,
                    fvScale,
                    fuOffset,
                    fvOffset,
                    bSplat,
                    usBegin,
                    usEnd,
                    fTerrainTexCoordBase))
    {
        m_Textures.resize(m_Textures.size() - 1);
        return false;
    }

    return true;
}

bool CTextureSet::RemoveTexture(uint32_t ulIndex)
{
    if (GetTextureCount() <= ulIndex)
        return false;

    TTextureVector::iterator itor = m_Textures.begin() + ulIndex;
    m_Textures.erase(itor);
    return true;
}

bool CTextureSet::Save(const char *c_pszFileName)
{
    FILE *pFile = fopen(c_pszFileName, "w");

    if (!pFile)
        return false;

    fprintf(pFile, "TextureSet\n");
    fprintf(pFile, "\n");

    fprintf(pFile, "TextureCount %ld\n", GetTextureCount() - 1); // -1 À» ÇÏ´Â ÀÌÀ¯´Â Áö¿ì°³ ¶§¹®ÀÓ
    fprintf(pFile, "\n");

    for (uint32_t i = 1; i < GetTextureCount(); ++i)
    {
        TTerrainTexture &rTex = m_Textures[i];

        fprintf(pFile, "Start Texture%03d\n", i);
        fprintf(pFile, "    \"%s\"\n", rTex.stFilename.c_str());
        fprintf(pFile, "    %f\n", rTex.UScale);
        fprintf(pFile, "    %f\n", rTex.VScale);
        fprintf(pFile, "    %f\n", rTex.UOffset);
        fprintf(pFile, "    %f\n", rTex.VOffset);
        fprintf(pFile, "    %d\n", rTex.bSplat);
        fprintf(pFile, "    %hu\n", rTex.Begin);
        fprintf(pFile, "    %hu\n", rTex.End);
        fprintf(pFile, "End Texture%03d\n", i);
    }

    fclose(pFile);
    return true;
}
