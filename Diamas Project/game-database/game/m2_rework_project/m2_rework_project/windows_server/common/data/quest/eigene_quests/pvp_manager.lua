-- "Yapim Ali Caßßar"

quest pvp_manager begin
	state start begin
		when 20011.chat."GM:PvP Turnuvasina Isinlan " with pc.is_gm() begin
			say("Gitmek istedigine emin misin?")
			say("")
			local a = select("Evet","Yok")
			if a == 1 then	
				pc.warp(28, 857300,3400)
			end
		end

		when login with pc.get_map_index() == 28 and (pvp_attenders == nil or pvp_attenders[pvp_manager.get_my_attender_id()] == nil or (pvp_attenders[pvp_manager.get_my_attender_id()].fight_mode != 1 and pvp_attenders[pvp_manager.get_my_attender_id()].fight_mode != 2)) begin

			if pvp_attenders == nil then
				pvp_manager.guard_check_position()
				timer("pvp_guard_1", 10)
				return
			end
			if pvp_attenders[pvp_manager.get_my_attender_id()] == nil then
				pvp_manager.guard_check_position()
				timer("pvp_guard_1", 10)
				return
			end
			if pvp_attenders[pvp_manager.get_my_attender_id()].fight_mode != 1 and pvp_attenders[pvp_manager.get_my_attender_id()].fight_mode != 2 then
				pvp_manager.guard_check_position()
				timer("pvp_guard_1", 10)
			end
		end
		when pvp_guard_1.timer begin
			if pc.get_map_index() != 26 then
				return
			end
			pvp_manager.guard_check_position()
			timer("pvp_guard_2", 10)
		end
		when pvp_guard_2.timer begin
			if pc.get_map_index() != 26 then
				return
			end
			pvp_manager.guard_check_position()
			timer("pvp_guard_1", 10)
		end
		function guard_check_position()
			if pc.get_local_x() >= 9999 and pc.get_local_y() >= 9999 and pc.get_local_x() <= 9999 and pc.get_local_y() <= 9999 then --arena 1
				pvp_manager.guard_reset()
			end
			if pc.get_local_x() >= 999 and pc.get_local_y() >= 9999 and pc.get_local_x() <= 9999 and pc.get_local_y() <= 9999 then --arena 2
				pvp_manager.guard_reset()
			end
			if pc.get_local_x() >= 999 and pc.get_local_y() >= 9999 and pc.get_local_x() <= 9999 and pc.get_local_y() <= 9999 then --arena 3
				pvp_manager.guard_reset()
			end
			if pc.get_local_x() >= 9999 and pc.get_local_y() >= 9999 and pc.get_local_x() <= 9999 and pc.get_local_y() <= 9999 then --arena 4
				pvp_manager.guard_reset()
			end
		end
		
		function guard_reset()
			notice_in_map(""..pc.get_name().." Arenadan uzaklastirildi!")
			pc.warp(28, 344000,502500)
		end
		
		when 20011.chat."PvP Turnuvasina Katil " with game.get_event_flag("pvp") == 1 begin
			if party.is_party() then
				setskin(NOWINDOW)
				syschat("Grubun varken bunu yapamazsin.")
				return
			end
			say_title("PvP Turnuvasina Katil:")
				say("Ne yapmak istiyorsun?")
				local s = select("Evet isinla beni!" , "Vazgectim" ) 
				if s == 1 then
				pc.warp(28, 857300,3400)
				else return
			end
		end
		
		when 20078.chat."Sehre Don " with pc.get_map_index() == 28 begin
			say("Sehre donmek istiyor musun?")
			local s = select("Evet ", "Hayýr ")
			if s == 1 then
				warp_to_village()
			end
		end
		when 20078.chat."Turnuva alanini temizle canim " with pc.get_map_index() == 28 and pc.is_gm() begin	
			say_title("PvP Turnuvasi canim: ")
			say("")
			say("BOLGEYI TEMIZLEMEYI ONAYLIYOR MUSUN CANIM?")
			say("")
			local teleport_all = select("Evet","YokGGGGGGGGG")
			if teleport_all == 1 then
				warp_all_to_village(28, 10)
			end
		end
		when 20078.chat."GM: PvP TURNUVA IPTAL OLSUN MU canim " with pc.get_map_index() == 28 and pc.is_gm() and game.get_event_flag("pvp") != 0 begin
			say_title("GM: PvP TURNUVA IPTAL OLSUN MU canim:")
			say("MUSUN IPTAL ETMEK PVP TURNUVA?")
			say("")
			local s = select("Evet ", "YOOOOOOG ")
			if s == 1 then
				game.set_event_flag("pvp", 0)
				pvp_attenders = {}
				notice_all("PvP Turnuvasi sona ermistir!")
				notice_all("Bir dahaki turnuvada gorusmek uzere!")
				say_title("PvP Turnuvasi iptal edildi!")
				say("PvP Turnuvasi iptal edildi!")
				say("")
			end
		end
		
		when 20078.chat."GM: Sinif sec canim " with pc.get_map_index() == 28 and pc.is_gm() and game.get_event_flag("pvp") == 0 and ae_pvp_tournament == nil begin
			say("")
			say("Turnuva icin sinif secmeyen top.")
			say("")
			local sinif = select("Savasci ", "Ninja ", "Sura ", "Saman ", "Random ", "YOK ISTEMIYOM SECMICEM VAZGECTIM ")
			if sinif == 1 then
				game.set_event_flag("pvp_sinif",1)
				say("Savasciyi sectim canim.")
			elseif sinif == 2 then
				game.set_event_flag("pvp_sinif",2)
				say("Ninja sectim canim.")
			elseif sinif == 3 then
				game.set_event_flag("pvp_sinif",3)
				say("Sura sectim canim.")
			elseif sinif == 4 then
				game.set_event_flag("pvp_sinif",4)
				say("Saman sectim canim.")
			elseif sinif == 5 then
				game.set_event_flag("pvp_sinif",5)
				say("Random sectim canim.")
			end
		end

		when 20078.chat."GM: PvP TURNUVA BASLAT canim " with pc.get_map_index() == 28 and pc.is_gm() and game.get_event_flag("pvp") == 0 and ae_pvp_tournament == nil begin
			say_title("GM: PvP TURNUVA BASLAT canim:")
			if game.get_event_flag("pvp_sinif") == 0 then
				say("Sinif secimi yapmadin canim seni zorlarim.")
				return
			end
			say("Min level siniri gir canim (Min: 15):")
			local str_min_level = input()
			say_title("Turnuvayi baslat?:")
			local min_level = tonumber(str_min_level)
			if min_level == nil then
				say("Minimum leveli girmedin canim.")
				say("")
				return
			end
			if tonumber(str_min_level) < 15 then
				min_level = 15
			end
			say("Minimum leveli su sekilde ayarladim : "..tostring(min_level)..".")
			say("Maximum leveli girmedin canim:")
			local str_max_level = input()
			say_title("Turnuvayi baslat?:")
			local max_level = tonumber(str_max_level)
			if max_level == nil then
				say("Maximum leveli girmedin canim.")
				say("")
				return
			end
			if tonumber(str_max_level) < min_level then
				say("Maks level min levelden yuksek olmali canim.")
				say("")
				return
			end
			
			game.set_event_flag("pvp_minlevel", min_level)
			game.set_event_flag("pvp_maxlevel", max_level)
			
			say("Min Level: "..tostring(min_level).." olarak ayarlandi canim.")
			say("Max Level: "..tostring(max_level).." olarak ayarlandi canim.")
			say("")
			local s = select("Baslat ", "iptal et ")
			if s == 2 then
				return
			end
			say_title("PvP Turnuvasini baslat:")
			say("")
			say("PvP Turnuvasi basladi.")
			say("")
			say_reward("Level Limiti: "..game.get_event_flag("pvp_minlevel").." - "..game.get_event_flag("pvp_maxlevel"))
			pvp_manager.open_registrations()
		end

		function open_registrations()
			game.set_event_flag("pvp", 1)
			attender_count = 0
			game.set_event_flag("pvp_tournament", get_global_time())
			pvp_attenders = {}
		
		arena = {
						-- arena 1
						{
							{ 113, 86 },
							{ 88, 112 },
							0,
							"SLEEPING",
							0,
						},
						-- arena 2
						{
							{ 136, 115 },
							{ 164, 87 },
							0,
							"SLEEPING",
							0,
						},
						-- arena 3
						{
							{ 165, 137 },
							{ 137, 165 },
							0,
							"SLEEPING",
							0,
						},
						-- arena 4
						{
							{ 85, 165 },
							{ 113, 139 },
							0,
							"SLEEPING",
							0,
						},
					}
			
			
			big_notice_all("PvP Turnuvasi baslamistir.")
			big_notice_all("Girisler icin Uriel'e gidiniz!")
			local sinif = game.get_event_flag("pvp_sinif")
			local siniff = ""
			if sinif == 1 then
				siniff = "Savasci "
				notice_in_map(siniff.."Arasi PvP turnuvasi basladi, Level limiti: "..game.get_event_flag("pvp_minlevel").." - "..game.get_event_flag("pvp_maxlevel").."")
			elseif sinif == 2 then
				siniff = "Ninja "
				notice_in_map(siniff.."Arasi PvP turnuvasi basladi, Level limiti: "..game.get_event_flag("pvp_minlevel").." - "..game.get_event_flag("pvp_maxlevel").."")
			elseif sinif == 3 then
				siniff = "Sura "
				notice_in_map(siniff.."Arasi PvP turnuvasi basladi, Level limiti: "..game.get_event_flag("pvp_minlevel").." - "..game.get_event_flag("pvp_maxlevel").."")
			elseif sinif == 4 then
				siniff = "Saman "
				notice_in_map(siniff.."Arasi PvP turnuvasi basladi, Level limiti: "..game.get_event_flag("pvp_minlevel").." - "..game.get_event_flag("pvp_maxlevel").."")
			elseif sinif == 5 then
				siniff = "Random "
				notice_in_map("Level limiti: "..game.get_event_flag("pvp_minlevel").." - "..game.get_event_flag("pvp_maxlevel").."")
			end
			
		end
		when 20078.chat."Turnuvaya Katil " with pc.get_map_index() == 28 and game.get_event_flag("pvp") == 1 begin
			say_title(mob_name(20078)..":")
			if party.is_party() then
				say("Grubun varken bunu yapamazsin.")
				return
			end
			local sinif = game.get_event_flag("pvp_sinif")
			if sinif == 1 then
				if pc.get_job() != 0 then
					if pc.get_job() != 4 then
						say("Sadece savascilar girebilir.")
						return
					end
				end
			elseif sinif == 2 then
				if pc.get_job() != 1 then
					if pc.get_job() != 5 then
						say("Sadece ninjalar girebilir.")
						return
					end
				end
			elseif sinif == 3 then
				if pc.get_job() != 2 then
					if pc.get_job() != 6 then
						say("Sadece suralar girebilir.")
						return
					end
				end
			elseif sinif == 4 then
				if pc.get_job() != 3 then
					if pc.get_job() != 7 then
						say("Sadece samanlar girebilir.")
						return
					end
				end
			end
			if game.get_event_flag("pvp_tournament") == pc.getqf("pvp_tournament") then
				say("Turnuvaya zaten kaydoldun.")
				say("")
				return
			end
			if pc.get_level() < 15 then
				say("Levelin yetersiz.")
				say("")
				return
			end
			if pc.get_level() < game.get_event_flag("pvp_minlevel") or pc.get_level() > game.get_event_flag("pvp_maxlevel") then
				say("Levelin  "..game.get_event_flag("pvp_minlevel").." ile "..game.get_event_flag("pvp_maxlevel").." arasýnda olmasi gerek.")
				say("")
				return
			end
			say("PvP Turnuvasina hosgeldin.")
			say("Kayit olmadan okuyunuz!")
			say("Kacmak yasak!")
			say("Kutsama ile Girmek yasak!")
			say("Turnuva sirasinda Video alinmasi onerilirmektedir.")
			say("Turnuva sirasinda kufur ve asagilayici kelimeler kullanmayiniz.")
			say("Kurallara uymayanlar diskalifeye olacaktir.")
			say("Herkese basarilar dileriz.")
			-- local needed_item = 50027  bunu acip 50027 kodunu degistirirsen item koduyla canim olacak
			-- say_item_vnum(needed_item)
			local s = select("Evet, katilmak istiyorum", "Hayir, katilmak istemiyorum.")
			say_title(mob_name(20078)..":")
			if s == 1 then
				-- if pc.count_item(needed_item) < 1 then#----------- 
					-- say("Giris itemin yok canim.")
					-- say_item_vnum(needed_item)
					-- return
				-- end
				say("Turnuva baslayacagi zaman seni cagiracagim.")
				say("Turnuva basladiginda burada olmazsan direkt elenirsin.")
				say("Bol sanslar!")
				say("")
				if attender_count == nil then
					attender_count = 0
				end
				if pvp_attenders == nil then
					pvp_attenders = {}
				end

				pvp_attenders[attender_count+1] = {name = pc.getname(), level = pc.get_level(), partner = nil, winner = true, fight_mode = 0, arena = 0}
				pc.setqf("attender_id", attender_count+1)
				pc.setqf("pvp_tournament", game.get_event_flag("pvp_tournament"))
				attender_count = attender_count + 1
				say("Islemlerin tamamlandi!")
				
			elseif s == 2 then
				say("Ben de oyle tahmin etmistim, biraz adam ol da oyle gel canim :)")
				say("")
			end
		end

		when 20078.chat." PvP turnuvasi icin yapilmis kayitlar " with pc.get_map_index() == 28 and pc.is_gm() and game.get_event_flag("pvp") == 1 begin
			say_title(" PvP turnuvasi ")
			say("Su ana kadar "..tostring(attender_count).." oyuncu kayit oldu.")
			if ae_pvp_tournament != nil then
				say("")
				return
			end
			say(" Kayitlari kapatmak istiyor musun? ")
			say("")
			local s = select("Evet ", "Hayir ")
			if s == 1 then
				say_title("Kayitlari kapat:")
				say("Kayitlari kapattin.")
				say("")
				pvp_manager.close_registrations()
			end
		end
		
		function close_registrations()
			if attender_count == nil then 
				say("Nill value")
				return
			end
		
			if attender_count >= 1 then
				notice_all("Kayitlar kapatildi, bundan sonra kayit yaptiramazsin.")
				game.set_event_flag("pvp", 0)
			end
			
			game.set_event_flag("pvp", 2)
			pvp_manager.make_duel_list()
			duel_list_index = 0
			playerless_mode = false
			notice_all("PvP turnuvasi basliyor!")
			pvp_manager.refresh_arenas()
		end
		
		when 20078.chat."Simdi kiminle duello yapacagim?" with pc.get_map_index() == 28 and game.get_event_flag("pvp") == 2 and game.get_event_flag("pvp_tournament") == pc.getqf("pvp_tournament") begin
			say_title(mob_name(20078)..":")
			if pvp_manager.get_duel_partner() == nil then
				say("Sana uygun aday bulunamadi!")
				say("Sana uygun bir aday bulamazsak sonraki ")
				say("rounda gececeksin.")
				return
			end
			say("Rakibin:")
			say("")
			say_reward(""..pvp_manager.get_duel_partner().."")
		end

		function make_duel_list()

			duel_list = { }
			local i = 0
			for ai = 1, attender_count, 1 do
				if pvp_attenders[ai] != nil and pvp_attenders[ai].partner == nil and pvp_attenders[ai].winner == true then
					local partner_id = pvp_manager.findpartner(tostring(pvp_attenders[ai].name), pvp_attenders[ai].level)
					if partner_id != nil then
						pvp_attenders[ai].partner = tostring(pvp_attenders[partner_id].name)
						pvp_attenders[partner_id].partner = tostring(pvp_attenders[ai].name)
						duel_list[i] = {fighter_id1 = ai, fighter_id2 = partner_id}
						i = i + 1
					end
				end
			end
			
			for ai = 1, attender_count, 1 do
				if pvp_attenders[ai] != nil and pvp_attenders[ai].partner == nil and pvp_attenders[ai].winner == true then
					local partner_id = pvp_manager.findpartner_for_partnerless(tostring(pvp_attenders[ai].name), pvp_attenders[ai].level)
					if partner_id != nil then
						pvp_attenders[ai].partner = tostring(pvp_attenders[partner_id].name)
						pvp_attenders[partner_id].partner = tostring(pvp_attenders[ai].name)
						duel_list[i] = {fighter_id1 = ai, fighter_id2 = partner_id}
						i = i + 1
					end
				end
			end
		end
		
		
		function findpartner(name, level)
			local closest_level = nil
			local partner_id = nil
			for ai = 1, attender_count, 1 do
				if pvp_attenders[ai] != nil and pvp_attenders[ai].winner == true and tonumber(pvp_attenders[ai].level) <= level + 30 and tonumber(pvp_attenders[ai].level) >= level - 30 and tostring(pvp_attenders[ai].name) != name and pvp_attenders[ai].partner == nil and pvp_manager.is_player_in_map(tostring(pvp_attenders[ai].name)) == true then
					if closest_level == nil then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					elseif tonumber(pvp_attenders[ai].level) == tonumber(level) then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					elseif tonumber(level) < tonumber(closest_level) and tonumber(pvp_attenders[ai].level) < tonumber(closest_level) then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					elseif tonumber(level) > tonumber(closest_level) and tonumber(pvp_attenders[ai].level) > tonumber(closest_level) then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					end
				end
			end
			return partner_id
		end
		
		function findpartner_for_partnerless(name, level)
			local closest_level = nil
			local partner_id = nil
			for ai = 1, attender_count, 1 do
				if pvp_attenders[ai] != nil and pvp_attenders[ai].winner == true and tostring(pvp_attenders[ai].name) != name and pvp_attenders[ai].partner == nil and pvp_manager.is_player_in_map(tostring(pvp_attenders[ai].name)) == true then
					if closest_level == nil then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					elseif pvp_attenders[ai].level == level then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					elseif tonumber(level) < tonumber(closest_level) and pvp_attenders[ai].level < closest_level then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					elseif tonumber(level) > tonumber(closest_level) and pvp_attenders[ai].level > closest_level then
						closest_level = pvp_attenders[ai].level
						partner_id = ai
					end
				end
			end
			return partner_id
		end
		
		function is_player_in_map(name)
			if pc.getname() == name then
				return true
			end
			local myname = pc.getname()
			local target = find_pc_by_name(name)
			local t = pc.select(target)
			if pc.getname() == myname then
				pc.select(t)
				return false
			end
			pc.select(t)
			return true
		end
		
		
		when 20078.chat."GM: Arenayi temizle canim" with pc.is_gm() and game.get_event_flag("pvp") == 2 begin
			for aArena = 1, 4, 1 do
				arena[aArena][3] = 0
				arena[aArena][4] = "SLEEPING"
			end
			pvp_manager.refresh_arenas()
		end
		

		
		when 20078.chat."GM: PvP Turnuva Odulu " with pc.is_gm() begin
			say_title("Odul:")
			say("Su anki odul "..game.get_event_flag("pvp_price_count").."x "..item_name(game.get_event_flag("pvp_price_vnum")).." (vnum: "..game.get_event_flag("pvp_price_vnum")..")")
			say("")
			local s = select("Odulu degistir " , "Kapat ")
			if s == 2 then
				return
			end
			say_title("Odul:")
			say("Yeni odulun esyasinin kodunu gir:")
			say("")
			local vnum = input()
			say_title("Odul:")
			say("Yeni odulun esya adetini gir:")
			say("")
			local count = input()
			game.set_event_flag("pvp_price_count", count)
			game.set_event_flag("pvp_price_vnum", vnum)
			say_title("Odul degistirildi:")
			say("Odul degistirildi canim.")
			say("Yeni odul : "..count.."x "..item_name(vnum).." (vnum: "..vnum..")")
		end
		
		function refresh_arenas()

			local my_vid = pc.get_vid()
			pc.select(my_vid)
			pc.setf("pvp","block_player_move_attack", 0)

			local partner_vid = find_pc_by_name(pvp_manager.get_duel_partner())
			pc.select(partner_vid)
			pc.setf("pvp","block_player_move_attack", 0)

			for aArena = 1, 4, 1 do
				if duel_list[duel_list_index] == nil then
					if playerless_mode == true then
						pvp_manager.init_next_round()
					else
						pvp_manager.fight_playerless()
					end
					return
				end
				if arena[aArena][3] == 0 then
					arena[aArena][3] = 1
					pvp_manager.fight_init(duel_list[duel_list_index].fighter_id1, duel_list[duel_list_index].fighter_id2, aArena)
				end
			end
		end
		
		function fight_init(pid1, pid2, arena_id)
			duel_list_index = duel_list_index + 1
			if pvp_manager.is_player_in_map(tostring(pvp_attenders[pid1].name)) == false then
				notice_in_map(""..tostring(pvp_attenders[pid1].name).." adli oyuncu oyunda olmadigindan diskalifiye edildi.", pc.get_map_index())
				pvp_attenders[pid1].winner = false
				pvp_attenders[pid2].partner = nil
				arena[arena_id][3] = 0
				pvp_manager.refresh_arenas()
				return
			end
			if pvp_manager.is_player_in_map(tostring(pvp_attenders[pid2].name)) == false then
				notice_in_map(""..tostring(pvp_attenders[pid2].name).." adli oyuncu oyunda olmadigindan diskalifiye edildi.", pc.get_map_index())
				pvp_attenders[pid2].winner = false
				pvp_attenders[pid1].partner = nil
				arena[arena_id][3] = 0
				pvp_manager.refresh_arenas()
				return
			end
			notice_in_map(tostring(pvp_attenders[pid1].name).." ve "..tostring(pvp_attenders[pid2].name).." adli kisinin turnuvasi baslayacak!", pc.get_map_index())
			
			
			
			pvp_attenders[pid1].fight_mode = 1
			pvp_attenders[pid2].fight_mode = 1
			pvp_attenders[pid1].arena = arena_id
			pvp_attenders[pid2].arena = arena_id
			
			pvp_manager.local_pc_warp(tostring(pvp_attenders[pid1].name), arena[arena_id][1][1], arena[arena_id][1][2])
			pvp_manager.local_pc_warp(tostring(pvp_attenders[pid2].name), arena[arena_id][2][1], arena[arena_id][2][2])

		end
		
		when login with pc.get_map_index() == 28 and game.get_event_flag("pvp") == 2 and pvp_manager.get_fight_mode() == 1 begin
			affect.add(apply.MOV_SPEED, -10000, 2)
			pvp_manager.set_fight_mode(2)
			if arena[pvp_manager.get_arena()][3] == 2 then
				arena[pvp_manager.get_arena()][5] = 30
				loop_timer("fight_start_countdown", 1)
			else
				timer("partner_timeout", 40)
			end
			arena[pvp_manager.get_arena()][3] = 2
		end		
		
		when partner_timeout.timer begin
			if pvp_manager.is_player_in_map(pvp_manager.get_duel_partner()) == true or game.get_event_flag("pvp") != 2 or pvp_manager.get_fight_mode() != 2 then
				return
			end
			pvp_manager.set_fight_mode(3)
			if arena[pvp_manager.get_arena()][3] == 3 then
				return
			end
			arena[pvp_manager.get_arena()][3] = 3
			notice_in_map(pc.getname().." , "..pvp_manager.get_duel_partner().." ile savasti ancak duelloyu kaybetti! !", pc.get_map_index())
			pvp_attenders[pvp_manager.get_attender_id_by_name(pvp_manager.get_duel_partner())].fight_mode = 3
			pvp_attenders[pvp_manager.get_my_attender_id()].winner = true
			pvp_attenders[pvp_manager.get_attender_id_by_name(pvp_manager.get_duel_partner())].winner = false
			pc.warp_local(pc.get_map_index(), 12600, 4000)
		end

		when fight_start_countdown.timer begin
			if arena[pvp_manager.get_arena()][5] == -1 then
				return
			end
			local partner_vid = find_pc_by_name(pvp_manager.get_duel_partner())
			local my_vid = pc.get_vid()
			if arena[pvp_manager.get_arena()][5] == 0 then
				arena[pvp_manager.get_arena()][5] = -1
				pc.select(partner_vid)
				pc.setf("pvp","block_item",1)
				pc.setf("pvp","block_player_move_attack", 0)
				command("pvp "..my_vid)
				pc.select(my_vid)
				pc.setf("pvp","block_item",1)
				pc.setf("pvp","block_player_move_attack", 0)
				command("pvp "..partner_vid)
				notice_in_map(""..pc.getname().." ile "..pvp_manager.get_duel_partner().." arasindaki duello basladi!", pc.get_map_index())
				notice_in_map("|cffffcc00 <Turnuva> Karsilasma basladi!")
				timer("fight_start_countdown_shutdown", 1)
			else
				pc.select(partner_vid)
				if math.mod(arena[pvp_manager.get_arena()][5], 5) == 0 then
					if tostring(arena[pvp_manager.get_arena()][5]) == "20" then
						local f = "pvp"
						if pc.get_job() == 2 or pc.get_job() == 6 then
							pc.setf(f,"block_cozme",1)
						end
						notice_in_map("|cffffcc00 <Turnuva> Karsilasma baslayinca esya degistirme kapanacaktir.")
						syschat("")
					elseif tostring(arena[pvp_manager.get_arena()][5]) == "20" then
						local f = "pvp"
						pc.setf(f,"block_item", 1)
						if pc.get_job() == 2 or pc.get_job() == 6 then
							pc.setf(f,"block_cozme",0)
						end
					end
					syschat("Karsilasma "..tostring(arena[pvp_manager.get_arena()][5]).." saniye icinde baslayacak.")

				end
				affect.add(apply.MOV_SPEED, -10000, 2)
				pc.select(my_vid)
				if math.mod(arena[pvp_manager.get_arena()][5], 5) == 0 then
					if tostring(arena[pvp_manager.get_arena()][5]) == "20" then
						pc.setf("pvp","block_item",0)
						-- notice_in_map("[ Dikkat ]30 Saniye Sonra item Deðiþtirme Kapanacaktýr...")
						syschat("")
					elseif tostring(arena[pvp_manager.get_arena()][5]) == "20" then
						pc.setf("pvp","block_item",1)
				
						-- notice_in_map("[ Dikkat ]30 Saniye Sonra item Deðiþtirme Kapanacaktýr...")
					end
					syschat("Karsilasma "..tostring(arena[pvp_manager.get_arena()][5]).." saniye icinde baslayacak.")

				end
				affect.add(apply.MOV_SPEED, -10000, 2)
				arena[pvp_manager.get_arena()][5] = arena[pvp_manager.get_arena()][5] - 1
			end
		end
		
		when fight_start_countdown_shutdown.timer begin
			cleartimer("fight_start_countdown")
		end
		
		when logout with game.get_event_flag("pvp") == 2 and pc.get_map_index() == 28 and pvp_manager.get_fight_mode() == 2 begin
			pvp_manager.set_fight_mode(3)
			arena[pvp_manager.get_arena()][3] = 3
			notice_in_map(""..pvp_manager.get_duel_partner().." , "..pc.getname().." isimli rakibini yendi!", pc.get_map_index())
			pvp_attenders[pvp_manager.get_attender_id_by_name(pvp_manager.get_duel_partner())].fight_mode = 3
			pvp_attenders[pvp_manager.get_my_attender_id()].winner = false
			pvp_attenders[pvp_manager.get_attender_id_by_name(pvp_manager.get_duel_partner())].winner = true
			
			pvp_manager.local_pc_warp(pvp_manager.get_duel_partner(), 126, 40)
		end
		
		when kill with game.get_event_flag("pvp") == 2 and pc.get_map_index() == 28 and npc.is_pc() and pvp_manager.get_fight_mode() == 2 begin
			pvp_manager.set_fight_mode(3)
			if arena[pvp_manager.get_arena()][3] == 3 then
				return
			end
			arena[pvp_manager.get_arena()][3] = 3
			notice_in_map(""..pc.getname().." , "..pvp_manager.get_duel_partner().." isimli rakibini yendi!", pc.get_map_index())

			local my_vid = pc.get_vid()
			pc.select(my_vid)
			pc.setf("pvp","block_item", 0)

			local partner_vid = find_pc_by_name(pvp_manager.get_duel_partner())
			pc.select(partner_vid)
			pc.setf("pvp","block_item",0)

			pvp_attenders[pvp_manager.get_attender_id_by_name(pvp_manager.get_duel_partner())].fight_mode = 3
			pvp_attenders[pvp_manager.get_my_attender_id()].winner = true
			pvp_attenders[pvp_manager.get_attender_id_by_name(pvp_manager.get_duel_partner())].winner = false
			
			timer("fight_end", 5)
		end
		
		when fight_end.timer begin
			pc.warp_local(pc.get_map_index(), 12600, 4000)
			pvp_manager.local_pc_warp(pvp_manager.get_duel_partner(), 126, 40)
		end
		
		when letter with pc.get_map_index() == 28 and game.get_event_flag("pvp") == 2 and pvp_manager.get_fight_mode() == 3 begin
			pvp_manager.set_fight_mode(0)
			
			if pvp_attenders[pvp_manager.get_my_attender_id()].winner == true then
				arena[pvp_manager.get_arena()][3] = 0
				pvp_manager.refresh_arenas()
			end
		end
		
		function fight_playerless()
			for aArena = 1, 4, 1 do
				if arena[aArena][3] != 0 then
					return
				end
			end
			playerless_mode = true

			
			pvp_manager.make_duel_list()
			pvp_manager.refresh_arenas()
		end
		
		function init_next_round()
			for aArena = 1, 4, 1 do
				if arena[aArena][3] != 0 then
					return
				end
			end
			playerless_mode = false
			
			local winner_count = 0
			--[[local]] winner_id = 0
			for ai = 1, attender_count, 1 do
				pvp_attenders[ai].partner = nil
				pvp_attenders[ai].fight_mode = 0
				if pvp_attenders[ai] != nil and pvp_attenders[ai].winner == true then
					winner_count = winner_count + 1
					winner_id = ai
				end
			end
			
			if winner_count == 1 then
				notice_in_map("|cffffcc00"..pvp_attenders[winner_id].name.." turnuvayi kazandi, Tebrik ederiz!")
				-- notice_in_map("|cffffcc00"..pvp_attenders[winner_id].name.." ,"..game.get_event_flag("pvp_price_count").." adet "..item_name(game.get_event_flag("pvp_price_vnum")).." kazandi.")
				local target = find_pc_by_name(pvp_attenders[winner_id].name)
				local t = pc.select(target)
				
				pc.give_item2(game.get_event_flag("pvp_price_vnum"), game.get_event_flag("pvp_price_count"))
				pc.select(t)
				notice_all("|cffffcc00 PvP turnuvasi bitti.")
				game.set_event_flag("pvp", 0)
				-- pvp_attenders = nil
				return
			end
			
			
			pvp_manager.make_duel_list()
			duel_list_index = 0
			if winner_count == 2 then
				notice_in_map("|cffffcc00 Final roundu basliyor.")
			elseif winner_count <= 4 then
				notice_in_map("|cffffcc00 Yarý final roundu basliyor.")
			else
				notice_in_map("|cffffcc00 Round basliyor.")
			end
			pvp_manager.refresh_arenas()
		end
		
		when login with pc.get_map_index() != 354 begin
			local f = "pvp"
			pc.setf(f,"item_block",0)
			if pc.get_job() == 2 or pc.get_job() == 6 then
				pc.setf(f,"block_cozme",0)
			end
		end
		
		when login with pc.get_map_index() == 354 begin
			if pc.get_job() == 2 or pc.get_job() == 6 then
				local f = "pvp"
				pc.setf(f,"block_cozme",0)
			end
			if party.is_party() then
				local krallik = pc.get_empire()
				if krallik == 1 then
					pc.warp(469300, 964200)
				elseif krallik == 2 then
					pc.warp(55700, 157900)
				elseif krallik == 3 then
					pc.warp(969600, 278400)
				end
			end
		end
		
		function local_pc_warp(name, x, y)
			local target = find_pc_by_name(name)
			local t = pc.select(target)			if x == 126 and y == 40 then
				local f = "pvp"
				pc.setf(f,"item_block",0)
			end
			pc.warp_local(pc.get_map_index(), x*100, y*100)
			pc.select(t)
			if x == 126 and y == 40 then
				local f = "pvp"
				pc.setf(f,"item_block",0)
			end
		end
		
		function setqf_by_name(name, flag, value)
			local target = find_pc_by_name(name)
			local t = pc.select(target)
			pc.setqf(flag, value)
			pc.select(t)
		end
		
		function getqf_by_name(name, flag)
			local target = find_pc_by_name(name)
			local t = pc.select(target)
			local rflag = pc.getqf(flag)
			pc.select(t)
			return rflag
		end
		
		function get_attender_id_by_name(name)
			for ai = 1, attender_count, 1 do
				if tostring(pvp_attenders[ai].name) == name then
					return ai
				end
			end
			return 0
		end
		
		function get_my_attender_id()
			return pvp_manager.get_attender_id_by_name(pc.get_name())
		end
		
		function get_duel_partner()
			if pvp_attenders[pvp_manager.get_my_attender_id()] != nil and pvp_attenders[pvp_manager.get_my_attender_id()].partner != nil then
				return pvp_attenders[pvp_manager.get_my_attender_id()].partner
			else
				return nil
			end
		end
		
		function get_fight_mode()
			if pvp_attenders != nil and pvp_manager.get_my_attender_id() != 0 and pvp_attenders[pvp_manager.get_my_attender_id()] != nil and pvp_attenders[pvp_manager.get_my_attender_id()].fight_mode != nil then
				return pvp_attenders[pvp_manager.get_my_attender_id()].fight_mode
			else
				return 0
			end
		end
		
		function set_fight_mode(fmode)
			pvp_attenders[pvp_manager.get_my_attender_id()].fight_mode = fmode
		end
		
		function get_arena()
			return pvp_attenders[pvp_manager.get_my_attender_id()].arena
		end
		
		function set_arena(id)
			pvp_attenders[pvp_manager.get_my_attender_id()].arena = id
		end
		
		
		
	end
end
