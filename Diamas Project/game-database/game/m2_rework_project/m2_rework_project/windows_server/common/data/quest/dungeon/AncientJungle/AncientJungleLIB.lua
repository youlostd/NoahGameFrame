AncientJungleLIB = {};

AncientJungleLIB.Settings = function()
	if (AncientJungleLIB.data == nil) then
		AncientJungleLIB.data = {
			----------
			--- Requirements, map info, coordinates,  etc...
			----------
			
			["minimumLevel"] = 70, -- Minimum level for enter the dungeon
			["minimumPartyMembers"] = 2, -- Minimum count of players (If its party, members need to be around)
			
			["inside_index"] = 147, -- Dungeon index
			["inside_1f_pos"] = {20600, 22789}, -- Dungeon coordinates to 1. floor
			["inside_2f_pos"] = {20970, 22768}, -- Dungeon coordinates to 2. floor
			["inside_3f_pos"] = {21487, 22803}, -- Dungeon coordinates to 3. floor
			
			["outside_index"] = {1, 21, 41}, -- Index of maps, where are players teleported from the dungeon (Cities)
			["outside_pos"] = {4693, 9642, 557, 1579, 9694, 2786}, -- Coords of the maps where are players teleported from the dungeon (Cities)	
			
			["Item_ticket"] = 30862,
			
			----------
			--- Dungeon settings
			----------

			["KILL_COUNT_1_FLOOR"] = 168, --168,  --- Count of monsters players have to kill to drop a key (First three waves)
			["KILL_COUNT_3_FLOOR"] = 131, --131,  --- Count of monsters players have to kill to drop a key (Last four waves)
			
			["second_floor_stone_pos"] = {{629, 214}, {683, 215}, {683, 267}, {630, 268}},  --- Count of monsters players have to kill to drop a key (First three waves)		
			["third_floor_brazier_pos"] = {{1120, 260, 6}, {1151, 260, 6}, {1151, 291, 6}, {1119, 291, 6}},  --- Count of monsters players have to kill to drop a key (First three waves)
			
			["fire_drop_chance"] = 85,  --- Chance to drop Ancient fire (30865) from the golems in second floor (100/85)
			
			["SecondBoss_HP1"] = 1500000,  --- HP of second boss if players get 1 negative point
			["SecondBoss_HP2"] = 1800000,  --- HP of second boss if players get 2 negative points
			["SecondBoss_HP3"] = 2100000,  --- HP of second boss if players get 3 negative points
			
			["FinalBoss_Def_Normal"] = 180,  --- Normal defense of the boss (Put the same value as is in db)
			["FinalBoss_Def_High"] = 300,  --- Defense value in the case that players didnt destroy the final stone in limited time
			----------
			--- Timers
			----------			
			["time_to_proceed"] = 10,  --- Time within boss or next waves of monsters are spawned in the dungeon
			
			["timer_second_floor"] = 720,  --- Time within players have to finish whole second floor (12 minutes by deafult)
			
			["timer_to_destroy_secstone"] = 180,  --- Time within players have to destroy second stone (8703), otherwise the finall boss is gonna have much higher defense
			
			["timer_to_exit_dungeon"] = 120,  --- Time within players have to destroy second stone (8703), otherwise the finall boss is gonna have much higher defense
			
			["dungeon_cooldown"] = 3600,  --- Time within players can't enter the dungeon after they finish it (or were teleported out of dungeon)
									
			};
	end
	
	return AncientJungleLIB.data;
end


AncientJungleLIB.isActive = function()
	local pMapIndex = pc.get_map_index(); local map_index = AncientJungleLIB.Settings()["inside_index"];
	
	return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
end

AncientJungleLIB.clearDungeon = function()
	if (pc.in_dungeon()) then
		d.kill_all();
		d.clear_regen();
		d.kill_all();
	end return false;
end

AncientJungleLIB.checkEnter = function()
	local settings = AncientJungleLIB.Settings();
	say_size(350,350)
	addimage(25, 10, "aj_bg1.tga"); addimage(225, 225, "aj_guard.tga")
	say("[ENTER][ENTER]")
	say_title(mob_name(9393))
	
	if party.is_party() then
		local pids = party_get_member_pids();
		local minLev, minLevCheck, itemNeed, itemNeedCheck, Timetable, TimeNeedCheck = {}, true, {}, true, {}, true;
		
		
		if (not party.is_leader()) then
			say("If you want to enter the dungeon,[ENTER]let me talk with the group leader first.")
			return false;
		end
		
		if (party.get_near_count() < settings["minimumPartyMembers"]) then
			say(string.format("If you want to enter the dungeon,[ENTER]there must be atleast %d players with you...", settings["minimumPartyMembers"]))
			return false;
		end
		
		for index, pid in ipairs(pids) do
			q.begin_other_pc_block(pid);
				if (pc.get_level() < settings["minimumLevel"]) then
					table.insert(minLev, pc.get_name());
					minLevCheck = false;
				end
				
				if (pc.count_item(settings["Item_ticket"]) < 1) then
					table.insert(itemNeed, string.format("%s", pc.get_name()));
					itemNeedCheck = false;
				end
				
				local aj_flag = game.get_event_flag("ancient_ju_cooldown_"..pid.."");
				local current_time = get_time();
				
				if (aj_flag > current_time) then
					table.insert(Timetable, string.format("%s - %s", pc.get_name(), get_time_format(aj_flag - current_time)));
					TimeNeedCheck = false;
				end
			q.end_other_pc_block();
		end
		
		if (not minLevCheck) then
			say(string.format("If you want to enter the dungeon,[ENTER]every each group member must be level %d.[ENTER][ENTER]The next following players don't have the necessary level:", settings["minimumLevel"]))
			for i, str in next, minLev, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		if (not itemNeedCheck) then
			say("If you wish to enter the dungeon,[ENTER]every each group memeber must have:")
			say_item(""..item_name(settings["Item_ticket"]).."", settings["Item_ticket"], "")
			say("The next following players don't have the required item:")
			for i, str in next, itemNeed, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		if (not TimeNeedCheck) then
			say(string.format("Some members of your group still has to wait:", get_time_format(settings["dungeon_cooldown"])))
			for i, str in next, Timetable, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		return true;
	else  --- if its solo
	
		--[[if ((get_global_time() - pc.getf("easter2021_dungeon","exit_easter2021_time")) < settings["dungeon_cooldown"]) then
		
			local remaining_wait_time = (pc.getf("easter2021_dungeon","exit_easter2021_time") - get_global_time() + settings["dungeon_cooldown"])
			say("You have to wait until you can enter the dungeon again.")
			say_reward("You can go there again in: "..get_time_remaining(remaining_wait_time)..'[ENTER]')
			return
		end]]--
		local ownPid = pc.get_player_id();
		local aj_flag = game.get_event_flag("ancient_ju_cooldown_"..ownPid.."");
		local current_time = get_time();
		
		if (aj_flag > current_time) then
			say(string.format("If you want to enter, you need to wait %s.", get_time_format(aj_flag - current_time)))
			return false;
		end

		if (pc.get_level() < settings["minimumLevel"]) then
			say(string.format("The minimum level to enter the dungeon is %d.", settings["minimumLevel"]))
			return false;
		end
		
		if (pc.count_item(settings["Item_ticket"]) < 1) then
			say("If you want to enter the dungeon[ENTER]you must have:")
			say_item(""..item_name(settings["Item_ticket"]).."", settings["Item_ticket"], "")
			return false;
		end
	end
	
	return true;
end

AncientJungleLIB.CreateDungeon = function()
	local settings = AncientJungleLIB.Settings();
	
	if party.is_party() then
		local pids = party_get_member_pids();
		
		for i, pid in next, pids, nil do
			q.begin_other_pc_block(pid);
			pc.remove_item(settings["Item_ticket"], 1);
			q.end_other_pc_block();
		end
		return d.new_jump_party(settings["inside_index"], settings["inside_1f_pos"][1], settings["inside_1f_pos"][2]);
	else
		pc.remove_item(settings["Item_ticket"], 1);	
		return d.new_jump(settings["inside_index"], settings["inside_1f_pos"][1]*100, settings["inside_1f_pos"][2]*100); 
	end
end

AncientJungleLIB.setOutCoords = function()
	local empire = pc.get_empire()
	local settings = AncientJungleLIB.Settings();
	
	if empire == 1 then
		d.set_warp_location(settings["outside_index"][1], settings["outside_pos"][1], settings["outside_pos"][2]);
	elseif empire == 2 then
		d.set_warp_location(settings["outside_index"][2], settings["outside_pos"][3], settings["outside_pos"][4]);
	elseif empire == 3 then
		d.set_warp_location(settings["outside_index"][3], settings["outside_pos"][5], settings["outside_pos"][6]);
	end
end

AncientJungleLIB.setReward = function()
	local settings = AncientJungleLIB.Settings();
	
	if party.is_party() then
		for _, pid in pairs({party.get_member_pids()}) do
			q.begin_other_pc_block(pid);
			
			d.setf(string.format("player_%d_reward_state", pc.get_player_id()), 1);	
			
			q.end_other_pc_block();
		end
	else
		d.setf(string.format("player_%d_reward_state", pc.get_player_id()), 1);	
	end
end

AncientJungleLIB.setWaitingTime = function()
	local settings = AncientJungleLIB.Settings(); 
	local ownPid = pc.get_player_id();
	
	if party.is_party() then
		if (type(pids) != "table") then
			return game.set_event_flag("ancient_ju_cooldown_"..ownPid.."", settings["dungeon_cooldown"] + get_time());
		end
		
		for index, pid in next, pids, nil do
			return game.set_event_flag("ancient_ju_cooldown_"..pid.."", settings["dungeon_cooldown"] + get_time());
		end
	end
	return game.set_event_flag("ancient_ju_cooldown_"..ownPid.."", settings["dungeon_cooldown"] + get_time());
end
