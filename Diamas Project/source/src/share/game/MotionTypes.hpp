#ifndef METIN2_GAME_MOTIONTYPES_HPP
#define METIN2_GAME_MOTIONTYPES_HPP


#pragma once


#include "MotionConstants.hpp"

#include <xml/Types.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <vector>
#include <cstdint>



#pragma pack(push, 1)

union MotionId
{
	struct {
		uint8_t mode;
		uint16_t index;
		uint8_t subIndex;
	};

	uint32_t key;
};

struct MotionEventProto
{
	float startTime;
	float position[3];
};

struct MotionProto
{
	static const uint32_t kVersion = 1;

	uint32_t race;
	uint32_t key;

	float duration;

	float accumulation[3];

	// Currently only used in combo-motions
	float preInputTime;
	float directInputTime;
	float inputLimitTime;

	MotionEventProto events[MOTION_EVENT_MAX_NUM];
};

#pragma pack(pop)

BOOST_FORCEINLINE MotionId MakeMotionId(uint8_t mode,
                                        uint16_t index,
                                        uint8_t subIndex = 0)
{
	MotionId id;
	id.mode = mode;
	id.index = index;
	id.subIndex = subIndex;
	return id;
}

BOOST_FORCEINLINE MotionId MakeMotionId(uint32_t key)
{
	MotionId id;
	id.key = key;
	return id;
}

BOOST_FORCEINLINE uint32_t MakeMotionKey(uint8_t mode,
                                         uint16_t index,
                                         uint8_t subIndex = 0)
{
	MotionId id;
	id.mode = mode;
	id.index = index;
	id.subIndex = subIndex;
	return id.key;
}

bool ParseProto(MotionProto& entry, const XmlNode* node);
XmlNode* FormatProto(const MotionProto& entry, XmlMemoryPool* pool);



BOOST_FUSION_ADAPT_STRUCT(
	MotionEventProto,
	startTime,
	position
);

BOOST_FUSION_ADAPT_STRUCT(
	MotionProto,
	race,
	key,
	duration,
	accumulation,
	preInputTime,
	directInputTime,
	inputLimitTime,
	events
);

#endif // METIN2_GAME_MOTIONTYPES_HPP
