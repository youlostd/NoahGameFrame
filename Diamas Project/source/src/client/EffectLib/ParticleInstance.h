#ifndef METIN2_CLIENT_EFFECTLIB_PARTICLEINSTANCE_H
#define METIN2_CLIENT_EFFECTLIB_PARTICLEINSTANCE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterlib/GrpBase.h"
#include "../eterLib/Pool.h"
#include "EffectUpdateDecorator.h"
#include <optional>

class CParticleProperty;
class CEmitterProperty;

class CParticleInstance
{
    friend class CParticleSystemData;
    friend class CParticleSystemInstance;

public:
    CParticleInstance();
    ~CParticleInstance();

    [[nodiscard]] float GetRadiusApproximation() const;

    [[nodiscard]] DirectX::SimpleMath::Color GetColor() const
    {
        return Color(m_dcColor.color);
    }

    BOOL Update(double fElapsedTime, double fAngle);
    void Transform(const Matrix* c_matLocal);
    void Transform(const Matrix* c_matLocal, float c_fZRotation);

private:
    void UpdateRotation(double time, double elapsedTime);
    void UpdateTextureAnimation(double time, double elapsedTime);
    void UpdateScale(double time, double elapsedTime);
    void UpdateColor(double time, double elapsedTime);
    void UpdateGravity(double time, double elapsedTime);
    void UpdateAirResistance(double time, double elapsedTime);

public:
    static CParticleInstance *New();
    static void DestroySystem();
    static void Create();
    TPTVertex *GetParticleMeshPointer();
    void DeleteThis();
    void Destroy();

protected:
    void __Initialize();
private:
    Vector3 m_v3StartPosition;

    Vector3 m_v3Position;
    Vector3 m_v3LastPosition;
    Vector3 m_v3Velocity;

    Vector2 m_v2HalfSize;
    Vector2 m_v2Scale;

    float m_fRotation;
    float m_fRotationSpeed;
    DWORDCOLOR m_dcColor;

    uint8_t m_byTextureAnimationType;
    uint8_t m_byFrameIndex;
    uint8_t m_rotationType;
    uint8_t pad8;

    double m_frameTime;
    double m_fLifeTime;
    double m_fLastLifeTime;

    CParticleProperty *m_pParticleProperty;
    CEmitterProperty *m_pEmitterProperty;

    TPTVertex m_ParticleMesh[4];
public:
    static CDynamicPool<CParticleInstance> ms_kPool;
};
#endif /* METIN2_CLIENT_EFFECTLIB_PARTICLEINSTANCE_H */
