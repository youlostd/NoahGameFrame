#ifndef METIN2_CLIENT_ETERGRNLIB_MODELSHADER_HPP
#define METIN2_CLIENT_ETERGRNLIB_MODELSHADER_HPP

#include "../EterLib/GrpDevice.h"
#include "../EterLib/GrpShaderFX.h"

#include <Config.hpp>
#define WATER_USE_HEIGHT_TEXTURE
#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class GrpShaderFXPool;


class CEnvironmentShader
{
  public:
    CEnvironmentShader();
    virtual ~CEnvironmentShader();
    bool Init();
    void SetNormalTexture(CGraphicTexture *tex);
    void SetFoamTexture(CGraphicTexture *tex);
    void SetShoreTexture(CGraphicTexture *tex);
    void SetReflectionTexture(CGraphicTexture *tex);
    void SetReflectionTexture(LPDIRECT3DTEXTURE9 tex);
    void SetRefractionTexture(CGraphicTexture *tex);
    void SetRefractionTexture(LPDIRECT3DTEXTURE9 tex);
    void SetHeightTexture(CGraphicTexture *tex);
    void SetTime(float time);
    void SetMatrices(const Matrix &world, const Matrix &view, const Matrix &proj);

    void SetCameraPosition(const Vector4& pos);
    void SetLightPosition(const Vector4& pos);

    void SetLightColor(const DirectX::SimpleMath::Color& color);
    void SetLightAmbient(const DirectX::SimpleMath::Color& color);
    void SetSurfaceColor(const DirectX::SimpleMath::Color& color);
    void SetShoreColor(const DirectX::SimpleMath::Color& color);
    void SetDepthColor(const DirectX::SimpleMath::Color& color);

    GrpShaderFX *GetWaterShader() const;

    void SetWaterVertexFormat();

  private:
    void Destroy();
    void CreateShader();

  private:
    GrpShaderConstant m_cCameraDepthTexture;
    GrpShaderConstant m_cNormalTexture;
    GrpShaderConstant m_cFoamTexture;
    GrpShaderConstant m_cShoreTexture;
    GrpShaderConstant m_cReflectionTexture;
    GrpShaderConstant m_cReflectionTexture_TexelSize;
    GrpShaderConstant m_cRefractionTexture;
#ifdef WATER_USE_HEIGHT_TEXTURE
    GrpShaderConstant m_cHeightTexture;
#endif
#ifdef WATER_USE_MEAN_SKY_RADIANCE
    GrpShaderConstant m_cSkyTexture;
#endif
public:
    GrpShaderConstant m_cWorldSpaceCameraPos;
    GrpShaderConstant m_cWorldSpaceLightPos0;
    GrpShaderConstant m_cModelMatrix;
    GrpShaderConstant m_cModelMatrixInverse;
    GrpShaderConstant m_cViewProjectMatrix;
    GrpShaderConstant m_cViewProjectMatrixInverse;
    GrpShaderConstant m_cModelViewProjectMatrix;
private:


    GrpShaderConstant m_cTime;
    GrpShaderConstant m_cAmbientDensity;
    GrpShaderConstant m_cDiffuseDensity;
    GrpShaderConstant m_cHeightIntensity;
    GrpShaderConstant m_cNormalIntensity;
    GrpShaderConstant m_cTextureTiling;
    GrpShaderConstant m_cLightColor0;
    GrpShaderConstant m_cAmbientColor;
    GrpShaderConstant m_cSurfaceColor;
    GrpShaderConstant m_cShoreColor;
    GrpShaderConstant m_cDepthColor;
    GrpShaderConstant m_cWindDirection;
    GrpShaderConstant m_cWaveTiling;
    GrpShaderConstant m_cWaveSteepness;
    GrpShaderConstant m_cWaveAmplitudeFactor;
    GrpShaderConstant m_cWaveAmplitude;
    GrpShaderConstant m_cWavesIntensity;
    GrpShaderConstant m_cWavesNoise;
    GrpShaderConstant m_cWaterClarity;
    GrpShaderConstant m_cWaterTransparency;
    GrpShaderConstant m_cHorizontalExtinction;
    GrpShaderConstant m_cShininess;
    GrpShaderConstant m_cSpecularValues;
    GrpShaderConstant m_cRefractionValues;
    GrpShaderConstant m_cRefractionScale;
    GrpShaderConstant m_cRadianceFactor;
    GrpShaderConstant m_cDistortion;
    GrpShaderConstant m_cFoamRanges;
    GrpShaderConstant m_cFoamNoise;
    GrpShaderConstant m_cFoamTiling;
    GrpShaderConstant m_cFoamSpeed;
    GrpShaderConstant m_cFoamIntensity;
    GrpShaderConstant m_cShoreFade;

    GrpShaderFXPool *m_pShaderFXPool = nullptr;
    GrpShaderFX *m_waterShader = nullptr;

    VertexFormatHandle m_hWaterVertexFormat;
    uint8_t m_passIndex = 0;
};

#endif
