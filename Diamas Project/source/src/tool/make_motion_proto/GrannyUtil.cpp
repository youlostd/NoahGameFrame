#include "GrannyUtil.hpp"

#include <granny.h>
#include <SpdLog.hpp>

GrannyFile::GrannyFile(granny_file* file)
	: m_file(file)
{
	// ctor
}

GrannyFile::~GrannyFile()
{
	if (m_file)
		GrannyFreeFile(m_file);
}

GrannyFile::operator granny_file*()
{ return m_file; }

GrannyAnimationProcessor::GrannyAnimationProcessor()
	: m_modelFile(nullptr)
	, m_modelFileInfo(nullptr)
{
	// ctor
}

GrannyAnimationProcessor::~GrannyAnimationProcessor()
{
	if (m_modelFile)
		GrannyFreeFile(m_modelFile);
}

bool GrannyAnimationProcessor::LoadModel(const storm::String& filename)
{
	m_modelFile = GrannyReadEntireFile(filename.c_str());
	if (!m_modelFile) {
		SPDLOG_ERROR("Failed to load {0}", filename);
		return false;
	}

	m_modelFileInfo = GrannyGetFileInfo(m_modelFile);
	if (!m_modelFileInfo) {
		SPDLOG_ERROR("Failed to parse {0}", filename);
		return false;
	}

	if (m_modelFileInfo->ModelCount == 0) {
		SPDLOG_ERROR("File {0} has no models", filename);
		return false;
	}

	if (m_modelFileInfo->ModelCount > 1)
		SPDLOG_WARN("File {0} has too many models", filename);

	return true;
}

bool GrannyAnimationProcessor::ProcessAnimation(const storm::String& filename,
                                                AnimationInfo& animInfo)
{
	GrannyFile file(GrannyReadEntireFile(filename.c_str()));
	if (!file) {
		SPDLOG_ERROR("Failed to load {0}", filename);
		return false;
	}

	auto info = GrannyGetFileInfo(file);
	if (!info) {
		SPDLOG_ERROR("Failed to parse {0}", filename);
		return false;
	}

	if (info->AnimationCount == 0) {
		SPDLOG_ERROR("File {0} has no animations", filename);
		return false;
	}

	if (info->AnimationCount > 1)
		SPDLOG_WARN("File {0} has too many animations", filename);

	auto animation = info->Animations[0];

	auto modelInstance = GrannyInstantiateModel(m_modelFileInfo->Models[0]);
	auto control = GrannyPlayControlledAnimation(0.0f,
	                                             animation,
	                                             modelInstance);

	if (!control) {
		SPDLOG_ERROR("Failed to play animation {0}", filename);
		return false;
	}

	GrannySetControlRawLocalClock(control, animation->Duration);

	granny_real32 translation[3];
	granny_real32 rotation[3];

	GrannyGetRootMotionVectors(modelInstance,
	                           animation->Duration,
	                           translation,
	                           rotation,
	                           false);

	animInfo.duration = animation->Duration;
	animInfo.accumulation.x() = translation[0];
	animInfo.accumulation.y() = translation[1];
	animInfo.accumulation.z() = translation[2];

	GrannyFreeControl(control);
	GrannyFreeModelInstance(modelInstance);
	return true;
}


