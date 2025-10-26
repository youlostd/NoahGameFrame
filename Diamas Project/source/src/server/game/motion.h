#ifndef METIN2_SERVER_GAME_MOTION_H
#define METIN2_SERVER_GAME_MOTION_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <Eigen/Core>

#include <unordered_map>
#include <string>

struct MotionEvent
{
    // in ms
    uint32_t startTime;

    // in cm
    Eigen::Vector3f position;
};

struct Motion
{
    uint64_t key;

    // in ms
    uint32_t duration;
    uint32_t inputTime;

    // cached: -accumulation.y / duration
    float moveSpeed;

    // in cm
    Eigen::Vector3f accumulation;

    // Used for mob-skill hit calculation
    std::vector<MotionEvent> events;
};

Eigen::Vector3f GetRotatedAccumulation(const Motion *motion, float yaw);

class MotionManager
{
public:
    bool Initialize();

    const Motion *Get(uint32_t vnum, uint32_t key) const;

private:
    typedef std::unordered_map<uint64_t, Motion> Map;

    Map m_motions;
};

#endif /* METIN2_SERVER_GAME_MOTION_H */
