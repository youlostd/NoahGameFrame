define LADY_VALENTINE 9229
define LADY_VALENTINE_GHOST 9233
define RHODOLITE_ORE 9230
define RHODOLITE 9232
define MONUMENT_OF_LOVE 9231
define METIN_OF_LOVE 8428
define UNKNOWN_HERB 9234
define DRYADE 9235
define CHOCOLATE_METIN 8429
define VALENTINE_TREE 765

quest valentine2019_dungeon begin
	state start begin
	--QUEST FUNCTIONS
		function settings()
			return
			{
				["minimum_level"] = 150,
				["map_limit_enter_time"] = 0,
				["valentine_map_index"] = 506,
				["valentine_map_index_out"] = 75,
				["out_pos"] = {235, 173},
				["level_pos"] = {
					[1] = {146, 180},
					[2] = {362, 235}
				},
				["ticket"] = 30714, 
				["keys"] = {30715, 30716, 30717, 30718, 30719, 30720},
				-- First floor
				["lady_val_pos"] = {
					[1] = {153, 198},
					[2] = {145, 225},
					[3] = {361, 244}
				},
				["monument_ol_pos"] = {146, 227},
				["dryade_pos"] = {392, 247},
				["rore_pos"] = {360, 305},
				["choc_metin_pos"] = {383, 277},
				["val_tree_pos"] = {364, 301},
				["unknown_herb_pos"] = {
					[1] = {156, 205},
					[2] = {130, 205},
					[3] = {122, 218},
					[4] = {141, 231},
					[5] = {172, 247},
					[6] = {190, 253},
					[7] = {200, 270},
					[8] = {179, 295},
					[9] = {121, 301},
					[10] = {102, 294},
					[11] = {125, 272},
					[12] = {140, 260},
					[13] = {91, 260}
				},
			};
		end
		
		function get_regens(level)
			local regens = {
				[1] = "data/dungeon/valentine2019/regen_1.txt",
				[2] = "data/dungeon/valentine2019/regen_2.txt",
				[3] = "data/dungeon/valentine2019/regen_3.txt",
				[4] = "data/dungeon/valentine2019/regen_4.txt"
				};
			return d.set_regen_file(regens[level])
		end

		function party_get_member_pids()
			local pids = {party.get_member_pids()}
			
			return pids
		end
		
		function is_active()
			local pMapIndex = pc.get_map_index();
			local data = valentine2019_dungeon.settings();
			local map_index = data["valentine_map_index"];

			return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
		end
		
		function clear_valchasm()
			d.clear_regen();
			d.kill_all();
		end
		
		function check_enter()
			addimage(25, 10, "v2019_01.tga")
			addimage(230, 140, "lady_valentine.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9229))
			local settings = valentine2019_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say_reward("Let me talk with your leader first.")
				return
			end

			if party.is_party() and party.get_near_count() < 2 then
				say_reward("Your group must have atleast 2 players! ")
				say_reward("Otherwise I can not let you go there. ")
				return false;
			end
			
			local levelCheck = true
			local ticketCheck = true
			local enterCheck = true
			local notEnoughLevelMembers = {}
			local notEnoughTicketMembers = {}
			local enterCheckpass = {}
			local pids = {party.get_member_pids()}

			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.get_level() < settings["minimum_level"] then
					table.insert(notEnoughLevelMembers, pc.get_name())
					levelCheck = false
				end

				q.end_other_pc_block()
			end

			if not levelCheck then
				say_reward("If you want to enter the chasm,")
				say_reward("every each member must have atleast level 40.")
				say("")
				say_reward("These members has not required level: ")
				for i, n in next, notEnoughLevelMembers, nil do
					say_reward("- "..n)
				end
				return
			end
			
			if not enterCheck then
				say("Some members of the group still have to wait...")
				for i, n in next, enterCheckpass, nil do
					say_reward("- "..n)
				end
				return
			end
						
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
				if pc.count_item(settings.ticket) < 1 then
					table.insert(notEnoughTicketMembers, pc.get_name())
					ticketCheck = false
				end

				q.end_other_pc_block()
			end

			if not ticketCheck then
				local settings = valentine2019_dungeon.settings()
				say("If you want to enter the chasm,")
				say("every each member must have: ")
				say_item("Valentine chasm pass", settings.ticket, "")
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
			say_reward("You have 25 minutes to finish")
			say_reward("whole dungeon!")
			wait()
			valentine2019_dungeon.create_dungeon()
		end
				
		function create_dungeon()
			local settings = valentine2019_dungeon.settings()
			
			if party.is_party() then
				d.new_jump_party(settings["valentine_map_index"], settings["level_pos"][1][1], settings["level_pos"][1][2])
			else
				d.new_jump(settings["valentine_map_index"], settings["level_pos"][1][1], settings["level_pos"][1][2])
			end
			d.setf("v2019_floor", 1)
			d.spawn_mob(LADY_VALENTINE_GHOST, settings["lady_val_pos"][1][1], settings["lady_val_pos"][1][2])
			server_timer("valentine2019_dungeon_25min_left", 10*60, d.get_map_index())
		end
		
		--FUNCTIONS END
		
		--LOGIN IN MAP
		when login begin
			local indx = pc.get_map_index()
			local settings = valentine2019_dungeon.settings()
						
			if indx == settings["valentine_map_index"] then
				if not pc.in_dungeon() then
					warp_to_village()
				end
			end
		end

		--DUNGEON ENTER
		when LADY_VALENTINE.chat."Valentine chasm"  with not valentine2019_dungeon.is_active() begin
			local settings = valentine2019_dungeon.settings()
			addimage(25, 10, "v2019_01.tga")
			addimage(230, 140, "lady_valentine.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9229))
			say("")
			say("Are you asking what is this place?")
			say("Well, once upon a Valentine night,")
			say("a big tree came to our city.")
			say("He told us he hate Valentine's day.")
			say("Nothin else. And since then, every")
			say("Valentine night here is a portal")
			say("into Valentine chasm.")
			wait()
			addimage(25, 10, "v2019_01.tga")
			addimage(230, 140, "lady_valentine.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9229))
			say("")
			say("I'm the only person who knows how to")
			say("get there.")
			say("Are you sure you want to visit this place?")
			say("Its not a friendly place.")
			say("")
			say_reward("Do you want to go there?")
			if (select ("Yes", "No") == 1) then
				if ((get_global_time() - pc.getf('valentine2019_dungeon','exit_v2019dungeon_time')) < 0) then
				
					local remaining_wait_time = (pc.getf('valentine2019_dungeon','exit_v2019dungeon_time') - get_global_time() + 0)
					addimage(25, 10, "v2019_01.tga")
					addimage(230, 140, "lady_valentine.tga")
					say("")
					say("")
					say("")				
					say_title(c_mob_name(9229))
					say("You have to wait until you can enter the dungeon again")
					say("You can go there again in: "..get_time_remaining(remaining_wait_time)..'[ENTER]')
					return
				else
				valentine2019_dungeon.check_enter()
				end
			end
		end
		
		when LADY_VALENTINE.chat.'Time reset' with pc.is_gm() begin
			say('Do you want to reset the enter time?[ENTER]')
			if select('Reset time','Close') == 2 then return end
				addimage(25, 10, "v2019_01.tga")
				addimage(230, 140, "lady_valentine.tga")
				say("")
				say("")
				say("")
				say_title(c_mob_name(9229))
				say("")
				say("Time has been reseted.")
				pc.setf('valentine2019_dungeon','exit_v2019dungeon_time', 0)
				
				-- Dungeon Info
				pc.setqf("rejoin_time", get_time() - 0)
		end
		
		when LADY_VALENTINE_GHOST.chat."Who are you? " with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 1 begin
			local settings = valentine2019_dungeon.settings()
			addimage(25, 10, "v2019_02.tga")
			addimage(230, 140, "lady_valentine_ghost.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(LADY_VALENTINE_GHOST))
			say("")
			say("I'm not just common Lady.")
			say("i have some knowledge about magic.")
			say("I've teleported here in my mind.")
			say("So, physically I'm not here.")
			say("I will guide you as much as I can!")
			say("For now, destroy all those metin stones")
			say("and get a gem to unlock the monument.")
			wait()
			valentine2019_dungeon.get_regens(1)
			npc.purge()
			d.notice("Valentine chasm: Destroy all metins and get a gem!")
			d.spawn_mob(MONUMENT_OF_LOVE, settings["monument_ol_pos"][1], settings["monument_ol_pos"][2])
			d.setf("monument", 1)
		end
		
		when kill with valentine2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == METIN_OF_LOVE and d.getf("v2019_floor") == 1 begin
			local settings = valentine2019_dungeon.settings()
			if d.getf("monument") == 1 then
				d.notice("Valentine chasm: You still have to destroy 5 metins")
				d.setf("monument", 2)
			elseif d.getf("monument") == 2 then
				d.notice("Valentine chasm: You still have to destroy 4 metins")
				d.setf("monument", 3)
			elseif d.getf("monument") == 3 then
				d.notice("Valentine chasm: You still have to destroy 3 metins")
				d.setf("monument", 4)
			elseif d.getf("monument") == 4 then
				d.notice("Valentine chasm: You still have to destroy 2 metins")
				d.setf("monument", 5)
			elseif d.getf("monument") == 5 then
				d.notice("Valentine chasm: You still have to destroy 1 metin")
				d.setf("monument", 6)
			elseif d.getf("monument") == 6 then
				d.notice("Valentine chasm: You destroyed all metin stones!")
				d.notice("Valentine chasm: Now put the gem to monument and destroy it!")
				game.drop_item(settings.keys[1], 1)
				d.setf("monument", 0)
				d.setf("v2019_floor", 2)
			end
		end
		
		when MONUMENT_OF_LOVE.take with item.get_vnum() == 30715 and valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 2 begin
			local settings = valentine2019_dungeon.settings()
			pc.remove_item(settings.keys[1], 1)
			npc.kill("MONUMENT_OF_LOVE")
			valentine2019_dungeon.clear_valchasm()
			d.setf("v2019_floor", 3)
			timer("regen_spawn", 5)
			d.notice("Valentine chasm: Monsters are coming!")
			d.notice("Valentine chasm: Kill them all!")
		end		
		
		when LADY_VALENTINE_GHOST.chat."What next?" with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 4 begin
			local settings = valentine2019_dungeon.settings()
			local HerbPos = settings["unknown_herb_pos"]
			local randomNumber = math.random(1, table.getn(HerbPos));
			addimage(25, 10, "v2019_03.tga")
			addimage(230, 140, "lady_valentine_ghost.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(LADY_VALENTINE_GHOST))
				if party.is_party() and not party.is_leader() then
					say_reward("Let me talk with your leader please.")
				else
					say("")
					say("I'm almost able to teleport you into")
					say("next level.")
					say("But i need juice from this herb:")
					say_item("Unknown herb", settings["keys"][2], "")
					say("It should be somewhere here.")
					wait()
					d.spawn_mob(UNKNOWN_HERB, HerbPos[randomNumber][1], HerbPos[randomNumber][2]);
					d.notice("Valentine chasm: Find an unknown herb and take it to Lady.")
					d.setf("v2019_floor", 5)
				end
		end
		
		when UNKNOWN_HERB.click  with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 5 begin
			local settings = valentine2019_dungeon.settings()
			d.setf("v2019_floor", 6)
			game.drop_item(settings["keys"][2], 1)
			npc.purge()
		end
		
		when LADY_VALENTINE_GHOST.take with item.get_vnum() == 30716 and valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 6 begin
			local settings = valentine2019_dungeon.settings()
			pc.remove_item(settings["keys"][2], 1)
			npc.purge("LADY_VALENTINE_GHOST")
			valentine2019_dungeon.clear_valchasm()
			d.setf("v2019_floor", 7)
			timer("regen_spawn", 5)
			d.notice("Valentine chasm: You will be teleported to the next level.")
		end		
		
		when LADY_VALENTINE_GHOST.chat."Almost done" with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 8 begin
			local settings = valentine2019_dungeon.settings()
			addimage(25, 10, "v2019_04.tga")
			addimage(230, 140, "lady_valentine_ghost.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(LADY_VALENTINE_GHOST))
				if party.is_party() and not party.is_leader() then
					say_reward("Let me talk with your leader please.")
				else
					say("")
					say("Valentine tree is here. I can feel it!")
					say("If you kill him, we are much closer")
					say("to protect our village from this place!")
					say("He is here, but he is hiding.")
					say("I can feel some dark magic here.")
					say("")
					say_reward("Ohh, WATCH OUT!")
					say_reward("Monsters are coming!!")
					say_reward("I'm loosing my power here.")
					wait()
					npc.purge()
					valentine2019_dungeon.get_regens(3)
					d.notice("Valentine chasm: Kill all monsters!")
					d.setf("v2019_floor", 9)
					loop_timer("regen_clear", 15)
				end
		end
		
		when DRYADE.chat."Who are you?" with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 12 begin
			local settings = valentine2019_dungeon.settings()
			addimage(25, 10, "v2019_05.tga")
			addimage(230, 140, "dryade.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(DRYADE))
			if party.is_party() and not party.is_leader() then
				say_reward("Let me talk with your leader please.")
			else
				say("")
				say("Pffff...")
				say("I'm a protector of this place!!")
				say("I don't like what you're doing here!")
				say("I know where Valentine tree is.")
				say("I can unhide him! But i won't do it")
				say("for free of course.")
				say("I would like to have a rhodolite.")
				say("But the ores are hide.")
				wait()
				addimage(25, 10, "v2019_05.tga")
				addimage(230, 140, "dryade.tga")
				say("")
				say("")
				say("")
				say("")
				say_title(c_mob_name(DRYADE))
				say("Metin stone keep it in shadows.")
				say("If you kill the right one, the ore")
				say("will spawn.")
				say_reward("Destroy the right metin!")
				wait()
				npc.purge()
				d.notice("Valentine chasm: Destroy the right metin to spawn rhodolite ore!")
				d.setf("v2019_floor", 13)
				
				local position = {{379, 263}, {349, 260}, {330, 291}, {362, 311}, {394, 296}, {326, 267}}
	
				local n = number(1,1)
				for i = 1, 1 do
					if (i != n)
					then
						d.set_unique("fake"..i, d.spawn_mob( METIN_OF_LOVE, position[i][1], position[i][2]))
					end
				end

				local vid = d.spawn_mob( METIN_OF_LOVE, position[n][1], position[n][2])
				d.set_unique ("real_metin_stone",vid)
				if (is_test_server()) then
					-- real stone pos
					chat (position[n][1],position[n][2])
				end
			end
		end
		
		when kill with valentine2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == METIN_OF_LOVE and d.getf("v2019_floor") == 13 begin
			local settings = valentine2019_dungeon.settings()
			if d.is_unique_dead("real_metin_stone") then
				d.notice("Valentine chasm: Rhodolite ore was just spawned!")
				valentine2019_dungeon.clear_valchasm()
				game.drop_item(settings.owl_gemstone, 1)					
				d.setf("v2019_floor", 14)
				d.spawn_mob(RHODOLITE_ORE, settings["rore_pos"][1], settings["rore_pos"][2])
			else
				d.notice("Valentine chasm: There was nothing in this metin.")
			end
		end

		when RHODOLITE_ORE.click  with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 14 begin
			local settings = valentine2019_dungeon.settings()
			d.notice("Valentine chasm: This ore is too hard.")
			d.notice("Valentine chasm: Even a pick is too weak for it.")
			d.notice("Valentine chasm: Let's speak with dryade again.")
			d.spawn_mob(DRYADE, settings["dryade_pos"][1], settings["dryade_pos"][2])
			d.setf("v2019_floor", 15)
		end
		
		when DRYADE.chat."Ore is too hard" with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 15 begin
			local settings = valentine2019_dungeon.settings()
			addimage(25, 10, "v2019_06.tga")
			addimage(230, 140, "dryade.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(DRYADE))
				if party.is_party() and not party.is_leader() then
					say_reward("Let me talk with your leader please.")
				else
					say("")
					say("Of course it is, you fool!")
					say("This ore can be open only by")
					say_item("Magic dust", settings["keys"][3], "")
					say("I really want it. So i will give it to you.")
					wait()
					pc.give_item2(settings["keys"][3])
					d.notice("Valentine chasm: Open the ore with magic dust")
					d.setf("v2019_floor", 16)
				end
		end
		
		when RHODOLITE_ORE.take with item.get_vnum() == 30717 and valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 16 begin
			local settings = valentine2019_dungeon.settings()
			pc.remove_item(settings["keys"][3], 1)
			npc.kill("RHODOLITE_ORE")
			d.spawn_mob(RHODOLITE, settings["rore_pos"][1], settings["rore_pos"][2])
			d.setf("v2019_floor", 17)
		end

		when RHODOLITE.click  with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 17 begin
			local settings = valentine2019_dungeon.settings()
			pc.give_item2(settings["keys"][4], 1)
			npc.purge()
			d.notice("Valentine chasm: Give the rhodolite to dryade.")
			d.setf("v2019_floor", 18)
		end

		when DRYADE.chat."Here is the rhodolite" with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 18 begin
			local settings = valentine2019_dungeon.settings()
			addimage(25, 10, "v2019_07.tga")
			addimage(230, 140, "dryade.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(DRYADE))
				if pc.count_item(settings["keys"][4]) < 1 then
					say_reward("I don't think you have it.")
					say_reward("Don't mess with me!!")
					say_reward("You need")
					say_item("Rhodolite", settings["keys"][4], "")
				else
					pc.remove_item(settings["keys"][4], 1)
					say("")
					say("Thank you.")
					say("But i changed my mind a little bit.")
					say("I want something sweet!")
					say("This chocolate stone is hiding")
					say("very good chocolate inside.")
					wait()
					addimage(25, 10, "v2019_07.tga")
					addimage(230, 140, "dryade.tga")
					say("")
					say("")
					say("")
					say_title(c_mob_name(DRYADE))
					say("")
					say("Give me that chocolate. I will tell you")
					say("where the Valentine tree is then.")
					say_item("Chocolate", settings["keys"][5], "")
					wait()
					npc.purge()
					d.spawn_mob(CHOCOLATE_METIN, settings["choc_metin_pos"][1], settings["choc_metin_pos"][2])
					d.setf("v2019_floor", 19)
				end
		end
		
		when kill with valentine2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == CHOCOLATE_METIN and d.getf("v2019_floor") == 19 begin
			local settings = valentine2019_dungeon.settings()
			valentine2019_dungeon.clear_valchasm()
			game.drop_item(settings["keys"][5])
			d.notice("Valentine chasm: You get a chocolate.")
			d.notice("Valentine chasm: Look, Lady Valentine. Go to speak with her!")
			d.notice("Valentine chasm: Fast!!")
			d.spawn_mob(LADY_VALENTINE_GHOST, settings["lady_val_pos"][3][1], settings["lady_val_pos"][3][2])
			d.spawn_mob(DRYADE, settings["dryade_pos"][1], settings["dryade_pos"][2])
			d.setf("v2019_floor", 20)
		end
		
		when LADY_VALENTINE_GHOST.chat."Where've you been?" with valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 20 begin
			local settings = valentine2019_dungeon.settings()
			addimage(25, 10, "v2019_08.tga")
			addimage(230, 140, "lady_valentine_ghost.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(LADY_VALENTINE_GHOST))
				if pc.count_item(settings["keys"][5]) < 1 then
					say_reward("You don't have chocolate.")
					say_item("Chocolate", settings["keys"][5], "")
					say("")
					say_reward("I need to speak with person who has")
					say_reward("the chocolate")
				else
					say("")
					say("Dryade is a liar!")
					say("She is protecting the Valentine tree!!")
					say("You have to kill her to get to Valentine")
					say("tree.")
					wait()
					addimage(25, 10, "v2019_08.tga")
					addimage(230, 140, "lady_valentine_ghost.tga")
					say("")
					say("")
					say("")
					say_title(c_mob_name(LADY_VALENTINE_GHOST))
					say("")
					say("How?")
					say("I will posion this chocolate bar.")
					say("Give it to her!!")
					say("")
					say_item("Chocolate", settings["keys"][6], "")
					wait()
					pc.remove_item(settings["keys"][5], 1)
					pc.give_item2(settings["keys"][6], 1)
					npc.purge()
					d.notice("Valentine chasm: Give the poisoned chocolate to dryade!")
					d.setf("v2019_floor", 21)
				end
		end
		
		when DRYADE.take with item.get_vnum() == 30720 and valentine2019_dungeon.is_active() and d.getf("v2019_floor") == 21 begin
			local settings = valentine2019_dungeon.settings()
			pc.remove_item(settings["keys"][6], 1)
			npc.kill("DRYADE")
			valentine2019_dungeon.clear_valchasm()
			d.setf("v2019_floor", 22)
			timer("regen_spawn", 5)
			d.notice("Dryade: You've killed me!")
			d.notice("Dryade: You fooooool!")
		end		

		when kill with valentine2019_dungeon.is_active() and not npc.is_pc() and npc.get_race() == VALENTINE_TREE and d.getf("v2019_floor") == 23 begin
			local settings = valentine2019_dungeon.settings()
			valentine2019_dungeon.clear_valchasm()
			d.notice("Valentine chasm: You've killed valentine tree!")
			d.notice("Valentine chasm: Dungeon was succesfully finished!")
			d.notice("The group of "..pc.get_name().." has successfully killed the Valentine tree!")
			server_timer("final_succes_val", 115, d.get_map_index())
			d.setf("v2019_floor", 24)
		end
		

---- TIMERS
		
		when regen_spawn.timer begin
			local settings = valentine2019_dungeon.settings()
			if d.getf("v2019_floor") == 3 then
				valentine2019_dungeon.get_regens(2)
				loop_timer("regen_clear", 15);
			elseif d.getf("v2019_floor") == 7 then
				if party.is_party() then
					d.new_jump_party(settings["valentine_map_index"], settings["level_pos"][2][1], settings["level_pos"][2][2])
				else
					d.new_jump(settings["valentine_map_index"], settings["level_pos"][2][1], settings["level_pos"][2][2])
				end
				d.setf("v2019_floor", 8)
				d.spawn_mob(LADY_VALENTINE_GHOST, settings["lady_val_pos"][3][1], settings["lady_val_pos"][3][2])
			elseif d.getf("v2019_floor") == 10 then
				d.setf("v2019_floor", 11)
				valentine2019_dungeon.get_regens(4)
				loop_timer("regen_clear", 15);
			elseif d.getf("v2019_floor") == 22 then
				d.setf("v2019_floor", 23)
				d.spawn_mob(VALENTINE_TREE, settings["val_tree_pos"][1], settings["val_tree_pos"][2])
				d.notice("Valentine chasm: Kill the Valentine tree")
			end
		end
		
		
		when regen_clear.timer begin
			local settings = valentine2019_dungeon.settings()
			if d.getf("v2019_floor") == 3 then
				if (d.count_monster() == 0) then
					cleartimer("regen_clear");
					valentine2019_dungeon.clear_valchasm()
					d.setf("v2019_floor", 4)
					d.notice("Valentine chasm: You have killed all monsetrs!");
					d.notice("Valentine chasm: Let's speak with ghost of Lady Valentine");
					d.spawn_mob(LADY_VALENTINE_GHOST, settings["lady_val_pos"][1][1], settings["lady_val_pos"][1][2])
				else
					d.notice("Valentine chasm: You still have to kill %d mobs to move on.", d.count_monster());
				end
			elseif d.getf("v2019_floor") == 9 then
				if (d.count_monster() == 0) then
					cleartimer("regen_clear");
					valentine2019_dungeon.clear_valchasm()
					d.setf("v2019_floor", 10)
					d.notice("Valentine chasm: Another wave of monsters is coming!");
					d.notice("Valentine chasm: Valentine tree doesn't want to give up!");
					timer("regen_spawn", 5)
				else
					d.notice("Valentine chasm: You still have to kill %d mobs to move on.", d.count_monster());
				end
			elseif d.getf("v2019_floor") == 11 then
				if (d.count_monster() == 0) then
					cleartimer("regen_clear");
					valentine2019_dungeon.clear_valchasm()
					d.setf("v2019_floor", 12)
					d.notice("Valentine chasm: You have killed all monsetrs!");
					d.notice("Valentine chasm: But something is wrong.. Lady Valentine is not here");
					d.notice("Valentine chasm: Look, Dryade!");
					valentine2019_dungeon.clear_valchasm()
					d.spawn_mob(DRYADE, settings["dryade_pos"][1], settings["dryade_pos"][2])
				else
					d.notice("Valentine chasm: You still have to kill %d mobs to move on.", d.count_monster());
				end
			end
		end
		
		
		when valentine2019_dungeon_25min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Valentine chasm: 15 minutes left!")
				server_timer("valentine2019_dungeon_15min_left", 5*60, d.get_map_index())
			end
		end
		
		when valentine2019_dungeon_15min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Valentine chasm: 10 minutes left!")
				server_timer("valentine2019_dungeon_10min_left", 5*60, d.get_map_index())
			end
		end
		
		when valentine2019_dungeon_10min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Valentine chasm: 5 minutes left!")
				d.notice("Valentine chasm: You are running out of time! Hurry up!")
				server_timer("valentine2019_dungeon_5min_left", 5*60, d.get_map_index())
			end
		end
		
		
		when valentine2019_dungeon_5min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Valentine chasm: 1 minute left!")
				d.notice("Valentine chasm: You are almost failed!!")
				server_timer("valentine2019_dungeon_0min_left", 60, d.get_map_index())
			end
		end
		
		when valentine2019_dungeon_0min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				server_timer("final_val_dung", 1, d.get_map_index())
			end
		end
		
		when final_val_dung.server_timer begin
			local settings = valentine2019_dungeon.settings()
			if d.select(get_server_timer_arg()) then
				d.notice("Valentine chasm: Time has expired!")
				d.notice("Valentine chasm: You failed!")
				valentine2019_dungeon.clear_valchasm()
				d.set_warp_location(settings.valentine_map_index_out, settings.out_pos[1], settings.out_pos[2])
			end
			
			server_timer("final_exit_valchasm", 5, d.get_map_index())
		end
		
		when final_succes_val.server_timer begin
			local settings = valentine2019_dungeon.settings()
			if d.select(get_server_timer_arg()) then
				d.notice("Valentine chasm: You will be teleported out of dungeon!")
				valentine2019_dungeon.clear_valchasm()
				d.set_warp_location(settings.valentine_map_index_out, settings.out_pos[1], settings.out_pos[2])
			end
			
			server_timer("final_exit_valchasm", 5, d.get_map_index())
		end
		
		when final_exit_valchasm.server_timer begin
			if d.select(get_server_timer_arg()) then								
				d.exit_all()
			end
		end

		when logout with valentine2019_dungeon.is_active() begin 
			pc.setf('valentine2019_dungeon','exit_v2019dungeon_time', get_global_time())
			-- Dungeon Info
			pc.setqf("valentine2019_dungeon", get_time() + 0)
		end
	end
end
