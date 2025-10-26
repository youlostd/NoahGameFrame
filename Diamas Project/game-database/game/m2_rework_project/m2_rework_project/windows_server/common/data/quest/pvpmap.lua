define DUEL_MAP_INDEX 160
define TOURNAMENT_MAP_INDEX 175

quest duel_map_affect begin
  state start begin
    when login with pc.get_map_index() == DUEL_MAP_INDEX or pc.get_map_index() == TOURNAMENT_MAP_INDEX begin
      affect.add(apply.PENETRATE_PCT, 200, 60*60*24*365*60) --60Jahre
      affect.add(apply.HP_REGEN, -100, 60*60*24*365*60) --60Jahre
      affect.add(apply.POISON_PCT, 100, 60*60*24*365*60) --60Jahre
	  affect.add(apply.STEAL_HP, -100, 60*60*24*365*60) --60Jahre
    end
    when logout with pc.get_map_index() == DUEL_MAP_INDEX or pc.get_map_index() == TOURNAMENT_MAP_INDEX begin
      affect.remove()
    end
  end
end
