#ifndef METIN2_GAME_TYPES_HPP
#define METIN2_GAME_TYPES_HPP

#include <storm/Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <boost/fusion/adapted/struct.hpp>

#include <functional>

#include <game/Constants.hpp>
#include <game/MobConstants.hpp>
#include <game/ItemConstants.hpp>
#include <game/SkillConstants.hpp>
#include <Services.hpp>

#pragma pack(push, 1)

using Gold = int64_t;
using GuildAuthority = int64_t;
using Exp = uint32_t;
using ExchangeArg1 = int64_t;
using SocketValue = int64_t;
using CountType = uint32_t;
using PointValue = double;
using DamageValue = int64_t;
using SessionId = uint64_t;
using PlayerAlignment = int32_t;
using Level = uint8_t;
using ItemVnum = uint32_t;
using GenericVnum = uint32_t;
using PlayerId = uint32_t;
using ApplyValue = float;
using ApplyType = uint8_t;
using AttrType = ApplyType;
using AttrValue = ApplyValue;

struct Part {
	ItemVnum vnum;
	ItemVnum appearance;
	ItemVnum costume;
	ItemVnum costume_appearance;
};

BOOST_FUSION_ADAPT_STRUCT(
	Part,
	vnum,
	appearance,
	costume,
	costume_appearance
)

inline bool operator==(const Part& a, const Part& b)
{
	return a.appearance == b.appearance && a.vnum == b.vnum &&
	       a.costume == b.costume &&
	       a.costume_appearance == b.costume_appearance;
}

typedef struct SSimplePlayer {
	uint32_t id;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t empire;
	uint8_t byJob;
	uint8_t byLevel;
	uint32_t dwPlayMinutes;
	uint8_t byST, byHT, byDX, byIQ;
	Part parts[PART_MAX_NUM];
	uint8_t bChangeName, bChangeEmpire;
	int32_t mapIndex, x, y;
	char addr[46];
	uint16_t port;
	uint8_t skill_group;
	uint32_t last_play;

	Part GetPart(int idx) { return parts[idx]; }

} TSimplePlayer;

using SimplePlayer = SSimplePlayer;

BOOST_FUSION_ADAPT_STRUCT(
	SimplePlayer,
	id,
	name,
	empire,
	byJob,
	byLevel,
	dwPlayMinutes,
	byST,
	byHT,
	byDX,
	byIQ,
	parts,
	bChangeName,
	bChangeEmpire,
	mapIndex,
	x,
	y,
	addr,
	port,
	skill_group,
	last_play
)


typedef struct SAccountTable {
	uint32_t id;
	TSimplePlayer players[PLAYER_PER_ACCOUNT];

} TAccountTable;


typedef struct SQuickslot {
	uint8_t type;
	uint16_t pos;
} TQuickslot;

BOOST_FUSION_ADAPT_STRUCT(
	TQuickslot,
	type,
	pos
)

using QuickslotPair = std::pair<uint32_t, TQuickslot>;

inline bool operator==(const TQuickslot& a, const TQuickslot& b)
{
	return a.type == b.type && a.pos == b.pos;
}

inline bool operator!=(const TQuickslot& a, const TQuickslot& b)
{
	return !(a == b);
}

typedef struct SPlayerSkill
{
	uint8_t bMasterType = 0;
	uint8_t bLevel = 0;
	uint32_t color = 0;
	uint32_t tNextRead = 0;
} TPlayerSkill;

BOOST_FUSION_ADAPT_STRUCT(
	TPlayerSkill,
	bMasterType,
	bLevel,
	color,
	tNextRead
)

typedef struct SPlayerTitle
{
	char title[40 + 1];
	uint32_t color;
} TPlayerTitle;

BOOST_FUSION_ADAPT_STRUCT(
	TPlayerTitle,
	title,
	color
)

inline bool operator==(const TPlayerTitle& a, const TPlayerTitle& b)
{
	return !strcmp(a.title, b.title) &&
		a.color == b.color;
}

inline bool operator!=(const TPlayerTitle& a, const TPlayerTitle& b)
{
	return !(a == b);
}




using SkillPair = std::pair<uint32_t, TPlayerSkill>;

inline bool operator==(const TPlayerSkill& a, const TPlayerSkill& b)
{
	return a.bMasterType == b.bMasterType &&
		a.bLevel == b.bLevel &&
		a.color == b.color &&
		a.tNextRead == b.tNextRead;
}

inline bool operator!=(const TPlayerSkill& a, const TPlayerSkill& b)
{
	return !(a == b);
}

struct THorseInfo {
	uint8_t bLevel;
	bool bRiding;
	short sStamina;
	short sHealth;
	uint32_t dwHorseHealthDropTime;
};

typedef struct SPlayerTable {
	uint32_t id;

	char name[CHARACTER_NAME_MAX_LEN + 1];
#ifdef __FAKE_PC__
	char	fakepc_name[CHARACTER_NAME_MAX_LEN + 1];
#endif
	char ip[IP_ADDRESS_LENGTH + 1];

	uint8_t empire;

	uint8_t job;
	uint8_t voice;

	uint8_t level;
	uint8_t level_step;
	short st, ht, dx, iq;

	PointValue exp;
	Gold gold;

	uint8_t dir;
	int32_t x, y, z;
	int32_t lMapIndex;

	int32_t lExitX, lExitY;
	int32_t lExitMapIndex;

	int32_t hp;
	int32_t sp;

	int32_t sRandomHP;
	int32_t sRandomSP;

	int32_t playtime;

	int16_t stat_point;
	int16_t skill_point;
	int16_t sub_skill_point;
	int16_t horse_skill_point;
	int16_t tree_skill_point = 0;

#ifdef ENABLE_GEM_SYSTEM
	INT gem;
#endif

	TPlayerSkill skills[SKILL_MAX_NUM] = {};
	TQuickslot quickslot[QUICKSLOT_MAX_NUM] = {};
	TPlayerTitle title;

	uint8_t part_base;
	Part parts[PART_MAX_NUM];

	int16_t stamina;
	uint32_t blockMode;
	uint32_t chatFilter;

	uint8_t skill_group;
	PlayerAlignment lAlignment;

	THorseInfo horse;
	uint32_t logoff_interval;
	uint32_t last_play;

	uint32_t aiPremiumTimes[PREMIUM_MAX_NUM];
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
	int32_t		iOfflineShopFarmedOpeningTime;
	int32_t		iOfflineShopSpecialOpeningTime;
#endif
#ifdef ENABLE_BATTLE_PASS
	uint32_t	dwBattlePassEndTime;
#endif
} TPlayerTable;


typedef struct SEntityTable {
	uint32_t dwVnum;
} TEntityTable;


#define QUEST_NAME_MAX_LEN 32
#define QUEST_STATE_MAX_LEN 64

typedef struct SQuestTable {
	uint32_t dwPID;
	char szName[QUEST_NAME_MAX_LEN + 1];
	char szState[QUEST_STATE_MAX_LEN + 1];
	int32_t lValue;
} TQuestTable;



typedef struct SItemLimit {
	uint8_t bType;
	int32_t value;
} TItemLimit;

typedef struct SItemApply {
	ApplyType bType;
	ApplyValue lValue;
} TItemApply;

typedef struct SItemTable  {
	static const uint32_t kVersion = 3;

	uint32_t dwVnum;
	uint32_t dwVnumRange;
	char szName[ITEM_NAME_MAX_LEN + 1];
	char szLocaleName[ITEM_NAME_MAX_LEN + 1];
	uint8_t bType;
	uint8_t bSubType;

	uint8_t bWeight;
	uint8_t bSize;

	uint64_t dwAntiFlags;
	uint64_t dwFlags;
	uint32_t dwWearFlags;
	uint32_t dwImmuneFlag;

	Gold dwGold;
	Gold dwShopBuyPrice;

	TItemLimit aLimits[ITEM_LIMIT_MAX_NUM];
	TItemApply aApplies[ITEM_APPLY_MAX_NUM];
	int32_t alValues[ITEM_VALUES_MAX_NUM];

	uint32_t dwRefinedVnum;
	uint16_t wRefineSet;
	uint8_t bAlterToMagicItemPct;
	uint8_t bSpecular;
	uint8_t bGainSocketPct;

	int16_t sAddonType; // 기본 속성

	int32_t GetLevel() const
	{
		return aLimits[0].bType == LIMIT_LEVEL ? aLimits[0].value : 0;
	}

} TItemTable;



BOOST_FUSION_ADAPT_STRUCT(
	TItemTable,
	dwVnum,
	dwVnumRange,
	szName,
	szLocaleName,
	bType,
	bSubType,
	bWeight,
	bSize,
	dwAntiFlags,
	dwFlags,
	dwWearFlags,
	dwImmuneFlag,
	dwGold,
	dwShopBuyPrice,
	aLimits,
	aApplies,
	alValues,
	dwRefinedVnum,
	wRefineSet,
	bAlterToMagicItemPct,
	bSpecular,
	bGainSocketPct,
	sAddonType
);

BOOST_FUSION_ADAPT_STRUCT(
	TItemLimit,
	bType,
	value
);

BOOST_FUSION_ADAPT_STRUCT(
	TItemApply,
	bType,
	lValue
);



struct SwitchBotSlotData
{
    BOOST_FORCEINLINE bool AltConfigured(uint32_t index) const
    {
		// Could use any of but we need to check a limited set of attributes
		//return std::any_of(std::begin(attr[index]), std::end(attr[index]), [](const TItemApply& val)
		//{
		//    return val.bType && val.lValue != 0.0f;
		//});
        for (size_t i = 0; i < ITEM_MAX_NORM_ATTR_NUM; i++)
        {
            const auto& attribute = attr[index][i];
            if (attribute.bType && attribute.lValue != 0.0f )
            {
                return true;
            }
        }

        return false;
    }

    BOOST_FORCEINLINE bool AltRareConfigured(uint32_t index) const
    {
        for (size_t i = ITEM_MAX_NORM_ATTR_NUM; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
        {
            const auto& attribute = attr[index][i];
            if (attribute.bType && attribute.lValue != 0.0f)
            {
                return true;
            }
        }
        return false;
    }

    uint16_t slot;
    TItemApply attr[3][ITEM_ATTRIBUTE_MAX_NUM];
    uint8_t status;
};

BOOST_FUSION_ADAPT_STRUCT(
	SwitchBotSlotData,
	slot,
	attr,
	status
)

struct AffectData {
	uint16_t type;
	uint8_t subType;

	uint8_t pointType;
	PointValue pointValue;

	int32_t duration;
	int16_t spCost;

	uint32_t color;
};

BOOST_FUSION_ADAPT_STRUCT(
	AffectData,
	type,
	subType,
	pointType,
	pointValue,
	duration,
	spCost,
	color
)

inline bool operator==(const AffectData& a, const AffectData& b)
{
	return a.type == b.type &&
		a.subType == b.subType &&
		a.pointType == b.pointType &&
		a.pointValue == b.pointValue &&
		a.duration == b.duration &&
		a.spCost == b.spCost &&
		a.color == b.color;
}

inline bool operator<(const AffectData& a, const AffectData& b)
{
	if (a.type != b.type)
		return a.type < b.type;

	return a.pointType < b.pointType;
}


// ADMIN_MANAGER
typedef struct TAdminInfo {
	int m_ID;               //고유ID
	char m_szAccount[32];   //계정
	char m_szName[32];      //캐릭터이름
	char m_szContactIP[16]; //접근아이피
	char m_szServerIP[16];  //서버아이피
	int m_Authority;        //권한
} tAdminInfo;
// END_ADMIN_MANAGER

BOOST_FUSION_ADAPT_STRUCT(
	TAdminInfo,
	m_ID,
	m_szAccount,
	m_szName,
	m_szContactIP,
	m_szServerIP,
	m_Authority
);

// BOOT_LOCALIZATION
struct tLocale {
	char szValue[32];
	char szKey[32];
};
// BOOT_LOCALIZATION

BOOST_FUSION_ADAPT_STRUCT(
	tLocale,
	szValue,
	szKey
);


struct BlockedPC
{
	uint16_t mode;
	char name[CHARACTER_NAME_MAX_LEN + 1];
};

BOOST_FUSION_ADAPT_STRUCT(
	BlockedPC,
	mode,
	name
);

struct ElementLoadBlockedPC
{
	uint32_t pid;
	BlockedPC data;
};

BOOST_FUSION_ADAPT_STRUCT(
	ElementLoadBlockedPC,
	pid,
	data
);

struct LevelPetDisplayedFood {
    uint32_t min;
	uint32_t max;
	ItemVnum vnum;
};


struct TWikiItemOriginInfo {
	GenericVnum vnum;
	uint8_t is_mob;
};


struct WorldBossPosition {
	uint32_t mapIndex;

	WorldBossPosition(uint32_t mapIndex, uint32_t x, uint32_t y)
		: mapIndex{mapIndex},
		  x{x},
		  y{y} {
	}

	uint32_t x;
	uint32_t y;
};

typedef struct SWorldBossInfo {
	uint8_t	bDay;
	uint8_t	bHour;
	uint32_t	dwRaceNum;
	int32_t		iMapCount;
	std::vector<WorldBossPosition>	piMapIndexList;
	std::vector<std::string>	pszMapNameList;
	int32_t		iLastSpawnDay;
	int32_t		iLastSpawnHour;
	uint8_t	bIsDaily;

} TWorldBossInfo;

#pragma region Battlepass

#ifdef ENABLE_BATTLE_PASS
typedef struct SPlayerBattlePassMission
{
	uint32_t dwPlayerId;
	uint32_t dwMissionId;
	uint32_t dwBattlePassId;
	uint64_t dwExtraInfo;
	uint8_t bCompleted;
	uint8_t bIsUpdated;
} TPlayerBattlePassMission;

typedef struct SBattlePassRewardItem
{
	uint32_t	dwVnum;
	CountType	bCount;
} TBattlePassRewardItem;

typedef struct SBattlePassMissionInfo
{
	uint8_t	bMissionType;
	uint64_t	dwMissionInfo[3];
	TBattlePassRewardItem aRewardList[MISSION_REWARD_COUNT];
} TBattlePassMissionInfo;

typedef struct SBattlePassRanking
{
	uint8_t	bPos;
	char	playerName[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t	dwFinishTime;
} TBattlePassRanking;

typedef struct SBattlePassRegisterRanking
{
	uint8_t	bBattlePassId;
	char	playerName[CHARACTER_NAME_MAX_LEN + 1];
} TBattlePassRegisterRanking;


BOOST_FUSION_ADAPT_STRUCT(
	SPlayerBattlePassMission,
	dwPlayerId,
	dwMissionId,
	dwBattlePassId,
	dwExtraInfo,
	bCompleted,
	bIsUpdated
)

BOOST_FUSION_ADAPT_STRUCT(
	SBattlePassRewardItem,
	dwVnum,
	bCount
)

BOOST_FUSION_ADAPT_STRUCT(
	SBattlePassMissionInfo,
	bMissionType,
	dwMissionInfo,
	aRewardList
)

BOOST_FUSION_ADAPT_STRUCT(
	SBattlePassRanking,
	bPos,
	playerName,
	dwFinishTime
)

BOOST_FUSION_ADAPT_STRUCT(
	SBattlePassRegisterRanking,
	bBattlePassId,
	playerName
)



#endif

#pragma endregion 

struct HuntingReward
{
    ItemVnum vnum;
    CountType count;
};

struct HuntingMission
{
    uint32_t id;
    std::vector<uint32_t> mobVnums;
    uint32_t needCount;
    uint8_t minLevel;
	uint8_t isDaily = false;
	std::string name;

    std::vector<HuntingReward> rewards;
    std::vector<uint32_t> required;
};


typedef struct SPlayerHuntingMission
{
	uint32_t playerId;
	uint32_t missionId;
	uint32_t killCount;
	uint8_t day = 0;
	uint8_t bCompleted;
	uint8_t bIsUpdated;
} TPlayerHuntingMission;

struct PacketDataHuntingMissions
{
    uint32_t id;
    uint32_t mobVnum;
    uint32_t needCount;
    uint8_t minLevel;
};

struct PacketDataHuntingReward
{
    uint32_t id;
	HuntingReward data;
};

BOOST_FUSION_ADAPT_STRUCT(
	HuntingReward,
	vnum,
	count
);


BOOST_FUSION_ADAPT_STRUCT(
	HuntingMission,
	id,
	mobVnums,
	needCount,
	minLevel,
	isDaily,
	name,
	rewards,
	required
);

BOOST_FUSION_ADAPT_STRUCT(
	TPlayerHuntingMission,
	playerId,
	missionId,
	killCount,
	day,
	bCompleted,
	bIsUpdated
);


#pragma pack(pop)



#endif
