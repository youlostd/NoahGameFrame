quest AncientJungle_enter begin
	state start begin
		when 9393.chat."What is that place?" with not AncientJungleLIB.isActive() begin
			say_size(350,350)
			addimage(25, 10, "aj_bg1.tga"); addimage(225, 225, "aj_guard.tga");
			say_title(string.format("[ENTER][ENTER][ENTER]%s:[ENTER]", mob_name(npc.get_race())));
			
			say("Long long time ago, before our civilization even exist,[ENTER]there were another creatures. Another powers we don't[ENTER]know anymore today and it wasn't any good powers.[ENTER]I protect that place hidden from others[ENTER]and also keep the evil powers inside of that[ENTER]place.[ENTER]Unfortunately, something happened. I'm afraid I'm too old.[ENTER]They have started to break my spells. That's[ENTER]why I'm here. I need your help!")
		end
			
		when 9393.chat."Ancient jungle" with not AncientJungleLIB.isActive() begin
			local settings = AncientJungleLIB.Settings();

			addimage(25, 10, "aj_bg1.tga"); addimage(225, 170, "aj_guard.tga");
			say("[ENTER][ENTER]")
			say_title(string.format("%s:[ENTER]", mob_name(npc.get_race())))
			say_reward("Do you really want to enter the dungeon?")
			
			if (select("Yes!", "No") == 1) then
				if AncientJungleLIB.checkEnter() then
					AncientJungleLIB.CreateDungeon();
				end
			end
		end
	end
end	
		
		
