quest infected_garden begin
	state start begin
		------------------------------
		--QUEST FUNCTIONS AND SETTINGS
		------------------------------
		function settings()
			return
			{
				["infected_garden_index"] = 514, ---- Dungeon index
				["infected_garden_index_out"] = 75, ---- Map index, where players will be teleported from dungeon
				["out_pos"] = {478, 738}, ---- Coordinations, where players will be teleported from dungeon
				["start_pos"] = {190, 158}, ----- Dungeon coords
				["level_check"] = {
					["minimum"] = 150,
					["maximum"] = 170
				},
				["pass"] = 30724, --- Ticket
				["GroupType"] = false, ---- If its solo dungeon
				["Items"] = {30725, 30726, 30727, 30728, 30729, 30730}, 
				-- First floor
				["first_metin"] = 8431, 				
				["first_metin_pos"] = {190, 200},
				["second_metin"] = 8432, 				
				["second_metin_pos"] = {318, 501},
				["third_metin"] = 8433, 				
				["door"] = 9239, 				
				["door_pos"] = {
					[1] = {191, 225, 5},
					[2] = {192, 344, 5},
					[3] = {347, 504, 7},
					[4] = {465, 502, 7},
					[5] = {544, 282, 2}
				},
				["plant_npc"] = 9241, ----Botani
				["plant_obj00"] = 9242, 				
				["plant_obj00_pos"] = {
					[1] = {172, 187},
					[2] = {208, 137},
					[3] = {207, 219},
					[4] = {208, 166}
				},
				["first_boss"] = 469, 				
				["first_boss_pos"] = {190, 317},
				["second_boss"] = 470, 				
				["second_boss_pos"] = {209, 463},
				["third_boss"] = 471, 				
				["third_boss_pos"] = {395, 506},
				["pois_botani"] = 9244, 				
				["pois_botani_pos"] = {502, 495},
				["mushroom"] = 9245, 				
				["final_boss"] = 472, 				
				["final_boss_pos"] = {503, 233},
			};
		end
		
		--------PARTY AND ENTER
		function party_get_member_pids()
			local pids = {party.get_member_pids()}
			
			return pids
		end
		
		function is_infected()
			local pMapIndex = pc.get_map_index();
			local data = infected_garden.settings();
			local map_index = data["infected_garden_index"];

			return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
		end
		
		function clear_infectedgarden()
			d.clear_regen();
			d.kill_all();
		end
		
		function check_enter()
			addimage(25, 10, "infected_garden01.tga")
					say("")
					say("")
					say("")
			addimage(225, 150, "plant_npc.tga")
					say_title(c_mob_name(9241))
			local settings = infected_garden.settings()
			
			--if ((get_global_time() - pc.getf("infected_garden","exit_infected_garden_time")) < 60*60) then
			--
			--	local remaining_wait_time = (pc.getf("infected_garden","exit_infected_garden_time") - get_global_time() + 60*60)
			--	say(gameforge.infected_garden_quest._10_say)
			--	say_reward(string.format(gameforge.infected_garden_quest._20_sayReward, get_time_remaining(remaining_wait_time)..'[ENTER]'))
			--	return
			--end
			
			if party.is_party() then			
				if not party.is_leader() then
					say_reward(gameforge.infected_garden_quest._30_sayReward)
					return
				end

				if party.get_near_count() < 2 then
					say_reward(gameforge.infected_garden_quest._40_sayReward)
					return false;
				end
				
				local levelCheck = true
				local passCheck = true
				local notEnoughLevelMembers = {}
				local notEnoughLevelMaxMembers = {}
				local notEnoughTicketMembers = {}
				local pids = {party.get_member_pids()}

				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					if pc.get_level() < settings["level_check"]["minimum"] then
						table.insert(notEnoughLevelMembers, pc.get_name())
						levelCheck = false
					end

					q.end_other_pc_block()
				end

				if not levelCheck then
					say_reward(gameforge.infected_garden_quest._50_sayReward)
					for i, n in next, notEnoughLevelMembers, nil do
						say_title(string.format(gameforge.infected_garden_quest._60_sayTitle, n))
					end
					return
				end
				
				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					if pc.get_level() > settings["level_check"]["maximum"] then
						table.insert(notEnoughLevelMaxMembers, pc.get_name())
						levelCheck = false
					end

					q.end_other_pc_block()
				end

				if not levelCheck then
					say_reward(gameforge.infected_garden_quest._70_sayReward)
									say_reward(gameforge.infected_garden_quest._80_sayReward)
					for i, n in next, notEnoughLevelMaxMembers, nil do
						say_title(string.format(gameforge.infected_garden_quest._60_sayTitle, n))
					end
					return
				end
				
				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					if pc.count_item(settings.pass) < 1 then
						table.insert(notEnoughTicketMembers, pc.get_name())
						passCheck = false
					end

					q.end_other_pc_block()
				end

				if not passCheck then
					say_reward(gameforge.infected_garden_quest._90_sayReward)
					say_item(gameforge.infected_garden_quest._100_sayItem, settings.pass, "")
									say_reward(gameforge.infected_garden_quest._110_sayReward)
					for i, n in next, notEnoughTicketMembers, nil do
						say_title(string.format(gameforge.infected_garden_quest._60_sayTitle, n))
					end
					return
				end
			end
		
			if (not settings["GroupType"]) then
				if (pc.get_level() < settings["level_check"]["minimum"]) then
					say(string.format(gameforge.infected_garden_quest._120_say,  settings["level_check"]["minimum"]))
					return false;
				end
				
				if (pc.count_item(settings["pass"]) < 1) then
					say_reward(gameforge.infected_garden_quest._130_sayReward)
					say_item(gameforge.infected_garden_quest._100_sayItem, settings.pass, "")
					return false;
				end
			end
			
			return true;
		end
		
					
		function create_dungeon()
			local setting = infected_garden.settings()
			local plant00_objPos = setting["plant_obj00_pos"]
			local randomNumber = math.random(1, table.getn(plant00_objPos))
			local pids = {party.get_member_pids()}
			
			if party.is_party() then
				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					pc.remove_item(setting["pass"], 1)
					q.end_other_pc_block()
				end
				d.new_jump_party(setting["infected_garden_index"], setting["start_pos"][1], setting["start_pos"][2])
				d.setf("infected_garden_level", 1)
				server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
				d.spawn_mob_dir(setting["door"], setting["door_pos"][1][1], setting["door_pos"][1][2], setting["door_pos"][1][3], 1)
				d.regen_file("data/dungeon/infected_garden/regen_1thfloor_a.txt")
				d.spawn_mob_dir(setting["door"], setting["door_pos"][1][1], setting["door_pos"][1][2], setting["door_pos"][1][3])
				d.spawn_mob(setting["plant_obj00"], plant00_objPos[randomNumber][1], plant00_objPos[randomNumber][2]);
			end
			if (not setting["GroupType"]) then
				pc.remove_item(setting["pass"], 1)
				d.new_jump(setting["infected_garden_index"], setting["start_pos"][1]*100, setting["start_pos"][2]*100)
				d.setf("infected_garden_level", 1)
				server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
				d.regen_file("data/dungeon/infected_garden/regen_1thfloor_a.txt")
				d.spawn_mob_dir(setting["door"], setting["door_pos"][1][1], setting["door_pos"][1][2], setting["door_pos"][1][3])
				d.spawn_mob(setting["plant_obj00"], plant00_objPos[randomNumber][1], plant00_objPos[randomNumber][2]);
			end
			
			d.setf("start_time", get_global_time())
		end
			
		--FUNCTIONS END
		
		--LOGIN IN MAP
		when login begin
			local settings = infected_garden.settings()
						
			if pc.get_map_index() == settings["infected_garden_index"] then
				if not pc.in_dungeon() then
					warp_to_village()
				end
			end
			if d.getf("infected_garden_level") == 1 then
				addimage(25, 10, "infected_garden02.tga")
						say("")
						say("")
						say("")
				say_title(gameforge.infected_garden_quest._140_sayTitle)
				say(gameforge.infected_garden_quest._150_say)
				say_item(gameforge.infected_garden_quest._160_sayItem, settings.Items[1], "")
				say(gameforge.infected_garden_quest._170_say)
			end
		end

		--DUNGEON ENTER
		when 9241.chat.gameforge.infected_garden_quest._180_npcChat  with not infected_garden.is_infected() begin
			local settings = infected_garden.settings()
			addimage(25, 10, "infected_garden01.tga")
			say("")
			say("")
			say("")
			addimage(225, 150, "plant_npc.tga")
			say_title(c_mob_name(9241))
			say(gameforge.infected_garden_quest._190_say)
			wait()
			addimage(25, 10, "infected_garden01.tga")
			say("")
			say("")
			say("")
			addimage(225, 150, "plant_npc.tga")
			say_title(c_mob_name(9241))
			say(gameforge.infected_garden_quest._200_say)
			say_title(gameforge.infected_garden_quest._210_sayTitle)
			if select(gameforge.infected_garden_quest._220_select, gameforge.infected_garden_quest._230_select) == 1 then
				if infected_garden.check_enter() then
					say(gameforge.infected_garden_quest._240_say)
					say_reward(gameforge.infected_garden_quest._250_sayReward)
					wait()
					infected_garden.create_dungeon()
				end
			end
		end
		
		when 9241.chat.gameforge.infected_garden_quest._260_npcChat with pc.is_gm() begin
			addimage(25, 10, "infected_garden01.tga")
					say("")
		say("")
		say("")
			if  select(gameforge.infected_garden_quest._270_select, gameforge.infected_garden_quest._280_select) == 2 then return end
				addimage(25, 10, "infected_garden01.tga")
						say("")
		say("")
		say("")
								say_title(c_mob_name(9241))
				say(gameforge.infected_garden_quest._290_say)
				pc.setf('infected_garden','exit_infected_garden_time', 0)
				
				-- Dungeon Info
				pc.setqf("rejoin_time", get_time() - 3600)
		end		
		
		------------ 1. FLOOR		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 8431 and d.getf("infected_garden_level") == 3 begin
			local settings = infected_garden.settings()
			game.drop_item(settings["Items"][1], 1)
			d.setf("infected_garden_level", 4)
		end
		
		when 9242.take with item.get_vnum() == 30725 and infected_garden.is_infected() and d.getf("infected_garden_level") == 4 begin
			local settings = infected_garden.settings()
			npc.kill()
			pc.remove_item(settings["Items"][1], 1)
			game.drop_item(settings["Items"][2], 1)
			d.notice("Infected garden: Now open first door")
		end
		
		-----DOOR OPENING
		when 9239.take with item.get_vnum() == 30726 and infected_garden.is_infected() begin
			local settings = infected_garden.settings()
			pc.remove_item(settings["Items"][2], 1)
			npc.kill()
			infected_garden.clear_infectedgarden()
			if d.getf("infected_garden_level") == 4 then
				d.setf("infected_garden_level", 5)
				d.setf("infected_garden_metin1", 1)
				d.spawn_mob_dir(settings["door"], settings["door_pos"][2][1], settings["door_pos"][2][2], settings["door_pos"][2][3], 1)
				d.regen_file("data/dungeon/infected_garden/regen_2thfloor_b.txt")
				--d.set_regen_file("data/dungeon/infected_garden/regen_2thfloor_a.txt")
				d.notice("Infected garden: Great! First floor is done!")
				d.notice("Infected garden: In this floor destroy all metinstones")
			elseif d.getf("infected_garden_level") == 8 then
				d.setf("infected_garden_level", 9)
				addimage(25, 10, "infected_garden03.tga")
				say("")
				say("")
				say("")
				say_title(gameforge.infected_garden_quest._320_sayTitle)
				say(gameforge.infected_garden_quest._330_say)
				wait()
				addimage(25, 10, "infected_garden03.tga")
				say("")
				say("")
				say("")
				say_title(gameforge.infected_garden_quest._320_sayTitle)
				say(gameforge.infected_garden_quest._340_say)
				say_item(gameforge.infected_garden_quest._350_sayItem, settings.Items[3], "")
				say(gameforge.infected_garden_quest._360_say)
				d.setf("fountains_cleaning", 1)
				d.spawn_mob_dir(settings["door"], settings["door_pos"][3][1], settings["door_pos"][3][2], settings["door_pos"][3][3], 1)
				server_timer("infected_garden_spawner", 1, d.get_map_index())
			elseif d.getf("infected_garden_level") == 15 then
				d.setf("infected_garden_level", 16)
				addimage(25, 10, "infected_garden04.tga")
				say("")
				say("")
				say("")
				say_title(gameforge.infected_garden_quest._370_sayTitle)
				say(gameforge.infected_garden_quest._380_say)
				say_reward(gameforge.infected_garden_quest._390_sayReward)
				d.spawn_mob_dir(settings["door"], settings["door_pos"][4][1], settings["door_pos"][4][2], settings["door_pos"][4][3], 1)
				server_timer("infected_garden_spawner", 1, d.get_map_index())
			elseif d.getf("infected_garden_level") == 21 then
				d.setf("infected_garden_level", 22)
				d.spawn_mob_dir(settings["door"], settings["door_pos"][5][1], settings["door_pos"][5][2], settings["door_pos"][5][3], 1)
				d.spawn_mob(settings["pois_botani"], settings["pois_botani_pos"][1], settings["pois_botani_pos"][2])
				server_timer("infected_garden_spawner", 1, d.get_map_index())
			elseif d.getf("infected_garden_level") == 26 then
				d.setf("infected_garden_level", 27)
				addimage(25, 10, "infected_garden01.tga")
				say("")
				say("")
				say("")
				say_title(gameforge.infected_garden_quest._400_sayTitle)
				say(gameforge.infected_garden_quest._410_say)
				wait()
				server_timer("infected_garden_spawner", 1, d.get_map_index())
			end
		end
		
		
		-------------2. FLOOR		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 8431 and d.getf("infected_garden_level") == 5 begin
			local kills = 5;
			d.setf("infected_garden_metin1", d.getf("infected_garden_metin1")+1);
			if (d.getf("infected_garden_metin1") < kills) then
				d.notice("Infected garden: %d stones has left!", kills-d.getf("infected_garden_metin1"))
			else
				d.notice("Infected garden: All stones are destroyed!")
				d.notice("Infected garden: Monsters are coming!!")
				d.notice("Infected garden: Kill them all!")
				d.setf("infected_garden_metin1", 0)
				d.setf("infected_garden_level", 6)
				server_timer("infected_garden_spawner", 5, d.get_map_index())
			end -- if/else
		end -- when

		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 469 and d.getf("infected_garden_level") == 7 begin
			local settings = infected_garden.settings()
			d.notice("Infected garden: Good work!")
			d.notice("Infected garden: Let's open next door!")
			d.setf("infected_garden_level", 8)
			game.drop_item(settings["Items"][2], 1)
		end
		
		--------------3. FLOOR		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 8432 begin
			local settings = infected_garden.settings()
			if d.getf("infected_garden_level") == 10 then
				d.notice("You've just found first Lymph stone!")
				game.drop_item(settings["Items"][3], 1)
				
			elseif d.getf("infected_garden_level") == 11 then
				local kills = 7;
				d.setf("infected_garden_metin2", d.getf("infected_garden_metin2")+1);
				if (d.getf("infected_garden_metin2") < kills) then
					d.notice("Infected garden: %d stones has left!", kills-d.getf("infected_garden_metin2"))
				else
					d.notice("Infected garden: You've destroyed all the stones!")
					d.notice("Infected garden: Let's destroy another fountain!")
					game.drop_item(settings["Items"][3], 1)
					d.setf("infected_garden_metin2", 0)
					d.setf("infected_garden_level", 12)
				end -- if/else
				
			elseif d.getf("infected_garden_level") == 18 then
				local kills = 5;
				d.setf("infected_garden_metin2_2", d.getf("infected_garden_metin2_2")+1);
				if (d.getf("infected_garden_metin2_2") < kills) then
					d.notice("Infected garden: %d stones has left!", kills-d.getf("infected_garden_metin2_2"))
				else
					d.notice("Infected garden: All stones are destroyed!")
					d.notice("Infected garden: Monsters are coming!!")
					d.setf("infected_garden_metin2_2", 0)
					d.setf("infected_garden_level", 19)
					server_timer("infected_garden_spawner", 5, d.get_map_index())
				end -- if/else
			end
		end
		
		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 8433 begin
			local settings = infected_garden.settings()
			if d.getf("infected_garden_level") == 13 then
				d.notice("Infected garden: You're doing amazing!")
				d.notice("Infected garden: Let's open the last fountain")
				d.setf("infected_garden_level", 14)
				game.drop_item(settings["Items"][3], 1)
			elseif d.getf("infected_garden_level") == 28 then
				game.drop_item(settings["Items"][6])
			end
		end
		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 469 and d.getf("infected_garden_level") == 14 begin
			local settings = infected_garden.settings()
			if d.getf("firstboss_secondround") == 1 then
				d.notice("Infected garden: Excellent work!")
				d.notice("Infected garden: One more bastard!")
				d.setf("firstboss_secondround", 2)
			elseif d.getf("firstboss_secondround") == 2 then
				d.notice("Infected garden: Perfect! You've got next Lymph stone!")
				d.setf("infected_garden_level", 15)
				game.drop_item(settings["Items"][3], 1)
			end
		end
		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 470 and d.getf("infected_garden_level") == 15 begin
			local settings = infected_garden.settings()
			d.notice("Infected garden: You're doing amazing!")
			d.notice("Infected garden: Let's open the last fountain")
			d.setf("infected_garden_level", 15)
			game.drop_item(settings["Items"][3], 1)
		end

		when 9243.take with item.get_vnum() == 30727 and infected_garden.is_infected() begin
			local settings = infected_garden.settings()
			pc.remove_item(settings["Items"][3], 1)
			npc.kill()
			if d.getf("fountains_cleaning") == 1 then
				d.setf("infected_garden_level", 11)
				d.setf("infected_garden_metin2", 1)
				d.setf("fountains_cleaning", 2)
				d.notice("Infected garden: 4 more fountains has left!")
				d.notice("Infected garden: Next lymph stone is inside metin stone")
				d.notice("Infected garden: In the last one")
				d.regen_file("data/dungeon/infected_garden/regen_3thfloor_c.txt")
				
			elseif d.getf("fountains_cleaning") == 2 then
				d.setf("fountains_cleaning", 3)
				d.notice("Infected garden: 3 more fountains has left!")
				d.notice("Infected garden: 2 more fountains has left!")
				d.notice("Infected garden: The monsters starting to be really annoying!")
				d.notice("Infected garden: More and more of them are coming..")
				d.notice("Infected garden: Kill them all!")
				server_timer("infected_garden_spawner", 3, d.get_map_index())
				
			elseif d.getf("fountains_cleaning") == 3 then
				d.setf("fountains_cleaning", 4)
				d.notice("Infected garden: 2 more fountains has left!")
				d.notice("Infected garden: 2 big plants just arrived!")
				d.notice("Infected garden: It looks like this floor has no end..")
				d.notice("Infected garden: Kill them both!")
				server_timer("infected_garden_spawner", 3, d.get_map_index())
				
			elseif d.getf("fountains_cleaning") == 4 then
				d.setf("fountains_cleaning", 5)
				d.notice("Infected garden: Just one last fountain left!")
				d.notice("Infected garden: Infected big plant is the last opponent!")
				d.notice("Infected garden: Massacre it!")
				server_timer("infected_garden_spawner", 3, d.get_map_index())
				
			elseif d.getf("fountains_cleaning") == 5 then
				d.setf("fountains_cleaning", 0)
				game.drop_item(settings["Items"][2], 1)
			end
		end
		
		-----------------------------4. FLOOR
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 8431 and d.getf("infected_garden_level") == 16 begin
			local kills = 5;
			d.setf("infected_garden_metin1_2", d.getf("infected_garden_metin1_2")+1);
			if (d.getf("infected_garden_metin1_2") < kills) then
				d.notice("Infected garden: %d stones has left!", kills-d.getf("infected_garden_metin1_2"))
			else
				d.notice("Infected garden: All stones are destroyed!")
				d.notice("Infected garden: Monsters are coming!!")
				d.setf("infected_garden_metin1_2", 0)
				d.setf("infected_garden_level", 17)
				server_timer("infected_garden_spawner", 5, d.get_map_index())
			end -- if/else
		end
		
		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 471 and d.getf("infected_garden_level") == 20 begin
			local settings = infected_garden.settings()
			d.notice("Infected garden: Good job!")
			d.notice("Infected garden: Let's open another door of death.")
			d.notice("Infected garden: Hahaha....")
			d.setf("infected_garden_level", 21)
			game.drop_item(settings["Items"][2], 1)
		end

		-----------------------------5. FLOOR
		when 9244.chat.gameforge.infected_garden_quest._560_npcChat with infected_garden.is_infected() and d.getf("infected_garden_level") == 22 begin
			local settings = infected_garden.settings()
			addimage(25, 10, "infected_garden05.tga")
			say("")
			say("")
			say("")
			say_title(gameforge.infected_garden_quest._570_sayTitle)
			say(gameforge.infected_garden_quest._580_say)
			wait()
			addimage(25, 10, "infected_garden05.tga")
			say("")
			say("")
			say("")
			say_title(gameforge.infected_garden_quest._570_sayTitle)
			say(gameforge.infected_garden_quest._590_say)
			say_item(gameforge.infected_garden_quest._600_sayItem, settings.Items[4], "")
			wait()
			d.setf("infected_garden_level", 23)
			d.regen_file("data/dungeon/infected_garden/regen_5thfloor_a.txt")
			server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
		end
		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 470 and d.getf("infected_garden_level") == 24 begin
			local settings = infected_garden.settings()
			if d.getf("secondboss_secondround") == 1 then
				d.setf("secondboss_secondround", 2)
				
			elseif d.getf("secondboss_secondround") == 2 then				
				d.notice("Infected garden: Look! Bottle of watter!")
				d.setf("infected_garden_level", 25)
				game.drop_item(settings["Items"][4], 1)
			end
		end
		
		when 9244.take with item.get_vnum() == 30728 and infected_garden.is_infected() and d.getf("infected_garden_level") == 25 begin
			local settings = infected_garden.settings()
			local MushroomPos = {{640, 372}, {592, 351}, {565, 319}, {643, 397}, {590, 440}, {584, 478}, {543, 480}, {527, 518}}
			local mushroom_count = table.getn(MushroomPos)
			local randomNumber = number(1, table.getn(MushroomPos))
			pc.remove_item(30728, 1)
			addimage(25, 10, "infected_garden06.tga")
			say("")
			say("")
			say("")
			say_title(gameforge.infected_garden_quest._570_sayTitle)
			say(gameforge.infected_garden_quest._620_say)
			wait()
			for index = 1, mushroom_count, 1 do
				local realMushroomVID = d.spawn_mob(settings["mushroom"], MushroomPos[index][1], MushroomPos[index][2])
				if index == randomNumber then
					d.set_unique("real_mushroom", realMushroomVID)
				end
			end
		end
			
		when 9245.click with infected_garden.is_infected() and d.getf("infected_garden_level") == 25 begin
			local settings = infected_garden.settings()
			if (npc.get_vid() == d.get_unique_vid("real_mushroom")) then
				game.drop_item(settings["Items"][5], 1)
				d.notice("Infected garden: You've found great mushroom!")
				d.notice("Infected garden: Bring it fast to Poisoned botani!")
				npc.purge()
				d.setf("infected_garden_level", 26)
			else
				d.notice("Infected garden: Any of this mushrooms are good enough.")
				npc.purge()
			end
		end
		
		when 9244.take with item.get_vnum() == 30729 and infected_garden.is_infected() and d.getf("infected_garden_level") == 26 begin
			pc.remove_item(30729)

			local settings = infected_garden.settings()
			addimage(25, 10, "infected_garden06.tga")
			say("")
			say("")
			say("")
			say_title(gameforge.infected_garden_quest._570_sayTitle)
			say(gameforge.infected_garden_quest._650_say)
			wait()
			npc.kill()

			d.notice("Infected garden: Oh no, the water was poisoned")
			d.notice("Infected garden: Botani died....")
			d.notice("Infected garden: Open next door, quickly!!")
			game.drop_item(settings["Items"][2])
		end
		
		------------------ 6. FLOOR
		when 9246.take with item.get_vnum() == 30730 and infected_garden.is_infected()  begin
			npc.kill()
			pc.remove_item(30730, 1)
			d.spawn_mob(481, 541, 252)
		end
		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 481 and d.getf("infected_garden_level") == 28 begin
			local kills = 3;
			d.setf("infectedg_bees_count", d.getf("infectedg_bees_count")+1)
			
			if (d.getf("infectedg_bees_count") >= kills) then
				d.notice("Infected garden: All hives and bees are dead!")
				d.notice("Infected garden: Now kill both %s 's", c_mob_name(471))
				d.setf("infected_garden_level", 29)
				d.setf("infectedg_bees_count", 0)
				server_timer("infected_garden_spawner", 5, d.get_map_index())
			end -- if
		end
		
		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 471 and d.getf("infected_garden_level") == 29 begin
			local settings = infected_garden.settings()
			if d.getf("infectedg_thridboss_final") == 1 then
				d.setf("infectedg_thridboss_final", 2)
				
			elseif d.getf("infectedg_thridboss_final") == 2 then				
				d.notice("Infected garden: You did it!!")
				d.notice("Infected garden: ------------------------")
				d.notice("Infected garden: Something is happening!")
				d.notice("Infected garden: The garden tyrant is coming!")
				d.setf("infected_garden_level", 30)
				server_timer("infected_garden_spawner", 5, d.get_map_index())
			end
		end

		when kill with infected_garden.is_infected() and not npc.is_pc() and npc.get_race() == 472 and d.getf("infected_garden_level") == 30 begin
			local settings = infected_garden.settings()
			d.notice("Infected garden: You succesfully finished the dungeon.")
			d.notice("Infected garden: Congratulation!")
			d.notice("Infected garden: ------------------------------------")
			d.notice("Infected garden: You will be teleported out of dungeon in 2 minutes.")
			notice_all(gameforge.infected_garden_quest._190_notice_all1 .. pc.get_name() .. gameforge.infected_garden_quest._190_notice_all2) --- GLOBAL NOTICE
			highscore.register("infected_time", get_global_time() - d.getf("start_time"), 0)

			d.setf("infected_garden_level", 31)
			server_timer("infected_graden_final_exit", 115, d.get_map_index())
		end

		-------------------TIMERS
		when infected_garden_wave_kill.server_timer begin
			local settings = infected_garden.settings()
			if d.select(get_server_timer_arg()) then
				if d.getf("infected_garden_level") == 1 then  -----------1. FLOOR -> 1. WAVE
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 2)
						d.notice("Infected garden: Another wave of monsters is coming!")
						server_timer("infected_garden_spawner", 3, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d creatures to move on.", d.count_monster())
					end
					
				elseif d.getf("infected_garden_level") == 2 then  -----------1. FLOOR -> 2. WAVE
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 3)
						d.notice("Infected garden: You've killed all monsters!")
						d.notice("Infected garden: Destroy the metin!")
						d.spawn_mob(settings["first_metin"], settings["first_metin_pos"][1], settings["first_metin_pos"][2])
					else
						d.notice("Infected garden: You still have to defeat %d creatures to move on.", d.count_monster())
					end
					
				elseif d.getf("infected_garden_level") == 6 then  -----------2. -> FLOOR
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 7)
						d.notice("Infected garden: You've killed all monsters!")
						d.notice("--------------------------------------------")
						d.notice("Infected garden: The ground is shaking!!")
						d.notice("Infected garden: Big plant is coming!")
						server_timer("infected_garden_spawner", 3, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d creatures to move on.", d.count_monster());
					end
					
				elseif d.getf("infected_garden_level") == 9 then  -----------3. FLOOR
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 10)
						d.notice("Infected garden: You've killed all monsters.")
						d.notice("Infected garden: Now destroy a metin stone!")
						server_timer("infected_garden_spawner", 1, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d creatures to move on.", d.count_monster());
					end
					
				elseif d.getf("infected_garden_level") == 12 then  -----------3. FLOOR -> 2. WAVE
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 13)
						d.notice("Infected garden: Destroy that wierd metin stone to get next stone!")
						server_timer("infected_garden_spawner", 3, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d creatures to move on.", d.count_monster());
					end
					
				elseif d.getf("infected_garden_level") == 17 then  -----------4. FLOOR -> 1. WAVE
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 18)
						d.notice("Infected garden: Another metin wave is coming!")
						server_timer("infected_garden_spawner", 3, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d monsters to move on.", d.count_monster());
					end
					
				elseif d.getf("infected_garden_level") == 19 then  -----------4. FLOOR -> 2. WAVE
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 20)
						d.notice("Infected garden: %s is coming! Kill it!!", c_mob_name(settings["thirdboss"]))
						server_timer("infected_garden_spawner", 3, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d monsters to move on.", d.count_monster());
					end
					
				elseif d.getf("infected_garden_level") == 23 then  -----------5. FLOOR 
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 24)
						d.notice("Infected garden: Oh no.. %s is coming! And not just one..", c_mob_name(settings["second_boss"]))
						d.notice("Infected garden: Kill them both!!")
						server_timer("infected_garden_spawner", 3, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d monsters to move on.", d.count_monster());
					end
					
				elseif d.getf("infected_garden_level") == 27 then  -----------6. FLOOR
					if d.count_monster() == 0 then
						clear_server_timer("infected_garden_wave_kill", get_server_timer_arg())
						d.setf("infected_garden_level", 28)
						d.notice("Infected garden: It's not going to be easy.")
						d.notice("Infected garden: Destroy all metinstones to get a special fire.")
						d.notice("Infected garden: You need to destroy all the bee hives!")
						server_timer("infected_garden_spawner", 3, d.get_map_index())
					else
						d.notice("Infected garden: You still have to defeat %d monsters to move on.", d.count_monster())
					end
				end
			end
		end

		when infected_garden_spawner.server_timer begin
			local settings = infected_garden.settings()
			if d.select(get_server_timer_arg()) then
				if d.getf("infected_garden_level") == 4 then
					d.regen_file("data/dungeon/infected_garden/regen_1thfloor_a.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 2 then
					d.regen_file("data/dungeon/infected_garden/regen_1thfloor_a.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 6 then
					d.regen_file("data/dungeon/infected_garden/regen_2thfloor_a.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 7 then
					d.spawn_mob(settings["first_boss"], settings["first_boss_pos"][1], settings["first_boss_pos"][2])
					
				elseif d.getf("infected_garden_level") == 9 then
					d.regen_file("data/dungeon/infected_garden/regen_3thfloor_a.txt")
					d.regen_file("data/dungeon/infected_garden/regen_3thfloor_b.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 10 then
					d.spawn_mob(settings["second_metin"], settings["second_metin_pos"][1], settings["second_metin_pos"][2])
					
				elseif d.getf("infected_garden_level") == 12 then
					d.regen_file("data/dungeon/infected_garden/regen_3thfloor_d.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 13 then
					d.spawn_mob(settings["third_metin"], settings["second_metin_pos"][1], settings["second_metin_pos"][2])
					
				elseif d.getf("infected_garden_level") == 14 then
					d.regen_file("data/dungeon/infected_garden/regen_3thfloor_e.txt")
					d.setf("firstboss_secondround", 1)
					
				elseif d.getf("infected_garden_level") == 15 then
					d.spawn_mob(settings["second_boss"], settings["second_boss_pos"][1], settings["second_boss_pos"][2])
					
				elseif d.getf("infected_garden_level") == 16 then
					d.regen_file("data/dungeon/infected_garden/regen_4thfloor_a.txt")
					d.setf("infected_garden_metin1_2", 1)
					
				elseif d.getf("infected_garden_level") == 17 then
					d.regen_file("data/dungeon/infected_garden/regen_4thfloor_b.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 18 then
					d.regen_file("data/dungeon/infected_garden/regen_4thfloor_c.txt")
					d.setf("infected_garden_metin2_2", 1)
					
				elseif d.getf("infected_garden_level") == 19 then
					d.regen_file("data/dungeon/infected_garden/regen_4thfloor_d.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 20 then
					d.spawn_mob(settings["third_boss"], settings["third_boss_pos"][1], settings["third_boss_pos"][2])
					
				elseif d.getf("infected_garden_level") == 24 then
					d.regen_file("data/dungeon/infected_garden/regen_4thfloor_f.txt")
					d.setf("secondboss_secondround", 1)
					
				elseif d.getf("infected_garden_level") == 27 then
					d.regen_file("data/dungeon/infected_garden/regen_6thfloor_a.txt")
					server_loop_timer("infected_garden_wave_kill", 5, d.get_map_index())
					
				elseif d.getf("infected_garden_level") == 28 then
					d.regen_file("data/dungeon/infected_garden/regen_6thfloor_b.txt")
					d.regen_file("data/dungeon/infected_garden/regen_6thfloor_c.txt")
					d.setf("infectedg_bees_count", 1)
					
				elseif d.getf("infected_garden_level") == 29 then
					d.regen_file("data/dungeon/infected_garden/regen_6thfloor_d.txt")
					d.setf("infectedg_thridboss_final", 1)
					
				elseif d.getf("infected_garden_level") == 30 then
					d.spawn_mob(settings["final_boss"], settings["final_boss_pos"][1], settings["final_boss_pos"][2])
				end
			end
		end

		when infected_graden_final_exit.server_timer begin
			local settings = infected_garden.settings()
			if d.select(get_server_timer_arg()) then
				d.notice("Infected garden: You will be teleported out of dungeon!")
				infected_garden.clear_infectedgarden()
				d.set_warp_location(settings["infected_garden_index_out"], settings["out_pos"][1], settings["out_pos"][2])
			end
			
			server_timer("infected_graden_final_exit_out", 5, d.get_map_index())
		end
		
		when infected_graden_final_exit_out.server_timer begin
			if d.select(get_server_timer_arg()) then								
				d.exit_all()
			end
		end

		when logout with infected_garden.is_infected() begin 
			pc.setf("infected_garden","exit_infected_garden_time", get_global_time())
			-- Dungeon Info
			pc.setqf("infected_garden", get_time() + 3600)
		end
	end
end
