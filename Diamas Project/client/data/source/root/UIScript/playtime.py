import uiScriptLocale

ROOT = "d:/ymir work/ui/game/"
APPRECIATION = "d:/ymir work/effect/etc/rutbeler/"
ROOT_PATH = "d:/ymir work/ui/public/"
GOLD_COLOR	= 0xFFFEE3AE

BOARD_WIDTH = 250
BOARD_HEIGTH = 175

BACKGROUND_WITDH = BOARD_WIDTH-15
BACKGROUND_HEIGHT = BOARD_HEIGTH-50

window = {
	"name" : "PlayTime",
	'style': ('moveable', 'float', 'animated_board', ),
	"type": "window",
	"x" : 0,
	"y" : 0,

	"width" : BOARD_WIDTH,
	"height" : BOARD_HEIGTH,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",

			"x" : 0,
			"y" : 0,

			"width" : BOARD_WIDTH,
			"height" : BOARD_HEIGTH,

			"children" :
			(
				## Title
				{
					"name" : "titlebar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : 0,

					"width" : BOARD_WIDTH-12,
					"color": 0xCCCCCCC,

					"children" :
					(
						{ "name":"titlename", "type":"text", "x":0, "y":10, 
						"text" : "Rütbe Sistemi", 
						"outline": True,
						"color": 0xCCCCCCC,
						"horizontal_align":"center", "text_horizontal_align":"center" },
					),
				},

				{
					"name" : "board_back",
					"type" : "border_a",

					"width" : BOARD_WIDTH - 20,
					"height" : BOARD_HEIGTH - 40,
					"x" : 10,
					"y" : 30,
				},

				{
					"name" : "time_main",
					"type" : "image",


					"x" : 20,
					"y" : 50,


					"image" : APPRECIATION + "title.png",
					
					"children" :
					(
						{
							"name" : "text",
							"type" : "text",

							"x" : 0,
							"y" : 0,

							"text" : "Şuanki Rütben",
							
							"color": GOLD_COLOR,
							"all_align" : "center",
						},
						
						{ "name" : "time_Image", "type" : "image", "x" : 35+7, "y" : 20+4, "image" : APPRECIATION + "rutbe_1.png", },
						{ "name" : "time_Slot", "type" : "image", "x" : 31+7, "y" : 20, "image" : APPRECIATION + "box_app.png", },
					),
				},

				{ "name" : "time_Next", "type" : "image", "x" : 50+35+35, "y" : 25+50, "image" : APPRECIATION + "next.png", },

				{
					"name" : "time_next",
					"type" : "image",


					"x" : 25+105,
					"y" : 50,


					"image" : APPRECIATION + "title.png",
					
					"children" :
					(
						{
							"name" : "text",
							"type" : "text",

							"x" : 0,
							"y" : 0,

							"text" : "Sonraki Rütben",
							
							"color": GOLD_COLOR,
							"all_align" : "center",
						},
						
						{ "name" : "time_Next_Image", "type" : "image", "x" : 35+7, "y" : 20+4, "image" : APPRECIATION + "rutbe_1.png", },
						{ "name" : "time_Slot", "type" : "image", "x" : 31+7, "y" : 20, "image" : APPRECIATION + "box_app.png", },
					),
				},
				
				{
					"name": "my_playtime",
					"type": "text",
					
					"x" : 35,
					"y" : 115,
					
					"text" : "Şuanki dakikan: 1 dk",
					
					"color": GOLD_COLOR,
				},
				
				{
					"name": "my_next_playtime",
					"type": "text",
					
					"x" : 15,
					"y" : 115+25,
					
					"text" : "Sonraki rütne için kalan süre: 1 dk",
					
					"color": GOLD_COLOR,
				}
				
			),
		},
	),
}
