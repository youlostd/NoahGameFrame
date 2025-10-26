import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/public/"
BOARD_WIDTH = 600
BOARD_HEIGHT = 356

window = {
	"name" : "boniswitcher",
	"style" : ("moveable", "float",),

	"x" : (SCREEN_WIDTH - BOARD_WIDTH) / 2,
	"y" : (SCREEN_HEIGHT - BOARD_HEIGHT) / 2,

	"width" : BOARD_WIDTH,
	"height" : BOARD_HEIGHT,
	
	"children" :
	(
		{
			"name" : "board",
			"type" : "thinboard",
			
			"x" : 0,
			"y" : 0,
			
			"width" : BOARD_WIDTH,
			"height" : BOARD_HEIGHT,
			
			"children" :
			(
				{
					"name" : "titlebar",
					"type" : "titlebar",
					"style" : ("attach",),
					"color": "yellow",

					"x" : -3,
					"y" : 0,
					
					"width" : BOARD_WIDTH+3,
					
					"title" : "Switch Bonus",

				},
			),
		},
	),
}
