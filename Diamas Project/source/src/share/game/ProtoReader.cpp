// YMIR's ProtoReader.cpp (mainly unmodified)

#include "ProtoReader.hpp"
#include "CsvReader.hpp"

#include <game/ItemConstants.hpp>
#include <game/MobConstants.hpp>
#include <SpdLog.hpp>
#include <storm/StringUtil.hpp>

#include <cmath>
#include <sstream>
#include <cstdlib>

#include "MobTypes.hpp"

METIN2_BEGIN_NS

using namespace std;

inline bool str_to_number(bool& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (strtol(in, NULL, 10) != 0);
	return true;
}

inline bool str_to_number(char& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (char)strtol(in, NULL, 10);
	return true;
}

inline bool str_to_number(unsigned char& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (unsigned char)strtoul(in, NULL, 10);
	return true;
}

inline bool str_to_number(short& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (short)strtol(in, NULL, 10);
	return true;
}

inline bool str_to_number(unsigned short& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (unsigned short)strtoul(in, NULL, 10);
	return true;
}

inline bool str_to_number(int& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (int)strtol(in, NULL, 10);
	return true;
}

inline bool str_to_number(unsigned int& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (unsigned int)strtoul(in, NULL, 10);
	return true;
}

inline bool str_to_number(long& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (long)strtol(in, NULL, 10);
	return true;
}

inline bool str_to_number(long long& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (long)strtoll(in, NULL, 10);
	return true;
}

inline bool str_to_number(unsigned long& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (unsigned long)strtoul(in, NULL, 10);
	return true;
}

inline bool str_to_number(float& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (float)strtod(in, NULL);
	return true;
}

inline bool str_to_number(double& out, const char* in)
{
	if (0 == in || 0 == in[0])	return false;

	out = (double)strtod(in, NULL);
	return true;
}

inline string trim_left(const string& str)
{
	string::size_type n = str.find_first_not_of(" \t\v\n\r");
	return n == string::npos ? str : str.substr(n, str.length());
}

inline string trim_right(const string& str)
{
	string::size_type n = str.find_last_not_of(" \t\v\n\r");
	return n == string::npos ? str : str.substr(0, n + 1);
}

string trim(const string& str) { return trim_left(trim_right(str)); }

static string* StringSplit(string strOrigin, const string& strTok)
{
	int     cutAt;                            //자르는위치
	int     index = 0;                    //문자열인덱스
	string* strResult = new string[30];		  //결과return 할변수

	//strTok을찾을때까지반복
	while ((cutAt = strOrigin.find_first_of(strTok)) != strOrigin.npos)
	{
		if (cutAt > 0)  //자르는위치가0보다크면(성공시)
		{
			strResult[index++] = strOrigin.substr(0, cutAt);  //결과배열에추가
		}
		strOrigin = strOrigin.substr(cutAt + 1);  //원본은자른부분제외한나머지
	}

	if (strOrigin.length() > 0)  //원본이아직남았으면
	{
		strResult[index++] = strOrigin.substr(0, cutAt);  //나머지를결과배열에추가
	}

	for (int i = 0; i < index; i++)
	{
		strResult[i] = trim(strResult[i]);
	}

	return strResult;  //결과return
}


int get_Item_Type_Value(const string& inputString)
{
	string arType[] = { "ITEM_NONE", "ITEM_WEAPON",
		"ITEM_ARMOR", "ITEM_USE",
		"ITEM_AUTOUSE", "ITEM_MATERIAL",
		"ITEM_SPECIAL", "ITEM_TOOL",
		"ITEM_LOTTERY", "ITEM_ELK",					//10?

		"ITEM_METIN", "ITEM_CONTAINER",
		"ITEM_FISH", "ITEM_ROD",
		"ITEM_RESOURCE", "ITEM_CAMPFIRE",
		"ITEM_UNIQUE", "ITEM_SKILLBOOK",
		"ITEM_QUEST", "ITEM_POLYMORPH",				//20?

		"ITEM_TREASURE_BOX", "ITEM_TREASURE_KEY",
		"ITEM_SKILLFORGET", "ITEM_GIFTBOX",
		"ITEM_PICK", "ITEM_HAIR",
		"ITEM_TOTEM", "ITEM_BLEND",
		"ITEM_COSTUME", "ITEM_DS",					//30?

		"ITEM_SPECIAL_DS", "ITEM_EXTRACT",
		"ITEM_SECONDARY_COIN",						//33?

		"ITEM_RING",
		"ITEM_BELT",								//35?(EItemTypes 값으로 치면 34)
		"ITEM_PET",
		"ITEM_MEDIUM",
		"ITEM_GACHA",
		"ITEM_TOGGLE",
		"ITEM_TALISMAN",
	};


	int retInt = -1;
	//cout << "Type : " << typeStr << " -> ";
	for (int j = 0; j < static_cast<int>(sizeof(arType) / sizeof(arType[0])); j++) {
		string tempString = arType[j];
		if (inputString.find(tempString) != string::npos && tempString.find(inputString) != string::npos) {
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;

}

int get_Item_SubType_Value(int type_value, const string& inputString)
{

	static std::unordered_map<int, std::vector<std::string>> subtypes;

	subtypes[ITEM_WEAPON] = { "WEAPON_SWORD", "WEAPON_DAGGER", "WEAPON_BOW", "WEAPON_TWO_HANDED",
								"WEAPON_BELL", "WEAPON_FAN", "WEAPON_ARROW", "WEAPON_MOUNT_SPEAR",
								"WEAPON_CLAW","WEAPON_QUIVER", "WEAPON_BOUQUET" };

	subtypes[ITEM_ARMOR] = { "ARMOR_BODY", "ARMOR_HEAD", "ARMOR_SHIELD", "ARMOR_WRIST", "ARMOR_FOOTS",
								"ARMOR_NECK", "ARMOR_EAR", "ARMOR_NUM_TYPES" };

	subtypes[ITEM_USE] = { "USE_POTION", "USE_TALISMAN", "USE_TUNING", "USE_MOVE", "USE_TREASURE_BOX", "USE_MONEYBAG", "USE_BAIT",
								"USE_ABILITY_UP", "USE_AFFECT", "USE_CREATE_STONE", "USE_SPECIAL", "USE_POTION_NODELAY", "USE_CLEAR",
								"USE_INVISIBILITY", "USE_DETACHMENT", "USE_BUCKET", "USE_POTION_CONTINUE", "USE_CLEAN_SOCKET",
								"USE_CHANGE_ATTRIBUTE", "USE_ADD_ATTRIBUTE", "USE_ADD_ACCESSORY_SOCKET", "USE_PUT_INTO_ACCESSORY_SOCKET",
								"USE_ADD_ATTRIBUTE2", "USE_RECIPE", "USE_CHANGE_ATTRIBUTE2", "USE_BIND", "USE_UNBIND", "USE_TIME_CHARGE_PER",
								"USE_TIME_CHARGE_FIX", "USE_PUT_INTO_BELT_SOCKET", "USE_PUT_INTO_RING_SOCKET", "USE_CHEST", "USE_CHANGE_COSTUME_ATTR", "USE_RESET_COSTUME_ATTR",
								"USE_ADD_ATTRIBUTE_RARE", "USE_CHANGE_ATTRIBUTE_RARE", "USE_ENHANCE_TIME", "USE_CHANGE_ATTRIBUTE_PERM", "USE_ADD_ATTRIBUTE_PERM", "USE_MAKE_ACCESSORY_SOCKET_PERM",
								"USE_SILK_BOTARY", "USE_OFFLINE_SHOP_FARMED_TIME", "USE_OFFLINE_SHOP_SPECIAL_TIME", "USE_SET_TITLE",
								"USE_LEVEL_PET_FOOD", "USE_LEVEL_PET_ATTR_CHANGE", "USE_BATTLEPASS", "USE_ADD_SOCKETS"
	};

	subtypes[ITEM_AUTOUSE] = { "AUTOUSE_POTION", "AUTOUSE_ABILITY_UP", "AUTOUSE_BOMB", "AUTOUSE_GOLD", "AUTOUSE_MONEYBAG", "AUTOUSE_TREASURE_BOX" };

	subtypes[ITEM_MATERIAL] = { "MATERIAL_LEATHER", "MATERIAL_BLOOD", "MATERIAL_ROOT", "MATERIAL_NEEDLE", "MATERIAL_JEWEL",
								"MATERIAL_DS_REFINE_NORMAL", "MATERIAL_DS_REFINE_BLESSED", "MATERIAL_DS_REFINE_HOLLY" };

	subtypes[ITEM_SPECIAL] = { "SPECIAL_MAP", "SPECIAL_KEY", "SPECIAL_DOC", "SPECIAL_SPIRIT", "SPECIAL_ORE" };
	subtypes[ITEM_TOOL] = { "TOOL_FISHING_ROD" };
	subtypes[ITEM_LOTTERY] = { "LOTTERY_TICKET", "LOTTERY_INSTANT" };
	subtypes[ITEM_METIN] = { "METIN_NORMAL", "METIN_GOLD" };
	subtypes[ITEM_FISH] = { "FISH_ALIVE", "FISH_DEAD" };

	subtypes[ITEM_RESOURCE] = { "RESOURCE_FISHBONE", "RESOURCE_WATERSTONEPIECE", "RESOURCE_WATERSTONE", "RESOURCE_BLOOD_PEARL",
								"RESOURCE_BLUE_PEARL", "RESOURCE_WHITE_PEARL", "RESOURCE_BUCKET", "RESOURCE_CRYSTAL", "RESOURCE_GEM",
								"RESOURCE_STONE", "RESOURCE_METIN", "RESOURCE_ORE" };

	subtypes[ITEM_UNIQUE] = { "UNIQUE_NONE", "UNIQUE_BOOK", "UNIQUE_SPECIAL_RIDE", "UNIQUE_3", "UNIQUE_4", "UNIQUE_5",
								"UNIQUE_6", "UNIQUE_7", "UNIQUE_8", "UNIQUE_9", "USE_SPECIAL" };

	subtypes[ITEM_QUEST] = { "QUEST_SKILLBOOK" };
	subtypes[ITEM_PICK] = { "BASIC", "RELIC" };
	subtypes[ITEM_COSTUME] = { "COSTUME_BODY", "COSTUME_HAIR", "COSTUME_MOUNT", "COSTUME_ACCE", "COSTUME_WEAPON", "COSTUME_BODY_EFFECT", "COSTUME_WEAPON_EFFECT", "COSTUME_WING_EFFECT", "COSTUME_RANK" };
	subtypes[ITEM_DS] = { "DS_SLOT1", "DS_SLOT2", "DS_SLOT3", "DS_SLOT4", "DS_SLOT5", "DS_SLOT6" };
	subtypes[ITEM_EXTRACT] = { "EXTRACT_DRAGON_SOUL", "EXTRACT_DRAGON_HEART" };
	subtypes[ITEM_PET] = { "PET_EGG", "PET_UPBRINGING", "PET_BAG", "PET_FEEDSTUFF", "PET_SKILL", "PET_SKILL_DEL_BOOK" };
	subtypes[ITEM_MEDIUM] = { "MEDIUM_MOVE_COSTUME_ATTR", };
	subtypes[ITEM_TOGGLE] = { "TOGGLE_AUTO_RECOVERY_HP", "TOGGLE_AUTO_RECOVERY_SP", "TOGGLE_PET", "TOGGLE_MOUNT", "TOGGLE_ANTI_EXP", "TOGGLE_AFFECT", "TOGGLE_LEVEL_PET" };
	subtypes[ITEM_TALISMAN] = { "TALISMAN_1", "TALISMAN_2", "TALISMAN_3", "TALISMAN_4", "TALISMAN_5", "TALISMAN_6", "TALISMAN_7"  };
	subtypes[ITEM_RING] = { "RING_1", "RING_2", "RING_3", "RING_4", "RING_5", "RING_6", "RING_7"  };

	if (type_value < 0 || type_value >= ITEM_TYPE_MAX) {
		spdlog::error("Out of range type! (type_value: {}, max valid type: {})", type_value, ITEM_TYPE_MAX);
		return -1;
	}

	// Don't process if there are no subtypes for this type 
	if (subtypes.count(type_value) == 0) {
		return 0;
	}

	std::string trimmedInput = trim(inputString);

	// Allow no subtype for ITEM_QUEST
	if (type_value == ITEM_QUEST && (trimmedInput == "NONE" || trimmedInput == "0"))
		return 0;

	for (size_t i = 0, size = subtypes[type_value].size(); i < size; ++i)
	{
		//Found the subtype that's specified
		if (trimmedInput == subtypes[type_value][i])
			return i;
	}

	spdlog::error("Subtype {} is not valid for type {}", trimmedInput.c_str(), type_value);
	return -1;
}


int get_Item_AntiFlag_Value(const string& inputString)
{

	string arAntiFlag[] = { "ANTI_FEMALE", "ANTI_MALE", "ANTI_MUSA", "ANTI_ASSASSIN", "ANTI_SURA", "ANTI_MUDANG",
		"ANTI_GET", "ANTI_DROP", "ANTI_SELL", "ANTI_EMPIRE_A", "ANTI_EMPIRE_B", "ANTI_EMPIRE_C",
		"ANTI_SAVE", "ANTI_GIVE", "ANTI_PKDROP", "ANTI_STACK", "ANTI_MYSHOP", "ANTI_SAFEBOX", "ANTI_WOLFMAN",
		"ANTI_CHANGE_ATTR",	 "ANTI_DESTROY", "ANTI_QUICKSLOT", "ANTI_CHANGELOOK" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");	//프로토 정보 내용을 단어별로 쪼갠 배열.

	int comp = sizeof(arAntiFlag) / sizeof(arAntiFlag[0]);
	for (int i = 0; i < comp; ++i)
	{
		string tempString = arAntiFlag[i];
		for (int j = 0; j < 30; ++j)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];

			if (tempString2.empty())
				break;

			if (tempString2 == tempString) {
				retValue = retValue + (int)pow(2, i);
			}
		}
	}
	delete[]arInputString;
	//cout << "AntiFlag : " << antiFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_Flag_Value(const string& inputString)
{
	//Think - 24/03/12 - Cleaned this list to actually fit the flags it should
	string arFlag[] = {
		"ITEM_TUNABLE", //ITEM_FLAG_REFINEABLE - 0
		"ITEM_SAVE", //ITEM_FLAG_SAVE - 1 
		"ITEM_STACKABLE", //ITEM_FLAG_STACKABLE - 2
		"COUNT_PER_1GOLD", //ITEM_FLAG_COUNT_PER_1GOLD - 3
		"ITEM_SLOW_QUERY", //ITEM_FLAG_SLOW_QUERY - 4
		"", //ITEM_FLAG_UNUSED01 - 5
		"ITEM_UNIQUE", //ITEM_FLAG_UNIQUE - 6 - Unused
		"ITEM_MAKECOUNT", //ITEM_FLAG_MAKECOUNT - 7
		"ITEM_IRREMOVABLE", //ITEM_FLAG_IRREMOVABLE - 8
		"CONFIRM_WHEN_USE", //ITEM_FLAG_CONFIRM_WHEN_USE - 9
		"QUEST_USE",  //ITEM_FLAG_QUEST_USE - 10
		"QUEST_USE_MULTIPLE", //ITEM_FLAG_QUEST_USE_MULTIPLE - 11,
		"QUEST_GIVE", //ITEM_FLAG_QUEST_GIVE - 12,
		"LOG", //ITEM_FLAG_LOG - 13,
		"ITEM_APPLICABLE", //ITEM_FLAG_APPLICABLE - 14,
		"MONEY_ITEM", //ITEM_FLAG_MONEY_ITEM - 15
		"TIER_3",
		"TIER_4",

		//Unused and wrong position
		//"ITEM_QUEST", (was before LOG and after QUEST_GIVE, screwing up everything)
		//"IRREMOVABLE",
		//"SLOW_QUERY",
		//"REFINEABLE",
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");	//프로토 정보 내용을 단어별로 쪼갠 배열.
	int sizeOf = sizeof(arFlag) / sizeof(arFlag[0]);
	for (int i = 0; i < sizeOf; i++) {
		string tempString = arFlag[i];
		for (int j = 0; j < 30; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2.empty())
				break;

			if (tempString2 == tempString) {				//일치하는지 확인.
				retValue = retValue + (int)pow(2, i);
			}
		}
	}
	delete[]arInputString;

	//cout << "Flag : " << flagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_WearFlag_Value(const string& inputString)
{

	string arWearrFlag[] = { "WEAR_BODY", "WEAR_HEAD", "WEAR_FOOTS", "WEAR_WRIST", "WEAR_WEAPON", "WEAR_NECK", "WEAR_EAR", "WEAR_SHIELD", "WEAR_UNIQUE",
		"WEAR_ARROW", "WEAR_HAIR", "WEAR_ABILITY" };


	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");//프로토 정보 내용을 단어별로 쪼갠 배열.
	int sizeOf = sizeof(arWearrFlag) / sizeof(arWearrFlag[0]);
	for (int i = 0; i < sizeOf; i++) {
		string tempString = arWearrFlag[i];
		for (int j = 0; j < 30; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2 == tempString) {				//일치하는지 확인.
				retValue = retValue + (int)pow(2, i);
			}

			if (tempString2.empty())
				break;
		}
	}
	delete[]arInputString;
	//cout << "WearFlag : " << wearFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_Immune_Value(const string& inputString)
{

	string arImmune[] = { "PARA", "CURSE", "STUN", "SLEEP", "SLOW", "POISON", "TERROR" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	int sizeOf = sizeof(arImmune) / sizeof(arImmune[0]);
	for (int i = 0; i < sizeOf; i++) {
		string tempString = arImmune[i];
		for (int j = 0; j < 30; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2 == tempString) {				//일치하는지 확인.
				retValue = retValue + (int)pow(2, i);
			}

			if (tempString2.empty())
				break;
		}
	}
	delete[]arInputString;
	//cout << "Immune : " << immuneStr << " -> " << retValue << endl;

	return retValue;
}




int get_Item_LimitType_Value(const string& inputString)
{
	string arLimitType[] = { "LIMIT_NONE", "LEVEL", "STR", "DEX", "INT", "CON", "REAL_TIME", "REAL_TIME_FIRST_USE", "TIMER_BASED_ON_WEAR", "MAX_LEVEL", "MAP" };

	int retInt = -1;
	//cout << "LimitType : " << limitTypeStr << " -> ";
	int sizeOf = sizeof(arLimitType) / sizeof(arLimitType[0]);
	for (int j = 0; j < sizeOf; j++) {
		string tempString = arLimitType[j];
		string tempInputString = trim(inputString);
		if (tempInputString == tempString)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}


int get_Item_ApplyType_Value(const string& inputString)
{
	string arApplyType[] = { "APPLY_NONE", "APPLY_MAX_HP", "APPLY_MAX_SP", "APPLY_CON", "APPLY_INT", "APPLY_STR", "APPLY_DEX", "APPLY_ATT_SPEED",
			"APPLY_MOV_SPEED", "APPLY_CAST_SPEED", "APPLY_HP_REGEN", "APPLY_SP_REGEN", "APPLY_POISON_PCT", "APPLY_STUN_PCT",
			"APPLY_SLOW_PCT", "APPLY_CRITICAL_PCT", "APPLY_PENETRATE_PCT", "APPLY_ATTBONUS_HUMAN", "APPLY_ATTBONUS_ANIMAL",
			"APPLY_ATTBONUS_ORC", "APPLY_ATTBONUS_MILGYO", "APPLY_ATTBONUS_UNDEAD", "APPLY_ATTBONUS_DEVIL", "APPLY_STEAL_HP",
			"APPLY_STEAL_SP", "APPLY_MANA_BURN_PCT", "APPLY_DAMAGE_SP_RECOVER", "APPLY_BLOCK", "APPLY_DODGE", "APPLY_RESIST_SWORD",
			"APPLY_RESIST_TWOHAND", "APPLY_RESIST_DAGGER", "APPLY_RESIST_BELL", "APPLY_RESIST_FAN", "APPLY_RESIST_BOW", "APPLY_RESIST_FIRE",
			"APPLY_RESIST_ELEC", "APPLY_RESIST_MAGIC", "APPLY_RESIST_WIND", "APPLY_REFLECT_MELEE", "APPLY_REFLECT_CURSE", "APPLY_POISON_REDUCE",
			"APPLY_KILL_SP_RECOVER", "APPLY_EXP_DOUBLE_BONUS", "APPLY_GOLD_DOUBLE_BONUS", "APPLY_ITEM_DROP_BONUS", "APPLY_POTION_BONUS",
			"APPLY_KILL_HP_RECOVER", "APPLY_IMMUNE_STUN", "APPLY_IMMUNE_SLOW", "APPLY_IMMUNE_FALL", "APPLY_SKILL", "APPLY_BOW_DISTANCE",
			"APPLY_ATT_GRADE_BONUS", "APPLY_DEF_GRADE_BONUS", "APPLY_MAGIC_ATT_GRADE", "APPLY_MAGIC_DEF_GRADE", "APPLY_CURSE_PCT",
			"APPLY_MAX_STAMINA", "APPLY_ATTBONUS_WARRIOR", "APPLY_ATTBONUS_ASSASSIN", "APPLY_ATTBONUS_SURA", "APPLY_ATTBONUS_SHAMAN",
			"APPLY_ATTBONUS_MONSTER", "APPLY_MALL_ATTBONUS", "APPLY_MALL_DEFBONUS", "APPLY_MALL_EXPBONUS", "APPLY_MALL_ITEMBONUS",
			"APPLY_MALL_GOLDBONUS", "APPLY_MAX_HP_PCT", "APPLY_MAX_SP_PCT", "APPLY_SKILL_DAMAGE_BONUS", "APPLY_NORMAL_HIT_DAMAGE_BONUS",
			"APPLY_SKILL_DEFEND_BONUS", "APPLY_NORMAL_HIT_DEFEND_BONUS", "APPLY_PC_BANG_EXP_BONUS", "APPLY_PC_BANG_DROP_BONUS",
			"APPLY_EXTRACT_HP_PCT", "APPLY_RESIST_WARRIOR", "APPLY_RESIST_ASSASSIN", "APPLY_RESIST_SURA", "APPLY_RESIST_SHAMAN",
			"APPLY_ENERGY",	"APPLY_DEF_GRADE", "APPLY_COSTUME_ATTR_BONUS", "APPLY_MAGIC_ATTBONUS_PER", "APPLY_MELEE_MAGIC_ATTBONUS_PER",
			"APPLY_RESIST_ICE", "APPLY_RESIST_EARTH", "APPLY_RESIST_DARK", "APPLY_ANTI_CRITICAL_PCT", "APPLY_ANTI_PENETRATE_PCT",
			"APPLY_BOOST_CRITICAL_DMG", "APPLY_BOOST_PENETRATE_DMG", "APPLY_ATTBONUS_INSECT","APPLY_BLEEDING_PCT", "APPLY_BLEEDING_REDUCE",
			"APPLY_ATTBONUS_WOLFMAN", "APPLY_RESIST_WOLFMAN", "APPLY_RESIST_CLAW",	"APPLY_ACCEDRAIN_RATE","APPLY_ATTBONUS_METIN","APPLY_ATTBONUS_TRENT",
			"APPLY_ATTBONUS_BOSS", "APPLY_RESIST_HUMAN","APPLY_ENCHANT_ELECT", "APPLY_ENCHANT_FIRE", "APPLY_ENCHANT_ICE", "APPLY_ENCHANT_WIND", "APPLY_ENCHANT_EARTH",
			"APPLY_ENCHANT_DARK","APPLY_ATTBONUS_DESERT",  "APPLY_ATTBONUS_CZ", "APPLY_ATTBONUS_LEGEND", "APPLY_ATTBONUS_SHADOW"
	};

	int retInt = -1;
	//cout << "ApplyType : " << applyTypeStr << " -> ";
	int sizeOf = sizeof(arApplyType) / sizeof(arApplyType[0]);
	for (int j = 0; j < sizeOf; j++) {
		string tempString = arApplyType[j];
		string tempInputString = trim(inputString);
		if (tempInputString == tempString)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;

}


//몬스터 프로토도 읽는다.


int get_Mob_Rank_Value(const string& inputString)
{
	string arRank[] = { "PAWN", "S_PAWN", "KNIGHT", "S_KNIGHT", "BOSS", "KING", "LEGEND" };

	int retInt = -1;
	//cout << "Rank : " << rankStr << " -> ";
	int sizeOf = sizeof(arRank) / sizeof(arRank[0]);
	for (int j = 0; j < sizeOf; j++) {
		string tempString = arRank[j];
		string tempInputString = trim(inputString);
		if (tempInputString == tempString)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}


int get_Mob_Type_Value(const string& inputString)
{
	string arType[] = { "MONSTER", "NPC", "STONE", "WARP", "DOOR", "BUILDING", "PC", "POLYMORPH_PC", "HORSE", "GOTO", "SHOP", "PET", "MOUNT", "GROWTH_PET", "BUFFBOT" };

	int retInt = -1;
	//cout << "Type : " << typeStr << " -> ";
	int sizeOf = sizeof(arType) / sizeof(arType[0]);
	for (int j = 0; j < sizeOf; j++) {
		string tempString = arType[j];
		string tempInputString = trim(inputString);
		if (tempInputString == tempString)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_BattleType_Value(const string& inputString)
{
	string arBattleType[] = { "MELEE", "RANGE", "MAGIC", "SPECIAL", "POWER", "TANKER", "SUPER_POWER", "SUPER_TANKER" };

	int retInt = -1;
	//cout << "Battle Type : " << battleTypeStr << " -> ";
	int sizeOf = sizeof(arBattleType) / sizeof(arBattleType[0]);
	for (int j = 0; j < sizeOf; j++) {
		string tempString = arBattleType[j];
		string tempInputString = trim(inputString);
		if (tempInputString == tempString)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_Size_Value(const string& inputString)
{
	string arSize[] = { "SMALL", "MEDIUM", "BIG" };

	int retInt = 0;
	//cout << "Size : " << sizeStr << " -> ";
	int sizeOf = sizeof(arSize) / sizeof(arSize[0]);
	for (int j = 0; j < sizeOf; j++) {
		string tempString = arSize[j];
		string tempInputString = trim(inputString);
		if (tempInputString == tempString)
		{
			//cout << j << " ";
			retInt = j + 1;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_AIFlag_Value(const string& inputString)
{
	string arAIFlag[] = { "AGGR","NOMOVE","COWARD","NOATTSHINSU","NOATTCHUNJO","NOATTJINNO","ATTMOB","BERSERK","STONESKIN","GODSPEED","DEATHBLOW","REVIVE","HEALER" ,"COUNT" ,"NORECOVERY" ,"REFLECT" ,"FALL" ,"VIT" ,"RATTSPEED" ,"RCASTSPEED" ,"RHP_REGEN" ,"TIMEVIT" ,"NOPUSH" };


	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	int sizeOf = sizeof(arAIFlag) / sizeof(arAIFlag[0]);
	for (int i = 0; i < sizeOf; i++) {
		string tempString = arAIFlag[i];
		for (int j = 0; j < 30; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2 == tempString) {				//일치하는지 확인.
				retValue = retValue + (int)pow(2, i);
			}

			if (tempString2.empty())
				break;
		}
	}
	delete[]arInputString;
	//cout << "AIFlag : " << aiFlagStr << " -> " << retValue << endl;

	return retValue;
}
int get_Mob_RaceFlag_Value(const string& inputString)
{
	string arRaceFlag[] = { "ANIMAL", "UNDEAD", "DEVIL", "HUMAN", "ORC", "MILGYO", "INSECT", "FIRE", "ICE", "DESERT", "TREE",
		"ATT_ELEC", "ATT_FIRE", "ATT_ICE", "ATT_WIND", "ATT_EARTH", "ATT_DARK", "CZ", "SHADOW" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");	//프로토 정보 내용을 단어별로 쪼갠 배열.
	int sizeOf = sizeof(arRaceFlag) / sizeof(arRaceFlag[0]);
	for (int i = 0; i < sizeOf; i++) {
		string tempString = arRaceFlag[i];
		for (int j = 0; j < 30; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2 == tempString) {				//일치하는지 확인.
				retValue = retValue + (int)pow(2, i);
			}

			if (tempString2.empty())
				break;
		}
	}
	delete[]arInputString;
	//cout << "Race Flag : " << raceFlagStr << " -> " << retValue << endl;

	return retValue;
}
int get_Mob_ImmuneFlag_Value(const string& inputString)
{
	string arImmuneFlag[] = { "STUN", "SLOW", "FALL", "CURSE", "POISON", "TERROR", "REFLECT" };

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ",");				//프로토 정보 내용을 단어별로 쪼갠 배열.
	int sizeOf = sizeof(arImmuneFlag) / sizeof(arImmuneFlag[0]);
	for (int i = 0; i < sizeOf; i++) {
		string tempString = arImmuneFlag[i];
		for (int j = 0; j < 30; j++)		//최대 30개 단어까지. (하드코딩)
		{
			string tempString2 = arInputString[j];
			if (tempString2 == tempString) {				//일치하는지 확인.
				retValue = retValue + (int)pow(2, i);
			}

			if (tempString2.empty())
				break;
		}
	}
	delete[]arInputString;
	//cout << "Immune Flag : " << immuneFlagStr << " -> " << retValue << endl;

	return retValue;
}

//몹 테이블을 셋팅해준다.
bool Set_Proto_Mob_Table(TMobTable* mobTable, cCsvTable& csvTable)
{
	int col = 0;

	storm::ParseNumber(csvTable.AsStringByIndex(col++), mobTable->dwVnum);
	mobTable->szName = csvTable.AsStringByIndex(col++);
	mobTable->szLocaleName = mobTable->szName;

	//RANK
	int32_t rankValue = get_Mob_Rank_Value(csvTable.AsStringByIndex(col++));
	mobTable->bRank = rankValue;
	//TYPE
	int32_t typeValue = get_Mob_Type_Value(csvTable.AsStringByIndex(col++));
	mobTable->bType = typeValue;
	//BATTLE_TYPE
	int32_t battleTypeValue = get_Mob_BattleType_Value(csvTable.AsStringByIndex(col++));
	mobTable->bBattleType = battleTypeValue;

	storm::ParseNumber(csvTable.AsStringByIndex(col++), mobTable->bLevel);
	//SIZE
	storm::ParseNumber(csvTable.AsStringByIndex(col++), mobTable->wScale);
																	   //AI_FLAG
	int32_t aiFlagValue = get_Mob_AIFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwAIFlag = aiFlagValue;
	//mount_capacity;
	col++;
	//RACE_FLAG
	int raceFlagValue = get_Mob_RaceFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwRaceFlag = raceFlagValue;
	//11. IMMUNE_FLAG
	int immuneFlagValue = get_Mob_ImmuneFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwImmuneFlag = immuneFlagValue;

	storm::ParseNumber(csvTable.AsStringByIndex(col++), mobTable->bEmpire);
	col++;
	mobTable->bOnClickType = atoi(csvTable.AsStringByIndex(col++));

	mobTable->bStr = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bDex = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bCon = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bInt = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwDamageRange[0] = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwDamageRange[1] = atoi(csvTable.AsStringByIndex(col++));
	if(!storm::ParseNumber(csvTable.AsStringByIndex(col++), mobTable->dwMaxHP)) {
	    SPDLOG_ERROR("Failed to farser monster hp for vnum {}", mobTable->dwVnum);
	}
	mobTable->bRegenCycle = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bRegenPercent = atoi(csvTable.AsStringByIndex(col++));

	mobTable->dwGoldMin = atoll(csvTable.AsStringByIndex(col++));	//gold min
	mobTable->dwGoldMax = atoll(csvTable.AsStringByIndex(col++));	//gold max
	mobTable->dwExp = atoll(csvTable.AsStringByIndex(col++));
	mobTable->wDef = atoi(csvTable.AsStringByIndex(col++));
	mobTable->sAttackSpeed = atoi(csvTable.AsStringByIndex(col++));
	mobTable->sMovingSpeed = atoi(csvTable.AsStringByIndex(col++));
	mobTable->bAggresiveHPPct = atoi(csvTable.AsStringByIndex(col++));
	mobTable->wAggressiveSight = atoi(csvTable.AsStringByIndex(col++));
	mobTable->wAttackRange = atoi(csvTable.AsStringByIndex(col++));

	mobTable->dwDropItemVnum = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwResurrectionVnum = atoi(csvTable.AsStringByIndex(col++));	//resurrectionVnum


	for (char& cEnchant : mobTable->cEnchants)
		cEnchant = atoi(csvTable.AsStringByIndex(col++));

	for (char& cResist : mobTable->cResists)
		cResist = atoi(csvTable.AsStringByIndex(col++));

	mobTable->fDamMultiply = stof(std::string(csvTable.AsStringByIndex(col++)));
	mobTable->dwSummonVnum = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwDrainSP = atoi(csvTable.AsStringByIndex(col++));
	mobTable->dwMonsterColor = atoi(csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwPolymorphItemVnum, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->Skills[0].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[0].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[1].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[1].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[2].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[2].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[3].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[3].dwVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[4].bLevel, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->Skills[4].dwVnum, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->bBerserkPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bStoneSkinPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bGodSpeedPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bDeathBlowPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRevivePoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->despawnSeconds, csvTable.AsStringByIndex(col++));

	return true;
}

enum ItemProtoCells {
	ITEM_PROTO_CELL_ID,
	ITEM_PROTO_CELL_ITEM_NAME,
	ITEM_PROTO_CELL_ITEM_TYPE,
	ITEM_PROTO_CELL_SUB_TYPE,
	ITEM_PROTO_CELL_SIZE,
	ITEM_PROTO_CELL_ANTI_FLAG,
	ITEM_PROTO_CELL_FLAG,
	ITEM_PROTO_CELL_ITEM_WEAR,
	ITEM_PROTO_CELL_IMMUNE,
	ITEM_PROTO_CELL_SELL_PRICE,
	ITEM_PROTO_CELL_BUY_PRICE,
	ITEM_PROTO_CELL_REFINE,
	ITEM_PROTO_CELL_REFINESET,
	ITEM_PROTO_CELL_MAGIC_PCT,
	ITEM_PROTO_CELL_LIMIT_TYPE0,
	ITEM_PROTO_CELL_LIMIT_VALUE0,
	ITEM_PROTO_CELL_LIMIT_TYPE1,
	ITEM_PROTO_CELL_LIMIT_VALUE1,
	ITEM_PROTO_CELL_ADDON_TYPE0,
	ITEM_PROTO_CELL_ADDON_VALUE0,
	ITEM_PROTO_CELL_ADDON_TYPE1,
	ITEM_PROTO_CELL_ADDON_VALUE1,
	ITEM_PROTO_CELL_ADDON_TYPE2,
	ITEM_PROTO_CELL_ADDON_VALUE2,
	ITEM_PROTO_CELL_APPLY_TYPE3,
	ITEM_PROTO_CELL_APPLY_VALUE3,
	ITEM_PROTO_CELL_APPLY_TYPE4,
	ITEM_PROTO_CELL_APPLY_VALUE4,
	ITEM_PROTO_CELL_APPLY_TYPE5,
	ITEM_PROTO_CELL_APPLY_VALUE5,
	ITEM_PROTO_CELL_VALUE0,
	ITEM_PROTO_CELL_VALUE1,
	ITEM_PROTO_CELL_VALUE2,
	ITEM_PROTO_CELL_VALUE3,
	ITEM_PROTO_CELL_VALUE4,
	ITEM_PROTO_CELL_VALUE5,
	ITEM_PROTO_CELL_SPECULAR,
	ITEM_PROTO_CELL_SOCKET,
	ITEM_PROTO_CELL_ATTU_ADDON,
	ITEM_PROTO_CELL_MAX_NUM,
};


bool Set_Proto_Item_Table(TItemTable* itemTable, cCsvTable& csvTable)
{
	int col = 0;

	TItemApply aApplies[ITEM_APPLY_MAX_NUM];

	long long dataArray[39];
	int sizeOf = sizeof(dataArray) / sizeof(dataArray[0]);
	for (int i = 0; i < sizeOf; i++) {
		int64_t validCheck = 0;
		if (i == 1) {
			dataArray[i] = 0;
			validCheck = 0;
		}
		else if (i == ITEM_PROTO_CELL_ITEM_TYPE) {
			dataArray[i] = get_Item_Type_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_SUB_TYPE) {
			dataArray[i] = get_Item_SubType_Value(dataArray[i - 1], csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_ANTI_FLAG) {
			dataArray[i] = get_Item_AntiFlag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_FLAG) {
			dataArray[i] = get_Item_Flag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_ITEM_WEAR) {
			dataArray[i] = get_Item_WearFlag_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_IMMUNE) {
			dataArray[i] = get_Item_Immune_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_LIMIT_TYPE0) {
			dataArray[i] = get_Item_LimitType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_LIMIT_TYPE1) {
			dataArray[i] = get_Item_LimitType_Value(csvTable.AsStringByIndex(col));
			validCheck = dataArray[i];
		}
		else if (i == ITEM_PROTO_CELL_ADDON_TYPE0) {
			itemTable->aApplies[0].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = itemTable->aApplies[0].bType;
		}
		else if (i == ITEM_PROTO_CELL_ADDON_TYPE1) {
			itemTable->aApplies[1].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = itemTable->aApplies[1].bType;
		}
		else if (i == ITEM_PROTO_CELL_ADDON_TYPE2) {
			itemTable->aApplies[2].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = itemTable->aApplies[2].bType;
		}
		else if (i == ITEM_PROTO_CELL_APPLY_TYPE3) {
			itemTable->aApplies[3].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = itemTable->aApplies[3].bType;
		}
		else if (i == ITEM_PROTO_CELL_APPLY_TYPE4) {
			itemTable->aApplies[4].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = itemTable->aApplies[4].bType;
		}
		else if (i == ITEM_PROTO_CELL_APPLY_TYPE5) {
			itemTable->aApplies[5].bType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col));
			validCheck = itemTable->aApplies[5].bType;
		}
		else if (i == ITEM_PROTO_CELL_ADDON_VALUE0) {
			storm::ParseNumber(csvTable.AsStringByIndex(col), itemTable->aApplies[0].lValue);
			validCheck = 0;
		}
		else if (i == ITEM_PROTO_CELL_ADDON_VALUE1) {
			storm::ParseNumber(csvTable.AsStringByIndex(col), itemTable->aApplies[1].lValue);
			validCheck = 0;
		}
		else if (i == ITEM_PROTO_CELL_ADDON_VALUE2) {
			storm::ParseNumber(csvTable.AsStringByIndex(col), itemTable->aApplies[2].lValue);
			validCheck = 0;
		}
		else if (i == ITEM_PROTO_CELL_APPLY_VALUE3) {
			storm::ParseNumber(csvTable.AsStringByIndex(col), itemTable->aApplies[3].lValue);
			validCheck = 0;
		}
		else if (i == ITEM_PROTO_CELL_APPLY_VALUE4) {
			storm::ParseNumber(csvTable.AsStringByIndex(col), itemTable->aApplies[4].lValue);
			validCheck = 0;
		}
		else if (i == ITEM_PROTO_CELL_APPLY_VALUE5) {
			storm::ParseNumber(csvTable.AsStringByIndex(col), itemTable->aApplies[5].lValue);
			validCheck = 0;
		}
		else {
			int64_t ll;
			auto numStr = csvTable.AsStringByIndex(col);
			if (storm::ParseNumber(numStr, ll)) {
				dataArray[i] = ll;
			}
			else {
				spdlog::error("Failed to parse number in line {} column {}\n\t\tNumber was: {}", csvTable.GetCurRowNum(), col, numStr);
				dataArray[i] = 0;
			}


		}

		if (validCheck == -1)
		{
			std::ostringstream dataStream;

			for (int j = 0; j < i; ++j)
				dataStream << dataArray[j] << ",";

			//fprintf(stderr, "ItemProto Reading Failed : Invalid value.\n");
			spdlog::error("ItemProto Reading Failed : Invalid value. (index: {}, col: {}, value: {})", i, col, csvTable.AsStringByIndex(col));
			spdlog::error("\t{} ~ {} Values: {}", 0, i, dataStream.str().c_str());

			exit(0);
		}

		col = col + 1;
	}

	// vnum 및 vnum range 읽기.
	{
		std::string s(csvTable.AsStringByIndex(0));
		auto pos = s.find('~');
		// vnum 필드에 '~'가 없다면 패스
		if (std::string::npos == pos)
		{
			itemTable->dwVnum = dataArray[ITEM_PROTO_CELL_ID];
			itemTable->dwVnumRange = 0;
		}
		else
		{
			std::string s_start_vnum(s.substr(0, pos));
			std::string s_end_vnum(s.substr(pos + 1));

			int start_vnum = atoi(s_start_vnum.c_str());
			int end_vnum = atoi(s_end_vnum.c_str());
			if (0 == start_vnum || (0 != end_vnum && end_vnum < start_vnum))
			{
				spdlog::error("INVALID VNUM {}", s);
				return false;
			}
			itemTable->dwVnum = start_vnum;
			itemTable->dwVnumRange = end_vnum - start_vnum;
		}
	}

	strncpy(itemTable->szName, csvTable.AsStringByIndex(1), ITEM_NAME_MAX_LEN + 1);
	strncpy(itemTable->szLocaleName, itemTable->szName, ITEM_NAME_MAX_LEN + 1);

	itemTable->bType = (uint8_t)dataArray[ITEM_PROTO_CELL_ITEM_TYPE];
	itemTable->bSubType = (uint8_t)dataArray[ITEM_PROTO_CELL_SUB_TYPE];
	itemTable->bSize = (uint8_t)dataArray[ITEM_PROTO_CELL_SIZE];
	itemTable->dwAntiFlags = (uint32_t)dataArray[ITEM_PROTO_CELL_ANTI_FLAG];
	itemTable->dwFlags = (uint32_t)dataArray[ITEM_PROTO_CELL_FLAG];
	itemTable->dwWearFlags = (uint32_t)dataArray[ITEM_PROTO_CELL_ITEM_WEAR];
	itemTable->dwImmuneFlag = (uint32_t)dataArray[ITEM_PROTO_CELL_IMMUNE];
	itemTable->dwGold = (Gold)dataArray[ITEM_PROTO_CELL_SELL_PRICE];
	itemTable->dwShopBuyPrice = (Gold)dataArray[ITEM_PROTO_CELL_BUY_PRICE];
	itemTable->dwRefinedVnum = (uint32_t)dataArray[ITEM_PROTO_CELL_REFINE];
	itemTable->wRefineSet = (uint16_t)dataArray[ITEM_PROTO_CELL_REFINESET];
	itemTable->bAlterToMagicItemPct = (uint8_t)dataArray[ITEM_PROTO_CELL_MAGIC_PCT];


	int i;

	for (i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		itemTable->aLimits[i].bType = (uint8_t)dataArray[ITEM_PROTO_CELL_LIMIT_TYPE0 + i * 2];
		itemTable->aLimits[i].value = (long)dataArray[ITEM_PROTO_CELL_LIMIT_VALUE0 + i * 2];
	}

	/*for (i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		itemTable->aApplies[i].bType = (ApplyType)dataArray[ITEM_PROTO_CELL_ADDON_TYPE0 + i * 2];
		itemTable->aApplies[i].lValue = (ApplyValue)dataArray[ITEM_PROTO_CELL_ADDON_VALUE0 + i * 2];
	}*/

	for (i = 0; i < ITEM_VALUES_MAX_NUM; ++i)
		itemTable->alValues[i] = (long)dataArray[ITEM_PROTO_CELL_VALUE0 + i];

	itemTable->bSpecular = (long)dataArray[ITEM_PROTO_CELL_SPECULAR];//column for 'Specular'
	itemTable->bGainSocketPct = (long)dataArray[ITEM_PROTO_CELL_SOCKET];

	//test
	str_to_number(itemTable->bWeight, "0");

	itemTable->sAddonType = (short)dataArray[ITEM_PROTO_CELL_ATTU_ADDON];

	return true;
}

bool LoadItemProto(const std::string& filename,
	std::vector<TItemTable>& items)
{
	cCsvTable data;
	if (!data.Load(filename.c_str(), '\t')) {
		spdlog::error("Failed to load item_proto.txt: '{0}'", filename);
		return false;
	}

	// Skips the header
	data.Next();

	for (uint32_t i = 2; data.Next(); ++i) {
		TItemTable table = {};

		if (!Set_Proto_Item_Table(&table, data)) {
			spdlog::error("Failed to parse item data on line {0}", i);
			return false;
		}

		items.push_back(table);
	}

	return true;
}

bool LoadMobProto(const std::string& filename,
	std::vector<TMobTable>& mobs)
{
	cCsvTable data;
	if (!data.Load(filename.c_str(), '\t')) {
		spdlog::error("Failed to load mob_proto.txt: '{0}'", filename);
		return false;
	}

	// Skips the header
	data.Next();

	for (uint32_t i = 2; data.Next(); ++i) {
		TMobTable table = {};

		if (!Set_Proto_Mob_Table(&table, data)) {
			spdlog::error("Failed to parse mob data on line {0}", i);
			return false;
		}

		mobs.push_back(table);
	}

	return true;
}

METIN2_END_NS
