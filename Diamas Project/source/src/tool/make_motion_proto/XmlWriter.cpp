#include "XmlWriter.hpp"

#include <game/MotionTypes.hpp>
#include <game/ProtoXml.hpp>

#include <xml/Types.hpp>


namespace
{

void FormatRace(const Race& race, std::vector<MotionProto>& items)
{
	for (const auto& p : race.GetMotionMap()) {
		const auto id = MakeMotionId(p.first);

		uint32_t subIndex = 0;
		for (const auto& motion : p.second) {
			MotionProto entry = {};

			entry.race = race.GetVnum();
			entry.key = MakeMotionKey(id.mode, id.index, subIndex++);
			entry.duration = motion.GetDuration();

			const auto& accum = motion.GetAccumulation();
			entry.accumulation[0] = accum.x();
			entry.accumulation[1] = accum.y();
			entry.accumulation[2] = accum.z();

			const auto& combo = motion.GetComboData();
			entry.preInputTime = combo.preInputTime;
			entry.directInputTime = combo.directInputTime;
			entry.inputLimitTime = combo.inputLimitTime;

			const auto& events = motion.GetEvents();
			if (events.size() > MOTION_EVENT_MAX_NUM) {
				SPDLOG_ERROR("Too many motion events for {0}",
				          motion.GetFilename());
				return;
			}

			uint32_t i = 0;
			for (const MotionEvent& ev : events) {
				entry.events[i].startTime = ev.startTime;
				entry.events[i].position[0] = ev.position.x();
				entry.events[i].position[1] = ev.position.y();
				entry.events[i].position[2] = ev.position.z();
				++i;
			}

			items.push_back(entry);
		}
	}
}

}

bool WriteMotionProto(const storm::StringRef& filename,
                      const std::list<Race>& races)
{
	std::vector<MotionProto> items;

	for (const auto& race : races)
		FormatRace(race, items);

	return WriteProtoXml(filename, items, "motion-proto");
}

