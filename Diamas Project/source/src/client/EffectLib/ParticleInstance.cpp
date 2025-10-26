#include "StdAfx.h"
#include "ParticleInstance.h"
#include "ParticleProperty.h"
#include "Util.hpp"

#include "../EterLib/Camera.h"
#include <DirectXColors.h>
CDynamicPool<CParticleInstance> CParticleInstance::ms_kPool;

void CParticleInstance::DestroySystem()
{
    ms_kPool.Destroy();
}

void CParticleInstance::Create()
{
    ms_kPool.Create(10000);
}

CParticleInstance *CParticleInstance::New()
{
    return ms_kPool.Alloc();
}

void CParticleInstance::DeleteThis()
{
    Destroy();

    ms_kPool.Free(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CParticleInstance::GetRadiusApproximation() const
{
    return (m_v2HalfSize  * m_v2Scale).Length();
}

BOOL CParticleInstance::Update(double fElapsedTime, double fAngle)
{
    m_fLastLifeTime -= fElapsedTime;
    if (m_fLastLifeTime < 0.0)
        return false;

    const auto fLifePercentage = (m_fLifeTime - m_fLastLifeTime) / m_fLifeTime;

    UpdateRotation(fLifePercentage, fElapsedTime);
    UpdateTextureAnimation(fLifePercentage, fElapsedTime);
    UpdateScale(fLifePercentage, fElapsedTime);
    UpdateColor(fLifePercentage, fElapsedTime);
    UpdateGravity(fLifePercentage, fElapsedTime);
    UpdateAirResistance(fLifePercentage, fElapsedTime);

    m_v3LastPosition = m_v3Position;
    m_v3Position += m_v3Velocity * fElapsedTime;

	if (fAngle)
	{
		if (m_pParticleProperty->m_bAttachFlag)
		{
			float fCos, fSin;
			fAngle = XMConvertToRadians(fAngle);
			fCos = cos(fAngle);
			fSin = sin(fAngle);

			float rx = m_v3Position.x - m_v3StartPosition.x;
			float ry = m_v3Position.y - m_v3StartPosition.y;

			m_v3Position.x =   fCos * rx + fSin * ry + m_v3StartPosition.x;
			m_v3Position.y = - fSin * rx + fCos * ry + m_v3StartPosition.y;
		}
		else
		{
			Quaternion q,qc;
			q = Quaternion::CreateFromAxisAngle(m_pParticleProperty->m_v3ZAxis ,XMConvertToRadians(fAngle));
            q.Conjugate(qc);

			Quaternion qr(
				m_v3Position.x-m_v3StartPosition.x,
				m_v3Position.y-m_v3StartPosition.y,
				m_v3Position.z-m_v3StartPosition.z,
				0.0f);
			qr =  q * qr;
			qr = qr * qc;

			m_v3Position.x = qr.x;
			m_v3Position.y = qr.y;
			m_v3Position.z = qr.z;

			m_v3Position += m_v3StartPosition;
		}
	}

	return TRUE;
}

void CParticleInstance::Transform(const Matrix * c_matLocal)
{
	/////

	Vector3 v3Up;
	Vector3 v3Cross;

	if (!m_pParticleProperty->m_bStretchFlag)
	{
		CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
		const Vector3 & c_rv3Up = pCurrentCamera->GetUp();
		const Vector3 & c_rv3Cross = pCurrentCamera->GetCross();

		Vector3 v3Rotation;

		switch(m_pParticleProperty->m_byBillboardType) {
		case BILLBOARD_TYPE_LIE:
			{
				float fCos = cosf(XMConvertToRadians(m_fRotation)), fSin = sinf(XMConvertToRadians(m_fRotation));
				v3Up.x = fCos;
				v3Up.y = -fSin;
				v3Up.z = 0;
				v3Cross.x = fSin;
				v3Cross.y = fCos;
				v3Cross.z = 0;
			}
			break;
		case BILLBOARD_TYPE_2FACE:
		case BILLBOARD_TYPE_3FACE:
			// using setting with y, and local rotation at render
		case BILLBOARD_TYPE_Y:
			{
				v3Up = Vector3(0.0f,0.0f,1.0f);
				//v3Up = Vector3(cosf(XMConvertToRadians(m_fRotation)),0.0f,-sinf(XMConvertToRadians(m_fRotation)));
				const Vector3 & c_rv3View = pCurrentCamera->GetView();
				if (v3Up.x * c_rv3View.y - v3Up.y * c_rv3View.x<0)
					v3Up*=-1;
				v3Cross = v3Up.Cross(Vector3(c_rv3View.x,c_rv3View.y,0));
				v3Cross.Normalize();

				if (m_fRotation)
				{
					float fCos = -sinf(XMConvertToRadians(m_fRotation)); // + 90
					float fSin = cosf(XMConvertToRadians(m_fRotation));

					Vector3 v3Temp = v3Up * fCos - v3Cross * fSin;
					v3Cross = v3Cross * fCos + v3Up * fSin;
					v3Up = v3Temp;
				}

				//Vector3 v3Rotation;
				//D3DXVec3Cross(&v3Rotation, &v3Up, &v3Cross);

			}
			break;
		case BILLBOARD_TYPE_ALL:
		default:
			{
				// NOTE : Rotation Routine. Camera�� Up Vector�� Cross Vector ��ü�� View Vector ��������
				//        Rotation ��Ų��.
				// FIXME : �ݵ�� ����ȭ �� ��!
				if (m_fRotation==0.0f)
				{
					v3Up = -c_rv3Cross;
					v3Cross = c_rv3Up;
				}
				else
				{
					const Vector3 & c_rv3View = pCurrentCamera->GetView();
					Quaternion q,qc;
					q = Quaternion::CreateFromAxisAngle(c_rv3View, XMConvertToRadians(m_fRotation));
					q.Conjugate(qc); //D3DXQuaternionConjugate(&qc, &q);

					{
						Quaternion qr(-c_rv3Cross.x, -c_rv3Cross.y, -c_rv3Cross.z, 0);
						qr = qc * qr;
						qr = qr * q;
						v3Up.x = qr.x;
						v3Up.y = qr.y;
						v3Up.z = qr.z;
					}
					{
						Quaternion qr(c_rv3Up.x, c_rv3Up.y, c_rv3Up.z, 0);
						qr = qc * qr;
						qr = qr * q;
						v3Cross.x = qr.x;
						v3Cross.y = qr.y;
						v3Cross.z = qr.z;
					}

				}
				//D3DXMATRIX matRotation;

				//D3DXMatrixRotationAxis(&matRotation, &c_rv3View, XMConvertToRadians(m_fRotation));

				//D3DXVec3TransformCoord(&v3Up, &(-c_rv3Cross), &matRotation);
				//D3DXVec3TransformCoord(&v3Cross, &c_rv3Up, &matRotation);
			}
			break;
		}

	}
	else
	{
		v3Up = m_v3Position - m_v3LastPosition;

		if (c_matLocal)
		{
			//if (!m_pParticleProperty->m_bAttachFlag)
				v3Up = Vector3::TransformNormal(v3Up, *c_matLocal);
		}

		// NOTE: �ӵ��� ���̿� �ִ� ���� : log(velocity)��ŭ �þ��.
		float length = v3Up.Length();
		if (length == 0.0f)
		{
			v3Up = Vector3(0.0f,0.0f,1.0f);
		}
		else
			v3Up *=(1+log(1+length))/length;

		CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
		const Vector3 & c_rv3View = pCurrentCamera->GetView();
		v3Cross = v3Up.Cross(c_rv3View);
		v3Cross.Normalize();

	}

	v3Cross = -(m_v2HalfSize.x*m_v2Scale.x) * v3Cross;
	v3Up = (m_v2HalfSize.y*m_v2Scale.y) * v3Up;

	if (c_matLocal && m_pParticleProperty->m_bAttachFlag)
	{
		Vector3 v3Position = Vector3::Transform(m_v3Position, *c_matLocal);
		m_ParticleMesh[0].position = v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = v3Position + v3Up - v3Cross;
	}
	else
	{
		m_ParticleMesh[0].position = m_v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = m_v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = m_v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = m_v3Position + v3Up - v3Cross;
	}
}


void CParticleInstance::Transform(const Matrix * c_matLocal, const float c_fZRotation)
{

	Vector3 v3Up;
	Vector3 v3Cross;

	if (!m_pParticleProperty->m_bStretchFlag)
	{
		CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
		const Vector3 & c_rv3Up = pCurrentCamera->GetUp();
		const Vector3 & c_rv3Cross = pCurrentCamera->GetCross();

		Vector3 v3Rotation;

		switch(m_pParticleProperty->m_byBillboardType) {
		case BILLBOARD_TYPE_LIE:
			{
				float fCos = cosf(XMConvertToRadians(m_fRotation)), fSin = sinf(XMConvertToRadians(m_fRotation));
				v3Up.x = fCos;
				v3Up.y = -fSin;
				v3Up.z = 0;

				v3Cross.x = fSin;
				v3Cross.y = fCos;
				v3Cross.z = 0;
			}
			break;
		case BILLBOARD_TYPE_2FACE:
		case BILLBOARD_TYPE_3FACE:
			// using setting with y, and local rotation at render
		case BILLBOARD_TYPE_Y:
			{
				v3Up = Vector3(0.0f,0.0f,1.0f);
				//v3Up = Vector3(cosf(XMConvertToRadians(m_fRotation)),0.0f,-sinf(XMConvertToRadians(m_fRotation)));
				const Vector3 & c_rv3View = pCurrentCamera->GetView();
				if (v3Up.x * c_rv3View.y - v3Up.y * c_rv3View.x<0)
					v3Up*=-1;
				v3Cross = v3Up.Cross(Vector3(c_rv3View.x,c_rv3View.y,0));
				v3Cross.Normalize(v3Cross);

				if (m_fRotation)
				{
					float fCos = -sinf(XMConvertToRadians(m_fRotation)); // + 90
					float fSin = cosf(XMConvertToRadians(m_fRotation));

					Vector3 v3Temp = v3Up * fCos - v3Cross * fSin;
					v3Cross = v3Cross * fCos + v3Up * fSin;
					v3Up = v3Temp;
				}

				//Vector3 v3Rotation;
				//D3DXVec3Cross(&v3Rotation, &v3Up, &v3Cross);

			}
			break;
		case BILLBOARD_TYPE_ALL:
		default:
			{
				// NOTE : Rotation Routine. Camera�� Up Vector�� Cross Vector ��ü�� View Vector ��������
				//        Rotation ��Ų��.
				// FIXME : �ݵ�� ����ȭ �� ��!
				if (m_fRotation==0.0f)
				{
					v3Up = -c_rv3Cross;
					v3Cross = c_rv3Up;
				}
				else
				{
					const Vector3 & c_rv3View = pCurrentCamera->GetView();
					Matrix matRotation  = Matrix::CreateFromAxisAngle(c_rv3View, XMConvertToRadians(m_fRotation));
					v3Up = Vector3::Transform((-c_rv3Cross), matRotation);
					v3Cross = Vector3::Transform(c_rv3Up, matRotation);
				}
			}
			break;
		}
	}
	else
	{
		v3Up = m_v3Position - m_v3LastPosition;

		if (c_matLocal)
		{
			//if (!m_pParticleProperty->m_bAttachFlag)
				v3Up = Vector3::TransformNormal(v3Up, *c_matLocal);
		}

		// NOTE: �ӵ��� ���̿� �ִ� ���� : log(velocity)��ŭ �þ��.
		float length = v3Up.Length();
		if (length == 0.0f)
		{
			v3Up = Vector3(0.0f,0.0f,1.0f);
		}
		else
			v3Up *=(1+log(1+length))/length;
		//D3DXVec3Normalize(&v3Up,&v3Up);
		//v3Up *= 1+log(1+length);

		CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
		const Vector3 & c_rv3View = pCurrentCamera->GetView();
		v3Cross = v3Up.Cross(c_rv3View);
		v3Cross.Normalize(v3Cross);

	}

	if (c_fZRotation)
	{
		float x, y;
		float fCos = cosf(c_fZRotation);
		float fSin = sinf(c_fZRotation);

		x = v3Up.x;
		y = v3Up.y;
		v3Up.x = x * fCos - y * fSin;
		v3Up.y = y * fCos + x * fSin;

		x = v3Cross.x;
		y = v3Cross.y;
		v3Cross.x = x * fCos - y * fSin;
		v3Cross.y = y * fCos + x * fSin;
	}

	v3Cross = -(m_v2HalfSize.x*m_v2Scale.x) * v3Cross;
	v3Up = (m_v2HalfSize.y*m_v2Scale.y) * v3Up;

	if (c_matLocal && m_pParticleProperty->m_bAttachFlag)
	{
		Vector3 v3Position = Vector3::Transform(m_v3Position, *c_matLocal);
		m_ParticleMesh[0].position = v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = v3Position + v3Up - v3Cross;
	}
	else
	{
		m_ParticleMesh[0].position = m_v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = m_v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = m_v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = m_v3Position + v3Up - v3Cross;
	}
}


void CParticleInstance::UpdateRotation(double time, double elapsedTime)
{
    if (m_rotationType == CParticleProperty::ROTATION_TYPE_NONE)
        return;

    if (m_rotationType == CParticleProperty::ROTATION_TYPE_TIME_EVENT)
        m_fRotationSpeed = GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventRotation);

    m_fRotation += m_fRotationSpeed * elapsedTime;
}

void CParticleInstance::UpdateTextureAnimation(double time, double elapsedTime)
{
    if (m_byTextureAnimationType == CParticleProperty::TEXTURE_ANIMATION_TYPE_NONE)
        return;

    const auto frameDelay = m_pParticleProperty->GetTextureAnimationFrameDelay();
    const auto frameCount = m_pParticleProperty->GetTextureAnimationFrameCount();

    m_frameTime += elapsedTime;

    const auto elapsedFrames = static_cast<uint64_t>(m_frameTime / frameDelay);

    if (0 == elapsedFrames)
        return;

    m_frameTime -= elapsedFrames * frameDelay;

    switch (m_byTextureAnimationType)
    {
    case CParticleProperty::TEXTURE_ANIMATION_TYPE_CW:
        m_byFrameIndex += elapsedFrames;
        if (m_byFrameIndex >= frameCount)
            m_byFrameIndex = 0;
        break;

    case CParticleProperty::TEXTURE_ANIMATION_TYPE_CCW:
        m_byFrameIndex = std::min<uint8_t>(m_byFrameIndex - elapsedFrames, frameCount - 1);
        break;

    case CParticleProperty::TEXTURE_ANIMATION_TYPE_RANDOM_FRAME:
        if (frameCount != 0)
            m_byFrameIndex = GetRandom<uint32_t>(0, frameCount - 1);
        break;

    default:
        break;
    }
}

void CParticleInstance::UpdateScale(double time, double elapsedTime)
{
    if (!m_pParticleProperty->m_TimeEventScaleX.empty())
        m_v2Scale.x = GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventScaleX);

    if (!m_pParticleProperty->m_TimeEventScaleY.empty())
        m_v2Scale.y = GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventScaleY);
}

void CParticleInstance::UpdateColor(double time, double elapsedTime)
{
    if (m_pParticleProperty->m_TimeEventColor.empty())
        return;

    m_dcColor = GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventColor);
}

void CParticleInstance::UpdateGravity(double time, double elapsedTime)
{
    if (m_pParticleProperty->m_TimeEventGravity.empty())
        return;

    float gravity = GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventGravity);

    m_v3Velocity.z -= gravity * elapsedTime;
}

void CParticleInstance::UpdateAirResistance(double time, double elapsedTime)
{
    if (m_pParticleProperty->m_TimeEventAirResistance.empty())
        return;

    float r = GetTimeEventBlendValue(time, m_pParticleProperty->m_TimeEventAirResistance);

    m_v3Velocity *= 1.0f - r;
}

void CParticleInstance::Destroy()
{
    __Initialize();
}

void CParticleInstance::__Initialize()
{
    //*

    m_v3Position = Vector3(0.0f, 0.0f, 0.0f);
    m_v3LastPosition = m_v3Position;
    m_v3Velocity = Vector3(0.0f, 0.0f, 0.0f);

    m_v2Scale = Vector2(1.0f, 1.0f);
#ifdef WORLD_EDITOR
    m_Color = DirectX::Colors::White;
#else
    m_dcColor = 0xffffffff;
#endif

    m_byFrameIndex = 0;
    m_ParticleMesh[0].texCoord = Vector2(0.0f, 1.0f);
    m_ParticleMesh[1].texCoord = Vector2(0.0f, 0.0f);
    m_ParticleMesh[2].texCoord = Vector2(1.0f, 1.0f);
    m_ParticleMesh[3].texCoord = Vector2(1.0f, 0.0f);
}

CParticleInstance::CParticleInstance()
{
    __Initialize();
}

CParticleInstance::~CParticleInstance()
{
    Destroy();
}

TPTVertex *CParticleInstance::GetParticleMeshPointer()
{
    return m_ParticleMesh;
}
