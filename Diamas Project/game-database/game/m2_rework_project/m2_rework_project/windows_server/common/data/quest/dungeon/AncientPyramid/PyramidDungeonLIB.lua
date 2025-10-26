PyramidDungeonLIB = {};

PyramidDungeonLIB.Settings = function()
	if (PyramidDungeonLIB.data == nil) then
		PyramidDungeonLIB.data = {
			-- [[ Requirements, Items, Map info ]]
			["minimumLevel"] = 150, -- Minimum level for enter the dungeon
			["minimumPartyMembers"] = 2, -- Minimum count of players (If its party)
			["inside_index"] = 230, -- Dungeon index
			
			["outside_index"] = 230, --Index of map you go to after the dungeon is completed (or you failed)
			["outside_pos"] = {91, 130}, -- Coordinates of map you go to after the dungeon is completed (or you failed)
			
			["pyramid_ticket"] = 30799,
			["Items_2floor"] = {30800, 30801, 30802, 30803}, -- Pergamens needed in 2 floor to destroy 9339
			["Item_5floor"] = 30804, -- Bundle of wisdom - needed item to destroy anubis head
			["Item_6floor"] = 30805, -- Bundle of wisdom - needed item to destroy anubis head
			["Item_reward"] = 30806, -- Bundle of wisdom - needed item to destroy anubis head
			
			["insidePos_1f"] = {91, 89}, -- First floor coordinates
			["insidePos_2f"] = {306, 93}, -- Second floor coordinates
			["insidePos_3f"] = {576, 80}, -- Third floor  coordinates
			["insidePos_4f"] = {96, 425}, -- Fourth floor  coordinates
			["insidePos_5f"] = {177, 1030}, -- Fifth floor  coordinates
			["insidePos_6f"] = {799, 1127}, -- Final floor  coordinates
			
			["3f_stone_pos"] = {{587, 108}, {564, 108}, {564, 128}}, --- Posotion of stones in third floor (pharaoh stones - 8475)
			
			["4f_boss_pos"] = {{1033, 137}, {1033, 169}, {1033, 211}}, --- Position of Ra bosses (4156) in the end of 4th floor
			
			["5f_npc_pos"] = {{166, 1044, 4}, {188, 1045, 6}, {188, 1016, 8}, {166, 1015, 2}}, --- Position of Anubis heads npcs in 5th floor
			
			--["dungeon_cooldown"] = 3600, -- Time after player can enter the dungeon again (1 hour - 3600 sec)
			["dungeon_timer"] = 3600, -- Time whithin players have to finish the whole dungeon (1 hour - 3600 sec)
			 
			["time_until_destroy_first_stone"] = 240, -- If the first stone is not destroyed in that time, dungeon is closed (in seconds - 4 minutes)
			
			["time_until_destroy_second_stone"] = 1440, -- If the real pharaoh stone isnt destroyed in this time, the dungoen is closed (in seconds - 24 minutes)
			
			["time_to_destroy_final_stone"] = 300, -- If you can destroy final stone in this time (8475), you will continue directly to boss (in seconds - 5 minutes)
			
			-------
			----Final boss settings
			-------
			--["time_until_pillar_is_spawned"] = 240, -- After this time (since boss is spawned), protecting pillar is spawned and you need to destroy it in defined time (in seconds - 4 minutes)
			--["time_until_pillar_is_killed"] = 180, -- If the pillar is not destroyed in this time, dungeon has failed (in seconds - 30 seconds)
			
			--["outside_index"] = {1, 21, 41}, -- Index of maps, where are players teleported from the dungeon (Cities)
			--["outsidePos"] = {4693, 9642, 557, 1579, 9694, 2786}, -- Coords of the maps where are players teleported from the dungeon (Cities)	
						
			
			
			};
	end
	
	return PyramidDungeonLIB.data;
end


PyramidDungeonLIB.clearTimers = function()
	clear_server_timer("PyramidDungeon_full_timer", get_server_timer_arg())
	--clear_server_timer("PyramidDungeon_6f_pillar_d", get_server_timer_arg())
	--clear_server_timer("PyramidDungeon_6th_PillarSpawn", get_server_timer_arg())
end

PyramidDungeonLIB.isActive = function()
	local pMapIndex = pc.get_map_index(); local map_index = PyramidDungeonLIB.Settings()["inside_index"];
	
	return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
end

PyramidDungeonLIB.clearDungeon = function()
	if (pc.in_dungeon()) then
		d.clear_regen();
		d.kill_all();
	end return false;
end

PyramidDungeonLIB.checkEnter = function()
	local settings = PyramidDungeonLIB.Settings();
	say_size(350,350)
	addimage(25, 10, "pyramid_dungeon_bg1.tga"); addimage(225, 200, "pyramid_guard.tga")
	say("[ENTER][ENTER]")
	say_title(c_mob_name(9338))
	--if pc.has_active_dungeon() then
	--	say("You still have an active dungeon.")
	--	say("Complete it or wait for it to expire.")
	--	return false;
	--end
	
	if party.is_party() then
		local pids = party_get_member_pids();
		local minLev, minLevCheck, itemNeed, itemNeedCheck = {}, true, {}, true;
		
		--if not party.is_map_member_flag_lt("exit_pyramid_dungeon_time", get_global_time() - settings["dungeon_cooldown"] ) then
		--	say_reward("Some members still have to wait[ENTER]until they can join the dungeon again.")
		--	return false;
		--end
		
		if (not party.is_leader()) then
			say("If you want to enter Ancient pyramid,[ENTER]let me talk with the group leader first.")
			return false;
		end
		
		if (party.get_near_count() < settings["minimumPartyMembers"]) then
			say(string.format("If you want to enter the Ancient pyramid,[ENTER]there must be atleast %s players with you...", settings["minimumPartyMembers"]))
			return false;
		end
		
		for index, pid in ipairs(pids) do
			q.begin_other_pc_block(pid);
				if (pc.get_level() < settings["minimumLevel"]) then
					table.insert(minLev, pc.get_name());
					minLevCheck = false;
				end
				
				if (pc.count_item(settings["pyramid_ticket"]) < 1) then
					table.insert(itemNeed, string.format("%s", pc.get_name()));
					itemNeedCheck = false;
				end
			q.end_other_pc_block();
		end
		
		if (not minLevCheck) then
			say(string.format("If you want to enter the Ancient pyramid,[ENTER]every each group member must be level %s.[ENTER][ENTER]The next following players don't have the necessary level:", settings["minimumLevel"]))
			for i, str in next, minLev, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		if (not itemNeedCheck) then
			say("If you wish to enter the Ancient pyramid,[ENTER]every each group memeber must have:")
			say_item(""..c_item_name(settings["pyramid_ticket"]).."", settings["pyramid_ticket"], "")
			say("The next following players don't have the required item:")
			for i, str in next, itemNeed, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		

		
		return true;
	else  --- if its solo
	
		--if ((get_global_time() - pc.getf("pyramid_dungeon","exit_pyramid_dungeon_time")) < settings["dungeon_cooldown"]) then
		
		--	local remaining_wait_time = (pc.getf("pyramid_dungeon","exit_pyramid_dungeon_time") - get_global_time() + settings["dungeon_cooldown"])
		--	say("You have to wait until you can enter the dungeon again.")
		--	say_reward("You can go there again in: "..get_time_remaining(remaining_wait_time)..'[ENTER]')
		--	return
		--end

		if (pc.get_level() < settings["minimumLevel"]) then
			say(string.format("The minimum level to enter the dungeon is %s.", settings["minimumLevel"]))
			return false;
		end
		
		if (pc.count_item(settings["pyramid_ticket"]) < 1) then
			say("If you want to enter the Ancient pyramid[ENTER]you must have:")
			say_item(""..c_item_name(settings["pyramid_ticket"]).."", settings["pyramid_ticket"], "")
			return false;
		end
	end
	
	return true;
end

PyramidDungeonLIB.CreateDungeon = function()
	local settings = PyramidDungeonLIB.Settings();
	
	if party.is_party() then
		local pids = party_get_member_pids();
		
		for i, pid in next, pids, nil do
			q.begin_other_pc_block(pid);
			pc.remove_item(settings["pyramid_ticket"], 1);
			q.end_other_pc_block();
		end
		return d.new_jump_party(settings["inside_index"], settings["insidePos_1f"][1], settings["insidePos_1f"][2]);
	else
		pc.remove_item(settings["pyramid_ticket"], 1);	
		return d.new_jump(settings["inside_index"], settings["insidePos_1f"][1]*100, settings["insidePos_1f"][2]*100); 
	end
end

PyramidDungeonLIB.setOutCoords = function()
	local settings = PyramidDungeonLIB.Settings();
	
	d.set_warp_location(settings["inside_index"][1], settings["insidePos_1f"][1], settings["insidePos_1f"][2]);
end