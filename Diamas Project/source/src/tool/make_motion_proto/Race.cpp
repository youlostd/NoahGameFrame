#include "Race.hpp"

#include <base/GroupTextTree.hpp>

#include <game/MotionTypes.hpp>
#include <SpdLog.hpp>

Race::Race(uint32_t vnum)
	: m_vnum(vnum)
{
}

const std::vector<Motion>* Race::GetMotions(uint32_t key) const
{
	// This function retrieves all sub-motions.
	STORM_ASSERT(MakeMotionId(key).subIndex == 0, "No sub indexing allowed");

	const auto it = m_motions.find(key);
	if (it != m_motions.end())
		return &it->second;

	return nullptr;
}

const Motion* Race::GetMotion(uint32_t key) const
{
	const auto id = MakeMotionId(key);

	const auto& mot = GetMotions(MakeMotionKey(id.mode, id.index));
	if (mot && id.subIndex < mot->size())
		return &(*mot)[id.subIndex];

	return nullptr;
}

bool Race::LoadModelScript(const storm::StringRef& filename)
{
	GroupTextMemory mem(storm::GetDefaultAllocator());
	GroupTextReader reader(&mem);

	if (!reader.LoadFile(filename))
		return false;

	m_modelPath = reader.GetProperty("BaseModelFileName");
	if (m_modelPath.empty()) {
		SPDLOG_ERROR("No BaseModelFileName set");
		return false;
	}

	return true;
}

void Race::AddMotion(uint32_t key, const Motion& motion)
{
	STORM_ASSERT(MakeMotionId(key).subIndex == 0, "No sub indexing allowed");
	m_motions[key].push_back(motion);
}


