quest UnderwaterDungeon_zone begin
	state start begin
		when login with UnderwaterDungeonLIB.isInDungeon() begin
			local data = UnderwaterDungeonLIB.ReturnData(); local outside_coords_l = data["outSidePos"];
			
			d.set_warp_location(data["outside_index"], outside_coords_l["x"], outside_coords_l["y"]);
			if (party.is_party() and party.is_leader() or not party.is_party()) then
				d.regen_file("data/dungeon/underwater_dungeon/1th_floor_regen.txt");
				d.regen_file("data/dungeon/underwater_dungeon/fishes.txt");
				d.setf("underwaterDungeon_state", 1);
				
				server_timer("underwaterDungeon_timeOut", data["time_until_you_got_kicked"], d.get_map_index());
				loop_timer("underwaterDungeon_1floor_monster_count", 5);
				
d.notice(gameforge.UnderwaterDungeon_zone._10_dNotice, get_time_format(data["time_until_you_got_kicked"]))
				d.notice(gameforge.UnderwaterDungeon_zone._20_dNotice)
			end
		end
		
		when underwaterDungeon_1floor_monster_count.timer begin
			if (d.count_monster() == 0) then
				cleartimer("underwaterDungeon_1floor_monster_count");
				UnderwaterDungeonLIB.ClearDungeon();
				
				local data = UnderwaterDungeonLIB.ReturnData(); local metinPos = data["firstFloorMetinPos"]; local randomNumber = math.random(1, table.getn(metinPos));
				d.spawn_mob(8416, metinPos[randomNumber][1], metinPos[randomNumber][2]);
				
				d.notice(gameforge.UnderwaterDungeon_zone._30_dNotice)
			else
d.notice(gameforge.UnderwaterDungeon_zone._40_dNotice, d.count_monster());
			end
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 8416 and d.getf("underwaterDungeon_state") == 1 begin
			UnderwaterDungeonLIB.ClearDungeon();
			timer("UnderWater_2floor", 5);
			
			d.notice(gameforge.UnderwaterDungeon_zone._50_dNotice)
		end
		
		when UnderWater_2floor.timer begin
			d.setf("underwaterDungeon_state", d.getf("underwaterDungeon_state") + 1);
			d.regen_file("data/dungeon/underwater_dungeon/2th_floor_regen.txt");
			d.regen_file("data/dungeon/underwater_dungeon/fishes.txt");
			loop_timer("underwaterDungeon_2floor_monster_count", 5);
			
			d.notice(gameforge.UnderwaterDungeon_zone._60_dNotice, c_mob_name(9216))
			d.jump_all(567,201);
		end
		
		when underwaterDungeon_2floor_monster_count.timer begin
			if (d.count_monster() == 0) then
				cleartimer("underwaterDungeon_2floor_monster_count");
				UnderwaterDungeonLIB.ClearDungeon();
				
				d.spawn_mob(9216, 663, 253);
				
				d.notice(gameforge.UnderwaterDungeon_zone._70_dNotice)
d.notice(gameforge.UnderwaterDungeon_zone._80_dNotice, c_mob_name(9216));
			else
d.notice(gameforge.UnderwaterDungeon_zone._40_dNotice, d.count_monster());
			end
		end
		
		when 9216.click with UnderwaterDungeonLIB.isInDungeon() and d.getf("underwaterDungeon_state") == 2 begin
			if (d.getf("underWaterSecondFlorrMission") == 0) then
				local data = UnderwaterDungeonLIB.ReturnData(); local clamPos = data["clamPosCoords"]; local randomNumber = math.random(1, table.getn(clamPos));
				d.spawn_mob(9217, clamPos[randomNumber][1], clamPos[randomNumber][2]);
				d.setf("underWaterSecondFlorrMission", d.getf("underWaterSecondFlorrMission") + 1);
				d.regen_file("data/dungeon/underwater_dungeon/3th_floor_regen.txt");
				
				d.notice(gameforge.UnderwaterDungeon_zone._90_dNotice)
			end
			
			if (d.getf("underWaterSecondFlorrMission") == 1 and d.getf("underwaterDungeon_clamRec") == 1) then
				if (pc.count_item(30704) == 1) then
					d.notice(gameforge.UnderwaterDungeon_zone._100_dNotice)
					
					d.spawn_mob(8418, 668, 264); pc.remove_item(30704, 1);
					d.setf("underWaterSecondFlorrMission", d.getf("underWaterSecondFlorrMission") + 1);
				end
			end
		end
		
		when 9217.click with UnderwaterDungeonLIB.isInDungeon() and d.getf("underwaterDungeon_state") == 2 and d.getf("underwaterDungeon_clamRec") == 0 begin
d.notice(gameforge.UnderwaterDungeon_zone._110_dNotice, c_item_name(30704), c_mob_name(9216))
			d.setf("underwaterDungeon_clamRec", 1); pc.give_item2(30704, 1); npc.purge();
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 8418 and d.getf("underwaterDungeon_state") == 2 begin
d.notice(gameforge.UnderwaterDungeon_zone._120_dNotice, c_mob_name(986))
			d.spawn_mob(986, 668, 264);
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 986 and d.getf("underwaterDungeon_state") == 2 begin
d.notice(gameforge.UnderwaterDungeon_zone._130_dNotice, c_mob_name(986))
			UnderwaterDungeonLIB.ClearDungeon();
			
			timer("UnderWater_3floor", 5);
		end
		
		when UnderWater_3floor.timer begin
			d.setf("underwaterDungeon_state", d.getf("underwaterDungeon_state") + 1);
			local data = UnderwaterDungeonLIB.ReturnData(); local sealPos = data["sealPosCoords"];
			
			for index = 1, table.getn(sealPos), 1 do
				local strSeal = string.format(gameforge.UnderwaterDungeon_zone._140_stringFormat, index); local vid = d.spawn_mob(9218, sealPos[index][1], sealPos[index][2]);
				d.set_unique(strSeal, vid);
			end d.setf("thirdFloorSeal", 1);
			
			d.notice(gameforge.UnderwaterDungeon_zone._150_dNotice)
			d.jump_all(191, 697);
		end
		
		when 9218.click with UnderwaterDungeonLIB.isInDungeon() and d.getf("underwaterDungeon_state") == 3 begin
			if (npc.get_vid() == d.get_unique_vid("seal_1") and d.getf("thirdFloorSeal") == 1 and d.getf("underwaterDungeon_firstBlock") == 0) then
				d.regen_file("data/dungeon/underwater_dungeon/4th_floor_regen.txt");
				d.regen_file("data/dungeon/underwater_dungeon/fishes.txt");
				loop_timer("underwaterDungeon_3floor_seal_1", 5); d.setf("underwaterDungeon_firstBlock", 1);
				
				d.notice(gameforge.UnderwaterDungeon_zone._160_dNotice)
			end
			
			if (npc.get_vid() == d.get_unique_vid("seal_2") and d.getf("thirdFloorSeal") == 2 and d.getf("underwaterDungeon_secondBlock") == 0) then
				d.regen_file("data/dungeon/underwater_dungeon/5th_floor_stone.txt");
				d.setf("underwaterDungeon_secondBlock", 1); d.setf("underwaterDungeon_metinCount", 1);
				
				d.notice(gameforge.UnderwaterDungeon_zone._170_dNotice)
			end
			
			if (npc.get_vid() == d.get_unique_vid("seal_3") and d.getf("thirdFloorSeal") == 3 and d.getf("underwaterDungeon_thirdBlock") == 0) then
				d.regen_file("data/dungeon/underwater_dungeon/6th_floor_regen.txt");
				loop_timer("underwaterDungeon_3floor_seal_3", 5); d.setf("underwaterDungeon_thirdBlock", 1); d.setf("underwaterDungeon_waveCount", 1);
				
				d.notice(gameforge.UnderwaterDungeon_zone._180_dNotice)
			end
d.notice(gameforge.UnderwaterDungeon_zone._190_dNotice, d.getf("thirdFloorSeal"));
		end
		
		when underwaterDungeon_3floor_seal_1.timer begin
			if (d.count_monster() == 0) then
				cleartimer("underwaterDungeon_3floor_seal_1");
				
				game.drop_item(30705, 1);
				d.notice(gameforge.UnderwaterDungeon_zone._200_dNotice)
d.notice(gameforge.UnderwaterDungeon_zone._210_dNotice, c_item_name(30705));
			else
d.notice(gameforge.UnderwaterDungeon_zone._40_dNotice, d.count_monster());
			end
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 8417 and d.getf("underwaterDungeon_state") == 3 begin
			local metinFlag = d.getf("underwaterDungeon_metinCount");
			
			if (metinFlag < 4) then
				d.setf("underwaterDungeon_metinCount", metinFlag + 1);
				
d.notice(gameforge.UnderwaterDungeon_zone._220_dNotice, 4 - metinFlag)
			else
				game.drop_item(30705, 1);
				
				d.notice(gameforge.UnderwaterDungeon_zone._200_dNotice)
d.notice(gameforge.UnderwaterDungeon_zone._210_dNotice, c_item_name(30705));
			end
		end
		
		when underwaterDungeon_3floor_seal_3.timer begin
			if (d.count_monster() == 0) then
				if (d.getf("underwaterDungeon_waveCount") < 1) then
					d.setf("underwaterDungeon_waveCount", d.getf("underwaterDungeon_waveCount") + 1);
					d.regen_file("data/dungeon/underwater_dungeon/6th_floor_regen.txt");
					
					d.notice(gameforge.UnderwaterDungeon_zone._230_dNotice)
				else
					cleartimer("underwaterDungeon_3floor_seal_3"); d.spawn_mob(985, 260, 701);
					
					d.notice(gameforge.UnderwaterDungeon_zone._70_dNotice)
d.notice(gameforge.UnderwaterDungeon_zone._240_dNotice, c_mob_name(985));
				end
			else
			end
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 985 and d.getf("underwaterDungeon_state") == 3 begin
			d.notice(gameforge.UnderwaterDungeon_zone._260_dNotice)
d.notice(gameforge.UnderwaterDungeon_zone._270_dNotice, c_item_name(30705))
			
			game.drop_item(30705);
		end
		
		when 9218.take with item.get_vnum() == 30705 and UnderwaterDungeonLIB.isInDungeon() and d.getf("underwaterDungeon_state") == 3 begin
			if (npc.get_vid() == d.get_unique_vid("seal_1") and d.getf("underwaterDungeon_firstBlock") == 1 and d.getf("thirdFloorSeal") == 1) then
				d.setf("thirdFloorSeal", d.getf("thirdFloorSeal") + 1);
				d.notice(gameforge.UnderwaterDungeon_zone._280_dNotice) npc.kill(); pc.remove_item(30705, 1); return;
			end
			
			if (npc.get_vid() == d.get_unique_vid("seal_2") and d.getf("underwaterDungeon_secondBlock") == 1 and d.getf("thirdFloorSeal") == 2) then
				d.setf("thirdFloorSeal", d.getf("thirdFloorSeal") + 1);
				d.notice(gameforge.UnderwaterDungeon_zone._280_dNotice) npc.kill(); pc.remove_item(30705, 1); return;
			end
			
			if (npc.get_vid() == d.get_unique_vid("seal_3") and d.getf("underwaterDungeon_thirdBlock") == 1 and d.getf("thirdFloorSeal") == 3) then
				UnderwaterDungeonLIB.ClearDungeon();
				d.notice(gameforge.UnderwaterDungeon_zone._290_dNotice) npc.kill(); pc.remove_item(30705, 1); timer("UnderWater_4floor", 5); return;
			end
d.notice(gameforge.UnderwaterDungeon_zone._190_dNotice, d.getf("thirdFloorSeal"));
		end
		
		when UnderWater_4floor.timer begin
			d.setf("underwaterDungeon_state", d.getf("underwaterDungeon_state") + 1);
			timer("underwater4thFloorTimer", time_min_to_sec(5));
			d.regen_file("data/dungeon/underwater_dungeon/7th_floor_npc.txt");
			d.regen_file("data/dungeon/underwater_dungeon/7th_floor_regen.txt");
			d.regen_file("data/dungeon/underwater_dungeon/fishes.txt");
			
			local specifiedMobs = {980, 980, 980};
			d.setf("underwater4thMob", specifiedMobs[math.random(1, table.getn(specifiedMobs))]); d.setf("underwater4thMobCount", 1); d.setf("underwaterDungeon_specifiedMobCheck", 1);

d.notice(gameforge.UnderwaterDungeon_zone._300_dNotice, c_mob_name(d.getf("underwater4thMob")))
			d.jump_all(658, 767);
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == d.getf("underwater4thMob") and d.getf("underwaterDungeon_state") == 4 and d.getf("underwaterDungeon_specifiedMobCheck") == 1 begin
			local mobCount = d.getf("underwater4thMobCount");
			
			if (mobCount < 23) then
				d.setf("underwater4thMobCount", mobCount + 1);
d.notice(gameforge.UnderwaterDungeon_zone._310_dNotice, 23 - mobCount)
			else
				UnderwaterDungeonLIB.ClearDungeon();
				d.regen_file("data/dungeon/underwater_dungeon/7th_floor_npc.txt");
				d.spawn_mob(988, 842, 713); d.spawn_mob(988, 831, 696); d.setf("underwater4thTwoBoss", 1); d.setf("underwaterDungeon_specifiedMobCheck", 0);
				
				d.notice(gameforge.UnderwaterDungeon_zone._320_dNotice)
d.notice(gameforge.UnderwaterDungeon_zone._330_dNotice, c_mob_name(988));
			end
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() != d.getf("underwater4thMob") and d.getf("underwaterDungeon_state") == 4 and d.getf("underwaterDungeon_specifiedMobCheck") == 1 begin
			d.notice(gameforge.UnderwaterDungeon_zone._340_dNotice)
			
			UnderwaterDungeonLIB.ClearDungeon();
			server_timer("underwaterDungeon_timeOut", 3, d.get_map_index())
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 988 and d.getf("underwaterDungeon_state") == 4 begin
			local bossFlag = d.getf("underwater4thTwoBoss");
			
			if (bossFlag < 2) then
				d.setf("underwater4thTwoBoss", bossFlag + 1);
d.notice(gameforge.UnderwaterDungeon_zone._350_dNotice, c_mob_name(988))
			else
d.notice(gameforge.UnderwaterDungeon_zone._360_dNotice, c_mob_name(988))
d.notice(gameforge.UnderwaterDungeon_zone._370_dNotice, c_item_name(30706), c_mob_name(9219))
				game.drop_item(30706);
			end
		end
		
		when 9219.take with item.get_vnum() == 30706 and UnderwaterDungeonLIB.isInDungeon() and d.getf("underwaterDungeon_state") == 4 begin
			item.remove()
			npc.kill(); cleartimer("underwater4thFloorTimer"); d.spawn_mob(8418, 722, 793);
			
			d.notice(gameforge.UnderwaterDungeon_zone._380_dNotice)
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 8418 and d.getf("underwaterDungeon_state") == 4 begin
			d.regen_file("data/dungeon/underwater_dungeon/8th_floor_regen.txt");
			loop_timer("underwaterDungeon_4floor_monster_count", 5);
			
			d.notice(gameforge.UnderwaterDungeon_zone._390_dNotice)
		end
		
		when underwaterDungeon_4floor_monster_count.timer begin
			if (d.count_monster() == 0) then
				if (math.random(1, 1) == 1) then
					cleartimer("underwaterDungeon_4floor_monster_count");
					UnderwaterDungeonLIB.ClearDungeon();
					
					local position = {{844, 894}, {852, 904}, {855, 919}, {828, 922}, {814, 908}, {822, 893}}
		
					local n = number(1,1)
					for i = 1, 1 do
						if (i != n)
						then
							d.set_unique("fake"..i, d.spawn_mob( 8417, position[i][1], position[i][2]))
						end
					end

					local vid = d.spawn_mob( 8417, position[n][1], position[n][2])
					d.set_unique ("real",vid)
					if (is_test_server()) then
						-- real stone pos
						chat (position[n][1],position[n][2])
					end
					
					d.notice(gameforge.UnderwaterDungeon_zone._400_dNotice)
				else
					d.regen_file("data/dungeon/underwater_dungeon/8th_floor_regen.txt");
					d.notice(gameforge.UnderwaterDungeon_zone._410_dNotice)
				end
			else
d.notice(gameforge.UnderwaterDungeon_zone._40_dNotice, d.count_monster());
			end
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and npc.get_race() == 8417 and d.getf("underwaterDungeon_state") == 4 begin
			if d.is_unique_dead("real") then
				d.notice(gameforge.UnderwaterDungeon_zone._420_dNotice)
				
				UnderwaterDungeonLIB.ClearDungeon();
				d.spawn_mob(987, 835, 907);
			else
				d.notice(gameforge.UnderwaterDungeon_zone._430_dNotice)
			end
		end
		
		when kill with UnderwaterDungeonLIB.isInDungeon() and not npc.is_pc() and npc.get_race() == 987 and d.getf("underwaterDungeon_state") == 4 begin
			d.notice(gameforge.UnderwaterDungeon_zone._440_dNotice)
			notice_all(gameforge.UnderwaterDungeon_zone._190_notice_all1 .. pc.get_name() .. gameforge.UnderwaterDungeon_zone._190_notice_all2) --- GLOBAL NOTICE
			if party.is_party() then
				highscore.register("ud_pt_time", get_global_time() - d.getf("start_time"), 0)
			else
				highscore.register("ud_time", get_global_time() - d.getf("start_time"), 0)
			end
			pc.setqf("987_kills", pc.getqf("987_kills") +  1)			
			UnderwaterDungeonLIB.ClearDungeon();
			server_timer("underwaterDungeon_timeOut", 120, d.get_map_index())
		end
		
		when underwaterDungeon_timeOut.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.notice(gameforge.UnderwaterDungeon_zone._460_dNotice)
				d.exit_all();
			end
		end
		
		when underwater4thFloorTimer.timer begin
			d.notice(gameforge.UnderwaterDungeon_zone._460_dNotice)
			d.exit_all();
		end
		
		when 9220.chat.gameforge.UnderwaterDungeon_zone._470_npcChat with not UnderwaterDungeonLIB.isInDungeon() begin
say_title(string.format(gameforge.UnderwaterDungeon_zone._480_sayTitle, c_mob_name(npc.get_race())))
			say(gameforge.UnderwaterDungeon_zone._490_say)
			if select(gameforge.snow_dungeon._250_select, gameforge.UnderwaterDungeon_zone._500_select) == 1 then
				if UnderwaterDungeonLIB.CheckInfo() then
					UnderwaterDungeonLIB.EnterInDungeon();
					d.setf("start_time", get_global_time())
				end
			end
		end
	end
end