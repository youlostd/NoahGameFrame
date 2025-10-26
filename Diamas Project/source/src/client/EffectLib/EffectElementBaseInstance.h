#ifndef METIN2_CLIENT_EFFECTLIB_EFFECTELEMENTBASEINSTANCE_H
#define METIN2_CLIENT_EFFECTLIB_EFFECTELEMENTBASEINSTANCE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "EffectElementBase.h"

class CEffectElementBaseInstance
{
public:
    CEffectElementBaseInstance();
    virtual ~CEffectElementBaseInstance();

    void SetDataPointer(CEffectElementBase *pElement);

    void Initialize();
    void Destroy();

    void SetLocalMatrixPointer(const Matrix *c_pMatrix);
    bool Update(double fElapsedTime);
    void Render(uint32_t addColor);

    virtual bool isActive();
    void SetActive();
    void SetDeactive();
    bool isHiddenByLod();
    void SetHiddenByLod();
    void SetShownByLod();

    void SetAlwaysRender(bool val)
    {
        m_isAlwaysShow = val;
    }

protected:
    virtual void OnSetDataPointer(CEffectElementBase *pElement) = 0;

    virtual void OnInitialize() = 0;
    virtual void OnDestroy() = 0;

    virtual bool OnUpdate(double fElapsedTime) = 0;
    virtual void OnRender(uint32_t addColor) = 0;

protected:
    const Matrix *m_localMat;
    double m_localTime;
    uint64_t m_startTime;
    double m_elapsedTime;
    double m_remainingTime;
    bool m_isStart;
    bool m_isAlwaysShow = false;
    bool m_isActive;
    bool m_isHiddenByLod = false;
    uint8_t pad8;

private:
    CEffectElementBase *m_pBase;
};
#endif /* METIN2_CLIENT_EFFECTLIB_EFFECTELEMENTBASEINSTANCE_H */
