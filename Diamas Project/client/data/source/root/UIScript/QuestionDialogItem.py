import uiScriptLocale

window = {
	"name" : "QuestionDialog",
	"style" : ("moveable", "float",),

	"x" : SCREEN_WIDTH/2 - 125,
	"y" : SCREEN_HEIGHT/2 - 52,

	"width" : 340,
	"height" : 115,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",

			"x" : 0,
			"y" : 0,

			"width" : 340,
			"height" : 115,

			"children" :
			(
				{
					"name" : "message",
					"type" : "text",

					"x" : 0,
					"y" : -20,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.MESSAGE,

					"horizontal_align" : "center",
					"vertical_align" : "center",
				},
				{
					"name" : "accept",
					"type" : "button",

					"x" : -150,
					"y" : 63,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.YES,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
				},
				{
					"name" : "destroy",
					"type" : "button",

					"x" : 0,
					"y" : 63,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.DESTROY,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
				},
				{
					"name" : "cancel",
					"type" : "button",

					"x" : 150,
					"y" : 63,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.NO,

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