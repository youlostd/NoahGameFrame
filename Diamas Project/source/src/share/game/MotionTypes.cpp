
#include <xml/Util.hpp>
#include <base/Exception.hpp>

#include <storm/StringUtil.hpp>
#include "MotionTypes.hpp"
#include <SpdLog.hpp>

bool ParseProto(MotionProto& entry, const XmlNode* node)
{
	std::string_view str;
	uint32_t val;

	//
	// Root node
	//

	GetAttribute(node, "race", entry.race);

	uint32_t mode;
	ParseStringTableEntry(node, "mode", &GetMotionModeValue, mode);

	uint32_t index;
	ParseStringTableEntry(node, "index", &GetMotionValue, index);

	uint8_t subIndex;
	GetAttribute(node, "sub-index", subIndex);

	entry.key = MakeMotionKey(mode, index, subIndex);

	GetAttribute(node, "duration", entry.duration);

	//
	// <accumulation> node
	//

	auto accum = node->first_node("accumulation");
	if (accum) {
		GetAttribute(accum, "x", entry.accumulation[0]);
		GetAttribute(accum, "y", entry.accumulation[1]);
		GetAttribute(accum, "z", entry.accumulation[2]);
	}


	//
	// <combo> node
	//

	auto combo = node->first_node("combo");
	if (combo) {
		GetAttribute(combo, "pre-input-time",
		             entry.preInputTime);
		GetAttribute(combo, "direct-input-time",
		             entry.directInputTime);
		GetAttribute(combo, "input-limit-time",
		             entry.inputLimitTime);
	}

	//
	// <events> node
	//

	auto events = node->first_node("events");
	if (events) {
		int i = 0;
		for (auto e = events->first_node(); e; e = e->next_sibling()) {
			METIN2_XML_PARSE_ASSERT(e, i < MOTION_EVENT_MAX_NUM);
			auto& event = entry.events[i++];

			GetAttribute(e, "start-time", event.startTime);
			TryGetAttribute(e, "x", event.position[0]);
			TryGetAttribute(e, "y", event.position[1]);
			TryGetAttribute(e, "z", event.position[2]);
		}
	}


	return true;
}

static bool HasEvents(const MotionProto& entry)
{
	for (int i = 0; i < MOTION_EVENT_MAX_NUM; ++i) {
		if (entry.events[i].startTime != 0.0f)
			return true;
	}

	return false;
}

XmlNode* FormatProto(const MotionProto& entry, XmlMemoryPool* pool)
{
auto node = pool->allocate_node(rapidxml::node_element, "motion");

	storm::StringRef strref;

	//
	// Root node
	//

	AddAttribute(pool, node, "race", entry.race);

	auto id = MakeMotionId(entry.key);

	ThrowErrorIfNot(GetMotionModeString(strref, id.mode),
	                "Failed to format motion mode: {0}",
	                id.mode);
	AddAttribute(pool, node, "mode", strref);

	ThrowErrorIfNot(GetMotionString(strref, id.index),
	                "Failed to format motion index: {0}",
	                id.index);
	AddAttribute(pool, node, "index", strref);

	AddAttribute(pool, node, "sub-index", id.subIndex);
	AddAttribute(pool, node, "duration", entry.duration);

	//
	// <accumulation> node
	//

	const bool needsAccumulation = entry.accumulation[0] != 0.0f ||
	                               entry.accumulation[1] != 0.0f ||
	                               entry.accumulation[2] != 0.0f;

	if (needsAccumulation) {
		auto accum = pool->allocate_node(rapidxml::node_element, "accumulation");

		AddAttribute(pool, accum, "x", entry.accumulation[0]);
		AddAttribute(pool, accum, "y", entry.accumulation[1]);
		AddAttribute(pool, accum, "z", entry.accumulation[2]);

		node->append_node(accum);
	}

	//
	// <combo> node
	//

	if (entry.preInputTime != 0.0f ||
	    entry.directInputTime != 0.0f ||
	    entry.inputLimitTime != 0.0f) {
		auto combo = pool->allocate_node(rapidxml::node_element, "combo");

		if (entry.preInputTime != 0.0f)
			AddAttribute(pool, combo, "pre-input-time",
			             entry.preInputTime);

		if (entry.directInputTime != 0.0f)
			AddAttribute(pool, combo, "direct-input-time",
			             entry.directInputTime);

		if (entry.inputLimitTime != 0.0f)
			AddAttribute(pool, combo, "input-limit-time",
			             entry.inputLimitTime);

		node->append_node(combo);
	}

	//
	// <events> node
	//

	if (HasEvents(entry)) {
		auto events = pool->allocate_node(rapidxml::node_element, "events");

		for (uint32_t i = 0; i < MOTION_EVENT_MAX_NUM; ++i) {
			const auto& ev = entry.events[i];
			if (ev.startTime == 0.0f)
				continue;

			auto event = pool->allocate_node(rapidxml::node_element, "event");

			AddAttribute(pool, event, "start-time", ev.startTime);

			if (ev.position[0] != 0.0f)
				AddAttribute(pool, event, "x", ev.position[0]);

			if (ev.position[1] != 0.0f)
				AddAttribute(pool, event, "y", ev.position[1]);

			if (ev.position[2] != 0.0f)
				AddAttribute(pool, event, "z", ev.position[2]);

			events->append_node(event);
		}

		node->append_node(events);
	}

	return node;
}
