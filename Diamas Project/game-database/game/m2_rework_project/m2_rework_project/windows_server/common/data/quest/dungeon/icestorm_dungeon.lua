define ENTER_MAP_INDEX 167
define DUNGEON_MAP_INDEX 226
define MIN_LEVEL 150
define MAX_LEVEL 150
define LOCAL_BASE_X 193
define LOCAL_BASE_Y 229
define ENTRY_ITEM 71175
define ONLY_GROUP_CAN_ENTER false
define PATH_TO_REGENFILES "data/dungeon/icestorm_dungeon/"
define EXIT_MAP_INDEX 167
define EXIT_MAP_X 264
define EXIT_MAP_Y 184

quest icestorm_dungeon begin
	state start begin
		when 9259.chat."Dungeon!" with pc.get_map_index() == ENTER_MAP_INDEX begin
			enterDungeon(DUNGEON_ID_ICESTORM, DUNGEON_MAP_INDEX, LOCAL_BASE_X, LOCAL_BASE_Y, ONLY_GROUP_CAN_ENTER, EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when 70007.use with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			pc.warp(ENTER_MAP_INDEX, 26400, 18400)
		end

		when login with pc.get_map_index() == DUNGEON_MAP_INDEX and not pc.in_dungeon() begin
			pc.warp(EXIT_MAP_INDEX, 26400, 18400)
		end

		when login with pc.in_dungeon(DUNGEON_MAP_INDEX) and isDungeonLeader() and getDungeonStage() == 0 begin
			d.regen_file(PATH_TO_REGENFILES .. "stage_1.txt")
			d.notice("Easter dungeon: 10 minutes left!")
			--d.notice("Stage 1: Kill all bosses in 45 seconds")
			pc.setqf("dungeon_start_time", get_global_time())
			d.add_boss_vnum(9263)
			d.add_boss_vnum(9264)
			icestorm_dungeon.setDungeonTimeout(60 * 60)
			--icestorm_dungeon.setTimeout(45)
			setDungeonWarpLocation(ENTER_MAP_INDEX, 264, 184)
			increaseDungeonStage()
			setDungeonExitCounter(10)
			setNeededMonsterCount(15)
		end

		when kill with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local race = npc.race

			if stage == 1 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 2: Kill all bosses")

					d.spawn_mob(8300, 271, 243)
					d.spawn_mob(8300, 269, 258)
					d.spawn_mob(8300, 268, 270)
					d.spawn_mob(8300, 266, 282)
					d.spawn_mob(8300, 265, 292)
					d.spawn_mob(8300, 270, 250)

					increaseDungeonStage()
					setNeededMonsterCount(6)
				end
			elseif stage == 2 and race == 8300 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_3.txt")

					--d.notice("Stage 3: Kill all monsters in 45 seconds")
					--icestorm_dungeon.setTimeout(45)
					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 3 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 4: Drop item 30721")

					d.spawn_mob(8752, 263, 369)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 4 and race == 8752 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.regen_file(PATH_TO_REGENFILES .. "stage_5.txt")
					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 5 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.spawn_mob(8300, 129, 323)
					d.spawn_mob(8300, 109, 309)
					d.spawn_mob(8300, 93, 292)
					d.spawn_mob(8300, 80, 270)
					d.spawn_mob(8300, 65, 246)
					d.spawn_mob(8300, 55, 208)
					--icestorm_dungeon.setTimeout(60)
					increaseDungeonStage()
					setNeededMonsterCount(6)
				end
			elseif stage == 6 and race == 8300 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 7: Kill 10 metinstones")

					d.regen_file(PATH_TO_REGENFILES .. "stage_7.txt")
					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 7 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 8: Kill 5 minibosses and put the item on 9255")

					d.spawn_mob(8752, 170, 102)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 8 and race == 8752 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.regen_file(PATH_TO_REGENFILES .. "stage_9.txt")
					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 9 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

				d.spawn_mob(8300, 255, 93)
				d.spawn_mob(8300, 265, 85)
				d.spawn_mob(8300, 276, 76)
				d.spawn_mob(8300, 289, 76)
				d.spawn_mob(8300, 300, 77)
				d.spawn_mob(8300, 313, 78)
				increaseDungeonStage()
				setNeededMonsterCount(6)
				end
			elseif stage == 10 and race == 8300 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.regen_file(PATH_TO_REGENFILES .. "stage_15.txt")
					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 11 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.spawn_mob(8752, 373, 85)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 12 and race == 8752 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.regen_file(PATH_TO_REGENFILES .. "stage_18.txt")

					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 13 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 2: Kill all bosses")
					--d.notice("Stage 17")

					d.spawn_mob(8300, 392, 76)
					d.spawn_mob(8300, 384, 79)
					d.spawn_mob(8300, 376, 83)
					d.spawn_mob(8300, 369, 88)
					d.spawn_mob(8300, 363, 91)
					d.spawn_mob(8300, 356, 95)

					increaseDungeonStage()
					setNeededMonsterCount(6)
				end
			elseif stage == 14 and race == 8300 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.set_unique("stone", d.spawn_mob(8304, 373, 85))
					--d.notice("Timer started, getDungeonMapIndex: " .. tostring(getDungeonMapIndex()))
					server_loop_timer("check_stone_hp", 1, getDungeonMapIndex())
					increaseDungeonStage()
				end
			elseif stage == 16 and race == 8750 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 20")
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 21: Kill the metinstone up to 25%")
					d.set_unique("stone", d.spawn_mob(8304, 373, 85))
					d.unique_set_hp_perc("stone", 50)
					increaseDungeonStage()
				end
			elseif stage == 18 and race == 8753 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 21")
					--d.notice("Stage 23: Kill the metinstone")
					d.set_unique("stone", d.spawn_mob(8304, 373, 85))
					d.unique_set_hp_perc("stone", 25)
					increaseDungeonStage()
				end
			elseif stage == 19 and race == 8304 then
				--d.notice("Stage 22")
				--d.notice("Stage 24: kill the boss x")
				clear_server_timer("check_stone_hp", getDungeonMapIndex())

				d.spawn_mob(8753, 373, 85)
				increaseDungeonStage()
				setNeededMonsterCount(1)
			elseif stage == 20 and race == 8753 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 24")
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 26: Kill the end boss in 200 seconds")
					d.regen_file(PATH_TO_REGENFILES .. "stage_boss.txt")
					increaseDungeonStage()
				end
			elseif stage == 21 and race == 9263 or race == 9264 then
				--d.notice("Stage 25")
				pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 5) == 0 then
					--notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(9263), pc.getqf("boss_kills"))
					notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(9263), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
					--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
				end
				
				dungeon_info.set_ranking(DUNGEON_ID_ICESTORM, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(race))
				pc.update_dungeon_progress(DUNGEON_ID_ICESTORM)
				d.completed()
				--setDungeonExitCounter(30)
				setDungeonWarpLocation(ENTER_MAP_INDEX, 264, 184)
				--d.notice("Easter dungeon: You will be teleported in 30 seconds.")
				server_timer("exit_dungeon_final", 15, pc.get_map_index())
			end
		end

		-- Server timers
		when check_stone_hp.server_timer begin
			if (d.select(get_server_timer_arg())) then
				local stage = getDungeonStage()

				--d.notice(string.format("%d - %d", stage, d.unique_get_hp_perc("stone")))

				if stage == 15 and d.unique_get_hp_perc("stone") <= 50 then
					--d.notice("Stage 19")
					--d.notice("Stage 20: Kill all monsters in 30 seconds")
					d.purge_unique("stone")
					d.regen_file(PATH_TO_REGENFILES .. "stage_20.txt")

					--icestorm_dungeon.setTimeout(30)
					increaseDungeonStage()
					setNeededMonsterCount(15)
				elseif stage == 17 and d.unique_get_hp_perc("stone") <= 25 then
					--d.notice("Stage 20")
					--d.notice("Stage 22: Kill the three metinstones 8004, 8005 and 8006")
					d.purge_unique("stone")

					d.spawn_mob(8753, 373, 85)

					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
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
