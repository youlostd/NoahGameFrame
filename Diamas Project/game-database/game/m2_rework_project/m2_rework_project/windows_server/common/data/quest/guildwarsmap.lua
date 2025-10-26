--define GUILDWARS_MAP_INDEX 171

quest guildwars_map_affect begin
  state start begin
    when login with isInDungeonRange(171) begin
      affect.add(apply.SKILL_DEFEND_BONUS, 15, 60*60*24*365*60) --60Jahre
    end
    when logout with isInDungeonRange(171) begin
      affect.remove()
    end
  end
end
