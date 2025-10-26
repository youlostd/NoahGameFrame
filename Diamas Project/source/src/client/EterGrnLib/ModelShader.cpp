#include "ModelShader.h"

#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"

#include <Utils.h>

CGrannyModelShader::CGrannyModelShader() : m_pShaderFXPool(NULL)
{
}

CGrannyModelShader::~CGrannyModelShader()
{
    Destroy();
}

bool CGrannyModelShader::Init()
{
    if (!m_pShaderFXPool)
        m_pShaderFXPool = new GrpShaderFXPool();
    _ASSERT(m_pShaderFXPool);

    CreateShader();
    GraphicVertexAttributeEx rigidAttributes[] = {
        ///////// mesh data /////////////////
        GraphicVertexAttributeEx(0, TYPE_VERTEX, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_NORMAL, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_TANGENT, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_BINORMAL, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_TEXCOORD, 0, FORMAT_FLOAT, 2),
    };

    GraphicVertexAttributeEx rigidTwoTexAttributes[] = {
        ///////// mesh data /////////////////
        GraphicVertexAttributeEx(0, TYPE_VERTEX, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_NORMAL, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_TANGENT, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_BINORMAL, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_TEXCOORD, 0, FORMAT_FLOAT, 2),
        GraphicVertexAttributeEx(0, TYPE_TEXCOORD, 1, FORMAT_FLOAT, 2),
    };
    GraphicVertexAttributeEx skinningAttributes[] = {
        ///////// mesh data /////////////////
        GraphicVertexAttributeEx(0, TYPE_VERTEX, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_BLENDWEIGHT, 0, FORMAT_UBYTE, 4),
        GraphicVertexAttributeEx(0, TYPE_BLENDINDICES, 0, FORMAT_UBYTEUN, 4),
        GraphicVertexAttributeEx(0, TYPE_NORMAL, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_TANGENT, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_BINORMAL, 0, FORMAT_FLOAT, 3),
        GraphicVertexAttributeEx(0, TYPE_TEXCOORD, 0, FORMAT_FLOAT, 2),

    };

    m_hRigidVertexTex2Format = Engine::GetDevice().CreateVertexFormat(rigidTwoTexAttributes, _countof(rigidTwoTexAttributes));
    m_hRigidVertexFormat = Engine::GetDevice().CreateVertexFormat(rigidAttributes, _countof(rigidAttributes));
    m_hSkinningVertexFormat = Engine::GetDevice().CreateVertexFormat(skinningAttributes, _countof(skinningAttributes));

    if (m_shader)
    {
        m_cMatObj2World = m_shader->GetConstant("ObjToWorld");
        m_cMatViewProj = m_shader->GetConstant("g_viewProj");
        m_cDirFromLight = m_shader->GetConstant("DirFromLight");
        m_cLightColor = m_shader->GetConstant("LightColour");
        m_cDiffuseTexture = m_shader->GetConstant("diffuse_texture");
        m_cOpacityTexture = m_shader->GetConstant("opacity_texture");
        m_cShadowTexture = m_shader->GetConstant("g_txShadow");
        m_cSkinningMatrices = m_shader->GetConstant("BoneMatrices");
        m_cDiffuseColor = m_shader->GetConstant("g_vDiffuseColor");
        m_cFogColor = m_shader->GetConstant("g_vFogColor");
        m_cFogParams = m_shader->GetConstant("g_vFogParams");
        m_cMaterialDiffuse = m_shader->GetConstant("g_vMaterialDiffuse");
        m_cMaterialAmbient = m_shader->GetConstant("g_vMaterialAmbient");
        m_cViewToLightProj = m_shader->GetConstant("g_mViewToLightProj");
        m_cAlphaBlendValue = m_shader->GetConstant("g_fAlphaValue");
        m_cShadowTexMatrix = m_shader->GetConstant("shadowTexMatrix");
        m_cCameraPos = m_shader->GetConstant("g_vCameraPos");
        m_cSpecularMat = m_shader->GetConstant("g_specularMat");
        m_cSpecularTexture = m_shader->GetConstant("g_specularTex");
        m_cSpecularEnable = m_shader->GetConstant("g_specularEnable");
        m_cSpecularPower = m_shader->GetConstant("g_specularPower");




    }

    return m_shader != nullptr;
}

void CGrannyModelShader::Destroy()
{

    SAFE_DELETE(m_pShaderFXPool);

    m_cMatObj2World.Destroy();
    m_cMatViewProj.Destroy();
    m_cMatView2Clip.Destroy();
    m_cDirFromLight.Destroy();
    m_cLightColor.Destroy();
    m_cDiffuseTexture.Destroy();
    m_cSkinningMatrices.Destroy();
}

void CGrannyModelShader::CreateShader()
{
    _ASSERT(m_pShaderFXPool);
    m_shader = Engine::GetDevice().CreateShaderFX("shaders/SimpleModelShader.fx", {}, m_pShaderFXPool);
}

GrpShaderFX *CGrannyModelShader::GetShader() const
{
    return m_shader;
}

void CGrannyModelShader::SetTransformConstant(const Matrix &matViewProj)
{
    m_cMatViewProj.SetMatrixTranspose(matViewProj);
}

void CGrannyModelShader::SetWorldMatrix(const Matrix &matWorld)
{
    m_cMatObj2World.SetMatrixTranspose(matWorld);
}

void CGrannyModelShader::SetViewToLightProjection(const Matrix &matViewToLight)
{
    m_cViewToLightProj.SetMatrix(matViewToLight);
}

void CGrannyModelShader::SetShadowTexMatrix(const Matrix &matShadowTex)
{
    m_cShadowTexMatrix.SetMatrix(matShadowTex);
}

void CGrannyModelShader::SetSpecularPower(float power)
{
    m_cSpecularPower.SetFloat(power);
}

void CGrannyModelShader::SetSpecularEnable(bool enable)
{
    m_cSpecularEnable.SetValue(TYPE_BOOL, &enable);
}

void CGrannyModelShader::SetSpecularMatrix(const Matrix & matSpecular)
{
    m_cSpecularMat.SetMatrix(matSpecular);
}

void CGrannyModelShader::SetLightDirection(const Vector3 &dir)
{
    m_cDirFromLight.SetVector3Array(&dir, 3);
}

void CGrannyModelShader::SetLightColor(const Vector4 &color)
{
    m_cLightColor.SetVector4(color);
}
void CGrannyModelShader::SetCameraPosition(const Vector3 &pos)
{
    m_cCameraPos.SetVector3(pos);
}
void CGrannyModelShader::SetFogParams(float fogStart, float fogEnd)
{
    m_cFogParams.SetVector3(fogStart, fogEnd, fogEnd - fogStart);
}

void CGrannyModelShader::SetFogColor(const DirectX::SimpleMath::Color &color)
{
    m_cFogColor.SetVector3(color.ToVector3());
}

void CGrannyModelShader::SetDiffuseAddColor(const DirectX::SimpleMath::Color &color)
{
    m_cDiffuseColor.SetVector4(color.ToVector4());
}

void CGrannyModelShader::SetMaterial(const D3DMATERIAL9 &mat)
{
    m_cMaterialAmbient.SetVector4(mat.Ambient.r, mat.Ambient.g, mat.Ambient.b, mat.Ambient.a);
    m_cMaterialDiffuse.SetVector4(mat.Diffuse.r, mat.Diffuse.g, mat.Diffuse.b, mat.Diffuse.a);
}

void CGrannyModelShader::SetShadowTexture(LPDIRECT3DTEXTURE9 tex)
{
    m_cShadowTexture.SetTexture(tex);
}

void CGrannyModelShader::SetShadowTexture(CGraphicTexture *tex)
{
    m_cShadowTexture.SetTexture(tex);
}

void CGrannyModelShader::SetSpecularTexture(LPDIRECT3DTEXTURE9 tex)
{
    m_cSpecularTexture.SetTexture(tex);
}

void CGrannyModelShader::SetSpecularTexture(CGraphicTexture *tex)
{
    m_cSpecularTexture.SetTexture(tex);
}

void CGrannyModelShader::SetDiffuseTexture(LPDIRECT3DTEXTURE9 tex)
{
    m_cDiffuseTexture.SetTexture(tex);
}

void CGrannyModelShader::SetDiffuseTexture(CGraphicTexture *tex)
{
    m_cDiffuseTexture.SetTexture(tex);
}

void CGrannyModelShader::SetOpacityTexture(LPDIRECT3DTEXTURE9 tex)
{
    m_cOpacityTexture.SetTexture(tex);
}

void CGrannyModelShader::SetOpacityTexture(CGraphicTexture *tex)
{
    m_cOpacityTexture.SetTexture(tex);
}

void CGrannyModelShader::SetSkinningMatrices(const Matrix *m, int count)
{
    m_cSkinningMatrices.SetMatrixArray(m, count);
}

void CGrannyModelShader::SetAlphaBlendValue(float val)
{
    m_cAlphaBlendValue.SetFloat(val);
}

void CGrannyModelShader::SetSkinningVertexFormat()
{
    Engine::GetDevice().SetVertexFormat(m_hSkinningVertexFormat);
}

void CGrannyModelShader::SetRigidVertexFormat()
{
    Engine::GetDevice().SetVertexFormat(m_hRigidVertexFormat);
}

void CGrannyModelShader::SetRigidWithTwoTextureVertexFormat()
{
    Engine::GetDevice().SetVertexFormat(m_hRigidVertexTex2Format);
}
