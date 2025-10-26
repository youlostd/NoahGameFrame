quest slime_dungeon_3 begin
	state start begin
	--QUEST FUNCTIONS
		function settings()
			return
			{
				["slime_dung_index_3"] = 242,
				["slime_dung_warp"] = {17356, 1277},
				--["owl_map_index_out"] = 75,
				--["out_pos"] = {196, 143},
				["level_check"] = {
					["minimum"] = 150,
					["maximum"] = 150
				},
				["ticket"] = 71175,
				["slime_metin"] = 28430,
				["slime_metin_pos"] = {245, 256},
				["keys"] = {30721, 30722},
				["slime_queen"] = 2768,
				["slime_queen_pos"] = {285, 260},
			};
		end
		
		function get_regens(level)
			local regens = {
				[1] = "data/dungeon/slime_cave3/regen_1.txt",
				[2] = "data/dungeon/slime_cave3/regen_2.txt",
				[3] = "data/dungeon/slime_cave3/regen_3.txt"};
			return d.set_regen_file(regens[level])
		end

		function party_get_member_pids()
			local pids = {party.get_member_pids()}
			
			return pids
		end
		
		function is_slimed_3()
			local pMapIndex = pc.get_map_index();
			local data = slime_dungeon_3.settings();
			local map_index = data["slime_dung_index_3"];

			return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
		end
		
		function clear_slimedungeon()
			d.clear_regen();
			d.kill_all();
		end
		
		function check_enter()
			addimage(25, 10, "slime_dungeon.tga")
			addimage(230, 150, "thurang.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9236))
			say("")
			local settings = slime_dungeon_3.settings()

			if party.is_party() and not party.is_leader() then
				say(gameforge.crystal_dungeon2._20_say)
				return
			end
			
			if party.is_party() and party.get_near_count() < 2 then
				say(gameforge.crystal_dungeon2._30_say)
				return
			end
			--if party.get_near_count() < 1 then
			--	say_reward("Your group must have atleast 2 players and")
			--	say_reward("those players have to be around.")
			--	say_reward("Otherwise i can not let you there. ")
			--	return false;
			--end
			
			local pids = {party.get_member_pids()}
			local passcheck = true
			local Levelcheck = true
			local notEnoughLevelMembers = {}
			local notEnoughLevelMaxMembers = {}
			local notEnoughTicketMembers = {}

			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() < settings["level_check"]["minimum"] then
					table.insert(notEnoughLevelMembers, pc.get_name())
					Levelcheck = false
				end

				q.end_other_pc_block()
			end

			if not Levelcheck then
				say_reward("If you want to enter Slime queen nest,")
				say_reward("every each member must have atleast level 40.")
				say("")
				say_reward("These members has not required level: ")
				for i, n in next, notEnoughLevelMembers, nil do
					say_title("- "..n)
				end
				return
			end
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() > settings["level_check"]["maximum"] then
					table.insert(notEnoughLevelMaxMembers, pc.get_name())
					Levelcheck = false
				end

				q.end_other_pc_block()
			end

			if not Levelcheck then
				say_reward("If you want to enter the Slime queen nest,")
				say_reward("every each member must have maximum level 70.")
				say("")
				say_reward("Next members do not have enough level:")
				for i, n in next, notEnoughLevelMaxMembers, nil do
					say_title("- "..n)
				end
				return
			end
			
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.count_item(settings.ticket) < 1 then
					table.insert(notEnoughTicketMembers, pc.get_name())
					passcheck = false
				end

				q.end_other_pc_block()
			end

			if not passcheck then
				say("If you want to enter the Slime queen nest,")
				say(string.format("every each member must have: %s.", c_item_name(settings.ticket)))
				say("")
				say("These members don't have the ticket:")
				for i, n in next, notEnoughTicketMembers, nil do
					say_reward("- "..n)
				end
				return
			end

			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				pc.remove_item(settings.ticket, 1)
				q.end_other_pc_block()
			end

			say("After you press continue,")
			say("every each memeber of the group will be teleported!")
			say("Be careful! All of you!")
			say("")
			say_reward("You have only 15 minutes to finish")
			say_reward("whole dungeon!")
			say_reward("Be fast!")
			wait()
			slime_dungeon_3.create_dungeon()
			d.add_boss_vnum(2768)
		end
				
		function create_dungeon()
			local settings = slime_dungeon_3.settings()
			if party.is_party() then
				d.new_jump_party(settings["slime_dung_index_3"], settings["slime_dung_warp"][1], settings["slime_dung_warp"][2])
				pc.setqf("dungeon_start_time", get_global_time())
			else
				d.new_jump(settings["slime_dung_index_3"], settings["slime_dung_warp"][1], settings["slime_dung_warp"][2])
				pc.setqf("dungeon_start_time", get_global_time())
			end

			d.setf("slimedung_level", 1)
			d.spawn_mob(settings["slime_metin"], settings["slime_metin_pos"][1], settings["slime_metin_pos"][2])
			server_timer("slime_dungeon_3_15minutesleft", 15*60, d.get_map_index())
		end
		--FUNCTIONS END

		--DUNGEON CHECK
		
		when 70007.use with pc.in_dungeon(242) begin
			pc.warp(167, 26400, 18400)
		end
		
		when login begin
			local indx = pc.get_map_index()
			local settings = slime_dungeon_3.settings()
						
			if indx == settings["slime_dung_index_3"] then
				if not pc.in_dungeon() then
					warp_to_village()
				end
			end
		end
		
		--MAP ENTER
		--DUNGEON ENTER
		when 29236.chat."Anime-Dungeon (150)"  with not slime_dungeon_3.is_slimed_3() begin
			local settings = slime_dungeon_3.settings()
			addimage(25, 10, "slime_dungeon.tga")
			addimage(230, 150, "thurang.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9236))
			say("")
			say("Hi warrior!")
			say("I'm Thu-Rang, guardian of Slime queen")
			say("nest. I'm responsible that slime")
			say("slime monsters are not coming out from")
			say("the nest. Sometimes there are just too")
			say("many of them and we need some warriors")
			say("to kill them.")
			wait()
			addimage(25, 10, "slime_dungeon.tga")
			addimage(230, 150, "thurang.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9236))
			say("")
			say("Are you brave enough?")
			say("I hope that yes, Slime queen is")
			say("strong bitch.")
			say("")
			say_title("Do you really want to enter this place? ")
			if (select ("Yes", "No") == 1) then
				slime_dungeon_3.check_enter()
			end
		end
		
		when kill with slime_dungeon_3.is_slimed_3() and not npc.is_pc() and npc.get_race() == 28430 and d.getf("slimedung_level") == 1 begin
			d.notice("Slime queen nest: You succesfuly destroyed the metin!")
			d.notice("Slime queen nest: Monsters are coming!!")
			d.notice("Slime queen nest: Kill them all!")
			d.setf("slimedung_level", 2)
			timer("slime_wave_spawn_3", 1)
		end
		
		when 9237.chat."Open the stone" with slime_dungeon_3.is_slimed_3()  and d.getf("slimedung_level") == 3 begin
		
		if party.is_party() and not party.is_leader() then
				say(gameforge.crystal_dungeon2._20_say)
				return
			end
			
			local settings = slime_dungeon_3.settings()
			addimage(25, 10, "slime_dungeon.tga")
			say("")
			say("")
			say("")
			say("This stone has some magical power here.")
			say("It's a little hole right here.")
			say("I think we have to put some slime into")
			say("it and block the hole.")
			say("Oh yeah!")
			wait()
			d.setf("slimedung_level", 4)
			timer("slime_wave_spawn_3", 1)
		end
		
		when kill with slime_dungeon_3.is_slimed_3() and not npc.is_pc() and npc.get_race() == 28430 and d.getf("slimedung_level") == 4 begin
			local settings = slime_dungeon_3.settings()
			if d.getf("slime_stone") == 1 then
				d.notice("Slime queen nest: 3 metin stones left!")
				d.setf("slime_stone", 2)
			elseif d.getf("slime_stone") == 2 then
				d.notice("Slime queen nest: 2 metin stones left!")
				d.setf("slime_stone", 3)
			elseif d.getf("slime_stone") == 3 then
				d.notice("Slime queen nest: 1 metin stone left!")
				d.setf("slime_stone", 4)
			elseif d.getf("slime_stone") == 4 then
				d.notice("Slime queen nest: You've destroyed all metin stones.")
				d.notice("Slime queen nest: Pick up the phial and take some slime!")
				d.setf("slime_stone", 0)
				d.setf("slimedung_level", 5)
				game.drop_item(settings["keys"][1], 1)
			end
		end
		
		when 9238.take with item.get_vnum() == 30721 and slime_dungeon_3.is_slimed_3() and d.getf("slimedung_level") == 5 begin
			local settings = slime_dungeon_3.settings()
			pc.remove_item(settings["keys"][1], 1)
			pc.give_item2(settings["keys"][2], 1)
			npc.purge()
			d.notice("Slime queen nest: Now put the slime into the seal stone!!")
			d.setf("slimedung_level", 6)
		end
		
		when 9237.take with item.get_vnum() == 30722 and slime_dungeon_3.is_slimed_3() and d.getf("slimedung_level") == 6 begin
			local settings = slime_dungeon_3.settings()
			pc.remove_item(settings["keys"][2], 1)
			npc.kill(9237)
			slime_dungeon_3.clear_slimedungeon()
			d.notice("Slime queen nest: The evil power is gone!")
			d.notice("Slime queen nest: Monsters are coming!!")
			d.setf("slimedung_level", 8)
			timer("slime_wave_spawn_3", 1)
		end
		
		when kill with slime_dungeon_3.is_slimed_3() and not npc.is_pc() and npc.get_race() == 2768 and d.getf("slimedung_level") == 8 begin
			slime_dungeon_3.clear_slimedungeon()
			d.notice("Slime queen nest: You succesfully finished the dungeon!")
			d.notice("Slime queen nest: You will be teleported in 2 minutes")
			d.notice("Slime queen nest: out of dungeon.")
			pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
			if math.mod(pc.getqf("boss_kills"), 1) == 0 then
				--notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(4158), pc.getqf("boss_kills"))
				notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(2768), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
				--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
			end
			dungeon_info.set_ranking(DUNGEON_ID_SLIME_3, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(npc.get_race()))
			pc.update_dungeon_progress(DUNGEON_ID_SLIME_3)
			d.completed()
			server_timer("slime_dungeon_3_is_done", 30, d.get_map_index())
			d.setf("slimedung_level", 9)
		end

		-- SPAWN OR NOTICE TIMER
		when slime_wave_spawn_3.timer begin
			local settings = slime_dungeon_3.settings()
			if d.getf("slimedung_level") == 2 then
				slime_dungeon_3.get_regens(1)
				loop_timer("slime_wave_kill", 5);
			elseif d.getf("slimedung_level") == 4 then
				d.setf("slime_stone", 1)
				slime_dungeon_3.get_regens(3)
				d.notice("Slime queen nest: Destroy all metinstones and find a phial")
				d.notice("Slime queen nest: resist against this slime!")
			--elseif d.getf("slimedung_level") == 7 then
			--	slime_dungeon.get_regens(1)
			--	loop_timer("slime_wave_kill", 15);
			elseif d.getf("slimedung_level") == 8 then
				d.spawn_mob(settings["slime_queen"], settings["slime_queen_pos"][1], settings["slime_queen_pos"][2])
			end
		end

		-- LOOP TIMER FOR KILLING MONSTERS
		when slime_wave_kill.timer begin
			local settings = slime_dungeon_3.settings()
			if d.getf("slimedung_level") == 2 then
				if (d.count_monster() == 0) then
					slime_dungeon_3.clear_slimedungeon()
					--cleartimer("wave_kill")
					d.setf("slimedung_level", 3)
					d.notice("Slime queen nest: You have killed all monsters!");
					d.notice("Slime queen nest: A seal stone just appeared!");
					d.notice("Slime queen nest: Let's check it out!");
					slime_dungeon_3.get_regens(2)
				else
					d.notice(string.format("Slime queen nest: You still have to kill %d mobs to move on.", d.count_monster()));
				end
			--elseif d.getf("slimedung_level") == 7 then
			--	if (d.count_monster() == 0) then
			--		cleartimer("wave_kill2");
			--		d.setf("slimedung_level", 8)
			--		d.notice("Slime queen nest: You have killed all monsters!");
			--		d.notice("Slime queen nest: A slime queen is coming!!!");
			--		timer("slime_wave_spawn_3", 5)
			--	else
			--		d.notice(string.format("Slime queen nest: You still have to kill %d mobs to move on.", d.count_monster()));
			--	end
			end
		end
		
		when slime_dungeon_3_15minutesleft.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Slime queen nest: !!!!! ONLY 10 MINUTES LEFT!!!!!")
				server_timer("slime_dungeon_3_10minutesleft", 10*60, d.get_map_index())
			end
		end
		
		when slime_dungeon_3_10minutesleft.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Slime queen nest: !!!!! ONLY 5 MINUTES LEFT!!!!!")
				d.notice("Slime queen nest: You're running out of time!")
				server_timer("slime_dungeon_3_5minutesleft", 5*60, d.get_map_index())
			end
		end
		
		when slime_dungeon_3_5minutesleft.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Slime queen nest: !!!!! ONLY 1 MINUTES LEFT!!!!!")
				d.notice("Slime queen nest: You are almost failed!")
				server_timer("slime_dungeon_3_1minutesleft", 60, d.get_map_index())
			end
		end
		
		when slime_dungeon_3_1minutesleft.server_timer begin
			if d.select(get_server_timer_arg()) then
				server_timer("slime_dungeon_3_is_done", 1, d.get_map_index())
			end
		end
		
		when slime_dungeon_3_is_done.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Slime queen nest: You will be teleported out of dungeon!")
				slime_dungeon_3.clear_slimedungeon()
				d.set_warp_location(167, 264, 184)
			end
			
			server_timer("final_exit_slime", 2, d.get_map_index())
		end
		
		when final_exit_slime.server_timer begin
			if d.select(get_server_timer_arg()) then								
				d.exit_all()
			end
		end									
	end
end

