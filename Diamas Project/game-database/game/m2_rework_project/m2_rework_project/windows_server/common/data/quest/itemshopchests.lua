quest itemshopchests begin
	state start begin
		when 253735.use begin
		
			if pc.in_dungeon() then
				say("Du kannst das Item hier nicht nutzen!")
				return
			end
		
			if pc.get_empty_inventory_count() < 45 then
				chat(gameforge.anfangstruhe._10_chat)
				return
			else
				pc.give_item2(8989)
				pc.give_item2(72809)
				pc.give_item2(18089)
				pc.give_item2(8919)
				pc.give_item2(85002)
				pc.give_item2(56022)
				pc.give_item2(55973)
				pc.remove_item(253735, 1)
			end
		end
		
		when 253736.use begin
		
			if pc.in_dungeon() then
				say("Du kannst das Item hier nicht nutzen!")
				return
			end
		
			if pc.get_empty_inventory_count() < 45 then
				chat(gameforge.anfangstruhe._10_chat)
				return
			else
				pc.give_item2(8996)
				pc.give_item2(72816)
				pc.give_item2(8896)
				pc.give_item2(56020)
				pc.give_item2(55969)
				pc.remove_item(253736, 1)
			end
		end
		
		when 253737.use begin
		
			if pc.in_dungeon() then
				say("Du kannst das Item hier nicht nutzen!")
				return
			end
		
			if pc.get_empty_inventory_count() < 45 then
				chat(gameforge.anfangstruhe._10_chat)
				return
			else
				pc.give_item2(9516)
				pc.give_item2(72826)
				pc.give_item2(8946)
				pc.give_item2(55981)
				pc.give_item2(55937)
				pc.remove_item(253737, 1)
			end
		end
		
		when 253738.use begin
		
			if pc.in_dungeon() then
				say("Du kannst das Item hier nicht nutzen!")
				return
			end
		
			if pc.get_empty_inventory_count() < 45 then
				chat(gameforge.anfangstruhe._10_chat)
				return
			else
				pc.give_item2(9526)
				pc.give_item2(72836)
				pc.give_item2(8926)
				pc.give_item2(55934)
				pc.give_item2(55976)
				pc.remove_item(253738, 1)
			end
		end
		
		when 253739.use begin
		
			if pc.in_dungeon() then
				say("Du kannst das Item hier nicht nutzen!")
				return
			end
		
			if pc.get_empty_inventory_count() < 45 then
				chat(gameforge.anfangstruhe._10_chat)
				return
			else
				pc.give_item2(9536)
				pc.give_item2(72846)
				pc.give_item2(8906)
				pc.give_item2(55984)
				pc.give_item2(55978)
				pc.remove_item(253739, 1)
			end
		end
		
		when 253740.use begin
		
			if pc.in_dungeon() then
				say("Du kannst das Item hier nicht nutzen!")
				return
			end
		
			if pc.get_empty_inventory_count() < 45 then
				chat(gameforge.anfangstruhe._10_chat)
				return
			else
				pc.give_item2(9546)
				pc.give_item2(72856)
				pc.give_item2(8936)
				pc.give_item2(55994)
				pc.give_item2(56050)
				pc.remove_item(253740, 1)
			end
		end
	end
end
