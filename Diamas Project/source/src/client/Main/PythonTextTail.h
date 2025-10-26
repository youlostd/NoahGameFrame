#ifndef METIN2_CLIENT_MAIN_PYTHONTEXTTAIL_H
#define METIN2_CLIENT_MAIN_PYTHONTEXTTAIL_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>

class CGraphicTextInstance;
class CGraphicMarkInstance;
class CGraphicObjectInstance;

/*
 *	따라다니는 텍스트 처리
 */
class CPythonTextTail : public CSingleton<CPythonTextTail>
{
public:
    struct TextTail
    {
        TextTail(uint32_t vid, const char *text,
                 CGraphicObjectInstance *owner,
                 float height, const DirectX::SimpleMath::Color &color);
        ~TextTail();

        std::unique_ptr<CGraphicImageInstance> pGuildCrownInstance;
        std::unique_ptr<CGraphicImageInstance> pLangInstance;
        std::unique_ptr<CGraphicTextInstance> pTextInstance;
        std::unique_ptr<CGraphicTextInstance> pOwnerTextInstance;

        CGraphicMarkInstance *pMarkInstance;
        std::unique_ptr<CGraphicTextInstance> pCustomTitleTextInstance;
        std::unique_ptr<CGraphicTextInstance> pGuildNameTextInstance;
        std::unique_ptr<CGraphicTextInstance> pAIFlagTextInstance;
        std::unique_ptr<CGraphicTextInstance> pTitleTextInstance;
        std::unique_ptr<CGraphicTextInstance> pPrefixTextInstance;
        std::unique_ptr<CGraphicTextInstance> pLevelTextInstance;
#ifdef ENABLE_PLAYTIME_ICON
        std::unique_ptr<CGraphicImageInstance> pPlayTimeInstance;
#endif

        // Todo : 이건 전부 VID로 바꾸도록 한다.
        //        도중 캐릭터가 없어질 경우 튕길 가능성이 있음
        CGraphicObjectInstance *pOwner;

        uint32_t dwVirtualID;

        int32_t x, y, z;
        float fDistanceFromPlayer;
        DirectX::SimpleMath::Color Color;
        bool bNameFlag; // 이름도 함께 켤것인지의 플래그
        bool isPc;

        int32_t xStart, yStart;
        int32_t xEnd, yEnd;

        uint32_t LivingTime;

        float fHeight;
    };

    typedef std::unordered_map<uint32_t, std::unique_ptr<TextTail>> TTextTailMap;

public:
    CPythonTextTail(void);
    virtual ~CPythonTextTail(void);

    void GetInfo(std::string *pstInfo);

    void Initialize();
    void Destroy();
    void Clear();

    void UpdateAllTextTail();
    void UpdateShowingTextTail();
    void Render();

    void ArrangeTextTail();
    void HideAllTextTail();
    void ShowAllNPCTextTail();
    void ShowAllMonsterTextTail();
    void ShowAllItemTextTail();
    void ShowAllPCTextTail();

    void ShowAllTextTail();
    void ShowCharacterTextTail(uint32_t VirtualID);
    void ShowPlayerTextTail(uint32_t VirtualID);
    void ShowMonsterTextTail(uint32_t VirtualID);
    void ShowNpcTextTail(uint32_t VirtualID);

    void ShowItemTextTail(uint32_t VirtualID);

    void RegisterCharacterTextTail(uint32_t dwGuildID, uint32_t dwVirtualID, const DirectX::SimpleMath::Color &c_rColor,
                                   float fAddHeight = 10.0f);
    void RegisterItemTextTail(uint32_t VirtualID, const char *c_szText, CGraphicObjectInstance *pOwner);
    void RegisterChatTail(uint32_t VirtualID, const char *c_szChat);
    void RegisterInfoTail(uint32_t VirtualID, const char *c_szChat);
    void SetCharacterTextTailColor(uint32_t VirtualID, const DirectX::SimpleMath::Color &c_rColor);
    void UpdateCharacterName(uint32_t VirtualID, const std::string &name);
    void SetItemTextTailOwner(uint32_t dwVID, const char *c_szName);
    void DeleteCharacterTextTail(uint32_t VirtualID);
    void DeleteItemTextTail(uint32_t VirtualID);

    int Pick(int ixMouse, int iyMouse);
    void SelectItemName(uint32_t dwVirtualID);

    bool GetTextTailPosition(uint32_t dwVID, float *px, float *py, float *pz);
    bool IsChatTextTail(uint32_t dwVID);

    void EnablePKTitle(bool bFlag);
    void AttachTitle(uint32_t dwVID, const char *c_szName, const DirectX::SimpleMath::Color &c_rColor);
    void DetachTitle(uint32_t dwVID);
    void AttachPrefix(uint32_t dwVID, const char *c_szName, const DirectX::SimpleMath::Color &c_rColor);
    void DetachPrefix(uint32_t dwVID);
    void AttachCustomTitle(uint32_t dwVID, const char *c_szName, const DirectX::SimpleMath::Color &c_rColor);
    void DetachCustomTitle(uint32_t dwVID);

    void AttachLevel(uint32_t dwVID, const char *c_szText, const DirectX::SimpleMath::Color &c_rColor);
    void DetachLevel(uint32_t dwVID);

protected:
    void UpdateTextTail(TextTail *pTextTail);
    void RenderTextTailBox(TextTail *pTextTail);
    void UpdateDistance(const TPixelPosition &c_rCenterPosition, TextTail *pTextTail);

    bool isIn(TextTail *pSource, TextTail *pTarget);

protected:
    TTextTailMap m_CharacterTextTailMap;
    TTextTailMap m_ItemTextTailMap;
    TTextTailMap m_ChatTailMap;

    // TODO(tim): Use ordered vectors sorted by camera distance?
    std::unordered_set<TextTail *> m_CharacterTextTailList;
    std::unordered_set<TextTail *> m_ItemTextTailList;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONTEXTTAIL_H */
