import uiScriptLocale

window = {
	"name" : "InputWithColor",

	"x" : 0,
	"y" : 0,

	"style" : ("moveable", "float",),

	"width" : 400,
	"height" : 186,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 270,
			"height" : 186,

			"title" : "",

			"children" :
			(

				## Text
				{
					"name" : "Description",
					"type" : "text",
					"text" : "",
					"horizontal_align" : "center",
					"x" : 0,
					"y" : 40,
				},

				## Input Slot
				{
					"name" : "InputSlot",
					"type" : "slotbar",

					"x" : 0,
					"y" : 55,
					"width" : 90,
					"height" : 18,
					"horizontal_align" : "center",

					"children" :
					(
						{
							"name" : "InputValue",
							"type" : "editline",

							"x" : 3,
							"y" : 3,

							"width" : 90,
							"height" : 18,

							"input_limit" : 12,
						},
					),
				},

				{
					'name': 'Preview',
					'type': 'bar',
					"horizontal_align" : "center",

					'x': 0,
					'y':  71,
					'width': 88,
					'height': 23
				},
				
				## Button
				{
					"name" : "AcceptButton",
					"type" : "button",

					"x" : -165,
					"y" : 114,
					"horizontal_align" : "center",

					"text" : uiScriptLocale.OK,

					# "text_height": -5,
					# "text_color": 0xffffc539,
					'default_image': "d:/ymir work/ui/dragonsoul/button_01.tga",
					'over_image': "d:/ymir work/ui/dragonsoul/button_02.tga",
					'down_image': "d:/ymir work/ui/dragonsoul/button_03.tga",
					'x_scale': 1.0,
					'y_scale': 1.0,
				},				
				## Preview Button
				{
					"name" : "PreviewButton",
					"type" : "button",

					"x" : 0,
					"y" : 114,
					"horizontal_align" : "center",

					"text" : uiScriptLocale.GUILD_BUILDING_PREVIEW,

					# "text_height": -5,
					# "text_color": 0xffffc539,
					'default_image': "d:/ymir work/ui/dragonsoul/button_01.tga",
					'over_image': "d:/ymir work/ui/dragonsoul/button_02.tga",
					'down_image': "d:/ymir work/ui/dragonsoul/button_03.tga",
					'x_scale': 1.0,
					'y_scale': 1.0,
				},
				{
					"name" : "CancelButton",
					"type" : "button",

					"x" : 170, 
					"y" : 114,
					"horizontal_align" : "center",

					"text" : uiScriptLocale.CANCEL,

					# "text_height": -5,
					# "text_color": 0xffffc539,
					'default_image': "d:/ymir work/ui/dragonsoul/button_01.tga",
					'over_image': "d:/ymir work/ui/dragonsoul/button_02.tga",
					'down_image': "d:/ymir work/ui/dragonsoul/button_03.tga",
					'x_scale': 1.0,
					'y_scale': 1.0,
				},
				{
					"name": "ExtraButton",
					"type": "button",

					"x": 115 + 55,
					"y": 75,
					"horizontal_align": "center",

					"text": uiScriptLocale.COLOR_PICKER_TITLE2,

					# "text_height": -5,
					# "text_color": 0xffffc539,
					'default_image': "d:/ymir work/ui/dragonsoul/button_01.tga",
					'over_image': "d:/ymir work/ui/dragonsoul/button_02.tga",
					'down_image': "d:/ymir work/ui/dragonsoul/button_03.tga",
					'x_scale': 1.0,
					'y_scale': 1.0,
				},
			),
		},
	),
}