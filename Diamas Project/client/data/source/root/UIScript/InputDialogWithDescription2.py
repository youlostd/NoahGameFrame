import uiScriptLocale

TEMPORARY_HEIGHT = 16

window = {
	"name" : "InputDialog_WithDescription",

	"x" : 0,
	"y" : 0,

	"style" : ("moveable", "float",),

	"width" : 170,
	"height" : 136 + TEMPORARY_HEIGHT,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 170,
			"height" : 136 + TEMPORARY_HEIGHT,

			"title" : "",

			"children" :
			(

				## Text
				{
					"name" : "Description1",
					"type" : "text",
					"text" : "",
					"horizontal_align" : "center",
					"horizontal_align" : "center",
					"x" : 0,
					"y" : 34,
				},
				{
					"name" : "Description2",
					"type" : "text",
					"text" : "",
					"horizontal_align" : "center",
					"horizontal_align" : "center",
					"x" : 0,
					"y" : 34 + TEMPORARY_HEIGHT,
				},

				## Input Slot
				{
					"name" : "InputSlot",
					"type" : "slotbar",

					"x" : 0,
					"y" : 51 + TEMPORARY_HEIGHT,
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

				## Button
				{
					"name" : "AcceptButton",
					"type" : "button",

					"x" : -80,
					"y" : 84 + TEMPORARY_HEIGHT,
					"horizontal_align" : "center",

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
					"name" : "CancelButton",
					"type" : "button",

					"x" : 82,
					"y" : 84 + TEMPORARY_HEIGHT,
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