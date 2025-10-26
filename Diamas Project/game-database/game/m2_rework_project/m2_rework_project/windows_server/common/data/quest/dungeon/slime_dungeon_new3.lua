define ENTER_MAP_INDEX 167
define DUNGEON_MAP_INDEX 242
define MIN_LEVEL 150
define MAX_LEVEL 150
define LOCAL_BASE_X 230
define LOCAL_BASE_Y 255
define ENTRY_ITEM 71175
define ONLY_GROUP_CAN_ENTER false
define PATH_TO_REGENFILES "data/dungeon/slime_dungeon_new_3/"
define EXIT_MAP_INDEX 167
define EXIT_MAP_X 264
define EXIT_MAP_Y 184

quest slime_dungeon_new_3 begin
	state start begin
		when 29236.chat."Dungeon!" with pc.get_map_index() == ENTER_MAP_INDEX begin
			enterDungeon(DUNGEON_ID_SLIME_3, DUNGEON_MAP_INDEX, LOCAL_BASE_X, LOCAL_BASE_Y, ONLY_GROUP_CAN_ENTER, EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when 70007.use with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			pc.warp(ENTER_MAP_INDEX, 26400, 18400)
		end

		when login with pc.get_map_index() == DUNGEON_MAP_INDEX and not pc.in_dungeon() begin
			pc.warp(EXIT_MAP_INDEX, 26400, 18400)
		end

		when login with pc.in_dungeon(DUNGEON_MAP_INDEX) and isDungeonLeader() and getDungeonStage() == 0 begin
			d.regen_file(PATH_TO_REGENFILES .. "stage_1.txt")
			d.spawn_mob(29237, 214, 269)
			d.spawn_mob(29237, 249, 295)
			d.spawn_mob(29237, 288, 266)
			d.spawn_mob(29237, 271, 229)
			d.spawn_mob(29237, 225, 222)
			d.notice("Easter dungeon: 10 minutes left!")
			--d.notice("Easter dungeon: You still have to defeat %s monsters to move on.", d.count_monster());
			pc.setqf("dungeon_start_time", get_global_time())
			d.add_boss_vnum(2768)
			d.add_boss_vnum(27681)
			--d.notice("Stage 1: Kill the monster!")
			slime_dungeon_new_3.setDungeonTimeout(60 * 60)
			setDungeonWarpLocation(ENTER_MAP_INDEX, 264, 184)
			setDungeonExitCounter(10)
			increaseDungeonStage()
			setNeededMonsterCount(30)
		end

		when kill with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local race = npc.race

			if stage == 1 and race == 2766 then
				setMonsterCount(getMonsterCount() + 1)

			local isLastMonster = getMonsterCount() == getNeededMonsterCount()
			
				if isLastMonster then
                    game.drop_item(30721, 1)
                --end
				--d.regen_file(PATH_TO_REGENFILES .. "stage_2.txt")
				--d.notice("Stage 1: Kill all Metinstones!")

				--slime_dungeon_new_3.setTimeout(5 * 60)
				--increaseDungeonStage()
				--setNeededMonsterCount(30)
				end
			elseif stage == 2 and race == 2767 then
				setMonsterCount(getMonsterCount() + 1)

				local isLastMonster = getMonsterCount() == getNeededMonsterCount()
				
				if isLastMonster then
                    game.drop_item(30721, 1)
                --end
					--slime_dungeon_new_3.clearTimeout()
					--d.purge()
					--slime_dungeon_new_3.jumpAll(658, 249)
					--slime_dungeon_new_3.setTimeout(5 * 60)

					--d.regen_file(PATH_TO_REGENFILES .. "stage_3.txt")
					--d.notice("Stage 1: Kill all Monsters!")
					--increaseDungeonStage()
					--setNeededMonsterCount(5)
				end
			elseif stage == 3 and race == 28430 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--slime_dungeon_new_3.clearTimeout()

					d.spawn_mob(28430, 255, 265)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 4 and race == 28430 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.regen_file(PATH_TO_REGENFILES .. "stage_8.txt")
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 5 and race == 2766 then
				setMonsterCount(getMonsterCount() + 1)

				local isLastMonster = getMonsterCount() == getNeededMonsterCount()
				
				if isLastMonster then
                    game.drop_item(30721, 1)
                --end
					--slime_dungeon_new_3.clearTimeout()

					--d.spawn_mob(28430, 255, 265)
					--increaseDungeonStage()
					--setNeededMonsterCount(1)
				end
			elseif stage == 6 and race == 28430 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.regen_file(PATH_TO_REGENFILES .. "stage_11.txt")
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 7 and race == 2767 then
				setMonsterCount(getMonsterCount() + 1)

				local isLastMonster = getMonsterCount() == getNeededMonsterCount()

				if isLastMonster then
                    game.drop_item(30721, 1)
                --end
					--slime_dungeon_new_3.clearTimeout()

					--d.regen_file(PATH_TO_REGENFILES .. "stage_14.txt")
					--increaseDungeonStage()
					--setNeededMonsterCount(5)
				end
			elseif stage == 8 and race == 28430 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.regen_file(PATH_TO_REGENFILES .. "stage_15.txt")
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 9 and race == 2766 then
				setMonsterCount(getMonsterCount() + 1)

				local isLastMonster = getMonsterCount() == getNeededMonsterCount()

				if isLastMonster then
                    game.drop_item(30721, 1)
                --end
					--d.regen_file(PATH_TO_REGENFILES .. "stage_boss.txt")
					--increaseDungeonStage()
				end
			elseif stage == 10 and race == 2768 or race == 27681 then
				pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 1) == 0 then
					--notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(719), pc.getqf("boss_kills"))
					notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(2768), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
					--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
				end

				dungeon_info.set_ranking(DUNGEON_ID_SLIME_3, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(race))
				pc.update_dungeon_progress(DUNGEON_ID_SLIME_3)
				d.completed()
				--setDungeonExitCounter(30)
				setDungeonWarpLocation(ENTER_MAP_INDEX, 264, 184)
				--d.notice("Easter dungeon: You will be teleported in 30 seconds.")
				server_timer("exit_dungeon_final", 15, pc.get_map_index())
			end
		end
	--end
--end	

	when 29237.take with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local vnum = item.vnum
			
			--if not isDungeonLeader() then
			--	say(gameforge.infected_garden_quest._30_sayReward)
			--	return
			--end
			
			if vnum == 30721 and stage == 1 then
				npc.purge()
				item.remove()
				d.regen_file(PATH_TO_REGENFILES .. "stage_2.txt")
				--pc.give_item2(30705)
				--d.spawn_mob(9224, 250, 427)
				increaseDungeonStage()
				setNeededMonsterCount(30)
			elseif vnum == 30721 and stage == 2 then
				npc.purge()
				item.remove()
				d.regen_file(PATH_TO_REGENFILES .. "stage_3.txt")
				--pc.give_item2(30705)
				--d.spawn_mob(9224, 250, 427)
				increaseDungeonStage()
				setNeededMonsterCount(5)
			elseif vnum == 30721 and stage == 5 then
				npc.purge()
				item.remove()
				d.spawn_mob(28430, 255, 265)
				increaseDungeonStage()
				setNeededMonsterCount(1)
			elseif vnum == 30721 and stage == 7 then
				npc.purge()
				item.remove()
				d.regen_file(PATH_TO_REGENFILES .. "stage_14.txt")
				--pc.give_item2(30705)
				--d.spawn_mob(9224, 250, 427)
				increaseDungeonStage()
				setNeededMonsterCount(5)
			elseif vnum == 30721 and stage == 9 then
				npc.purge()
				item.remove()
				d.regen_file(PATH_TO_REGENFILES .. "stage_boss.txt")
				--pc.give_item2(30705)
				--d.spawn_mob(9224, 250, 427)
				increaseDungeonStage()
			end
		end
	end	

	state __FUNCTIONS__ begin
		function clearTimeout()
			clear_server_timer("timeout", d.get_map_index())
		end

		function setTimeout(timeout)
			server_timer("timeout", timeout, d.get_map_index())
		end
		
		function clearDungeonTimeout()
			clear_server_timer("dungeon_timeout", d.get_map_index())
		end

		function setDungeonTimeout(timeout)
			server_timer("dungeon_timeout", timeout, d.get_map_index())
		end
	end
end
