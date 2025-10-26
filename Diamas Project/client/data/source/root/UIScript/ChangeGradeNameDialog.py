import uiScriptLocale

window = {
	"name" : "ChangeGradeNameDialog",

	"x" : 0,
	"y" : 0,

	"style" : ("moveable", "float",),

	"width" : 190,
	"height" : 120,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 190,
			"height" : 120,

			"title" : uiScriptLocale.GUILD_GRADE_CHANGE_GRADE_NAME,

			"children" :
			(

				## GradeName Slot
				{
					"name" : "GradeNameSlot",
					"type" : "image",

					"x" : 0,
					"y" : 47,
					"horizontal_align" : "center",

					"image" : "d:/ymir work/ui/public/Parameter_Slot_02.sub",

					"children" :
					(
						{
							"name" : "GradeNameValue",
							"type" : "editline",

							"x" : 3,
							"y" : 3,

							"width" : 60,
							"height" : 18,

							"input_limit" : 8,
						},
					),
				},

				## Button
				{
					"name" : "AcceptButton",
					"type" : "button",

					"x" : 170/2 - 61 - 5,
					"y" : 68,

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

					"x" : 185/2 + 5,
					"y" : 68,

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