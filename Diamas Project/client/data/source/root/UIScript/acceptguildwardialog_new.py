import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/newdungeon/"
IMG_DIR = "d:/ymir work/ui/game/gameoption/"

BOARD_WIDTH = 250
BOARD_HEIGTH = 300

window = {
	"name" : "InputDialog",
	'style': ('moveable', 'float', 'animated_board', ),
	"type": "window",
	"x" : 0,
	"y" : 0,

	"width" : BOARD_WIDTH,
	"height" : BOARD_HEIGTH,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : BOARD_WIDTH,
			"height" : BOARD_HEIGTH,

			"title" : uiScriptLocale.GUILD_WAR_ACCEPT,

			"children" :
			(
				## Input Slot
				{
					"name" : "InputName",
					"type" : "text",

					"x" : 15,
					"y" : 40,

					"text" : uiScriptLocale.GUILD_WAR_ENEMY,
				},
				{
					"name" : "InputSlot",
					"type" : "slotbar",

					"x" : 80,
					"y" : 37,
					"width" : 130,
					"height" : 18,

					"children" :
					(
						{
							"name" : "InputValue",
							"type" : "text",

							"x" : 3,
							"y" : 3,

							"width" : 90,
							"height" : 18,

							"input_limit" : 12,
						},
					),
				},
				## Input Slot
				{
					"name" : "GameType", "x" : 15, "y" : 65, "width" : 65+45*4, "height" : 20,
					
					"children" :
					(
						{"name" : "GameTypeLabel", "type" : "text", "x" : 0, "y" : 3, "text" : uiScriptLocale.GUILD_WAR_BATTLE_TYPE,},
						{
							"name" : "NormalButton",
							"type" : "radio_button",

							"x" : 65,
							"y" : 0,

							"text" : uiScriptLocale.GUILD_WAR_NORMAL,

							"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
							"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
							"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
						},
						{
							"name" : "WarpButton",
							"type" : "radio_button",

							"x" : 65+45*1,
							"y" : 0,

							"text" : uiScriptLocale.GUILD_WAR_WARP,
							
							"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
							"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
							"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
						},
						{
							"name" : "CTFButton",
							"type" : "radio_button",

							"x" : 65+45*2,
							"y" : 0,

							"text" : uiScriptLocale.GUILD_WAR_CTF,

							"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
							"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
							"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
						},
					),
				},
				## Input Slot
				{
					"name" : "InputNameScore",
					"type" : "text",

					"x" : 15,
					"y" : 95,

					"text" : "Maks. Skor",
				},
				{
					"name" : "InputSlotScore",
					"type" : "slotbar",

					"x" : 80,
					"y" : 92,
					"width" : 130,
					"height" : 18,

					"children" :
					(
						{
							"name" : "InputValueScore",
							"type" : "text",

							"x" : 3,
							"y" : 3,

							"width" : 90,
							"height" : 18,

							"input_limit" : 12,
						},
					),
				},
				## Input Slot
				{
					"name" : "InputNameUser",
					"type" : "text",

					"x" : 15,
					"y" : 125,

					"text" : "Maks. Oyuncu",
				},
				{
					"name" : "InputSlotUser",
					"type" : "slotbar",

					"x" : 80,
					"y" : 122,
					"width" : 130,
					"height" : 18,

					"children" :
					(
						{
							"name" : "InputValueUser",
							"type" : "text",

							"x" : 3,
							"y" : 3,

							"width" : 90,
							"height" : 18,

							"input_limit" : 12,
						},
					),
				},
				## Input Slot
				{
					"name" : "InputNameMinSeviye",
					"type" : "text",

					"x" : 15,
					"y" : 155,

					"text" : "Min. Seviye",
				},
				{
					"name" : "InputSlotMinSeviye",
					"type" : "slotbar",

					"x" : 80,
					"y" : 152,
					"width" : 130,
					"height" : 18,

					"children" :
					(
						{
							"name" : "InputValueMinSeviye",
							"type" : "text",

							"x" : 3,
							"y" : 3,

							"width" : 90,
							"height" : 18,

							"input_limit" : 12,
						},
					),
				},				
				{
					"name" : "titleimg",
					"type" : "image",
					"x" : 15+30,
					"y" : 185,
					"image" : ROOT_PATH + "title_bio.png",
					'children': 
					(	
						{
							'name': 'missionname',
							'type': 'text',
							'text': 'Engel Seçenekleri',
							'all_align': 'center',
							'x': 0,
							'y': 0,
						},
					),
				},
				{
					"name" : "block_mount",
					"type" : "toggle_button",
					"x" : 15,
					"y" : 195+20,
					# "text" : "Binek",
					"text_x" : 20,
					# "default_image" : ROOT_PATH + "checkbutton_empty.png",
					# "over_image" : ROOT_PATH + "checkbutton_empty_over.png",
					# "down_image" : ROOT_PATH + "checkbutton.png",
				},
				{
					"name" : "block_warrior",
					"type" : "toggle_button",
					"x" : 15,
					"y" : 195+20*2,
					"text" : "Savaşçı",
					"text_x" : 20,
					"default_image" : ROOT_PATH + "checkbutton_empty.png",
					"over_image" : ROOT_PATH + "checkbutton_empty_over.png",
					"down_image" : ROOT_PATH + "checkbutton.png",
				},
				{
					"name" : "block_assassin",
					"type" : "toggle_button",
					"x" : 15+65,
					"y" : 195+20*2,
					"text" : "Ninja",
					"text_x" : 20,
					"default_image" : ROOT_PATH + "checkbutton_empty.png",
					"over_image" : ROOT_PATH + "checkbutton_empty_over.png",
					"down_image" : ROOT_PATH + "checkbutton.png",
				},
				{
					"name" : "block_sura",
					"type" : "toggle_button",
					"x" : 15+55*2,
					"y" : 195+20*2,
					"text" : "Sura",
					"text_x" : 20,
					"default_image" : ROOT_PATH + "checkbutton_empty.png",
					"over_image" : ROOT_PATH + "checkbutton_empty_over.png",
					"down_image" : ROOT_PATH + "checkbutton.png",
				},
				{
					"name" : "block_shaman",
					"type" : "toggle_button",
					"x" : 15+55*3,
					"y" : 195+20*2,
					"text" : "Şaman",
					"text_x" : 20,
					"default_image" : ROOT_PATH + "checkbutton_empty.png",
					"over_image" : ROOT_PATH + "checkbutton_empty_over.png",
					"down_image" : ROOT_PATH + "checkbutton.png",
				},
				## Button
				{
					"name" : "AcceptButton",
					"type" : "button",

					"x" : - 61 - 5,
					"y" : 188+20*4,
					"horizontal_align" : "center",

					"text" : uiScriptLocale.OK,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
				{
					"name" : "CancelButton",
					"type" : "button",

					"x" : 5 + 70,
					"y" : 188+20*4,
					"horizontal_align" : "center",

					"text" : uiScriptLocale.CANCEL,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},			
			),
		},
	),
}