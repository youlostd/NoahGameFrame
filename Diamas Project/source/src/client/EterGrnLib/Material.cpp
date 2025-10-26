#include "Material.h"
#include "StdAfx.h"

#include "GrannyState.hpp"
#include "../eterlib/MatrixStack.h"

#include "../eterbase/Filename.h"
#include "../eterlib/GrpScreen.h"
#include "../eterlib/ResourceManager.h"
#include "../eterlib/StateManager.h"
#include "Mesh.h"
#include <utility>

CGraphicImageInstance CGrannyMaterial::ms_akSphereMapInstance[SPHEREMAP_NUM];

Vector3 CGrannyMaterial::ms_v3SpecularTrans(0.0f, 0.0f, 0.0f);
Matrix CGrannyMaterial::ms_matSpecular;

DirectX::SimpleMath::Color g_fSpecularColor = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.0f);

CGrannyMaterial::CGrannyMaterial(granny_material *material)
    : m_pgrnMaterial(material), m_fSpecularPower(0.0f), m_bSpecularEnable(false),
      m_dwLastCullRenderStateForTwoSideRendering(D3DCULL_CW), m_bSphereMapIndex(0),
      m_pfnApplyRenderState(&CGrannyMaterial::__ApplyDiffuseRenderState),
      m_pfnRestoreRenderState(&CGrannyMaterial::__RestoreDiffuseRenderState)
{
    granny_texture *diffuseTexture = NULL;
    granny_texture *opacityTexture = NULL;

    if (material)
    {
        if (material->MapCount > 1 && !strnicmp(material->Name, "Blend", 5))
        {
            diffuseTexture = GrannyGetMaterialTextureByType(material->Maps[0].Material, GrannyDiffuseColorTexture);
            opacityTexture = GrannyGetMaterialTextureByType(material->Maps[1].Material, GrannyDiffuseColorTexture);
        }
        else
        {
            diffuseTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyDiffuseColorTexture);
            opacityTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyOpacityTexture);
        }

        // Two-Side 렌더링이 필요한 지 검사
        {
            granny_int32 twoSided = 0;
            granny_data_type_definition TwoSidedFieldType[] = {
                {GrannyInt32Member, "Two-sided"},
                {GrannyEndMember},
            };

            granny_variant twoSideResult;
            const bool ok = GrannyFindMatchingMember(material->ExtendedData.Type, material->ExtendedData.Object,
                                                     "Two-sided", &twoSideResult);

            if (ok && twoSideResult.Type)
            {
                GrannyConvertSingleObject(twoSideResult.Type, twoSideResult.Object, TwoSidedFieldType, &twoSided, NULL);

                m_bTwoSideRender = 1 == twoSided;
            }
            else
            {
                m_bTwoSideRender = false;
            }
        }
    }

    if (diffuseTexture)
        m_roImage[0] = __GetImagePointer(diffuseTexture->FromFileName);

    if (opacityTexture)
        m_roImage[1] = __GetImagePointer(opacityTexture->FromFileName);

    // 오퍼시티가 있으면 블렌딩 메쉬
    if (m_roImage[1])
        m_eType = TYPE_BLEND_PNT;
    else
        m_eType = TYPE_DIFFUSE_PNT;
}

bool CGrannyMaterial::IsIn(const char *c_szImageName, int *piStage)
{
    std::string strImageName = c_szImageName;
    CFileNameHelper::StringPath(strImageName);

    granny_texture *pgrnDiffuseTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyDiffuseColorTexture);
    if (pgrnDiffuseTexture)
    {
        std::string strDiffuseFileName = pgrnDiffuseTexture->FromFileName;
        CFileNameHelper::StringPath(strDiffuseFileName);
        if (strDiffuseFileName == strImageName)
        {
            *piStage = 0;
            return true;
        }
    }

    granny_texture *pgrnOpacityTexture = GrannyGetMaterialTextureByType(m_pgrnMaterial, GrannyOpacityTexture);
    if (pgrnOpacityTexture)
    {
        std::string strOpacityFileName = pgrnOpacityTexture->FromFileName;
        CFileNameHelper::StringPath(strOpacityFileName);
        if (strOpacityFileName == strImageName)
        {
            *piStage = 1;
            return true;
        }
    }

    return false;
}

void CGrannyMaterial::SetImagePointer(int iStage, CGraphicImage::Ptr pImage)
{
    assert(iStage < 2 && "CGrannyMaterial::SetImagePointer");
    m_roImage[iStage] = std::move(pImage);
}

void CGrannyMaterial::SetSpecularInfo(bool bFlag, float fPower, uint8_t uSphereMapIndex)
{
    m_fSpecularPower = fPower;
    m_bSphereMapIndex = uSphereMapIndex;
    m_bSpecularEnable = bFlag;

    if (bFlag)
    {
        m_pfnApplyRenderState = &CGrannyMaterial::__ApplySpecularRenderState;
        m_pfnRestoreRenderState = &CGrannyMaterial::__RestoreSpecularRenderState;
    }
    else
    {
        m_pfnApplyRenderState = &CGrannyMaterial::__ApplyDiffuseRenderState;
        m_pfnRestoreRenderState = &CGrannyMaterial::__RestoreDiffuseRenderState;
    }
}

void CGrannyMaterial::ApplyRenderState()
{
    assert(m_pfnApplyRenderState != NULL && "CGrannyMaterial::SaveRenderState");
    (this->*m_pfnApplyRenderState)();
}

void CGrannyMaterial::RestoreRenderState()
{
    assert(m_pfnRestoreRenderState != NULL && "CGrannyMaterial::RestoreRenderState");
    (this->*m_pfnRestoreRenderState)();
}

CGrannyMaterial::EType CGrannyMaterial::GetType() const
{
    return m_eType;
}

CGraphicImage::Ptr CGrannyMaterial::GetImagePointer(int iStage) const
{
    return m_roImage[iStage];
}

const CGraphicTexture *CGrannyMaterial::GetDiffuseTexture() const
{
    if (!m_roImage[0])
        return NULL;

    return m_roImage[0]->GetTexturePointer();
}

const CGraphicTexture *CGrannyMaterial::GetOpacityTexture() const
{
    if (!m_roImage[1])
        return NULL;

    return m_roImage[1]->GetTexturePointer();
}

LPDIRECT3DTEXTURE9 CGrannyMaterial::GetD3DTexture(int iStage) const
{
    const auto &ratImage = m_roImage[iStage];
    if (!ratImage)
        return NULL;

    const CGraphicTexture *pTexture = ratImage->GetTexturePointer();
    return pTexture->GetD3DTexture();
}

bool CGrannyMaterial::operator==(granny_material *material) const
{
    return m_pgrnMaterial == material;
}

bool CGrannyMaterial::__IsSpecularEnable() const
{
    return m_bSpecularEnable;
}

float CGrannyMaterial::__GetSpecularPower() const
{
    return m_fSpecularPower;
}

extern const std::string &GetModelLocalPath();

CGraphicImage::Ptr CGrannyMaterial::__GetImagePointer(const char *fileName)
{
    assert(*fileName != '\0');

    CResourceManager &rkResMgr = CResourceManager::Instance();

    // SUPPORT_LOCAL_TEXTURE
    int fileName_len = strlen(fileName);
    if (fileName_len > 2 && fileName[1] != ':')
    {
        char localFileName[256];
        const std::string &modelLocalPath = GetModelLocalPath();

        int localFileName_len = modelLocalPath.length() + 1 + fileName_len;
        if (localFileName_len < sizeof(localFileName) - 1)
        {
            _snprintf(localFileName, sizeof(localFileName), "%s%s", GetModelLocalPath().c_str(), fileName);
            return rkResMgr.LoadResource<CGraphicImage>(localFileName);
        }
    }
    // END_OF_SUPPORT_LOCAL_TEXTURE

    return rkResMgr.LoadResource<CGraphicImage>(fileName);
}

void CGrannyMaterial::__ApplyDiffuseRenderState()
{
}

void CGrannyMaterial::__RestoreDiffuseRenderState()
{
}

void CGrannyMaterial::__ApplySpecularRenderState()
{

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetSpecularTexture(ms_akSphereMapInstance[m_bSphereMapIndex].GetTexturePointer());
        ms->SetSpecularMatrix(ms_matSpecular);
        ms->SetSpecularPower(__GetSpecularPower());
        ms->SetSpecularEnable(m_bSpecularEnable);
    }

}

void CGrannyMaterial::__RestoreSpecularRenderState()
{
        auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetSpecularTexture((LPDIRECT3DTEXTURE9)nullptr);
        ms->SetSpecularMatrix(Matrix::Identity);
        ms->SetSpecularPower(0.0f);
        ms->SetSpecularEnable(false);
    }
}

void CGrannyMaterial::CreateSphereMap(UINT uMapIndex, const char *c_szSphereMapImageFileName)
{
    CResourceManager &rkResMgr = CResourceManager::Instance();
    auto r = rkResMgr.LoadResource<CGraphicImage>(c_szSphereMapImageFileName);
    ms_akSphereMapInstance[uMapIndex].SetImagePointer(r);
}

void CGrannyMaterial::DestroySphereMap()
{
    for (UINT uMapIndex = 0; uMapIndex < SPHEREMAP_NUM; ++uMapIndex)
        ms_akSphereMapInstance[uMapIndex].Destroy();
}

void CGrannyMaterial::TranslateSpecularMatrix(float fAddX, float fAddY, float fAddZ)
{
    static float SPECULAR_TRANSLATE_MAX = 1000000.0f;

    ms_v3SpecularTrans.x += fAddX;
    ms_v3SpecularTrans.y += fAddY;
    ms_v3SpecularTrans.z += fAddZ;

    if (ms_v3SpecularTrans.x >= SPECULAR_TRANSLATE_MAX)
        ms_v3SpecularTrans.x = 0.0f;

    if (ms_v3SpecularTrans.y >= SPECULAR_TRANSLATE_MAX)
        ms_v3SpecularTrans.y = 0.0f;

    if (ms_v3SpecularTrans.z >= SPECULAR_TRANSLATE_MAX)
        ms_v3SpecularTrans.z = 0.0f;

    ms_matSpecular = Matrix::CreateTranslation(ms_v3SpecularTrans.x, ms_v3SpecularTrans.y, ms_v3SpecularTrans.z);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CGrannyMaterialPalette::SetMaterialImage(const char *materialName, CGraphicImage::Ptr image)
{
    for (auto &material : m_materials)
    {
        int stage;
        if (!material.IsIn(materialName, &stage))
            continue;

        material.SetImagePointer(stage, image);
        break;
    }
}

void CGrannyMaterialPalette::SetMaterialData(const char *materialName, const SMaterialData &data)
{
    if (materialName)
    {
        for (auto &material : m_materials)
        {
            int stage;
            if (!material.IsIn(materialName, &stage))
                continue;

            material.SetImagePointer(stage, data.pImage);
            material.SetSpecularInfo(data.isSpecularEnable, data.fSpecularPower, data.bSphereMapIndex);
            break;
        }
    }
    else
    {
        for (auto &material : m_materials)
        {
            material.SetSpecularInfo(data.isSpecularEnable, data.fSpecularPower, data.bSphereMapIndex);
            break;
        }
    }
}

void CGrannyMaterialPalette::SetSpecularInfo(const char *materialName, bool enable, float power)
{
    if (materialName)
    {
        for (auto &material : m_materials)
        {
            int stage;
            if (!material.IsIn(materialName, &stage))
                continue;

            material.SetSpecularInfo(enable, power, 0);
            break;
        }
    }
    else
    {
        for (auto &material : m_materials)
        {
            material.SetSpecularInfo(enable, power, 0);
            break;
        }
    }
}

std::size_t CGrannyMaterialPalette::RegisterMaterial(granny_material *material)
{
    std::size_t size = m_materials.size();
    for (std::size_t i = 0; i != size; ++i)
    {
        if (m_materials[i] == material)
            return i;
    }

    m_materials.emplace_back(material);
    return size;
}

CGrannyMaterial &CGrannyMaterialPalette::GetMaterialRef(std::size_t index)
{
    assert(index < m_materials.size());
    return m_materials[index];
}

std::size_t CGrannyMaterialPalette::GetMaterialCount() const
{
    return m_materials.size();
}
