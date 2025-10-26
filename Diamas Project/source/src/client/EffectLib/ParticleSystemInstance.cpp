#include "ParticleSystemInstance.h"
#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpDevice.h"
#include "ParticleInstance.h"
#include "ParticleSystemData.h"
#include "StdAfx.h"
#include "Util.hpp"

CDynamicPool<CParticleSystemInstance> CParticleSystemInstance::ms_kPool;

void CParticleSystemInstance::DestroySystem()
{
    ms_kPool.Destroy();

    CParticleInstance::DestroySystem();
    // CRayParticleInstance::DestroySystem();
}

CParticleSystemInstance *CParticleSystemInstance::New()
{
    return ms_kPool.Alloc();
}

void CParticleSystemInstance::Delete(CParticleSystemInstance *pkPSInst)
{
    pkPSInst->Destroy();
    ms_kPool.Free(pkPSInst);
}

DWORD CParticleSystemInstance::GetEmissionCount() const
{
    return m_dwCurrentEmissionCount;
}

void CParticleSystemInstance::SetScale(float scale)
{
    m_scale = scale;
}

void CParticleSystemInstance::Emit(double elapsedTime)
{
    float fEmissionCount;
    m_pEmitterProperty->GetEmissionCountPerSecond(m_localTime, &fEmissionCount);

    double fCreatingValue = fEmissionCount * (elapsedTime / 1.0f) + m_fEmissionResidue;
    int iCreatingCount = int(fCreatingValue);
    m_fEmissionResidue = float(fCreatingValue) - float(iCreatingCount);

    const int currentEmissionCount = GetEmissionCount();
    const int iMaxEmissionCount = int(m_pEmitterProperty->GetMaxEmissionCount());
    const int iNextEmissionCount = int(currentEmissionCount + iCreatingCount);
    iCreatingCount -= std::max(0, iNextEmissionCount - iMaxEmissionCount);

    float fLifeTime = 0.0f;
    float fEmittingSize = 0.0f;
    Vector3 _v3TimePosition;
    Vector3 _v3Velocity;
    float fVelocity = 0.0f;
    Vector2 v2HalfSize;
    float fLieRotation = 0;
    if (iCreatingCount)
    {
        m_pEmitterProperty->GetParticleLifeTime(m_localTime, &fLifeTime);
        if (fLifeTime == 0.0f)
        {
            return;
        }

        m_pEmitterProperty->GetEmittingSize(m_localTime, &fEmittingSize);

        m_pData->GetPosition(m_localTime, _v3TimePosition);

        m_pEmitterProperty->GetEmittingDirectionX(m_localTime, &_v3Velocity.x);
        m_pEmitterProperty->GetEmittingDirectionY(m_localTime, &_v3Velocity.y);
        m_pEmitterProperty->GetEmittingDirectionZ(m_localTime, &_v3Velocity.z);

        m_pEmitterProperty->GetEmittingVelocity(m_localTime, &fVelocity);

        m_pEmitterProperty->GetParticleSizeX(m_localTime, &v2HalfSize.x);
        m_pEmitterProperty->GetParticleSizeY(m_localTime, &v2HalfSize.y);

        v2HalfSize *= m_scale;

        if (BILLBOARD_TYPE_LIE == m_pParticleProperty->m_byBillboardType && m_localMat)
        {
            float fsx = m_localMat->_32;
            float fcx = sqrtf(1.0f - fsx * fsx);

            if (fcx >= 0.00001f)
                fLieRotation = DirectX::XMConvertToDegrees(atan2f(-m_localMat->_12, m_localMat->_22));
        }
    }

    for (int i = 0; i < iCreatingCount; ++i)
    {
        CParticleInstance *pInstance;

        pInstance = CParticleInstance::New();
        pInstance->m_pParticleProperty = m_pParticleProperty;
        pInstance->m_pEmitterProperty = m_pEmitterProperty;

        // LifeTime
        pInstance->m_fLifeTime = fLifeTime;
        pInstance->m_fLastLifeTime = fLifeTime;

        // Position
        switch (m_pEmitterProperty->GetEmitterShape())
        {
        case CEmitterProperty::EMITTER_SHAPE_POINT:
            pInstance->m_v3Position.x = 0.0f;
            pInstance->m_v3Position.y = 0.0f;
            pInstance->m_v3Position.z = 0.0f;
            break;

        case CEmitterProperty::EMITTER_SHAPE_ELLIPSE:
            pInstance->m_v3Position.x = GetRandom(-500.0f, 500.0f);
            pInstance->m_v3Position.y = GetRandom(-500.0f, 500.0f);
            pInstance->m_v3Position.z = 0.0f;
            pInstance->m_v3Position.Normalize();

            if (m_pEmitterProperty->isEmitFromEdge())
            {
                pInstance->m_v3Position *= m_pEmitterProperty->m_fEmittingRadius + fEmittingSize;
            }
            else
            {
                pInstance->m_v3Position *= GetRandom(0.0f, m_pEmitterProperty->m_fEmittingRadius) + fEmittingSize;
            }
            break;

        case CEmitterProperty::EMITTER_SHAPE_SQUARE:
            pInstance->m_v3Position.x = GetRandom(-m_pEmitterProperty->m_v3EmittingSize.x / 2.0f,
                                                  m_pEmitterProperty->m_v3EmittingSize.x / 2.0f) +
                                        fEmittingSize;
            pInstance->m_v3Position.y = GetRandom(-m_pEmitterProperty->m_v3EmittingSize.y / 2.0f,
                                                  m_pEmitterProperty->m_v3EmittingSize.y / 2.0f) +
                                        fEmittingSize;
            pInstance->m_v3Position.z = GetRandom(-m_pEmitterProperty->m_v3EmittingSize.z / 2.0f,
                                                  m_pEmitterProperty->m_v3EmittingSize.z / 2.0f) +
                                        fEmittingSize;
            break;

        case CEmitterProperty::EMITTER_SHAPE_SPHERE:
            pInstance->m_v3Position.x = GetRandom(-500.0f, 500.0f);
            pInstance->m_v3Position.y = GetRandom(-500.0f, 500.0f);
            pInstance->m_v3Position.z = GetRandom(-500.0f, 500.0f);
            pInstance->m_v3Position.Normalize();

            if (m_pEmitterProperty->isEmitFromEdge())
            {
                pInstance->m_v3Position *= m_pEmitterProperty->m_fEmittingRadius + fEmittingSize;
            }
            else
            {
                pInstance->m_v3Position *= GetRandom(0.0f, m_pEmitterProperty->m_fEmittingRadius) + fEmittingSize;
            }
            break;
        default:
            break;
        }

        // Position
        Vector3 v3TimePosition = _v3TimePosition;

        pInstance->m_v3Position += v3TimePosition;

        if (m_localMat && !m_pParticleProperty->m_bAttachFlag)
        {
            pInstance->m_v3Position = Vector3::Transform(pInstance->m_v3Position, *m_localMat);
            v3TimePosition = Vector3::Transform(v3TimePosition, *m_localMat);
        }
        pInstance->m_v3StartPosition = v3TimePosition;
        // NOTE : Update를 호출하지 않고 Rendering 되기 때문에 length가 0이 되는 문제가 있다.
        //        Velocity를 구한 후 그만큼 빼준 값으로 초기화 해주도록 바꿨음 - [levites]
        // pInstance->m_v3LastPosition = pInstance->m_v3Position;

        // Direction & Velocity
        pInstance->m_v3Velocity.x = 0.0f;
        pInstance->m_v3Velocity.y = 0.0f;
        pInstance->m_v3Velocity.z = 0.0f;

        if (CEmitterProperty::EMITTER_ADVANCED_TYPE_INNER == m_pEmitterProperty->GetEmitterAdvancedType())
        {
            pInstance->m_v3Velocity = (pInstance->m_v3Position - v3TimePosition);
            pInstance->m_v3Velocity.Normalize();
            pInstance->m_v3Velocity *= -100.0f;
        }
        else if (CEmitterProperty::EMITTER_ADVANCED_TYPE_OUTER == m_pEmitterProperty->GetEmitterAdvancedType())
        {
            if (m_pEmitterProperty->GetEmitterShape() == CEmitterProperty::EMITTER_SHAPE_POINT)
            {
                pInstance->m_v3Velocity.x = GetRandom(-100.0f, 100.0f);
                pInstance->m_v3Velocity.y = GetRandom(-100.0f, 100.0f);
                pInstance->m_v3Velocity.z = GetRandom(-100.0f, 100.0f);
            }
            else
            {
                pInstance->m_v3Velocity = (pInstance->m_v3Position - v3TimePosition);
                pInstance->m_v3Velocity.Normalize();
                pInstance->m_v3Velocity *= 100.0f;
            }
        }

        Vector3 v3Velocity = _v3Velocity;
        if (m_localMat && !m_pParticleProperty->m_bAttachFlag)
        {
            v3Velocity = Vector3::TransformNormal(v3Velocity, *m_localMat);
            //D3DXVec3TransformNormal(&v3Velocity, &v3Velocity, m_localMat);
        }

        pInstance->m_v3Velocity += v3Velocity;

        if (m_pEmitterProperty->m_v3EmittingDirection.x > 0.0f)
            pInstance->m_v3Velocity.x += GetRandom(-m_pEmitterProperty->m_v3EmittingDirection.x / 2.0f,
                                                   m_pEmitterProperty->m_v3EmittingDirection.x / 2.0f) *
                                         1000.0f;

        if (m_pEmitterProperty->m_v3EmittingDirection.y > 0.0f)
            pInstance->m_v3Velocity.y += GetRandom(-m_pEmitterProperty->m_v3EmittingDirection.y / 2.0f,
                                                   m_pEmitterProperty->m_v3EmittingDirection.y / 2.0f) *
                                         1000.0f;

        if (m_pEmitterProperty->m_v3EmittingDirection.z > 0.0f)
            pInstance->m_v3Velocity.z += GetRandom(-m_pEmitterProperty->m_v3EmittingDirection.z / 2.0f,
                                                   m_pEmitterProperty->m_v3EmittingDirection.z / 2.0f) *
                                         1000.0f;

        pInstance->m_v3Velocity *= fVelocity;

		// Size
		pInstance->m_v2HalfSize = v2HalfSize;

		// Rotation
		pInstance->m_fRotation = m_pParticleProperty->m_wRotationRandomStartingBegin;
		if (m_pParticleProperty->m_wRotationRandomStartingEnd > m_pParticleProperty->m_wRotationRandomStartingBegin)
			pInstance->m_fRotation = GetRandom<float>(m_pParticleProperty->m_wRotationRandomStartingBegin,
			                                               m_pParticleProperty->m_wRotationRandomStartingEnd);

		// Rotation - Lie 일 경우 LocalMatrix 의 Rotation 값을 Random 에 적용한다.
		//            매번 할 필요는 없을듯. 어느 정도의 최적화가 필요. - [levites]
		if (BILLBOARD_TYPE_LIE == m_pParticleProperty->m_byBillboardType && m_localMat)
			pInstance->m_fRotation += fLieRotation;

		auto rotationType = m_pParticleProperty->m_byRotationType;

		if (m_pParticleProperty->m_fRotationSpeed == 0.0f &&
		    rotationType != CParticleProperty::ROTATION_TYPE_TIME_EVENT)
			rotationType = CParticleProperty::ROTATION_TYPE_NONE;
		else if (rotationType == CParticleProperty::ROTATION_TYPE_RANDOM_DIRECTION)
			rotationType = GetRandom(0, 1) ? CParticleProperty::ROTATION_TYPE_CW:
			                                    CParticleProperty::ROTATION_TYPE_CCW;

		switch (rotationType) {
			case CParticleProperty::ROTATION_TYPE_CW:
				pInstance->m_fRotationSpeed = m_pParticleProperty->m_fRotationSpeed;
				break;

			case CParticleProperty::ROTATION_TYPE_CCW:
				pInstance->m_fRotationSpeed = -m_pParticleProperty->m_fRotationSpeed;
				break;
		}

		pInstance->m_rotationType = rotationType;

        // Texture Animation
        pInstance->m_byFrameIndex = 0;
        pInstance->m_byTextureAnimationType = m_pParticleProperty->GetTextureAnimationType();

        if (m_pParticleProperty->GetTextureAnimationFrameCount() > 1)
        {
            if (CParticleProperty::TEXTURE_ANIMATION_TYPE_RANDOM_DIRECTION ==
                m_pParticleProperty->GetTextureAnimationType())
            {
                if (GetRandom(0, 1))
                {
                    pInstance->m_byFrameIndex = 0;
                    pInstance->m_byTextureAnimationType = CParticleProperty::TEXTURE_ANIMATION_TYPE_CW;
                }
                else
                {
                    pInstance->m_byFrameIndex = m_pParticleProperty->GetTextureAnimationFrameCount() - 1;
                    pInstance->m_byTextureAnimationType = CParticleProperty::TEXTURE_ANIMATION_TYPE_CCW;
                }
            }
            if (m_pParticleProperty->m_bTexAniRandomStartFrameFlag)
            {
                pInstance->m_byFrameIndex =
                    GetRandom<uint32_t>(0, m_pParticleProperty->GetTextureAnimationFrameCount() - 1);
            }
        }

        // Simple Update
        {
            pInstance->m_v3LastPosition = pInstance->m_v3Position - (pInstance->m_v3Velocity * elapsedTime);
            pInstance->m_v2Scale.x = m_pParticleProperty->m_TimeEventScaleX.front().m_Value;
            pInstance->m_v2Scale.y = m_pParticleProperty->m_TimeEventScaleY.front().m_Value;
            // pInstance->m_v2Scale = m_pParticleProperty->m_TimeEventScaleXY.front().m_Value;
            pInstance->m_dcColor = m_pParticleProperty->m_TimeEventColor.front().m_Value;
        }

        m_ParticleInstanceListVector[pInstance->m_byFrameIndex].push_back(pInstance);
        m_dwCurrentEmissionCount++;
    }
}

bool CParticleSystemInstance::InFrustum(CParticleInstance *pInstance) const {
  if (m_pParticleProperty->m_bAttachFlag)
    return CScreen::GetFrustum().Contains(BoundingSphere(
               Vector3(pInstance->m_v3Position.x + m_localMat->_41,
                       pInstance->m_v3Position.y + m_localMat->_42,
                       pInstance->m_v3Position.z + m_localMat->_43),
               pInstance->GetRadiusApproximation())) !=
           ContainmentType::DISJOINT;

  return CScreen::GetFrustum().Contains(
             BoundingSphere(pInstance->m_v3Position,
                            pInstance->GetRadiusApproximation())) !=
         ContainmentType::DISJOINT;
}

bool CParticleSystemInstance::OnUpdate(double fElapsedTime)
{
    bool bMakeParticle = true;
    /////

    if (m_localTime >= m_pEmitterProperty->GetCycleLength())
    {
        if (m_pEmitterProperty->isCycleLoop() && --m_iLoopCount != 0)
        {
            if (m_iLoopCount < 0)
                m_iLoopCount = 0;
            m_localTime = m_localTime - m_pEmitterProperty->GetCycleLength();
        }
        else
        {
            bMakeParticle = false;
            m_iLoopCount = 1;
            if (GetEmissionCount() == 0)
                return false;
        }
    }

    /////

    int dwFrameIndex;
    int dwFrameCount = m_pParticleProperty->GetTextureAnimationFrameCount();

    float fAngularVelocity;
    m_pEmitterProperty->GetEmittingAngularVelocity(m_localTime, &fAngularVelocity);

	if (fAngularVelocity && !m_pParticleProperty->m_bAttachFlag)
    {
        m_pParticleProperty->m_v3ZAxis = Vector3::TransformNormal(Vector3::Forward, *m_localMat);
    }

    for (dwFrameIndex = 0; dwFrameIndex < dwFrameCount; dwFrameIndex++)
    {
        auto itor = m_ParticleInstanceListVector[dwFrameIndex].begin();
        for (; itor != m_ParticleInstanceListVector[dwFrameIndex].end();)
        {
            CParticleInstance *pInstance = *itor;

            if (!pInstance->Update(fElapsedTime, fAngularVelocity))
            {
                pInstance->DeleteThis();

                itor = m_ParticleInstanceListVector[dwFrameIndex].erase(itor);
                m_dwCurrentEmissionCount--;
            }
            else
            {
                if (pInstance->m_byFrameIndex != dwFrameIndex)
                {
                    m_ParticleInstanceListVector[dwFrameCount + pInstance->m_byFrameIndex].push_back(*itor);
                    itor = m_ParticleInstanceListVector[dwFrameIndex].erase(itor);
                }
                else
                    ++itor;
            }
        }
    }

    if (isActive() && (!isHiddenByLod() || m_isAlwaysShow) && bMakeParticle)
        Emit(fElapsedTime);

    for (dwFrameIndex = 0; dwFrameIndex < dwFrameCount; ++dwFrameIndex)
    {
        m_ParticleInstanceListVector[dwFrameIndex].splice(m_ParticleInstanceListVector[dwFrameIndex].end(),
                                                          m_ParticleInstanceListVector[dwFrameIndex + dwFrameCount]);
        m_ParticleInstanceListVector[dwFrameIndex + dwFrameCount].clear();
    }
    m_SortedParticleInstanceMap.clear();
    for (DWORD frameIndex = 0; frameIndex < m_kVct_pkImgInst.size(); frameIndex++)
    {
        auto* tex  = m_kVct_pkImgInst[frameIndex]->GetTextureReference().GetD3DTexture();
        auto  itor = m_ParticleInstanceListVector[frameIndex].begin();
        for (; itor != m_ParticleInstanceListVector[frameIndex].end(); ++itor)
        {
            if (!m_isAlwaysShow && !InFrustum(*itor))
                continue;
            m_SortedParticleInstanceMap[tex].emplace_back(*itor);
        }
    }
    return true;
}

namespace NParticleRenderer
{
struct TwoSideRenderer
{
    const Matrix *pmat;
    uint32_t m_addColor;

    TwoSideRenderer(uint32_t addColor, const Matrix *pmat = NULL) : pmat(pmat), m_addColor{addColor} {
    }

    BOOST_FORCEINLINE void operator()(CParticleInstance *pInstance) const
    {
        const auto color = BlendColor(pInstance->GetColor(), Color(m_addColor));
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, color.BGRA());

        pInstance->Transform(pmat, DirectX::XMConvertToRadians(-30.0f));
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(),
                                            sizeof(TPTVertex));

        pInstance->Transform(pmat, DirectX::XMConvertToRadians(+30.0f));
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(),
                                            sizeof(TPTVertex));
    }
};

struct ThreeSideRenderer
{
    const Matrix *pmat;
    uint32_t addColor;

    ThreeSideRenderer(uint32_t addColor, const Matrix *pmat = NULL) : pmat(pmat), addColor(addColor)
    {
    }

    BOOST_FORCEINLINE void operator()(CParticleInstance *pInstance) const
    {
        const auto color = BlendColor(pInstance->GetColor(), Color(addColor));
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, color.BGRA());

        pInstance->Transform(pmat);
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(),
                                            sizeof(TPTVertex));
        pInstance->Transform(pmat, DirectX::XMConvertToRadians(-60.0f));
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(),
                                            sizeof(TPTVertex));
        pInstance->Transform(pmat, DirectX::XMConvertToRadians(+60.0f));
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(),
                                            sizeof(TPTVertex));
    }
};

struct NormalRenderer
{
    uint32_t addColor;

    NormalRenderer(uint32_t addColor) : addColor(addColor)
    {
        // ctor
    }

    BOOST_FORCEINLINE void operator()(CParticleInstance *pInstance) const
    {
        const auto color = BlendColor(pInstance->GetColor(), Color(addColor));
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, color.BGRA());

        pInstance->Transform(nullptr);
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(),
                                            sizeof(TPTVertex));
    }
};

struct AttachRenderer
{
    const Matrix *pmat;
    uint32_t addColor;

    AttachRenderer(uint32_t addColor, const Matrix *pmat) : pmat(pmat), addColor(addColor)
    {
    }

    void operator()(CParticleInstance *pInstance)
    {
        const auto color = BlendColor(pInstance->GetColor(), Color(addColor));
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, color.BGRA());

        pInstance->Transform(pmat);
        Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(),
                                            sizeof(TPTVertex));
    }
};
} // namespace NParticleRenderer

void CParticleSystemInstance::OnRender(uint32_t addColor)
{
    CScreen::Identity();

    STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, m_pParticleProperty->m_bySrcBlendType);
    STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, m_pParticleProperty->m_byDestBlendType);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, m_pParticleProperty->m_byColorOperationType);



    switch (m_pParticleProperty->m_byBillboardType)
    {
    case BILLBOARD_TYPE_2FACE:
        ForEachParticleRendering(!m_pParticleProperty->m_bAttachFlag
                                     ? NParticleRenderer::TwoSideRenderer(addColor)
                                     : NParticleRenderer::TwoSideRenderer(addColor, m_localMat));
        return;
    case BILLBOARD_TYPE_3FACE:
        ForEachParticleRendering(!m_pParticleProperty->m_bAttachFlag
                                     ? NParticleRenderer::ThreeSideRenderer(addColor)
                                     : NParticleRenderer::ThreeSideRenderer(addColor, m_localMat));
        return;

    default: {
        !m_pParticleProperty->m_bAttachFlag
            ? ForEachParticleRendering(NParticleRenderer::NormalRenderer(addColor))
            : ForEachParticleRendering(NParticleRenderer::AttachRenderer(addColor, m_localMat));
    }
    break;
    }
}

void CParticleSystemInstance::OnSetDataPointer(CEffectElementBase *pElement)
{
    m_pData = (CParticleSystemData *)pElement;

    m_dwCurrentEmissionCount = 0;
    m_pParticleProperty = m_pData->GetParticlePropertyPointer();
    m_pEmitterProperty = m_pData->GetEmitterPropertyPointer();
    m_iLoopCount = m_pEmitterProperty->GetLoopCount();
    m_ParticleInstanceListVector.resize(m_pParticleProperty->GetTextureAnimationFrameCount() * 2 + 2);

    /////

    assert(m_kVct_pkImgInst.empty());
    m_kVct_pkImgInst.reserve(m_pParticleProperty->m_ImageVector.size());
    for (const auto &i : m_pParticleProperty->m_ImageVector)
    {
        auto pkImgInstNew = std::make_unique<CGraphicImageInstance>();
        pkImgInstNew->SetImagePointer(i);
        m_kVct_pkImgInst.emplace_back(std::move(pkImgInstNew));
    }
}

void CParticleSystemInstance::OnInitialize()
{
    m_dwCurrentEmissionCount = 0;
    m_iLoopCount = 0;
    m_fEmissionResidue = 0.0f;
}

void CParticleSystemInstance::OnDestroy()
{
    for (auto &rkLst_kParticleInst : m_ParticleInstanceListVector)
    {
        for (auto &particleInstance : rkLst_kParticleInst)
            particleInstance->DeleteThis();

        rkLst_kParticleInst.clear();
    }

    m_ParticleInstanceListVector.clear();

    m_kVct_pkImgInst.clear();
}

CParticleSystemInstance::CParticleSystemInstance()
{
    Initialize();
}

CParticleSystemInstance::~CParticleSystemInstance()
{
    assert(m_ParticleInstanceListVector.empty());
    assert(m_kVct_pkImgInst.empty());
}
