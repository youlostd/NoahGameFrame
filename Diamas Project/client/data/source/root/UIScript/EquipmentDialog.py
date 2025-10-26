import uiScriptLocale
import item
RESOURCE = "d:/ymir work/ui/gui/inventory/"

window = {
	"name" : "EquipmentDialog",
	"style" : ("moveable", "float",),

	"x" : 0,
	"y" : 0,

	"width" : 180 + 140,
	"height" : (210 + 47 + 47 + 50),

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 180,
			"height" : 260,

			"title" : "Character Name",

			"children" :
			(
				{
					"name":"EquipmentBaseImage",
					"type":"image",
					"style" : ("attach",),

					"x" : 0,
					"y" : 25,
					"horizontal_align" : "center",
					"vertical_align" : "center",

					"image": RESOURCE + "equipment_bg.png",

					"children" :
					(

						{
							"name" : "EquipmentSlot",
							"type" : "slot",

							"x" : 3,
							"y" : 6,

							"width": 150,
							"height": 182,

							"slot" : (
								{"index": 0, "x": 39, "y": 37, "width": 32,
								 "height": 64},
								{"index": 1, "x": 39, "y": 2, "width": 32,
								 "height": 32},
								{"index": 2, "x": 39, "y": 145, "width": 32,
								 "height": 32},
								{"index": 3, "x": 75, "y": 67, "width": 32,
								 "height": 32},
								{"index": 4, "x": 3, "y": 3, "width": 32,
								 "height": 96},
								{"index": 5, "x": 114, "y": 67, "width": 32,
								 "height": 32},
								{"index": 6, "x": 114, "y": 35, "width": 32,
								 "height": 32},
								{"index": 7, "x": 2, "y": 145, "width": 32,
								 "height": 32},
								{"index": 8, "x": 75, "y": 145, "width": 32,
								 "height": 32},
								{"index": 9, "x": 114, "y": 2, "width": 32,
								 "height": 32},
								{"index": 10, "x": 75, "y": 35, "width": 32,
								 "height": 32},
								{"index": item.EQUIPMENT_RING1, "x": 2, "y": 106, "width": 32,
								 "height": 32},
								{"index": item.EQUIPMENT_RING2, "x": 75, "y": 106, "width": 32,
								 "height": 32},
								{"index": item.EQUIPMENT_BELT, "x": 39, "y": 106, "width": 32,
								 "height": 32},
									),
						},

					),

				},
			),
		},

		{
			"name": "board_costume",
			"type": "board",

			"x": 180,
			"y": 0,

			"width": 140,
			"height": (180 + 47 + 47 + 50),

			"children":
				(
					## Title
					{
						"name": "TitleBarCostume",
						"type": "titlebar",
						"style": ("attach",),

						"x": 6,
						"y": -3,

						"width": 130,
						"r": 0.95, "g": 0.71, "b": 0.04, "a": 1,
                        "title" : uiScriptLocale.COSTUME_WINDOW_TITLE,

		
					},

					## Equipment Slot
					{
						"name": "Costume_Base",
						"type": "expanded_image",

						"x": 13,
						"y": 38,

						"image": "d:/ymir work/ui/costume/new_costume_bg.jpg",

						"children":
							(

								{
									"name": "CostumeSlot",
									"type": "slot",

									"x": 3,
									"y": 3,

									"width": 131,
									"height": 155 + 47 + 50,

									"slot": (
										{"index": 0, "x": 62, "y": 45, "width": 32,
										 "height": 64},
										{"index": 1, "x": 62, "y": 9, "width": 32,
										 "height": 32},
										{"index": 2, "x": 13, "y": 14, "width": 32,
										 "height": 96},
										{"index": 3, "x": 13, "y": 127, "width": 32,
										 "height": 32},
										{"index": 4, "x": 13, "y": 177, "width": 32,
										 "height": 32},
										{"index": 5, "x": 62, "y": 176, "width": 32,
										 "height": 32},
										{"index": 6, "x": 62, "y": 126, "width": 32,
										 "height": 32},
										{"index": 7, "x": 13, "y": 176 + 50, "width": 32,
										 "height": 32},
									),
								},
							),
					},

				),
		},

	),
}
