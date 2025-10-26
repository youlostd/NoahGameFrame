import uiScriptLocale

ROOT = "d:/ymir work/ui/minimap/"

window = {
	"name" : "AtlasWindow",
	"style" : ("moveable", "float",),

	"x" : SCREEN_WIDTH - 136 - 256 - 10,
	"y" : 25,

	"width" : 389,
	"height" : 490,

	"children" :
	(
		## BOARD
		{
			"name" : "board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 389,
			"height" : 490,


            "x" : 0,
			"y" : -2,
            "color": 0x000000,
			"title" : "Eşya Ön izleme",
		},
		
		{
			"name"	:	"item_renderer",
			"type"	:	"item_render_target",
			
			"x"	:	10,
			"y"	:	40,
			
			"width"	:	370,
			"height"	:	440,
			
			"index"	: 0
			
		}
	),
}
