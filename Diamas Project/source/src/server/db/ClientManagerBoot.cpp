#include "ClientManager.h"
#include "Main.h"
#include "CsvReader.h"
#include <fmt/format.h>
#include "csv.h"
#include "base/ClientDb.hpp"
#include <storm/io/TextFileLoader.hpp>
#include <storm/Tokenizer.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

extern int g_test_server;
extern std::string g_stLocaleNameColumn;

bool CClientManager::LoadItemNames(const std::string& filename)
{
    bsys::error_code ec;
	std::string file;
	storm::ReadFileToString(filename, file, ec);

	if (ec) {
		spdlog::error("Failed to load item-desc '{0}' with '{1}'",
		          filename.c_str(), ec);
		return false;
	}
	std::vector<std::string> lines;
	storm::Tokenize(file,
	                "\r\n",
	                lines);

	std::vector<storm::String> args;

	int i = 0;
	for (const auto& line : lines) {

		args.clear();
		storm::Tokenize(line,
		                "\t",
		                args);

		if (args.empty())
			continue;

		if (args.size() < 2) {
			spdlog::error(
			          "item-desc '{0}' line {1} has only {2} tokens",
			          filename, i, args.size());
			continue;
		}

		uint32_t vnum;
		if (!storm::ParseNumber(args[0], vnum)) {
			spdlog::error(
			          "item-desc '{0}' line {1} vnum is invalid",
			          filename, i);
			return false;
		}

		m_itemNames[vnum] = args[1];
	}

	return true;
}

bool CClientManager::InitializeTables()
{
	/*if (!InitializeMobTable())
	{
		spdlog::error("InitializeMobTable FAILED");
		return false;
	}*/

	if (!InitializeItemTable())
	{
		spdlog::error("InitializeItemTable FAILED");
		return false; 
	}

	if (!LoadItemNames("data/locale/de/item_desc.txt"))
	{
		spdlog::error("Could not load data/locale/de/item_desc.txt");
		return false; 
	}

	if (!InitializeBanwordTable())
	{
		spdlog::error("InitializeBanwordTable FAILED");
		return false;
	}

	if (!InitializeLandTable())
	{
		spdlog::error("InitializeLandTable FAILED");
		return false;
	}

	if (!InitializeObjectTable())
	{
		spdlog::error("InitializeObjectTable FAILED");
		return false;
	}

#ifdef __OFFLINE_SHOP__
	if (!LoadOfflineShops())
	{
		spdlog::error("LoadOfflineShops FAILED");
		return false;
	}

	if (!LoadNextOfflineShopItemID())
	{
		spdlog::error("LoadNextOfflineShopItemID FAILED");
		return false;
	}
#endif

#ifdef ENABLE_BATTLE_PASS
	if (!LoadBattlePassRanking())
	{
		spdlog::error("LoadBattlePassRanking FAILED");
		return false;
	}
#endif
	
	MirrorItemTableIntoDb();

    if (!InitializeDungeonRanking())
	{
		spdlog::error("InitializeDungeonRanking FAILED");
		return false;
	}
	//
	return true;
}


bool CClientManager::MirrorItemTableIntoDb()
{
	std::string stQuery =
		"REPLACE INTO `item_proto` (`vnum`, `vnum_range`, `name`, `locale_name`, `type`, `subtype`, `size`, `antiflag`, `flag`, `wearflag`, `immuneflag`, `gold`, `shop_buy_price`, `refined_vnum`, `refine_set`, `refine_set2`, `magic_pct`, `limittype0`, `limitvalue0`, `limittype1`, `limitvalue1`, `applytype0`, `applyvalue0`, `applytype1`, `applyvalue1`, `applytype2`, `applyvalue2`, `applytype3`, `applyvalue3`, `applytype4`, `applyvalue4`, `applytype5`, `applyvalue5`, `value0`, `value1`, `value2`, `value3`, `value4`, `value5`, `specular`, `socket_pct`, `addon_type`) VALUES ('{vnum}', '{vnum_range}', '{name}', '{locale_name}', '{type}', '{subtype}', '{size}', '{antiflag}', '{flag}', '{wearflag}', '{immuneflag}', '{gold}', '{shop_buy_price}', '{refined_vnum}', '{refine_set}', '{refine_set2}', '{magic_pct}', '{limittype0}', '{limitvalue0}', '{limittype1}', '{limitvalue1}', '{applytype0}', '{applyvalue0}', '{applytype1}', '{applyvalue1}', '{applytype2}', '{applyvalue2}', '{applytype3}', '{applyvalue3}', '{applytype4}', '{applyvalue4}', '{applytype5}', '{applyvalue5}', '{value0}', '{value1}', '{value2}', '{value3}', '{value4}', '{value5}', '{specular}', '{socket_pct}', '{addon_type}');";
	
	char escapedLocaleName[ITEM_NAME_MAX_LEN * 4 + 1];

	for (auto& [vnum, t] : m_map_itemTableByVnum) {
		
		if(m_itemNames.find(vnum) != m_itemNames.end()) {
			CDBManager::instance().EscapeString(escapedLocaleName, m_itemNames[vnum].c_str(), m_itemNames[vnum].length());
		} else {
			CDBManager::instance().EscapeString(escapedLocaleName, static_cast<const char*>(t.szLocaleName), sizeof(t.szLocaleName));
		}

		CDBManager::instance().AsyncQuery(fmt::format(stQuery, fmt::arg("vnum", t.dwVnum),
		                                              fmt::arg("vnum_range", t.dwVnumRange),
		                                              fmt::arg("name", ""),
		                                              fmt::arg("locale_name", escapedLocaleName),
		                                              fmt::arg("type", t.bType), fmt::arg("subtype", t.bSubType),
		                                              fmt::arg("size", t.bSize), fmt::arg("antiflag", t.dwAntiFlags),
		                                              fmt::arg("flag", t.dwFlags), fmt::arg("wearflag", t.dwWearFlags),
		                                              fmt::arg("immuneflag", t.dwImmuneFlag),
		                                              fmt::arg("gold", t.dwGold),
		                                              fmt::arg("shop_buy_price", t.dwShopBuyPrice),
		                                              fmt::arg("refined_vnum", t.dwRefinedVnum),
		                                              fmt::arg("refine_set", t.wRefineSet), fmt::arg("refine_set2", 0),
		                                              fmt::arg("magic_pct", t.bAlterToMagicItemPct),
		                                              fmt::arg("limittype0", t.aLimits[0].bType),
		                                              fmt::arg("limitvalue0", t.aLimits[0].value),
		                                              fmt::arg("limittype1", t.aLimits[1].bType),
		                                              fmt::arg("limitvalue1", t.aLimits[1].value),
		                                              fmt::arg("applytype0", t.aApplies[0].bType),
		                                              fmt::arg("applyvalue0", t.aApplies[0].lValue),
		                                              fmt::arg("applytype1", t.aApplies[1].bType),
		                                              fmt::arg("applyvalue1", t.aApplies[1].lValue),
		                                              fmt::arg("applytype2", t.aApplies[2].bType),
		                                              fmt::arg("applyvalue2", t.aApplies[2].lValue),
		                                              fmt::arg("applytype3", t.aApplies[3].bType),
		                                              fmt::arg("applyvalue3", t.aApplies[3].lValue),
		                                              fmt::arg("applytype4", t.aApplies[4].bType),
		                                              fmt::arg("applyvalue4", t.aApplies[4].lValue),
		                                              fmt::arg("applytype5", t.aApplies[5].bType),
		                                              fmt::arg("applyvalue5", t.aApplies[5].lValue),
		                                              fmt::arg("value0", t.alValues[0]),
		                                              fmt::arg("value1", t.alValues[1]),
		                                              fmt::arg("value2", t.alValues[2]),
		                                              fmt::arg("value3", t.alValues[3]),
		                                              fmt::arg("value4", t.alValues[4]),
		                                              fmt::arg("value5", t.alValues[5]),
		                                              fmt::arg("specular", t.bSpecular),
		                                              fmt::arg("socket_pct", t.bGainSocketPct),
		                                              fmt::arg("addon_type", t.sAddonType)));
	}

	return true;
}


class FCompareVnum
{
	public:
		bool operator () (const TEntityTable & a, const TEntityTable & b) const
		{
			return (a.dwVnum < b.dwVnum);
		}
};

/*
bool CClientManager::InitializeMobTableFile()
{
	map<int,std::string> localMap;

	io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<'\t'>, io::throw_on_overflow, io::single_and_empty_line_comment<'#'>> names("mob_names.txt");
	  names.read_header(io::ignore_extra_column, "VNUM", "LOCALE_NAME");
	  uint32_t vnum; std::string name;
	  while(names.read_row(vnum, name)){
			localMap[vnum] = name.c_str();
	  }



	m_vec_mobTable.clear();
	m_vec_mobTable.reserve(2100);

	io::CSVReader<72, io::trim_chars<' '>, io::no_quote_escape<'\t'>, io::ignore_overflow, io::single_and_empty_line_comment<'#'>> proto("mob_proto.txt");
	proto.read_header(io::ignore_extra_column, "VNUM", "NAME", "RANK", "TYPE", "BATTLE_TYPE", "LEVEL", "SIZE",
	                  "AI_FLAG", "MOUNT_CAPACITY", "RACE_FLAG", "IMMUNE_FLAG", "EMPIRE", "FOLDER", "ON_CLICK", "ST",
	                  "DX", "HT", "IQ", "DAMAGE_MIN", "DAMAGE_MAX", "MAX_HP", "REGEN_CYCLE", "REGEN_PERCENT",
	                  "GOLD_MIN", "GOLD_MAX", "EXP", "DEF", "ATTACK_SPEED", "MOVE_SPEED", "AGGRESSIVE_HP_PCT",
	                  "AGGRESSIVE_SIGHT", "ATTACK_RANGE", "DROP_ITEM", "RESURRECTION_VNUM", "ENCHANT_CURSE",
	                  "ENCHANT_SLOW", "ENCHANT_POISON", "ENCHANT_STUN", "ENCHANT_CRITICAL", "ENCHANT_PENETRATE",
	                  "RESIST_SWORD", "RESIST_TWOHAND", "RESIST_DAGGER", "RESIST_BELL", "RESIST_FAN", "RESIST_BOW",
	                  "RESIST_FIRE", "RESIST_ELECT", "RESIST_MAGIC", "RESIST_WIND", "RESIST_POISON", "RESIST_BLEEDING",
	                  "DAM_MULTIPLY", "SUMMON", "DRAIN_SP", "MOB_COLOR", "POLYMORPH_ITEM", "SKILL_LEVEL0",
	                  "SKILL_VNUM0", "SKILL_LEVEL1", "SKILL_VNUM1", "SKILL_LEVEL2", "SKILL_VNUM2", "SKILL_LEVEL3",
	                  "SKILL_VNUM3", "SKILL_LEVEL4", "SKILL_VNUM4", "SP_BERSERK", "SP_STONESKIN", "SP_GODSPEED",
	                  "SP_DEATHBLOW", "SP_REVIVE");
	  

	try {
		TMobTable mobTable;

		std::string rankValue;
		std::string mobType;
		std::string battleType;
		std::string aiFlag;
		std::string raceFlag;
		std::string immuneFlag;
		std::string mobName;
		std::string folder;


		while(proto.read_row(mobTable.dwVnum, mobName, rankValue, mobType, battleType,
		                     mobTable.bLevel, mobTable.wScale, aiFlag, mobTable.bMountCapacity,
		                     raceFlag, immuneFlag, mobTable.bEmpire, folder, mobTable.bOnClickType,
		                     mobTable.bStr, mobTable.bDex, mobTable.bCon, mobTable.bInt, mobTable.dwDamageRange[0], mobTable.dwDamageRange[1],
							 mobTable.dwMaxHP, mobTable.bRegenCycle, mobTable.bRegenPercent,
							 mobTable.dwGoldMin, mobTable.dwGoldMax, mobTable.dwExp, mobTable.wDef, mobTable.sAttackSpeed,
							 mobTable.sMovingSpeed, mobTable.bAggresiveHPPct, mobTable.wAggressiveSight, mobTable.wAttackRange,
							 mobTable.dwDropItemVnum, mobTable.dwResurrectionVnum, mobTable.cEnchants[0],
							 mobTable.cEnchants[1], mobTable.cEnchants[2], mobTable.cEnchants[3], mobTable.cEnchants[4], mobTable.cEnchants[5],
							 mobTable.cResists[0], mobTable.cResists[1],mobTable.cResists[2],mobTable.cResists[3],mobTable.cResists[4],mobTable.cResists[5],
							 mobTable.cResists[6], mobTable.cResists[7],mobTable.cResists[8],mobTable.cResists[9],mobTable.cResists[10],mobTable.cResists[11],
							 mobTable.fDamMultiply, mobTable.dwSummonVnum, mobTable.dwDrainSP, mobTable.dwMobColor, mobTable.dwPolymorphItemVnum,
							 mobTable.Skills[0].bLevel, mobTable.Skills[0].dwVnum, mobTable.Skills[1].bLevel, mobTable.Skills[1].dwVnum,
							 mobTable.Skills[2].bLevel, mobTable.Skills[2].dwVnum,mobTable.Skills[3].bLevel, mobTable.Skills[3].dwVnum,
							 mobTable.Skills[4].bLevel, mobTable.Skills[4].dwVnum, mobTable.bBerserkPoint, mobTable.bStoneSkinPoint,
							 mobTable.bGodSpeedPoint, mobTable.bDeathBlowPoint, mobTable.bRevivePoint)) {

			
			mobTable.bRank = get_Mob_Rank_Value(rankValue);
			mobTable.bType = get_Mob_Type_Value(mobType);
			mobTable.bBattleType = get_Mob_BattleType_Value(battleType);
			mobTable.dwAIFlag = get_Mob_AIFlag_Value(aiFlag);
			mobTable.dwRaceFlag = get_Mob_RaceFlag_Value(raceFlag);
			mobTable.dwImmuneFlag = get_Mob_ImmuneFlag_Value(immuneFlag);

			strlcpy(mobTable.szName, mobName.c_str(), sizeof(mobTable.szName));
					
			if (auto it = localMap.find(mobTable.dwVnum); it != localMap.end()) {
				strlcpy(mobTable.szLocaleName, it->second.c_str(), sizeof(mobTable.szLocaleName));
			}
			else {
				strlcpy(mobTable.szLocaleName, mobTable.szName, sizeof(mobTable.szLocaleName));
			}

			m_vec_mobTable.emplace_back(mobTable);
		  }

		sort(m_vec_mobTable.begin(), m_vec_mobTable.end(), FCompareVnum());
	} catch(const std::exception& ec) {
		spdlog::critical("{}", ec.what());
		std::exit(0);
	}
	

	
	return true;
}

bool CClientManager::InitializeMobTable()
{
	SPDLOG_INFO( "InitializeMobTable: Loading from file ...");

	bool res = InitializeMobTableFile();
	if (!res) {
		SPDLOG_INFO( "InitializeMobTable: Loading from file failed.");
	}

	SPDLOG_INFO( "InitializeMobTable: Loaded {} mobs.", m_vec_mobTable.size());

	return res;
}
*/

bool CClientManager::InitializeItemTableFile()
{
	std::vector<TItemTable> v;
	if (!LoadClientDbFromFile("data/item_proto_server", v))
		return false;

	for (const auto& item : v) {
		m_map_itemTableByVnum.insert(std::make_pair(item.dwVnum, item));
	}

	/*
	 *
	 *
	map<int, std::string> localMap;
	try {

		io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<'\t'>, io::ignore_overflow, io::
		              single_and_empty_line_comment<'#'>> names("item_names.txt");
		names.read_header(io::ignore_extra_column, "VNUM", "LOCALE_NAME");
		uint32_t vnum;
		std::string name;
		while(names.read_row(vnum, name)) {
			localMap[vnum] = name;
		}
	}
	catch(const std::exception& ec) {
		spdlog::critical("{}", ec.what());
		std::exit(0);
	}
	try {
		io::CSVReader<39, io::trim_chars<' '>, io::no_quote_escape<'\t'>, io::ignore_overflow, io::
		              single_and_empty_line_comment<'#'>> proto("item_proto.txt");
		proto.read_header(io::ignore_extra_column, "ID", "ITEM_NAME", "ITEM_TYPE", "SUB_TYPE", "SIZE", "ANTI_FLAG",
		                  "FLAG",
		                  "ITEM_WEAR", "IMMUNE", "SELL_PRICE", "BUY_PRICE", "REFINE", "REFINESET", "MAGIC_PCT",
		                  "LIMIT_TYPE0", "LIMIT_VALUE0", "LIMIT_TYPE1", "LIMIT_VALUE1", "ADDON_TYPE0", "ADDON_VALUE0",
		                  "ADDON_TYPE1", "ADDON_VALUE1", "ADDON_TYPE2", "ADDON_VALUE2", "APPLY_TYPE3", "APPLY_VALUE3",
		                  "APPLY_TYPE4", "APPLY_VALUE4", "APPLY_TYPE5", "APPLY_VALUE5", "VALUE0", "VALUE1", "VALUE2",
		                  "VALUE3", "VALUE4", "VALUE5", "SPECULAR", "SOCKET", "ATTU_ADDON");


		std::string itemVnum;
		std::string itemName;
		std::string itemType;
		std::string itemSubType;
		std::string antiFlag;
		std::string flag;
		std::string wearFlag;
		std::string immuneFlag;
		std::string limitType0;
		std::string limitValue0;
		std::string limitType1;
		std::string limitValue1;
		std::string applyType0;
		std::string applyType1;
		std::string applyType2;
		std::string applyType3;
		std::string applyType4;
		std::string applyType5;

		TItemTable itemTable;

		while(proto.read_row(itemVnum, itemName, itemType, itemSubType, itemTable.bSize, antiFlag, flag, wearFlag,
		                     immuneFlag,
		                     itemTable.dwGold, itemTable.dwShopBuyPrice, itemTable.dwRefinedVnum, itemTable.wRefineSet,
		                     itemTable.bAlterToMagicItemPct, limitType0, itemTable.aLimits[0].lValue, limitType1,
		                     itemTable.aLimits[1].lValue,

		                     applyType0, itemTable.aApplies[0].lValue,
		                     applyType1, itemTable.aApplies[1].lValue,
		                     applyType2, itemTable.aApplies[2].lValue,
		                     applyType3, itemTable.aApplies[3].lValue,
		                     applyType4, itemTable.aApplies[4].lValue,
		                     applyType5, itemTable.aApplies[5].lValue,
		                     itemTable.alValues[0], itemTable.alValues[1], itemTable.alValues[2],
		                     itemTable.alValues[3], itemTable.alValues[4], itemTable.alValues[5],
		                     itemTable.bSpecular, itemTable.bGainSocketPct, itemTable.sAddonType)) {

			// Process the item vnum field
			{
				if(auto pos = itemVnum.find('~'); std::string::npos == pos) {
					str_to_number(itemTable.dwVnum, itemVnum.c_str());
					itemTable.dwVnumRange = 0;
				} else {
					auto s_start_vnum(itemVnum.substr(0, pos));
					auto s_end_vnum(itemVnum.substr(pos + 1));

					int start_vnum = 0, end_vnum = 0;
					str_to_number(start_vnum, s_start_vnum.c_str());;
					str_to_number(end_vnum, s_end_vnum.c_str());

					if(0 == start_vnum || (0 != end_vnum && end_vnum < start_vnum)) {
						spdlog::error("INVALID VNUM {}", itemVnum.c_str());
						return false;
					}
					itemTable.dwVnum = start_vnum;
					itemTable.dwVnumRange = end_vnum - start_vnum;
				}
			}

			strlcpy(itemTable.szName, itemName.c_str(), ITEM_NAME_MAX_LEN + 1);
			auto it = localMap.find(itemTable.dwVnum);

			strlcpy(itemTable.szLocaleName, (it != localMap.end()) ? it->second.c_str() : itemTable.szName,
			        ITEM_NAME_MAX_LEN + 1);

			itemTable.bType = get_Item_Type_Value(itemType);
			itemTable.bSubType = get_Item_SubType_Value(itemTable.bType, itemSubType);
			itemTable.dwAntiFlags = get_Item_AntiFlag_Value(antiFlag);
			itemTable.dwFlags = get_Item_Flag_Value(flag);
			itemTable.dwWearFlags = get_Item_WearFlag_Value(wearFlag);
			itemTable.dwImmuneFlag = get_Item_Immune_Value(immuneFlag);
			itemTable.aLimits[0].bType = get_Item_LimitType_Value(limitType0);
			itemTable.aLimits[1].bType = get_Item_LimitType_Value(limitType1);

			itemTable.aApplies[0].bType = get_Item_ApplyType_Value(applyType0);
			itemTable.aApplies[1].bType = get_Item_ApplyType_Value(applyType1);
			itemTable.aApplies[2].bType = get_Item_ApplyType_Value(applyType2);
			itemTable.aApplies[3].bType = get_Item_ApplyType_Value(applyType3);
			itemTable.aApplies[4].bType = get_Item_ApplyType_Value(applyType4);
			itemTable.aApplies[5].bType = get_Item_ApplyType_Value(applyType5);


			auto ref = m_vec_itemTable.emplace_back(itemTable);
			m_map_itemTableByVnum.emplace(itemTable.dwVnum, &ref);
		}
	}
	catch(const std::exception& ec) {
		spdlog::critical("{}", ec.what());
		std::exit(0);
	}


	sort(m_vec_itemTable.begin(), m_vec_itemTable.end(), FCompareVnum());
		 */

	return true;
}

bool CClientManager::InitializeItemTable()
{
	SPDLOG_INFO( "InitializeItemTable: Loading from file ...");
	bool res = InitializeItemTableFile();
	if (!res)
	{
		SPDLOG_INFO( "InitializeItemTable: Loading from file failed.");
	}
	//SPDLOG_INFO( "InitializeItemTable: Loaded {} items.\n", m_vec_itemTable.size());

	return res;
}

bool CClientManager::InitializeBanwordTable()
{
	m_vec_banwordTable.clear();

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery("SELECT word FROM banword"));

	SQLResult * pRes = pkMsg->Get();

	if (pRes->uiNumRows == 0)
		return true;

	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TBanwordTable t;

		if (data[0])
		{
			strlcpy(t.szWord, data[0], sizeof(t.szWord));
			m_vec_banwordTable.push_back(t);
		}
	}

	SPDLOG_INFO( "BANWORD: total %d", m_vec_banwordTable.size());
	return true;
}

bool CClientManager::InitializeLandTable()
{
	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery("SELECT id, map_index, x, y, width, height, guild_id, guild_level_limit, price "
		"FROM land WHERE enable='YES' ORDER BY id"));
	SQLResult * pRes = pkMsg->Get();

	if (!m_vec_kLandTable.empty())
	{
		SPDLOG_INFO( "RELOAD: land");
		m_vec_kLandTable.clear();
	}

	m_vec_kLandTable.reserve((uint32_t)pRes->uiNumRows);

	MYSQL_ROW	data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TLand t;

			memset(&t, 0, sizeof(t));

			int col = 0;

			str_to_number(t.id, data[col++]);
			str_to_number(t.mapIndex, data[col++]);
			str_to_number(t.x, data[col++]);
			str_to_number(t.y, data[col++]);
			str_to_number(t.width, data[col++]);
			str_to_number(t.height, data[col++]);
			str_to_number(t.guildId, data[col++]);
			str_to_number(t.guildLevelLimit, data[col++]);
			str_to_number(t.price, data[col++]);

			SPDLOG_INFO( "LAND: {0} map {1} {2}x{3} w {4} h {5}",
			          t.id, t.mapIndex,
			          t.x, t.y, t.width, t.height);

			m_vec_kLandTable.push_back(t);
		}

	return true;
}

void parse_pair_number_string(const char * c_pszString, std::vector<std::pair<int, int> > & vec)
{
	// format: 10,1/20,3/300,50
	const char * t = c_pszString;
	const char * p = strchr(t, '/');
	std::pair<int, int> k;

	char szNum[32 + 1];
	char * comma;

	while (p)
	{
		if (isdigit(*t))
		{
			strlcpy(szNum, t, std::min<size_t>(sizeof(szNum), (p-t)+1));

			comma = strchr(szNum, ',');

			if (comma)
			{
				*comma = '\0';
				str_to_number(k.second, comma+1);
			}
			else
				k.second = 0;

			str_to_number(k.first, szNum);
			vec.push_back(k);
		}

		t = p + 1;
		p = strchr(t, '/');
	}

	if (isdigit(*t))
	{
		strlcpy(szNum, t, sizeof(szNum));

		comma = strchr(const_cast<char*>(t), ',');

		if (comma)
		{
			*comma = '\0';
			str_to_number(k.second, comma+1);
		}
		else
			k.second = 0;

		str_to_number(k.first, szNum);
		vec.push_back(k);
	}
}

bool CClientManager::InitializeObjectTable()
{

	std::unique_ptr<SQLMsg> pkMsg(CDBManager::instance().DirectQuery("SELECT id, land_id, vnum, map_index, x, y, x_rot, y_rot, z_rot, life FROM object ORDER BY id"));
	SQLResult * pRes = pkMsg->Get();

	if (!m_map_pkObjectTable.empty())
	{
		SPDLOG_INFO( "RELOAD: object");
		m_map_pkObjectTable.clear();
	}

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			auto k = std::make_unique<TObject>();

			int col = 0;

			str_to_number(k->id, data[col++]);
			str_to_number(k->landId, data[col++]);
			str_to_number(k->vnum, data[col++]);
			str_to_number(k->mapIndex, data[col++]);
			str_to_number(k->x, data[col++]);
			str_to_number(k->y, data[col++]);
			str_to_number(k->xRot, data[col++]);
			str_to_number(k->yRot, data[col++]);
			str_to_number(k->zRot, data[col++]);
			str_to_number(k->life, data[col++]);

			SPDLOG_INFO("OBJ: {0} vnum {1} map {2} {3}x{4} life {5}",
			          k->id, k->vnum, k->mapIndex, k->x, k->y, k->life);

			m_map_pkObjectTable.emplace(k->id, std::move(k));
		}

	return true;
}
