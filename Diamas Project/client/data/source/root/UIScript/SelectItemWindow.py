import uiScriptLocale

window = {
	"name" : "SelectItemWindow",

	"x" : 100,
	"y" : 20,

	"style" : ("moveable", "float",),

	"width" : 184,
	"height" : 342,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",

			"x" : 0,
			"y" : 0,

			"width" : 184,
			"height" : 342,

			"children" :
			(
				## Title
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : 0,

					# "width" : 169,
					# "color": "yellow",
                    "title": uiScriptLocale.SELECT_METIN_STONE_TITLE,

				},

				## Item Slot
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 12,
					"y" : 34,

					"start_index" : 0,
					"x_count" : 5,
					"y_count" : 8,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub",
				},

				## Button
				{
					"name" : "ExitButton",
					"type" : "button",

					"x" : 0,
					"y" : 14,

					"text" : uiScriptLocale.CLOSE,
					"horizontal_align" : "center",
					"vertical_align" : "bottom",

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
