#ifndef METIN2_TOOL_MAKEMOTIONPROTO_GRANNYSCENE_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_GRANNYSCENE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <granny.h>

METIN2_BEGIN_NS

class GrannyScene
{
	public:
		// Default bone-count used to initialize the poses.
		static const uint32_t kInitialBoneCount = 64;

		GrannyScene(uint32_t boneCount = kInitialBoneCount);
		~GrannyScene();

		granny_local_pose* GetLocalPose(uint32_t boneCount);
		granny_world_pose* GetWorldPose(uint32_t boneCount);

	private:
		granny_local_pose* m_sharedLocalPose;
		granny_world_pose* m_sharedWorldPose;
};

METIN2_END_NS

#endif // METIN2_TOOL_MAKEMOTIONPROTO_GRANNYSCENE_HPP
