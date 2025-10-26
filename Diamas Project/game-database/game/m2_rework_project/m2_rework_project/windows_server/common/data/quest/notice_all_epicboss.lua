define EPIC_JAGRAS 7191
define EPIC_PUKEI 4721
define EPIC_BARROTH 6761
define EPIC_RATHIAN 8781
define EPIC_ODOGARON 8801
define EPIC_KADASHI 9264
define EPIC_DEMON 2831
define EPIC_KULVE 4159
define EPIC_VAAL 24389

quest noticeepic begin
	state start begin
		when kill with pc.in_dungeon() begin
			if npc.get_race() == EPIC_JAGRAS then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_JAGRAS))
			end
			if npc.get_race() == EPIC_PUKEI then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_PUKEI))
			end
			if npc.get_race() == EPIC_BARROTH then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_BARROTH))
			end
			if npc.get_race() == EPIC_RATHIAN then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_RATHIAN))
			end
			if npc.get_race() == EPIC_ODOGARON then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_ODOGARON))
			end
			if npc.get_race() == EPIC_KADASHI then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_KADASHI))
			end
			if npc.get_race() == EPIC_DEMON then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_DEMON))
			end
			if npc.get_race() == EPIC_KULVE then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_KULVE))
			end
			if npc.get_race() == EPIC_VAAL then
				notice_all("%s hat den %s gekillt", pc.get_name(), c_mob_name(EPIC_VAAL))
			end
		end
	end
end
