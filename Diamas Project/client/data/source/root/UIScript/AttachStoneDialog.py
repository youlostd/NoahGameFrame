import uiScriptLocale

window = {
	"name" : "AttachStoneDialog",
	"style" : ("moveable", "float",),

	"x" : 0,
	"y" : 0,

	"width" : 0,
	"height" : 0,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width" : 0,
			"height" : 0,
			"children" :
			(
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : -3,

					"width" : 0,
					"color" : "yellow",

					"r":0.95,"g":0.71,"b":0.04,"a":1,
					"title" : uiScriptLocale.ATTACH_METIN_TITLE,

				},
				{
					"name" : "AttachMetinArrow",
					"type" : "image",
					"x" : 0,
					"y" : -33,
					"horizontal_align" : "center",
					"vertical_align" : "center",
					"image" : "d:/ymir work/ui/game/windows/attach_metin_arrow.sub",
				},

				{
					"name" : "Question",
					"type" : "text",
					"text" : uiScriptLocale.ATTACH_METIN_INFO,
					"vertical_align" : "bottom",
					"horizontal_align" : "center",
					"x" : 0,
					"y" : 42,
				},
				{
					"name" : "AcceptButton",
					"type" : "button",

					"x" : -85,
					"y" : 13,

					"text" : uiScriptLocale.YES,
					"horizontal_align" : "center",
					"vertical_align" : "bottom",

					"text_height": -5,
					"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
					'x_scale': 1.0,
					'y_scale': 1.0,
				},
				{
					"name" : "CancelButton",
					"type" : "button",

					"x" : 85,
					"y" : 13,

					"text" : uiScriptLocale.NO,
					"horizontal_align" : "center",
					"vertical_align" : "bottom",
					"text_height": -5,
					"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
					'x_scale': 1.0,
					'y_scale': 1.0,
				},
			),
		},
	),
}