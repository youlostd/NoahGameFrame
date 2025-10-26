#include "StdAfx.h"
#include "ActorInstanceAccessor.h"
#define WORLD_EDITOR

#include <Gamelib/FlyingObjectManager.h>

#include <EffectLib/EffectManager.h>


METIN2_BEGIN_NS

void CActorInstanceAccessor::ClearAttachingObject()
{
	for (auto itor = m_AttachingObjectList.begin(); itor != m_AttachingObjectList.end(); ++itor)
	{
		SAttachingModelInstance & rInstance = *itor;
		delete rInstance.pModelInstance;
		rInstance.pModelInstance = NULL;
	}

	m_AttachingObjectList.clear();
}

void CActorInstanceAccessor::AttachObject(const char * c_szFileName, const char * c_szBoneName)
{
	if (0 == strlen(c_szFileName))
		return;

	SAttachingModelInstance ModelInstance;

	ModelInstance.pThing = CResourceManager::Instance().LoadResource<CGraphicThing>(c_szFileName);

	if (1 != ModelInstance.pThing->GetModelCount())
		return;

	ModelInstance.strBoneName = c_szBoneName;
	ModelInstance.pModelInstance = new CModelInstanceAccessor;

	ModelInstance.pModelInstance->SetLinkedModelPointer(ModelInstance.pThing->GetModelPointer(0),
	                                                    nullptr);

	m_AttachingObjectList.push_back(ModelInstance);
}

void CActorInstanceAccessor::UpdateAttachingObject()
{
	auto pModelInstance = GetModelInstancePointer(0);
	if (!pModelInstance)
		return;

	for (auto itor = m_AttachingObjectList.begin(); itor != m_AttachingObjectList.end(); ++itor)
	{
		SAttachingModelInstance & rInstance = *itor;
		rInstance.pModelInstance->AdvanceTime(m_fSecondElapsed);
		rInstance.pModelInstance->Deform(&GetIdentityMatrix(), 0.0f);

		int boneIndex;
		if (!pModelInstance->GetBoneIndexByName(rInstance.strBoneName.c_str(),
		                                        &boneIndex))
			continue;

		rInstance.pModelInstance->SetParentModelInstance(pModelInstance, boneIndex);
	}
}

void CActorInstanceAccessor::RenderAttachingObject()
{
	for (auto itor = m_AttachingObjectList.begin(); itor != m_AttachingObjectList.end(); ++itor)
	{
		SAttachingModelInstance & rInstance = *itor;
		rInstance.pModelInstance->RenderWithOneTexture();
	}
}

void CActorInstanceAccessor::SetMotion()
{
	m_kCurMotNode.iMotionType = MOTION_TYPE_LOOP;
	m_kCurMotNode.fStartTime = GetLocalTime();
	m_kCurMotNode.dwMotionKey = 0;
	m_kCurMotNode.fEndTime = 0.0f;
	m_kCurMotNode.fSpeedRatio = 1.0f;
	m_kCurMotNode.dwcurFrame = 0;
	m_kCurMotNode.dwFrameCount = CActorInstance::GetMotionDuration(0) / (1.0f / g_fGameFPS);
	CGraphicThingInstance::SetMotion(0, 0.0f, 1);
}

void CActorInstanceAccessor::SetLocalTime(float fLocalTime)
{
	CGraphicThingInstance::__SetLocalTime(fLocalTime);
	m_kCurMotNode.dwcurFrame = fLocalTime*g_fGameFPS;
}

void CActorInstanceAccessor::SetMotionData(CRaceMotionData * pMotionData)
{
	m_pkCurRaceMotionData = pMotionData;

	// RegisterEffect
	for (uint32_t i = 0; i < pMotionData->GetMotionEventDataCount(); ++i)
	{
		const CRaceMotionData::TMotionEventData * c_pData;
		if (!pMotionData->GetMotionEventDataPointer(i, &c_pData))
			continue;

		if (MOTION_EVENT_TYPE_EFFECT == c_pData->iType)
		{
			const auto c_pEffectData = (const CRaceMotionData::TMotionEffectEventData *)c_pData;
			CEffectManager::Instance().RegisterEffect(c_pEffectData->strEffectFileName.c_str());
		}
		else if (MOTION_EVENT_TYPE_FLY == c_pData->iType)
		{
			const auto c_pFlyData = (const CRaceMotionData::TMotionFlyEventData *)c_pData;
			CFlyingManager::Instance().RegisterFlyingData(c_pFlyData->strFlyFileName.c_str());
		}
	}
}

float CActorInstanceAccessor::GetMotionDuration()
{
	return CActorInstance::GetMotionDuration(0);
}

uint32_t CActorInstanceAccessor::GetBoneCount()
{
	CGrannyModel * pModel = m_pModelThing->GetModelPointer(0);
	granny_model * pgrnModel = pModel->GetGrannyModelPointer();
	return pgrnModel->Skeleton->BoneCount;
}

BOOL CActorInstanceAccessor::GetBoneName(uint32_t dwIndex, std::string * pstrBoneName)
{
	CGrannyModel * pModel = m_pModelThing->GetModelPointer(0);
	granny_model * pgrnModel = pModel->GetGrannyModelPointer();

	if (dwIndex >= uint32_t(pgrnModel->Skeleton->BoneCount))
		return FALSE;

	*pstrBoneName = pgrnModel->Skeleton->Bones[dwIndex].Name;
	return TRUE;
}

BOOL CActorInstanceAccessor::GetBoneMatrix(uint32_t dwBoneIndex, Matrix ** ppMatrix)
{
	return CGraphicThingInstance::GetBoneMatrix(0, dwBoneIndex, ppMatrix);
}

BOOL CActorInstanceAccessor::GetBoneIndexByName(const char * c_szBoneName, int * pBoneIndex) const
{
	if (m_modelInstances.empty())
		return FALSE;

	const auto c_pModelInstance = m_modelInstances[0];
	c_pModelInstance->GetBoneIndexByName(c_szBoneName, pBoneIndex);
	return TRUE;
}

BOOL CActorInstanceAccessor::SetAccessorModel(CGraphicThing::Ptr pThing)
{
	assert(pThing);

	if (!pThing)
		return FALSE;

	ClearModel();

	if (0 == pThing->GetModelCount())
	{
		spdlog::error("No models");
		return FALSE;
	}

	CGrannyModel * pModel = pThing->GetModelPointer(0);
	if (!pModel)
	{
		spdlog::error("No models");
		return FALSE;
	}

	CGraphicThingInstance::ReserveModelThing(2);
	CGraphicThingInstance::ReserveModelInstance(2);
	CGraphicThingInstance::RegisterModelThing(0, pThing);
	CGraphicThingInstance::SetModelInstance(0, 0, 0);

	m_pModelThing = pThing;
	return TRUE;
}

BOOL CActorInstanceAccessor::SetAccessorMotion(CGraphicThing::Ptr pThing)
{
	if (!pThing)
		return FALSE;

	ClearMotion();

	if (0 == pThing->GetMotionCount())
	{
		spdlog::error("No motions");
		return FALSE;
	}

	CGrannyMotion * pMotion = pThing->GetMotionPointer(0);
	if (!pMotion)
	{
		spdlog::error("No motions");
		return FALSE;
	}

	CGraphicThingInstance::RegisterMotionThing(0, 0, pThing);
	SetMotion();

	m_pMotionThing = pThing;
	return TRUE;
}

BOOL CActorInstanceAccessor::IsModelThing()
{
	return NULL != m_pModelThing;
}

BOOL CActorInstanceAccessor::IsMotionThing()
{
	return NULL != m_pMotionThing;
}

void CActorInstanceAccessor::ClearModel()
{
	m_pModelThing = NULL;
}

void CActorInstanceAccessor::ClearMotion()
{
	m_pMotionThing = NULL;
}

void CActorInstanceAccessor::ClearAttachingEffect()
{
	__ClearAttachingEffect();
}

CActorInstanceAccessor::CActorInstanceAccessor()
{
	m_pModelThing = NULL;
	m_pMotionThing = NULL;

	m_kCurMotNode.fStartTime = 0.0f;
	m_kCurMotNode.fEndTime = 0.0f;
}

CActorInstanceAccessor::~CActorInstanceAccessor()
{
}

METIN2_END_NS
