#include "skill.h"

#include <game/SkillTypes.hpp>
#include <game/AffectConstants.hpp>

#include <base/ClientDb.hpp>

void CSkillProto::SetPointVar(const std::string &strName, double dVar)
{
    kPointPoly.SetVar(strName, dVar);
    kPointPoly2.SetVar(strName, dVar);
    kPointPoly3.SetVar(strName, dVar);
    kMasterBonusPoly.SetVar(strName, dVar);
}

void CSkillProto::SetDurationVar(const std::string &strName, double dVar)
{
    kDurationPoly.SetVar(strName, dVar);
    kDurationPoly2.SetVar(strName, dVar);
    kDurationPoly3.SetVar(strName, dVar);
}

void CSkillProto::SetSPCostVar(const std::string &strName, double dVar)
{
    kSPCostPoly.SetVar(strName, dVar);
    kGrandMasterAddSPCostPoly.SetVar(strName, dVar);
}

CSkillManager::CSkillManager()
{
}

CSkillManager::~CSkillManager()
{
}

bool CSkillManager::Initialize()
{
   m_protoMap.clear();

    std::vector<SkillProto> elements;
    if (!LoadClientDbFromFile("data/skill_proto_server", elements))
    {
        SPDLOG_ERROR("Failed to load skill proto");
        return false;
    }

    for (const auto &entry : elements)
    {
        auto proto = std::make_unique<CSkillProto>();

        proto->dwVnum = entry.vnum;
        proto->name = entry.name;
        proto->dwType = entry.type;
        proto->bSkillAttrType = entry.attrType;
        proto->dwFlag = entry.flags;
        if (entry.maxLevel != 0)
            proto->bMaxLevel = entry.maxLevel;
        else
            proto->bMaxLevel = 1;
        proto->bLevelLimit = entry.levelLimit;

        proto->requirements = entry.requirements;

        if (!proto->levelStep.Analyze(entry.levelStep.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse level-step poly {0}",
                entry.spCost);
            continue;
        }

        if (!proto->kSPCostPoly.Analyze(entry.spCost.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse sp-cost poly {0}",
                entry.spCost);
            continue;
        }

        if (!proto->kGrandMasterAddSPCostPoly.Analyze(entry.grandMasterSpCost.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse sp-cost poly {0}",
                entry.grandMasterSpCost);
            continue;
        }

        proto->dwTargetRange = entry.targetRange;
        proto->lMaxHit = entry.maxHitCount;
        proto->iSplashRange = entry.splashRange;

        if (!proto->kSplashAroundDamageAdjustPoly.Analyze(entry.splashDamage.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse splash-damage poly {0}",
                entry.splashDamage);
            continue;
        }

        proto->bPointOn = entry.point;
        proto->bPointOn2 = entry.point2;
        proto->bPointOn3 = entry.point3;

        proto->dwAffectFlag = entry.affectFlag;
        proto->dwAffectFlag2 = entry.affectFlag2;

        if (!proto->kPointPoly.Analyze(entry.value.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse point poly {0}",
                entry.value);
            continue;
        }

        if (!proto->kPointPoly2.Analyze(entry.value2.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse point poly {0}",
                entry.value2);
            continue;
        }

        if (!proto->kPointPoly3.Analyze(entry.value3.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse point poly {0}",
                entry.value3);
            continue;
        }

        if (!proto->kDurationPoly.Analyze(entry.duration.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse duration poly {0}",
                entry.duration);
            continue;
        }

        if (!proto->kDurationPoly2.Analyze(entry.duration2.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse duration poly {0}",
                entry.duration2);
            continue;
        }

        if (!proto->kDurationPoly3.Analyze(entry.duration3.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse duration poly {0}",
                entry.duration3);
            continue;
        }

        if (!proto->kDurationSPCostPoly.Analyze(entry.durationSpCost.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse duration-sp poly {0}",
                entry.durationSpCost);
            continue;
        }

        if (!proto->kCooldownPoly.Analyze(entry.cooltime.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse cooldown poly {0}",
                entry.cooltime);
            continue;
        }

        if (!proto->kMasterBonusPoly.Analyze(entry.masterValue.c_str()))
        {
            SPDLOG_ERROR(
                "Failed to parse master point poly {0}",
                entry.masterValue);
            continue;
        }

        m_protoMap.emplace(entry.vnum, std::move(proto));
    }
    return true;
}

CSkillProto *CSkillManager::Get(uint32_t vnum)
{
    const auto it = m_protoMap.find(vnum);
    if (it != m_protoMap.end())
        return it->second.get();

    return nullptr;
}

CSkillProto *CSkillManager::Get(const storm::StringRef &name)
{
    auto f = [&name](const ProtoMap::value_type &p) -> bool { return p.second->name == name; };

    const auto it = std::find_if(m_protoMap.begin(), m_protoMap.end(), f);
    if (it != m_protoMap.end())
        return it->second.get();

    return nullptr;
}
