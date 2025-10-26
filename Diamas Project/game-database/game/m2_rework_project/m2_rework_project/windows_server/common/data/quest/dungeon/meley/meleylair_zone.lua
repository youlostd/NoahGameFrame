quest meleylair_zone begin
	state start begin
		when 20419.chat.gameforge.meley_dungeon._500_chat with MeleyLair.IsMeleyMap() begin
			say_title(c_mob_name(20419))
			say("")
			say(gameforge.meley_dungeon._510_chat)
			say("")
			local agree = select(gameforge.meley_dungeon._000_yes, gameforge.meley_dungeon._005_no)
			if agree == 1 then
				MeleyLair.Leave()
			else
				setskin(NOWINDOW)
				return
			end
		end
		
		when login begin
			if MeleyLair.IsMeleyMap() then
				MeleyLair.Check()
			end
			if pc.get_map_index() == 215 then
				pc.go_home()
			end
		end

		
		when 20420.click with MeleyLair.IsMeleyMap() begin
			say_title(c_mob_name(20420))
			say("")
			local menu = select(gameforge.meley_dungeon._530_chat, gameforge.meley_dungeon._430_select)
			if menu == 1 then
				say_title(c_mob_name(20420))
				say("")
				if not MeleyLair.CanGetReward() then
					say(gameforge.meley_dungeon._540_chat)
				else
					say(gameforge.meley_dungeon._550_chat)
					say(gameforge.meley_dungeon._560_chat)
					local reward_menu = select(gameforge.meley_dungeon._570_chat, gameforge.meley_dungeon._580_chat, gameforge.meley_dungeon._430_select)
					if reward_menu == 1 then
						say_title(c_mob_name(20420))
						say("")
						say(gameforge.meley_dungeon._590_chat)
						local agree = select(gameforge.meley_dungeon._000_yes, gameforge.meley_dungeon._005_no)
						if agree == 1 then
							MeleyLair.Reward(reward_menu)
							say_title(c_mob_name(20420))
							say("")
							say(gameforge.meley_dungeon._600_chat)
						else
							setskin(NOWINDOW)
							return
						end
					elseif reward_menu == 2 then
						say_title(c_mob_name(20420))
						say("")
						say(gameforge.meley_dungeon._610_chat)
						local agree = select(gameforge.meley_dungeon._000_yes, gameforge.meley_dungeon._005_no)
						if agree == 1 then
							MeleyLair.Reward(reward_menu)
							say_title(c_mob_name(20420))
							say("")
							say(gameforge.meley_dungeon._600_chat)
						else
							setskin(NOWINDOW)
							return
						end
					else
						setskin(NOWINDOW)
						return
					end
				end
			else
				setskin(NOWINDOW)
				return
			end
		end
		
		when 20388.click."" begin
		end
    end
end
