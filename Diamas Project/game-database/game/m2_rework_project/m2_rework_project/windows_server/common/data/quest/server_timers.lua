quest server_timers begin
	state start begin
		when exit_dungeon.server_timer begin
			if d.select(get_server_timer_arg()) then
				local counter = decreaseDungeonExitCounter()

				if modulo(counter, 5) == 0 and counter > 0 then
					d.notice("Du wirst in %s Sekunden herausteleportiert.", counter)
				end

				if counter == 0 then
					d.exit_dungeon()
					clear_server_timer("exit_dungeon", get_server_timer_arg())
				end
			end
		end

		when exit_dungeon_final.server_timer begin
			if d.select(get_server_timer_arg()) then
				d.exit_dungeon()
			end
		end

		when timeout.server_timer begin
			if d.select(get_server_timer_arg()) then
				setDungeonEndFlag()
				server_loop_timer("exit_dungeon", 1, get_server_timer_arg())
			end
		end

		when dungeon_timeout.server_timer begin
			if d.select(get_server_timer_arg()) then
				setDungeonEndFlag()
				server_loop_timer("exit_dungeon", 1, get_server_timer_arg())
			end
		end
	end
end
