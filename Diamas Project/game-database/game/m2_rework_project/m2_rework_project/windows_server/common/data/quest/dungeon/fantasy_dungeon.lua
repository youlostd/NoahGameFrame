define DUNGEON_MAP_INDEX 224
define ENTER_MAP_INDEX 163
define LOCAL_BASE_X 128
define LOCAL_BASE_Y 127
define MIN_LEVEL 100
define MAX_LEVEL 125
define ENTRY_ITEM 71175
define ONLY_GROUP_CAN_ENTER false
define PATH_TO_REGENFILES "data/dungeon/fantasydungeon/"
define EXIT_MAP_INDEX 163
define EXIT_MAP_X 934
define EXIT_MAP_Y 124

quest fantasy_dungeon begin
	state start begin
		when 20395.chat."Dungeon!" with pc.get_map_index() == ENTER_MAP_INDEX begin
			enterDungeon(DUNGEON_ID_NEVERLAND, DUNGEON_MAP_INDEX, LOCAL_BASE_X, LOCAL_BASE_Y, ONLY_GROUP_CAN_ENTER, EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when 70007.use with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			pc.warp(ENTER_MAP_INDEX, 93400, 12400)
		end

		when login with pc.get_map_index() == DUNGEON_MAP_INDEX and not pc.in_dungeon() begin
			pc.warp(EXIT_MAP_INDEX, 93400, 12400)
		end

		when login with pc.in_dungeon(DUNGEON_MAP_INDEX) and isDungeonLeader() and getDungeonStage() == 0 begin
			d.regen_file(PATH_TO_REGENFILES .. "stage_1.txt")
			d.notice("Easter dungeon: 10 minutes left!")
			pc.setqf("dungeon_start_time", get_global_time())
			d.add_boss_vnum(880)
			d.add_boss_vnum(8801)
			--d.notice("Stage 1: Kill all monsters in 120 seconds")
			fantasy_dungeon.setDungeonTimeout(60 * 60)
			--fantasy_dungeon.setTimeout(5 * 60)
			setDungeonWarpLocation(ENTER_MAP_INDEX, 934, 124)
			increaseDungeonStage()
			setDungeonExitCounter(10)
			setNeededMonsterCount(30)
		end

		when kill with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local race = npc.race

			if stage == 1 and race == 9330 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--fantasy_dungeon.clearTimeout()

					--d.notice("Stage 2: Kill the bosses 870")
					d.spawn_mob(870, 127, 127)
					d.spawn_mob(870, 138, 127)
					increaseDungeonStage()
					setNeededMonsterCount(2)
				end
			elseif stage == 2 and race == 870 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_2.txt")

					--d.notice("Stage 3: Kill all metinstones and collect the item 174384")
					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 3 and race == 9334 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then	
				d.regen_file(PATH_TO_REGENFILES .. "stage_5.txt")
					--d.notice("thank you for the item, the next stage will be beginning")
					--d.notice("Stage 4: Kill all monsters")

					--fantasy_dungeon.setTimeout(5 * 60)
					increaseDungeonStage()
					setNeededMonsterCount(30)
				end
			elseif stage == 4 and race == 9330 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--fantasy_dungeon.clearTimeout()

					d.regen_file(PATH_TO_REGENFILES .. "stage_3.txt")
					--d.notice("Stage 5: Kill the 10 metinstones")
					increaseDungeonStage()
					setNeededMonsterCount(10)
				end
			elseif stage == 5 and race == 8303 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_4.txt")
					--d.notice("Stage 6: Kill the miniboss 871, that must drop one item")
					increaseDungeonStage()
					setNeededMonsterCount(2)
				end
			elseif stage == 6 and race == 871 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then	
				d.spawn_mob(873, 127, 127)
				increaseDungeonStage()
				setNeededMonsterCount(1)
				end
			elseif stage == 7 and race == 873 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
			
				d.regen_file(PATH_TO_REGENFILES .. "stage_6.txt")
				--d.notice("thanks for the item, the final boss is coming soon")
				--d.notice("Stage 8: Kill all monsters in 60 seconds")

				--fantasy_dungeon.setTimeout(5 * 60)
				increaseDungeonStage()
				setNeededMonsterCount(30)
				end
			elseif stage == 8 and race == 9332 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--fantasy_dungeon.clearTimeout()

					--fantasy_dungeon.setTimeout(5 * 60)
					d.regen_file(PATH_TO_REGENFILES .. "stage_boss.txt")
					increaseDungeonStage()
				end
			elseif stage == 9 and race == 880 or race == 8801 then
				pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 5) == 0 then
					--notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(880), pc.getqf("boss_kills"))
					notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(880), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
					--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
				end
				
				dungeon_info.set_ranking(DUNGEON_ID_NEVERLAND, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(race))
				pc.update_dungeon_progress(DUNGEON_ID_NEVERLAND)
				d.completed()
				--setDungeonExitCounter(30)
				setDungeonWarpLocation(ENTER_MAP_INDEX, 934, 124)
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
