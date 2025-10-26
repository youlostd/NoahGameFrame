#include "StdAfx.h"
#include "ModelInstance.h"
#include "Model.h"

namespace
{
static granny_control *PlayAnimation(float startTime, granny_animation *ani,
                                     granny_model_instance *mi)
{
    // We don't use GrannyPlayControlledAnimation() here because
    // it produces nasty warning messages if it can't play the animation,
    // which will happen frequently, since this function is called
    // on all ModelInstances of a ThingInstance and animations generally
    // don't need to support all of them.
    //
    // Log message: Unable to find matching track_group for Model: "%s" in Animation: "%s"

    auto* builder = GrannyBeginControlledAnimation(startTime, ani);
    if (!builder)
        return nullptr;

    granny_int32 trackGroupIndex;
    if (!GrannyFindTrackGroupForModel(ani,
                                      GrannyGetSourceModel(mi)->Name,
                                      &trackGroupIndex))
        // Animation cannot be played on this model - ignore it
        return nullptr;

    //GrannySetTrackGroupLOD(builder, trackGroupIndex, true, 1.0f); // TODO: Check the documentation
    GrannySetTrackGroupTarget(builder, trackGroupIndex, mi);
    return GrannyEndControlledAnimation(builder);
}
}

bool CGrannyModelInstance::IsMotionPlaying()
{
    if (!m_pgrnCtrl)
        return false;

    if (GrannyControlIsComplete(m_pgrnCtrl))
        return false;

    return true;
}

void CGrannyModelInstance::SetMotionPointer(const CGrannyMotion *pMotion, float blendTime, int loopCount,
                                            float speedRatio)
{
    // TODO(tim): Remove this
    if (!m_worldPose || !m_modelInstance)
        return;

  	granny_model_instance * pgrnModelInstance = m_modelInstance;
	if (!pgrnModelInstance)
		return;

	float localTime = GetLocalTime();

	bool isFirst=false;
	if (m_pgrnCtrl)
	{
		//float durationLeft = GrannyGetControlDurationLeft(m_pgrnCtrl);
		//float easeOutTime = (blendTime < durationLeft) ? blendTime : durationLeft;
		//float oldCtrlFinishTime = GrannyEaseControlOut(m_pgrnCtrl, blendTime); //easeOutTime);
		GrannySetControlEaseOutCurve(m_pgrnCtrl, localTime, localTime + blendTime, 1.0f, 1.0f, 0.0f, 0.0f);

		GrannySetControlEaseIn(m_pgrnCtrl, false);
		GrannySetControlEaseOut(m_pgrnCtrl, true);

		//Tracef("easeOut %f\n", easeOutTime);
		GrannyCompleteControlAt(m_pgrnCtrl, localTime + blendTime);
		//GrannyCompleteControlAt(m_pgrnCtrl, oldCtrlFinishTime);
		//GrannyCompleteControlAt(m_pgrnCtrl, localTime);
		GrannyFreeControlIfComplete(m_pgrnCtrl);
	}
	else
	{
		isFirst=true;
	}

	m_pgrnCtrl = GrannyPlayControlledAnimation(localTime, pMotion->GetGrannyAnimationPointer(), pgrnModelInstance);
	if (!m_pgrnCtrl)
		return;

	GrannySetControlSpeed(m_pgrnCtrl, speedRatio);
	GrannySetControlLoopCount(m_pgrnCtrl, loopCount);

	if (isFirst)
	{
		GrannySetControlEaseIn(m_pgrnCtrl, false);
		GrannySetControlEaseOut(m_pgrnCtrl, false);
	}
	else
	{
		GrannySetControlEaseIn(m_pgrnCtrl, true);
		GrannySetControlEaseOut(m_pgrnCtrl, false);
		if (blendTime > 0.0f)
			GrannySetControlEaseInCurve(m_pgrnCtrl, localTime, localTime + blendTime, 0.0f, 0.0f, 1.0f, 1.0f);
	}

	//GrannyEaseControlIn(m_pgrnCtrl, blendTime, false);
	GrannyFreeControlOnceUnused(m_pgrnCtrl);
	//Tracef("easeIn %f\n", blendTime);
}

// TODO(tim): Remove this
void CGrannyModelInstance::ChangeMotionPointer(const CGrannyMotion *pMotion, int loopCount, float speedRatio)
{
  	granny_model_instance * pgrnModelInstance = m_modelInstance;
	if (!pgrnModelInstance)
		return;

	// 보간 되는 앞부분을 스킵 하기 위해 LocalTime 을 어느 정도 무시한다. - [levites]
	float fSkipTime = 0.3f;
	float localTime = GetLocalTime() - fSkipTime;

	if (m_pgrnCtrl)
	{
		GrannySetControlEaseIn(m_pgrnCtrl, false);
		GrannySetControlEaseOut(m_pgrnCtrl, false);
		GrannyCompleteControlAt(m_pgrnCtrl, localTime);
		GrannyFreeControlIfComplete(m_pgrnCtrl);
	}

	m_pgrnCtrl = GrannyPlayControlledAnimation(localTime, pMotion->GetGrannyAnimationPointer(), pgrnModelInstance);
	if (!m_pgrnCtrl)
		return;

	GrannySetControlSpeed(m_pgrnCtrl, speedRatio);
	GrannySetControlLoopCount(m_pgrnCtrl, loopCount);
	GrannySetControlEaseIn(m_pgrnCtrl, false);
	GrannySetControlEaseOut(m_pgrnCtrl, false);

	GrannyFreeControlOnceUnused(m_pgrnCtrl);
}

void CGrannyModelInstance::SetMotionAtEnd()
{
    if (!m_pgrnCtrl)
        return;

    float endingTime = GrannyGetControlLocalDuration(m_pgrnCtrl);
    GrannySetControlRawLocalClock(m_pgrnCtrl, endingTime);
}

uint32_t CGrannyModelInstance::GetLoopIndex()
{
    if (m_pgrnCtrl)
        return GrannyGetControlLoopIndex(m_pgrnCtrl);
    return 0;
}

void CGrannyModelInstance::PrintControls()
{
}
