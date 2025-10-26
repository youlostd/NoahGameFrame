#ifndef METIN2_CLIENT_EFFECTLIB_EFFECTELEMENTBASE_H
#define METIN2_CLIENT_EFFECTLIB_EFFECTELEMENTBASE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "Type.h"

class CEffectElementBase
{
public:
    CEffectElementBase();
    virtual ~CEffectElementBase();

    void Clear();
    bool isData();

    bool LoadScript(CTextFileLoader &rTextFileLoader);

    void GetPosition(double fTime, Vector3 &rPosition);
    float GetStartTime();
    /*
            bool isVisible(float fTime);
            void GetAlpha(float fTime, float * pAlpha);
            void GetScale(float fTime, float * pScale);
    */

protected:
    virtual void OnClear() = 0;
    virtual bool OnIsData() = 0;
    virtual bool OnLoadScript(CTextFileLoader &rTextFileLoader) = 0;

protected:
    float m_fStartTime;
    TTimeEventTablePosition m_TimeEventTablePosition;
    /*
            TTimeEventTable				m_TimeEventTableVisible;
            TTimeEventTableFloat		m_TimeEventAlpha;
            TTimeEventTableFloat		m_TimeEventScale;
    */
};
#endif /* METIN2_CLIENT_EFFECTLIB_EFFECTELEMENTBASE_H */
