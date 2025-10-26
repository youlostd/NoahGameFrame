#include "GrannyScene.hpp"

#include <granny.h>



GrannyScene::GrannyScene(uint32_t boneCount)
	: m_sharedLocalPose(GrannyNewLocalPose(boneCount))
	, m_sharedWorldPose(GrannyNewWorldPoseNoComposite(boneCount))
{
	// ctor
}

GrannyScene::~GrannyScene()
{
	GrannyFreeLocalPose(m_sharedLocalPose);
	GrannyFreeWorldPose(m_sharedWorldPose);
}

granny_local_pose* GrannyScene::GetLocalPose(uint32_t boneCount)
{
	if (boneCount > GrannyGetLocalPoseBoneCount(m_sharedLocalPose)) {
		GrannyFreeLocalPose(m_sharedLocalPose);
		m_sharedLocalPose = GrannyNewLocalPose(boneCount);
	}

	return m_sharedLocalPose;
}

granny_world_pose* GrannyScene::GetWorldPose(uint32_t boneCount)
{
	if (boneCount > GrannyGetWorldPoseBoneCount(m_sharedWorldPose)) {
		GrannyFreeWorldPose(m_sharedWorldPose);
		m_sharedWorldPose = GrannyNewWorldPoseNoComposite(boneCount);
	}

	return m_sharedWorldPose;
}



