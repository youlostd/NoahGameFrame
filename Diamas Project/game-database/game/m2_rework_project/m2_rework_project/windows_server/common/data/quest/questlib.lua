CONFIRM_NO = 0
CONFIRM_YES = 1
CONFIRM_OK = 1
CONFIRM_TIMEOUT = 2

MALE = 0
FEMALE = 1

--quest.create = function(f) return coroutine.create(f) end
--quest.process = function(co,args) return coroutine.resume(co, args) end
setstate = q.setstate
newstate = q.setstate

q.set_clock = function(name, value) q.set_clock_name(name) q.set_clock_value(value) end
q.set_counter = function(name, value) q.set_counter_name(name) q.set_counter_value(value) end

-- d.set_folder = function (path) raw_script("[SET_PATH path;"..pat50062d
-- d.set_folder = function (path) path.show_cinematic("[SET_PATH path;"..path.."]") end
-- party.run_cinematic = function (path) party.show_cinematic("[RUN_CINEMATIC value;"..path.."]") end

newline = "[ENTER]"
function color256(r, g, b) return "[COLOR r;"..(r/255.0).."|g;"..(g/255.0).."|b;"..(b/255.0).."]" end
function color(r,g,b) return "[COLOR r;"..r.."|g;"..g.."|b;"..b.."]" end
function delay(v) return "[DELAY value;"..v.."]" end
function setcolor(r,g,b) raw_script(color(r,g,b)) end
function setdelay(v) raw_script(delay(v)) end
function resetcolor(r,g,b) raw_script("[/COLOR]") end
function resetdelay(v) raw_script("[/DELAY]") end

-- trim a string
function trim(s) return (string.gsub(s, "^%s*(.-)%s*$", "%1")) end

-- minimap에 동그라미 표시
function addmapsignal(x,y) raw_script("[ADDMAPSIGNAL x;"..x.."|y;"..y.."]") end

-- minimap 동그라미들 모두 클리어
function clearmapsignal() raw_script("[CLEARMAPSIGNAL]") end

-- 클라이언트에서 보여줄 대화창 배경 그림을 정한다.
function setbgimage(src) raw_script("[BGIMAGE src;") raw_script(src) raw_script("]") end

-- 대화창에 이미지를 보여준다.
function addimage(x,y,src) raw_script("[IMAGE x;"..x.."|y;"..y) raw_script("|src;") raw_script(src) raw_script("]") end
function addvideo(x,y,src) raw_script("[VIDEO x;"..x.."|y;"..y) raw_script("|src;") raw_script(src) raw_script("]") end

-- generate when a linebreak in the functions: d.notice,notice,notice_all
function notice_multiline( str , func )
    local p = 0
    local i = 0
    while true do
        i = string.find( str, "%[ENTER%]", i+1 )
        if i == nil then
            if string.len(str) > p then
                func( string.sub( str, p, string.len(str) ) )
            end
            break
        end
        func( string.sub( str, p, i-1 ) )
        p = i + 7
    end
end 

function makequestbutton(name)
    raw_script("[QUESTBUTTON idx;")
    raw_script(""..q.getcurrentquestindex()) 
    raw_script("|name;")
    raw_script(name) raw_script("]")
end

function make_quest_button_ex(name, icon_type, icon_name)
    test_chat(icon_type)
    test_chat(icon_name)
    raw_script("[QUESTBUTTON idx;")
    raw_script(""..q.getcurrentquestindex()) 
    raw_script("|name;")
    raw_script(name)
    raw_script("|icon_type;")
    raw_script(icon_type)
    raw_script("|icon_name;")
    raw_script(icon_name)
    raw_script("]")
end

function make_quest_button(name) makequestbutton(name) end

function send_letter_ex(name, icon_type, icon_name) make_quest_button_ex(name, icon_type, icon_name) set_skin(NOWINDOW) q.set_title(name) q.start() end

function send_letter(name) makequestbutton(name) set_skin(NOWINDOW) q.set_title(name) q.start() end
function clear_letter() q.done() end
function say_title(name) say(color256(255, 230, 186)..name..color256(196, 196, 196)) end
function say_reward(name) say(color256(255, 200, 200)..name..color256(196, 196, 196)) end
function say_pc_name() say(pc.get_name()..":") end
function say_size(width, height) say("[WINDOW_SIZE width;"..width.."|height;"..height.."]") end
function setmapcenterposition(x,y)
    raw_script("[SETCMAPPOS x;")
    raw_script(x.."|y;")
    raw_script(y.."]")
end
function say_item(name, vnum, desc)
    say("[INSERT_IMAGE image_type;item|idx;"..vnum.."|title;"..name.."|desc;"..desc.."|index;".. 0 .."|total;".. 1 .."]")
end
function say_show_item(vnum)
    say("[INSERT_IMAGE image_type;item|idx;"..vnum.."|index;".. 0 .."|total;".. 1 .."]")
end
function say_item_vnum(vnum)
    say_item("", vnum, "")
end
function say_item_vnum_inline(vnum,index,total)
    if index >= total then
        return
    end
    if total > 3 then
        return
    end
    raw_script("[INSERT_IMAGE image_type;item|idx;"..vnum.."|title;"..item_name(vnum).."|desc;".."".."|index;"..index.."|total;"..total.."]")
end
function pc_is_novice()
    if pc.get_skill_group()==0 then
        return true
    else
        return false
    end
end
function pc_get_exp_bonus(exp, text)
    say_reward(text)
    pc.give_exp2(exp)
end
function pc_get_village_map_index(index)
    return village_map[pc.get_empire()][index]
end
function pc_has_even_id()
    return math.mod(pc.get_player_id(),2) == 0
end

function pc_get_account_id()
    return math.mod(pc.get_account_id(), 2) !=0
end

function pc_in_given_dungeon(map_index)
	return pc.in_dungeon() and (map_index * 10000) <= pc.get_map_index() and (map_index * 11000) > pc.get_map_index()
end

village_map = {
    {1, 3},
    {21, 23},
    {41, 43},
}

function npc_is_same_empire()
    if pc.get_empire()==npc.empire then
        return true
    else
        return false
    end
end

function npc_get_skill_teacher_race(pc_empire, pc_job, sub_job)
    if 1==sub_job then
        if 0==pc_job then
            return WARRIOR1_NPC_LIST[pc_empire]
        elseif 1==pc_job then
            return ASSASSIN1_NPC_LIST[pc_empire]
        elseif 2==pc_job then
            return SURA1_NPC_LIST[pc_empire]
        elseif 3==pc_job then
            return SHAMAN1_NPC_LIST[pc_empire]
        end	
    elseif 2==sub_job then
        if 0==pc_job then
            return WARRIOR2_NPC_LIST[pc_empire]
        elseif 1==pc_job then
            return ASSASSIN2_NPC_LIST[pc_empire]
        elseif 2==pc_job then
            return SURA2_NPC_LIST[pc_empire]
        elseif 3==pc_job then
            return SHAMAN2_NPC_LIST[pc_empire]
        end	
    end

    return 0
end 


function pc_find_square_guard_vid()
    if pc.get_empire()==1 then 
        return find_npc_by_vnum(11000) 
    elseif pc.get_empire()==2 then
        return find_npc_by_vnum(11002)
    elseif pc.get_empire()==3 then
        return find_npc_by_vnum(11004)
    end
    return 0
end

function pc_find_skill_teacher_vid(sub_job)
    local vnum=npc_get_skill_teacher_race(pc.get_empire(), pc.get_job(), sub_job)
    return find_npc_by_vnum(vnum)
end

function pc_find_square_guard_vid()
    local pc_empire=pc.get_empire()
    if pc_empire==1 then
        return find_npc_by_vnum(11000)
    elseif pc_empire==2 then
        return find_npc_by_vnum(11002)
    elseif pc_empire==3 then
        return find_npc_by_vnum(11004)
    end
end

function npc_is_same_job()
    local pc_job=pc.get_job()
    local npc_vnum=npc.get_race()

    -- test_chat("pc.job:"..pc.get_job())
    -- test_chat("npc_race:"..npc.get_race())
    -- test_chat("pc.skill_group:"..pc.get_skill_group())
    if pc_job==0 then
        if table_is_in(WARRIOR1_NPC_LIST, npc_vnum) then return true end
        if table_is_in(WARRIOR2_NPC_LIST, npc_vnum) then return true end
    elseif pc_job==1 then
        if table_is_in(ASSASSIN1_NPC_LIST, npc_vnum) then return true end
        if table_is_in(ASSASSIN2_NPC_LIST, npc_vnum) then return true end
    elseif pc_job==2 then
        if table_is_in(SURA1_NPC_LIST, npc_vnum) then return true end
        if table_is_in(SURA2_NPC_LIST, npc_vnum) then return true end
    elseif pc_job==3 then
        if table_is_in(SHAMAN1_NPC_LIST, npc_vnum) then return true end
        if table_is_in(SHAMAN2_NPC_LIST, npc_vnum) then return true end
    end

    return false
end

function npc_get_job()
    local npc_vnum=npc.get_race()

    if table_is_in(WARRIOR1_NPC_LIST, npc_vnum) then return COND_WARRIOR_1 end
    if table_is_in(WARRIOR2_NPC_LIST, npc_vnum) then return COND_WARRIOR_2 end
    if table_is_in(ASSASSIN1_NPC_LIST, npc_vnum) then return COND_ASSASSIN_1 end
    if table_is_in(ASSASSIN2_NPC_LIST, npc_vnum) then return COND_ASSASSIN_2 end
    if table_is_in(SURA1_NPC_LIST, npc_vnum) then return COND_SURA_1 end
    if table_is_in(SURA2_NPC_LIST, npc_vnum) then return COND_SURA_2 end
    if table_is_in(SHAMAN1_NPC_LIST, npc_vnum) then return COND_SHAMAN_1 end
    if table_is_in(SHAMAN2_NPC_LIST, npc_vnum) then return COND_SHAMAN_2 end
    return 0

end

function time_min_to_sec(value)
    return 60*value
end

function time_hour_to_sec(value)
    return 3600*value
end

function next_time_set(value, test_value)
	if test_value == nil then
		test_value = value
	end
    local nextTime=get_time()+value
    if is_test_server() then
        nextTime=get_time()+test_value
    end
    pc.setqf("__NEXT_TIME__", nextTime)
end

function next_time_is_now(value)
    if get_time()>=pc.getqf("__NEXT_TIME__") then
        return true
    else
        return false
    end
end

function table_get_random_item(self)
    return self[number(1, table.getn(self))]
end

function table_is_in(self, test)
    for i = 1, table.getn(self) do
        if self[i]==test then
            return true
        end
    end
    return false
end


function giveup_quest_menu(title)
    local s=select("Proceed", "Give up")
    if 2==s then 
    say(title.." I really like the quest")
    say("Do you want to give up?")
    local s=select("Yes, it is", "No")
    if 1==s then
        say(title.."I gave up the quest")
        restart_quest()
    end
    end
end

function restart_quest()
    set_state("start")
    q.done()
end

function complete_quest()
    set_state("__COMPLETE__")
    q.done()
end

function giveup_quest()
    set_state("__GIVEUP__")
    q.done()
end

function complete_quest_state(state_name)
    set_state(state_name)
    q.done()
end

function test_chat(log)
    if is_test_server() then
        chat(log)
    end
end

function bool_to_str(is)
    if is then
        return "true"
    else
        return "false"
    end
end

WARRIOR1_NPC_LIST 	= {20300, 20320, 20340, }
WARRIOR2_NPC_LIST 	= {20301, 20321, 20341, }
ASSASSIN1_NPC_LIST 	= {20302, 20322, 20342, }
ASSASSIN2_NPC_LIST 	= {20303, 20323, 20343, }
SURA1_NPC_LIST 		= {20304, 20324, 20344, }
SURA2_NPC_LIST 		= {20305, 20325, 20345, }
SHAMAN1_NPC_LIST 	= {20306, 20326, 20346, }
SHAMAN2_NPC_LIST 	= {20307, 20327, 20347, }

function skill_group_dialog(e, j, g) -- e = 제국, j = 직업, g = 그룹
    e = 1 -- XXX 메시지가 나라별로 있다가 하나로 통합되었음
    

    -- 다른 직업이거나 다른 제국일 경우
    if pc.job != j then
        say(gameforge.skill_group.dialog[e][pc.job][3])
    elseif pc.get_skill_group() == 0 then
        if pc.level < 5 then
            say(gameforge.skill_group.dialog[e][j][g][1])
            return
        end
        say(gameforge.skill_group.dialog[e][j][g][2])
        local answer = select(gameforge.yes, gameforge.no)

        if answer == 1 then
            --say(gameforge.skill_group.dialog[e][j][g][2])
            pc.set_skill_group(g)
        else
            --say(gameforge.skill_group.dialog[e][j][g][3])
        end
    --elseif pc.get_skill_group() == g then
        --say(gameforge.skill_group.dialog[e][j][g][4])
    --else
        --say(gameforge.skill_group.dialog[e][j][g][5])
    end
end

function show_horse_menu()
    if horse.is_mine() then			
        say(gameforge.horse_menu.menu)

        local s = 0
        if horse.is_dead() then
            s = select(gameforge.horse_menu.revive, gameforge.horse_menu.ride, gameforge.horse_menu.unsummon, gameforge.horse_menu.close)
        else
            s = select(gameforge.horse_menu.feed, gameforge.horse_menu.ride, gameforge.horse_menu.unsummon, gameforge.horse_menu.close)
        end

        if s==1 then
            if horse.is_dead() then
                horse.revive()
            else
                local food = horse.get_grade() + 50054 - 1
                if pc.countitem(food) > 0 then
                pc.removeitem(food, 1)
                horse.feed()
                else
                say(gameforge.need_item_prefix..item_name(food)..locale.need_item_postfix);
                end
            end
        elseif s==2 then
            horse.ride()
        elseif s==3 then
            horse.unsummon()
        elseif s==4 then
            -- do nothing
        end
    end
end

npc_index_table = {
    ['race'] = npc.getrace,
    ['empire'] = npc.get_empire,
}

pc_index_table = {
    ['weapon']		= pc.getweapon,
    ['level']		= pc.get_level,
    ['hp']		= pc.gethp,
    ['maxhp']		= pc.getmaxhp,
    ['sp']		= pc.getsp,
    ['maxsp']		= pc.getmaxsp,
    ['exp']		= pc.get_exp,
    ['nextexp']		= pc.get_next_exp,
    ['job']		= pc.get_job,
    ['money']		= pc.getmoney,
    ['gold'] 		= pc.getmoney,
    ['name'] 		= pc.getname,
    ['playtime'] 	= pc.getplaytime,
    ['leadership'] 	= pc.getleadership,
    ['empire'] 		= pc.getempire,
    ['skillgroup'] 	= pc.get_skill_group,
    ['x'] 		= pc.getx,
    ['y'] 		= pc.gety,
    ['local_x'] 	= pc.get_local_x,
    ['local_y'] 	= pc.get_local_y,
}

item_index_table = {
    ['vnum']		= item.get_vnum,
    ['name']		= item.get_name,
    ['size']		= item.get_size,
    ['count']		= item.get_count,
    ['type']		= item.get_type,
    ['sub_type']	= item.get_sub_type,
    ['refine_vnum']	= item.get_refine_vnum,
    ['level']		= item.get_level,
}

guild_war_bet_price_table = 
{
    10000,
    30000,
    50000,
    100000
}

function npc_index(t,i) 
    local npit = npc_index_table
    if npit[i] then
    return npit[i]()
    else
    return rawget(t,i)
    end
end

function pc_index(t,i) 
    local pit = pc_index_table
    if pit[i] then
    return pit[i]()
    else
    return rawget(t,i)
    end
end

function item_index(t, i)
    local iit = item_index_table
    if iit[i] then
    return iit[i]()
    else
    return rawget(t, i)
    end
end

setmetatable(pc,{__index=pc_index})
setmetatable(npc,{__index=npc_index})
setmetatable(item,{__index=item_index})

--coroutine을 이용한 선택항 처리
function select(...)
    return q.yield('select', arg)
end

function select_table(table)
    return q.yield('select', table)
end

-- coroutine을 이용한 다음 엔터 기다리기
function wait()
    q.yield('wait')
end

function input()
    return q.yield('input')
end

function confirm(vid, msg, timeout)
    return q.yield('confirm', vid, msg, timeout)
end

function select_item()
    setskin(NOWINDOW)
    return q.yield('select_item')
end

--전역 변수 접근과 관련된 계열
NOWINDOW = 0
NORMAL = 1
CINEMATIC = 2
SCROLL = 3

WARRIOR = 0
ASSASSIN = 1
SURA = 2
SHAMAN = 3

COND_WARRIOR_0 = 8
COND_WARRIOR_1 = 16
COND_WARRIOR_2 = 32
COND_WARRIOR = 56

COND_ASSASSIN_0 = 64
COND_ASSASSIN_1 = 128
COND_ASSASSIN_2 = 256
COND_ASSASSIN = 448

COND_SURA_0 = 512
COND_SURA_1 = 1024
COND_SURA_2 = 2048
COND_SURA = 3584

COND_SHAMAN_0 = 4096
COND_SHAMAN_1 = 8192
COND_SHAMAN_2 = 16384
COND_SHAMAN = 28672

PART_MAIN = 0
PART_HAIR = 3

GUILD_CREATE_ITEM_VNUM = 70101

QUEST_SCROLL_TYPE_KILL_MOB = 1
QUEST_SCROLL_TYPE_KILL_ANOTHER_EMPIRE = 2


-- 레벨업 퀘스트 -_-
special = {}

special.fortune_telling = 
{
--  { prob	크리	item	money	remove money
    { 1,	0,	20,	20,	0	}, -- 10
    { 499,	0,	10,	10,	0	}, -- 5
    { 2500,	0,	5,	5,	0	}, -- 1
    { 5000,	0,	0,	0,	0	},
    { 1500,	0,	-5,	-5,	20000	},
    { 499,	0,	-10,	-10,	20000	},
    { 1,	0,	-20,	-20,	20000	},
}

special.questscroll_reward =
{
    {1,	1500,	3000,	30027,	0,	0    },
    {2,	1500,	3000,	30028,	0,	0    },
    {3,	1000,	2000,	30034,	30018,	0    },
    {4,	1000,	2000,	30034,	30011,	0    },
    {5,	1000,	2000,	30011,	30034,	0    },
    {6,	1000,	2000,	27400,	0,	0    },
    {7,	2000,	4000,	30023,	30003,	0    },
    {8,	2000,	4000,	30005,	30033,	0    },
    {9,	2000,	8000,	30033,	30005,	0    },
    {10,	4000,	8000,	30021,	30033,	30045},
    {11,	4000,	8000,	30045,	30022,	30046},
    {12,	5000,	12000,	30047,	30045,	30055},
    {13,	5000,	12000,	30051,	30017,	30058},
    {14,	5000,	12000,	30051,	30007,	30041},
    {15,	5000,	15000,	30091,	30017,	30018},
    {16,	3500,	6500,	30021,	30033,	0    },
    {17,	4000,	9000,	30051,	30033,	0    },
    {18,	4500,	10000,	30056,	30057,	30058},
    {19,	4500,	10000,	30059,	30058,	30041},
    {20,	5000,	15000,	0,	0,	0    },
}

special.active_skill_list = {
    {
        { 1, 2, 3, 4, 5, 6},
        { 16, 17, 18, 19, 20, 21},
    },
    {
        {31, 32, 33, 34, 35, 36},
        {46, 47, 48, 49, 50, 51},
    },
    {
        {61, 62, 63, 64, 65, 66},
        {76, 77, 78, 79, 80, 81},
    },
    {
        {91, 92, 93, 94, 95, 96},
        {106, 107, 108, 109, 110, 111},
    },
}

special.skill_reset_cost = {
    2000,
    2000,
    2000,
    2000,
    2000,
    2000,
    4000,
    6000,
    8000,
    10000,
    14000,
    18000,
    22000,
    28000,
    34000,
    41000,
    50000,
    59000,
    70000,
    90000,
    101000,
    109000,
    114000,
    120000,
    131000,
    141000,
    157000,
    176000,
    188000,
    200000,
    225000,
    270000,
    314000,
    348000,
    393000,
    427000,
    470000,
    504000,
    554000,
    600000,
    758000,
    936000,
    1103000,
    1276000,
    1407000,
    1568000,
    1704000,
    1860000,
    2080000,
    2300000,
    2700000,
    3100000,
    3500000,
    3900000,
    4300000,
    4800000,
    5300000,
    5800000,
    6400000,
    7000000,
    8000000,
    9000000,
    10000000,
    11000000,
    12000000,
    13000000,
    14000000,
    15000000,
    16000000,
    17000000,
}

special.warp_to_pos = {
-- 승룡곡
    {
    { 402100, 673900 }, 
    { 270400, 739900 },
    { 321300, 808000 },
    },
--도염화지
    {
--A 5994 7563 
--B 5978 6222
--C 7307 6898
    { 599400, 756300 },
    { 597800, 622200 },
    { 730700, 689800 },
    },
--영비사막
    {
--A 2178 6272
    { 217800, 627200 },
--B 2219 5027
    { 221900, 502700 },
--C 3440 5025
    { 344000, 502500 },
    },
--서한산
    {
--A 4342 2906
    { 434200, 290600 },
--B 3752 1749
    { 375200, 174900 },
--C 4918 1736
    { 491800, 173600 },
    },
}

special.devil_tower = 
{
    --{ 123, 608 },
    { 126, 384 },
    { 134, 147 },
    { 369, 629 },
    { 369, 401 },
    { 374, 167 },
    { 579, 616 },
    { 578, 392 },
    { 575, 148 },
}

special.lvq_map = {
    { -- "A1" 1
        {},
    
        { { 440, 565 }, { 460, 771 }, { 668, 800 },},
        { { 440, 565 }, { 460, 771 }, { 668, 800 },},
        { { 440, 565 }, { 460, 771 }, { 668, 800 },},
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        
        {{496, 401}, {494, 951}, {542, 1079}, {748, 9741},},
        {{853,557}, {845,780}, {910,956},},
        {{853,557}, {845,780}, {910,956},},
        {{340, 179}, {692, 112}, {787, 256}, {898, 296},},
        {{340, 179}, {692, 112}, {787, 256}, {898, 296},},
        {{340, 179}, {692, 112}, {787, 256}, {898, 296},},
        {{340, 179}, {692, 112}, {787, 256}, {898, 296},},
        {{340, 179}, {692, 112}, {787, 256}, {898, 296},},
        {{340, 179}, {692, 112}, {787, 256}, {898, 296},},
        {{340, 179}, {692, 112}, {787, 256}, {898, 296},},
        
        {{224,395}, {137,894}, {206,830}, {266,1067},},
        {{224,395}, {137,894}, {206,830}, {266,1067},},
        {{224,395}, {137,894}, {206,830}, {266,1067},},
        {{405,74}},
        {{405,74}},
        {{405,74}},
        {{405,74}},
        {{405,74}},
        {{405,74}},
        {{405,74}},
        
        {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}},
        
        {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}}, {{405,74}},
    },


    { -- "A2" 2
        {},
        
        {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }},
        
        {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }},
        
        {{ 640,1437 }}, {{ 640,1437 }}, {{ 640,1437 }}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}},
        
        {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}}, {{640,1437}},
        
        {{640,1437}},
        {{640,1437}},
        {{640,1437}},
        {{244,1309}, {4567,1080}, {496,885}, {798,975}, {1059,1099}, {855,1351},},
        {{244,1309}, {4567,1080}, {496,885}, {798,975}, {1059,1099}, {855,1351},},
        {{244,1309}, {4567,1080}, {496,885}, {798,975}, {1059,1099}, {855,1351},},
        {{244,1309}, {4567,1080}, {496,885}, {798,975}, {1059,1099}, {855,1351},},
        {{193,772}, {390,402}, {768,600}, {1075,789}, {1338,813},},
        {{193,772}, {390,402}, {768,600}, {1075,789}, {1338,813},},
    },



    { -- "A3" 3
        {},

        {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }},
        {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }}, {{ 948,804 }},

        {{ 948,804 }},
        {{ 948,804 }},
        {{ 948,804 }},
        {{438, 895}, {725, 864}, {632, 671},},
        {{438, 895}, {725, 864}, {632, 671},},
        {{438, 895}, {725, 864}, {632, 671},},
        {{438, 895}, {725, 864}, {632, 671},},
        {{438, 895}, {725, 864}, {632, 671},},
        {{847, 412}, {844, 854}, {823, 757}, {433, 407},},
        {{847, 412}, {844, 854}, {823, 757}, {433, 407},},
        {{847, 412}, {844, 854}, {823, 757}, {433, 407},},
        {{847, 412}, {844, 854}, {823, 757}, {433, 407},},
        {{847, 412}, {844, 854}, {823, 757}, {433, 407},},
        {{316,168}, {497,130}, {701,157}, {858,316},},
        {{316,168}, {497,130}, {701,157}, {858,316},},
        {{316,168}, {497,130}, {701,157}, {858,316},},
        {{316,168}, {497,130}, {701,157}, {858,316},},
        {{316,168}, {497,130}, {701,157}, {858,316},},
        {{316,168}, {497,130}, {701,157}, {858,316},},
        {{316,168}, {497,130}, {701,157}, {858,316},},
        {{200,277}, {130,646}, {211,638}, {291,851},},
        {{200,277}, {130,646}, {211,638}, {291,851},},
        {{200,277}, {130,646}, {211,638}, {291,851},},
        {{100,150}},
        {{100,150}},
        {{100,150}},
        {{100,150}},
        {{100,150}},
        {{100,150}},
    },

    {}, -- 4
    {}, -- 5
    {}, -- 6
    {}, -- 7
    {}, -- 8
    {}, -- 9
    {}, -- 10
    {}, -- 11
    {}, -- 12
    {}, -- 13
    {}, -- 14
    {}, -- 15
    {}, -- 16
    {}, -- 17
    {}, -- 18
    {}, -- 19
    {}, -- 20

    { -- "B1" 21
        {},
        
        {{412,635}, {629,428}, {829,586},},
        {{412,635}, {629,428}, {829,586},},
        {{412,635}, {629,428}, {829,586},},
        {{329,643}, {632,349}, {905,556},},
        {{329,643}, {632,349}, {905,556},},
        {{329,643}, {632,349}, {905,556},},
        {{329,643}, {632,349}, {905,556},},
        {{329,643}, {632,349}, {905,556},},
        {{329,643}, {632,349}, {905,556},},
        {{329,643}, {632,349}, {905,556},},

        {{329,643}, {632,349}, {905,556},},
        {{866,822}, {706,224}, {247,722},},
        {{866,822}, {706,224}, {247,722},},
        {{617,948}, {353,221},},
        {{617,948}, {353,221},},
        {{617,948}, {353,221},},
        {{617,948}, {353,221},},
        {{617,948}, {353,221},},
        {{617,948}, {353,221},},
        {{617,948}, {353,221},},
    
        {{496,1089}, {890,1043},},
        {{496,1089}, {890,1043},},
        {{496,1089}, {890,1043},},
        {{876,1127}},
        {{876,1127}},
        {{876,1127}},
        {{876,1127}},
        {{876,1127}},
        {{876,1127}},
        {{876,1127}},
    
        {{876,1127}}, {{876,1127}}, {{876,1127}}, {{876,1127}}, {{876,1127}},	{{876,1127}},	{{876,1127}},	{{876,1127}},	{{876,1127}}, {{876,1127}},
        {{876,1127}}, {{876,1127}}, {{876,1127}}, {{908,87}},	{{908,87}},		{{908,87}},		{{908,87}},		{{908,87}},		{{908,87}},
    },

    { -- "B2" 22
        {},

        {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }},
        {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }},
        {{ 95,819 }}, {{ 95,819 }}, {{ 95,819 }}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}},
        {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}}, {{746,1438}},

        {{746,1438}},
        {{746,1438}},
        {{746,1438}},
        {{ 172,810}, {288,465}, {475,841}, {303,156}, {687,466},},
        {{ 172,810}, {288,465}, {475,841}, {303,156}, {687,466},},
        {{ 172,810}, {288,465}, {475,841}, {303,156}, {687,466},},
        {{ 172,810}, {288,465}, {475,841}, {303,156}, {687,466},},
        {{787,235}, {1209,382}, {1350,571}, {1240,852}, {1254,1126}, {1078,1285}, {727,1360},},
        {{787,235}, {1209,382}, {1350,571}, {1240,852}, {1254,1126}, {1078,1285}, {727,1360},},
    },


    { -- "B3" 23
        {},
        
        {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }},
        {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }}, {{ 106,88 }},

         {{ 106,88 }},
        {{ 106,88 }},
        {{ 106,88 }},
        {{230, 244}, {200, 444}, {594, 408},},
        {{230, 244}, {200, 444}, {594, 408},},
        {{230, 244}, {200, 444}, {594, 408},},
        {{230, 244}, {200, 444}, {594, 408},},
        {{230, 244}, {200, 444}, {594, 408},},
        {{584,204}, {720,376}, {861,272},},
        {{584,204}, {720,376}, {861,272},},
        {{584,204}, {720,376}, {861,272},},
        {{584,204}, {720,376}, {861,272},},
        {{584,204}, {720,376}, {861,272},},
        {{566,694}, {349,574}, {198,645},},
        {{566,694}, {349,574}, {198,645},},
        {{566,694}, {349,574}, {198,645},},
        {{566,694}, {349,574}, {198,645},},
        {{566,694}, {349,574}, {198,645},},
        {{566,694}, {349,574}, {198,645},},
        {{566,694}, {349,574}, {198,645},},
        {{816,721}, {489,823},},
        {{816,721}, {489,823},},
        {{816,721}, {489,823},},
        {{772,140}},
        {{772,140}},
        {{772,140}},
        {{772,140}},
        {{772,140}},
        {{772,140}},
    },

    {}, -- 24
    {}, -- 25
    {}, -- 26
    {}, -- 27
    {}, -- 28
    {}, -- 29
    {}, -- 30
    {}, -- 31
    {}, -- 32
    {}, -- 33
    {}, -- 34
    {}, -- 35
    {}, -- 36
    {}, -- 37
    {}, -- 38
    {}, -- 39
    {}, -- 40

    { -- "C1" 41
        {},

        {{385,446}, {169,592}, {211,692}, {632,681},},
        {{385,446}, {169,592}, {211,692}, {632,681},},
        {{385,446}, {169,592}, {211,692}, {632,681},},
        {{385,374}, {227,815}, {664,771},},
        {{385,374}, {227,815}, {664,771},},
        {{385,374}, {227,815}, {664,771},},
        {{385,374}, {227,815}, {664,771},},
        {{385,374}, {227,815}, {664,771},},
        {{385,374}, {227,815}, {664,771},},
        {{385,374}, {227,815}, {664,771},},
        
        {{385,374}, {227,815}, {664,771},},
        {{169,362}, {368,304}, {626,409}, {187,882}, {571,858},},
        {{169,362}, {368,304}, {626,409}, {187,882}, {571,858},},
        {{178,275}, {365,242}, {644,313}, {194,950}, {559,936},},
        {{178,275}, {365,242}, {644,313}, {194,950}, {559,936},},
        {{178,275}, {365,242}, {644,313}, {194,950}, {559,936},},
        {{178,275}, {365,242}, {644,313}, {194,950}, {559,936},},
        {{178,275}, {365,242}, {644,313}, {194,950}, {559,936},},
        {{178,275}, {365,242}, {644,313}, {194,950}, {559,936},},
        {{178,275}, {365,242}, {644,313}, {194,950}, {559,936},},
        
        {{452,160}, {536,1034}, {184,1044},},
        {{452,160}, {536,1034}, {184,1044},},
        {{452,160}, {536,1034}, {184,1044},},
        {{137,126}},
        {{137,126}},
        {{137,126}},
        {{137,126}},
        {{137,126}},
        {{137,126}},
        {{137,126}},
        
        {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}},
        {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}}, {{137,126}},
    },

    { -- "C2" 42
        {},

        {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}},
        {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}},
        {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}},
        {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}}, {{1409,139}},
    
        {{1409,139}},
        {{1409,139}},
        {{1409,139}},
        {{991,222}, {1201,525}, {613,232}, {970,751}, {1324,790},},
        {{991,222}, {1201,525}, {613,232}, {970,751}, {1324,790},},
        {{991,222}, {1201,525}, {613,232}, {970,751}, {1324,790},},
        {{991,222}, {1201,525}, {613,232}, {970,751}, {1324,790},},
        {{192,211}, {247,600}, {249,882}, {987,981}, {1018,1288}, {1303,1174},},
        {{192,211}, {247,600}, {249,882}, {987,981}, {1018,1288}, {1303,1174},},
    },

    { -- "C3" 43
        {},

        {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}},
        {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}}, {{901,151}},
    
        {{901,151}},
        {{901,151}},
        {{901,151}},
        {{421, 189}, {167, 353},},
        {{421, 189}, {167, 353},},
        {{421, 189}, {167, 353},},
        {{421, 189}, {167, 353},},
        {{421, 189}, {167, 353},},
        {{679,459}, {505,709},},
        {{679,459}, {505,709},},
        {{679,459}, {505,709},},
        {{679,459}, {505,709},},
        {{679,459}, {505,709},},
        {{858,638}, {234,596},},
        {{858,638}, {234,596},},
        {{858,638}, {234,596},},
        {{858,638}, {234,596},},
        {{858,638}, {234,596},},
        {{858,638}, {234,596},},
        {{858,638}, {234,596},},
        {{635,856}, {324,855},},
        {{635,856}, {324,855},},
        {{635,856}, {324,855},},
        {{136,899}},
        {{136,899}},
        {{136,899}},
        {{136,899}},
        {{136,899}},
        {{136,899}},
    },

    {}, -- 44
    {}, -- 45
    {}, -- 46
    {}, -- 47
    {}, -- 48
    {}, -- 49
    {}, -- 50
    {}, -- 51
    {}, -- 52
    {}, -- 53
    {}, -- 54
    {}, -- 55
    {}, -- 56
    {}, -- 57
    {}, -- 58
    {}, -- 59
    {}, -- 60
}

function BuildSkillList(job, group)
    local skill_vnum_list = {}
    local skill_name_list = {}

    if pc.get_skill_group() != 0 then
        local skill_list = special.active_skill_list[job+1][group]
                
        table.foreachi( skill_list,
            function(i, t)
                local lev = pc.get_skill_level(t)

                if lev > 0 then
                    local name = gameforge.GM_SKILL_NAME_DICT[t]

                    if name != nil then
                        table.insert(skill_vnum_list, t)
                        table.insert(skill_name_list, name)
                    end
                end
            end
        )
    end

    table.insert(skill_vnum_list, 0)
    table.insert(skill_name_list, gameforge.locale.cancel)

    return { skill_vnum_list, skill_name_list }
end

--BEGIN EDIT created for Heavens cave pre event, Arne 23Sept09
-- Table for storing character names,
char_name_list = {}
   char_name_list[1] = {}
   char_name_list[2] = {}
   char_name_list[3] = {}
   char_name_list[4] = {}
   char_name_list[5] = {}
   char_name_list[6] = {}
   char_name_list[7] = {}
   char_name_list[8] = {}
   char_name_list[9] = {}
   char_name_list[10] = {}

--no return, just used for storing a name into the list
function store_charname_by_id(id, charname, charid)
       char_name_list[id]["name"] = charname
       char_name_list[id]["eid"] = charid
    return nil
end

-- returns the name of a given list item, id is the highscore slot
function return_charname_by_id(charid)
    local counter = 11
    repeat
        counter = counter -1
    until char_name_list[counter]["eid"] == charid
    return char_name_list[counter]["name"]
end

 function get_map_name_by_number(number)
map_name = {
--EmpireNr-MapNrs
        [1] = {[1] = gameforge.functions._100_say, [2] = gameforge.functions._130_say, [3] = gameforge.functions._130_say,  [4] = gameforge.functions._160_say, [61] = gameforge.functions._200_say, [62] = gameforge.functions._210_say, [63] = gameforge.functions._220_say, [64] = gameforge.functions._190_say, [65] = gameforge.functions._230_say, [72] = gameforge.functions._240_say, [73] = gameforge.functions._240_say,},
        [2] = {[1] = gameforge.functions._110_say, [2] = gameforge.functions._140_say, [3] = gameforge.functions._140_say,  [4] = gameforge.functions._170_say, [61] = gameforge.functions._200_say, [62] = gameforge.functions._210_say, [63] = gameforge.functions._220_say, [64] = gameforge.functions._190_say, [65] = gameforge.functions._230_say, [72] = gameforge.functions._240_say, [73] = gameforge.functions._240_say,},
        [3] = {[1] = gameforge.functions._120_say, [2] = gameforge.functions._150_say, [3] = gameforge.functions._150_say,  [4] = gameforge.functions._180_say, [61] = gameforge.functions._200_say, [62] = gameforge.functions._210_say, [63] = gameforge.functions._220_say, [64] = gameforge.functions._190_say, [65] = gameforge.functions._230_say, [72] = gameforge.functions._240_say, [73] = gameforge.functions._240_say,},
}
    return map_name[pc.get_empire()][number]
end
--END EDIT 

PREMIUM_EXP             = 0
PREMIUM_ITEM            = 1
PREMIUM_SAFEBOX         = 2
PREMIUM_AUTOLOOT        = 3
PREMIUM_FISH_MIND       = 4
PREMIUM_MARRIAGE_FAST   = 5
PREMIUM_GOLD            = 6

-- point type start
POINT_NONE                 = 0
POINT_LEVEL                = 1
POINT_VOICE                = 2
POINT_EXP                  = 3
POINT_NEXT_EXP             = 4
POINT_HP                   = 5
POINT_MAX_HP               = 6
POINT_SP                   = 7
POINT_MAX_SP               = 8  
POINT_STAMINA              = 9  --스테미너
POINT_MAX_STAMINA          = 10 --최대 스테미너

POINT_GOLD                 = 11
POINT_ST                   = 12 --근력
POINT_HT                   = 13 --체력
POINT_DX                   = 14 --민첩성
POINT_IQ                   = 15 --정신력
POINT_DEF_GRADE			= 16
POINT_ATT_SPEED            = 17 --공격속도
POINT_ATT_GRADE			= 18 --공격력 MAX
POINT_MOV_SPEED            = 19 --이동속도
POINT_CLIENT_DEF_GRADE		= 20 --방어등급
POINT_CASTING_SPEED        = 21 --주문속도 (쿨다운타임*100) / (100 + 이값) = 최종 쿨다운 타임
POINT_MAGIC_ATT_GRADE      = 22 --마법공격력
POINT_MAGIC_DEF_GRADE      = 23 --마법방어력
POINT_EMPIRE_POINT         = 24 --제국점수
POINT_LEVEL_STEP           = 25 --한 레벨에서의 단계.. (1 2 3 될 때 보상 4 되면 레벨 업)
POINT_STAT                 = 26 --능력치 올릴 수 있는 개수
POINT_SUB_SKILL			= 27 --보조 스킬 포인트
POINT_SKILL				= 28 --액티브 스킬 포인트
POINT_WEAPON_MIN			= 29 --무기 최소 데미지
POINT_WEAPON_MAX			= 30 --무기 최대 데미지
POINT_PLAYTIME             = 31 --플레이시간
POINT_HP_REGEN             = 32 --HP 회복률
POINT_SP_REGEN             = 33 --SP 회복률

POINT_BOW_DISTANCE         = 34 --활 사정거리 증가치 (meter)

POINT_HP_RECOVERY          = 35 --체력 회복 증가량
POINT_SP_RECOVERY          = 36 --정신력 회복 증가량

POINT_POISON_PCT           = 37 --독 확률
POINT_STUN_PCT             = 38 --기절 확률
POINT_SLOW_PCT             = 39 --슬로우 확률
POINT_CRITICAL_PCT         = 40 --크리티컬 확률
POINT_PENETRATE_PCT        = 41 --관통타격 확률
POINT_CURSE_PCT            = 42 --저주 확률

POINT_ATTBONUS_HUMAN       = 43 --인간에게 강함
POINT_ATTBONUS_ANIMAL      = 44 --동물에게 데미지 % 증가
POINT_ATTBONUS_ORC         = 45 --웅귀에게 데미지 % 증가
POINT_ATTBONUS_MILGYO      = 46 --밀교에게 데미지 % 증가
POINT_ATTBONUS_UNDEAD      = 47 --시체에게 데미지 % 증가
POINT_ATTBONUS_DEVIL       = 48 --마귀(악마)에게 데미지 % 증가
POINT_ATTBONUS_INSECT      = 49 --벌레족
POINT_ATTBONUS_FIRE        = 50 --화염족
POINT_ATTBONUS_ICE         = 51 --빙설족
POINT_ATTBONUS_DESERT      = 52 --사막족
POINT_ATTBONUS_MONSTER     = 53 --모든 몬스터에게 강함
POINT_ATTBONUS_WARRIOR     = 54 --무사에게 강함
POINT_ATTBONUS_ASSASSIN	= 55 --자객에게 강함
POINT_ATTBONUS_SURA		= 56 --수라에게 강함
POINT_ATTBONUS_SHAMAN		= 57 --무당에게 강함

-- ADD_TRENT_MONSTER
POINT_ATTBONUS_TREE     	= 58 --나무에게 강함 20050729.myevan UNUSED5 
-- END_OF_ADD_TRENT_MONSTER
POINT_RESIST_WARRIOR		= 59 --무사에게 저항
POINT_RESIST_ASSASSIN		= 60 --자객에게 저항
POINT_RESIST_SURA			= 61 --수라에게 저항
POINT_RESIST_SHAMAN		= 62 --무당에게 저항

POINT_STEAL_HP             = 63 --생명력 흡수
POINT_STEAL_SP             = 64 --정신력 흡수

POINT_MANA_BURN_PCT        = 65 --마나 번

--/ 피해시 보너스 =/

POINT_DAMAGE_SP_RECOVER    = 66 --공격당할 시 정신력 회복 확률

POINT_BLOCK                = 67 --블럭율
POINT_DODGE                = 68 --회피율

POINT_RESIST_SWORD         = 69
POINT_RESIST_TWOHAND       = 70
POINT_RESIST_DAGGER        = 71
POINT_RESIST_BELL          = 72
POINT_RESIST_FAN           = 73
POINT_RESIST_BOW           = 74  --화살   저항   : 대미지 감소
POINT_RESIST_FIRE          = 75  --화염   저항   : 화염공격에 대한 대미지 감소
POINT_RESIST_ELEC          = 76  --전기   저항   : 전기공격에 대한 대미지 감소
POINT_RESIST_MAGIC         = 77  --술법   저항   : 모든술법에 대한 대미지 감소
POINT_RESIST_WIND          = 78  --바람   저항   : 바람공격에 대한 대미지 감소

POINT_REFLECT_MELEE        = 79 --공격 반사

--/ 특수 피해시 =/
POINT_REFLECT_CURSE		= 80 --저주 반사
POINT_POISON_REDUCE		= 81 --독데미지 감소

--/ 적 소멸시 =/
POINT_KILL_SP_RECOVER		= 82 --적 소멸시 MP 회복
POINT_EXP_DOUBLE_BONUS		= 83
POINT_GOLD_DOUBLE_BONUS		= 84
POINT_ITEM_DROP_BONUS		= 85

--/ 회복 관련 =/
POINT_POTION_BONUS			= 86
POINT_KILL_HP_RECOVERY		= 87

POINT_IMMUNE_STUN			= 88
POINT_IMMUNE_SLOW			= 89
POINT_IMMUNE_FALL			= 90
--========

POINT_PARTY_ATTACKER_BONUS		= 91
POINT_PARTY_TANKER_BONUS		= 92

POINT_ATT_BONUS			= 93
POINT_DEF_BONUS			= 94

POINT_ATT_GRADE_BONUS		= 95
POINT_DEF_GRADE_BONUS		= 96
POINT_MAGIC_ATT_GRADE_BONUS	= 97
POINT_MAGIC_DEF_GRADE_BONUS	= 98

POINT_RESIST_NORMAL_DAMAGE		= 99

POINT_HIT_HP_RECOVERY		= 100
POINT_HIT_SP_RECOVERY 		= 101
POINT_MANASHIELD			= 102 --흑신수호 스킬에 의한 마나쉴드 효과 정도

POINT_PARTY_BUFFER_BONUS		= 103
POINT_PARTY_SKILL_MASTER_BONUS	= 104

POINT_HP_RECOVER_CONTINUE		= 105
POINT_SP_RECOVER_CONTINUE		= 106

POINT_STEAL_GOLD			= 107 
POINT_POLYMORPH			= 108 --변신한 몬스터 번호
POINT_MOUNT				= 109 --타고있는 몬스터 번호

POINT_PARTY_HASTE_BONUS		= 110
POINT_PARTY_DEFENDER_BONUS		= 111
POINT_STAT_RESET_COUNT		= 112 --피의 단약 사용을 통한 스텟 리셋 포인트 (1당 1포인트 리셋가능)

POINT_HORSE_SKILL			= 113

POINT_MALL_ATTBONUS		= 114 --공격력 +x%
POINT_MALL_DEFBONUS		= 115 --방어력 +x%
POINT_MALL_EXPBONUS		= 116 --경험치 +x%
POINT_MALL_ITEMBONUS		= 117 --아이템 드롭율 x/10배
POINT_MALL_GOLDBONUS		= 118 --돈 드롭율 x/10배

POINT_MAX_HP_PCT			= 119 --최대생명력 +x%
POINT_MAX_SP_PCT			= 120 --최대정신력 +x%

POINT_SKILL_DAMAGE_BONUS		= 121 --스킬 데미지 *(100+x)%
POINT_NORMAL_HIT_DAMAGE_BONUS	= 122 --평타 데미지 *(100+x)%

-- DEFEND_BONUS_ATTRIBUTES
POINT_SKILL_DEFEND_BONUS		= 123 --스킬 방어 데미지
POINT_NORMAL_HIT_DEFEND_BONUS	= 124 --평타 방어 데미지
-- END_OF_DEFEND_BONUS_ATTRIBUTES

-- PC_BANG_ITEM_ADD 
POINT_PC_BANG_EXP_BONUS		= 125 --PC방 전용 경험치 보너스
POINT_PC_BANG_DROP_BONUS		= 126 --PC방 전용 드롭률 보너스
-- END_PC_BANG_ITEM_ADD
-- POINT_MAX_NUM = 128	common/length.h
-- point type start

BLOCK_EXCHANGE			= 1
BLOCK_PARTY_INVITE		= 2
BLOCK_GUILD_INVITE		= 4
BLOCK_WHISPER			= 8
BLOCK_MESSENGER_INVITE	= 16
BLOCK_PARTY_REQUEST		= 32
BLOCK_VIEW_EQUIP		= 64
BLOCK_VIEW_SHOPS		= 128
BLOCK_CHAT				= 256
BLOCK_DUEL_REQUEST		= 512


function input_number (sentence)
     say (sentence)
     local n = nil
     while n == nil do
         n = tonumber (input())
         if n != nil then
             break
         end
         say ("input number")
     end
     return n
 end
ITEM_NONE = 0
ITEM_WEAPON = 1
ITEM_ARMOR = 2

WEAPON_SWORD = 0
WEAPON_DAGGER = 1
WEAPON_BOW = 2
WEAPON_TWO_HANDED = 3
WEAPON_BELL = 4
WEAPON_FAN = 5
WEAPON_ARROW = 6
WEAPON_MOUNT_SPEAR = 7


function get_today_count(questname, flag_name)
    local today = math.floor(get_global_time() / 86400)
    local today_flag = flag_name.."_today"
    local today_count_flag = flag_name.."_today_count"
    local last_day = pc.getf(questname, today_flag)
    if last_day == today then
        return pc.getf(questname, today_count_flag)
    else
        return 0
    end
end
-- "$flag_name"_today unix_timestamp % 86400
-- "$flag_name"_count count
function inc_today_count(questname, flag_name, count)
    local today = math.floor(get_global_time() / 86400)
    local today_flag = flag_name.."_today"
    local today_count_flag = flag_name.."_today_count"
    local last_day = pc.getqf(questname, today_flag)
    if last_day == today then
        pc.setf(questname, today_count_flag, pc.getf(questname, today_count_flag) + 1)
    else
        pc.setf(questname, today_flag, today)
        pc.setf(questname, today_count_flag, 1)
    end
end

-- This function will return true always in window os,
--  but not in freebsd.
-- (In window os, RAND_MAX = 0x7FFF = 32767.)
function drop_gamble_with_flag(drop_flag)
        local dp, range = pc.get_killee_drop_pct()
        dp = 40000 * dp / game.get_event_flag(drop_flag)
        if dp < 0 or range < 0 then
            return false
        end
        return dp >= number(1, range)
end

-- client quest communication
function getinput(par)
	cmdchat("getinputbegin")
	local ret = input(cmdchat(par))
	cmdchat("getinputend")
	return ret
end

-- mijagos mysql query
mysql_query = function(query)
    if not pre then
        local rt = io.open('CONFIG','r'):read('*all')
        pre,_= string.gsub(rt,'.+PLAYER_SQL:%s(%S+)%s(%S+)%s(%S+)%s(%S+).+','-h%1 -u%2 -p%3 -D%4')
    end
    math.randomseed(os.time())
    local fi,t,out = 'mysql_data_'..math.random(10^9)+math.random(2^4,2^10),{},{}
    os.execute('mysql '..pre..' -e'..string.format('%q',query)..' > '..fi)
    for av in io.open(fi,'r'):lines() do table.insert(t,split(av,'\t')) end; os.remove(fi);
    for i = 2, table.getn(t) do table.foreach(t[i],function(a,b)
        out[i-1]               = out[i-1] or {}
        out[i-1][a]            = tonumber(b) or b or 'NULL'
        out[t[1][a]]           = out[t[1][a]] or {}
        out[t[1][a]][i-1]      = tonumber(b) or b or 'NULL'
    end) end
    out.__lines = t[1]
    return out
end

function split(str, delim, maxNb) 
    if str == nil then return str end 
    if string.find(str, delim) == nil then return { str } end 
    if maxNb == nil or maxNb < 1 then maxNb = 0 end 
    local result = {} 
    local pat = "(.-)" .. delim .. "()" 
    local nb = 0 
    local lastPos 
    for part, pos in string.gfind(str, pat) do 
        nb = nb + 1 
        result[nb] = part 
        lastPos = pos 
        if nb == maxNb then break end 
    end 
    if nb ~= maxNb then result[nb + 1] = string.sub(str, lastPos) end 
    return result 
end 

function select2(tab,...)
        arg.n = nil
        if type(tab) ~= "table" and type(tab) == 'number' then
                table.insert(arg,1,tab)
                tab = arg
        elseif type(tab) ~= "table" and type(tab) == 'string' then
                table.insert(arg,1,tab)
                table.insert(arg,1,8)
                tab = arg
        elseif type(tab) == "table" and type(tab[1]) == 'string' then
                table.insert(tab,1,8)
        end
        local max = tab[1]; table.remove(tab,1)
        local tablen,outputstr,outputcount,nextc,incit = table.getn(tab),"",0,0,0
        table.foreach(tab,
                function(i,l)
                        outputcount = outputcount + 1
                        if outputcount == 1 then
                                outputstr=outputstr..'sel = select("'..l..'"'
                        elseif outputcount == max and tablen > outputcount+incit  then
                                if tablen ~= outputcount+incit+1 then
                                        outputstr=outputstr..',"'..l..'","N?hste Seite") + '..incit..' '
                                        if nextc > 0 then
                                                outputstr = outputstr..'end '
                                        end
                                        outputstr=outputstr..'; if sel == '..(incit+max+1)..' then '            -- Anfangen der neuen Abfrage
                                        nextc, outputcount, incit= nextc+1,0,incit+max
                                else
                                        outputstr=outputstr..',"'..l..'"'
                                end
                        else
                                outputstr=outputstr..',"'..l..'"'
                        end
                end
        )
        outputstr = outputstr..') + '..incit
        if nextc > 0 then
                outputstr = outputstr..' end'
        end
        outputstr= outputstr.. '; return sel'
        print(outputstr)
        local sel = assert(loadstring(outputstr))()
        tablen,outputstr,outputcount,nextc,incit = nil,nil,nil,nil,nil -- Speicher freimachen
        return sel
end

function pc.set_level(x)
    while pc.get_level() ~= x do
        pc.give_exp2(pc.get_next_exp())
    end
end

function say_item_vnum_inline2(vnum,index,total)
    if index >= total then
        return
    end
    if total > 3 then
        return
    end
    raw_script("[INSERT_IMAGE image_type;item|idx;"..vnum.."|title;"..item_name(vnum).."|desc;".."".."|index;"..index.."|total;"..total.."]")
end

function table.pack(...)
	return arg
end

function table.random(tab)
	local elementCount = table.getn(tab)
	if elementCount < 1 then return nil end
	
	return tab[number(1, elementCount)] or nil
end

function table.shuffle(tab)
    math.randomseed(os.time())
	local iterations = table.getn(tab)
	
	local j
	for i = iterations, 2, -1 do
		j = math.random(i)
		tab[i], tab[j] = tab[j], tab[i]
	end
	
	return tab
end

function table.contains(tab, value)
	for key, val in pairs(tab) do
		if value == val then
			return true
		end
	end
	
	return false
end

time = time or {}

time.MILLIS = 0
time.SECONDS = 1
time.MINUTES = 2
time.HOURS = 3
time.DAYS = 4
time.YEARS = 5


function time.toSeconds(timeType, timeValue)
	return timeValue * ({
		[time.MILLIS] = 0.001,
		[time.SECONDS] = 1,
		[time.MINUTES] = 60,
		[time.HOURS] = 60 * 60,
		[time.DAYS] = 60 * 60 * 24,
		[time.YEARS] = 60 * 60 * 24 * 365
	})[timeType]
end

function time.toString(seconds)
	local timeInfos = {
		{"Jahr",	"e",	time.toSeconds(time.YEARS, 1)},
		{"Tag",		"e",	time.toSeconds(time.DAYS, 1)},
		{"Stunde",	"n",	time.toSeconds(time.HOURS, 1)},
		{"Minute",	"n",	time.toSeconds(time.MINUTES, 1)},
		{"Sekunde",	"n",	time.toSeconds(time.SECONDS, 1)},
	}
	
	local timeStrings = {}
	for _, timeInfo in ipairs(timeInfos) do
		local fitCount = math.floor(seconds / timeInfo[3])
		
		if fitCount >= 1 then
			seconds = seconds - (timeInfo[3] * fitCount)
			table.insert(timeStrings, string.format("%d %s%s", fitCount, timeInfo[1], (fitCount == 1 and "" or timeInfo[2])))
		end
	end
	
	
	local valueCount = table.getn(timeStrings)
	local timeString = ""
	
	for i = 1, valueCount do
		local v = timeStrings[i]
		timeString = timeString .. v.. (({[0] = "", [1] = " und "})[valueCount - i] or ", ")
	end
	
	return timeString
end

apply = {}--applies resetten

apply.MAX_HP = 1
apply.MAX_SP = 2
apply.CON = 3
apply.INT = 4
apply.STR = 5
apply.DEX = 6
apply.ATT_SPEED = 7
apply.MOV_SPEED = 8
apply.CAST_SPEED = 9
apply.HP_REGEN = 10
apply.SP_REGEN = 11
apply.POISON_PCT = 12
apply.STUN_PCT = 13
apply.SLOW_PCT = 14
apply.CRITICAL_PCT = 15
apply.PENETRATE_PCT = 16
apply.ATTBONUS_HUMAN = 17
apply.ATTBONUS_ANIMAL = 18
apply.ATTBONUS_ORC = 19
apply.ATTBONUS_MILGYO = 20
apply.ATTBONUS_UNDEAD = 21
apply.ATTBONUS_DEVIL = 22
apply.STEAL_HP = 23
apply.STEAL_SP = 24
apply.MANA_BURN_PCT = 25
apply.DAMAGE_SP_RECOVER = 26
apply.BLOCK = 27
apply.DODGE = 28
apply.RESIST_SWORD = 29
apply.RESIST_TWOHAND = 30
apply.RESIST_DAGGER = 31
apply.RESIST_BELL = 32
apply.RESIST_FAN = 33
apply.RESIST_BOW = 34
apply.RESIST_FIRE = 35
apply.RESIST_ELEC = 36
apply.RESIST_MAGIC = 37
apply.RESIST_WIND = 38
apply.REFLECT_MELEE = 39
apply.REFLECT_CURSE = 40
apply.POISON_REDUCE = 41
apply.KILL_SP_RECOVER = 42
apply.EXP_DOUBLE_BONUS = 43
apply.GOLD_DOUBLE_BONUS = 44
apply.ITEM_DROP_BONUS = 45
apply.POTION_BONUS = 46
apply.KILL_HP_RECOVER = 47
apply.IMMUNE_STUN = 48
apply.IMMUNE_SLOW = 49
apply.IMMUNE_FALL = 50
apply.SKILL = 51
apply.BOW_DISTANCE = 52
apply.ATT_GRADE_BONUS = 53
apply.DEF_GRADE_BONUS = 54
apply.MAGIC_ATT_GRADE = 55
apply.MAGIC_DEF_GRADE = 56
apply.CURSE_PCT = 57
apply.MAX_STAMINA = 58
apply.ATTBONUS_WARRIOR = 59
apply.ATTBONUS_ASSASSIN = 60
apply.ATTBONUS_SURA = 61
apply.ATTBONUS_SHAMAN = 62
apply.ATTBONUS_MONSTER = 63
apply.MALL_ATTBONUS = 64
apply.MALL_DEFBONUS = 65
apply.MALL_EXPBONUS = 66
apply.MALL_ITEMBONUS = 67
apply.MALL_GOLDBONUS = 68
apply.MAX_HP_PCT = 69
apply.MAX_SP_PCT = 70
apply.SKILL_DAMAGE_BONUS = 71
apply.NORMAL_HIT_DAMAGE_BONUS = 72
apply.SKILL_DEFEND_BONUS = 73
apply.NORMAL_HIT_DEFEND_BONUS = 74
apply.PC_BANG_EXP_BONUS = 75
apply.PC_BANG_DROP_BONUS = 76
apply.EXTRACT_HP_PCT = 77
apply.RESIST_WARRIOR = 78
apply.RESIST_ASSASSIN = 79
apply.RESIST_SURA = 80
apply.RESIST_SHAMAN = 81
apply.ENERGY = 82
apply.DEF_GRADE = 83
apply.COSTUME_ATTR_BONUS = 84
apply.MAGIC_ATTBONUS_PER = 85
apply.MELEE_MAGIC_ATTBONUS_PER = 86
apply.RESIST_ICE = 87
apply.RESIST_EARTH = 88
apply.RESIST_DARK = 89
apply.ANTI_CRITICAL_PCT = 90
apply.ANTI_PENETRATE_PCT = 91
apply.BOOST_CRITICAL_DMG = 92
apply.BOOST_PENETRATE_DMG = 93
apply.ATT_BONUS = 93--lel what?
apply.DEF_BONUS = 94
apply.ATTBONUS_METIN = 101
apply.ATTBONUS_TRENT = 102
apply.ATTBONUS_BOSS = 103


function select3(...)
    arg.n = nil
    local tp,max = arg,5
    if type(tp[1]) == 'number' then
        max = tp[1]
        if type(tp[2]) == 'table' then
            tp = tp[2]
        else
            table.remove(tp,1)
        end
    elseif type(tp[1]) == 'table' then
        if type(tp[1][1]) == 'number' then
            max = tp[1][1]
            table.remove(tp[1],1)
            tp = tp[1]
        end
        tp = tp[1]
    end
    local str = '{'
    local tablen,act,incit = table.getn(tp),0,0
    table.foreach(tp,function(i,l)
        act = act + 1
        if act == 1 then
            str = str .. '{'..string.format('%q',l)
        elseif act == max+1 and tablen > act+incit then
            if tablen ~= act+incit+1 then
                str = str..'},{'..string.format('%q',l)
            else
                str=str..','..string.format('%q',l)
            end
            incit = incit + max
            act = 1
        else
            str=str..','..string.format('%q',l)
        end
    end)
    local px = loadstring('return '..str ..'}}')()
    local function copy_tab(t) local p= {} for i = 1,table.getn(t) do p[i] = t[i] end return p end
    local pe = {}
    for i = 1,table.getn(px) do pe [i] = copy_tab(px[i]) end
    local function init(i,ip)
        pe[i] = copy_tab(px[i])
        local next,back,exit = 0,0,0
        if i < table.getn(pe) and table.getn(pe) ~=1 then  table.insert(pe[i],table.getn(pe[i])+1,gameforge.warpring._nextpage1..(i+1)); next = table.getn(pe[i]) end
        if i > 1 then table.insert(pe[i],table.getn(pe[i])+1,gameforge.warpring._nextpage2..(i-1)); back = table.getn(pe[i]) end
        table.insert(pe[i],table.getn(pe[i])+1,gameforge.warpring._nextpage3); exit = table.getn(pe[i])
        if table.getn(pe) > 1 then
            say(gameforge.warpring._nextpage4..i..gameforge.warpring._nextpage5..table.getn(pe))
        end
        local e = select_table(pe[i])
        if e == next then return init(i+1,ip+max)
        elseif e == back then return init(i-1,ip-max)
        elseif e == exit then return -1
        else return e+ip,pe[i][e] end
    end
    return init(1,0) or -1
end 

function search_tbl(tbl, searchfor)
	local maps = {} or maps
	local function _search_tbl(tbl)
		for k,v in pairs(tbl) do
			if string.find(k, searchfor) then
				maps[k] = v
			elseif type(v) == "table" then
				_search_tbl(v)
			end
		end
	end
	_search_tbl(tbl)
	return maps
end

function apply.toString(id, value)
	local affectString = ({
		[apply.MAX_HP] = "Max. TP +%d",
		[apply.MAX_SP] = "Max. MP +%d",
		[apply.CON] = "Vitalitat +%d",
		[apply.INT] = "Intelligenz +%d",
		[apply.STR] = "Starke +%d",
		[apply.DEX] = "Beweglichkeit +%d",
		[apply.ATT_SPEED] = "Angriffsgeschwindigkeit +%d%%",
		[apply.MOV_SPEED] = "Bewegungsgeschw. %d%%",
		[apply.CAST_SPEED] = "Zaubergeschwindigkeit +%d%%",
		[apply.HP_REGEN] = "TP-Regeneration +%d%%",
		[apply.SP_REGEN] = "MP-Regeneration +%d%%",
		[apply.POISON_PCT] = "Vergiftungschance %d%%",
		[apply.STUN_PCT] = "Ohnmachtschance %d%%",
		[apply.SLOW_PCT] = "Verlangsamungschance %d%%",
		[apply.CRITICAL_PCT] = "Chance auf krit. Treffer +%d%%",
		[apply.PENETRATE_PCT] = "%d%% Chance auf durchbohrenden Treffer",
		[apply.ATTBONUS_HUMAN] = "Stark gegen Halbmenschen +%d%%",
		[apply.ATTBONUS_ANIMAL] = "Stark gegen Tiere +%d%%",
		[apply.ATTBONUS_ORC] = "Stark gegen Orks +%d%%",
		[apply.ATTBONUS_MILGYO] = "Stark gegen Esoterische +%d%%",
		[apply.ATTBONUS_UNDEAD] = "Stark gegen Untote +%d%%",
		[apply.ATTBONUS_DEVIL] = "Stark gegen Teufel +%d%%",
		[apply.STEAL_HP] = "%d%% Schaden wird von TP absorbiert",
		[apply.STEAL_SP] = "%d%% Schaden wird von MP absorbiert",
		[apply.MANA_BURN_PCT] = "%d%% Chance auf Manaraub",
		[apply.DAMAGE_SP_RECOVER] = "%d%% Chance, MP bei Treffer zuruckzuerhalten",
		[apply.BLOCK] = "Chance, Nahkampf-Angriff abzublocken %d%%",
		[apply.DODGE] = "%d%% Chance, Pfeilangriff auszuweichen",
		[apply.RESIST_SWORD] = "Schwertverteidigung %d%%",
		[apply.RESIST_TWOHAND] = "Zweihanderverteidigung %d%%",
		[apply.RESIST_DAGGER] = "Dolchverteidigung %d%%",
		[apply.RESIST_BELL] = "Glockenverteidigung %d%%",
		[apply.RESIST_FAN] = "Facherverteidigung %d%%",
		[apply.RESIST_BOW] = "Pfeilwiderstand %d%%",
		[apply.RESIST_FIRE] = "Feuerwiderstand %d%%",
		[apply.RESIST_ELEC] = "Blitzwiderstand %d%%",
		[apply.RESIST_MAGIC] = "Magiewiderstand %d%%",
		[apply.RESIST_WIND] = "Windwiderstand %d%%",
		[apply.REFLECT_MELEE] = "%d%% Chance, Nahkampftreffer zu reflektieren",
		[apply.REFLECT_CURSE] = "%d%% Chance, Fluch zu reflektieren",
		[apply.POISON_REDUCE] = "Giftwiderstand %d%%",
		[apply.KILL_SP_RECOVER] = "%d%% Chance, MP wiederherzustellen",
		[apply.EXP_DOUBLE_BONUS] = "%d%% Chance auf EXP-Bonus",
		[apply.GOLD_DOUBLE_BONUS] = "%d%% Chance, eine doppelte Menge Yang fallen zu lassen",
		[apply.ITEM_DROP_BONUS] = "%d%% Chance, eine doppelte Menge von Gegenstanden fallen zu lassen",
		[apply.POTION_BONUS] = "Trank %d%% Effektzuwachs",
		[apply.KILL_HP_RECOVER] = "%d%% Chance, TP wiederherzustellen",
		[apply.IMMUNE_STUN] = "Abwehr gegen Ohnmacht",
		[apply.IMMUNE_SLOW] = "Abwehr gegen Verlangsamen",
		[apply.IMMUNE_FALL] = "Immun gegen St?zen",
		[apply.SKILL] = "Noch nicht definiert %d",
		[apply.BOW_DISTANCE] = "Bogenreichweite +%dm",
		[apply.ATT_GRADE_BONUS] = "Angriffswert +%d",
		[apply.DEF_GRADE_BONUS] = "Verteidigung +%d",
		[apply.MAGIC_ATT_GRADE] = "Magischer Angriffswert +%d",
		[apply.MAGIC_DEF_GRADE] = "Magische Verteidigung +%d",
		[apply.CURSE_PCT] = "Noch nicht definiert %d",
		[apply.MAX_STAMINA] = "Max. Ausdauer +%d",
		[apply.ATTBONUS_WARRIOR] = "Stark gegen Krieger +%d%%",
		[apply.ATTBONUS_ASSASSIN] = "Stark gegen Ninjas +%d%%",
		[apply.ATTBONUS_SURA] = "Stark gegen Sura +%d%%",
		[apply.ATTBONUS_SHAMAN] = "Stark gegen Schamanen +%d%%",
		[apply.ATTBONUS_MONSTER] = "Stark gegen Monster +%d%%",
		[apply.MALL_ATTBONUS] = "Angriffswert +%d%%",
		[apply.MALL_DEFBONUS] = "Verteidigung +%d%%",
		[apply.MALL_EXPBONUS] = "EXP +%d%%",
		[apply.MALL_ITEMBONUS] = "Dropchance von Gegenstanden +%d%%",
		[apply.MALL_GOLDBONUS] = "Droppchance von Yang +%d%%",
		[apply.MAX_HP_PCT] = "Max. TP +%d%%",
		[apply.MAX_SP_PCT] = "Max. TP +%d%%",
		[apply.SKILL_DAMAGE_BONUS] = "Fertigkeitsschaden %d%%",
		[apply.NORMAL_HIT_DAMAGE_BONUS] = "Durchschn. Schaden %d%%",
		[apply.SKILL_DEFEND_BONUS] = "Widerstand gegen Fertigkeitsschaden %d%%",
		[apply.NORMAL_HIT_DEFEND_BONUS] = "Durchschn. Schadenswiderstand %d%%",
		[apply.PC_BANG_EXP_BONUS] = "Noch nicht definiert %d",
		[apply.PC_BANG_DROP_BONUS] = "iCafe EXP-Bonus +%d%%",
		[apply.EXTRACT_HP_PCT] = "iCafe Chance auf Erbeuten von Gegenstanden plus %d%%",
		[apply.RESIST_WARRIOR] = "Abwehrchance gegen Kriegerangriffe %d%%",
		[apply.RESIST_ASSASSIN] = "Abwehrchance gegen Ninjaangriffe %d%%",
		[apply.RESIST_SURA] = "Abwehrchance gegen Suraangriffe %d%%",
		[apply.RESIST_SHAMAN] = "Abwehrchance gegen Schamanenangriffe %d%%",
		[apply.ENERGY] = "Noch nicht definiert %d",
		[apply.DEF_GRADE] = "Noch nicht definiert %d",
		[apply.COSTUME_ATTR_BONUS] = "Noch nicht definiert %d",
		[apply.MAGIC_ATTBONUS_PER] = "Noch nicht definiert %d",
		[apply.MELEE_MAGIC_ATTBONUS_PER] = "Noch nicht definiert %d",
		[apply.RESIST_ICE] = "Noch nicht definiert %d",
		[apply.RESIST_EARTH] = "Noch nicht definiert %d",
		[apply.RESIST_DARK] = "Noch nicht definiert %d",
		[apply.ANTI_CRITICAL_PCT] = "Abwehr gegen kritischen Treffer +%d%%",
		[apply.ANTI_PENETRATE_PCT] = "Abwehr gegen durchbohrenden Treffer +%d%%",
		[apply.BOOST_CRITICAL_DMG] = "Noch nicht definiert %d",
		[apply.BOOST_PENETRATE_DMG] = "Noch nicht definiert %d",
		[apply.ATT_BONUS] = "%d%% Schadenserh?ung",
		[apply.DEF_BONUS] = "%d%% Schadensreduzierung",
	})[id]
	
	value = value or 0
	
	if not affectString then
		return string.format("%d: %d", id, value)
	end
	
	return string.format(affectString, value)
end


function get_time_remaining(seconds)
	if seconds == nil then
		return "ERROR NIL"
	end
	if seconds <= 60 then
		return string.format("%d sec", seconds)
	else
		local minutes = math.floor(seconds / 60)
		seconds = math.mod(seconds, 60)
		if minutes <= 60 then
			return string.format("%d min and %d sec", minutes, seconds)
		else
			local hours = math.floor(minutes / 60)
			minutes = math.mod(minutes, 60)
			if hours <= 24 then
				return string.format("%d hour(s) and %d min", hours, minutes)
			else
				local days = math.floor(hours / 24)
				hours = math.mod(hours, 24)
				return string.format("%d day(s) and %d hour(s)", days, hours)
			end
		end
	end
end


dofile("data/quest/QL_Libs/dungeons.lua");
--dofile("data/quest/QL_Libs/UnderwaterDungeonLIB.lua");


--If you dont have this function (from Syreldar's quest functions)
party_get_member_pids = function()
	local pids = {party.get_member_pids()};

	return pids;
end

--If you dont have this function (from Syreldar's quest functions)
table_shuffle = function(table_ex)
    for i = table.getn(table_ex), 2, -1 do
        local random_element = math.random(i);
        table_ex[i], table_ex[random_element] = table_ex[random_element], table_ex[i];
    end -- for
   
    return table_ex;
end -- function

DUNGEON_ID_NONE = 0
DUNGEON_ID_OWL = 1
DUNGEON_ID_GARDEN = 2
DUNGEON_ID_DEEP_CAVE = 3
DUNGEON_ID_NEVERLAND = 4
DUNGEON_ID_WOOD = 5
DUNGEON_ID_ICESTORM = 6
DUNGEON_ID_SHADOW_TOWER = 7
DUNDEON_ID_PYRA = 8
DUNGEON_ID_DEMON = 9
DUNGEON_ID_SLIME_1 = 10
DUNGEON_ID_SLIME_2 = 11
DUNGEON_ID_SLIME_3 = 12
DUNGEON_ID_SLIME_4 = 13


dofile("data/quest/dungeon/ShadowZone3/ShadowZoneLIB3.lua");
dofile("data/quest/dungeon/AncientPyramid/PyramidDungeonLIB.lua");
--dofile("data/quest/dungeon/AncientJungle/AncientJungleLIB.lua");
