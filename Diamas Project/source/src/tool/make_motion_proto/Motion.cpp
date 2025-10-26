#include "Motion.hpp"

#include <base/GroupTextTreeUtil.hpp>
#include <base/GroupTextTree.hpp>
#include <SpdLog.hpp>


ComboInputData::ComboInputData()
	: preInputTime(0.0f)
	, directInputTime(0.0f)
	, inputLimitTime(0.0f)
{
	// ctor
}

Motion::Motion()
	: m_duration(0.0f)
	, m_accumulation(0.0f, 0.0f, 0.0f)
{
	// ctor
}

bool Motion::LoadFile(const storm::StringRef& filename)
{
	GroupTextMemory mem(storm::GetDefaultAllocator());
	GroupTextReader reader(&mem);

	if (!reader.LoadFile(filename))
		return false;

	if (!GetGroupProperty(&reader, "MotionDuration", m_duration)) {
		SPDLOG_ERROR("Failed to parse motion duration");
		return false;
	}

	m_filename = reader.GetProperty("MotionFileName");
	if (m_filename.empty()) {
		SPDLOG_ERROR("No MotionFileName set");
		return false;
	}

	// Sadly not a required property...
	GetGroupProperty(&reader, "Accumulation", m_accumulation);

	// This is necessary for mob skills
	const auto evdata = reader.GetGroup("MotionEventData");
	if (evdata) {
		auto f = [this] (const std::pair<storm::String, GroupTextNode*>& p) {
			if (p.second->GetType() != GroupTextNode::kGroup)
				return;

			auto node = static_cast<GroupTextGroup*>(p.second);

			MotionEvent ev;

			if (!GetGroupProperty(node, "StartingTime", ev.startTime))
				// Ignore events without start time
				return;

			auto sphere = node->GetGroup("SphereData00");
			if (!sphere)
				// Ignore events without sphere-data
				return;

			if (!GetGroupProperty(sphere, "Position", ev.position)) {
				SPDLOG_ERROR("SphereData has no position");
				return;
			}

			m_events.push_back(ev);
		};

		const auto& events = evdata->GetChildren();
		std::for_each(events.begin(), events.end(), f);
	}

	// PC combo data
	const auto combo = reader.GetGroup("ComboInputData");
	if (combo) {
		GetGroupProperty(combo, "PreInputTime", m_comboData.preInputTime);
		GetGroupProperty(combo, "DirectInputTime", m_comboData.directInputTime);
		GetGroupProperty(combo, "InputLimitTime", m_comboData.inputLimitTime);
	}

	return true;
}


