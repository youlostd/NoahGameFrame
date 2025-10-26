#ifndef METIN2_CLIENT_EFFECTLIB_SIMPLELIGHTDATA_H
#define METIN2_CLIENT_EFFECTLIB_SIMPLELIGHTDATA_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterLib/TextFileLoader.h"

#include "Type.h"
#include "EffectElementBase.h"

class CLightData : public CEffectElementBase
{
    friend class CLightInstance;
public:
    CLightData();
    virtual ~CLightData();

    void GetRange(float fTime, float &rRange);
    float GetDuration();

    bool isLoop()
    {
        return m_bLoopFlag;
    }

    int GetLoopCount()
    {
        return m_iLoopCount;
    }

    void InitializeLight(D3DLIGHT9 &light);

protected:
    void OnClear();
    bool OnIsData();

    bool OnLoadScript(CTextFileLoader &rTextFileLoader);

protected:
    float m_fMaxRange;
    float m_fDuration;
    TTimeEventTableFloat m_TimeEventTableRange;

    DirectX::SimpleMath::Color m_cAmbient;
    DirectX::SimpleMath::Color m_cDiffuse;

    bool m_bLoopFlag;
    int m_iLoopCount;

    float m_fAttenuation0;
    float m_fAttenuation1;
    float m_fAttenuation2;

public:
    static void DestroySystem();

    static CLightData *New();
    static void Delete(CLightData *pkData);

    static CDynamicPool<CLightData> ms_kPool;
};
#endif /* METIN2_CLIENT_EFFECTLIB_SIMPLELIGHTDATA_H */
