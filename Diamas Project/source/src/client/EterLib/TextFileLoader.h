#ifndef METIN2_CLIENT_ETERLIB_TEXTFILELOADER_HPP
#define METIN2_CLIENT_ETERLIB_TEXTFILELOADER_HPP

#pragma once

#include "../EterBase/FileLoader.h"
#include "../EterLib/Util.h"
#include "../EterLib/Pool.h"

class CTextFileLoader
{
public:
    typedef struct SGroupNode
    {
        void SetGroupName(const std::string &c_rstGroupName);
        bool IsGroupNameKey(uint32_t dwGroupNameKey);

        const std::string &GetGroupName();

        CTokenVector *GetTokenVector(std::string_view c_rstGroupName);
        bool IsExistTokenVector(std::string_view c_rstGroupName);
        void InsertTokenVector(const std::string &c_rstGroupName, const CTokenVector &c_rkVct_stToken);

        uint32_t m_dwGroupNameKey;
        std::string m_strGroupName;

        std::map<uint32_t, CTokenVector> m_kMap_dwKey_kVct_stToken;

        SGroupNode *pParentNode;
        std::vector<SGroupNode *> ChildNodeVector;

        static SGroupNode *New();
        static void Delete(SGroupNode *pkNode);

        static void DestroySystem();
        static CDynamicPool<SGroupNode> ms_kPool;
    } TGroupNode;

    typedef std::vector<TGroupNode *> TGroupNodeVector;

    class CGotoChild
    {
    public:
        CGotoChild(CTextFileLoader *pOwner, const char *c_szKey)
            : m_pOwner(pOwner)
        {
            m_pOwner->SetChildNode(c_szKey);
        }

        CGotoChild(CTextFileLoader *pOwner, uint32_t dwIndex)
            : m_pOwner(pOwner)
        {
            m_pOwner->SetChildNode(dwIndex);
        }

        ~CGotoChild()
        {
            m_pOwner->SetParentNode();
        }

        CTextFileLoader *m_pOwner;
    };

public:
    static void DestroySystem();

    static void SetCacheMode();

    static CTextFileLoader *Cache(const char *c_szFileName);

public:
    CTextFileLoader();
    virtual ~CTextFileLoader();

    void Destroy();

    bool Load(const char *c_szFileName);
    const char *GetFileName();

    bool IsEmpty();

    void SetTop();
    uint32_t GetChildNodeCount();
    bool SetChildNode(const char *c_szKey);
    bool SetChildNode(const std::string &c_rstrKeyHead, uint32_t dwIndex);
    bool SetChildNode(uint32_t dwIndex);
    bool SetParentNode();
    bool GetCurrentNodeName(std::string *pstrName);
    bool IsToken(std::string_view c_rstrKey);

    bool GetTokenVector(std::string_view c_rstrKey, CTokenVector **ppTokenVector);
    bool GetTokenBoolean(std::string_view c_rstrKey, bool *pData);
    bool GetTokenByte(std::string_view c_rstrKey, uint8_t *pData);
    bool GetTokenWord(std::string_view c_rstrKey, uint16_t *pData);
    bool GetTokenInteger(std::string_view c_rstrKey, int *pData);
    bool GetTokenDoubleWord(std::string_view c_rstrKey, uint32_t *pData);
    bool GetTokenDouble(std::string_view c_rstrKey, double *pData);
    bool GetTokenFloat(std::string_view c_rstrKey, float *pData);
    bool GetTokenVector2(std::string_view c_rstrKey, Vector2 *pVector2);
    bool GetTokenVector3(std::string_view c_rstrKey, Vector3 *pVector3);
    bool GetTokenD3DXVECTOR4(std::string_view c_rstrKey, Vector4 *pD3DXVECTOR4);

    bool GetTokenPosition(std::string_view c_rstrKey, Vector3 *pVector);
    bool GetTokenD3DXQUATERNION(std::string_view c_rstrKey, Quaternion *pQ);
    bool GetTokenDirection(std::string_view c_rstrKey, D3DVECTOR *pVector);
    bool GetTokenColor(std::string_view c_rstrKey, DirectX::SimpleMath::Color *pColor);
    bool GetTokenColor(std::string_view c_rstrKey, D3DCOLORVALUE *pColor);
    bool GetTokenString(std::string_view c_rstrKey, std::string *pString);

protected:
    void __DestroyGroupNodeVector();

    bool LoadGroup(TGroupNode *pGroupNode);

protected:
    std::string m_strFileName;
    std::string m_data;

    uint32_t m_dwcurLineIndex;

    CMemoryTextFileLoader m_textFileLoader;

    TGroupNode m_GlobalNode;
    TGroupNode *m_pcurNode;

    std::vector<SGroupNode *> m_kVct_pkNode;

protected:
    static std::map<uint32_t, CTextFileLoader *> ms_kMap_dwNameKey_pkTextFileLoader;
    static bool ms_isCacheMode;
};

#endif
