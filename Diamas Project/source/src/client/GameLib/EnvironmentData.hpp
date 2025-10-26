#ifndef METIN2_CLIENT_GAMELIB_ENVIRONMENTDATA_HPP
#define METIN2_CLIENT_GAMELIB_ENVIRONMENTDATA_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

enum
{
    ENV_DIRLIGHT_BACKGROUND,
    ENV_DIRLIGHT_CHARACTER,
    ENV_DIRLIGHT_NUM
};

struct EnvironmentData
{
    typedef std::pair<DirectX::SimpleMath::Color, DirectX::SimpleMath::Color> GradientColor;

    EnvironmentData();

    bool Load(const char *filename);
    bool Save(std::string_view filename) const;

    // World Editor APIs:
    bool GetSkyBoxGradientColor(uint32_t index, GradientColor &gradientColor) const;
    bool SetSkyBoxGradientColor(uint32_t index, const GradientColor &gradientColor);

    void InsertGradientUpper();
    void InsertGradientLower();
    void DeleteGradient(uint32_t index);

    // Light
    bool bDirLightsEnable[ENV_DIRLIGHT_NUM];
    D3DLIGHT9 DirLights[ENV_DIRLIGHT_NUM];

    // Material
    D3DMATERIAL9 Material;

    // Fog
    bool bFogEnable;
    bool bDensityFog;

    float m_fFogNearDistance;
    float m_fFogFarDistance;

    DirectX::SimpleMath::Color FogColor;

    // Filtering
    bool bFilteringEnable;
    DirectX::SimpleMath::Color FilteringColor;
    uint8_t byFilteringAlphaSrc;
    uint8_t byFilteringAlphaDest;

    // Wind
    float fWindStrength;
    float fWindRandom;

    // SkyBox
    Vector3 v3SkyBoxScale;
    bool bSkyBoxTextureRenderMode;

    uint8_t bySkyBoxGradientLevelUpper;
    uint8_t bySkyBoxGradientLevelLower;

    std::string strSkyBoxFaceFileName[6]; // order : front/back/left/right/top/bottom

    Vector2 v2CloudScale;
    float fCloudHeight;
    Vector2 v2CloudTextureScale;
    Vector2 v2CloudSpeed;
    std::string strCloudTextureFileName;
    GradientColor CloudGradientColor;
    std::vector<GradientColor> SkyBoxGradientColorVector;

    // LensFlare
    bool bLensFlareEnable;
    DirectX::SimpleMath::Color LensFlareBrightnessColor;
    float fLensFlareMaxBrightness;

    bool bMainFlareEnable;
    std::string strMainFlareTextureFileName;
    float fMainFlareSize;

    bool bReserve; // 외부에서 설정을 고치지 않음
};

#endif
