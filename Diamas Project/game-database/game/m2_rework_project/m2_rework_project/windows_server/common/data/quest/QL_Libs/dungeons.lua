function isBlockedDungeon()
	-- If all dungeons are blocked globally
	if game.get_event_flag("block_dungeons") == 1 then
		return true
	end

	return false
end

function getDungeonMapIndex()
	if party.is_party() then
		return party.getf("map_index")
	else
		return pc.getf("dungeon", "map_index")
	end
end

function isDungeonLeader()
	return party.is_party() and party.is_leader() or not party.is_party()
end

function getDungeonBaseMapIndex()
	return d.get_base_map_index()
end

function setDungeonMapIndex(base_map_index)

	if party.is_party() then
		party.setf("map_index", d.get_map_index())
	else
		pc.setf("dungeon", "map_index", d.get_map_index())
	end
end

function setBasePositions(local_x, local_y)
	d.setf("local_x", local_x)
	d.setf("local_y", local_y)
end

function setMonsterCount(count)
	d.setf("monster_count", count)
end

function increaseMonsterCount()
	setMonsterCount(getMonsterCount() + 1)
end

function getMonsterCount()
	return d.getf("monster_count")
end

function setNeededMonsterCount(count)
	d.setf("needed_monster_count", count)
end

function getNeededMonsterCount()
	return d.getf("needed_monster_count")
end

function getBasePositions()
	return getDungeonBaseMapIndex(), d.getf("local_x"), d.getf("local_y")
end

function setDungeonWarpLocation(_map_index, _x, _y)
	map_index, x, y = getBasePositions()
	d.set_warp_location(_map_index or map_index, _x or x, _y or y)
end

function isInDungeonBase(map_index)
	return pc.get_map_index() == map_index
end

function isInDungeonRange(map_index)
	return (pc.get_map_index() >= map_index * 10000) and (pc.get_map_index() < (map_index + 1) * 10000)
end

function isInDungeon(map_index)
	if not pc.in_dungeon() then
		return false
	end

	if pc.get_map_index() < 10000 then
		return false
	end

	return isInDungeonRange(map_index or getDungeonBaseMapIndex())
end


function getDungeonStage()
	return d.getf("stage")
end

function setDungeonStage(stage)
	d.setf("stage", stage)
end

function increaseDungeonStage()
	setMonsterCount(0)
	setNeededMonsterCount(0)
	setDungeonStage(getDungeonStage() + 1)
	return getDungeonStage()
end

function enterDungeon(dungeonId, map_index, local_x, local_y, only_group, exit_map_index, exit_map_x, exit_map_y)
	if isBlockedDungeon() then
		say(gameforge.dungeon_crystal.deactivated)
		return
	end

	if only_group and not party.is_party() then
		if not pc.is_gm() then
			say(gameforge.dungeon_crystal.party_err)
			return
		end

		say(gameforge.dungeon_crystal.party_err_but_gm)
	end

	if dungeon_info.get_ticket_vnum(dungeonId) > 0 and pc.count_item(dungeon_info.get_ticket_vnum(dungeonId)) < 1 then
		say(string.format(gameforge.owl_dungeon._60_say, c_item_name(71175)))
		--setskin(NOWINDOW)
		return
	end

	
	--if pc.has_active_dungeon() then
	--	say("You still have an active dungeon.")
	--	say("Complete it or wait for it to expire.")
	--	return;
	--end

	if party.is_party() then
		if party.is_leader() then
			local pids = {party.get_member_pids()}
			local t = false
			local name = ""

			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
					if pc.get_level() < dungeon_info.get_min_level(dungeonId) or pc.get_level() > dungeon_info.get_max_level(dungeonId) then
						t = true
						name = pc.get_name()
					end
				q.end_other_pc_block()

				if t then
					say(string.format(gameforge.dungeon_crystal.group_need_level, name, dungeon_info.get_min_level(dungeonId), dungeon_info.get_max_level(dungeonId)))
					return
				end
			end
			for i, pid in next, pids, nil do
				q.begin_other_pc_block(pid)
					if get_global_time() < dungeon_info.get_cooldown_end(dungeonId) then
						t = true
						name = pc.get_name()
					end
				q.end_other_pc_block()
				
				if t then
					say(string.format("%s still has cooldown for this dungeon.", name))
					return
				end

			end

			
		else
			say(gameforge.infected_garden_quest._30_sayReward)
			return
		end
	else
		if pc.get_level() < dungeon_info.get_min_level(dungeonId) or pc.get_level() > dungeon_info.get_max_level(dungeonId) then
			say(string.format(gameforge.dungeon_crystal.need_level, dungeon_info.get_min_level(dungeonId), dungeon_info.get_max_level(dungeonId)))
			return
		end
		
				
		if get_global_time() < dungeon_info.get_cooldown_end(dungeonId) then
			say("The cooldown for this dungeon has not expired yet.")
			return;
		end
	end

	if dungeon_info.get_ticket_vnum(dungeonId) > 0 and dungeon_info.get_ticket_count(dungeonId) > 0 then
		pc.remove_item(dungeon_info.get_ticket_vnum(dungeonId) , dungeon_info.get_ticket_count(dungeonId))
	end

	if party.is_party() then
		d.new_jump_party(map_index, local_x, local_y)
	else
		d.new_jump(map_index, local_x, local_y)
	end
	d.set_rejoin_pos(local_x*100, local_y*100)
	d.set_exit_location(exit_map_index, exit_map_x, exit_map_y)
	setDungeonMapIndex(map_index)
	setBasePositions(local_x, local_y)
end

function decreaseDungeonExitCounter() -- Same as i-- implementation
	local oldValue = getDungeonExitCounter()
	setDungeonExitCounter(getDungeonExitCounter() - 1)
	return oldValue
end

function setDungeonExitCounter(counterValue)
	d.setf("exit_dungeon_counter", counterValue)
end

function getDungeonExitCounter()
	return d.getf("exit_dungeon_counter")
end

function isDungenEndFlag()
	return d.getf("dungeon_end") == 1
end

function setDungeonEndFlag()
	d.setf("dungeon_end", 1)
end

function setCooldownUsingDungeonInfo(dungeonId)
	if not party.is_party() then
		dungeon_info.set_cooldown(dungeonId, dungeon_info.get_cooldown(dungeonId))
	else
		local pids = party_get_member_pids();
		for index, pid in ipairs(pids) do
			q.begin_other_pc_block(pid);
				dungeon_info.set_cooldown(dungeonId, dungeon_info.get_cooldown(dungeonId))
			q.end_other_pc_block();
		end
	end
end

function modulo(v1, v2)
	return v1-(math.floor(v1/v2)*v2)
end

function exitDungeon()
	setDungeonEndFlag()
	setDungeonExitCounter(30)

	server_loop_timer("exit_dungeon", 1, getDungeonMapIndex())
end

function getIndex(arr, element)
	-- Returns the index of the value in array or -1

	for index = 1, table.getn(arr) do
		if arr[index] == element then
			return index
		end
	end

	return -1
end

function contains(tbl, val)
	-- Returns true if the table tbl contains the value val

	for i = 1, table.getn(tbl) do
		if tbl[i] == val then 
			return true
		end
	end
	
	return false
end


function contains_key(tbl, key)
	-- Returns true if the table tbl contains the key

	for tbl_key, _ in next, tbl, nil do
		if tbl_key == key then
			return true
		end
	end

	return false
end

function tableLength(T)
	-- Returns the length of dictionarys for example, where table.getn() returns 0

	local count = 0
	for _ in pairs(T) do count = count + 1 end
	return count
end

function choice(tbl)
	local rand = number(1, table.getn(tbl))
	return tbl[rand]
end
