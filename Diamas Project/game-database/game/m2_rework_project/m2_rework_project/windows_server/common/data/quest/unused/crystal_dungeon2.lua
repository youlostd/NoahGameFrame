quest crystal_dungeon begin
	state start begin
		--Functions
		function settings()
			return
			{
				["map_index"] = 504,
				["spawm_pos"] = {
					[1] = {280, 136},
					[2] = {658, 249},
					[3] = {240, 627},
					[4] = {404, 664},
					[5] = {773, 747},
				},
				["level_check"] = {
					["minimum"] = 150,
					["maximum"] = 170
				},
				["keys"] = {30701, 30702, 30703},
				["metin_stones"] = {8411, 8412, 8413, 8414}
			};
		end
		
		function spawn_boss(level)
			local boss_settings = {
				[3] = {948, 741, 191},
				[5] = {949, 431, 544},
				[7] = {950, 773, 681}
				};
			
			return d.spawn_mob(boss_settings[level][1], boss_settings[level][2], boss_settings[level][3])
		end
		
		function spawn_metins_4thfloor()
			local setting = crystal_dungeon.settings()
			local r = number (1,8)
			if r == 1 then
				d.spawn_mob(setting["metin_stones"][3], 250, 659)
				d.spawn_mob(setting["metin_stones"][2], 239, 714)
				d.spawn_mob(setting["metin_stones"][2], 257, 745)
				d.spawn_mob(setting["metin_stones"][2], 282, 777)
				d.spawn_mob(setting["metin_stones"][2], 321, 767)
				d.spawn_mob(setting["metin_stones"][2], 371, 770)
				d.spawn_mob(setting["metin_stones"][2], 397, 737)
				d.spawn_mob(setting["metin_stones"][2], 410, 714)
			elseif r == 2 then
				d.spawn_mob(setting["metin_stones"][2], 250, 659)
				d.spawn_mob(setting["metin_stones"][3], 239, 714)
				d.spawn_mob(setting["metin_stones"][2], 257, 745)
				d.spawn_mob(setting["metin_stones"][2], 282, 777)
				d.spawn_mob(setting["metin_stones"][2], 321, 767)
				d.spawn_mob(setting["metin_stones"][2], 371, 770)
				d.spawn_mob(setting["metin_stones"][2], 397, 737)
				d.spawn_mob(setting["metin_stones"][2], 410, 714)
			elseif r == 3 then
				d.spawn_mob(setting["metin_stones"][2], 250, 659)
				d.spawn_mob(setting["metin_stones"][2], 239, 714)
				d.spawn_mob(setting["metin_stones"][3], 257, 745)
				d.spawn_mob(setting["metin_stones"][2], 282, 777)
				d.spawn_mob(setting["metin_stones"][2], 321, 767)
				d.spawn_mob(setting["metin_stones"][2], 371, 770)
				d.spawn_mob(setting["metin_stones"][2], 397, 737)
				d.spawn_mob(setting["metin_stones"][2], 410, 714)
			elseif r == 4 then
				d.spawn_mob(setting["metin_stones"][2], 250, 659)
				d.spawn_mob(setting["metin_stones"][2], 239, 714)
				d.spawn_mob(setting["metin_stones"][2], 257, 745)
				d.spawn_mob(setting["metin_stones"][3], 282, 777)
				d.spawn_mob(setting["metin_stones"][2], 321, 767)
				d.spawn_mob(setting["metin_stones"][2], 371, 770)
				d.spawn_mob(setting["metin_stones"][2], 397, 737)
				d.spawn_mob(setting["metin_stones"][2], 410, 714)
			elseif r == 5 then
				d.spawn_mob(setting["metin_stones"][2], 250, 659)
				d.spawn_mob(setting["metin_stones"][2], 239, 714)
				d.spawn_mob(setting["metin_stones"][2], 257, 745)
				d.spawn_mob(setting["metin_stones"][2], 282, 777)
				d.spawn_mob(setting["metin_stones"][3], 321, 767)
				d.spawn_mob(setting["metin_stones"][2], 371, 770)
				d.spawn_mob(setting["metin_stones"][2], 397, 737)
				d.spawn_mob(setting["metin_stones"][2], 410, 714)
			elseif r == 6 then
				d.spawn_mob(setting["metin_stones"][2], 250, 659)
				d.spawn_mob(setting["metin_stones"][2], 239, 714)
				d.spawn_mob(setting["metin_stones"][2], 257, 745)
				d.spawn_mob(setting["metin_stones"][2], 282, 777)
				d.spawn_mob(setting["metin_stones"][2], 321, 767)
				d.spawn_mob(setting["metin_stones"][3], 371, 770)
				d.spawn_mob(setting["metin_stones"][2], 397, 737)
				d.spawn_mob(setting["metin_stones"][2], 410, 714)
			elseif r == 7 then
				d.spawn_mob(setting["metin_stones"][2], 250, 659)
				d.spawn_mob(setting["metin_stones"][2], 239, 714)
				d.spawn_mob(setting["metin_stones"][2], 257, 745)
				d.spawn_mob(setting["metin_stones"][2], 282, 777)
				d.spawn_mob(setting["metin_stones"][2], 321, 767)
				d.spawn_mob(setting["metin_stones"][2], 371, 770)
				d.spawn_mob(setting["metin_stones"][3], 397, 737)
				d.spawn_mob(setting["metin_stones"][2], 410, 714)
			elseif r == 8 then
				d.spawn_mob(setting["metin_stones"][2], 250, 659)
				d.spawn_mob(setting["metin_stones"][2], 239, 714)
				d.spawn_mob(setting["metin_stones"][2], 257, 745)
				d.spawn_mob(setting["metin_stones"][2], 282, 777)
				d.spawn_mob(setting["metin_stones"][2], 321, 767)
				d.spawn_mob(setting["metin_stones"][2], 371, 770)
				d.spawn_mob(setting["metin_stones"][2], 397, 737)
				d.spawn_mob(setting["metin_stones"][3], 410, 714)
			end
		end
		
		function make_dungeon()
			local setting = crystal_dungeon.settings()
			if party.is_party() then
				d.new_jump_party(setting["map_index"], setting["spawm_pos"][1][1], setting["spawm_pos"][1][2])
			else
				d.new_jump(setting["map_index"], setting["spawm_pos"][1][1], setting["spawm_pos"][1][2])
			end
			d.setf("start_time", get_global_time())
			d.setf("crystal_level", 1)
			d.setf("level_four_login", 0)
			crystal_dungeon.get_regens(1)
		end

		function get_regens(level)
			local regens = {
				[1] = "data/dungeon/crystal_dungeon/regen_1.txt",
				[2] = "data/dungeon/crystal_dungeon/regen_2.txt",
				[3] = "data/dungeon/crystal_dungeon/regen_3.txt",
				[4] = "data/dungeon/crystal_dungeon/regen_4a.txt",
				[5] = "data/dungeon/crystal_dungeon/regen_4b.txt",
				[6] = "data/dungeon/crystal_dungeon/regen_5.txt",
				[7] = "data/dungeon/crystal_dungeon/regen_5_metins.txt"};
			
			return d.set_regen_file(regens[level])
		end
		
		function party_get_member_pids()
			local pids = {party.get_member_pids()}
			
			return pids
		end
		
		function crystalIsInDungeon()
			local pMapIndex = pc.get_map_index();
			local data = crystal_dungeon.settings();
			local map_index = data["map_index"];

			return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
		end
		
		------
		------[Dungeon enter]
		------
		when 9214.chat.gameforge.crystal_dungeon2._10_npcChat begin
			local settings = crystal_dungeon.settings()
			
			say_title(c_mob_name(9214))
						
			if party.is_party() and not party.is_leader() then
				say(gameforge.crystal_dungeon2._20_say)
				return
			end
			
			if party.is_party() and party.get_near_count() < 2 then
				say(gameforge.crystal_dungeon2._30_say)
				return
			end
			
			local levelCheck, maximlevelCheck = true, true
			local notEnoughLevelMembers, notEnoughmaximLevelMembers = {}, {}
			local pids = {party.get_member_pids()}
			
			for i, pid in next, crystal_dungeon.party_get_member_pids(), nil do
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
				say(gameforge.crystal_dungeon2._40_say)
say(string.format(gameforge.crystal_dungeon2._50_say, settings["level_check"]["minimum"]))
				say(gameforge.crystal_dungeon2._60_say)
				for i, name in next, notEnoughLevelMembers, nil do
					say(string.format(gameforge.demon_dungeon._50_say, name))
				end
				return
			end
			
			if not maximlevelCheck then
				say(gameforge.crystal_dungeon2._40_say)
say(string.format(gameforge.crystal_dungeon2._50_say, settings["level_check"]["maximum"]))
				say(gameforge.crystal_dungeon2._60_say)
				for i, name in next, notEnoughmaximLevelMembers, nil do
					say(string.format(gameforge.demon_dungeon._50_say, name))
				end
				return
			end
			
			say(gameforge.crystal_dungeon2._70_say)
			wait()
			crystal_dungeon.make_dungeon()
		end
		
		----
		----Dungeon start
		----
		
		when login with crystal_dungeon.crystalIsInDungeon() begin
			local get_flag = d.getf("crystal_level")
			local four_login = d.getf("level_four_login")
			local six_login = d.getf("level_six_login")
			
			if get_flag == 1 then
				say_title(gameforge.crystal_dungeon2._80_sayTitle)
				say(gameforge.crystal_dungeon2._90_say)
				if d.getf("metin_stone") == 0 then
					d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_1))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, d.getf("mobs"), 83))
				else
d.set_mission_message(string.format(gameforge.crystal_dungeon2._110_d_set_mission_message, c_mob_name(8411)))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, d.getf("mobs"), 83))
				end
			elseif get_flag == 2 then
				say_title(gameforge.crystal_dungeon2._120_sayTitle)
				say(gameforge.crystal_dungeon2._130_say)
				d.setf("crystal_seal", 0)
				d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_2))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, d.getf("mobs_2"), 73))

			elseif get_flag == 3 then
				say_title(gameforge.crystal_dungeon2._140_sayTitle)
				say(gameforge.crystal_dungeon2._150_say)
				say_title(gameforge.crystal_dungeon2._160_sayTitle)
				
				if d.getf("crystal_boss1") == 0 then
					d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_22))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, d.getf("mobs_3"), 105))
				else
					d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_3))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, 0, 1))
				end
				if four_login == 0 then
					d.setf("level_four_login", 1)
					d.setf("crystal_level", 3)
					clear_server_timer("level_four_timer", get_server_timer_arg())
					clear_server_timer("exit_before", get_server_timer_arg())
					server_timer("level_four_timer", 10 * 60, d.get_map_index())
				end
			elseif get_flag == 4 then
				say_title(gameforge.crystal_dungeon2._170_sayTitle)
				say(gameforge.crystal_dungeon2._180_say)
				say_title(gameforge.crystal_dungeon2._190_sayTitle)
				d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_4))
				--d.set_mission_sub_message("")

			elseif get_flag == 5 then
				say_title(gameforge.crystal_dungeon2._200_sayTitle)
				say(gameforge.crystal_dungeon2._210_say)
				say_title(gameforge.crystal_dungeon2._220_sayTitle)
				
				if d.getf("crystal_dino") == 0 then
					d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_5))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, d.getf("mobs_5"), 75))
				else
					d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_6))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, 0, 1))
				end
			elseif get_flag == 6 then
				say_title(gameforge.crystal_dungeon2._230_sayTitle)
				say(gameforge.crystal_dungeon2._240_say)
				say_title(gameforge.crystal_dungeon2._250_sayTitle)
				
				if d.getf("crystal_dragon") == 1 then
					d.set_mission_message((gameforge.crystal_dungeon2.set_mission_message_7))
				else
					d.set_mission_message((gameforge.crystal_dungeon2.set_mission_message_8))
				end
				
				if six_login == 0 then
					d.setf("level_six_login", 1)
					d.setf("crystal_level", 6)
					clear_server_timer("level_six_timer", get_server_timer_arg())
					clear_server_timer("exit_before2", get_server_timer_arg())
					server_timer("level_six_timer", 15 * 60, d.get_map_index())
				end
			else
				return
			end
		end
		

		
		when kill with npc.get_race() == 8411 and crystal_dungeon.crystalIsInDungeon() begin
			local count, check_metin1 = d.getf("metins") + 1, false;
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_level") == 1 then
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, count, 3)) 

				if count < 3 and check_metin1 == false then
					d.setf("metins", count)
				else
					if d.getf("metin_stone1") == 0 then
						d.clear_mission_message()
						d.notice(gameforge.crystal_dungeon2._260_dNotice)
						d.kill_all()
						d.setf("crystal_level", 2)
						d.setf("metin_stone1", 1)
						cleartimer("c_level_timer")
						timer("c_level_timer", 7)
					end
					check_metin1 = true
				end
			end
		end
		

		
		when 9210.take with item.vnum == 30701 and crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			local n = number (1,1000)			
			d.notice(gameforge.crystal_dungeon2._280_dNotice)
			d.clear_mission_message()
			d.kill_unique(9210)
			d.clear_regen()
			d.kill_all()
			d.setf("crystal_level", 3)
			pc.remove_item(setting["keys"][1], 1)
			cleartimer("c_level_timer")
			timer("c_level_timer", 12)
			
		end
		
		when kill with crystal_dungeon.crystalIsInDungeon() begin
			if (npc.get_race() == 944 or  npc.get_race() == 945 or  npc.get_race() == 946) and d.getf("crystal_level") == 5then
				local count, check_4 = d.getf("mobs_5") + 1, false;
				local setting = crystal_dungeon.settings()
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, count, 75)) 
						--75
				if count < 75 and check_4 == false then
					d.setf("mobs_5", count)
				else
					if d.getf("crystal_dino") == 0 then
						d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_10))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, 0, 1))
						d.clear_regen()
						d.kill_all()
						timer("crystal_dino", 10)
						d.setf("crystal_dino", 1)
					end
					check_4 = true
				end
			end
				
			if (npc.get_race() == 942 or npc.get_race() == 943 or npc.get_race() == 944) and d.getf("crystal_level") == 2 then
				local count, check_2 = d.getf("mobs_2") + 1, false;
				local setting = crystal_dungeon.settings()				
						--73
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, count, 73)) 

				if count < 73 and check_2 == false then
					d.setf("mobs_2", count)
				else
					if d.getf("crystal_seal") == 0 then
						d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_11))
						--d.set_mission_sub_message("")
						game.drop_item(setting["keys"][1], 1)
						d.setf("crystal_seal", 1)
					end
					check_2 = true
				end
			end
				
			if (npc.get_race() == 940 or npc.get_race() == 941 or npc.get_race() == 942) and d.getf("crystal_level") == 1 then
				local count, check = d.getf("mobs") + 1, false;
				local setting = crystal_dungeon.settings()
				
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, count, 83)) 

						--83
				if count < 83 and check == false then
					d.setf("mobs", count)
				else
					if d.getf("metin_stone") == 0 then
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, 0, 3)) 

						d.clear_regen()
						d.kill_all()
						d.spawn_mob(setting["metin_stones"][1], 261, 276)
						d.spawn_mob(setting["metin_stones"][1], 242, 303)
						d.spawn_mob(setting["metin_stones"][1], 293, 305)
d.notice(gameforge.crystal_dungeon2._290_dNotice, c_mob_name(8411))
						d.setf("metin_stone", 1)
					end
					check = true
				end
				
			end
		
			if (npc.get_race() == 940 or npc.get_race() == 943 or npc.get_race() == 944 or npc.get_race() == 945) and d.getf("crystal_level") == 3 then
				local count, check3 = d.getf("mobs_3") + 1, false;
			
						--105
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, count, 105)) 

				if count < 105 and check3 == false then
					d.setf("mobs_3", count)
				else
					if d.getf("crystal_boss1") == 0 then
						d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_12))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, 0, 1)) 

						d.kill_all()
						d.clear_regen()
						d.notice(gameforge.crystal_dungeon2._300_dNotice)
						timer("third_boss", 10)
						d.setf("third_boss", 1)
						d.setf("crystal_boss1", 1)
					end
					check3 = true
				end
			end
		end
		
		when 948.kill with crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_level") == 3 then
				d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_13))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, 1, 1)) 
				d.clear_regen()
				d.kill_all()
				d.setf("crystal_level", 4)				
				d.notice(gameforge.crystal_dungeon2._310_dNotice)
				cleartimer("level_four_timer")
				cleartimer("c_level_timer")
				timer("c_level_timer", 10)
			else
				return
			end
		end
		
		when 8412.kill with crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_level") == 4 then
				game.drop_item(setting["keys"][2],1)
			end				
		end
		
		when 8413.kill with crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_level") == 4 then
				game.drop_item(setting["keys"][3],1)
			end
		end
		
		when 9212.take with item.vnum == 30702 and crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			pc.remove_item(setting["keys"][2], 1)
			d.spawn_mob(9213, 405, 687)
			timer("wait_eff1", 2)
		end
		
		when 9212.take with item.vnum == 30703 and crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			pc.remove_item(setting["keys"][3], 1)
			d.spawn_mob(9213, 405, 687)
			timer("wait_eff2", 2)
		end
		

		
		when 949.kill with crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_level") == 5 then
				d.set_mission_message(string.format(gameforge.crystal_dungeon2.set_mission_message_14))
d.set_mission_sub_message(string.format(gameforge.crystal_dungeon2._100_d_set_mission_sub_message, 1, 1))
				d.clear_regen()
				d.kill_all()
				d.setf("crystal_level", 6)				
				d.notice(gameforge.crystal_dungeon2._320_dNotice)
				cleartimer("c_level_timer")
				timer("c_level_timer", 10)
			else
				return
			end
		end

		when 8414.kill with crystal_dungeon.crystalIsInDungeon() begin
			local count, check_5 = d.getf("metins_6") + 1, false;
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_level") == 6 then
				if count < 7 and check_5 == false then
					d.setf("metins_6", count)
				else
					if d.getf("crystal_dragon") == 0 then
						d.set_mission_message((gameforge.crystal_dungeon2.set_mission_message_15))
						d.notice(gameforge.crystal_dungeon2._330_dNotice)
						cleartimer("level_six_timer")
						cleartimer("c_level_timer")
						d.kill_all()
						d.setf("crystal_level", 7)
						d.setf("crystal_dragon", 1)
						cleartimer("c_level_timer")
						timer("c_level_timer", 7)
					end
					check_5 = true
				end
			end
		end

		when 950.kill with crystal_dungeon.crystalIsInDungeon() begin
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_level") == 7 then
				d.clear_mission_message()
				d.clear_regen()
				d.kill_all()
				d.notice(gameforge.crystal_dungeon2._340_dNotice)
				notice_all(gameforge.crystal_dungeon2._190_notice_all1 .. pc.get_name() .. gameforge.crystal_dungeon2._190_notice_all2) --- GLOBAL NOTICE
				if party.is_party() then
					highscore.register("crd_pt_time", get_global_time() - d.getf("start_time"), 0)
				else
					highscore.register("crd_time", get_global_time() - d.getf("start_time"), 0)
				end
			
				highscore.register("crystal_dungeon_time", get_global_time() - d.getf("start_time"), 0)
				pc.setqf("950_kills", pc.getqf("950_kills") +  1)	
				cleartimer("c_level_timer")
				server_timer("final", 3 * 60, d.get_map_index())
			else
				return
			end
		end

		---
		--- Dungeon end
		---
		
		---
		--- TIMERS
		---		
		when effect_purge.timer begin
			d.kill_unique(9211)
		end
		
		when third_boss.timer begin
			local setting = crystal_dungeon.settings()
			
			if d.getf("third_boss") == 1 then
				crystal_dungeon.spawn_boss(3)
				d.setf("third_boss", 0)
			else
				return
			end
		end
		
		when crystal_dino.timer begin
			local setting = crystal_dungeon.settings()
			
			if d.getf("crystal_dino") == 1 then
				crystal_dungeon.spawn_boss(5)
				d.notice(gameforge.crystal_dungeon2._360_dNotice, c_mob_name(949))
				d.setf("crystal_dino", 0)
			else
				return
			end
		end
		
		when wait_eff1.timer begin
			d.notice(gameforge.crystal_dungeon2._370_dNotice)
		end
		
		when wait_eff2.timer begin
			d.clear_mission_message()
			d.notice(gameforge.crystal_dungeon2._380_dNotice)
			d.kill_unique(9212)
			d.clear_regen()
			d.kill_all()
			d.setf("crystal_level", 5)
			cleartimer("c_level_timer")
			timer("c_level_timer", 12)
		end
		
		when level_four_timer.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.set_warp_location(41, 478, 738)
				d.notice(gameforge.demon_dungeon._210_dNotice)
				
				server_timer("exit_before", 10, get_server_timer_arg())
			end
		end
		
		when level_six_timer.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.set_warp_location(41, 478, 738)
				d.notice(gameforge.demon_dungeon._210_dNotice)
				
				server_timer("exit_before2", 10, get_server_timer_arg())
			end
		end

		when final.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.set_warp_location(41, 478, 738)
				d.notice(gameforge.crystal_dungeon2._390_dNotice)
				
				server_timer("final_warp", 10, get_server_timer_arg())
			end
		end

		when exit_before.server_timer begin
			if d.select(get_server_timer_arg()) then
				clear_server_timer("exit_before", get_server_timer_arg())
				clear_server_timer("final", get_server_timer_arg())
				clear_server_timer("level_four_timer", get_server_timer_arg())
				d.exit_all()
			end
		end
		
		when exit_before2.server_timer begin
			if d.select(get_server_timer_arg()) then
				clear_server_timer("exit_before2", get_server_timer_arg())
				clear_server_timer("final", get_server_timer_arg())
				clear_server_timer("level_six_timer", get_server_timer_arg())
				d.exit_all()
			end
		end
		
		when final_warp.server_timer begin
			if d.select(get_server_timer_arg()) then
				clear_server_timer("final_warp", get_server_timer_arg())
				clear_server_timer("final", get_server_timer_arg())
				clear_server_timer("final", get_server_timer_arg())
				d.exit_all()
			end
		end
		
		when c_level_timer.timer begin
			local get_flag = d.getf("crystal_level")
			local setting = crystal_dungeon.settings()
			
			d.clear_mission_message()
			d.clear_regen()
			d.kill_all()
			
			if get_flag == 2 then
				d.jump_all(setting["spawm_pos"][2][1], setting["spawm_pos"][2][2])
				crystal_dungeon.get_regens(2)
				d.spawn_mob(9210, 753, 172)
				d.setf("crystal_level", 2)
			end
			if get_flag == 3 then
				d.jump_all(setting["spawm_pos"][2][1], setting["spawm_pos"][2][2])
				crystal_dungeon.get_regens(3)
				d.setf("crystal_level", 3)
			end
			if get_flag == 4 then
				d.jump_all(setting["spawm_pos"][3][1], setting["spawm_pos"][3][2])
				crystal_dungeon.get_regens(4)
				d.setf("crystal_level", 4)
				crystal_dungeon.spawn_metins_4thfloor()
			end
			if get_flag == 5 then
				d.jump_all(setting["spawm_pos"][4][1], setting["spawm_pos"][4][2])
				crystal_dungeon.get_regens(5)
				d.setf("crystal_level", 5)
			end
			if get_flag == 6 then
				d.jump_all(setting["spawm_pos"][5][1], setting["spawm_pos"][5][2])
				crystal_dungeon.get_regens(6)
				crystal_dungeon.get_regens(7)
				d.setf("crystal_level", 6)
			end
			if get_flag == 7 then
				crystal_dungeon.spawn_boss(7)
				d.setf("crystal_level", 7)
			end
		end
		
	end
end
