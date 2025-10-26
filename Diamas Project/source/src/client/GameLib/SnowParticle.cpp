#include "StdAfx.h"
#include "SnowParticle.h"

const float c_fSnowDistance = 70000.0f;

std::vector<CSnowParticle *> CSnowParticle::ms_kVct_SnowParticlePool;

void CSnowParticle::SetCameraVertex(const Vector3 &rv3Up, const Vector3 &rv3Cross)
{
    m_v3Up = rv3Up * m_fHalfWidth;
    m_v3Cross = rv3Cross * m_fHalfHeight;
}

bool CSnowParticle::IsActivate()
{
    return m_bActivate;
}

void CSnowParticle::Update(float fElapsedTime, const Vector3 &c_rv3Pos)
{
    m_v3Position += m_v3Velocity * fElapsedTime;

    m_v3Position.x += m_v3Cross.x * sin(m_fcurRadian) / 10.0f;
    m_v3Position.y += m_v3Cross.y * sin(m_fcurRadian) / 10.0f;
    m_fcurRadian += m_fPeriod * fElapsedTime;

    if (m_v3Position.z < c_rv3Pos.z - 500.0f)
        m_bActivate = false;
    else if (abs(m_v3Position.x - c_rv3Pos.x) > c_fSnowDistance)
        m_bActivate = false;
    else if (abs(m_v3Position.y - c_rv3Pos.y) > c_fSnowDistance)
        m_bActivate = false;
}

void CSnowParticle::GetVerticies(SParticleVertex &rv3Vertex1, SParticleVertex &rv3Vertex2,
                                 SParticleVertex &rv3Vertex3, SParticleVertex &rv3Vertex4)
{
    rv3Vertex1.v3Pos = m_v3Position - m_v3Cross - m_v3Up;
    rv3Vertex1.u = 0.0f;
    rv3Vertex1.v = 0.0f;

    rv3Vertex2.v3Pos = m_v3Position + m_v3Cross - m_v3Up;
    rv3Vertex2.u = 1.0f;
    rv3Vertex2.v = 0.0f;

    rv3Vertex3.v3Pos = m_v3Position - m_v3Cross + m_v3Up;
    rv3Vertex3.u = 0.0f;
    rv3Vertex3.v = 1.0f;

    rv3Vertex4.v3Pos = m_v3Position + m_v3Cross + m_v3Up;
    rv3Vertex4.u = 1.0f;
    rv3Vertex4.v = 1.0f;
}

void CSnowParticle::Init(const Vector3 &c_rv3Pos)
{
    float fRot = Random::get(0.0f, 36000.0f) / 100.0f;
    float fDistance = Random::get(0.0f, c_fSnowDistance) / 10.0f;

    m_v3Position.x = c_rv3Pos.x + fDistance * sin((double)DirectX::XMConvertToRadians(fRot));
    m_v3Position.y = c_rv3Pos.y + fDistance * cos((double)DirectX::XMConvertToRadians(fRot));
    m_v3Position.z = c_rv3Pos.z + Random::get(1500.0f, 2000.0f);
    m_v3Velocity.x = 0.0f;
    m_v3Velocity.y = 0.0f;
    m_v3Velocity.z = Random::get(-50.0f, -200.0f);
    m_fHalfWidth = Random::get(2.0f, 7.0f);
    m_fHalfHeight = m_fHalfWidth;
    m_bActivate = true;
    m_bChangedSize = false;

    m_fPeriod = Random::get(1.5f, 5.0f);
    m_fcurRadian = Random::get(-1.6f, 1.6f);
    m_fAmplitude = Random::get(1.0f, 3.0f);
}

CSnowParticle *CSnowParticle::New()
{
    if (ms_kVct_SnowParticlePool.empty())
    {
        return new CSnowParticle;
    }

    CSnowParticle *pParticle = ms_kVct_SnowParticlePool.back();
    ms_kVct_SnowParticlePool.pop_back();
    return pParticle;
}

void CSnowParticle::Delete(CSnowParticle *pSnowParticle)
{
    ms_kVct_SnowParticlePool.push_back(pSnowParticle);
}

void CSnowParticle::DestroyPool()
{
    stl_wipe(ms_kVct_SnowParticlePool);
}

CSnowParticle::CSnowParticle()
{
}

CSnowParticle::~CSnowParticle()
{
}
