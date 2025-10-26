import uiScriptLocale
import localeInfo

window = {
	"name" : "QuestionDialog",
	"style" : ("moveable", "float",),

	"x" : SCREEN_WIDTH/2 - 125,
	"y" : SCREEN_HEIGHT/2 - 52,

	"width" : 340,
	"height" : 80,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board_with_titlebar",
			"close_button": False,

			"x" : 0,
			"y" : 0,

			"width" : 340,
			"height" : 80,
            "title": "Sınıf Seçin",
			"children" :
			(
				{
					"name" : "message",
					"type" : "text",

					"x" : 0,
					"y" : 40,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.MESSAGE,

				},
				{
					"name" : "accept1",
					"type" : "button",

					"x" : -155,
					"y" : 43,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.YES,
                    "default_image": "d:/ymir work/ui/dragonsoul/button_01.tga",
                    "over_image": "d:/ymir work/ui/dragonsoul/button_02.tga",
                    "down_image": "d:/ymir work/ui/dragonsoul/button_03.tga",
                    'x_scale': 1.0,
                    'y_scale': 1.0,
				},
				{
					"name" : "accept2",
					"type" : "button",

					"x" : 0,
					"y" : 43,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.YES,
                    "default_image": "d:/ymir work/ui/dragonsoul/button_01.tga",
                    "over_image": "d:/ymir work/ui/dragonsoul/button_02.tga",
                    "down_image": "d:/ymir work/ui/dragonsoul/button_03.tga",
                    'x_scale': 1.0,
                    'y_scale': 1.0,
				},
				{
					"name" : "cancel",
					"type" : "button",

					"x" : 155,
					"y" : 43,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : uiScriptLocale.NO,
                    "default_image": "d:/ymir work/ui/dragonsoul/button_01.tga",
                    "over_image": "d:/ymir work/ui/dragonsoul/button_02.tga",
                    "down_image": "d:/ymir work/ui/dragonsoul/button_03.tga",
                    'x_scale': 1.0,
                    'y_scale': 1.0,
				},
			),
		},
	),
}
