import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/public/"
glyphenstartindex = 1020

window = {
	"name" : "Guildstorage",

	"x" : SCREEN_WIDTH/2 - 300,
	"y" : 10,

	"style" : ("moveable", "float",),

	"width" : 600,
	"height" : 370,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width" : 600,
			"height" : 370,

			"children" :
			(
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : 0,

					"width" : 600 - 15,
					"title" : uiScriptLocale.GUILDSTORAGE_TITLE,

				},
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 25,
					"y" : 45,

					"start_index" : 0,
					"x_count" : 17,
					"y_count" : 6,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub"
				},
				{
					"name" : "glyphenbg",
					"type" : "image",
					"x" : 25+34,
					"y" : 247,


					"image" : "d:/ymir work/ui/guildstorage/glyphebox_bg.tga",
					"children" : 
					(
						{
							"name" : "glyphe1bg",
							"type" : "image",
							"x" : 14,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_1.tga",
						},
						{
							"name" : "glypenslot1",
							"type" : "slot",

							"x" : 14+3,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+0, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_01",
							"type" : "radio_button",

							"x" : 12,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_01,

							"children" :
							(
								{
									"name" : "Inventory_Tab_01_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "I",
								},
							),
						},




						{
							"name" : "glyphe2bg",
							"type" : "image",
							"x" : 14+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_2.tga",
						},
						{
							"name" : "glypenslot2",
							"type" : "slot",

							"x" : 14+3+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+1, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_02",
							"type" : "radio_button",

							"x" : 12+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_02,

							"children" :
							(
								{
									"name" : "Inventory_Tab_01_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "II",
								},
							),
						},



						{
							"name" : "glyphe3bg",
							"type" : "image",
							"x" : 14+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_3.tga",
						},
						{
							"name" : "glypenslot3",
							"type" : "slot",

							"x" : 14+3+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+2, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_03",
							"type" : "radio_button",

							"x" : 12+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_03,

							"children" :
							(
								{
									"name" : "Inventory_Tab_03_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "III",
								},
							),
						},

						{
							"name" : "glyphe4bg",
							"type" : "image",
							"x" : 14+46+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_4.tga",
						},
						{
							"name" : "glypenslot4",
							"type" : "slot",

							"x" : 14+3+46+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+3, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_04",
							"type" : "radio_button",

							"x" : 12+46+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_04,

							"children" :
							(
								{
									"name" : "Inventory_Tab_04_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "IV",
								},
							),
						},


						{
							"name" : "glyphe5bg",
							"type" : "image",
							"x" : 14+46+46+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_5.tga",
						},
						{
							"name" : "glypenslot5",
							"type" : "slot",

							"x" : 14+3+46+46+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+4, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_05",
							"type" : "radio_button",

							"x" : 12+46+46+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_05,

							"children" :
							(
								{
									"name" : "Inventory_Tab_05_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "V",
								},
							),
						},


						{
							"name" : "glyphe6bg",
							"type" : "image",
							"x" : 14+46+46+46+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_6.tga",
						},
						{
							"name" : "glypenslot6",
							"type" : "slot",

							"x" : 14+3+46+46+46+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+5, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_06",
							"type" : "radio_button",

							"x" : 12+46+46+46+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_06,

							"children" :
							(
								{
									"name" : "Inventory_Tab_06_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "VI",
								},
							),
						},


						{
							"name" : "glyphe7bg",
							"type" : "image",
							"x" : 14+46+46+46+46+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_7.tga",
						},
						{
							"name" : "glypenslot7",
							"type" : "slot",

							"x" : 14+3+46+46+46+46+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+6, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_07",
							"type" : "radio_button",

							"x" : 12+46+46+46+46+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_07,

							"children" :
							(
								{
									"name" : "Inventory_Tab_07_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "VII",
								},
							),
						},



						{
							"name" : "glyphe8bg",
							"type" : "image",
							"x" : 14+46+46+46+46+46+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_8.tga",
						},
						{
							"name" : "glypenslot8",
							"type" : "slot",

							"x" : 14+3+46+46+46+46+46+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+7, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_08",
							"type" : "radio_button",

							"x" : 12+46+46+46+46+46+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_08,

							"children" :
							(
								{
									"name" : "Inventory_Tab_08_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "VIII",
								},
							),
						},


						{
							"name" : "glyphe9bg",
							"type" : "image",
							"x" : 14+46+46+46+46+46+46+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_9.tga",
						},
						{
							"name" : "glypenslot9",
							"type" : "slot",

							"x" : 14+3+46+46+46+46+46+46+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+8, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_09",
							"type" : "radio_button",

							"x" : 12+46+46+46+46+46+46+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_09,

							"children" :
							(
								{
									"name" : "Inventory_Tab_09_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "IX",
								},
							),
						},

						{
							"name" : "glyphe10bg",
							"type" : "image",
							"x" : 14+46+46+46+46+46+46+46+46+46,
							"y" : 4,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg_10.tga",
						},
						{
							"name" : "glypenslot10",
							"type" : "slot",

							"x" : 14+3+46+46+46+46+46+46+46+46+46,
							"y" : 4+3,

							"width" : 32,
							"height" : 32,

							"image" : "d:/ymir work/ui/guildstorage/slot_glyphe_bg.tga",

							"slot" : (
										{"index":glyphenstartindex+9, "x":0, "y":0, "width":32, "height":32},
									),
						},
						{
							"name" : "Inventory_Tab_10",
							"type" : "radio_button",

							"x" : 12+46+46+46+46+46+46+46+46+46,
							"y" : 45,

							"default_image" :  ROOT_PATH + "Small_Button_01.sub",
							"over_image" :  ROOT_PATH + "Small_Button_02.sub",
							"down_image" :  ROOT_PATH + "Small_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Small_Button_03.sub",
							"tooltip_text" : uiScriptLocale.GUILDSTORAGE_PAGE_10,

							"children" :
							(
								{
									"name" : "Inventory_Tab_10_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "X",
								},
							),
						},

					),
				},
				{
					"name" : "glyphenbg",
					"type" : "image",
					"x" : 23,
					"y" : 320,


					"image" : "d:/ymir work/ui/guildstorage/optionbox_bg.tga",
					"children" : 
					(
						{
							"name" : "yang",
							"type" : "text",
					
							"x" : 20,
							"y" : 10,
							
							"r" : 1.0,
							"g" : 1.0,
							"b" : 1.0,

							"text" : uiScriptLocale.GUILD_MONEY,
					
							"children" :
							(
								{
									"name" : "yang_slot",
									"type" : "image",
				
									"x" : 43,
									"y" : -2,
				
									"image" : "d:/ymir work/ui/public/parameter_slot_04.sub",
								},
								{
									"name" : "yang_value",
									"type" : "text",
									##"type" : "editline",
									##"input_limit" : 12,
									##"width" : 90,
									##"height" : 20,
									"text" : "12345678 Yang",
									"x" : 43+3,
									"y" : 0,

									"r" : 1.0,
									"g" : 1.0,
									"b" : 1.0,
								},
							),
						},
						{
							"name" : "einzahlenbtn",
							"type" : "button",

							"x" : 192,
							"y" : 6,

							"default_image" :  ROOT_PATH + "Large_Button_01.sub",
							"over_image" :  ROOT_PATH + "Large_Button_02.sub",
							"down_image" :  ROOT_PATH + "Large_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Large_Button_03.sub",

							"children" :
							(
								{
									"name" : "einzahlenbtntxt",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",
									"r" : 1.0,
									"g" : 1.0,
									"b" : 1.0,

									"text" : uiScriptLocale.GUILDSTORAGE_PUT_GOLD,
								},
							),
						},
						{
							"name" : "auszahlenbtn",
							"type" : "button",
							"x" : 284,
							"y" : 6,

							"default_image" :  ROOT_PATH + "Large_Button_01.sub",
							"over_image" :  ROOT_PATH + "Large_Button_02.sub",
							"down_image" :  ROOT_PATH + "Large_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Large_Button_03.sub",

							"children" :
							(
								{
									"name" : "btntxt",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",
									"r" : 1.0,
									"g" : 1.0,
									"b" : 1.0,

									"text" : uiScriptLocale.GUILDSTORAGE_TAKE_GOLD,
								},
							),
						},
						{
							"name" : "optionenbtn",
							"type" : "button",

							"x" : 375,
							"y" : 6,

							"default_image" :  ROOT_PATH + "Large_Button_01.sub",
							"over_image" :  ROOT_PATH + "Large_Button_02.sub",
							"down_image" :  ROOT_PATH + "Large_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Large_Button_03.sub",

							"children" :
							(
								{
									"name" : "btntxt",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",
									"r" : 1.0,
									"g" : 1.0,
									"b" : 1.0,

									"text" : uiScriptLocale.GUILDSTORAGE_OPTIONS_BUTTON,
								},
							),
						},
						{
							"name" : "logButton",
							"type" : "button",

							"x" : 375+91,
							"y" : 6,

							"default_image" :  ROOT_PATH + "Large_Button_01.sub",
							"over_image" :  ROOT_PATH + "Large_Button_02.sub",
							"down_image" :  ROOT_PATH + "Large_Button_03.sub",
							"disable_image" :  ROOT_PATH + "Large_Button_03.sub",

							"children" :
							(
								{
									"name" : "btntxt",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",
									"r" : 1.0,
									"g" : 1.0,
									"b" : 1.0,

									"text" : uiScriptLocale.GUILDSTORAGE_LOG_BUTTON,
								},
							),
						},
					),
				},
			),
		},
	),
}