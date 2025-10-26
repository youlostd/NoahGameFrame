#include "StdAfx.h"
#include "GameType.h"
#include "../effectLib/EffectManager.h"
#include <base/GroupText.hpp>
#include <base/GroupTextTreeUtil.hpp>

extern float g_fGameFPS = 60.0f;

CDynamicPool<NRaceData::TCollisionData> NRaceData::g_CollisionDataPool;
CDynamicPool<NRaceData::TAttachingEffectData> NRaceData::g_EffectDataPool;
CDynamicPool<NRaceData::TAttachingObjectData> NRaceData::g_ObjectDataPool;

void NRaceData::DestroySystem()
{
    g_CollisionDataPool.Destroy();
    g_EffectDataPool.Destroy();
    g_ObjectDataPool.Destroy();
}

/////////////////////////////////////////////////////////////////////////////////
// Character Attaching Collision Data
bool NRaceData::LoadAttackData(CTextFileLoader &rTextFileLoader, TAttackData *pData)
{
    if (!rTextFileLoader.GetTokenInteger("attacktype", &pData->iAttackType))
    {
        pData->iAttackType = ATTACK_TYPE_SPLASH;
    }
    if (!rTextFileLoader.GetTokenInteger("hittingtype", &pData->iHittingType))
        return false;
    if (!rTextFileLoader.GetTokenFloat("stiffentime", &pData->fStiffenTime))
        return false;
    if (!rTextFileLoader.GetTokenFloat("invisibletime", &pData->fInvisibleTime))
        return false;
    if (!rTextFileLoader.GetTokenFloat("externalforce", &pData->fExternalForce))
        return false;
    if (!rTextFileLoader.GetTokenInteger("hitlimitcount", &pData->iHitLimitCount))
    {
        pData->iHitLimitCount = 0;
    }

    return true;
}

bool NRaceData::LoadAttackData(const GroupTextGroup *group, TAttackData *pData)
{
    if (!GetGroupProperty(group, "AttackType", pData->iAttackType))
        pData->iAttackType = ATTACK_TYPE_SPLASH;

    if (!GetGroupProperty(group, "HitLimitCount", pData->iHitLimitCount))
        pData->iHitLimitCount = 0;

    if (!GetGroupProperty(group, "HittingType", pData->iHittingType))
    {
        SPDLOG_ERROR("Missing property HittingType in {}", group->GetName());
        return false;
    }

    if (!GetGroupProperty(group, "StiffenTime", pData->fStiffenTime))
    {
        SPDLOG_ERROR("Missing property StiffenTime in {}", group->GetName());
        return false;
    }

    if (!GetGroupProperty(group, "InvisibleTime", pData->fInvisibleTime))
    {
        SPDLOG_ERROR("Missing property InvisibleTime in {}", group->GetName());
        return false;
    }

    if (!GetGroupProperty(group, "ExternalForce", pData->fExternalForce))
    {
        SPDLOG_ERROR("Missing property ExternalForce in {}", group->GetName());
        return false;
    }
    return true;
}

bool NRaceData::THitData::Load(const GroupTextGroup *group)
{
    if (!GetGroupProperty(group, "AttackingStartTime", fAttackStartTime))
    {
        SPDLOG_ERROR("Missing property AttackingStartTime in {}", group->GetName());
        return false;
    }

    if (!GetGroupProperty(group, "AttackingEndTime", fAttackEndTime))
    {
        SPDLOG_ERROR("Missing property AttackingEndTime in {}", group->GetName());
        return false;
    }

    if (!GetGroupProperty(group, "AttackingBone", strBoneName))
        strBoneName = "";

    if (!GetGroupProperty(group, "WeaponLength", fWeaponLength))
        fWeaponLength = 0.0f;

    mapHitPosition.clear();

    const auto &list = group->GetList("HitPosition");
    if (list)
    {
        for (const auto &line : list->GetLines())
        {
            std::vector<std::string> tokens = {};

            boost::split(tokens, line, boost::is_any_of(" "));

            if (tokens.empty())
                continue;

            if (tokens.size() < 7)
                continue;

            float time{};
            NRaceData::THitTimePosition hp{};
            storm::ParseNumber(tokens[0], time);

            storm::ParseNumber(tokens[1], hp.v3LastPosition.x);
            storm::ParseNumber(tokens[2], hp.v3LastPosition.y);
            storm::ParseNumber(tokens[3], hp.v3LastPosition.z);
            storm::ParseNumber(tokens[4], hp.v3Position.x);
            storm::ParseNumber(tokens[5], hp.v3Position.y);
            storm::ParseNumber(tokens[6], hp.v3Position.z);

            mapHitPosition[time] = hp;
        }
    }

    return true;
}

bool NRaceData::THitData::Load(CTextFileLoader &rTextFileLoader)
{
    if (!rTextFileLoader.GetTokenFloat("attackingstarttime", &fAttackStartTime))
        return false;
    if (!rTextFileLoader.GetTokenFloat("attackingendtime", &fAttackEndTime))
        return false;

    if (!rTextFileLoader.GetTokenString("attackingbone", &strBoneName))
        strBoneName = "";
    if (!rTextFileLoader.GetTokenFloat("weaponlength", &fWeaponLength))
        fWeaponLength = 0.0f;

    mapHitPosition.clear();

    CTokenVector *tv;
    if (rTextFileLoader.GetTokenVector("hitposition", &tv))
    {
        CTokenVector::iterator it = tv->begin();

        while (it != tv->end())
        {
            float time;
            NRaceData::THitTimePosition hp;
            storm::ParseNumber(*it++, time);
            storm::ParseNumber(*it++, hp.v3LastPosition.x);
            storm::ParseNumber(*it++, hp.v3LastPosition.y);
            storm::ParseNumber(*it++, hp.v3LastPosition.z);
            storm::ParseNumber(*it++, hp.v3Position.x);
            storm::ParseNumber(*it++, hp.v3Position.y);
            storm::ParseNumber(*it++, hp.v3Position.z);

            mapHitPosition[time] = hp;
        }
    }

    return true;
}

bool NRaceData::LoadMotionAttackData(const GroupTextGroup *group, TMotionAttackData *pData)
{
    if (!LoadAttackData(group, pData))
        return FALSE;

    if (!GetGroupProperty(group, "MotionType", pData->iMotionType))
    {
        if (!GetGroupProperty(group, "AttackingType", pData->iMotionType))
        {
            return false;
        }
    }

    pData->HitDataContainer.clear();

    uint32_t dwHitDataCount;
    if (!GetGroupProperty(group, "HitDataCount", dwHitDataCount))
    {
        pData->HitDataContainer.emplace_back(SHitData());
        auto &rHitData = *(pData->HitDataContainer.rbegin());
        if (!rHitData.Load(group))
        {
            SPDLOG_ERROR("Failed to load hit data in {}", group->GetName());
            return false;
        }
    }
    else
    {
        for (auto &child : group->GetChildren())
        {
            const auto node = child.second;
            if (node->GetType() != GroupTextNode::kGroup)
                continue;

            auto grp = static_cast<GroupTextGroup *>(node);
            pData->HitDataContainer.push_back(SHitData());
            auto &rHitData = *(pData->HitDataContainer.rbegin());
            if (!rHitData.Load(grp))
            {
                SPDLOG_ERROR("Failed to load hit data in {}", group->GetName());
                return false;
            }
        }
    }

    return TRUE;
}

bool NRaceData::LoadMotionAttackData(CTextFileLoader &rTextFileLoader, TMotionAttackData *pData)
{
    if (!LoadAttackData(rTextFileLoader, pData))
        return FALSE;

    // NOTE : ������ AttackingType�� MotionType���� �ٲ����
    //        ���� ����Ÿ�� ��� ������ ���� ������ �аԲ�..
    if (!rTextFileLoader.GetTokenInteger("motiontype", &pData->iMotionType))
    {
        if (!rTextFileLoader.GetTokenInteger("attackingtype", &pData->iMotionType))
            return FALSE;
    }

    pData->HitDataContainer.clear();

    uint32_t dwHitDataCount;
    if (!rTextFileLoader.GetTokenDoubleWord("hitdatacount", &dwHitDataCount))
    {
        pData->HitDataContainer.push_back(SHitData());
        THitData &rHitData = *(pData->HitDataContainer.rbegin());
        if (!rHitData.Load(rTextFileLoader))
            return FALSE;
    }
    else
    {
        for (uint32_t i = 0; i < dwHitDataCount; ++i)
        {
            if (!rTextFileLoader.SetChildNode(i))
                return FALSE;

            pData->HitDataContainer.push_back(SHitData());
            THitData &rHitData = *(pData->HitDataContainer.rbegin());
            if (!rHitData.Load(rTextFileLoader))
                return FALSE;

            rTextFileLoader.SetParentNode();
        }
    }

    return TRUE;
}

bool NRaceData::LoadCollisionData(const GroupTextGroup *group, TCollisionData *pCollisionData)
{
    if (!GetGroupProperty(group, "CollisionType", pCollisionData->iCollisionType))
        return false;

    uint32_t dwSphereDataCount;
    if (!GetGroupProperty(group, "SphereDataCount", dwSphereDataCount))
        return false;

    pCollisionData->SphereDataVector.clear();
    pCollisionData->SphereDataVector.resize(dwSphereDataCount);

    auto i = 0;
    for (auto &child : group->GetChildren())
    {
        const auto node = child.second;
        if (node->GetType() != GroupTextNode::kGroup)
            continue;

        auto grp = static_cast<GroupTextGroup *>(node);

        auto &r = pCollisionData->SphereDataVector[i].GetAttribute();

        if (!GetGroupProperty(grp, "Radius", r.fRadius))
            return false;

        if (!GetGroupProperty(grp, "Position", r.v3Position))
            return false;

        ++i;
    }

    return true;
}

bool NRaceData::LoadCollisionData(CTextFileLoader &rTextFileLoader, TCollisionData *pCollisionData)
{
    if (!rTextFileLoader.GetTokenInteger("collisiontype", &pCollisionData->iCollisionType))
        return false;

    uint32_t dwSphereDataCount;
    if (!rTextFileLoader.GetTokenDoubleWord("spheredatacount", &dwSphereDataCount))
        return false;

    pCollisionData->SphereDataVector.clear();
    pCollisionData->SphereDataVector.resize(dwSphereDataCount);
    for (uint32_t i = 0; i < dwSphereDataCount; ++i)
    {
        TSphereData &r = pCollisionData->SphereDataVector[i].GetAttribute();

        if (!rTextFileLoader.SetChildNode("spheredata", i))
            return false;

        if (!rTextFileLoader.GetTokenFloat("radius", &r.fRadius))
            return false;
        if (!rTextFileLoader.GetTokenPosition("position", &r.v3Position))
            return false;

        rTextFileLoader.SetParentNode();
    }

    return true;
}

void NRaceData::SaveEffectData(storm::File &File, int iTabCount, const TAttachingEffectData &c_rEffectData)
{
    PrintfTabs(File, iTabCount, "EffectScriptName     \"%s\"\n", c_rEffectData.strFileName.c_str());

    PrintfTabs(File, iTabCount, "EffectPosition       %f %f %f\n", c_rEffectData.v3Position.x,
               c_rEffectData.v3Position.y, c_rEffectData.v3Position.z);
    PrintfTabs(File, iTabCount, "EffectRotation       %f %f %f\n", c_rEffectData.v3Rotation.x,
               c_rEffectData.v3Rotation.y, c_rEffectData.v3Rotation.z);
}

void NRaceData::SaveObjectData(storm::File &File, int iTabCount, const TAttachingObjectData &c_rObjectData)
{
    PrintfTabs(File, iTabCount, "ObjectScriptName     \"%s\"\n", c_rObjectData.strFileName.c_str());
}

void NRaceData::SaveEffectData(FILE *File, int iTabCount, const TAttachingEffectData &c_rEffectData)
{
    PrintfTabs(File, iTabCount, "EffectScriptName     \"%s\"\n", c_rEffectData.strFileName.c_str());

    PrintfTabs(File, iTabCount, "EffectPosition       %f %f %f\n", c_rEffectData.v3Position.x,
               c_rEffectData.v3Position.y, c_rEffectData.v3Position.z);
    PrintfTabs(File, iTabCount, "EffectRotation       %f %f %f\n", c_rEffectData.v3Rotation.x,
               c_rEffectData.v3Rotation.y, c_rEffectData.v3Rotation.z);
}

void NRaceData::SaveObjectData(FILE *File, int iTabCount, const TAttachingObjectData &c_rObjectData)
{
    PrintfTabs(File, iTabCount, "ObjectScriptName     \"%s\"\n", c_rObjectData.strFileName.c_str());
}

bool NRaceData::LoadEffectData(CTextFileLoader &rTextFileLoader, TAttachingEffectData *pEffectData)
{
    if (!rTextFileLoader.GetTokenString("effectscriptname", &pEffectData->strFileName))
        return false;

    if (!rTextFileLoader.GetTokenPosition("effectposition", &pEffectData->v3Position))
    {
        pEffectData->v3Position = Vector3(0.0f, 0.0f, 0.0f);
    }

    if (!rTextFileLoader.GetTokenPosition("effectrotation", &pEffectData->v3Rotation))
    {
        //pEffectData->qRotation = Quaternion(0.0f,0.0f,0.0f,1.0f);
        pEffectData->v3Rotation = Vector3(0.0f, 0.0f, 0.0f);
    }

    /*if (!*/
    // TODO DELETEME FIXME
    StringPath(pEffectData->strFileName);
    CEffectManager::Instance().RegisterEffect(pEffectData->strFileName.c_str());
    /*) return false;*/

    return true;
}

bool NRaceData::LoadObjectData(CTextFileLoader &rTextFileLoader, TAttachingObjectData *pObjectData)
{
    if (!rTextFileLoader.GetTokenString("objectscriptname", &pObjectData->strFileName))
        return false;

    return true;
}

void NRaceData::SaveAttackData(storm::File &File, int iTabCount, const TAttackData &c_rData)
{
    PrintfTabs(File, iTabCount, "AttackType           %d\n", c_rData.iAttackType);
    PrintfTabs(File, iTabCount, "HittingType          %d\n", c_rData.iHittingType);
    PrintfTabs(File, iTabCount, "StiffenTime          %f\n", c_rData.fStiffenTime);
    PrintfTabs(File, iTabCount, "InvisibleTime        %f\n", c_rData.fInvisibleTime);
    PrintfTabs(File, iTabCount, "ExternalForce        %f\n", c_rData.fExternalForce);
    PrintfTabs(File, iTabCount, "HitLimitCount        %d\n", c_rData.iHitLimitCount);
}

void NRaceData::SaveMotionAttackData(storm::File &File, int iTabCount, const TMotionAttackData &c_rData)
{
    SaveAttackData(File, iTabCount, c_rData);

    PrintfTabs(File, iTabCount, "\n");
    PrintfTabs(File, iTabCount, "MotionType           %d\n", c_rData.iMotionType);
    PrintfTabs(File, iTabCount, "HitDataCount         %d\n", c_rData.HitDataContainer.size());

    uint32_t dwHitDataNumber = 0;
    THitDataContainer::const_iterator itor = c_rData.HitDataContainer.begin();
    for (; itor != c_rData.HitDataContainer.end(); ++itor, ++dwHitDataNumber)
    {
        const THitData &c_rHitData = *itor;

        PrintfTabs(File, iTabCount, "Group HitData%02d\n", dwHitDataNumber);
        PrintfTabs(File, iTabCount, "{\n");
        PrintfTabs(File, iTabCount + 1, "AttackingStartTime   %f\n", c_rHitData.fAttackStartTime);
        PrintfTabs(File, iTabCount + 1, "AttackingEndTime     %f\n", c_rHitData.fAttackEndTime);

        PrintfTabs(File, iTabCount + 1, "AttackingBone        \"%s\"\n", c_rHitData.strBoneName.c_str());
        PrintfTabs(File, iTabCount + 1, "WeaponLength         %f\n", c_rHitData.fWeaponLength);
        PrintfTabs(File, iTabCount + 1, "List HitPosition\n");
        PrintfTabs(File, iTabCount + 1, "{\n");

        NRaceData::THitTimePositionMap::const_iterator it;
        for (it = c_rHitData.mapHitPosition.begin(); it != c_rHitData.mapHitPosition.end(); ++it)
        {
            PrintfTabs(File, iTabCount + 2, "%f %f %f %f %f %f %f\n", it->first,
                       it->second.v3LastPosition.x, it->second.v3LastPosition.y, it->second.v3LastPosition.z,
                       it->second.v3Position.x, it->second.v3Position.y, it->second.v3Position.z);
        }
        PrintfTabs(File, iTabCount + 1, "}\n");
        PrintfTabs(File, iTabCount, "}\n");
    }
}

void NRaceData::SaveCollisionData(storm::File &File, int iTabCount, const TCollisionData &c_rCollisionData)
{
    PrintfTabs(File, iTabCount, "CollisionType        %d\n", c_rCollisionData.iCollisionType);
    PrintfTabs(File, iTabCount, "\n");

    const CSphereCollisionInstanceVector &c_rSphereDataVector = c_rCollisionData.SphereDataVector;

    PrintfTabs(File, iTabCount, "SphereDataCount      %d\n", c_rSphereDataVector.size());

    for (uint32_t i = 0; i < c_rSphereDataVector.size(); ++i)
    {
        const TSphereData &c_rAttr = c_rSphereDataVector[i].GetAttribute();

        PrintfTabs(File, iTabCount, "Group SphereData%02d\n", i);

        PrintfTabs(File, iTabCount, "{\n");

        PrintfTabs(File, iTabCount + 1, "Radius           %f\n",
                   c_rAttr.fRadius);

        PrintfTabs(File, iTabCount + 1, "Position         %f %f %f\n",
                   c_rAttr.v3Position.x,
                   c_rAttr.v3Position.y,
                   c_rAttr.v3Position.z);

        PrintfTabs(File, iTabCount, "}\n");
    }
}

void NRaceData::SaveAttackData(FILE *File, int iTabCount, const TAttackData &c_rData)
{
    PrintfTabs(File, iTabCount, "AttackType           %d\n", c_rData.iAttackType);
    PrintfTabs(File, iTabCount, "HittingType          %d\n", c_rData.iHittingType);
    PrintfTabs(File, iTabCount, "StiffenTime          %f\n", c_rData.fStiffenTime);
    PrintfTabs(File, iTabCount, "InvisibleTime        %f\n", c_rData.fInvisibleTime);
    PrintfTabs(File, iTabCount, "ExternalForce        %f\n", c_rData.fExternalForce);
    PrintfTabs(File, iTabCount, "HitLimitCount        %d\n", c_rData.iHitLimitCount);
}

void NRaceData::SaveMotionAttackData(FILE *File, int iTabCount, const TMotionAttackData &c_rData)
{
    SaveAttackData(File, iTabCount, c_rData);

    PrintfTabs(File, iTabCount, "\n");
    PrintfTabs(File, iTabCount, "MotionType           %d\n", c_rData.iMotionType);
    PrintfTabs(File, iTabCount, "HitDataCount         %d\n", c_rData.HitDataContainer.size());

    uint32_t dwHitDataNumber = 0;
    THitDataContainer::const_iterator itor = c_rData.HitDataContainer.begin();
    for (; itor != c_rData.HitDataContainer.end(); ++itor, ++dwHitDataNumber)
    {
        const THitData &c_rHitData = *itor;

        PrintfTabs(File, iTabCount, "Group HitData%02d\n", dwHitDataNumber);
        PrintfTabs(File, iTabCount, "{\n");
        PrintfTabs(File, iTabCount + 1, "AttackingStartTime   %f\n", c_rHitData.fAttackStartTime);
        PrintfTabs(File, iTabCount + 1, "AttackingEndTime     %f\n", c_rHitData.fAttackEndTime);

        PrintfTabs(File, iTabCount + 1, "AttackingBone        \"%s\"\n", c_rHitData.strBoneName.c_str());
        PrintfTabs(File, iTabCount + 1, "WeaponLength         %f\n", c_rHitData.fWeaponLength);
        PrintfTabs(File, iTabCount + 1, "List HitPosition\n");
        PrintfTabs(File, iTabCount + 1, "{\n");

        NRaceData::THitTimePositionMap::const_iterator it;
        for (it = c_rHitData.mapHitPosition.begin(); it != c_rHitData.mapHitPosition.end(); ++it)
        {
            PrintfTabs(File, iTabCount + 2, "%f %f %f %f %f %f %f\n", it->first,
                       it->second.v3LastPosition.x, it->second.v3LastPosition.y, it->second.v3LastPosition.z,
                       it->second.v3Position.x, it->second.v3Position.y, it->second.v3Position.z);
        }
        PrintfTabs(File, iTabCount + 1, "}\n");
        PrintfTabs(File, iTabCount, "}\n");
    }
}

void NRaceData::SaveCollisionData(FILE *File, int iTabCount, const TCollisionData &c_rCollisionData)
{
    PrintfTabs(File, iTabCount, "CollisionType        %d\n", c_rCollisionData.iCollisionType);
    PrintfTabs(File, iTabCount, "\n");

    const CSphereCollisionInstanceVector &c_rSphereDataVector = c_rCollisionData.SphereDataVector;

    PrintfTabs(File, iTabCount, "SphereDataCount      %d\n", c_rSphereDataVector.size());

    for (uint32_t i = 0; i < c_rSphereDataVector.size(); ++i)
    {
        const TSphereData &c_rAttr = c_rSphereDataVector[i].GetAttribute();

        PrintfTabs(File, iTabCount, "Group SphereData%02d\n", i);

        PrintfTabs(File, iTabCount, "{\n");

        PrintfTabs(File, iTabCount + 1, "Radius           %f\n",
                   c_rAttr.fRadius);

        PrintfTabs(File, iTabCount + 1, "Position         %f %f %f\n",
                   c_rAttr.v3Position.x,
                   c_rAttr.v3Position.y,
                   c_rAttr.v3Position.z);

        PrintfTabs(File, iTabCount, "}\n");
    }
}

/////////////////////////////////////////////////////////////////////////////////
// Attaching Data
void NRaceData::SaveAttachingData(FILE *File, int iTabCount, const TAttachingDataVector &c_rAttachingDataVector)
{
    uint32_t dwAttachingDataCount = c_rAttachingDataVector.size();

    PrintfTabs(File, iTabCount, "AttachingDataCount       %d\n", dwAttachingDataCount);
    PrintfTabs(File, iTabCount, "\n");

    for (uint32_t i = 0; i < dwAttachingDataCount; ++i)
    {
        const NRaceData::TAttachingData &c_rAttachingData = c_rAttachingDataVector[i];

        PrintfTabs(File, iTabCount, "Group AttachingData%02d\n", i);
        PrintfTabs(File, iTabCount, "{\n", i);

        PrintfTabs(File, iTabCount + 1, "AttachingDataType    %d\n", c_rAttachingData.dwType);
        PrintfTabs(File, iTabCount + 1, "\n", i);

        PrintfTabs(File, iTabCount + 1, "isAttaching          %d\n", c_rAttachingData.isAttaching);
        PrintfTabs(File, iTabCount + 1, "AttachingModelIndex  %d\n", c_rAttachingData.dwAttachingModelIndex);
        PrintfTabs(File, iTabCount + 1, "AttachingBoneName    \"%s\"\n", c_rAttachingData.strAttachingBoneName.c_str());
        PrintfTabs(File, iTabCount + 1, "\n");

        switch (c_rAttachingData.dwType)
        {
        case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
            NRaceData::SaveCollisionData(File, iTabCount + 1, *c_rAttachingData.pCollisionData);
            break;
        case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
            NRaceData::SaveEffectData(File, iTabCount + 1, *c_rAttachingData.pEffectData);
            break;
        case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
            NRaceData::SaveObjectData(File, iTabCount + 1, *c_rAttachingData.pObjectData);
            break;
        }

        PrintfTabs(File, iTabCount, "}\n");

        if (i != dwAttachingDataCount - 1)
            PrintfTabs(File, iTabCount, "\n");
    }
}

void NRaceData::SaveAttachingData(storm::File &File, int iTabCount, const TAttachingDataVector &c_rAttachingDataVector)
{
    uint32_t dwAttachingDataCount = c_rAttachingDataVector.size();

    PrintfTabs(File, iTabCount, "AttachingDataCount       %d\n", dwAttachingDataCount);
    PrintfTabs(File, iTabCount, "\n");

    for (uint32_t i = 0; i < dwAttachingDataCount; ++i)
    {
        const NRaceData::TAttachingData &c_rAttachingData = c_rAttachingDataVector[i];

        PrintfTabs(File, iTabCount, "Group AttachingData%02d\n", i);
        PrintfTabs(File, iTabCount, "{\n", i);

        PrintfTabs(File, iTabCount + 1, "AttachingDataType    %d\n", c_rAttachingData.dwType);
        PrintfTabs(File, iTabCount + 1, "\n", i);

        PrintfTabs(File, iTabCount + 1, "isAttaching          %d\n", c_rAttachingData.isAttaching);
        PrintfTabs(File, iTabCount + 1, "AttachingModelIndex  %d\n", c_rAttachingData.dwAttachingModelIndex);
        PrintfTabs(File, iTabCount + 1, "AttachingBoneName    \"%s\"\n", c_rAttachingData.strAttachingBoneName.c_str());
        PrintfTabs(File, iTabCount + 1, "\n");

        switch (c_rAttachingData.dwType)
        {
        case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
            NRaceData::SaveCollisionData(File, iTabCount + 1, *c_rAttachingData.pCollisionData);
            break;
        case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
            NRaceData::SaveEffectData(File, iTabCount + 1, *c_rAttachingData.pEffectData);
            break;
        case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
            NRaceData::SaveObjectData(File, iTabCount + 1, *c_rAttachingData.pObjectData);
            break;
        }

        PrintfTabs(File, iTabCount, "}\n");

        if (i != dwAttachingDataCount - 1)
            PrintfTabs(File, iTabCount, "\n");
    }
}

bool NRaceData::LoadAttachingData(CTextFileLoader &rTextFileLoader, TAttachingDataVector *pAttachingDataVector)
{
    uint32_t dwDataCount;
    if (!rTextFileLoader.GetTokenDoubleWord("attachingdatacount", &dwDataCount))
        return false;

    pAttachingDataVector->clear();
    pAttachingDataVector->resize(dwDataCount);

    for (uint32_t i = 0; i < dwDataCount; ++i)
    {
        NRaceData::TAttachingData &rAttachingData = pAttachingDataVector->at(i);
        if (!rTextFileLoader.SetChildNode("attachingdata", i))
            return false;

        if (!rTextFileLoader.GetTokenDoubleWord("attachingdatatype", &rAttachingData.dwType))
            return false;

        if (!rTextFileLoader.GetTokenBoolean("isattaching", &rAttachingData.isAttaching))
            return false;
        if (!rTextFileLoader.GetTokenDoubleWord("attachingmodelindex", &rAttachingData.dwAttachingModelIndex))
            return false;
        if (!rTextFileLoader.GetTokenString("attachingbonename", &rAttachingData.strAttachingBoneName))
            return false;

        switch (rAttachingData.dwType)
        {
        case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
            rAttachingData.pCollisionData = g_CollisionDataPool.Alloc();
            if (!NRaceData::LoadCollisionData(rTextFileLoader, rAttachingData.pCollisionData))
                return false;
            break;
        case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
            rAttachingData.pEffectData = g_EffectDataPool.Alloc();
            if (!NRaceData::LoadEffectData(rTextFileLoader, rAttachingData.pEffectData))
                return false;
            break;
        case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
            rAttachingData.pObjectData = g_ObjectDataPool.Alloc();
            if (!NRaceData::LoadObjectData(rTextFileLoader, rAttachingData.pObjectData))
                return false;
            break;
        }

        rTextFileLoader.SetParentNode();
    }
    return true;
}
