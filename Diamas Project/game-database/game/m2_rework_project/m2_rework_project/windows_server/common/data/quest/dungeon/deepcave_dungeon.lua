define ENTER_MAP_INDEX 219
define DUNGEON_MAP_INDEX 213
define MIN_LEVEL 75
define MAX_LEVEL 100
define LOCAL_BASE_X 536
define LOCAL_BASE_Y 495
define ENTRY_ITEM 71175
define ONLY_GROUP_CAN_ENTER false
define PATH_TO_REGENFILES "data/dungeon/deep_cave/"
define EXIT_MAP_INDEX 219
define EXIT_MAP_X 370
define EXIT_MAP_Y 297

quest deep_cave begin
	state start begin
		when 9214.chat."Dungeon!" with pc.get_map_index() == ENTER_MAP_INDEX begin
			enterDungeon(DUNGEON_ID_DEEP_CAVE, DUNGEON_MAP_INDEX, LOCAL_BASE_X, LOCAL_BASE_Y, ONLY_GROUP_CAN_ENTER, EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when 70007.use with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			pc.warp(ENTER_MAP_INDEX, 37000, 29700)
		end

		when login with pc.get_map_index() == DUNGEON_MAP_INDEX and not pc.in_dungeon() begin
			pc.warp(EXIT_MAP_INDEX, 37000, 29700)
		end

		when login with pc.in_dungeon(DUNGEON_MAP_INDEX) and isDungeonLeader() and getDungeonStage() == 0 begin
			d.regen_file(PATH_TO_REGENFILES .. "stage_1.txt")
			d.notice("Easter dungeon: 10 minutes left!")
			pc.setqf("dungeon_start_time", get_global_time())
			d.add_boss_vnum(676)
			d.add_boss_vnum(6761)
			--d.notice("Stage 1: Kill the monster!")
			deep_cave.setDungeonTimeout(60 * 60)
			setDungeonWarpLocation(ENTER_MAP_INDEX, 370, 297)
			setDungeonExitCounter(10)
			increaseDungeonStage()
			setNeededMonsterCount(1)
		end

		when kill with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local race = npc.race

			if stage == 1 and race == 8416 then
				d.regen_file(PATH_TO_REGENFILES .. "stage_2.txt")
				--d.notice("Stage 1: Kill all Metinstones!")

				--deep_cave.setTimeout(5 * 60)
				increaseDungeonStage()
				setNeededMonsterCount(5)
			elseif stage == 2 and race == 8416 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--deep_cave.clearTimeout()
					--d.purge()
					--deep_cave.jumpAll(658, 249)
					--deep_cave.setTimeout(5 * 60)

					d.regen_file(PATH_TO_REGENFILES .. "stage_3.txt")
					--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 3 and race == 672 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--deep_cave.clearTimeout()

					d.spawn_mob(674, 536, 492)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 4 and race == 674 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.regen_file(PATH_TO_REGENFILES .. "stage_8.txt")
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 5 and race == 8417 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--deep_cave.clearTimeout()

					d.spawn_mob(674, 536, 492)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 6 and race == 674 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.regen_file(PATH_TO_REGENFILES .. "stage_11.txt")
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 7 and race == 8418 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--deep_cave.clearTimeout()

					d.spawn_mob(675, 536, 492)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 8 and race == 675 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
					d.regen_file(PATH_TO_REGENFILES .. "stage_14.txt")
				--d.notice("Stage 1: Kill all Monsters!")
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 9 and race == 673 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_boss.txt")
					increaseDungeonStage()
				end
			elseif stage == 10 and race == 676 or race == 6761 then
				pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 5) == 0 then
					--notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(676), pc.getqf("boss_kills"))
					notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(676), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
					--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
				end
				
				dungeon_info.set_ranking(DUNGEON_ID_DEEP_CAVE, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(race))
				pc.update_dungeon_progress(DUNGEON_ID_DEEP_CAVE)
				d.completed()
				--setDungeonExitCounter(30)
				setDungeonWarpLocation(ENTER_MAP_INDEX, 370, 297)
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
