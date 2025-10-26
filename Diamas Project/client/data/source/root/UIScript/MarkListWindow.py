import uiScriptLocale

window = {
	"name" : "MarkListWindow",

	"x" : SCREEN_WIDTH - 270,
	"y" : SCREEN_HEIGHT - 400 - 50,

	"style" : ("moveable", "float",),

	"width" : 220,
	"height" : 360,

	"children" :
	(

		{
			"name" : "board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 220,
			"height" : 360,
			"title" : uiScriptLocale.MARKLIST_TITLE,
		},

		{
			"name" : "ScrollBar",
			"type" : "scrollbar",

			"x" : 27,
			"y" : 40,
			"size" : 220,
			"horizontal_align" : "right",
		},

		{
			"name" : "ok",
			"type" : "button",

			"x" : 10,
			"y" : 265,

			"width" : 61,
			"height" : 21,

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
			"name" : "cancel",
			"type" : "button",

			"x" : 90,
			"y" : 265,

			"width" : 41,
			"height" : 21,

			"text" : uiScriptLocale.CANCEL,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
		},

		{
			"name" : "refresh",
			"type" : "button",

			"x" : 55,
			"y" : 305,

			"width" : 41,
			"height" : 21,

			"text" : uiScriptLocale.MARKLIST_REFRESH,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
		},
	)
}