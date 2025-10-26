#ifndef METIN2_SERVER_GAME_CONSTANTS_H
#define METIN2_SERVER_GAME_CONSTANTS_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/MobConstants.hpp>
#include <game/DbPackets.hpp>

enum EMonsterChatState
{
    MONSTER_CHAT_WAIT,
    MONSTER_CHAT_ATTACK,
    MONSTER_CHAT_CHASE,
    MONSTER_CHAT_ATTACKED,
};

typedef struct SMobRankStat
{
    int iGoldPercent; // µ·ÀÌ ³ª¿Ã È®·ü
} TMobRankStat;

typedef struct SMobStat
{
    uint8_t byLevel;
    uint16_t HP;
    uint32_t dwExp;
    uint16_t wDefGrade;
} TMobStat;

typedef struct SBattleTypeStat
{
    int AttGradeBias;
    int DefGradeBias;
    int MagicAttGradeBias;
    int MagicDefGradeBias;
} TBattleTypeStat;

typedef struct SJobInitialPoints
{
    short st, ht, dx, iq;
    int max_hp, max_sp;
    int hp_per_ht, sp_per_iq;
    int hp_per_lv_begin, hp_per_lv_end;
    int sp_per_lv_begin, sp_per_lv_end;
    int max_stamina;
    int stamina_per_con;
    int stamina_per_lv_begin, stamina_per_lv_end;
    int create_lv;
    int start_lv;
    uint32_t create_pos[3][3];
} TJobInitialPoints;

typedef struct __coord
{
    int x, y;
} Coord;

enum
{
    FORTUNE_BIG_LUCK,
    FORTUNE_LUCK,
    FORTUNE_SMALL_LUCK,
    FORTUNE_NORMAL,
    FORTUNE_SMALL_BAD_LUCK,
    FORTUNE_BAD_LUCK,
    FORTUNE_BIG_BAD_LUCK,
    FORTUNE_MAX_NUM,
};

const int STONE_INFO_MAX_NUM = 21;
const int STONE_LEVEL_MAX_NUM = 4;

struct SStoneDropInfo
{
    uint32_t dwMobVnum;
    int iDropPct;
    int iLevelPct[STONE_LEVEL_MAX_NUM + 1];
};

inline bool operator <(const SStoneDropInfo &l, uint32_t r) { return l.dwMobVnum < r; }

inline bool operator <(uint32_t l, const SStoneDropInfo &r) { return l < r.dwMobVnum; }

inline bool operator <(const SStoneDropInfo &l, const SStoneDropInfo &r) { return l.dwMobVnum < r.dwMobVnum; }

extern const TMobRankStat MobRankStats[MOB_RANK_MAX_NUM];

extern TBattleTypeStat BattleTypeStats[BATTLE_TYPE_MAX_NUM];

extern const uint32_t party_exp_distribute_table[PLAYER_MAX_LEVEL_CONST + 1];

extern const uint32_t exp_table[PLAYER_EXP_TABLE_MAX + 1];

extern const uint32_t guild_exp_table[GUILD_MAX_LEVEL + 1];
extern const uint32_t guild_exp_table2[GUILD_MAX_LEVEL + 1];

#define MAX_EXP_DELTA_OF_LEV	31
#define PERCENT_LVDELTA(me, victim) aiPercentByDeltaLev[std::clamp((victim + 15) - me, 0, MAX_EXP_DELTA_OF_LEV - 1)]
#define PERCENT_LVDELTA_BOSS(me, victim) aiPercentByDeltaLevForBoss[std::clamp((victim + 15) - me, 0, MAX_EXP_DELTA_OF_LEV - 1)]
#define CALCULATE_VALUE_LVDELTA(me, victim, val) ((val * PERCENT_LVDELTA(me, victim)) / 100)
extern const int aiPercentByDeltaLev[MAX_EXP_DELTA_OF_LEV];
extern const int aiPercentByDeltaLevForBoss[MAX_EXP_DELTA_OF_LEV];

#define ARROUND_COORD_MAX_NUM	161
extern Coord aArroundCoords[ARROUND_COORD_MAX_NUM];
extern TJobInitialPoints JobInitialPoints[JOB_MAX_NUM];

extern const int MAX_DEFENSE_PCT;
extern const int aiMobEnchantApplyIdx[MOB_ENCHANTS_MAX_NUM];
extern const int aiMobResistsApplyIdx[MOB_RESISTS_MAX_NUM];

extern const int aSkillAttackAffectProbByRank[MOB_RANK_MAX_NUM];

extern const int aiItemMagicAttributePercentHigh[ITEM_ATTRIBUTE_MAX_LEVEL]; // 1°³±îÁö
extern const int aiItemMagicAttributePercentLow[ITEM_ATTRIBUTE_MAX_LEVEL];

extern int aiItemAttributeAddPercent[ITEM_ATTRIBUTE_MAX_NUM];

extern const int aiWeaponSocketQty[WEAPON_NUM_TYPES];
extern const int aiArmorSocketQty[ARMOR_NUM_TYPES];
extern const int aiSocketPercentByQty[5][4];

extern const int aiExpLossPercents[PLAYER_EXP_TABLE_MAX + 1];

extern const int *aiSkillPowerByLevel;
extern const int aiSkillPowerByLevel_euckr[SKILL_MAX_LEVEL + 1];

extern const int aiPolymorphPowerByLevel[SKILL_MAX_LEVEL + 1];

extern const int aiGrandMasterSkillBookCountForLevelUp[10];
extern const int aiGrandMasterSkillBookMinCount[10];
extern const int aiGrandMasterSkillBookMaxCount[10];
extern const int CHN_aiPartyBonusExpPercentByMemberCount[17];
extern const int KOR_aiPartyBonusExpPercentByMemberCount[17];
extern const int KOR_aiUniqueItemPartyBonusExpPercentByMemberCount[17];

extern const int aiChainLightningCountBySkillLevel[SKILL_MAX_LEVEL + 1];
extern const char *c_apszEmpireNames[EMPIRE_MAX_NUM];
extern const char *c_apszEmpireNamesAlt[EMPIRE_MAX_NUM];
extern const char *c_apszEmpireColor[EMPIRE_MAX_NUM];
extern const char *c_apszPrivNames[MAX_PRIV_NUM];
extern const char *c_apszPremiumNames[PREMIUM_MAX_NUM];

extern const SStoneDropInfo aStoneDrop[STONE_INFO_MAX_NUM];

typedef struct
{
    long lMapIndex;
    Gold iWarPrice;
    int iWinnerPotionRewardPctToWinner;
    int iLoserPotionRewardPctToWinner;
    int iInitialScore;
    int iEndScore;
} TGuildWarInfo;

extern TGuildWarInfo KOR_aGuildWarInfo[GUILD_WAR_TYPE_MAX_NUM];

// ACCESSORY_REFINE
enum
{
    ITEM_ACCESSORY_SOCKET_MAX_NUM = 3
};

extern const int aiAccessorySocketAddPct[ITEM_ACCESSORY_SOCKET_MAX_NUM];
extern const int aiAccessorySocketEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
extern const int aiAccessorySocketPermEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];

extern const int aiAccessorySocketDegradeTime[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
extern const int aiAccessorySocketPutPct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
long FN_get_apply_type(const char *apply_type_string);
const char *FN_get_apply_string(uint8_t bType);

// END_OF_ACCESSORY_REFINE
uint32_t GetGuildLevelExp(uint32_t level);

uint8_t GetApplyPoint(uint8_t applyType);
uint8_t GetPointApply(uint8_t pointType);
int GetApplyTypeValueByString(const std::string &inputString);
/// Get the minimum belt grade required for |slot|
const uint32_t *GetUsableSkillList(uint8_t bJob, uint8_t bSkillGroup);

#endif /* METIN2_SERVER_GAME_CONSTANTS_H */
