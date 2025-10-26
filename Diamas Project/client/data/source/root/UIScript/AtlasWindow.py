import uiScriptLocale

ROOT = "d:/ymir work/ui/minimap/"

window = {
	"name" : "AtlasWindow",
	"style" : ("moveable", "float",),

	"x" : SCREEN_WIDTH - 136 - 256 - 10 + 10,
	"y" : 25,

	"width" : 256,
	"height" : 256 + 38 + 20,

	"children" :
	(
		## BOARD
		{
			"name" : "board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 256,
			"height" : 256 + 38 + 20 + 10,


            "x" : 0,
			"y" : -2,
            "color": 0x000000,
			"title" : uiScriptLocale.ZONE_MAP,
			"children" :
			(
			),
		},
	),
}
