#pragma once

#include <Config.hpp>
#include <optional>

#include "../EterLib/VideoModeList.hpp"
#include "../eterlib/Camera.h"

class CClientConfig
{
  public:
    enum
    {
        FREQUENCY_MAX_NUM = 30,
        RESOLUTION_MAX_NUM = 100
    };

    enum EPickupIgnores
    {
        PICKUP_IGNORE_SWORD,
        PICKUP_IGNORE_DAGGER,
        PICKUP_IGNORE_BOW,
        PICKUP_IGNORE_TWO_HAND,
        PICKUP_IGNORE_BELL,
        PICKUP_IGNORE_FAN,
        PICKUP_IGNORE_ARMOR,
        PICKUP_IGNORE_HEAD,
        PICKUP_IGNORE_SHIELD,
        PICKUP_IGNORE_WRIST,
        PICKUP_IGNORE_FOOTS,
        PICKUP_IGNORE_NECK,
        PICKUP_IGNORE_EAR,
        PICKUP_IGNORE_ETC,
        PICKUP_IGNORE_RING,
        PICKUP_IGNORE_TALISMAN,
        PICKUP_IGNORE_ELIXIR,
        PICKUP_IGNORE_MAX_NUM,
    };

    typedef struct SResolution
    {
        uint32_t width;
        uint32_t height;
        uint32_t bpp; // bits per pixel (high-color = 16bpp, true-color = 32bpp)

        uint32_t frequency[20];
        uint8_t frequency_count;
    } TResolution;

    typedef struct SConfig
    {
        uint32_t width;
        uint32_t height;
        uint32_t bpp;
        uint32_t frequency;

        uint8_t is_software_cursor;
        uint8_t is_show_fps;
        int iDistance;
        int8_t iCameraDistanceMode;
        int iShadowLevel;
        int iNearbyShopsDisplayed;

        float music_volume;
        float voice_volume;

        int gamma;

        uint8_t bWindowed;
        uint8_t bViewChat;
        uint8_t bAlwaysShowNamePlayer;
        uint8_t bAlwaysShowNameNpc;
        uint8_t bAlwaysShowNameMonster;
        uint8_t bAlwaysShowNameItem;

        uint8_t bShowDamage;
        uint8_t bShowSalesText;
        uint8_t bShowAcce;
        uint8_t bShowSpecular;
        uint8_t bShowCostume;

        uint8_t bShowGeneralEffect;
        uint8_t bShowBuffEffects;
        uint8_t bShowSkillEffects;

        uint8_t bShowMobAggr;
        uint8_t bShowMobLevel;
        uint8_t bEnableNight;
        uint8_t bEnableSnow;
        uint8_t bEnableSnowTexture;
        uint8_t bShowMoneyLog;
        uint8_t bShowItemLog;
        uint8_t bPickupInfoWindowActive;
        uint8_t bItemHighlight;
        uint8_t bFogMode;
        uint8_t bShowCountryFlags;
        uint8_t bCollectEquipment;
        uint8_t bCollectUseableEquipment;
        uint8_t bHidePets;
        uint8_t bHideGrowthPets;
        uint8_t bHideShops;
        uint8_t bHideMounts;
        uint8_t bEnableEmojiSystem;
        uint8_t b_arPickupIgnore[PICKUP_IGNORE_MAX_NUM];
        uint8_t bUseShaderCache = false;
        uint8_t bShowFriendNotifcations = true;
        uint8_t bShowGuildNotifcations = true;
        uint8_t bShowGMNotifcations = true;
#ifdef ENABLE_PERSPECTIVE_VIEW
        float fField;
#endif
    } TConfig;

  public:
    CClientConfig();
    ~CClientConfig() = default;

    CClientConfig(const CClientConfig &other) = delete;

    CClientConfig(CClientConfig &&other)
        : m_videoModes(std::move(other.m_videoModes)),
          m_Config(std::move(other.m_Config)),
          m_OldConfig(std::move(other.m_OldConfig)),
          m_isInterfaceConfig(other.m_isInterfaceConfig)
    {
    }

    CClientConfig & operator=(const CClientConfig &other) = delete;

    CClientConfig & operator=(CClientConfig &&other)
    {
        if (this == &other)
            return *this;
        m_videoModes = std::move(other.m_videoModes);
        m_Config = std::move(other.m_Config);
        m_OldConfig = std::move(other.m_OldConfig);
        m_isInterfaceConfig = other.m_isInterfaceConfig;
        return *this;
    }

    // Config
    void SetDefaultConfig();
    bool LoadConfig();
    bool SaveConfig();
    void SetConfig(TConfig *set_config);
    void ChangeSystem();

    uint32_t GetGamma();
    uint8_t IsShowFPS();
    void SetShowFPS(uint8_t val);
    TConfig *GetConfig();
    uint32_t GetWidth();
    uint32_t GetHeight();
    uint32_t GetBPP();
    uint32_t GetFrequency();

    bool IsSoftwareCursor();
    bool IsWindowed();
    void SetWindowed(bool val);
    bool IsViewChat();
    bool IsAlwaysShowPlayerName();
    bool IsAlwaysShowMonsterName();
    bool IsAlwaysShowNPCName();
    bool IsAlwaysShowItemName();
    bool IsShowDamage();
    bool IsShowSalesText();

    bool IsShowCountryFlags()
    {
        return m_Config.bShowCountryFlags;
    }

    bool IsCollectEquipment()
    {
        return m_Config.bCollectEquipment;
    }

    bool IsCollectUseableEquipment()
    {
        return m_Config.bCollectUseableEquipment;
    }

    bool IsHideShops()
    {
        return m_Config.bHideShops;
    }

    bool IsHidePets()
    {
        return m_Config.bHidePets;
    }
    bool IsUsingShaderCache()
    {
        return m_Config.bUseShaderCache;
    }
    bool IsHideGrowthPets()
    {
        return m_Config.bHideGrowthPets;
    }

    bool IsHideMounts()
    {
        return m_Config.bHideMounts;
    }

    bool IsShowGMNotifications() const
    {
        return m_Config.bShowGMNotifcations;
    }
    void SetShowGMNotifications(bool iFlag)
    {
        m_Config.bShowGMNotifcations = iFlag;
    }
    bool IsShowFriendNotifications() const
    {
        return m_Config.bShowFriendNotifcations;
    }
    void SetShowFriendNotifications(bool iFlag)
    {
        m_Config.bShowFriendNotifcations = iFlag;
    }
    bool IsShowGuildNotifications() const
    {
        return m_Config.bShowGuildNotifcations;
    }
    void SetShowGuildNotifications(bool iFlag)
    {
        m_Config.bShowGuildNotifcations = iFlag;
    }

    bool IsFogMode();
    bool IsEnableNightMode();
    bool IsEnableSnowTexture();
    bool IsEnableEmojiSystem();
    bool IsShowMoneyLog();

    void SetViewChatFlag(int iFlag);
    void SetAlwaysShowPlayerNameFlag(int iFlag);
    void SetAlwaysShowNPCNameFlag(int iFlag);
    void SetAlwaysShowMonsterNameFlag(int iFlag);
    void SetAlwaysShowItemNameFlag(int iFlag);

    void SetShowDamageFlag(int iFlag);
    void SetShowSalesTextFlag(int iFlag);
    void SetFogMode(int iFlag);
    void SetEnableNightMode(int iFlag);
    void SetEnableeSnowTexture(int iFlag);
    void SetEnableEmojiSystem(int iFlag);
    void SetShowMoneyLog(int iFlag);
    bool IsShowItemLog();
    void SetShowItemLog(int iFlag);
    bool IsPickupInfoWindowActive();
    void SetPickupInfoWindowActive(int iFlag);

    void SetShowCountryFlags(bool iFlag)
    {
        m_Config.bShowCountryFlags = iFlag;
    }

    void SetCollectEquipment(bool iFlag)
    {
        m_Config.bCollectEquipment = iFlag;
    }

    void SetCollectUseableEquipment(bool iFlag)
    {
        m_Config.bCollectUseableEquipment = iFlag;
    }

    void SetHideShops(bool iFlag)
    {
        m_Config.bHideShops = iFlag;
    }

    void SetHidePets(bool iFlag)
    {
        m_Config.bHidePets = iFlag;
    }

    void SetHideGrowthPets(bool iFlag)
    {
        m_Config.bHideGrowthPets = iFlag;
    }

    void SetHideMounts(bool iFlag)
    {
        m_Config.bHideMounts = iFlag;
    }

#if defined(WJ_SHOW_MOB_INFO)
    void SetShowMobAIFlag(int iFlag);
    bool IsShowMobAIFlag();
    void SetShowMobLevel(int iFlag);
    bool IsEnableSnowFall();
    void SetEnableSnowFall(int iFlag);
    bool IsShowMobLevel();
#endif
#if defined(WJ_SHOW_ACCE_ONOFF)
    bool IsShowAcce() const;
    void SetShowAcce(int iFlag);
    bool IsShowCostume() const;
    void SetShowCostume(int iFlag);
#endif
    bool IsShowSpecular();
    void SetShowSpecular(int iFlag);
    bool IsShowGeneralEffects();
    void SetShowGeneralEffects(int iFlag);
    bool IsShowBuffEffects();
    void SetShowBuffEffects(int iFlag);
    bool IsShowSkillEffects();
    void SetShowSkillEffects(int iFlag);
    bool IsUsingItemHighlight();
    void SetUsingItemHighlight(int iFlag);

    bool GetPickupIgnore(int iType);
    void SetPickupIgnore(int iType, int iIgnore);

    /// Display

    int GetResolutionCount() const;
    bool SetResolution(int index);
    bool IsResolutionByDescription(const std::string &desc);
    std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> GetResolution(int index);

    // Sound
    float GetMusicVolume();
    float GetSoundVolume();
    void SetMusicVolume(float fVolume);
    void SetSoundVolumef(float fVolume);

    int GetViewDistance();
    void SetViewDistance(int distance);
    int GetShadowLevel();
    void SetShadowLevel(unsigned int level);

    int8_t GetCameraDistanceMode() const
    {
        return m_Config.iCameraDistanceMode;
    }

    void SetCameraDistanceMode(int8_t distanceMode);

    int GetNearbyShopsDisplayed();
    void SetNearbyShopsDisplayed(unsigned int newValue);
    void CreateVideoModeList();

#ifdef ENABLE_PERSPECTIVE_VIEW
    float GetFieldPerspective() { return m_Config.fField; }

    void SetFieldPerspective(float fValue)
    {
        m_Config.fField = fValue;
    }
#endif

  protected:
    std::unique_ptr<Graphics::VideoModeList> m_videoModes;

    TConfig m_Config;
    TConfig m_OldConfig;

    bool m_isInterfaceConfig;
};
