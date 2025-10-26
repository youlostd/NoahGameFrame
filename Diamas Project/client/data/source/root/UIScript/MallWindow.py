import uiScriptLocale

window = {
	"name" : "SafeboxWindow",

	"x" : 100,
	"y" : 20,

	"style" : ("moveable", "float",),

	"width" : 176,
	"height" : 415,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",

			"x" : 0,
			"y" : 0,

			"width" : 176,
			"height" : 415,

			"children" :
			(
				## Title
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : 0,
                    
					"width" : 161,
                    "title": uiScriptLocale.MALL_TITLE,

				},

				## Button
				{
					"name" : "ExitButton",
					"type" : "button",

					"x" : 0,
					"y" : 30,

					"text" : uiScriptLocale.CLOSE,
					"horizontal_align" : "center",
					"vertical_align" : "bottom",

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
				},

			),
		},
	),
}
