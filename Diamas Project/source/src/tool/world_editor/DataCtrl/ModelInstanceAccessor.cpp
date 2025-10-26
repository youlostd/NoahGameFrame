#include "StdAfx.h"

#include <SpdLog.hpp>

METIN2_BEGIN_NS

void CModelInstanceAccessor::UpdateLocalTime(float fLocalTime)
{
	m_fLocalTime = fLocalTime;
}

float CModelInstanceAccessor::GetLocalTime()
{
	return m_fLocalTime;
}

float CModelInstanceAccessor::GetDuration()
{
	if (!m_pcurMotionThing)
		return 0.0f;

	if (m_pcurMotionThing->GetMotionCount() == 0)
		return 0.0f;

	return m_pcurMotionThing->GetMotionPointer(0)->GetDuration();
}

BOOL CModelInstanceAccessor::SetAccessorModel(CGraphicThing::Ptr pThing)
{
	assert(pThing);
	if (!pThing)
		return FALSE;

	ClearModel();

	if (0 == pThing->GetModelCount())
	{
		spdlog::error("모델 파일이 아닙니다", "Error");
		return FALSE;
	}

	CGrannyModel * pModel = pThing->GetModelPointer(0);
	if (!pModel)
	{
		spdlog::error("정상적인 모델 파일이 아닙니다", "Error");
		return FALSE;
	}

	SetLinkedModelPointer(pModel, nullptr);
	m_modelThing = pThing;
	return TRUE;
}

BOOL CModelInstanceAccessor::SetAccessorMotion(CGraphicThing::Ptr pThing)
{
	if (!pThing)
		return FALSE;

	ReleaseArtificialMotion();
	ClearMotion();

	if (0 == pThing->GetMotionCount())
	{
		spdlog::error("모션 파일이 아닙니다!");
		return FALSE;
	}

	auto pMotion = pThing->GetMotionPointer(0);
	if (!pMotion)
	{
		spdlog::error("Failed to load!");
		return FALSE;
	}

	SetArtificialMotion(pMotion);

	m_pcurMotionThing = pThing;
	return TRUE;
}

void CModelInstanceAccessor::SetArtificialMotion(const CGrannyMotion * pMotion)
{
	float localTime = GetLocalTime();

	if (m_pgrnCtrl)
	{
		// Delete animation at once
		//GrannySetControlEaseOutCurve(m_pgrnCtrl, localTime, localTime, 1.0f, 1.0f, 0.0f, 0.0f);
		//GrannySetControlEaseIn(m_pgrnCtrl, false);
		//GrannySetControlEaseOut(m_pgrnCtrl, true);
		GrannyFreeControl(m_pgrnCtrl);
	}

	granny_animation * pgrnAnimation = pMotion->GetGrannyAnimationPointer();
	granny_model_instance * pgrnModelInstance = m_modelInstance;
	if (NULL == pgrnModelInstance)
	{
		spdlog::error("pgrnModelInstance is NULL");
		return;
	}

 	m_pgrnCtrl = GrannyPlayControlledAnimation(localTime, pgrnAnimation, pgrnModelInstance);
	if (NULL == m_pgrnCtrl)
	{
		spdlog::error("m_pgrnCtrl is NULL");
		return;
	}

	GrannySetControlLoopCount(m_pgrnCtrl, 0);
}

void CModelInstanceAccessor::ReleaseArtificialMotion()
{
	if (m_pgrnCtrl)
	{
		GrannyFreeControl(m_pgrnCtrl);
		m_pgrnCtrl = NULL;
	}
}

// Interceptor Functions
void CModelInstanceAccessor::Deform(const Matrix* c_pWorldMatrix)
{
	if (!m_model)
		return;

	UpdateWorldPose(c_pWorldMatrix);
	UpdateWorldMatrices(c_pWorldMatrix);

}

void CModelInstanceAccessor::UpdateWorldPose(const Matrix* c_pWorldMatrix)
{
	granny_skeleton* pgrnSkeleton = GrannyGetSourceSkeleton(m_modelInstance);

	if (m_pgrnLocalPose)
	{
		if (m_boneCount < pgrnSkeleton->BoneCount)
		{
			GrannyFreeLocalPose(m_pgrnLocalPose);

			m_boneCount = pgrnSkeleton->BoneCount;
			m_pgrnLocalPose = GrannyNewLocalPose(m_boneCount);
		}
	}
	else
	{
		m_boneCount = pgrnSkeleton->BoneCount;
		m_pgrnLocalPose = GrannyNewLocalPose(m_boneCount);
	}

	float localTime = GetLocalTime();
	const Matrix* pAttachBoneMatrix = nullptr;
	if (m_attachedTo)
		pAttachBoneMatrix = (const Matrix *) m_attachedTo->GetBoneMatrixPointer(m_attachedToBone);

	GrannySetModelClock(m_modelInstance, localTime);
	GrannyFreeCompletedModelControls(m_modelInstance);

	GrannySampleModelAnimations(m_modelInstance, 0, pgrnSkeleton->BoneCount, m_pgrnLocalPose);
	GrannyBuildWorldPose(pgrnSkeleton,
	                     0,
	                     pgrnSkeleton->BoneCount,
	                     m_pgrnLocalPose,
	                     (granny_real32 const *)pAttachBoneMatrix,
	                     m_worldPose);
}

BOOL CModelInstanceAccessor::IsModelThing()
{
	return NULL != m_modelThing;
}

BOOL CModelInstanceAccessor::IsMotionThing()
{
	return NULL != m_pcurMotionThing;
}

uint32_t CModelInstanceAccessor::GetBoneCount()
{
	CGrannyModel * pModel = m_modelThing->GetModelPointer(0);
	granny_model * pgrnModel = pModel->GetGrannyModelPointer();
	return pgrnModel->Skeleton->BoneCount;
}

BOOL CModelInstanceAccessor::GetBoneName(uint32_t dwIndex, std::string * pstrBoneName)
{
	CGrannyModel * pModel = m_modelThing->GetModelPointer(0);
	granny_model * pgrnModel = pModel->GetGrannyModelPointer();

	if (dwIndex >= uint32_t(pgrnModel->Skeleton->BoneCount))
		return FALSE;

	*pstrBoneName = pgrnModel->Skeleton->Bones[dwIndex].Name;
	return TRUE;
}

void CModelInstanceAccessor::ClearModel()
{
	m_modelThing = NULL;
}

void CModelInstanceAccessor::ClearMotion()
{
	if (m_pcurMotionThing)
	{
		m_pMotionBackupList.push_back(m_pcurMotionThing);
		m_pcurMotionThing = NULL;
	}
}

CModelInstanceAccessor::CModelInstanceAccessor()
{
	m_boneCount = 0;
	m_pgrnLocalPose = NULL;
	m_pgrnCtrl = NULL;

	m_modelThing = NULL;
	m_pcurMotionThing = NULL;
}

CModelInstanceAccessor::~CModelInstanceAccessor()
{
	ReleaseArtificialMotion();
}

METIN2_END_NS
