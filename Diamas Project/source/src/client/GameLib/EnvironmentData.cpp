#include "StdAfx.h"
#include "EnvironmentData.hpp"

#include "../EterLib/TextFileLoader.h"

EnvironmentData::EnvironmentData()
    : bFogEnable(false)
      , bDensityFog(false)
      , m_fFogNearDistance(25600.0f * 0.5f)
      , m_fFogFarDistance(25600.0f * 0.7f)
      , FogColor(0.5f, 0.5f, 0.5f, 1.0f)
      , bFilteringEnable(false)
      , FilteringColor(0.3f, 0.1f, 0.1f, 0.0f)
      , byFilteringAlphaSrc(D3DBLEND_ONE)
      , byFilteringAlphaDest(D3DBLEND_ONE)
      , fWindStrength(0.2f)
      , fWindRandom(0.0f)
      , v3SkyBoxScale(3500.0f, 3500.0f, 3500.0f)
      , bSkyBoxTextureRenderMode(false)
      , bySkyBoxGradientLevelUpper(0)
      , bySkyBoxGradientLevelLower(0)
      , v2CloudScale(200000.0f, 200000.0f)
      , fCloudHeight(30000.0f)
      , v2CloudTextureScale(4.0f, 4.0f)
      , v2CloudSpeed(0.001f, 0.001f)
      , bLensFlareEnable(false)
      , LensFlareBrightnessColor(1.0f, 1.0f, 1.0f, 1.0f)
      , fLensFlareMaxBrightness(1.0f)
      , bMainFlareEnable(false)
      , fMainFlareSize(0.2f)
      , bReserve(false)
{
    for (int i = 0; i != ENV_DIRLIGHT_NUM; ++i)
    {
        bDirLightsEnable[i] = false;
        DirLights[i].Type = D3DLIGHT_DIRECTIONAL;
        DirLights[i].Direction = {0.5f, 0.5f, -0.5f};
        DirLights[i].Position = {0.0f, 0.0f, 0.0f};
        DirLights[i].Specular = {1.0f, 1.0f, 1.0f, 1.0f};
        DirLights[i].Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
        DirLights[i].Ambient = {0.5f, 0.5f, 0.5f, 1.0f};
        DirLights[i].Range = 0.0f;   // Used by Point Light & Spot Light
        DirLights[i].Falloff = 1.0f; // Used by Spot Light
        DirLights[i].Theta = 0.0f;   // Used by Spot Light
        DirLights[i].Phi = 0.0f;     // Used by Spot Light
        DirLights[i].Attenuation0 = 0.0f;
        DirLights[i].Attenuation1 = 1.0f;
        DirLights[i].Attenuation2 = 0.0f;
    }

    Material.Diffuse = {0.8f, 0.8f, 0.8f, 1.0f};
    Material.Ambient = {0.8f, 0.8f, 0.8f, 1.0f};
    Material.Emissive = {0.8f, 0.8f, 0.8f, 1.0f};
    Material.Specular = {0.0f, 0.0f, 0.0f, 1.0f};
    Material.Power = 0.0f;

    CloudGradientColor.first = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);
    CloudGradientColor.second = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);
}

bool EnvironmentData::Load(const char *filename)
{
    CTextFileLoader textLoader;

    if (!textLoader.Load(filename))
        return false;

    textLoader.SetTop();

    textLoader.GetTokenBoolean("reserved", &bReserve);

    if (textLoader.SetChildNode("directionallight"))
    {
        D3DVECTOR v3Dir;
        textLoader.GetTokenDirection("direction", &v3Dir);

        if (textLoader.SetChildNode("background"))
        {
            DirLights[ENV_DIRLIGHT_BACKGROUND].Direction = v3Dir;
            textLoader.GetTokenBoolean("enable", &bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND]);
            textLoader.GetTokenColor("diffuse", &DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse);
            textLoader.GetTokenColor("ambient", &DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient);
            textLoader.SetParentNode();
        }

        if (textLoader.SetChildNode("character"))
        {
            DirLights[ENV_DIRLIGHT_CHARACTER].Direction = v3Dir;
            textLoader.GetTokenBoolean("enable", &bDirLightsEnable[ENV_DIRLIGHT_CHARACTER]);
            textLoader.GetTokenColor("diffuse", &DirLights[ENV_DIRLIGHT_CHARACTER].Diffuse);
            textLoader.GetTokenColor("ambient", &DirLights[ENV_DIRLIGHT_CHARACTER].Ambient);
            textLoader.SetParentNode();
        }

        textLoader.SetParentNode();
    }

    if (textLoader.SetChildNode("material"))
    {
        textLoader.GetTokenColor("diffuse", &Material.Diffuse);
        textLoader.GetTokenColor("ambient", &Material.Ambient);
        textLoader.GetTokenColor("emissive", &Material.Emissive);
        textLoader.SetParentNode();
    }

    if (textLoader.SetChildNode("fog"))
    {
        textLoader.GetTokenBoolean("enable", &bFogEnable);
        textLoader.GetTokenBoolean("isdensity", &bDensityFog);
        textLoader.GetTokenFloat("neardistance", &m_fFogNearDistance);
        textLoader.GetTokenFloat("fardistance", &m_fFogFarDistance);
        textLoader.GetTokenColor("color", &FogColor);
        textLoader.SetParentNode();
    }

    if (textLoader.SetChildNode("filter"))
    {
        textLoader.GetTokenBoolean("enable", &bFilteringEnable);
        textLoader.GetTokenColor("color", &FilteringColor);
        textLoader.GetTokenByte("alphasrc", &byFilteringAlphaSrc);
        textLoader.GetTokenByte("alphadest", &byFilteringAlphaDest);
        textLoader.SetParentNode();
    }

    if (textLoader.SetChildNode("skybox"))
    {
        textLoader.GetTokenBoolean("btexturerendermode", (bool *)&bSkyBoxTextureRenderMode);
        textLoader.GetTokenVector3("scale", &v3SkyBoxScale);
        textLoader.GetTokenByte("gradientlevelupper", &bySkyBoxGradientLevelUpper);
        textLoader.GetTokenByte("gradientlevellower", &bySkyBoxGradientLevelLower);

        textLoader.GetTokenString("frontfacefilename", &strSkyBoxFaceFileName[0]);
        textLoader.GetTokenString("backfacefilename", &strSkyBoxFaceFileName[1]);
        textLoader.GetTokenString("leftfacefilename", &strSkyBoxFaceFileName[2]);
        textLoader.GetTokenString("rightfacefilename", &strSkyBoxFaceFileName[3]);
        textLoader.GetTokenString("topfacefilename", &strSkyBoxFaceFileName[4]);
        textLoader.GetTokenString("bottomfacefilename", &strSkyBoxFaceFileName[5]);

        textLoader.GetTokenVector2("cloudscale", &v2CloudScale);
        textLoader.GetTokenFloat("cloudheight", &fCloudHeight);
        textLoader.GetTokenVector2("cloudtexturescale", &v2CloudTextureScale);
        textLoader.GetTokenVector2("cloudspeed", &v2CloudSpeed);
        textLoader.GetTokenString("cloudtexturefilename", &strCloudTextureFileName);

        CTokenVector *pTokenVectorCloudColor;
        if (textLoader.GetTokenVector("cloudcolor", &pTokenVectorCloudColor))
            if (0 == pTokenVectorCloudColor->size() % 8)
            {
                storm::ParseNumber(pTokenVectorCloudColor->at(0), CloudGradientColor.first.x);
                storm::ParseNumber(pTokenVectorCloudColor->at(1), CloudGradientColor.first.y);
                storm::ParseNumber(pTokenVectorCloudColor->at(2), CloudGradientColor.first.z);
                storm::ParseNumber(pTokenVectorCloudColor->at(3), CloudGradientColor.first.w);

                storm::ParseNumber(pTokenVectorCloudColor->at(4), CloudGradientColor.second.x);
                storm::ParseNumber(pTokenVectorCloudColor->at(5), CloudGradientColor.second.y);
                storm::ParseNumber(pTokenVectorCloudColor->at(6), CloudGradientColor.second.z);
                storm::ParseNumber(pTokenVectorCloudColor->at(7), CloudGradientColor.second.w);
            }

        uint8_t byGradientCount = bySkyBoxGradientLevelUpper + bySkyBoxGradientLevelLower;
        CTokenVector *pTokenVector;
        if (textLoader.GetTokenVector("gradient", &pTokenVector))
            if (0 == pTokenVector->size() % 8)
                if (byGradientCount == pTokenVector->size() / 8)
                {
                    SkyBoxGradientColorVector.clear();
                    SkyBoxGradientColorVector.resize(byGradientCount);
                    for (uint32_t i = 0; i < byGradientCount; ++i)
                    {
                        storm::ParseNumber(pTokenVector->at(i * 8 + 0), SkyBoxGradientColorVector[i].first.x);
                        storm::ParseNumber(pTokenVector->at(i * 8 + 1), SkyBoxGradientColorVector[i].first.y);
                        storm::ParseNumber(pTokenVector->at(i * 8 + 2), SkyBoxGradientColorVector[i].first.z);
                        storm::ParseNumber(pTokenVector->at(i * 8 + 3), SkyBoxGradientColorVector[i].first.w);

                        storm::ParseNumber(pTokenVector->at(i * 8 + 4), SkyBoxGradientColorVector[i].second.x);
                        storm::ParseNumber(pTokenVector->at(i * 8 + 5), SkyBoxGradientColorVector[i].second.y);
                        storm::ParseNumber(pTokenVector->at(i * 8 + 6), SkyBoxGradientColorVector[i].second.z);
                        storm::ParseNumber(pTokenVector->at(i * 8 + 7), SkyBoxGradientColorVector[i].second.w);
                    }
                }

        textLoader.SetParentNode();
    }

    if (textLoader.SetChildNode("lensflare"))
    {
        textLoader.GetTokenBoolean("enable", &bLensFlareEnable);
        textLoader.GetTokenColor("brightnesscolor", &LensFlareBrightnessColor);
        textLoader.GetTokenFloat("maxbrightness", &fLensFlareMaxBrightness);
        textLoader.GetTokenBoolean("mainflareenable", &bMainFlareEnable);
        textLoader.GetTokenString("mainflaretexturefilename", &strMainFlareTextureFileName);
        textLoader.GetTokenFloat("mainflaresize", &fMainFlareSize);

        textLoader.SetParentNode();
    }

    return true;
}

bool EnvironmentData::Save(std::string_view filename) const
{
    return true;
}

bool EnvironmentData::GetSkyBoxGradientColor(uint32_t index,
                                             GradientColor &gradientColor) const
{
    if (index == SkyBoxGradientColorVector.size())
    {
        gradientColor = CloudGradientColor;
        return true;
    }
    else if (index >= SkyBoxGradientColorVector.size())
    {
        return false;
    }

    gradientColor = SkyBoxGradientColorVector[index];
    return true;
}

bool EnvironmentData::SetSkyBoxGradientColor(uint32_t index,
                                             const GradientColor &gradientColor)
{
    if (index == SkyBoxGradientColorVector.size())
    {
        CloudGradientColor = gradientColor;
        return true;
    }
    else if (index >= SkyBoxGradientColorVector.size())
    {
        return false;
    }

    SkyBoxGradientColorVector[index] = gradientColor;
    return true;
}

void EnvironmentData::InsertGradientUpper()
{
    GradientColor gradientColor;
    gradientColor.first = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);
    gradientColor.second = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);

    SkyBoxGradientColorVector.insert(SkyBoxGradientColorVector.begin() + bySkyBoxGradientLevelUpper,
                                     gradientColor);
    ++bySkyBoxGradientLevelUpper;
}

void EnvironmentData::InsertGradientLower()
{
    GradientColor gradientColor;
    gradientColor.first = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);
    gradientColor.second = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);

    SkyBoxGradientColorVector.insert(SkyBoxGradientColorVector.begin() +
                                     bySkyBoxGradientLevelUpper +
                                     bySkyBoxGradientLevelLower,
                                     gradientColor);
    ++bySkyBoxGradientLevelLower;
}

void EnvironmentData::DeleteGradient(uint32_t index)
{
    if (index >= SkyBoxGradientColorVector.size())
        return;

    SkyBoxGradientColorVector.erase(SkyBoxGradientColorVector.begin() + index);

    if (index < bySkyBoxGradientLevelUpper)
    {
        if (bySkyBoxGradientLevelUpper > 0)
            --bySkyBoxGradientLevelUpper;
    }
    else
    {
        if (bySkyBoxGradientLevelLower > 0)
            --bySkyBoxGradientLevelLower;
    }
}
