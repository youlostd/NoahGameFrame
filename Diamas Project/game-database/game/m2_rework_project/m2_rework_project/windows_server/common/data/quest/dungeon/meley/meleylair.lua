quest meleylair begin
	state start begin
	
		when 20419.chat.gameforge.meley_dungeon._010_chat with pc.get_map_index() == MeleyLair.GetSubMapIndex() begin
			say_title(c_mob_name(20419))
			say("")
			say(gameforge.meley_dungeon._020_say)
			say("")
			wait()
			say_title(c_mob_name(20419))
			say("")
			say(gameforge.meley_dungeon._030_say)
		end
		
		when 20419.chat.gameforge.meley_dungeon._110_chat with pc.get_map_index() == MeleyLair.GetSubMapIndex() and pc.is_guild_master() begin
			say_title(c_mob_name(20419))
			say("")
			local registered, channel = MeleyLair.IsRegistered()
			if registered then
				say(string.format(gameforge.meley_dungeon._120_say, channel))
				return
			end
			
			local requirment1, requirment2 = MeleyLair.GetRequirments()
			say(string.format(gameforge.meley_dungeon._130_say, requirment1, requirment2))
			local menu = select(gameforge.meley_dungeon._000_yes, gameforge.meley_dungeon._005_no)
			if menu == 1 then
				local result1, result2 = MeleyLair.Register()
				if result1 == 0 then
					setskin(NOWINDOW)
					return
				end
				
				local resultMsg = {
								[1] = gameforge.meley_dungeon._140_msg,
								[2] = gameforge.meley_dungeon._150_msg,
								[3] = gameforge.meley_dungeon._160_msg,
								[4] = gameforge.meley_dungeon._170_msg,
								[5] = gameforge.meley_dungeon._180_msg,
								[6] = gameforge.meley_dungeon._190_msg,
								[7] = gameforge.meley_dungeon._200_msg,
				}
				
				say_title(c_mob_name(20419))
				say("")
				if result1 != 2 and result1 != 5 then
					if result1 == 7 then
						local hours = string.format(gameforge.meley_dungeon._10_stringFormat, math.floor(result2 / 3600));
						local minutes = string.format(gameforge.meley_dungeon._10_stringFormat, math.floor(result2 / 60 - (hours * 60)));
						local seconds = string.format(gameforge.meley_dungeon._10_stringFormat, math.floor(result2 - hours * 3600 - minutes * 60));
						local timeConv = string.format(hours..":"..minutes..":"..seconds)
						say(string.format(resultMsg[result1], timeConv))
					else
						say(string.format(resultMsg[result1], result2))
					end
				else
					say(resultMsg[result1])
				end
			else
				setskin(NOWINDOW)
				return
			end
		end
		
		when 20419.chat.gameforge.meley_dungeon._310_chat with pc.get_map_index() == MeleyLair.GetSubMapIndex() and pc.has_guild() and npc.lock() begin
			say_title(c_mob_name(20419))
			say("")
			say(gameforge.meley_dungeon._320_chat)
			say("")
			
			if not party.is_party() then
				say_reward("You cannot enter this dungeon [ENTER] without a party")
				return
			end
			
			if party.get_leader_pid() != guild.get_master_pid() then
				say_reward("The group leader must be the guild master.")
				return
			end
			
			
			
			local agree = select(gameforge.meley_dungeon._000_yes, gameforge.meley_dungeon._005_no)
			if agree == 1 then
				local registered, limit = MeleyLair.Enter()
				if not registered and limit == 0 then
					say_title(c_mob_name(20419))
					say("")
					if pc.is_guild_master() then
						say(gameforge.meley_dungeon._330_chat)
					else
						say(gameforge.meley_dungeon._340_chat)
					end
					npc.unlock()
					
					return
				elseif not registered and limit > 0 and limit < 5 then
					say_title(c_mob_name(20419))
					say("")
					say(string.format(gameforge.meley_dungeon._350_chat, limit))
					npc.unlock()
					return
				elseif limit == 1 then
					say_title(c_mob_name(20419))
					say("")
					say(string.format(gameforge.meley_dungeon._360_chat, MeleyLair.GetPartecipantsLimit()))
					npc.unlock()
					return
				elseif limit == 2 then
					say_title(c_mob_name(20419))
					say("")
					say(gameforge.meley_dungeon._370_chat)
					npc.unlock()
					return
				elseif limit == 3 then
					say_title(c_mob_name(20419))
					say("")
					say(gameforge.meley_dungeon._380_chat)
					npc.unlock()
					return
				elseif limit == 4 then
					say_title(c_mob_name(20419))
					say("")
					say(gameforge.meley_dungeon._390_chat)
					npc.unlock()
					return
				elseif limit == 5 then
					say_title(c_mob_name(20419))
					say("")
					say(gameforge.meley_dungeon._400_chat)
					npc.unlock()
					return
				end
					npc.unlock()
				return
			else
				setskin(NOWINDOW)
				npc.unlock()

				return
			end
		end
    end
end
