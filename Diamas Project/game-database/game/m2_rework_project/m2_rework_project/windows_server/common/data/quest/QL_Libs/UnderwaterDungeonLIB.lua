--[[ -- Init - UnderwaterDungeonLIB Class ]]

UnderwaterDungeonLIB = {};

UnderwaterDungeonLIB.ReturnData = function()
	if (UnderwaterDungeonLIB.data == nil) then
		UnderwaterDungeonLIB.data = {
			-- [[ Requirements ]]
			["minimumLevel"] = 150,
			
			["inside_index"] = 501,
			["outside_index"] = 75,
			
			["requireItem"] = {["vnum"] = 71173, ["count"] = 1},
			
			["onlyGroupDungeon"] = false,
			["MinimumMemebersParty"] = 2,
			
			-- [[ Dungeon Itself ]]
			["inSidePos"] = {["x"] = 133, ["y"] = 121},
			["outSidePos"] = {["x"] = 195, ["y"] = 154},
			
			["firstFloorMetinPos"] = {{158,134}, {158,134}, {158,134}, {158,134}, {158,134}, {158,134}},
			["clamPosCoords"] = {{663, 251}, {663, 251}, {663, 251}, {663, 251}, {663, 251}, {663, 251}, {663, 251}, {663, 251}, {663, 251}, {663, 251}},
			["sealPosCoords"] = {{300, 732}, {300, 710}, {300, 689}},
			
			["time_until_you_got_kicked"] = time_min_to_sec(120)
		};
	end
	
	return UnderwaterDungeonLIB.data;
end

UnderwaterDungeonLIB.isInDungeon = function()
	local pMapIndex = pc.get_map_index(); local map_index = UnderwaterDungeonLIB.ReturnData()["inside_index"];
	
	return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
end

UnderwaterDungeonLIB.ClearDungeon = function()
	if (pc.in_dungeon()) then
		d.clear_regen();
		d.kill_all();
	end return false;
end

UnderwaterDungeonLIB.CheckInfo = function()
	local data = UnderwaterDungeonLIB.ReturnData(); local dataMinLev = data["minimumLevel"]; local dataItem = data["requireItem"];
	
	if party.is_party() then
		local pids = party_get_member_pids();
		local minLev, minLevCheck, itemNeed, itemNeedCheck = {}, true, {}, true;
		
		if (not party.is_leader()) then
			say("Lass mich mit dem Gruppenführer sprechen!")
			return false;
		end
		
		if (party.get_near_count() < data["MinimumMemebersParty"]) then
			say(string.format("Du benötigst mindestens %d Spieler in der Gruppe.", data["MinimumMemebersParty"]))
			return false;
		end
		
		for index, pid in ipairs(pids) do
			q.begin_other_pc_block(pid);
				if (pc.get_level() < dataMinLev) then
					table.insert(minLev, pc.get_name());
					minLevCheck = false;
				end
				
				if (pc.count_item(dataItem["vnum"]) < dataItem["count"]) then
					table.insert(itemNeed, string.format("%s", pc.get_name()));
					itemNeedCheck = false;
				end
			q.end_other_pc_block();
		end
		
		if (not minLevCheck) then
			say(string.format("Das Level %d ist eine Mindestanforderung. Diese Spieler sind nicht Level 150:", dataMinLev))
			for i, str in next, minLev, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		if (not itemNeedCheck) then
			say(string.format("Der Gegenstand x%d - %s wird benötigt. Diese Spieler haben nicht die Gegenstände:", dataItem["count"], item_name(dataItem["vnum"])))
			for i, str in next, itemNeed, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		return true;
	end
	
	if (not data["onlyGroupDungeon"]) then
		if (pc.get_level() < dataMinLev) then
			say(string.format("Du musst mindestens %d sein.", dataMinLev))
			return false;
		end
		
		if (pc.count_item(dataItem["vnum"]) < dataItem["count"]) then
			say(string.format("Der Gegenstand x%d - %s wird benötigt.", dataItem["count"], item_name(dataItem["vnum"])))
			return false;
		end
	end
	
	return true;
end

UnderwaterDungeonLIB.EnterInDungeon = function()
	local data = UnderwaterDungeonLIB.ReturnData();
	local dataItem = data["requireItem"]; 
	local enter_coords = data["inSidePos"];
	
	if party.is_party() then
		local pids = party_get_member_pids();
		
		for i, pid in next, pids, nil do
			q.begin_other_pc_block(pid);
			pc.remove_item(dataItem["vnum"], dataItem["count"]);
			q.end_other_pc_block();
		end
		return d.new_jump_party(data["inside_index"], enter_coords["x"], enter_coords["y"]);
	end
	
	if (not data["onlyGroupDungeon"]) then
		pc.remove_item(dataItem["vnum"], dataItem["count"])
		return d.new_jump(data["inside_index"], enter_coords["x"], enter_coords["y"])
	end
end