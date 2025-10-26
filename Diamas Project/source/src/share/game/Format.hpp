#ifndef METIN2_TOOL_CONVPROTO_FORMAT_HPP
#define METIN2_TOOL_CONVPROTO_FORMAT_HPP

#include <Config.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

namespace old
{
using Gold = int64_t;
using PointValue = int32_t;
using GuildAuthority = int64_t;
using Exp = uint32_t;
using ExchangeArg1 = int64_t;
using SocketValue = int64_t;

enum EMisc
{
	CHARACTER_NAME_MAX_LEN	= 36,
	MOB_SKILL_MAX_NUM		= 5,
	ITEM_NAME_MAX_LEN = 30,
	ITEM_VALUES_MAX_NUM = 6,
	ITEM_LIMIT_MAX_NUM = 2,
	ITEM_APPLY_MAX_NUM = 6,
	ITEM_SOCKET_MAX_NUM = 6,
	ITEM_ATTRIBUTE_MAX_NUM = 7,
};

enum ECharType
{
	CHAR_TYPE_MONSTER,
	CHAR_TYPE_NPC,
	CHAR_TYPE_STONE,
	CHAR_TYPE_WARP,
	CHAR_TYPE_DOOR,
	CHAR_TYPE_BUILDING,
	CHAR_TYPE_PC,
	CHAR_TYPE_POLYMORPH_PC,
	CHAR_TYPE_HORSE,
	CHAR_TYPE_GOTO,
	CHAR_TYPE_PET,
};

enum EBattleType
{
	BATTLE_TYPE_MELEE,
	BATTLE_TYPE_RANGE,
	BATTLE_TYPE_MAGIC,
	BATTLE_TYPE_SPECIAL,
	BATTLE_TYPE_POWER,
	BATTLE_TYPE_TANKER,
	BATTLE_TYPE_SUPER_POWER,
	BATTLE_TYPE_SUPER_TANKER,
	BATTLE_TYPE_MAX_NUM
};

enum EMobRank
{
	MOB_RANK_PAWN,
	MOB_RANK_S_PAWN,
	MOB_RANK_KNIGHT,
	MOB_RANK_S_KNIGHT,
	MOB_RANK_BOSS,
	MOB_RANK_KING,
	MOB_RANK_MAX_NUM
};

enum ERaceFlags
{
	RACE_FLAG_ANIMAL	= (1 << 0),
	RACE_FLAG_UNDEAD	= (1 << 1),
	RACE_FLAG_DEVIL		= (1 << 2),
	RACE_FLAG_HUMAN		= (1 << 3),
	RACE_FLAG_ORC		= (1 << 4),
	RACE_FLAG_MILGYO	= (1 << 5),
	RACE_FLAG_INSECT	= (1 << 6),
	RACE_FLAG_FIRE		= (1 << 7),
	RACE_FLAG_ICE		= (1 << 8),
	RACE_FLAG_DESERT	= (1 << 9),
	RACE_FLAG_TREE		= (1 << 10),
	RACE_FLAG_ATT_ELEC	= (1 << 11),
	RACE_FLAG_ATT_FIRE	= (1 << 12),
	RACE_FLAG_ATT_ICE	= (1 << 13),
	RACE_FLAG_ATT_WIND	= (1 << 14),
	RACE_FLAG_ATT_EARTH	= (1 << 15),
	RACE_FLAG_ATT_DARK	= (1 << 16),
	RACE_FLAG_CZ	= (1 << 18),
	RACE_FLAG_SHADOW	= (1 << 19),

};


enum EMobEnchants
{
	MOB_ENCHANT_CURSE,
	MOB_ENCHANT_SLOW,
	MOB_ENCHANT_POISON,
	MOB_ENCHANT_STUN,
	MOB_ENCHANT_CRITICAL,
	MOB_ENCHANT_PENETRATE,
	MOB_ENCHANTS_MAX_NUM
};

enum EMobResists
{
	MOB_RESIST_SWORD,
	MOB_RESIST_TWOHAND,
	MOB_RESIST_DAGGER,
	MOB_RESIST_BELL,
	MOB_RESIST_FAN,
	MOB_RESIST_BOW,
	MOB_RESIST_FIRE,
	MOB_RESIST_ELECT,
	MOB_RESIST_MAGIC,
	MOB_RESIST_WIND,
	MOB_RESIST_POISON,
	MOB_RESIST_BLEEDING,
	MOB_RESISTS_MAX_NUM
};


enum EItemTypes
{
    ITEM_NONE,              //0
    ITEM_WEAPON,            //1//¹«±â
    ITEM_ARMOR,             //2//°©¿Ê
    ITEM_USE,               //3//¾ÆÀÌÅÛ »ç¿ë
    ITEM_AUTOUSE,           //4
    ITEM_MATERIAL,          //5
    ITEM_SPECIAL,           //6 //½ºÆä¼È ¾ÆÀÌÅÛ
    ITEM_TOOL,              //7
    ITEM_LOTTERY,           //8//º¹±Ç
    ITEM_ELK,               //9//µ·
    ITEM_METIN,             //10
    ITEM_CONTAINER,         //11
    ITEM_FISH,              //12//³¬½Ã
    ITEM_ROD,               //13
    ITEM_RESOURCE,          //14
    ITEM_CAMPFIRE,          //15
    ITEM_UNIQUE,            //16
    ITEM_SKILLBOOK,         //17
    ITEM_QUEST,             //18
    ITEM_POLYMORPH,         //19
    ITEM_TREASURE_BOX,      //20//º¸¹°»óÀÚ
    ITEM_TREASURE_KEY,      //21//º¸¹°»óÀÚ ¿­¼è
    ITEM_SKILLFORGET,       //22
    ITEM_GIFTBOX,           //23
    ITEM_PICK,              //24
    ITEM_HAIR,              //25//¸Ó¸®
    ITEM_TOTEM,             //26//ÅäÅÛ
	ITEM_BLEND,				//27//»ý¼ºµÉ¶§ ·£´ýÇÏ°Ô ¼Ó¼ºÀÌ ºÙ´Â ¾à¹°
	ITEM_COSTUME,			//28//ÄÚ½ºÃõ ¾ÆÀÌÅÛ (2011³â 8¿ù Ãß°¡µÈ ÄÚ½ºÃõ ½Ã½ºÅÛ¿ë ¾ÆÀÌÅÛ)
	ITEM_DS,				//29 //¿ëÈ¥¼®
	ITEM_SPECIAL_DS,		//30 // Æ¯¼öÇÑ ¿ëÈ¥¼® (DS_SLOT¿¡ Âø¿ëÇÏ´Â UNIQUE ¾ÆÀÌÅÛÀÌ¶ó »ý°¢ÇÏ¸é µÊ)
	ITEM_EXTRACT,			//31 ÃßÃâµµ±¸.
	ITEM_SECONDARY_COIN,	//32 ?? ¸íµµÀü??
	ITEM_RING,				//33 ¹ÝÁö
	ITEM_BELT,				//34 º§Æ®
	ITEM_PET,				//35 º§Æ®
	ITEM_MEDIUM,			//36 º§Æ®
	ITEM_GACHA,				//37 º§Æ®
	ITEM_TOGGLE,			//38 º§Æ®
	ITEM_TYPE_MAX			//39 º§Æ®
};

enum EMetinSubTypes
{
	METIN_NORMAL,
	METIN_GOLD,
};

enum EWeaponSubTypes
{
	WEAPON_SWORD,
	WEAPON_DAGGER,
	WEAPON_BOW,
	WEAPON_TWO_HANDED,
	WEAPON_BELL,
	WEAPON_FAN,
	WEAPON_ARROW,
	WEAPON_MOUNT_SPEAR,
	WEAPON_CLAW,
	WEAPON_NUM_TYPES,
};

enum EArmorSubTypes
{
	ARMOR_BODY,
	ARMOR_HEAD,
	ARMOR_SHIELD,
	ARMOR_WRIST,
	ARMOR_FOOTS,
	ARMOR_NECK,
	ARMOR_EAR,
	ARMOR_NUM_TYPES
};

enum ECostumeSubTypes
{
	COSTUME_BODY = ARMOR_BODY,			// [Áß¿ä!!] ECostumeSubTypes enum value´Â  Á¾·ùº°·Î EArmorSubTypesÀÇ ±×°Í°ú °°¾Æ¾ß ÇÔ.
	COSTUME_HAIR = ARMOR_HEAD,			// ÀÌ´Â ÄÚ½ºÃõ ¾ÆÀÌÅÛ¿¡ Ãß°¡ ¼Ó¼ºÀ» ºÙÀÌ°Ú´Ù´Â »ç¾÷ºÎÀÇ ¿äÃ»¿¡ µû¶ó¼­ ±âÁ¸ ·ÎÁ÷À» È°¿ëÇÏ±â À§ÇÔÀÓ.
	COSTUME_MOUNT,
	COSTUME_ACCE,
	COSTUME_WEAPON,
	COSTUME_BODY_EFFECT,
	COSTUME_WEAPON_EFFECT,
	COSTUME_WING_EFFECT,
	COSTUME_RANK,
	COSTUME_NUM_TYPES,
};

enum EDragonSoulSubType
{
	DS_SLOT1,
	DS_SLOT2,
	DS_SLOT3,
	DS_SLOT4,
	DS_SLOT5,
	DS_SLOT6,
	DS_SLOT_MAX,
};

enum EFishSubTypes
{
	FISH_ALIVE,
	FISH_DEAD,
};

enum EResourceSubTypes
{
	RESOURCE_FISHBONE,
	RESOURCE_WATERSTONEPIECE,
	RESOURCE_WATERSTONE,
	RESOURCE_BLOOD_PEARL,
	RESOURCE_BLUE_PEARL,
	RESOURCE_WHITE_PEARL,
	RESOURCE_BUCKET,
	RESOURCE_CRYSTAL,
	RESOURCE_GEM,
	RESOURCE_STONE,
	RESOURCE_METIN,
	RESOURCE_ORE,
};

enum EUniqueSubTypes
{
	UNIQUE_NONE,
	UNIQUE_BOOK,
	UNIQUE_SPECIAL_RIDE,
	UNIQUE_SPECIAL_MOUNT_RIDE,
};

enum EUseSubTypes
{
	USE_POTION,					// 0
	USE_TALISMAN,
	USE_TUNING,
	USE_MOVE,
	USE_TREASURE_BOX,
	USE_MONEYBAG,
	USE_BAIT,
	USE_ABILITY_UP,
	USE_AFFECT,
	USE_CREATE_STONE,
	USE_SPECIAL,				// 10
	USE_POTION_NODELAY,
	USE_CLEAR,
	USE_INVISIBILITY,
	USE_DETACHMENT,
	USE_BUCKET,
	USE_POTION_CONTINUE,
	USE_CLEAN_SOCKET,
	USE_CHANGE_ATTRIBUTE,
	USE_ADD_ATTRIBUTE,
	USE_ADD_ACCESSORY_SOCKET,	// 20
	USE_PUT_INTO_ACCESSORY_SOCKET,
	USE_ADD_ATTRIBUTE2,
	USE_RECIPE,
	USE_CHANGE_ATTRIBUTE2,
	USE_BIND,
	USE_UNBIND,
	USE_TIME_CHARGE_PER,
	USE_TIME_CHARGE_FIX,				// 28
	USE_PUT_INTO_BELT_SOCKET,			// 29 º§Æ® ¼ÒÄÏ¿¡ »ç¿ëÇÒ ¼ö ÀÖ´Â ¾ÆÀÌÅÛ 
	USE_PUT_INTO_RING_SOCKET,			// 30 ¹ÝÁö ¼ÒÄÏ¿¡ »ç¿ëÇÒ ¼ö ÀÖ´Â ¾ÆÀÌÅÛ (À¯´ÏÅ© ¹ÝÁö ¸»°í, »õ·Î Ãß°¡µÈ ¹ÝÁö ½½·Ô)
	USE_CHEST,
	USE_CHANGE_COSTUME_ATTR,
	USE_RESET_COSTUME_ATTR,
	USE_ADD_ATTRIBUTE_RARE,
	USE_CHANGE_ATTRIBUTE_RARE,
	USE_ENHANCE_TIME,
	USE_CHANGE_ATTRIBUTE_PERM,
	USE_ADD_ATTRIBUTE_PERM,
	USE_MAKE_ACCESSORY_SOCKET_PERM,
	USE_SILK_BOTARY,
	USE_OFFLINE_SHOP_FARMED_TIME,
	USE_OFFLINE_SHOP_SPECIAL_TIME,
};

enum EExtractSubTypes
{
	EXTRACT_DRAGON_SOUL,
	EXTRACT_DRAGON_HEART,
};

enum EAutoUseSubTypes
{
	AUTOUSE_POTION,
	AUTOUSE_ABILITY_UP,
	AUTOUSE_BOMB,
	AUTOUSE_GOLD,
	AUTOUSE_MONEYBAG,
	AUTOUSE_TREASURE_BOX
};

enum EMaterialSubTypes
{
	MATERIAL_LEATHER,
	MATERIAL_BLOOD,
	MATERIAL_ROOT,
	MATERIAL_NEEDLE,
	MATERIAL_JEWEL,
	MATERIAL_DS_REFINE_NORMAL,
	MATERIAL_DS_REFINE_BLESSED,
	MATERIAL_DS_REFINE_HOLLY,
};

enum ESpecialSubTypes
{
	SPECIAL_MAP,
	SPECIAL_KEY,
	SPECIAL_DOC,
	SPECIAL_SPIRIT,
};

enum EToolSubTypes
{
	TOOL_FISHING_ROD
};

enum ELotterySubTypes
{
	LOTTERY_TICKET,
	LOTTERY_INSTANT
};

enum EItemFlag
{
	ITEM_FLAG_REFINEABLE		= (1 << 0),
	ITEM_FLAG_SAVE			= (1 << 1),
	ITEM_FLAG_STACKABLE		= (1 << 2),	// ¿©·¯°³ ÇÕÄ¥ ¼ö ÀÖÀ½
	ITEM_FLAG_COUNT_PER_1GOLD	= (1 << 3),
	ITEM_FLAG_SLOW_QUERY		= (1 << 4),
	ITEM_FLAG_UNUSED01		= (1 << 5),	// UNUSED
	ITEM_FLAG_UNIQUE		= (1 << 6), // UNUSED
	ITEM_FLAG_MAKECOUNT		= (1 << 7),
	ITEM_FLAG_IRREMOVABLE		= (1 << 8),
	ITEM_FLAG_CONFIRM_WHEN_USE	= (1 << 9),
	ITEM_FLAG_QUEST_USE		= (1 << 10),
	ITEM_FLAG_QUEST_USE_MULTIPLE	= (1 << 11),
	ITEM_FLAG_QUEST_GIVE		= (1 << 12), //UNUSED
	ITEM_FLAG_LOG			= (1 << 13),
	ITEM_FLAG_APPLICABLE		= (1 << 14), //UNUSED
	ITEM_FLAG_MONEY_ITEM	= (1 << 15),
	ITEM_FLAG_TIER_3 = (1 << 16),
	ITEM_FLAG_TIER_4 = (1 << 17),
};

enum EItemAntiFlag
{
	ITEM_ANTIFLAG_FEMALE	= (1 << 0), // ¿©¼º »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_MALE		= (1 << 1), // ³²¼º »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_WARRIOR	= (1 << 2), // ¹«»ç »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_ASSASSIN	= (1 << 3), // ÀÚ°´ »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_SURA		= (1 << 4), // ¼ö¶ó »ç¿ë ºÒ°¡ 
	ITEM_ANTIFLAG_SHAMAN	= (1 << 5), // ¹«´ç »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_GET		= (1 << 6), // ÁýÀ» ¼ö ¾øÀ½
	ITEM_ANTIFLAG_DROP		= (1 << 7), // ¹ö¸± ¼ö ¾øÀ½
	ITEM_ANTIFLAG_SELL		= (1 << 8), // ÆÈ ¼ö ¾øÀ½
	ITEM_ANTIFLAG_EMPIRE_A	= (1 << 9), // A Á¦±¹ »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_EMPIRE_B	= (1 << 10), // B Á¦±¹ »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_EMPIRE_C	= (1 << 11), // C Á¦±¹ »ç¿ë ºÒ°¡
	ITEM_ANTIFLAG_SAVE		= (1 << 12), // ÀúÀåµÇÁö ¾ÊÀ½
	ITEM_ANTIFLAG_GIVE		= (1 << 13), // °Å·¡ ºÒ°¡
	ITEM_ANTIFLAG_PKDROP	= (1 << 14), // PK½Ã ¶³¾îÁöÁö ¾ÊÀ½
	ITEM_ANTIFLAG_STACK		= (1 << 15), // ÇÕÄ¥ ¼ö ¾øÀ½
	ITEM_ANTIFLAG_MYSHOP	= (1 << 16), // °³ÀÎ »óÁ¡¿¡ ¿Ã¸± ¼ö ¾øÀ½
	ITEM_ANTIFLAG_SAFEBOX	= (1 << 17), // Ã¢°í¿¡ ³ÖÀ» ¼ö ¾øÀ½
	ITEM_ANTIFLAG_WOLFMAN = (1 << 18),	// °³ÀÎ »óÁ¡¿¡ ¿Ã¸± ¼ö ¾øÀ½
	ITEM_ANTIFLAG_CHANGE_ATTRIBUTE = (1 << 19),
	ITEM_ANTIFLAG_DESTROY = (1 << 20),	
	ITEM_ANTIFLAG_QUICKSLOT	= (1 << 21), // 퀵슬롯에 등록 불가.
};

enum EItemWearableFlag
{
	WEARABLE_BODY	= (1 << 0),
	WEARABLE_HEAD	= (1 << 1),
	WEARABLE_FOOTS	= (1 << 2),
	WEARABLE_WRIST	= (1 << 3),
	WEARABLE_WEAPON	= (1 << 4),
	WEARABLE_NECK	= (1 << 5),
	WEARABLE_EAR	= (1 << 6),
	WEARABLE_UNIQUE	= (1 << 7),
	WEARABLE_SHIELD	= (1 << 8),
	WEARABLE_ARROW	= (1 << 9),
	WEARABLE_HAIR	= (1 << 10),
	WEARABLE_ABILITY		= (1 << 11),
	WEARABLE_COSTUME_BODY	= (1 << 12),
};

enum ELimitTypes
{
	LIMIT_NONE,

	LIMIT_LEVEL,
	LIMIT_STR,
	LIMIT_DEX,
	LIMIT_INT,
	LIMIT_CON,

	/// Âø¿ë ¿©ºÎ¿Í »ó°ü ¾øÀÌ ½Ç½Ã°£À¸·Î ½Ã°£ Â÷°¨ (socket0¿¡ ¼Ò¸ê ½Ã°£ÀÌ ¹ÚÈû: unix_timestamp Å¸ÀÔ)
	LIMIT_REAL_TIME,						

	/// ¾ÆÀÌÅÛÀ» ¸Ç Ã³À½ »ç¿ë(È¤Àº Âø¿ë) ÇÑ ¼ø°£ºÎÅÍ ¸®¾óÅ¸ÀÓ Å¸ÀÌ¸Ó ½ÃÀÛ 
	/// ÃÖÃÊ »ç¿ë Àü¿¡´Â socket0¿¡ »ç¿ë°¡´É½Ã°£(ÃÊ´ÜÀ§, 0ÀÌ¸é ÇÁ·ÎÅäÀÇ limit value°ª »ç¿ë) °ªÀÌ ¾²¿©ÀÖ´Ù°¡ 
	/// ¾ÆÀÌÅÛ »ç¿ë½Ã socket1¿¡ »ç¿ë È½¼ö°¡ ¹ÚÈ÷°í socket0¿¡ unix_timestamp Å¸ÀÔÀÇ ¼Ò¸ê½Ã°£ÀÌ ¹ÚÈû.
	LIMIT_REAL_TIME_START_FIRST_USE,

	/// ¾ÆÀÌÅÛÀ» Âø¿ë ÁßÀÏ ¶§¸¸ »ç¿ë ½Ã°£ÀÌ Â÷°¨µÇ´Â ¾ÆÀÌÅÛ
	/// socket0¿¡ ³²Àº ½Ã°£ÀÌ ÃÊ´ÜÀ§·Î ¹ÚÈû. (¾ÆÀÌÅÛ ÃÖÃÊ »ç¿ë½Ã ÇØ´ç °ªÀÌ 0ÀÌ¸é ÇÁ·ÎÅäÀÇ limit value°ªÀ» socket0¿¡ º¹»ç)
	LIMIT_TIMER_BASED_ON_WEAR,

	LIMIT_MAX_LEVEL,
	LIMIT_MAP,

	LIMIT_MAX_NUM
};

enum EAttrAddonTypes
{
	ATTR_ADDON_NONE,
	// positive values are reserved for set
	ATTR_DAMAGE_ADDON = -1,
};

enum EApplyTypes
{
	APPLY_NONE,			// 0
	APPLY_MAX_HP,		// 1
	APPLY_MAX_SP,		// 2
	APPLY_CON,			// 3
	APPLY_INT,			// 4
	APPLY_STR,			// 5
	APPLY_DEX,			// 6
	APPLY_ATT_SPEED,	// 7
	APPLY_MOV_SPEED,	// 8
	APPLY_CAST_SPEED,	// 9
	APPLY_HP_REGEN,		// 10
	APPLY_SP_REGEN,		// 11
	APPLY_POISON_PCT,	// 12
	APPLY_STUN_PCT,		// 13
	APPLY_SLOW_PCT,		// 14
	APPLY_CRITICAL_PCT,		// 15
	APPLY_PENETRATE_PCT,	// 16
	APPLY_ATTBONUS_HUMAN,	// 17
	APPLY_ATTBONUS_ANIMAL,	// 18
	APPLY_ATTBONUS_ORC,		// 19
	APPLY_ATTBONUS_MILGYO,	// 20
	APPLY_ATTBONUS_UNDEAD,	// 21
	APPLY_ATTBONUS_DEVIL,	// 22
	APPLY_STEAL_HP,			// 23
	APPLY_STEAL_SP,			// 24
	APPLY_MANA_BURN_PCT,	// 25
	APPLY_DAMAGE_SP_RECOVER,	// 26
	APPLY_BLOCK,			// 27
	APPLY_DODGE,			// 28
	APPLY_RESIST_SWORD,		// 29
	APPLY_RESIST_TWOHAND,	// 30
	APPLY_RESIST_DAGGER,	// 31
	APPLY_RESIST_BELL,		// 32
	APPLY_RESIST_FAN,		// 33
	APPLY_RESIST_BOW,		// 34
	APPLY_RESIST_FIRE,		// 35
	APPLY_RESIST_ELEC,		// 36
	APPLY_RESIST_MAGIC,		// 37
	APPLY_RESIST_WIND,		// 38
	APPLY_REFLECT_MELEE,	// 39
	APPLY_REFLECT_CURSE,	// 40
	APPLY_POISON_REDUCE,	// 41
	APPLY_KILL_SP_RECOVER,	// 42
	APPLY_EXP_DOUBLE_BONUS,	// 43
	APPLY_GOLD_DOUBLE_BONUS,	// 44
	APPLY_ITEM_DROP_BONUS,	// 45
	APPLY_POTION_BONUS,		// 46
	APPLY_KILL_HP_RECOVER,	// 47
	APPLY_IMMUNE_STUN,		// 48
	APPLY_IMMUNE_SLOW,		// 49
	APPLY_IMMUNE_FALL,		// 50
	APPLY_SKILL,			// 51
	APPLY_BOW_DISTANCE,		// 52
	APPLY_ATT_GRADE_BONUS,	// 53
	APPLY_DEF_GRADE_BONUS,	// 54
	APPLY_MAGIC_ATT_GRADE,	// 55
	APPLY_MAGIC_DEF_GRADE,	// 56
	APPLY_CURSE_PCT,		// 57
	APPLY_MAX_STAMINA,		// 58
	APPLY_ATTBONUS_WARRIOR,	// 59
	APPLY_ATTBONUS_ASSASSIN,	// 60
	APPLY_ATTBONUS_SURA,	// 61
	APPLY_ATTBONUS_SHAMAN,	// 62
	APPLY_ATTBONUS_MONSTER,	// 63
	APPLY_MALL_ATTBONUS,			// 64 °ø°Ý·Â +x%
	APPLY_MALL_DEFBONUS,			// 65 ¹æ¾î·Â +x%
	APPLY_MALL_EXPBONUS,			// 66 °æÇèÄ¡ +x%
	APPLY_MALL_ITEMBONUS,			// 67 ¾ÆÀÌÅÛ µå·ÓÀ² x/10¹è
	APPLY_MALL_GOLDBONUS,			// 68 µ· µå·ÓÀ² x/10¹è
	APPLY_MAX_HP_PCT,				// 69 ÃÖ´ë »ý¸í·Â +x%
	APPLY_MAX_SP_PCT,				// 70 ÃÖ´ë Á¤½Å·Â +x%
	APPLY_SKILL_DAMAGE_BONUS,		// 71 ½ºÅ³ µ¥¹ÌÁö * (100+x)%
	APPLY_NORMAL_HIT_DAMAGE_BONUS,	// 72 ÆòÅ¸ µ¥¹ÌÁö * (100+x)%
	APPLY_SKILL_DEFEND_BONUS,		// 73 ½ºÅ³ µ¥¹ÌÁö ¹æ¾î * (100-x)%
	APPLY_NORMAL_HIT_DEFEND_BONUS,	// 74 ÆòÅ¸ µ¥¹ÌÁö ¹æ¾î * (100-x)%
	APPLY_PC_BANG_EXP_BONUS,		// 75 PC¹æ ¾ÆÀÌÅÛ EXP º¸³Ê½º
	APPLY_PC_BANG_DROP_BONUS,		// 76 PC¹æ ¾ÆÀÌÅÛ µå·ÓÀ² º¸³Ê½º

	APPLY_EXTRACT_HP_PCT,			// 77 »ç¿ë½Ã HP ¼Ò¸ð

	APPLY_RESIST_WARRIOR,			// 78 ¹«»ç¿¡°Ô ÀúÇ×
	APPLY_RESIST_ASSASSIN,			// 79 ÀÚ°´¿¡°Ô ÀúÇ×
	APPLY_RESIST_SURA,				// 80 ¼ö¶ó¿¡°Ô ÀúÇ×
	APPLY_RESIST_SHAMAN,			// 81 ¹«´ç¿¡°Ô ÀúÇ×
	APPLY_ENERGY,					// 82 ±â·Â
	APPLY_DEF_GRADE,				// 83 ¹æ¾î·Â. DEF_GRADE_BONUS´Â Å¬¶ó¿¡¼­ µÎ¹è·Î º¸¿©Áö´Â ÀÇµµµÈ ¹ö±×(...)°¡ ÀÖ´Ù.
	APPLY_COSTUME_ATTR_BONUS,		// 84 ÄÚ½ºÆ¬ ¾ÆÀÌÅÛ¿¡ ºÙÀº ¼Ó¼ºÄ¡ º¸³Ê½º
	APPLY_MAGIC_ATTBONUS_PER,		// 85 ¸¶¹ý °ø°Ý·Â +x%
	APPLY_MELEE_MAGIC_ATTBONUS_PER,			// 86 ¸¶¹ý + ¹Ð¸® °ø°Ý·Â +x%
	
	APPLY_RESIST_ICE,		// 87 ³Ã±â ÀúÇ×
	APPLY_RESIST_EARTH,		// 88 ´ëÁö ÀúÇ×
	APPLY_RESIST_DARK,		// 89 ¾îµÒ ÀúÇ×

	APPLY_ANTI_CRITICAL_PCT,	//90 Å©¸®Æ¼ÄÃ ÀúÇ×
	APPLY_ANTI_PENETRATE_PCT,	//91 °üÅëÅ¸°Ý ÀúÇ×

	APPLY_BOOST_CRITICAL_DMG, //92. If you hit critical, your total damage is increased by X%
	APPLY_BOOST_PENETRATE_DMG, //93. If you pierce, your total damage is increased by X%
	APPLY_ATTBONUS_INSECT,	// 94

	APPLY_BLEEDING_PCT,	// 95
	APPLY_BLEEDING_REDUCE,	// 96
	APPLY_ATTBONUS_WOLFMAN,	// 97
	APPLY_RESIST_WOLFMAN,	// 98
	APPLY_RESIST_CLAW,	// 99
	APPLY_ACCEDRAIN_RATE,	// 100

	APPLY_ATTBONUS_METIN,	// 101
	APPLY_ATTBONUS_TRENT,	// 102
	APPLY_ATTBONUS_BOSS,	// 103
	
	APPLY_RESIST_HUMAN,
	APPLY_ENCHANT_ELECT,
	APPLY_ENCHANT_FIRE,
	APPLY_ENCHANT_ICE,
	APPLY_ENCHANT_WIND,
	APPLY_ENCHANT_EARTH,
	APPLY_ENCHANT_DARK,
	APPLY_ATTBONUS_DESERT,
	APPLY_ATTBONUS_CZ,
	APPLY_ATTBONUS_LEGEND,
	
	MAX_APPLY_NUM,              // 
};

enum EImmuneFlags
{
	IMMUNE_STUN		= (1 << 0),
	IMMUNE_SLOW		= (1 << 1),
	IMMUNE_FALL		= (1 << 2),
	IMMUNE_CURSE	= (1 << 3),
	IMMUNE_POISON	= (1 << 4),
	IMMUNE_TERROR	= (1 << 5),
	IMMUNE_REFLECT	= (1 << 6),
	IMMUNE_BLEEDING	= (1 << 7),
};

enum EAIFlags
{
	AIFLAG_AGGRESSIVE = (1 << 0),
	AIFLAG_NOMOVE = (1 << 1),
	AIFLAG_COWARD = (1 << 2),
	AIFLAG_NOATTACKSHINSU = (1 << 3),
	AIFLAG_NOATTACKJINNO = (1 << 4),
	AIFLAG_NOATTACKCHUNJO = (1 << 5),
	AIFLAG_ATTACKMOB = (1 << 6),
	AIFLAG_BERSERK = (1 << 7),
	AIFLAG_STONESKIN = (1 << 8),
	AIFLAG_GODSPEED = (1 << 9),
	AIFLAG_DEATHBLOW = (1 << 10),
	AIFLAG_REVIVE = (1 << 11),
	AIFLAG_HEALER = (1 << 12),
	AIFLAG_COUNT = (1 << 13), // Attack Count instead of damage
	AIFLAG_NORECOVERY = (1 << 14), // Disables recovery
	AIFLAG_REFLECT = (1 << 15), // 20% Reflect on attacks
	AIFLAG_FALL = (1 << 16), // Gives affect AFFECT_MOUNT_FALL to players
	AIFLAG_VIT = (1 << 17), // Unknown
	AIFLAG_RATTSPEED = (1 << 18), // Gives affect AFFECT_ATT_SPEED_SLOW to players
	AIFLAG_RCASTSPEED = (1 << 19), // Gives affect AFFECT_REDUCE_CAST_SPEED to players
	AIFLAG_RHP_REGEN = (1 << 20), // Gives affect AFFECT_NO_RECOVERY to players
	AIFLAG_TIMEVIT = (1 << 21), // Unknown
	AIFLAG_NOPUSH = (1 << 22),
};

enum EOnClickEvents
{
	ON_CLICK_NONE,
	ON_CLICK_SHOP,
	ON_CLICK_TALK,
	ON_CLICK_MAX_NUM
};

#pragma pack(push, 1)


typedef struct SItemLimit
{
	uint8_t bType;
	int32_t lValue;
} TItemLimit;

typedef struct SItemApply
{
	uint8_t bType;
	int32_t lValue;
} TItemApply;

typedef struct SItemTable
{
	static const uint32_t kVersion = 2;

	uint32_t       dwVnum;
	uint32_t       dwVnumRange;
	std::string       szName;
	std::string        szLocaleName;
	uint8_t        bType;
	uint8_t        bSubType;

	uint8_t        bWeight;
	uint8_t        bSize;

	uint32_t       dwAntiFlags;
	uint32_t       dwFlags;
	uint32_t       dwWearFlags;
	uint32_t       dwImmuneFlag;

	Gold       dwIBuyItemPrice;
	Gold		dwISellItemPrice;

	TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
	TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
	long        alValues[ITEM_VALUES_MAX_NUM];
	SocketValue       alSockets[ITEM_SOCKET_MAX_NUM];
	uint32_t       dwRefinedVnum;
	uint16_t		wRefineSet;
	uint8_t        bAlterToMagicItemPct;
	uint8_t		bSpecular;
	uint8_t        bGainSocketPct;
	short int sAddonType; // ±âº» ¼Ó¼º

} TItemTable;

#pragma pack(pop)




}



BOOST_FUSION_ADAPT_STRUCT(
	old::TItemTable,
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
	dwIBuyItemPrice,
	dwISellItemPrice,
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
	old::TItemLimit,
	bType,
	lValue
);

BOOST_FUSION_ADAPT_STRUCT(
	old::TItemApply,
	bType,
	lValue
);



class ItemList;
class NpcList;


METIN2_END_NS

#endif
