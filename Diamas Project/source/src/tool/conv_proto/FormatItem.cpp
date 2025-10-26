
#include "ItemList.hpp"
#include "NpcList.hpp"

#include <game/Constants.hpp>
#include <game/ItemTypes.hpp>
#include <game/MobTypes.hpp>

#include <storm/StringUtil.hpp>

#include <cstdio>
#include <cstring>
#include "spdlog/spdlog.h"
#include <game/Format.hpp>

METIN2_BEGIN_NS

namespace
{

// Most of this method is generated...
bool ConvertItemType(ItemProto& entry, const old::TItemTable& table)
{
	switch (table.bType) {
		case old::ITEM_NONE: entry.type = ITEM_NONE; break;

		case old::ITEM_WEAPON: {
			entry.type = ITEM_WEAPON;

			switch (table.bSubType) {
				case old::WEAPON_SWORD: entry.subType = WEAPON_SWORD; break;
				case old::WEAPON_DAGGER: entry.subType = WEAPON_DAGGER; break;
				case old::WEAPON_BOW: entry.subType = WEAPON_BOW; break;
				case old::WEAPON_TWO_HANDED: entry.subType = WEAPON_TWO_HANDED; break;
				case old::WEAPON_BELL: entry.subType = WEAPON_BELL; break;
				case old::WEAPON_FAN: entry.subType = WEAPON_FAN; break;
				case old::WEAPON_ARROW: entry.subType = WEAPON_ARROW; break;
				case old::WEAPON_MOUNT_SPEAR: entry.subType = WEAPON_MOUNT_SPEAR; break;
				case old::WEAPON_CLAW: entry.subType = WEAPON_CLAW; break;

				default:
					spdlog::error("Failed to find weapon subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_ARMOR: {
			entry.type = ITEM_ARMOR;

			switch (table.bSubType) {
				case old::ARMOR_BODY: entry.subType = ARMOR_BODY; break;
				case old::ARMOR_HEAD: entry.subType = ARMOR_HEAD; break;
				case old::ARMOR_SHIELD: entry.subType = ARMOR_SHIELD; break;
				case old::ARMOR_WRIST: entry.subType = ARMOR_WRIST; break;
				case old::ARMOR_FOOTS: entry.subType = ARMOR_FOOTS; break;
				case old::ARMOR_NECK: entry.subType = ARMOR_NECK; break;
				case old::ARMOR_EAR: entry.subType = ARMOR_EAR; break;

				default:
					spdlog::error("Failed to find armor subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_USE: {
			entry.type = ITEM_USE;

			switch (table.bSubType) {
				case old::USE_POTION: entry.subType = USE_POTION; break;
				case old::USE_TALISMAN: entry.subType = USE_TALISMAN; break;
				case old::USE_TUNING: entry.subType = USE_TUNING; break;
				case old::USE_MOVE: entry.subType = USE_MOVE; break;
				case old::USE_TREASURE_BOX: entry.subType = USE_TREASURE_BOX; break;
				case old::USE_MONEYBAG: entry.subType = USE_MONEYBAG; break;
				case old::USE_BAIT: entry.subType = USE_BAIT; break;
				case old::USE_ABILITY_UP: entry.subType = USE_ABILITY_UP; break;
				case old::USE_AFFECT: entry.subType = USE_AFFECT; break;
				case old::USE_CREATE_STONE: entry.subType = USE_CREATE_STONE; break;

				case old::USE_SPECIAL: {
						entry.subType = USE_SPECIAL;
					break;
				}

				case old::USE_POTION_NODELAY: entry.subType = USE_POTION_NODELAY; break;
				case old::USE_CLEAR: entry.subType = USE_CLEAR; break;
				case old::USE_INVISIBILITY: entry.subType = USE_INVISIBILITY; break;
				case old::USE_DETACHMENT: entry.subType = USE_DETACHMENT; break;
				case old::USE_BUCKET: entry.subType = USE_BUCKET; break;
				case old::USE_POTION_CONTINUE: entry.subType = USE_POTION_CONTINUE; break;
				case old::USE_CLEAN_SOCKET: entry.subType = USE_CLEAN_SOCKET; break;
				case old::USE_CHANGE_ATTRIBUTE: entry.subType = USE_CHANGE_ATTRIBUTE; break;
				case old::USE_ADD_ATTRIBUTE: entry.subType = USE_ADD_ATTRIBUTE; break;
				case old::USE_ADD_ACCESSORY_SOCKET: entry.subType = USE_ADD_ACCESSORY_SOCKET; break;
				case old::USE_PUT_INTO_ACCESSORY_SOCKET: entry.subType = USE_PUT_INTO_ACCESSORY_SOCKET; break;
				case old::USE_ADD_ATTRIBUTE2: entry.subType = USE_ADD_ATTRIBUTE2; break;
				case old::USE_RECIPE: entry.subType = USE_RECIPE; break;
				case old::USE_CHANGE_ATTRIBUTE2: entry.subType = USE_CHANGE_ATTRIBUTE2; break;
				case old::USE_BIND: entry.subType = USE_BIND; break;
				case old::USE_UNBIND: entry.subType = USE_UNBIND; break;
				case old::USE_TIME_CHARGE_PER: entry.subType = USE_TIME_CHARGE_PER; break;
				case old::USE_TIME_CHARGE_FIX: entry.subType = USE_TIME_CHARGE_FIX; break;
				case old::USE_PUT_INTO_BELT_SOCKET: entry.subType = USE_PUT_INTO_BELT_SOCKET; break;
				case old::USE_PUT_INTO_RING_SOCKET: entry.subType = USE_PUT_INTO_RING_SOCKET; break;
				case old::USE_CHANGE_COSTUME_ATTR: return false;
				case old::USE_RESET_COSTUME_ATTR: return false;

				default:
				entry.subType = table.bSubType;
					spdlog::error("Failed to find use subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_AUTOUSE: return false;

		case old::ITEM_MATERIAL: {
			entry.type = ITEM_MATERIAL;

			switch (table.bSubType) {
				case old::MATERIAL_LEATHER: entry.subType = MATERIAL_LEATHER; break;
				case old::MATERIAL_BLOOD: entry.subType = MATERIAL_BLOOD; break;
				case old::MATERIAL_ROOT: entry.subType = MATERIAL_ROOT; break;
				case old::MATERIAL_NEEDLE: entry.subType = MATERIAL_NEEDLE; break;
				case old::MATERIAL_JEWEL: entry.subType = MATERIAL_JEWEL; break;
				case old::MATERIAL_DS_REFINE_NORMAL: entry.subType = MATERIAL_DS_REFINE_NORMAL; break;
				case old::MATERIAL_DS_REFINE_BLESSED: entry.subType = MATERIAL_DS_REFINE_BLESSED; break;
				case old::MATERIAL_DS_REFINE_HOLLY: entry.subType = MATERIAL_DS_REFINE_HOLLY; break;

				default:
					spdlog::error("Failed to find material subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_SPECIAL: {
			entry.type = ITEM_SPECIAL;

			const bool isHorseFood = table.dwVnum > 50053 && table.dwVnum < 50057;
			const bool isHorseRevival = table.dwVnum > 50056 && table.dwVnum < 50060;

			if (isHorseFood)
				entry.subType = SPECIAL_HORSE_FOOD;
			else if (isHorseRevival)
				entry.subType = SPECIAL_HORSE_REVIVAL;
			else
				entry.subType = SPECIAL_NONE;

			break;
		}

		case old::ITEM_TOOL: return false;
		case old::ITEM_LOTTERY: return false;
		case old::ITEM_ELK: entry.type = ITEM_ELK; break;

		case old::ITEM_METIN: {
			entry.type = ITEM_METIN;

			switch (table.bSubType) {
				case old::METIN_NORMAL: entry.subType = METIN_NORMAL; break;
				case old::METIN_GOLD: entry.subType = METIN_GOLD; break;

				default:
					spdlog::error("Failed to find metin subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_CONTAINER: return false;

		case old::ITEM_FISH: {
			entry.type = ITEM_FISH;

			switch (table.bSubType) {
				case old::FISH_ALIVE: entry.subType = FISH_ALIVE; break;
				case old::FISH_DEAD: entry.subType = FISH_DEAD; break;

				default:
					spdlog::error("Failed to find fish subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_ROD: entry.type = ITEM_ROD; break;

		case old::ITEM_RESOURCE: {
			entry.type = ITEM_RESOURCE;

			switch (table.bSubType) {
				case old::RESOURCE_FISHBONE: entry.subType = RESOURCE_FISHBONE; break;
				case old::RESOURCE_WATERSTONEPIECE: entry.subType = RESOURCE_WATERSTONEPIECE; break;
				case old::RESOURCE_WATERSTONE: entry.subType = RESOURCE_WATERSTONE; break;
				case old::RESOURCE_BLOOD_PEARL: entry.subType = RESOURCE_BLOOD_PEARL; break;
				case old::RESOURCE_BLUE_PEARL: entry.subType = RESOURCE_BLUE_PEARL; break;
				case old::RESOURCE_WHITE_PEARL: entry.subType = RESOURCE_WHITE_PEARL; break;
				case old::RESOURCE_BUCKET: entry.subType = RESOURCE_BUCKET; break;
				case old::RESOURCE_CRYSTAL: entry.subType = RESOURCE_CRYSTAL; break;
				case old::RESOURCE_GEM: entry.subType = RESOURCE_GEM; break;
				case old::RESOURCE_STONE: entry.subType = RESOURCE_STONE; break;
				case old::RESOURCE_METIN: entry.subType = RESOURCE_METIN; break;
				case old::RESOURCE_ORE: entry.subType = RESOURCE_ORE; break;

				default:
					spdlog::error("Failed to find resource subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_CAMPFIRE: entry.type = ITEM_CAMPFIRE; break;

		case old::ITEM_UNIQUE: {
			entry.type = ITEM_UNIQUE;

			switch (table.bSubType) {
				case old::UNIQUE_NONE: entry.subType = UNIQUE_NONE; break;
				case old::UNIQUE_BOOK: entry.subType = UNIQUE_BOOK; break;
				case old::UNIQUE_SPECIAL_RIDE:
				case old::UNIQUE_SPECIAL_MOUNT_RIDE:
					entry.type = ITEM_TOGGLE;
					entry.subType = TOGGLE_MOUNT;
					break;

				// They really fucked this one up...
				case old::USE_SPECIAL /*10*/: {
					const bool isSilkBotary = table.dwVnum == 71049;

					if (isSilkBotary) {
						entry.type = ITEM_USE;
						entry.subType = USE_SILK_BOTARY;
					} else {
						SPDLOG_WARN("Unhandled silk botary item {0}", table.dwVnum);
						return false;
					}

					break;
				}

				default:
					spdlog::error("Failed to find unique subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_SKILLBOOK: entry.type = ITEM_SKILLBOOK; break;
		case old::ITEM_QUEST: entry.type = ITEM_QUEST; break;

		case old::ITEM_POLYMORPH: {
			entry.type = ITEM_POLYMORPH;

			const bool isBall1 = table.dwVnum > 70103 && table.dwVnum < 70108;
			const bool isBall2 = table.dwVnum == 71093;

			if (isBall1 || isBall2)
				entry.subType = POLYMORPH_BALL;

			if (table.dwVnum == 50322)
				entry.subType = POLYMORPH_BOOK;

			break;
		}

		case old::ITEM_TREASURE_BOX: entry.type = ITEM_TREASURE_BOX; break;
		case old::ITEM_TREASURE_KEY: entry.type = ITEM_TREASURE_KEY; break;
		case old::ITEM_SKILLFORGET: entry.type = ITEM_SKILLFORGET; break;
		case old::ITEM_GIFTBOX: entry.type = ITEM_GIFTBOX; break;
		case old::ITEM_PICK: entry.type = ITEM_PICK; break;

		case old::ITEM_HAIR: {
			entry.type = ITEM_COSTUME;
			entry.subType = COSTUME_HAIR;
			break;
		}

		case old::ITEM_TOTEM: return false;
		case old::ITEM_BLEND: entry.type = ITEM_BLEND; break;
		case old::ITEM_COSTUME: {
			entry.type = ITEM_COSTUME;
			entry.antiFlags |= ITEM_ANTIFLAG_CHANGE_ATTRIBUTE;

			switch (table.bSubType) {
				case old::COSTUME_BODY: entry.subType = COSTUME_BODY; break;
				case old::COSTUME_HAIR: entry.subType = COSTUME_HAIR; break;
				case old::COSTUME_RANK: entry.subType = COSTUME_RANK; break;

				case old::COSTUME_MOUNT:
					entry.type = ITEM_TOGGLE;
					entry.subType = TOGGLE_MOUNT;
					break;
				case old::COSTUME_ACCE:
					entry.subType = COSTUME_ACCE;
					break;

				default:
					spdlog::error("Failed to find costume subType {0}", table.bSubType);
					return false;
			}

			break;
		}

		case old::ITEM_DS:  entry.type = ITEM_DS; break;
		case old::ITEM_SPECIAL_DS: entry.type = ITEM_SPECIAL_DS; break;
		case old::ITEM_EXTRACT: entry.type = ITEM_EXTRACT; break;

		case old::ITEM_SECONDARY_COIN: entry.type = ITEM_SECONDARY_COIN; break;
		case old::ITEM_RING: entry.type = ITEM_RING; break;
		case old::ITEM_BELT: entry.type = ITEM_BELT; break;
		case old::ITEM_PET: entry.type = ITEM_PET; break;
	}

	return true;
}

void ConvertItemFlags(ItemProto& entry, const old::TItemTable& table)
{
	if (table.dwFlags & old::ITEM_FLAG_REFINEABLE)
		entry.flags |= ITEM_FLAG_REFINEABLE;
	if (table.dwFlags & old::ITEM_FLAG_SAVE)
		entry.flags |= ITEM_FLAG_SAVE;
	if (table.dwFlags & old::ITEM_FLAG_STACKABLE)
		entry.flags |= ITEM_FLAG_STACKABLE;
	if (table.dwFlags & old::ITEM_FLAG_COUNT_PER_1GOLD)
		entry.flags |= ITEM_FLAG_COUNT_PER_1GOLD;
	if (table.dwFlags & old::ITEM_FLAG_SLOW_QUERY)
		entry.flags |= ITEM_FLAG_SLOW_QUERY;
	if (table.dwFlags & old::ITEM_FLAG_UNUSED01)
		entry.flags |= ITEM_FLAG_UNUSED01;
	if (table.dwFlags & old::ITEM_FLAG_UNIQUE)
		entry.flags |= ITEM_FLAG_UNIQUE;
	if (table.dwFlags & old::ITEM_FLAG_MAKECOUNT)
		entry.flags |= ITEM_FLAG_MAKECOUNT;
	if (table.dwFlags & old::ITEM_FLAG_IRREMOVABLE)
		entry.flags |= ITEM_FLAG_IRREMOVABLE;
	if (table.dwFlags & old::ITEM_FLAG_CONFIRM_WHEN_USE)
		entry.flags |= ITEM_FLAG_CONFIRM_WHEN_USE;
	if (table.dwFlags & old::ITEM_FLAG_QUEST_USE)
		entry.flags |= ITEM_FLAG_QUEST_USE;
	if (table.dwFlags & old::ITEM_FLAG_QUEST_USE_MULTIPLE)
		entry.flags |= ITEM_FLAG_QUEST_USE_MULTIPLE;
	if (table.dwFlags & old::ITEM_FLAG_QUEST_GIVE)
		entry.flags |= ITEM_FLAG_QUEST_GIVE;
	if (table.dwFlags & old::ITEM_FLAG_LOG)
		entry.flags |= ITEM_FLAG_LOG;
	if (table.dwFlags & old::ITEM_FLAG_APPLICABLE)
		entry.flags |= ITEM_FLAG_APPLICABLE;
}

void ConvertItemAntiFlags(ItemProto& entry, const old::TItemTable& table)
{
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_FEMALE)
		entry.antiFlags |= ITEM_ANTIFLAG_FEMALE;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_MALE)
		entry.antiFlags |= ITEM_ANTIFLAG_MALE;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_WARRIOR)
		entry.antiFlags |= ITEM_ANTIFLAG_WARRIOR;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_ASSASSIN)
		entry.antiFlags |= ITEM_ANTIFLAG_ASSASSIN;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_SURA)
		entry.antiFlags |= ITEM_ANTIFLAG_SURA;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_SHAMAN)
		entry.antiFlags |= ITEM_ANTIFLAG_SHAMAN;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_GET)
		entry.antiFlags |= ITEM_ANTIFLAG_GET;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_DROP)
		entry.antiFlags |= ITEM_ANTIFLAG_DROP;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_SELL)
		entry.antiFlags |= ITEM_ANTIFLAG_SELL;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_EMPIRE_A)
		entry.antiFlags |= ITEM_ANTIFLAG_EMPIRE_A;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_EMPIRE_B)
		entry.antiFlags |= ITEM_ANTIFLAG_EMPIRE_B;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_EMPIRE_C)
		entry.antiFlags |= ITEM_ANTIFLAG_EMPIRE_C;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_SAVE)
		entry.antiFlags |= ITEM_ANTIFLAG_SAVE;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_GIVE)
		entry.antiFlags |= ITEM_ANTIFLAG_GIVE;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_PKDROP)
		entry.antiFlags |= ITEM_ANTIFLAG_PKDROP;
	//if (table.dwAntiFlags & old::ITEM_ANTIFLAG_STACK)
	//	entry.antiFlags |= ITEM_ANTIFLAG_STACK;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_MYSHOP)
		entry.antiFlags |= ITEM_ANTIFLAG_MYSHOP;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_SAFEBOX)
		entry.antiFlags |= ITEM_ANTIFLAG_SAFEBOX;
	if (table.dwAntiFlags & old::ITEM_ANTIFLAG_WOLFMAN)
		entry.antiFlags |= ITEM_ANTIFLAG_WOLFMAN;
}

void ConvertWearFlags(ItemProto& entry, const old::TItemTable& table)
{
	if (entry.type == ITEM_METIN) {
		// limit.hpp: ITEM_VALUE_METIN_WEARFLAGS = 3
		entry.values[3] = table.dwWearFlags;
	}
}

void ConvertItemLimit(ItemLimit& limit, const old::TItemLimit& limit2)
{
	if (limit2.lValue == 0)
		return;

	switch (limit2.bType) {
		case old::LIMIT_NONE: limit.type = LIMIT_NONE; break;
		case old::LIMIT_LEVEL: limit.type = LIMIT_LEVEL; break;
		case old::LIMIT_STR: limit.type = LIMIT_STR; break;
		case old::LIMIT_DEX: limit.type = LIMIT_DEX; break;
		case old::LIMIT_INT: limit.type = LIMIT_INT; break;
		case old::LIMIT_CON: limit.type = LIMIT_CON; break;
		case old::LIMIT_REAL_TIME: limit.type = LIMIT_REAL_TIME; break;
		case old::LIMIT_REAL_TIME_START_FIRST_USE: limit.type = LIMIT_REAL_TIME_START_FIRST_USE; break;
		case old::LIMIT_TIMER_BASED_ON_WEAR: limit.type = LIMIT_TIMER_BASED_ON_WEAR; break;
	}

	limit.value = limit2.lValue;
}

void ConvertItemApply(ItemApply& apply, const old::TItemApply& apply2)
{
	if (apply2.lValue == 0)
		return;

	switch (apply2.bType) {
		case old::APPLY_NONE: apply.type = APPLY_NONE; break;
		case old::APPLY_MAX_HP: apply.type = APPLY_MAX_HP; break;
		case old::APPLY_MAX_SP: apply.type = APPLY_MAX_SP; break;
		case old::APPLY_CON: apply.type = APPLY_CON; break;
		case old::APPLY_INT: apply.type = APPLY_INT; break;
		case old::APPLY_STR: apply.type = APPLY_STR; break;
		case old::APPLY_DEX: apply.type = APPLY_DEX; break;
		case old::APPLY_ATT_SPEED: apply.type = APPLY_ATT_SPEED; break;
		case old::APPLY_MOV_SPEED: apply.type = APPLY_MOV_SPEED; break;
		case old::APPLY_CAST_SPEED: apply.type = APPLY_CAST_SPEED; break;
		case old::APPLY_HP_REGEN: apply.type = APPLY_HP_REGEN; break;
		case old::APPLY_SP_REGEN: apply.type = APPLY_SP_REGEN; break;
		case old::APPLY_POISON_PCT: apply.type = APPLY_POISON_PCT; break;
		case old::APPLY_STUN_PCT: apply.type = APPLY_STUN_PCT; break;
		case old::APPLY_SLOW_PCT: apply.type = APPLY_SLOW_PCT; break;
		case old::APPLY_CRITICAL_PCT: apply.type = APPLY_CRITICAL_PCT; break;
		case old::APPLY_PENETRATE_PCT: apply.type = APPLY_PENETRATE_PCT; break;
		case old::APPLY_ATTBONUS_HUMAN: apply.type = APPLY_ATTBONUS_HUMAN; break;
		case old::APPLY_ATTBONUS_ANIMAL: apply.type = APPLY_ATTBONUS_ANIMAL; break;
		case old::APPLY_ATTBONUS_ORC: apply.type = APPLY_ATTBONUS_ORC; break;
		case old::APPLY_ATTBONUS_MILGYO: apply.type = APPLY_ATTBONUS_MILGYO; break;
		case old::APPLY_ATTBONUS_UNDEAD: apply.type = APPLY_ATTBONUS_UNDEAD; break;
		case old::APPLY_ATTBONUS_DEVIL: apply.type = APPLY_ATTBONUS_DEVIL; break;
		case old::APPLY_STEAL_HP: apply.type = APPLY_STEAL_HP; break;
		case old::APPLY_STEAL_SP: apply.type = APPLY_STEAL_SP; break;
		case old::APPLY_MANA_BURN_PCT: apply.type = APPLY_MANA_BURN_PCT; break;
		case old::APPLY_DAMAGE_SP_RECOVER: apply.type = APPLY_DAMAGE_SP_RECOVER; break;
		case old::APPLY_BLOCK: apply.type = APPLY_BLOCK; break;
		case old::APPLY_DODGE: apply.type = APPLY_DODGE; break;
		case old::APPLY_RESIST_SWORD: apply.type = APPLY_RESIST_SWORD; break;
		case old::APPLY_RESIST_TWOHAND: apply.type = APPLY_RESIST_TWOHAND; break;
		case old::APPLY_RESIST_DAGGER: apply.type = APPLY_RESIST_DAGGER; break;
		case old::APPLY_RESIST_BELL: apply.type = APPLY_RESIST_BELL; break;
		case old::APPLY_RESIST_FAN: apply.type = APPLY_RESIST_FAN; break;
		case old::APPLY_RESIST_BOW: apply.type = APPLY_RESIST_BOW; break;
		case old::APPLY_RESIST_FIRE: apply.type = APPLY_RESIST_FIRE; break;
		case old::APPLY_RESIST_ELEC: apply.type = APPLY_RESIST_ELEC; break;
		case old::APPLY_RESIST_MAGIC: apply.type = APPLY_RESIST_MAGIC; break;
		case old::APPLY_RESIST_WIND: apply.type = APPLY_RESIST_WIND; break;
		case old::APPLY_REFLECT_MELEE: apply.type = APPLY_REFLECT_MELEE; break;
		case old::APPLY_REFLECT_CURSE: apply.type = APPLY_REFLECT_CURSE; break;
		case old::APPLY_POISON_REDUCE: apply.type = APPLY_POISON_REDUCE; break;
		case old::APPLY_KILL_SP_RECOVER: apply.type = APPLY_KILL_SP_RECOVER; break;
		case old::APPLY_EXP_DOUBLE_BONUS: apply.type = APPLY_EXP_DOUBLE_BONUS; break;
		case old::APPLY_GOLD_DOUBLE_BONUS: apply.type = APPLY_GOLD_DOUBLE_BONUS; break;
		case old::APPLY_ITEM_DROP_BONUS: apply.type = APPLY_ITEM_DROP_BONUS; break;
		case old::APPLY_POTION_BONUS: apply.type = APPLY_POTION_BONUS; break;
		case old::APPLY_KILL_HP_RECOVER: apply.type = APPLY_KILL_HP_RECOVER; break;
		case old::APPLY_IMMUNE_STUN: apply.type = APPLY_IMMUNE_STUN; break;
		case old::APPLY_IMMUNE_SLOW: apply.type = APPLY_IMMUNE_SLOW; break;
		case old::APPLY_IMMUNE_FALL: apply.type = APPLY_IMMUNE_FALL; break;
		case old::APPLY_SKILL: apply.type = APPLY_SKILL; break;
		case old::APPLY_BOW_DISTANCE: apply.type = APPLY_BOW_DISTANCE; break;
		case old::APPLY_ATT_GRADE_BONUS: apply.type = APPLY_ATT_GRADE_BONUS; break;
		case old::APPLY_DEF_GRADE_BONUS: apply.type = APPLY_DEF_GRADE_BONUS; break;
		case old::APPLY_MAGIC_ATT_GRADE: apply.type = APPLY_MAGIC_ATT_GRADE; break;
		case old::APPLY_MAGIC_DEF_GRADE: apply.type = APPLY_MAGIC_DEF_GRADE; break;
		case old::APPLY_CURSE_PCT: apply.type = APPLY_CURSE_PCT; break;
		case old::APPLY_MAX_STAMINA: apply.type = APPLY_MAX_STAMINA; break;
		case old::APPLY_ATTBONUS_WARRIOR: apply.type = APPLY_ATTBONUS_WARRIOR; break;
		case old::APPLY_ATTBONUS_ASSASSIN: apply.type = APPLY_ATTBONUS_ASSASSIN; break;
		case old::APPLY_ATTBONUS_SURA: apply.type = APPLY_ATTBONUS_SURA; break;
		case old::APPLY_ATTBONUS_SHAMAN: apply.type = APPLY_ATTBONUS_SHAMAN; break;
		case old::APPLY_ATTBONUS_MONSTER: apply.type = APPLY_ATTBONUS_MONSTER; break;
		case old::APPLY_MALL_ATTBONUS: apply.type = APPLY_MALL_ATTBONUS; break;
		case old::APPLY_MALL_DEFBONUS: apply.type = APPLY_MALL_DEFBONUS; break;
		case old::APPLY_MALL_EXPBONUS: apply.type = APPLY_MALL_EXPBONUS; break;
		case old::APPLY_MALL_ITEMBONUS: apply.type = APPLY_MALL_ITEMBONUS; break;
		case old::APPLY_MALL_GOLDBONUS: apply.type = APPLY_MALL_GOLDBONUS; break;
		case old::APPLY_MAX_HP_PCT: apply.type = APPLY_MAX_HP_PCT; break;
		case old::APPLY_MAX_SP_PCT: apply.type = APPLY_MAX_SP_PCT; break;
		case old::APPLY_SKILL_DAMAGE_BONUS: apply.type = APPLY_SKILL_DAMAGE_BONUS; break;
		case old::APPLY_NORMAL_HIT_DAMAGE_BONUS: apply.type = APPLY_NORMAL_HIT_DAMAGE_BONUS; break;
		case old::APPLY_SKILL_DEFEND_BONUS: apply.type = APPLY_SKILL_DEFEND_BONUS; break;
		case old::APPLY_NORMAL_HIT_DEFEND_BONUS: apply.type = APPLY_NORMAL_HIT_DEFEND_BONUS; break;
		case old::APPLY_PC_BANG_EXP_BONUS: apply.type = APPLY_PC_BANG_EXP_BONUS; break;
		case old::APPLY_PC_BANG_DROP_BONUS: apply.type = APPLY_PC_BANG_DROP_BONUS; break;
		case old::APPLY_EXTRACT_HP_PCT: apply.type = APPLY_EXTRACT_HP_PCT; break;
		case old::APPLY_RESIST_WARRIOR: apply.type = APPLY_RESIST_WARRIOR; break;
		case old::APPLY_RESIST_ASSASSIN: apply.type = APPLY_RESIST_ASSASSIN; break;
		case old::APPLY_RESIST_SURA: apply.type = APPLY_RESIST_SURA; break;
		case old::APPLY_RESIST_SHAMAN: apply.type = APPLY_RESIST_SHAMAN; break;
		case old::APPLY_ENERGY: apply.type = APPLY_ENERGY; break;
		case old::APPLY_DEF_GRADE: apply.type = APPLY_DEF_GRADE; break;
		case old::APPLY_COSTUME_ATTR_BONUS: apply.type = APPLY_COSTUME_ATTR_BONUS; break;
		case old::APPLY_MAGIC_ATTBONUS_PER: apply.type = APPLY_MAGIC_ATTBONUS_PER; break;
		case old::APPLY_MELEE_MAGIC_ATTBONUS_PER: apply.type = APPLY_MELEE_MAGIC_ATTBONUS_PER; break;
		case old::APPLY_RESIST_ICE: apply.type = APPLY_RESIST_ICE; break;
		case old::APPLY_RESIST_EARTH: apply.type = APPLY_RESIST_EARTH; break;
		case old::APPLY_RESIST_DARK: apply.type = APPLY_RESIST_DARK; break;
		case old::APPLY_ANTI_CRITICAL_PCT: apply.type = APPLY_ANTI_CRITICAL_PCT; break;
		case old::APPLY_ANTI_PENETRATE_PCT: apply.type = APPLY_ANTI_PENETRATE_PCT; break;
	}

	apply.value = apply2.lValue;
}

bool ShouldRemove(const old::TItemTable& table)
{


	return false;
}

}

bool ConvertItemProto(ItemProto& entry, const old::TItemTable& table,
                      const ItemList& itemList)
{
	// Don't even bother processing items we don't need
	if (ShouldRemove(table))
		return false;

	entry.vnum = table.dwVnum;
	entry.vnumRange = table.dwVnumRange;

	entry.type =table.bType;
	entry.subType =table.bSubType;

	entry.size = table.bSize;

	ConvertItemFlags(entry, table);
	ConvertItemAntiFlags(entry, table);

	entry.sellPrice = table.dwISellItemPrice;
	entry.buyPrice = table.dwIBuyItemPrice;
	entry.limits.resize(old::ITEM_LIMIT_MAX_NUM);

	for (int i = 0; i < old::ITEM_LIMIT_MAX_NUM; ++i)
		ConvertItemLimit(entry.limits[i], table.aLimits[i]);
	entry.applies.resize(old::ITEM_APPLY_MAX_NUM);
	for (int i = 0; i < old::ITEM_APPLY_MAX_NUM; ++i)
		ConvertItemApply(entry.applies[i], table.aApplies[i]);

	for (int i = 0; i < ITEM_VALUES_MAX_NUM && i < old::ITEM_VALUES_MAX_NUM; ++i)
		entry.values[i] = table.alValues[i];

	entry.socketCount = table.bGainSocketPct;

	entry.refines[0].refinedVnum = table.dwRefinedVnum;
	entry.refines[0].refineSet = table.wRefineSet;

	entry.magicItemPct = table.bAlterToMagicItemPct;

	entry.attrAddonType = table.sAddonType;

	entry.specular = table.bSpecular;

	const auto* listEntry = itemList.Get(entry.vnum);
	if (listEntry) {
		entry.iconFilename = listEntry->iconFilename;
		entry.modelFilename = listEntry->modelFilename;
	} else {
		char buf[200];
		// Compatibility to old icon loading
		snprintf(buf, sizeof(buf), "icon/item/%05d.tga", entry.vnum);
		entry.iconFilename.assign(buf, strlen(buf));
		SPDLOG_INFO("Adding default icon 'icon/item/{0}.tga'", entry.vnum);
	}

	ConvertWearFlags(entry, table);
	return true;
}

METIN2_END_NS
