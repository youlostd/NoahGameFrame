#ifndef METIN2_CLIENT_GAMELIB_SNOWPARTICLE_H
#define METIN2_CLIENT_GAMELIB_SNOWPARTICLE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

struct SParticleVertex
{
    Vector3 v3Pos;
    float u, v;
};

struct BlurVertex
{
    Vector3 pos;
    FLOAT rhw;
    uint32_t color;
    FLOAT tu, tv;

    static const uint32_t FVF;

    BlurVertex(Vector3 p, float w, uint32_t c, float u, float v) : pos(p), rhw(w), color(c), tu(u), tv(v)
    {
    }
    ~BlurVertex(){};
};

class CSnowParticle
{
  public:
    CSnowParticle();
    ~CSnowParticle();

    static CSnowParticle *New();
    static void Delete(CSnowParticle *pSnowParticle);
    static void DestroyPool();

    void Init(const Vector3 &c_rv3Pos);

    void SetCameraVertex(const Vector3 &rv3Up, const Vector3 &rv3Cross);
    bool IsActivate();

    void Update(float fElapsedTime, const Vector3 &c_rv3Pos);
    void GetVerticies(SParticleVertex &rv3Vertex1, SParticleVertex &rv3Vertex2, SParticleVertex &rv3Vertex3,
                      SParticleVertex &rv3Vertex4);

  protected:
    bool m_bActivate;
    bool m_bChangedSize;
    float m_fHalfWidth;
    float m_fHalfHeight;

    Vector3 m_v3Velocity;
    Vector3 m_v3Position;

    Vector3 m_v3Up;
    Vector3 m_v3Cross;

    float m_fPeriod;
    float m_fcurRadian;
    float m_fAmplitude;

  public:
    static std::vector<CSnowParticle *> ms_kVct_SnowParticlePool;
};
#endif /* METIN2_CLIENT_GAMELIB_SNOWPARTICLE_H */
