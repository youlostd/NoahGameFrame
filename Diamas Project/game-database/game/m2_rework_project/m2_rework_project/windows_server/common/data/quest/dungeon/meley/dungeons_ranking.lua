quest dungeons_ranking begin
	state start begin
		when 20421.chat."Ranking" begin
			say_title(c_mob_name(20420))
			say("")
			local menu = select("Meley Lair", "Close")
			if menu == 1 then
				MeleyLair.OpenRanking()
				setskin(NOWINDOW)
				return
			else
				setskin(NOWINDOW)
				return
			end
		end
    end
end
