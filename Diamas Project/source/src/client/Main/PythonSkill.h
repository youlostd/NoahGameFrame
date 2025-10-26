#pragma once

#include <Config.hpp>

#include "../gamelib/ItemData.h"
#include "../EterBase/Poly/Poly.h"
#include "game/SkillTypes.hpp"

class CInstanceBase;

class CPythonSkill : public CSingleton<CPythonSkill>
{
public:
    enum
    {
        SKILL_TYPE_NONE,
        SKILL_TYPE_ACTIVE,
        SKILL_TYPE_SUPPORT,
        SKILL_TYPE_GUILD,
        SKILL_TYPE_HORSE,
        SKILL_TYPE_MAX_NUM,
    };

    enum ESkillTableTokenType
    {
        TABLE_TOKEN_TYPE_VNUM,
        TABLE_TOKEN_TYPE_NAME,
        TABLE_TOKEN_TYPE_TYPE,
        TABLE_TOKEN_TYPE_LEVEL_STEP,
        TABLE_TOKEN_TYPE_MAX_LEVEL,
        TABLE_TOKEN_TYPE_LEVEL_LIMIT,
        TABLE_TOKEN_TYPE_POINT_ON,
        TABLE_TOKEN_TYPE_POINT_POLY,
        TABLE_TOKEN_TYPE_SP_COST_POLY,
        TABLE_TOKEN_TYPE_DURATION_POLY,
        TABLE_TOKEN_TYPE_DURATION_SP_COST_POLY,
        TABLE_TOKEN_TYPE_COOLDOWN_POLY,
        TABLE_TOKEN_TYPE_MASTER_BONUS_POLY,
        TABLE_TOKEN_TYPE_ATTACK_GRADE_POLY,
        TABLE_TOKEN_TYPE_FLAG,
        TABLE_TOKEN_TYPE_AFFECT_FLAG,
        TABLE_TOKEN_TYPE_POINT_ON2,
        TABLE_TOKEN_TYPE_POINT_POLY2,
        TABLE_TOKEN_TYPE_DURATION_POLY2,
        TABLE_TOKEN_TYPE_AFFECT_FLAG2,
        TABLE_TOKEN_TYPE_PREREQUISITE_SKILL_VNUM,
        TABLE_TOKEN_TYPE_PREREQUISITE_SKILL_LEVEL,
        TABLE_TOKEN_TYPE_PREREQUISITE_SKILL_VNUM2,
        TABLE_TOKEN_TYPE_PREREQUISITE_SKILL_LEVEL2,
        TABLE_TOKEN_TYPE_SKILL_TYPE,
        TABLE_TOKEN_TYPE_MAX_HIT,
        TABLE_TOKEN_TYPE_SPLASH_AROUND_DAMAGE_ADJUST_POLY,
        TABLE_TOKEN_TYPE_TARGET_RANGE,
        TABLE_TOKEN_TYPE_SPLASH_RANGE,
        TABLE_TOKEN_TYPE_MAX_NUM,
    };

    enum ESkillDescTokenType
    {
        DESC_TOKEN_TYPE_VNUM,
        DESC_TOKEN_TYPE_JOB,
        DESC_TOKEN_TYPE_NAME1,
        DESC_TOKEN_TYPE_NAME2,
        DESC_TOKEN_TYPE_NAME3,
        DESC_TOKEN_TYPE_DESCRIPTION,
        DESC_TOKEN_TYPE_CONDITION1,
        DESC_TOKEN_TYPE_CONDITION2,
        DESC_TOKEN_TYPE_CONDITION3,
        DESC_TOKEN_TYPE_CONDITION4,
        DESC_TOKEN_TYPE_ATTRIBUTE,
        DESC_TOKEN_TYPE_WEAPON,
        DESC_TOKEN_TYPE_ICON_NAME,
        DESC_TOKEN_TYPE_MOTION_INDEX,
        DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM,
        DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA,
        DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA,
        DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_1,
        DESC_TOKEN_TYPE_AFFECT_MIN_1,
        DESC_TOKEN_TYPE_AFFECT_MAX_1,
        DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_2,
        DESC_TOKEN_TYPE_AFFECT_MIN_2,
        DESC_TOKEN_TYPE_AFFECT_MAX_2,
        DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_3,
        DESC_TOKEN_TYPE_AFFECT_MIN_3,
        DESC_TOKEN_TYPE_AFFECT_MAX_3,
        DESC_TOKEN_TYPE_LEVEL_LIMIT,
        DESC_TOKEN_TYPE_MAX_LEVEL,
        DESC_TOKEN_TYPE_MAX_NUM,

        CONDITION_COLUMN_COUNT = 3,
        AFFECT_COLUMN_COUNT = 3,
        AFFECT_STEP_COUNT = 3,
    };

    enum
    {
        SKILL_ATTRIBUTE_NEED_TARGET = (1 << 0),
        SKILL_ATTRIBUTE_TOGGLE = (1 << 1),
        SKILL_ATTRIBUTE_WEAPON_LIMITATION = (1 << 2),
        SKILL_ATTRIBUTE_MELEE_ATTACK = (1 << 3),
        SKILL_ATTRIBUTE_USE_HP = (1 << 4),
        SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION = (1 << 5),
        SKILL_ATTRIBUTE_STANDING_SKILL = (1 << 6),
        SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE = (1 << 7),
        SKILL_ATTRIBUTE_CAN_USE_FOR_ME = (1 << 8),
        SKILL_ATTRIBUTE_NEED_CORPSE = (1 << 9),
        SKILL_ATTRIBUTE_FAN_RANGE = (1 << 10),
        SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH = (1 << 11),
        SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE = (1 << 12),
        SKILL_ATTRIBUTE_NEED_POISON_BOTTLE = (1 << 13),
        SKILL_ATTRIBUTE_ATTACK_SKILL = (1 << 14),
        SKILL_ATTRIBUTE_TIME_INCREASE_SKILL = (1 << 15),
        // ÁõÁö¼ú Àü¿ë attribute
        SKILL_ATTRIBUTE_CHARGE_ATTACK = (1 << 16),
        SKILL_ATTRIBUTE_PASSIVE = (1 << 17),
        SKILL_ATTRIBUTE_CANNOT_LEVEL_UP = (1 << 18),
        SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR = (1 << 19),
        SKILL_ATTRIBUTE_MOVING_SKILL = (1 << 20),
        SKILL_ATTRIBUTE_HORSE_SKILL = (1 << 21),
        SKILL_ATTRIBUTE_CIRCLE_RANGE = (1 << 22),
        SKILL_ATTRIBUTE_SEARCH_TARGET = (1 << 23),
    };

    enum
    {
        SKILL_NEED_WEAPON_SWORD = (1 << WEAPON_SWORD),
        SKILL_NEED_WEAPON_DAGGER = (1 << WEAPON_DAGGER),
        SKILL_NEED_WEAPON_BOW = (1 << WEAPON_BOW),
        SKILL_NEED_WEAPON_TWO_HANDED = (1 << WEAPON_TWO_HANDED),
        SKILL_NEED_WEAPON_DOUBLE_SWORD = (1 << WEAPON_DAGGER),
        SKILL_NEED_WEAPON_BELL = (1 << WEAPON_BELL),
        SKILL_NEED_WEAPON_FAN = (1 << WEAPON_FAN),
        SKILL_NEED_WEAPON_ARROW = (1 << WEAPON_ARROW),
        SKILL_NEED_WEAPON_CLAW = (1 << WEAPON_CLAW),
        SKILL_NEED_WEAPON_EMPTY_HAND = (1 << WEAPON_NONE),
    };

    enum
    {
        VALUE_TYPE_FREE,
        VALUE_TYPE_MIN,
        VALUE_TYPE_MAX,
    };

    enum
    {
        SKILL_GRADE_COUNT = 4,
        SKILL_EFFECT_COUNT = 4,
        //스킬은 초,중,고급 이외에 마스터일때 이펙트 추가.
        SKILL_GRADE_STEP_COUNT = 20,
        SKILL_GRADEGAP = 25,
    };

    typedef struct SAffectData
    {
        std::string strAffectDescription;
        std::string strAffectMinFormula;
        std::string strAffectMaxFormula;
    } TAffectData;

    typedef struct SAffectDataNew
    {
        std::string strPointType;
        std::string strPointPoly;
    } TAffectDataNew;

    typedef struct SRequireStatData
    {
        uint8_t byPoint;
        uint8_t byLevel;
    } TRequireStatData;

    typedef struct SGradeData
    {
        std::string strName;
        CGraphicImage::Ptr pImage;
        uint16_t wMotionIndex;
    } TGradeData;

    struct PrerequisiteSkillData
    {
        PrerequisiteSkillData(uint8_t skillIndex, uint8_t skillLevel)
            : skillIndex{skillIndex},
              skillLevel{skillLevel}
        {
        }

        uint8_t skillIndex;
        uint8_t skillLevel;
    };

    typedef struct SSkillData
    {
        static uint32_t MELEE_SKILL_TARGET_RANGE;
        // Functions
        SSkillData();
        uint32_t GetTargetRange() const;
        bool CanChangeDirection();
        bool IsFanRange();
        bool IsCircleRange();
        bool IsAutoSearchTarget();
        bool IsNeedTarget();
        bool IsNeedCorpse();
        bool IsToggleSkill();
        bool IsUseHPSkill();
        bool IsStandingSkill();
        bool CanUseWeaponType(uint32_t dwWeaponType);
        bool IsOnlyForAlliance();
        bool CanUseForMe();
        bool CanUseIfNotEnough();
        bool IsNeedEmptyBottle();
        bool IsNeedPoisonBottle();
        bool IsNeedBow();
        bool IsAttackSkill();
        bool IsHorseSkill();
        bool IsMovingSkill();
        bool IsTimeIncreaseSkill();
        bool IsMeleeSkill();
        bool IsChargeSkill();
        bool IsOnlyForGuildWar();

        bool GetState(const char *c_szStateName, int *piState, int iMinMaxType = VALUE_TYPE_FREE);
        float ProcessFormula(CPoly *pPoly, float fSkillLevel = 0.0f, int iSkillActualLevel = 0,
                             int iMinMaxType = VALUE_TYPE_FREE);
        const char *GetAffectDescription(uint32_t dwIndex, float fSkillLevel, int iActualSkillLevel);
        uint32_t GetSkillLevelStep(float fSkillPoint);
        uint32_t GetSkillCoolTime(float fSkillPoint);
        int GetNeedSP(float fSkillPoint);
        uint32_t GetContinuationSP(float fSkillPoint);
        uint32_t GetMotionLoopCount(float fSkillPoint);
        uint32_t GetTargetCount(float fSkillPoint);
        uint32_t GetDuration(float fSkillPoint);
        uint32_t GetSkillMotionIndex(int iGrade = -1);
        uint8_t GetMaxLevel();
        uint8_t GetLevelUpPoint();
        bool IsCanUseSkill();
        bool IsNoMotion();

        const std::string GetName() const;
        uint8_t GetType() const;

        ///////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////

        // Variable
        uint8_t byType;
        uint32_t dwSkillIndex;
        uint8_t byMaxLevel;
        uint8_t byLevelUpPoint;
        uint8_t byLevelLimit;
        bool bNoMotion;

        std::string strName;
        std::string strIconFileName;
        std::string strDescription;
        std::string strMotionScriptFileName;

        std::vector<std::string> ConditionDataVector;
        std::vector<TAffectData> AffectDataVector;
        std::vector<TAffectDataNew> AffectDataNewVector;
        std::vector<TRequireStatData> RequireStatDataVector;
        std::string strCoolTimeFormula;
        std::string strTargetCountFormula;
        std::string strMotionLoopCountFormula;
        std::string strNeedSPFormula;
        std::string strContinuationSPFormula;
        std::string strDuration;
        std::string strLevelStepFormula;

        uint32_t dwSkillAttribute;
        uint32_t dwNeedWeapon;
        uint32_t dwTargetRange;
        uint16_t wMotionIndex;      // ¾ø¾îÁú º¯¼ö
        uint16_t wMotionIndexForMe; // ¾ø¾îÁú º¯¼ö

        std::vector<SkillRequirement> prerequisiteSkills;

        TGradeData GradeData[SKILL_EFFECT_COUNT] = {};

        CGraphicImage::Ptr pImage;

        /////

        static std::map<std::string, uint32_t> ms_StatusNameMap;
        static std::map<std::string, uint32_t> ms_NewMinStatusNameMap;
        static std::map<std::string, uint32_t> ms_NewMaxStatusNameMap;
        static uint32_t ms_dwTimeIncreaseSkillNumber;
    } TSkillData;

    enum EPetSkillTokenType
    {
        PET_SKILL_TOKEN_VNUM,
        PET_SKILL_TOKEN_NAME,
        PET_SKILL_TOKEN_ICON,
        PET_SKILL_TOKEN_SKILL_TYPE,
        PET_SKILL_TOKEN_DESC,
        PET_SKILL_TOKEN_COOLTIME,
        PET_SKILL_TOKEN_MAX_NUM,
    };

    enum EPetSkillType
    {
        PET_SKILL_TYPE_NONE,
        PET_SKILL_TYPE_PASSIVE,
        PET_SKILL_TYPE_AUTO,
    };

    typedef struct SPetSkillData
    {
        uint32_t skillId;
        std::string name;
        std::string icon;
        uint8_t type;
        std::string description;
        uint32_t cooltime;

        CGraphicImage::Ptr pImage;
    } TPetSkillData;

    using TSkillDataMap = std::map<uint32_t, TSkillData>;
    using TPetSkillDataMap = std::map<uint32_t, TPetSkillData>;
    using TSkillMotionIndexMap = std::map<uint32_t, uint32_t>;
    using TPathNameMap = std::map<std::string, std::string>;

public:
    CPythonSkill();
    virtual ~CPythonSkill();

    void Destroy();
    bool GetPetSkillInfo(uint32_t dwSkillIndex, TPetSkillData **ppSkillData);
    bool RegisterPetSkill(const char *c_szFileName);

    bool RegisterSkillTable(const char *c_szFileName);
    bool RegisterSkillDesc(const char *c_szFileName);
    bool GetSkillData(uint32_t dwSkillIndex, TSkillData **ppSkillData);
    bool GetSkillDataByName(const char *c_szName, TSkillData **ppSkillData);

    void SetPathName(const char *c_szFileName);
    const char *GetPathName();

protected:
    void __RegisterGradeIconImage(TSkillData &rData, const char *c_szHeader, const char *c_szImageName);
    void __RegisterDirectIconImage(TSkillData &rData, const char *c_szImage);
    void __RegisterNormalIconImage(TSkillData &rData, const char *c_szHeader, const char *c_szImageName);
    void __RegisterPetSkillIcon(TPetSkillData &rData, const char *c_szHeader, const char *c_szImageName);

protected:
    TSkillDataMap m_SkillDataMap;
    TPetSkillDataMap m_PetSkillDataMap;

    std::string m_strPathName;

    std::map<std::string, uint32_t> m_SkillTypeIndexMap;
    std::map<std::string, uint32_t> m_SkillAttributeIndexMap;
    std::map<std::string, uint32_t> m_SkillNeedWeaponIndexMap;
    std::map<std::string, uint32_t> m_SkillWeaponTypeIndexMap;
    TPathNameMap m_PathNameMap;
};
