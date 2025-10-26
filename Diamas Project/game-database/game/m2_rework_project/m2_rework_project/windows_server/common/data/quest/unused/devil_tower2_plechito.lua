quest devil_tower2_plechito begin
	state start begin
		--[[ FUNCTION STATEMENT BEGIN ]]
		function get_regens(level)
			local regens = {
				[1] = "data/dungeon/devil_tower2_regens/floor_1.txt",
				[2] = "data/dungeon/devil_tower2_regens/floor_2.txt",
				[3] = "data/dungeon/devil_tower2_regens/floor_3.txt",
				[4] = "data/dungeon/devil_tower2_regens/floor_4.txt",
				[5] = "data/dungeon/devil_tower2_regens/floor_5.txt",
				[6] = "data/dungeon/devil_tower2_regens/floor_6.txt",
				[7] = "data/dungeon/devil_tower2_regens/floor_7.txt",
				[8] = "data/dungeon/devil_tower2_regens/floor_8.txt"};
			
			return d.set_regen_file(regens[level])
		end
		
		function spawn_boss(level)
			local boss_settings = {
				[2] = {806, 382, 191},
				[4] = {817, 394, 394},
				[7] = {1094, 209, 609},
				[8] = {816, 564, 613}};
			
			return d.spawn_mob(boss_settings[level][1], boss_settings[level][2], boss_settings[level][3])
		end
		
		function settings()
			return
			{
				["map_index"] = 502,
				["spawm_pos"] = {
					[1] = {86,99},
					[2] = {332,109},
					[3] = {598,96},
					[4] = {330,360},
					[5] = {166,333},
					[6] = {166,333},
					[7] = {212,540},
					[8] = {560,546}
				},
				["level_check"] = {
					["minimum"] = 150,
					["maximum"] = 170
				},
				["keys"] = {30301, 30302}
			}
		end
		
		function party_get_member_pids()
			local pids = {party.get_member_pids()}
			
			return pids
		end
		
		function make_dungeon()
			local setting = devil_tower2_plechito.settings()
			
			if party.is_party() then
				d.new_jump_party(setting["map_index"], setting["spawm_pos"][1][1], setting["spawm_pos"][1][2])
			else
				d.new_jump(setting["map_index"], setting["spawm_pos"][1][1], setting["spawm_pos"][1][2])
			end
			d.setf("lair_level", 1)
			d.setf("start_time", get_global_time())
			devil_tower2_plechito.get_regens(1)
		end
		
		function is_in_dt2()
			local setting = devil_tower2_plechito.settings()

			local pMapIndex = pc.get_map_index();
			local map_index = 502;
	
			return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
		end
		
		--[[ FUNCTION STATEMENT END ]]
		
		--[[ TOWER STATEMENT BEGIN ]]
		
		when login begin
			local setting = devil_tower2_plechito.settings()

			if mid == setting["map_index"] then
				if not pc.in_dungeon() then
					warp_to_village()
				end
			end
			
			local get_flag = d.getf("lair_level")
			
			if get_flag == 2 then
				if d.getf("level_two_login") == 0 then
					d.setf("level_two_login", 1)
					clear_server_timer("level_two_timer", get_server_timer_arg())
					clear_server_timer("exit_done", get_server_timer_arg())
					server_timer("level_two_timer", 10 * 60, d.get_map_index())
				end
			end
			

		end
		
		when kill with npc.get_race() >= 801 and npc.get_race() <= 805 and devil_tower2_plechito.is_in_dt2() begin
			local count, check = d.getf("mobs") + 1, false;
			
			if d.getf("lair_level") == 1 then
				if count < 100 and check == false then
					d.setf("mobs", count)
				else
					if d.getf("metin_stone") == 0 then
						d.clear_regen()
						d.kill_all()
						d.spawn_mob(8401, 146, 151)
d.set_mission_message(string.format(gameforge.devil_tower2_plechito._10_d_set_mission_message, c_mob_name(8401)))
						d.setf("metin_stone", 1)
					end
					check = true
				end
			end
		end
		
		when kill with npc.get_race() == 8401 and devil_tower2_plechito.is_in_dt2() begin
			local setting = devil_tower2_plechito.settings()
			
			if d.getf("lair_level") == 1 then
				d.notice(gameforge.devil_tower2_plechito._20_dNotice)
				d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_1)
				d.setf("lair_level", 2)
				cleartimer("dt2_level_timer")
				timer("dt2_level_timer", 3)
			end
			if d.getf("lair_level") == 3 then
				local items = {setting["keys"][1], setting["keys"][2]}
				local chance = number(3, 3)
				
				if chance <= 2 then
					game.drop_item(items[1], 1)
				elseif chance == 3 then
					game.drop_item(items[2], 1)
				end
			end
		end
		
		when 9201.take with item.vnum == 30301 and devil_tower2_plechito.is_in_dt2() begin
			d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_2)
		end
		
		when 9201.take with item.vnum == 30302 and devil_tower2_plechito.is_in_dt2() begin
			local setting = devil_tower2_plechito.settings()
			
			d.notice(gameforge.devil_tower2_plechito._30_dNotice)
			d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_3)
			npc.purge()
			d.clear_regen()
			d.kill_all()
			d.setf("lair_level", 4)
			pc.remove_item(setting["keys"][2], 1)
			cleartimer("dt2_level_timer")
			timer("dt2_level_timer", 3)
		end
		
		when kill with npc.get_race() == 805 or npc.get_race() == 804 or npc.get_race() == 803 or npc.get_race() == 809 or npc.get_race() == 810 and devil_tower2_plechito.is_in_dt2() begin
			local count, check_2 = d.getf("mobs_level_2") + 1, false;
			
			if d.getf("lair_level") == 2 then
				if count < 40 and check_2 == false then
					d.setf("mobs_level_2", count)
				else
					if d.getf("boss_check") == 0 then
						d.clear_regen()
						d.kill_all()
						d.set_mission_message(string.format(gameforge.devil_tower2_plechito._40_d_set_mission_message, c_mob_name(806)))
						devil_tower2_plechito.spawn_boss(2)
						d.setf("boss_check", 1)
					end
					check_2 = true
				end
			end
		end
		
		when kill with npc.get_race() == 806 and devil_tower2_plechito.is_in_dt2() begin
			d.notice("You have won the fight against %s !", c_mob_name(806))
			d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_4)
			d.setf("lair_level", 3)
			cleartimer("dt2_level_timer")
			cleartimer("level_two_timer")
			timer("dt2_level_timer", 3)
		end
		
		when kill with npc.get_race() == 807 or npc.get_race() == 808 or npc.get_race() == 809 or npc.get_race() == 812  and devil_tower2_plechito.is_in_dt2() begin
			local count, check_3 = d.getf("mobs_level_3") + 1, false;
			
			if d.getf("lair_level") == 4 then
				if count < 40 and check_3 == false then
					d.setf("mobs_level_3", count)
				else
					if d.getf("boss_check_2") == 0 then
						d.clear_regen()
						d.kill_all()
						devil_tower2_plechito.spawn_boss(4)
						d.set_mission_message(string.format(gameforge.devil_tower2_plechito._40_d_set_mission_message, c_mob_name(817)))
						d.notice(gameforge.devil_tower2_plechito._60_dNotice)
						d.setf("boss_check_2", 1)
					end
					check_3 = true
				end
			end
		end
		
		when kill with npc.get_race() == 817 and devil_tower2_plechito.is_in_dt2() begin
			d.notice("You have won the fight against %s !", c_mob_name(817))
			d.setf("lair_level", 5)
			cleartimer("dt2_level_timer")
			timer("dt2_level_timer", 3)
		end
		
		when kill with npc.get_race() == 8402 and devil_tower2_plechito.is_in_dt2() begin
			local chance = number(1, 2)
			
			if chance <= 2 then
				if d.getf("first_root") == 0 then
					d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_5)
					d.setf("first_root", 1)
				else
					d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_6)
					d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_7)
					d.setf("lair_level", 6)
					cleartimer("dt2_level_timer")
					timer("dt2_level_timer", 3)
				end
			end
		end
		
		when kill with (npc.get_race() == 812 or npc.get_race() == 813 or npc.get_race() == 814) and devil_tower2_plechito.is_in_dt2() begin
			local count = d.getf("mobs_level_4") + 1;
			local purge_count = d.getf("purge_count")
			
			if d.getf("lair_level") == 6 then
				if count < 30 then
					d.setf("mobs_level_4", count)
				else
					if d.getf("seal_check") == 0 then
						if purge_count < 3 then
d.notice(gameforge.devil_tower2_plechito._70_dNotice, 4-d.getf("purge_count"))
							d.notice(gameforge.devil_tower2_plechito._80_dNotice)
						else
							d.notice(gameforge.devil_tower2_plechito._90_dNotice)
						end
						d.setf("lock_seal", 0)
						d.setf("seal_check", 1)
					end
				end
			end
		end
		
		when 9202.click with devil_tower2_plechito.is_in_dt2() and npc.lock() begin
			local purge_count = d.getf("purge_count")
			
			if d.getf("lair_level") == 6 then
				if d.getf("lock_seal") == 0 then
					if d.getf("seal_check") == 1 then
						if purge_count < 3 then
							d.notice(gameforge.devil_tower2_plechito._100_dNotice)
							devil_tower2_plechito.get_regens(6)
							npc.unlock()
							npc.purge()
							d.setf("mobs_level_4", 0)
							d.setf("purge_count", purge_count+1)
						else
							d.notice(gameforge.devil_tower2_plechito._110_dNotice)
							d.setf("lair_level", 7)
							npc.unlock()
							npc.purge()
							cleartimer("dt2_level_timer")
							timer("dt2_level_timer", 3)
						end
						d.setf("lock_seal", 0)
					end
					npc.unlock()
					d.setf("lock_seal", 1)
					d.setf("seal_check", 0)
				end
			end
		end
		
		when kill with npc.get_race() == 811 or npc.get_race() == 814 or npc.get_race() == 815 and devil_tower2_plechito.is_in_dt2() begin
			local count, check_4 = d.getf("mobs_level_5") + 1, false;
			
			if d.getf("lair_level") == 7 then
				if count < 40 and check_4 == false then
					d.setf("mobs_level_5", count)
				else
					if d.getf("boss_check_4") == 0 then
						d.clear_regen()
						d.kill_all()
						devil_tower2_plechito.spawn_boss(7)
						d.notice(gameforge.devil_tower2_plechito._120_dNotice, c_mob_name(1094))
						d.setf("boss_check_4", 1)
					end
					check_4 = true
				end
			end
		end
		
		when kill with npc.get_race() == 1094 and devil_tower2_plechito.is_in_dt2() begin
			d.notice(gameforge.devil_tower2_plechito._130_dNotice, c_mob_name(1094))
			d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_8)
			d.clear_regen()
			d.kill_all()
			d.setf("lair_level", 8)
			cleartimer("dt2_level_timer")
			timer("dt2_level_timer", 3)
		end
		
		when kill with npc.get_race() == 816 and devil_tower2_plechito.is_in_dt2() begin
			d.notice(gameforge.devil_tower2_plechito._140_dNotice, c_mob_name(816))
			notice_all(gameforge.devil_tower2_plechito._190_notice_all1 .. pc.get_name() .. gameforge.devil_tower2_plechito._190_notice_all2) --- GLOBAL NOTICE
			d.set_mission_message(gameforge.devil_tower2_plechito.set_mission_message_9)
			d.clear_regen()
			d.kill_all()
			d.setf("lair_level", 0)
			if party.is_party() then
				highscore.register("dt2_pt_time", get_global_time() - d.getf("start_time"), 0)
			else
				highscore.register("dt2_time", get_global_time() - d.getf("start_time"), 0)
			end
			pc.setqf("816_kills", pc.getqf("816_kills") +  1)		
			clear_server_timer("final", get_server_timer_arg())
			server_timer("final", 2*60, get_server_timer_arg())
		end
		
		--[[ TOWER STATEMENT END ]]
		
		--[[ TIMER STATEMENT BEGIN ]]
		
		when dt2_level_timer.timer begin
			local get_flag = d.getf("lair_level")
			local setting = devil_tower2_plechito.settings()
			
			d.clear_regen()
			d.kill_all()
			
			if get_flag == 2 then
				d.jump_all(setting["spawm_pos"][2][1], setting["spawm_pos"][2][2])
				devil_tower2_plechito.get_regens(2)
				d.setf("lair_level", 2)
			end
			if get_flag == 3 then
				d.jump_all(setting["spawm_pos"][3][1], setting["spawm_pos"][3][2])

				devil_tower2_plechito.get_regens(3)
				d.setf("lair_level", 3)
			end
			if get_flag == 4 then
				d.jump_all(setting["spawm_pos"][4][1], setting["spawm_pos"][4][2])

				devil_tower2_plechito.get_regens(4)
				d.setf("lair_level", 4)
			end
			if get_flag == 5 then
				d.jump_all(setting["spawm_pos"][5][1], setting["spawm_pos"][5][2])

				devil_tower2_plechito.get_regens(5)
				d.setf("lair_level", 5)
			end
			if get_flag == 6 then
				d.jump_all(setting["spawm_pos"][6][1], setting["spawm_pos"][6][2])

				devil_tower2_plechito.get_regens(6)
				d.spawn_mob(9202, 146, 409)
				d.spawn_mob(9202, 142, 331)
				d.spawn_mob(9202, 106, 372)
				d.spawn_mob(9202, 186, 369)
				d.setf("lair_level", 6)
			end
			if get_flag == 7 then
				d.jump_all(setting["spawm_pos"][7][1], setting["spawm_pos"][7][2])

				devil_tower2_plechito.get_regens(7)
				d.setf("lair_level", 7)
			end
			if get_flag == 8 then
				d.jump_all(setting["spawm_pos"][8][1], setting["spawm_pos"][8][2])

				devil_tower2_plechito.get_regens(8)
				d.setf("lair_level", 8)
			end
		end
		
		when level_two_timer.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.set_warp_location(41, 478, 738)
				d.notice(gameforge.devil_tower2_plechito._160_dNotice)
				
				server_timer("exit_done", 3, get_server_timer_arg())
			end
		end
		
		when exit_done.server_timer begin
			if d.select(get_server_timer_arg()) then
				clear_server_timer("exit_done", get_server_timer_arg())
				clear_server_timer("final", get_server_timer_arg())
				clear_server_timer("level_two_timer", get_server_timer_arg())
				d.exit_all()
			end
		end
		
		when final.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice(gameforge.devil_tower2_plechito._170_dNotice)
				
				clear_server_timer("exit_done", get_server_timer_arg())
				server_timer("exit_done", 3, get_server_timer_arg())
			end
		end
		
		--[[ TIMER STATEMENT END ]]
		
		--[[ CHAT BEGIN ]]
		when 9203.chat.gameforge.devil_tower2_plechito._180_npcChat begin
			local settings = devil_tower2_plechito.settings()
			
			say_title(c_mob_name(9203))

						if party.is_party() and not party.is_leader() then
				say(gameforge.devil_tower2_plechito._190_say)
				return
			end
			
			if party.is_party() and party.get_near_count() < 2 then
				say(gameforge.devil_tower2_plechito._200_say)
				return
			end
			
			local levelCheck, maximlevelCheck = true, true
			local notEnoughLevelMembers, notEnoughmaximLevelMembers = {}, {}
			local pids = {party.get_member_pids()}
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				
				if pc.get_level() < settings["level_check"]["minimum"] then
					table.insert(notEnoughLevelMembers, pc.get_name())
					levelCheck = false
				end
				if pc.get_level() > settings["level_check"]["maximum"] then
					table.insert(notEnoughmaximLevelMembers, pc.get_name())
					maximlevelCheck = false
				end
				
				q.end_other_pc_block()
			end
			
			if not levelCheck then
				say(gameforge.devil_tower2_plechito._210_say)
say(string.format(gameforge.devil_tower2_plechito._220_say, settings["level_check"]["minimum"]))
				say(gameforge.devil_tower2_plechito._230_say)
				for i, name in next, notEnoughLevelMembers, nil do
					say(string.format(gameforge.demon_dungeon._50_say, name))
				end
				return
			end
			
			if not maximlevelCheck then
				say(gameforge.devil_tower2_plechito._210_say)
say(string.format(gameforge.devil_tower2_plechito._220_say, settings["level_check"]["maximum"]))
				say(gameforge.devil_tower2_plechito._230_say)
				for i, name in next, notEnoughmaximLevelMembers, nil do
					say(string.format(gameforge.demon_dungeon._50_say, name))
				end
				return
			end
			
			say(gameforge.devil_tower2_plechito._240_say)
			wait()
			devil_tower2_plechito.make_dungeon()
		end
		--[[ CHAT END ]]
	end
end