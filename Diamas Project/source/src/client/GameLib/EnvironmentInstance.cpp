#include "StdAfx.h"
#include "EnvironmentInstance.hpp"
#include "EnvironmentData.hpp"

#include "../EterLib/SkyBox.h"
#include "../EterLib/LensFlare.h"
#include "../EterLib/ScreenFilter.h"
#include "../EterLib/StateManager.h"

EnvironmentInstance::EnvironmentInstance()
    : m_data(nullptr)
{
    // ctor
}

void EnvironmentInstance::SetData(const EnvironmentData *data)
{
    m_data = data;
    // No need to do anything else if we're passed nullptr
    if (!data)
        return;

    m_fogNearDistance = data->m_fFogNearDistance;
    m_fogFarDistance = data->m_fFogFarDistance;
}

void EnvironmentInstance::SetFogDistance(float nearD, float farD) const
{
    m_fogNearDistance = nearD;
    m_fogFarDistance = farD;
}

void EnvironmentInstance::ApplyTo(CSkyBox &skyBox)
{
    skyBox.SetSkyBoxScale(m_data->v3SkyBoxScale);
    skyBox.SetGradientLevel(m_data->bySkyBoxGradientLevelUpper,
                            m_data->bySkyBoxGradientLevelLower);

    if (m_data->bSkyBoxTextureRenderMode)
        skyBox.SetRenderMode(CSkyObject::SKY_RENDER_MODE_TEXTURE);
    else
        skyBox.SetRenderMode(CSkyObject::SKY_RENDER_MODE_DIFFUSE);

    for (int i = 0; i < 6; ++i)
        skyBox.SetFaceTexture(m_data->strSkyBoxFaceFileName[i], i);

    skyBox.SetCloudTexture(m_data->strCloudTextureFileName);
    skyBox.SetCloudScale(m_data->v2CloudScale);
    skyBox.SetCloudHeight(m_data->fCloudHeight);
    skyBox.SetCloudTextureScale(m_data->v2CloudTextureScale);
    skyBox.SetCloudScrollSpeed(m_data->v2CloudSpeed);
    skyBox.Refresh();

    // Temporary
    skyBox.SetCloudColor(m_data->CloudGradientColor.first,
                         m_data->CloudGradientColor.second, 1);

    if (!m_data->SkyBoxGradientColorVector.empty())
        skyBox.SetSkyColor(m_data->SkyBoxGradientColorVector,
                           m_data->SkyBoxGradientColorVector,
                           1);
    // Temporary

    skyBox.StartTransition();
}

void EnvironmentInstance::ApplyTo(CLensFlare &lensFlare)
{
    lensFlare.CharacterizeFlare(m_data->bLensFlareEnable == 1,
                                m_data->bMainFlareEnable == 1,
                                m_data->fLensFlareMaxBrightness,
                                m_data->LensFlareBrightnessColor);

    lensFlare.Initialize("d:/ymir work/environment");

    if (!m_data->strMainFlareTextureFileName.empty())
        lensFlare.SetMainFlare(m_data->strMainFlareTextureFileName,
                               m_data->fMainFlareSize);
}

void EnvironmentInstance::ApplyTo(CScreenFilter &screenFilter)
{
    screenFilter.SetEnable(m_data->bFilteringEnable);
    screenFilter.SetBlendType(m_data->byFilteringAlphaSrc, m_data->byFilteringAlphaDest);
    screenFilter.SetColor(m_data->FilteringColor);
}

void EnvironmentInstance::BeginPass() const
{
    if (!m_data)
        return;

	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, true);
    if (m_fogEnable)
        STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, m_data->bFogEnable);
    else
        STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);

    STATEMANAGER.SetMaterial(&m_data->Material);

    // Directional Light
    if (m_data->bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND])
    {
        STATEMANAGER.SetLight(0, &m_data->DirLights[ENV_DIRLIGHT_BACKGROUND]);
        ms_lpd3dDevice->LightEnable(0, true);
    }
    else
    {
        ms_lpd3dDevice->LightEnable(0, false);
    }

    if (m_data->bFogEnable)
    {
        const uint32_t fogColor = m_data->FogColor.BGRA().c;
        STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, fogColor);

        if (m_data->bDensityFog)
        {
            const float density = 0.00015f;
            STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP);           // pixel fog
            STATEMANAGER.SetRenderState(D3DRS_FOGDENSITY, *((uint32_t *)&density)); // vertex fog
        }
        else
        {
            STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);                // vertex fox
            STATEMANAGER.SetRenderState(D3DRS_RANGEFOGENABLE, true);                        // vertex fox
            STATEMANAGER.SetRenderState(D3DRS_FOGSTART, *((uint32_t *)&m_fogNearDistance)); // USED BY D3DFOG_LINEAR
            STATEMANAGER.SetRenderState(D3DRS_FOGEND, *((uint32_t *)&m_fogFarDistance));    // USED BY D3DFOG_LINEAR
        }
    }
}

void EnvironmentInstance::EndPass() const
{
    if (!m_data)
        return;

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
    STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}
