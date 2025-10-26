#ifndef METIN2_CLIENT_EFFECTLIB_SIMPLELIGHTINSTANCE_H
#define METIN2_CLIENT_EFFECTLIB_SIMPLELIGHTINSTANCE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterLib/GrpScreen.h"

#include "EffectElementBaseInstance.h"
#include "SimpleLightData.h"

class CLightInstance : public CEffectElementBaseInstance
{
public:
    friend class CLightData;

    CLightInstance();
    virtual ~CLightInstance();

protected:

    void OnSetDataPointer(CEffectElementBase *pElement);

    void OnInitialize();
    void OnDestroy();

    bool OnUpdate(double fElapsedTime);
    void OnRender(uint32_t addColor);

    DWORD m_LightID;
    CLightData *m_pData;
    uint32_t m_dwRangeIndex;

    int32_t m_iLoopCount;

public:
    static void DestroySystem();

    static CLightInstance *New();
    static void Delete(CLightInstance *pkData);

    static CDynamicPool<CLightInstance> ms_kPool;
};
#endif /* METIN2_CLIENT_EFFECTLIB_SIMPLELIGHTINSTANCE_H */
