#ifndef METIN2_CLIENT_EFFECTLIB_PARTICLESYSTEMINSTANCE_H
#define METIN2_CLIENT_EFFECTLIB_PARTICLESYSTEMINSTANCE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "EffectElementBaseInstance.h"
#include "ParticleInstance.h"
#include "ParticleProperty.h"

#include "../eterLib/GrpImageInstance.h"
#include "../eterlib/GrpScreen.h"
#include "../eterlib/StateManager.h"
#include "EmitterProperty.h"

// Next time when trying to replace this
// remember to check the InFrustum function
// also a particle system has between 1 and about 50 particles

class CParticleSystemInstance : public CEffectElementBaseInstance
{
  public:
    static void DestroySystem();

    static CParticleSystemInstance *New();
    static void Delete(CParticleSystemInstance *pkData);

    static CDynamicPool<CParticleSystemInstance> ms_kPool;

  public:
    template <typename T> BOOST_FORCEINLINE void ForEachParticleRendering(T &&FunObj);

    CParticleSystemInstance();
    virtual ~CParticleSystemInstance();

    void OnSetDataPointer(CEffectElementBase *pElement);
    void SetScale(float scale);

    void Emit(double elapsedTime);

    __forceinline bool InFrustum(CParticleInstance *pInstance) const;

    DWORD GetEmissionCount() const;

  protected:
    void OnInitialize();
    void OnDestroy();

    bool OnUpdate(double fElapsedTime);
    void OnRender(uint32_t addColor);

  protected:
    float m_fEmissionResidue{};
    float m_scale = 1.0f;
    DWORD m_dwCurrentEmissionCount{};
    int m_iLoopCount{};

    typedef std::list<CParticleInstance *> TParticleInstanceList;
    typedef std::vector<TParticleInstanceList> TParticleInstanceListVector;
    TParticleInstanceListVector m_ParticleInstanceListVector;
    std::unordered_map<LPDIRECT3DTEXTURE9, std::vector<CParticleInstance*>> m_SortedParticleInstanceMap;

    typedef std::vector<std::unique_ptr<CGraphicImageInstance>> TImageInstanceVector;
    TImageInstanceVector m_kVct_pkImgInst;

    CParticleSystemData *m_pData{};

    CParticleProperty *m_pParticleProperty{};
    CEmitterProperty *m_pEmitterProperty{};
};

template <typename T> void CParticleSystemInstance::ForEachParticleRendering(T &&FunObj)
{
			DWORD dwFrameIndex;
			for(dwFrameIndex=0; dwFrameIndex<m_kVct_pkImgInst.size(); dwFrameIndex++)
			{
				STATEMANAGER.SetTexture(0, m_kVct_pkImgInst[dwFrameIndex]->GetTextureReference().GetD3DTexture());
				TParticleInstanceList::iterator itor = m_ParticleInstanceListVector[dwFrameIndex].begin();
				for (; itor != m_ParticleInstanceListVector[dwFrameIndex].end(); ++itor)
				{
					if (!InFrustum(*itor) && !m_isAlwaysShow)
						return;
					std::forward<T>(FunObj)(*itor);
				}
			}
}
#endif /* METIN2_CLIENT_EFFECTLIB_PARTICLESYSTEMINSTANCE_H */
