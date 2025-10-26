define OWL_GHOST 9225
define FIRST_TOTEM 9226
define OWL_METIN1 8425
define OWL_METIN2 8426
define OWL_HURT 9227
define SURAN 718
define FEATHER 9236
define OWL_STATUE 8427
define RU_TAIG 719
define EPIC 7191

quest owl_dungeon begin
	state start begin
	--QUEST FUNCTIONS
		function settings()
			return
			{
				["owl_map_index"] = 212,
				["owl_map_index_out"] = 164,
				["out_pos"] = {187, 185},
				["level_pos"] = {
					[1] = {327, 163},
					[2] = {572, 152},
					[3] = {196, 591},
					[4] = {613, 566}
				},
				["level_check"] = {
					["minimum"] = 20,
					["maximum"] = 150
				},
				["keys"] = {30301, 30302}, 
				["ticket"] = 0, 
				-- First floor
				["owl_ghost"] = 9225, --Main npc
				["owl_ghost_pos"] = {
					[1] = {318, 162},
					[2] = {200, 156},
					[3] = {562, 158},
					[4] = {197, 587},
					[5] = {201, 581},
					[6] = {611, 566}
				},
				["feather_pos"] = {
					[1] = {207, 578},
					[2] = {207, 578},
					[3] = {207, 578},
					[4] = {207, 578},
					[5] = {207, 578},
					[6] = {207, 578},
					[7] = {207, 578},
					[8] = {207, 578},
					[9] = {207, 578},
					[10] = {207, 578},
					[11] = {207, 578},
					[12] = {207, 578},
					[13] = {207, 578}
				},
				["first_totem_pos"] = {215, 165},
				["key_1thfloor"] = 30707,
				["key_1bthfloor"] = 30708,				
				["owl_gemstone"] = 30709,				
				["suran_blood"] = 30710,				
				["owl_feather"] = 30711,				
				["healing_potion"] = 30712,				
				["owl_metin_pos"] = {564, 203},
				["suran_pos"] = {
					[1] = {200, 576},
					[2] = {578, 553},
					[3] = {586, 544}
				},
			--	["rutaig_pos"] = {582, 548},
				["owl_hurt_pos"] = {203, 576}
			};
		end
		
		function get_regens(level)
			local regens = {
				[1] = "data/dungeon/owl_dungeon/floor_1.txt",
				[2] = "data/dungeon/owl_dungeon/floor_1b.txt",
				[3] = "data/dungeon/owl_dungeon/floor_2a.txt",
				[4] = "data/dungeon/owl_dungeon/floor_2b.txt",
				[5] = "data/dungeon/owl_dungeon/floor_2c.txt",
				[6] = "data/dungeon/owl_dungeon/floor_3a.txt",
				[7] = "data/dungeon/owl_dungeon/floor_4a.txt",
				[8] = "data/dungeon/owl_dungeon/floor_4b.txt",
				[9] = "data/dungeon/owl_dungeon/floor_boss.txt"};
			return d.set_regen_file(regens[level])
		end

		function party_get_member_pids()
			local pids = {party.get_member_pids()}
			
			return pids
		end
		
		function is_owled()
			local pMapIndex = pc.get_map_index();
			local data = owl_dungeon.settings();
			local map_index = data["owl_map_index"];

			return pc.in_dungeon() and pMapIndex >= map_index*10000 and pMapIndex < (map_index+1)*10000;
		end
		
		function clear_owldungeon()
			d.clear_regen();
			d.kill_all();
		end
		
		function check_enter()
			addimage(25, 10, "owl_dungeon00.tga")
			addimage(238, 75, "owlghost.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9225))
			local settings = owl_dungeon.settings()

			--if not party.is_party() then
			--	say(gameforge.owl_dungeon._10_say)
			--	return
			--end
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end

			if party.is_party() and party.get_near_count() < 2 then
				say(gameforge.owl_dungeon._20_say)
				return false;
			end
			
			local levelCheck = true
			local ticketCheck = true
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
				say(gameforge.owl_dungeon._30_say)
				for i, n in next, notEnoughLevelMembers, nil do
					say_reward(string.format(gameforge.infected_garden_quest._60_sayTitle, n))
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
				say(gameforge.owl_dungeon._40_say)
				for i, n in next, notEnoughLevelMaxMembers, nil do
					say_reward(string.format(gameforge.infected_garden_quest._60_sayTitle, n))
				end
				return
			end
			if settings.ticket then
				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					if pc.count_item(settings.ticket) < 1 then
						table.insert(notEnoughTicketMembers, pc.get_name())
						ticketCheck = false
					end

					q.end_other_pc_block()
				end

				if not ticketCheck then
					say(gameforge.owl_dungeon._50_say)
					say(string.format(gameforge.owl_dungeon._60_say, c_item_name(settings.ticket)))
					say(gameforge.owl_dungeon._70_say)
					for i, n in next, notEnoughTicketMembers, nil do
						say_reward(string.format(gameforge.infected_garden_quest._60_sayTitle, n))
					end
					return
				end

				for i, pid in next, pids, nil do
					q.begin_other_pc_block(pid)
					pc.remove_item(settings.ticket, 1)
					q.end_other_pc_block()
				end
			end

			say(gameforge.owl_dungeon._80_say)
			wait()
			owl_dungeon.create_dungeon()
		end
				
		function create_dungeon()
			local setting = owl_dungeon.settings()
			
			
			if party.is_party() then
				d.new_jump_party(setting["owl_map_index"], setting["level_pos"][1][1], setting["level_pos"][1][2])
			else
				d.new_jump(setting["owl_map_index"], setting["level_pos"][1][1], setting["level_pos"][1][2])
			end
			d.setf("owl_level_1", 1)
			--d.setf("start_time", get_global_time())
			d.spawn_mob(setting["owl_ghost"], setting["owl_ghost_pos"][1][1], setting["owl_ghost_pos"][1][2])
			server_timer("owl_dungeon_45min_left", 45*60, d.get_map_index())
		end
		
		--FUNCTIONS END
		
		--LOGIN IN MAP
		when login begin
			local indx = pc.get_map_index()
			local settings = owl_dungeon.settings()
						
			if indx == settings["owl_map_index"] then
				if not pc.in_dungeon() then
					warp_to_village()
				end
			end
		end

		--DUNGEON ENTER
		when OWL_GHOST.chat.gameforge.owl_dungeon._90_npcChat with not owl_dungeon.is_owled() begin
			local settings = owl_dungeon.settings()
			addimage(25, 10, "owl_dungeon00.tga")
			addimage(238, 75, "owlghost.tga")
			say("")
			say("")
			say("")
			say_title(c_mob_name(9225))
			say(gameforge.owl_dungeon._110_say)
			say_title(gameforge.infected_garden_quest._210_sayTitle)
			if select(gameforge.infected_garden_quest._220_select, gameforge.infected_garden_quest._230_select) == 1 then
				owl_dungeon.check_enter()
			end
		end
		
		when OWL_GHOST.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 1 begin
			local settings = owl_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
			owl_dungeon.get_regens(1)
			npc.purge()
			d.notice("Jagras: Kill all owls")
			d.setf("totem", 1)
			loop_timer("owldungeon_1thfloor", 2);
			d.spawn_mob(FIRST_TOTEM, settings["first_totem_pos"][1], settings["first_totem_pos"][2])
		end
		
		when owldungeon_1thfloor.timer begin
			local settings = owl_dungeon.settings()
			if (d.count_monster() == 0) then
				cleartimer("owldungeon_1thfloor");
				
				game.drop_item(settings.key_1thfloor, 1);
				d.notice("Jagras: You have killed all monsetrs.");
				d.notice("Jagras: Now place %s to the totem and destroy it!", c_item_name(settings.key_1thfloor));
			else
				d.notice("Jagras: You still have to kill %s mobs to get a key.", d.count_monster());
			end
		end
		--when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == 710 or npc.get_race() == 711 or npc.get_race() == 712 and d.getf("owl_level_1") == 1 begin
		
		when FIRST_TOTEM.take with item.get_vnum() == 30707 and owl_dungeon.is_owled() and d.getf("totem") == 1 and d.getf("owl_level_1") == 1 begin
			local settings = owl_dungeon.settings()
			
			--if party.is_party() and not party.is_leader() then
			--	say(gameforge.infected_garden_quest._30_sayReward)
			--	return
			--end
			
			pc.remove_item(settings.key_1thfloor, 1)
			d.notice("Jagras: Something happened...")
			--d.notice("Jagras: This key doesn t works. Next %s are coming!!" , c_mob_name(712))
			d.notice("Jagras: Kill them fast!!")
			d.setf("owl_level_1", 2)
			owl_dungeon.get_regens(2)
			loop_timer("owldungeon_1bthfloor", 2);
		end
		
		when owldungeon_1bthfloor.timer begin
			local settings = owl_dungeon.settings()
			if (d.count_monster() == 0) then
				cleartimer("owldungeon_1bthfloor");
				
				game.drop_item(settings.key_1bthfloor, 1);
				d.notice("Jagras: You have killed all monsetrs.");
				d.notice("Jagras: Now place %s to the totem and destroy it!", c_item_name(settings.key_1bthfloor))
			else
				d.notice("Jagras: You still have to kill %s mobs to get a key.", d.count_monster())
			end
		end
	
		when FIRST_TOTEM.take with item.get_vnum() == 30708 and owl_dungeon.is_owled() and d.getf("totem") == 1 and d.getf("owl_level_1") == 2 begin
			local settings = owl_dungeon.settings()
			
			--if party.is_party() and not party.is_leader() then
			--	say(gameforge.infected_garden_quest._30_sayReward)
			--	return
			--end
			
			pc.remove_item(settings.key_1bthfloor, 1)
			npc.kill(FIRST_TOTEM)
			owl_dungeon.clear_owldungeon()
			timer("owlghost_spawn1floor", 2)
			d.spawn_mob(OWL_GHOST, settings["owl_ghost_pos"][2][1], settings["owl_ghost_pos"][2][2])
		end
		
		when OWL_GHOST.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 2 begin
			local settings = owl_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
						npc.purge()
						d.jump_all(settings["level_pos"][2][1], settings["level_pos"][2][2])
						d.setf("owl_level_1", 3)
						d.setf("totem", 0)
						d.spawn_mob(OWL_GHOST, settings["owl_ghost_pos"][3][1], settings["owl_ghost_pos"][3][2])
					end
	
		when OWL_GHOST.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 3 begin
			local settings = owl_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
					npc.purge()
					d.setf("owl_level_1", 4)
					d.spawn_mob(OWL_METIN1, settings["owl_metin_pos"][1], settings["owl_metin_pos"][2])
					d.notice("Jagras: Destroy the metin in 2 minutes!!")
					--d.notice("Jagras: If you won t destroy it in 2 minutes,")
					d.notice("Jagras: you will be out of the dungeon.")
					server_timer("final_owl", 2*60, d.get_map_index())
				end
	
		when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == OWL_METIN1 and d.getf("owl_level_1") == 4 begin
			local settings = owl_dungeon.settings()
			d.notice("Jagras: You succesfuly destroyed the metin!")
			d.notice("Jagras: Monsters are coming!!")
			d.notice("Jagras: Kill them all!")
			d.setf("owl_level_1", 5)
			server_timer("metin_succes1", 1, d.get_map_index())
			timer("wave_spawn", 2)
		end
		
		when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == OWL_METIN1 and d.getf("owl_level_1") == 7 begin
			local settings = owl_dungeon.settings()
			if d.getf("metinstone_second") == 1 then
				--d.notice("Jagras: 4 metin stones left!")
			--	d.setf("metinstone_second", 2)
			--elseif d.getf("metinstone_second") == 2 then
			--	d.notice("Jagras: 3 metin stones left!")
			--	d.setf("metinstone_second", 3)
			--elseif d.getf("metinstone_second") == 3 then
			--	d.notice("Jagras: 2 metin stones left!")
			--	d.setf("metinstone_second", 4)
			--elseif d.getf("metinstone_second") == 4 then
			--	d.notice("Jagras: 1 metin stone left!")
			--	d.setf("metinstone_second", 5)
			--elseif d.getf("metinstone_second") == 5 then
			--	d.notice("Jagras: You ve destroyed all metin stones.")
			--	d.notice("Jagras: The gate to next floor is open!")
				d.setf("metinstone_second", 0)
				d.setf("owl_level_1", 8)
				timer("wave_spawn", 2)
			end
		end
		
		
		when OWL_GHOST.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 9 begin
			local settings = owl_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
					npc.purge()
					d.spawn_mob(OWL_HURT, settings["owl_hurt_pos"][1], settings["owl_hurt_pos"][2])
				end

		when OWL_HURT.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 9 begin
			local settings = owl_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
					d.setf("owl_level_1", 10)
					--local metinStonePos = {{200, 576}};
					--for index = 1, table.getn(metinStonePos), 1 do
						--local randomNumber = math.random(1, table.getn(metinStonePos));
						d.spawn_mob(OWL_METIN2, 200, 576)
						--table.remove(metinStonePos, randomNumber);
						--local realMetinVID = d.spawn_mob(OWL_METIN2, metinStonePos[randomNumber][1], metinStonePos[randomNumber][2]);
						--d.set_unique("real_metin_stone", realMetinVID);
					end
		
		when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == OWL_METIN2 and d.getf("owl_level_1") == 10 begin
			local settings = owl_dungeon.settings()
			--if (npc.get_vid() == d.get_unique_vid("real_metin_stone")) then
				d.notice("Jagras: You've found first ingredient!")
				d.notice("Jagras: Be careful! Owls are coming!")
				d.notice("Jagras: Kill them all!")
				--d.kill_unique("OWL_METIN2")
				game.drop_item(settings.owl_gemstone, 1)					
				d.setf("owl_level_1", 11)
				timer("wave_spawn", 2)
			--else
				d.notice("Jagras: There was nothing in this metin.")
			end

		when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == SURAN and d.getf("owl_level_1") == 12 begin
			local settings = owl_dungeon.settings()
			game.drop_item(settings.suran_blood, 1)
			d.notice("Jagras: Suran was defeated!");
			d.notice("Jagras: You get another ingredient!");
			d.setf("owl_level_1", 13)
			d.spawn_mob(OWL_GHOST, settings["owl_ghost_pos"][5][1], settings["owl_ghost_pos"][5][2])
		end
		
		when OWL_GHOST.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 13 begin
			local settings = owl_dungeon.settings()
			local FeatherPos = settings["feather_pos"]
			local randomNumber = math.random(1, table.getn(FeatherPos));
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
					d.setf("owl_level_1", 14)
					d.spawn_mob(FEATHER, FeatherPos[randomNumber][1], FeatherPos[randomNumber][2]);
				end
		
		when FEATHER.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 14 begin
			local settings = owl_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
			npc.purge()
			game.drop_item(settings.owl_feather, 1)
			d.setf("owl_level_1", 15)
		end

		when OWL_GHOST.chat.gameforge.owl_dungeon._430_npcChat with owl_dungeon.is_owled() and d.getf("owl_level_1") == 15 begin
			local settings = owl_dungeon.settings()
				addimage(25, 10, "owl_dungeon03.tga")
				addimage(238, 75, "owlghost.tga")
				say("")
				say("")
				say("")
				say_title(c_mob_name(9225))
				if party.is_party() and not party.is_leader() then
					say_reward(gameforge.owl_dungeon._230_say)
				else
					if pc.count_item(settings.owl_gemstone) < 1 then
						say(gameforge.owl_dungeon._440_say)
						say_item_vnum(30709)
						--say_item(gameforge.owl_dungeon._450_sayItem, settings.owl_gemstone, "")
												say_reward(gameforge.owl_dungeon._460_sayReward)
					elseif pc.count_item(settings.suran_blood) < 1 then
						say(gameforge.owl_dungeon._440_say)
						say_item_vnum(30710)
						--say_item(gameforge.owl_dungeon._470_sayItem, settings.suran_blood, "")
												say_reward(gameforge.owl_dungeon._460_sayReward)
					elseif pc.count_item(settings.owl_feather) < 1 then
						say(gameforge.owl_dungeon._440_say)
						say_item_vnum(30711)
						--say_item(gameforge.owl_dungeon._480_sayItem, settings.owl_feather, "")
												say_reward(gameforge.owl_dungeon._460_sayReward)
					else
						pc.remove_item(settings.owl_gemstone, 1)
						pc.remove_item(settings.suran_blood, 1)
						pc.remove_item(settings.owl_feather, 1)
						pc.give_item2(settings.healing_potion, 1)
						say(gameforge.owl_dungeon._490_say)
						say_item_vnum(30712)
						--say_item(gameforge.owl_dungeon._500_sayItem, settings.healing_potion, "")
												say_reward(gameforge.owl_dungeon._510_sayReward)
						wait()
						npc.purge()
						d.setf("owl_level_1", 16)
					end
				end
		end
				
		when OWL_HURT.take with item.get_vnum() == 30712 and owl_dungeon.is_owled() and d.getf("owl_level_1") == 16 begin
			local settings = owl_dungeon.settings()
			
			--if party.is_party() and not party.is_leader() then
			--	say(gameforge.infected_garden_quest._30_sayReward)
			--	return
			--end
			
			pc.remove_item(settings.healing_potion, 1)
			npc.kill("OWL_HURT")
			timer("wave_spawn", 2)
			timer("wave_kill", 1)
			d.notice("Jagras: Lira is alive thanks to you!")
			d.notice("Jagras: Final floor is open now!")
		end

		when OWL_GHOST.click with owl_dungeon.is_owled() and d.getf("owl_level_1") == 17 begin
			local settings = owl_dungeon.settings()
			
			if party.is_party() and not party.is_leader() then
				say(gameforge.infected_garden_quest._30_sayReward)
				return
			end
			
					npc.purge()
					d.setf("owl_level_1", 18)
					d.setf("kill_statue", 1)
					owl_dungeon.get_regens(7)
					server_timer("final_owl", 3*60, d.get_map_index())
				end
		
		when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == OWL_STATUE and d.getf("owl_level_1") == 18 begin
			local settings = owl_dungeon.settings()
			if d.getf("kill_statue") == 1 then
			--	d.notice("Jagras: 3 statues has left!")
			--	d.setf("kill_statue", 2)
			--elseif d.getf("kill_statue") == 2 then
			--	d.notice("Jagras: 2 statues has left!")
			--	d.setf("kill_statue", 3)
			--elseif d.getf("kill_statue") == 3 then
			--	d.notice("Jagras: 1 statue has left!")
			--	d.setf("kill_statue", 4)
			--elseif d.getf("kill_statue") == 4 then
			--	d.notice("Jagras: All statues was destroyed!")
			--	d.notice("Jagras: Monsters are coming!")
			--	d.notice("Jagras: Kill them all!")
				d.setf("kill_statue", 0)
				d.setf("owl_level_1", 19)
				timer("wave_spawn", 2)
				server_timer("statue_succes", 1, d.get_map_index())
			end
		end
		
		when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == SURAN and d.getf("owl_level_1") == 20 begin
			local settings = owl_dungeon.settings()
			if d.getf("suran_kill") == 1 then
				d.setf("suran_kill", 2)
			elseif d.getf("suran_kill") == 2 then
				d.setf("suran_kill", 0)
				d.notice("Jagras: Perfect! Be prepared!")
				d.notice("Jagras: Jagras-Mu-Tai is coming!!!")
				d.setf("owl_level_1", 21)
				timer("wave_spawn", 2)
			end
		end
		
		when kill with owl_dungeon.is_owled() and not npc.is_pc() and npc.get_race() == RU_TAIG or npc.get_race() == EPIC and d.getf("owl_level_1") == 21 begin
			owl_dungeon.clear_owldungeon()
			d.notice("Jagras: You succesfully finished the dungeon!")
			d.notice("Jagras: You will be teleported in 2 minutes")

			pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 5) == 0 then
					notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(719), pc.getqf("boss_kills"))
				end
			--if party.is_party() then
			--	highscore.register("od_pt_time", get_global_time() - d.getf("start_time"), 0)
			--else
			--	highscore.register("od_time", get_global_time() - d.getf("start_time"), 0)
			--end
			
			--pc.setqf(RU_TAIG.."_kills", pc.getqf(RU_TAIG.."_kills") + 1)
			
			
			server_timer("final_succes_owl", 30, d.get_map_index())
			d.setf("owl_level_1", 22)
		end
		
		when wave_kill.timer begin
			local settings = owl_dungeon.settings()
			if d.getf("owl_level_1") == 5 then
				if (d.count_monster() == 0) then
					cleartimer("wave_kill");
					d.setf("owl_level_1", 6)
					d.notice("Jagras: You have killed all monsetrs!");
					d.notice("Jagras: Watch out! Stronger monsters are coming!");
					timer("wave_spawn", 2)
				else
					d.notice("Jagras: You still have to kill %s mobs to move on.", d.count_monster())
				end
			elseif d.getf("owl_level_1") == 6 then
				if (d.count_monster() == 0) then
					cleartimer("wave_kill");
					d.setf("owl_level_1", 7)
					d.notice("Jagras: You have killed all monsetrs!");
					d.notice("Jagras: Destroy all metin stones!");
					timer("wave_spawn", 2)
				else
					d.notice("Jagras: You still have to kill %s mobs to move on.", d.count_monster())
				end
			elseif d.getf("owl_level_1") == 11 then
				if (d.count_monster() == 0) then
					cleartimer("wave_kill");
					d.setf("owl_level_1", 12)
					d.notice("Jagras: Suran is here!");
					d.notice("Jagras: Defeat him!");
					timer("wave_spawn", 2)
				else
					d.notice("Jagras: You still have to kill %s mobs to move on.", d.count_monster())
				end
			elseif d.getf("owl_level_1") == 16 then
				d.purge_area()
			elseif d.getf("owl_level_1") == 19 then
				if (d.count_monster() == 0) then
					cleartimer("wave_kill");
					d.setf("owl_level_1", 20)
					d.notice("Jagras: Great job!");
					d.notice("Jagras: Be careful now!");
					d.notice("Jagras: 2 Surans are coming!");
					d.notice("Jagras: Kill them!");
					timer("wave_spawn", 2)
				else
					d.notice("Jagras: You still have to kill %s mobs to move on.", d.count_monster())
				end
			end
		end
		
		when wave_spawn.timer begin
			local settings = owl_dungeon.settings()
			if d.getf("owl_level_1") == 5 then
				owl_dungeon.get_regens(3)
				loop_timer("wave_kill", 2);
			elseif d.getf("owl_level_1") == 6 then
				owl_dungeon.get_regens(4)
				loop_timer("wave_kill", 2);
			elseif d.getf("owl_level_1") == 7 then
				owl_dungeon.get_regens(5)
				d.setf("metinstone_second", 1)
			elseif d.getf("owl_level_1") == 8 then
				owl_dungeon.clear_owldungeon()
				d.jump_all(settings["level_pos"][3][1], settings["level_pos"][3][2])					
				d.spawn_mob(OWL_GHOST, settings["owl_ghost_pos"][4][1], settings["owl_ghost_pos"][4][2])
				d.setf("owl_level_1", 9)
			elseif d.getf("owl_level_1") == 11 then
				owl_dungeon.get_regens(6)
				loop_timer("wave_kill", 2);
			elseif d.getf("owl_level_1") == 12 then
				d.spawn_mob(SURAN, settings["suran_pos"][1][1], settings["suran_pos"][1][2])
			elseif d.getf("owl_level_1") == 16 then
				owl_dungeon.clear_owldungeon()
				d.jump_all(settings["level_pos"][4][1], settings["level_pos"][4][2])
				d.setf("owl_level_1", 17)
				d.spawn_mob(OWL_GHOST, settings["owl_ghost_pos"][6][1], settings["owl_ghost_pos"][6][2])					
			elseif d.getf("owl_level_1") == 19 then
				owl_dungeon.get_regens(8)
				loop_timer("wave_kill", 2);					
			elseif d.getf("owl_level_1") == 20 then
				d.spawn_mob(SURAN, settings["suran_pos"][2][1], settings["suran_pos"][2][2])
				d.spawn_mob(SURAN, settings["suran_pos"][3][1], settings["suran_pos"][3][2])
				d.setf("suran_kill", 1)					
			elseif d.getf("owl_level_1") == 21 then
				owl_dungeon.get_regens(9)
			end
		end
		
		when metin_succes1.server_timer begin
			if d.select(get_server_timer_arg()) then
				clear_server_timer("final_owl", get_server_timer_arg())
			end
		end
		
		when statue_succes.server_timer begin
			if d.select(get_server_timer_arg()) then
				clear_server_timer("final_owl", get_server_timer_arg())
			end
		end
		
		when owl_dungeon_45min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Jagras: 30 minutes left!")
				server_timer("owl_dungeon_30min_left", 30*60, d.get_map_index())
			end
		end
		
		when owl_dungeon_30min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Jagras: 15 minutes left!")
				server_timer("owl_dungeon_15min_left", 15*60, d.get_map_index())
			end
		end
		
		when owl_dungeon_15min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Jagras: 10 minutes left!")
				server_timer("owl_dungeon_10min_left", 10*60, d.get_map_index())
			end
		end
		
		when owl_dungeon_10min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Jagras: 5 minutes left!")
				d.notice("Jagras: Hurry up!!")
				server_timer("owl_dungeon_5min_left", 5*60, d.get_map_index())
			end
		end
		
		when owl_dungeon_5min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice("Jagras: 1 minute left!")
				d.notice("Jagras: You are almost failed!!")
				server_timer("owl_dungeon_1min_left", 60, d.get_map_index())
			end
		end
		
		when owl_dungeon_1min_left.server_timer begin
			if d.select(get_server_timer_arg()) then
				server_timer("final_owl", 1, d.get_map_index())
			end
		end
		
		when final_owl.server_timer begin
			local settings = owl_dungeon.settings()
			if d.select(get_server_timer_arg()) then
				d.notice("Jagras: Time has expired!")
				d.notice("Jagras: You failed!")
				owl_dungeon.clear_owldungeon()
				d.set_warp_location(settings["owl_map_index_out"], settings["out_pos"][1], settings["out_pos"][2])
			end
			
			server_timer("final_exit_owl", 5, d.get_map_index())
		end
		
		when final_succes_owl.server_timer begin
			local settings = owl_dungeon.settings()
			if d.select(get_server_timer_arg()) then
				d.notice("Jagras: You will be teleported out of dungeon!")
				owl_dungeon.clear_owldungeon()
				d.set_warp_location(settings["owl_map_index_out"], settings["out_pos"][1], settings["out_pos"][2])
			end
			
			server_timer("final_exit_owl", 5, d.get_map_index())
		end
		
		when final_exit_owl.server_timer begin
			if d.select(get_server_timer_arg()) then								
				d.exit_all()
			end
		end									
	end
end
