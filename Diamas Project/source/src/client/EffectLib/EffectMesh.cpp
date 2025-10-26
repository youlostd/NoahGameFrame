#include "EffectMesh.h"
#include "StdAfx.h"

#include <storm/io/View.hpp>

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
CDynamicPool<CEffectMesh::SEffectMeshData> CEffectMesh::SEffectMeshData::ms_kPool;

CEffectMesh::SEffectMeshData *CEffectMesh::SEffectMeshData::New()
{
    return ms_kPool.Alloc();
}

void CEffectMesh::SEffectMeshData::Delete(SEffectMeshData *pkData)
{
    pkData->EffectFrameDataVector.clear();
    pkData->pImageVector.clear();

    ms_kPool.Free(pkData);
}

void CEffectMesh::SEffectMeshData::DestroySystem()
{
    ms_kPool.Destroy();
}

uint32_t CEffectMesh::GetFrameCount()
{
    return m_iFrameCount;
}

uint32_t CEffectMesh::GetMeshCount()
{
    return m_pEffectMeshDataVector.size();
}

CEffectMesh::TEffectMeshData *CEffectMesh::GetMeshDataPointer(uint32_t dwMeshIndex)
{
    assert(dwMeshIndex < m_pEffectMeshDataVector.size());
    return m_pEffectMeshDataVector[dwMeshIndex];
}

std::vector<CGraphicImage::Ptr> *CEffectMesh::GetTextureVectorPointer(uint32_t dwMeshIndex)
{
    if (dwMeshIndex >= m_pEffectMeshDataVector.size())
        return NULL;

    return &m_pEffectMeshDataVector[dwMeshIndex]->pImageVector;
}

std::vector<CGraphicImage::Ptr> &CEffectMesh::GetTextureVectorReference(uint32_t dwMeshIndex)
{
    return m_pEffectMeshDataVector[dwMeshIndex]->pImageVector;
}

bool CEffectMesh::Load(int iSize, const void *c_pvBuf)
{
    if (!c_pvBuf)
        return false;

    const auto *c_pbBuf = static_cast<const uint8_t *>(c_pvBuf);

    char szHeader[10 + 1];
    memcpy(szHeader, c_pbBuf, 10 + 1);
    c_pbBuf += 10 + 1;

    if (0 == strcmp("EffectData", szHeader))
    {
        if (!__LoadData_Ver001(iSize, c_pbBuf))
            return false;
    }
    else if (0 == strcmp("MDEData002", szHeader))
    {
        if (!__LoadData_Ver002(iSize, c_pbBuf))
            return false;
    }
    else
    {
        return false;
    }

    m_isData = true;
    return true;
}

bool CEffectMesh::__LoadData_Ver002(int iSize, const uint8_t *c_pbBuf)
{
    std::vector<Vector3> v3VertexVector;
    std::vector<int> iIndexVector;
    std::vector<Vector2> v3TextureVertexVector;
    std::vector<int> iTextureIndexVector;

    m_iGeomCount = *(int *)c_pbBuf;
    c_pbBuf += 4;
    m_iFrameCount = *(int *)c_pbBuf;
    c_pbBuf += 4;

    m_pEffectMeshDataVector.clear();
    m_pEffectMeshDataVector.resize(m_iGeomCount);

    for (int n = 0; n < m_iGeomCount; ++n)
    {
        SEffectMeshData *pMeshData = SEffectMeshData::New();

        memcpy(pMeshData->szObjectName, c_pbBuf, 32);
        c_pbBuf += 32;
        memcpy(pMeshData->szDiffuseMapFileName, c_pbBuf, 128);
        c_pbBuf += 128;

        pMeshData->EffectFrameDataVector.clear();
        pMeshData->EffectFrameDataVector.resize(m_iFrameCount);

        for (int i = 0; i < m_iFrameCount; ++i)
        {
            TEffectFrameData &rFrameData = pMeshData->EffectFrameDataVector[i];

            memcpy(&rFrameData.byChangedFrame, c_pbBuf, sizeof(uint8_t));
            c_pbBuf += sizeof(uint8_t);

            memcpy(&rFrameData.fVisibility, c_pbBuf, sizeof(float));
            c_pbBuf += sizeof(float);

            memcpy(&rFrameData.dwVertexCount, c_pbBuf, sizeof(uint32_t));
            c_pbBuf += sizeof(uint32_t);

            memcpy(&rFrameData.dwIndexCount, c_pbBuf, sizeof(uint32_t));
            c_pbBuf += sizeof(uint32_t);

            memcpy(&rFrameData.dwTextureVertexCount, c_pbBuf, sizeof(uint32_t));
            c_pbBuf += sizeof(uint32_t);

            v3VertexVector.clear();
            v3VertexVector.resize(rFrameData.dwVertexCount);
            iIndexVector.clear();
            iIndexVector.resize(rFrameData.dwIndexCount);
            v3TextureVertexVector.clear();
            v3TextureVertexVector.resize(rFrameData.dwTextureVertexCount);
            iTextureIndexVector.clear();
            iTextureIndexVector.resize(rFrameData.dwIndexCount);

            memcpy(&v3VertexVector[0], c_pbBuf, rFrameData.dwVertexCount * sizeof(Vector3));
            c_pbBuf += rFrameData.dwVertexCount * sizeof(Vector3);
            memcpy(&iIndexVector[0], c_pbBuf, rFrameData.dwIndexCount * sizeof(int));
            c_pbBuf += rFrameData.dwIndexCount * sizeof(int);
            memcpy(&v3TextureVertexVector[0], c_pbBuf, rFrameData.dwTextureVertexCount * sizeof(Vector2));
            c_pbBuf += rFrameData.dwTextureVertexCount * sizeof(Vector2);
            memcpy(&iTextureIndexVector[0], c_pbBuf, rFrameData.dwIndexCount * sizeof(int));
            c_pbBuf += rFrameData.dwIndexCount * sizeof(int);

            ///////////////////////////////

            rFrameData.PTVertexVector.clear();
            rFrameData.PTVertexVector.resize(rFrameData.dwIndexCount);
            for (uint32_t j = 0; j < rFrameData.dwIndexCount; ++j)
            {
                TPTVertex &rVertex = rFrameData.PTVertexVector[j];

                uint32_t dwIndex = iIndexVector[j];
                uint32_t dwTextureIndex = iTextureIndexVector[j];

                assert(dwIndex < v3VertexVector.size());
                assert(dwTextureIndex < v3TextureVertexVector.size());

                rVertex.position = v3VertexVector[dwIndex];
                rVertex.texCoord = v3TextureVertexVector[dwTextureIndex];
                rVertex.texCoord.y *= -1;
            }
        }

        ////////////////////////////////////

        pMeshData->pImageVector.clear();

        std::string strExtension;
        GetFileExtension(pMeshData->szDiffuseMapFileName, strlen(pMeshData->szDiffuseMapFileName), &strExtension);
        stl_lowers(strExtension);

        if ("ifl" == strExtension)
        {
            auto File = GetVfs().Open(pMeshData->szDiffuseMapFileName, kVfsOpenFullyBuffered);

            if (File)
            {
                const auto size = File->GetSize();

                storm::View text(storm::GetDefaultAllocator());
                File->GetView(0, text, size);

                CMemoryTextFileLoader textFileLoader;
                textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(text.GetData()), size));

                std::string strPathName;
                GetOnlyPathName(pMeshData->szDiffuseMapFileName, strPathName);

                std::string strTextureFileName;
                for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
                {
                    const std::string &c_rstrFileName = textFileLoader.GetLineString(i);

                    if (c_rstrFileName.empty())
                        continue;

                    strTextureFileName = strPathName;
                    strTextureFileName += c_rstrFileName;

                    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(strTextureFileName);
                    if (!r)
                    {
                        SPDLOG_ERROR("Failed to load texture {0} referenced by {1}", strTextureFileName,
                                     pMeshData->szDiffuseMapFileName);
                        return false;
                    }

                    pMeshData->pImageVector.push_back(r);
                }
            }
        }
        else
        {
            auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(pMeshData->szDiffuseMapFileName);
            if (!r)
            {
                SPDLOG_ERROR("Failed to load texture {0}", pMeshData->szDiffuseMapFileName);
                return false;
            }

            pMeshData->pImageVector.push_back(r);
        }

        ////////////////////////////////////

        m_pEffectMeshDataVector[n] = pMeshData;
    }

    return TRUE;
}

bool CEffectMesh::__LoadData_Ver001(int iSize, const uint8_t *c_pbBuf)
{
    std::vector<Vector3> v3VertexVector;
    std::vector<int> iIndexVector;
    std::vector<Vector2> v3TextureVertexVector;
    std::vector<int> iTextureIndexVector;

    m_iGeomCount = *(int *)c_pbBuf;
    c_pbBuf += 4;
    m_iFrameCount = *(int *)c_pbBuf;
    c_pbBuf += 4;

    m_pEffectMeshDataVector.clear();
    m_pEffectMeshDataVector.resize(m_iGeomCount);

    for (int n = 0; n < m_iGeomCount; ++n)
    {
        SEffectMeshData *pMeshData = SEffectMeshData::New();

        memcpy(pMeshData->szObjectName, c_pbBuf, 32);
        c_pbBuf += 32;
        memcpy(pMeshData->szDiffuseMapFileName, c_pbBuf, 128);
        c_pbBuf += 128;

        //

        uint32_t dwVertexCount;
        uint32_t dwIndexCount;
        uint32_t dwTextureVertexCount;

        memcpy(&dwVertexCount, c_pbBuf, sizeof(uint32_t));
        c_pbBuf += sizeof(uint32_t);

        memcpy(&dwIndexCount, c_pbBuf, sizeof(uint32_t));
        c_pbBuf += sizeof(uint32_t);

        memcpy(&dwTextureVertexCount, c_pbBuf, sizeof(uint32_t));
        c_pbBuf += sizeof(uint32_t);

        pMeshData->EffectFrameDataVector.clear();
        pMeshData->EffectFrameDataVector.resize(m_iFrameCount);

        for (int i = 0; i < m_iFrameCount; ++i)
        {
            TEffectFrameData &rFrameData = pMeshData->EffectFrameDataVector[i];

            rFrameData.dwVertexCount = dwVertexCount;
            rFrameData.dwIndexCount = dwIndexCount;
            rFrameData.dwTextureVertexCount = dwTextureVertexCount;

            v3VertexVector.clear();
            v3VertexVector.resize(rFrameData.dwVertexCount);
            iIndexVector.clear();
            iIndexVector.resize(rFrameData.dwIndexCount);
            v3TextureVertexVector.clear();
            v3TextureVertexVector.resize(rFrameData.dwTextureVertexCount);
            iTextureIndexVector.clear();
            iTextureIndexVector.resize(rFrameData.dwIndexCount);

            memcpy(&rFrameData.fVisibility, c_pbBuf, sizeof(float));
            c_pbBuf += sizeof(float);
            memcpy(&v3VertexVector[0], c_pbBuf, rFrameData.dwVertexCount * sizeof(Vector3));
            c_pbBuf += rFrameData.dwVertexCount * sizeof(Vector3);
            memcpy(&iIndexVector[0], c_pbBuf, rFrameData.dwIndexCount * sizeof(int));
            c_pbBuf += rFrameData.dwIndexCount * sizeof(int);
            memcpy(&v3TextureVertexVector[0], c_pbBuf, rFrameData.dwTextureVertexCount * sizeof(Vector2));
            c_pbBuf += rFrameData.dwTextureVertexCount * sizeof(Vector2);
            memcpy(&iTextureIndexVector[0], c_pbBuf, rFrameData.dwIndexCount * sizeof(int));
            c_pbBuf += rFrameData.dwIndexCount * sizeof(int);

            ///////////////////////////////

            rFrameData.PTVertexVector.clear();
            rFrameData.PTVertexVector.resize(rFrameData.dwIndexCount);
            for (uint32_t j = 0; j < rFrameData.dwIndexCount; ++j)
            {
                TPTVertex &rVertex = rFrameData.PTVertexVector[j];

                uint32_t dwIndex = iIndexVector[j];
                uint32_t dwTextureIndex = iTextureIndexVector[j];

                assert(dwIndex < v3VertexVector.size());
                assert(dwTextureIndex < v3TextureVertexVector.size());

                rVertex.position = v3VertexVector[dwIndex];
                rVertex.texCoord = v3TextureVertexVector[dwTextureIndex];
                rVertex.texCoord.y *= -1;
            }
        }

        ////////////////////////////////////

        pMeshData->pImageVector.clear();

        std::string strExtension;
        GetFileExtension(pMeshData->szDiffuseMapFileName, strlen(pMeshData->szDiffuseMapFileName), &strExtension);
        stl_lowers(strExtension);

        if ("ifl" == strExtension)
        {
            auto File = GetVfs().Open(pMeshData->szDiffuseMapFileName, kVfsOpenFullyBuffered);

            if (File)
            {
                const auto size = File->GetSize();

                storm::View text(storm::GetDefaultAllocator());
                File->GetView(0, text, size);

                CMemoryTextFileLoader textFileLoader;

                textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(text.GetData()), size));

                std::string strPathName;
                GetOnlyPathName(pMeshData->szDiffuseMapFileName, strPathName);

                std::string strTextureFileName;
                for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
                {
                    const std::string &c_rstrFileName = textFileLoader.GetLineString(i);

                    if (c_rstrFileName.empty())
                        continue;

                    strTextureFileName = strPathName;
                    strTextureFileName += c_rstrFileName;

                    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(strTextureFileName);
                    if (!r)
                    {
                        SPDLOG_ERROR("Failed to load texture {0} referenced by {1}", strTextureFileName,
                                     pMeshData->szDiffuseMapFileName);
                        return false;
                    }

                    pMeshData->pImageVector.push_back(r);
                }
            }
        }
        else
        {
            auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(pMeshData->szDiffuseMapFileName);
            if (!r)
            {
                SPDLOG_WARN("Failed to load Mesh texture {0}", pMeshData->szDiffuseMapFileName);
                r = CResourceManager::Instance().LoadResource<CGraphicImage>(CFileNameHelper::NoExtension(pMeshData->szDiffuseMapFileName) + ".dds");
                if (!r)
                {
                    SPDLOG_ERROR("Failed to load Mesh texture {0}", pMeshData->szDiffuseMapFileName);
                    return false;
                }

            }

            pMeshData->pImageVector.push_back(r);
        }

        ////////////////////////////////////

        m_pEffectMeshDataVector[n] = pMeshData;
    }

    return TRUE;
}

bool CEffectMesh::GetMeshElementPointer(uint32_t dwMeshIndex, TEffectMeshData **ppMeshData)
{
    if (dwMeshIndex >= m_pEffectMeshDataVector.size())
        return FALSE;

    *ppMeshData = m_pEffectMeshDataVector[dwMeshIndex];

    return TRUE;
}

CEffectMesh::CEffectMesh()
{
    m_iGeomCount = 0;
    m_iFrameCount = 0;
    m_isData = false;
}

CEffectMesh::~CEffectMesh()
{
    if (!m_isData)
        return;

    for (uint32_t i = 0; i < m_pEffectMeshDataVector.size(); ++i)
    {
        m_pEffectMeshDataVector[i]->pImageVector.clear();
        m_pEffectMeshDataVector[i]->EffectFrameDataVector.clear();

        SEffectMeshData::Delete(m_pEffectMeshDataVector[i]);
    }
    m_pEffectMeshDataVector.clear();

    m_isData = false;
}

void CEffectMesh::Clear()
{
    if (!m_isData)
        return;

    for (uint32_t i = 0; i < m_pEffectMeshDataVector.size(); ++i)
    {
        m_pEffectMeshDataVector[i]->pImageVector.clear();
        m_pEffectMeshDataVector[i]->EffectFrameDataVector.clear();

        SEffectMeshData::Delete(m_pEffectMeshDataVector[i]);
    }
    m_pEffectMeshDataVector.clear();

    m_isData = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

CDynamicPool<CEffectMeshScript> CEffectMeshScript::ms_kPool;

void CEffectMeshScript::DestroySystem()
{
    ms_kPool.Destroy();
}

CEffectMeshScript *CEffectMeshScript::New()
{
    return ms_kPool.Alloc();
}

void CEffectMeshScript::Delete(CEffectMeshScript *pkData)
{
    pkData->Clear();
    ms_kPool.Free(pkData);
}

void CEffectMeshScript::ReserveMeshData(uint32_t dwMeshCount)
{
    if (m_MeshDataVector.size() == dwMeshCount)
        return;

    m_MeshDataVector.clear();
    m_MeshDataVector.resize(dwMeshCount);

    for (uint32_t i = 0; i < m_MeshDataVector.size(); ++i)
    {
        TMeshData &rMeshData = m_MeshDataVector[i];

        rMeshData.byBillboardType = MESH_BILLBOARD_TYPE_NONE;
        rMeshData.bBlendingEnable = TRUE;
        rMeshData.byBlendingSrcType = D3DBLEND_SRCCOLOR;
        rMeshData.byBlendingDestType = D3DBLEND_ONE;
        rMeshData.bTextureAlphaEnable = FALSE;

        rMeshData.byColorOperationType = D3DTOP_MODULATE;
        rMeshData.ColorFactor = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);

        rMeshData.bTextureAnimationLoopEnable = true;
        rMeshData.fTextureAnimationFrameDelay = 0.02f;
        rMeshData.dwTextureAnimationStartFrame = 0;
    }
}

const char *CEffectMeshScript::GetMeshFileName()
{
    return m_strMeshFileName.c_str();
}

bool CEffectMeshScript::CheckMeshIndex(uint32_t dwMeshIndex)
{
    if (dwMeshIndex >= m_MeshDataVector.size())
        return false;

    return true;
}

bool CEffectMeshScript::GetMeshDataPointer(uint32_t dwMeshIndex, TMeshData **ppMeshData)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return false;

    *ppMeshData = &m_MeshDataVector[dwMeshIndex];

    return true;
}

int CEffectMeshScript::GetMeshDataCount()
{
    return m_MeshDataVector.size();
}

int CEffectMeshScript::GetBillboardType(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return 0;

    return m_MeshDataVector[dwMeshIndex].byBillboardType;
}

bool CEffectMeshScript::isBlendingEnable(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return FALSE;

    return m_MeshDataVector[dwMeshIndex].bBlendingEnable;
}

uint8_t CEffectMeshScript::GetBlendingSrcType(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return false;

    return m_MeshDataVector[dwMeshIndex].byBlendingSrcType;
}

uint8_t CEffectMeshScript::GetBlendingDestType(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return false;

    return m_MeshDataVector[dwMeshIndex].byBlendingDestType;
}

bool CEffectMeshScript::isTextureAlphaEnable(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return false;

    return m_MeshDataVector[dwMeshIndex].bTextureAlphaEnable;
}

bool CEffectMeshScript::GetColorOperationType(uint32_t dwMeshIndex, uint8_t *pbyType)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return FALSE;

    *pbyType = m_MeshDataVector[dwMeshIndex].byColorOperationType;

    return TRUE;
}

bool CEffectMeshScript::GetColorFactor(uint32_t dwMeshIndex, DirectX::SimpleMath::Color *pColor)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return FALSE;

    *pColor = m_MeshDataVector[dwMeshIndex].ColorFactor;

    return TRUE;
}

bool CEffectMeshScript::GetTimeTableAlphaPointer(uint32_t dwMeshIndex, TTimeEventTableFloat **pTimeEventAlpha)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return FALSE;

    *pTimeEventAlpha = &m_MeshDataVector[dwMeshIndex].TimeEventAlpha;

    return TRUE;
}

bool CEffectMeshScript::isMeshAnimationLoop()
{
    return m_isMeshAnimationLoop;
}

int CEffectMeshScript::GetMeshAnimationLoopCount()
{
    return m_iMeshAnimationLoopCount;
}

float CEffectMeshScript::GetMeshAnimationFrameDelay()
{
    return m_fMeshAnimationFrameDelay;
}

bool CEffectMeshScript::isTextureAnimationLoop(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return false;

    return m_MeshDataVector[dwMeshIndex].bTextureAnimationLoopEnable;
}

float CEffectMeshScript::GetTextureAnimationFrameDelay(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return 0.0f;

    return m_MeshDataVector[dwMeshIndex].fTextureAnimationFrameDelay;
}

uint32_t CEffectMeshScript::GetTextureAnimationStartFrame(uint32_t dwMeshIndex)
{
    if (!CheckMeshIndex(dwMeshIndex))
        return 0;

    return m_MeshDataVector[dwMeshIndex].dwTextureAnimationStartFrame;
}

bool CEffectMeshScript::OnLoadScript(CTextFileLoader &rTextFileLoader)
{
    if (rTextFileLoader.GetTokenString("meshfilename", &m_strMeshFileName))
    {
        if (!IsGlobalFileName(m_strMeshFileName.c_str()))
        {
            m_strMeshFileName = GetOnlyPathName(rTextFileLoader.GetFileName()) + m_strMeshFileName;
        }
    }
    else
    {
        return FALSE;
    }

    if (!rTextFileLoader.GetTokenInteger("meshanimationloopenable", &m_isMeshAnimationLoop))
        return FALSE;
    if (!rTextFileLoader.GetTokenInteger("meshanimationloopcount", &m_iMeshAnimationLoopCount))
    {
        m_iMeshAnimationLoopCount = 0;
    }
    if (!rTextFileLoader.GetTokenFloat("meshanimationframedelay", &m_fMeshAnimationFrameDelay))
        return FALSE;

    uint32_t dwMeshElementCount;
    if (!rTextFileLoader.GetTokenDoubleWord("meshelementcount", &dwMeshElementCount))
        return FALSE;

    m_MeshDataVector.clear();
    m_MeshDataVector.resize(dwMeshElementCount);
    for (uint32_t i = 0; i < m_MeshDataVector.size(); ++i)
    {
        CTextFileLoader::CGotoChild GotoChild(&rTextFileLoader, i);

        TMeshData &rMeshData = m_MeshDataVector[i];

        if (!rTextFileLoader.GetTokenByte("billboardtype", &rMeshData.byBillboardType))
            return FALSE;
        if (!rTextFileLoader.GetTokenBoolean("blendingenable", &rMeshData.bBlendingEnable))
            return FALSE;
        if (!rTextFileLoader.GetTokenByte("blendingsrctype", &rMeshData.byBlendingSrcType))
            return FALSE;
        if (!rTextFileLoader.GetTokenByte("blendingdesttype", &rMeshData.byBlendingDestType))
            return FALSE;

        if (!rTextFileLoader.GetTokenBoolean("textureanimationloopenable", &rMeshData.bTextureAnimationLoopEnable))
            return FALSE;
        if (!rTextFileLoader.GetTokenFloat("textureanimationframedelay", &rMeshData.fTextureAnimationFrameDelay))
            return FALSE;
        if (!rTextFileLoader.GetTokenDoubleWord("textureanimationstartframe", &rMeshData.dwTextureAnimationStartFrame))
        {
            rMeshData.dwTextureAnimationStartFrame = 0;
        }

        if (!rTextFileLoader.GetTokenByte("coloroperationtype", &rMeshData.byColorOperationType))
        {
            rMeshData.byColorOperationType = D3DTOP_MODULATE;
        }
        if (!rTextFileLoader.GetTokenColor("colorfactor", &rMeshData.ColorFactor))
        {
            rMeshData.ColorFactor = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
        }

        if (!GetTokenTimeEventFloat(rTextFileLoader, "timeeventalpha", &rMeshData.TimeEventAlpha))
        {
            rMeshData.TimeEventAlpha.clear();
        }
    }

    return TRUE;
}

bool CEffectMeshScript::OnIsData()
{
    if (0 == m_strMeshFileName.length())
        return false;

    return true;
}

void CEffectMeshScript::OnClear()
{
    m_isMeshAnimationLoop = false;
    m_iMeshAnimationLoopCount = 0;
    m_fMeshAnimationFrameDelay = 0.02f;

    m_MeshDataVector.clear();
    m_strMeshFileName = "";
}

CEffectMeshScript::CEffectMeshScript()
{
}

CEffectMeshScript::~CEffectMeshScript()
{
}
