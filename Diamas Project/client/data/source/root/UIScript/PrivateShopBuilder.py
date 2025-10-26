import uiScriptLocale

window = {
	"name" : "PrivateShopBuilder",

	"x" : 0,
	"y" : 0,

	"style" : ("moveable", "float",),

	"width" : 450,
	"height" : 600,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width" : 368 - 24,
			"height" : 358 + 26 + 32,

			"children" :
			(
				## Title
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : 8,

					"width" : 368 - 24 - 8,
					"color" : "gray",
                    "title": uiScriptLocale.PRIVATE_SHOP_TITLE,

				},

				## Name_Static
				#{
				#	"name" : "Name_Static", "type" : "text", "x" : 15, "y" : 35 + 3, "text" : uiScriptLocale.PRIVATE_SHOP_NAME,
				#},
				## Name
				{
					"name" : "NameSlot",
					"type" : "slotbar",
					"x" : 13,
					"y" : 55,
					"width" : 368 - 24 - 25,
					"height" : 18,

					"children" :
					(
						{
							"name" : "NameLine",
							"type" : "text",
							"x" : 3,
							"y" : 3,
							"width" : 157,
							"height" : 15,
							"input_limit" : 25,
							"text" : "1234567890123456789012345",
						},
					),
				},

				## Item Slot
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 12,
					"y" : 54 + 26,

					"start_index" : 0,
					"x_count" : 10,
					"y_count" : 9,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub",
				},

				## Ok
				{
					"name" : "OkButton",
					"type" : "button",

					"x" : 21 * 3,
					"y" : 315 + 26 + 32,

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

				## Close
				{
					"name" : "CloseButton",
					"type" : "button",

					"x" : 104 * 2,
					"y" : 315 + 26 + 32,

					"width" : 61,
					"height" : 21,

					"text" : uiScriptLocale.CLOSE,

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