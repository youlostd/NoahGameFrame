#include "../EterLib/Engine.hpp"
#include "StdAfx.h"

#include <optional>

#include "../EterPythonLib/PythonGraphic.h"
#include <pak/Vfs.hpp>

#include <array>
#include <fstream>
#include <storm/StringUtil.hpp>
#include <storm/Tokenizer.hpp>
#include <streambuf>
#include <string>

#define DEF_STR(x) #x

#define DEFAULT_VALUE_ALWAYS_SHOW_NAME 1

int CClientConfig::GetResolutionCount() const
{
    return m_videoModes ? m_videoModes->count() : 0;
}

bool CClientConfig::SetResolution(int index)
{
    if (!m_videoModes)
        return false;

    if (index >= m_videoModes->count())
        return false;

    m_Config.width = m_videoModes->item(index)->getWidth();
    m_Config.height = m_videoModes->item(index)->getHeight();
    m_Config.bpp = m_videoModes->item(index)->getColourDepth();
    m_Config.frequency = m_videoModes->item(index)->getRefreshRate();

    SaveConfig();
    return true;
}

bool CClientConfig::IsResolutionByDescription(const std::string &desc)
{
    if (!m_videoModes)
        return false;

    if (m_videoModes->item(desc))
        return true;

    return false;
}

std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> CClientConfig::GetResolution(int index)
{
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    if (!m_videoModes)
        return std::nullopt;

    if (index >= m_videoModes->count())
        return std::nullopt;

    width = m_videoModes->item(index)->getWidth();
    height = m_videoModes->item(index)->getHeight();
    bpp = m_videoModes->item(index)->getColourDepth();

    return std::make_tuple(width, height, bpp);
}

uint32_t CClientConfig::GetWidth()
{
    return m_Config.width;
}

uint32_t CClientConfig::GetHeight()
{
    return m_Config.height;
}

uint32_t CClientConfig::GetBPP()
{
    return m_Config.bpp;
}

uint32_t CClientConfig::GetFrequency()
{
    return m_Config.frequency;
}

bool CClientConfig::IsSoftwareCursor()
{
    return m_Config.is_software_cursor;
}

float CClientConfig::GetMusicVolume()
{
    return m_Config.music_volume;
}

float CClientConfig::GetSoundVolume()
{
    return m_Config.voice_volume;
}

void CClientConfig::SetMusicVolume(float fVolume)
{
    m_Config.music_volume = fVolume;
}

void CClientConfig::SetSoundVolumef(float fVolume)
{
    m_Config.voice_volume = fVolume;
}

int CClientConfig::GetViewDistance()
{
    return m_Config.iDistance;
}

void CClientConfig::SetViewDistance(int distance)
{
    m_Config.iDistance = distance;
}

int CClientConfig::GetShadowLevel()
{
    return m_Config.iShadowLevel;
}

void CClientConfig::SetShadowLevel(unsigned int level)
{
    m_Config.iShadowLevel = std::min<unsigned>(level, 5);
    // CPythonBackground::instance().RefreshShadowLevel();
}

void CClientConfig::SetCameraDistanceMode(int8_t distanceMode)
{
    std::array<float, 3> cameraDistanceModeToDistance = {2500.0f, 3500.0f, 4500.0f};

    CCamera::SetCameraMaxDistance(cameraDistanceModeToDistance[distanceMode]);
    m_Config.iCameraDistanceMode = distanceMode;
}

int CClientConfig::GetNearbyShopsDisplayed()
{
    return m_Config.iNearbyShopsDisplayed;
}

void CClientConfig::SetNearbyShopsDisplayed(unsigned int newvalue)
{
    m_Config.iNearbyShopsDisplayed = std::min(newvalue, 40u);
}

void CClientConfig::CreateVideoModeList()
{
    m_videoModes = std::make_unique<Graphics::VideoModeList>();
}

CClientConfig::TConfig *CClientConfig::GetConfig()
{
    return &m_Config;
}

void CClientConfig::SetConfig(TConfig *pNewConfig)
{
    m_Config = *pNewConfig;
}

uint32_t CClientConfig::GetGamma()
{
    return m_Config.gamma;
}

uint8_t CClientConfig::IsShowFPS()
{
    return m_Config.is_show_fps;
}

void CClientConfig::SetShowFPS(uint8_t val)
{
    m_Config.is_show_fps = val;
}

void CClientConfig::SetDefaultConfig()
{
    memset(&m_Config, 0, sizeof(m_Config));

    m_Config.width = 1024;
    m_Config.height = 768;
    m_Config.bpp = 32;
    m_Config.bWindowed = true;

    m_Config.is_software_cursor = false;
    m_Config.iDistance = 3;

    m_Config.gamma = 1;
    m_Config.music_volume = 0.5f;
    m_Config.voice_volume = 0.0f;

    m_Config.is_show_fps = true;

    m_Config.iShadowLevel = 3;
    m_Config.iCameraDistanceMode = 0;
    m_Config.iNearbyShopsDisplayed = 20;
    m_Config.bViewChat = true;
    m_Config.bAlwaysShowNamePlayer = DEFAULT_VALUE_ALWAYS_SHOW_NAME;
    m_Config.bAlwaysShowNameNpc = DEFAULT_VALUE_ALWAYS_SHOW_NAME;
    m_Config.bAlwaysShowNameMonster = DEFAULT_VALUE_ALWAYS_SHOW_NAME;
    m_Config.bAlwaysShowNameItem = DEFAULT_VALUE_ALWAYS_SHOW_NAME;

    m_Config.bShowDamage = true;
    m_Config.bShowSalesText = true;
    m_Config.bShowAcce = true;
    m_Config.bShowCostume = true;
    m_Config.bItemHighlight = true;
    m_Config.bShowMobAggr = true;
    m_Config.bShowMobLevel = true;
    m_Config.bEnableNight = false;
    m_Config.bEnableSnow = false;
    m_Config.bEnableSnowTexture = false;
    m_Config.bShowMoneyLog = true;

    m_Config.bShowItemLog = true;
    m_Config.bPickupInfoWindowActive = false;

    m_Config.bShowSpecular = true;
    m_Config.bShowGeneralEffect = true;
    m_Config.bShowBuffEffects = true;
    m_Config.bShowSkillEffects = true;

    m_Config.bFogMode = true;
    m_Config.bShowCountryFlags = true;
    m_Config.bCollectEquipment = true;
    m_Config.bCollectUseableEquipment = false;
    m_Config.bHidePets = false;
    m_Config.bHideShops = false;
    m_Config.bHideMounts = false;
    m_Config.bHideGrowthPets = false;
    m_Config.bEnableEmojiSystem = true;
    m_Config.bShowFriendNotifcations = true;
    m_Config.bShowGuildNotifcations = true;
    m_Config.bShowGMNotifcations = true;
#ifdef ENABLE_PERSPECTIVE_VIEW
    m_Config.fField = 0.0f;
#endif
}

bool CClientConfig::IsWindowed()
{
    return m_Config.bWindowed;
}

void CClientConfig::SetWindowed(bool val)
{
    m_Config.bWindowed = val;
}

bool CClientConfig::IsViewChat()
{
    return m_Config.bViewChat;
}

void CClientConfig::SetViewChatFlag(int iFlag)
{
    m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CClientConfig::IsAlwaysShowPlayerName()
{
    return m_Config.bAlwaysShowNamePlayer;
}

bool CClientConfig::IsAlwaysShowMonsterName()
{
    return m_Config.bAlwaysShowNameMonster;
}

bool CClientConfig::IsAlwaysShowNPCName()
{
    return m_Config.bAlwaysShowNameNpc;
}

bool CClientConfig::IsAlwaysShowItemName()
{
    return m_Config.bAlwaysShowNameItem;
}

void CClientConfig::SetAlwaysShowPlayerNameFlag(int iFlag)
{
    m_Config.bAlwaysShowNamePlayer = iFlag == 1 ? true : false;
}

void CClientConfig::SetAlwaysShowNPCNameFlag(int iFlag)
{
    m_Config.bAlwaysShowNameNpc = iFlag == 1 ? true : false;
}

void CClientConfig::SetAlwaysShowMonsterNameFlag(int iFlag)
{
    m_Config.bAlwaysShowNameMonster = iFlag == 1 ? true : false;
}

void CClientConfig::SetAlwaysShowItemNameFlag(int iFlag)
{
    m_Config.bAlwaysShowNameItem = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowDamage()
{
    return m_Config.bShowDamage;
}

void CClientConfig::SetShowDamageFlag(int iFlag)
{
    m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowSalesText()
{
    return m_Config.bShowSalesText;
}

#if defined(WJ_SHOW_ACCE_ONOFF)
bool CClientConfig::IsShowAcce() const
{
    return m_Config.bShowAcce;
}

void CClientConfig::SetShowAcce(int iFlag)
{
    m_Config.bShowAcce = iFlag == 1 ? true : false;
}
#endif

bool CClientConfig::IsShowCostume() const
{
    return m_Config.bShowCostume;
}

void CClientConfig::SetShowCostume(int iFlag)
{
    m_Config.bShowCostume = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowMoneyLog()
{
    return m_Config.bShowMoneyLog;
}

void CClientConfig::SetShowMoneyLog(int iFlag)
{
    m_Config.bShowMoneyLog = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowItemLog()
{
    return m_Config.bShowItemLog;
}

void CClientConfig::SetShowItemLog(int iFlag)
{
    m_Config.bShowItemLog = iFlag == 1 ? true : false;
}

bool CClientConfig::IsPickupInfoWindowActive()
{
    return m_Config.bPickupInfoWindowActive;
}

void CClientConfig::SetPickupInfoWindowActive(int iFlag)
{
    m_Config.bPickupInfoWindowActive = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowSpecular()
{
    return m_Config.bShowSpecular;
}

void CClientConfig::SetShowSpecular(int iFlag)
{
    m_Config.bShowSpecular = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowGeneralEffects()
{
    return m_Config.bShowGeneralEffect;
}

void CClientConfig::SetShowGeneralEffects(int iFlag)
{
    m_Config.bShowGeneralEffect = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowBuffEffects()
{
    return m_Config.bShowBuffEffects;
}

void CClientConfig::SetShowBuffEffects(int iFlag)
{
    m_Config.bShowBuffEffects = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowSkillEffects()
{
    return m_Config.bShowSkillEffects;
}

void CClientConfig::SetShowSkillEffects(int iFlag)
{
    m_Config.bShowSkillEffects = iFlag == 1 ? true : false;
}

bool CClientConfig::IsUsingItemHighlight()
{
    return m_Config.bItemHighlight;
}

void CClientConfig::SetUsingItemHighlight(int iFlag)
{
    m_Config.bItemHighlight = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowMobLevel()
{
    return m_Config.bShowMobLevel;
}

void CClientConfig::SetShowMobLevel(int iFlag)
{
    m_Config.bShowMobLevel = iFlag == 1 ? true : false;
}

bool CClientConfig::IsShowMobAIFlag()
{
    return m_Config.bShowMobAggr;
}

void CClientConfig::SetShowMobAIFlag(int iFlag)
{
    m_Config.bShowMobAggr = iFlag == 1 ? true : false;
}

bool CClientConfig::IsEnableSnowFall()
{
    return m_Config.bEnableSnow;
}

void CClientConfig::SetEnableSnowFall(int iFlag)
{
    m_Config.bEnableSnow = iFlag == 1 ? true : false;
}

void CClientConfig::SetShowSalesTextFlag(int iFlag)
{
    m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

void CClientConfig::SetFogMode(int iFlag)
{
    m_Config.bFogMode = iFlag == 1 ? true : false;
}

bool CClientConfig::IsFogMode()
{
    return m_Config.bFogMode;
}

bool CClientConfig::IsEnableNightMode()
{
    return m_Config.bEnableNight;
}

void CClientConfig::SetEnableNightMode(int iFlag)
{
    m_Config.bEnableNight = iFlag == 1 ? true : false;
}

bool CClientConfig::IsEnableSnowTexture()
{
    return m_Config.bEnableSnowTexture;
}

void CClientConfig::SetEnableeSnowTexture(int iFlag)
{
    m_Config.bEnableSnowTexture = iFlag == 1 ? true : false;
}

bool CClientConfig::IsEnableEmojiSystem()
{
    return m_Config.bEnableEmojiSystem;
}

void CClientConfig::SetEnableEmojiSystem(int iFlag)
{
    m_Config.bEnableEmojiSystem = iFlag == 1 ? true : false;
}

float ConvertRatioVolumeToApplyVolume(float fRatioVolume)
{
    if (0.1f > fRatioVolume)
        return fRatioVolume;

    return (float)std::pow(10.0f, (-1.0f + fRatioVolume));
}

float ConvertGradeVolumeToApplyVolume(int grade)
{
    return ConvertRatioVolumeToApplyVolume(grade / 5.0f);
}

bool CClientConfig::LoadConfig()
{
    std::ifstream t("metin2.cfg");
    if (!t.is_open())
    {
        SPDLOG_INFO("Configuration file not found using default configuration");
        return false;
    }

    std::string text;

    t.seekg(0, std::ios::end);
    text.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    text.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

    std::vector<std::string> lines;
    storm::Tokenize(text, "\r\n", lines);

    std::vector<storm::StringRef> args;

    int i = 0;
    for (const auto &line : lines)
    {
        args.clear();
        storm::ExtractArguments(line, args);

        if (args.empty())
            continue;

        if (args.size() != 2)
        {
            SPDLOG_WARN("Config '{0}' line {1} has only {2} token(s)", "metin2.cfg", i, args.size());
            continue;
        }

        SPDLOG_INFO("{} -> {}", args[0], args[1]);

        if (storm::EqualsIgnoreCase(args[0], "WIDTH"))
        {
            storm::ParseNumber(args[1], m_Config.width);
        }
        else if (storm::EqualsIgnoreCase(args[0], "HEIGHT"))
        {
            storm::ParseNumber(args[1], m_Config.height);
        }
        else if (storm::EqualsIgnoreCase(args[0], "BPP"))
        {
            storm::ParseNumber(args[1], m_Config.bpp);
        }
        else if (storm::EqualsIgnoreCase(args[0], "FREQUENCY"))
        {
            storm::ParseNumber(args[1], m_Config.frequency);
        }
        else if (storm::EqualsIgnoreCase(args[0], "SOFTWARE_CURSOR"))
        {
            storm::ParseNumber(args[1], m_Config.is_software_cursor);
        }
        else if (storm::EqualsIgnoreCase(args[0], "VISIBILITY"))
        {
            storm::ParseNumber(args[1], m_Config.iDistance);
        }
        else if (storm::EqualsIgnoreCase(args[0], "MUSIC_VOLUME"))
        {
            if (args[1].find('.') == std::string_view::npos)
            {
                uint8_t grade;
                storm::ParseNumber(args[1], grade);
                m_Config.music_volume = ConvertGradeVolumeToApplyVolume(grade);
            }
            else
            {
                storm::ParseNumber(args[1], m_Config.music_volume);
            }
        }
        else if (storm::EqualsIgnoreCase(args[0], "VOICE_VOLUME"))
        {
            if (args[1].find('.') == std::string_view::npos)
            {
                uint8_t grade;
                storm::ParseNumber(args[1], grade);
                m_Config.voice_volume = ConvertGradeVolumeToApplyVolume(grade);
            }
            else
            {
                storm::ParseNumber(args[1], m_Config.voice_volume);
            }
        }
        else if (storm::EqualsIgnoreCase(args[0], "GAMMA"))
        {
            storm::ParseNumber(args[1], m_Config.gamma);
        }

        else if (storm::EqualsIgnoreCase(args[0], "SHOW_FPS"))
        {
            storm::ParseNumber(args[1], m_Config.is_show_fps);
        }
        else if (storm::EqualsIgnoreCase(args[0], "WINDOWED"))
        {
            storm::ParseNumber(args[1], m_Config.bWindowed);
        }
        else if (storm::EqualsIgnoreCase(args[0], "SHADOW_LEVEL"))
            storm::ParseNumber(args[1], m_Config.iShadowLevel);
        else if (storm::EqualsIgnoreCase(args[0], "CAMERA_DISTANCE_MODE"))
            storm::ParseNumber(args[1], m_Config.iCameraDistanceMode);
        else if (storm::EqualsIgnoreCase(args[0], "NEARBY_SHOPS_DISPLAYED"))
            storm::ParseNumber(args[1], m_Config.iNearbyShopsDisplayed);
        else if (storm::EqualsIgnoreCase(args[0], "VIEW_CHAT"))
            storm::ParseNumber(args[1], m_Config.bViewChat);
        else if (storm::EqualsIgnoreCase(args[0], "ALWAYS_VIEW_PLAYER_NAME"))
            storm::ParseNumber(args[1], m_Config.bAlwaysShowNamePlayer);
        else if (storm::EqualsIgnoreCase(args[0], "ALWAYS_VIEW_NPC_NAME"))
            storm::ParseNumber(args[1], m_Config.bAlwaysShowNameNpc);
        else if (storm::EqualsIgnoreCase(args[0], "ALWAYS_VIEW_MONSTER_NAME"))
            storm::ParseNumber(args[1], m_Config.bAlwaysShowNameMonster);
        else if (storm::EqualsIgnoreCase(args[0], "ALWAYS_VIEW_ITEM_NAME"))
            storm::ParseNumber(args[1], m_Config.bAlwaysShowNameItem);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_DAMAGE"))
            storm::ParseNumber(args[1], m_Config.bShowDamage);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_SALESTEXT"))
            storm::ParseNumber(args[1], m_Config.bShowSalesText);
#if defined(WJ_SHOW_MOB_INFO)
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_MOBLEVEL"))
            storm::ParseNumber(args[1], m_Config.bShowMobLevel);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_MOBAIFLAG"))
            storm::ParseNumber(args[1], m_Config.bShowMobAggr);
#endif
#if defined(WJ_SHOW_ACCE_ONOFF)
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_ACCE"))
            storm::ParseNumber(args[1], m_Config.bShowAcce);
#endif
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_COSTUME"))
            storm::ParseNumber(args[1], m_Config.bShowCostume);

        else if (storm::EqualsIgnoreCase(args[0], "SHOW_GENERAL_EFFECTS"))
            storm::ParseNumber(args[1], m_Config.bShowGeneralEffect);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_BUFF_EFFECTS"))
            storm::ParseNumber(args[1], m_Config.bShowBuffEffects);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_SKILL_EFFECTS"))
            storm::ParseNumber(args[1], m_Config.bShowSkillEffects);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_MONEY_LOG"))
            storm::ParseNumber(args[1], m_Config.bShowMoneyLog);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_ITEM_LOG"))
            storm::ParseNumber(args[1], m_Config.bShowItemLog);
        else if (storm::EqualsIgnoreCase(args[0], "PICKUP_INFO_WINDOW_ACTIVE"))
            storm::ParseNumber(args[1], m_Config.bPickupInfoWindowActive);
        else if (storm::EqualsIgnoreCase(args[0], "USE_ITEM_HIGHLIGHT"))
            storm::ParseNumber(args[1], m_Config.bItemHighlight);
        else if (storm::EqualsIgnoreCase(args[0], "FOG_MODE_ON"))
            storm::ParseNumber(args[1], m_Config.bFogMode);
        else if (storm::EqualsIgnoreCase(args[0], "NIGHT_MODE_ON"))
            storm::ParseNumber(args[1], m_Config.bEnableNight);
        else if (storm::EqualsIgnoreCase(args[0], "SNOW_MODE_ON"))
            storm::ParseNumber(args[1], m_Config.bEnableSnow);
        else if (storm::EqualsIgnoreCase(args[0], "SNOW_TEXTURE_MODE_ON"))
            storm::ParseNumber(args[1], m_Config.bEnableSnowTexture);
        else if (storm::EqualsIgnoreCase(args[0], "SHOW_COUNTRY_FLAGS"))
            storm::ParseNumber(args[1], m_Config.bShowCountryFlags);
        else if (storm::EqualsIgnoreCase(args[0], "COLLECT_EQUIPMENT"))
            storm::ParseNumber(args[1], m_Config.bCollectEquipment);
        else if (storm::EqualsIgnoreCase(args[0], "COLLECT_USEABLE_EQUIPMENT"))
            storm::ParseNumber(args[1], m_Config.bCollectUseableEquipment);
        else if (storm::EqualsIgnoreCase(args[0], "HIDE_PETS"))
            storm::ParseNumber(args[1], m_Config.bHidePets);
        else if (storm::EqualsIgnoreCase(args[0], "HIDE_GROWTH_PETS"))
            storm::ParseNumber(args[1], m_Config.bHideGrowthPets);
        else if (storm::EqualsIgnoreCase(args[0], "HIDE_SHOPS"))
            storm::ParseNumber(args[1], m_Config.bHideShops);
        else if (storm::EqualsIgnoreCase(args[0], "HIDE_MOUNTS"))
            storm::ParseNumber(args[1], m_Config.bHideMounts);
        else if (storm::EqualsIgnoreCase(args[0], "ENABLE_EMOJI_SYSTEM"))
            storm::ParseNumber(args[1], m_Config.bEnableEmojiSystem);
        else if (storm::EqualsIgnoreCase(args[0], "ENABLE_FRIEND_NOTIFY"))
            storm::ParseNumber(args[1], m_Config.bShowFriendNotifcations);
        else if (storm::EqualsIgnoreCase(args[0], "ENABLE_GUILD_NOTIFY"))
            storm::ParseNumber(args[1], m_Config.bShowGuildNotifcations);
        else if (storm::EqualsIgnoreCase(args[0], "ENABLE_GM_NOTIFY"))
            storm::ParseNumber(args[1], m_Config.bShowGMNotifcations);
#ifdef ENABLE_PERSPECTIVE_VIEW
        else if (storm::EqualsIgnoreCase(args[0], "PERSPECTIVE_FIELD"))
            storm::ParseNumber(args[1], m_Config.fField);
#endif

        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_SWORD)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_SWORD]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_DAGGER)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_DAGGER]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_BOW)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_BOW]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_TWO_HAND)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_TWO_HAND]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_BELL)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_BELL]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_FAN)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_FAN]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_ARMOR)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_ARMOR]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_HEAD)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_HEAD]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_SHIELD)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_SHIELD]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_WRIST)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_WRIST]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_FOOTS)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_FOOTS]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_NECK)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_NECK]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_EAR)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_EAR]);
        else if (!storm::EqualsIgnoreCase(args[0], DEF_STR(PICKUP_IGNORE_ETC)))
            storm::ParseNumber(args[1], m_Config.b_arPickupIgnore[PICKUP_IGNORE_ETC]);

    }

    if (m_Config.bWindowed)
    {
        unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
        unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

        if (m_Config.width >= screen_width)
        {
            m_Config.width = screen_width;
        }
        if (m_Config.height >= screen_height)
        {
            m_Config.height = screen_height;
        }
    }

    m_OldConfig = m_Config;

    return true;
}

bool CClientConfig::SaveConfig()
{
    FILE *fp;

    if (NULL == (fp = fopen("metin2.cfg", "wt")))
        return false;

    fprintf(fp,
            "WIDTH						%d\n"
            "HEIGHT						%d\n"
            "BPP						%d\n"
            "FREQUENCY					%d\n"
            "SOFTWARE_CURSOR			%d\n"
            "VISIBILITY					%d\n"
            "MUSIC_VOLUME				%.3f\n"
            "VOICE_VOLUME				%.3f\n"
            "GAMMA						%d\n",
            m_Config.width, m_Config.height, m_Config.bpp, m_Config.frequency, m_Config.is_software_cursor,
            m_Config.iDistance, m_Config.music_volume, m_Config.voice_volume, m_Config.gamma);

    fprintf(fp, "WINDOWED				%d\n", m_Config.bWindowed);
    if (m_Config.bViewChat == 0)
        fprintf(fp, "VIEW_CHAT				%d\n", m_Config.bViewChat);
    if (m_Config.bAlwaysShowNamePlayer != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
        fprintf(fp, "ALWAYS_VIEW_PLAYER_NAME		%d\n", m_Config.bAlwaysShowNamePlayer);
    if (m_Config.bAlwaysShowNameNpc != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
        fprintf(fp, "ALWAYS_VIEW_NPC_NAME		%d\n", m_Config.bAlwaysShowNameNpc);
    if (m_Config.bAlwaysShowNameMonster != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
        fprintf(fp, "ALWAYS_VIEW_MONSTER_NAME		%d\n", m_Config.bAlwaysShowNameMonster);
    if (m_Config.bAlwaysShowNameItem != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
        fprintf(fp, "ALWAYS_VIEW_ITEM_NAME		%d\n", m_Config.bAlwaysShowNameItem);

    if (m_Config.bShowDamage == 0)
        fprintf(fp, "SHOW_DAMAGE		%d\n", m_Config.bShowDamage);
    if (m_Config.bShowSalesText == 0)
        fprintf(fp, "SHOW_SALESTEXT		%d\n", m_Config.bShowSalesText);
    fprintf(fp, "SHOW_FPS		%d\n", m_Config.is_show_fps);

    fprintf(fp, "FOG_MODE_ON		%d\n", m_Config.bFogMode);
    fprintf(fp, "NIGHT_MODE_ON		%d\n", m_Config.bEnableNight);
    fprintf(fp, "SNOW_MODE_ON		%d\n", m_Config.bEnableSnow);
    fprintf(fp, "SNOW_TEXTURE_MODE_ON		%d\n", m_Config.bEnableSnowTexture);

#if defined(WJ_SHOW_MOB_INFO)
    fprintf(fp, "SHOW_MOBLEVEL		%d\n", m_Config.bShowMobLevel);
    fprintf(fp, "SHOW_MOBAIFLAG		%d\n", m_Config.bShowMobAggr);
#endif
#if defined(WJ_SHOW_ACCE_ONOFF)
    fprintf(fp, "SHOW_ACCE		%d\n", m_Config.bShowAcce);
#endif
    fprintf(fp, "SHOW_COSTUME		%d\n", m_Config.bShowCostume);
    fprintf(fp, "SHOW_GENERAL_EFFECTS		%d\n", m_Config.bShowGeneralEffect);
    fprintf(fp, "SHOW_BUFF_EFFECTS		%d\n", m_Config.bShowBuffEffects);
    fprintf(fp, "SHOW_SKILL_EFFECTS		%d\n", m_Config.bShowSkillEffects);

    fprintf(fp, "SHOW_COUNTRY_FLAGS		%d\n", m_Config.bShowCountryFlags);
    fprintf(fp, "COLLECT_EQUIPMENT		%d\n", m_Config.bCollectEquipment);
    fprintf(fp, "COLLECT_USEABLE_EQUIPMENT		%d\n", m_Config.bCollectUseableEquipment);
    fprintf(fp, "HIDE_PETS				%d\n", m_Config.bHidePets);
    fprintf(fp, "HIDE_GROWTH_PETS				%d\n", m_Config.bHideGrowthPets);

    fprintf(fp, "HIDE_SHOPS				%d\n", m_Config.bHideShops);
    fprintf(fp, "HIDE_MOUNTS			%d\n", m_Config.bHideMounts);
    fprintf(fp, "ENABLE_EMOJI_SYSTEM			%d\n", m_Config.bEnableEmojiSystem);

    fprintf(fp, "ENABLE_FRIEND_NOTIFY			%d\n", m_Config.bShowFriendNotifcations);
    fprintf(fp, "ENABLE_GUILD_NOTIFY			%d\n", m_Config.bShowGuildNotifcations);
    fprintf(fp, "ENABLE_GM_NOTIFY			%d\n", m_Config.bShowGMNotifcations);
#ifdef ENABLE_PERSPECTIVE_VIEW
    fprintf(fp, "PERSPECTIVE_FIELD		     %.2f\n", m_Config.fField);

#endif
    fprintf(fp, "USE_ITEM_HIGHLIGHT		%d\n", m_Config.bItemHighlight);
    fprintf(fp, "SHOW_MONEY_LOG		%d\n", m_Config.bShowMoneyLog);
    fprintf(fp, "SHOW_ITEM_LOG		%d\n", m_Config.bShowItemLog);
    fprintf(fp, "PICKUP_INFO_WINDOW_ACTIVE		%d\n", m_Config.bPickupInfoWindowActive);

    fprintf(fp, "CAMERA_DISTANCE_MODE		%d\n", m_Config.iCameraDistanceMode);
    fprintf(fp, "SHADOW_LEVEL			%d\n", m_Config.iShadowLevel);
    fprintf(fp, "NEARBY_SHOPS_DISPLAYED	%d\n", m_Config.iNearbyShopsDisplayed);

    for (int i = 0; i < PICKUP_IGNORE_MAX_NUM; ++i)
    {
        char ignoreTypes[PICKUP_IGNORE_MAX_NUM][50] = {
            DEF_STR(PICKUP_IGNORE_WEAPON), DEF_STR(PICKUP_IGNORE_ARMOR), DEF_STR(PICKUP_IGNORE_HEAD),
            DEF_STR(PICKUP_IGNORE_SHIELD), DEF_STR(PICKUP_IGNORE_WRIST), DEF_STR(PICKUP_IGNORE_FOOTS),
            DEF_STR(PICKUP_IGNORE_NECK),   DEF_STR(PICKUP_IGNORE_EAR),   DEF_STR(PICKUP_IGNORE_ETC),
        };

        if (m_Config.b_arPickupIgnore[i])
        {
            fprintf(fp, "%s %d\n", ignoreTypes[i], m_Config.b_arPickupIgnore[i]);
        }
    }

    fprintf(fp, "\n");

    fclose(fp);
    return true;
}

bool CClientConfig::GetPickupIgnore(int iType)
{
    if (iType < 0 || iType >= PICKUP_IGNORE_MAX_NUM)
        return false;

    return m_Config.b_arPickupIgnore[iType];
}

void CClientConfig::SetPickupIgnore(int iType, int iIgnore)
{
    if (iType < 0 || iType >= PICKUP_IGNORE_MAX_NUM)
        return;

    m_Config.b_arPickupIgnore[iType] = iIgnore == 1 ? true : false;
}

void CClientConfig::ChangeSystem()
{
}

CClientConfig::CClientConfig()
{
    memset(&m_Config, 0, sizeof(TConfig));

    SetDefaultConfig();
}
