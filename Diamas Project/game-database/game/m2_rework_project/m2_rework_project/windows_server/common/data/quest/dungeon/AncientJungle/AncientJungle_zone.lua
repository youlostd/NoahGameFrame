quest AncientJungle_zone begin
	state start begin
		when login with AncientJungleLIB.isActive() begin
			local settings = AncientJungleLIB.Settings();			
			if (party.is_party() and party.is_leader() or not party.is_party()) then
				d.setf("AncientJungle_floor", 1);
				d.setf("AncientJungle_1w_monsters_k", 1);
				
				AncientJungleLIB.setReward();
				AncientJungleLIB.setOutCoords();
				
				d.spawn_mob_dir(9394, 248, 311, 5);
				d.spawn_mob_dir(9395, 248, 320, 5);
			end
		end
		
		---------
		-- 1. Floor
		-- Players talk to NPC once they spawn in the dungeon (9394), then monsters are spawned
		-------------
		when 9394.chat."Who are you" with AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 1 begin
			addimage(25, 10, "aj_bg2.tga"); addimage(225, 150, "aj_enemy.tga")
			say("[ENTER][ENTER]")
			say_title(string.format("%s:[ENTER]", mob_name(npc.get_race())))
			
			if (party.is_party() and not party.is_leader()) then
				say("I will speak only with your leader!![ENTER]Get away from me!")
				return;
			end
			
			say("Ohh. So the old man really asked you people[ENTER]for help. How funny. Nothing can't stop us now![ENTER]We are too strong already, and you will see it[ENTER]in few minutes! You worm!")
			wait()
			setskin(NOWINDOW);
			npc.purge();
			
			d.regen_file("data/dungeon/ancient_jungle/regen_1f.txt");
			
			d.notice(string.format("Ancient jungle: Grow the %s! When its 3x bigger in size, %s will be uncovered!", mob_name(9395), mob_name(4406)));
		end
		
		---------
		-- 1. Floor
		-- Players talk to NPC once they spawn in the dungeon (9394), then monsters are spawned
		-------------
		when kill with AncientJungleLIB.isActive() and not npc.is_pc() and d.getf("AncientJungle_floor") == 1 and d.getf("AncientJungle_1w_monsters_k") == 1 begin
			local settings = AncientJungleLIB.Settings();		
			local KILL_COUNT = settings["KILL_COUNT_1_FLOOR"];
			local n = d.getf("AncientJungle_1w_monsters_c") + 1
			
			d.setf("AncientJungle_1w_monsters_c", n)
			
			if pc.get_x() > 20456 and pc.get_y() > 22672 and pc.get_x() < 20738 and pc.get_y() < 22906 then				
				
				if n >= KILL_COUNT then
					game.drop_item(30863, 1)
					
					d.setf("AncientJungle_1w_monsters_c", 0)
					d.setf("AncientJungle_1w_monsters_k", 0)
					d.setf("AncientJungle_crystal", 1)
				end
			end
		end
		
		---------
		-- 1. Floor
		-- Growing the crystal - 2nd level
		-------------
		when 9395.take with item.get_vnum() == 30863 and AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 1 and d.getf("AncientJungle_crystal") == 1 begin
			local settings = AncientJungleLIB.Settings();
			
			d.setf("AncientJungle_crystal", 0)
			d.spawn_mob_dir(9396, 248, 320, 5);
			
			item.remove()
			npc.kill()
			
			d.notice("Ancient Jungle: It's still too small yet")
			server_timer("AncientJungle_FirstWave", settings["time_to_proceed"], d.get_map_index())
		end
				
		---------
		-- 1. Floor
		-- Growing the crystal - 3rd level
		-------------
		when 9396.take with item.get_vnum() == 30863 and AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 1 and d.getf("AncientJungle_crystal") == 1 begin
			local settings = AncientJungleLIB.Settings();
			
			d.setf("AncientJungle_crystal", 0)
			d.spawn_mob_dir(9397, 248, 320, 5);
			
			item.remove()
			npc.kill()
			
			d.notice("Ancient Jungle: This size is perfect. You only need to activate it!")
			server_timer("AncientJungle_FirstWave", settings["time_to_proceed"], d.get_map_index())
		end
				
		---------
		-- 1. Floor
		-- Growing the crystal - 3rd level
		-------------
		when 9397.take with item.get_vnum() == 30863 and AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 1 and d.getf("AncientJungle_crystal") == 1 begin
			local settings = AncientJungleLIB.Settings();
			
			d.setf("AncientJungle_crystal", 0)
			d.spawn_mob_dir(9398, 248, 320, 5);
			
			item.remove()
			npc.purge()
			
			d.setf("AncientJungle_1Boss", 1)			
			d.spawn_mob(4406, 248, 261)
		end
				
		---------
		-- 1. Floor
		-- Timer for spawn next waves
		-------------
		when AncientJungle_FirstWave.server_timer begin
			if d.select(get_server_timer_arg()) then
			
				d.setf("AncientJungle_1w_monsters_k", 1)			
				d.regen_file("data/dungeon/ancient_jungle/regen_1f.txt");
			end
		end
		
		---------
		-- 1. Floor
		-- Timer for spawn first boss
		-------------
		when 4406.kill with AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 1 begin
			local settings = AncientJungleLIB.Settings();
			local time_to_proceed = settings["time_to_proceed"]
							
			AncientJungleLIB.clearDungeon();
			d.notice(string.format("Ancient Jungle: You will proceed in %s seconds", time_to_proceed));
			
			server_timer("AncientJungle_FirstJump", time_to_proceed, d.get_map_index())
		end
		
		---------
		-- 1. Floor
		-- Jump to second floor + all settings
		-------------
		when AncientJungle_FirstJump.server_timer begin
			if d.select(get_server_timer_arg()) then
				local settings = AncientJungleLIB.Settings();
				local JumpPos = settings["inside_2f_pos"]; 
				local Time = settings["timer_second_floor"]
				local minutes = math.floor(Time / 60)
				local position = settings["second_floor_stone_pos"]; 
				local n = number(1,4); 			
			
				d.setf("AncientJungle_floor", 2);
				d.setf("AncientJungle_FirstStone", 1);
				
				d.spawn_mob_dir(9399, 656, 240, 7);
				d.set_regen_file("data/dungeon/ancient_jungle/regen_2fa.txt");
				
				table_shuffle(position);
				
				for i = 1, 4 do
					if (i != n) then
						d.set_unique("fake"..i, d.spawn_mob(8702, position[i][1], position[i][2]))
					end
				end
			
				d.set_unique ("real", d.spawn_mob(8702, position[n][1], position[n][2]))
				
				d.jump_all(JumpPos[1], JumpPos[2]);
				
				d.notice(string.format("Ancient jungle: Find a real %s. You will find a key to activate the %s!", mob_name(8702), mob_name(9399)));
				d.notice("Ancient jungle: You will earn a negative point for every fake stone");
				d.notice("Ancient jungle: The more fake points you have, the more HP will have a boss in this floor.");
				d.notice(string.format("Ancient jungle: You have to finish this floor within %s minutes, otherwise you failed", minutes));
				
				server_timer("AncientJungle_SecondFloor_timer", Time, d.get_map_index())
			end
		end


---------------------------------------------
---- 2. Floor
---------------------------------------------

		---------
		-- 2. Floor
		-- Destryoing the stones in second floor (players have to find the real one)
		-------------
		when 8702.kill with AncientJungleLIB.isActive() and not npc.is_pc() and d.getf("AncientJungle_floor") == 2 begin
			local settings = AncientJungleLIB.Settings(); 
			local n = d.getf("AncientJungle_wrong_count") + 1
			
			if d.getf("AncientJungle_FirstStone") == 1 then
				if npc.get_vid() == d.get_unique_vid("real") then
					d.clear_regen();
					d.purge_area(2091800, 2269700, 2111100, 2285500);
					
					d.setf("AncientJungle_FirstStone", 0);
					d.setf("AncientJungle_SealActivate", 1);
					
					d.notice("Ancient jungle: You have destroyed the right stone!")
					d.notice(string.format("Ancient jungle: You can activate %s now!", mob_name(9399)));
					
					game.drop_item(30864, 1);
				else
					d.setf("AncientJungle_wrong_count", n);	
					
					d.notice("Ancient jungle: This one was just an illusion!")
					d.notice(string.format("Ancient jungle: You have %d negative points.", d.getf("AncientJungle_wrong_count")));
				end
			end
		end
		
		---------
		-- 2. Floor
		-- Players put dropped item (30864) to the seal (9399) and activate it, golems are spawned
		-------------
		when 9399.take with item.get_vnum() == 30864 and AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 2 and d.getf("AncientJungle_SealActivate") == 1 begin
			local settings = AncientJungleLIB.Settings();
			
			d.setf("AncientJungle_SealActivate", 0);
			d.setf("AncientJungle_GolemDrop", 1);
			
			d.spawn_mob_dir(9400, 656, 240, 7);
			
			item.remove()
			npc.purge()
			
			d.set_regen_file("data/dungeon/ancient_jungle/regen_2fb.txt");
			
			d.notice(string.format("Ancient jungle: Kill %s until you find %s!", mob_name(4405), item_name(30865)));
		end
		
		---------
		-- 2. Floor
		-- Players drop item (30865) from golems (4405)
		-------------
		when kill with AncientJungleLIB.isActive() and not npc.is_pc() and d.getf("AncientJungle_floor") == 2 and npc.get_race() == 4405 begin
			local settings = AncientJungleLIB.Settings();
			local DROP_CHANCE = settings["fire_drop_chance"]
			
			if pc.get_x() > 20918 and pc.get_y() > 22697 and pc.get_x() < 21111 and pc.get_y() < 22855 then				
				
				if d.getf("AncientJungle_GolemDrop") == 1 then
					
					if number(1,DROP_CHANCE) == 1 then
						d.clear_regen();
						game.drop_item(30865, 1);
						
						d.setf("AncientJungle_GolemDrop", 0);						
						d.setf("AncientJungle_SealDestroy", 1);						
					end					
				end
			end
		end
			
		---------
		-- 2. Floor
		-- Players put dropped item (30865) to seal (9400) and destroy it, boss (4407) is spawned
		-------------
		when 9400.take with item.get_vnum() == 30865 and AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 2 and d.getf("AncientJungle_SealDestroy") == 1 begin
			local settings = AncientJungleLIB.Settings();
			local x = 656 + number(1, 40)
			local y = 240 + number(1, 40)
			
			AncientJungleLIB.clearDungeon();
			item.remove();
			
			d.setf("AncientJungle_SealDestroy", 0);
			d.setf("AncientJungle_SecondBoss", 1);
			
			d.set_unique("SecondBoss", d.spawn_mob(4407, x, y));
			
			if d.getf("AncientJungle_wrong_count") == 1 then
				d.unique_set_maxhp("SecondBoss", settings["SecondBoss_HP1"])
			elseif d.getf("AncientJungle_wrong_count") == 2 then
				d.unique_set_maxhp("SecondBoss", settings["SecondBoss_HP_2"])
			elseif d.getf("AncientJungle_wrong_count") == 3 then
				d.unique_set_maxhp("SecondBoss", settings["SecondBoss_HP_3"])
			end
			
			d.notice(string.format("Ancient jungle: Kill %s!", mob_name(4407)));
		end
		
		---------
		-- 2. Floor
		-- Players kill second boss (4407)
		-------------
		when 4407.kill with AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 2  and d.getf("AncientJungle_SecondBoss") == 1 begin
			local settings = AncientJungleLIB.Settings();
			local time_to_proceed = settings["time_to_proceed"]
			
			clear_server_timer("AncientJungle_SecondFloor_timer", d.get_map_index());
			
			AncientJungleLIB.clearDungeon();
			d.notice(string.format("Ancient Jungle: You will proceed in %s seconds", time_to_proceed));
			
			server_timer("AncientJungle_SecondJump", time_to_proceed, d.get_map_index())
		end
		
		---------
		-- 2. Floor
		-- Jump to third floor + all settings
		-------------
		when AncientJungle_SecondJump.server_timer begin
			if d.select(get_server_timer_arg()) then
				local settings = AncientJungleLIB.Settings();
				local JumpPos = settings["inside_3f_pos"]; 
				local BrazierPos = settings["third_floor_brazier_pos"];
				local n = number(1,4)
			
				d.setf("AncientJungle_floor", 3);
				d.setf("AncientJungle_NpcTalk_1", 1);

				d.spawn_mob_dir(9394, 1135, 263, 1);
				
				for i = 1, 4 do
					d.set_unique("brazier_"..i, d.spawn_mob_dir(9401, BrazierPos[i][1], BrazierPos[i][2], BrazierPos[i][3]))
				end
				
				d.jump_all(JumpPos[1], JumpPos[2]);
			end
		end


		---------
		-- 2. Floor
		-- Second floor timer, after its filled, players failed
		-------------
		when AncientJungle_SecondFloor_timer.server_timer begin
			if d.select(get_server_timer_arg()) then
			
				d.notice("Ancient jungle: You have failed");			
				d.exit_all();
			end
		end
		
---------------------------------------------
---- 3. Floor
---------------------------------------------
	
		---------
		-- 3. Floor
		-- Players talk to NPC at the start of 3rd floor (9394), then stone is spawned and timer setted
		-------------
		when 9394.chat."Who are you??" with AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 3 and d.getf("AncientJungle_NpcTalk_1") == 1 begin
			local settings = AncientJungleLIB.Settings();
			local Timer = settings["timer_to_destroy_secstone"]
			local minutes = math.floor(Timer / 60)
			
			addimage(25, 10, "aj_bg3.tga"); addimage(225, 150, "aj_enemy.tga")
			say("[ENTER][ENTER]")
			say_title(string.format("%s:[ENTER]", mob_name(npc.get_race())))
			
			if (party.is_party() and not party.is_leader()) then
				say("I will speak only with your leader!![ENTER]Get away from me!")
				return;
			end
			
			say("How did you get here?!! It's not possible![ENTER]But it's not the end!! This was just[ENTER]a beggining! You will die here!!")
			wait()
			setskin(NOWINDOW);
			npc.purge();
			
			d.spawn_mob(8703, 1135, 275);
			d.setf("AncientJungle_SecondStone", 1);
			d.setf("AncientJungle_NpcTalk_1", 0);
			
			d.notice(string.format("Ancient jungle: Destroy the %s as fast as you can!", mob_name(8703)));
			d.notice(string.format("Ancient jungle: If you won't destroy it within %s minutes,", minutes));
			d.notice("Ancient jungle: the final boss is gonna has higher defense.");
		
			server_timer("AncientJungle_SecondStoneKill", Timer, d.get_map_index())
		end
		
		---------
		-- 3. Floor
		-- Destryoing second stone
		-------------
		when 8703.kill with AncientJungleLIB.isActive() and not npc.is_pc() and d.getf("AncientJungle_floor") == 3 and d.getf("AncientJungle_SecondStone") == 1 begin
			local settings = AncientJungleLIB.Settings(); 
			
			clear_server_timer("AncientJungle_SecondStoneKill", d.get_map_index());
			
			d.setf("AncientJungle_SecondStone", 0);
			d.setf("AncientJungle_2w_monsters_k", 1);
			
			d.regen_file("data/dungeon/ancient_jungle/regen_3f.txt");
			
			d.notice("Ancient jungle: You made it in time!")
			d.notice(string.format("Ancient jungle: Now fire up all %s!", mob_name(9401)));					
		end
		
		---------
		-- 3. Floor
		-- Timer checker for the second stone, if players haven't destryoed it in definted time, final boss is gonna has much higher defense
		-------------
		when AncientJungle_SecondStoneKill.server_timer begin
			if d.select(get_server_timer_arg()) then
				if d.getf("AncientJungle_SecondStone") == 1 then
					d.setf("AncientJungle_FinalBossDef", 1)
					
					d.notice(string.format("Ancient jungle: Time is up, %s gained more power!", mob_name(4408)));
				else
					return;
				end
			end
		end
		
		---------
		-- 3. Floor
		-- Players kill 4x wave of monsters to get item (30866) and fire up all braziers
		-------------
		when kill with AncientJungleLIB.isActive() and not npc.is_pc() and d.getf("AncientJungle_floor") == 3 and d.getf("AncientJungle_2w_monsters_k") == 1 begin
			local settings = AncientJungleLIB.Settings();		
			local KILL_COUNT = settings["KILL_COUNT_3_FLOOR"];
			local n = d.getf("AncientJungle_2w_monsters_c") + 1
			
			d.setf("AncientJungle_2w_monsters_c", n)
			
			if pc.get_x() > 21418 and pc.get_y() > 22741 and pc.get_x() < 21579 and pc.get_y() < 22878 then				
				
				if n >= KILL_COUNT then
					game.drop_item(30866, 1)
					
					d.setf("AncientJungle_2w_monsters_c", 0)
					d.setf("AncientJungle_2w_monsters_k", 0)
					d.setf("AncientJungle_Brazier", 1)
				end
			end
		end
		
		---------
		-- 3. Floor
		-- Players put dropped item (30866) to brazier (9401) and fire it up
		-------------
		when 9401.take with item.get_vnum() == 30866 and AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 3 and d.getf("AncientJungle_Brazier") == 1 begin
			local settings = AncientJungleLIB.Settings();
			local Position = settings["third_floor_brazier_pos"]
			local n = d.getf("AncientJungle_Brazier_c") + 1
			
			item.remove();
			d.setf("AncientJungle_Brazier", 0);
			d.setf("AncientJungle_Brazier_c", n);
			
			if npc.get_vid() == d.get_unique_vid("brazier_1") then
				d.spawn_mob_dir(9402, Position[1][1], Position[1][2], Position[1][3]);
				d.kill_unique("brazier_1");
				
			elseif npc.get_vid() == d.get_unique_vid("brazier_2") then
				d.spawn_mob_dir(9402, Position[2][1], Position[2][2], Position[2][3]);
				d.kill_unique("brazier_2");
				
			elseif npc.get_vid() == d.get_unique_vid("brazier_3") then
				d.spawn_mob_dir(9402, Position[3][1], Position[3][2], Position[3][3]);
				d.kill_unique("brazier_3");
				
			elseif npc.get_vid() == d.get_unique_vid("brazier_4") then
				d.spawn_mob_dir(9402, Position[4][1], Position[4][2], Position[4][3]);
				d.kill_unique("brazier_4");
			end
			
			if d.getf("AncientJungle_Brazier_c") >= 4 then
				d.purge_area(2141800, 2274100, 2157900, 2287800);
							
				d.spawn_mob_dir(9394, 1135, 263, 1);
				
				d.notice(string.format("Ancient jungle: Great job! All %s are on fire!", mob_name(9401)));
				
				d.setf("AncientJungle_FinalNPCTalk", 1);
			else
				server_timer("AncientJungle_FinalWave", settings["time_to_proceed"], d.get_map_index())
			end			
		end

		---------
		-- 3. Floor
		-- Timer for spawn next waves
		-------------
		when AncientJungle_FinalWave.server_timer begin
			if d.select(get_server_timer_arg()) then
			
				d.setf("AncientJungle_2w_monsters_k", 1)			
				d.regen_file("data/dungeon/ancient_jungle/regen_3f.txt");
			end
		end
		
		---------
		-- 3. Floor
		-- Players talk to NPC at the start of 3rd floor (9394), then server timer is set 
		-------------
		when 9394.chat."Who are you??" with AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 3 and d.getf("AncientJungle_FinalNPCTalk") == 1 begin
			local settings = AncientJungleLIB.Settings();
			
			addimage(25, 10, "aj_bg3.tga"); addimage(225, 150, "aj_enemy.tga")
			say("[ENTER][ENTER]")
			say_title(string.format("%s:[ENTER]", mob_name(npc.get_race())))
			
			if (party.is_party() and not party.is_leader()) then
				say("I will speak only with your leader.[ENTER]")
				return;
			end
			
			say("I was also protector of this place.[ENTER]But the ancient mystical power got me.[ENTER]My mind was blind and heart like a stone.[ENTER]Tell them, that I'm sorry...[ENTER]Kill the originator of that evil, I will[ENTER]reveal him. You just have to kill him![ENTER]Bye!")
			wait()
			setskin(NOWINDOW);
			npc.kill();
			
			d.setf("AncientJungle_FinalBoss", 1);
			
			d.notice("Ancient jungle: Ancient evil power is coming!");
		
			server_timer("AncientJungle_FinallBossSpawn", settings["time_to_proceed"], d.get_map_index())
		end
		
		------
		-- 3. Floor
		-- Timer for spawn final boss
		-------------
		when AncientJungle_FinallBossSpawn.server_timer begin
			if d.select(get_server_timer_arg()) then
				local settings = AncientJungleLIB.Settings();
				local DEFENSE_NORMAL = settings["FinalBoss_Def_Normal"]
				local DEFENSE_HIGH = settings["FinalBoss_Def_High"]
			
				d.set_unique("FinalBoss", d.spawn_mob(4408, 1135, 275));
				
				if d.getf("AncientJungle_FinalBossDef") == 1 then
					d.unique_set_def_grade("FinalBoss", DEFENSE_HIGH)
				else
					d.unique_set_def_grade("FinalBoss", DEFENSE_NORMAL)
				end
			end
		end
		
		---------
		-- 3. Floor
		-- Players kill final boss (4408)
		-------------
		when 4408.kill with AncientJungleLIB.isActive() and d.getf("AncientJungle_floor") == 3  and d.getf("AncientJungle_FinalBoss") == 1 begin
			local settings = AncientJungleLIB.Settings();
			local time_to_proceed = settings["time_to_proceed"]
			
			clear_server_timer("AncientJungle_SecondFloor_timer", d.get_map_index());
			
			AncientJungleLIB.clearDungeon();
			d.notice(string.format("Ancient Jungle: Good job! %s was beated!", mob_name(4408)));
			d.notice("Ancient Jungle: You will be teleported out of dungeon in few moments.");
			
			d.setf("AncientJungle_CanTakeReward", 1);
			
			d.spawn_mob_dir(9403, 1135, 238, 1);
			
			server_timer("AncientJungle_FinalExit", settings["timer_to_exit_dungeon"], d.get_map_index())
		end
		
		-----
		--Players can take reward from the chest
		-----	
		when 9403.chat."Take reward" with AncientJungleLIB.isActive() and d.getf("AncientJungle_CanTakeReward") == 1 begin
			setskin(NOWINDOW);
			
			if d.getf(string.format("player_%d_reward_state", pc.get_player_id())) == 1 then
				pc.give_item2(30867, 1);
				
				d.setf(string.format("player_%d_reward_state", pc.get_player_id()), 0);	
			else
				syschat("Ancient Jungle: You already took the reward!")
			end 
		end
		
		------
		-- 3. Floor
		-- Server timer for final exit
		-------------
		when AncientJungle_FinalExit.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.exit_all();
			end
		end
		
		------------
		-- Set waiting time for next enter (1 hour - 3600 seconds)
		------------
		
		when logout with AncientJungleLIB.isActive() begin
			local settings = AncientJungleLIB.Settings(); 	
						
			if not pc.is_gm() then
				AncientJungleLIB.setWaitingTime()
			end
		end
	end
end	
		
		
