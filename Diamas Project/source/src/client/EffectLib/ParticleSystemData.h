#ifndef METIN2_CLIENT_EFFECTLIB_PARTICLESYSTEMDATA_H
#define METIN2_CLIENT_EFFECTLIB_PARTICLESYSTEMDATA_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterLib/TextFileLoader.h"

#include "EffectElementBase.h"
#include "EmitterProperty.h"
#include "ParticleProperty.h"
//#include "ParticleInstance.h"

class CParticleInstance;

class CParticleSystemData : public CEffectElementBase
{
public:
    static CDynamicPool<CParticleSystemData> ms_kPool;

    CParticleSystemData();
    virtual ~CParticleSystemData();

    CEmitterProperty *GetEmitterPropertyPointer();
    CParticleProperty *GetParticlePropertyPointer();

    void ChangeTexture(const char *c_szFileName);

    // Invalidates cached properties - useful for editing
    void UpdateCache();

    static CParticleSystemData *New();
    static void Delete(CParticleSystemData *pkData);

    static void DestroySystem();

protected:
    bool OnLoadScript(CTextFileLoader &loader);

    void OnClear();
    bool OnIsData();

    CEmitterProperty m_EmitterProperty;
    CParticleProperty m_ParticleProperty;

private:
    bool LoadEmitterProperty(CTextFileLoader &loader);
    bool LoadParticleProperty(CTextFileLoader &loader);
};
#endif /* METIN2_CLIENT_EFFECTLIB_PARTICLESYSTEMDATA_H */
