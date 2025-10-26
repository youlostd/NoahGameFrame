#ifndef METIN2_CLIENT_ETERGRNLIB_MATERIAL_HPP
#define METIN2_CLIENT_ETERGRNLIB_MATERIAL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Util.h"

#include "../EterLib/GrpImageInstance.h"

struct SMaterialData
{
    CGraphicImage::Ptr pImage;
    float fSpecularPower;
    bool isSpecularEnable;
    uint8_t bSphereMapIndex;
};

class CGrannyMaterial
{
public:
    enum EType
    {
        TYPE_DIFFUSE_PNT,
        TYPE_BLEND_PNT,
        TYPE_MAX_NUM
    };

    CGrannyMaterial(granny_material *material);

    bool IsIn(const char *c_szImageName, int *iStage);

    void SetImagePointer(int iStage, CGraphicImage::Ptr pImage);
    void SetSpecularInfo(bool bFlag, float fPower, uint8_t uSphereMapIndex);

    void ApplyRenderState();
    void RestoreRenderState();

    CGrannyMaterial::EType GetType() const;
    CGraphicImage::Ptr GetImagePointer(int iStage) const;

    const CGraphicTexture *GetDiffuseTexture() const;
    const CGraphicTexture *GetOpacityTexture() const;

    LPDIRECT3DTEXTURE9 GetD3DTexture(int iStage) const;

    bool IsTwoSided() const
    {
        return m_bTwoSideRender;
    }

    bool operator==(granny_material *material) const;

    static void CreateSphereMap(UINT uMapIndex, const char *c_szSphereMapImageFileName);
    static void DestroySphereMap();

    static void TranslateSpecularMatrix(float fAddX, float fAddY, float fAddZ);

protected:
    CGraphicImage::Ptr __GetImagePointer(const char *c_szFileName);

    bool __IsSpecularEnable() const;
    float __GetSpecularPower() const;

    void __ApplyDiffuseRenderState();
    void __RestoreDiffuseRenderState();
    void __ApplySpecularRenderState();
    void __RestoreSpecularRenderState();

    granny_material *m_pgrnMaterial;
    CGraphicImage::Ptr m_roImage[3];
    EType m_eType;

    float m_fSpecularPower;
    bool m_bSpecularEnable;
    bool m_bTwoSideRender;
    uint32_t m_dwLastCullRenderStateForTwoSideRendering;
    uint8_t m_bSphereMapIndex;

    void (CGrannyMaterial::*m_pfnApplyRenderState)();
    void (CGrannyMaterial::*m_pfnRestoreRenderState)();

private:
    enum
    {
        SPHEREMAP_NUM = 10,
    };

    static Matrix ms_matSpecular;
    static Vector3 ms_v3SpecularTrans;

    static CGraphicImageInstance ms_akSphereMapInstance[SPHEREMAP_NUM];
};

class CGrannyMaterialPalette
{
public:
    // TODO(tim): Remove this
    void Clear()
    {
        m_materials.clear();
    }

    std::size_t RegisterMaterial(granny_material *material);

    void SetMaterialImage(const char *materialName, CGraphicImage::Ptr image);
    void SetMaterialData(const char *materialName, const SMaterialData &data);
    void SetSpecularInfo(const char *materialName, bool enable, float power);

    CGrannyMaterial &GetMaterialRef(std::size_t index);
    std::size_t GetMaterialCount() const;

protected:
    std::vector<CGrannyMaterial> m_materials;
};

#endif
