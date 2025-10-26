#include "motion.h"

#include <base/ClientDb.hpp>

#include <storm/math/AngleUtil.hpp>
#include <cfloat>
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>
#include <Eigen/Geometry>

namespace
{
union CombinedMotionKey
{
    struct
    {
        uint32_t vnum;
        uint32_t motionKey;
    };

    uint64_t key;
};
}

Eigen::Vector3f GetRotatedAccumulation(const Motion *motion, float yaw)
{
    Eigen::AngleAxisf yawAxis(storm::RadiansFromDegrees(yaw),
                              Eigen::Vector3f::UnitZ());
    Eigen::Vector3f a = yawAxis * motion->accumulation;
    // motion->accumulation has an inverted y
    a.y() = -a.y();
    return a;
}

bool MotionManager::Initialize()
{
    std::vector<MotionProto> entries;
    if (!LoadClientDbFromFile("data/server_motion_proto", entries))
    {
        SPDLOG_CRITICAL("Could not load server_motion_proto");
        return false;
    }

    Eigen::AngleAxisf yaw(0.0f, Eigen::Vector3f::UnitZ());

    for (const auto &entry : entries)
    {
        //
        // Only combo/attack motions use DirectInputTime
        // (We could also simply check the motion key here, but
        // this code is shorter.)
        //

        CombinedMotionKey combined;
        combined.vnum = entry.race;
        combined.motionKey = entry.key;

        Motion motion;
        motion.key = combined.key;

        if (entry.duration != 0.0f)
            motion.moveSpeed = -entry.accumulation[1] / entry.duration;
        else
            motion.moveSpeed = 0.0f;

        motion.duration = static_cast<uint32_t>(entry.duration * 1000.0f);
        motion.inputTime = static_cast<uint32_t>(entry.directInputTime * 1000.0f);

        motion.accumulation.x() = entry.accumulation[0];
        motion.accumulation.y() = entry.accumulation[1];
        motion.accumulation.z() = entry.accumulation[2];

        if (entry.duration != 0.0f)
            motion.moveSpeed = (yaw * motion.accumulation).norm() / entry.duration;
        else
            motion.moveSpeed = 0.0f;

        for (uint32_t i = 0; i < MOTION_EVENT_MAX_NUM; ++i)
        {
            const auto &event = entry.events[i];
            if (event.startTime == 0.0f)
                continue;

            MotionEvent ev;
            ev.startTime = static_cast<uint32_t>(event.startTime * 1000.0f);
            ev.position.x() = event.position[0];
            ev.position.y() = event.position[1];
            ev.position.z() = event.position[2];

            motion.events.push_back(ev);
        }

        m_motions.emplace(combined.key, motion);
    }

    return true;
}

const Motion *MotionManager::Get(uint32_t vnum, uint32_t key) const
{
    CombinedMotionKey combined;
    combined.vnum = vnum;
    combined.motionKey = key;

    const auto it = m_motions.find(combined.key);
    if (m_motions.end() != it)
        return &it->second;

    return nullptr;
}
