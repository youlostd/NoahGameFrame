#ifndef METIN2_CLIENT_EFFECTLIB_EFFECTINSTANCE_H
#define METIN2_CLIENT_EFFECTLIB_EFFECTINSTANCE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterlib/GrpObjectInstance.h"
#include "../eterlib/Pool.h"
#include "../mileslib/Type.h"

#include "EffectElementBaseInstance.h"
#include "EffectData.h"
#include "EffectMeshInstance.h"
#include "ParticleSystemInstance.h"
#include "SimpleLightInstance.h"
#include <game/EffectConstants.hpp>


class CEffectInstance : public CGraphicObjectInstance
{
public:
    typedef std::vector<CEffectElementBaseInstance *> TEffectElementInstanceVector;

    enum
    {
        ID = EFFECT_OBJECT
    };

    int GetType() const
    {
        return CEffectInstance::ID;
    }

    bool GetBoundingSphere(Vector3 &v3Center, float &fRadius);
    void SetBaseEffectId(uint32_t id);
    void SetEffectKind(uint32_t kind);
    uint32_t GetEffectKind();

    static void DestroySystem();

    static CEffectInstance *New();
    static void Delete(CEffectInstance *pkEftInst);

    static void ResetRenderingEffectCount();
    static int GetRenderingEffectCount();

public:
    CEffectInstance();
    virtual ~CEffectInstance();

    bool LessRenderOrder(CEffectInstance *pkEftInst);

    void SetEffectDataPointer(CEffectData *pEffectData);

    void SetScale(float scale);

    void Clear();
    bool isAlive();
    void SetAlwaysRender(bool val) override;
    void SetLodShow();
    bool isHiddenByLod();
    void SetHiddenByLod();
    void SetActive();
    bool isActive();
    void SetDeactive();
    void SetGlobalMatrix(const Matrix &c_rmatGlobal);

    void SetAddColor(uint32_t color)
    {
        m_addColor = color;
    }

    uint32_t GetAddColor() const
    {
        return m_addColor;
    }

    void OnUpdate();
    void OnRender();
    void UpdateLODLevel();

    void OnBlendRender()
    {
    } // Not used
    void OnRenderToShadowMap(bool renderPart)
    {
    } // Not used
    void OnRenderShadow()
    {
    } // Not used
    void OnRenderPCBlocker()
    {
    } // Not used

protected:
    void __Initialize();

    void __SetParticleData(CParticleSystemData *pData);
    void __SetMeshData(CEffectMeshScript *pMesh);
    void __SetLightData(CLightData *pData);

    virtual void OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector)
    {
    } // Not used
    virtual void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance)
    {
    }

    virtual bool OnGetObjectHeight(float fX, float fY, float *pfHeight)
    {
        return false;
    }

protected:
    bool m_isAlive{};
    uint32_t m_dwFrame{};
    Matrix m_matGlobal;

    CEffectData *m_pkEftData{};

    std::vector<CParticleSystemInstance *> m_ParticleInstanceVector;
    std::vector<CEffectMeshInstance *> m_MeshInstanceVector;
    std::vector<CLightInstance *> m_LightInstanceVector;

    TSoundInstanceVector *m_pSoundInstanceVector{};

    float m_fBoundingSphereRadius{};
    Vector3 m_v3BoundingSpherePosition;

    float m_fLastTime{};
    uint32_t m_addColor = 0;

    float m_scale = 1.0f;
    uint32_t m_baseEffectId = 0;
    uint32_t m_effectKind = EFFECT_KIND_NONE;

public:
    static CDynamicPool<CEffectInstance> ms_kPool;
    static int ms_iRenderingEffectCount;
};
#endif /* METIN2_CLIENT_EFFECTLIB_EFFECTINSTANCE_H */
