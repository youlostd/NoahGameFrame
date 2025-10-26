#ifndef METIN2_CLIENT_GAMELIB_ENVIRONMENTINSTANCE_HPP
#define METIN2_CLIENT_GAMELIB_ENVIRONMENTINSTANCE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EterLib/GrpBase.h"

class CSkyBox;
class CLensFlare;
class CScreenFilter;

struct EnvironmentData;

class EnvironmentInstance : public CGraphicBase
{
  public:
    EnvironmentInstance();

    void SetData(const EnvironmentData *data);
    const EnvironmentData *GetData() const
    {
        return m_data;
    }

    // The client needs a way to override the EnvironmentData fog values...
    // TODO(tim): Cannot use |near|, |far| here since windows #defines them...
    void SetFogDistance(float nearD, float farD) const;
    void SetFogEnable(bool isEnable)
    {
        m_fogEnable = isEnable;
    }

    float GetFogNearDistance() const
    {
        return m_fogNearDistance;
    }
    float GetFogFarDistance() const
    {
        return m_fogFarDistance;
    }


    void ApplyTo(CSkyBox &skyBox);
    void ApplyTo(CLensFlare &lensFlare);
    void ApplyTo(CScreenFilter &screenFilter);

    void BeginPass() const;
    void EndPass() const;

    operator bool() const
    {
        return !!m_data;
    }

  private:
    const EnvironmentData *m_data;
    bool m_fogEnable = true;

    mutable float m_fogNearDistance;
    mutable float m_fogFarDistance;
};

#endif
