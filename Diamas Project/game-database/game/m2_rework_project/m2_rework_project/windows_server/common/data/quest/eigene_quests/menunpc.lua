quest npc_menu_secenek begin
    state start begin
        when 9010.chat."Kategori" begin
        setskin ( NOWINDOW )
        npc.open_shop(30)
        end
    end
end