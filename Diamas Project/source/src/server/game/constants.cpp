#include "char.h"
#include <boost/algorithm/clamp.hpp>
#include <game/MotionConstants.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "skill.h"
TJobInitialPoints JobInitialPoints[JOB_MAX_NUM] =
    /*
       {
       int st, ht, dx, iq;
       int max_hp, max_sp;
       int hp_per_ht, sp_per_iq;
       int hp_per_lv_begin, hp_per_lv_end;
       int sp_per_lv_begin, sp_per_lv_end;
       int max_stamina;
       int stamina_per_con;
       int stamina_per_lv_begin, stamina_per_lv_end;
       }
     */
    {
        // TODO: Clean this up
        // str con dex int max HP max SP  CON/HP INT/SP  HP per lv  MP per lv  max stam  stam/con stam/lv create lv   start lv   start pos
        {
            6, 4, 3, 3, 700, 200, 40, 20, 44, 44, 22, 22, 1600, 50, 3, 3, 0, 99,
            {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
        }, // JOB_WARRIOR 16
        {
            4, 3, 6, 3, 700, 200, 40, 20, 44, 44, 22, 22, 1600, 50, 3, 3, 0, 99,
            {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
        }, // JOB_ASSASSIN 16
        {
            5, 3, 3, 5, 700, 200, 40, 20, 44, 44, 22, 22, 1600, 50, 3, 3, 0, 99,
            {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
        }, // JOB_SURA 16
        {
            3, 4, 3, 6, 700, 200, 40, 20, 44, 44, 22, 22, 1600, 50, 3, 3, 0, 99,
            {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
        }, // JOB_SHAMANa 16
        {
            2, 6, 6, 2, 700, 200, 40, 20, 44, 44, 22, 22, 1600, 50, 3, 3, 0, 1, {
                {91, 37300, 36400}, // TODO(noff): TRANSLATE COORDINATES
                {92, 37300, 36400},
                {93, 37300, 36400}
            }
        }, // JOB_WOLFMAN  16
    };

const TMobRankStat MobRankStats[MOB_RANK_MAX_NUM] =
    /*
       {
       int         iGoldPercent;
       }
     */
    {
        {20,},  // MOB_RANK_PAWN, 
        {20,},  // MOB_RANK_S_PAWN, 
        {25,},  // MOB_RANK_KNIGHT, 
        {30,},  // MOB_RANK_S_KNIGHT, 
        {100,}, // MOB_RANK_BOSS, 
        {100,}, // MOB_RANK_KING,
        {150,}  // MOB_RANK_LEGENDARY,
    };

TBattleTypeStat BattleTypeStats[BATTLE_TYPE_MAX_NUM] =
    /*
       {
       int         AttGradeBias;
       int         DefGradeBias;
       int         MagicAttGradeBias;
       int         MagicDefGradeBias;
       }
     */
    {
        {0, 0, 0, -10},      // BATTLE_TYPE_MELEE, 
        {10, -20, -10, -15}, // BATTLE_TYPE_RANGE,
        {-5, -5, 10, 10},    // BATTLE_TYPE_MAGIC,  
        {0, 0, 0, 0},        // BATTLE_TYPE_SPECIAL,
        {10, -10, 0, -15},   // BATTLE_TYPE_POWER,  
        {-10, 10, -10, 0},   // BATTLE_TYPE_TANKER,
        {20, -20, 0, -10},   // BATTLE_TYPE_SUPER_POWER,
        {-20, 20, -10, 0},   // BATTLE_TYPE_SUPER_TANKER,
    };

const uint32_t exp_table[PLAYER_EXP_TABLE_MAX + 1] =
{
    0, //	0
    300,
    800,
    1500,
    2500,
    4300,
    7200,
    11000,
    17000,
    24000,
    33000, //	10
    43000,
    58000,
    76000,
    100000,
    130000,
    169000,
    219000,
    283000,
    365000,
    472000, //	20
    610000,
    705000,
    813000,
    937000,
    1077000,
    1237000,
    1418000,
    1624000,
    1857000,
    2122000, //	30
    2421000,
    2761000,
    3145000,
    3580000,
    4073000,
    4632000,
    5194000,
    5717000,
    6264000,
    6837000, //	40
    7600000,
    8274000,
    8990000,
    9753000,
    10560000,
    11410000,
    12320000,
    13270000,
    14280000,
    15340000, //	50
    16870000,
    18960000,
    19980000,
    21420000,
    22930000,
    24530000,
    26200000,
    27960000,
    29800000,
    32780000, //	60
    36060000,
    39670000,
    43640000,
    48000000,
    52800000,
    58080000,
    63890000,
    70280000,
    77310000,
    85040000, //	70
    93540000,
    102900000,
    113200000,
    124500000,
    137000000,
    150700000,
    165700000,
    236990000,
    260650000,
    286780000, //	80
    315380000,
    346970000,
    381680000,
    419770000,
    461760000,
    508040000,
    558740000,
    614640000,
    676130000,
    743730000, //	90
    1041222000,
    1145344200,
    1259878620,
    1385866482,
    1524453130,
    1676898443,
    1844588288,
    2029047116,
    2050000000, //	99레벨 일 때 필요경험치 (100레벨이 되기 위한)
    2150000000, //	100
    2210000000,
    2250000000,
    2280000000,
    2310000000,
    2330000000, //	105
    2350000000,
    2370000000,
    2390000000,
    2400000000,
    2410000000, //	110
    2420000000,
    2430000000,
    2440000000,
    2450000000,
    2460000000, //	115
    2470000000,
    2480000000,
    2490000000,
    2490000000,
    2500000000, //	120
    2500000000,
    2500000000,
    2500000000,
    2500000000,
    2500000000, //	125
    2500000000,
    2500000000,
    2500000000,
    2500000000,
    2500000000, //	130
    2500000000,
    2500000000,
    2500000000,
    2500000000,
    2500000000, //	135
    3000000000,
    3000000000,
    3000000000,
    3000000000,
    3000000000, //	140
    3000000000,
    3000000000,
    3000000000,
    3000000000,
    3000000000, //	145
    3000000000,
    3000000000,
    3000000000,
    3000000000,
    3000000000, //	150
    3000000000,
    3000000000,
    3000000000,
    3000000000,
    3000000000, //	155
    3000000000,
    3000000000,
    3000000000,
    3000000000,
    3500000000, //	160
    3500000000,
    3500000000,
    3500000000,
    3500000000,
    3500000000, //	165
    4000000000,
    4000000000,
    4000000000,
    4000000000,
    4000000000, //	170
};

// 적과 나와의 레벨차이에 의한 계산에 사용되는 테이블
// MIN(MAX_EXP_DELTA_OF_LEV - 1, (적렙 + 15) - 내렙))
const int aiPercentByDeltaLevForBoss[MAX_EXP_DELTA_OF_LEV] =
{
    1,   // -15  0
    3,   // -14  1
    5,   // -13  2
    7,   // -12  3
    15,  // -11  4
    30,  // -10  5
    60,  // -9   6
    90,  // -8   7
    91,  // -7   8
    92,  // -6   9
    93,  // -5   10
    94,  // -4   11
    95,  // -3   12
    97,  // -2   13
    99,  // -1   14
    100, // 0    15
    105, // 1    16
    110, // 2    17
    115, // 3    18
    120, // 4    19
    125, // 5    20
    130, // 6    21
    135, // 7    22
    140, // 8    23
    145, // 9    24
    150, // 10   25
    155, // 11   26
    160, // 12   27
    165, // 13   28
    170, // 14   29
    180  // 15   30
};

const int aiPercentByDeltaLev[MAX_EXP_DELTA_OF_LEV] =
{
    1,   //  -15 0
    5,   //  -14 1
    10,  //  -13 2
    20,  //  -12 3
    30,  //  -11 4
    50,  //  -10 5
    70,  //  -9  6
    80,  //  -8  7
    85,  //  -7  8
    90,  //  -6  9
    92,  //  -5  10
    94,  //  -4  11
    96,  //  -3  12
    98,  //  -2  13
    100, //  -1  14
    100, //  0   15
    105, //  1   16
    110, //  2   17
    115, //  3   18
    120, //  4   19
    125, //  5   20
    130, //  6   21
    135, //  7   22
    140, //  8   23
    145, //  9   24
    150, //  10  25
    155, //  11  26
    160, //  12  27
    165, //  13  28
    170, //  14  29
    180, //  15  30
};

const uint32_t party_exp_distribute_table[PLAYER_MAX_LEVEL_CONST + 1] =
{
    0,
    10, 10, 10, 10, 15, 15, 20, 25, 30, 40,                               // 1 - 10
    50, 60, 80, 100, 120, 140, 160, 184, 210, 240,                        // 11 - 20
    270, 300, 330, 360, 390, 420, 450, 480, 510, 550,                     // 21 - 30
    600, 640, 700, 760, 820, 880, 940, 1000, 1100, 1180,                  // 31 - 40
    1260, 1320, 1380, 1440, 1500, 1560, 1620, 1680, 1740, 1800,           // 41 - 50
    1860, 1920, 2000, 2100, 2200, 2300, 2450, 2600, 2750, 2900,           // 51 - 60
    3050, 3200, 3350, 3500, 3650, 3800, 3950, 4100, 4250, 4400,           // 61 - 70
    4600, 4800, 5000, 5200, 5400, 5600, 5800, 6000, 6200, 6400,           // 71 - 80
    6600, 6900, 7100, 7300, 7600, 7800, 8000, 8300, 8500, 8800,           // 81 - 90
    9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000,           // 91 - 100
    10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, // 101 - 110
    12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, // 111 - 120
    12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, // 121 - 130
    12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, // 131 - 140
    12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, // 141 - 150
    12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, // 151 - 160
    12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, 12000, // 161 - 170

};

Coord aArroundCoords[ARROUND_COORD_MAX_NUM] =
{
    {0, 0},
    {0, 50},
    {35, 35},
    {50, -0},
    {35, -35},
    {0, -50},
    {-35, -35},
    {-50, 0},
    {-35, 35},
    {0, 100},
    {71, 71},
    {100, -0},
    {71, -71},
    {0, -100},
    {-71, -71},
    {-100, 0},
    {-71, 71},
    {0, 150},
    {106, 106},
    {150, -0},
    {106, -106},
    {0, -150},
    {-106, -106},
    {-150, 0},
    {-106, 106},
    {0, 200},
    {141, 141},
    {200, -0},
    {141, -141},
    {0, -200},
    {-141, -141},
    {-200, 0},
    {-141, 141},
    {0, 250},
    {177, 177},
    {250, -0},
    {177, -177},
    {0, -250},
    {-177, -177},
    {-250, 0},
    {-177, 177},
    {0, 300},
    {212, 212},
    {300, -0},
    {212, -212},
    {0, -300},
    {-212, -212},
    {-300, 0},
    {-212, 212},
    {0, 350},
    {247, 247},
    {350, -0},
    {247, -247},
    {0, -350},
    {-247, -247},
    {-350, 0},
    {-247, 247},
    {0, 400},
    {283, 283},
    {400, -0},
    {283, -283},
    {0, -400},
    {-283, -283},
    {-400, 0},
    {-283, 283},
    {0, 450},
    {318, 318},
    {450, -0},
    {318, -318},
    {0, -450},
    {-318, -318},
    {-450, 0},
    {-318, 318},
    {0, 500},
    {354, 354},
    {500, -0},
    {354, -354},
    {0, -500},
    {-354, -354},
    {-500, 0},
    {-354, 354},
    {0, 550},
    {389, 389},
    {550, -0},
    {389, -389},
    {0, -550},
    {-389, -389},
    {-550, 0},
    {-389, 389},
    {0, 600},
    {424, 424},
    {600, -0},
    {424, -424},
    {0, -600},
    {-424, -424},
    {-600, 0},
    {-424, 424},
    {0, 650},
    {460, 460},
    {650, -0},
    {460, -460},
    {0, -650},
    {-460, -460},
    {-650, 0},
    {-460, 460},
    {0, 700},
    {495, 495},
    {700, -0},
    {495, -495},
    {0, -700},
    {-495, -495},
    {-700, 0},
    {-495, 495},
    {0, 750},
    {530, 530},
    {750, -0},
    {530, -530},
    {0, -750},
    {-530, -530},
    {-750, 0},
    {-530, 530},
    {0, 800},
    {566, 566},
    {800, -0},
    {566, -566},
    {0, -800},
    {-566, -566},
    {-800, 0},
    {-566, 566},
    {0, 850},
    {601, 601},
    {850, -0},
    {601, -601},
    {0, -850},
    {-601, -601},
    {-850, 0},
    {-601, 601},
    {0, 900},
    {636, 636},
    {900, -0},
    {636, -636},
    {0, -900},
    {-636, -636},
    {-900, 0},
    {-636, 636},
    {0, 950},
    {672, 672},
    {950, -0},
    {672, -672},
    {0, -950},
    {-672, -672},
    {-950, 0},
    {-672, 672},
    {0, 1000},
    {707, 707},
    {1000, -0},
    {707, -707},
    {0, -1000},
    {-707, -707},
    {-1000, 0},
    {-707, 707},
};

const uint32_t guild_exp_table[GUILD_MAX_LEVEL + 1] = {
    0,         15000UL,    45000UL,   90000UL,   160000UL,  235000UL,
    325000UL,  430000UL,   550000UL,  685000UL,  835000UL,  1000000UL,
    1500000UL, 2100000UL,  2800000UL, 3600000UL, 4500000UL, 6500000UL,
    8000000UL, 10000000UL, 42000000UL};

// INTERNATIONAL_VERSION 길드경험치
const uint32_t guild_exp_table2[GUILD_MAX_LEVEL + 1] = {
    0,         15000UL,    45000UL,   90000UL,   160000UL,  235000UL,
    325000UL,  430000UL,   550000UL,  685000UL,  835000UL,  1000000UL,
    1500000UL, 2100000UL,  2800000UL, 3600000UL, 4500000UL, 6500000UL,
    8000000UL, 10000000UL, 42000000UL
    // 20
    /*21000000UL,
    22000000UL,
    23000000UL,
    24000000UL,
    25000000UL,
    26000000UL,
    27000000UL,
    28000000UL,
    29000000UL,
    30000000UL,
    31000000UL,
    32000000UL,
    33000000UL,
    34000000UL,
    35000000UL,
    36000000UL,
    37000000UL,
    38000000UL,
    39000000UL,
    40000000UL,
*/
};
// END_OF_INTERNATIONAL_VERSION 길드경험치

const uint32_t kGuildExpTable[GUILD_MAX_LEVEL + 1] = {
    0,
    6000UL,
    18000UL,
    36000UL,
    64000UL,
    94000UL,
    130000UL,
    172000UL,
    220000UL,
    274000UL,
    334000UL, // 10
    400000UL,
    600000UL,
    840000UL,
    1120000UL,
    1440000UL,
    1800000UL,
    2600000UL,
    3200000UL,
    4000000UL,

};

uint32_t GetGuildLevelExp(uint32_t level)
{
    if (level >= GUILD_MAX_LEVEL)
        return 0;

    return kGuildExpTable[level];
}

const int MAX_DEFENSE_PCT = 60; //Maximum defense a player can have against a given mob attack type

const int aiMobEnchantApplyIdx[MOB_ENCHANTS_MAX_NUM] =
{
    APPLY_CURSE_PCT,
    APPLY_SLOW_PCT,
    APPLY_POISON_PCT,
    APPLY_STUN_PCT,
    APPLY_CRITICAL_PCT,
    APPLY_PENETRATE_PCT,
};

const int aiMobResistsApplyIdx[MOB_RESISTS_MAX_NUM] =
{
    APPLY_RESIST_SWORD,
    APPLY_RESIST_TWOHAND,
    APPLY_RESIST_DAGGER,
    APPLY_RESIST_BELL,
    APPLY_RESIST_FAN,
    APPLY_RESIST_BOW,
    APPLY_RESIST_FIRE,
    APPLY_RESIST_ELEC,
    APPLY_RESIST_MAGIC,
    APPLY_RESIST_WIND,
    APPLY_POISON_REDUCE,
    APPLY_BLEEDING_REDUCE,
};

const int aiSocketPercentByQty[5][4] =
{
    {0, 0, 0, 0},
    {3, 0, 0, 0},
    {10, 1, 0, 0},
    {15, 10, 1, 0},
    {20, 15, 10, 1}
};

const int aiWeaponSocketQty[WEAPON_NUM_TYPES] =
{
    3, // WEAPON_SWORD,
    3, // WEAPON_DAGGER,
    3, // WEAPON_BOW,
    3, // WEAPON_TWO_HANDED,
    3, // WEAPON_BELL,
    3, // WEAPON_FAN,
    0, // WEAPON_ARROW,
    0, // WEAPON_MOUNT_SPEAR
    3, // WEAPON_CLAW,
    0, // WEAPON_QUIVER,
};

const int aiArmorSocketQty[ARMOR_NUM_TYPES] =
{
    3, // ARMOR_BODY,
    1, // ARMOR_HEAD,
    1, // ARMOR_SHIELD,
    0, // ARMOR_WRIST,
    0, // ARMOR_FOOTS,
    0  // ARMOR_ACCESSORY
};

struct ApplyInfo
{
    uint8_t applyType;
    uint8_t pointType;
};

int GetApplyTypeValueByString(const std::string &inputString)
{
    std::string arApplyType[] = {"APPLY_NONE",
                                 "APPLY_MAX_HP",
                                 "APPLY_MAX_SP",
                                 "APPLY_CON",
                                 "APPLY_INT",
                                 "APPLY_STR",
                                 "APPLY_DEX",
                                 "APPLY_ATT_SPEED",
                                 "APPLY_MOV_SPEED",
                                 "APPLY_CAST_SPEED",
                                 "APPLY_HP_REGEN",
                                 "APPLY_SP_REGEN",
                                 "APPLY_POISON_PCT",
                                 "APPLY_STUN_PCT",
                                 "APPLY_SLOW_PCT",
                                 "APPLY_CRITICAL_PCT",
                                 "APPLY_PENETRATE_PCT",
                                 "APPLY_ATTBONUS_HUMAN",
                                 "APPLY_ATTBONUS_ANIMAL",
                                 "APPLY_ATTBONUS_ORC",
                                 "APPLY_ATTBONUS_MILGYO",
                                 "APPLY_ATTBONUS_UNDEAD",
                                 "APPLY_ATTBONUS_DEVIL",
                                 "APPLY_STEAL_HP",
                                 "APPLY_STEAL_SP",
                                 "APPLY_MANA_BURN_PCT",
                                 "APPLY_DAMAGE_SP_RECOVER",
                                 "APPLY_BLOCK",
                                 "APPLY_DODGE",
                                 "APPLY_RESIST_SWORD",
                                 "APPLY_RESIST_TWOHAND",
                                 "APPLY_RESIST_DAGGER",
                                 "APPLY_RESIST_BELL",
                                 "APPLY_RESIST_FAN",
                                 "APPLY_RESIST_BOW",
                                 "APPLY_RESIST_FIRE",
                                 "APPLY_RESIST_ELEC",
                                 "APPLY_RESIST_MAGIC",
                                 "APPLY_RESIST_WIND",
                                 "APPLY_REFLECT_MELEE",
                                 "APPLY_REFLECT_CURSE",
                                 "APPLY_POISON_REDUCE",
                                 "APPLY_KILL_SP_RECOVER",
                                 "APPLY_EXP_DOUBLE_BONUS",
                                 "APPLY_GOLD_DOUBLE_BONUS",
                                 "APPLY_ITEM_DROP_BONUS",
                                 "APPLY_POTION_BONUS",
                                 "APPLY_KILL_HP_RECOVER",
                                 "APPLY_IMMUNE_STUN",
                                 "APPLY_IMMUNE_SLOW",
                                 "APPLY_IMMUNE_FALL",
                                 "APPLY_SKILL",
                                 "APPLY_BOW_DISTANCE",
                                 "APPLY_ATT_GRADE_BONUS",
                                 "APPLY_DEF_GRADE_BONUS",
                                 "APPLY_MAGIC_ATT_GRADE",
                                 "APPLY_MAGIC_DEF_GRADE",
                                 "APPLY_CURSE_PCT",
                                 "APPLY_MAX_STAMINA",
                                 "APPLY_ATTBONUS_WARRIOR",
                                 "APPLY_ATTBONUS_ASSASSIN",
                                 "APPLY_ATTBONUS_SURA",
                                 "APPLY_ATTBONUS_SHAMAN",
                                 "APPLY_ATTBONUS_MONSTER",
                                 "APPLY_MALL_ATTBONUS",
                                 "APPLY_MALL_DEFBONUS",
                                 "APPLY_MALL_EXPBONUS",
                                 "APPLY_MALL_ITEMBONUS",
                                 "APPLY_MALL_GOLDBONUS",
                                 "APPLY_MAX_HP_PCT",
                                 "APPLY_MAX_SP_PCT",
                                 "APPLY_SKILL_DAMAGE_BONUS",
                                 "APPLY_NORMAL_HIT_DAMAGE_BONUS",
                                 "APPLY_SKILL_DEFEND_BONUS",
                                 "APPLY_NORMAL_HIT_DEFEND_BONUS",
                                 "APPLY_PC_BANG_EXP_BONUS",
                                 "APPLY_PC_BANG_DROP_BONUS",
                                 "APPLY_EXTRACT_HP_PCT",
                                 "APPLY_RESIST_WARRIOR",
                                 "APPLY_RESIST_ASSASSIN",
                                 "APPLY_RESIST_SURA",
                                 "APPLY_RESIST_SHAMAN",
                                 "APPLY_ENERGY",
                                 "APPLY_DEF_GRADE",
                                 "APPLY_COSTUME_ATTR_BONUS",
                                 "APPLY_MAGIC_ATTBONUS_PER",
                                 "APPLY_MELEE_MAGIC_ATTBONUS_PER",
                                 "APPLY_RESIST_ICE",
                                 "APPLY_RESIST_EARTH",
                                 "APPLY_RESIST_DARK",
                                 "APPLY_ANTI_CRITICAL_PCT",
                                 "APPLY_ANTI_PENETRATE_PCT",
                                 "APPLY_BOOST_CRITICAL_DMG",
                                 "APPLY_BOOST_PENETRATE_DMG",
                                 "APPLY_ATTBONUS_INSECT",
                                 "APPLY_BLEEDING_PCT",
                                 "APPLY_BLEEDING_REDUCE",
                                 "APPLY_ATTBONUS_WOLFMAN",
                                 "APPLY_RESIST_WOLFMAN",
                                 "APPLY_RESIST_CLAW",
                                 "APPLY_ACCEDRAIN_RATE",
                                 "APPLY_ATTBONUS_METIN",
                                 "APPLY_ATTBONUS_TRENT",
                                 "APPLY_ATTBONUS_BOSS",
                                 "APPLY_RESIST_HUMAN",
                                 "APPLY_ENCHANT_ELECT",
                                 "APPLY_ENCHANT_FIRE",
                                 "APPLY_ENCHANT_ICE",
                                 "APPLY_ENCHANT_WIND",
                                 "APPLY_ENCHANT_EARTH",
                                 "APPLY_ENCHANT_DARK",
                                 "APPLY_ATTBONUS_DESERT",
                                 "APPLY_ATTBONUS_CZ",
                                 "APPLY_ATTBONUS_LEGEND"
    };

    int retInt = -1;
    // cout << "ApplyType : " << applyTypeStr << " -> ";
    int sizeOf = sizeof(arApplyType) / sizeof(arApplyType[0]);
    for (int j = 0; j < sizeOf; j++)
    {
        std::string tempString = arApplyType[j];
        std::string tempInputString = boost::trim_copy(inputString);
        if (tempInputString == tempString)
        {
            // cout << j << " ";
            retInt = j;
            break;
        }
    }
    // cout << endl;

    return retInt;
}

const ApplyInfo kApplyPointMapping[] = {
    {APPLY_NONE, POINT_NONE},
    {APPLY_MAX_HP, POINT_MAX_HP},
    {APPLY_MAX_SP, POINT_MAX_SP},
    {APPLY_CON, POINT_HT},
    {APPLY_INT, POINT_IQ},
    {APPLY_STR, POINT_ST},
    {APPLY_DEX, POINT_DX},
    {APPLY_ATT_SPEED, POINT_ATT_SPEED},
    {APPLY_MOV_SPEED, POINT_MOV_SPEED},
    {APPLY_CAST_SPEED, POINT_CASTING_SPEED},
    {APPLY_HP_REGEN, POINT_HP_REGEN},
    {APPLY_SP_REGEN, POINT_SP_REGEN},
    {APPLY_POISON_PCT, POINT_POISON_PCT},
    {APPLY_STUN_PCT, POINT_STUN_PCT},
    {APPLY_SLOW_PCT, POINT_SLOW_PCT},
    {APPLY_CRITICAL_PCT, POINT_CRITICAL_PCT},
    {APPLY_PENETRATE_PCT, POINT_PENETRATE_PCT},
    {APPLY_ATTBONUS_HUMAN, POINT_ATTBONUS_HUMAN},
    {APPLY_ATTBONUS_ANIMAL, POINT_ATTBONUS_ANIMAL},
    {APPLY_ATTBONUS_ORC, POINT_ATTBONUS_ORC},
    {APPLY_ATTBONUS_MILGYO, POINT_ATTBONUS_MILGYO},
    {APPLY_ATTBONUS_UNDEAD, POINT_ATTBONUS_UNDEAD},
    {APPLY_ATTBONUS_DEVIL, POINT_ATTBONUS_DEVIL},
    {APPLY_STEAL_HP, POINT_STEAL_HP},
    {APPLY_STEAL_SP, POINT_STEAL_SP},
    {APPLY_MANA_BURN_PCT, POINT_MANA_BURN_PCT},
    {APPLY_DAMAGE_SP_RECOVER, POINT_DAMAGE_SP_RECOVER},
    {APPLY_BLOCK, POINT_BLOCK},
    {APPLY_DODGE, POINT_DODGE},
    {APPLY_RESIST_SWORD, POINT_RESIST_SWORD},
    {APPLY_RESIST_TWOHAND, POINT_RESIST_TWOHAND},
    {APPLY_RESIST_DAGGER, POINT_RESIST_DAGGER},
    {APPLY_RESIST_BELL, POINT_RESIST_BELL},
    {APPLY_RESIST_FAN, POINT_RESIST_FAN},
    {APPLY_RESIST_BOW, POINT_RESIST_BOW},
    {APPLY_RESIST_FIRE, POINT_RESIST_FIRE},
    {APPLY_RESIST_ELEC, POINT_RESIST_ELEC},
    {APPLY_RESIST_MAGIC, POINT_RESIST_MAGIC},
    {APPLY_RESIST_WIND, POINT_RESIST_WIND},
    {APPLY_REFLECT_MELEE, POINT_REFLECT_MELEE},
    {APPLY_REFLECT_CURSE, POINT_REFLECT_CURSE},
    {APPLY_POISON_REDUCE, POINT_POISON_REDUCE},
    {APPLY_KILL_SP_RECOVER, POINT_KILL_SP_RECOVER},
    {APPLY_EXP_DOUBLE_BONUS, POINT_EXP_DOUBLE_BONUS},
    {APPLY_GOLD_DOUBLE_BONUS, POINT_GOLD_DOUBLE_BONUS},
    {APPLY_ITEM_DROP_BONUS, POINT_ITEM_DROP_BONUS},
    {APPLY_POTION_BONUS, POINT_POTION_BONUS},
    {APPLY_KILL_HP_RECOVER, POINT_KILL_HP_RECOVERY},
    {APPLY_IMMUNE_STUN, POINT_IMMUNE_STUN},
    {APPLY_IMMUNE_SLOW, POINT_IMMUNE_SLOW},
    {APPLY_IMMUNE_FALL, POINT_IMMUNE_FALL},
    {APPLY_SKILL, POINT_NONE},
    {APPLY_BOW_DISTANCE, POINT_BOW_DISTANCE},
    {APPLY_ATT_GRADE_BONUS, POINT_ATT_GRADE_BONUS},
    {APPLY_DEF_GRADE_BONUS, POINT_DEF_GRADE_BONUS},
    {APPLY_MAGIC_ATT_GRADE, POINT_MAGIC_ATT_GRADE_BONUS},
    {APPLY_MAGIC_DEF_GRADE, POINT_MAGIC_DEF_GRADE_BONUS},
    {APPLY_CURSE_PCT, POINT_CURSE_PCT},
    {APPLY_MAX_STAMINA, POINT_MAX_STAMINA},
    {APPLY_ATTBONUS_WARRIOR, POINT_ATTBONUS_WARRIOR},
    {APPLY_ATTBONUS_ASSASSIN, POINT_ATTBONUS_ASSASSIN},
    {APPLY_ATTBONUS_SURA, POINT_ATTBONUS_SURA},
    {APPLY_ATTBONUS_SHAMAN, POINT_ATTBONUS_SHAMAN},
    {APPLY_ATTBONUS_MONSTER, POINT_ATTBONUS_MONSTER},
    {APPLY_MALL_ATTBONUS, POINT_MALL_ATTBONUS},
    {APPLY_MALL_DEFBONUS, POINT_MALL_DEFBONUS},
    {APPLY_MALL_EXPBONUS, POINT_MALL_EXPBONUS},
    {APPLY_MALL_ITEMBONUS, POINT_MALL_ITEMBONUS},
    {APPLY_MALL_GOLDBONUS, POINT_MALL_GOLDBONUS},
    {APPLY_MAX_HP_PCT, POINT_MAX_HP_PCT},
    {APPLY_MAX_SP_PCT, POINT_MAX_SP_PCT},
    {APPLY_SKILL_DAMAGE_BONUS, POINT_SKILL_DAMAGE_BONUS},
    {APPLY_NORMAL_HIT_DAMAGE_BONUS, POINT_NORMAL_HIT_DAMAGE_BONUS},

    {APPLY_SKILL_DEFEND_BONUS, POINT_SKILL_DEFEND_BONUS},
    {APPLY_NORMAL_HIT_DEFEND_BONUS, POINT_NORMAL_HIT_DEFEND_BONUS},

    {APPLY_PC_BANG_EXP_BONUS, POINT_PC_BANG_EXP_BONUS},
    {APPLY_PC_BANG_DROP_BONUS, POINT_PC_BANG_DROP_BONUS},

    {APPLY_EXTRACT_HP_PCT, POINT_NONE},

    {APPLY_RESIST_WARRIOR, POINT_RESIST_WARRIOR},
    {APPLY_RESIST_ASSASSIN, POINT_RESIST_ASSASSIN},
    {APPLY_RESIST_SURA, POINT_RESIST_SURA},
    {APPLY_RESIST_SHAMAN, POINT_RESIST_SHAMAN},
    {APPLY_ENERGY, POINT_ENERGY},
    {APPLY_DEF_GRADE, POINT_DEF_GRADE},
    {APPLY_COSTUME_ATTR_BONUS, POINT_COSTUME_ATTR_BONUS},
    {APPLY_MAGIC_ATTBONUS_PER, POINT_MAGIC_ATT_BONUS_PER},
    {APPLY_MELEE_MAGIC_ATTBONUS_PER, POINT_MELEE_MAGIC_ATT_BONUS_PER},
    {APPLY_RESIST_ICE, POINT_RESIST_ICE},
    {APPLY_RESIST_EARTH, POINT_RESIST_EARTH},
    {APPLY_RESIST_DARK, POINT_RESIST_DARK},
    {APPLY_ANTI_CRITICAL_PCT, POINT_RESIST_CRITICAL},
    {APPLY_ANTI_PENETRATE_PCT, POINT_RESIST_PENETRATE},
    {APPLY_BOOST_CRITICAL_DMG, POINT_BOOST_CRITICAL},
    {APPLY_BOOST_PENETRATE_DMG, POINT_BOOST_PENETRATE},
    {APPLY_ATTBONUS_INSECT, POINT_ATTBONUS_INSECT},
    {APPLY_BLEEDING_REDUCE, POINT_BLEEDING_REDUCE},
    {APPLY_BLEEDING_PCT, POINT_BLEEDING_PCT},
    {APPLY_ATTBONUS_WOLFMAN, POINT_ATTBONUS_WOLFMAN},
    {APPLY_RESIST_WOLFMAN, POINT_RESIST_WOLFMAN},
    {APPLY_RESIST_CLAW, POINT_RESIST_CLAW},
    {APPLY_ACCEDRAIN_RATE, POINT_NONE},
    {APPLY_ATTBONUS_METIN, POINT_ATTBONUS_METIN},
    {APPLY_ATTBONUS_TRENT, POINT_ATTBONUS_TRENT},
    {APPLY_ATTBONUS_BOSS, POINT_ATTBONUS_BOSS},
    {APPLY_RESIST_HUMAN, POINT_RESIST_HUMAN},
    {APPLY_ENCHANT_ELECT, POINT_ENCHANT_ELECT},
    {APPLY_ENCHANT_FIRE, POINT_ENCHANT_FIRE},
    {APPLY_ENCHANT_ICE, POINT_ENCHANT_ICE},
    {APPLY_ENCHANT_WIND, POINT_ENCHANT_WIND},
    {APPLY_ENCHANT_EARTH, POINT_ENCHANT_EARTH},
    {APPLY_ENCHANT_DARK, POINT_ENCHANT_DARK},
    {APPLY_ATTBONUS_CZ, POINT_ATTBONUS_CZ},
    {APPLY_ATTBONUS_LEGEND, POINT_ATTBONUS_LEGEND},
    {APPLY_ATTBONUS_SHADOW, POINT_ATTBONUS_SHADOW},

};

const int aiItemMagicAttributePercentHigh[ITEM_ATTRIBUTE_MAX_LEVEL] =
{
    //25, 25, 40, 8, 2,
    30, 40, 20, 8, 2
};

const int aiItemMagicAttributePercentLow[ITEM_ATTRIBUTE_MAX_LEVEL] =
{
    //45, 25, 20, 10, 0,
    50, 40, 10, 0, 0
};

// ADD_ITEM_ATTRIBUTE
int aiItemAttributeAddPercent[ITEM_ATTRIBUTE_MAX_NUM] =
{
    100, 80, 60, 50, 40, 0, 0,
};
// END_OF_ADD_ITEM_ATTRIBUTE

const int aiExpLossPercents[PLAYER_EXP_TABLE_MAX + 1] =
{
    0,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 4, // 1 - 10
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 11 - 20
    4, 4, 4, 4, 4, 4, 4, 3, 3, 3, // 21 - 30
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 31 - 40
    3, 3, 3, 3, 2, 2, 2, 2, 2, 2, // 41 - 50
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 51 - 60
    2, 2, 1, 1, 1, 1, 1, 1, 1, 1, // 61 - 70
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 71 - 80
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 81 - 90
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 91 - 100
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 101 - 110
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 111 - 120
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 121 - 130
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 131 - 140
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 141 - 150
};

// ADD_GRANDMASTER_SKILL
const int aiGrandMasterSkillBookCountForLevelUp[10] =
{
    3, 3, 5, 5, 7, 7, 10, 10, 10, 20,
};

const int aiGrandMasterSkillBookMinCount[10] =
{
    //	1, 1, 3, 5, 10, 15, 20, 30, 40, 50, 
    //	3, 3, 5, 5, 10, 10, 15, 15, 20, 30
    1, 1, 1, 2, 2, 3, 3, 4, 5, 6
};

const int aiGrandMasterSkillBookMaxCount[10] =
{
    //	6, 15, 30, 45, 60, 80, 100, 120, 160, 200,
    //	6, 10, 15, 20, 30, 40, 50, 60, 70, 80
    5, 7, 9, 11, 13, 15, 20, 25, 30, 35
};
// END_OF_ADD_GRANDMASTER_SKILL

const int CHN_aiPartyBonusExpPercentByMemberCount[17] =
{
    0, 0, 12, 18, 26, 40, 53, 70, 100
};

// UPGRADE_PARTY_BONUS
const int KOR_aiPartyBonusExpPercentByMemberCount[17] =
{
    0,
    0,
    66 * 2 - 100,
    55 * 3 - 100,
    44 * 4 - 100,
    38 * 5 - 100, // 
    34 * 6 - 100, // 
    30 * 7 - 100, // 30% * 7 - 100
    30 * 8 - 100,
    160, // 170
    180, // 210
    200, // 220
    220, // 230
    240, // 240
    250, // 250
    250, // 250
    250, // 250
};

const int KOR_aiUniqueItemPartyBonusExpPercentByMemberCount[17] =
{
    0,
    0,
    30,
    30,
    30,
    30,
    30,
    30,
    10 * 8,
    10 * 9,
    10 * 10,
    10 * 11,
    10 * 12,
    10 * 13,
    10 * 14,
    10 * 15,
    10 * 16,
};
// END_OF_UPGRADE_PARTY_BONUS

const int aiChainLightningCountBySkillLevel[SKILL_MAX_LEVEL + 1] =
{
    0, // 0
    2, // 1
    2, // 2
    2, // 3
    2, // 4
    2, // 5
    2, // 6
    2, // 7
    2, // 8
    3, // 9
    3, // 10
    3, // 11
    3, // 12
    3, // 13
    3, // 14
    3, // 15
    3, // 16
    3, // 17
    3, // 18
    4, // 19
    4, // 20
    4, // 21
    4, // 22
    4, // 23
    5, // 24
    5, // 25
    5, // 26
    5, // 27
    5, // 28
    5, // 29
    5, // 30
    5, // 31
    5, // 32
    5, // 33
    5, // 34
    5, // 35
    5, // 36
    5, // 37
    5, // 38
    5, // 39
    5, // 40
    5, // 41
    5, // 42
    5, // 43
    5, // 44
    5, // 45
    5, // 46
    5, // 47
    5, // 48
    5, // 49
    5, // 50
};

const SStoneDropInfo aStoneDrop[STONE_INFO_MAX_NUM] =
{
    //  mob		pct	{+0	+1	+2	+3	+4}
    {8005, 60, {30, 30, 30, 9, 1}},
    {8006, 60, {28, 29, 31, 11, 1}},
    {8007, 60, {24, 29, 32, 13, 2}},
    {8008, 60, {22, 28, 33, 15, 2}},
    {8009, 60, {21, 27, 33, 17, 2}},
    {8010, 60, {18, 26, 34, 20, 2}},
    {8011, 60, {14, 26, 35, 22, 3}},
    {8012, 60, {10, 26, 37, 24, 3}},
    {8013, 60, {2, 26, 40, 29, 3}},
    {8014, 60, {0, 26, 41, 30, 3}},
    {8024, 60, {0, 26, 41, 30, 3}},
    {8025, 60, {0, 26, 41, 30, 3}},
    {8026, 60, {0, 26, 41, 30, 3}},
    {8027, 60, {0, 26, 41, 30, 3}},
    {8030, 60, {0, 26, 41, 30, 3}},

    {8051, 60, {0, 26, 41, 30, 8}},
    {8052, 60, {0, 26, 41, 30, 8}},
    {8053, 60, {0, 26, 41, 30, 8}},
    {8054, 60, {0, 26, 41, 30, 8}},
    {8055, 60, {0, 26, 41, 30, 8}},
    {8056, 60, {0, 26, 41, 30, 8}},

};

const char *c_apszEmpireNames[EMPIRE_MAX_NUM] =
{
    "전제국",
    "신수국",
    "천조국",
    "진노국"
};

const char *c_apszEmpireNamesAlt[EMPIRE_MAX_NUM] =
{
    "None",
    "[|cf6222200Shinsoo]|r]",
    "[|cfeef2600Chunjo|r]",
    "[|c264AFE00Jinno|r]"
};
const char *c_apszEmpireColor[EMPIRE_MAX_NUM] =
{
    "ffffffff",
    "fff62222",
    "ffeef260",
    "ff64AFE0"
};

const char *c_apszPrivNames[MAX_PRIV_NUM] =
{
    "",
    "아이템이 나올 확률",
    "돈이 나올 확률",
    "돈 대박이 나올 확률",
    "경험치 배율",
    "Double Drop Amount",

};

const char *c_apszPremiumNames[PREMIUM_MAX_NUM] =
{
    "EXP",
    "ITEM",
    "SAFEBOX",
    "AUTOLOOT",
    "FISH_MIND",
    "",
    "GOLD",
    "",
    ""
};

const int aiPolymorphPowerByLevel[SKILL_MAX_LEVEL + 1] =
{
    10,  // 1
    11,  // 2
    11,  // 3
    12,  // 4
    13,  // 5
    13,  // 6
    14,  // 7
    15,  // 8
    16,  // 9
    17,  // 10
    18,  // 11
    19,  // 12
    20,  // 13
    22,  // 14
    23,  // 15
    24,  // 16
    26,  // 17
    27,  // 18
    29,  // 19
    31,  // 20
    33,  // 21
    35,  // 22
    37,  // 23
    39,  // 24
    41,  // 25
    44,  // 26
    46,  // 27
    49,  // 28
    52,  // 29
    55,  // 30
    59,  // 31
    62,  // 32
    66,  // 33
    70,  // 34
    74,  // 35
    79,  // 36
    84,  // 37
    89,  // 38
    94,  // 39
    100, // 40
};

TGuildWarInfo KOR_aGuildWarInfo[GUILD_WAR_TYPE_MAX_NUM] =
    /*
       {
       long lMapIndex;
       Gold iWarPrice;
       int iWinnerPotionRewardPctToWinner;
       int iLoserPotionRewardPctToWinner;
       int iInitialScore;
       int iEndScore;
       };
     */
    {
        {0, 0, 0, 0, 0, 0},
        {110, 0, 100, 50, 0, 100},
        {111, 0, 100, 50, 0, 10},
    };

//
// 악세서리 소켓용 수치들
//

// 다이아몬드로 소켓을 추가할 때 확률
const int aiAccessorySocketAddPct[ITEM_ACCESSORY_SOCKET_MAX_NUM] =
{
    50, 50, 50
};

// 악세서리 수치 값의 몇%만큼의 성능을 추가하는지
const int aiAccessorySocketEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
    0, 10, 20, 40
};

// 악세서리 수치 값의 몇%만큼의 성능을 추가하는지
const int aiAccessorySocketPermEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
    0, 15, 30, 60
};

// 소켓 지속시간 24, 12, 6
const int aiAccessorySocketDegradeTime[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
    0, 3600 * 24, 3600 * 12, 3600 * 6
};

// 소켓 장착 성공률
const int aiAccessorySocketPutPct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
    100, 100, 100, 0
};
// END_OF_ACCESSORY_REFINE

uint8_t GetApplyPoint(uint8_t applyType)
{
    for (const auto &mapping : kApplyPointMapping)
    {
        if (mapping.applyType == applyType)
            return mapping.pointType;
    }

    return POINT_NONE;
}

uint8_t GetPointApply(uint8_t pointType)
{
    for (const auto &mapping : kApplyPointMapping)
    {
        if (mapping.pointType == pointType)
            return mapping.applyType;
    }

    return APPLY_NONE;
}

static const uint32_t UsableSkillList[JOB_MAX_NUM][2][6] =
{
    {{1, 2, 3, 4, 5, 6}, {16, 17, 18, 19, 20, 21}},
    {{31, 32, 33, 34, 35, 36}, {46, 47, 48, 49, 50, 51}},
    {{61, 62, 63, 64, 65, 66}, {76, 77, 78, 79, 80, 81}},
    {{91, 92, 93, 94, 95, 96}, {106, 107, 108, 109, 110, 111}},
#ifdef __WOLFMAN__
	{ { 170, 171, 172, 173, 174, 175 }, { 170, 171, 172, 173, 174, 175 } },
#endif
};

const uint32_t *GetUsableSkillList(uint8_t bJob, uint8_t bSkillGroup)
{
    if (bSkillGroup == 0)
        return nullptr;

    return UsableSkillList[bJob][bSkillGroup - 1];
}
