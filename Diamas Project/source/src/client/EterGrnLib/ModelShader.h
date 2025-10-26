
#pragma once

#include "../EterLib/GrpDevice.h"
#include "../EterLib/GrpShaderFX.h"

class GrpShaderFXPool;

enum
{
    MODEL_SHADER_PASS_DIFFUSE,
    MODEL_SHADER_PASS_SHADOW,
    MODEL_SHADER_PASS_BLEND,
    MODEL_SHADER_PASS_OPACITY,
    MODEL_SHADER_PASS_DIFFUSE_OBJECT,

};


class CGrannyModelShader
{
  public:
    CGrannyModelShader();
    virtual ~CGrannyModelShader();
    bool Init();
    GrpShaderFX *GetShader() const;
    void SetTransformConstant(const Matrix &matViewProj);
    void SetWorldMatrix(const Matrix &matWorld);
    void SetViewToLightProjection(const Matrix &matViewToLight);
    void SetShadowTexMatrix(const Matrix &matShadowTex);
    void SetSpecularPower(float power);
    void SetSpecularEnable(bool enable);
    void SetSpecularMatrix(const Matrix &matSpecular);
    void SetLightDirection(const Vector3 &dir);
    void SetLightColor(const Vector4 &color);
    void SetCameraPosition(const Vector3 &pos);

    void SetFogParams(float fogStart, float fogEnd);
    void SetFogColor(const DirectX::SimpleMath::Color &color);
    void SetDiffuseAddColor(const DirectX::SimpleMath::Color &color);
    void SetMaterial(const D3DMATERIAL9 &mat);
    void SetShadowTexture(LPDIRECT3DTEXTURE9 tex);
    void SetDiffuseTexture(LPDIRECT3DTEXTURE9 tex);
    void SetDiffuseTexture(CGraphicTexture *tex);
    void SetOpacityTexture(LPDIRECT3DTEXTURE9 tex);
    void SetOpacityTexture(CGraphicTexture *tex);
    void SetShadowTexture(CGraphicTexture* tex);
    void SetSpecularTexture(LPDIRECT3DTEXTURE9 tex);
    void SetSpecularTexture(CGraphicTexture *tex);

    void SetSkinningMatrices(const Matrix *m, int count);
    void SetAlphaBlendValue(float val);
    void SetSkinningVertexFormat();
    void SetRigidVertexFormat();
    void SetRigidWithTwoTextureVertexFormat();

    uint8_t GetPassIndex() const { return m_passIndex; }
    void SetPassIndex(uint8_t passIndex) { m_passIndex = passIndex; }
private:
    void Destroy();
    void CreateShader();

  private:
    GrpShaderConstant m_cMatObj2World;
    GrpShaderConstant m_cMatViewProj;
    GrpShaderConstant m_cMatView2Clip;
    GrpShaderConstant m_cDirFromLight;
    GrpShaderConstant m_cLightColor;
    GrpShaderConstant m_cDiffuseColor;
    GrpShaderConstant m_cFogColor;
    GrpShaderConstant m_cFogParams;
    GrpShaderConstant m_cMaterialDiffuse;
    GrpShaderConstant m_cMaterialAmbient;
    GrpShaderConstant m_cViewToLightProj;
    GrpShaderConstant m_cAlphaBlendValue;
    GrpShaderConstant m_cShadowTexMatrix;
    GrpShaderConstant m_cSkinningMatrices;
    GrpShaderConstant m_cDiffuseTexture;
    GrpShaderConstant m_cOpacityTexture;
    GrpShaderConstant m_cShadowTexture;
    GrpShaderConstant m_cSpecularTexture;
    GrpShaderConstant m_cSpecularMat;
    GrpShaderConstant m_cSpecularPower;
    GrpShaderConstant m_cSpecularEnable;

    GrpShaderConstant m_cCameraPos;

    GrpShaderFXPool *m_pShaderFXPool = nullptr;
    GrpShaderFX *m_shader = nullptr;

    VertexFormatHandle m_hRigidVertexFormat;
    VertexFormatHandle m_hRigidVertexTex2Format;

    VertexFormatHandle m_hSkinningVertexFormat;


    uint8_t m_passIndex = 0;
};

