#include "stdafx.h"
#include "../eterLib/StateManager.h"
#include "../eterLib/Camera.h"

#include "FlyingData.h"
#include "FlyTrace.h"

#include <StepTimer.h>


#include "../EterLib/Engine.hpp"
#include "../EterLib/GrpScreen.h"

CDynamicPool<CFlyTrace>		CFlyTrace::ms_kPool;

void CFlyTrace::DestroySystem()
{
	ms_kPool.Destroy();
}

CFlyTrace* CFlyTrace::New()
{
	return ms_kPool.Alloc();
}

void CFlyTrace::Delete(CFlyTrace* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

CFlyTrace::CFlyTrace()
{
	__Initialize();
}

CFlyTrace::~CFlyTrace()
{
	Destroy();
}


void CFlyTrace::__Initialize()
{
	m_bRectShape=false;
	m_dwColor=0;
	m_fSize=0.0f;
	m_fTailLength=0.0f;
}

void CFlyTrace::Destroy()
{
	m_TimePositionDeque.clear();

	__Initialize();
}

void CFlyTrace::UpdateNewPosition(const Vector3 & v3Position)
{
	m_TimePositionDeque.push_front(TTimePosition(DX::StepTimer::Instance().GetTotalSeconds(),v3Position));
	while(!m_TimePositionDeque.empty() && m_TimePositionDeque.back().first+m_fTailLength<DX::StepTimer::Instance().GetTotalSeconds())
	{
		m_TimePositionDeque.pop_back();
	}
}

void CFlyTrace::Create(const CFlyingData::TFlyingAttachData & rFlyingAttachData)
{
	//assert(rFlyingAttachData.bHasTail);
	m_dwColor = rFlyingAttachData.dwTailColor;
	m_fTailLength = rFlyingAttachData.fTailLength;
	m_fSize = rFlyingAttachData.fTailSize;
	m_bRectShape = rFlyingAttachData.bRectShape;
}


void CFlyTrace::Update()
{

}

//1. 알파를 쓰려면 색깔만 줄수있다.
//2. 텍스쳐를 쓰려면 알파 없다-_-


struct TFlyVertex
{
	Vector3 p;
	DWORD c;
	Vector2 t;
	TFlyVertex(){};
	TFlyVertex(const Vector3& p, DWORD c, const Vector2 & t):p(p),c(c),t(t){}
};

struct TFlyVertexSet
{
	TFlyVertex v[6];
	TFlyVertexSet(TFlyVertex * pv)
	{
		memcpy(v,pv,sizeof(v));
	}
	bool operator < (const TFlyVertexSet& ) const
	{
		return false;
	}
	TFlyVertexSet & operator = ( const TFlyVertexSet& rhs )
	{
		memcpy(v,rhs.v,sizeof(v));
		return *this;
	}
};

typedef std::vector<std::pair<float, TFlyVertexSet> > TFlyVertexSetVector;

void CFlyTrace::Render()
{
	if (m_TimePositionDeque.size()<=1)
		return;
	TFlyVertexSetVector VSVector;

	Engine::GetDevice().SetDepthFunc(D3DCMP_LESS);
    Engine::GetDevice().SetDepthEnable(true, false);

	STATEMANAGER.SaveTransform(D3DTS_WORLD, &Matrix::Identity);
	Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);

	STATEMANAGER.SaveRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD );
	//STATEMANAGER.SaveRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD );
	
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	//STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, /*(m_bUseTexture)?D3DTOP_SELECTARG2:*/D3DTOP_SELECTARG2);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	//STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, /*(m_bUseTexture)?D3DTOP_SELECTARG2:*/D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);

    Matrix m = Matrix::Identity;
    CScreen s;
    s.UpdateViewMatrix();
	CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCurrentCamera)
		return;

	const Matrix & M = pCurrentCamera->GetViewMatrix();
	m = Matrix::Identity;
	Vector3 F(pCurrentCamera->GetView());
	m._31 = F.x;
	m._32 = F.y;
	m._33 = F.z;

	auto & frustum = s.GetFrustum();
	//frustum.BuildViewFrustum(ms_matView * ms_matProj);

	TTimePositionDeque::iterator it1, it2;
	it2 = it1 = m_TimePositionDeque.begin();
	++it2;
	for(;it2!=m_TimePositionDeque.end();++it2,++it1)
	{
		const Vector3& rkOld=it1->second;
		const Vector3& rkNew=it2->second;
		Vector3 B = rkNew - rkOld;

		float radius = std::max(fabs(B.x),std::max(fabs(B.y),fabs(B.z)))/2;
		Vector3d c(it1->second.x+B.x*0.5f,
			it1->second.y+B.y*0.5f,
			it1->second.z+B.z*0.5f
			);
        if (frustum.Contains(BoundingSphere(c, radius)) == DISJOINT)
			continue;

		float rate1 = (1-(DX::StepTimer::Instance().GetTotalSeconds()-it1->first)/m_fTailLength);
		float rate2 = (1-(DX::StepTimer::Instance().GetTotalSeconds()-it2->first)/m_fTailLength);
		float size1 = m_fSize;
		float size2 = m_fSize;
		if (!m_bRectShape)
		{
			size1 *= rate1;
			size2 *= rate2;
		}
		TFlyVertex v[6] =
		{
			TFlyVertex(Vector3(0.0f,size1,0.0f), m_dwColor,Vector2(0.0f,0.0f)),
			TFlyVertex(Vector3(-size1,0.0f,0.0f),m_dwColor,Vector2(0.0f,0.5f)),
			TFlyVertex(Vector3(size1,0.0f,0.0f), m_dwColor,Vector2(0.5f,0.0f)),
			TFlyVertex(Vector3(-size2,0.0f,0.0f),m_dwColor,Vector2(0.5f,1.0f)),
			TFlyVertex(Vector3(size2,0.0f,0.0f), m_dwColor,Vector2(1.0f,0.5f)),
			TFlyVertex(Vector3(0.0f,-size2,0.0f),m_dwColor,Vector2(1.0f,1.0f)),

			/*TVertex(Vector3(0.0f,size1,0.0f), ((DWORD)(0x40*rate1)<<24) + 0x0000ff,Vector2(0.0f,0.0f)),
			TVertex(Vector3(-size1,0.0f,0.0f),((DWORD)(0x40*rate1)<<24) + 0x0000ff,Vector2(0.0f,0.0f)),
			TVertex(Vector3(size1,0.0f,0.0f), ((DWORD)(0x40*rate1)<<24) + 0x0000ff,Vector2(0.0f,0.0f)),
			TVertex(Vector3(-size2,0.0f,0.0f),((DWORD)(0x40*rate2)<<24) + 0x0000ff,Vector2(0.0f,0.0f)),
			TVertex(Vector3(size2,0.0f,0.0f), ((DWORD)(0x40*rate2)<<24) + 0x0000ff,Vector2(0.0f,0.0f)),
			TVertex(Vector3(0.0f,-size2,0.0f),((DWORD)(0x40*rate2)<<24) + 0x0000ff,Vector2(0.0f,0.0f)),*/

			/*TVertex(Vector3(0.0f,size1,0.0f),0x20ff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(-size1,0.0f,0.0f),0x20ff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(size1,0.0f,0.0f),0x20ff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(-size2,0.0f,0.0f),0x20ff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(size2,0.0f,0.0f),0x20ff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(0.0f,-size2,0.0f),0x20ff0000,Vector2(0.0f,0.0f)),*/

			/*TVertex(Vector3(0.0f,size1,0.0f),0xffff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(-size1,0.0f,0.0f),0xffff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(size1,0.0f,0.0f),0xffff0000,Vector2(0.0f,0.0f)),
			TVertex(Vector3(-size2,0.0f,0.0f),0xff0000ff,Vector2(0.0f,0.0f)),
			TVertex(Vector3(size2,0.0f,0.0f),0xff0000ff,Vector2(0.0f,0.0f)),
			TVertex(Vector3(0.0f,-size2,0.0f),0xff0000ff,Vector2(0.0f,0.0f)),*/
		};


		Vector3 E(M._41,M._42,M._43);
		E = pCurrentCamera->GetEye();
		E-=it1->second;

		Vector3 P;
		P = B.Cross(E);

		Vector3 U;
		U = F.Cross(P);
		U.Normalize();
		Vector3 R;
		R = F.Cross(U);
		//D3DXMatrixIdentity(&m);
		m._21 = U.x;
		m._22 = U.y;
		m._23 = U.z;
		m._11 = R.x;
		m._12 = R.y;
		m._13 = R.z;
		int i;
		for(i=0;i<6;i++)
			v[i].p = Vector3::TransformNormal(v[i].p, m);
		for(i=0;i<3;i++)
			v[i].p += it1->second;
		for(;i<6;i++)
			v[i].p += it2->second;

		VSVector.push_back(std::make_pair(-E.Dot(pCurrentCamera->GetView()),TFlyVertexSet(v)));
		//OLD: STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, v, sizeof(TVertex));
		//OLD: STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v+1, sizeof(TVertex));
	}

	std::sort(VSVector.begin(),VSVector.end());

	for(TFlyVertexSetVector::iterator it = VSVector.begin();it!=VSVector.end();++it)
	{
		Engine::GetDevice().DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, it->second.v, sizeof(TVertex));
	}
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreTransform(D3DTS_WORLD);
    Engine::GetDevice().SetDepthEnable(true, true);
	Engine::GetDevice().SetDepthFunc(D3DCMP_LESSEQUAL);
	STATEMANAGER.RestoreRenderState(D3DRS_BLENDOP);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);

}


