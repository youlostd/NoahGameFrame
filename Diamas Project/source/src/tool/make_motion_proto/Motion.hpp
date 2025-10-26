#ifndef METIN2_TOOL_MAKEMOTIONPROTO_MOTION_HPP
#define METIN2_TOOL_MAKEMOTIONPROTO_MOTION_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/String.hpp>

#include <Eigen/Core>

#include <vector>



struct ComboInputData
{
	ComboInputData();

	float preInputTime;
	float directInputTime;
	float inputLimitTime;
};

struct MotionEvent
{
	float startTime;
	Eigen::Vector3f position;
};

class Motion
{
	public:
		Motion();

		bool LoadFile(const storm::StringRef& filename);

		const storm::String& GetFilename() const;

		float GetDuration() const;
		void SetDuration(float duration);

		const Eigen::Vector3f& GetAccumulation() const;
		void SetAccumulation(const Eigen::Vector3f& v);

		const ComboInputData& GetComboData() const;
		const std::vector<MotionEvent>& GetEvents() const;

	private:
		storm::String m_filename;
		float m_duration;
		Eigen::Vector3f m_accumulation;
		ComboInputData m_comboData;
		std::vector<MotionEvent> m_events;
};

BOOST_FORCEINLINE const storm::String& Motion::GetFilename() const
{ return m_filename; }

BOOST_FORCEINLINE float Motion::GetDuration() const
{ return m_duration; }

BOOST_FORCEINLINE void Motion::SetDuration(float duration)
{ m_duration = duration; }

BOOST_FORCEINLINE const Eigen::Vector3f& Motion::GetAccumulation() const
{ return m_accumulation; }

BOOST_FORCEINLINE void Motion::SetAccumulation(const Eigen::Vector3f& v)
{ m_accumulation = v; }

BOOST_FORCEINLINE const ComboInputData& Motion::GetComboData() const
{ return m_comboData; }

BOOST_FORCEINLINE const std::vector<MotionEvent>& Motion::GetEvents() const
{ return m_events; }



#endif // METIN2_TOOL_MAKEMOTIONPROTO_MOTION_HPP
