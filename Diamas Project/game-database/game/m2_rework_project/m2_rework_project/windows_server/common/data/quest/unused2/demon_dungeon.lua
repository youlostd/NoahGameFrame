quest demon_dungeon begin
	state start begin
		function settings()
			return
			{
				["map_index"] = 210,
				["ticket_vnum"] = 0,
				["entry_pos"] = {332, 266},
				["second_floor"] = {733, 523},
				["third_floor"] = {853, 853},
				["forth_floor"] = {386, 813},
				["first_boss"] = 274,
				["second_boss"] = 278,
				["third_boss"] = 280,
				["forth_boss"] = 283,
				["boss_pos_1"] = {356, 260},
				["boss_pos_2"] = {662, 498},
				["boss_pos_3"] = {718, 858},
				["boss_pos_4"] = {319, 886},
				["minimum_level"] = 20,
				["maximum"] = 55,
				["regen"] = "data/dungeon/demon_dungeon/regen_1thfloor.txt",
				["regen_2"] = "data/dungeon/demon_dungeon/regen_2thfloor.txt",
				["regen_3"] = "data/dungeon/demon_dungeon/regen_3thfloor.txt",
				["regen_4"] = "data/dungeon/demon_dungeon/regen_4thfloor.txt",
			}
		end
		
		function enter_dungeon()
			local setting = demon_dungeon.settings()
			
			clear_server_timer("exit_almost_done_demon", get_server_timer_arg())
			clear_server_timer("exit_done_demon", get_server_timer_arg())
			if party.is_party() then
				d.new_jump_party(setting.map_index, setting.entry_pos[1], setting.entry_pos[2])
			else
				d.new_jump(setting.map_index, setting.entry_pos[1], setting.entry_pos[2])
			end
			server_timer("exit_almost_done_demon", 10*60, d.get_map_index())

			--d.setf("group_timer", get_global_time(), 30 * 60)
			--d.setf("start_time", get_global_time())

			d.set_regen_file(setting.regen)
			d.setf("stage", 1)
		end
		
		function clear_dungeon()
			d.notice(gameforge.demon_dungeon._10_dNotice)
			
			d.clear_regen()
			d.kill_all()
			cleartimer("upp_level")
			timer("upp_level", 1)
		end
		
		function check_dungeon()
			say_title(c_mob_name(20599))
			local settings = demon_dungeon.settings()

			if party.is_party() and not party.is_leader() then
				say(gameforge.demon_dungeon._20_say)
				return
			end

			if party.is_party() and party.get_near_count() < 2 then
				say(gameforge.demon_dungeon._30_say)
				return
			end

			local levelCheck = true
			local ticketCheck = true
			local notEnoughLevelMembers = {}
			local notEnoughLevelMaxMembers = {}
			local notEnoughTicketMembers = {}
			local pids = {party.get_member_pids()}

			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() < settings.minimum_level then
					table.insert(notEnoughLevelMembers, pc.get_name())
					levelCheck = false
				end

				q.end_other_pc_block()
			end

			if not levelCheck then
				say(gameforge.demon_dungeon._40_say)
				for i, n in next, notEnoughLevelMembers, nil do
					say(string.format(gameforge.demon_dungeon._50_say, n))
				end
				return
			end
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() > settings.maximum then
					table.insert(notEnoughLevelMaxMembers, pc.get_name())
					levelCheck = false
				end

				q.end_other_pc_block()
			end

			if not levelCheck then
				say(gameforge.demon_dungeon._40_say)
				for i, n in next, notEnoughLevelMaxMembers, nil do
					say(string.format(gameforge.demon_dungeon._50_say, n))
				end
				return
			end
			
			if settings.ticket_vnum then
				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					if pc.count_item(settings.ticket_vnum) < 1 then
						table.insert(notEnoughTicketMembers, pc.get_name())
						ticketCheck = false
					end

					q.end_other_pc_block()
				end

				if not ticketCheck then
					say(gameforge.demon_dungeon._60_say)
	say(string.format(gameforge.demon_dungeon._70_say, c_item_name(settings.ticket_vnum)))
					say(gameforge.demon_dungeon._80_say)
					for i, n in next, notEnoughTicketMembers, nil do
						say(string.format(gameforge.demon_dungeon._50_say, n))
					end
					return
				end

				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					pc.remove_item(settings.ticket_vnum, 1)
					q.end_other_pc_block()
				end
			end
			
			say(gameforge.demon_dungeon._90_say)
			wait()
			demon_dungeon.enter_dungeon()
		end
		
		-- FIRST STAGE --
		
		when login  begin
			local settings = demon_dungeon.settings()
			if pc.get_map_index() == settings["map_index"] then
				if not pc.in_dungeon() then
						warp_to_village()
					end
				end
			end	
		
		when 273.kill begin
			local count_kill_for_key, count = 6, d.getf("count") + 1
			
			if d.getf("stage_key_done") == 0 then
				d.setf("count", count)
			end
			
			if count == count_kill_for_key then
				game.drop_item(50084, 1)
				d.setf("count", 0)
				d.setf("stage_key_done", 1)
			end
		end
		
		when 50084.use with d.getf("stage") == 1 begin
			local settings = demon_dungeon.settings()
			
			if d.getf("used_first_chey") >= 1 then
				syschat(gameforge.demon_dungeon._100_chat)
			else
			
				pc.remove_item(50084)
				
				d.spawn_mob(settings.first_boss, settings.boss_pos_1[1], settings.boss_pos_1[2])
				d.setf("used_first_chey", 1)
				d.notice(gameforge.demon_dungeon._110_dNotice)
			end
		end
		
		when 274.kill with d.getf("stage") == 1 begin
		
			d.notice(gameforge.demon_dungeon._120_dNotice)
			demon_dungeon.clear_dungeon()
		end
		
		-- SECOND STAGE --
		
		when 277.kill begin
			local count_kill_for_key2, count2 = 5, d.getf("count2") + 1
			
			if d.getf("stage_key_done2") == 0 then
				d.setf("count2", count2)
			end
			
			if count2 == count_kill_for_key2 then
				game.drop_item(50084, 1)
				d.setf("count2", 0)
				d.setf("stage_key_done2", 1)
			end
		end
		
		when 50084.use with d.getf("stage") == 2 begin
			local settings = demon_dungeon.settings()
			
			if d.getf("used_first_chey2") >= 1 then
				syschat(gameforge.demon_dungeon._100_chat)
			else
			
				pc.remove_item(50084)
				
				d.spawn_mob(settings.second_boss, settings.boss_pos_2[1], settings.boss_pos_2[2])
				d.setf("used_first_chey2", 1)
				d.notice(gameforge.demon_dungeon._130_dNotice)
			end
		end
		
		when 278.kill with d.getf("stage") == 2 begin
		
			d.notice(gameforge.demon_dungeon._140_dNotice)
			demon_dungeon.clear_dungeon()
		end
		
		-- THIRD STAGE --
		
		when 279.kill begin
			local count_kill_for_key3, count3 = 4, d.getf("count3") + 1
			
			if d.getf("stage_key_done3") == 0 then
				d.setf("count3", count3)
			end
			
			if count3 == count_kill_for_key3 then
				game.drop_item(50084, 1)
				d.setf("count3", 0)
				d.setf("stage_key_done3", 1)
			end
		end
		
		when 50084.use with d.getf("stage") == 3 begin
			local settings = demon_dungeon.settings()
			
			if d.getf("used_first_chey3") >= 1 then
				syschat(gameforge.demon_dungeon._100_chat)
			else
			
				pc.remove_item(50084)
				
				d.spawn_mob(settings.third_boss, settings.boss_pos_3[1], settings.boss_pos_3[2])
				d.setf("used_first_chey3", 1)
				d.notice(gameforge.demon_dungeon._150_dNotice)
			end
		end
		
		when 280.kill with d.getf("stage") == 3 begin
		
			d.notice(gameforge.demon_dungeon._160_dNotice)
			demon_dungeon.clear_dungeon()
		end
		
		-- FORTH STAGE --
		
		when 281.kill or 282.kill begin
			local count_kill_for_key4, count4 = 20, d.getf("count4") + 1
			
			if d.getf("stage_key_done4") == 0 then
				d.setf("count4", count4)
			end
			
			if count4 == count_kill_for_key4 then
				game.drop_item(50084, 1)
				d.setf("count4", 0)
				d.setf("stage_key_done4", 1)
			end
		end
		
		when 50084.use with d.getf("stage") == 4 begin
			local settings = demon_dungeon.settings()
			
			if d.getf("used_first_chey4") >= 1 then
				syschat(gameforge.demon_dungeon._100_chat)
			else
			
				pc.remove_item(50084)
				
				d.spawn_mob(settings.forth_boss, settings.boss_pos_4[1], settings.boss_pos_4[2])
				d.setf("used_first_chey4", 1)
				d.notice(gameforge.demon_dungeon._170_dNotice)
			end
		end
		
		when 283.kill with d.getf("stage") == 4 begin
		
			cleartimer("demon_dungeon_exit_3")
			cleartimer("demon_dungeon_exit")
			timer("demon_dungeon_exit", 10)
			d.kill_all()

			--if party.is_party() then
			--	highscore.register("dem_pt_time", get_global_time() - d.getf("start_time"), 0)
			--else
			--	highscore.register("dem_time", get_global_time() - d.getf("start_time"), 0)
			--end

			--pc.setqf("283_kills", pc.getqf("283_kills") +  1)		
			
			d.notice(gameforge.demon_dungeon._180_dNotice)
			notice_all(gameforge.demon_dungeon._190_notice_all1 .. pc.get_name() .. gameforge.demon_dungeon._190_notice_all2) --- GLOBAL NOTICE
		end
		
		when logout with pc.in_dungeon() begin
			if pc.count_item(50084) >= 1 then
				pc.remove_item(50084, pc.count_item(50084))
			end
		end
		
		-- TEST ZONE -- 
		
		when 71175.use begin
			local count_key_1 = d.getf("count")
			local count_key_2 = d.getf("count2")
			local count_key_3 = d.getf("count3")
			local count_key_4 = d.getf("count4")
			local stage = d.getf("stage")
			
			syschat(string.format(gameforge.demon_dungeon._200_chat, count_key_1.." [ENTER]Second stage count "..count_key_2.." [ENTER]Third stage count "..count_key_3.." [ENTER]Forth stage count "..count_key_4.." [ENTER]Stage counter "..stage))
		end
		-- Start Timers --
		
		when exit_almost_done_demon.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.set_warp_location(161, 404, 703)

				d.notice(gameforge.demon_dungeon._210_dNotice)
				server_timer("exit_done_demon", 3, get_server_timer_arg())
			end
		end

		when exit_done_demon.server_timer begin
			if d.select(get_server_timer_arg()) then
				clear_server_timer("exit_done_demon", get_server_timer_arg())
				clear_server_timer("exit_almost_done_demon", get_server_timer_arg())
				d.exit_all()
			end
		end
		
		when demon_dungeon_exit.timer begin
			local index = pc.get_map_index()
			
			if index >= (210 * 10000) and index < (210+1 * 10000) then
				d.notice(gameforge.demon_dungeon._220_dNotice)
				timer("demon_dungeon_exit_3", 3)
			end
		end
			
		when demon_dungeon_exit_3.timer begin
			local index = pc.get_map_index()
			
			if index >= (210 * 10000) and index < (210+1 * 10000) then
				d.exit_all()
			end
		end
		
		when upp_level.timer begin
			local stage, settings = d.getf("stage"), demon_dungeon.settings()
			
			if stage == 1 then
				d.setf("stage", 2)
				d.set_regen_file(settings.regen_2)
				d.jump_all(settings.second_floor[1] ,settings.second_floor[2])
				notice_multiline(gameforge.demon_dungeon.multiline_01,d.notice)
				notice_multiline(gameforge.demon_dungeon.multiline_02,d.notice)
			end
			
			if stage == 2 then
				d.set_regen_file(settings.regen_3)
				d.setf("stage", 3)
				d.jump_all(settings.third_floor[1] ,settings.third_floor[2])
				notice_multiline(gameforge.demon_dungeon.multiline_03,d.notice)
				notice_multiline(gameforge.demon_dungeon.multiline_04,d.notice)
			end
			
			if stage == 3 then 
				d.set_regen_file(settings.regen_4)
				d.setf("stage", 4)
				d.jump_all(settings.forth_floor[1] ,settings.forth_floor[2])
				notice_multiline(gameforge.demon_dungeon.multiline_05,d.notice)
				notice_multiline(gameforge.demon_dungeon.multiline_06,d.notice)
			end
		end
			
		-- END TIMERS --

		when 20599.chat.gameforge.demon_dungeon._230_npcChat begin
			local setting = demon_dungeon.settings()
			
			say_title(c_mob_name(20599))
			say(gameforge.demon_dungeon._240_say)
say_reward(string.format(gameforge.demon_dungeon._250_sayReward, setting.minimum_level))
say_reward(string.format(gameforge.demon_dungeon._260_sayReward, setting.maximum))
say_reward(string.format(gameforge.demon_dungeon._270_sayReward, c_item_name(setting.ticket_vnum)))
			say_reward(gameforge.demon_dungeon._280_sayReward)
			wait()
			say_title(c_mob_name(20599))
			say(gameforge.demon_dungeon._290_say)
			if select(gameforge.demon_dungeon._300_select, gameforge.demon_dungeon._310_select) == 1 then
				demon_dungeon.check_dungeon()
			end
		end
	end
end
