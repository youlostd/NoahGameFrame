#include "base/Crc32.hpp"

#include <game/WarpManager.hpp>

#include <base/GroupTextTree.hpp>
#include <base/GroupTextTreeUtil.hpp>

void WarpManager::LoadList(const GroupTextGroup *grp, const GroupTextList *list)
{
    std::vector<storm::StringRef> tokens;

    for (const auto &line : list->GetLines())
    {
        tokens.clear();
        storm::ExtractArguments(line, tokens);

        if (tokens.size() < 3)
            continue;

        WarpInfo wi{};
        storm::ParseNumber(tokens[0], wi.mapIndex);
        storm::ParseNumber(tokens[1], wi.x);
        storm::ParseNumber(tokens[2], wi.y);

        if (tokens.size() >= 5)
        {
            storm::ParseNumber(tokens[3], wi.minLevel);
            storm::ParseNumber(tokens[4], wi.maxLevel);
        }

        m_allWarps.emplace(ComputeCrc32(0, &wi, sizeof(wi)), wi);
    
    }
}

bool WarpManager::LoadWarpTable(const std::string &filename)
{
    GroupTextMemory mem(storm::GetDefaultAllocator());
    GroupTextReader reader(&mem);

    if (!reader.LoadFile(filename))
    {
        spdlog::error("Failed to load warpinfo");
        return false;
    }

    for (const auto &p : reader.GetChildren())
    {

        const auto *node = p.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        const auto *grp = static_cast<const GroupTextGroup *>(node);

        std::string name;
        GetGroupProperty(grp, "Name", name);

        uint32_t id;
        GetGroupProperty(grp, "Id", id);

        for (const auto &q : grp->GetChildren())
        {
            const auto *lnode = q.second;
            if (lnode->GetType() != GroupTextNode::kList)
                continue;

            const auto *list = static_cast<const GroupTextList *>(lnode);
            LoadList(grp, list);
        }
    }

    return false;
}
