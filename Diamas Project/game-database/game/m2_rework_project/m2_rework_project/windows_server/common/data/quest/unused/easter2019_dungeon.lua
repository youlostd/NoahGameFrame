quest easter2019_dungeon begin
	state start begin
	------------------------------
	--QUEST FUNCTIONS AND SETTINGS
	------------------------------
	function settings()
		return
		{
			["easter2019_dungeon_index"] = 29, ---- Dungeon index
			["easter2019_dungeon_index_out"] = 75, ---- Map index, where players will be teleported out of dungeon
			["easter2019_out_pos"] = {23500, 17300}, ---- Coordinations, where players will be teleported out of dungeon
			["dungeon_floor_pos"] = {				----- Dungeon coords
				[1] = {39900, 55200},
				[2] = {399, 552} 
			},
			["level_check"] = {
				["minimum"] = 75,
				["maximum"] = 149
			},
			["pass"] = 30731, --- Ticket
			["GroupType"] = false, ---- If its solo dungeon
			["Items"] = {30732, 30733}, 
			["first_metin"] = 8435, 				
			["first_metin_pos"] = {390, 242},
			["second_metin"] = 8434, 				
			["second_metin_pos"] = {391, 449},
			["third_metin"] = 8436, 				
			["third_metin_pos"] = {407, 615},
			["door"] = 9248, 				
			["door_pos"] = {
				[1] = {383, 276, 5},
				[2] = {381, 637, 8}
			},
			["bridge_pos"] = {398, 481, 1},
			["bad_rabbit_pos"] = {406, 473, 5},
			["fence_pos"] = {398, 481, 5},
			["main_boss"] = 487, 				
			["main_boss_pos"] = {181, 672},

		};
	end
	
	--------PARTY AND ENTER
	function party_get_member_pids()
		local pids = {party.get_member_pids()}
		
		return pids
	end
	
	function is_active()
		local pMapIndex = pc.get_map_index();
		local data = easter2019_dungeon.settings();
		local map_index = data["easter2019_dungeon_index"];

		return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
	end
	
	function clear_easterdungeon()
		d.clear_regen();
		d.kill_all();
	end
	
	function check_enter()
		addimage(25, 10, "easter2019_1.tga")
		addimage(225, 150, "easter_rabbit.tga")
		say("")
		say("")
		say("")
		say_title(c_mob_name(30129))
		local settings = easter2019_dungeon.settings()
		
		if ((get_global_time() - pc.getf("easter2019_dungeon","exit_easter2019_dungeon_time")) < 1) then
		
			local remaining_wait_time = (pc.getf("easter2019_dungeon","exit_easter2019_dungeon_time") - get_global_time() + 1)
			say(gameforge.easter2019_dungeon._100_say1)
			say_reward(gameforge.easter2019_dungeon._100_say2..get_time_remaining(remaining_wait_time)..'[ENTER]')
			return
		end
		
		if party.is_party() then			
			if not party.is_leader() then
				say_reward(gameforge.easter2019_dungeon._100_say3)
				return
			end

			if party.get_near_count() < 2 then
				say_reward(gameforge.easter2019_dungeon._100_say4)
				say_reward(gameforge.easter2019_dungeon._100_say5)
				say_reward(gameforge.easter2019_dungeon._100_say6)
				return false;
			end
			
			local levelCheck = true
			local passCheck = true
			local MemberHaveLowLevel = {}
			local MemberHaveHighLevel = {}
			local MemberHaveNoTicket = {}
			local pids = {party.get_member_pids()}
			
			if not party.is_map_member_flag_lt("exit_easter2019_dungeon_time", get_global_time() - 60 * 60 ) then
				say_reward(gameforge.easter2019_dungeon._100_say7)
				say_reward(gameforge.easter2019_dungeon._100_say8)
				return false;
			end
						
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() < settings["level_check"]["minimum"] then
					table.insert(MemberHaveLowLevel, pc.get_name())
					levelCheck = false
				end

				q.end_other_pc_block()
			end

			if not levelCheck then
				say_reward(gameforge.easter2019_dungeon._100_say9)
				say_reward(gameforge.easter2019_dungeon._100_say10)
				say_reward("")
				say_reward(gameforge.easter2019_dungeon._100_say11)
				for i, n in next, MemberHaveLowLevel, nil do
					say_title("- "..n)
				end
				return
			end
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() > settings["level_check"]["maximum"] then
					table.insert(MemberHaveHighLevel, pc.get_name())
					levelCheck = false
				end

				q.end_other_pc_block()
			end

			if not levelCheck then
				say_reward(gameforge.easter2019_dungeon._100_say12)
				say_reward(gameforge.easter2019_dungeon._100_say13)
				say("")
				say_reward(gameforge.easter2019_dungeon._100_say14)
				for i, n in next, MemberHaveHighLevel, nil do
					say_title("- "..n)
				end
				return
			end
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.count_item(settings.pass) < 1 then
					table.insert(MemberHaveNoTicket, pc.get_name())
					passCheck = false
				end

				q.end_other_pc_block()
			end

			if not passCheck then
				say_reward(gameforge.easter2019_dungeon._100_say15)
				say_reward(gameforge.easter2019_dungeon._100_say16)
				say_item(gameforge.easter2019_dungeon._100_say17, settings.pass, "")
				say("")
				say_reward(gameforge.easter2019_dungeon._100_say18)
				for i, n in next, MemberHaveNoTicket, nil do
					say_title("- "..n)
				end
				return
			end
	
		else
		
			if ((get_global_time() - pc.getf("easter2019_dungeon","exit_easter2019_dungeon_time")) < 1) then
			
				local remaining_wait_time = (pc.getf("easter2019_dungeon","exit_easter2019_dungeon_time") - get_global_time() + 1)
				say(gameforge.easter2019_dungeon._100_say19)
				say_reward(gameforge.easter2019_dungeon._100_say20..get_time_remaining(remaining_wait_time)..'[ENTER]')
				return
			end
			
			if (pc.get_level() < settings["level_check"]["minimum"]) then
				say(string.format(gameforge.easter2019_dungeon._100_say21, settings["level_check"]["minimum"]))
				return false;
			end
			
			if (pc.get_level() > settings["level_check"]["maximum"]) then
				say(string.format(gameforge.easter2019_dungeon._100_say21_1, settings["level_check"]["maximum"]))
				return false;
			end
			
			if (pc.count_item(settings["pass"]) < 1) then
				say_reward(gameforge.easter2019_dungeon._100_say22)
				say_reward(gameforge.easter2019_dungeon._100_say23)
				say_item(gameforge.easter2019_dungeon._100_say24, settings.pass, "")
				return false;
			end
		end
		
		return true;
	end
	
	------------DUNGEON ENTER
	function create_dungeon()
		local setting = easter2019_dungeon.settings()
		local pids = {party.get_member_pids()}
		
		if party.is_party() then
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				pc.remove_item(setting["pass"], 1)
				q.end_other_pc_block()
			end
			d.new_jump_party(setting["easter2019_dungeon_index"], setting["dungeon_floor_pos"][1][1], setting["dungeon_floor_pos"][1][2])
			d.setf("easter2019_dungeon_level", 1)
			d.regen_file("data/dungeon/easter2019_dungeon/regen_1a.txt")
			d.spawn_mob_dir(setting["door"], setting["door_pos"][1][1], setting["door_pos"][1][2], setting["door_pos"][1][3])
			server_timer("easter2019_dungeon_30min_left", 15*60, d.get_map_index())
			server_loop_timer("easter2019_dungeon_wave_kill", 15, d.get_map_index())
		else
			pc.remove_item(setting["pass"], 1)
			d.new_jump(setting["easter2019_dungeon_index"], setting["dungeon_floor_pos"][1][1]*100, setting["dungeon_floor_pos"][1][2]*100)
			d.setf("easter2019_dungeon_level", 1)
			d.regen_file("data/dungeon/easter2019_dungeon/regen_1a.txt")
			d.spawn_mob_dir(setting["door"], setting["door_pos"][1][1], setting["door_pos"][1][2], setting["door_pos"][1][3])
			server_timer("easter2019_dungeon_30min_left", 15*60, d.get_map_index())
			server_loop_timer("easter2019_dungeon_wave_kill", 15, d.get_map_index())
		end
		d.setf("start_time", get_global_time())

	end
			
		--FUNCTIONS END
		
		--LOGIN IN MAP
		when login  begin
			local settings = easter2019_dungeon.settings()
						
			if pc.get_map_index() == settings["easter2019_dungeon_index"] then
				if not pc.in_dungeon() then
					if not pc.is_gm() then
						warp_to_village()
					end
				end
			end
			if d.getf("easter2019_dungeon_level") == 1 then
				addimage(25, 10, "easter2019_2.tga")
				say("")
				say("")
				say("")
				say_title(gameforge.easter2019_dungeon._100_say25)
				say("")
				say(gameforge.easter2019_dungeon._100_say26)
				say_item(gameforge.easter2019_dungeon._100_say27, settings.Items[1], "")
				say(gameforge.easter2019_dungeon._100_say28)
				say(gameforge.easter2019_dungeon._100_say29)
			end
		end

		--DUNGEON ENTER
		when 30129.chat.gameforge.easter2019_dungeon._100_say55  with not easter2019_dungeon.is_active() begin
			local settings = easter2019_dungeon.settings()
			addimage(25, 10, "easter2019_1.tga")
			addimage(225, 150, "easter_rabbit.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(30129))
			say("")
			say(gameforge.easter2019_dungeon._100_say30)
			say(gameforge.easter2019_dungeon._100_say31)
			say(gameforge.easter2019_dungeon._100_say32)
			say(gameforge.easter2019_dungeon._100_say33)
			say(gameforge.easter2019_dungeon._100_say34)
			say(gameforge.easter2019_dungeon._100_say35)
			say(gameforge.easter2019_dungeon._100_say36)
			say(gameforge.easter2019_dungeon._100_say37)
			wait()
			addimage(25, 10, "easter2019_1.tga")
			addimage(225, 150, "easter_rabbit.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(30129))
			say("")
			say(gameforge.easter2019_dungeon._100_say38)
			say(gameforge.easter2019_dungeon._100_say39)
			say(gameforge.easter2019_dungeon._100_say40)
			say(gameforge.easter2019_dungeon._100_say41)
			say(gameforge.easter2019_dungeon._100_say42)
			say(gameforge.easter2019_dungeon._100_say43)
			say("")
			say_title(gameforge.easter2019_dungeon._100_say44)
			if (select (gameforge.easter2019_dungeon._100_say45, gameforge.easter2019_dungeon._100_say46) == 1) then
				if easter2019_dungeon.check_enter() then
					say(gameforge.easter2019_dungeon._100_say47)
					say(gameforge.easter2019_dungeon._100_say48)
					say(gameforge.easter2019_dungeon._100_say49)
					say("")
					say_reward(gameforge.easter2019_dungeon._100_say50)
					say_reward(gameforge.easter2019_dungeon._100_say51)
					wait()
					easter2019_dungeon.create_dungeon()
				end
			end
		end
		
		when 30129.chat."Time reset" with pc.is_gm() begin
			addimage(25, 10, "easter2019_1.tga")
			say("")
			say("")
			say("")
			if select('Reset time','Close') == 2 then return end
				addimage(25, 10, "easter2019_1.tga")
				say("")
				say("")
				say("")
				say_title(c_mob_name(30129))
				say("")
				say(gameforge.easter2019_dungeon._100_say52)
				pc.setf('easter2019_dungeon','exit_easter2019_dungeon_time', 0)
				
				-- Dungeon Info
				--pc.setqf("rejoin_time", get_time() - 1)
		end		
		
		------------ 1. FLOOR		
		when kill with easter2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == 8435 and d.getf("easter2019_dungeon_level") == 2 begin
			local settings = easter2019_dungeon.settings()
			game.drop_item(settings["Items"][1], 1)
			d.setf("easter2019_dungeon_level", 3)
		end
		
		when 9248.take with item.get_vnum() == 30732 and easter2019_dungeon.is_active() begin
			local settings = easter2019_dungeon.settings()
			if d.getf("easter2019_dungeon_level") == 3 then
				npc.kill()
				pc.remove_item(settings["Items"][1], 1)
				d.setf("easter2019_dungeon_level", 4)
				d.notice("Easter dungeon: Now destroy all metin stones.")
				d.regen_file("data/dungeon/easter2019_dungeon/regen_2a.txt")
				d.spawn_mob_dir(9247, settings["bridge_pos"][1], settings["bridge_pos"][2], settings["bridge_pos"][3])
				d.spawn_mob_dir(9250, settings["bad_rabbit_pos"][1], settings["bad_rabbit_pos"][2], settings["bad_rabbit_pos"][3])
				d.spawn_mob_dir(9249, settings["fence_pos"][1], settings["fence_pos"][2], settings["fence_pos"][3])
			elseif d.getf("easter2019_dungeon_level") == 12 then
				npc.kill()
				pc.remove_item(settings["Items"][1], 1)
				d.notice("Easter dungeon: Defeat Evil easter rabbit!")
				d.spawn_mob(settings["main_boss"], settings["main_boss_pos"][1], settings["main_boss_pos"][2])
				d.regen_file("data/dungeon/easter2019_dungeon/regen_4a.txt")
				d.setf("easter2019_dungeon_level", 13)
			end
		end
		
		---------------2. FLOOR
		when kill with easter2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == 8435 and d.getf("easter2019_dungeon_level") == 4 begin
			local kills = 6;
			d.setf("ed2019_egg1", d.getf("ed2019_egg1")+1);
			if (d.getf("ed2019_egg1") < kills) then
				d.notice(string.format("Easter dungeon: %d eggs has left!", kills-d.getf("ed2019_egg1")))
			else
				d.notice("Easter dungeon: Well done! Now kill all monsters!")
				d.setf("easter2019_dungeon_level", 5)
				server_timer("easter2019_dungeon_spawner", 5, d.get_map_index())
			end 
		end 
		
		when kill with easter2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == 8434 and d.getf("easter2019_dungeon_level") == 6 begin
			local settings = easter2019_dungeon.settings()
			game.drop_item(settings["Items"][2], 1)
			d.notice("Easter dungeon: Give the coin to rabbit!")
			d.setf("easter2019_dungeon_level", 7)
		end
		
		when 9250.take with item.get_vnum() == 30733 and easter2019_dungeon.is_active() and d.getf("easter2019_dungeon_level") == 7 begin
			local settings = easter2019_dungeon.settings()
			d.kill_all()
			pc.remove_item(settings["Items"][2], 1)
			d.setf("easter2019_dungeon_level", 8)
			d.notice("Easter dungeon: He has cheated on you!")
			d.notice("Easter dungeon: Be careful! Monsters are coming!")
			server_timer("easter2019_dungeon_spawner", 5, d.get_map_index())
		end
		---------------------3. FLOOR
		when kill with easter2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == 8434 and d.getf("easter2019_dungeon_level") == 9 begin
			local kills = 4;
			d.setf("ed2019_metin1", d.getf("ed2019_metin1")+1);
			if (d.getf("ed2019_metin1") < kills) then
				d.notice(string.format("Easter dungeon: %d metin stones has left!", kills-d.getf("ed2019_metin1")))
			else
				d.notice("Easter dungeon: Well done! Now kill all monsters!")
				d.setf("easter2019_dungeon_level", 10)
				server_timer("easter2019_dungeon_spawner", 5, d.get_map_index())
			end 
		end 
		
		when kill with easter2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == 8436 and d.getf("easter2019_dungeon_level") == 11 begin
			local settings = easter2019_dungeon.settings()
			game.drop_item(settings["Items"][1], 1)
			d.setf("easter2019_dungeon_level", 12)
		end
		
		---------------------4. FLOOR
		when kill with easter2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == 487 and d.getf("easter2019_dungeon_level") == 13 begin
			local settings = easter2019_dungeon.settings()
			d.kill_all()
			d.setf("easter2019_dungeon_level", 14)
			d.notice("Easter dungeon: You've succefuly finished the dungeon!")
			d.notice("Easter dungeon: You will be teleported in 2 minutes.")
			notice_all(gameforge.easter2019_dungeon._100_say53 .. pc.get_name() .. gameforge.easter2019_dungeon._100_say54) --- GLOBAL NOTICE
			if party.is_party() then
				highscore.register("eev_pt_time", get_global_time() - d.getf("start_time"), 0)
			else
				highscore.register("eev_time", get_global_time() - d.getf("start_time"), 0)
			end
			server_timer("easter2019_dung_final_exit", 115, d.get_map_index())
		end
				
		-------------------TIMERS
		when easter2019_dungeon_wave_kill.server_timer begin
			local settings = easter2019_dungeon.settings()
			if d.select(get_server_timer_arg()) then
				if d.getf("easter2019_dungeon_level") == 1 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_level", 2)
						d.notice("Easter dungeon: Well done! Now destroy an easter egg!")
						server_timer("easter2019_dungeon_spawner", 3, d.get_map_index())
					else
						d.notice(string.format("Easter dungeon: You still have to defeat %d monsters to move on.", d.count_monster()));
					end
				elseif d.getf("easter2019_dungeon_level") == 5 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_level", 6)
						d.notice("Easter dungeon: You've succesfuly killed all monsters.")
						d.notice("Easter dungeon: Destroy a metin stone to move on!")
						server_timer("easter2019_dungeon_spawner", 3, d.get_map_index())
					else
						d.notice(string.format("Easter dungeon: You still have to defeat %d monsters to move on.", d.count_monster()));
					end
				elseif d.getf("easter2019_dungeon_level") == 8 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_level", 9)
						d.notice("Easter dungeon: You've succesfuly killed all monsters.")
						d.notice("Easter dungeon: You will jump to next level!")
						server_timer("easter2019_dungeon_spawner", 5, d.get_map_index())
					else
						d.notice(string.format("Easter dungeon: You still have to defeat %d monsters to move on.", d.count_monster()));
					end
				elseif d.getf("easter2019_dungeon_level") == 10 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_level", 11)
						d.notice("Easter dungeon: You've succesfuly killed all monsters.")
						d.notice("Easter dungeon: Destroy evil egg to get a key.")
						server_timer("easter2019_dungeon_spawner", 5, d.get_map_index())
					else
						d.notice(string.format("Easter dungeon: You still have to defeat %d monsters to move on.", d.count_monster()));
					end
				end
			end
		end

		when easter2019_dungeon_spawner.server_timer begin
			local settings = easter2019_dungeon.settings()
			if d.select(get_server_timer_arg()) then
				if d.getf("easter2019_dungeon_level") == 2 then
					d.spawn_mob(settings["first_metin"], settings["first_metin_pos"][1], settings["first_metin_pos"][2])
				elseif d.getf("easter2019_dungeon_level") == 5 then
					d.regen_file("data/dungeon/easter2019_dungeon/regen_2b.txt")
					server_loop_timer("easter2019_dungeon_wave_kill", 15, d.get_map_index())
				elseif d.getf("easter2019_dungeon_level") == 6 then
					d.spawn_mob(settings["second_metin"], settings["second_metin_pos"][1], settings["second_metin_pos"][2])
				elseif d.getf("easter2019_dungeon_level") == 8 then
					d.regen_file("data/dungeon/easter2019_dungeon/regen_2b.txt")
					server_loop_timer("easter2019_dungeon_wave_kill", 15, d.get_map_index())
				elseif d.getf("easter2019_dungeon_level") == 9 then
					d.jump_all(settings["dungeon_floor_pos"][2][1], settings["dungeon_floor_pos"][2][2])
					d.spawn_mob_dir(settings["door"], settings["door_pos"][2][1], settings["door_pos"][2][2], settings["door_pos"][2][3])
					d.notice("Easter dungeon: Kill all metin stones!")
					d.regen_file("data/dungeon/easter2019_dungeon/regen_3a.txt")
				elseif d.getf("easter2019_dungeon_level") == 10 then
					d.regen_file("data/dungeon/easter2019_dungeon/regen_3b.txt")
					server_loop_timer("easter2019_dungeon_wave_kill", 15, d.get_map_index())
				elseif d.getf("easter2019_dungeon_level") == 11 then
					d.spawn_mob(settings["third_metin"], settings["third_metin_pos"][1], settings["third_metin_pos"][2])
				end
			end
		end
		
		when easter2019_dungeon_30min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Easter dungeon: 15 minutes left!")
				server_timer("easter2019_dungeon_10min_left", 5*60, d.get_map_index())
			end
		end
		
		when easter2019_dungeon_10min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Easter dungeon: 10 minutes left!")
				server_timer("easter2019_dungeon_5min_left", 5*60, d.get_map_index())
			end
		end
		
		when easter2019_dungeon_5min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Easter dungeon: 5 minutes left! The time is running out!")
				server_timer("easter2019_dungeon_1min_left", 4*60, d.get_map_index())
			end
		end
		
		when easter2019_dungeon_1min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Easter dungeon: 1 minutes left! You're almost failed!")
				server_timer("easter2019_dung_final_exit", 60, d.get_map_index())
			end
		end
						
		when easter2019_dung_final_exit.server_timer begin
			local settings = easter2019_dungeon.settings()
			if d.select(get_server_timer_arg()) then
				d.notice("Easter dungeon: You will be teleported out of dungeon!")
				easter2019_dungeon.clear_easterdungeon()
				d.set_warp_location(settings["easter2019_dungeon_index_out"], settings["easter2019_out_pos"][1], settings["easter2019_out_pos"][2])
			end
			
			server_timer("easter2019_dung_final_exit_out", 5, d.get_map_index())
		end
		
		when easter2019_dung_final_exit_out.server_timer begin
			if d.select(get_server_timer_arg()) then								
				d.exit_all()
			end
		end

		when logout with easter2019_dungeon.is_active() begin 
			pc.setf("easter2019_dungeon","exit_easter2019_dungeon_time", get_global_time())
			--pc.setqf("easter2019_dungeon", get_time() + 1) -----Cooldown for next enter [1 hour]
		end
	end
end
