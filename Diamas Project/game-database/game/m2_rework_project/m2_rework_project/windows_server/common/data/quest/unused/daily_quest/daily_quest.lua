--[[ 

Questmanager for daily Quests.

By ProfEnte

]]

DAILYQUESTS = {

-- Configuration
  ['CONFIG'] = {
	['MINQUESTS'] = 3, -- Minimale Anzahl der verfuegbaren Quest's pro Tag
	['MAXQUESTS'] = 5, -- Maximale Anzahl der verfuegbaren Quest's pro Tag
	['ALLOWED_QUEST_TYPES'] = {'KILLQUEST','COLLECTQUEST'}, -- Welche Art von Quests werden verwendet
},

['QUEST'] = {

	['COLLECTQUEST'] = {

		['INDEX'] = nil, -- Do not change (DAILYQUESTS.QUEST.COLLECTQUEST.INDEX)

		['MISSIONS'] = {
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 30,
				},
				
				['QUESTNAME'] = "Das Sammeln der Bärentatzen [Part 1]!",
				['QUESTTEXT'] = "Sammle 20 tatzen! Doch bedenke, die Bären machen es dir nicht leicht![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Amulett der ew. Liebe (3h)[ENTER]- 20.000 Yang[ENTER]- 10.000 Erfahrung",
				
				['QUESTICON'] = 71145,
				
				['COLLECT'] = {
					['MOBVNUM'] = {111,112},{113}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Bärentatze", "Alte Bärentatze"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {10,10},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71186}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 20000,
					['EXP'] = 10000,
				},
			},
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 30,
				},
				
				['QUESTNAME'] = "Das Sammeln der Glocke [Part 1]!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 10x Gegenstand Verzaubern[ENTER]- 20.000 Yang[ENTER]- 10.000 Erfahrung",
				
				['QUESTICON'] = 71084,
				
				['COLLECT'] = {
					['MOBVNUM'] = {101,102},{103}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10},
						},
					['YANG'] = 20000,
					['EXP'] = 10000,
				},
			},
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 30,
				},
				
				['QUESTNAME'] = "Das Sammeln der Wolfshaut [Part 1]!",
				['QUESTTEXT'] = "Sammle 10 Wolfshaut![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 10x Muschel[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 27987,
				
				['COLLECT'] = {
					['MOBVNUM'] = {104,105},{106}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Wolfshaut", "Schwarze Wolfshaut"},
					['NEED'] = {10,10},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10,0},
						},
					['YANG'] = 20000,
					['EXP'] = 10000,
				},
			},
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 50,
				},
				
				['QUESTNAME'] = "Der Vegetarier hat Hunger!",
				['QUESTTEXT'] = "Sammle 25x Tofusteak und 3x Scharfer Pfeffer[ENTER]Drop von: Schwarzer Ork(-Riese)[ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 100x Gegenstand verzaubern[ENTER]- 10x Gegenstand verstaerken[ENTER]",
				
				['QUESTICON'] = 71084,
				
				['COLLECT'] = {
					['MOBVNUM'] = {636},{637}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Tofusteak", "Scharfer Pfeffer"},  -- Eher ein Pseudo Item, einfach den Namen des Items eingeben. Seele, Kuchenstueck.. whatever, man bekommt es nicht wirklich
					['NEED'] = {25,3},
					['DROPCHANCE'] = {15,5},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,71085}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {100,10},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
			
			-- NEUE D-QUEST PLATZ
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 45,
				},
				
				['QUESTNAME'] = "Forschungsstation Wüste!",
				['QUESTTEXT'] = "Sammle 20 Wüstensand![ENTER]Diesen erhälst du von:[ENTER]Wüstenflugauge[ENTER]Königsskorpion[ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Ebenholzarmband[ENTER]- 3x Diamant",
				
				['QUESTICON'] = 14100,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {2102,2103} }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Wüstensand"},
					['NEED'] = {20},
					['DROPCHANCE'] = {25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {14106,50621}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,3},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
			
		
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 45,
				},
				
				['QUESTNAME'] = "Forschungsstation Wüste [Kaptiel 2]",
				['QUESTTEXT'] = "Sammle 25 Säbel![ENTER]Drop bei: Wüstenbandit[ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Himmelstränenohrringe[ENTER]- 1x Blaue Perle",
				
				['QUESTICON'] = 17200,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {2108} }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Säbel"},
					['NEED'] = {25},
					['DROPCHANCE'] = {20},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {17207,27993}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 45,
				},
				
				['QUESTNAME'] = "Gesichtsbemalung",
				['QUESTTEXT'] = "Sammle 15 Gesichtsfarben von den Dunklen Arahanen[ENTER] und Dunklen Kämpfern[ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segenschriftrollen[ENTER]- 10 Muscheln[ENTER]- 300.000Yang",
				
				['QUESTICON'] = 25040,
				
				['COLLECT'] = {
					['MOBVNUM'] = {702,703}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Gesichtsfarbe"},
					['NEED'] = {15},
					['DROPCHANCE'] = {20},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {25040,27987}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10,10},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 65,
				},
				
				['QUESTNAME'] = "Nachschub für die Armee",
				['QUESTTEXT'] = "Sammle 25 Armschutz von den Baumfroschanführern[ENTER]Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Himmelstränenhalskette[ENTER]- 1x Blaue Perle[ENTER]",
				
				['QUESTICON'] = 16200,
				
				['COLLECT'] = {
					['MOBVNUM'] = {1302}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Armschutz"},
					['NEED'] = {25},
					['DROPCHANCE'] = {20},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {16200,27993}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,1},
						},
					['YANG'] = 3000000,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 35,
					['MAX'] = 54,
				},
				
				['QUESTNAME'] = "Forschungstation Orktal [Kapitel 1]",
				['QUESTTEXT'] = "Sammle 25 Backenzahn von den Schwarzen Orks[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Himmeltränenarmband+7",
				
				['QUESTICON'] = 14200,
				
				['COLLECT'] = {
					['MOBVNUM'] = {636}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Backenzahn"},
					['NEED'] = {25},
					['DROPCHANCE'] = {15},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {14207}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 45,
				},
				
				['QUESTNAME'] = "Forschungsstation Orktal [Kapitel 2]",
				['QUESTTEXT'] = "Sammle 10 Zauberstäbe von den Ork-Zauberern[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Elixier des Forschers[ENTER]-300.000 Yang",
				
				['QUESTICON'] = 76020,
				
				['COLLECT'] = {
					['MOBVNUM'] = {604,634,654}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Zauberstab"},
					['NEED'] = {10},
					['DROPCHANCE'] = {25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {39023}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {3},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			{
				['LEVEL'] = {
					['MIN'] = 20,
					['MAX'] = 35,
				},
				
				['QUESTNAME'] = "Forschungsstation Orktal [Kapitel 3]",
				['QUESTTEXT'] = "Sammle 10 Keulen von den Stolzen Orks[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Elixier des Forschers[ENTER]- 300.000Yang",
				
				['QUESTICON'] = 76020,
				
				['COLLECT'] = {
					['MOBVNUM'] = {631}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Keule"},
					['NEED'] = {10},
					['DROPCHANCE'] = {20},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {39023}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {3},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			
				{
				['LEVEL'] = {
					['MIN'] = 45,
					['MAX'] = 54,
				},
				
				['QUESTNAME'] = "Forschungsstation Eisland [Kapitel 1]",
				['QUESTTEXT'] = "Sammle 25 Proben von den Plagenmonstern[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 1x Weiße Perle[ENTER] - 1x Blaue Perle[ENTER] - 1x Rote Perle",
				
				['QUESTICON'] = 27992,
				
				['COLLECT'] = {
					['MOBVNUM'] = {902,903,904,905,906,907,931,932,933,934,935,936,937}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Probe"},
					['NEED'] = {25},
					['DROPCHANCE'] = {10},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27992,27993,27994}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,1,1},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 75,
				},
				
				['QUESTNAME'] = "Forschungsstation Eisland [Kapitel 2]",
				['QUESTTEXT'] = "Sammle 15 Eis von den Eisgolems[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 2x Drachengott-Angriff[ENTER] - 2x Drachengott-Verteidigung[ENTER] - 2x Drachengott-Leben[ENTER] - 2x Drachengott-Intelligenz",
				
				['QUESTICON'] = 71027,
				
				['COLLECT'] = {
					['MOBVNUM'] = {1107}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Eis"},
					['NEED'] = {15},
					['DROPCHANCE'] = {20},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {39017,39018,39019,39020}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {2,2,2,2},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 54,
				},
				
				['QUESTNAME'] = "Forschungsstation Wüste [Kapitel 3]",
				['QUESTTEXT'] = "Sammle 25x Gift von den Klauengiftspinnen[ENTER] und Soldatengiftspinnen im Spinnendungeon 1[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Phönixschuhe[ENTER] - 1x Seelenstein",
				
				['QUESTICON'] = 15200,
				
				['COLLECT'] = {
					['MOBVNUM'] = {2034,2035}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Gift"},
					['NEED'] = {25},
					['DROPCHANCE'] = {10},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {15200,50513}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 75,
				},
				
				['QUESTNAME'] = "Nachschub für die Armee [Kapitel 2]",
				['QUESTTEXT'] = "Sammle 15 Ogerpanzer von den Ogern[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 10x Elixier des Forschers",
				
				['QUESTICON'] = 76020,
				
				['COLLECT'] = {
					['MOBVNUM'] = {1601,1602,1603}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Ogerpanzer"},
					['NEED'] = {15},
					['DROPCHANCE'] = {20},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71035}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 65,
					['MAX'] = 85,
				},
				
				['QUESTNAME'] = "Nachschub für die Armee [Kapitel 3]",
				['QUESTTEXT'] = "Sammle 10 Äxte von den Axtkämpfern und Tausendkämpfern[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 5x Elixier des Forschers[ENTER] - 1x Weiße Perle",
				
				['QUESTICON'] = 76020,
				
				['COLLECT'] = {
					['MOBVNUM'] = {1402,1403}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Axt"},
					['NEED'] = {10},
					['DROPCHANCE'] = {33},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71035,27992}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {5,1},
						},
					['YANG'] = 300000,
					['EXP'] = 0,
				},
			},
			
			{
				['LEVEL'] = {
					['MIN'] = 10,
					['MAX'] = 20,
				},
				
				['QUESTNAME'] = "Forschungsstation Schlangenfeld [Kapitel 2]",
				['QUESTTEXT'] = "Sammle 10x Bärenfußhaut von den Bären[ENTER] im Schlangenfeld.[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 10x Muschel[ENTER] - 10x Switcher[ENTER] - 100.000Yang",
				
				['QUESTICON'] = 71084,
				
				['COLLECT'] = {
					['MOBVNUM'] = {139,140,141,142}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Bärenfußhaut"},
					['NEED'] = {10},
					['DROPCHANCE'] = {40},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,71084}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10,10},
						},
					['YANG'] = 100000,
					['EXP'] = 0,
				},
			},
			
			{
				['LEVEL'] = {
					['MIN'] = 5,
					['MAX'] = 15,
				},
				
				['QUESTNAME'] = "Forschungsstation Schlangenfeld[Kapitel 1]",
				['QUESTTEXT'] = "Sammle 10x Wolfspelz von den Wölfen[ENTER] im Schlangenfeld.[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 10x Muschel[ENTER] - 10x Switcher[ENTER] - 100.000Yang",
				
				['QUESTICON'] = 71084,
				
				['COLLECT'] = {
					['MOBVNUM'] = {131,132,133,134,135,136}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Wolfspelz"},
					['NEED'] = {10},
					['DROPCHANCE'] = {40},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,71084}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10,10},
						},
					['YANG'] = 100000,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 75,
				},
				
				['QUESTNAME'] = "Forschungsstation Schlangenfeld [Kapitel 3]",
				['QUESTTEXT'] = "Sammle 20x Stein von den Steinbeißern[ENTER] und Steingolems im Schlangenfeld.[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 4x Quarzsand[ENTER] - 4x Fingerknochen[ENTER] - 1x Seelenstein[ENTER] - 1x Blaue Perle",
				
				['QUESTICON'] = 30195,
				
				['COLLECT'] = {
					['MOBVNUM'] = {139,140,141,142}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Stein"},
					['NEED'] = {20},
					['DROPCHANCE'] = {15},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,71084}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10,10},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
		
			
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 85,
				},
				
				['QUESTNAME'] = "Forschungsstation Grotte [Kapitel 1]",
				['QUESTTEXT'] = "Sammle 30x Eis der Verdammnis.[ENTER] Dieses erhälst du von Unterwelt-Eisinsekten,[ENTER] Unterwelt-Eislöwe und Unterwelt-Eissplitter[ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 2x Drachengottverteidigung[ENTER] - 100.000.000 Erfahrung",
				
				['QUESTICON'] = 71028,
				
				['COLLECT'] = {
					['MOBVNUM'] = {1331,1333,1334}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Eis der Verdammnis"},
					['NEED'] = {30},
					['DROPCHANCE'] = {15},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {39020}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {2},
						},
					['YANG'] = 0,
					['EXP'] = 100000000,
				},
			},
		
		
			{
				['LEVEL'] = {
					['MIN'] = 80,
					['MAX'] = 90,
				},
				
				['QUESTNAME'] = "Forschungsstation Grotte [Kapitel 2]",
				['QUESTTEXT'] = "Sammle 20x Eisjuwel.[ENTER] Dieses erhälst du von Unterwelt-Eisgolem,[ENTER] Unterwelt-Yeti und Unterwelt-Eismann[ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 10x Elixier des Forschers[ENTER] - 2x Drachengott-Leben[ENTER] - 10x Kritischer Kampf[ENTER] - 10x Durchbohrender Kampf",
				
				['QUESTICON'] = 76020,
				
				['COLLECT'] = {
					['MOBVNUM'] = {1135,1136,1137}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Eisjuwel"},
					['NEED'] = {20},
					['DROPCHANCE'] = {15},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71035,71027,71044,71045}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10,2,10,10},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
		
			
			{
				['LEVEL'] = {
					['MIN'] = 90,
					['MAX'] = 99,
				},
				
				['QUESTNAME'] = "Forschungsstation Grotte [Kapitel 5]",
				['QUESTTEXT'] = "Sammle 25x Zinschwerter.[ENTER] Dieses erhälst du von Setau-Soldat[ENTER] und Setau-Kommandant. [ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 10x Elixier des Forschers - 2x Drachengott-Leben[ENTER] - 10x Kritischer Kampf - 10x Durchbohrender Kampf",
				
				['QUESTICON'] = 71044,
				
				['COLLECT'] = {
					['MOBVNUM'] = {2411,2414}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Zinschwert"},
					['NEED'] = {20},
					['DROPCHANCE'] = {10},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71035,71027,71044,71045}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {10,2,10,10},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 80,
					['MAX'] = 90,
				},
				
				['QUESTNAME'] = "Forschungsstation Grotte [Kapitel 4]",
				['QUESTTEXT'] = "Sammle 25x Zindolche.[ENTER] Dieses erhälst du von Setau-Kämpfern[ENTER] und Setau-Bogenschützen. [ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 2x Erfahrungsring[ENTER] - 10x Fingerknochen[ENTER] - 150.000.000 Erfahrung",
				
				['QUESTICON'] = 70005,
				
				['COLLECT'] = {
					['MOBVNUM'] = {2401,2402}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Zindolch"},
					['NEED'] = {20},
					['DROPCHANCE'] = {7},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {70005,30193}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {2,10},
						},
					['YANG'] = 0,
					['EXP'] = 150000000,
				},
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 90,
					['MAX'] = 99,
				},
				
				['QUESTNAME'] = "Forschungsstation Grotte [Kapitel 6]",
				['QUESTTEXT'] = "Sammle 10x Karte der Verdammnis.[ENTER] Diese erhälst du von General-Yonghan. [ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- 1x Magischer Stein - 1x Weiße Perle[ENTER] - 1x Blaue Perle - 1x Rote Perle[ENTER] - 25x Segen des Drachen",
				
				['QUESTICON'] = 51505,
				
				['COLLECT'] = {
					['MOBVNUM'] = {2492}, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Karte der Verdammnis"},
					['NEED'] = {10},
					['DROPCHANCE'] = {15},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {25041,27992,27993,27994,39013}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {2,1,1,1,25},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
			},
			
		
			
		},
	},

	['KILLQUEST'] = {

		['INDEX'] = nil, -- Do not change (DAILYQUESTS.QUEST.KILLQUEST.INDEX)

		['MISSIONS'] = {
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 55,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 2]",
				['QUESTTEXT'] = "Toete 100x Metin des Schattens[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {8009},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181}, -- Wenn kein Item dann {0}
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ring der Freude[ENTER]"
			},
			-- ##Low Metinstein Quest 1 anfang##
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 30,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1]",
				['QUESTTEXT'] = "Toete 25x Metin der Schlacht und 15x Metin der Schwaerze[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 76023,
				
				['MOBVNUM'] = {8003,8005},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {25,15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76023,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {40,10},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 40x Grüner Zauber[ENTER]- 10x Truhe der Fortbildung[ENTER]"
			},
			-- ##Low Metinstein Quest 2 anfang##
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 30,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.1]",
				['QUESTTEXT'] = "Toete 15x Metin des Kampfs und 20x Metin der Gier[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {8002,8004},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {15,20},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,76023}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {25,25},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 25x Gegenstand Verzaubern[ENTER]- 25xGruener Zauber (Gegenstand verzaubern bis Level 40)[ENTER] "
			},
			-- ##Low Metinstein Quest 2 ende##
			-- ##DT Quest 1 start##
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 85,
				},
				['QUESTNAME'] = "Die bedrohliche Situation des Daemonenturmes [Kapitel 1]",
				['QUESTTEXT'] = "Toete 200x Gemeiner Soldat und 100x Gemeiner Bogenschuetze[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {1031,1032},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {200,100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,100},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Seelenstein[ENTER]- 100x Gegenstand Verzaubern!"
			},
			-- ##DT Quest 1 ende##
			-- ##Grotte Quest 1 Start##
						{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 105,
				},
				['QUESTNAME'] = "Die Grotte ruft! [Kapitel 1]",
				['QUESTTEXT'] = "Toete 200x Unterwelt-Yetis sowie 150x Unterwelt-Eisgolems[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {1136,1137},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {200,150},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {200,200},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 400x Gegenstand Verzaubern[ENTER]"
			},
			-- ##Grotte Quest 1 ende##
			-- ##Grotte Quest 2 Start##
									{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 105,
				},
				['QUESTNAME'] = "Die Grotte ruft! [Kapitel 2]",
				['QUESTTEXT'] = "Toete 300x Setaou-Kaempfer, 250x Setaou-Jaeger, 100 Setaou-Seherinnen sowie 75x Unterwelt-Eisgolems[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 53008,
				
				['MOBVNUM'] = {2401,2402,2403,1137},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {300,250,100,75},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {53012,39013}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,10},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Bao-Bao Siegel[ENTER]- 10x Segen des Drachen [ENTER] "
			},
			-- Grotte Quest 2 Ende
			-- ##DT Killquest 2 Anfang##
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 85,
				},
				['QUESTNAME'] = "Die bedrohliche Situation des Daemonenturmes [Kapitel 2]",
				['QUESTTEXT'] = "Toete 200x Gemeiner Geist Soldat und 100x Gemeiner Geist Schuetze sowie 20x Gemeine Geist Schamanen[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 30195,
				
				['MOBVNUM'] = {1061,1062,1064},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {200,100,20},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {30195,27987}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {3,25},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 3x Quarzsand[ENTER]- 25x Muschel[ENTER] "
			},
			-- DT Killquest 2 Ende
			-- ###################
			-- Low Metinstein Killquest Start			
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 30,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.2]",
				['QUESTTEXT'] = "Toete 15x Metin des Kummers und 15x Metin der Gier[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 17100,
				
				['MOBVNUM'] = {8001,8004},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {15,15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,17104}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {30,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 30x Gegenstand Verzaubern[ENTER]- Ein paar Ebenholzohrringe+4[ENTER] "
			},
			-- Low Metinstein Killquest Ende
			-- 40-55 Metins Killquest Start			
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 60,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3]",
				['QUESTTEXT'] = "Toete 50x Metin des Teufels [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27992,
				
				['MOBVNUM'] = {8011},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {50},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27992,27993}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Eine weiße Perle[ENTER]- Eine Blaue Perle[ENTER] "
			},
			-- 40-55 Metins Killquest ende
			
						-- 60-80 Metins Killquest Start			
			{
				['LEVEL'] = {
					['MIN'] = 60,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.1]",
				['QUESTTEXT'] = "Toete 70x Metin des Mordes[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {8014},-- Wieviel kills von den Viechern?
				['TOKILL'] = {70},-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181,27987}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,30},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Ring der Freude(3h)[ENTER]- 30x Muscheln (Diese koennten Perlen enthalten!)[ENTER] "
			},
			-- 60-80 Metins Killquest ende
			
			-- RW Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 60,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.2]",
				['QUESTTEXT'] = "Toete 30x Metin Tu-Young sowie 30x Metin Jeon-Un[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 76020,
				
				['MOBVNUM'] = {8026,8027},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {30,30},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71035,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {10,150},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 10x Elixier des Forschers[ENTER]- 150x Gegenstand Verzaubern[ENTER]"
			},
			-- RW Metinstein Killquest Ende
			-- Die Affen rasten aus
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 60,
				},
				['QUESTNAME'] = "Entdecke die Affendungeons!!",
				['QUESTTEXT'] = "Toete jeweils 50x Starker Affensoldat, Starker Affenwerfer, Starker Affenkaempfer, Starker Affengeneral, Starker Steinaffe, Starker Goldaffe sowie 1x den Affen-Lord[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {5121,5122,5123,5124,5125,5126,5163},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {50,50,50,50,50,50,1},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 750000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Seelenstein[ENTER]- 750000 Yang!![ENTER]"
			},
			-- Affen ende
			-- Sensikill
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 105,
				},
				['QUESTNAME'] = "Der Wächter des Dämonenturmes!",
				['QUESTTEXT'] = "Der beruechtigte Sensenmann macht Aerger! Bringe ihn 2x um![ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = boss_box,
				
				['MOBVNUM'] = {1093},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {2},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71178}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 1250000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Sensenmanntruhe[ENTER]- 1250000 Yang!![ENTER]"
			},
			-- Sensikill
			
			-- Eishexekill
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 105,
				},
				['QUESTNAME'] = "Die dunkelblaue Große Hexe!",
				['QUESTTEXT'] = "Die Eishexe muss vernichtet werden! Bitte vernichte Sie[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {1192},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {1},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {100},
						},
					['YANG'] = 5000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 100x Gegenstand Verzaubern[ENTER]- 5000000 Yang!![ENTER]"
			},
			-- Eishexekill
			-- Beransetaukill
			{
				['LEVEL'] = {
					['MIN'] = 90,
					['MAX'] = 99,
				},
				['QUESTNAME'] = "Der legendäre Drache Bera-Setaou!",
				['QUESTTEXT'] = "Beran-Setau versucht die Macht an sich zu reissen! Du musst ihn stoppen! Toete ihn[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {2493},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {1},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 12500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ring der Freude[ENTER]- 12500000 Yang!![ENTER]"
			},
			-- Beransetaukill
			
			-- razadorkill
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Der König der Dämonen!!!",
				['QUESTTEXT'] = "Du musst die Dämonenkönige Es gibt keinen Ausweg! Vernichte sie, [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 30193,
				
				['MOBVNUM'] = {1092,1091},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {2,2},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {30193,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {6,0},
						},
					['YANG'] = 500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 6x Fingerknochen[ENTER]- 500000 Yang!![ENTER]"
			},
			-- razadorkill
			
			-- tigergeist
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Der Tigergeist aus dem Tempel [Kapitel 2]",
				['QUESTTEXT'] = "Es gibt nicht viel zu sagen, jedoch muss der Gelbe Tigergeist aus dem Tempel getoetet werden, [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27993,
				
				['MOBVNUM'] = {1304},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {3},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27993,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,0},
						},
					['YANG'] = 500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Blaue Perle[ENTER]- 500000 Yang!![ENTER]"
			},
			-- tigergeist
			
			-- tigergeist2
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Der Tigergeist aus dem Tempel [Kapitel 1]",
				['QUESTTEXT'] = "Es gibt nicht viel zu sagen, jedoch muss der Gelbe Tigergeist aus dem Tempel getoetet werden, [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27992,
				
				['MOBVNUM'] = {1304},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {3},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27992,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,0},
						},
					['YANG'] = 500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Weiße Perle[ENTER]- 500000 Yang!![ENTER]"
			},
			-- tigergeist2
			
			-- tigergeist3
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Der Tigergeist aus dem Tempel [Kapitel 3]",
				['QUESTTEXT'] = "Es gibt nicht viel zu sagen, jedoch muss der Gelbe Tigergeist aus dem Tempel getoetet werden, [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27994,
				
				['MOBVNUM'] = {1304},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {3},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27994,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,0},
						},
					['YANG'] = 500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Blutrote Perle[ENTER]- 500000 Yang!![ENTER]"
			},
			-- tigergeist3
			
			-- Spinnenkill
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 65,
				},
				['QUESTNAME'] = "Alles voller Spinnweben!",
				['QUESTTEXT'] = "Aaah, mein Gesicht ist verklebt! Du musst die Spinnenköniginen erledigen[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {2091,0},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {8,0},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {50,0},
						},
					['YANG'] = 500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 50x Gegentand verzaubern[ENTER]- 500000 Yang!![ENTER]"
			},
			-- Spinnenkill
			-- Hauptmann map2 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 40,
				},
				['QUESTNAME'] = "Der brutale Hauptmann von Map2!",
				['QUESTTEXT'] = "Töte 10 brutale Hauptmänner [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {591,0},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {10,0},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {50,0},
						},
					['YANG'] = 250000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 50x Gegenstand Verzaubern[ENTER]- 250000 Yang!![ENTER]"
			},
			-- Hauptmann map2 stop
			
			-- oberork start
			{
				['LEVEL'] = {
					['MIN'] = 35,
					['MAX'] = 55,
				},
				['QUESTNAME'] = "Der Oberork randaliert!",
				['QUESTTEXT'] = "Die verrückten Oberorks randalieren! tötet sie [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {691,0},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5,0},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {50,0},
						},
					['YANG'] = 100000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 50x Gegenstand verzaubern[ENTER]"
			},
			-- oberork stop
			
						-- Hauptmann map2 stop
			
			-- neunschwanz start
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Das Monster mit 9 Schwänzen",
				['QUESTTEXT'] = "Töte die Neunschwäne [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27992,
				
				['MOBVNUM'] = {1901,0},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {4,0},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27992,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,10},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Weiße Perle![ENTER]- 10x Truhe der Fortbildung![ENTER]"
			},
			-- neunschwanz stop
			
			
			-- tiergottheiten start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 35,
				},
				['QUESTNAME'] = "Die Tiergottheiten",
				['QUESTTEXT'] = "Töte Lykos, Scrofa, Bera, Tigris [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {191,192,193,194},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {1,1,1,1},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {30,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 30x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- tiergottheiten stop
			
			
			-- RW Metinstein 2 Killquest Ende
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.0]",
				['QUESTTEXT'] = "Toete 25x Metin Pung-Ma und 35x Metin Ma-An[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text

				['QUESTICON'] = 53008,
				
				['MOBVNUM'] = {8024,8025},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {25,35},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {53012,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,2},
						},
					['YANG'] = 10000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Bao-Bao Siegel![ENTER]"
			},
			
			-- NEUE D-QUEST PLATZ
			
			
			-- Low Metinstein Killquest 1 Start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 10,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.2]",
				['QUESTTEXT'] = "Toete 15x Metin des Kummers um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 71084,
		
				['MOBVNUM'] = {8001},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {25040,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein paar Segenschriftrollen [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 1 Ende
			
			
			-- Low Metinstein Killquest 2 Start
				
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.3]",
				['QUESTTEXT'] = "Toete 15x Metin des Kampfes [ENTER]um diese Quest abzuschliessen![Enter]Viel Glueck dabei!", --[ENTER]	
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {8002},
				-- Wieviele Kills von den Viechern?
				['TOKILL'] = {15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {20540,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Segenschriftrollen [ENTER] - Ein paar Gegenstand verzaubern[ENTER]Viel Glueck dabei!", --[ENTER]
			},
			-- Low Metinstein Killquest 2 Ende
			
			
			-- Low Metinstein Killquest 3 Start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.4]",
				['QUESTTEXT'] = "Toete 15x Metin des Schlacht um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 71084,
		
				['MOBVNUM'] = {8003},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {25040,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein paar Segenschriftrollen [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 3 Ende
			
			
			-- Low Metinstein Killquest 4 Start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.5]",
				['QUESTTEXT'] = "Toete 15x Metin der Gier um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 71084,
		
				['MOBVNUM'] = {8004},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {25040,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein paar Segenschriftrollen [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 4 Ende	

			
			-- Low Metinstein Killquest 5 Start
			{
				['LEVEL'] = {
					['MIN'] = 20,
					['MAX'] = 35,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.6]",
				['QUESTTEXT'] = "Toete 15x Metin der Schwaerze um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 17100,
		
				['MOBVNUM'] = {8005},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {25},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50323,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {50,25},
						},
					['YANG'] = 100000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein paar Segenschriftrollen [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 5 Ende
			
			
			-- Low Metinstein Killquest 6 Start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.7]",
				['QUESTTEXT'] = "Toete 15x Metin der Schlacht, 15x Metin der Gier, 15x Metin der Schwärze[ENTER] um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 17100,
		
				['MOBVNUM'] = {8003,8004,8005},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {15,15,15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {17104,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {30,30},
						},
					['YANG'] = 100000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein paar Segenschriftrollen [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 6 Ende
			
			
},			-- Low Metinstein Killquest 7 Start
			{
				['LEVEL'] = {
					['MIN'] = 20,
					['MAX'] = 35,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.2]",
				['QUESTTEXT'] = "Toete 25x Metin der Dunkelheit um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 71084,
		
				['MOBVNUM'] = {8006},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {25},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50323,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {50,25},
						},
					['YANG'] = 100000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein paar Fertigkeitsbücher [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 7 Ende
			
			
			-- Low Metinstein Killquest 8 Start
			{
				['LEVEL'] = {
					['MIN'] = 20,
					['MAX'] = 35,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 1.8]",
				['QUESTTEXT'] = "Toete 25x Metin der Eifersucht um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = book_01,
		
				['MOBVNUM'] = {8007},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {25},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50323,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {50,25},
						},
					['YANG'] = 100000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein paar Fertigkeitsbücher [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 8 Ende
			
			
		-- 40-55 Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 40,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3.1]",
				['QUESTTEXT'] = "Toete 30x Metin der Seele um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 50513,
		
				['MOBVNUM'] = {8008},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {30},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,35},
						},
					['YANG'] = 150000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Einen Seelenstein [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- 40-55 Metinstein Killquest Ende
			
			
			-- Low Metinstein Killquest 9 Start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 40,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3.2]",
				['QUESTTEXT'] = "Toete 30x Metin der Dunkelheit und 30x Metin der Eifersucht[ENTER] um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 50513,
		
				['MOBVNUM'] = {8006,8007},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {30,30},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,35},
						},
					['YANG'] = 150000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Einen Seelenstein [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- Low Metinstein Killquest 9 Ende
			
			
			-- 40-55 Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 30,
					['MAX'] = 40,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3.3]",
				['QUESTTEXT'] = "Toete 30x Metin der Seele um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 50513,
		
				['MOBVNUM'] = {8008},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {30},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,35},
						},
					['YANG'] = 150000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Einen Seelenstein [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- 40-55 Metinstein Killquest Ende
			
			
			-- 40-55 Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 50,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3.4]",
				['QUESTTEXT'] = "Toete 100x Metin des Schattens um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 53008,
		
				['MOBVNUM'] = {8009},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {53012,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,100},
						},
					['YANG'] = 250000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Bao-Bao-Siegel [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- 40-55 Metinstein Killquest Ende
			
			
			-- 40-55 Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 50,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3.5]",
				['QUESTTEXT'] = "Toete 20x Metin der Seele, 20x Metin des Schattens [ENTER] 20x Metin der Haerte um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 13040,
		
				['MOBVNUM'] = {8008,8009,8010},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {20,20,20},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {13047,27993}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,1},
						},
					['YANG'] = 250000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Einen Seelenstein [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- 40-55 Metinstein Killquest Ende
			
			
			-- 40-55 Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 60,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3.6]",
				['QUESTTEXT'] ="Toete 40x Metin der Haerte um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 27992,
		
				['MOBVNUM'] = {8010},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {40},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27992}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 150000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Eine Weiße Perle [ENTER]"
			},
			-- 40-55 Metinstein Killquest Ende
			
			
			-- 40-55 Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 60,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 3.7]",
				['QUESTTEXT'] = "Toete 100x Metin des Teufels um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 53008,
		
				['MOBVNUM'] = {8011},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {53012,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,50},
						},
					['YANG'] = 350000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Bao-Bao-Siegel [ENTER] - Ein paar Gegenstand verzaubern[ENTER]"
			},
			-- 40-55 Metinstein Killquest Ende
			
			
			-- 55-65 Metinstein Killquest Start
			{
				['LEVEL'] = {
					['MIN'] = 40,
					['MAX'] = 65,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 9]",
				['QUESTTEXT'] = "Toete 60x Metin des Teufels und 60x Metin des Todes[ENTER] um diese Quest abzuschliessen! [ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeilen im Text
			
				['QUESTICON'] = 50513,
		
				['MOBVNUM'] = {8011,8013},
				-- Wieviele kills von den Viechern?
				['TOKILL'] = {60,60},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,30},
						},
					['YANG'] = 500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Einen Seelenstein [ENTER] - Ein paar Truhen der Fortbildung[ENTER]"
			},
			-- 55-65 Metinstein Killquest Ende
			
			
			-- 60-80 Metins Killquest 2 Start			
			{
				['LEVEL'] = {
					['MIN'] = 60,
					['MAX'] = 75,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.1]",
				['QUESTTEXT'] = "Toete 30x Metin des Todes[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71124,
				
				['MOBVNUM'] = {8013},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {30},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71188,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,100},
						},
					['YANG'] = 750000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Einen Weißen Löwen[ENTER]- 100x Gegenstand verzaubern[ENTER] "
			},
			-- 60-80 Metins Killquest 2 ende
			
			
			-- 60-80 Metins Killquest 3 Start			
			{
				['LEVEL'] = {
					['MIN'] = 60,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.1]",
				['QUESTTEXT'] = "Toete 100x Metin des Mordes[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {8014},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181,71025}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,5},
						},
					['YANG'] = 750000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Ring der Freude[ENTER]- Ein paar Stein des Schmiedes[ENTER] "
			},
			-- 60-80 Metins Killquest 3 ende
			
			
			-- 60-80 Metins Killquest 4 Start			
			{
				['LEVEL'] = {
					['MIN'] = 60,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.1]",
				['QUESTTEXT'] = "Toete 35x Metin Pung-Ma und 35x Metin Ma-An[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {8024,8025},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {35,35},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,100},
						},
					['YANG'] = 1000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Ring der Freude[ENTER]- 100x Gegenstand veruaubern [ENTER] "
			},
			-- 60-80 Metins Killquest 4 ende
			
			
			-- RW Metinstein Killquest 2 Start
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 90,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.2]",
				['QUESTTEXT'] = "Toete 100x Metin Tu-Young sowie [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {8026},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71035,71025,50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10,1},
						},
					['YANG'] = 1500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Elixier des Forschers[ENTER]- 10x Stein des Schmiedes[ENTER]- 1x Seelenstein"
			},
			-- RW Metinstein Killquest 2 Ende
			
			
			-- RW Metinstein Killquest 3 Start
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 99,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.3]",
				['QUESTTEXT'] = "Toete 100x Metin Jeon-Un[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27992,
				
				['MOBVNUM'] = {8027},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27992,27993,27994}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,1,1},
						},
					['YANG'] = 1500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Weiße Perle[ENTER]- 1x Blaue Perle[ENTER]- 1x Rote Perle"
			},
			-- RW Metinstein Killquest 3 Ende
			
			
			-- RW Metinstein Killquest 4 Start
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 99,
				},
				['QUESTNAME'] = "Die Metinsteine bedrohen uns! [Kapitel 4.2]",
				['QUESTTEXT'] = "Toete 50x Metin Tu-Young sowie 50x Metin Jeon-Un[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 53008,
				
				['MOBVNUM'] = {8026,8027},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {50,50},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {53012,71025}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,10},
						},
					['YANG'] = 2000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Bao-Bao-Siegel[ENTER]- 10x Stein des Schmieds[ENTER]"
			},
			-- RW Metinstein Killquest 4 Ende
			
			-- Tiergottheiten start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Tiergottheiten [Kapitel 1]",
				['QUESTTEXT'] = "Töte 5x Lykos [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27987,
				
				['MOBVNUM'] = {191},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Tiergottheiten stop
			
			
			-- Tiergottheiten 2 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Tiergottheiten [Kapitel 1.1]",
				['QUESTTEXT'] = "Töte 5x Bera [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27987,
				
				['MOBVNUM'] = {193},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Tiergottheiten 2	stop
			
			
			-- Tiergottheiten 3 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Tiergottheiten [Kapitel 1.2]",
				['QUESTTEXT'] = "Töte 5x Scrofa [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27987,
				
				['MOBVNUM'] = {192},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Tiergottheiten 3	stop
			
			
			-- Tiergottheiten 4 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Die Tiergottheiten [Kapitel 1.3]",
				['QUESTTEXT'] = "Töte 5x Tigris [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27987,
				
				['MOBVNUM'] = {194},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Tiergottheiten 4	stop
			
			
			-- Map1-Map2 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Rette unsere Staedte [Kapitel 1]",
				['QUESTTEXT'] = "Töte 5x Jin-Hee [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27987,
				
				['MOBVNUM'] = {394},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Muschel [ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Map1-Map2 stop
			
			
			-- Map1-Map2 2 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 20,
				},
				['QUESTNAME'] = "Rette unsere Staedte [Kapitel 1.1]",
				['QUESTTEXT'] = "Töte 5x Se-Rang [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27987,
				
				['MOBVNUM'] = {393},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Muschel [ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Map1-Map2 2 stop
			
			
			-- Map1-Map2 3 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 35,
				},
				['QUESTNAME'] = "Rette unsere Staedte [Kapitel 1.2]",
				['QUESTTEXT'] = "Töte 5x Bo [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {492},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {15,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 15x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Map1-Map2 3 stop
			
			
			-- Map1-Map2 4 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 35,
				},
				['QUESTNAME'] = "Rette unsere Staedte [Kapitel 1.3]",
				['QUESTTEXT'] = "Töte 5x Goo-Pae [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {493},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {15,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 15x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Map1-Map2 4 stop
			
			
			-- Map1-Map2 5 start
			{
				['LEVEL'] = {
					['MIN'] = 1,
					['MAX'] = 35,
				},
				['QUESTNAME'] = "Rette unsere Staedte [Kapitel 1.4]",
				['QUESTTEXT'] = "Töte 5x Mahon [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71084,
				
				['MOBVNUM'] = {493},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71084,50127}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {15,10},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 15x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Map1-Map2 5 stop
			
			
			-- Map1-Map2 6 start
			{
				['LEVEL'] = {
					['MIN'] = 20,
					['MAX'] = 40,
				},
				['QUESTNAME'] = "Rette unsere Staedte [Kapitel 1.5]",
				['QUESTTEXT'] = "Töte 10x Brutaler Hauptmann [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {591},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {10},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 50000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Gegenstand Verzaubern[ENTER]- 10x Truhe der Fortbildung sowie 50.000 Yang!![ENTER]"
			},
			-- Map1-Map2 6 stop
			
			
			-- Roter Wald 2 start
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 90,
				},
				['QUESTNAME'] = "Die Bedrohung aus dem Wald",
				['QUESTTEXT'] = "Toete 500x Rote Geisterweide[ENTER] und 250x Roter Geisterbaum [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 70005,
				
				['MOBVNUM'] = {2314,2311},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {500,250},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {70005}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 2000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Erfahrungsring[ENTER]- 2.000.000 Yang!![ENTER]"
			},
			-- Roter Wald 2 stop
			
			
			-- Land der Riesen - Schlangenfeld start
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 85,
				},
				['QUESTNAME'] = "Der Angriff der Riesen[Kapitel 2]",
				['QUESTTEXT'] = "Toete 100x Tausendkämpfer[ENTER] um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27992,
				
				['MOBVNUM'] = {1403},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27992,50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Weiße Perle[ENTER] 1x Seelenstein[ENTER]"
			},
			-- Land der Riesen - Schlangenfeld stop
			
			
			-- Land der Riesen - Schlangenfeld 2 start
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 85,
				},
				['QUESTNAME'] = "Der Angriff der Riesen[Kapitel 1]",
				['QUESTTEXT'] = "Toete 150x Steinbeisser,[ENTER] 150x Riesiger Felsgolem[ENTER] und 150x Steingolem [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 30195,
				
				['MOBVNUM'] = {1502,1503,1501},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {150,150,150},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {30195,27992,27993,27994}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,1,1,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Quarzsand[ENTER] -1x Weiße Perle[ENTER] -1x Blaue Perle[ENTER] -1 Rote Perle[ENTER]"
			},
			-- Land der Riesen - Schlangenfeld 2 stop
			
			
			-- Geisterwald 2 start
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 65,
				},
				['QUESTNAME'] = "Dezimierung der Geisterbäume",
				['QUESTTEXT'] = "Toete 500x Geisterbäume[ENTER] um der Verbreitung einzudämmen [ENTER]und diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {2301},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {500},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 2000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Seelenstein[ENTER]- 2.000.000 Yang!![ENTER]"
			},
			-- Geisterwald 2 stop
			
			
			-- Roter Wald 2 start
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 90,
				},
				['QUESTNAME'] = "Ausbreitung des Feuers",
				['QUESTTEXT'] = "Toete 250x Flamme[ENTER] und 250x Flammenkrieger [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50633,
				
				['MOBVNUM'] = {2204,2205},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {250,250},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50633,50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {2,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 2x  Himmelsträne[ENTER]- 1x Seelenstein![ENTER]"
			},
			-- Roter Wald 2 stop
			
			
			-- Dämonenturm 2 start
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 75,
				},
				['QUESTNAME'] = "Dämonenanführer",
				['QUESTTEXT'] = "Toete 5x den Stolzen Dämonenkönig,[ENTER] 5x den Dämonenkönig[ENTER] und 2x den Sensenmann [ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {1091,1092,1093},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {5,5,2},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {25041, 50513, 27992}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,1,1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Magischen Stein[ENTER]- 1x Seelenstein[ENTER] -1x Weiße Perle[ENTER]"
			},
			-- Dämonenturm 2 stop
			
			
			-- Tempel start
			{
				['LEVEL'] = {
					['MIN'] = 45,
					['MAX'] = 75,
				},
				['QUESTNAME'] = "Buhmann",
				['QUESTTEXT'] = "Toete 100x den Buhmann,[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27987,
				
				['MOBVNUM'] = {1303},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27987}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {25},
						},
					['YANG'] = 200000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 25x Muscheln[ENTER]- 200.000Yang[ENTER]"
			},
			-- Tempel stop
			
			
			-- Eisland 2 start
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 76,
				},
				['QUESTNAME'] = "Das Eisland übt den Aufstand!",
				['QUESTTEXT'] = "Toete 150x Frostiger Eismann[ENTER] und 100x Yeti[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {1105,1106},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {150,100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 200000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1Seelenstein[ENTER]- 200.000Yang[ENTER]"
			},
			-- Eisland 2 stop
			
			-- bosse 2 start
			{
				['LEVEL'] = {
					['MIN'] = 35,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Die kleinen Bosse!",
				['QUESTTEXT'] = "Töte 6x Oberork[ENTER] 6x Neunschwanz[ENTER] 6x Königinnenspinne[ENTER] 6x Flammenkönig[ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {691,1901,2091,2206},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {6,6,6,6},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,200},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1xRing der Freude[ENTER]- 200x Gegenstand Verzaubern[ENTER]"
			},
			-- bosse 2 stop
			
			
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 65,
				},
				['QUESTNAME'] = "Der Geisterwald ruft!",
				['QUESTTEXT'] = "Töte 150x Geisterbaumstumpf[ENTER] 100x Dryaden[ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 27994,
				
				['MOBVNUM'] = {2302,2303},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {150,100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {27994,27987}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,25},
						},
					['YANG'] = 200000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Rote Perle[ENTER]- 25x Muschel [ENTER]- 200.000Yang[ENTER]"
			},
			
			
			
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 75,
				},
				['QUESTNAME'] = "Alles brennt!",
				['QUESTTEXT'] = "Töte 250x Flammengeister[ENTER] Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {2202},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {250},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513, 27987}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Seelenstein[ENTER]- 25x Muschel[ENTER]"
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 35,
					['MAX'] = 80,
				},
				['QUESTNAME'] = "Töte die Bosse",
				['QUESTTEXT'] = "Töte 10x Oberork[ENTER] 10x Neunschwanz[ENTER] 10x Königinnenspinne[ENTER] 10x Flammenkönig[ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {691,1901,2091,2206},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {10,10,10,10},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 20000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1xRing der Freude[ENTER]- 20.000.000Yang[ENTER]"
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 55,
					['MAX'] = 78,
				},
				['QUESTNAME'] = "Die Felsengolems bedrohen uns!",
				['QUESTTEXT'] = "Töte 100x Rieseiger Felsengolem [ENTER]Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {1503},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {100},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1},
						},
					['YANG'] = 1000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Seelenstein[ENTER]- 1.000.000Yang[ENTER]"
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 95,
					['MAX'] = 99,
				},
				['QUESTNAME'] = "Die Grotte ruft!",
				['QUESTTEXT'] = "Töte 250x Setaou-Magistra, 10x General Yonghan[ENTER] und 5x General Huashin [ENTER]Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 30179,
				
				['MOBVNUM'] = {2413,2492,2495},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {250,10,5},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {30179,39024,39025}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {3,5,5},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 3x Gewundener Schlüssen[ENTER] - 5x Kritischer Kampf[ENTER] - 5x Durchbohrender Kampf"
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 90,
				},
				['QUESTNAME'] = "Die Untoten Bosse!",
				['QUESTTEXT'] = "Töte 1x Azrael, 1x Sensenmann[ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71143,
				
				['MOBVNUM'] = {2598,1093},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {1,1},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {70005, 71181}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,1},
						},
					['YANG'] = 10000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 1x Ring der Freude[ENTER] - 1x Erfahrungsring[ENTER] - 10.000.000Yang"
			},
		
		
			{
				['LEVEL'] = {
					['MIN'] = 50,
					['MAX'] = 75,
				},
				['QUESTNAME'] = "Der Schmied-Wächter!",
				['QUESTTEXT'] = "Töte 15x Stolzer Dämonenkönig [ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71020,
				
				['MOBVNUM'] = {1092},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {15},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71051}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {800},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 800x Gegenstand verhexen"
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 85,
				},
				['QUESTNAME'] = "Töte die eisige Hexe!",
				['QUESTTEXT'] = "Töte 1x Große Eishexe.[ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 71020,
				
				['MOBVNUM'] = {1192},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {1},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71045,71044,71020}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {5,5,10},
						},
					['YANG'] = 1000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 5x Kritischer Kampf[ENTER] - 5x Durchbohrender Kampf[ENTER] - 10x Segen des Drachen"
			},
			
			
			{
				['LEVEL'] = {
					['MIN'] = 90,
					['MAX'] = 99,
				},
				['QUESTNAME'] = "Der unbezwingbare Drache!",
				['QUESTTEXT'] = "Töte 1x Beran-Setaou.[ENTER] Viel Glück dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 30228,
				
				['MOBVNUM'] = {2493},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {1},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71027,71028,71029,71030,71084}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {10,10,10,10,200},
						},
					['YANG'] = 1000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 10x Drachengott-Angriff[ENTER] - 10x Drachengott-Verteidigung[ENTER]  - 10x Drachengott-Leben[ENTER] - 10x Drachengott-Intelligenz[ENTER] - 200x Gegenstand verzaubern"
			},
			
		}
			
		}	
			
	}		
			
			
			
			
--[[ MAXQUESTS 


LOAD
for i = 1, table.getn(MAXQUESTS) do
	pc.getf('daily_quest','quest_'..i)
end

SET
for i = 1, table.getn(MAXQUESTS) do
	pc.setf('daily_quest','quest_'..i,0)
end

CLEAR
for i = 1, table.getn(MAXQUESTS) do
	pc.setf('daily_quest','quest_'..i,1)
end
]]

dquest = {}

function dquest.clear()
	cmdchat('DAILYQUEST CLEAR_QUEST')
end

function dquest.refresh()
	cmdchat('DAILYQUEST REFRESH_QUEST')
end

function dquest.reload_quests()
	dquest.clear()
	for i = 1, pc.getf('daily_quest','quest_count') do
		local questTypes = DAILYQUESTS.CONFIG.ALLOWED_QUEST_TYPES
		for x = 1, table.getn(questTypes) do
			local missions = DAILYQUESTS.QUEST[questTypes[x]]
			local questIndex = missions.INDEX
			local missionIndex = pc.getf('daily_quest','quest_'..i..'_'..questIndex)
			-- chat('quest '..i)
			if missionIndex != 0 then
				-- chat('questadd '..i..' name: '..missions.MISSIONS[missionIndex].QUESTNAME)
				dquest.add(i, missions.INDEX, string.gsub(missions.MISSIONS[missionIndex].QUESTNAME,' ','_'),  missions.MISSIONS[missionIndex].QUESTICON)
			end
		end
	end
	dquest.refresh()
end

function dquest.add(questIndex,questType,questName, questIcon)
	cmdchat('DAILYQUEST ADD_QUEST/'..questIndex..'/'..questType..'/'..questName..'/'..questIcon)
	-- chat('DAILYQUEST ADD_QUEST/'..questIndex..'/'..questType..'/'..questName..'/'..questIcon)
end

function dquest.get_current_mission(questType,questNr, questIndex)
	return DAILYQUESTS.QUEST[questType].MISSIONS[pc.getf('daily_quest','quest_'..questNr..'_'..questIndex)]
end

function dquest.set_quest_complete(questType,questNr, questIndex)
	local mission = pc.getf('daily_quest','quest_'..questNr..'_'..questIndex)
	if mission == 0 then return end
	pc.setf('daily_quest','quest_'..questNr..'_'..questIndex,0)
	-- chat('delete: daily_quest quest_'..questNr..'_'..questIndex)
	local varCount = {}
	
	if questType == 'KILLQUEST' then
		varCount = DAILYQUESTS.QUEST.KILLQUEST.MISSIONS[mission].MOBVNUM
	-- more varCount's
	elseif questType == 'COLLECTQUEST' then
		varCount = DAILYQUESTS.QUEST.COLLECTQUEST.MISSIONS[mission].COLLECT.ITEMNAME
	end
	
	for i = 1, table.getn(varCount) do
		pc.setf('daily_quest','quest_'..questNr..'_'..questIndex..'_var_'..varCount[i],0)
		-- chat('delete: daily_quest quest_'..questNr..'_'..questIndex..'_var_'..varCount[i])
	end
end