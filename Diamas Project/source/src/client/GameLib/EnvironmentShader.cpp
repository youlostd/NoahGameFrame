#include "EnvironmentShader.h"

#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"

#include <Utils.h>

CEnvironmentShader::CEnvironmentShader() : m_pShaderFXPool(NULL), m_hWaterVertexFormat{0}
{
    Init();
}

CEnvironmentShader::~CEnvironmentShader()
{
    Destroy();
}

bool CEnvironmentShader::Init()
{
    if (!m_pShaderFXPool)
        m_pShaderFXPool = new GrpShaderFXPool();
    _ASSERT(m_pShaderFXPool);

    CreateShader();

    GraphicVertexAttributeEx waterVertex[] = {
        ///////// mesh data /////////////////
        GraphicVertexAttributeEx(0, TYPE_VERTEX, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_TEXCOORD, 0, FORMAT_FLOAT, 1),

    };

    m_hWaterVertexFormat = Engine::GetDevice().CreateVertexFormat(waterVertex, _countof(waterVertex));

    if (m_waterShader)
    {
        m_cCameraDepthTexture = m_waterShader->GetConstant("_CameraDepthTexture");
        m_cNormalTexture = m_waterShader->GetConstant("_NormalTexture");
        m_cFoamTexture = m_waterShader->GetConstant("_FoamTexture");
        m_cShoreTexture = m_waterShader->GetConstant("_ShoreTexture");
        m_cReflectionTexture = m_waterShader->GetConstant("_ReflectionTexture");
        m_cReflectionTexture_TexelSize = m_waterShader->GetConstant("_ReflectionTexture_TexelSize");
        m_cRefractionTexture = m_waterShader->GetConstant("_RefractionTexture");
#ifdef WATER_USE_HEIGHT_TEXTURE
        m_cHeightTexture = m_waterShader->GetConstant("_HeightTexture");
#endif
#ifdef WATER_USE_MEAN_SKY_RADIANCE
        m_cSkyTexture = m_waterShader->GetConstant("_SkyTexture");
#endif

        m_cWorldSpaceCameraPos = m_waterShader->GetConstant("_WorldSpaceCameraPos");
        m_cWorldSpaceLightPos0 = m_waterShader->GetConstant("_WorldSpaceLightPos0");
        m_cModelMatrix = m_waterShader->GetConstant("_ModelMatrix");
        m_cModelMatrixInverse = m_waterShader->GetConstant("_ModelMatrixInverse");
        m_cViewProjectMatrix = m_waterShader->GetConstant("_ViewProjectMatrix");
        m_cViewProjectMatrixInverse = m_waterShader->GetConstant("_ViewProjectMatrixInvers");
        m_cModelViewProjectMatrix = m_waterShader->GetConstant("_ModelViewProjectMatrix");
        m_cTime = m_waterShader->GetConstant("_Time");
        m_cAmbientDensity = m_waterShader->GetConstant("_AmbientDensity");
        m_cDiffuseDensity = m_waterShader->GetConstant("_DiffuseDensity");
        m_cHeightIntensity = m_waterShader->GetConstant("_HeightIntensity");
        m_cNormalIntensity = m_waterShader->GetConstant("_NormalIntensity");
        m_cTextureTiling = m_waterShader->GetConstant("_TextureTiling");
        m_cLightColor0 = m_waterShader->GetConstant("_LightColor0");
        m_cAmbientColor = m_waterShader->GetConstant("_AmbientColor");
        m_cSurfaceColor = m_waterShader->GetConstant("_SurfaceColor");
        m_cShoreColor = m_waterShader->GetConstant("_ShoreColor");
        m_cDepthColor = m_waterShader->GetConstant("_DepthColor");
        m_cWindDirection = m_waterShader->GetConstant("_WindDirection");
        m_cWaveTiling = m_waterShader->GetConstant("_WaveTiling");
        m_cWaveSteepness = m_waterShader->GetConstant("_WaveSteepness");
        m_cWaveAmplitudeFactor = m_waterShader->GetConstant("_WaveAmplitudeFactor");
        m_cWaveAmplitude = m_waterShader->GetConstant("_WaveAmplitude");
        m_cWavesIntensity = m_waterShader->GetConstant("_WavesIntensity");
        m_cWavesNoise = m_waterShader->GetConstant("_WavesNoise");
        m_cWaterClarity = m_waterShader->GetConstant("_WaterClarity");
        m_cWaterTransparency = m_waterShader->GetConstant("_WaterTransparency");
        m_cHorizontalExtinction = m_waterShader->GetConstant("_HorizontalExtinction");
        m_cShininess = m_waterShader->GetConstant("_Shininess");
        m_cSpecularValues = m_waterShader->GetConstant("_SpecularValues");
        m_cRefractionValues = m_waterShader->GetConstant("_RefractionValues");
        m_cRefractionScale = m_waterShader->GetConstant("_RefractionScale");
        m_cRadianceFactor = m_waterShader->GetConstant("_RadianceFactor");
        m_cDistortion = m_waterShader->GetConstant("_Distortion");
        m_cFoamRanges = m_waterShader->GetConstant("_FoamRanges");
        m_cFoamNoise = m_waterShader->GetConstant("_FoamNoise");
        m_cFoamTiling = m_waterShader->GetConstant("_FoamTiling");
        m_cFoamSpeed = m_waterShader->GetConstant("_FoamSpeed");
        m_cFoamIntensity = m_waterShader->GetConstant("_FoamIntensity");
        m_cShoreFade = m_waterShader->GetConstant("_ShoreFade");
    }

    return m_waterShader != nullptr;
}

void CEnvironmentShader::SetNormalTexture(CGraphicTexture *tex)
{
    m_cNormalTexture.SetTexture(tex);
}

void CEnvironmentShader::SetFoamTexture(CGraphicTexture *tex)
{
    m_cFoamTexture.SetTexture(tex);
}

void CEnvironmentShader::SetShoreTexture(CGraphicTexture *tex)
{
    m_cShoreTexture.SetTexture(tex);
}

void CEnvironmentShader::SetReflectionTexture(CGraphicTexture *tex)
{
    m_cReflectionTexture.SetTexture(tex);
}

void CEnvironmentShader::SetReflectionTexture(LPDIRECT3DTEXTURE9 tex)
{
    m_cReflectionTexture.SetTexture(tex);
}

void CEnvironmentShader::SetRefractionTexture(CGraphicTexture *tex)
{
    m_cRefractionTexture.SetTexture(tex);
}
void CEnvironmentShader::SetRefractionTexture(LPDIRECT3DTEXTURE9 tex)
{
    m_cRefractionTexture.SetTexture(tex);
}
#ifdef WATER_USE_HEIGHT_TEXTURE

void CEnvironmentShader::SetHeightTexture(CGraphicTexture *tex)
{
    m_cHeightTexture.SetTexture(tex);
}

#endif

void CEnvironmentShader::SetTime(float time)
{
    m_cTime.SetFloat(time);
}

void CEnvironmentShader::SetMatrices(const Matrix &world, const Matrix &view, const Matrix &proj)
{
    float fDeterminantD3DMatView = view.Determinant();

    m_cModelMatrix.SetMatrixTranspose(world);
    Matrix worldInv = world.Invert();
    m_cModelMatrixInverse.SetMatrixTranspose(worldInv);
  //  Matrix viewProjInv;
  //  DirectX::SimpleMath::MatrixInverse(&viewProjInv, &fDeterminantD3DMatView, &viewProjInv);

    m_cViewProjectMatrix.SetMatrixTranspose(view * proj);
    //m_cViewProjectMatrixInverse.SetMatrixTranspose(viewProjInv);

    Matrix worldViewProj = world * view * proj;
    m_cModelViewProjectMatrix.SetMatrixTranspose(worldViewProj);
}

void CEnvironmentShader::SetCameraPosition(const Vector4 &pos)
{
    m_cWorldSpaceCameraPos.SetVector4(pos);
}

void CEnvironmentShader::SetLightPosition(const Vector4 &pos)
{
    m_cWorldSpaceLightPos0.SetVector4(pos);
}

void CEnvironmentShader::SetLightColor(const DirectX::SimpleMath::Color &color)
{
  //  m_cLightColor0.SetVector4(color.r, color.g, color.b, color.a);
}

void CEnvironmentShader::SetLightAmbient(const DirectX::SimpleMath::Color &color)
{
 //     m_cAmbientColor.SetVector4(color.r, color.g, color.b, color.a);
}

void CEnvironmentShader::SetSurfaceColor(const DirectX::SimpleMath::Color &color)
{
   //   m_cSurfaceColor.SetVector3(color.r, color.g, color.b);
}

void CEnvironmentShader::SetShoreColor(const DirectX::SimpleMath::Color &color)
{
  //    m_cShoreColor.SetVector3(color.r, color.g, color.b);
}

void CEnvironmentShader::SetDepthColor(const DirectX::SimpleMath::Color &color)
{
    //  m_cDepthColor.SetVector3(color.r, color.g, color.b);
}

void CEnvironmentShader::Destroy()
{

    SAFE_DELETE(m_pShaderFXPool);

    m_cCameraDepthTexture.Destroy();
    m_cNormalTexture.Destroy();
    m_cFoamTexture.Destroy();
    m_cShoreTexture.Destroy();
    m_cReflectionTexture.Destroy();
    m_cReflectionTexture_TexelSize.Destroy();
    m_cRefractionTexture.Destroy();
#ifdef WATER_USE_HEIGHT_TEXTURE
    m_cHeightTexture.Destroy();
#endif
#ifdef WATER_USE_MEAN_SKY_RADIANCE
    m_cSkyTexture.Destroy();
#endif
    m_cWorldSpaceCameraPos.Destroy();
    m_cWorldSpaceLightPos0.Destroy();
    m_cModelMatrix.Destroy();
    m_cModelMatrixInverse.Destroy();
    m_cViewProjectMatrix.Destroy();
    m_cViewProjectMatrixInverse.Destroy();
    m_cModelViewProjectMatrix.Destroy();
    m_cTime.Destroy();
    m_cAmbientDensity.Destroy();
    m_cDiffuseDensity.Destroy();
    m_cHeightIntensity.Destroy();
    m_cNormalIntensity.Destroy();
    m_cTextureTiling.Destroy();
    m_cLightColor0.Destroy();
    m_cAmbientColor.Destroy();
    m_cSurfaceColor.Destroy();
    m_cShoreColor.Destroy();
    m_cDepthColor.Destroy();
    m_cWindDirection.Destroy();
    m_cWaveTiling.Destroy();
    m_cWaveSteepness.Destroy();
    m_cWaveAmplitudeFactor.Destroy();
    m_cWaveAmplitude.Destroy();
    m_cWavesIntensity.Destroy();
    m_cWavesNoise.Destroy();
    m_cWaterClarity.Destroy();
    m_cWaterTransparency.Destroy();
    m_cHorizontalExtinction.Destroy();
    m_cShininess.Destroy();
    m_cSpecularValues.Destroy();
    m_cRefractionValues.Destroy();
    m_cRefractionScale.Destroy();
    m_cRadianceFactor.Destroy();
    m_cDistortion.Destroy();
    m_cFoamRanges.Destroy();
    m_cFoamNoise.Destroy();
    m_cFoamTiling.Destroy();
    m_cFoamSpeed.Destroy();
    m_cFoamIntensity.Destroy();
    m_cShoreFade.Destroy();
}

void CEnvironmentShader::CreateShader()
{
    _ASSERT(m_pShaderFXPool);
    std::vector<std::string> waterShaderDefines;

#ifdef WATER_USE_HEIGHT_TEXTURE
    waterShaderDefines.push_back("USE_DISPLACEMENT");
    waterShaderDefines.push_back("1");
#endif
    waterShaderDefines.push_back("USE_FOAM");
    waterShaderDefines.push_back("1");
    waterShaderDefines.push_back("REVERSED_Z");
    waterShaderDefines.push_back("1");

    m_waterShader = Engine::GetDevice().CreateShaderFX("shaders/water.fx", waterShaderDefines, m_pShaderFXPool);
}

GrpShaderFX *CEnvironmentShader::GetWaterShader() const
{
    return m_waterShader;
}

void CEnvironmentShader::SetWaterVertexFormat()
{
    Engine::GetDevice().SetVertexFormat(m_hWaterVertexFormat);
}
