define ENTER_MAP_INDEX 220
define DUNGEON_MAP_INDEX 210
define MIN_LEVEL 55
define MAX_LEVEL 75
define LOCAL_BASE_X 503
define LOCAL_BASE_Y 233
define ENTRY_ITEM 71175
define ONLY_GROUP_CAN_ENTER false
define PATH_TO_REGENFILES "data/dungeon/infected_garden_new/"
define EXIT_MAP_INDEX 220
define EXIT_MAP_X 227
define EXIT_MAP_Y 165

quest infected_garden_new begin
	state start begin
		when 9241.chat."Dungeon!" with pc.get_map_index() == ENTER_MAP_INDEX begin
			enterDungeon(DUNGEON_ID_GARDEN, DUNGEON_MAP_INDEX, LOCAL_BASE_X, LOCAL_BASE_Y, ONLY_GROUP_CAN_ENTER, EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when 70007.use with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			pc.warp(ENTER_MAP_INDEX, 22700, 16500)
		end

		when login with pc.get_map_index() == DUNGEON_MAP_INDEX and not pc.in_dungeon() begin
			pc.warp(EXIT_MAP_INDEX, 22700, 16500)
		end

		when login with pc.in_dungeon(DUNGEON_MAP_INDEX) and isDungeonLeader() and getDungeonStage() == 0 begin
			d.regen_file(PATH_TO_REGENFILES .. "stage_1.txt")
			d.notice("Easter dungeon: 10 minutes left!")
			pc.setqf("dungeon_start_time", get_global_time())
			d.add_boss_vnum(472)
			d.add_boss_vnum(4721)
			--d.notice("Stage 1: Kill the monster!")
			infected_garden_new.setDungeonTimeout(60 * 60)
			setDungeonWarpLocation(ENTER_MAP_INDEX, 227, 165)
			setDungeonExitCounter(10)
			increaseDungeonStage()
			setNeededMonsterCount(30)
		end

		when kill with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local race = npc.race

			if stage == 1 and race == 462 then
			
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
				d.regen_file(PATH_TO_REGENFILES .. "stage_2.txt")
				increaseDungeonStage()
				setNeededMonsterCount(30)
				end
			elseif stage == 2 and race == 466 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--infected_garden_new.clearTimeout()
					--d.purge()
					--infected_garden_new.jumpAll(658, 249)
					--infected_garden_new.setTimeout(5 * 60)

					d.regen_file(PATH_TO_REGENFILES .. "stage_3.txt")
					--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 3 and race == 8431 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--infected_garden_new.clearTimeout()

					d.regen_file(PATH_TO_REGENFILES .. "stage_8.txt")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 4 and race == 468 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.spawn_mob(469, 503, 233)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 5 and race == 469 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--infected_garden_new.clearTimeout()

					d.regen_file(PATH_TO_REGENFILES .. "stage_11.txt")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 6 and race == 473 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.regen_file(PATH_TO_REGENFILES .. "stage_13.txt")
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 7 and race == 474 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--infected_garden_new.clearTimeout()

					d.regen_file(PATH_TO_REGENFILES .. "stage_17.txt")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 8 and race == 477 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.spawn_mob(469, 503, 233)
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 9 and race == 469 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_14.txt")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 10 and race == 475 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_15.txt")
					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 11 and race == 8432 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_16.txt")
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 12 and race == 476 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_boss.txt")
					increaseDungeonStage()
				end
			elseif stage == 13 and race == 472 or race == 4721 then
				pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 5) == 0 then
					--notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(472), pc.getqf("boss_kills"))
					notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(472), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
					--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
				end
				
				dungeon_info.set_ranking(DUNGEON_ID_GARDEN, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(race))
				pc.update_dungeon_progress(DUNGEON_ID_GARDEN)
				d.completed()
				--setDungeonExitCounter(30)
				setDungeonWarpLocation(ENTER_MAP_INDEX, 227, 165)
				--d.notice("Easter dungeon: You will be teleported in 30 seconds.")
				server_timer("exit_dungeon_final", 15, pc.get_map_index())
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
