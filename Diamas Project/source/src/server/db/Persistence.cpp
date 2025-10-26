#include "Persistence.hpp"
#include "DBManager.h"
#include "Main.h"

bool CreatePlayerCreateQuery(const TPlayerCreatePacket& p, std::string& query)
{
	const auto* format = "INSERT player SET "
		"created_at = NOW(),"
		"account_id = {account_id},"
		"slot = {slot},"
		"name = '{name}',"
		"empire = {empire},"
		"job = {job}, "
		"voice = {voice}, "
		"dir = {dir}, "
		"x = {x}, "
		"y = {y}, "
		"z = {z}, "
		"map_index = {map_index}, "
		"exit_x = {exit_x}, "
		"exit_y = {exit_y}, "
		"exit_map_index = {exit_map_index}, "
		"hp = {hp}, "
		"mp = {mp}, "
		"stamina = {stamina}, "
		"random_hp = {random_hp}, "
		"random_sp = {random_sp}, "
		"playtime = {playtime}, "
		"level = {level}, "
		"level_step = {level_step}, "
		"st = {st}, "
		"ht = {ht}, "
		"dx = {dx}, "
		"iq = {iq}, "
		"gold = {gold}, "
		"exp = {exp}, "
		"stat_point = {stat_point}, "
		"skill_point = {skill_point}, "
		"sub_skill_point = {sub_skill_point}, "
		"ip = '{ip}', "
		"part_base = {part_base},"
		"part_main = {part_main},"
		"part_main_appearance = {part_main_appearance},"
		"part_main_costume = {part_main_costume},"
		"part_main_costume_appearance = {part_main_costume_appearance},"
		"part_hair = {part_hair},"
		"part_hair_appearance = {part_hair_appearance},"
		"part_hair_costume = {part_hair_costume},"
		"part_hair_costume_appearance = {part_hair_costume_appearance},"
		"part_acce = {part_acce},"
		"part_acce_appearance = {part_acce_appearance},"
		"part_acce_costume = {part_acce_costume},"
		"part_acce_costume_appearance = {part_acce_costume_appearance},"
		"last_play = NOW(), "
		"skill_group = {skill_group}, "
		"alignment = {alignment}, "
		"horse_level = {horse_level}, "
		"horse_riding = {horse_riding}, "
		"horse_hp = {horse_hp}, "
		"horse_hp_droptime = {horse_hp_droptime}, "
		"horse_stamina = {horse_stamina}, "
		"horse_skill_point = {horse_skill_point},"
		"tree_skill_point = {tree_skill_point},"
		"block_mode = {block_mode},"
		"chat_filter = {chat_filter},"

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
		"offline_shop_farmed_opening_time = {offline_shop_farmed_opening_time}, "
		"offline_shop_special_opening_time = {offline_shop_special_opening_time}, "
#endif
		"battle_pass_end_time = {battle_pass_end_time} "
		";";

	const auto& tab = p.player_table;

	query = fmt::format(format,
		fmt::arg("account_id", p.account_id),
		fmt::arg("slot", p.slot),
		fmt::arg("name", tab.name),
		fmt::arg("job", tab.job),
		fmt::arg("empire", tab.empire),
		fmt::arg("voice", tab.voice),
		fmt::arg("dir", tab.dir),
		fmt::arg("x", tab.x),
		fmt::arg("y", tab.y),
		fmt::arg("z", tab.z),
		fmt::arg("map_index", tab.lMapIndex),
		fmt::arg("exit_x", tab.lExitX),
		fmt::arg("exit_y", tab.lExitY),
		fmt::arg("exit_map_index", tab.lExitMapIndex),
		fmt::arg("hp", tab.hp),
		fmt::arg("mp", tab.sp),
		fmt::arg("stamina", tab.stamina),
		fmt::arg("random_hp", tab.sRandomHP),
		fmt::arg("random_sp", tab.sRandomSP),
		fmt::arg("playtime", tab.playtime),
		fmt::arg("level", tab.level),
		fmt::arg("level_step", tab.level_step),
		fmt::arg("st", tab.st),
		fmt::arg("ht", tab.ht),
		fmt::arg("dx", tab.dx),
		fmt::arg("iq", tab.iq),
		fmt::arg("gold", tab.gold),
		fmt::arg("exp", tab.exp ),
		fmt::arg("stat_point", tab.stat_point),
		fmt::arg("skill_point", tab.skill_point),
		fmt::arg("sub_skill_point", tab.sub_skill_point),
		fmt::arg("ip", tab.ip),
		fmt::arg("part_base", tab.part_base),
		fmt::arg("part_main", tab.parts[PART_MAIN].vnum),
		fmt::arg("part_main_appearance", tab.parts[PART_MAIN].appearance),
		fmt::arg("part_main_costume", tab.parts[PART_MAIN].costume),
		fmt::arg("part_main_costume_appearance", tab.parts[PART_MAIN].costume_appearance),
		fmt::arg("part_hair", tab.parts[PART_HAIR].vnum),
		fmt::arg("part_hair_appearance", tab.parts[PART_HAIR].appearance),
		fmt::arg("part_hair_costume", tab.parts[PART_HAIR].costume),
		fmt::arg("part_hair_costume_appearance", tab.parts[PART_HAIR].costume_appearance),
		fmt::arg("part_acce", tab.parts[PART_ACCE].vnum),
		fmt::arg("part_acce_appearance", tab.parts[PART_ACCE].appearance),
		fmt::arg("part_acce_costume", tab.parts[PART_ACCE].costume),
		fmt::arg("part_acce_costume_appearance", tab.parts[PART_ACCE].costume_appearance),
		fmt::arg("last_play", tab.last_play),
		fmt::arg("skill_group", tab.skill_group),
		fmt::arg("alignment", tab.lAlignment),
		fmt::arg("horse_level", tab.horse.bLevel),
		fmt::arg("horse_riding", tab.horse.bRiding),
		fmt::arg("horse_hp", tab.horse.sHealth),
		fmt::arg("horse_hp_droptime", tab.horse.dwHorseHealthDropTime),
		fmt::arg("horse_stamina", tab.horse.sStamina),
		fmt::arg("horse_skill_point", tab.horse_skill_point),
		fmt::arg("tree_skill_point", tab.tree_skill_point),
		fmt::arg("id", tab.id),
		fmt::arg("block_mode", tab.blockMode),
		fmt::arg("chat_filter", tab.chatFilter),
		fmt::arg("offline_shop_farmed_opening_time", tab.iOfflineShopFarmedOpeningTime),
		fmt::arg("offline_shop_special_opening_time", tab.iOfflineShopSpecialOpeningTime),
		fmt::arg("battle_pass_end_time", tab.dwBattlePassEndTime)
		);


	return true;
}


bool CreatePlayerSaveQuery(const TPlayerTable& tab, std::string& query)
{
	// Following fields aren't updated:
	// * name
	// * empire
	std::string_view format = "UPDATE player SET "
		"job = {job}, "
		"voice = {voice}, "
		"dir = {dir}, "
#ifdef __FAKE_PC__
		"fakepc_name = '{fakepc_name}',"
#endif
		"x = {x}, "
		"y = {y}, "
		"z = {z}, "
		"map_index = {map_index}, "
		"exit_x = {exit_x}, "
		"exit_y = {exit_y}, "
		"exit_map_index = {exit_map_index}, "
		"hp = {hp}, "
		"mp = {mp}, "
		"stamina = {stamina}, "
		"random_hp = {random_hp}, "
		"random_sp = {random_sp}, "
		"playtime = {playtime}, "
		"level = {level}, "
		"level_step = {level_step}, "
		"st = {st}, "
		"ht = {ht}, "
		"dx = {dx}, "
		"iq = {iq}, "
		"gold = {gold}, "
		"exp = {exp}, "
		"stat_point = {stat_point}, "
		"skill_point = {skill_point}, "
		"sub_skill_point = {sub_skill_point}, "
		"ip = '{ip}', "
		"part_base = {part_base},"
		"part_main = {part_main},"
		"part_main_appearance = {part_main_appearance},"
		"part_main_costume = {part_main_costume},"
		"part_main_costume_appearance = {part_main_costume_appearance},"
		"part_hair = {part_hair},"
		"part_hair_appearance = {part_hair_appearance},"
		"part_hair_costume = {part_hair_costume},"
		"part_hair_costume_appearance = {part_hair_costume_appearance},"
		"part_acce = {part_acce},"
		"part_acce_appearance = {part_acce_appearance},"
		"part_acce_costume = {part_acce_costume},"
		"part_acce_costume_appearance = {part_acce_costume_appearance},"
		"last_play = NOW(), "
		"skill_group = {skill_group}, "
		"alignment = {alignment}, "
		"horse_level = {horse_level}, "
		"horse_riding = {horse_riding}, "
		"horse_hp = {horse_hp}, "
		"horse_hp_droptime = {horse_hp_droptime}, "
		"horse_stamina = {horse_stamina}, "
		"horse_skill_point = {horse_skill_point},"
		"tree_skill_point = {tree_skill_point},"
		"block_mode = {block_mode},"
		"chat_filter = {chat_filter}"
	#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
		",offline_shop_farmed_opening_time = {offline_shop_farmed_opening_time}, "
		"offline_shop_special_opening_time = {offline_shop_special_opening_time}, "
#endif
		"battle_pass_end_time = {battle_pass_end_time}"
		" WHERE id={id}";



	query = fmt::format(format,
		fmt::arg("job", tab.job),
		fmt::arg("voice", tab.voice),
		fmt::arg("dir", tab.dir),
#ifdef __FAKE_PC__
		fmt::arg("fakepc_name", tab.fakepc_name),
#endif
		fmt::arg("x", tab.x),
		fmt::arg("y", tab.y),
		fmt::arg("z", tab.z),
		fmt::arg("map_index", tab.lMapIndex),
		fmt::arg("exit_x", tab.lExitX),
		fmt::arg("exit_y", tab.lExitY),
		fmt::arg("exit_map_index", tab.lExitMapIndex),
		fmt::arg("hp", tab.hp),
		fmt::arg("mp", tab.sp),
		fmt::arg("stamina", tab.stamina),
		fmt::arg("random_hp", tab.sRandomHP),
		fmt::arg("random_sp", tab.sRandomSP),
		fmt::arg("playtime", tab.playtime),
		fmt::arg("level", tab.level),
		fmt::arg("level_step", tab.level_step),
		fmt::arg("st", tab.st),
		fmt::arg("ht", tab.ht),
		fmt::arg("dx", tab.dx),
		fmt::arg("iq", tab.iq),
		fmt::arg("gold", tab.gold),
		fmt::arg("exp", tab.exp ),
		fmt::arg("stat_point", tab.stat_point),
		fmt::arg("skill_point", tab.skill_point),
		fmt::arg("sub_skill_point", tab.sub_skill_point),
		fmt::arg("ip", tab.ip),
		fmt::arg("part_base", tab.part_base),
		fmt::arg("part_main", tab.parts[PART_MAIN].vnum),
		fmt::arg("part_main_appearance", tab.parts[PART_MAIN].appearance),
		fmt::arg("part_main_costume", tab.parts[PART_MAIN].costume),
		fmt::arg("part_main_costume_appearance", tab.parts[PART_MAIN].costume_appearance),
		fmt::arg("part_hair", tab.parts[PART_HAIR].vnum),
		fmt::arg("part_hair_appearance", tab.parts[PART_HAIR].appearance),
		fmt::arg("part_hair_costume", tab.parts[PART_HAIR].costume),
		fmt::arg("part_hair_costume_appearance", tab.parts[PART_HAIR].costume_appearance),
		fmt::arg("part_acce", tab.parts[PART_ACCE].vnum),
		fmt::arg("part_acce_appearance", tab.parts[PART_ACCE].appearance),
		fmt::arg("part_acce_costume", tab.parts[PART_ACCE].costume),
		fmt::arg("part_acce_costume_appearance", tab.parts[PART_ACCE].costume_appearance),
		fmt::arg("last_play", tab.last_play),
		fmt::arg("skill_group", tab.skill_group),
		fmt::arg("alignment", tab.lAlignment),
		fmt::arg("horse_level", tab.horse.bLevel),
		fmt::arg("horse_riding", tab.horse.bRiding),
		fmt::arg("horse_hp", tab.horse.sHealth),
		fmt::arg("horse_hp_droptime", tab.horse.dwHorseHealthDropTime),
		fmt::arg("horse_stamina", tab.horse.sStamina),
		fmt::arg("horse_skill_point", tab.horse_skill_point),
		fmt::arg("tree_skill_point", tab.tree_skill_point),
		fmt::arg("id", tab.id),
		fmt::arg("block_mode", tab.blockMode),
		fmt::arg("chat_filter", tab.chatFilter),
		fmt::arg("offline_shop_farmed_opening_time", tab.iOfflineShopFarmedOpeningTime),
		fmt::arg("offline_shop_special_opening_time", tab.iOfflineShopSpecialOpeningTime),
		fmt::arg("battle_pass_end_time", tab.dwBattlePassEndTime)
	);


	return true;
}

bool CreatePlayerLoadQuery(uint32_t pid, std::string& query)
{
	query += fmt::format("SELECT "
			"id,name,"
#ifdef __FAKE_PC__
		"fakepc_name,"
#endif
			"empire,job,voice,dir,x,y,z,map_index,exit_x,exit_y,exit_map_index,hp,mp,stamina,random_hp,random_sp,playtime,"
			"gold,level,level_step,st,ht,dx,iq,exp,"
			"stat_point, skill_point, sub_skill_point, part_base,"
			"part_main,part_main_appearance,part_main_costume,part_main_costume_appearance,"
			"part_hair,part_hair_appearance,part_hair_costume,part_hair_costume_appearance,"
			"part_acce,part_acce_appearance,part_acce_costume,part_acce_costume_appearance,"
			"skill_group,alignment,horse_level,horse_riding,horse_hp,horse_hp_droptime,horse_stamina,"
#ifdef ENABLE_GEM_SYSTEM			
			"gaya,",
#endif
			"UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(last_play),horse_skill_point,tree_skill_point,UNIX_TIMESTAMP(last_play),block_mode, chat_filter"
#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
				", offline_shop_farmed_opening_time, offline_shop_special_opening_time"
#endif
		",battle_pass_end_time"
		" FROM player WHERE id={} AND deleted = 0",

	         pid);
	return true;
}

bool CreatePlayerTableFromRes(MYSQL_RES* res, TPlayerTable& tab)
{
	
	if (mysql_num_rows(res) == 0)	// µ¥ÀÌÅÍ ¾øÀ½
		return false;

	MYSQL_ROW row = mysql_fetch_row(res);

	int	col = 0;

	//  "id,name,empire,job,voice,dir,x,y,z,map_index,exit_x,exit_y,exit_map_index,hp,mp,stamina,random_hp,random_sp,playtime,"

	str_to_number(tab.id, row[col++]);
	storm::CopyStringSafe(tab.name, row[col++], sizeof(tab.name));
	str_to_number(tab.empire, row[col++]);
	str_to_number(tab.job, row[col++]);
	str_to_number(tab.voice, row[col++]);
	str_to_number(tab.dir, row[col++]);
	str_to_number(tab.x, row[col++]);
	str_to_number(tab.y, row[col++]);
	str_to_number(tab.z, row[col++]);
	str_to_number(tab.lMapIndex, row[col++]);
	str_to_number(tab.lExitX, row[col++]);
	str_to_number(tab.lExitY, row[col++]);
	str_to_number(tab.lExitMapIndex, row[col++]);
	str_to_number(tab.hp, row[col++]);
	str_to_number(tab.sp, row[col++]);
	str_to_number(tab.stamina, row[col++]);
	str_to_number(tab.sRandomHP, row[col++]);
	str_to_number(tab.sRandomSP, row[col++]);
	str_to_number(tab.playtime, row[col++]);

	//	"gold,level,level_step,st,ht,dx,iq,exp,"

	str_to_number(tab.gold, row[col++]);
	str_to_number(tab.level, row[col++]);
	str_to_number(tab.level_step, row[col++]);
	str_to_number(tab.st, row[col++]);
	str_to_number(tab.ht, row[col++]);
	str_to_number(tab.dx, row[col++]);
	str_to_number(tab.iq, row[col++]);
	str_to_number(tab.exp, row[col++]);
	//	"stat_point,skill_point,sub_skill_point,stat_reset_count,part_base,part_hair,part_acce,"

	str_to_number(tab.stat_point, row[col++]);
	str_to_number(tab.skill_point, row[col++]);
	str_to_number(tab.sub_skill_point, row[col++]);
	str_to_number(tab.part_base, row[col++]);
	str_to_number(tab.parts[PART_MAIN].vnum, row[col++]);
	str_to_number(tab.parts[PART_MAIN].appearance, row[col++]);
	str_to_number(tab.parts[PART_MAIN].costume, row[col++]);
	str_to_number(tab.parts[PART_MAIN].costume_appearance, row[col++]);
	str_to_number(tab.parts[PART_HAIR].vnum, row[col++]);
	str_to_number(tab.parts[PART_HAIR].appearance, row[col++]);
	str_to_number(tab.parts[PART_HAIR].costume, row[col++]);
	str_to_number(tab.parts[PART_HAIR].costume_appearance, row[col++]);
	str_to_number(tab.parts[PART_ACCE].vnum, row[col++]);
	str_to_number(tab.parts[PART_ACCE].appearance, row[col++]);
	str_to_number(tab.parts[PART_ACCE].costume, row[col++]);
	str_to_number(tab.parts[PART_ACCE].costume_appearance, row[col++]);

	//	"skill_level,quickslot,skill_group,alignment,mobile,horse_level,horse_riding,horse_hp,horse_hp_droptime,horse_stamina,"

	str_to_number(tab.skill_group, row[col++]);
	str_to_number(tab.lAlignment, row[col++]);
	str_to_number(tab.horse.bLevel, row[col++]);
	str_to_bool(tab.horse.bRiding, row[col++]);
	str_to_number(tab.horse.sHealth, row[col++]);
	str_to_number(tab.horse.dwHorseHealthDropTime, row[col++]);
	str_to_number(tab.horse.sStamina, row[col++]);

	//	"UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(last_play),horse_skill_point,UNIX_TIMESTAMP(last_play), block_mode FROM player WHERE id=%d",

	str_to_number(tab.logoff_interval, row[col++]);
	str_to_number(tab.horse_skill_point, row[col++]);
	str_to_number(tab.tree_skill_point, row[col++]);
#ifdef ENABLE_GEM_SYSTEM
	str_to_number(pkTab->gem, row[col++]);
#endif	
	
	str_to_number(tab.last_play, row[col++]);

	str_to_number(tab.blockMode, row[col++]);
	str_to_number(tab.chatFilter, row[col++]);

	str_to_number(tab.iOfflineShopFarmedOpeningTime, row[col++]);
	str_to_number(tab.iOfflineShopSpecialOpeningTime, row[col++]);
	str_to_number(tab.dwBattlePassEndTime, row[col++]);

	// reset sub_skill_point
	{
		tab.skills[123].bLevel = 0; // SKILL_CREATE

		if (tab.level > 9)
		{
			int max_point = tab.level - 9;

			int skill_point =
				std::min<int>(20, tab.skills[121].bLevel) +	// SKILL_LEADERSHIP			Åë¼Ö·Â
				std::min<int>(20, tab.skills[124].bLevel) +	// SKILL_MINING				Ã¤±¤
				std::min<int>(10, tab.skills[131].bLevel) +	// SKILL_HORSE_SUMMON		¸»¼ÒÈ¯
				std::min<int>(20, tab.skills[141].bLevel) +	// SKILL_ADD_HP				HPº¸°­
				std::min<int>(20, tab.skills[142].bLevel);		// SKILL_RESIST_PENETRATE	°üÅëÀúÇ×

			tab.sub_skill_point = max_point - skill_point;
		}
		else
			tab.sub_skill_point = 0;
	}

	return true;
}


bool CreateItemSaveQuery(const TPlayerItem& item, std::string& query)
{
	using namespace fmt::literals;

	query = fmt::format(
		"REPLACE INTO item (id, owner_id, `window`, pos, count, vnum, trans_vnum, is_gm_owner, is_blocked, seal_date, price, socket0, socket1, socket2, socket3, socket4, socket5, "
		"attrtype0, attrvalue0,"
		"attrtype1, attrvalue1,"
		"attrtype2, attrvalue2,"
		"attrtype3, attrvalue3,"
		"attrtype4, attrvalue4,"
		"attrtype5, attrvalue5,"
		"attrtype6, attrvalue6) VALUES({id}, {owner_id}, {window}, {pos}, {count}, {vnum}, {trans_vnum}, {is_gm_owner}, {is_blocked}, {seal_date}, {price}, {socket0}, {socket1}, {socket2}, {socket3}, {socket4}, {socket5}, {attrtype0}, {attrvalue0}, {attrtype1}, {attrvalue1}, {attrtype2}, {attrvalue2}, {attrtype3}, {attrvalue3}, {attrtype4}, {attrvalue4}, {attrtype5}, {attrvalue5}, {attrtype6}, {attrvalue6})",
		"id"_a = item.id,
		"owner_id"_a = item.owner,
		"window"_a = item.window,
		"pos"_a = item.pos,
		"count"_a = item.data.count,
		"vnum"_a = item.data.vnum,
		"trans_vnum"_a = item.data.transVnum,
		"is_gm_owner"_a = (int)item.is_gm_owner,
		"is_blocked"_a = (int)item.is_blocked,
		"seal_date"_a = item.data.nSealDate,
		"price"_a = item.price,
		"socket0"_a = item.data.sockets[0],
		"socket1"_a = item.data.sockets[1],
		"socket2"_a = item.data.sockets[2],
		"socket3"_a = item.data.sockets[3],
		"socket4"_a = item.data.sockets[4],
		"socket5"_a = item.data.sockets[5],
		"attrtype0"_a = item.data.attrs[0].bType,
		"attrvalue0"_a = item.data.attrs[0].sValue,
		"attrtype1"_a = item.data.attrs[1].bType,
		"attrvalue1"_a = item.data.attrs[1].sValue,
		"attrtype2"_a = item.data.attrs[2].bType,
		"attrvalue2"_a = item.data.attrs[2].sValue,
		"attrtype3"_a = item.data.attrs[3].bType,
		"attrvalue3"_a = item.data.attrs[3].sValue,
		"attrtype4"_a = item.data.attrs[4].bType,
		"attrvalue4"_a = item.data.attrs[4].sValue,
		"attrtype5"_a = item.data.attrs[5].bType,
		"attrvalue5"_a = item.data.attrs[5].sValue,
		"attrtype6"_a = item.data.attrs[6].bType,
		"attrvalue6"_a = item.data.attrs[6].sValue);

	return true;
}

bool CreateItemTableFromRes(MYSQL_RES* res, uint32_t owner, std::vector<TPlayerItem>& items)
{
	
	if (!res) {
		items.clear();
		return true;
	}

	std::size_t rows = mysql_num_rows(res);
	if (rows == 0) { // 데이터 없음
		items.clear();
		return true;
	}

	items.resize(rows);

	for (size_t i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		auto& item = items[i];

		int cur = 0;
		// "SELECT id,window+0,pos,count,vnum,seal_date,socket0,socket1,socket2,attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6 "

		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		str_to_number(item.id, row[cur++]);
		str_to_number(item.window, row[cur++]);
		str_to_number(item.pos, row[cur++]);
		str_to_number(item.data.count, row[cur++]);
		str_to_number(item.data.vnum, row[cur++]);
		str_to_number(item.data.transVnum, row[cur++]);
		str_to_number(item.data.nSealDate, row[cur++]);
		str_to_number(item.is_gm_owner, row[cur++]);
		str_to_number(item.is_blocked, row[cur++]);
		str_to_number(item.price, row[cur++]);

		for(auto& socket : item.data.sockets) {
			storm::ParseNumber(row[cur++], socket);
		}

		for(auto& j : item.data.attrs) {
			str_to_number(j.bType, row[cur++]);
			str_to_number(j.sValue, row[cur++]);
		}

		item.owner = owner;
	}

	return true;

}


void CreateQuickslotLoadQuery(uint32_t pid, std::string& query)
{
	query += fmt::format("SELECT idx, type, pos FROM player_quickslot WHERE pid={}", pid);
}

void CreateQuickslotTableFromRes(MYSQL_RES* res,
	std::vector<QuickslotPair>& quickslots)
{
	const std::size_t rows = mysql_num_rows(res);
	quickslots.resize(rows);

	for (std::size_t i = 0; i != rows; ++i) {
		MYSQL_ROW row = mysql_fetch_row(res);
		int cur = 0;

		str_to_number(quickslots[i].first, row[cur++]);
		str_to_number(quickslots[i].second.type, row[cur++]);
		str_to_number(quickslots[i].second.pos, row[cur++]);
	}
}

void CreateQuickslotSaveQuery(uint32_t pid, uint32_t index,
	const TQuickslot& slot, std::string& query)
{
	query += fmt::format("INSERT INTO player_quickslot "
		"(pid, idx, type, pos) VALUES ({}, {}, {}, {})"
		"ON DUPLICATE KEY UPDATE type=VALUES(type), pos=VALUES(pos)",
		pid, index, slot.type, slot.pos);;
}

void CreateQuickslotDeleteQuery(uint32_t pid, uint32_t index,
	std::string& query)
{
	query += fmt::format("DELETE FROM player_quickslot WHERE pid={} AND idx={}",
		pid, index);
}

void CreateTitleLoadQuery(uint32_t pid, std::string& query)
{
	query += fmt::format("SELECT title, color "
		"FROM player_title WHERE pid={}", pid);
}

void CreateSkillLoadQuery(uint32_t pid, std::string& query)
{
	query += fmt::format("SELECT vnum, master_type, level, next_read, color "
		"FROM player_skill WHERE pid={}", pid);
}

void CreateSkillTableFromRes(MYSQL_RES* res, std::vector<SkillPair>& skills)
{
	const std::size_t rows = mysql_num_rows(res);
	skills.resize(rows);

	for (std::size_t i = 0; i != rows; ++i) {
		MYSQL_ROW row = mysql_fetch_row(res);
		int cur = 0;

		str_to_number(skills[i].first, row[cur++]);
		str_to_number(skills[i].second.bMasterType, row[cur++]);
		str_to_number(skills[i].second.bLevel, row[cur++]);
		str_to_number(skills[i].second.tNextRead, row[cur++]);
		str_to_number(skills[i].second.color, row[cur++]);
	}
}

void CreateTitleSaveQuery(uint32_t pid,
	const TPlayerTitle& title, std::string& query)
{
	query += fmt::format(
		"INSERT INTO `player_title` "
		"(pid, title, color) VALUES "
		"({}, '{}', {}) ON DUPLICATE KEY UPDATE "
		"title=VALUES(title), "
		"color=VALUES(color) ",
		pid, title.title, title.color);;
}

void CreateSkillSaveQuery(uint32_t pid, uint32_t vnum,
	const TPlayerSkill& skill, std::string& query)
{
	query += fmt::format("INSERT INTO player_skill "
		"(pid, vnum, master_type, level, next_read, color) VALUES "
		"({}, {}, {}, {}, {}, {}) ON DUPLICATE KEY UPDATE "
		"master_type=VALUES(master_type), "
		"level=VALUES(level), color=VALUES(color), "
		"next_read=VALUES(next_read)",
		pid, vnum, skill.bMasterType,
		skill.bLevel, skill.tNextRead, skill.color);
}


void CreateTitleDeleteQuery(uint32_t pid, std::string& query)
{
	query += fmt::format("DELETE FROM player_title WHERE pid={}", pid);
}


void CreateSkillDeleteQuery(uint32_t pid, uint32_t vnum, std::string& query)
{
	query += fmt::format("DELETE FROM player_skill WHERE pid={} AND vnum={}",pid, vnum);
}

