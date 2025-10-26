import uiScriptLocale

TEMPORARY_HEIGHT = 16

window = {
	"name" : "DoubleInputDialog_WithDescription",

	"x" : 0,
	"y" : 0,

	"style" : ("moveable", "float",),

	"width" : 170,
	"height" : 193,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 170,
			"height" : 193,

			"title" : "",

			"children" :
			(
				## Input Slot 1
				{
					"name" : "InputSlot1",
					"type" : "slotbar",

					"x" : 0,
					"y" : 80,
					"width" : 90,
					"height" : 18,
					"horizontal_align" : "center",

					"children" :
					(
						{
							"name" : "InputValue1",
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
					"name" : "Description1",
					"type" : "text",
					"text" : "",
					"horizontal_align" : "center",
					"horizontal_align" : "center",
					"x" : 0,
					"y" : 60,
				},

				## Input Slot 2
				{
					"name" : "InputSlot2",
					"type" : "slotbar",

					"x" : 0,
					"y" : 123,
					"width" : 90,
					"height" : 18,
					"horizontal_align" : "center",

					"children" :
					(
						{
							"name" : "InputValue2",
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
					"name" : "Description2",
					"type" : "text",
					"text" : "",
					"horizontal_align" : "center",
					"horizontal_align" : "center",
					"x" : 0,
					"y" : 103,
				},

				## Button
				{
					"name" : "AcceptButton",
					"type" : "button",

					"x" : -61 - 70,
					"y" : 150,
					"horizontal_align" : "center",

					"text" : uiScriptLocale.OK,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
				{
					"name" : "CancelButton",
					"type" : "button",

					"x" : 70,
					"y" : 150,
					"horizontal_align" : "center",

					"text" : uiScriptLocale.CANCEL,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},

			),
		},
	),
}