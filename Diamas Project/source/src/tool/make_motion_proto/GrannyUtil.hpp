#ifndef METIN2_TOOL_MAKEMOTIONPROTO_GRANNYUTIL_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_GRANNYUTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <Eigen/Core>

struct granny_file;
struct granny_file_info;


class GrannyFile
{
	public:
		GrannyFile(granny_file* file);
		~GrannyFile();

		operator granny_file*();

	private:
		granny_file* m_file;
};

struct AnimationInfo
{
	float duration;
	Eigen::Vector3f accumulation;
};

class GrannyAnimationProcessor
{
	public:
		GrannyAnimationProcessor();
		~GrannyAnimationProcessor();

		bool LoadModel(const storm::String& filename);

		bool ProcessAnimation(const storm::String& filename,
		                      AnimationInfo& animInfo);

	private:
		granny_file* m_modelFile;
		granny_file_info* m_modelFileInfo;
};

#endif // METIN2_TOOL_MAKEMOTIONPROTO_GRANNYUTIL_HPP
