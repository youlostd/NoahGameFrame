import uiScriptLocale

window = {
	"name" : "SliderDialog",

	"x" : 100,
	"y" : 100,

	"style" : ("moveable", "float",),

	"width" : 170,
	"height" : 123,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 170,
			"height" : 123,
			"title" : "Slider",

			"children" :
			(

				## Money Slot
				{
					"name" : "money_slot",
					"type" : "image",

					"x" : 20,
					"y" : 47,

					"image" : "d:/ymir work/ui/public/Parameter_Slot_02.sub",

					"children" :
					(
						{
							"name" : "money_value",
							"type" : "editline",

							"x" : 3,
							"y" : 1,

							"width" : 60,
							"height" : 18,

							"input_limit" : 6,
							"only_number" : 1,

							"text" : "1",
						},
						{
							"name" : "max_value",
							"type" : "text",

							"x" : 63,
							"y" : 0,

							"text" : "/ 999999",
						},
					),
				},

				## Button
				{
					"name" : "accept_button",
					"type" : "button",

					"x" : 146/2 - 61 - 5,
					"y" : 73,

					"text" : uiScriptLocale.OK,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
				},
				{
					"name" : "cancel_button",
					"type" : "button",

					"x" : 170/2 + 5,
					"y" : 73,

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