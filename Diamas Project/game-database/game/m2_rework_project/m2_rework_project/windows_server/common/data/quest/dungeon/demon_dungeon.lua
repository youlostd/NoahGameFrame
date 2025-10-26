define ENTER_MAP_INDEX 167
define DUNGEON_MAP_INDEX 231
define MIN_LEVEL 150
define MAX_LEVEL 150
define LOCAL_BASE_X 332
define LOCAL_BASE_Y 266
define ENTRY_ITEM 71175
define ONLY_GROUP_CAN_ENTER false
define PATH_TO_REGENFILES "data/dungeon/demon_dungeon/"
define EXIT_MAP_INDEX 167
define EXIT_MAP_X 264
define EXIT_MAP_Y 184

quest demon_dungeon begin
	state start begin
		when 9256.chat."Dungeon!" with pc.get_map_index() == ENTER_MAP_INDEX begin
			enterDungeon(DUNGEON_ID_DEMON, DUNGEON_MAP_INDEX, LOCAL_BASE_X, LOCAL_BASE_Y, ONLY_GROUP_CAN_ENTER, EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when 70007.use with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			pc.warp(ENTER_MAP_INDEX, 26400, 18400)
		end

		when login with pc.get_map_index() == DUNGEON_MAP_INDEX and not pc.in_dungeon() begin
			pc.warp(EXIT_MAP_INDEX, EXIT_MAP_X, EXIT_MAP_Y)
		end

		when login with pc.in_dungeon(DUNGEON_MAP_INDEX) and isDungeonLeader() and getDungeonStage() == 0 begin
			d.regen_file(PATH_TO_REGENFILES .. "regen_1thfloor.txt")
			d.notice("Easter dungeon: 10 minutes left!")
			--d.notice("Stage 1: Kill all bosses in 45 seconds")
			pc.setqf("dungeon_start_time", get_global_time())
			d.add_boss_vnum(283)
			d.add_boss_vnum(2831)
			demon_dungeon.setDungeonTimeout(60 * 60)
			--demon_dungeon.setTimeout(45)
			setDungeonWarpLocation(ENTER_MAP_INDEX, 264, 184)
			increaseDungeonStage()
			setDungeonExitCounter(10)
			setNeededMonsterCount(24)
		end

		when kill with pc.in_dungeon(DUNGEON_MAP_INDEX) begin
			local stage = getDungeonStage()
			local race = npc.race

			if stage == 1 and race == 273 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--demon_dungeon.clearTimeout()
					--d.notice("Stage 2: Kill all bosses")

					d.spawn_mob(8401, 291, 256)
					d.spawn_mob(8401, 357, 233)
					d.spawn_mob(8401, 386, 272)
					d.spawn_mob(8401, 350, 298)
					d.spawn_mob(8401, 359, 272)
					d.spawn_mob(8401, 369, 245)

					increaseDungeonStage()
					setNeededMonsterCount(6)
				end
			elseif stage == 2 and race == 8401 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.regen_file(PATH_TO_REGENFILES .. "regen_2thfloor.txt")

					--d.notice("Stage 3: Kill all monsters in 45 seconds")
					--demon_dungeon.setTimeout(45)
					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 3 and race == 277 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--demon_dungeon.clearTimeout()
					--d.notice("Stage 4: Drop item 30721")

					d.spawn_mob(274, 142, 110)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 4 and race == 274 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.set_rejoin_pos(73300, 52300)
					d.jump_all_delayed(733, 523, 3)
					--d.jump_all(733, 523)
					d.regen_file(PATH_TO_REGENFILES .. "regen_3thfloor.txt")
					increaseDungeonStage()
					setNeededMonsterCount(28)
				end
			elseif stage == 5 and race == 279 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.spawn_mob(8401, 731, 523)
					d.spawn_mob(8401, 770, 521)
					d.spawn_mob(8401, 802, 519)
					d.spawn_mob(8401, 842, 496)
					d.spawn_mob(8401, 846, 439)
					d.spawn_mob(8401, 851, 401)
					--demon_dungeon.setTimeout(60)
					increaseDungeonStage()
					setNeededMonsterCount(6)
				end
			elseif stage == 6 and race == 8401 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--demon_dungeon.clearTimeout()
					--d.notice("Stage 7: Kill 10 metinstones")

					d.regen_file(PATH_TO_REGENFILES .. "regen_4thfloor.txt")
					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 7 and race == 281 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--demon_dungeon.clearTimeout()
					--d.notice("Stage 8: Kill 5 minibosses and put the item on 9255")

					d.spawn_mob(274, 733, 167)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 8 and race == 274 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.set_rejoin_pos(85300, 85300)
					d.jump_all_delayed(853, 853, 3)
					--d.jump_all(853, 853)
					d.regen_file(PATH_TO_REGENFILES .. "regen_5thfloor.txt")
					increaseDungeonStage()
					setNeededMonsterCount(40)
				end
			elseif stage == 9 and race == 275 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

				d.spawn_mob(8401, 743, 862)
				d.spawn_mob(8401, 706, 873)
				d.spawn_mob(8401, 723, 893)
				d.spawn_mob(8401, 681, 852)
				d.spawn_mob(8401, 732, 838)
				d.spawn_mob(8401, 747, 890)
				increaseDungeonStage()
				setNeededMonsterCount(6)
				end
			elseif stage == 10 and race == 8401 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.regen_file(PATH_TO_REGENFILES .. "regen_6thfloor.txt")
					increaseDungeonStage()
					setNeededMonsterCount(35)
				end
			elseif stage == 11 and race == 276 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.spawn_mob(278, 708, 863)
					increaseDungeonStage()
					setNeededMonsterCount(1)
				end
			elseif stage == 12 and race == 278 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then

					d.regen_file(PATH_TO_REGENFILES .. "regen_7thfloor.txt")

					increaseDungeonStage()
					setNeededMonsterCount(5)
				end
			elseif stage == 13 and race == 282 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--demon_dungeon.clearTimeout()
					--d.notice("Stage 2: Kill all bosses")
					--d.notice("Stage 17")

				d.spawn_mob(8401, 743, 862)
				d.spawn_mob(8401, 706, 873)
				d.spawn_mob(8401, 723, 893)
				d.spawn_mob(8401, 681, 852)
				d.spawn_mob(8401, 732, 838)
				d.spawn_mob(8401, 747, 890)

					increaseDungeonStage()
					setNeededMonsterCount(6)
				end
			elseif stage == 14 and race == 8401 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					d.set_rejoin_pos(38600, 81300)
					d.jump_all_delayed(386, 813, 3)
					--d.jump_all(386, 813)

					d.set_unique("stone", d.spawn_mob(8402, 329, 862))
					server_loop_timer("check_stone_hp_2", 1, getDungeonMapIndex())
					--d.notice("Metin verschwindet 50% ")

					increaseDungeonStage()
				end
			elseif stage == 16 and race == 282 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 20")
					--demon_dungeon.clearTimeout()
					--d.notice("Stage 21: Kill the metinstone up to 25%")
					d.set_unique("stone", d.spawn_mob(8402, 329, 902))
					d.unique_set_hp_perc("stone", 50)
					increaseDungeonStage()
				end
			elseif stage == 18 and race == 280 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 21")
					--d.notice("Stage 23: Kill the metinstone")
					d.set_unique("stone", d.spawn_mob(8402, 307, 834))
					d.unique_set_hp_perc("stone", 25)
					increaseDungeonStage()
				end
			elseif stage == 19 and race == 8402 then
				--d.notice("Stage 22")
				--d.notice("Stage 24: kill the boss x")
				clear_server_timer("check_stone_hp_2", getDungeonMapIndex())

				d.spawn_mob(280, 314, 881)
				increaseDungeonStage()
				setNeededMonsterCount(1)
			elseif stage == 20 and race == 280 then
				setMonsterCount(getMonsterCount() + 1)

				if getMonsterCount() == getNeededMonsterCount() then
					--d.notice("Stage 24")
					--demon_dungeon.clearTimeout()
					--d.notice("Stage 26: Kill the end boss in 200 seconds")
					d.regen_file(PATH_TO_REGENFILES .. "regen_9thfloor.txt")
					increaseDungeonStage()
				end
			elseif stage == 21 and race == 283 or race == 2831 then
				--d.notice("Stage 25")
				pc.setqf("boss_kills", pc.getqf("boss_kills") + 1)
				if math.mod(pc.getqf("boss_kills"), 1) == 0 then
					notice_all("%s hat den %s bereits %s Mal gekillt (%s Sekunden)", pc.get_name(), c_mob_name(283), pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"))
					--notice_all("%s benötigte %s Sekunden", pc.get_name(), get_global_time() - pc.getqf("dungeon_start_time"))
				end
				
				dungeon_info.set_ranking(DUNGEON_ID_DEMON, pc.getqf("boss_kills"), get_global_time() - pc.getqf("dungeon_start_time"), d.get_boss_max_dmg(race))
				pc.update_dungeon_progress(DUNGEON_ID_DEMON)
				setCooldownUsingDungeonInfo(DUNGEON_ID_DEMON)

				d.completed()
				--setDungeonExitCounter(30)
				setDungeonWarpLocation(ENTER_MAP_INDEX, 264, 184)
				--d.notice("Easter dungeon: You will be teleported in 30 seconds.")
				server_timer("exit_dungeon_final", 15, pc.get_map_index())
			end
		end

		-- Server timers
		when check_stone_hp_2.server_timer begin
			if (d.select(get_server_timer_arg())) then
				local stage = getDungeonStage()

				--d.notice(string.format("%d - %d", stage, d.unique_get_hp_perc("stone")))

				if stage == 15 and d.unique_get_hp_perc("stone") <= 50 then
					--d.notice("Stage 19")

					d.purge_unique("stone")
					d.regen_file(PATH_TO_REGENFILES .. "regen_8thfloor.txt")
					--d.notice("kill 5 monster")

					--demon_dungeon.setTimeout(30)
					increaseDungeonStage()
					setNeededMonsterCount(5)
				elseif stage == 17 and d.unique_get_hp_perc("stone") <= 25 then
					--d.notice("Stage 20")
					--d.notice("Stage 22: Kill the three metinstones 8004, 8005 and 8006")
					d.purge_unique("stone")

					d.spawn_mob(280, 319, 827)

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
