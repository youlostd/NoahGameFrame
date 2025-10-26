ShadowZoneLIB3 = {};

ShadowZoneLIB3.Settings = function()
	if (ShadowZoneLIB3.data == nil) then
		ShadowZoneLIB3.data = {
			--------------------------------------			
			-- Map settings (Coordinates, indexes)
			--------------------------------------						
			["inside_index"] = 332, -- Dungeon index
			
			["pos_1_floor"] = {16894, 22609}, -- Dungeon coordinations to 1. Floor
			["pos_2_floor"] = {1073, 459}, -- Dungeon coordinations to 2. Floor
			["pos_3_floor"] = {359, 143}, -- Dungeon coordinations to 3. Floor
			["pos_4_floor"] = {660, 116}, -- Dungeon coordinations to 4. Floor
			["pos_5_floor"] = {437, 399}, -- Dungeon coordinations to 5. Floor
			["pos_6_floor"] = {984, 186}, -- Dungeon coordinations to 6. Floor
			["pos_7_floor"] = {1109, 1171}, -- Dungeon coordinations to 7. Floor
			["pos_8_floor"] = {105, 1259}, -- Dungeon coordinations to 8. Floor
			["pos_9_floor"] = {500, 1327}, -- Dungeon coordinations to 9. Floor
			
			["outside_index"] = 320, -- Index of maps, where are players teleported from the dungeon (Cities)
			["outside_pos"] = {266, 300}, -- Coords of the maps where are players teleported from the dungeon (Cities)	
			
			--------------------------------------			
			-- Dungeon settings
			--------------------------------------			
			["minimumLevel"] = 150, -- Minimum level for enter the dungeon
			["minimumPartyMembers"] = 2, -- Minimum count of players (If its party)
			["maximumPartyMembers"] = 4, -- Max count of players (If its party)
			
			["regen_file"] = "data/dungeon/shadow_zone2/", -- Path where regens are stored
			
			["drop_chance_8f"] = 150, -- Path where regens are stored
			
			--["dead_count_solo"] = 3, -- Path where regens are stored
			--["dead_count_party"] = 6, -- Path where regens are stored

			--------------------------------------			
			-- Items (Ticket, quest items, potions, etc..)--
			--------------------------------------			
			["Item_ticket"] = 30844,
			--["Item_reward"] = 30854,
			
			--------------------------------------			
			-- Boss settings --
			--------------------------------------			
			--- Spawn Coordinates
			["portal_gate_pos"] = {{128, 201, 5}, {1060, 581, 5}, {473, 140, 7}, {661, 194, 5}, {984, 136, 1}, {1110, 1171, 5}, {105, 1146, 1}},  --- Spawn coordinates of Gate npc (9382),all floors, one by on except (5th floor, there is a timer)
			
			["first_boss_pos"] = {1062, 562},  --- Spawn coordinates of first boss (Shadow king - 4382)
			["second_boss_pos"] = {657, 174},  --- Spawn coordinates of second boss (Shadow reaper - 4383)
			["fourth_boss_pos"] = {984, 136},  --- Spawn coordinates of fourth boss (Shadow Azrael - 4385)
			["fifth_boss_pos"] = {1110, 1124},  --- Spawn coordinates of fifth boss (Queen of suffering - 4386)
			["sixth_boss_pos"] = {105, 1202},  --- Spawn coordinates of fifth boss (Queen of suffering - 4387)
			["random_boss_pos"] = {667, 1332},  --- Spawn coordinates of random boss before final boss is spawned (4382-4387)
			["final_boss_pos"] = {727, 1341, 7},  --- Spawn coordinates of final boss (Queen of suffering - 4388)
			
			
			["second_stone_pos"] = {{388, 122}, {429, 106}, {453, 143}, {427, 174}, {387, 164}},  --- Spawn coordinates of all stones in third floor			
			["third_stone_pos"] = {{366, 481}, {292, 484}, {365, 801}, {489, 676}, {512, 878}, {587, 878}, {606, 968}, {725, 906}, {869, 760}, {782, 672}},  --- Spawn coordinates of all stones in third floor
			["fourth_stone_pos"] = {984, 159},  --- Spawn coordinates of first boss (Shadow king - 4382)
			["sixth_stone_pos"] = {664, 1337},  --- Spawn coordinates of final stone (8488)
			
			["reaper_statue_pos"] = {695, 159, 7},  --- Spawn coordinates of Staute of reaper, 3 phases
			["pillar2_pos"] = {105, 1150, 1},  --- Spawn coordinates of pillar in 8th floor
			["chest_pos"] = {742, 1343, 7},  --- Spawn coordinates of final stone (8488)
			
			--- IDs
			["portal_gate"] = 29382,
			["second_stone"] = 28495,
			["third_stone"] = 28496,
			
			["bosses_3"] = {24382, 24383, 24384, 24385, 24386, 24387},  --- All bosses of the dungeon
			
			---States
			["REAPER_HP_1"] = 947483647,
			["REAPER_HP_2"] = 1047483647,
			["REAPER_HP_3"] = 1147483647,
			["REAPER_HP_4"] = 1247483647,

			["ABBADON_HP_NORMAL"] = 2847483647,  --- This HP will be set for 6th boss (4387) if Shadow amulet (30850) ISN'T active
			["ABBADON_HP_SMALL"] = 2847483647, --- This HP will be set for 6th boss (4387) if Shadow amulet (30850) IS  active
			--------------------------------------			
			-- Dungeon settings
			--------------------------------------			
			["FIRST_STONE_COUNT"] = 3, -- Count of stones needed to destroy in first floor (regen_1b.txt)
			["FIFTH_STONE_COUNT"] = 8, -- Count of stones needed to destroy in 7th floor (regen_7a.txt)
			
			["KILL_COUNT_2_FLOOR"] = 527, ---527 -- Count of monsters needed to kill in first floor (regen_2a.txt)
			["KILL_COUNT_4_FLOOR"] = 148, ---148 -- Count of monsters needed to kill in fourth floor (regen_4a.txt)
			["KILL_COUNT_9_FLOOR"] = 302, ---322 -- Count of monsters needed to kill in fourth floor (regen_9a.txt)
			
			--------------------------------------			
			-- Timers--
			--------------------------------------			
			["time_to_jump"] = 5, --- Time after players will be teleported to next floor
			
			["timer_first_floor"] = 300, --- 5 minutes --- Time to finish first floor (destroy 3x stone), otherwise they will be teleported out of the dungeon
			["timer_second_floor"] = 600,  ---  10 minutes --- Time to finish first floor (destroy 3x stone), otherwise they will be teleported out of the dungeon, otherwise they will be teleported out of the dungeon
			["timer_fourth_floor"] = 600,  ---  10 minutes --- Time to kill all waves of monsters, build a statue of Shadow reaper and kill Shadow reaper boss, otherwise they will be teleported out of the dungeon
			["timer_fifth_floor"] = 720, --- 12 minutes --- Time to find a stone and kill third boss (Shadow death), otherwise they will be teleported out of the dungeon
			["timer_sixth_floor"] = 420, --- 7 minutes --- Time to finish sixth floor (Desotry pillar and kill Azrael), otherwise they will be teleported out of the dungeon
			["timer_eight_floor"] = 600, --- 10 minutes --- Time to finish eight floor (Destroy seal, kill boss), otherwise they will be teleported out of the dungeon
			
			["time_to_spawn_6_boss"] = 10, --- Time to finish first floor (destroy 3x stone), otherwise they will be teleported out of the dungeon
			
			["time_to_kill_reaper"] = 180, ---- Time to finish first floor (destroy 3x stone), otherwise they will be teleported out of the dungeon
			
			["time_to_kill_final_boss"] = 300, --- Time to finish first floor (destroy 3x stone), otherwise they will be teleported out of the dungeon
			
			["time_to_destroy_finalstone"] = 240, 
			
			["time_to_final_exit"] = 30, -- After that time, players will be teleported out of dungeon
			
			--["dungeon_cooldown"] = 3600, --- Time until players can't enter dungeon afte they finish it
			};
	end
	
	return ShadowZoneLIB3.data;
end

ShadowZoneLIB3.isActive = function()
	local pMapIndex = pc.get_map_index(); local map_index = ShadowZoneLIB3.Settings()["inside_index"];
	
	return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
end

ShadowZoneLIB3.clearDungeon = function()
	if (pc.in_dungeon()) then
		d.kill_all();
		d.clear_regen();
		d.kill_all();
	end return false;
end

ShadowZoneLIB3.checkEnter = function()
	local settings = ShadowZoneLIB3.Settings();
	addimage(25, 10, "shadowzone_bg1.tga"); 
	say("[ENTER][ENTER]")
	say_title(c_mob_name(9376))
	--if pc.has_active_dungeon() then
	--	say("You still have an active dungeon.")
	--	say("Complete it or wait for it to expire.")
	--	return false;
	--end
		
	if party.is_party() then
		local pids = party_get_member_pids();
		local minLev, minLevCheck, itemNeed, itemNeedCheck = {}, true, {}, true;
		
		--if not party.is_map_member_flag_lt("exit_shadow_dungeon_time", get_global_time() - settings["dungeon_cooldown"] ) then
		--	say_reward("Some members still have to wait[ENTER]until they can join the dungeon again.")
		--	return false;
		--end
		
		if (not party.is_leader()) then
			say("If you want to enter Shadow tower,[ENTER]let me talk with the group leader first.")
			return false;
		end
		
		if (party.get_near_count() < settings["minimumPartyMembers"]) then
			say(string.format("If you want to enter the Shadow tower,[ENTER]there must be atleast %s players with you...", settings["minimumPartyMembers"]))
			return false;
		end
		
		if (party.get_near_count() > settings["maximumPartyMembers"]) then
			say(string.format("If you want to enter the Shadow tower,[ENTER]there must be less than %s players with you...", settings["maximumPartyMembers"]))
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
			q.end_other_pc_block();
		end
		
		if (not minLevCheck) then
			say(string.format("If you want to enter the Shadow tower,[ENTER]every each group member must be level %s.[ENTER][ENTER]The next following players don't have the necessary level:", settings["minimumLevel"]))
			for i, str in next, minLev, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		if (not itemNeedCheck) then
			say("If you wish to enter the Shadow tower,[ENTER]every each group memeber must have:")
			say_item(""..c_item_name(settings["Item_ticket"]).."", settings["Item_ticket"], "")
			say("The next following players don't have the required item:")
			for i, str in next, itemNeed, nil do
				say(string.format("- %s", str))
			end
			return false;
		end
		
		return true;
	else  --- if its solo
	
		--if ((get_global_time() - pc.getf("shadow_dungeon","exit_shadow_dungeon_time")) < settings["dungeon_cooldown"]) then
		
		--	local remaining_wait_time = (pc.getf("shadow_dungeon","exit_shadow_dungeon_time") - get_global_time() + settings["dungeon_cooldown"])
		--	say("You have to wait until you can enter the dungeon again.")
		--	say_reward("You can go there again in: "..get_time_remaining(remaining_wait_time)..'[ENTER]')
		--	return
		--end

		if (pc.get_level() < settings["minimumLevel"]) then
			say(string.format("The minimum level to enter the dungeon is %s.", settings["minimumLevel"]))
			return false;
		end
		
		if (pc.count_item(settings["Item_ticket"]) < 1) then
			say("If you want to enter the Shadow tower[ENTER]you must have:")
			say_item(""..c_item_name(settings["Item_ticket"]).."", settings["Item_ticket"], "")
			return false;
		end
	end
	
	return true;
end

ShadowZoneLIB3.CreateDungeon = function()
	local settings = ShadowZoneLIB3.Settings();
	
	if party.is_party() then
		local pids = party_get_member_pids();
		
		for i, pid in next, pids, nil do
			q.begin_other_pc_block(pid);
			q.end_other_pc_block();
		end
		d.setf("ShadowZone_start_f", 1);						
		return d.new_jump_party(settings["inside_index"], settings["pos_1_floor"][1], settings["pos_1_floor"][2]);
	else
		d.setf("ShadowZone_start_f", 1);						
		return d.new_jump(settings["inside_index"], settings["pos_1_floor"][1]*100, settings["pos_1_floor"][2]*100); 
	end
end


ShadowZoneLIB3.spawnSecondStone = function()
	local settings = ShadowZoneLIB3.Settings();
	local Stone = settings["second_stone"];
	local stone_pos = settings["second_stone_pos"];
	local n = number(1,5); 			

	table_shuffle(stone_pos);

	for i = 1, 5 do
		if (i != n) then
			d.set_unique("fake"..i, d.spawn_mob(Stone, stone_pos[i][1], stone_pos[i][2]))
		end
	end

	d.set_unique ("real", d.spawn_mob(Stone, stone_pos[n][1], stone_pos[n][2]))
end

ShadowZoneLIB3.spawnPortalGate = function()
	local settings = ShadowZoneLIB3.Settings();
	local Floor = d.getf("ShadowZone_floor")
	local PortalGate = settings["portal_gate"]
	local Position = settings["portal_gate_pos"]
	
	if Floor == 1 then
		d.spawn_mob_dir(PortalGate, Position[1][1], Position[1][2], Position[1][3]);
	elseif Floor == 2 then
		d.spawn_mob_dir(PortalGate, Position[2][1], Position[2][2], Position[2][3]);
	elseif Floor == 3 then
		d.spawn_mob_dir(PortalGate, Position[3][1], Position[3][2], Position[3][3]);
	elseif Floor == 4 then
		d.spawn_mob_dir(PortalGate, Position[4][1], Position[4][2], Position[4][3]);
	elseif Floor == 6 then
		d.spawn_mob_dir(PortalGate, Position[5][1], Position[5][2], Position[5][3]);
	elseif Floor == 7 then
		d.spawn_mob_dir(PortalGate, Position[6][1], Position[6][2], Position[6][3]);
	elseif Floor == 8 then
		d.spawn_mob_dir(PortalGate, Position[7][1], Position[7][2], Position[7][3]);
	end
end

ShadowZoneLIB3.spawnThirdStone = function()
	local settings = ShadowZoneLIB3.Settings();
	local Stone = settings["third_stone"];
	local stone_pos = settings["third_stone_pos"];
	local n = number(1,10); 			

	table_shuffle(stone_pos);

	d.set_unique ("real_stone", d.spawn_mob(Stone, stone_pos[n][1], stone_pos[n][2]))
end

ShadowZoneLIB3.spawnRandomBoss_3 = function()
	local settings = ShadowZoneLIB3.Settings();
	local Boss_3 = settings["bosses_3"];
	local boss_pos = settings["random_boss_pos"];
	local n = number(1,6); 			

	table_shuffle(Boss_3);

	d.set_unique ("RandomBoss_3", d.spawn_mob(Boss_3[n], boss_pos[1], boss_pos[2]))
end

ShadowZoneLIB3.setReward = function()
	local settings = ShadowZoneLIB3.Settings();
	
	if party.is_party() then
		for _, pid in pairs({party.get_member_pids()}) do
			q.begin_other_pc_block(pid);
			
			d.setf(string.format("player_%s_reward_state", pc.get_player_id()), 1);	
			
			q.end_other_pc_block();
		end
	else
		d.setf(string.format("player_%s_reward_state", pc.get_player_id()), 1);	
	end
end

ShadowZoneLIB3.noCoolTimer = function()
	if party.is_party() then
		for _, pid in pairs({party.get_member_pids()}) do
			q.begin_other_pc_block(pid);
			
			cmdchat(string.format("SetDungeonCoolTime %s %s", d.getf("ShadowZone_floor"), d.getf("ShadowZone_nocool_timer")));
			cmdchat("ClearDungeonCoolTime");
			
			q.end_other_pc_block();
		end
	
	else
	
		cmdchat(string.format("SetDungeonCoolTime %s %s", d.getf("ShadowZone_floor"), d.getf("ShadowZone_nocool_timer")));
		cmdchat("ClearDungeonCoolTime");
	end
end

ShadowZoneLIB3.clearCoolTimer = function()
	if party.is_party() then
		for _, pid in pairs({party.get_member_pids()}) do
			q.begin_other_pc_block(pid);
			
			cmdchat("ClearDungeonCoolTime");
			
			q.end_other_pc_block();
		end
	
	else
	
		cmdchat("ClearDungeonCoolTime");		
	end
end


ShadowZoneLIB3.setCoolTimer = function()
	local settings = ShadowZoneLIB3.Settings();
	local Floor = d.getf("ShadowZone_floor")
	local FinalStone = d.getf("ShadowZone_9f_StoneD")
	local FinalBoss = d.getf("ShadowZone_FinalBoss")
	
	if party.is_party() then
		for _, pid in pairs({party.get_member_pids()}) do
			q.begin_other_pc_block(pid);
			
			
			if Floor == 1 then
				d.setf("ShadowZone_floor2_timer", settings["timer_first_floor"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor1_timer")))
			
			elseif Floor == 2 then
				d.setf("ShadowZone_floor2_timer", settings["timer_second_floor"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor2_timer")))
			
			elseif Floor == 4 then
				d.setf("ShadowZone_floor4_timer", settings["timer_fourth_floor"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor4_timer")))
			
			elseif Floor == 5 then
				d.setf("ShadowZone_floor5_timer", settings["timer_fifth_floor"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor5_timer")))
			
			elseif Floor == 6 then
				d.setf("ShadowZone_floor6_timer", settings["timer_sixth_floor"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor6_timer")))
			
			elseif Floor == 8 then
				d.setf("ShadowZone_floor8_timer", settings["timer_eight_floor"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor8_timer")))
			end
			
			if FinalStone == 1 then
				d.setf("ShadowZone_floor9_timer1", settings["time_to_destroy_finalstone"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor9_timer1")))
			end
			
			if FinalBoss == 1 then
				d.setf("ShadowZone_floor9_timer2", settings["time_to_kill_final_boss"]);	
				cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor9_timer2")))
			end		
			
			q.end_other_pc_block();
		end
		
	else
	
		
		if Floor == 1 then
			d.setf("ShadowZone_floor1_timer", settings["timer_first_floor"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor1_timer")))
		
		elseif Floor == 2 then
			d.setf("ShadowZone_floor2_timer", settings["timer_second_floor"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor2_timer")))
		
		elseif Floor == 4 then
			d.setf("ShadowZone_floor4_timer", settings["timer_fourth_floor"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor4_timer")))
		
		elseif Floor == 5 then
			d.setf("ShadowZone_floor5_timer", settings["timer_fifth_floor"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor5_timer")))
		
		elseif Floor == 6 then
			d.setf("ShadowZone_floor6_timer", settings["timer_sixth_floor"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor6_timer")))
		
		elseif Floor == 8 then
			d.setf("ShadowZone_floor8_timer", settings["timer_eight_floor"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor8_timer")))
		end
		
		if FinalStone == 1 then
			d.setf("ShadowZone_floor9_timer1", settings["time_to_destroy_finalstone"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor9_timer1")))
		end
		
		if FinalBoss == 1 then
			d.setf("ShadowZone_floor9_timer2", settings["time_to_kill_final_boss"]);	
			cmdchat(string.format("SetDungeonCoolTime %s %s", Floor, d.getf("ShadowZone_floor9_timer2")))
		end
	end
end

