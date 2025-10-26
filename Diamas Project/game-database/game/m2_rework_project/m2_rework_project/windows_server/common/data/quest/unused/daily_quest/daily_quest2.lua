--[[ 

Questmanager for daily Quests.

By ProfEnte

]]

DAILYQUESTS = {

-- Configuration
  ['CONFIG'] = {
	['MINQUESTS'] = 50, -- Minimale Anzahl der verfuegbaren Quest's pro Tag
	['MAXQUESTS'] = 50, -- Maximale Anzahl der verfuegbaren Quest's pro Tag
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
				
				['QUESTNAME'] = "Das Sammeln der Glocke [Part 1]!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				
				['QUESTNAME'] = "Das Sammeln der Glocke [Part 2]!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Wolfshaut", "Schwarze Wolfshaut"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				
				['QUESTNAME'] = "Das Sammeln der Glocke [Part 4]!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				
				['QUESTNAME'] = "Das Sammeln der Glocke [Part 5]!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				
				['QUESTNAME'] = "Das Sammeln der Glocke [Part 6]!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				
				['QUESTNAME'] = "Das Sammeln der Glocke7!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				
				['QUESTNAME'] = "Das Sammeln der Glocke8!",
				['QUESTTEXT'] = "Sammle 20 Glocken![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Segensschriftrolle[ENTER]- 20000 Yang[ENTER]- 10000 Erfahrung",
				
				['QUESTICON'] = 90000,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {101,102}, {103}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Glocke", "Schwarze Glocke"},
					['NEED'] = {20,20},
					['DROPCHANCE'] = {25,25},
				},
				
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {76016,0}, -- Wenn kein Item dann {0} bei beidem
						['ITEMCOUNT'] = {1,0},
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
				['QUESTTEXT'] = "Sammle 25x Tofusteaks und 3 Scharfen Pfeffer[ENTER]Beachte jedoch, von Salat schrumpft der Bizeps!![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				['REWARDTEXT'] = "- Gegenstand verzaubern[ENTER]- Gegenstand verstaerken[ENTER]",
				
				['QUESTICON'] = 71084,
				
				['COLLECT'] = {
					['MOBVNUM'] = { {636,637}, {8009}, }, -- es koennen mehrere Mobs das "Item" droppen
					['ITEMNAME'] = {"Tofusteak", "Scharfer Pfeffer"},  -- Eher ein Pseudo Item, einfach den Namen des Items eingeben. Seele, Kuchenstueck.. whatever, man bekommt es nicht wirklich
					['NEED'] = {25,3},
					['DROPCHANCE'] = {5,15},
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
			-- HIER NEUE D-QUEST (SAMMEL) WENN DANN EINTRAGEN, IMMER AM ENDE
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
				['REWARDTEXT'] = "- Seelenstein[ENTER]- 100x Gegenstand Verzaubern![ENTER] "
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
				['REWARDTEXT'] = "- 400x Gegenstand Verzaubern[ENTER]-[ENTER] "
			},
			-- ##Grotte Quest 1 ende##
			-- ##Grotte Quest 2 Start##
									{
				['LEVEL'] = {
					['MIN'] = 75,
					['MAX'] = 105,
				},
				['QUESTNAME'] = "Die Grotte ruft! [Kapitel 2]",
				['QUESTTEXT'] = "Toete 300x Setaou-Kaempfer, 250x Setaou-Jaeger, 100 Setaou-Seherinnen sowie 75x Setaou-Anfuehrer[ENTER]um diese Quest abzuschliessen![ENTER]Viel Glueck dabei!", -- [ENTER] -> Neue Zeile im Text
				
				['QUESTICON'] = 53008,
				
				['MOBVNUM'] = {2401,2402,2403,2404},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {300,250,100,75},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {53012,71020}, -- Wenn kein Item dann 0
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
				
				['MOBVNUM'] = {8019},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {70},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71181,27987}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,30},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Ein Ring der Freude[ENTER]- 30x Muscheln (Diese koennten Perlen enthalten!)[ENTER] "
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
				
				['QUESTICON'] = 71035,
				
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
				
				['QUESTICON'] = 71178,
				
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
				
				['QUESTICON'] = 71136,
				
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
						['ITEMCOUNT'] = {12,0},
						},
					['YANG'] = 500000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- 12x Fingerknochen[ENTER]- 500000 Yang!![ENTER]"
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
				
				['QUESTICON'] = 50513,
				
				['MOBVNUM'] = {2091,0},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {8,0},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {50513,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,0},
						},
					['YANG'] = 5000000,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Seelenstein[ENTER]- 5000000 Yang!![ENTER]"
			},
			-- Spinnenkill
			-- Hauptmann map2 start
			{
				['LEVEL'] = {
					['MIN'] = 20,
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
				
				['QUESTICON'] = 71229,
				
				['MOBVNUM'] = {691,0},
				-- Wieviel kills von den Viechern?
				['TOKILL'] = {8,0},
				-- Belohnung
				['REWARD'] = {
					['ITEM'] = {
						['ITEMVNUM'] = {71229,0}, -- Wenn kein Item dann 0
						['ITEMCOUNT'] = {1,0},
						},
					['YANG'] = 0,
					['EXP'] = 0,
				},
				['REWARDTEXT'] = "- Schwarzer-Panther-Siegel[ENTER]"
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
					['MIN'] = 5,
					['MAX'] = 25,
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
			-- HIER NEUE D-QUEST (KILL) WENN DANN EINTRAGEN, IMMER AM ENDE
			
		},
	},
},

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