#include "StdAfx.h"
#include "Pool.h"
#include "TextFileLoader.h"

#include <string>
#include "../EterBase/CRC32.h"
#include <pak/Vfs.hpp>
#include "../EterBase/MappedFile.h"
#include <storm/StringUtil.hpp>

#include <base/Crc32.hpp>

#include "pak/Util.hpp"

std::map<uint32_t, CTextFileLoader *> CTextFileLoader::ms_kMap_dwNameKey_pkTextFileLoader;
bool CTextFileLoader::ms_isCacheMode = false;

CDynamicPool<CTextFileLoader::SGroupNode> CTextFileLoader::SGroupNode::ms_kPool;

CTokenVector *CTextFileLoader::SGroupNode::GetTokenVector(std::string_view c_rstGroupName)
{
    uint32_t dwGroupNameKey = ComputeCrc32(0, c_rstGroupName.data(), c_rstGroupName.length());

    const auto f = m_kMap_dwKey_kVct_stToken.find(dwGroupNameKey);
    if (m_kMap_dwKey_kVct_stToken.end() == f)
        return NULL;

    return &f->second;
}

bool CTextFileLoader::SGroupNode::IsExistTokenVector(std::string_view c_rstGroupName)
{
    uint32_t dwGroupNameKey = ComputeCrc32(0, c_rstGroupName.data(), c_rstGroupName.length());

    if (m_kMap_dwKey_kVct_stToken.end() == m_kMap_dwKey_kVct_stToken.find(dwGroupNameKey))
        return false;

    return true;
}

void CTextFileLoader::SGroupNode::InsertTokenVector(const std::string &c_rstGroupName,
                                                    const CTokenVector &c_rkVct_stToken)
{
    uint32_t dwGroupNameKey = ComputeCrc32(0, c_rstGroupName.c_str(), c_rstGroupName.length());

    m_kMap_dwKey_kVct_stToken.insert(std::make_pair(dwGroupNameKey, c_rkVct_stToken));
}

const std::string &CTextFileLoader::SGroupNode::GetGroupName()
{
    return m_strGroupName;
}

bool CTextFileLoader::SGroupNode::IsGroupNameKey(uint32_t dwGroupNameKey)
{
    if (dwGroupNameKey == m_dwGroupNameKey)
        return true;

    return false;
}

void CTextFileLoader::SGroupNode::SetGroupName(const std::string &c_rstGroupName)
{
    m_strGroupName = c_rstGroupName;
    stl_lowers(m_strGroupName);

    m_dwGroupNameKey = ComputeCrc32(0, m_strGroupName.c_str(), m_strGroupName.length());
}

CTextFileLoader::SGroupNode *CTextFileLoader::SGroupNode::New()
{
    return ms_kPool.Alloc();
}

void CTextFileLoader::SGroupNode::Delete(SGroupNode *pkNode)
{
    pkNode->m_kMap_dwKey_kVct_stToken.clear();
    pkNode->ChildNodeVector.clear();
    pkNode->m_strGroupName = "";
    pkNode->m_dwGroupNameKey = 0;
    ms_kPool.Free(pkNode);
}

void CTextFileLoader::SGroupNode::DestroySystem()
{
    ms_kPool.Destroy();
}

CTextFileLoader *CTextFileLoader::Cache(const char *c_szFileName)
{
    uint32_t dwNameKey = GetCaseCRC32(c_szFileName, strlen(c_szFileName));
    auto f = ms_kMap_dwNameKey_pkTextFileLoader.find(dwNameKey);
    if (ms_kMap_dwNameKey_pkTextFileLoader.end() != f)
    {
        if (!ms_isCacheMode)
        {
            delete f->second;

            CTextFileLoader *pkNewTextFileLoader = new CTextFileLoader;
            pkNewTextFileLoader->Load(c_szFileName);
            f->second = pkNewTextFileLoader;
        }
        f->second->SetTop();
        return f->second;
    }

    CTextFileLoader *pkNewTextFileLoader = new CTextFileLoader;
    pkNewTextFileLoader->Load(c_szFileName);

    ms_kMap_dwNameKey_pkTextFileLoader.insert(
        std::map<uint32_t, CTextFileLoader *>::value_type(dwNameKey, pkNewTextFileLoader));
    return pkNewTextFileLoader;
}

void CTextFileLoader::SetCacheMode()
{
    ms_isCacheMode = true;
}

void CTextFileLoader::DestroySystem()
{
    {
        std::map<uint32_t, CTextFileLoader *>::iterator i;
        for (i = ms_kMap_dwNameKey_pkTextFileLoader.begin(); i != ms_kMap_dwNameKey_pkTextFileLoader.end(); ++i)
            delete i->second;
        ms_kMap_dwNameKey_pkTextFileLoader.clear();
    }

    SGroupNode::DestroySystem();
}

void CTextFileLoader::Destroy()
{
    __DestroyGroupNodeVector();
}

CTextFileLoader::CTextFileLoader()
{
    SetTop();

    m_GlobalNode.m_strGroupName = "global";
    m_GlobalNode.pParentNode = NULL;

    m_kVct_pkNode.reserve(128);
}

CTextFileLoader::~CTextFileLoader()
{
    Destroy();
}

void CTextFileLoader::__DestroyGroupNodeVector()
{
    std::vector<SGroupNode *>::iterator i;
    for (i = m_kVct_pkNode.begin(); i != m_kVct_pkNode.end(); ++i)
        SGroupNode::Delete(*i);
    m_kVct_pkNode.clear();
}

const char *CTextFileLoader::GetFileName()
{
    return m_strFileName.c_str();
}

bool CTextFileLoader::IsEmpty()
{
    return m_strFileName.empty();
}

bool CTextFileLoader::Load(const char *c_szFileName)
{
    m_strFileName = "";
    if (!GetVfs().Exists(c_szFileName))
        return false;

    m_data = LoadFileToString(GetVfs(), c_szFileName).value_or("");

    m_strFileName = c_szFileName;
    m_dwcurLineIndex = 0;

    m_textFileLoader.Bind(m_data);
    return LoadGroup(&m_GlobalNode);
}

bool CTextFileLoader::LoadGroup(TGroupNode *pGroupNode)
{
    CTokenVector stTokenVector;
    int nLocalGroupDepth = 0;

    for (; m_dwcurLineIndex < m_textFileLoader.GetLineCount(); ++m_dwcurLineIndex)
    {
        int iRet;

        if ((iRet = m_textFileLoader.SplitLine2(m_dwcurLineIndex, &stTokenVector)) != 0)
        {
            if (iRet == -2)
                SPDLOG_ERROR("cannot find \" in {0}:{1}", m_strFileName.c_str(), m_dwcurLineIndex);
            continue;
        }

        stl_lowers(stTokenVector[0]);

        if ('{' == stTokenVector[0][0])
        {
            nLocalGroupDepth++;
            continue;
        }

        if ('}' == stTokenVector[0][0])
        {
            nLocalGroupDepth--;
            break;
        }

        // Group
        if (0 == stTokenVector[0].compare("group"))
        {
            if (2 != stTokenVector.size())
            {
                assert(!"There is no group name!");
                continue;
            }

            TGroupNode *pNewNode = TGroupNode::New();
            m_kVct_pkNode.push_back(pNewNode);

            pNewNode->pParentNode = pGroupNode;
            pNewNode->SetGroupName(stTokenVector[1]);
            pGroupNode->ChildNodeVector.push_back(pNewNode);

            ++m_dwcurLineIndex;

            if (false == LoadGroup(pNewNode))
                return false;
        }
            // List
        else if (0 == stTokenVector[0].compare("list"))
        {
            if (2 != stTokenVector.size())
            {
                assert(!"There is no list name!");
                continue;
            }

            CTokenVector stSubTokenVector;

            stl_lowers(stTokenVector[1]);
            std::string key = stTokenVector[1];

            stTokenVector.clear();

            ++m_dwcurLineIndex;
            for (; m_dwcurLineIndex < m_textFileLoader.GetLineCount(); ++m_dwcurLineIndex)
            {
                if (!m_textFileLoader.SplitLine(m_dwcurLineIndex, &stSubTokenVector))
                    continue;

                if ('{' == stSubTokenVector[0][0])
                    continue;

                if ('}' == stSubTokenVector[0][0])
                    break;

                for (uint32_t j = 0; j < stSubTokenVector.size(); ++j)
                {
                    stTokenVector.push_back(stSubTokenVector[j]);
                }
            }

            pGroupNode->InsertTokenVector(key, stTokenVector);
            //pGroupNode->LocalTokenVectorMap.insert(std::make_pair(key, stTokenVector));
        }
        else
        {
            std::string key = stTokenVector[0];

            if (1 == stTokenVector.size())
            {
                SPDLOG_ERROR("CTextFileLoader::LoadGroup : must have a value (filename: {0} line: {1} key: {2})",
                              m_strFileName.c_str(),
                              m_dwcurLineIndex,
                              key.c_str());
                break;
            }

            stTokenVector.erase(stTokenVector.begin());
            pGroupNode->InsertTokenVector(key, stTokenVector);
            //pGroupNode->LocalTokenVectorMap.insert(std::make_pair(key, stTokenVector));
        }
    }

    return (nLocalGroupDepth == 0);
}

void CTextFileLoader::SetTop()
{
    m_pcurNode = &m_GlobalNode;
}

uint32_t CTextFileLoader::GetChildNodeCount()
{
    if (!m_pcurNode)
    {
        assert(!"Node to access has not set!");
        return 0;
    }

    return m_pcurNode->ChildNodeVector.size();
}

bool CTextFileLoader::SetChildNode(const char *c_szKey)
{
    if (!m_pcurNode)
    {
        assert(!"Node to access has not set!");
        return false;
    }

    uint32_t dwKey = ComputeCrc32(0, c_szKey, strlen(c_szKey));

    for (uint32_t i = 0; i < m_pcurNode->ChildNodeVector.size(); ++i)
    {
        TGroupNode *pGroupNode = m_pcurNode->ChildNodeVector[i];
        if (pGroupNode->IsGroupNameKey(dwKey))
        {
            m_pcurNode = pGroupNode;
            return true;
        }
    }

    return false;
}

bool CTextFileLoader::SetChildNode(const std::string &c_rstrKeyHead, uint32_t dwIndex)
{
    char szKey[32 + 1];
    _snprintf(szKey, sizeof(szKey), "%s%02u", c_rstrKeyHead.c_str(), dwIndex);

    return SetChildNode(szKey);
}

bool CTextFileLoader::SetChildNode(uint32_t dwIndex)
{
    if (!m_pcurNode)
    {
        assert(!"Node to access has not set!");
        return false;
    }

    if (dwIndex >= m_pcurNode->ChildNodeVector.size())
    {
        assert(!"Node index to set is too large to access!");
        return false;
    }

    m_pcurNode = m_pcurNode->ChildNodeVector[dwIndex];

    return true;
}

bool CTextFileLoader::SetParentNode()
{
    if (!m_pcurNode)
    {
        assert(!"Node to access has not set!");
        return false;
    }

    if (NULL == m_pcurNode->pParentNode)
    {
        assert(!"Current group node is already top!");
        return false;
    }

    m_pcurNode = m_pcurNode->pParentNode;

    return true;
}

bool CTextFileLoader::GetCurrentNodeName(std::string *pstrName)
{
    if (!m_pcurNode)
        return false;
    if (NULL == m_pcurNode->pParentNode)
        return false;

    *pstrName = m_pcurNode->GetGroupName();

    return true;
}

bool CTextFileLoader::IsToken(std::string_view c_rstrKey)
{
    if (!m_pcurNode)
    {
        assert(!"Node to access has not set!");
        return false;
    }

    return m_pcurNode->IsExistTokenVector(c_rstrKey);
    //return m_pcurNode->LocalTokenVectorMap.end() != m_pcurNode->LocalTokenVectorMap.find(c_rstrKey);
}

bool CTextFileLoader::GetTokenVector(std::string_view c_rstrKey, CTokenVector **ppTokenVector)
{
    if (!m_pcurNode)
    {
        assert(!"Node to access has not set!");
        return false;
    }

    CTokenVector *pkRetTokenVector = m_pcurNode->GetTokenVector(c_rstrKey);
    if (!pkRetTokenVector)
        return false;

    *ppTokenVector = pkRetTokenVector;

    //CTokenVectorMap::iterator itor = m_pcurNode->LocalTokenVectorMap.find(c_rstrKey);
    //if (m_pcurNode->LocalTokenVectorMap.end() == itor)
    //{
    //Tracef(" CTextFileLoader::GetTokenVector - Failed to find the key %s [%s :: %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
    //	return false;
    //}

    //*ppTokenVector = &itor->second;

    return true;
}

bool CTextFileLoader::GetTokenBoolean(std::string_view c_rstrKey, bool *pData)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenBoolean - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    *pData = bool(atoi(pTokenVector->at(0).c_str()));

    return true;
}

bool CTextFileLoader::GetTokenByte(std::string_view c_rstrKey, uint8_t *pData)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenByte - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    return storm::ParseNumber(pTokenVector->at(0), *pData);
}

bool CTextFileLoader::GetTokenWord(std::string_view c_rstrKey, uint16_t *pData)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenWord - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    return storm::ParseNumber(pTokenVector->at(0), *pData);
}

bool CTextFileLoader::GetTokenInteger(std::string_view c_rstrKey, int *pData)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenInteger - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    return storm::ParseNumber(pTokenVector->at(0), *pData);
}

bool CTextFileLoader::GetTokenDoubleWord(std::string_view c_rstrKey, uint32_t *pData)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenInteger - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    return storm::ParseNumber(pTokenVector->at(0), *pData);
}

bool CTextFileLoader::GetTokenDouble(std::string_view c_rstrKey, double *pData)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenFloat - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    return storm::ParseNumber(pTokenVector->at(0), *pData);
}

bool CTextFileLoader::GetTokenFloat(std::string_view c_rstrKey, float *pData)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenFloat - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    return storm::ParseNumber(pTokenVector->at(0).c_str(), *pData);;
}

bool CTextFileLoader::GetTokenVector2(std::string_view c_rstrKey, Vector2 *pVector2)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->size() != 2)
    {
        //Tracef(" CTextFileLoader::GetTokenVector2 - This key should have 2 values %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }


    storm::ParseNumber(pTokenVector->at(0).c_str(), pVector2->x);
    storm::ParseNumber(pTokenVector->at(1).c_str(), pVector2->y);
    return true;
}

bool CTextFileLoader::GetTokenVector3(std::string_view c_rstrKey, Vector3 *pVector3)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->size() != 3)
    {
        return false;
    }

    storm::ParseNumber(pTokenVector->at(0).c_str(), pVector3->x);
    storm::ParseNumber(pTokenVector->at(1).c_str(), pVector3->y);
    storm::ParseNumber(pTokenVector->at(2).c_str(), pVector3->z);
    return true;
}

bool CTextFileLoader::GetTokenD3DXVECTOR4(std::string_view c_rstrKey, Vector4 *pD3DXVECTOR4)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->size() != 4)
    {
        //Tracef(" CTextFileLoader::GetTokenVector3 - This key should have 3 values %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    storm::ParseNumber(pTokenVector->at(0).c_str(), pD3DXVECTOR4->x);
    storm::ParseNumber(pTokenVector->at(1).c_str(), pD3DXVECTOR4->y);
    storm::ParseNumber(pTokenVector->at(2).c_str(), pD3DXVECTOR4->z);
    storm::ParseNumber(pTokenVector->at(3).c_str(), pD3DXVECTOR4->w);
    return true;
}

bool CTextFileLoader::GetTokenPosition(std::string_view c_rstrKey, Vector3 *pVector)
{
    return GetTokenVector3(c_rstrKey, pVector);
}

bool CTextFileLoader::GetTokenD3DXQUATERNION(std::string_view c_rstrKey, Quaternion *pQ)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->size() != 4)
    {
        //Tracef(" CTextFileLoader::GetTokenVector3 - This key should have 3 values %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    storm::ParseNumber(pTokenVector->at(0).c_str(), pQ->x);
    storm::ParseNumber(pTokenVector->at(1).c_str(), pQ->y);
    storm::ParseNumber(pTokenVector->at(2).c_str(), pQ->z);
    storm::ParseNumber(pTokenVector->at(3).c_str(), pQ->w);

    return true;
}

bool CTextFileLoader::GetTokenDirection(std::string_view c_rstrKey, D3DVECTOR *pVector)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->size() != 3)
    {
        //Tracef(" CTextFileLoader::GetTokenDirection - This key should have 3 values %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }


    storm::ParseNumber(pTokenVector->at(0).c_str(), pVector->x);
    storm::ParseNumber(pTokenVector->at(1).c_str(), pVector->y);
    storm::ParseNumber(pTokenVector->at(2).c_str(), pVector->z);


    return true;
}

bool CTextFileLoader::GetTokenColor(std::string_view c_rstrKey, DirectX::SimpleMath::Color *pColor)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->size() != 4)
    {
        //Tracef(" CTextFileLoader::GetTokenColor - This key should have 4 values %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    storm::ParseNumber(pTokenVector->at(0).c_str(), pColor->x);
    storm::ParseNumber(pTokenVector->at(1).c_str(), pColor->y);
    storm::ParseNumber(pTokenVector->at(2).c_str(), pColor->z);
    storm::ParseNumber(pTokenVector->at(3).c_str(), pColor->w);

    return true;
}

bool CTextFileLoader::GetTokenColor(std::string_view c_rstrKey, D3DCOLORVALUE *pColor)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->size() != 4)
    {
        //Tracef(" CTextFileLoader::GetTokenColor - This key should have 4 values %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    storm::ParseNumber(pTokenVector->at(0).c_str(), pColor->r);
    storm::ParseNumber(pTokenVector->at(1).c_str(), pColor->g);
    storm::ParseNumber(pTokenVector->at(2).c_str(), pColor->b);
    storm::ParseNumber(pTokenVector->at(3).c_str(), pColor->a);

    return true;
}

bool CTextFileLoader::GetTokenString(std::string_view c_rstrKey, std::string *pString)
{
    CTokenVector *pTokenVector;
    if (!GetTokenVector(c_rstrKey, &pTokenVector))
        return false;

    if (pTokenVector->empty())
    {
        //Tracef(" CTextFileLoader::GetTokenString - Failed to find the value %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
        return false;
    }

    *pString = pTokenVector->at(0);

    return true;
}
