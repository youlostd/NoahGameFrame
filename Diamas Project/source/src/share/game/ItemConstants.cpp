#include <game/ItemConstants.hpp>

#include <storm/StringFlags.hpp>

METIN2_BEGIN_NS

using UInt32Table = storm::StringValueTable<uint32_t>;

UInt32Table kItemTypes[] = {
	{ "NONE", ITEM_NONE }, 
	{ "WEAPON", ITEM_WEAPON }, 
	{ "ARMOR", ITEM_ARMOR }, 
	{ "USE", ITEM_USE }, 
	{ "AUTOUSE", ITEM_AUTOUSE }, 
	{ "MATERIAL", ITEM_MATERIAL }, 
	{ "SPECIAL", ITEM_SPECIAL }, 
	{ "TOOL", ITEM_TOOL }, 
	{ "LOTTERY", ITEM_LOTTERY }, 
	{ "ELK", ITEM_ELK }, 
	{ "METIN", ITEM_METIN }, 
	{ "CONTAINER", ITEM_CONTAINER }, 
	{ "FISH", ITEM_FISH }, 
	{ "ROD", ITEM_ROD }, 
	{ "RESOURCE", ITEM_RESOURCE }, 
	{ "CAMPFIRE", ITEM_CAMPFIRE }, 
	{ "UNIQUE", ITEM_UNIQUE }, 
	{ "SKILLBOOK", ITEM_SKILLBOOK }, 
	{ "QUEST", ITEM_QUEST }, 
	{ "POLYMORPH", ITEM_POLYMORPH }, 
	{ "TREASURE_BOX", ITEM_TREASURE_BOX }, 
	{ "TREASURE_KEY", ITEM_TREASURE_KEY }, 
	{ "SKILLFORGET", ITEM_SKILLFORGET }, 
	{ "GIFTBOX", ITEM_GIFTBOX }, 
	{ "PICK", ITEM_PICK }, 
	{ "HAIR", ITEM_HAIR }, 
	{ "TOTEM", ITEM_TOTEM }, 
	{ "BLEND", ITEM_BLEND }, 
	{ "COSTUME", ITEM_COSTUME }, 
	{ "DS", ITEM_DS }, 
	{ "SPECIAL_DS", ITEM_SPECIAL_DS }, 
	{ "EXTRACT", ITEM_EXTRACT }, 
	{ "SECONDARY_COIN", ITEM_SECONDARY_COIN }, 
	{ "RING", ITEM_RING }, 
	{ "BELT", ITEM_BELT }, 
	{ "PET", ITEM_PET }, 
	{ "MEDIUM", ITEM_MEDIUM }, 
	{ "GACHA", ITEM_GACHA }, 
	{ "TOGGLE", ITEM_TOGGLE },
	{ "TALISMAN", ITEM_TALISMAN },
};

UInt32Table kWeaponSubTypes[] = {
	{"SWORD", WEAPON_SWORD},
	{"DAGGER", WEAPON_DAGGER},
	{"BOW", WEAPON_BOW},
	{"TWO_HANDED", WEAPON_TWO_HANDED},
	{"BELL", WEAPON_BELL},
	{"FAN", WEAPON_FAN},
	{"ARROW", WEAPON_ARROW},
	{"MOUNT_SPEAR", WEAPON_MOUNT_SPEAR},
	{"CLAW", WEAPON_CLAW},
	{"QUIVER", WEAPON_QUIVER},
};

UInt32Table kArmorSubTypes[] = {
	{"BODY", ARMOR_BODY},
	{"HEAD", ARMOR_HEAD},
	{"SHIELD", ARMOR_SHIELD},
	{"WRIST", ARMOR_WRIST},
	{"FOOTS", ARMOR_FOOTS},
	{"NECK", ARMOR_NECK},
	{"EAR", ARMOR_EAR},
};

UInt32Table kUseSubTypes[] = {
	{"POTION", USE_POTION},
	{"TALISMAN", USE_TALISMAN},
	{"TUNING", USE_TUNING},
	{"MOVE", USE_MOVE},
	{"TREASURE_BOX", USE_TREASURE_BOX},
	{"MONEYBAG", USE_MONEYBAG},
	{"BAIT", USE_BAIT},
	{"ABILITY_UP", USE_ABILITY_UP},
	{"AFFECT", USE_AFFECT},
	{"CREATE_STONE", USE_CREATE_STONE},
	{"SPECIAL", USE_SPECIAL},
	{"POTION_NODELAY", USE_POTION_NODELAY},
	{"CLEAR", USE_CLEAR},
	{"INVISIBILITY", USE_INVISIBILITY},
	{"DETACHMENT", USE_DETACHMENT},
	{"BUCKET", USE_BUCKET},
	{"POTION_CONTINUE", USE_POTION_CONTINUE},
	{"CLEAN_SOCKET", USE_CLEAN_SOCKET},
	{"CHANGE_ATTRIBUTE", USE_CHANGE_ATTRIBUTE},
	{"ADD_ATTRIBUTE", USE_ADD_ATTRIBUTE},
	{"ADD_ACCESSORY_SOCKET", USE_ADD_ACCESSORY_SOCKET},
	{"PUT_INTO_ACCESSORY_SOCKET", USE_PUT_INTO_ACCESSORY_SOCKET},
	{"ADD_ATTRIBUTE2", USE_ADD_ATTRIBUTE2},
	{"RECIPE", USE_RECIPE},
	{"CHANGE_ATTRIBUTE2", USE_CHANGE_ATTRIBUTE2},
	{"BIND", USE_BIND},
	{"UNBIND", USE_UNBIND},
	{"TIME_CHARGE_PER", USE_TIME_CHARGE_PER},
	{"TIME_CHARGE_FIX", USE_TIME_CHARGE_FIX},
	{"PUT_INTO_BELT_SOCKET", USE_PUT_INTO_BELT_SOCKET},
	{"PUT_INTO_RING_SOCKET", USE_PUT_INTO_RING_SOCKET},
	{ "CHANGE_COSTUME_ATTR", USE_CHANGE_COSTUME_ATTR, }, 
	{ "RESET_COSTUME_ATTR", USE_RESET_COSTUME_ATTR, }, 
	{ "ADD_ATTRIBUTE_RARE", USE_ADD_ATTRIBUTE_RARE, }, 
	{ "CHANGE_ATTRIBUTE_RARE", USE_CHANGE_ATTRIBUTE_RARE, }, 
	{ "ENHANCE_TIME", USE_ENHANCE_TIME, }, 
	{ "CHANGE_ATTRIBUTE_PERM", USE_CHANGE_ATTRIBUTE_PERM, }, 
	{ "ADD_ATTRIBUTE_PERM", USE_ADD_ATTRIBUTE_PERM, }, 
	{ "MAKE_ACCESSORY_SOCKET_PERM", USE_MAKE_ACCESSORY_SOCKET_PERM, }, 
	{ "SILK_BOTARY", USE_SILK_BOTARY, }, 
	{ "OFFLINE_SHOP_FARMED_TIME", USE_OFFLINE_SHOP_FARMED_TIME, }, 
	{ "OFFLINE_SHOP_SPECIAL_TIME", USE_OFFLINE_SHOP_SPECIAL_TIME, },
	{ "SET_TITLE", USE_SET_TITLE, },
	{ "PET_FOOD", USE_LEVEL_PET_FOOD, },
	{ "LEVEL_PET_CHANGE_ATTR", USE_LEVEL_PET_CHANGE_ATTR, },
	{ "BATTLEPASS", USE_BATTLEPASS, },
	{ "ADD_SOCKETS", USE_ADD_SOCKETS, },
};

UInt32Table kMaterialSubTypes[] = {
	{"LEATHER", MATERIAL_LEATHER},
	{"BLOOD", MATERIAL_BLOOD},
	{"ROOT", MATERIAL_ROOT},
	{"NEEDLE", MATERIAL_NEEDLE},
	{"JEWEL", MATERIAL_JEWEL},
	{"DS_REFINE_NORMAL", MATERIAL_DS_REFINE_NORMAL},
	{"DS_REFINE_BLESSED", MATERIAL_DS_REFINE_BLESSED},
	{"DS_REFINE_HOLLY", MATERIAL_DS_REFINE_HOLLY},
};

UInt32Table kSpecialSubTypes[] = {
	{"NONE", SPECIAL_NONE},
	{"HORSE_FOOD", SPECIAL_HORSE_FOOD},
	{"HORSE_REVIVAL", SPECIAL_HORSE_REVIVAL},
};

UInt32Table kMetinSubTypes[] = {
	{"NORMAL", METIN_NORMAL},
	{"GOLD", METIN_GOLD},
};

UInt32Table kFishSubTypes[] = {
	{"ALIVE", FISH_ALIVE},
	{"DEAD", FISH_DEAD},
};

UInt32Table kResourceSubTypes[] = {
	{"FISHBONE", RESOURCE_FISHBONE},
	{"WATERSTONEPIECE", RESOURCE_WATERSTONEPIECE},
	{"WATERSTONE", RESOURCE_WATERSTONE},
	{"BLOOD_PEARL", RESOURCE_BLOOD_PEARL},
	{"BLUE_PEARL", RESOURCE_BLUE_PEARL},
	{"WHITE_PEARL", RESOURCE_WHITE_PEARL},
	{"BUCKET", RESOURCE_BUCKET},
	{"CRYSTAL", RESOURCE_CRYSTAL},
	{"GEM", RESOURCE_GEM},
	{"STONE", RESOURCE_STONE},
	{"METIN", RESOURCE_METIN},
	{"ORE", RESOURCE_ORE},
};

UInt32Table kUniqueSubTypes[] = {
	{"NONE", UNIQUE_NONE},
	{"BOOK", UNIQUE_BOOK},
};

UInt32Table kPolymorphSubTypes[] = {
	{"BALL", POLYMORPH_BALL},
	{"BOOK", POLYMORPH_BOOK},
};

UInt32Table kCostumeSubTypes[] = {
	{"BODY", COSTUME_BODY},
	{"HAIR", COSTUME_HAIR},
	{"MOUNT", COSTUME_MOUNT, }, 
	{"ACCE", COSTUME_ACCE }, 
	{"WEAPON", COSTUME_WEAPON }, 
	{"BODY_EFFECT", COSTUME_BODY_EFFECT }, 
	{"WEAPON_EFFECT", COSTUME_WEAPON_EFFECT }, 
	{"WING_EFFECT", COSTUME_WING_EFFECT },
	{"RANK", COSTUME_RANK },
};

UInt32Table kToggleSubTypes[] = {
	{"AUTO_RECOVERY_HP", TOGGLE_AUTO_RECOVERY_HP},
	{"AUTO_RECOVERY_SP", TOGGLE_AUTO_RECOVERY_SP},
	{"MOUNT", TOGGLE_MOUNT},
	{"PET", TOGGLE_PET},
	{"ANTI_EXP", TOGGLE_ANTI_EXP},
	{"AFFECT", TOGGLE_AFFECT},
	{"LEVEL_PET", TOGGLE_LEVEL_PET},

};

UInt32Table kDsSubTypes[] = {
	{ "SLOT1", DS_SLOT1 }, 
	{ "SLOT2", DS_SLOT2 }, 
	{ "SLOT3", DS_SLOT3 }, 
	{ "SLOT4", DS_SLOT4 }, 
	{ "SLOT5", DS_SLOT5 }, 
	{ "SLOT6", DS_SLOT6 }, 
};

UInt32Table kExtractSubTypes[] = {
	{ "DRAGON_SOUL", EXTRACT_DRAGON_SOUL}, 
	{ "DRAGON_HEART", EXTRACT_DRAGON_HEART }, 
};

UInt32Table kItemLimitTypes[] = {
	{"NONE", LIMIT_NONE},
	{"LEVEL", LIMIT_LEVEL},
	{"STR", LIMIT_STR},
	{"DEX", LIMIT_DEX},
	{"INT", LIMIT_INT},
	{"CON", LIMIT_CON},
	{"REAL_TIME", LIMIT_REAL_TIME},
	{"REAL_TIME_START_FIRST_USE", LIMIT_REAL_TIME_START_FIRST_USE},
	{"TIMER_BASED_ON_WEAR", LIMIT_TIMER_BASED_ON_WEAR},
	{"MAX_LEVEL", LIMIT_MAX_LEVEL},
	{"MAP", LIMIT_MAP},

};

UInt32Table kItemAddonTypes[] = {
	{"NONE", ATTR_ADDON_NONE},
	{"DAMAGE", static_cast<uint32_t>(ATTR_DAMAGE_ADDON)},
};

UInt32Table kItemFlags[] = {
	{"REFINEABLE", ITEM_FLAG_REFINEABLE},
	{"SAVE", ITEM_FLAG_SAVE},
	{"STACKABLE", ITEM_FLAG_STACKABLE},
	{"COUNT_PER_1GOLD", ITEM_FLAG_COUNT_PER_1GOLD},
	{"SLOW_QUERY", ITEM_FLAG_SLOW_QUERY},
	{"UNUSED01", ITEM_FLAG_UNUSED01},
	{"UNIQUE", ITEM_FLAG_UNIQUE},
	{"MAKECOUNT", ITEM_FLAG_MAKECOUNT},
	{"IRREMOVABLE", ITEM_FLAG_IRREMOVABLE},
	{"CONFIRM_WHEN_USE", ITEM_FLAG_CONFIRM_WHEN_USE},
	{"QUEST_USE", ITEM_FLAG_QUEST_USE},
	{"QUEST_USE_MULTIPLE", ITEM_FLAG_QUEST_USE_MULTIPLE},
	{"QUEST_GIVE", ITEM_FLAG_QUEST_GIVE},
	{"LOG", ITEM_FLAG_LOG},
	{"APPLICABLE", ITEM_FLAG_APPLICABLE},
};

UInt32Table kItemAntiFlags[] = {
	{"FEMALE", ITEM_ANTIFLAG_FEMALE},
	{"MALE", ITEM_ANTIFLAG_MALE},
	{"WARRIOR", ITEM_ANTIFLAG_WARRIOR},
	{"ASSASSIN", ITEM_ANTIFLAG_ASSASSIN},
	{"SURA", ITEM_ANTIFLAG_SURA},
	{"SHAMAN", ITEM_ANTIFLAG_SHAMAN},
	{"GET", ITEM_ANTIFLAG_GET},
	{"DROP", ITEM_ANTIFLAG_DROP},
	{"SELL", ITEM_ANTIFLAG_SELL},
	{"EMPIRE_A", ITEM_ANTIFLAG_EMPIRE_A},
	{"EMPIRE_B", ITEM_ANTIFLAG_EMPIRE_B},
	{"EMPIRE_C", ITEM_ANTIFLAG_EMPIRE_C},
	{"SAVE", ITEM_ANTIFLAG_SAVE},
	{"GIVE", ITEM_ANTIFLAG_GIVE},
	{"PKDROP", ITEM_ANTIFLAG_PKDROP},
	{"STACK", ITEM_ANTIFLAG_STACK},
	{"MYSHOP", ITEM_ANTIFLAG_MYSHOP},
	{"SAFEBOX", ITEM_ANTIFLAG_SAFEBOX},
	{"WOLFMAN", ITEM_ANTIFLAG_WOLFMAN},
	{"CHANGE_ATTRIBUTE", ITEM_ANTIFLAG_CHANGE_ATTRIBUTE},
};

UInt32Table kAttributeSets[] = {
	{"WEAPON", ATTRIBUTE_SET_WEAPON},
	{"BODY", ATTRIBUTE_SET_BODY},
	{"WRIST", ATTRIBUTE_SET_WRIST},
	{"FOOTS", ATTRIBUTE_SET_FOOTS},
	{"NECK", ATTRIBUTE_SET_NECK},
	{"HEAD", ATTRIBUTE_SET_HEAD},
	{"SHIELD", ATTRIBUTE_SET_SHIELD},
	{"EAR", ATTRIBUTE_SET_EAR},
	{"COSTUME_HAIR", ATTRIBUTE_SET_COSTUME_HAIR},
	{"COSTUME_BODY", ATTRIBUTE_SET_COSTUME_BODY},
	{"COSTUME_WEAPON", ATTRIBUTE_SET_COSTUME_WEAPON},
	{"LEVEL_PET", ATTRIBUTE_SET_LEVEL_PET},
};

UInt32Table kRefineFlags[] = {
	{"REMOVE_TRANSUMATION", REFINE_FLAG_REMOVE_TRANSMUTATION},
};
UInt32Table kRefinerFlags[] = {
	{"ONLY_WEAPON", REFINER_FLAG_ONLY_WEAPON},
	{"ONLY_ARMOR", REFINER_FLAG_ONLY_ARMOR},
	{"ONLY_ACCESSORY", REFINER_FLAG_ONLY_ACCESSORY},
	{"FAILURE_DOWNGRADE", REFINER_FLAG_FAILURE_DOWNGRADE},
	{"FAILURE_IGNORE", REFINER_FLAG_FAILURE_IGNORE},
	{"NO_MATERIALS", REFINER_FLAG_NO_MATERIALS}
};

/**
	벨트는 총 +0 ~ +9 레벨을 가질 수 있으며, 레벨에 따라 7단계 등급으로 구분되어 인벤토리가 활성 화 된다.
	벨트 레벨에 따른 사용 가능한 셀은 아래 그림과 같음. 현재 등급 >= 활성가능 등급이면 사용 가능.
	(단, 현재 레벨이 0이면 무조건 사용 불가, 괄호 안의 숫자는 등급)

		2(1)  4(2)  6(4)  8(6)
		5(3)  5(3)  6(4)  8(6)
		7(5)  7(5)  7(5)  8(6)
		9(7)  9(7)  9(7)  9(7)

	벨트 인벤토리의 크기는 4x4 (16칸)
*/
static const uint8_t kBeltSlotMinGrade[BELT_INVENTORY_SLOT_COUNT] = {
	1, 2, 4, 6,
	3, 3, 4, 6,
	5, 5, 5, 6,
	7, 7, 7, 7
};

bool GetItemTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kItemTypes); }

bool GetItemTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kItemTypes); }

bool GetItemSubTypeString(storm::StringRef& s, uint32_t type, uint32_t val)
{
	switch (type) {
#define METIN2_HANDLE_TYPE(cap_name, cam_name) \
		case ITEM_ ## cap_name: return storm::FormatValueWithTable(s, val, k ## cam_name ## SubTypes); break

		METIN2_HANDLE_TYPE(WEAPON, Weapon);
		METIN2_HANDLE_TYPE(ARMOR, Armor);
		METIN2_HANDLE_TYPE(USE, Use);
		METIN2_HANDLE_TYPE(MATERIAL, Material);
		METIN2_HANDLE_TYPE(SPECIAL, Special);
		METIN2_HANDLE_TYPE(METIN, Metin);
		METIN2_HANDLE_TYPE(FISH, Fish);
		METIN2_HANDLE_TYPE(RESOURCE, Resource);
		METIN2_HANDLE_TYPE(UNIQUE, Unique);
		METIN2_HANDLE_TYPE(POLYMORPH, Polymorph);
		METIN2_HANDLE_TYPE(COSTUME, Costume);
		METIN2_HANDLE_TYPE(TOGGLE, Toggle);
		METIN2_HANDLE_TYPE(DS, Ds);
		METIN2_HANDLE_TYPE(EXTRACT, Extract);


#undef METIN2_HANDLE_TYPE

		default:
			if (val == 0) {
				s = "NONE";
				return true;
			}

			break;
	}

	return false;
}

bool GetItemSubTypeValue(const storm::StringRef& s, uint32_t type, uint32_t& val)
{
	switch (type) {
#define METIN2_HANDLE_TYPE(cap_name, cam_name) \
		case ITEM_ ## cap_name: return storm::ParseStringWithTable(s, val, k ## cam_name ## SubTypes); break

		METIN2_HANDLE_TYPE(WEAPON, Weapon);
		METIN2_HANDLE_TYPE(ARMOR, Armor);
		METIN2_HANDLE_TYPE(USE, Use);
		METIN2_HANDLE_TYPE(MATERIAL, Material);
		METIN2_HANDLE_TYPE(SPECIAL, Special);
		METIN2_HANDLE_TYPE(METIN, Metin);
		METIN2_HANDLE_TYPE(FISH, Fish);
		METIN2_HANDLE_TYPE(RESOURCE, Resource);
		METIN2_HANDLE_TYPE(UNIQUE, Unique);
		METIN2_HANDLE_TYPE(POLYMORPH, Polymorph);
		METIN2_HANDLE_TYPE(COSTUME, Costume);
		METIN2_HANDLE_TYPE(TOGGLE, Toggle);
		METIN2_HANDLE_TYPE(DS, Ds);
		METIN2_HANDLE_TYPE(EXTRACT, Extract);

#undef METIN2_HANDLE_TYPE

		default:
			if (s == "NONE") {
				val = 0;
				return true;
			}

			break;
	}

	return false;
}

bool GetItemLimitTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kItemLimitTypes); }

bool GetItemLimitTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kItemLimitTypes); }

bool GetItemAddonTypeString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kItemAddonTypes); }

bool GetItemAddonTypeValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kItemAddonTypes); }

bool GetItemFlagsString(storm::String& s, uint32_t val)
{ return storm::FormatBitflagWithTable(s, val, kItemFlags); }

bool GetItemFlagsValue(const storm::String& s, uint32_t& val)
{ return storm::ParseBitflagStringWithTable(s, val, kItemFlags); }

bool GetItemAntiFlagsString(storm::String& s, uint32_t val)
{ return storm::FormatBitflagWithTable(s, val, kItemAntiFlags); }

bool GetItemAntiFlagsValue(const storm::String& s, uint32_t& val)
{ return storm::ParseBitflagStringWithTable(s, val, kItemAntiFlags); }

bool GetItemAttributeSetString(storm::StringRef& s, uint32_t val)
{ return storm::FormatValueWithTable(s, val, kAttributeSets); }

bool GetItemAttributeSetValue(const storm::StringRef& s, uint32_t& val)
{ return storm::ParseStringWithTable(s, val, kAttributeSets); }


bool GetRefineFlagsString(storm::String& s, uint32_t val)
{ return storm::FormatBitflagWithTable(s, val, kRefineFlags); }

bool GetRefineFlagsValue(const storm::String& s, uint32_t& val)
{ return storm::ParseBitflagStringWithTable(s, val, kRefineFlags); }

bool GetRefinerFlagsString(storm::String& s, uint32_t val)
{ return storm::FormatBitflagWithTable(s, val, kRefinerFlags); }

bool GetRefinerFlagsValue(const storm::String& s, uint32_t& val)
{ return storm::ParseBitflagStringWithTable(s, val, kRefinerFlags); }


uint8_t GetBeltGradeForSlot(uint8_t slot)
{
	STORM_ASSERT(slot < BELT_INVENTORY_SLOT_COUNT, "range error");
	return kBeltSlotMinGrade[slot];
}

bool CanMoveIntoBeltInventory(uint32_t type, uint32_t subType)
{
	switch (type) {
		case ITEM_USE: {
			switch (subType) {
				case USE_POTION:
				case USE_POTION_NODELAY:
				case USE_ABILITY_UP:
					return true;
			}
		}
	}

	return false;
}

METIN2_END_NS
