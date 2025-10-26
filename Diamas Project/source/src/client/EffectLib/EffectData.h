#ifndef METIN2_CLIENT_EFFECTLIB_EFFECTDATA_H
#define METIN2_CLIENT_EFFECTLIB_EFFECTDATA_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../milesLib/Type.h"

#include "ParticleSystemData.h"
#include "EffectMesh.h"
#include "SimpleLightData.h"

class CEffectData
{
public:
    typedef std::vector<CParticleSystemData *> TParticleVector;
    typedef std::vector<CEffectMeshScript *> TMeshVector;
    typedef std::vector<CLightData *> TLightVector;

public:
    CEffectData();
    virtual ~CEffectData();

    void Clear();
    bool LoadScript(const char *c_szFileName);
    bool LoadSoundScriptData(const char *c_szFileName);

    uint32_t GetParticleCount();
    CParticleSystemData *GetParticlePointer(uint32_t dwPosition);

    uint32_t GetMeshCount();
    CEffectMeshScript *GetMeshPointer(uint32_t dwPosition);

    uint32_t GetLightCount();
    CLightData *GetLightPointer(uint32_t dwPosition);

    TSoundInstanceVector *GetSoundInstanceVector();

    float GetBoundingSphereRadius();
    Vector3 GetBoundingSpherePosition();

    const char *GetFileName() const;

protected:
    void __ClearParticleDataVector();
    void __ClearLightDataVector();
    void __ClearMeshDataVector();

    // FIXME : �� �κ��� �״��� ���� ���� �ʴ´�. ���� ���̵� ã�Ƴ��� ��ġ��.
    //         ������ (Ưȭ��) ������ �������̽� ������ ����� �ٲ��� �Ѵٴ� ���� ���� ���ϴ�. - [levites]
    virtual CParticleSystemData *AllocParticle();
    virtual CEffectMeshScript *AllocMesh();
    virtual CLightData *AllocLight();

protected:
    TParticleVector m_ParticleVector;
    TMeshVector m_MeshVector;
    TLightVector m_LightVector;
    TSoundInstanceVector m_SoundInstanceVector;

    float m_fBoundingSphereRadius;
    Vector3 m_v3BoundingSpherePosition;

    std::string m_strFileName;

public:
    static void DestroySystem();

    static CEffectData *New();
    static void Delete(CEffectData *pkData);

    static CDynamicPool<CEffectData> ms_kPool;
};
#endif /* METIN2_CLIENT_EFFECTLIB_EFFECTDATA_H */
