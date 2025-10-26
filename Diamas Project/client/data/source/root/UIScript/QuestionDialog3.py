import uiScriptLocale

window = {
	"name" : "QuestionDialog",
	"style" : ("moveable", "float",),

	"x" : SCREEN_WIDTH/2 - 125,
	"y" : SCREEN_HEIGHT/2 - 52,

	"width" : 280,
	"height" : 115+15,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",

			"x" : 0,
			"y" : 0,

			"width" : 280,
			"height" : 115+15,

			"children" :
			(
				{
					"name" : "message1",
					"type" : "text",

					"x" : 0,
					"y" : 30,

					"text" : uiScriptLocale.MESSAGE,

					"horizontal_align" : "center",
				},
				{
					"name" : "message2",
					"type" : "text",

					"x" : 0,
					"y" : 45,

					"text" : uiScriptLocale.MESSAGE,

					"horizontal_align" : "center",
				},
				{
					"name" : "message3",
					"type" : "text",

					"x" : 0,
					"y" : 45+15,

					"text" : uiScriptLocale.MESSAGE,

					"horizontal_align" : "center",
				},
				{
					"name" : "accept",
					"type" : "button",

					"x" : -80,
					"y" : 68+15,

					"width" : 81,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.CONTINUE,

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

					"x" : +80,
					"y" : 68+15,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.CANCEL,

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