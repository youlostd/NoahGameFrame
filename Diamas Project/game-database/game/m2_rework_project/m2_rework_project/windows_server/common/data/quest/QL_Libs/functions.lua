party_get_member_pids = function()
	local pids = {party.get_member_pids()};

	return pids;
end

get_time_format = function(sec)
	local sec = tonumber(sec);
	local final_str = "";
	local string_type = "";
	
	local epoch = {["hour"] = 1, ["day"] = 1, ["month"] = 1, ["year"] = 1970};
	local time_formats = {
		{["date"] = tonumber(os.date('%S', sec)),                ["plural"] = "s", ["singular"] = "s"},
		{["date"] = tonumber(os.date('%M', sec)),                ["plural"] = "m", ["singular"] = "m"},
		{["date"] = tonumber(os.date('%H', sec))-epoch["hour"],  ["plural"] = "h",   ["singular"] = "h"},
		{["date"] = tonumber(os.date('%d', sec))-epoch["day"],   ["plural"] = "d",    ["singular"] = "d"},
		{["date"] = tonumber(os.date('%m', sec))-epoch["month"], ["plural"] = "mon",  ["singular"] = "mon"},
		{["date"] = tonumber(os.date('%Y', sec))-epoch["year"],  ["plural"] = "year",   ["singular"] = "year"}
	};
	
	for strings, data in time_formats do
		if (data["date"] > 0) then
			if (strings > 1) then
				final_str = string.format(" %s", final_str);
			end
			if (data["date"] > 1) then
				string_type = data["plural"];
			else
				string_type = data["singular"];
			end
			final_str = string.format("%d %s%s", data["date"], string_type, final_str);
		end
	end
	
	return final_str;
end

pc_is_gm = function(arg)
	if pc.is_gm() then
		if (type(arg) == "table") then
			for index, value in ipairs(arg) do
				if (pc.get_name() == value) then return true; end
			end
			return false;
		end
		
		if (pc.get_name() == arg) then return true; end
	end
	return false;
end

isNextMultiply = function(numToRoundUp, tableNum, multiplyBy)
	local multiple = 0;
	
	for index = 1, tableNum do
		if (multiple * multiplyBy == numToRoundUp) then
			return true;
		end
		multiple = multiple + 1;
	end
	return false;
end

isQuestAvailable = function(isitem)
	if not pc.can_warp() then say("Trebuie sa astepti 10 secunde pentru a continua!") return false; end
	if isitem then if (pc.count_item(item.get_vnum()) == 0) then say("Nu ai obiectul in inventar!") return false; end end
	if pc.is_trade0() then syschat("Inchide fereastra de trade!") return false; end
	if pc.is_busy0() then syschat("Inchide celelalte ferestre!") return false; end
	if pc.get_empty_inventory_count() <= 25 then syschat("Ai inventarul plin. Arunca ceva din inventar.") return false; end
	
	return true;
end -- func

changeItems = function(itemChangeVnum, itemChangeCount, itemRecVnum, itemRecCount)
	local ReqItemCount = pc.count_item(itemChangeVnum);
	if (ReqItemCount >= itemChangeCount) then
		local FloorMod = math.floor(ReqItemCount);
		local RewItemCount = math.floor(FloorMod / itemChangeCount); local RemItemCount = math.floor(itemChangeCount * RewItemCount);
		
		return pc.give_item2(itemRecVnum, itemRecCount * RewItemCount), pc.remove_item(itemChangeVnum, RemItemCount);
	end
	return false;
end

TradeWindow = function(itemChangeVnum, itemChangeCount, itemRecVnum, itemRecCount)
	say_title(string.format("%s:[ENTER]", mob_name(npc.get_race())))
	say(string.format("Aici poti schimba x%d - %s pentru x%d - %s.[ENTER]Doresti sa faci acest lucru?", itemChangeCount, item_name(itemChangeVnum), itemRecCount, item_name(itemRecVnum)))
	if (select("Da", "Nu") == 1) then
		local ItemCheck = NewWayOfChangeItems.changeItems(itemChangeVnum, itemChangeCount, itemRecVnum, itemRecCount);
		if (not ItemCheck) then say(string.format("Mai ai nevoie de cel putin x%d - %s[ENTER]pentru a face negotul.", itemChangeCount-pc.count_item(itemChangeVnum), item_name(itemChangeVnum))) return false; end
	end
	return true;
end

luaSetF = function(strName, value)
	local playerId = pc.get_player_id();
	
	if (luaisNilF(strName)) then return luaUpdateF(strName, value); end
	return luaInsertF(strName, value);
end

luaInsertF = function(strName, value)
	local playerId = pc.get_player_id();
	return mysql_direct_query("INSERT INTO `luaRealLibrary` (pid, name, value) VALUES ("..playerId..", '"..strName.."', "..value..");");
end

luaUpdateF = function(strName, value)
	local playerId = pc.get_player_id();
	return mysql_direct_query("UPDATE player.luaRealLibrary SET value = "..value.." WHERE player.luaRealLibrary.pid = "..playerId.." AND player.luaRealLibrary.name = '"..strName.."';");
end

luaGetF = function(strName)
	local playerId = pc.get_player_id();
	local index, values = mysql_direct_query("SELECT player.luaRealLibrary.value as value FROM player.luaRealLibrary WHERE player.luaRealLibrary.pid = "..playerId.." AND player.luaRealLibrary.name = '"..strName.."';");
	
	if (index > 0) then
		return values[index]["value"];
	end return 0;
end

luaisNilF = function(strName)
	local playerId = pc.get_player_id();
	local index, values = mysql_direct_query("SELECT player.luaRealLibrary.value as value FROM player.luaRealLibrary WHERE player.luaRealLibrary.pid = "..playerId.." AND player.luaRealLibrary.name = '"..strName.."';");
	
	if (index > 0) then
		return true;
	end return false;
end

isTableType = function(arg)
	if (type(arg) == "table") then return true; end return false;
end

function returnCommaString(amount)
	local value = amount;
	while true do
		value, k = string.gsub(value, "^(-?%d+)(%d%d%d)", '%1.%2');
		if (k == 0) then break; end
	end return value;
end
