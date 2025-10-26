#include "StdAfx.h"
#include "PythonSkill.h"

#include <pak/ClientDb.hpp>

#include "../EterBase/Poly/Poly.h"
#include <pak/Vfs.hpp>
#include "InstanceBase.h"
#include "PythonPlayer.h"
#include "../EterLib/ResourceManager.h"
#include "game/SkillTypes.hpp"
#include "pak/Util.hpp"

std::map<std::string, uint32_t> CPythonSkill::SSkillData::ms_StatusNameMap;
std::map<std::string, uint32_t> CPythonSkill::SSkillData::ms_NewMinStatusNameMap;
std::map<std::string, uint32_t> CPythonSkill::SSkillData::ms_NewMaxStatusNameMap;
uint32_t CPythonSkill::SSkillData::ms_dwTimeIncreaseSkillNumber = 0;

bool CPythonSkill::RegisterSkillTable(const char *c_szFileName)
{
    std::vector<SkillProto> entries;
    if (!LoadClientDb(GetVfs(), c_szFileName, entries))
        return false;

    for (const auto &entry : entries)
    {
        auto it = m_SkillDataMap.find(entry.vnum);
        if (it == m_SkillDataMap.end())
        {
            SPDLOG_DEBUG("SkillProto: Skill {0} doesn't exist in proto but was requested by skilldesc", entry.vnum);
            continue;
        }

        auto &skillData = it->second;

        skillData.strNeedSPFormula = entry.spCost;
        skillData.strCoolTimeFormula = entry.cooltime;
        skillData.strContinuationSPFormula = entry.durationSpCost;
        skillData.strDuration = entry.duration;
        skillData.strLevelStepFormula = entry.levelStep;

        if (entry.targetRange != 0)
            skillData.dwTargetRange = entry.targetRange;

        if (entry.maxLevel != 0)
            skillData.byMaxLevel = entry.maxLevel;
        else
            skillData.byMaxLevel = 1;

        if (entry.levelLimit != 0)
            skillData.byLevelLimit = entry.levelLimit;

        skillData.prerequisiteSkills = entry.requirements;

        // OVERWRITE_SKILLPROTO_POLY
        bool USE_SKILL_PROTO = true;

        switch (entry.vnum)
        {
        case 34:
            // GUILD_SKILL_DISPLAY_BUG_FIX
        case 151:
        case 152:
        case 153:
        case 154:
        case 155:
        case 156:
        case 157:
            // END_OF_GUILD_SKILL_DISPLAY_BUG_FIX
            USE_SKILL_PROTO = false;
            break;
        }

        if (!skillData.AffectDataVector.empty() && USE_SKILL_PROTO)
        {
            auto &affect = skillData.AffectDataVector[0];

            if (strstr(entry.value.c_str(), "atk") != NULL ||
                strstr(entry.value.c_str(), "mwep") != NULL ||
                strstr(entry.value.c_str(), "number") != NULL)
            {
                affect.strAffectMinFormula = entry.value;
                boost::replace_all(affect.strAffectMinFormula, "number", "min");
                boost::replace_all(affect.strAffectMinFormula, "atk", "minatk");
                boost::replace_all(affect.strAffectMinFormula, "mwep", "minmwep");

                affect.strAffectMaxFormula = entry.value;
                boost::replace_all(affect.strAffectMaxFormula, "number", "max");
                boost::replace_all(affect.strAffectMaxFormula, "atk", "maxatk");
                boost::replace_all(affect.strAffectMaxFormula, "mwep", "maxmwep");

                switch (entry.vnum)
                {
                case 1: // 삼연참
                    affect.strAffectMinFormula += "* 3";
                    affect.strAffectMaxFormula += "* 3";
                    break;
                }
            }
            else
            {
                affect.strAffectMinFormula = entry.value;
                affect.strAffectMaxFormula.clear();
            }
        }
        // END_OF_OVERWRITE_SKILLPROTO_POLY
    }

    return true;
}

void CPythonSkill::__RegisterGradeIconImage(TSkillData &rData, const char *c_szHeader, const char *c_szImageName)
{
    for (int j = 0; j < SKILL_GRADE_COUNT; ++j)
    {
        TGradeData &rGradeData = rData.GradeData[j];

        char szCount[8 + 1];
        _snprintf(szCount, sizeof szCount, "_%02d", std::min(3, j + 1));

        std::string strFileName = "";
        strFileName += c_szHeader;
        strFileName += c_szImageName;
        strFileName += szCount;
        strFileName += ".sub";
        rGradeData.pImage = CResourceManager::Instance().LoadResource<CGraphicImage>(strFileName);
    }
}

void CPythonSkill::__RegisterDirectIconImage(TSkillData &rData, const char *c_szImage)
{
    rData.pImage = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szImage);
    for (int j = 0; j < SKILL_GRADE_COUNT; ++j)
    {
        TGradeData &rGradeData = rData.GradeData[std::min(3, j)];
        rGradeData.pImage = rData.pImage;
    }
}

void CPythonSkill::__RegisterNormalIconImage(TSkillData &rData, const char *c_szHeader, const char *c_szImageName)
{
    std::string strFileName = "";
    strFileName += c_szHeader;
    strFileName += c_szImageName;
    strFileName += ".sub";
    rData.pImage = CResourceManager::Instance().LoadResource<CGraphicImage>(strFileName);
    for (int j = 0; j < SKILL_GRADE_COUNT; ++j)
    {
        TGradeData &rGradeData = rData.GradeData[std::min(3, j)];
        rGradeData.pImage = rData.pImage;
    }
}

void CPythonSkill::__RegisterPetSkillIcon(TPetSkillData &rData, const char *c_szHeader, const char *c_szImageName)
{
    std::string strFileName = "";
    strFileName += c_szHeader;
    strFileName += c_szImageName;
    strFileName += ".sub";
    rData.pImage = CResourceManager::Instance().LoadResource<CGraphicImage>(strFileName);
}

bool CPythonSkill::RegisterPetSkill(const char *c_szFileName)
{
   /* if (!GetVfs().Open(c_szFileName))
        return false;

    CMemoryTextFileLoader textFileLoader;
    textFileLoader.Bind({reinterpret_cast<const char *>(kFile.Get()), kFile.Size()});

    CTokenVector TokenVector;
    for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLineByTab(i, &TokenVector))
            continue;

        if (PET_SKILL_TOKEN_VNUM >= TokenVector.size())
        {
            SPDLOG_ERROR("PetSkill.line({0}).NO_VNUM_ERROR\n", i + 1);
            continue;
        }

        auto skillIndex = std::stoul(TokenVector[PET_SKILL_TOKEN_VNUM]);
        if (skillIndex == 0)
        {
            SPDLOG_ERROR("PetSkill.line({0}).NO_INDEX_ERROR\n", i + 1);
            continue;
        }

        m_PetSkillDataMap.emplace(skillIndex, TPetSkillData());
        auto &rSkillData = m_PetSkillDataMap[skillIndex];

        rSkillData.skillId = skillIndex;
        rSkillData.name = TokenVector[PET_SKILL_TOKEN_NAME];
        rSkillData.icon = TokenVector[PET_SKILL_TOKEN_ICON];
        auto type = TokenVector[PET_SKILL_TOKEN_SKILL_TYPE];
        if (type == "PASSIVE")
            rSkillData.type = PET_SKILL_TYPE_PASSIVE;
        else if (type == "AUTO")
            rSkillData.type = PET_SKILL_TYPE_AUTO;
        else
            rSkillData.type = PET_SKILL_TYPE_NONE;

        rSkillData.description = TokenVector[PET_SKILL_TOKEN_DESC];
        rSkillData.cooltime = std::stoul(TokenVector[PET_SKILL_TOKEN_COOLTIME]);

        char szIconFileNameHeader[64 + 1];
        _snprintf(szIconFileNameHeader, sizeof szIconFileNameHeader, "%sskill/pet/", g_strImagePath.c_str());

        __RegisterPetSkillIcon(rSkillData, szIconFileNameHeader, rSkillData.icon.c_str());
    }*/

    return true;
}

extern const uint32_t c_iSkillIndex_Riding;

bool CPythonSkill::RegisterSkillDesc(const char *c_szFileName)
{
    auto str = LoadFileToString(GetVfs(), c_szFileName);
    if (!str)
        return false;

    CMemoryTextFileLoader textFileLoader;
    textFileLoader.Bind(str.value());

    CTokenVector TokenVector;
    for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
    {
        if (!textFileLoader.SplitLineByTab(i, &TokenVector))
            continue;

        if (DESC_TOKEN_TYPE_VNUM >= TokenVector.size())
        {
            SPDLOG_ERROR("SkillDesc.line({0}).NO_VNUM_ERROR\n", i + 1);
            continue;
        }

        uint32_t iSkillIndex = atoi(TokenVector[DESC_TOKEN_TYPE_VNUM].c_str());
        if (iSkillIndex == 0)
        {
            SPDLOG_ERROR("SkillDesc.line({0}).NO_INDEX_ERROR\n", i + 1);
            continue;
        }

        if (DESC_TOKEN_TYPE_JOB >= TokenVector.size())
        {
            SPDLOG_ERROR("SkillDesc.line({0}).NO_JOB_ERROR\n", i + 1);
            continue;
        }

        m_SkillDataMap.emplace(iSkillIndex, TSkillData());
        TSkillData &rSkillData = m_SkillDataMap[iSkillIndex];

        // Vnum
        rSkillData.dwSkillIndex = iSkillIndex;

        // Type
        {
            std::string strJob = TokenVector[DESC_TOKEN_TYPE_JOB];
            stl_lowers(strJob);

            auto it = m_SkillTypeIndexMap.find(strJob);
            if (m_SkillTypeIndexMap.end() == it)
            {
                SPDLOG_ERROR("CPythonSkill::RegisterSkillDesc(dwSkillIndex={0}, strType={1}).STRANGE_SKILL_TYPE",
                              iSkillIndex, strJob.c_str());
                continue;
            }

            rSkillData.byType = uint8_t(it->second);
        }

        // Name
        {
            rSkillData.strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
            rSkillData.GradeData[0].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
            rSkillData.GradeData[1].strName = TokenVector[DESC_TOKEN_TYPE_NAME2];
            rSkillData.GradeData[2].strName = TokenVector[DESC_TOKEN_TYPE_NAME3];

            if (rSkillData.GradeData[1].strName.empty())
                rSkillData.GradeData[1].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
            if (rSkillData.GradeData[2].strName.empty())
                rSkillData.GradeData[2].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
        }

        // Description
        {
            rSkillData.strDescription = TokenVector[DESC_TOKEN_TYPE_DESCRIPTION];
        }

        // Condition
        {
            rSkillData.ConditionDataVector.clear();
            for (int j = 0; j < CONDITION_COLUMN_COUNT; ++j)
            {
                const std::string &c_rstrCondition = TokenVector[DESC_TOKEN_TYPE_CONDITION1 + j];
                if (!c_rstrCondition.empty())
                    rSkillData.ConditionDataVector.push_back(c_rstrCondition);
            }
        }

        // Attribute
        {
            const std::string &c_rstrAttribute = TokenVector[DESC_TOKEN_TYPE_ATTRIBUTE];
            CTokenVector AttributeTokenVector;
            SplitLine(c_rstrAttribute.c_str(), "|", &AttributeTokenVector);

            for (CTokenVector::iterator it = AttributeTokenVector.begin(); it != AttributeTokenVector.end(); ++it)
            {
                std::string &rstrToken = *it;
                std::map<std::string, uint32_t>::iterator it2 = m_SkillAttributeIndexMap.find(rstrToken);
                if (m_SkillAttributeIndexMap.end() == it2)
                {
                    SPDLOG_ERROR("CPythonSkill::RegisterSkillDesc(dwSkillIndex={0}) - Strange Skill Attribute({1})",
                                  iSkillIndex, rstrToken.c_str());
                    continue;
                }
                rSkillData.dwSkillAttribute |= it2->second;
            }
        }

        // Weapon
        {
            const std::string &c_rstrWeapon = TokenVector[DESC_TOKEN_TYPE_WEAPON];
            CTokenVector WeaponTokenVector;
            SplitLine(c_rstrWeapon.c_str(), "|", &WeaponTokenVector);

            for (CTokenVector::iterator it = WeaponTokenVector.begin(); it != WeaponTokenVector.end(); ++it)
            {
                std::string &rstrToken = *it;
                std::map<std::string, uint32_t>::iterator it2 = m_SkillNeedWeaponIndexMap.find(rstrToken);
                if (m_SkillNeedWeaponIndexMap.end() == it2)
                {
                    SPDLOG_ERROR("CPythonSkill::RegisterSkillDesc(dwSkillIndex={0}) - Strange Skill Need Weapon({1})",
                                  iSkillIndex, rstrToken.c_str());
                    continue;
                }
                rSkillData.dwNeedWeapon |= it2->second;
            }
        }

        // Icon Name
        {
            const std::string &c_rstrJob = TokenVector[DESC_TOKEN_TYPE_JOB];
            const std::string &c_rstrIconName = TokenVector[DESC_TOKEN_TYPE_ICON_NAME];

            if (iSkillIndex >= 180 && iSkillIndex <= 202)
            {
                __RegisterDirectIconImage(rSkillData, c_rstrIconName.c_str());
            }
            else if (c_iSkillIndex_Riding == iSkillIndex)
            {
                char szIconFileNameHeader[64 + 1];
                _snprintf(szIconFileNameHeader, sizeof szIconFileNameHeader, "%sskill/common/support/",
                          g_strImagePath.c_str());

                __RegisterGradeIconImage(rSkillData, szIconFileNameHeader, c_rstrIconName.c_str());
            }
            else if (m_PathNameMap.end() != m_PathNameMap.find(c_rstrJob))
            {
                char szIconFileNameHeader[64 + 1];
                _snprintf(szIconFileNameHeader, sizeof szIconFileNameHeader, "%sskill/%s/", g_strImagePath.c_str(),
                          m_PathNameMap[c_rstrJob].c_str());

                switch (rSkillData.byType)
                {
                case SKILL_TYPE_ACTIVE: {
                    __RegisterGradeIconImage(rSkillData, szIconFileNameHeader, c_rstrIconName.c_str());
                    break;
                }
                case SKILL_TYPE_SUPPORT:
                case SKILL_TYPE_GUILD:
                case SKILL_TYPE_HORSE: {
                    __RegisterNormalIconImage(rSkillData, szIconFileNameHeader, c_rstrIconName.c_str());
                    break;
                }
                }
            }
        }

        // Motion Index
        if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_INDEX)
        {
            const std::string &c_rstrMotionIndex = TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX];
            if (c_rstrMotionIndex.empty())
            {
                rSkillData.bNoMotion = TRUE;
                rSkillData.wMotionIndex = 0;
            }
            else
            {
                rSkillData.bNoMotion = FALSE;
                rSkillData.wMotionIndex = (uint16_t)atoi(c_rstrMotionIndex.c_str());
            }
        }
        else
        {
            rSkillData.wMotionIndex = 0;
        }

        if (TokenVector.size() > DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA)
            rSkillData.strTargetCountFormula = TokenVector[DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA].c_str();
        if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA)
            rSkillData.strMotionLoopCountFormula = TokenVector[DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA].c_str();

        rSkillData.AffectDataNewVector.clear();
        rSkillData.AffectDataNewVector.reserve(3);

        // Affect
        for (int j = 0; j < AFFECT_COLUMN_COUNT; ++j)
        {
            int iDescriptionSlotIndex = DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_1 + j * AFFECT_STEP_COUNT;
            int iMinSlotIndex = DESC_TOKEN_TYPE_AFFECT_MIN_1 + j * AFFECT_STEP_COUNT;
            int iMaxSlotIndex = DESC_TOKEN_TYPE_AFFECT_MAX_1 + j * AFFECT_STEP_COUNT;

            if (TokenVector.size() > iDescriptionSlotIndex)
                if (!TokenVector[iDescriptionSlotIndex].empty())
                {
                    rSkillData.AffectDataVector.push_back(TAffectData());

                    TAffectData &rAffectData = *rSkillData.AffectDataVector.rbegin();
                    rAffectData.strAffectDescription.clear();
                    rAffectData.strAffectMinFormula.clear();
                    rAffectData.strAffectMaxFormula.clear();

                    rAffectData.strAffectDescription = TokenVector[iDescriptionSlotIndex];
                    if (TokenVector.size() > iMinSlotIndex)
                        if (!TokenVector[iMinSlotIndex].empty())
                            rAffectData.strAffectMinFormula = TokenVector[iMinSlotIndex];
                    if (TokenVector.size() > iMaxSlotIndex)
                        if (!TokenVector[iMaxSlotIndex].empty())
                            rAffectData.strAffectMaxFormula = TokenVector[iMaxSlotIndex];
                }
        }

        if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM)
        {
            int numGrade = atoi(TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM].c_str());
            if (SKILL_EFFECT_COUNT < numGrade)
            {
                SPDLOG_ERROR("%s[{0}] °¡ µî±Þ Á¦ÇÑ[{1}]À» ³Ñ¾î°¬½À´Ï´Ù.", rSkillData.strName.c_str(),
                              TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM].c_str(), SKILL_EFFECT_COUNT);
                return false;
            }
            for (int iGrade = 0; iGrade < numGrade; iGrade++)
            {
                //if (iGrade == SKILL_GRADE_COUNT)
                //	rSkillData.GradeData[iGrade] = rSkillData.GradeData[iGrade-1];
                rSkillData.GradeData[iGrade].wMotionIndex = rSkillData.wMotionIndex + iGrade * SKILL_GRADEGAP;
            }
            for (int iSpace = numGrade; iSpace < SKILL_EFFECT_COUNT; iSpace++)
            {
                rSkillData.GradeData[iSpace].wMotionIndex = rSkillData.wMotionIndex;
            }
        }

        if (TokenVector.size() > DESC_TOKEN_TYPE_LEVEL_LIMIT)
        {
            const std::string &c_rstrLevelLimit = TokenVector[DESC_TOKEN_TYPE_LEVEL_LIMIT];
            if (c_rstrLevelLimit.empty())
                rSkillData.byLevelLimit = 0;
            else
                storm::ParseNumber(c_rstrLevelLimit, rSkillData.byLevelLimit);

        }

        if (TokenVector.size() > DESC_TOKEN_TYPE_MAX_LEVEL)
        {
            const std::string &c_rstrMaxLevel = TokenVector[DESC_TOKEN_TYPE_MAX_LEVEL];
            const uint32_t LevelLimitEmpty = 1;
            if (c_rstrMaxLevel.empty())
            {
                if (rSkillData.byLevelLimit > LevelLimitEmpty)
                    rSkillData.byMaxLevel = rSkillData.byLevelLimit;
                else
                    rSkillData.byMaxLevel = 20;
            }
            else
            {
                storm::ParseNumber(c_rstrMaxLevel, rSkillData.byMaxLevel);
            }
        }
    }

    return true;
}

void CPythonSkill::Destroy()
{
    m_SkillDataMap.clear();
}

bool CPythonSkill::GetPetSkillInfo(uint32_t dwSkillIndex, TPetSkillData **ppSkillData)
{
    auto it = m_PetSkillDataMap.find(dwSkillIndex);
    if (m_PetSkillDataMap.end() == it)
        return false;

    *ppSkillData = &it->second;
    return true;
}

bool CPythonSkill::GetSkillData(uint32_t dwSkillIndex, TSkillData **ppSkillData)
{
    auto it = m_SkillDataMap.find(dwSkillIndex);

    if (m_SkillDataMap.end() == it)
        return FALSE;

    *ppSkillData = &it->second;
    return TRUE;
}

bool CPythonSkill::GetSkillDataByName(const char *c_szName, TSkillData **ppSkillData)
{
    TSkillDataMap::iterator itor = m_SkillDataMap.begin();
    for (; itor != m_SkillDataMap.end(); ++itor)
    {
        TSkillData *pData = &itor->second;
        if (0 == pData->strName.compare(c_szName))
        {
            *ppSkillData = &itor->second;
            return true;
        }
    }
    return false;
}

void CPythonSkill::SetPathName(const char *c_szFileName)
{
    m_strPathName = c_szFileName;
}

const char *CPythonSkill::GetPathName()
{
    return m_strPathName.c_str();
}

CPythonSkill::CPythonSkill()
{
    m_SkillTypeIndexMap.emplace("none", SKILL_TYPE_NONE);
    m_SkillTypeIndexMap.emplace("active", SKILL_TYPE_ACTIVE);
    m_SkillTypeIndexMap.emplace("support", SKILL_TYPE_SUPPORT);
    m_SkillTypeIndexMap.emplace("guild", SKILL_TYPE_GUILD);
    m_SkillTypeIndexMap.emplace("horse", SKILL_TYPE_HORSE);
    m_SkillTypeIndexMap.emplace("warrior", SKILL_TYPE_ACTIVE);
    m_SkillTypeIndexMap.emplace("assassin", SKILL_TYPE_ACTIVE);
    m_SkillTypeIndexMap.emplace("sura", SKILL_TYPE_ACTIVE);
    m_SkillTypeIndexMap.emplace("shaman", SKILL_TYPE_ACTIVE);
    m_SkillTypeIndexMap.emplace("wolfman", SKILL_TYPE_ACTIVE);
    m_SkillTypeIndexMap.emplace("passive", SKILL_TYPE_ACTIVE);
    m_SkillAttributeIndexMap.emplace("NEED_POISON_BOTTLE", SKILL_ATTRIBUTE_NEED_POISON_BOTTLE);
    m_SkillAttributeIndexMap.emplace("NEED_EMPTY_BOTTLE", SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE);
    m_SkillAttributeIndexMap.emplace("CAN_USE_IF_NOT_ENOUGH", SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH);
    m_SkillAttributeIndexMap.emplace("FAN_RANGE", SKILL_ATTRIBUTE_FAN_RANGE);
    m_SkillAttributeIndexMap.emplace("NEED_CORPSE", SKILL_ATTRIBUTE_NEED_CORPSE);
    m_SkillAttributeIndexMap.emplace("NEED_TARGET", SKILL_ATTRIBUTE_NEED_TARGET);
    m_SkillAttributeIndexMap.emplace("TOGGLE", SKILL_ATTRIBUTE_TOGGLE);
    m_SkillAttributeIndexMap.emplace("WEAPON_LIMITATION", SKILL_ATTRIBUTE_WEAPON_LIMITATION);
    m_SkillAttributeIndexMap.emplace("MELEE_ATTACK", SKILL_ATTRIBUTE_MELEE_ATTACK);
    m_SkillAttributeIndexMap.emplace("CHARGE_ATTACK", SKILL_ATTRIBUTE_CHARGE_ATTACK);
    m_SkillAttributeIndexMap.emplace("USE_HP", SKILL_ATTRIBUTE_USE_HP);
    m_SkillAttributeIndexMap.emplace("CAN_CHANGE_DIRECTION", SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION);
    m_SkillAttributeIndexMap.emplace("STANDING_SKILL", SKILL_ATTRIBUTE_STANDING_SKILL);
    m_SkillAttributeIndexMap.emplace("ONLY_FOR_ALLIANCE", SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE);
    m_SkillAttributeIndexMap.emplace("CAN_USE_FOR_ME", SKILL_ATTRIBUTE_CAN_USE_FOR_ME);
    m_SkillAttributeIndexMap.emplace("ATTACK_SKILL", SKILL_ATTRIBUTE_ATTACK_SKILL);
    m_SkillAttributeIndexMap.emplace("MOVING_SKILL", SKILL_ATTRIBUTE_MOVING_SKILL);
    m_SkillAttributeIndexMap.emplace("HORSE_SKILL", SKILL_ATTRIBUTE_HORSE_SKILL);
    m_SkillAttributeIndexMap.emplace("TIME_INCREASE_SKILL", SKILL_ATTRIBUTE_TIME_INCREASE_SKILL);
    m_SkillAttributeIndexMap.emplace("PASSIVE", SKILL_ATTRIBUTE_PASSIVE);
    m_SkillAttributeIndexMap.emplace("CANNOT_LEVEL_UP", SKILL_ATTRIBUTE_CANNOT_LEVEL_UP);
    m_SkillAttributeIndexMap.emplace("ONLY_FOR_GUILD_WAR", SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR);
    m_SkillAttributeIndexMap.emplace("CIRCLE_RANGE", SKILL_ATTRIBUTE_CIRCLE_RANGE);
    m_SkillAttributeIndexMap.emplace("SEARCH_TARGET", SKILL_ATTRIBUTE_SEARCH_TARGET);
    m_SkillNeedWeaponIndexMap.emplace("SWORD", SKILL_NEED_WEAPON_SWORD);
    m_SkillNeedWeaponIndexMap.emplace("DAGGER", SKILL_NEED_WEAPON_DAGGER);
    m_SkillNeedWeaponIndexMap.emplace("BOW", SKILL_NEED_WEAPON_BOW);
    m_SkillNeedWeaponIndexMap.emplace("TWO_HANDED", SKILL_NEED_WEAPON_TWO_HANDED);
    m_SkillNeedWeaponIndexMap.emplace("DOUBLE_SWORD", SKILL_NEED_WEAPON_DOUBLE_SWORD);
    m_SkillNeedWeaponIndexMap.emplace("BELL", SKILL_NEED_WEAPON_BELL);
    m_SkillNeedWeaponIndexMap.emplace("FAN", SKILL_NEED_WEAPON_FAN);
    m_SkillNeedWeaponIndexMap.emplace("ARROW", SKILL_NEED_WEAPON_ARROW);
    m_SkillNeedWeaponIndexMap.emplace("CLAW", SKILL_NEED_WEAPON_CLAW);
    m_SkillNeedWeaponIndexMap.emplace("EMPTY_HAND", SKILL_NEED_WEAPON_EMPTY_HAND);

    m_SkillWeaponTypeIndexMap.emplace("SWORD", WEAPON_SWORD);
    m_SkillWeaponTypeIndexMap.emplace("DAGGER", WEAPON_DAGGER);
    m_SkillWeaponTypeIndexMap.emplace("BOW", WEAPON_BOW);
    m_SkillWeaponTypeIndexMap.emplace("TWO_HANDED", WEAPON_TWO_HANDED);
    m_SkillWeaponTypeIndexMap.emplace("DOUBLE_SWORD", WEAPON_DAGGER);
    m_SkillWeaponTypeIndexMap.emplace("BELL", WEAPON_BELL);
    m_SkillWeaponTypeIndexMap.emplace("FAN", WEAPON_FAN);
    m_SkillWeaponTypeIndexMap.emplace("ARROW", WEAPON_ARROW);
    m_SkillWeaponTypeIndexMap.emplace("CLAW", WEAPON_CLAW);

    SSkillData::ms_StatusNameMap.emplace("chain", POINT_NONE);
    SSkillData::ms_StatusNameMap.emplace("HR", POINT_HIT_RATE);
    SSkillData::ms_StatusNameMap.emplace("LV", POINT_LEVEL);
    SSkillData::ms_StatusNameMap.emplace("Level", POINT_LEVEL);
    SSkillData::ms_StatusNameMap.emplace("MaxHP", POINT_MAX_HP);
    SSkillData::ms_StatusNameMap.emplace("MaxSP", POINT_MAX_SP);
    SSkillData::ms_StatusNameMap.emplace("MinMWEP", POINT_MIN_WEP);
    SSkillData::ms_StatusNameMap.emplace("MaxMWEP", POINT_MAX_WEP);
    SSkillData::ms_StatusNameMap.emplace("MinWEP", POINT_MIN_WEP);
    SSkillData::ms_StatusNameMap.emplace("MaxWEP", POINT_MAX_WEP);
    SSkillData::ms_StatusNameMap.emplace("MinATK", POINT_WEAPON_MIN);
    SSkillData::ms_StatusNameMap.emplace("MaxATK", POINT_WEAPON_MAX);
    SSkillData::ms_StatusNameMap.emplace("ATKSPD", POINT_ATT_SPEED);
    SSkillData::ms_StatusNameMap.emplace("AttackPower", POINT_WEAPON_MIN);
    SSkillData::ms_StatusNameMap.emplace("AtkMin", POINT_WEAPON_MIN);
    SSkillData::ms_StatusNameMap.emplace("AtkMax", POINT_WEAPON_MAX);
    SSkillData::ms_StatusNameMap.emplace("DefencePower", POINT_DEF_GRADE);
    SSkillData::ms_StatusNameMap.emplace("DEF", POINT_DEF_GRADE);
    SSkillData::ms_StatusNameMap.emplace("MWEP", POINT_MAGIC_ATT_GRADE);

    SSkillData::ms_StatusNameMap.emplace("MagicAttackPower", POINT_MAGIC_ATT_GRADE);
    SSkillData::ms_StatusNameMap.emplace("INT", POINT_IQ);
    SSkillData::ms_StatusNameMap.emplace("STR", POINT_ST);
    SSkillData::ms_StatusNameMap.emplace("DEX", POINT_DX);
    SSkillData::ms_StatusNameMap.emplace("CON", POINT_HT);

    SSkillData::ms_StatusNameMap.emplace("minatk", POINT_WEAPON_MIN);
    SSkillData::ms_StatusNameMap.emplace("maxatk", POINT_WEAPON_MAX);
    SSkillData::ms_StatusNameMap.emplace("minmtk", POINT_MIN_WEP);
    SSkillData::ms_StatusNameMap.emplace("maxmtk", POINT_MAX_WEP);

    // GUILD_SKILL_DISPLAY_BUG_FIX
    SSkillData::ms_StatusNameMap.emplace("maxhp", POINT_MAX_HP);
    SSkillData::ms_StatusNameMap.emplace("maxsp", POINT_MAX_SP);
    SSkillData::ms_StatusNameMap.emplace("odef", POINT_DEF_GRADE);
    // END_OF_GUILD_SKILL_DISPLAY_BUG_FIX

    SSkillData::ms_StatusNameMap.emplace("minwep", POINT_MIN_WEP);
    SSkillData::ms_StatusNameMap.emplace("maxwep", POINT_MAX_WEP);
    SSkillData::ms_StatusNameMap.emplace("minmwep", POINT_MIN_MAGIC_WEP);
    SSkillData::ms_StatusNameMap.emplace("maxmwep", POINT_MAX_MAGIC_WEP);
    SSkillData::ms_StatusNameMap.emplace("lv", POINT_LEVEL);
    SSkillData::ms_StatusNameMap.emplace("ar", POINT_HIT_RATE);
    SSkillData::ms_StatusNameMap.emplace("iq", POINT_IQ);
    SSkillData::ms_StatusNameMap.emplace("str", POINT_ST);
    SSkillData::ms_StatusNameMap.emplace("dex", POINT_DX);
    SSkillData::ms_StatusNameMap.emplace("con", POINT_HT);

    /////

    SSkillData::ms_NewMinStatusNameMap.emplace("atk", POINT_WEAPON_MIN);
    SSkillData::ms_NewMinStatusNameMap.emplace("mtk", POINT_MIN_WEP);
    SSkillData::ms_NewMinStatusNameMap.emplace("wep", POINT_MIN_WEP);
    SSkillData::ms_NewMinStatusNameMap.emplace("lv", POINT_LEVEL);
    SSkillData::ms_NewMinStatusNameMap.emplace("ar", POINT_HIT_RATE);
    SSkillData::ms_NewMinStatusNameMap.emplace("iq", POINT_IQ);
    SSkillData::ms_NewMinStatusNameMap.emplace("str", POINT_ST);
    SSkillData::ms_NewMinStatusNameMap.emplace("dex", POINT_DX);
    SSkillData::ms_NewMinStatusNameMap.emplace("con", POINT_HT);

    SSkillData::ms_NewMaxStatusNameMap.emplace("atk", POINT_WEAPON_MAX);
    SSkillData::ms_NewMaxStatusNameMap.emplace("mtk", POINT_MAX_WEP);
    SSkillData::ms_NewMaxStatusNameMap.emplace("wep", POINT_MAX_WEP);
    SSkillData::ms_NewMaxStatusNameMap.emplace("lv", POINT_LEVEL);
    SSkillData::ms_NewMaxStatusNameMap.emplace("ar", POINT_HIT_RATE);
    SSkillData::ms_NewMaxStatusNameMap.emplace("iq", POINT_IQ);
    SSkillData::ms_NewMaxStatusNameMap.emplace("str", POINT_ST);
    SSkillData::ms_NewMaxStatusNameMap.emplace("dex", POINT_DX);
    SSkillData::ms_NewMaxStatusNameMap.emplace("con", POINT_HT);

    m_PathNameMap.emplace("WARRIOR", "warrior");
    m_PathNameMap.emplace("ASSASSIN", "assassin");
    m_PathNameMap.emplace("SURA", "sura");
    m_PathNameMap.emplace("SHAMAN", "shaman");
    m_PathNameMap.emplace("WOLFMAN", "wolfman");
    m_PathNameMap.emplace("PASSIVE", "passive");
    m_PathNameMap.emplace("SUPPORT", "common/support");
    m_PathNameMap.emplace("GUILD", "common/guild");
    m_PathNameMap.emplace("HORSE", "common/horse");
}

CPythonSkill::~CPythonSkill()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t CPythonSkill::SSkillData::MELEE_SKILL_TARGET_RANGE = 170;

const std::string CPythonSkill::SSkillData::GetName() const
{
    return strName;
}

uint8_t CPythonSkill::SSkillData::GetType() const
{
    return byType;
}

bool CPythonSkill::SSkillData::IsMeleeSkill()
{
    if (dwSkillAttribute & SKILL_ATTRIBUTE_MELEE_ATTACK)
        return TRUE;

    return FALSE;
}

bool CPythonSkill::SSkillData::IsChargeSkill()
{
    if (dwSkillAttribute & SKILL_ATTRIBUTE_CHARGE_ATTACK)
        return TRUE;

    return FALSE;
}

bool CPythonSkill::SSkillData::IsOnlyForGuildWar()
{
    if (dwSkillAttribute & SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR)
        return TRUE;

    return FALSE;
}

uint32_t CPythonSkill::SSkillData::GetTargetRange() const
{
    if (dwSkillAttribute & SKILL_ATTRIBUTE_MELEE_ATTACK)
        return MELEE_SKILL_TARGET_RANGE;

    if (dwSkillAttribute & SKILL_ATTRIBUTE_CHARGE_ATTACK)
        return MELEE_SKILL_TARGET_RANGE;

    return dwTargetRange;
}

bool CPythonSkill::SSkillData::CanChangeDirection()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION);
}

bool CPythonSkill::SSkillData::IsFanRange()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_FAN_RANGE);
}

bool CPythonSkill::SSkillData::IsCircleRange()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CIRCLE_RANGE);
}

bool CPythonSkill::SSkillData::IsAutoSearchTarget()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_SEARCH_TARGET);
}

bool CPythonSkill::SSkillData::IsNeedCorpse()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_CORPSE);
}

bool CPythonSkill::SSkillData::IsNeedTarget()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_TARGET);
}

bool CPythonSkill::SSkillData::IsToggleSkill()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_TOGGLE);
}

bool CPythonSkill::SSkillData::IsUseHPSkill()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_USE_HP);
}

bool CPythonSkill::SSkillData::IsStandingSkill()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_STANDING_SKILL);
}

bool CPythonSkill::SSkillData::CanUseWeaponType(uint32_t dwWeaponType)
{
    if (!(dwSkillAttribute & SKILL_ATTRIBUTE_WEAPON_LIMITATION))
        return TRUE;

    return 0 != (dwNeedWeapon & 1 << dwWeaponType);
}

bool CPythonSkill::SSkillData::IsOnlyForAlliance()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE);
}

bool CPythonSkill::SSkillData::CanUseForMe()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_USE_FOR_ME);
}

bool CPythonSkill::SSkillData::CanUseIfNotEnough()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH);
}

bool CPythonSkill::SSkillData::IsNeedEmptyBottle()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE);
}

bool CPythonSkill::SSkillData::IsNeedPoisonBottle()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_POISON_BOTTLE);
}

bool CPythonSkill::SSkillData::IsNeedBow()
{
    if (!(dwSkillAttribute & SKILL_ATTRIBUTE_WEAPON_LIMITATION))
        return FALSE;

    return 0 != (dwNeedWeapon & SKILL_NEED_WEAPON_BOW);
}

bool CPythonSkill::SSkillData::IsHorseSkill()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_HORSE_SKILL);
}

bool CPythonSkill::SSkillData::IsMovingSkill()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_MOVING_SKILL);
}

bool CPythonSkill::SSkillData::IsAttackSkill()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_ATTACK_SKILL);
}

bool CPythonSkill::SSkillData::IsTimeIncreaseSkill()
{
    return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_TIME_INCREASE_SKILL);
}

bool CPythonSkill::SSkillData::GetState(const char *c_szStateName, int *piState, int iMinMaxType)
{
    std::map<std::string, uint32_t> *pStatusNameMap = NULL;

    switch (iMinMaxType)
    {
    case VALUE_TYPE_FREE:
        pStatusNameMap = &ms_StatusNameMap;
        break;
    case VALUE_TYPE_MIN:
        pStatusNameMap = &ms_NewMinStatusNameMap;
        break;
    case VALUE_TYPE_MAX:
        pStatusNameMap = &ms_NewMaxStatusNameMap;
        break;
    default:
        return false;
        break;
    }

    std::map<std::string, uint32_t>::iterator it = pStatusNameMap->find(c_szStateName);

    if (it != pStatusNameMap->end())
    {
        *piState = CPythonPlayer::Instance().GetStatus(it->second);
    }

    return true;
}

float CPythonSkill::SSkillData::ProcessFormula(CPoly *pPoly, float fSkillLevel /*= 0.0f*/,
                                               int iSkillActualLevel /*= 0*/, int iMinMaxType /*= VALUE_TYPE_FREE*/)
{
    if (pPoly->Analyze())
    {
        for (uint32_t i = 0; i < pPoly->GetVarCount(); ++i)
        {
            const char *c_szVarName = pPoly->GetVarName(i);
            float fState;
            if (!strcmp("SkillPoint", c_szVarName) || !strcmp("k", c_szVarName))
            {
                fState = fSkillLevel;
            }
            else if (!strcmp("SkillLevel", c_szVarName))
            {
                fState = (float)iSkillActualLevel;
            }
            else
            {
                int iState;
                if (!GetState(c_szVarName, &iState, iMinMaxType))
                    return 0.0f;

                fState = float(iState);

                if (!strcmp("ar", c_szVarName))
                    fState /= 100.0f;
            }
            pPoly->SetVar(c_szVarName, fState);
        }
    }
    else
    {
        SPDLOG_ERROR("skillGetAffect - Strange Formula [{0}]", strName.c_str());
        return 0.0f;
    }

    return pPoly->Eval();
}

const char *CPythonSkill::SSkillData::GetAffectDescription(uint32_t dwIndex, float fSkillLevel, int iActualSkillLevel)
{
    if (dwIndex >= AffectDataVector.size())
        return NULL;

    const std::string &c_rstrAffectDescription = AffectDataVector[dwIndex].strAffectDescription;
    const std::string &c_rstrAffectMinFormula = AffectDataVector[dwIndex].strAffectMinFormula;
    const std::string &c_rstrAffectMaxFormula = AffectDataVector[dwIndex].strAffectMaxFormula;

    CPoly minPoly;
    CPoly maxPoly;
    minPoly.SetStr(c_rstrAffectMinFormula.c_str());
    maxPoly.SetStr(c_rstrAffectMaxFormula.c_str());

    // OVERWRITE_SKILLPROTO_POLY
    float fMinValue = ProcessFormula(&minPoly, fSkillLevel, iActualSkillLevel);
    float fMaxValue = ProcessFormula(&maxPoly, fSkillLevel, iActualSkillLevel);

    if (fMinValue < 0.0)
        fMinValue = - fMinValue;
    if (fMaxValue < 0.0)
        fMaxValue = - fMaxValue;

    if (strstr(c_rstrAffectDescription.c_str(), "%.0f"))
    {
        fMinValue = floorf(fMinValue);
        fMaxValue = floorf(fMaxValue);
    }

    static char szDescription[64 + 1];
    _snprintf(szDescription, sizeof szDescription, c_rstrAffectDescription.c_str(), fMinValue, fMaxValue);

    return szDescription;
}

uint32_t CPythonSkill::SSkillData::GetSkillLevelStep(float fSkillPoint)
{
    if (strLevelStepFormula.empty())
        return 0;

    CPoly poly;
    poly.Analyze(strLevelStepFormula.c_str());

    poly.SetVar("k", fSkillPoint);

    return poly.Eval();
}

uint32_t CPythonSkill::SSkillData::GetSkillCoolTime(float fSkillPoint)
{
    if (strCoolTimeFormula.empty())
        return 0;

    CPoly poly;
    poly.SetStr(strCoolTimeFormula.c_str());

    /*
        Apply casting speed when calculating the formula
    */
    int iCastingSpeed = CPythonPlayer::Instance().GetStatus(POINT_CASTING_SPEED);
    auto iSkillCoolTime = ProcessFormula(&poly, fSkillPoint);

    int i = 100 - iCastingSpeed;

    if (i > 0)
        i = 100 + i;
    else if (i < 0)
        i = 10000 / (100 - i);
    else
        i = 100;

    return uint32_t(iSkillCoolTime * i / 100);
}

uint32_t CPythonSkill::SSkillData::GetTargetCount(float fSkillPoint)
{
    if (strTargetCountFormula.empty())
        return 0;

    CPoly poly;
    poly.SetStr(strTargetCountFormula.c_str());

    return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

uint32_t CPythonSkill::SSkillData::GetSkillMotionIndex(int iGrade)
{
    if (-1 != iGrade)
    {
        if (iGrade >= 0 && iGrade < SKILL_EFFECT_COUNT + 1)
        {
            return GradeData[std::min(3, iGrade)].wMotionIndex;
        }
    }

    return wMotionIndex;
}

uint8_t CPythonSkill::SSkillData::GetMaxLevel()
{
    return byMaxLevel;
}

uint8_t CPythonSkill::SSkillData::GetLevelUpPoint()
{
    return byLevelUpPoint;
}

bool CPythonSkill::SSkillData::IsNoMotion()
{
    return bNoMotion;
}

bool CPythonSkill::SSkillData::IsCanUseSkill()
{
    if (0 != (dwSkillAttribute & SKILL_ATTRIBUTE_PASSIVE))
        return false;

    return true;
}

uint32_t CPythonSkill::SSkillData::GetMotionLoopCount(float fSkillPoint)
{
    if (strMotionLoopCountFormula.empty())
        return 0;

    CPoly poly;
    poly.SetStr(strMotionLoopCountFormula);

    return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

int CPythonSkill::SSkillData::GetNeedSP(float fSkillPoint)
{
    if (strNeedSPFormula.empty())
        return 0;

    CPoly poly;
    poly.SetStr(strNeedSPFormula.c_str());

    return int(ProcessFormula(&poly, fSkillPoint));
}

uint32_t CPythonSkill::SSkillData::GetContinuationSP(float fSkillPoint)
{
    if (strContinuationSPFormula.empty())
        return 0;

    CPoly poly;
    poly.SetStr(strContinuationSPFormula.c_str());

    return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

uint32_t CPythonSkill::SSkillData::GetDuration(float fSkillPoint)
{
    if (strDuration.empty())
        return 0;

    CPoly poly;
    poly.SetStr(strDuration.c_str());

    return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

CPythonSkill::SSkillData::SSkillData()
{
    byType = SKILL_TYPE_ACTIVE;
    byMaxLevel = 20;
    byLevelUpPoint = 1;
    dwSkillAttribute = 0;
    dwNeedWeapon = 0;
    dwTargetRange = 0;
    strCoolTimeFormula = "";
    strMotionLoopCountFormula = "";
    strNeedSPFormula = "";
    strContinuationSPFormula = "";

    strDuration = "";
    byLevelLimit = 0;
    bNoMotion = FALSE;

    strName = "";
    pImage = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *skillSetPathName(PyObject *poSelf, PyObject *poArgs)
{
    std::string szPathName;
    if (!PyTuple_GetString(poArgs, 0, szPathName))
        return Py_BadArgument();

    CPythonSkill::Instance().SetPathName(szPathName.c_str());
    Py_RETURN_NONE;
}

PyObject *skillRegisterSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
        return Py_BadArgument();

    std::string strFullFileName;
    strFullFileName = CPythonSkill::Instance().GetPathName();
    strFullFileName += szFileName;

    Py_RETURN_NONE;
}

PyObject *skillLoadSkillData(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *skillRegisterSkillDesc(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    if (!CPythonSkill::Instance().RegisterSkillDesc(szFileName.c_str()))
        return Py_BuildException("Failed to load Skill Desc");

    Py_RETURN_NONE;
}

PyObject *skillClearSkillData(PyObject *poSelf, PyObject *poArgs)
{
    CPythonSkill::SSkillData::ms_dwTimeIncreaseSkillNumber = 0;
    Py_RETURN_NONE;
}

PyObject *skillGetSkillName(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iSkillGrade = -1;
    PyTuple_GetInteger(poArgs, 1, &iSkillGrade);

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillName - Failed to find skill by %d", iSkillIndex);

    if (-1 != iSkillGrade)
        if (iSkillGrade >= 0 && iSkillGrade <= CPythonSkill::SKILL_GRADE_COUNT)
        {
            return Py_BuildValue("s", c_pSkillData->GradeData[std::min(2, iSkillGrade)].strName.c_str());
        }

    return Py_BuildValue("s", c_pSkillData->strName.c_str());
}

PyObject *skillGetSkillDescription(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillDescription - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("s", c_pSkillData->strDescription.c_str());
}

PyObject *skillGetSkillType(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillType - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->byType);
}

PyObject *skillGetSkillConditionDescriptionCount(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillConditionDescriptionCount - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->ConditionDataVector.size());
}

PyObject *skillGetSkillConditionDescription(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iConditionIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iConditionIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillConditionDescription() - Failed to find skill by %d", iSkillIndex);

    if (iConditionIndex >= c_pSkillData->ConditionDataVector.size())
        return Py_BuildValue("None");

    return Py_BuildValue("s", c_pSkillData->ConditionDataVector[iConditionIndex].c_str());
}

PyObject *skillGetSkillAffectDescriptionCount(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillAffectDescriptionCount - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->AffectDataVector.size());
}

PyObject *skillGetSkillAffectDescription(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iAffectIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iAffectIndex))
        return Py_BadArgument();

    float fSkillPoint;
    if (!PyTuple_GetFloat(poArgs, 2, &fSkillPoint))
        return Py_BadArgument();

    int iSkillLevel = 0;
    PyTuple_GetInteger(poArgs, 3, &iSkillLevel);

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillAffectDescription - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("s", c_pSkillData->GetAffectDescription(iAffectIndex, fSkillPoint, iSkillLevel));
}

PyObject *skillGetSkillLevelStep(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    float fSkillPoint;
    if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillLevelStep - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->GetSkillLevelStep(fSkillPoint));
}

PyObject *skillGetSkillCoolTime(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    float fSkillPoint;
    if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillCoolTime - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->GetSkillCoolTime(fSkillPoint));
}

PyObject *skillGetSkillNeedSP(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    float fSkillPoint;
    if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillNeedSP Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->GetNeedSP(fSkillPoint));
}

PyObject *skillGetSkillContinuationSP(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    float fSkillPoint;
    if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillContinuationSP - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->GetContinuationSP(fSkillPoint));
}

PyObject *skillGetSkillMaxLevel(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillMaxLevel - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->GetMaxLevel());
}

PyObject *skillGetSkillLevelUpPoint(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillLevelUpPoint - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->GetLevelUpPoint());
}

PyObject *skillGetSkillLevelLimit(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillLevelLimit - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->byLevelLimit);
}

PyObject *skillIsSkillRequirement(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.IsSkillRequirement - Failed to find skill by %d", iSkillIndex);

    if (!c_pSkillData->prerequisiteSkills.empty())
    {
        for (auto &requirement : c_pSkillData->prerequisiteSkills)
        {
            CPythonSkill::SSkillData *pRequireSkillData;
            if (!CPythonSkill::Instance().GetSkillData(requirement.skillVnum, &pRequireSkillData))
            {
                SPDLOG_ERROR("skill.IsSkillRequirement - Failed to find skill : [{0}/{1}] can't find [{2}]\n",
                              c_pSkillData->dwSkillIndex, c_pSkillData->strName.c_str(), requirement.skillVnum);
                return Py_BuildValue("i", FALSE);
            }

            uint32_t dwRequireSkillSlotIndex;
            if (!CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(
                pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex))
                return Py_BuildValue("i", FALSE);
        }
    }

    return Py_BuildValue("i", !c_pSkillData->prerequisiteSkills.empty());
}

PyObject *skillGetSkillRequirementData(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillRequirementData - Failed to find skill by %d", iSkillIndex);

    py::list l;
    for (auto &requirement : c_pSkillData->prerequisiteSkills)
    {
        CPythonSkill::SSkillData *pRequireSkillData;
        if (!CPythonSkill::Instance().GetSkillData(requirement.skillVnum, &pRequireSkillData))
            continue;

        int ireqLevel = (int)ceil(
            float(requirement.skillLevel) / float(std::max<uint8_t>(1, pRequireSkillData->byLevelUpPoint)));
        l.append(py::make_tuple(requirement.skillVnum, ireqLevel));
    }

    return Py_BuildValue("O", l.ptr());
}

PyObject *skillGetSkillRequireStatCount(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillRequireStatCount - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->RequireStatDataVector.size());
}

PyObject *skillGetSkillRequireStatData(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();
    int iStatIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iStatIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetSkillRequireStatData - Failed to find skill by %d", iSkillIndex);

    if (iStatIndex >= c_pSkillData->RequireStatDataVector.size())
        return Py_BuildValue("ii", 0, 0);

    const CPythonSkill::TRequireStatData &c_rRequireStatData = c_pSkillData->RequireStatDataVector[iStatIndex];

    return Py_BuildValue("ii", c_rRequireStatData.byPoint, c_rRequireStatData.byLevel);
}

PyObject *skillCanLevelUpSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();
    int iSkillLevel;
    if (!PyTuple_GetInteger(poArgs, 1, &iSkillLevel))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.CanLevelUpSkill - Failed to find skill by %d", iSkillIndex);

    if (iSkillLevel >= c_pSkillData->GetMaxLevel() && c_pSkillData->GetMaxLevel() != 1)
        return Py_BuildValue("i", FALSE);

    for (auto &requirement : c_pSkillData->prerequisiteSkills)
    {
        CPythonSkill::SSkillData *pRequireSkillData;
        if (CPythonSkill::Instance().GetSkillData(requirement.skillVnum, &pRequireSkillData))
        {
            uint32_t dwRequireSkillSlotIndex;
            if (CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(
                pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex))
            {
                int iSkillGrade = CPythonPlayer::Instance().GetSkillGrade(dwRequireSkillSlotIndex);
                int iSkillLevel = CPythonPlayer::Instance().GetSkillLevel(dwRequireSkillSlotIndex);
                if (iSkillGrade <= 0)
                    if (iSkillLevel < requirement.skillLevel)
                        return Py_BuildValue("i", FALSE);
            }
        }
    }

    for (uint32_t i = 0; i < c_pSkillData->RequireStatDataVector.size(); ++i)
    {
        const CPythonSkill::TRequireStatData &c_rRequireStatData = c_pSkillData->RequireStatDataVector[i];
        if (CPythonPlayer::Instance().GetStatus(c_rRequireStatData.byPoint) < c_rRequireStatData.byLevel)
            return Py_BuildValue("i", FALSE);
    }

    if (0 != (c_pSkillData->dwSkillAttribute & CPythonSkill::SKILL_ATTRIBUTE_CANNOT_LEVEL_UP))
        return Py_BuildValue("i", FALSE);

    return Py_BuildValue("i", TRUE);
}

PyObject *skillCheckRequirementSueccess(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iRequirementIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iRequirementIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.CheckRequirementSueccess - Failed to find skill by %d", iSkillIndex);

    auto &requirement = c_pSkillData->prerequisiteSkills[iRequirementIndex];

    CPythonSkill::SSkillData *pRequireSkillData;
    if (CPythonSkill::Instance().GetSkillData(requirement.skillVnum, &pRequireSkillData))
    {
        uint32_t dwRequireSkillSlotIndex;
        if (CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(pRequireSkillData->dwSkillIndex,
                                                                     &dwRequireSkillSlotIndex))
        {
            int iSkillLevel = CPythonPlayer::Instance().GetSkillLevel(dwRequireSkillSlotIndex);

           if (iSkillLevel < requirement.skillLevel)
              return Py_BuildValue("i", FALSE);
            
        }
    }

    return Py_BuildValue("i", TRUE);
}

PyObject *skillGetNeedCharacterLevel(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetNeedCharacterLevel - Failed to find skill by %d", iSkillIndex);

    std::vector<CPythonSkill::TRequireStatData>::iterator itor = c_pSkillData->RequireStatDataVector.begin();
    for (; itor != c_pSkillData->RequireStatDataVector.end(); ++itor)
    {
        const CPythonSkill::TRequireStatData &c_rRequireStatData = *itor;

        if (POINT_LEVEL == c_rRequireStatData.byPoint)
            return Py_BuildValue("i", c_rRequireStatData.byLevel);
    }

    return Py_BuildValue("i", 0);
}

PyObject *skillIsToggleSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.IsToggleSkill - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->IsToggleSkill());
}

PyObject *skillIsUseHPSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.IsUseHPSkill - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->IsUseHPSkill());
}

PyObject *skillIsStandingSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.IsStandingSkill - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->IsStandingSkill());
}

PyObject *skillCanUseSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.CanUseSkill - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->IsCanUseSkill());
}

PyObject *skillIsLevelUpSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.IsLevelUpSkill - Failed to find skill by %d", iSkillIndex);

    // ¸ðµÎ ·¹º§¾÷ °¡´É

    return Py_BuildValue("i", TRUE);
}

PyObject *skillGetIconName(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildValue("s", "");

    return Py_BuildValue("s", c_pSkillData->strIconFileName.c_str());
}

PyObject *skillGetIconImage(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildValue("s", ""); // ÀÍ¼Á¼ÇÀ» ³»´Â ´ë½Å 0À» ¸®ÅÏÇÑ´Ù.

    auto img = c_pSkillData->pImage;
    if (!img)
        return Py_BuildValue("s", ""); // ÀÍ¼Á¼ÇÀ» ³»´Â ´ë½Å 0À» ¸®ÅÏÇÑ´Ù.

    return Py_BuildValue("s", c_pSkillData->pImage->GetFileName());
}

PyObject *skillGetIconInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetIconInstance - Failed to find skill by %d", iSkillIndex);

    auto pImageInstance = new CGraphicImageInstance;
    pImageInstance->SetImagePointer(c_pSkillData->pImage);

    const auto capsule = PyCapsule_New(pImageInstance, nullptr, ImageCapsuleDestroyer);

    return capsule;
}

PyObject *skillGetIconImageNew(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iGradeIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildValue("s", ""); // ÀÍ¼Á¼ÇÀ» ³»´Â ´ë½Å 0À» ¸®ÅÏÇÑ´Ù.

    if (!c_pSkillData)
        return Py_BuildValue("s", ""); // ÀÍ¼Á¼ÇÀ» ³»´Â ´ë½Å 0À» ¸®ÅÏÇÑ´Ù.

    if (iGradeIndex < 0)
        iGradeIndex = 0;

    if (iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
        iGradeIndex = CPythonSkill::SKILL_GRADE_COUNT - 1;

    return Py_BuildValue("s", c_pSkillData->GradeData[std::min(2, iGradeIndex)].pImage->GetFileName());
}

PyObject *skillGetIconInstanceNew(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iGradeIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetIconInstanceNew - Failed to find skill by %d", iSkillIndex);

    //CGraphicImage::Ptr pImage = c_pSkillData->pImage;

    if (iGradeIndex < 0)
        iGradeIndex = 0;

    if (iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
        iGradeIndex = CPythonSkill::SKILL_GRADE_COUNT - 1;

    auto pImageInstance = new CGraphicImageInstance;
    pImageInstance->SetImagePointer(c_pSkillData->GradeData[std::min(2, iGradeIndex)].pImage);

    const auto capsule = PyCapsule_New(pImageInstance, nullptr, ImageCapsuleDestroyer);

    return capsule;
}

PyObject *skillGetGradeData(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iGradeIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetGradeData - Failed to find skill by %d", iSkillIndex);

    if (iGradeIndex < 0 || iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
        return Py_BuildException("Strange grade index [%d]", iSkillIndex, iGradeIndex);

    return Py_BuildValue("O", c_pSkillData->GradeData[std::min(3, iGradeIndex)]);
}

PyObject *skillGetNewAffectDataCount(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetNewAffectDataCount - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->AffectDataNewVector.size());
}

PyObject *skillGetNewAffectData(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    int iAffectIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iAffectIndex))
        return Py_BadArgument();

    float fSkillLevel;
    if (!PyTuple_GetFloat(poArgs, 2, &fSkillLevel))
        return Py_BadArgument();

    int iRealSkillLevel;
    if (!PyTuple_GetInteger(poArgs, 3, &iRealSkillLevel))
        return Py_BadArgument();

    CPythonSkill::SSkillData *pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &pSkillData))
        return Py_BuildException("skill.GetNewAffectData - Failed to find skill by %d", iSkillIndex);

    if (iAffectIndex < 0 || iAffectIndex >= pSkillData->AffectDataNewVector.size())
        return Py_BuildException(" skill.GetNewAffectData - Strange AffectIndex %d", iAffectIndex);

    CPythonSkill::TAffectDataNew &rAffectData = pSkillData->AffectDataNewVector[iAffectIndex];

    CPoly minPoly;
    CPoly maxPoly;
    minPoly.SetRandom(CPoly::RANDOM_TYPE_FORCE_MIN);
    maxPoly.SetRandom(CPoly::RANDOM_TYPE_FORCE_MAX);
    minPoly.SetStr(rAffectData.strPointPoly);
    maxPoly.SetStr(rAffectData.strPointPoly);
    float fMinValue = pSkillData->ProcessFormula(&minPoly, fSkillLevel, iRealSkillLevel, CPythonSkill::VALUE_TYPE_MIN);
    float fMaxValue = pSkillData->ProcessFormula(&maxPoly, fSkillLevel, iRealSkillLevel, CPythonSkill::VALUE_TYPE_MAX);

    return Py_BuildValue("sff", rAffectData.strPointType.c_str(), fMinValue, fMaxValue);
}

PyObject *skillGetDuration(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();
    float fSkillLevel;
    if (!PyTuple_GetFloat(poArgs, 1, &fSkillLevel))
        return Py_BadArgument();

    CPythonSkill::SSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
        return Py_BuildException("skill.GetDuration - Failed to find skill by %d", iSkillIndex);

    return Py_BuildValue("i", c_pSkillData->GetDuration(fSkillLevel));
}

PyObject *skillGetPetSkillInfo(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SPetSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetPetSkillInfo(iSkillIndex, &c_pSkillData))
        return Py_BuildValue("ssii", "", "", 0, 0);

    return Py_BuildValue("ssii", c_pSkillData->name.c_str(), c_pSkillData->description.c_str(), c_pSkillData->type,
                         c_pSkillData->cooltime);
}

PyObject *skillGetPetSkillIconImage(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SPetSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetPetSkillInfo(iSkillIndex, &c_pSkillData))
        return Py_BuildValue("s", ""); // ÀÍ¼Á¼ÇÀ» ³»´Â ´ë½Å 0À» ¸®ÅÏÇÑ´Ù.

    return Py_BuildValue("s", c_pSkillData->pImage->GetFileName());
}

PyObject *skillGetPetSkillIconPath(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BadArgument();

    CPythonSkill::SPetSkillData *c_pSkillData;
    if (!CPythonSkill::Instance().GetPetSkillInfo(iSkillIndex, &c_pSkillData))
        return Py_BuildValue("s", 0); // ÀÍ¼Á¼ÇÀ» ³»´Â ´ë½Å 0À» ¸®ÅÏÇÑ´Ù.

    return Py_BuildValue("s", c_pSkillData->icon.c_str());
}

extern "C" void initskill()
{
    static PyMethodDef s_methods[] =
    {
        {"SetPathName", skillSetPathName, METH_VARARGS},
        {"RegisterSkill", skillRegisterSkill, METH_VARARGS},
        {"LoadSkillData", skillLoadSkillData, METH_VARARGS},
        {"ClearSkillData", skillClearSkillData, METH_VARARGS},

        /////

        {"GetSkillName", skillGetSkillName, METH_VARARGS},
        {"GetSkillDescription", skillGetSkillDescription, METH_VARARGS},
        {"GetSkillType", skillGetSkillType, METH_VARARGS},
        {"GetSkillConditionDescriptionCount", skillGetSkillConditionDescriptionCount, METH_VARARGS},
        {"GetSkillConditionDescription", skillGetSkillConditionDescription, METH_VARARGS},
        {"GetSkillAffectDescriptionCount", skillGetSkillAffectDescriptionCount, METH_VARARGS},
        {"GetSkillAffectDescription", skillGetSkillAffectDescription, METH_VARARGS},
        {"GetSkillCoolTime", skillGetSkillCoolTime, METH_VARARGS},
        {"GetSkillLevelStep", skillGetSkillLevelStep, METH_VARARGS},

        {"GetSkillNeedSP", skillGetSkillNeedSP, METH_VARARGS},
        {"GetSkillContinuationSP", skillGetSkillContinuationSP, METH_VARARGS},
        {"GetSkillMaxLevel", skillGetSkillMaxLevel, METH_VARARGS},
        {"GetSkillLevelUpPoint", skillGetSkillLevelUpPoint, METH_VARARGS},
        {"GetSkillLevelLimit", skillGetSkillLevelLimit, METH_VARARGS},
        {"IsSkillRequirement", skillIsSkillRequirement, METH_VARARGS},
        {"GetSkillRequirementData", skillGetSkillRequirementData, METH_VARARGS},
        {"GetSkillRequireStatCount", skillGetSkillRequireStatCount, METH_VARARGS},
        {"GetSkillRequireStatData", skillGetSkillRequireStatData, METH_VARARGS},
        {"CanLevelUpSkill", skillCanLevelUpSkill, METH_VARARGS},
        {"IsLevelUpSkill", skillIsLevelUpSkill, METH_VARARGS},
        {"CheckRequirementSueccess", skillCheckRequirementSueccess, METH_VARARGS},
        {"GetNeedCharacterLevel", skillGetNeedCharacterLevel, METH_VARARGS},
        {"IsToggleSkill", skillIsToggleSkill, METH_VARARGS},
        {"IsUseHPSkill", skillIsUseHPSkill, METH_VARARGS},
        {"IsStandingSkill", skillIsStandingSkill, METH_VARARGS},
        {"CanUseSkill", skillCanUseSkill, METH_VARARGS},
        {"GetIconName", skillGetIconName, METH_VARARGS},
        {"GetIconImage", skillGetIconImage, METH_VARARGS},
        {"GetIconImageNew", skillGetIconImageNew, METH_VARARGS},
        {"GetIconInstance", skillGetIconInstance, METH_VARARGS},
        {"GetIconInstanceNew", skillGetIconInstanceNew, METH_VARARGS},
        {"GetGradeData", skillGetGradeData, METH_VARARGS},

        {"GetNewAffectDataCount", skillGetNewAffectDataCount, METH_VARARGS},
        {"GetNewAffectData", skillGetNewAffectData, METH_VARARGS},
        {"GetDuration", skillGetDuration, METH_VARARGS},

        {"GetPetSkillInfo", skillGetPetSkillInfo, METH_VARARGS},
        {"GetPetSkillIconImage", skillGetPetSkillIconImage, METH_VARARGS},
        {"GetPetSkillIconPath", skillGetPetSkillIconPath, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("skill", s_methods);
    PyModule_AddIntConstant(poModule, "SKILL_TYPE_NONE", CPythonSkill::SKILL_TYPE_NONE);
    PyModule_AddIntConstant(poModule, "SKILL_TYPE_ACTIVE", CPythonSkill::SKILL_TYPE_ACTIVE);
    PyModule_AddIntConstant(poModule, "SKILL_TYPE_SUPPORT", CPythonSkill::SKILL_TYPE_SUPPORT);
    PyModule_AddIntConstant(poModule, "SKILL_TYPE_GUILD", CPythonSkill::SKILL_TYPE_GUILD);
    PyModule_AddIntConstant(poModule, "SKILL_TYPE_HORSE", CPythonSkill::SKILL_TYPE_HORSE);
    PyModule_AddIntConstant(poModule, "SKILL_TYPE_MAX_NUM", CPythonSkill::SKILL_TYPE_MAX_NUM);

    PyModule_AddIntConstant(poModule, "SKILL_GRADE_COUNT", CPythonSkill::SKILL_GRADE_COUNT);
    PyModule_AddIntConstant(poModule, "SKILL_GRADE_STEP_COUNT", CPythonSkill::SKILL_GRADE_STEP_COUNT);
    PyModule_AddIntConstant(poModule, "SKILL_GRADEGAP", CPythonSkill::SKILL_GRADEGAP);
    PyModule_AddIntConstant(poModule, "SKILL_EFFECT_COUNT", CPythonSkill::SKILL_EFFECT_COUNT);
}
