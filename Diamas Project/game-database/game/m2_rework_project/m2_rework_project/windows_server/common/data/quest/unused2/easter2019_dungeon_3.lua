quest easter2019_dungeon_3 begin
	state start begin
	------------------------------
	--QUEST FUNCTIONS AND SETTINGS
	------------------------------
	function settings()
		return
		{
			["easter2019_dungeon_3_index"] = 233, ---- Dungeon index
			["easter2019_dungeon_3_index_out"] = 234, ---- Map index, where players will be teleported out of dungeon
			["easter2019_out_pos_3"] = {402,222}, ---- Coordinations, where players will be teleported out of dungeon
			["dungeon_floor_pos_3"] = {				----- Dungeon coords
				[1] = {40500, 59100},
				[2] = {405, 591} 
			},
			["level_check_3"] = {
				["minimum_3"] = 150,
				["maximum_3"] = 150
			},
			["pass_3"] = 71175, --- Ticket
			["GroupType_3"] = false, ---- If its solo dungeon
			["Items_3"] = {30732, 30733}, 
			["first_metin_3"] = 28435, 				
			["first_metin_pos_3"] = {390, 242},
			["second_metin_3"] = 28434, 				
			["second_metin_pos_3"] = {391, 449},
			["third_metin_3"] = 28436, 				
			["third_metin_pos_3"] = {407, 615},
			["door_3"] = 9248, 				
			["door_pos_3"] = {
				[1] = {383, 276, 5},
				[2] = {381, 637, 8}
			},
			["bridge_pos_3"] = {398, 481, 1},
			["bad_rabbit_pos_3"] = {406, 473, 5},
			["fence_pos_3"] = {398, 481, 5},
			--["main_boss_3"] = 2487, 				
			--["main_boss_pos_3"] = {181, 672},

		};
	end
	
	--------PARTY AND ENTER
	function party_get_member_pids()
		local pids = {party.get_member_pids()}
		
		return pids
	end
	
	function is_active_3()
		local pMapIndex = pc.get_map_index();
		local data = easter2019_dungeon_3.settings();
		local map_index = data["easter2019_dungeon_3_index"];

		return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
	end
	
	function clear_easterdungeon_3()
		d.clear_regen();
		d.kill_all();
	end
	
	function check_enter_3()
		addimage(25, 10, "easter2019_1.tga")
		addimage(225, 150, "easter_rabbit.tga")
		say("")
		say("")
		say("")
		say_title(c_mob_name(32129))
		local settings = easter2019_dungeon_3.settings()
		
		--if ((get_global_time() - pc.getf("easter2019_dungeon_2","exit_easter2019_dungeon_2_time")) < 1) then
		
		--	local remaining_wait_time = (pc.getf("easter2019_dungeon_2","exit_easter2019_dungeon_2_time") - get_global_time() + 1)
		--	say(gameforge.easter2019_dungeon._100_say1)
		--	say_reward(gameforge.easter2019_dungeon._100_say2..get_time_remaining(remaining_wait_time)..'[ENTER]')
		--	return
		--end
		
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
			
			local levelCheck_3 = true
			local passCheck_3 = true
			local MemberHaveLowLevel_3 = {}
			local MemberHaveHighLevel_3 = {}
			local MemberHaveNoTicket_3 = {}
			local pids = {party.get_member_pids()}
			
			--if not party.is_map_member_flag_lt("exit_easter2019_dungeon_2_time", get_global_time() - 1) then
			--	say_reward(gameforge.easter2019_dungeon._100_say7)
			--	say_reward(gameforge.easter2019_dungeon._100_say8)
			--	return false;
			--end
						
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() < settings["level_check_3"]["minimum_3"] then
					table.insert(MemberHaveLowLevel_3, pc.get_name())
					levelCheck_3 = false
				end

				q.end_other_pc_block()
			end

			if not levelCheck_3 then
				say_reward(gameforge.easter2019_dungeon._100_say9)
				say_reward(gameforge.easter2019_dungeon._100_say10)
				say_reward("")
				say_reward(gameforge.easter2019_dungeon._100_say11)
				for i, n in next, MemberHaveLowLevel_3, nil do
					say_title("- "..n)
				end
				return
			end
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() > settings["level_check_3"]["maximum_3"] then
					table.insert(MemberHaveHighLevel_3, pc.get_name())
					levelCheck_3 = false
				end

				q.end_other_pc_block()
			end

			if not levelCheck_3 then
				say_reward(gameforge.easter2019_dungeon._100_say12)
				--say_reward(gameforge.easter2019_dungeon._100_say13)
				say("")
				say_reward(gameforge.easter2019_dungeon._100_say14)
				for i, n in next, MemberHaveHighLevel_3, nil do
					say_title("- "..n)
				end
				return
			end
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.count_item(settings.pass_3) < 1 then
					table.insert(MemberHaveNoTicket_3, pc.get_name())
					passCheck_3 = false
				end

				q.end_other_pc_block()
			end

			if not passCheck_3 then
				say_reward(gameforge.easter2019_dungeon._100_say15)
				say_reward(gameforge.easter2019_dungeon._100_say16)
				say_item(gameforge.easter2019_dungeon._100_say17, settings.pass_3, "")
				say("")
				say_reward(gameforge.easter2019_dungeon._100_say18)
				for i, n in next, MemberHaveNoTicket_3, nil do
					say_title("- "..n)
				end
				return
			end
	
		else
		
			--if ((get_global_time() - pc.getf("easter2019_dungeon_2","exit_easter2019_dungeon_2_time")) < 1) then
			
			--	local remaining_wait_time = (pc.getf("easter2019_dungeon_2","exit_easter2019_dungeon_2_time") - get_global_time() + 1)
			--	say(gameforge.easter2019_dungeon._100_say19)
			--	say_reward(gameforge.easter2019_dungeon._100_say20..get_time_remaining(remaining_wait_time)..'[ENTER]')
			--	return
			--end
			
			if (pc.get_level() < settings["level_check_3"]["minimum_3"]) then
				say(string.format(gameforge.easter2019_dungeon._100_say21, settings["level_check_3"]["minimum_3"]))
				return false;
			end
			
			if (pc.get_level() > settings["level_check_3"]["maximum_3"]) then
				say(string.format(gameforge.easter2019_dungeon._100_say21_1, settings["level_check_3"]["maximum_3"]))
				return false;
			end
			
			if (pc.count_item(settings["pass_3"]) < 1) then
				say_reward(gameforge.easter2019_dungeon._100_say22)
				say_reward(gameforge.easter2019_dungeon._100_say23)
				say_item(gameforge.easter2019_dungeon._100_say24, settings.pass_3, "")
				return false;
			end
		end
		
		return true;
	end
	
	------------DUNGEON ENTER
	function create_dungeon_3()
		local settings = easter2019_dungeon_3.settings()
		local pids = {party.get_member_pids()}
		
		if party.is_party() then
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				pc.remove_item(settings["pass_3"], 1)
				q.end_other_pc_block()
			end
			d.new_jump_party(settings["easter2019_dungeon_3_index"], settings["dungeon_floor_pos_3"][1][1], settings["dungeon_floor_pos_3"][1][2])
			d.setf("easter2019_dungeon_3_level", 1)
			d.regen_file("data/dungeon/easter2019_dungeon_3/regen_1a.txt")
			d.spawn_mob_dir(settings["door_3"], settings["door_pos_3"][1][1], settings["door_pos_3"][1][2], settings["door_pos_3"][1][3])
			server_timer("easter2019_dungeon_3_30min_left", 30*60, d.get_map_index())
			server_loop_timer("easter2019_dungeon_3_wave_kill", 2, d.get_map_index())
		elseif (not settings["GroupType_3"]) then
				if settings["pass_3"] then
					pc.remove_item(settings["pass_3"], 1)
				end
			pc.remove_item(settings["pass_3"], 1)
			d.new_jump(settings["easter2019_dungeon_3_index"], settings["dungeon_floor_pos_3"][1][1]*100, settings["dungeon_floor_pos_3"][1][2]*100)
			d.setf("easter2019_dungeon_3_level", 1)
			d.regen_file("data/dungeon/easter2019_dungeon_3/regen_1a.txt")
			d.spawn_mob_dir(settings["door_3"], settings["door_pos_3"][1][1], settings["door_pos_3"][1][2], settings["door_pos_3"][1][3])
			server_timer("easter2019_dungeon_3_30min_left", 30*60, d.get_map_index())
			server_loop_timer("easter2019_dungeon_3_wave_kill", 2, d.get_map_index())
		end
		--d.setf("start_time", get_global_time())

	end
			
		--FUNCTIONS END
		
		--LOGIN IN MAP
		
		
		when 70007.use with pc.in_dungeon(233) begin
			pc.warp(234, 40200, 22200)
		end
		
		when login begin
			local settings = easter2019_dungeon_3.settings()
						
			if pc.get_map_index() == settings["easter2019_dungeon_3_index"] then
				if not pc.in_dungeon() then
						warp_to_village()
					end
				end
			end

		--DUNGEON ENTER
		when 32129.chat.gameforge.easter2019_dungeon._100_say57 with not easter2019_dungeon_3.is_active_3() begin
			local settings = easter2019_dungeon_3.settings()
			addimage(25, 10, "easter2019_1.tga")
			addimage(225, 150, "easter_rabbit.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(32129))
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
			say_title(c_mob_name(32129))
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
				if easter2019_dungeon_3.check_enter_3() then
					say(gameforge.easter2019_dungeon._100_say47)
					say(gameforge.easter2019_dungeon._100_say48)
					say(gameforge.easter2019_dungeon._100_say49)
					say("")
					say_reward(gameforge.easter2019_dungeon._100_say50)
					say_reward(gameforge.easter2019_dungeon._100_say51)
					wait()
					easter2019_dungeon_3.create_dungeon_3()
				end
			end
		end
		
		--when 32129.chat."Time reset" with pc.is_gm() begin
		--	addimage(25, 10, "easter2019_1.tga")
		--	say("")
		--	say("")
		--	say("")
		--	if select('Reset time','Close') == 2 then return end
		--		addimage(25, 10, "easter2019_1.tga")
		--		say("")
		--		say("")
		--		say("")
		--		say_title(c_mob_name(32129))
		--		say("")
		--		say(gameforge.easter2019_dungeon._100_say52)
		--		pc.setf('easter2019_dungeon_2','exit_easter2019_dungeon_2_time', 0)
				
				-- Dungeon Info
				--pc.setqf("rejoin_time", get_time() - 1)
		--end		
		
		------------ 1. FLOOR		
		when kill with easter2019_dungeon_3.is_active_3() and not npc.is_pc() and npc.get_race() == 28435 and d.getf("easter2019_dungeon_3_level") == 2 begin
			local settings = easter2019_dungeon_3.settings()
			game.drop_item(settings["Items_3"][1], 1)
			d.setf("easter2019_dungeon_3_level", 3)
		end
		
		when 9248.take with item.get_vnum() == 30732 and easter2019_dungeon_3.is_active_3() begin
			local settings = easter2019_dungeon_3.settings()
			if d.getf("easter2019_dungeon_3_level") == 3 then
				npc.kill()
				item.remove()
				d.setf("easter2019_dungeon_3_level", 4)
				d.notice("Easter dungeon: Now destroy all metin stones.")
				d.regen_file("data/dungeon/easter2019_dungeon_3/regen_2a.txt")
				d.spawn_mob_dir(9247, settings["bridge_pos_3"][1], settings["bridge_pos_3"][2], settings["bridge_pos_3"][3])
				d.spawn_mob_dir(9250, settings["bad_rabbit_pos_3"][1], settings["bad_rabbit_pos_3"][2], settings["bad_rabbit_pos_3"][3])
				d.spawn_mob_dir(9249, settings["fence_pos_3"][1], settings["fence_pos_3"][2], settings["fence_pos_3"][3])
			elseif d.getf("easter2019_dungeon_3_level") == 12 then
				npc.kill()
				pc.remove_item(settings["Items_3"][1], 1)
				d.notice("Easter dungeon: Defeat Evil easter rabbit!")
				d.regen_file("data/dungeon/easter2019_dungeon_3/regen_boss.txt")
				d.regen_file("data/dungeon/easter2019_dungeon_3/regen_4a.txt")
				d.setf("easter2019_dungeon_3_level", 13)
			end
		end
		
		---------------2. FLOOR
		when kill with easter2019_dungeon_3.is_active_3() and not npc.is_pc() and npc.get_race() == 28435 and d.getf("easter2019_dungeon_3_level") == 4 begin
			local kills = 6;
			d.setf("ed2019_egg1_3", d.getf("ed2019_egg1_3")+1);
			if (d.getf("ed2019_egg1_3") < kills) then
				d.notice("Easter dungeon: %s eggs has left!", kills-d.getf("ed2019_egg1_3"))
			else
				d.notice("Easter dungeon: Well done! Now kill all monsters!")
				d.setf("easter2019_dungeon_3_level", 5)
				server_timer("easter2019_dungeon_3_spawner", 2, d.get_map_index())
			end 
		end 
		
		when kill with easter2019_dungeon_3.is_active_3() and not npc.is_pc() and npc.get_race() == 28434 and d.getf("easter2019_dungeon_3_level") == 6 begin
			local settings = easter2019_dungeon_3.settings()
			game.drop_item(settings["Items_3"][2], 1)
			d.notice("Easter dungeon: Give the coin to rabbit!")
			d.setf("easter2019_dungeon_3_level", 7)
		end
		
		when 9250.take with item.get_vnum() == 30733 and easter2019_dungeon_3.is_active_3() and d.getf("easter2019_dungeon_3_level") == 7 begin
			local settings = easter2019_dungeon_3.settings()
			d.kill_all()
			item.remove()
			d.setf("easter2019_dungeon_3_level", 8)
			d.notice("Easter dungeon: He has cheated on you!")
			d.notice("Easter dungeon: Be careful! Monsters are coming!")
			server_timer("easter2019_dungeon_3_spawner", 2, d.get_map_index())
		end
		---------------------3. FLOOR
		when kill with easter2019_dungeon_3.is_active_3() and not npc.is_pc() and npc.get_race() == 28434 and d.getf("easter2019_dungeon_3_level") == 9 begin
			local kills = 4;
			d.setf("ed2019_metin1_3", d.getf("ed2019_metin1_3")+1);
			if (d.getf("ed2019_metin1_3") < kills) then
				d.notice("Easter dungeon: %s metin stones has left!", kills-d.getf("ed2019_metin1_3"))
			else
				d.notice("Easter dungeon: Well done! Now kill all monsters!")
				d.setf("easter2019_dungeon_3_level", 10)
				server_timer("easter2019_dungeon_3_spawner", 2, d.get_map_index())
			end 
		end 
		
		when kill with easter2019_dungeon_3.is_active_3() and not npc.is_pc() and npc.get_race() == 28436 and d.getf("easter2019_dungeon_3_level") == 11 begin
			local settings = easter2019_dungeon_3.settings()
			game.drop_item(settings["Items_3"][1], 1)
			d.setf("easter2019_dungeon_3_level", 12)
		end
		
		---------------------4. FLOOR
		when kill with easter2019_dungeon_3.is_active_3() and not npc.is_pc() and npc.get_race() == 2487 or npc.get_race() == 2488 and d.getf("easter2019_dungeon_3_level") == 13 begin
			local settings = easter2019_dungeon_3.settings()
			d.kill_all()
			d.setf("easter2019_dungeon_3_level", 14)
			d.notice("Easter dungeon: You've succefuly finished the dungeon!")
			d.notice("Easter dungeon: You will be teleported in 30 seconds.")
			pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
			if math.mod(pc.getqf("boss_kills"), 5) == 0 then
				notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(2487), pc.getqf("boss_kills"))
			end
			--if party.is_party() then
			--	highscore.register("eev_pt_time", get_global_time() - d.getf("start_time"), 0)
			--else
			--	highscore.register("eev_time", get_global_time() - d.getf("start_time"), 0)
			--end
			server_timer("easter2019_dung_final_exit_3", 30, d.get_map_index())
		end
				
		-------------------TIMERS
		when easter2019_dungeon_3_wave_kill.server_timer begin
			local settings = easter2019_dungeon_3.settings()
			if d.select(get_server_timer_arg()) then
				if d.getf("easter2019_dungeon_3_level") == 1 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_3_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_3_level", 2)
						d.notice("Easter dungeon: Well done! Now destroy an easter egg!")
						server_timer("easter2019_dungeon_3_spawner", 3, d.get_map_index())
					else
						d.notice("Easter dungeon: You still have to defeat %s monsters to move on.", d.count_monster())
					end
				elseif d.getf("easter2019_dungeon_3_level") == 5 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_3_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_3_level", 6)
						d.notice("Easter dungeon: You've succesfuly killed all monsters.")
						d.notice("Easter dungeon: Destroy a metin stone to move on!")
						server_timer("easter2019_dungeon_3_spawner", 3, d.get_map_index())
					else
						d.notice("Easter dungeon: You still have to defeat %s monsters to move on.", d.count_monster())
					end
				elseif d.getf("easter2019_dungeon_3_level") == 8 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_3_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_3_level", 9)
						d.notice("Easter dungeon: You've succesfuly killed all monsters.")
						d.notice("Easter dungeon: You will jump to next level!")
						server_timer("easter2019_dungeon_3_spawner", 2, d.get_map_index())
					else
						d.notice("Easter dungeon: You still have to defeat %s monsters to move on.", d.count_monster())
					end
				elseif d.getf("easter2019_dungeon_3_level") == 10 then
					if d.count_monster() == 0 then
						clear_server_timer("easter2019_dungeon_3_wave_kill", get_server_timer_arg())
						d.setf("easter2019_dungeon_3_level", 11)
						d.notice("Easter dungeon: You've succesfuly killed all monsters.")
						d.notice("Easter dungeon: Destroy evil egg to get a key.")
						server_timer("easter2019_dungeon_3_spawner", 2, d.get_map_index())
					else
						d.notice("Easter dungeon: You still have to defeat %s monsters to move on.", d.count_monster());
					end
				end
			end
		end

		when easter2019_dungeon_3_spawner.server_timer begin
			local settings = easter2019_dungeon_3.settings()
			if d.select(get_server_timer_arg()) then
				if d.getf("easter2019_dungeon_3_level") == 2 then
					d.spawn_mob(settings["first_metin_3"], settings["first_metin_pos_3"][1], settings["first_metin_pos_3"][2])
				elseif d.getf("easter2019_dungeon_3_level") == 5 then
					d.regen_file("data/dungeon/easter2019_dungeon_3/regen_2b.txt")
					server_loop_timer("easter2019_dungeon_3_wave_kill", 2, d.get_map_index())
				elseif d.getf("easter2019_dungeon_3_level") == 6 then
					d.spawn_mob(settings["second_metin_3"], settings["second_metin_pos_3"][1], settings["second_metin_pos_3"][2])
				elseif d.getf("easter2019_dungeon_3_level") == 8 then
					d.regen_file("data/dungeon/easter2019_dungeon_3/regen_2b.txt")
					server_loop_timer("easter2019_dungeon_3_wave_kill", 2, d.get_map_index())
				elseif d.getf("easter2019_dungeon_3_level") == 9 then
					d.jump_all(settings["dungeon_floor_pos_3"][2][1], settings["dungeon_floor_pos_3"][2][2])
					d.spawn_mob_dir(settings["door_3"], settings["door_pos_3"][2][1], settings["door_pos_3"][2][2], settings["door_pos_3"][2][3])
					d.notice("Easter dungeon: Kill all metin stones!")
					d.regen_file("data/dungeon/easter2019_dungeon_3/regen_3a.txt")
				elseif d.getf("easter2019_dungeon_3_level") == 10 then
					d.regen_file("data/dungeon/easter2019_dungeon_3/regen_3b.txt")
					server_loop_timer("easter2019_dungeon_3_wave_kill", 2, d.get_map_index())
				elseif d.getf("easter2019_dungeon_3_level") == 11 then
					d.spawn_mob(settings["third_metin_3"], settings["third_metin_pos_3"][1], settings["third_metin_pos_3"][2])
				end
			end
		end
		
					
		when easter2019_dungeon_3_30min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Easter dungeon: 1 minutes left! You're almost failed!")
				server_timer("easter2019_dung_final_exit_3", 60, d.get_map_index())
			end
		end
					
		when easter2019_dung_final_exit_3.server_timer begin
			local settings = easter2019_dungeon_3.settings()
			if d.select(get_server_timer_arg()) then
				d.notice("Easter dungeon: You will be teleported out of dungeon!")
				easter2019_dungeon_3.clear_easterdungeon_3()
				d.set_warp_location(settings["easter2019_dungeon_3_index_out"], settings["easter2019_out_pos_3"][1], settings["easter2019_out_pos_3"][2])
			end
			
			server_timer("easter2019_dung_final_exit_out_3", 2, d.get_map_index())
		end
		
		when easter2019_dung_final_exit_out_3.server_timer begin
			if d.select(get_server_timer_arg()) then								
				d.exit_all()
			end
		end

		--when logout with easter2019_dungeon_2.is_active_2() begin 
		--	pc.setf("easter2019_dungeon_2","exit_easter2019_dungeon_2_time", get_global_time())
			--pc.setqf("easter2019_dungeon_2", get_time() + 1) -----Cooldown for next enter [1 hour]
		--end
	end
end
