define ENTER_MAP_INDEX 161
define DUNGEON_MAP_INDEX 225
define MIN_LEVEL 125
define MAX_LEVEL 150
define LOCAL_BASE_X 128
define LOCAL_BASE_Y 127
define ENTRY_ITEM 71175
define ONLY_GROUP_CAN_ENTER false
define PATH_TO_REGENFILES "data/dungeon/wooddungeon/"
define EXIT_MAP_INDEX 161
define EXIT_MAP_X 402
define EXIT_MAP_Y 704

quest wood_dungeon begin
	state start begin
		when 9253.chat."Dungeon!" with pc.get_map_index() == ENTER_MAP_INDEX begin
			enterDungeon(DUNGEON_ID_WOOD, DUNGEON_MAP_INDEX, LOCAL_BASE_X, LOCAL_BASE_Y, ONLY_GROUP_CAN_ENTER, EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when 70007.use with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			pc.warp(ENTER_MAP_INDEX, 40200, 70400)
		end

		when login with pc.get_map_index() == DUNGEON_MAP_INDEX and not pc.in_dungeon() begin
			pc.warp(EXIT_MAP_INDEX, 40200, 70400)
		end

		when login with pc.in_dungeon(DUNGEON_MAP_INDEX) and isDungeonLeader() and getDungeonStage() == 0 begin
			d.regen_file(PATH_TO_REGENFILES .. "stage_1.txt")
			d.notice("Easter dungeon: 10 minutes left!")
			--d.notice("Stage 1: Kill all bosses in 45 seconds")
			pc.setqf("dungeon_start_time", get_global_time())
			d.add_boss_vnum(878)
			d.add_boss_vnum(8781)
			wood_dungeon.setDungeonTimeout(60 * 60)
			--wood_dungeon.setTimeout(45)
			setDungeonWarpLocation(ENTER_MAP_INDEX, 402, 704)
			increaseDungeonStage()
			setDungeonExitCounter(10)
			setNeededMonsterCount(15)
		end

		when kill with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local race = npc.race

			if stage == 1 and race == 862 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "stage_3.txt")

					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 2 and race == 862 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.spawn_mob(874, 127, 127)

					--d.notice("Stage 3: Kill all monsters in 45 seconds")
					--wood_dungeon.setTimeout(45)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 3 and race == 874 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--wood_dungeon.clearTimeout()
					--d.notice("Stage 4: Drop item 30721")

					d.spawn_mob(865, 110, 127)
					d.spawn_mob(865, 112, 118)
					d.spawn_mob(865, 120, 111)
					d.spawn_mob(865, 131, 110)
					d.spawn_mob(865, 140, 115)
					d.spawn_mob(865, 144, 123)
					d.spawn_mob(865, 143, 133)
					d.spawn_mob(865, 137, 139)
					d.spawn_mob(865, 128, 143)
					d.spawn_mob(865, 115, 140)
					increaseDungeonStage()
					setNeededMonsterCount(10)
				end
			elseif stage == 4 and race == 865 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

				d.regen_file(PATH_TO_REGENFILES .. "stage_5.txt")
				increaseDungeonStage()
				setNeededMonsterCount(15)
				end
			elseif stage == 5 and race == 864 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

				d.regen_file(PATH_TO_REGENFILES .. "stage_6.txt")
				increaseDungeonStage()
				setNeededMonsterCount(15)
				end
			elseif stage == 6 and race == 867 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--wood_dungeon.clearTimeout()
					--d.notice("Stage 7: Kill 10 metinstones")

					d.spawn_mob(872, 127, 127)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 7 and race == 872 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--wood_dungeon.clearTimeout()
					--d.notice("Stage 8: Kill 5 minibosses and put the item on 9255")

					d.spawn_mob(877, 110, 127)
					d.spawn_mob(877, 112, 118)
					d.spawn_mob(877, 120, 111)
					d.spawn_mob(877, 131, 110)
					d.spawn_mob(877, 140, 115)
					d.spawn_mob(877, 144, 123)
					d.spawn_mob(877, 143, 133)
					d.spawn_mob(877, 137, 139)
					d.spawn_mob(877, 128, 143)
					d.spawn_mob(877, 115, 140)
					increaseDungeonStage()
					setNeededMonsterCount(10)
				end
			elseif stage == 8 and race == 877 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

				d.regen_file(PATH_TO_REGENFILES .. "stage_9.txt")
				increaseDungeonStage()
				setNeededMonsterCount(15)
				end
			elseif stage == 9 and race == 868 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
				d.regen_file(PATH_TO_REGENFILES .. "stage_14.txt")
				increaseDungeonStage()
				setNeededMonsterCount(15)
				end
			elseif stage == 10 and race == 869 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 15: Kill all metinstones")
					d.spawn_mob(876, 127, 127)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 11 and race == 876 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 16: Kill miniboss x")
					d.regen_file(PATH_TO_REGENFILES .. "stage_15.txt")
					increaseDungeonStage()
					setNeededMonsterCount(15)
				end
			elseif stage == 12 and race == 875 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--icestorm_dungeon.clearTimeout()
					--d.notice("Stage 26: Kill the end boss in 200 seconds")
					d.regen_file(PATH_TO_REGENFILES .. "stage_boss.txt")
					increaseDungeonStage()
				end
			elseif stage == 13 and race == 878 or race == 8781 then
				pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 5) == 0 then
					--notice_all("%s hat den %s bereits %s Mal gekillt", pc.get_name(), c_mob_name(878), pc.getqf("boss_kills"))
					notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(878), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
					--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
				end
				
				dungeon_info.set_ranking(DUNGEON_ID_WOOD, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(race))
				pc.update_dungeon_progress(DUNGEON_ID_WOOD)
				d.completed()
				--setDungeonExitCounter(30)
				setDungeonWarpLocation(ENTER_MAP_INDEX, 402, 704)
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
