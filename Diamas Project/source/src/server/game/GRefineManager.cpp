#include "GRefineManager.h"

#include "base/ClientDb.hpp"

CRefineManager::CRefineManager()
{
}

CRefineManager::~CRefineManager()
{
}

bool CRefineManager::Initialize()
{
    m_map_RefineRecipe.clear();

    std::vector<RefineProto> v;
    if (!LoadClientDbFromFile("data/refine_proto", v))
        return false;

    for (auto proto : v) { m_map_RefineRecipe.emplace(proto.id, proto); }

    SPDLOG_INFO("REFINE: COUNT {0}", m_map_RefineRecipe.size());
    return true;
}

const RefineProto *CRefineManager::GetRefineRecipe(uint32_t vnum)
{
    if (vnum == 0)
        return nullptr;

    auto it = m_map_RefineRecipe.find(vnum);
    SPDLOG_INFO("REFINE: FIND {0} {1}", vnum, it == m_map_RefineRecipe.end() ? "false" : "true");

    if (it == m_map_RefineRecipe.end()) { return nullptr; }

    return &it->second;
}
