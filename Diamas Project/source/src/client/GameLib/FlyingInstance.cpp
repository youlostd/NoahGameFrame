#include "Stdafx.h"
#include "MapManager.h"
#include "FlyingData.h"
#include "FlyTrace.h"
#include "FlyingInstance.h"

#include <StepTimer.h>

#include "FlyingObjectManager.h"
#include "FlyTarget.h"
#include "FlyHandler.h"
#include "ActorInstance.h"

#include "../EterLib/GrpMath.h"
#include "../EffectLib/EffectManager.h"

#include <base/Random.hpp>

#include "../EterLib/CullingManager.h"

class CCullingManager;
CDynamicPool<CFlyingInstance> CFlyingInstance::ms_kPool;

CFlyingInstance::CFlyingInstance()
{
	__Initialize();
}

CFlyingInstance::~CFlyingInstance()
{
	Destroy();
}

void CFlyingInstance::__Initialize()
{
	m_qAttachRotation=m_qRot=Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
	m_v3Accel=m_v3LocalVelocity=m_v3Velocity=m_v3Position=Vector3(0.0f, 0.0f, 0.0f);

	m_pHandler=NULL;
	m_pData=NULL;
	m_pOwner=NULL;

	m_bAlive=false;
	m_canAttack=false;

	m_dwSkillIndex = 0;

	m_iPierceCount=0;

	m_fStartTime=0.0f;
	m_fRemainRange=0.0f;

	m_bTargetHitted = false;
	m_HittedObjectSet.clear();
}

void CFlyingInstance::Clear()
{
	Destroy();
}

void CFlyingInstance::Destroy()
{
	m_FlyTarget.Clear();

	ClearAttachInstance();

	__Initialize();
}

void CFlyingInstance::BuildAttachInstance()
{
	for(int i=0;i<m_pData->GetAttachDataCount();i++)
	{
		CFlyingData::TFlyingAttachData & rfad = m_pData->GetAttachDataReference(i);

		switch(rfad.iType)
		{
			case CFlyingData::FLY_ATTACH_OBJECT:
				// NOT Implemented
				assert(!"FlyingInstance.cpp:BuildAttachInstance Not implemented FLY_ATTACH_OBJECT");
				break;
			case CFlyingData::FLY_ATTACH_EFFECT: {
				CEffectManager & rem = CEffectManager::Instance();
				TAttachEffectInstance aei;

				DWORD dwCRC = ComputeCrc32(0, rfad.strFilename.c_str(),rfad.strFilename.size());

				aei.iAttachIndex = i;
				aei.dwEffectInstanceIndex = rem.CreateEffect(dwCRC);
				aei.pFlyTrace = NULL;

				if (rfad.bHasTail)
				{
					aei.pFlyTrace = CFlyTrace::New();
					aei.pFlyTrace->Create(rfad);
				}

				if (aei.dwEffectInstanceIndex != 0)
					m_vecAttachEffectInstance.push_back(aei);
				break;
		}
			}
	}
}

void CFlyingInstance::Create(CFlyingData* pData, const Vector3& c_rv3StartPos, const CFlyTarget & c_rkTarget, bool canAttack)
{
	m_FlyTarget = c_rkTarget;
	m_canAttack = canAttack;

	__SetDataPointer(pData, c_rv3StartPos);
	__SetTargetDirection(m_FlyTarget);
}

void CFlyingInstance::__SetTargetDirection(const CFlyTarget& c_rkTarget)
{
	Vector3 v3TargetPos=c_rkTarget.GetFlyTargetPosition();

	// 임시 코드
	if (m_pData->m_bMaintainParallel)
	{
		v3TargetPos.z += 50.0f;
	}

	Vector3 v3TargetDir=v3TargetPos-m_v3Position;
	v3TargetDir.Normalize();

	__SetTargetNormalizedDirection(v3TargetDir);
}

void CFlyingInstance::__SetTargetNormalizedDirection(const Vector3 & v3NomalizedDirection)
{
	Quaternion q = SafeRotationNormalizedArc(Vector3::Down,v3NomalizedDirection);
    m_qRot = m_qRot * q;
	m_v3Velocity = Vector3::Transform(m_v3LocalVelocity, m_qRot);
	m_v3Accel = Vector3::Transform(m_pData->m_v3Accel, m_qRot);
}

// 2004. 3. 26. myevan. 기능을 몰라 일단 주석 처리. 적절한 네이밍이 필요. 게임에서 사용하지 않는다면 툴에서 툴 전용으로 상속받아 만들도록 하자
void CFlyingInstance::SetFlyTarget(const CFlyTarget & cr_Target)
{
	//m_pFlyTarget = pTarget;
	m_FlyTarget = cr_Target;
	//SetStartTargetPosition(m_FlyTarget.GetFlyTargetPosition());

	__SetTargetDirection(m_FlyTarget);
}

void CFlyingInstance::AdjustDirectionForHoming(const Vector3 & v3TargetPosition)
{
	Vector3 vTargetDir(v3TargetPosition);
	vTargetDir -= m_v3Position;
	vTargetDir.Normalize();

	Vector3 vVel;
    m_v3Velocity.Normalize(vVel);

	if (Vector3::DistanceSquared(vVel, vTargetDir) < 0.001f)
		return;

	Quaternion q = SafeRotationNormalizedArc(vVel,vTargetDir);

	if (m_pData->m_fHomingMaxAngle > 180)
	{

		m_v3Velocity = Vector3::Transform(m_v3Velocity, q);
		m_v3Accel = Vector3::Transform(m_v3Accel, q);
		m_qRot = q * m_qRot;
		return;
	}

	float c = cosf(XMConvertToRadians(m_pData->m_fHomingMaxAngle));
	float s = sinf(XMConvertToRadians(m_pData->m_fHomingMaxAngle));

	if (q.w <= -1.0f + 0.0001f)
	{
		q.x = 0;
		q.y = 0;
		q.z = s;
		q.w = c;
	}
	else if (q.w <= c && q.w <= 1.0f - 0.0001f)
	{
		float factor = s / sqrtf(1.0f - q.w * q.w);
		q.x *= factor;
		q.y *= factor;
		q.z *= factor;
		q.w = c;
	}
	/*else
	{
	}*/
	m_v3Velocity = Vector3::Transform(m_v3Velocity, q);
	m_v3Accel = Vector3::Transform(m_v3Accel, q);
	m_qRot = m_qRot * q;
}

void CFlyingInstance::UpdateAttachInstance()
{
	// Update Attach Rotation
	Quaternion q = Quaternion::CreateFromYawPitchRoll(
		XMConvertToRadians(m_pData->m_v3AngVel.y)*DX::StepTimer::Instance().GetElapsedSeconds(),
		XMConvertToRadians(m_pData->m_v3AngVel.x)*DX::StepTimer::Instance().GetElapsedSeconds(),
		XMConvertToRadians(m_pData->m_v3AngVel.z)*DX::StepTimer::Instance().GetElapsedSeconds());

	m_qAttachRotation = m_qAttachRotation * q;
	q = m_qAttachRotation * m_qRot;

	CEffectManager & rem = CEffectManager::Instance();
	TAttachEffectInstanceVector::iterator it;
	for(it = m_vecAttachEffectInstance.begin();it!=m_vecAttachEffectInstance.end();++it)
	{
		CFlyingData::TFlyingAttachData & rfad = m_pData->GetAttachDataReference(it->iAttachIndex);
		assert(rfad.iType == CFlyingData::FLY_ATTACH_EFFECT);
		Matrix m;
		switch(rfad.iFlyType)
		{
			case CFlyingData::FLY_ATTACH_TYPE_LINE:
				m = Matrix::CreateFromQuaternion(m_qRot);
				//D3DXMatrixRotationQuaternion(&m,&q);
				m._41=m_v3Position.x;
				m._42=m_v3Position.y;
				m._43=m_v3Position.z;
				break;
			case CFlyingData::FLY_ATTACH_TYPE_MULTI_LINE:
				{
					Vector3 p(
						-sinf(XMConvertToRadians(rfad.fRoll))*rfad.fDistance,
						0.0f,
						-cosf(XMConvertToRadians(rfad.fRoll))*rfad.fDistance);
					//Vec3TransformQuaternionSafe(&p,&p,&m_qRot);
					p = Vector3::Transform(p, q);
					p+=m_v3Position;
					//D3DXMatrixRotationQuaternion(&m,&m_qRot);
					m = Matrix::CreateFromQuaternion(q);
					m._41=p.x;
					m._42=p.y;
					m._43=p.z;
				}
				break;
			case CFlyingData::FLY_ATTACH_TYPE_SINE:
				{
					float angle = (DX::StepTimer::Instance().GetTotalSeconds() - m_fStartTime)*2*3.1415926535897931f/rfad.fPeriod;
					Vector3 p(
						-sinf(XMConvertToRadians(rfad.fRoll))*rfad.fAmplitude*sinf(angle),
						0.0f,
						-cosf(XMConvertToRadians(rfad.fRoll))*rfad.fAmplitude*sinf(angle));
					p = Vector3::Transform(p, q);
					//Vec3TransformQuaternionSafe(&p,&p,&m_qRot);
					p+=m_v3Position;
                    m = Matrix::CreateFromQuaternion(q);
					//D3DXMatrixRotationQuaternion(&m,&m_qRot);
					m._41=p.x;
					m._42=p.y;
					m._43=p.z;
					//assert(!"NOT IMPLEMENTED");
				}
				break;
			case CFlyingData::FLY_ATTACH_TYPE_EXP:
				{
					float dt = DX::StepTimer::Instance().GetTotalSeconds() - m_fStartTime;
					float angle = dt/rfad.fPeriod;
					Vector3 p(
						-sinf(XMConvertToRadians(rfad.fRoll))*rfad.fAmplitude*exp(-angle)*angle,
						0.0f,
						-cosf(XMConvertToRadians(rfad.fRoll))*rfad.fAmplitude*exp(-angle)*angle);
					//Vec3TransformQuaternionSafe(&p,&p,&m_qRot);
					p = Vector3::Transform(p, q);
					p+=m_v3Position;
					m = Matrix::CreateFromQuaternion(q);
					//D3DXMatrixRotationQuaternion(&m,&m_qRot);
					m._41=p.x;
					m._42=p.y;
					m._43=p.z;
					//assert(!"NOT IMPLEMENTED");
				}
				break;
		}

		const auto effect = rem.GetEffectInstance(it->dwEffectInstanceIndex);
		if (effect) {
		    effect->SetGlobalMatrix(m);
			effect->SetAddColor(m_addColor);
		}

		if (it->pFlyTrace)
			it->pFlyTrace->UpdateNewPosition(Vector3(m._41,m._42,m._43));
	}
}
struct FCheckBackgroundDuringFlying {
	CDynamicSphereInstance s;
	bool bHit;
	FCheckBackgroundDuringFlying(const Vector3 & v1, const Vector3 & v2)
	{
		s.fRadius = 1.0f;
		s.v3LastPosition = v1;
		s.v3Position = v2;
		bHit = false;
	}
	void operator () (CGraphicObjectInstance * p)
	{
		if (!p)
			return;

		if (!bHit && p->GetType() != ACTOR_OBJECT)
		{
			if (p->CollisionDynamicSphere(s))
			{
				bHit = true;
			}
		}
	}
	bool IsHitted()
	{
		return bHit;
	}
};

struct FCheckAnotherMonsterDuringFlying {
	CDynamicSphereInstance s;
	CGraphicObjectInstance * pInst;
	const CActorInstance * pOwner;
	FCheckAnotherMonsterDuringFlying(const CActorInstance * pOwner, const Vector3 & v1, const Vector3 & v2)
		: pOwner(pOwner)
	{
		s.fRadius = 10.0f;
		s.v3LastPosition = v1;
		s.v3Position = v2;
		pInst = 0;
	}
	void operator () (CGraphicObjectInstance * p)
	{
		if (!p)
			return;

		if (!pInst && p->GetType() == ACTOR_OBJECT)
		{
			CActorInstance * pa = (CActorInstance*) p;
			if (pa != pOwner && pa->TestCollisionWithDynamicSphere(s))
			{
				pInst = p;
			}
		}
	}
	bool IsHitted()
	{
		return pInst!=0;
	}
	CGraphicObjectInstance * GetHittedObject()
	{
		return pInst;
	}
};


bool CFlyingInstance::Update()
{
	if (!m_bAlive)
		return false;

	if (m_pData->m_bIsHoming &&
		m_pData->m_fHomingStartTime + m_fStartTime < DX::StepTimer::Instance().GetTotalSeconds())
	{
		if (m_FlyTarget.IsObject())
			AdjustDirectionForHoming(m_FlyTarget.GetFlyTargetPosition());
	}

	Vector3 v3LastPosition = m_v3Position;

	m_v3Velocity += m_v3Accel*DX::StepTimer::Instance().GetElapsedSeconds();
	m_v3Velocity.z+=m_pData->m_fGravity * DX::StepTimer::Instance().GetElapsedSeconds();
	Vector3 v3Movement = m_v3Velocity * DX::StepTimer::Instance().GetElapsedSeconds();
	float _fMoveDistance = v3Movement.Length();
	float fCollisionSphereRadius = std::max(_fMoveDistance*2, m_pData->m_fCollisionSphereRadius);
	m_fRemainRange -= _fMoveDistance;
	m_v3Position += v3Movement;

	UpdateAttachInstance();

	if (m_fRemainRange<0)
	{
		if (m_pHandler)
			m_pHandler->OnExplodingOutOfRange();

		__Explode(false);
		return false;
	}

	if (m_FlyTarget.IsObject())
	{
		if (!m_bTargetHitted)
		{
			if (square_distance_between_linesegment_and_point(m_v3Position,v3LastPosition,m_FlyTarget.GetFlyTargetPosition())<m_pData->m_fBombRange*m_pData->m_fBombRange)
			{
				m_bTargetHitted = true;

				if (m_canAttack)
				{
					IFlyTargetableObject* pVictim=m_FlyTarget.GetFlyTarget();
					if (pVictim)
					{
						pVictim->OnShootDamage();
					}
				}

				if (m_pHandler)
				{
					m_pHandler->OnExplodingAtTarget(m_dwSkillIndex);
				}

				if (m_iPierceCount)
				{
					m_iPierceCount--;
					__Bomb();
				}
				else
				{
					__Explode();
					return false;
				}

				return true;
			}
		}
	}
	else if (m_FlyTarget.IsPosition())
	{
		if (square_distance_between_linesegment_and_point(m_v3Position,v3LastPosition,m_FlyTarget.GetFlyTargetPosition())<m_pData->m_fBombRange*m_pData->m_fBombRange)
		{
			__Explode();
			return false;
		}
	}

	Vector3d vecStart, vecDir;
	vecStart = v3LastPosition;
	vecDir = v3Movement;

	CCullingManager & rkCullingMgr = CCullingManager::Instance();

	if (m_pData->m_bHitOnAnotherMonster)
	{
		FCheckAnotherMonsterDuringFlying kCheckAnotherMonsterDuringFlying(m_pOwner, v3LastPosition,m_v3Position);
		rkCullingMgr.ForInRange(vecStart,fCollisionSphereRadius, &kCheckAnotherMonsterDuringFlying);
		if (kCheckAnotherMonsterDuringFlying.IsHitted())
		{
			CActorInstance * pHittedInstance = (CActorInstance*)kCheckAnotherMonsterDuringFlying.GetHittedObject();
			if (m_HittedObjectSet.end() == m_HittedObjectSet.find(pHittedInstance))
			{
				m_HittedObjectSet.insert(pHittedInstance);

				if (m_pHandler)
				{
					m_pHandler->OnExplodingAtAnotherTarget(m_dwSkillIndex, pHittedInstance->GetVirtualID());
				}

				if (m_iPierceCount)
				{
					m_iPierceCount--;
					__Bomb();
				}
				else
				{
					__Explode();
					return false;
				}

				return true;
			}
		}
	}

	if (m_pData->m_bHitOnBackground)
	{
		// 지형 충돌

		if (CFlyingManager::Instance().GetMapManagerPtr())
		{
			float fGroundHeight = CFlyingManager::Instance().GetMapManagerPtr()->GetTerrainHeight(m_v3Position.x,-m_v3Position.y);
			if (fGroundHeight>m_v3Position.z)
			{
				if (m_pHandler)
					m_pHandler->OnExplodingAtBackground();

				__Explode();
				return false;
			}
		}

		// 건물+나무 충돌

		FCheckBackgroundDuringFlying kCheckBackgroundDuringFlying(v3LastPosition,m_v3Position);
		rkCullingMgr.ForInRange(vecStart,fCollisionSphereRadius, &kCheckBackgroundDuringFlying);

		if (kCheckBackgroundDuringFlying.IsHitted())
		{
			if (m_pHandler)
				m_pHandler->OnExplodingAtBackground();

			__Explode();
			return false;
		}
	}

	return true;
}

void CFlyingInstance::ClearAttachInstance()
{
	CEffectManager & rkEftMgr = CEffectManager::Instance();

	TAttachEffectInstanceVector::iterator i;
	for(i = m_vecAttachEffectInstance.begin();i!=m_vecAttachEffectInstance.end();++i)
	{
		rkEftMgr.DestroyEffectInstance(i->dwEffectInstanceIndex);

		if (i->pFlyTrace)
			CFlyTrace::Delete(i->pFlyTrace);

		i->iAttachIndex=0;
		i->dwEffectInstanceIndex=0;
		i->pFlyTrace=NULL;
	}
	m_vecAttachEffectInstance.clear();
}

void CFlyingInstance::__Explode(bool bBomb)
{
	if (!m_bAlive)
		return;

	m_bAlive = false;

	if (bBomb)
		__Bomb();
}

void CFlyingInstance::__Bomb()
{
	auto& mgr = CEffectManager::Instance();
	if (!m_pData->m_dwBombEffectID)
		return;

	const auto index = mgr.CreateEffect(m_pData->m_dwBombEffectID);

	Matrix m = Matrix::CreateTranslation(m_v3Position);

	const auto effect = mgr.GetEffectInstance(index);
	if (effect) {
	    effect->SetAddColor(m_addColor);
		effect->SetGlobalMatrix(m);
	}
}

void CFlyingInstance::Render()
{
	if (!m_bAlive)
		return;
	RenderAttachInstance();
}

void CFlyingInstance::RenderAttachInstance()
{
	TAttachEffectInstanceVector::iterator it;
	for(it = m_vecAttachEffectInstance.begin();it!=m_vecAttachEffectInstance.end();++it)
	{
		if (it->pFlyTrace)
			it->pFlyTrace->Render();
	}
}

void CFlyingInstance::SetDataPointer(CFlyingData * pData, const Vector3 & v3StartPosition)
{
	__SetDataPointer(pData, v3StartPosition);
}

void CFlyingInstance::__SetDataPointer(CFlyingData * pData, const Vector3 & v3StartPosition)
{
	m_pData = pData;
	m_qRot = Quaternion(0.0f,0.0f,0.0f,1.0f),
	m_v3Position = (v3StartPosition);
	m_bAlive = (true);

	m_fStartTime = DX::StepTimer::Instance().GetTotalSeconds();

	m_qRot = Quaternion::CreateFromYawPitchRoll(XMConvertToRadians(pData->m_fRollAngle-90.0f),0.0f,XMConvertToRadians(pData->m_fConeAngle));
	if (pData->m_bSpreading)
	{
		Quaternion q1, q2;
		q2 = Quaternion::CreateFromAxisAngle(Vector3::Backward,(GetRandom(-3.141592f/3,+3.141592f/3)+GetRandom(-3.141592f/3,+3.141592f/3))/2);
		q1 = Quaternion::CreateFromAxisAngle(Vector3::Down, GetRandom(0.0f,2*3.1415926535897931f));
		q1 = q2 * q1;
		m_qRot = q1 * m_qRot;
	}
	m_v3Velocity = m_v3LocalVelocity = Vector3(0.0f,-pData->m_fInitVel,0.0f);
	m_v3Accel = pData->m_v3Accel;
	m_fRemainRange = pData->m_fRange;
	m_qAttachRotation = Quaternion(0.0f,0.0f,0.0f,1.0f);

	BuildAttachInstance();
	UpdateAttachInstance();

	m_iPierceCount = pData->m_iPierceCount;
}


