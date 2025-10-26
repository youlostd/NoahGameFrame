import uiScriptLocale

ROOT = "d:/ymir work/ui/public/"

window = {
	"name" : "RestartDialog",
	"style" : ("float",),

	"x" : 200,
	"y" : 50,

	"width" : 205,
	"height" : 105,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board_with_titlebar",
			"style": ("attach",),
			"close_button": False,

			"title": "Restart",

			"x" : 0,
			"y" : 0,

			"width" : 205,
			"height" : 130,

			"r" : 0.3333,
			"g" : 0.2941,
			"b" : 0.2588,
			"a" : 1.0,

			"children" :
			(
				{
					"name" : "restart_here_button",
					"type" : "button",

					"x" : 10,
					"y" : 50,

					"text" : uiScriptLocale.RESTART_HERE,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 1.0,
                    'y_scale': 1.0,
				},
				{
					"name" : "restart_town_button",
					"type" : "button",

					"x" : 10,
					"y" : 82,

					"text" : uiScriptLocale.RESTART_TOWN,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 1.0,
                    'y_scale': 1.0,
				},
			),
		},
	),
}
