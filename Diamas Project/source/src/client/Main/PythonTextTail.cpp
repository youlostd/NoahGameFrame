//
// Ä³¸¯ÅÍ¸¦ µû¶ó´Ù´Ï´Â ÅØ½ºÆ® °ü·Ã ¼Ò½º (ÀÌ¸§, ±æµåÀÌ¸§, ±æµå¸¶Å© µî)
//
#include "stdafx.h"
#include "InstanceBase.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"
#include "Locale.h"
#include "MarkManager.h"
#include "../EterLib/GrpImage.h"
#include "../EterLib/ResourceManager.h"
#include "../EterLib/FontManager.hpp"
#include "../EterLib/Engine.hpp"
#include "../EterLib/TextTag.h"
#include "../EterBase/Timer.h"
#if defined(WJ_SHOW_MOB_INFO)
#include "../EterLib/Engine.hpp"
#endif

constexpr auto textTailFontName = "Roboto:12s";
//constexpr auto textTailFontName = "Nunito Sans:12";

namespace
{
template <size_t N>
class TextInstanceList
{
public:
    enum NameTextInstanceOrder
    {
        kLevel,
        kTitle,
        kPrefix,
        kName,
        kAiFlag,
    };

    TextInstanceList(CGraphicTextInstance *instances[N]);

    int32_t GetWidth() const
    {
        return m_width;
    }

    int32_t GetHeight() const
    {
        return m_height;
    }

    void Render(int32_t x, int32_t y, int32_t z);

private:
    struct Entry
    {
        Entry()
            : delta(0.0f)
              , instance(nullptr)
        {
            // ctor
        }

        int32_t delta;
        CGraphicTextInstance *instance;
    };

    Entry m_list[N];
    int32_t m_width;
    int32_t m_height;
};

template <size_t N>
TextInstanceList<N>::TextInstanceList(CGraphicTextInstance *instances[N])
    : m_width(0)
      , m_height(0)
{
    for (size_t i = 0, j = 0; i < N; ++i)
    {
        auto instance = instances[i];
        if (!instance)
            continue;

        if (instances[kPrefix])
        {
            if (kLevel != j && i != kAiFlag && i != kName)
                m_width += 4; // Padding between elements
        }
        else
        {
            if (kLevel != j && i != kAiFlag)
                m_width += 4; // Padding between elements
        }

        m_list[j].delta = m_width;
        m_list[j].instance = instance;

        m_width += instance->GetWidth();
        m_height = std::max<int32_t>(m_height, instance->GetHeight());

        ++j;
    }
}

template <size_t N>
void TextInstanceList<N>::Render(int32_t x, int32_t y, int32_t z)
{
    for (size_t i = 0; i < N; ++i)
    {
        if (!m_list[i].instance)
            continue;

        m_list[i].instance->Render(x + m_list[i].delta, y, z);
    }
}
}

const DirectX::SimpleMath::Color c_TextTail_Player_Color = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
const DirectX::SimpleMath::Color c_TextTail_Monster_Color = DirectX::SimpleMath::Color(1.0f, 0.0f, 0.0f, 1.0f);
const DirectX::SimpleMath::Color c_TextTail_Item_Color = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
const DirectX::SimpleMath::Color c_TextTail_Chat_Color = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
const DirectX::SimpleMath::Color c_TextTail_Info_Color = DirectX::SimpleMath::Color(1.0f, 0.785f, 0.785f, 1.0f);
const DirectX::SimpleMath::Color c_TextTail_Guild_Name_Color = Color(0xFFEFD3FF);
const float c_TextTail_Name_Position = -10.0f;
const float c_fxMarkPosition = 1.5f;
const float c_fyGuildNamePosition = 15.0f;
const float c_fyCustomTitlePosition = 15.0f;

const float c_fyMarkPosition = 17.0f + 11.0f;
bool bPKTitleEnable = true;

// TEXTTAIL_LIVINGTIME_CONTROL
long gs_TextTail_LivingTime = 5000;

long TextTail_GetLivingTime()
{
    assert(gs_TextTail_LivingTime > 1000);
    return gs_TextTail_LivingTime;
}

void TextTail_SetLivingTime(long livingTime)
{
    gs_TextTail_LivingTime = livingTime;
}

// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

CPythonTextTail::TextTail::TextTail(uint32_t vid, const char *text,
                                    CGraphicObjectInstance *owner,
                                    float height, const DirectX::SimpleMath::Color &color)
    : pTextInstance(new CGraphicTextInstance())
      , pMarkInstance(nullptr)
      , pOwner(owner)
      , dwVirtualID(vid)
      , x(-100)
      , y(-100)
      , z(0)
      , fDistanceFromPlayer(0.0f)
      , Color(color)
      , bNameFlag(false)
      , xStart(-2)
      , yStart(-1)
      , LivingTime(0)
      , fHeight(height)
{
    pTextInstance->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
    pTextInstance->SetValue(text);
    pTextInstance->SetColor(color.R(), color.G(), color.B());
    pTextInstance->Update();

    xEnd = pTextInstance->GetWidth() + 2;
    yEnd = pTextInstance->GetHeight() + 1;
}

CPythonTextTail::TextTail::~TextTail()
{
    if (pMarkInstance)
        CGraphicMarkInstance::Delete(pMarkInstance);
}

void CPythonTextTail::GetInfo(std::string *pstInfo)
{
    pstInfo->append(fmt::format("TextTail: ChatTail {}, ChrTail (Map {}, List {}), ItemTail (Map {}, List {})",
                                m_ChatTailMap.size(),
                                m_CharacterTextTailMap.size(), m_CharacterTextTailList.size(),
                                m_ItemTextTailMap.size(), m_ItemTextTailList.size()));
}

void CPythonTextTail::UpdateAllTextTail()
{
    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
    if (pInstance)
    {
        TPixelPosition pixelPos;
        pInstance->NEW_GetPixelPosition(&pixelPos);

        TTextTailMap::iterator itorMap;

        for (itorMap = m_CharacterTextTailMap.begin(); itorMap != m_CharacterTextTailMap.end(); ++itorMap)
        {
            UpdateDistance(pixelPos, itorMap->second.get());
        }

        for (itorMap = m_ItemTextTailMap.begin(); itorMap != m_ItemTextTailMap.end(); ++itorMap)
        {
            UpdateDistance(pixelPos, itorMap->second.get());
        }

        for (const auto &p : m_ChatTailMap)
        {
            UpdateDistance(pixelPos, p.second.get());

            // NOTE : Chat TextTail이 있으면 캐릭터 이름도 출력한다.
            if (p.second->bNameFlag)
            {
                uint32_t dwVID = p.first;
                ShowCharacterTextTail(dwVID);
            }
        }
    }
}

void CPythonTextTail::UpdateShowingTextTail()
{
    for (auto &tt : m_ItemTextTailList)
        UpdateTextTail(tt);

    for (const auto &pair : m_ChatTailMap)
        UpdateTextTail(pair.second.get());

    for (auto &tt : m_CharacterTextTailList)
    {
        UpdateTextTail(tt);

        // NOTE : Chat TextTail이 있을 경우 위치를 바꾼다.
        const auto itor = m_ChatTailMap.find(tt->dwVirtualID);
        if (m_ChatTailMap.end() != itor && itor->second->bNameFlag)
            tt->y = itor->second->y - 17.0f;
    }
}

void CPythonTextTail::UpdateTextTail(TextTail *pTextTail)
{
    if (!pTextTail->pOwner)
        return;

    auto &rpyGraphic = CPythonGraphic::Instance();

    CPythonGraphic::Identity();

    const Vector3 &c_rv3Position = pTextTail->pOwner->GetPosition();
    float x, y, z;

    rpyGraphic.ProjectPosition(c_rv3Position.x,
                               c_rv3Position.y,
                               c_rv3Position.z + pTextTail->fHeight,
                               &x,
                               &y,
                               &z);

    pTextTail->x = floorf(x);
    pTextTail->y = floorf(y);

    // NOTE : 13m 밖에 있을때만 깊이를 넣습니다 - [levites]
    if (pTextTail->fDistanceFromPlayer < 1300.0f)
    {
        pTextTail->z = 0;
    }
    else
    {
        pTextTail->z = z * CPythonGraphic::Instance().GetOrthoDepth() * -1.0f;
        pTextTail->z += 10;
    }
}

void CPythonTextTail::ArrangeTextTail()
{
    uint32_t dwTime = ELTimer_GetMSec();

    for (auto *tt : m_ItemTextTailList)
    {
        int yTemp = 5;
        int LimitCount = 0;

        for (auto it = m_ItemTextTailList.begin(); it != m_ItemTextTailList.end();)
        {
            TextTail *pCompareTextTail = *it;
            if (*it == tt)
            {
                ++it;
                continue;
            }

            if (LimitCount >= 20)
                break;

            if (isIn(tt, pCompareTextTail))
            {
                tt->y = pCompareTextTail->y + pCompareTextTail->yEnd + yTemp;

                it = m_ItemTextTailList.begin();
                ++LimitCount;
                continue;
            }

            ++it;
        }

        tt->pTextInstance->SetColor(tt->Color.R(),
                                    tt->Color.G(),
                                    tt->Color.B());
    }

    for (auto tt : m_ItemTextTailList)
        tt->pTextInstance->SetColor(tt->Color.R(), tt->Color.G(), tt->Color.B());

    for (auto itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end();)
    {
        TextTail *pTextTail = itorChat->second.get();

        if (pTextTail->LivingTime < dwTime)
        {
            itorChat = m_ChatTailMap.erase(itorChat);
            continue;
        }
        else
            ++itorChat;

        pTextTail->pTextInstance->SetColor(pTextTail->Color.BGRA().c);
    }
}

void CPythonTextTail::Render()
{
    for (auto *pTextTail : m_CharacterTextTailList)
    {
        // Mark 위치 업데이트
        auto &guildMark = pTextTail->pMarkInstance;
        auto &guildName = pTextTail->pGuildNameTextInstance;
        auto &guildCrown = pTextTail->pGuildCrownInstance;
        auto &customTitle = pTextTail->pCustomTitleTextInstance;
#ifdef ENABLE_PLAYTIME_ICON
        auto &playTime = pTextTail->pPlayTimeInstance;
#endif

        if (guildMark && guildName) {
            guildName->Update();

            int textW = guildName->GetWidth();
            int markW = guildMark->GetWidth();

            guildMark->SetPosition(pTextTail->x - textW / 2 - markW -
                                       c_fxMarkPosition,
                                   pTextTail->y - c_fyMarkPosition);
            guildMark->Render();

            guildName->Render(pTextTail->x - textW / 2,
                              pTextTail->y - c_fyGuildNamePosition -
                                  guildName->GetHeight(),
                              pTextTail->z);
        }

        if (customTitle) {
            customTitle->Update();

            int textW = customTitle->GetWidth();

            auto renderPosY = c_fyCustomTitlePosition;
            if (guildName) {
                renderPosY += 15.0f;
            }

            customTitle->Render(pTextTail->x - textW / 2,
                                pTextTail->y - renderPosY -
                                    customTitle->GetHeight(),
                                pTextTail->z);
        }

        if (guildCrown) {
            auto renderPosY = c_fyMarkPosition + 24;

            if (customTitle)
                renderPosY += 24;

            guildCrown->SetPosition(pTextTail->x - guildCrown->GetWidth() + 16,
                                    pTextTail->y - renderPosY);
            guildCrown->Render();
        }

        auto* levelTextInstance =
            Engine::GetSettings().IsShowMobLevel() || pTextTail->isPc
                ? pTextTail->pLevelTextInstance.get()
                : nullptr;
        auto* aiFlagTextInstance =
            Engine::GetSettings().IsShowMobAIFlag() && !pTextTail->isPc
                ? pTextTail->pAIFlagTextInstance.get()
                : nullptr;

        CGraphicTextInstance *instances[5] = {
            levelTextInstance,
            pTextTail->pTitleTextInstance.get(),
            pTextTail->pPrefixTextInstance.get(),
            pTextTail->pTextInstance.get(),
            aiFlagTextInstance,
        };

        TextInstanceList<5> list(instances);
        const auto listX = pTextTail->x - list.GetWidth() + instances[TextInstanceList<5>::kName]->GetWidth() / 2;
        const auto listY = pTextTail->y - list.GetHeight();
        list.Render(listX, listY, pTextTail->z);

 auto& langImage = pTextTail->pLangInstance;
 /*     if (langImage && Engine::GetSettings().IsShowCountryFlags())
       {
            langImage->SetPosition(pTextTail->x + 45, listY + 2);
           langImage->Render();
       }
*/
#ifdef ENABLE_PLAYTIME_ICON
        if (playTime) 
        {
            if (langImage) {
                
                playTime->SetPosition(listX - 20, listY - 2);
                playTime->Render();
            }
        }
#endif
    }

    // Render boxes first
    for (auto *pTextTail : m_ItemTextTailList)
        RenderTextTailBox(pTextTail);

    for (auto *pTextTail : m_ItemTextTailList)
    {
        if (pTextTail->pOwnerTextInstance)
        {
            auto halfW = pTextTail->pTextInstance->GetWidth() / 2;
            pTextTail->pTextInstance->Render(pTextTail->x - halfW,
                                             pTextTail->y + 15.0f,
                                             pTextTail->z);

            halfW = pTextTail->pOwnerTextInstance->GetWidth() / 2;
            pTextTail->pOwnerTextInstance->Render(pTextTail->x - halfW,
                                                  pTextTail->y,
                                                  pTextTail->z);
        }
        else
        {
            auto halfW = pTextTail->pTextInstance->GetWidth() / 2;
            pTextTail->pTextInstance->Render(pTextTail->x - halfW,
                                             pTextTail->y,
                                             pTextTail->z);
        }
    }

    for (const auto &p : m_ChatTailMap)
    {
        auto pTextTail = p.second.get();
        if (pTextTail->pOwner->isShow())
        {
            const auto halfW = pTextTail->pTextInstance->GetWidth() / 2;
            pTextTail->pTextInstance->Render(pTextTail->x - halfW,
                                             pTextTail->y - pTextTail->pTextInstance->GetHeight(),
                                             pTextTail->z);
        }
    }
}

void CPythonTextTail::RenderTextTailBox(TextTail *pTextTail)
{
    const int32_t halfW = (pTextTail->xEnd - pTextTail->xStart) / 2;

    // 검은색 테두리
    CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
    CPythonGraphic::Instance().RenderBox2d(pTextTail->x - halfW + pTextTail->xStart,
                                           pTextTail->y + pTextTail->yStart,
                                           pTextTail->x - halfW + pTextTail->xEnd,
                                           pTextTail->y + pTextTail->yEnd,
                                           pTextTail->z);

    // 검은색 투명박스
    CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
    CPythonGraphic::Instance().RenderBar2d(pTextTail->x - halfW + pTextTail->xStart,
                                           pTextTail->y + pTextTail->yStart,
                                           pTextTail->x - halfW + pTextTail->xEnd,
                                           pTextTail->y + pTextTail->yEnd,
                                           pTextTail->z);
}

void CPythonTextTail::HideAllTextTail()
{
    // NOTE : Show All을 해준뒤 Hide All을 해주지 않으면 문제 발생 가능성 있음
    //        디자인 자체가 그렇게 깔끔하게 되지 않았음 - [levites]
    m_CharacterTextTailList.clear();
    m_ItemTextTailList.clear();
}

void CPythonTextTail::UpdateDistance(const TPixelPosition &c_rCenterPosition, TextTail *pTextTail)
{
    const Vector3 &c_rv3Position = pTextTail->pOwner->GetPosition();
    Vector2 v2Distance(c_rv3Position.x - c_rCenterPosition.x, -c_rv3Position.y - c_rCenterPosition.y);
    pTextTail->fDistanceFromPlayer = v2Distance.Length();
}

void CPythonTextTail::ShowAllPCTextTail()
{
    for (const auto &p : m_CharacterTextTailMap)
    {
        if (p.second->fDistanceFromPlayer < 3500.0f)
            ShowPlayerTextTail(p.first);
    }
}

void CPythonTextTail::ShowAllNPCTextTail()
{
    for (const auto &p : m_CharacterTextTailMap)
    {
        if (p.second->fDistanceFromPlayer < 3500.0f)
            ShowNpcTextTail(p.first);
    }
}

void CPythonTextTail::ShowAllMonsterTextTail()
{
    for (const auto &p : m_CharacterTextTailMap)
    {
        if (p.second->fDistanceFromPlayer < 3500.0f)
            ShowMonsterTextTail(p.first);
    }
}

void CPythonTextTail::ShowAllItemTextTail()
{
    for (const auto &p : m_ItemTextTailMap)
    {
        if (p.second->fDistanceFromPlayer < 3500.0f)
            ShowItemTextTail(p.first);
    }
}

void CPythonTextTail::ShowAllTextTail()
{
    for (const auto &p : m_CharacterTextTailMap)
    {
        if (p.second->fDistanceFromPlayer < 3500.0f)
            ShowCharacterTextTail(p.first);
    }

    for (const auto &p : m_ItemTextTailMap)
    {
        if (p.second->fDistanceFromPlayer < 3500.0f)
            ShowItemTextTail(p.first);
    }
}

void CPythonTextTail::ShowCharacterTextTail(uint32_t VirtualID)
{
    const auto itor = m_CharacterTextTailMap.find(VirtualID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto pTextTail = itor->second.get();

    // NOTE : ShowAll 시에는 모든 Instance 의 Pointer 를 찾아서 체크하므로 부하가 걸릴 가능성도 있다.
    //        CInstanceBase 가 TextTail 을 직접 가지고 있는 것이 가장 좋은 형태일 듯..

    if (!pTextTail->pOwner->isShow())
        return;

    auto pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
    if (!pInstance)
        return;

    if (pInstance->IsGuildWall())
        return;

    if (pInstance->IsNPC() == 20387)
        return;

    if (pInstance->CanPickInstance(true))
        m_CharacterTextTailList.insert(pTextTail);
}

void CPythonTextTail::ShowPlayerTextTail(uint32_t VirtualID)
{
    const auto itor = m_CharacterTextTailMap.find(VirtualID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto pTextTail = itor->second.get();

    // NOTE : ShowAll 시에는 모든 Instance 의 Pointer 를 찾아서 체크하므로 부하가 걸릴 가능성도 있다.
    //        CInstanceBase 가 TextTail 을 직접 가지고 있는 것이 가장 좋은 형태일 듯..
    if (!pTextTail->pOwner->isShow())
        return;

    auto pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
    if (!pInstance)
        return;

    if (!pInstance->IsPC())
        return;

    if (pInstance->CanPickInstance(true))
        m_CharacterTextTailList.insert(pTextTail);
}

void CPythonTextTail::ShowMonsterTextTail(uint32_t VirtualID)
{
    const auto itor = m_CharacterTextTailMap.find(VirtualID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto pTextTail = itor->second.get();

    // NOTE : ShowAll 시에는 모든 Instance 의 Pointer 를 찾아서 체크하므로 부하가 걸릴 가능성도 있다.
    //        CInstanceBase 가 TextTail 을 직접 가지고 있는 것이 가장 좋은 형태일 듯..
    if (!pTextTail->pOwner->isShow())
        return;

    auto pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
    if (!pInstance)
        return;

    if (!pInstance->IsEnemy() && !pInstance->IsStone())
        return;

    if (pInstance->CanPickInstance(true))
        m_CharacterTextTailList.insert(pTextTail);
}

void CPythonTextTail::ShowNpcTextTail(uint32_t VirtualID)
{
    const auto itor = m_CharacterTextTailMap.find(VirtualID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto pTextTail = itor->second.get();

    // NOTE : ShowAll 시에는 모든 Instance 의 Pointer 를 찾아서 체크하므로 부하가 걸릴 가능성도 있다.
    //        CInstanceBase 가 TextTail 을 직접 가지고 있는 것이 가장 좋은 형태일 듯..
    if (!pTextTail->pOwner->isShow())
        return;

    auto pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
    if (!pInstance)
        return;

    if (!pInstance->IsNPC())
        return;

    if (pInstance->CanPickInstance(true))
        m_CharacterTextTailList.insert(pTextTail);
}

void CPythonTextTail::ShowItemTextTail(uint32_t VirtualID)
{
    const auto itor = m_ItemTextTailMap.find(VirtualID);
    if (m_ItemTextTailMap.end() == itor)
        return;

    m_ItemTextTailList.emplace(itor->second.get());
}

bool CPythonTextTail::isIn(CPythonTextTail::TextTail *pSource, CPythonTextTail::TextTail *pTarget)
{
    float x1Source = pSource->x + pSource->xStart;
    float y1Source = pSource->y + pSource->yStart;

    float x2Source = pSource->x + pSource->xEnd;
    float y2Source = pSource->y + pSource->yEnd;

    float x1Target = pTarget->x + pTarget->xStart;
    float y1Target = pTarget->y + pTarget->yStart;

    float x2Target = pTarget->x + pTarget->xEnd;
    float y2Target = pTarget->y + pTarget->yEnd;

    if (x1Source <= x2Target && x2Source >= x1Target &&
        y1Source <= y2Target && y2Source >= y1Target)
    {
        return true;
    }

    return false;
}

void CPythonTextTail::RegisterCharacterTextTail(uint32_t dwGuildID, uint32_t dwVirtualID, const DirectX::SimpleMath::Color &c_rColor,
                                                float fAddHeight)
{
    auto pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVirtualID);
    if (!pCharacterInstance)
        return;

    std::string name = pCharacterInstance->GetNameString();

    auto pTextTail = std::make_unique<TextTail>(dwVirtualID,
                                                name.c_str(),
                                                pCharacterInstance->GetGraphicThingInstancePtr(),
                                                pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() +
                                                fAddHeight,
                                                c_rColor);

    pTextTail->isPc = pCharacterInstance->IsPC();

    auto lang = pCharacterInstance->GetLanguageCode();
    if (lang && pTextTail->isPc)
    {
        auto lcCode = GetLanguageNameByID(lang);
        if (lcCode)
        {
            auto image = CResourceManager::Instance().LoadResource<CGraphicImage>(
                fmt::format("D:/ymir work/ui/game/emote/{}.sub", lcCode.value_or("en")));
            pTextTail->pLangInstance.reset(new CGraphicImageInstance);
            pTextTail->pLangInstance->SetImagePointer(image);
        }
    }

    if (pCharacterInstance->IsGuildLeader() || pCharacterInstance->IsGuildGeneral())
    {
        pTextTail->pGuildCrownInstance.reset(new CGraphicImageInstance());
        if (pCharacterInstance->IsGuildLeader())
            pTextTail->pGuildCrownInstance->SetImagePointer(
                CResourceManager::Instance().LoadResource<CGraphicImage>(
                    "d:/ymir work/effect/etc/guild_crown/crown_4.tga"));
        else
            pTextTail->pGuildCrownInstance->SetImagePointer(
                CResourceManager::Instance().LoadResource<CGraphicImage>(
                    "d:/ymir work/effect/etc/guild_crown/crown_2.tga"));
    }

#ifdef ENABLE_PLAYTIME_ICON
	if (pCharacterInstance->IsPC())
	{
            pTextTail->pPlayTimeInstance.reset(new CGraphicImageInstance());
            std::string_view rutbe_icon{pCharacterInstance->GetMyTimeIcon()};

            if (!rutbe_icon.empty())
            {
                pTextTail->pPlayTimeInstance->SetImagePointer(
                    CResourceManager::Instance().LoadResource<CGraphicImage>(
                    rutbe_icon));
            }
	}
#endif
#ifdef ENABLE_GAMEMASTER_TEXTAIL
        // game master textail
 //   if (pCharacterInstance->IsGameMaster())
 //       pTextTail->pTextInstance->SetOutLineColor(0.8f, 0.8f, 0.8f, 0.6f);
    pCharacterInstance->Update();
#endif

    if (0 != dwGuildID)
    {
        pTextTail->pMarkInstance = CGraphicMarkInstance::New();

        uint32_t dwMarkID = CGuildMarkManager::Instance().GetMarkID(dwGuildID);

        if (dwMarkID != CGuildMarkManager::INVALID_MARK_ID)
        {
            std::string markImagePath;

            if (CGuildMarkManager::Instance().GetMarkImageFilename(dwMarkID / CGuildMarkImage::MARK_TOTAL_COUNT,
                                                                   markImagePath))
            {
                pTextTail->pMarkInstance->SetImageFileName(markImagePath.c_str());
                pTextTail->pMarkInstance->Load();
                pTextTail->pMarkInstance->SetIndex(dwMarkID % CGuildMarkImage::MARK_TOTAL_COUNT);
            }
        }

        auto &prGuildNameInstance = pTextTail->pGuildNameTextInstance;
        prGuildNameInstance.reset(new CGraphicTextInstance());
        prGuildNameInstance->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
        prGuildNameInstance->SetOutline(true);
        std::string guild_name{CPythonGuild::Instance().GetGuildName(dwGuildID).value_or("Noname")};

        if (CPythonGuild::Instance().GetGuildName(dwGuildID).has_value())
        {
            if (pCharacterInstance->IsGuildLeader())
                guild_name = "[Lider] - " + guild_name;
            else if (pCharacterInstance->IsGuildGeneral())
                guild_name = "[General] - " + guild_name;
        }

        prGuildNameInstance->SetValue(guild_name);        prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color.R(),
                                      c_TextTail_Guild_Name_Color.G(),
                                      c_TextTail_Guild_Name_Color.B());
        prGuildNameInstance->Update();
    }


#if defined(WJ_SHOW_MOB_INFO)
    if (IS_SET(pCharacterInstance->GetAIFlag(), CInstanceBase::AIFLAG_AGGRESSIVE))
    {
        auto &prAIFlagInstance = pTextTail->pAIFlagTextInstance;
        prAIFlagInstance.reset(new CGraphicTextInstance());
        prAIFlagInstance->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
        prAIFlagInstance->SetOutline(true);
        prAIFlagInstance->SetValue("*");
        prAIFlagInstance->SetColor(c_TextTail_Monster_Color.R(), c_TextTail_Monster_Color.G(), c_TextTail_Monster_Color.B());
        prAIFlagInstance->Update();
    }
#endif

    pTextTail->pTextInstance->SetOutline(true);

    m_CharacterTextTailMap.emplace(std::make_pair(dwVirtualID, std::move(pTextTail)));
}

void CPythonTextTail::RegisterItemTextTail(uint32_t VirtualID, const char *c_szText, CGraphicObjectInstance *pOwner)
{
    auto pTextTail = std::make_unique<TextTail>(VirtualID, c_szText, pOwner,
                                                c_TextTail_Name_Position,
                                                c_TextTail_Item_Color);
    m_ItemTextTailMap.emplace(std::make_pair(VirtualID, std::move(pTextTail)));
}

void CPythonTextTail::RegisterChatTail(uint32_t VirtualID, const char *c_szChat)
{
    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);
    if (!pCharacterInstance)
        return;

    const auto itor = m_ChatTailMap.find(VirtualID);
    if (m_ChatTailMap.end() != itor)
    {
        auto &pTextTail = itor->second;

        pTextTail->Color = c_TextTail_Chat_Color;
        pTextTail->LivingTime = ELTimer_GetMSec() + TextTail_GetLivingTime();
        pTextTail->bNameFlag = true;

        pTextTail->pTextInstance->SetValue(c_szChat);
        pTextTail->pTextInstance->SetColor(c_TextTail_Chat_Color.BGRA().c);
        pTextTail->pTextInstance->Update();
        return;
    }

    auto pTextTail = std::make_unique<TextTail>(VirtualID,
                                                c_szChat,
                                                pCharacterInstance->GetGraphicThingInstancePtr(),
                                                pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f,
                                                c_TextTail_Chat_Color);

    pTextTail->LivingTime = ELTimer_GetMSec() + TextTail_GetLivingTime();
    pTextTail->bNameFlag = true;

    pTextTail->pTextInstance->SetOutline(true);

    m_ChatTailMap.emplace(std::make_pair(VirtualID, std::move(pTextTail)));
}

void CPythonTextTail::RegisterInfoTail(uint32_t VirtualID, const char *c_szChat)
{
    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);
    if (!pCharacterInstance)
        return;

    const auto itor = m_ChatTailMap.find(VirtualID);
    if (m_ChatTailMap.end() != itor)
    {
        auto &pTextTail = itor->second;

        pTextTail->Color = c_TextTail_Info_Color;
        pTextTail->LivingTime = ELTimer_GetMSec() + TextTail_GetLivingTime();
        pTextTail->bNameFlag = false;

        pTextTail->pTextInstance->SetValue(c_szChat);
        pTextTail->pTextInstance->SetColor(c_TextTail_Info_Color.BGRA().c);
        pTextTail->pTextInstance->Update();
        return;
    }

    auto pTextTail = std::make_unique<TextTail>(VirtualID,
                                                c_szChat,
                                                pCharacterInstance->GetGraphicThingInstancePtr(),
                                                pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 50.0f,
                                                c_TextTail_Info_Color);

    pTextTail->LivingTime = ELTimer_GetMSec() + TextTail_GetLivingTime();
    pTextTail->bNameFlag = false;

    pTextTail->pTextInstance->SetOutline(true);

    m_ChatTailMap.emplace(std::make_pair(VirtualID, std::move(pTextTail)));
}

bool CPythonTextTail::GetTextTailPosition(uint32_t dwVID, float *px, float *py, float *pz)
{
    const auto it = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == it)
        return false;

    *px = it->second->x;
    *py = it->second->y;
    *pz = it->second->z;
    return true;
}

bool CPythonTextTail::IsChatTextTail(uint32_t dwVID)
{
    return m_ChatTailMap.end() != m_ChatTailMap.find(dwVID);
}

void CPythonTextTail::SetCharacterTextTailColor(uint32_t VirtualID, const DirectX::SimpleMath::Color &c_rColor)
{
    const auto it = m_CharacterTextTailMap.find(VirtualID);
    if (m_CharacterTextTailMap.end() == it)
        return;

    it->second->pTextInstance->SetColor(c_rColor.BGRA().c);
    it->second->Color = c_rColor;
}

void CPythonTextTail::UpdateCharacterName(uint32_t VirtualID, const std::string &name)
{
    const auto it = m_CharacterTextTailMap.find(VirtualID);
    if (m_CharacterTextTailMap.end() == it)
        return;

    it->second->pTextInstance->SetValue(name);
    it->second->pTextInstance->Update();
}

void CPythonTextTail::SetItemTextTailOwner(uint32_t dwVID, const char *c_szName)
{
    const auto itor = m_ItemTextTailMap.find(dwVID);
    if (m_ItemTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;

    if (strlen(c_szName) > 0)
    {
        if (!pTextTail->pOwnerTextInstance)
            pTextTail->pOwnerTextInstance.reset(new CGraphicTextInstance());

        std::string strName = c_szName;
        strName += "'s";

        pTextTail->pOwnerTextInstance->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
        pTextTail->pOwnerTextInstance->SetValue(strName);
        pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
        pTextTail->pOwnerTextInstance->Update();

        int xOwnerSize = pTextTail->pOwnerTextInstance->GetWidth();
        int yOwnerSize = pTextTail->pOwnerTextInstance->GetHeight();

        pTextTail->xEnd = std::max(pTextTail->xEnd, xOwnerSize + 2);
        pTextTail->yEnd += yOwnerSize + 4;
    }
    else
    {
        if (pTextTail->pOwnerTextInstance)
            pTextTail->pOwnerTextInstance.reset();

        int xSize = pTextTail->pTextInstance->GetWidth();
        int ySize = pTextTail->pTextInstance->GetHeight();

        pTextTail->xEnd = xSize + 2;
        pTextTail->yEnd = ySize + 1;
    }
}

void CPythonTextTail::DeleteCharacterTextTail(uint32_t VirtualID)
{
    const auto itorCharacter = m_CharacterTextTailMap.find(VirtualID);
    const auto itorChat = m_ChatTailMap.find(VirtualID);

    if (m_CharacterTextTailMap.end() != itorCharacter)
        m_CharacterTextTailMap.erase(itorCharacter);
    else
        SPDLOG_DEBUG("DeleteCharacterTextTail - Find VID[{0}] Error", VirtualID);

    if (m_ChatTailMap.end() != itorChat)
        m_ChatTailMap.erase(itorChat);
}

void CPythonTextTail::DeleteItemTextTail(uint32_t VirtualID)
{
    const auto itor = m_ItemTextTailMap.find(VirtualID);
    if (m_ItemTextTailMap.end() == itor)
    {
        SPDLOG_DEBUG("DeleteItemTextTail - None Item Text Tail");
        return;
    }

    m_ItemTextTailMap.erase(itor);
}

int CPythonTextTail::Pick(int ixMouse, int iyMouse)
{
    for (const auto &p : m_ItemTextTailMap)
    {
        auto &pTextTail = p.second;

        const int32_t halfW = (pTextTail->xEnd - pTextTail->xStart) / 2;

        const int32_t left = pTextTail->x - halfW + pTextTail->xStart;
        const int32_t right = pTextTail->x - halfW + pTextTail->xEnd;
        const int32_t top = pTextTail->y + pTextTail->yStart;
        const int32_t bottom = pTextTail->y + pTextTail->yEnd;

        if (ixMouse >= left && ixMouse < right &&
            iyMouse >= top && iyMouse < bottom)
        {
            SelectItemName(p.first);
            return p.first;
        }
    }

    return -1;
}

void CPythonTextTail::SelectItemName(uint32_t dwVirtualID)
{
    const auto itor = m_ItemTextTailMap.find(dwVirtualID);
    if (m_ItemTextTailMap.end() == itor)
        return;

    itor->second->pTextInstance->SetColor(0.1f, 0.9f, 0.1f);
}

void CPythonTextTail::AttachTitle(uint32_t dwVID, const char *c_szName, const DirectX::SimpleMath::Color &c_rColor)
{
    if (!bPKTitleEnable)
        return;

    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;

    auto &prTitle = pTextTail->pTitleTextInstance;
    if (!prTitle)
    {
        prTitle.reset(new CGraphicTextInstance());
        prTitle->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
        prTitle->SetOutline(true);
    }

    prTitle->SetValue(c_szName);
    prTitle->SetColor(c_rColor.BGRA().c);
    prTitle->Update();
}

void CPythonTextTail::DetachTitle(uint32_t dwVID)
{
    if (!bPKTitleEnable)
        return;

    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;
    if (pTextTail->pTitleTextInstance)
        pTextTail->pTitleTextInstance = NULL;
}

void CPythonTextTail::AttachPrefix(uint32_t dwVID, const char *c_szName, const DirectX::SimpleMath::Color &c_rColor)
{
    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;

    auto &prPrefix = pTextTail->pPrefixTextInstance;
    if (!prPrefix)
    {
        prPrefix.reset(new CGraphicTextInstance());
        prPrefix->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
        prPrefix->SetOutline(true);
    }

    prPrefix->SetValue(c_szName);
    prPrefix->SetColor(c_rColor.BGRA().c);
    prPrefix->Update();
}

void CPythonTextTail::DetachPrefix(uint32_t dwVID)
{
    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;
    if (pTextTail->pPrefixTextInstance)
        pTextTail->pPrefixTextInstance = NULL;
}

void CPythonTextTail::AttachCustomTitle(uint32_t dwVID, const char *c_szName, const DirectX::SimpleMath::Color &c_rColor)
{
    if (!bPKTitleEnable)
        return;

    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;

    auto &prTitle = pTextTail->pCustomTitleTextInstance;
    if (!prTitle)
    {
        prTitle.reset(new CGraphicTextInstance());
        prTitle->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
        prTitle->SetOutline(true);
    }

    prTitle->SetValue(c_szName);
    prTitle->SetColor(c_rColor.BGRA().c);
    prTitle->Update();
}

void CPythonTextTail::DetachCustomTitle(uint32_t dwVID)
{
    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;
    if (pTextTail->pCustomTitleTextInstance)
        pTextTail->pCustomTitleTextInstance = NULL;
}

void CPythonTextTail::EnablePKTitle(bool bFlag)
{
    bPKTitleEnable = bFlag;
}

void CPythonTextTail::AttachLevel(uint32_t dwVID, const char *c_szText, const DirectX::SimpleMath::Color &c_rColor)
{
    if (!bPKTitleEnable)
        return;

    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;

    auto &prLevel = pTextTail->pLevelTextInstance;
    if (!prLevel)
    {
        prLevel.reset(new CGraphicTextInstance());
        prLevel->SetTextPointer(Engine::GetFontManager().LoadFont(textTailFontName));
        prLevel->SetOutline(true);
    }

    prLevel->SetValue(c_szText);
    prLevel->SetColor(c_rColor.BGRA().c);
    prLevel->Update();
}

void CPythonTextTail::DetachLevel(uint32_t dwVID)
{
    if (!bPKTitleEnable)
        return;

    const auto itor = m_CharacterTextTailMap.find(dwVID);
    if (m_CharacterTextTailMap.end() == itor)
        return;

    auto &pTextTail = itor->second;
    if (pTextTail->pLevelTextInstance)
        pTextTail->pLevelTextInstance = NULL;
}

void CPythonTextTail::Initialize()
{
}

void CPythonTextTail::Destroy()
{
}

void CPythonTextTail::Clear()
{
    m_CharacterTextTailMap.clear();
    m_CharacterTextTailList.clear();
    m_ItemTextTailMap.clear();
    m_ItemTextTailList.clear();
    m_ChatTailMap.clear();
}

CPythonTextTail::CPythonTextTail()
{
    Clear();
}

CPythonTextTail::~CPythonTextTail()
{
    Destroy();
}
