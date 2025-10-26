#ifndef METIN2_SERVER_GAME_SKILL_H
#define METIN2_SERVER_GAME_SKILL_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/SkillConstants.hpp>

#include <thecore/poly.hpp>
#include <base/Singleton.hpp>

#include <storm/String.hpp>

#include <unordered_map>
#include <memory>

#include "game/SkillTypes.hpp"
#include <base/robin_hood.h>

struct CSkillProto
{
    storm::String name;
    uint32_t dwVnum; // 번호

    uint32_t dwType;     // 0: 전직업, 1: 무사, 2: 자객, 3: 수라, 4: 무당
    uint8_t bMaxLevel;   // 최대 수련도
    uint8_t bLevelLimit; // 레벨제한
    int iSplashRange;    // 스플래쉬 거리 제한

    uint8_t bPointOn; // 어디에 결과값을 적용 시키는가? (타격치, MAX HP, HP REGEN 등등등)
    CPoly kPointPoly; // 결과값 만드는 공식

    CPoly kSPCostPoly;                   // 사용 SP 공식
    CPoly kDurationPoly;                 // 지속 시간 공식
    CPoly kDurationSPCostPoly;           // 지속 SP 공식
    CPoly kCooldownPoly;                 // 쿨다운 시간 공식
    CPoly kMasterBonusPoly;              // 마스터일 때 보너스 공식
    CPoly kSplashAroundDamageAdjustPoly; // 스플래쉬 공격일 경우 주위 적에게 입히는 데미지 감소 비율

    // Note: Affect-flags here are "normalized"
    // (i.e. value 0 really represents bit 0, while AFF_NONE is used
    // for "no affect-flag")

    uint32_t dwFlag;       // 스킬옵션
    uint32_t dwAffectFlag; // 스킬에 맞은 경우 적용되는 Affect

    CPoly levelStep; // 한번에 올리는데 필요한 스킬 포인트 수

    int32_t lMaxHit;

    uint8_t bSkillAttrType;

    // 2차 적용
    uint8_t bPointOn2;
    CPoly kPointPoly2;
    CPoly kDurationPoly2;
    uint32_t dwFlag2;
    uint32_t dwAffectFlag2;

    uint32_t dwTargetRange;

    bool IsChargeSkill() const { return dwVnum == SKILL_TANHWAN || dwVnum == SKILL_HORSE_CHARGE; }

    // 3차 적용
    uint8_t bPointOn3;
    CPoly kPointPoly3;
    CPoly kDurationPoly3;

    CPoly kGrandMasterAddSPCostPoly;

    std::vector<SkillRequirement> requirements;

    void SetPointVar(const std::string &strName, double dVar);
    void SetDurationVar(const std::string &strName, double dVar);
    void SetSPCostVar(const std::string &strName, double dVar);
};

class CSkillManager : public singleton<CSkillManager>
{
public:
    CSkillManager();
    ~CSkillManager();

    bool Initialize();

    CSkillProto *Get(uint32_t dwVnum);
    CSkillProto *Get(const storm::StringRef &name);

protected:
    typedef robin_hood::unordered_map<
        uint32_t,
        std::unique_ptr<CSkillProto>
    > ProtoMap;

    ProtoMap m_protoMap;
};

#endif /* METIN2_SERVER_GAME_SKILL_H */
